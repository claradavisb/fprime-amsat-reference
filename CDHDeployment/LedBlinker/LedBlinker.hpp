#ifndef CDHDeployment_LedBlinker_HPP
#define CDHDeployment_LedBlinker_HPP

#include "CDHDeployment/LedBlinker/LedBlinkerComponentAc.hpp"

namespace CDHDeployment {

class LedBlinker : public LedBlinkerComponentBase {
  public:
    LedBlinker(const char* compName);
    ~LedBlinker();

  private:
    void seqCmdIn_handler(FwIndexType portNum, Fw::ComBuffer& data, U32 context) override;
    void seqCmdStatusIn_handler(FwIndexType portNum, FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdResponse& response) override;
    void schedIn_handler(FwIndexType portNum, U32 context) override;

    bool m_blinkPending = false;
};

} // namespace CDHDeployment

#endif
