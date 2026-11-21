#include "CuriosityReference/RadioSupport/RadioSupport.hpp"

#include <Fw/Types/Assert.hpp>

namespace CuriosityReference {

RadioSupport::RadioSupport(const char* name)
    : RadioSupportComponentBase(name), m_buffer{}, m_allocated(false), m_frequencyId(0) {}

void RadioSupport::configure(FwPrmIdType frequencyId) {
    this->m_frequencyId = frequencyId;
}

Fw::Buffer RadioSupport::allocate_handler(FwIndexType portNum, FwSizeType size) {
    (void)portNum;
    if (this->m_allocated || size > sizeof(this->m_buffer)) {
        return Fw::Buffer();
    }
    this->m_allocated = true;
    return Fw::Buffer(this->m_buffer, sizeof(this->m_buffer));
}

void RadioSupport::deallocate_handler(FwIndexType portNum, Fw::Buffer& buffer) {
    (void)portNum;
    if (buffer.getData() == nullptr && buffer.getSize() == 0) {
        return;
    }
    FW_ASSERT(this->m_allocated);
    FW_ASSERT(buffer.getData() == this->m_buffer);
    this->m_allocated = false;
}

Fw::ParamValid RadioSupport::getParameter_handler(FwIndexType portNum, FwPrmIdType id, Fw::ParamBuffer& value) {
    (void)portNum;
    if (id != this->m_frequencyId) {
        return Fw::ParamValid::INVALID;
    }
    const Fw::SerializeStatus status = value.serializeFrom(static_cast<U32>(915000000));
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    return Fw::ParamValid::VALID;
}

}  // namespace CuriosityReference
