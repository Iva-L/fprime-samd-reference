# Radio gateway work in progress

This directory currently contains only the portable radio transport library.
There is **no gateway deployment yet**: it cannot forward UART traffic or
operate the radio. Continue to use `CuriosityReference` for the echo test.

The transport sources are copied from the STM32 reference project's
`FprimeStm32BaremetalReference/Components/RadioTransport` directory. Keep both
copies identical when changing the protocol. The gateway must use the STM32
deployment's spacecraft ID to configure `TcStream` and must preserve bytes
that the parser has not consumed while a complete frame is pending.
