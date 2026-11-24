# Radio gateway (work in progress)

`RadioGateway` is a separate SAMD deployment, leaving the echo deployment
`CuriosityReference` unchanged. The current implementation is **not ready for
hardware testing as a transparent radio link**:

- It reassembles STM32 telemetry fragments and forwards complete 1,024-byte
  frames on the gateway UART. A busy UART causes additional frames to be dropped.
- UART commands are counted and discarded. Radio command transmission, ACKs,
  retries, reset/session handling, and half-duplex arbitration are not integrated.
- The portable command classes start disarmed and require a fresh manually
  authorized session, but no deployment controls or reset detection are wired.
- Gateway startup, diagnostic events, and fatal reports are not sent over the
  UART, so they cannot be mistaken for STM32 CCSDS telemetry.

The transport sources are copied from the STM32 reference project's
`FprimeStm32BaremetalReference/Components/RadioTransport` directory. Keep both
copies identical when changing the protocol. When command uplink is added, the
gateway must use the STM32 deployment's spacecraft ID to configure `TcStream`
and preserve bytes that the parser has not consumed while a frame is pending.

For development, a debugger can inspect `RadioGateway::gateway.diagnostics`:
`droppedFrames`, `rejectedPackets`, `allocationFailures`, `uartErrors`,
`unsupportedUartChunks`, `radioEvents`, and `lastRadioEvent`. Counters saturate
at their maximum values; `lastRadioEvent` is an F Prime event ID, not an event
payload. The debugger can also inspect `Samd21::gatewayFatalPackets` and
`Samd21::gatewayBailLine` for fatal handling. These variables are volatile, not
persistent across reset, and do not replace a complete operational health
reporting mechanism.
