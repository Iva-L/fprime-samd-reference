#include "RadioSupportTester.hpp"

namespace CuriosityReference {

const FwSizeType RadioSupportTester::MAX_HISTORY_SIZE;
const FwEnumStoreType RadioSupportTester::TEST_INSTANCE_ID;

RadioSupportTester::RadioSupportTester()
    : RadioSupportGTestBase("RadioSupportTester", MAX_HISTORY_SIZE), component("RadioSupport"), returnImmediately(false) {
    this->initComponents();
    this->connectPorts();
    this->component.configure(0xD003);
}

RadioSupportTester::~RadioSupportTester() {
    this->component.deinit();
}

Fw::Buffer RadioSupportTester::allocate(FwSizeType size) {
    return this->invoke_to_allocate(0, size);
}

void RadioSupportTester::deallocate(Fw::Buffer& buffer) {
    this->invoke_to_deallocate(0, buffer);
}

Fw::ParamValid RadioSupportTester::getParameter(FwPrmIdType id, Fw::ParamBuffer& value) {
    return this->invoke_to_getParameter(0, id, value);
}

void RadioSupportTester::from_transmit_handler(FwIndexType portNum, Fw::Buffer& data,
                                               const ComCfg::FrameContext& context) {
    this->pushFromPortEntry_transmit(data, context);
    if (this->returnImmediately) {
        this->invoke_to_sendReturn(0, data, context);
    }
}

void RadioSupportTester::testBufferOwnership() {
    Fw::Buffer oversized = this->allocate(Rfm9X::ManagerCfg::MAX_PAYLOAD_LENGTH + 1);
    ASSERT_EQ(oversized.getData(), nullptr);
    this->deallocate(oversized);

    Fw::Buffer buffer = this->allocate(Rfm9X::ManagerCfg::MAX_PAYLOAD_LENGTH);
    ASSERT_NE(buffer.getData(), nullptr);
    ASSERT_EQ(buffer.getSize(), Rfm9X::ManagerCfg::MAX_PAYLOAD_LENGTH);
    Fw::Buffer busy = this->allocate(1);
    ASSERT_EQ(busy.getData(), nullptr);
    this->deallocate(busy);
    ASSERT_EQ(this->allocate(1).getData(), nullptr);
    this->deallocate(buffer);
    Fw::Buffer reused = this->allocate(1);
    ASSERT_EQ(reused.getData(), buffer.getData());
    this->deallocate(reused);
}

void RadioSupportTester::testStartupFrequency() {
    Fw::ParamBuffer value;
    ASSERT_EQ(this->getParameter(0xD002, value), Fw::ParamValid::INVALID);
    ASSERT_EQ(this->getParameter(0xD003, value), Fw::ParamValid::VALID);
    U32 frequency = 0;
    ASSERT_EQ(value.deserializeTo(frequency), Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(frequency, 915000000U);
}

void RadioSupportTester::testSendString() {
    this->sendCmd_SEND_STRING(0, 0, Fw::CmdStringArg(""));
    ASSERT_CMD_RESPONSE(0, RadioSupportComponentBase::OPCODE_SEND_STRING, 0, Fw::CmdResponse::EXECUTION_ERROR);
    this->clearHistory();
    this->sendCmd_SEND_STRING(0, 1, Fw::CmdStringArg("hi"));
    ASSERT_CMD_RESPONSE(0, RadioSupportComponentBase::OPCODE_SEND_STRING, 1, Fw::CmdResponse::OK);
    ASSERT_from_transmit_SIZE(1);
    Fw::Buffer sent = this->fromPortHistory_transmit->at(0).data;
    ComCfg::FrameContext context = this->fromPortHistory_transmit->at(0).context;
    ASSERT_EQ(sent.getSize(), 2u);
    ASSERT_EQ(sent.getData()[0], 'h');
    ASSERT_EQ(sent.getData()[1], 'i');
    this->sendCmd_SEND_STRING(0, 2, Fw::CmdStringArg("no"));
    ASSERT_CMD_RESPONSE(1, RadioSupportComponentBase::OPCODE_SEND_STRING, 2, Fw::CmdResponse::EXECUTION_ERROR);
    this->invoke_to_sendReturn(0, sent, context);
    this->sendCmd_SEND_STRING(0, 3, Fw::CmdStringArg("ok"));
    ASSERT_CMD_RESPONSE(2, RadioSupportComponentBase::OPCODE_SEND_STRING, 3, Fw::CmdResponse::OK);
    Fw::Buffer second = this->fromPortHistory_transmit->at(1).data;
    this->invoke_to_sendReturn(0, second, context);
    this->returnImmediately = true;
    this->sendCmd_SEND_STRING(0, 4, Fw::CmdStringArg("x"));
    ASSERT_CMD_RESPONSE(3, RadioSupportComponentBase::OPCODE_SEND_STRING, 4, Fw::CmdResponse::EXECUTION_ERROR);
    this->returnImmediately = false;
    this->sendCmd_SEND_STRING(0, 5, Fw::CmdStringArg("y"));
    ASSERT_CMD_RESPONSE(4, RadioSupportComponentBase::OPCODE_SEND_STRING, 5, Fw::CmdResponse::OK);
    Fw::Buffer last = this->fromPortHistory_transmit->at(3).data;
    this->invoke_to_sendReturn(0, last, context);
}

}  // namespace CuriosityReference
