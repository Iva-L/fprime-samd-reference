module RadioGateway {
    @ Radio telemetry reassembly and asynchronous UART forwarding.
    passive component Gateway {
        sync input port allocate: Fw.BufferGet
        sync input port deallocate: Fw.BufferSend
        sync input port getParameter: Fw.PrmGet
        sync input port radioReceive: Svc.ComDataWithContext
        sync input port radioEvent: Fw.Log
        output port radioReturn: Svc.ComDataWithContext
        output port uartSend: Fw.BufferSend
        sync input port uartReturn: Drv.ByteStreamData
        sync input port uartReceive: Drv.ByteStreamData
        output port uartReceiveReturn: Fw.BufferSend
        sync input port run: Svc.Sched
    }
}
