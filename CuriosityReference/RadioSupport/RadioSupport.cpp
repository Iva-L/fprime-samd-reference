#include "CuriosityReference/RadioSupport/RadioSupport.hpp"

#include <Fw/Types/Assert.hpp>
#include <cstring>

namespace CuriosityReference {

RadioSupport::RadioSupport(const char* name)
    : RadioSupportComponentBase(name), m_buffer{}, m_allocated(false), m_frequencyId(0), m_lock(), m_payload{},
      m_pending(false), m_submitting(false), m_rejected(false) {}

void RadioSupport::configure(FwPrmIdType frequencyId) {
    this->m_frequencyId = frequencyId;
}

void RadioSupport::SEND_STRING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdStringArg& payload) {
    const FwSizeType size = payload.length();
    bool accepted = false;
    {
        Os::ScopeLock lock(this->m_lock);
        if (size > 0 && size <= sizeof(this->m_payload) && !this->m_pending) {
            std::memcpy(this->m_payload, payload.toChar(), size);
            this->m_pending = true;
            this->m_submitting = true;
            this->m_rejected = false;
            accepted = true;
        }
    }
    if (accepted) {
        Fw::Buffer buffer(this->m_payload, size);
        ComCfg::FrameContext context;
        this->transmit_out(0, buffer, context);
        Os::ScopeLock lock(this->m_lock);
        this->m_submitting = false;
        accepted = !this->m_rejected;
        if (!accepted) {
            this->m_pending = false;
        }
    }
    if (accepted) {
        this->log_ACTIVITY_HI_TestPayloadQueued(size);
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    } else {
        this->log_WARNING_HI_TestSendRejected();
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

void RadioSupport::receive_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    (void)portNum;
    this->transmit_out(0, data, context);
}

void RadioSupport::sendReturn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    (void)portNum;
    if (data.getData() == this->m_payload) {
        Os::ScopeLock lock(this->m_lock);
        FW_ASSERT(this->m_pending);
        if (this->m_submitting) {
            this->m_rejected = true;
        } else {
            this->m_pending = false;
        }
    } else {
        this->returnBuffer_out(0, data, context);
    }
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
