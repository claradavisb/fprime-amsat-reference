module CDHDeployment {

  @ Single-path command proxy that blinks GPIO 27 (txLed) on command dispatch.
  @ Instantiate once per uplink path (TCP and RF) to satisfy the seqCmdStatus
  @ match constraint which requires each matched port to connect to a separate instance.
  passive component LedBlinker {

    @ Receive command from one uplink sender
    sync input port seqCmdIn: Fw.Com

    @ Forward command to cmdDisp.seqCmdBuff
    output port seqCmdOut: Fw.Com

    @ Receive command status back from cmdDisp.seqCmdStatus
    sync input port seqCmdStatusIn: Fw.CmdResponse

    @ Forward status back to original sender
    output port seqCmdStatusOut: Fw.CmdResponse

    @ Rate group tick — turns LED off after one cycle
    sync input port schedIn: Svc.Sched

  }

}
