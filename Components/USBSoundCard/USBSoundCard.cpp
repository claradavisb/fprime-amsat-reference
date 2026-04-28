// ======================================================================
// \title  USBSoundCard.cpp
// \author cdavi125 and Claude Code
// \brief  cpp file for USBSoundCard component implementation class
// ======================================================================

#include "Components/USBSoundCard/USBSoundCard.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

namespace Components {

USBSoundCard::USBSoundCard(const char *const compName)
    : USBSoundCardComponentBase(compName),
      m_captureActive(false),
      m_kissSockFd(-1),
      m_kissConnected(false),
      m_kissBufferPos(0),
      m_kissInFrame(false),
      m_kissEscaping(false)
{
    initializeKissRxBuffer();
    initializeKissSocket();

    memset(m_kissBuffer, 0, sizeof(m_kissBuffer));
}

USBSoundCard::~USBSoundCard() {
    if (m_kissSockFd >= 0) {
        close(m_kissSockFd);
    }
    if (m_kissRxBuffer.getData() != nullptr) {
        delete[] m_kissRxBuffer.getData();
    }
}


void USBSoundCard::START_CAPTURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    if (m_captureActive) {
        this->log_WARNING_HI_AUDIO_CAPTURE_ALREADY_STARTED();
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    m_captureActive = true;
    this->log_ACTIVITY_LO_AUDIO_CAPTURE_STARTED();
    this->tlmWrite_DEVICE_CONNECTED(true);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void USBSoundCard::STOP_CAPTURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    if (!m_captureActive) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    m_captureActive = false;
    this->log_ACTIVITY_LO_AUDIO_CAPTURE_STOPPED();
    this->tlmWrite_DEVICE_CONNECTED(false);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void USBSoundCard::run_handler(FwIndexType portNum, U32 context) {
    readKissData();
}

void USBSoundCard::initializeKissRxBuffer() {
    U8* data = new U8[1024];
    m_kissRxBuffer.setData(data);
    m_kissRxBuffer.setSize(1024);
}

void USBSoundCard::initializeKissSocket() {
    m_kissSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_kissSockFd < 0) {
        printf("[KISS] Failed to create socket: %s\n", strerror(errno));
        m_kissConnected = false;
        return;
    }

    int flags = fcntl(m_kissSockFd, F_GETFL, 0);
    fcntl(m_kissSockFd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8001);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(m_kissSockFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (ret < 0 && errno != EINPROGRESS) {
        printf("[KISS] Failed to connect to Direwolf KISS port: %s\n", strerror(errno));
        close(m_kissSockFd);
        m_kissSockFd = -1;
        m_kissConnected = false;
        return;
    }

    m_kissConnected = true;
    printf("[KISS] Connected to Direwolf KISS interface on port 8001\n");
}

void USBSoundCard::readKissData() {
    static const U8 KISS_FEND  = 0xC0;
    static const U8 KISS_FESC  = 0xDB;
    static const U8 KISS_TFEND = 0xDC;
    static const U8 KISS_TFDD  = 0xDD;

    if (!m_kissConnected) {
        static U32 retryCounter = 0;
        if (++retryCounter % 100 == 0) {
            initializeKissSocket();
        }
        return;
    }

    U8 raw[512];
    ssize_t bytesRead = read(m_kissSockFd, raw, sizeof(raw));

    if (bytesRead < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            printf("[KISS] Read error: %s\n", strerror(errno));
            close(m_kissSockFd);
            m_kissSockFd = -1;
            m_kissConnected = false;
        }
        return;
    }

    if (bytesRead == 0) {
        printf("[KISS] Connection closed, will reconnect\n");
        close(m_kissSockFd);
        m_kissSockFd = -1;
        m_kissConnected = false;
        return;
    }

    for (ssize_t i = 0; i < bytesRead; i++) {
        U8 byte = raw[i];

        if (byte == KISS_FEND) {
            if (m_kissInFrame && m_kissBufferPos > 1) {
                // m_kissBuffer[0] is KISS command byte
                if ((m_kissBuffer[0] & 0x0F) == 0x00) {
                    processKissFrame(&m_kissBuffer[1], m_kissBufferPos - 1);
                }
            }
            m_kissInFrame = true;
            m_kissBufferPos = 0;
            m_kissEscaping = false;
        } else if (m_kissInFrame) {
            if (m_kissEscaping) {
                if (byte == KISS_TFEND) {
                    byte = KISS_FEND;
                } else if (byte == KISS_TFDD) {
                    byte = KISS_FESC;
                }
                m_kissEscaping = false;
            } else if (byte == KISS_FESC) {
                m_kissEscaping = true;
                continue;
            }

            if (m_kissBufferPos < sizeof(m_kissBuffer)) {
                m_kissBuffer[m_kissBufferPos++] = byte;
            } else {
                printf("[KISS] Buffer overflow, dropping frame\n");
                m_kissInFrame = false;
                m_kissBufferPos = 0;
            }
        }
    }
}

void USBSoundCard::processKissFrame(const U8* data, FwSizeType size) {
    if (data == nullptr || size == 0) {
        return;
    }

    if (size > m_kissRxBuffer.getSize()) {
        printf("[KISS] Frame too large (%lu bytes), dropping\n", size);
        return;
    }

    memcpy(m_kissRxBuffer.getData(), data, size);
    m_kissRxBuffer.setSize(size);

    printf("[KISS] Received AX.25 frame, %lu bytes, forwarding to deframer\n", size);
    this->ax25Out_out(0, m_kissRxBuffer);
}

} // namespace Components