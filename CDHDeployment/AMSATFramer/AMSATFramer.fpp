module Svc {

  passive component AMSATFramer {

    # COM-with-context data path
    sync input  port dataIn:        Svc.ComDataWithContext
    output      port dataOut:       Svc.ComDataWithContext
    sync input  port dataReturnIn:  Svc.ComDataWithContext
    output      port dataReturnOut: Svc.ComDataWithContext
    output      port comStatusOut:  Fw.SuccessCondition

    # TCP forwarding — sends raw F Prime packet to TCP framer so GDS receives events
    output port tcpOut: Svc.ComDataWithContext

    # Buffer allocation
    output port bufferAllocate:   Fw.BufferGet
    output port bufferDeallocate: Fw.BufferSend

    # Standard ports
    time  get   port timeCaller
    event       port logOut
    text event  port logTextOut

    # Commands
    command recv port cmdIn
    command reg  port cmdRegOut
    command resp port cmdResponseOut

    sync command TEST_SEND_DATA(testValue: U32)

    # Events
    event FrameCreated(frameSize: U32) \
      severity activity low \
      format "AMSAT AX.25 frame created, size: {}"

    event InvalidInputBuffer \
      severity warning high \
      format "Invalid F Prime buffer received (too small or malformed)"

    event BufferAllocationFailed \
      severity warning high \
      format "Failed to allocate buffer for AMSAT frame"

    event TestDataSent(value: U32) \
      severity activity high \
      format "Test F Prime telemetry sent with value: {}"
  }
}