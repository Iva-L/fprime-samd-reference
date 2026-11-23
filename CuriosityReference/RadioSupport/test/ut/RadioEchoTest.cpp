#include "CuriosityReference/RadioSupport/RadioSupport.hpp"
#include "Rfm9XManagerGTestBase.hpp"
#include "fprime-sensors/Rfm9X/Components/Rfm9XManager/Rfm9XManager.hpp"
#include "fprime-sensors/Rfm9X/Components/Rfm9XSim/Rfm9XSimModel.hpp"

namespace {

class RadioEchoTester final : public Rfm9X::Rfm9XManagerGTestBase {
  public:
    RadioEchoTester()
        : Rfm9XManagerGTestBase("RadioEchoTester", 2048),
          radio("radio"),
          support("support"),
          model(),
          pending(false),
          pendingWrite(),
          pendingRead() {
        this->init();
        this->setIdBase(0xD000);
        this->radio.init(0);
        this->support.init(0);
        this->radio.setIdBase(0xD000);
        this->support.configure(0xD003);

        this->radio.set_CmdStatus_OutputPort(0, this->get_from_CmdStatus(0));
        this->radio.set_logOut_OutputPort(0, this->get_from_logOut(0));
#if FW_ENABLE_TEXT_LOGGING == 1
        this->radio.set_logTextOut_OutputPort(0, this->get_from_logTextOut(0));
#endif
        this->radio.set_timeCaller_OutputPort(0, this->get_from_timeCaller(0));
        this->radio.set_tlmOut_OutputPort(0, this->get_from_tlmOut(0));
        this->radio.set_prmGet_OutputPort(0, this->support.get_getParameter_InputPort(0));
        this->radio.set_allocate_OutputPort(0, this->support.get_allocate_InputPort(0));
        this->radio.set_deallocate_OutputPort(0, this->support.get_deallocate_InputPort(0));
        this->radio.set_dataOut_OutputPort(0, this->support.get_receive_InputPort(0));
        this->support.set_transmit_OutputPort(0, this->radio.get_dataIn_InputPort(0));
        this->radio.set_dataReturnOut_OutputPort(0, this->support.get_sendReturn_InputPort(0));
        this->support.set_returnBuffer_OutputPort(0, this->radio.get_dataReturnIn_InputPort(0));
        this->radio.set_resetGpio_OutputPort(0, this->get_from_resetGpio(0));
        this->radio.set_spiWriteRead_OutputPort(0, this->get_from_spiWriteRead(0));
        this->connect_to_run(0, this->radio.get_run_InputPort(0));
        this->connect_to_spiWriteReadComplete(0, this->radio.get_spiWriteReadComplete_InputPort(0));
        this->radio.configure(125);
        this->radio.loadParameters();
    }

    void testEcho() {
        for (U32 tick = 0; tick < 1024 && this->eventsSize_RadioReady == 0; tick++) {
            this->tick();
        }
        ASSERT_EVENTS_RadioReady_SIZE(1);
        ASSERT_EQ(this->model.readRegisterValue(Rfm9X::Reg::FRF_MSB), 0xE4);
        ASSERT_EQ(this->model.readRegisterValue(Rfm9X::Reg::FRF_MID), 0xC0);
        ASSERT_EQ(this->model.readRegisterValue(Rfm9X::Reg::FRF_LSB), 0x00);

        U8 payload[255];
        for (FwSizeType i = 0; i < sizeof(payload); i++) {
            payload[i] = static_cast<U8>(i);
        }
        this->model.injectAirData(payload, sizeof(payload));
        U8 echoed[sizeof(payload)] = {};
        FwSizeType size = 0;
        for (U32 tick = 0; tick < 2048 && size == 0; tick++) {
            this->tick();
            size = this->model.retrievePacket(echoed, sizeof(echoed));
        }
        ASSERT_EQ(size, sizeof(payload)) << "received telemetry entries: " << this->tlmHistory_PacketsReceived->size()
                                         << ", transmitted telemetry entries: "
                                         << this->tlmHistory_PacketsTransmitted->size()
                                         << ", send failures: " << this->eventHistory_SendFailed->size() << ", reason: "
                                         << (this->eventHistory_SendFailed->size() > 0
                                                 ? static_cast<int>(this->eventHistory_SendFailed->at(0).reason.e)
                                                 : -1)
                                         << ", allocation failures: " << this->eventHistory_AllocationFailed->size();
        for (FwSizeType i = 0; i < size; i++) {
            ASSERT_EQ(echoed[i], payload[i]);
        }

        // Wait for TX completion and buffer return, then prove it can echo again.
        for (U32 tick = 0; tick < 128; tick++) {
            this->tick();
        }
        const U8 second[] = {0, 0x7F, 0xFF};
        this->model.injectAirData(second, sizeof(second));
        size = 0;
        for (U32 tick = 0; tick < 2048 && size == 0; tick++) {
            this->tick();
            size = this->model.retrievePacket(echoed, sizeof(echoed));
        }
        ASSERT_EQ(size, sizeof(second));
        for (FwSizeType i = 0; i < size; i++) {
            ASSERT_EQ(echoed[i], second[i]);
        }
    }

  private:
    void from_spiWriteRead_handler(FwIndexType portNum, Fw::Buffer& write, Fw::Buffer& read) override {
        (void)portNum;
        ASSERT_FALSE(this->pending);
        ASSERT_EQ(write.getSize(), read.getSize());
        this->model.spiTransaction(write.getData(), read.getData(), write.getSize());
        this->pendingWrite = write;
        this->pendingRead = read;
        this->pending = true;
    }

    void tick() {
        // The model advances airtime only on SPI bytes, while real airtime
        // also passes during the 125 ms between rate-group ticks.
        const U8 clock[] = {Rfm9X::Reg::VERSION, 0};
        U8 ignored[sizeof(clock)] = {};
        for (U32 i = 0; i < 32; i++) {
            this->model.spiTransaction(clock, ignored, sizeof(clock));
        }
        this->invoke_to_run(0, 0);
        if (this->pending) {
            this->pending = false;
            Fw::Buffer write = this->pendingWrite;
            Fw::Buffer read = this->pendingRead;
            this->invoke_to_spiWriteReadComplete(0, write, read, Drv::SpiStatus::SPI_OK);
        }
    }

    Rfm9X::Rfm9XManager radio;
    CuriosityReference::RadioSupport support;
    Rfm9X::Rfm9XSimModel model;
    bool pending;
    Fw::Buffer pendingWrite;
    Fw::Buffer pendingRead;
};

TEST(RadioEcho, BinaryPayloadAndBufferReuse) {
    RadioEchoTester tester;
    tester.testEcho();
}

}  // namespace
