// ======================================================================
// \title  USBSoundCard.hpp
// \author root
// \brief  hpp file for USBSoundCard component implementation class
// ======================================================================

#ifndef Components_USBSoundCard_HPP
#define Components_USBSoundCard_HPP

#include "Components/USBSoundCard/USBSoundCardComponentAc.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace Components {

class USBSoundCard final : public USBSoundCardComponentBase {
public:
  USBSoundCard(const char *const compName);
  ~USBSoundCard();


PRIVATE:

  void START_CAPTURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
  void STOP_CAPTURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;


  void run_handler(FwIndexType portNum, U32 context) override;

PRIVATE:
  bool m_captureActive;

  void initializeKissRxBuffer();

  void initializeKissSocket();

  void readKissData();

  void processKissFrame(const U8* data, FwSizeType size);

  int m_kissSockFd;
  bool m_kissConnected;
  U8 m_kissBuffer[1024];
  U32 m_kissBufferPos;
  bool m_kissInFrame;
  bool m_kissEscaping;
  Fw::Buffer m_kissRxBuffer;
};

} // namespace Components

#endif // Components_USBSoundCard_HPP