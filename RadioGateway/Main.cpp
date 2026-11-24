#include <RadioGateway/Top/TopTopologyAc.hpp>
#include <RadioGateway/Top/TopTopology.hpp>
#include <Fw/Types/Assert.hpp>

void operator delete(void* ptr) noexcept {
    FW_ASSERT(false, static_cast<FwAssertArgType>(reinterpret_cast<PlatformPointerCastType>(ptr)));
}

void operator delete(void* ptr, unsigned int size) noexcept {
    (void)size;
    FW_ASSERT(false, static_cast<FwAssertArgType>(reinterpret_cast<PlatformPointerCastType>(ptr)));
}

int main() {
    RadioGateway::TopologyState state{};
    RadioGateway::setupTopology(state);
    for (;;) {
        RadioGateway::cycler.cycle();
        asm("wfi");
    }
}
