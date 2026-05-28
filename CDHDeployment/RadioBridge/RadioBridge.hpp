#ifndef RadioBridge_RadioBridge_HPP
#define RadioBridge_RadioBridge_HPP

#include "CDHDeployment/RadioBridge/RadioBridgeComponentAc.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include <string>

namespace RadioBridge {

class RadioBridge : public RadioBridgeComponentBase {
  public:
    RadioBridge(const char* const compName);
    ~RadioBridge();
    
  private:
    void dataIn_handler(
        FwIndexType portNum,
        Fw::Buffer& fwBuffer,
        const ComCfg::FrameContext& context
    ) override;
    
    bool transmitAX25Frame(const U8* data, FwSizeType size);

    std::string decodeCallsign(const U8* encoded);

    void assertPTT(bool tx);
};

} // namespace RadioBridge

#endif