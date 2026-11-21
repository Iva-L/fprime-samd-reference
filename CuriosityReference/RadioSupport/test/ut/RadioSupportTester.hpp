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

  private:
    void connectPorts();
    void initComponents();
    Fw::Buffer allocate(FwSizeType size);
    void deallocate(Fw::Buffer& buffer);
    Fw::ParamValid getParameter(FwPrmIdType id, Fw::ParamBuffer& value);

    RadioSupport component;
};

}  // namespace CuriosityReference
#endif
