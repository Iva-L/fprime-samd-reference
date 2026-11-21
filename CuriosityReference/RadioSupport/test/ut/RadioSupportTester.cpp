#include "RadioSupportTester.hpp"

namespace CuriosityReference {

const FwSizeType RadioSupportTester::MAX_HISTORY_SIZE;
const FwEnumStoreType RadioSupportTester::TEST_INSTANCE_ID;

RadioSupportTester::RadioSupportTester()
    : RadioSupportGTestBase("RadioSupportTester", MAX_HISTORY_SIZE), component("RadioSupport") {
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

}  // namespace CuriosityReference
