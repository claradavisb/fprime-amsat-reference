#ifndef Svc_AMSATFramer_HPP
#define Svc_AMSATFramer_HPP

#include "CDHDeployment/AMSATFramer/AMSATFramerComponentAc.hpp"
#include "Fw/Types/BasicTypes.hpp"

namespace Svc {

class AMSATFramer : public AMSATFramerComponentBase {
 public:
  AMSATFramer(const char* const compName);
  ~AMSATFramer();

  void setSourceCallsign(const char* callsign, U8 ssid);
  void setDestCallsign(const char* callsign, U8 ssid);
  void sendReadySignal();

 protected:
  void dataIn_handler(
      FwIndexType portNum,
      Fw::Buffer& data,
      const ComCfg::FrameContext& context
  ) override;

  void dataReturnIn_handler(
      FwIndexType portNum,
      Fw::Buffer& data,
      const ComCfg::FrameContext& context
  ) override;

  void TEST_SEND_DATA_cmdHandler(
      FwOpcodeType opCode,
      U32 cmdSeq,
      U32 testValue
  ) override;

 private:
  static const U16 crc16Table[256];

  enum : U8 {
    AX25_CONTROL = 0x03,
    AX25_PID     = 0xF0,
    AX25_FLAG    = 0x7E
  };

  static constexpr U32 AX25_CALLSIGN_LEN = 6;
  static constexpr const char* DEFAULT_SRC_CALL  = "N0CALL";
  static constexpr const char* DEFAULT_DEST_CALL = "CQ";
  static constexpr U8  DEFAULT_SRC_SSID  = 0;
  static constexpr U8  DEFAULT_DEST_SSID = 0;

  static constexpr U8  AX25_SSID_RESERVED = 0x60;
  static constexpr U8  AX25_SSID_LAST     = 0x61;

  char m_srcCallsign[AX25_CALLSIGN_LEN + 1];
  char m_destCallsign[AX25_CALLSIGN_LEN + 1];
  U8   m_srcSSID;
  U8   m_destSSID;

  FwSizeType encodeAddress(U8* dest, const char* callsign, U8 ssid, bool isLast);
  static U16 calculateCRC16(const U8* data, FwSizeType length);
};

} // namespace Svc

#endif