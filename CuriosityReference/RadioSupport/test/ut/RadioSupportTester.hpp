#ifndef CURIOSITY_REFERENCE_RADIO_SUPPORT_TESTER_HPP
#define CURIOSITY_REFERENCE_RADIO_SUPPORT_TESTER_HPP

#include "CuriosityReference/RadioSupport/RadioSupport.hpp"
#include "CuriosityReference/RadioSupport/RadioSupportGTestBase.hpp"

namespace CuriosityReference {

class RadioSupportTester final : public RadioSupportGTestBase {
  public:
    static const FwSizeType MAX_HISTORY_SIZE = 10;
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    RadioSupportTester();
    ~RadioSupportTester();
    void testBufferOwnership();
    void testStartupFrequency();
    void testSendString();

  private:
    void connectPorts();
    void initComponents();
    Fw::Buffer allocate(FwSizeType size);
    void deallocate(Fw::Buffer& buffer);
    Fw::ParamValid getParameter(FwPrmIdType id, Fw::ParamBuffer& value);
    void from_transmit_handler(FwIndexType portNum, Fw::Buffer& data,
                               const ComCfg::FrameContext& context) override;

    RadioSupport component;
    bool returnImmediately;
};

}  // namespace CuriosityReference
#endif
