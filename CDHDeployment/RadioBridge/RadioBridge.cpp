// ======================================================================
// \title  RadioBridge.cpp
// \author madisonw
// \brief  Component that receives AX.25 frames and transmits via Direwolf/SR105U
// ======================================================================

#include "CDHDeployment/RadioBridge/RadioBridge.hpp"
#include "Fw/Types/Assert.hpp"
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace RadioBridge {

RadioBridge::RadioBridge(const char* const compName)
    : RadioBridgeComponentBase(compName) {
    printf("\n========================================\n");
    printf("RadioBridge Component Initialized!\n");
    printf("Ready to receive AX.25 frames\n");
    printf("========================================\n\n");
}

RadioBridge::~RadioBridge() {}

void RadioBridge::dataIn_handler(
    FwIndexType portNum,
    Fw::Buffer& fwBuffer,
    const ComCfg::FrameContext& context
) {
    printf("\n========================================\n");
    printf("RadioBridge::dataIn_handler CALLED!\n");
    printf("Received AX.25 frame, size: %lu bytes\n", fwBuffer.getSize());
    printf("========================================\n");

    if (fwBuffer.getData() == nullptr || fwBuffer.getSize() == 0) {
        printf("ERROR: Invalid buffer received\n");
        Fw::LogStringArg errorStr("Invalid buffer");
        this->log_WARNING_HI_RADIO_TX_FAILED(errorStr);
        this->dataReturnOut_out(0, fwBuffer, context);
        return;
    }

    this->log_ACTIVITY_LO_FrameReceived(static_cast<U32>(fwBuffer.getSize()));

    printf("\nReceived AX.25 frame (hex):\n");
    const U8* data = fwBuffer.getData();
    for (FwSizeType i = 0; i < fwBuffer.getSize(); i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n\n");

    this->log_ACTIVITY_LO_RADIO_TX_STARTED();

    if (transmitAX25Frame(data, fwBuffer.getSize())) {
        this->log_ACTIVITY_HI_RADIO_TX_SUCCESS();
        printf("[RadioBridge] Transmission successful!\n\n");
    } else {
        Fw::LogStringArg errorStr("RF transmission failed");
        this->log_WARNING_HI_RADIO_TX_FAILED(errorStr);
        printf("[RadioBridge] Transmission failed!\n\n");
    }

    this->dataReturnOut_out(0, fwBuffer, context);
}

bool RadioBridge::transmitAX25Frame(const U8* data, FwSizeType size) {
    printf("\n========== TRANSMITTING AX.25 FRAME ==========\n");
    printf("Frame size: %lu bytes\n", size);

    if (size < 20) {
        printf("ERROR: Frame too small to be valid AX.25\n");
        return false;
    }

    if (data[0] != 0x7E || data[size-1] != 0x7E) {
        printf("ERROR: Invalid frame flags\n");
        return false;
    }

    std::string destCall = decodeCallsign(&data[1]);
    U8 destSSID = (data[7] >> 1) & 0x0F;
    std::string srcCall = decodeCallsign(&data[8]);
    U8 srcSSID = (data[14] >> 1) & 0x0F;

    printf("Parsed addresses:\n");
    printf("  Source:      %s-%d\n", srcCall.c_str(), srcSSID);
    printf("  Destination: %s-%d\n", destCall.c_str(), destSSID);

    // Assert PTT: switch SR105U to TX mode on 434.9 MHz
    assertPTT(true);
    printf("PTT asserted (TX mode)\n");

    // Open TCP connection to local Direwolf KISS server on port 8001.
    // Direwolf encodes the raw AX.25 frame as AFSK audio and plays it to the
    // Pi's audio device (plughw:2,0 → SR105U audio input → FM TX at 434.9 MHz).
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("ERROR: socket() failed: %s\n", strerror(errno));
        assertPTT(false);
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8001);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        printf("ERROR: connect() to Direwolf KISS failed: %s\n", strerror(errno));
        close(sock);
        assertPTT(false);
        return false;
    }

    // Build KISS frame.
    // Strip outer AX.25 flags (data[0] and data[size-1]) and FCS (data[size-3..size-2]).
    // Direwolf recomputes and appends FCS when encoding for transmission.
    // KISS byte stuffing: 0xC0 → 0xDB 0xDC, 0xDB → 0xDB 0xDD.
    const U8* frameContent = &data[1];
    FwSizeType frameLen = size - 4; // remove leading flag, FCS(2), trailing flag

    std::vector<U8> kissFrame;
    kissFrame.reserve(frameLen + 4);
    kissFrame.push_back(0xC0); // FEND
    kissFrame.push_back(0x00); // CMD: channel 0, data frame
    for (FwSizeType i = 0; i < frameLen; i++) {
        if (frameContent[i] == 0xC0) {
            kissFrame.push_back(0xDB);
            kissFrame.push_back(0xDC);
        } else if (frameContent[i] == 0xDB) {
            kissFrame.push_back(0xDB);
            kissFrame.push_back(0xDD);
        } else {
            kissFrame.push_back(frameContent[i]);
        }
    }
    kissFrame.push_back(0xC0); // FEND

    ssize_t sent = write(sock, kissFrame.data(), kissFrame.size());
    close(sock);

    if (sent != static_cast<ssize_t>(kissFrame.size())) {
        printf("ERROR: Incomplete KISS write (%zd of %lu bytes)\n",
               sent, kissFrame.size());
        assertPTT(false);
        return false;
    }

    printf("Sent %lu-byte KISS frame to Direwolf (raw frame: %lu bytes)\n",
           kissFrame.size(), frameLen);

    // Hold PTT for the duration Direwolf needs to encode and transmit the audio.
    // At 1200 bps AFSK: each byte = 8 bits @ ~6.67 ms; add 1.5 s for preamble/postamble.
    double txSeconds = (static_cast<double>(size) * 8.0 / 1200.0) + 1.5;
    printf("Holding PTT for %.1f seconds...\n", txSeconds);
    usleep(static_cast<useconds_t>(txSeconds * 1e6));

    // Deassert PTT: return SR105U to RX mode
    assertPTT(false);
    printf("PTT deasserted (RX mode)\n");

    printf("==============================================\n\n");
    return true;
}

std::string RadioBridge::decodeCallsign(const U8* encoded) {
    std::string callsign;
    for (int i = 0; i < 6; i++) {
        char c = static_cast<char>(encoded[i] >> 1);
        if (c != ' ') {
            callsign += c;
        }
    }
    return callsign;
}

void RadioBridge::assertPTT(bool tx) {
    // GPIO 20 is PTT: HIGH = transmit, LOW = receive (per fm_init.py in setup_pi.sh)
    // GPIO 20 is active-low PTT: LOW = transmit, HIGH = receive (per fm_init.py)
    { std::ofstream f("/sys/class/gpio/export");         if (f) f << "20"; }
    { std::ofstream f("/sys/class/gpio/gpio20/direction"); if (f) f << "out"; }
    { std::ofstream f("/sys/class/gpio/gpio20/value");     if (f) f << (tx ? "0" : "1"); }
}

} // namespace RadioBridge
