// ======================================================================
// \title  AMSATDeframer.hpp
// \author root
// \brief  hpp file for AMSATDeframer component implementation class
// ======================================================================

#ifndef Svc_AMSATDeframer_HPP
#define Svc_AMSATDeframer_HPP

#include "CDHDeployment/AMSATDeframer/AMSATDeframerComponentAc.hpp"

namespace Svc {

class AMSATDeframer final : public AMSATDeframerComponentBase {

  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct AMSATDeframer object
    AMSATDeframer(const char *const compName);

    //! Destroy AMSATDeframer object
    ~AMSATDeframer();

  private:
    void dataIn_handler(FwIndexType portNum, Fw::Buffer& buffer) override;

    void cmdResponseIn_handler(FwIndexType portNum,
                               FwOpcodeType opcode,
                               U32 cmdSeq,
                               const Fw::CmdResponse& response) override;

    static const U16 crc16Table[256];
    static U16 calculateCRC16(const U8* data, FwSizeType length);

    // AX.25 frame layout (KISS format, no HDLC flags):
    // [dest 7B][src 7B][ctrl 1B][pid 1B][payload...][crc low][crc high]
    static constexpr FwSizeType AX25_HEADER_SIZE = 16; // dest(7)+src(7)+ctrl(1)+pid(1)
    static constexpr FwSizeType AX25_CRC_SIZE    = 2;
    static constexpr FwSizeType AX25_MIN_SIZE    = AX25_HEADER_SIZE;
};

} // namespace Svc

#endif
