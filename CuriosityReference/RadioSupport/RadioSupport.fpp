module CuriosityReference {
    @ Fixed radio receive buffer and startup frequency, without a parameter database.
    passive component RadioSupport {
        sync input port allocate: Fw.BufferGet
        sync input port deallocate: Fw.BufferSend
        sync input port getParameter: Fw.PrmGet
    }
}
