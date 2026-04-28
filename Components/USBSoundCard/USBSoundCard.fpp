module Components {
    @ AMSat USB Sound Component - Audio Capture
    active component USBSoundCard {

        # ===============================================
        # COMMANDS - Audio capture
        # ===============================================

        @ Start audio capture
        async command START_CAPTURE opcode 0

        @ Stop audio capture
        async command STOP_CAPTURE opcode 1

        # ===============================================
        # TELEMETRY - Monitor audio health
        # ===============================================

        @ Current audio input level (0-255)
        telemetry AUDIO_INPUT_LEVEL: U32 id 0x1001

        @ Audio device connection status
        telemetry DEVICE_CONNECTED: bool id 0x1002

        @ Number of audio frames processed
        telemetry FRAMES_PROCESSED: U32 id 0x1003

        @ Peak audio level in last second
        telemetry AUDIO_PEAK_LEVEL: U32 id 0x1004

        # ===============================================
        # EVENTS - Essential logging
        # ===============================================

        @ Audio capture started
        event AUDIO_CAPTURE_STARTED severity activity low id 0 format "Audio capture started"

        @ Audio capture stopped
        event AUDIO_CAPTURE_STOPPED severity activity low id 1 format "Audio capture stopped"

        @ USB sound card disconnected
        event DEVICE_DISCONNECTED severity warning high id 2 format "USB sound card disconnected"

        @ Audio capture already started
        event AUDIO_CAPTURE_ALREADY_STARTED severity warning high id 3 format "Audio capture already started"

        @ Audio level too high
        event AUDIO_LEVEL_HIGH severity warning low id 4 format "Audio input level too high"

        # ===============================================
        # SCHEDULED INPUT - For periodic audio processing
        # ===============================================

        @ Port for receiving periodic calls from rate group
        sync input port run: Svc.Sched

        # ===============================================
        # OUTPUT PORTS
        # ===============================================

        @ Port for sending received AX.25 frames to AMSATDeframer
        output port ax25Out: Fw.BufferSend

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
