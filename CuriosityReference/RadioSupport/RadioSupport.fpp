module CuriosityReference {
    @ Fixed radio receive buffer and startup frequency, without a parameter database.
    passive component RadioSupport {
        sync input port allocate: Fw.BufferGet
        sync input port deallocate: Fw.BufferSend
        sync input port getParameter: Fw.PrmGet
        sync input port receive: Svc.ComDataWithContext
        output port transmit: Svc.ComDataWithContext
        sync input port sendReturn: Svc.ComDataWithContext
        output port returnBuffer: Svc.ComDataWithContext

        sync command SEND_STRING(payload: string)
        command reg port CmdReg
        command recv port CmdDisp
        command resp port CmdStatus
        time get port timeCaller
        event port logOut
        text event port logTextOut

        event TestPayloadQueued(payloadSize: FwSizeType) severity activity high \
            format "Queued {} byte command payload for transmission"
        event TestSendRejected severity warning high \
            format "Test message rejected"
    }
}
