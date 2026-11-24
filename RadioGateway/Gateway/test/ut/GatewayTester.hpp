#ifndef RADIO_GATEWAY_TESTER_HPP
#define RADIO_GATEWAY_TESTER_HPP

#include "RadioGateway/Gateway/Gateway.hpp"
#include "RadioGateway/Gateway/GatewayGTestBase.hpp"
#include <cstring>

namespace RadioGateway {

class GatewayTester final : public GatewayGTestBase {
  public:
    static constexpr FwEnumStoreType TEST_INSTANCE_ID = 0;
    GatewayTester() : GatewayGTestBase("GatewayTester", 50), component("Gateway") {
        initComponents();
        connectPorts();
        component.configure(0xc003);
    }
    ~GatewayTester() { component.deinit(); }

    void testReassemblyAndUartOwnership() {
        U8 frame[RadioTransport::MAX_FRAME_SIZE]{};
        for (std::size_t i = 0; i < sizeof frame; ++i) {
            frame[i] = static_cast<U8>(i);
        }
        sendFrame(frame, 1, false);
        ASSERT_from_uartSend_SIZE(0);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);
        Fw::Buffer first = fromPortHistory_uartSend->at(0).fwBuffer;
        ASSERT_EQ(sizeof frame, first.getSize());
        ASSERT_EQ(0, std::memcmp(first.getData(), frame, sizeof frame));
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);

        frame[0] = 77;
        sendFrame(frame, 2, false);
        EXPECT_EQ(1U, component.diagnostics.droppedFrames);
        EXPECT_EQ(0, first.getData()[0]);
        invoke_to_uartReturn(0, first, Drv::ByteStreamStatus::OP_OK);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);

        sendFrame(frame, 3, false);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(2);
        Fw::Buffer second = fromPortHistory_uartSend->at(1).fwBuffer;
        EXPECT_EQ(77, second.getData()[0]);
        invoke_to_uartReturn(0, second, Drv::ByteStreamStatus::SEND_RETRY);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(3);
        invoke_to_uartReturn(0, second, Drv::ByteStreamStatus::OP_OK);
    }

    void testMissingFragment() {
        U8 frame[RadioTransport::MAX_FRAME_SIZE]{};
        sendFrame(frame, 1, true);
        EXPECT_GT(component.diagnostics.rejectedPackets, 0U);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(0);
        sendFrame(frame, 2, false);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);
        Fw::Buffer sent = fromPortHistory_uartSend->at(0).fwBuffer;
        invoke_to_uartReturn(0, sent, Drv::ByteStreamStatus::OP_OK);
    }

    void testDiagnostics() {
        Fw::Buffer allocation = invoke_to_allocate(0, RadioTransport::MAX_PACKET_SIZE);
        ASSERT_NE(nullptr, allocation.getData());
        EXPECT_EQ(nullptr, invoke_to_allocate(0, 1).getData());
        invoke_to_deallocate(0, allocation);
        EXPECT_EQ(nullptr, invoke_to_allocate(0, RadioTransport::MAX_PACKET_SIZE + 1).getData());
        EXPECT_EQ(2U, component.diagnostics.allocationFailures);

        U8 invalid[]{1, 2};
        Fw::Buffer packet(invalid, sizeof invalid);
        ComCfg::FrameContext context;
        invoke_to_radioReceive(0, packet, context);
        EXPECT_EQ(1U, component.diagnostics.rejectedPackets);
        ASSERT_from_radioReturn_SIZE(1);

        U8 frame[RadioTransport::MAX_FRAME_SIZE]{};
        sendFrame(frame, 1, false);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);
        Fw::Buffer output = fromPortHistory_uartSend->at(0).fwBuffer;
        invoke_to_uartReturn(0, output, Drv::ByteStreamStatus::OTHER_ERROR);
        EXPECT_EQ(1U, component.diagnostics.uartErrors);
        EXPECT_EQ(1U, component.diagnostics.droppedFrames);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);

        Fw::Time timestamp;
        Fw::LogBuffer args;
        Fw::LogSeverity severity = Fw::LogSeverity::WARNING_HI;
        invoke_to_radioEvent(0, 123, timestamp, severity, args);
        EXPECT_EQ(1U, component.diagnostics.radioEvents);
        EXPECT_EQ(123U, component.diagnostics.lastRadioEvent);

        U8 command[]{1, 2};
        Fw::Buffer input(command, sizeof command);
        invoke_to_uartReceive(0, input, Drv::ByteStreamStatus::OP_OK);
        EXPECT_EQ(1U, component.diagnostics.unsupportedUartChunks);
        ASSERT_from_uartReceiveReturn_SIZE(1);
    }

    void testMixedAndInvalidFragments() {
        U8 frame[RadioTransport::MAX_FRAME_SIZE]{};
        U8 packet[RadioTransport::MAX_PACKET_SIZE];
        ComCfg::FrameContext context;
        auto send = [&](U32 id, std::size_t offset) {
            const auto size = RadioTransport::encodeFragment(RadioTransport::FrameKind::TELEMETRY, 1, id, frame,
                                                              sizeof frame, offset, packet, sizeof packet);
            Fw::Buffer input(packet, static_cast<FwSizeType>(size));
            invoke_to_radioReceive(0, input, context);
        };
        send(1, 0);
        send(2, RadioTransport::MAX_FRAGMENT_SIZE);
        EXPECT_EQ(1U, component.diagnostics.rejectedPackets);
        send(1, 0);
        U8 invalid[]{0};
        Fw::Buffer bad(invalid, sizeof invalid);
        invoke_to_radioReceive(0, bad, context);
        send(1, RadioTransport::MAX_FRAGMENT_SIZE);
        EXPECT_EQ(3U, component.diagnostics.rejectedPackets);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(0);
        sendFrame(frame, 3, false);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);
        Fw::Buffer output = fromPortHistory_uartSend->at(0).fwBuffer;
        invoke_to_uartReturn(0, output, Drv::ByteStreamStatus::OP_OK);
    }

    void testSharedBufferDuringRetryAndPartialOverlap() {
        U8 frame[RadioTransport::MAX_FRAME_SIZE]{};
        for (std::size_t i = 0; i < sizeof frame; ++i) {
            frame[i] = static_cast<U8>(i);
        }
        sendFrame(frame, 1, false);
        U8 packet[RadioTransport::MAX_PACKET_SIZE]{};
        ComCfg::FrameContext context;
        frame[0] = 77;
        auto size = RadioTransport::encodeFragment(RadioTransport::FrameKind::TELEMETRY, 0, 2, frame,
                                                    sizeof frame, 0, packet, sizeof packet);
        Fw::Buffer input(packet, static_cast<FwSizeType>(size));
        invoke_to_radioReceive(0, input, context);
        EXPECT_EQ(1U, component.diagnostics.droppedFrames);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);
        Fw::Buffer output = fromPortHistory_uartSend->at(0).fwBuffer;
        for (std::size_t i = 0; i < sizeof frame; ++i) {
            EXPECT_EQ(static_cast<U8>(i), output.getData()[i]);
        }
        invoke_to_uartReturn(0, output, Drv::ByteStreamStatus::SEND_RETRY);
        sendFrame(frame, 3, false);
        EXPECT_EQ(2U, component.diagnostics.droppedFrames);
        for (std::size_t i = 0; i < sizeof frame; ++i) {
            EXPECT_EQ(static_cast<U8>(i), output.getData()[i]);
        }
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(2);
        EXPECT_EQ(output.getData(), fromPortHistory_uartSend->at(1).fwBuffer.getData());
        invoke_to_uartReturn(0, output, Drv::ByteStreamStatus::OP_OK);
        size = RadioTransport::encodeFragment(RadioTransport::FrameKind::TELEMETRY, 0, 2, frame, sizeof frame,
                                              RadioTransport::MAX_FRAGMENT_SIZE, packet, sizeof packet);
        input = Fw::Buffer(packet, static_cast<FwSizeType>(size));
        invoke_to_radioReceive(0, input, context);
        EXPECT_GT(component.diagnostics.rejectedPackets, 0U);
        sendFrame(frame, 4, false);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(3);
        Fw::Buffer next = fromPortHistory_uartSend->at(2).fwBuffer;
        EXPECT_EQ(77U, next.getData()[0]);
        invoke_to_uartReturn(0, next, Drv::ByteStreamStatus::OP_OK);
    }

    void testImmediateUartRetry() {
        U8 frame[RadioTransport::MAX_FRAME_SIZE]{};
        frame[0] = 45;
        sendFrame(frame, 1, false);
        retryImmediately = true;
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(1);
        sendFrame(frame, 2, false);
        EXPECT_EQ(1U, component.diagnostics.droppedFrames);
        invoke_to_run(0, 0);
        ASSERT_from_uartSend_SIZE(2);
        Fw::Buffer output = fromPortHistory_uartSend->at(1).fwBuffer;
        EXPECT_EQ(45U, output.getData()[0]);
        invoke_to_uartReturn(0, output, Drv::ByteStreamStatus::OP_OK);
    }

  private:
    void initComponents();
    void connectPorts();
    void from_uartSend_handler(FwIndexType port, Fw::Buffer& buffer) override {
        GatewayTesterBase::from_uartSend_handler(port, buffer);
        if (retryImmediately) {
            retryImmediately = false;
            invoke_to_uartReturn(0, buffer, Drv::ByteStreamStatus::SEND_RETRY);
        }
    }
    void sendFrame(U8* frame, U32 id, bool skip) {
        U8 packet[RadioTransport::MAX_PACKET_SIZE];
        ComCfg::FrameContext context;
        unsigned fragment = 0;
        for (std::size_t offset = 0; offset < RadioTransport::MAX_FRAME_SIZE;
             offset += RadioTransport::MAX_FRAGMENT_SIZE, ++fragment) {
            const auto size = RadioTransport::encodeFragment(RadioTransport::FrameKind::TELEMETRY, 0, id, frame,
                                                              RadioTransport::MAX_FRAME_SIZE, offset, packet,
                                                              sizeof packet);
            if (skip && fragment == 1) {
                continue;
            }
            Fw::Buffer buffer(packet, static_cast<FwSizeType>(size));
            invoke_to_radioReceive(0, buffer, context);
        }
    }
    Gateway component;
    bool retryImmediately = false;
};

}  // namespace RadioGateway
#endif
