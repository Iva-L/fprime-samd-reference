module RadioGateway {
    deployment topology Top {
        instance cycler
        instance dmaDriver
        instance timeHandler
        instance rateGroupDriver
        instance rateGroup
        instance rateDriver
        instance comDriver
        instance radioSpi
        instance radioCs
        instance radioEnable
        instance gateway
        instance radio

        telemetry packets GatewayStatus {
            packet Status group 0 {
                radio.PacketsReceived
            }
        } omit {
                rateGroupDriver.MaxCycleTime
                rateGroupDriver.CycleTime
                rateGroup.MaxCycleTime
                rateGroup.CycleTime
                rateGroup.CycleCount
                rateGroup.PortCycleTime
                rateGroup.PortCycleTimeHWM
                rateDriver.CycleOverrun
                comDriver.rxOverflows
                comDriver.rxBytes
                comDriver.txBytes
                radio.PacketsTransmitted
                radio.RxCrcErrors
                radio.LastPacketRssi
                radio.LastPacketSnr
                radio.RadioState
        }

        time connections instance timeHandler

        connections Cyclic {
            cycler.cycleOut[0] -> radioSpi.activeIn
            cycler.cycleOut[1] -> comDriver.activeIn
            cycler.cycleOut[2] -> rateDriver.activeIn
            rateDriver.CycleOut -> rateGroupDriver.CycleIn
            rateGroupDriver.CycleOut[0] -> rateGroup.CycleIn
            rateGroup.RateGroupMemberOut[0] -> comDriver.schedIn
            rateGroup.RateGroupMemberOut[1] -> radio.run
            rateGroup.RateGroupMemberOut[2] -> gateway.run
        }

        connections Uart {
            gateway.uartSend -> comDriver.$send
            comDriver.sendReturnOut -> gateway.uartReturn
            comDriver.$recv -> gateway.uartReceive
            gateway.uartReceiveReturn -> comDriver.recvReturnIn
            comDriver.dmaQueueOut[Samd21.UsartDriver.DmaChannel.TX] -> dmaDriver.sendTransactionIn[DmaChannel.USART_TX]
            dmaDriver.transactionIsrOut[DmaChannel.USART_TX] -> comDriver.dmaReplyIn[Samd21.UsartDriver.DmaChannel.TX]
            comDriver.dmaQueueOut[Samd21.UsartDriver.DmaChannel.RX] -> dmaDriver.sendTransactionIn[DmaChannel.USART_RX]
            dmaDriver.transactionIsrOut[DmaChannel.USART_RX] -> comDriver.dmaReplyIn[Samd21.UsartDriver.DmaChannel.RX]
            comDriver.dmaRxCircular -> dmaDriver.linkToFrontIn[DmaChannel.USART_RX]
            comDriver.dmaRxRead -> dmaDriver.readWritebackIn[DmaChannel.USART_RX]
        }

        connections Radio {
            radio.spiWriteRead -> radioSpi.SpiWriteRead[0]
            radioSpi.SpiWriteReadComplete[0] -> radio.spiWriteReadComplete
            radioSpi.chipSelectGpioOut[0] -> radioCs.gpioWrite
            radio.resetGpio -> radioEnable.gpioWrite
            radioSpi.dmaTransactionOut[Samd21.SpiDriver.DmaChannel.MOSI] -> dmaDriver.sendTransactionIn[DmaChannel.SPI_MOSI]
            radioSpi.dmaTransactionAbortOut[Samd21.SpiDriver.DmaChannel.MOSI] -> dmaDriver.abortTransactionIn[DmaChannel.SPI_MOSI]
            dmaDriver.transactionIsrOut[DmaChannel.SPI_MOSI] -> radioSpi.dmaReplyIn[Samd21.SpiDriver.DmaChannel.MOSI]
            radioSpi.dmaTransactionOut[Samd21.SpiDriver.DmaChannel.MISO] -> dmaDriver.sendTransactionIn[DmaChannel.SPI_MISO]
            radioSpi.dmaTransactionAbortOut[Samd21.SpiDriver.DmaChannel.MISO] -> dmaDriver.abortTransactionIn[DmaChannel.SPI_MISO]
            dmaDriver.transactionIsrOut[DmaChannel.SPI_MISO] -> radioSpi.dmaReplyIn[Samd21.SpiDriver.DmaChannel.MISO]
            radio.allocate -> gateway.allocate
            radio.deallocate -> gateway.deallocate
            radio.prmGet -> gateway.getParameter
            radio.dataOut -> gateway.radioReceive
            gateway.radioReturn -> radio.dataReturnIn
            radio.logOut -> gateway.radioEvent
        }
    }
}
