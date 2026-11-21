#ifndef CURIOSITY_REFERENCE_RADIO_SUPPORT_HPP
#define CURIOSITY_REFERENCE_RADIO_SUPPORT_HPP

#include "CuriosityReference/RadioSupport/RadioSupportComponentAc.hpp"
#include "fprime-sensors/Rfm9X/Components/Rfm9XManager/Rfm9XManagerCfg/FppConstantsAc.hpp"

namespace CuriosityReference {

class RadioSupport final : public RadioSupportComponentBase {
  public:
    explicit RadioSupport(const char* name);
    void configure(FwPrmIdType frequencyId);

  private:
    Fw::Buffer allocate_handler(FwIndexType portNum, FwSizeType size) override;
    void deallocate_handler(FwIndexType portNum, Fw::Buffer& buffer) override;
    Fw::ParamValid getParameter_handler(FwIndexType portNum, FwPrmIdType id, Fw::ParamBuffer& value) override;

    U8 m_buffer[Rfm9X::ManagerCfg::MAX_PAYLOAD_LENGTH];
    bool m_allocated;
    FwPrmIdType m_frequencyId;
};

}  // namespace CuriosityReference
#endif
