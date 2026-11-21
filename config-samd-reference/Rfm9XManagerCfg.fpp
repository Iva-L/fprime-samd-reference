module Rfm9X {
    module ManagerCfg {
        constant TX_DONE_TIMEOUT_MS = 5000
        constant RX_TX_HOLDOFF_MS = 500
        constant TX_TX_HOLDOFF_MS = 500
        constant RX_HOLDOFF_STARVATION_MS = 2 * RX_TX_HOLDOFF_MS
        constant RESET_HOLD_MS = 1
        @ The reset port controls the breakout regulator's EN, not radio NRESET.
        constant RESET_SETTLE_MS = 20
        constant MAX_PAYLOAD_LENGTH = 255
        @ One received frame can be held for echo while the radio is busy.
        constant PENDING_TX_DEPTH = 1
    }
}
