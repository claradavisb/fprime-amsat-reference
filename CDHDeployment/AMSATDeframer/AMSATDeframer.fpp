module Svc {

    passive component AMSATDeframer {

        # ----------------------------------------------------------------------
        # Data ports
        # ----------------------------------------------------------------------

        @ Receive AX.25 frames from USBSoundCard (via Direwolf KISS interface)
        sync input port dataIn: Fw.BufferSend

        @ Forward extracted F′ command packets to command dispatcher
        output port comOut: Fw.Com

        @ Receive command status response from CmdDispatcher (required matched port for seqCmdBuff)
        sync input port cmdResponseIn: Fw.CmdResponse

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ AX.25 frame too short to be valid
        event FRAME_TOO_SHORT(frameSize: U32) \
            severity warning high id 0 \
            format "AX.25 frame too short: {} bytes"

        @ AX.25 CRC verification failed
        event FRAME_CRC_ERROR(frameSize: U32) \
            severity warning high id 1 \
            format "AX.25 CRC error on frame of {} bytes"

        @ F′ packet successfully extracted and dispatched
        event FRAME_DISPATCHED(payloadSize: U32) \
            severity activity low id 2 \
            format "F′ packet dispatched: {} bytes"

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}
