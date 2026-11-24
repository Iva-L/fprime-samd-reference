#include <RadioGateway/Top/TopTopologyAc.hpp>
#include <RadioGateway/Top/TopTopology.hpp>
#include <Fw/Com/ComBuffer.hpp>

namespace Samd21 {
volatile U32 gatewayFatalPackets = 0;
volatile U32 gatewayBailLine = 0;
void sendFatalPacket(Fw::ComBuffer& data) {
    (void)data;
    ++gatewayFatalPackets;
}
void sendBailFrame(FILE_NAME_ARG file, FwSizeType line) {
    (void)file;
    gatewayBailLine = line;
}
}

namespace RadioGateway {
void setupTopology(const TopologyState& state) {
    initComponents(state);
    setBaseIds();
    dmaDriver.configure();
    connectComponents();
    configComponents(state);
    Samd21::PassiveRateGroupDriver::DividerSet divisors{{{1, 0}, {8, 0}, {80, 0}}};
    Svc::PassiveRateGroup::ContextArray context{};
    rateGroupDriver.configure(divisors);
    rateGroup.configure(context);
    startTasks(state);
}
}
