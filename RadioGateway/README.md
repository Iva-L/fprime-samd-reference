# Radio gateway (work in progress)

`RadioGateway` is a separate SAMD deployment, leaving the echo deployment
`CuriosityReference` unchanged. The current implementation is **not ready for
hardware testing as a transparent radio link**:

- It reassembles STM32 telemetry fragments and forwards complete 1,024-byte
  frames on the gateway UART. The reassembly buffer is retained until UART
  completion; fragments arriving while it is pending or in flight are dropped.
  `droppedFrames` counts each observed first fragment in that period, including
  frames that might not otherwise have completed.
- UART commands are counted and discarded. Radio command transmission, ACKs,
  retries, reset/session handling, and half-duplex arbitration are not integrated.
- The portable command classes start disarmed and require a fresh manually
  authorized session, but no deployment controls or reset detection are wired.
- Gateway startup, diagnostic events, and fatal reports are not sent over the
  UART, so they cannot be mistaken for STM32 CCSDS telemetry.

The transport is supplied by the `lib/fprime-lora` submodule. Initialize the
submodule and regenerate the build cache when its location changes. Keep both
reference deployments on compatible library versions. When command uplink is added,
evaluate F Prime's `Svc.FrameAccumulator` and `CcsdsTcFrameDetector` before
using the experimental `TcStream`. The framework detector uses the gateway
build's `ComCfg.SpacecraftId`, which must match the STM32 dictionary; verify
its buffer allocation, overload behavior, and total memory use on the SAMD.

For development, a debugger can inspect `RadioGateway::gateway.diagnostics`:
`droppedFrames`, `rejectedPackets`, `allocationFailures`, `uartErrors`,
`unsupportedUartChunks`, `radioEvents`, and `lastRadioEvent`. Counters saturate
at their maximum values; `lastRadioEvent` is an F Prime event ID, not an event
payload. The debugger can also inspect `Samd21::gatewayFatalPackets` and
`Samd21::gatewayBailLine` for fatal handling. These variables are volatile, not
persistent across reset, and do not replace a complete operational health
reporting mechanism.
