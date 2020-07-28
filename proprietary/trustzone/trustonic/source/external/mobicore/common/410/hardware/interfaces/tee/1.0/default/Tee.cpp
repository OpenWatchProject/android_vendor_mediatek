/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "vendor.trustonic.tee@1.0-service"

//#define LOG_NDEBUG 0
//#define TEESERVICE_DEBUG
//#define TEESERVICE_MEMLEAK_CHECK

#include <mutex>
#include <vector>

#include <utils/Log.h>

#include <hidlmemory/mapping.h>

#include "driver.h"
#include "gp_types.h"
#include "mc_types.h"
#include "Tee.h"


class ::vendor::trustonic::Service {
    std::shared_ptr<::Driver> service_;
public:
    bool create() {
        if (!service_) {
            service_ = std::make_shared<::Driver>();
        }

        return service_.get();
    }

    auto get() const {
        return service_;
    }
};

namespace vendor {
namespace trustonic {
namespace tee {
namespace V1_0 {
namespace implementation {

struct Tee::Impl {
    GpManager gp_manager;
    McManager mc_manager;
    std::shared_ptr<TrustonicService> parent;

    class Buffer: public IBuffer {
        void* dest_ = nullptr;
        uint32_t size_ = 0;
        // Don't use after create as reference may be dangling
        const ::android::hardware::hidl_memory& hidl_memory_;
        ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    public:
        Buffer(
                const ::android::hardware::hidl_memory& hidl_memory,
                uint32_t size):
            size_(size), hidl_memory_(hidl_memory) {}

        Buffer(
                const std::shared_ptr<GpContext>& context,
                const ::android::hardware::hidl_memory& hidl_memory,
                uint64_t reference,
                uint32_t size,
                uint32_t flags):
            IBuffer(context, flags), size_(size), hidl_memory_(hidl_memory) {
            setReference(reference);
        }

        bool create() {
            if (hidl_memory_.size() < size_) {
                return false;
            }

            memory_ = ::android::hardware::mapMemory(hidl_memory_);
            if (!memory_) {
                ALOGE("Failed to map source buffer.");
                return false;
            }

            dest_ = memory_->getPointer();
            return true;
        }

        int fd() const override {
            return -1;
        }

        void* address() const override {
            return dest_;
        }

        uint32_t size() const override {
            return size_;
        }

        void updateDestination() override {
            // Copied in all cases so the copy back does not erase the data
            memory_->update();
        }

        void updateSource() override {
            if (flags() & TEEC_MEM_OUTPUT) {
                memory_->commit();
            }
        }
    };

    static TEEC_Result fromOperation(
            const std::shared_ptr<GpContext>& context,
            const ITee::Operation& operation_in,
            std::unique_ptr<TEEC_Operation>& operation_out,
            std::vector<std::shared_ptr<IBuffer>>& buffers,
            std::vector<std::unique_ptr<TEEC_SharedMemory>>& parents) {
        if (!operation_in.reference) {
            return TEEC_SUCCESS;
        }

        operation_out.reset(new TEEC_Operation);
        operation_out->paramTypes = 0;
        operation_out->started = operation_in.started;
        for (size_t i = 0; i < 4; i++) {
            const ITee::Operation::Param& param_in = operation_in.params[i];
            TEEC_Parameter& param_out = operation_out->params[i];
            operation_out->paramTypes |= param_in.type << (4 * i);
            switch (param_in.type) {
                case TEEC_NONE:
                case TEEC_VALUE_OUTPUT:
                    break;
                case TEEC_VALUE_INPUT:
                case TEEC_VALUE_INOUT:
                    param_out.value.a = param_in.value_a;
                    param_out.value.b = param_in.value_b;
                    break;
                case TEEC_MEMREF_TEMP_OUTPUT:
                case TEEC_MEMREF_TEMP_INPUT:
                case TEEC_MEMREF_TEMP_INOUT:
                    // A Temporary Memory Reference may be null, which can be
                    // used to request the required output size.
                    if (param_in.has_buffer) {
                        auto buffer = std::make_shared<Buffer>(
                                          context, param_in.buffer,
                                          param_in.reference, param_in.size,
                                          param_in.flags);
                        if (!buffer->create()) {
                            // Log'd in callee
                            return TEEC_ERROR_OUT_OF_MEMORY;
                        }

                        buffers.push_back(buffer);
                        param_out.tmpref.buffer = buffer->address();
                    } else {
                        param_out.tmpref.buffer = nullptr;
                    }

                    param_out.tmpref.size = param_in.size;
                    break;
                case TEEC_MEMREF_WHOLE:
                case TEEC_MEMREF_PARTIAL_OUTPUT:
                case TEEC_MEMREF_PARTIAL_INPUT:
                case TEEC_MEMREF_PARTIAL_INOUT:
                    auto buffer = context->getBuffer(param_in.reference);
                    if (!buffer) {
                        ALOGE("Failed to get buffer %jx for context %jx.",
                              param_in.reference, context->reference());
                        return TEEC_ERROR_BAD_PARAMETERS;
                    }

                    buffers.push_back(buffer);
                    auto parent = new TEEC_SharedMemory;
                    parents.emplace_back(parent);
                    param_out.memref.parent = parent;
                    param_out.memref.parent->buffer = buffer->address();
                    param_out.memref.parent->size = param_in.size;
                    param_out.memref.parent->flags = param_in.flags;
                    param_out.memref.offset = param_in.window_offset;
                    param_out.memref.size = param_in.window_size;
                    break;
            }
        }

        for (const auto& buffer: buffers) {
            buffer->updateDestination();
        }

        return TEEC_SUCCESS;
    }

    static void toOperation(
            const std::unique_ptr<TEEC_Operation>& operation_in,
            ITee::Operation& operation_out,
            std::vector<std::shared_ptr<IBuffer>>& buffers) {
        if (!operation_in) {
            return;
        }

        operation_out.started = operation_in->started;
        for (size_t i = 0; i < 4; i++) {
            const TEEC_Parameter& param_in = operation_in->params[i];
            ITee::Operation::Param& param_out = operation_out.params[i];
            param_out.type = (operation_in->paramTypes >> (4 * i)) & 0xf;
            switch (param_out.type) {
                case TEEC_NONE:
                case TEEC_VALUE_INPUT:
                    break;
                case TEEC_VALUE_OUTPUT:
                case TEEC_VALUE_INOUT:
                    param_out.value_a = param_in.value.a;
                    param_out.value_b = param_in.value.b;
                    break;
                case TEEC_MEMREF_TEMP_OUTPUT:
                case TEEC_MEMREF_TEMP_INPUT:
                case TEEC_MEMREF_TEMP_INOUT:
                    param_out.size = param_in.tmpref.size;
                    break;
                case TEEC_MEMREF_WHOLE:
                case TEEC_MEMREF_PARTIAL_OUTPUT:
                case TEEC_MEMREF_PARTIAL_INPUT:
                case TEEC_MEMREF_PARTIAL_INOUT:
                    // Whole also uses the partial field when returning a size
                    param_out.size = param_in.memref.size;
                    break;
            }
        }

        for (const auto& buffer: buffers) {
            buffer->updateSource();
        }
    }
};

// Using new here because std::make_unique is not part of C++11
Tee::Tee(std::shared_ptr<TrustonicService> service_): pimpl_(new Impl) {
    pimpl_->parent = service_;
}

Tee::~Tee() {
}

Return<void> Tee::flushAll() {
    pimpl_->gp_manager.flush();
    pimpl_->mc_manager.flush();
    return Void();
}

Return<void> Tee::flushPid(
        int32_t client_pid) {
    pimpl_->gp_manager.flushPid(client_pid);
    pimpl_->mc_manager.flushPid(client_pid);
    return Void();
}

Return<uint32_t> Tee::TEEC_InitializeContext(
        int32_t pid,
        uint64_t context,
        const ::android::hardware::hidl_string& name,
        bool has_name) {
    ALOGH("%s called by PID %d context %jx", __func__, pid, context);
    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to connect to service.");
        return TEEC_ERROR_COMMUNICATION;
    }

    auto gp_context = std::make_shared<GpContext>(service, context);
    const char* gp_name = nullptr;
    if (has_name) {
        gp_name = name.c_str();
    }

    auto result = service->get()->TEEC_InitializeContext(
                      gp_name, &gp_context->gp());
    if (result != TEEC_SUCCESS) {
        ALOGE("%s returns %u", __func__, result);
    } else {
        pimpl_->gp_manager.addContext(pid, gp_context);
    }

    return result;
}

Return<void> Tee::TEEC_FinalizeContext(
        int32_t pid,
        uint64_t context) {
    ALOGH("%s called by PID %d context %jx", __func__, pid, context);
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        return Void();
    }

    gp_context->getService()->get()->TEEC_FinalizeContext(&gp_context->gp());
    pimpl_->gp_manager.removeContext(pid, context);
    return Void();
}

Return<uint32_t> Tee::TEEC_RegisterSharedMemory(
        int32_t pid,
        uint64_t context,
        const ITee::GpSharedMemory& shr_mem) {
    ALOGH("%s called by PID %d context %jx", __func__, pid, context);
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        return TEEC_ERROR_BAD_STATE;
    }

    auto buffer = std::make_shared<Impl::Buffer>(
                      gp_context, shr_mem.buffer, shr_mem.reference,
                      shr_mem.size, shr_mem.flags);
    if (!buffer->create()) {
        // Log'd in callee
        return TEEC_ERROR_OUT_OF_MEMORY;
    }

    TEEC_SharedMemory gp_shr_mem;
    gp_shr_mem.buffer = buffer->address();
    gp_shr_mem.size = buffer->size();
    gp_shr_mem.flags = buffer->flags();
    auto result = gp_context->getService()->get()->TEEC_RegisterSharedMemory(
                      &gp_context->gp(), &gp_shr_mem);
    if (result == TEEC_SUCCESS) {
        gp_context->addBuffer(buffer);
        ALOGH("%s added buffer %jx to context %jx", __func__,
              buffer->reference(), context);
    }

    return result;
}

Return<void> Tee::TEEC_ReleaseSharedMemory(
        int32_t pid,
        uint64_t context,
        const ITee::GpSharedMemory& shr_mem) {
    ALOGH("%s called by PID %d context %jx", __func__, pid, context);
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        return Void();
    }

    auto buffer = gp_context->getBuffer(shr_mem.reference);
    if (!buffer) {
        ALOGE("No buffer %jx for PID %d in context %jx.",
              shr_mem.reference, pid, context);
        return Void();
    }

    TEEC_SharedMemory gp_shr_mem;
    gp_shr_mem.buffer = buffer->address();
    gp_shr_mem.size = buffer->size();
    gp_shr_mem.flags = buffer->flags();
    gp_context->getService()->get()->TEEC_ReleaseSharedMemory(&gp_shr_mem);
    ALOGH("%s removed buffer %jx from context %jx", __func__,
          buffer->reference(), context);
    gp_context->removeBuffer(buffer);
    return Void();
}

Return<void> Tee::TEEC_OpenSession(
        int32_t pid,
        uint64_t context,
        uint64_t session,
        const hidl_vec<uint8_t>& uuid,
        ITee::LoginType login_type,
        const hidl_vec<uint8_t>& login_data,
        const ITee::Operation& operation,
        TEEC_OpenSession_cb _hidl_cb) {
    ALOGH("%s called by PID %d context %jx", __func__, pid, context);
    uint32_t origin = TEEC_ORIGIN_API;
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        _hidl_cb(TEEC_ERROR_BAD_STATE, origin, 0, {});
        return Void();
    }

    auto gp_session = std::make_shared<GpSession>(
                       gp_context->getService(), gp_context, session);

    TEEC_UUID gp_uuid;
    ::memcpy(&gp_uuid, &uuid[0], uuid.size());
    const void* gp_login_data = nullptr;
    if (login_data.size()) {
        gp_login_data = &login_data[0];
    }

    std::unique_ptr<TEEC_Operation> gp_operation;
    // We need to keep the temporary buffers for the duration of the operation
    std::vector<std::shared_ptr<IBuffer>> buffers;
    // We may also need some temporary memory reference 'parents'
    std::vector<std::unique_ptr<TEEC_SharedMemory>> parents;
    auto result = Impl::fromOperation(
                      gp_context, operation, gp_operation, buffers, parents);
    if (result != TEEC_SUCCESS) {
        ALOGE("%s fromOperation returns %x", __func__, result);
        _hidl_cb(result, origin, 0, {});
        return Void();
    }

    gp_context->addSession(gp_session);
    gp_session->setOperation(operation.reference, gp_operation.get());
    result = gp_session->getService()->get()->TEEC_OpenSession(
                      &gp_context->gp(), &gp_session->gp(), &gp_uuid,
                      static_cast<uint32_t>(login_type), gp_login_data,
                      gp_operation.get(), &origin);
    gp_session->resetOperation();

    // Always update operation
    ITee::Operation operation_out;
    Impl::toOperation(gp_operation, operation_out, buffers);
    if (result != TEEC_SUCCESS) {
        gp_context->removeSession(gp_session->reference());
    }

    ALOGH("%s returns %x from %u id %x", __func__, result, origin,
          gp_session->gp().imp.sessionId);
    _hidl_cb(result, origin, gp_session->gp().imp.sessionId, operation_out);
    return Void();
}

Return<void> Tee::TEEC_CloseSession(
        int32_t pid,
        uint64_t context,
        uint64_t session) {
    ALOGH("%s called by PID %d context %jx session %jx", __func__,
          pid, context, session);
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        return Void();
    }

    auto gp_session = gp_context->getSession(session);
    if (!gp_session) {
        ALOGE("No session %jx in context %jx.", session, context);
        return Void();
    }

    gp_session->getService()->get()->TEEC_CloseSession(&gp_session->gp());
    gp_context->removeSession(session);
    return Void();
}

Return<void> Tee::TEEC_InvokeCommand(
        int32_t pid,
        uint64_t context,
        uint64_t session,
        uint32_t command_id,
        const ITee::Operation& operation,
        TEEC_InvokeCommand_cb _hidl_cb) {
    ALOGH("%s called by PID %d context %jx session %jx", __func__,
          pid, context, session);
    uint32_t origin = TEEC_ORIGIN_API;
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        _hidl_cb(TEEC_ERROR_BAD_STATE, origin, {});
        return Void();
    }

    auto gp_session = gp_context->getSession(session);
    if (!gp_session) {
        ALOGE("No session %jx in context %jx.", session, context);
        _hidl_cb(TEEC_ERROR_BAD_STATE, origin, {});
        return Void();
    }

    std::unique_ptr<TEEC_Operation> gp_operation;
    // We need to keep the temporary buffers for the duration of the operation
    std::vector<std::shared_ptr<IBuffer>> buffers;
    // We may also need some temporary memory reference 'parents'
    std::vector<std::unique_ptr<TEEC_SharedMemory>> parents;
    auto result = Impl::fromOperation(
                      gp_context, operation, gp_operation, buffers, parents);
    if (result != TEEC_SUCCESS) {
        ALOGE("%s fromOperation returns %x", __func__, result);
        _hidl_cb(result, origin, {});
        return Void();
    }

    gp_session->setOperation(operation.reference, gp_operation.get());
    result = gp_session->getService()->get()->TEEC_InvokeCommand(
                 &gp_session->gp(), command_id, gp_operation.get(), &origin);
    gp_session->resetOperation();
    // Always update operation
    ITee::Operation operation_out;
    Impl::toOperation(gp_operation, operation_out, buffers);
    if (result != TEEC_SUCCESS) {
        ALOGE("%s returns %x origin %u", __func__, result, origin);
    }

    _hidl_cb(result, origin, operation_out);
    return Void();
}

Return<void> Tee::TEEC_RequestCancellation(
        int32_t pid,
        uint64_t context,
        uint64_t session,
        uint64_t operation) {
    ALOGH("%s called by PID %d context %jx session %jx", __func__,
          pid, context, session);
    auto gp_context = pimpl_->gp_manager.getContext(pid, context);
    if (!gp_context) {
        ALOGE("No context %jx for PID %d.", context, pid);
        return Void();
    }

    auto gp_session = gp_context->getSession(session);
    if (!gp_session) {
        ALOGE("No session %jx in context %jx.", session, context);
        return Void();
    }

    auto gp_operation = gp_session->operation(operation);
    if (!gp_operation) {
        ALOGE("Failed to find operation %jx in session %jx.",
              operation, session);
        return Void();
    }

    gp_context->getService()->get()->TEEC_RequestCancellation(gp_operation);
    return Void();
}

Return<uint32_t> Tee::mcOpenDevice(
        int32_t pid) {
    ALOGH("%s called by PID %d", __func__, pid);
    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to connect to service.");
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    auto mc_result = service->get()->mcOpenDevice(0);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
    } else {
        pimpl_->mc_manager.addDevice(service, pid);
    }

    return mc_result;
}

Return<uint32_t> Tee::mcCloseDevice(
        int32_t pid) {
    ALOGH("%s called by PID %d", __func__, pid);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto mc_result = device->getService()->get()->mcCloseDevice(0);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
    } else {
        pimpl_->mc_manager.removeDevice(pid);
    }

    return mc_result;
}

Return<void> Tee::mcOpenSession(
        int32_t pid,
        const hidl_vec<uint8_t>& uuid,
        const ::android::hardware::hidl_memory& tci,
        uint32_t tci_len,
        mcOpenSession_cb _hidl_cb) {
    ALOGH("%s called by PID %d", __func__, pid);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN, /*id*/ 0);
        return Void();
    }

    auto session = std::make_shared<McSession>(device->getService());
    auto buffer = std::make_shared<Impl::Buffer>(tci, tci_len);
    if (!buffer->create()) {
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_NO_FREE_MEMORY, /*id*/ 0);
        return Void();
    }

    session->addBuffer(buffer);
    mcSessionHandle_t mc_session = { 0, 0 };
    mcUuid_t mc_uuid;
    ::memcpy(&mc_uuid, &uuid[0], uuid.size());
    session->updateDestination();
    auto mc_result = session->getService()->get()->mcOpenSession(
                         &mc_session,
                         &mc_uuid,
                         reinterpret_cast<uint8_t*>(buffer->address()),
                         buffer->size());
    session->updateSource();
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        _hidl_cb(mc_result, 0);
        return Void();
    }

    session->setId(mc_session.sessionId);
    device->addSession(session);
    _hidl_cb(mc_result, mc_session.sessionId);
    return Void();
}

Return<void> Tee::mcOpenTrustlet(
        int32_t pid,
        uint32_t spid,
        const ::android::hardware::hidl_memory& ta,
        uint32_t ta_len,
        const ::android::hardware::hidl_memory& tci,
        uint32_t tci_len,
        mcOpenTrustlet_cb _hidl_cb) {
    ALOGH("%s called by PID %d", __func__, pid);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN, /*id*/ 0);
        return Void();
    }

    auto session = std::make_shared<McSession>(device->getService());
    auto ta_buffer = std::make_shared<Impl::Buffer>(ta, ta_len);
    if (!ta_buffer->create()) {
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_NO_FREE_MEMORY, /*id*/ 0);
        return Void();
    }

    auto buffer = std::make_shared<Impl::Buffer>(tci, tci_len);
    if (!buffer->create()) {
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_NO_FREE_MEMORY, /*id*/ 0);
        return Void();
    }

    session->addBuffer(buffer);
    mcSessionHandle_t mc_session = { 0, 0 };
    session->updateDestination();
    auto mc_result = session->getService()->get()->mcOpenTrustlet(
                         &mc_session,
                         spid,
                         reinterpret_cast<uint8_t*>(ta_buffer->address()),
                         ta_buffer->size(),
                         reinterpret_cast<uint8_t*>(buffer->address()),
                         buffer->size());
    session->updateSource();
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        _hidl_cb(mc_result, 0);
        return Void();
    }

    {
        session->setId(mc_session.sessionId);
        device->addSession(session);
    }

    _hidl_cb(mc_result, mc_session.sessionId);
    ALOGH("%s returns %u", __func__, mc_result);
    return Void();
}

Return<uint32_t> Tee::mcCloseSession(
        int32_t pid,
        uint32_t id) {
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(id);
    if (!session) {
        ALOGE("Failed to find requested session.");
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    mcSessionHandle_t mc_session = { 0, 0 };
    mc_session.sessionId = id;
    auto mc_result = session->getService()->get()->mcCloseSession(&mc_session);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        return mc_result;
    }

    device->removeSession(id);
    return mc_result;
}

Return<uint32_t> Tee::mcNotify(
        int32_t pid,
        uint32_t id) {
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        return mc_result;
    }

    session->updateDestination();
    mcSessionHandle_t mc_session = { 0, 0 };
    mc_session.sessionId = id;
    mc_result = session->getService()->get()->mcNotify(&mc_session);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        return mc_result;
    }

    return mc_result;
}

Return<uint32_t> Tee::mcWaitNotification(
        int32_t pid,
        uint32_t id,
        int32_t timeout,
        bool /*partial*/) {
    ALOGH("%s called by PID %d id=%x timeout=%u", __func__, pid, id, timeout);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        return mc_result;
    }

    mcSessionHandle_t mc_session = { 0, 0 };
    mc_session.sessionId = id;
    mc_result = session->getService()->get()->mcWaitNotification(&mc_session, timeout);
    session->updateSource();
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        return mc_result;
    }

    return mc_result;
}

Return<void> Tee::mcMap(
        int32_t pid,
        uint32_t id,
        const ::android::hardware::hidl_memory& buf,
        uint32_t buf_len,
        mcMap_cb _hidl_cb) {
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        _hidl_cb(/*mc_result*/ mc_result, /*reference*/ 0);
        return Void();
    }

    auto buffer = std::make_shared<Impl::Buffer>(buf, buf_len);
    if (!buffer->create()) {
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_NO_FREE_MEMORY, /*reference*/ 0);
        return Void();
    }

    mcSessionHandle_t mc_session = { 0, 0 };
    mc_session.sessionId = id;
    mcBulkMap_t mapInfo;
    mc_result = session->getService()->get()->mcMap(&mc_session, buffer->address(), buffer->size(), &mapInfo);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        _hidl_cb(mc_result, /*reference*/ 0);
        return Void();
    }

    uint64_t reference;
#if ( __WORDSIZE == 64 )
    reference = mapInfo.sVirtualAddr;
#else
    reference = reinterpret_cast<uint64_t>(mapInfo.sVirtualAddr);
#endif
    buffer->setReference(reference);
    session->addBuffer(buffer);
    _hidl_cb(mc_result, reference);
    return Void();
}

Return<uint32_t> Tee::mcUnmap(
        int32_t pid,
        uint32_t id,
        uint64_t reference) {
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        return mc_result;
    }

    auto buffer = session->getBuffer(reference);
    if (!buffer) {
        ALOGE("Failed to find requested buffer.");
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    mcSessionHandle_t mc_session = { 0, 0 };
    mc_session.sessionId = id;
    mcBulkMap_t mapInfo;
    mapInfo.sVirtualLen = buffer->size();
#if ( __WORDSIZE == 64 )
    mapInfo.sVirtualAddr = static_cast<uint32_t>(reference);
#else
    mapInfo.sVirtualAddr = reinterpret_cast<void*>(reference);
#endif
    mc_result = session->getService()->get()->mcUnmap(&mc_session, buffer->address(), &mapInfo);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        return mc_result;
    }

    session->removeBuffer(buffer);
    return mc_result;
}

Return<void> Tee::mcGetSessionErrorCode(
        int32_t pid,
        uint32_t id,
        mcGetSessionErrorCode_cb _hidl_cb) {
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        _hidl_cb(/*mc_result*/ mc_result, /*sva*/ 0);
        return Void();
    }

    mcSessionHandle_t mc_session = { 0, 0 };
    mc_session.sessionId = id;
    int32_t last_err;
    mc_result = session->getService()->get()->mcGetSessionErrorCode(&mc_session, &last_err);
    if (mc_result != MC_DRV_OK) {
        ALOGE("%s returns %u", __func__, mc_result);
        _hidl_cb(/*mc_result*/ mc_result, /*last_err*/ 0);
        return Void();
    }

    _hidl_cb(/*mc_result*/ mc_result, /*last_err*/ last_err);
    return Void();
}

Return<void> Tee::mcGetMobiCoreVersion(
        int32_t pid,
        mcGetMobiCoreVersion_cb _hidl_cb) {
    ALOGH("%s called by PID %d", __func__, pid);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        _hidl_cb(/*mc_result*/ MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN, "", 0, 0, 0, 0, 0, 0, 0, 0);
        return Void();
    }

    mcVersionInfo_t version;
    auto mc_result = device->getService()->get()->mcGetMobiCoreVersion(0, &version);
    _hidl_cb(mc_result, version.productId,
             version.versionMci, version.versionSo,
             version.versionMclf, version.versionContainer,
             version.versionMcConfig, version.versionTlApi,
             version.versionDrApi, version.versionCmp);
    return Void();
}

Return<void> Tee::registerTeeCallback(
        const ::android::sp<::vendor::trustonic::tee::V1_0::ITeeCallback>& callback) {
    pimpl_->parent->setTeeCallback(callback);
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor
