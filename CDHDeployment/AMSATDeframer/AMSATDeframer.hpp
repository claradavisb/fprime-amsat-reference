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
    static const U16 crc16Table[256];
    static U16 calculateCRC16(const U8* data, FwSizeType length);
};

} // namespace Svc

#endif
