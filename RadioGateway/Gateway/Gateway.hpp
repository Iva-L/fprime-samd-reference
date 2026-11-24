#ifndef RADIO_GATEWAY_GATEWAY_HPP
#define RADIO_GATEWAY_GATEWAY_HPP

#include "RadioGateway/Gateway/GatewayComponentAc.hpp"
#include "RadioGateway/RadioTransport/RadioFragment.hpp"

namespace RadioGateway {

class Gateway final : public GatewayComponentBase {
  public:
    explicit Gateway(const char* name);
    void configure(FwPrmIdType frequencyId);

    struct Diagnostics {
        volatile U32 droppedFrames;
        volatile U32 rejectedPackets;
        volatile U32 allocationFailures;
        volatile U32 uartErrors;
        volatile U32 unsupportedUartChunks;
        volatile U32 radioEvents;
        volatile FwEventIdType lastRadioEvent;
    } diagnostics;

  private:
    Fw::Buffer allocate_handler(FwIndexType port, FwSizeType size) override;
    void deallocate_handler(FwIndexType port, Fw::Buffer& buffer) override;
    Fw::ParamValid getParameter_handler(FwIndexType port, FwPrmIdType id, Fw::ParamBuffer& value) override;
    void radioReceive_handler(FwIndexType port, Fw::Buffer& buffer, const ComCfg::FrameContext& context) override;
    void radioEvent_handler(FwIndexType port, FwEventIdType id, Fw::Time& timeTag,
                            const Fw::LogSeverity& severity, Fw::LogBuffer& args) override;
    void uartReturn_handler(FwIndexType port, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) override;
    void uartReceive_handler(FwIndexType port, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) override;
    void run_handler(FwIndexType port, U32 context) override;

    RadioTransport::Reassembler m_receiver;
    U8 m_rx[RadioTransport::MAX_PACKET_SIZE];
    U8 m_uart[RadioTransport::MAX_FRAME_SIZE];
    FwPrmIdType m_frequencyId;
    FwSizeType m_uartSize;
    bool m_rxAllocated;
    bool m_uartPending;
    bool m_uartInFlight;
};

}  // namespace RadioGateway
#endif
