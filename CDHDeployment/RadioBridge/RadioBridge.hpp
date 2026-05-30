#ifndef RadioBridge_RadioBridge_HPP
#define RadioBridge_RadioBridge_HPP

#include "CDHDeployment/RadioBridge/RadioBridgeComponentAc.hpp"
#include "Fw/Types/BasicTypes.hpp"

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
};

} // namespace RadioBridge

#endif