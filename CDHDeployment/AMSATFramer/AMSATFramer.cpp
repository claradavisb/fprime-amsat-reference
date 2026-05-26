// ======================================================================
// \title  AMSATFramer.cpp
// \author madisonw
// \brief  cpp file for AMSATFramer component implementation class
// ======================================================================

#include "CDHDeployment/AMSATFramer/AMSATFramer.hpp"
#include "Fw/Types/Assert.hpp"
#include <cstring>
#include <cstdio>

namespace Svc {

// CRC-16 lookup table for AX.25
const U16 AMSATFramer::crc16Table[256] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

AMSATFramer::AMSATFramer(const char* const compName)
    : AMSATFramerComponentBase(compName),
      m_srcSSID(DEFAULT_SRC_SSID),
      m_destSSID(DEFAULT_DEST_SSID) {
    strncpy(m_srcCallsign, DEFAULT_SRC_CALL, AX25_CALLSIGN_LEN);
    m_srcCallsign[AX25_CALLSIGN_LEN] = '\0';
    strncpy(m_destCallsign, DEFAULT_DEST_CALL, AX25_CALLSIGN_LEN);
    m_destCallsign[AX25_CALLSIGN_LEN] = '\0';

    printf("\n========================================\n");
    printf("AMSATFramer Component Initialized\n");
    printf("Source:      %s-%d\n", m_srcCallsign, m_srcSSID);
    printf("Destination: %s-%d\n", m_destCallsign, m_destSSID);
    printf("========================================\n\n");
    fflush(stdout);
}

AMSATFramer::~AMSATFramer() {}

// ----------------------------------------------------------------------
// Public methods
// ----------------------------------------------------------------------

void AMSATFramer::setSourceCallsign(const char* callsign, U8 ssid) {
    FW_ASSERT(callsign != nullptr);
    strncpy(m_srcCallsign, callsign, AX25_CALLSIGN_LEN);
    m_srcCallsign[AX25_CALLSIGN_LEN] = '\0';
    m_srcSSID = ssid & 0x0F;
}

void AMSATFramer::setDestCallsign(const char* callsign, U8 ssid) {
    FW_ASSERT(callsign != nullptr);
    strncpy(m_destCallsign, callsign, AX25_CALLSIGN_LEN);
    m_destCallsign[AX25_CALLSIGN_LEN] = '\0';
    m_destSSID = ssid & 0x0F;
}

// ----------------------------------------------------------------------
// Command handler implementations
// ----------------------------------------------------------------------

void AMSATFramer::TEST_SEND_DATA_cmdHandler(
    FwOpcodeType opCode,
    U32 cmdSeq,
    U32 testValue
) {
    printf("\n========================================\n");
    printf("TEST_SEND_DATA Command Received\n");
    printf("Test Value: %u\n", testValue);
    printf("========================================\n\n");

    // Create test data on stack
    const FwSizeType testDataSize = 20;
    U8 testData[20];
    FwSizeType offset = 0;

    testData[offset++] = 0x01;        // packet type
    testData[offset++] = 0x50;        // comp id hi
    testData[offset++] = 0x00;        // comp id lo
    testData[offset++] = 0x01;        // channel id

    U32 timestamp = 1234567890;
    testData[offset++] = (timestamp >> 24) & 0xFF;
    testData[offset++] = (timestamp >> 16) & 0xFF;
    testData[offset++] = (timestamp >> 8) & 0xFF;
    testData[offset++] = timestamp & 0xFF;

    testData[offset++] = (testValue >> 24) & 0xFF;
    testData[offset++] = (testValue >> 16) & 0xFF;
    testData[offset++] = (testValue >> 8) & 0xFF;
    testData[offset++] = testValue & 0xFF;

    for (; offset < testDataSize; offset++) {
        testData[offset] = 0xAA;
    }

    // Build AX.25 frame into an allocated buffer
    const FwSizeType amsatOverhead = 20;
    const FwSizeType amsatFrameSize = testDataSize + amsatOverhead;

    Fw::Buffer amsatFrame = this->bufferAllocate_out(0, amsatFrameSize);
    if (amsatFrame.getData() == nullptr) {
        printf("ERROR: Failed to allocate buffer for AX.25 frame\n");
        this->log_WARNING_HI_BufferAllocationFailed();
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    U8* framePtr = amsatFrame.getData();
    FwSizeType frameOffset = 0;

    framePtr[frameOffset++] = AX25_FLAG; // start flag
    frameOffset += encodeAddress(&framePtr[frameOffset], m_destCallsign, m_destSSID, false);
    frameOffset += encodeAddress(&framePtr[frameOffset], m_srcCallsign,  m_srcSSID,  true);
    framePtr[frameOffset++] = AX25_CONTROL;
    framePtr[frameOffset++] = AX25_PID;

    memcpy(&framePtr[frameOffset], testData, testDataSize);
    frameOffset += testDataSize;

    U16 crc = calculateCRC16(&framePtr[1], frameOffset - 1);
    framePtr[frameOffset++] = static_cast<U8>(crc & 0xFF);
    framePtr[frameOffset++] = static_cast<U8>((crc >> 8) & 0xFF);

    framePtr[frameOffset++] = AX25_FLAG; // end flag
    amsatFrame.setSize(frameOffset);

    // Send to RadioBridge
    ComCfg::FrameContext context;
    this->dataOut_out(0, amsatFrame, context);

    this->log_ACTIVITY_HI_TestDataSent(testValue);
    this->log_ACTIVITY_LO_FrameCreated(static_cast<U32>(frameOffset));
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ----------------------------------------------------------------------
// Handler implementations
// ----------------------------------------------------------------------

void AMSATFramer::dataIn_handler(
    FwIndexType portNum,
    Fw::Buffer& data,
    const ComCfg::FrameContext& context
) {
    printf("\n========================================\n");
    printf("AMSATFramer::dataIn_handler\n");
    printf("Input size: %lu bytes\n", data.getSize());
    printf("========================================\n");

    if (data.getSize() < 1) {
        printf("ERROR: Buffer too small\n");
        this->log_WARNING_HI_InvalidInputBuffer();
        this->dataReturnOut_out(0, data, context);
        Fw::Success failStatus = Fw::Success::FAILURE;
        this->comStatusOut_out(0, failStatus);
        return;
    }

    // Forward raw F Prime packet to TCP framer so GDS receives events/telemetry
    if (this->isConnected_tcpOut_OutputPort(0)) {
        this->tcpOut_out(0, data, context);
    }

    // Build AX.25 frame from incoming buffer
    const FwSizeType amsatOverhead = 20;
    const FwSizeType amsatFrameSize = data.getSize() + amsatOverhead;

    Fw::Buffer amsatFrame = this->bufferAllocate_out(0, amsatFrameSize);
    if (amsatFrame.getData() == nullptr) {
        printf("ERROR: Failed to allocate buffer\n");
        this->log_WARNING_HI_BufferAllocationFailed();
        this->dataReturnOut_out(0, data, context);
        Fw::Success failStatus = Fw::Success::FAILURE;
        this->comStatusOut_out(0, failStatus);
        return;
    }

    U8* framePtr = amsatFrame.getData();
    FwSizeType offset = 0;

    framePtr[offset++] = AX25_FLAG;
    offset += encodeAddress(&framePtr[offset], m_destCallsign, m_destSSID, false);
    offset += encodeAddress(&framePtr[offset], m_srcCallsign,  m_srcSSID,  true);
    framePtr[offset++] = AX25_CONTROL;
    framePtr[offset++] = AX25_PID;

    memcpy(&framePtr[offset], data.getData(), data.getSize());
    offset += data.getSize();

    U16 crc = calculateCRC16(&framePtr[1], offset - 1);
    framePtr[offset++] = static_cast<U8>(crc & 0xFF);
    framePtr[offset++] = static_cast<U8>((crc >> 8) & 0xFF);

    framePtr[offset++] = AX25_FLAG;
    amsatFrame.setSize(offset);

    this->log_ACTIVITY_LO_FrameCreated(static_cast<U32>(offset));

    // Forward the AX.25 frame to RadioBridge
    this->dataOut_out(0, amsatFrame, context);

    // Return the original buffer to comQueue and signal it to send the next packet
    this->dataReturnOut_out(0, data, context);
    Fw::Success okStatus = Fw::Success::SUCCESS;
    this->comStatusOut_out(0, okStatus);

    printf("[LIVE MODE] Frame forwarded to RadioBridge\n\n");
}

void AMSATFramer::dataReturnIn_handler(
    FwIndexType portNum,
    Fw::Buffer& data,
    const ComCfg::FrameContext& context
) {
    // RadioBridge has finished with the buffer; release it
    this->bufferDeallocate_out(0, data);
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

FwSizeType AMSATFramer::encodeAddress(U8* dest, const char* callsign, U8 ssid, bool isLast) {
    FW_ASSERT(dest != nullptr);
    FW_ASSERT(callsign != nullptr);

    FwSizeType i = 0;
    FwSizeType len = strnlen(callsign, AX25_CALLSIGN_LEN);

    for (i = 0; i < len && i < AX25_CALLSIGN_LEN; i++) {
        dest[i] = static_cast<U8>(callsign[i] << 1);
    }
    for (; i < AX25_CALLSIGN_LEN; i++) {
        dest[i] = static_cast<U8>(' ' << 1);
    }

    U8 ssidByte = static_cast<U8>((ssid & 0x0F) << 1);
    ssidByte |= isLast ? AX25_SSID_LAST : AX25_SSID_RESERVED;
    dest[AX25_CALLSIGN_LEN] = ssidByte;

    return 7;  // 6 callsign + 1 SSID
}

U16 AMSATFramer::calculateCRC16(const U8* data, FwSizeType length) {
    FW_ASSERT(data != nullptr);

    U16 crc = 0xFFFF;
    for (FwSizeType i = 0; i < length; i++) {
        crc = static_cast<U16>((crc >> 8) ^ crc16Table[(crc ^ data[i]) & 0xFF]);
    }
    return static_cast<U16>(crc ^ 0xFFFF);
}

}  // namespace Svc