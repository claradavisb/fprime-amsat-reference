// ======================================================================
// \title  RadioBridge.cpp
// \author madisonw
// \brief  Transmits F' telemetry over RF via gen_packets + rpitx
// ======================================================================

#include "CDHDeployment/RadioBridge/RadioBridge.hpp"
#include "Fw/Types/Assert.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace RadioBridge {

RadioBridge::RadioBridge(const char* const compName)
    : RadioBridgeComponentBase(compName) {
    printf("[RadioBridge] Initialized — using gen_packets + rpitx on GPIO 4\n");
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

    if (transmitAX25Frame(fwBuffer.getData(), fwBuffer.getSize())) {
        this->log_ACTIVITY_HI_RADIO_TX_SUCCESS();
    } else {
        Fw::LogStringArg err("gen_packets/rpitx failed");
        this->log_WARNING_HI_RADIO_TX_FAILED(err);
    }

    this->dataReturnOut_out(0, fwBuffer, context);
}

bool RadioBridge::transmitAX25Frame(const U8* data, FwSizeType size) {
    // AX.25 frame layout built by AMSATFramer:
    //   flag(1) + dest_addr(7) + src_addr(7) + ctrl(1) + pid(1) = 17 byte header
    //   F' payload
    //   crc(2) + flag(1) = 3 byte trailer
    static const FwSizeType HEADER  = 17;
    static const FwSizeType TRAILER = 3;

    if (size < HEADER + TRAILER + 1) {
        printf("[RadioBridge] Frame too small: %lu bytes\n", size);
        return false;
    }

    const U8*    payload     = &data[HEADER];
    FwSizeType   payloadSize = size - HEADER - TRAILER;

    // Hex-encode the F' payload so gen_packets can carry it as ASCII text
    // in the AX.25 info field. The GDS ax25_kiss_framer.py hex-decodes it back.
    char* hexBuf = new char[payloadSize * 2 + 1];
    for (FwSizeType i = 0; i < payloadSize; i++) {
        snprintf(&hexBuf[i * 2], 3, "%02x", payload[i]);
    }
    hexBuf[payloadSize * 2] = '\0';

    // Write gen_packets input file: SOURCECALL>DESTCALL:<hexdata>
    FILE* f = fopen("/tmp/fprime_telem.txt", "w");
    if (!f) {
        printf("[RadioBridge] Failed to open /tmp/fprime_telem.txt\n");
        delete[] hexBuf;
        return false;
    }
    fprintf(f, "W1AW>AMSAT0:%s\n", hexBuf);
    fclose(f);
    delete[] hexBuf;

    printf("[RadioBridge] Payload: %lu bytes → %lu hex chars\n",
           payloadSize, payloadSize * 2);

    // Encode as 1200 bps Bell 202 AFSK audio
    int ret = system("gen_packets -r 44100 -o /tmp/fprime_telem.wav /tmp/fprime_telem.txt 2>/dev/null");
    if (ret != 0) {
        printf("[RadioBridge] gen_packets failed (exit %d)\n", ret);
        return false;
    }

    // Transmit via rpitx on GPIO 4 at 434.9 MHz
    ret = system("rpitx -m RF -i /tmp/fprime_telem.wav -f 434900000 -s 44100 2>/dev/null");
    if (ret != 0) {
        printf("[RadioBridge] rpitx failed (exit %d)\n", ret);
        return false;
    }

    printf("[RadioBridge] Transmission complete\n");
    return true;
}

} // namespace RadioBridge
