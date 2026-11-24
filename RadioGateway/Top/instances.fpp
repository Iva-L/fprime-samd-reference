module RadioGateway {
    enum DmaChannel : U8 {
        USART_TX
        USART_RX
        SPI_MOSI
        SPI_MISO
    }

    instance cycler: Svc.PassiveCycler base id 0x1000
    instance dmaDriver: Samd21.DmaDriver base id 0x2000
    instance timeHandler: Samd21.ProcTime base id 0x3000
    instance rateGroupDriver: Samd21.PassiveRateGroupDriver base id 0x4000
    instance rateGroup: Svc.PassiveRateGroup base id 0x5000
    instance rateDriver: Samd21.RtcDriver base id 0x6000 {
        phase Fpp.ToCpp.Phases.configComponents """
        rateDriver.configure(Samd21::RtcDriver::ClockSource::UltraLowPowerOscillator,
                             Samd21::RtcDriver::TickRate::TICK_8_HZ);
        """
        phase Fpp.ToCpp.Phases.startTasks """
        rateDriver.enable();
        """
    }
    instance comDriver: Samd21.UsartDriver base id 0x7000 {
        phase Fpp.ToCpp.Phases.configComponents """
        Samd21::PinMux::configure(PINMUX_PA22D_SERCOM5_PAD0);
        Samd21::PinMux::configure(PINMUX_PB22D_SERCOM5_PAD2);
        comDriver.configure(Samd21::SercomKind::SERCOM_5,
                            Samd21::UsartDriver::RxPinOut::PAD2,
                            Samd21::UsartDriver::TxPinOut::PAD0,
                            Samd21::UsartDriver::ClockMode::INTERNAL,
                            Samd21::UsartDriver::CommunicationMode::ASYNC,
                            Samd21::UsartDriver::BaudRate::BAUD_115200,
                            Samd21::UsartDriver::DataOrder::LSB_FIRST,
                            Samd21::UsartDriver::DataBits::BITS_8,
                            Samd21::UsartDriver::StopBits::ONE,
                            Samd21::UsartDriver::Parity::NONE);
        """
    }
    instance radioSpi: Samd21.SpiDriver base id 0x8000 {
        phase Fpp.ToCpp.Phases.configComponents """
        radioCs.configureOutput(Samd21::GpioDriver::Group::PA, Samd21::GpioDriver::Pin::PIN_18, Fw::Logic::HIGH);
        radioEnable.configureOutput(Samd21::GpioDriver::Group::PA, Samd21::GpioDriver::Pin::PIN_20, Fw::Logic::LOW);
        Samd21::PinMux::configure(PINMUX_PA16C_SERCOM1_PAD0);
        Samd21::PinMux::configure(PINMUX_PA17C_SERCOM1_PAD1);
        Samd21::PinMux::configure(PINMUX_PA19C_SERCOM1_PAD3);
        radioSpi.configure(Samd21::SercomKind::SERCOM_1, 1000,
                           Samd21::SpiDriver::DataOrder::MSB,
                           Samd21::SpiDriver::ClockPolarity::IdleLow,
                           Samd21::SpiDriver::ClockPhase::SampleOnLeadingSck,
                           Samd21::SpiDriver::DataInPinout::PAD3,
                           Samd21::SpiDriver::DataOutPinout::MOSI_0_SCK_1_CS_2,
                           Samd21::SpiDriver::RunInStandby::DISABLED,
                           Samd21::SpiDriver::HardwareChipSelect::DISABLED);
        """
    }
    instance radioCs: Samd21.GpioDriver base id 0x9000
    instance radioEnable: Samd21.GpioDriver base id 0xA000
    instance gateway: RadioGateway.Gateway base id 0xB000
    instance radio: Rfm9X.Rfm9XManager base id 0xC000 {
        phase Fpp.ToCpp.Phases.startTasks """
        gateway.configure(radio.getIdBase() + 3);
        radio.configure(125);
        radio.loadParameters();
        """
    }
}
