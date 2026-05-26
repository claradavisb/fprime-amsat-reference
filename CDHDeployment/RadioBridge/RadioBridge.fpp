module RadioBridge {
  @ Component that receives AX.25 frames and transmits via direwolf/rpitx
  passive component RadioBridge {

    # ----------------------------------------------------------------------
    # Standard ports
    # ----------------------------------------------------------------------
    @ Port for requesting current time
    time get port timeCaller
    @ Port for sending events
    event port logOut
    @ Port for sending text events
    text event port logTextOut

    # ----------------------------------------------------------------------
    # Data ports (COM-with-context to match AMSATFramer)
    # ----------------------------------------------------------------------
    @ Receive AX.25 frames (Fw::Buffer + FrameContext) from AMSATFramer
    sync input port dataIn: Svc.ComDataWithContext

    @ Return the buffer after transmission (same context back)
    output port dataReturnOut: Svc.ComDataWithContext

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    @ Received AX.25 frame for transmission
    event FrameReceived(frameSize: U32) \
      severity activity low \
      format "Received AX.25 frame for transmission, size: {} bytes"

    @ Radio transmission started
    event RADIO_TX_STARTED \
      severity activity low \
      format "Radio transmission started via direwolf/rpitx"

    @ Radio transmission completed successfully
    event RADIO_TX_SUCCESS \
      severity activity high \
      format "Radio transmission completed successfully"

    @ Radio transmission failed
    event RADIO_TX_FAILED(error: string size 120) \
      severity warning high \
      format "Radio transmission failed: {}"
  }
}