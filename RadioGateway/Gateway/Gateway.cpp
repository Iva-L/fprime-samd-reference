#include "RadioGateway/Gateway/Gateway.hpp"

#include <Fw/Types/Assert.hpp>
#include <cstring>

namespace RadioGateway {

namespace {
void increment(volatile U32& value) {
    if (value != UINT32_MAX) {
        ++value;
    }
}
}

Gateway::Gateway(const char* name)
    : GatewayComponentBase(name), diagnostics{}, m_receiver(), m_rx{}, m_uart{}, m_frequencyId(0), m_uartSize(0),
      m_rxAllocated(false), m_uartPending(false), m_uartInFlight(false) {}

void Gateway::configure(FwPrmIdType frequencyId) {
    m_frequencyId = frequencyId;
}

Fw::Buffer Gateway::allocate_handler(FwIndexType port, FwSizeType size) {
    (void)port;
    if (m_rxAllocated || size > sizeof m_rx) {
        increment(diagnostics.allocationFailures);
        return Fw::Buffer();
    }
    m_rxAllocated = true;
    return Fw::Buffer(m_rx, sizeof m_rx);
}

void Gateway::deallocate_handler(FwIndexType port, Fw::Buffer& buffer) {
    (void)port;
    if (buffer.getData() == nullptr && buffer.getSize() == 0) {
        return;
    }
    FW_ASSERT(m_rxAllocated && buffer.getData() == m_rx);
    m_rxAllocated = false;
}

Fw::ParamValid Gateway::getParameter_handler(FwIndexType port, FwPrmIdType id, Fw::ParamBuffer& value) {
    (void)port;
    if (id != m_frequencyId) {
        return Fw::ParamValid::INVALID;
    }
    value.resetSer();
    const Fw::SerializeStatus status = value.serializeFrom(static_cast<U32>(915000000));
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    return Fw::ParamValid::VALID;
}

void Gateway::radioReceive_handler(FwIndexType port, Fw::Buffer& buffer, const ComCfg::FrameContext& context) {
    (void)port;
    RadioTransport::Fragment fragment{};
    if (!RadioTransport::decodeFragment(buffer.getData(), buffer.getSize(), fragment) ||
        fragment.kind != RadioTransport::FrameKind::TELEMETRY) {
        m_receiver.reset();
        increment(diagnostics.rejectedPackets);
    } else {
        const auto result = m_receiver.accept(buffer.getData(), buffer.getSize());
        if (result == RadioTransport::ReceiveResult::REJECTED) {
            increment(diagnostics.rejectedPackets);
        } else if (result == RadioTransport::ReceiveResult::COMPLETE) {
            if (m_receiver.size() != RadioTransport::MAX_FRAME_SIZE || m_uartPending || m_uartInFlight) {
                increment(diagnostics.droppedFrames);
            } else {
                m_uartSize = static_cast<FwSizeType>(m_receiver.size());
                std::memcpy(m_uart, m_receiver.data(), m_uartSize);
                m_uartPending = true;
            }
        }
    }
    this->radioReturn_out(0, buffer, context);
}

void Gateway::radioEvent_handler(FwIndexType port, FwEventIdType id, Fw::Time& timeTag,
                                 const Fw::LogSeverity& severity, Fw::LogBuffer& args) {
    (void)port;
    (void)timeTag;
    (void)severity;
    (void)args;
    diagnostics.lastRadioEvent = id;
    increment(diagnostics.radioEvents);
}

void Gateway::uartReturn_handler(FwIndexType port, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) {
    (void)port;
    FW_ASSERT(m_uartInFlight && buffer.getData() == m_uart);
    m_uartInFlight = false;
    if (status == Drv::ByteStreamStatus::OP_OK) {
        m_uartPending = false;
    } else if (status != Drv::ByteStreamStatus::SEND_RETRY) {
        m_uartPending = false;
        increment(diagnostics.droppedFrames);
        increment(diagnostics.uartErrors);
    }
}

void Gateway::uartReceive_handler(FwIndexType port, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) {
    (void)port;
    if (status != Drv::ByteStreamStatus::OP_OK) {
        increment(diagnostics.uartErrors);
    } else if (buffer.getSize() > 0) {
        increment(diagnostics.unsupportedUartChunks);
    }
    this->uartReceiveReturn_out(0, buffer);
}

void Gateway::run_handler(FwIndexType port, U32 context) {
    (void)port;
    (void)context;
    if (m_uartPending && !m_uartInFlight) {
        m_uartInFlight = true;
        Fw::Buffer buffer(m_uart, m_uartSize);
        this->uartSend_out(0, buffer);
    }
}

}  // namespace RadioGateway
