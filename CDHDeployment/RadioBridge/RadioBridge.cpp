// ======================================================================
// \title  RadioBridge.cpp
// \author madisonw
// \brief  Transmits F' telemetry over RF via Direwolf KISS TCP socket
// ======================================================================

#include "CDHDeployment/RadioBridge/RadioBridge.hpp"
#include "Fw/Types/Assert.hpp"
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace RadioBridge {

static constexpr int  KISS_PORT  = 8001;
static constexpr char KISS_HOST[] = "127.0.0.1";
static constexpr U8   KISS_FEND  = 0xC0;
static constexpr U8   KISS_FESC  = 0xDB;
static constexpr U8   KISS_TFEND = 0xDC;
static constexpr U8   KISS_TFESC = 0xDD;
static constexpr U8   KISS_DATA  = 0x00;

RadioBridge::RadioBridge(const char* const compName)
    : RadioBridgeComponentBase(compName) {
    printf("[RadioBridge] Initialized — using KISS socket to Direwolf (port %d)\n", KISS_PORT);
}

RadioBridge::~RadioBridge() {}

void RadioBridge::dataIn_handler(
    FwIndexType portNum,
    Fw::Buffer& fwBuffer,
    const ComCfg::FrameContext& context
) {
    if (fwBuffer.getData() == nullptr || fwBuffer.getSize() == 0) {
        Fw::LogStringArg err("Invalid buffer");
        this->log_WARNING_HI_RADIO_TX_FAILED(err);
        this->dataReturnOut_out(0, fwBuffer, context);
        return;
    }

    this->log_ACTIVITY_LO_FrameReceived(static_cast<U32>(fwBuffer.getSize()));
    this->log_ACTIVITY_LO_RADIO_TX_STARTED();

    if (transmitKISS(fwBuffer.getData(), fwBuffer.getSize())) {
        this->log_ACTIVITY_HI_RADIO_TX_SUCCESS();
    } else {
        Fw::LogStringArg err("KISS transmit failed");
        this->log_WARNING_HI_RADIO_TX_FAILED(err);
    }

    this->dataReturnOut_out(0, fwBuffer, context);
}

bool RadioBridge::transmitKISS(const U8* data, FwSizeType size) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("[RadioBridge] socket() failed\n");
        return false;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(KISS_PORT));
    inet_pton(AF_INET, KISS_HOST, &addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        printf("[RadioBridge] connect() to Direwolf KISS port failed\n");
        close(sock);
        return false;
    }

    // KISS frame: FEND + DATA(port 0) + <KISS-escaped AX.25 frame> + FEND
    // Worst case every byte escapes → 2*size + 3 overhead bytes
    FwSizeType kissSize = size * 2 + 3;
    U8* kissBuf = new U8[kissSize];
    FwSizeType k = 0;

    kissBuf[k++] = KISS_FEND;
    kissBuf[k++] = KISS_DATA;

    for (FwSizeType i = 0; i < size; i++) {
        if (data[i] == KISS_FEND) {
            kissBuf[k++] = KISS_FESC;
            kissBuf[k++] = KISS_TFEND;
        } else if (data[i] == KISS_FESC) {
            kissBuf[k++] = KISS_FESC;
            kissBuf[k++] = KISS_TFESC;
        } else {
            kissBuf[k++] = data[i];
        }
    }

    kissBuf[k++] = KISS_FEND;

    ssize_t sent = write(sock, kissBuf, k);
    close(sock);
    delete[] kissBuf;

    if (sent != static_cast<ssize_t>(k)) {
        printf("[RadioBridge] write() incomplete: sent %zd of %lu\n", sent, k);
        return false;
    }

    printf("[RadioBridge] Sent %lu-byte KISS frame (%lu payload) to Direwolf\n", k, size);
    return true;
}

} // namespace RadioBridge
