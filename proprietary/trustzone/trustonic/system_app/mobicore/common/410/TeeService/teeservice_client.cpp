/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

#define LOG_TAG "teeservice_client"
//#define LOG_NDEBUG 0

//#define TEESERVICE_DEBUG
//#define TEESERVICE_MEMLEAK_CHECK

#include <sys/mman.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

#include <binder/IServiceManager.h>
#include <cutils/ashmem.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

#include "vendor/trustonic/teeservice/BpTeeService.h"
#include "vendor/trustonic/teeservice/BnTeeServiceListener.h"
#include "gp_types.h"
#include "mc_types.h"
#include "teeservice_client.h"

static const int32_t client_version = 1;

using namespace trustonic;
using namespace vendor::trustonic;
using namespace vendor::trustonic::teeservice;

class ::vendor::trustonic::Service {
    // Global service created as part of mc_device creation which has its own
    // mutex, so no mutex needed here.
    ::android::sp<ITeeService> service_;
    ::android::sp<BnTeeServiceListener> local_;
    int32_t server_version_ = 0;
public:
    Service() {
        ::android::sp<::android::IBinder> binder =
                ::android::defaultServiceManager()->getService(
                    ::android::String16(
                        "vendor.trustonic.teeservice.ITeeService"));
        service_ = ::android::interface_cast<ITeeService>(binder);
        if (!service_.get()) {
            ALOGE("Failed to create service.");
            return;
        }

        local_ = new BnTeeServiceListener();
        service_->registerClient(client_version, local_, &server_version_);
        ALOGI("Registered with server version %d.", server_version_);
    }

    auto get() const {
        return service_;
    }
};

struct TeeServiceClient::Impl {
    std::mutex service_mutex;
    std::shared_ptr<Service> service;
    std::mutex mc_device_mutex;
    size_t mc_device_open_count = 0;
    McManager mc_manager;
    GpManager gp_manager;

    bool hasService() {
        std::lock_guard<std::mutex> lock(service_mutex);
        if (!service) {
            service = std::make_shared<Service>();
        }

        return service->get().get();
    }

    std::shared_ptr<McClient> createDevice() {
        std::lock_guard<std::mutex> lock(mc_device_mutex);
        if (!hasService()) {
            return nullptr;
        }

        if (!mc_manager.getDevice(0)) {
            mc_manager.addDevice(service, 0);
        }

        mc_device_open_count++;
        ALOGH("%s open count is %zu", __func__, mc_device_open_count);
        return mc_manager.getDevice(0);
    }

    std::shared_ptr<McClient> getDevice() {
        std::lock_guard<std::mutex> lock(mc_device_mutex);
        return mc_manager.getDevice(0);
    }

    void destroyDevice() {
        std::lock_guard<std::mutex> lock(mc_device_mutex);
        if (!mc_device_open_count) {
            ALOGH("%s open count is already 0", __func__);
            return;
        }

        if (!--mc_device_open_count) {
            ALOGH("%s open count reaches 0", __func__);
            mc_manager.removeDevice(0);
        }

        ALOGH("%s open count is %zu", __func__, mc_device_open_count);
    }

    // MC sessions/buffers tracking
    class RealBuffer: public IBuffer {
        void* dest_ = nullptr;
        uint32_t size_ = 0;
        int fd_ = -1;
    public:
        RealBuffer(
                uint32_t size):
            size_(size) {}

        RealBuffer(
                const std::shared_ptr<GpContext>& context,
                uint32_t size,
                uint32_t flags):
            IBuffer(context, flags), size_(size) {}

        ~RealBuffer() override {
            if (fd_ >= 0) {
                auto len = size_ ? size_ : 1;
                ::munmap(dest_, len);
                ::close(fd_);
            }
        }

        uint8_t* create() {
            if (!context() && !size_) {
                return nullptr;
            }

            // Cannot allocate/mmap size 0, but needed by GP
            auto size = size_ ? size_ : 1;
            fd_ = ::ashmem_create_region("teeservice", size);
            if (fd_ < 0) {
                ALOGE("Failed to allocate real buffer.");
                return nullptr;
            }

            dest_ = ::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            if (dest_ == MAP_FAILED) {
                dest_ = nullptr;
                ::close(fd_);
                fd_ = -1;
                ALOGE("Failed to map real buffer.");
                return nullptr;
            }

            if (context()) {
                setReference(reinterpret_cast<uint64_t>(dest_));
            }

            return static_cast<uint8_t*>(dest_);
        }

        int fd() const override {
            return fd_;
        }

        void* address() const override {
            return dest_;
        }

        uint32_t size() const override {
            return size_;
        }

        void updateDestination() override {}

        void updateSource() override {}
    };

    class ShadowBuffer: public IBuffer {
        void* src_ = nullptr;
        void* dest_ = nullptr;
        uint32_t size_ = 0;
        int fd_ = -1;
    public:
        ShadowBuffer(
                void* src,
                uint32_t size):
            src_(src), size_(size) {}

        ShadowBuffer(
                const std::shared_ptr<GpContext>& context,
                void* src,
                uint32_t size,
                uint32_t flags):
            IBuffer(context, flags), src_(src), size_(size) {}

        ~ShadowBuffer() override {
            if (fd_ >= 0) {
                auto len = size_ ? size_ : 1;
                ::munmap(dest_, len);
                ::close(fd_);
            }
        }

        bool create() {
            if (!context() && !size_) {
                return false;
            }

            auto size = size_ ? size_ : 1;
            fd_ = ::ashmem_create_region("teeservice", size);
            if (fd_ < 0) {
                ALOGE("Failed to allocate shadow buffer.");
                return false;
            }

            dest_ = ::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            if (dest_ == MAP_FAILED) {
                dest_ = nullptr;
                ::close(fd_);
                fd_ = -1;
                ALOGE("Failed to map shadow buffer.");
                return false;
            }

            if (context()) {
                setReference(reinterpret_cast<uint64_t>(dest_));
            }

            return true;
        }

        int fd() const override {
            return fd_;
        }

        void* address() const override {
            return src_;
        }

        uint32_t size() const override {
            return size_;
        }

        void updateDestination() override {
            // Copied in all cases so the copy back does not erase the data
            ::memcpy(dest_, src_, size_);
        }

        void updateSource() override {
            if (flags() & TEEC_MEM_OUTPUT) {
                ::memcpy(src_, dest_, size_);
            }
        }
    };

    static TEEC_Result fromOperation(
            const std::shared_ptr<GpContext>& context,
            const TEEC_Operation* operation_in,
            TeeServiceGpOperation* operation_out,
            std::vector<std::shared_ptr<IBuffer>>& buffers) {
        operation_out->reference = reinterpret_cast<uint64_t>(operation_in);
        if (!operation_out->reference) {
            return TEEC_SUCCESS;
        }

        operation_out->started = operation_in->started;
        for (size_t i = 0; i < 4; i++) {
            const TEEC_Parameter& param_in = operation_in->params[i];
            TeeServiceGpOperation::Param& param_out = operation_out->params[i];
            param_out.type = (operation_in->paramTypes >> (4 * i)) & 0xf;
            switch (param_out.type) {
                case TEEC_NONE:
                case TEEC_VALUE_OUTPUT:
                    break;
                case TEEC_VALUE_INPUT:
                case TEEC_VALUE_INOUT:
                    param_out.value_a = param_in.value.a;
                    param_out.value_b = param_in.value.b;
                    break;
                case TEEC_MEMREF_TEMP_OUTPUT:
                case TEEC_MEMREF_TEMP_INPUT:
                case TEEC_MEMREF_TEMP_INOUT:
                    // A Temporary Memory Reference may be null, which can be
                    // used to request the required output size.
                    if (param_in.tmpref.buffer) {
                        // Not part of GP protocol, to only copy when needed
                        uint32_t flags;
                        switch (param_out.type) {
                            case TEEC_MEMREF_TEMP_OUTPUT:
                                flags = TEEC_MEM_OUTPUT;
                                break;
                            case TEEC_MEMREF_TEMP_INPUT:
                                flags = TEEC_MEM_INPUT;
                                break;
                            case TEEC_MEMREF_TEMP_INOUT:
                                flags = TEEC_MEM_INOUT;
                                break;
                        }

                        auto buffer = std::make_shared<ShadowBuffer>(
                                          context,
                                          param_in.tmpref.buffer,
                                          param_in.tmpref.size,
                                          flags);
                        if (!buffer->create()) {
                            // Log'd in callee
                            return TEEC_ERROR_OUT_OF_MEMORY;
                        }

                        buffers.push_back(buffer);
                        param_out.reference = buffer->reference();
                        param_out.buffer = buffer->fd();
                        // Not part of GP protocol
                        param_out.flags = buffer->flags();
                    }

                    param_out.size = param_in.tmpref.size;
                    break;
                case TEEC_MEMREF_WHOLE:
                case TEEC_MEMREF_PARTIAL_OUTPUT:
                case TEEC_MEMREF_PARTIAL_INPUT:
                case TEEC_MEMREF_PARTIAL_INOUT:
                    auto buffer = context->getBuffer(
                                      param_in.memref.parent->buffer);
                    if (!buffer) {
                        ALOGE("Failed to get buffer %p for context %jx.",
                              param_in.memref.parent->buffer,
                              context->reference());
                        return TEEC_ERROR_BAD_PARAMETERS;
                    }

                    buffers.push_back(buffer);
                    param_out.reference = buffer->reference();
                    param_out.size = param_in.memref.parent->size;
                    param_out.flags = param_in.memref.parent->flags;
                    param_out.window_offset = param_in.memref.offset;
                    param_out.window_size = param_in.memref.size;
                    break;
            }
        }

        for (const auto& buffer: buffers) {
            buffer->updateDestination();
        }

        return TEEC_SUCCESS;
    }

    static void toOperation(
            const TeeServiceGpOperation& operation_in,
            TEEC_Operation* operation_out,
            std::vector<std::shared_ptr<IBuffer>>& buffers) {
        if (!operation_in.reference) {
            return;
        }

        if (!operation_out) {
            return;
        }

        operation_out->started = operation_in.started;
        for (size_t i = 0; i < 4; i++) {
            const TeeServiceGpOperation::Param& param_in = operation_in.params[i];
            TEEC_Parameter& param_out = operation_out->params[i];
            switch (param_in.type) {
                case TEEC_NONE:
                case TEEC_VALUE_INPUT:
                    break;
                case TEEC_VALUE_OUTPUT:
                case TEEC_VALUE_INOUT:
                    param_out.value.a = param_in.value_a;
                    param_out.value.b = param_in.value_b;
                    break;
                case TEEC_MEMREF_TEMP_OUTPUT:
                case TEEC_MEMREF_TEMP_INPUT:
                case TEEC_MEMREF_TEMP_INOUT:
                    param_out.tmpref.size = param_in.size;
                    break;
                case TEEC_MEMREF_WHOLE:
                case TEEC_MEMREF_PARTIAL_OUTPUT:
                case TEEC_MEMREF_PARTIAL_INPUT:
                case TEEC_MEMREF_PARTIAL_INOUT:
                    param_out.memref.size = param_in.size;
                    break;
            }
        }

        for (const auto& buffer: buffers) {
            buffer->updateSource();
        }
    }
};

// Using new here because std::make_unique is not part of C++11
TeeServiceClient::TeeServiceClient(): pimpl_(new Impl) {
}

TeeServiceClient::~TeeServiceClient() {
}

// Proprietary
void TeeServiceClient::TEEC_TT_RegisterPlatformContext(
        void*                   /*globalContext*/,
        void*                   /*localContext*/) {
}

struct TestData {
    bool        using_proxy = false;
    bool        using_proxy_protocol_v3 = false;
};

TEEC_Result TeeServiceClient::TEEC_TT_TestEntry(
        void*                   buff,
        size_t                  /*len*/,
        uint32_t*               /*tag*/) {
    // Behaves closely to copy proxy as buffers are managed locally
    TestData* test_data = static_cast<TestData*>(buff);
    test_data->using_proxy = true;
    test_data->using_proxy_protocol_v3 = true;
    // Do not propagate further in the libMcClient
    return TEEC_SUCCESS;
}

// Global Platform
TEEC_Result TeeServiceClient::TEEC_InitializeContext(
        const char*             name,
        TEEC_Context*           context) {
    ALOGH("%s called", __func__);
    if (!pimpl_->hasService()) {
        return TEEC_ERROR_COMMUNICATION;
    }

    auto gp_context = pimpl_->gp_manager.createContext(
                          pimpl_->service,
                          0,
                          reinterpret_cast<uint64_t>(context));
    TeeServiceGpContextIn params_in;
    params_in.context = reinterpret_cast<uint64_t>(context);
    params_in.has_name = name;
    if (params_in.has_name) {
        params_in.name = name;
    }

    int32_t aidl_return;
    auto status = gp_context->getService()->get()->TEEC_InitializeContext(
                      params_in, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return TEEC_ERROR_COMMUNICATION;
    }

    ALOGH("%s aidl_return=%x", __func__, aidl_return);
    return TEEC_SUCCESS;
}

void TeeServiceClient::TEEC_FinalizeContext(
        TEEC_Context*           context) {
    ALOGH("%s called", __func__);
    auto gp_context = pimpl_->gp_manager.getContext(
                          0, reinterpret_cast<uint64_t>(context));
    if (!gp_context) {
        ALOGE("No context.");
        return;
    }

    TeeServiceGpContextIn params_in;
    params_in.context = reinterpret_cast<uint64_t>(context);
    auto status = gp_context->getService()->get()->TEEC_FinalizeContext(
                      params_in);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return;
    }

    pimpl_->gp_manager.removeContext(0, params_in.context);
}

TEEC_Result TeeServiceClient::TEEC_RegisterSharedMemory(
        TEEC_Context*           context,
        TEEC_SharedMemory*      shared_mem) {
    ALOGH("%s called", __func__);
    auto gp_context = pimpl_->gp_manager.getContext(
                          0, reinterpret_cast<uint64_t>(context));
    if (!gp_context) {
        ALOGE("No context.");
        return TEEC_ERROR_BAD_STATE;
    }

    auto buffer = std::make_shared<Impl::ShadowBuffer>(
                      gp_context, shared_mem->buffer, shared_mem->size,
                      shared_mem->flags);
    if (!buffer->create()) {
        // Log'd in callee
        return TEEC_ERROR_OUT_OF_MEMORY;
    }

    TeeServiceGpSharedMemoryIn params_in;
    params_in.context = gp_context->reference();
    params_in.buffer = buffer->fd();
    params_in.reference = buffer->reference();
    params_in.size = buffer->size();
    params_in.flags = buffer->flags();

    int32_t aidl_return;
    auto status = gp_context->getService()->get()->TEEC_RegisterSharedMemory(
                      params_in, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return TEEC_ERROR_COMMUNICATION;
    }

    ALOGH("%s returns %x", __func__, aidl_return);
    if (aidl_return != TEEC_SUCCESS) {
        return aidl_return;
    }

    gp_context->addBuffer(buffer);
    ALOGH("%s added buffer %jx to context %jx", __func__,
          buffer->reference(), params_in.context);
    return TEEC_SUCCESS;
}

TEEC_Result TeeServiceClient::TEEC_AllocateSharedMemory(
        TEEC_Context*           context,
        TEEC_SharedMemory*      shared_mem) {
    ALOGH("%s called", __func__);
    auto gp_context = pimpl_->gp_manager.getContext(
                          0, reinterpret_cast<uint64_t>(context));
    if (!gp_context) {
        ALOGE("No context.");
        return TEEC_ERROR_BAD_STATE;
    }

    auto buffer = std::make_shared<Impl::RealBuffer>(
                      gp_context, shared_mem->size, shared_mem->flags);
    shared_mem->buffer = buffer->create();
    if (!shared_mem->buffer) {
        // Log'd in callee
        return TEEC_ERROR_OUT_OF_MEMORY;
    }

    TeeServiceGpSharedMemoryIn params_in;
    params_in.context = gp_context->reference();
    params_in.buffer = buffer->fd();
    params_in.reference = buffer->reference();
    params_in.size = buffer->size();
    params_in.flags = buffer->flags();

    int32_t aidl_return;
    auto status = gp_context->getService()->get()->TEEC_RegisterSharedMemory(
                      params_in, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return TEEC_ERROR_COMMUNICATION;
    }

    ALOGH("%s returns %x", __func__, aidl_return);
    if (aidl_return != TEEC_SUCCESS) {
        return aidl_return;
    }

    gp_context->addBuffer(buffer);
    ALOGH("%s added buffer %jx to context %jx", __func__,
          buffer->reference(), params_in.context);
    return TEEC_SUCCESS;
}

void TeeServiceClient::TEEC_ReleaseSharedMemory(
        TEEC_SharedMemory*      shared_mem) {
    ALOGH("%s called", __func__);
    auto gp_client = pimpl_->gp_manager.getClient(0);
    if (!gp_client) {
        ALOGE("No client.");
        return;
    }

    auto buffer = gp_client->getBuffer(shared_mem->buffer);
    if (!buffer) {
        // Log'd in callee
        return;
    }

    TeeServiceGpSharedMemoryIn params_in;
    auto gp_context = buffer->context();
    params_in.context = gp_context->reference();
    params_in.reference = buffer->reference();
    params_in.flags = buffer->flags();
    auto status = gp_context->getService()->get()->TEEC_ReleaseSharedMemory(
                      params_in);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return;
    }

    ALOGH("%s removed buffer %jx from context %jx", __func__,
          buffer->reference(), params_in.context);
    gp_context->removeBuffer(buffer);
}

TEEC_Result TeeServiceClient::TEEC_OpenSession(
        TEEC_Context*           context,
        TEEC_Session*           session,
        const TEEC_UUID*        destination,
        uint32_t                connection_method,
        const void*             connection_data,
        TEEC_Operation*         operation,
        uint32_t*               return_origin) {
    ALOGH("%s called", __func__);
    auto gp_client = pimpl_->gp_manager.getClient(0);
    if (!gp_client) {
        ALOGE("No client.");
        return TEEC_ERROR_BAD_STATE;
    }

    auto operation_reference = reinterpret_cast<uint64_t>(operation);
    if (operation_reference) {
        if (operation->started == 0) {
            ALOGH("check operation %jx for cancellation", operation_reference);
            if (gp_client->isCancelledOperation(operation_reference)) {
                if (return_origin) {
                    *return_origin = TEEC_ORIGIN_COMMS;
                }

                return TEEC_ERROR_CANCEL;
            }

            operation->started = 1;
        }
    }

    if (return_origin) {
        *return_origin = TEEC_ORIGIN_API;
    }

    auto context_reference = reinterpret_cast<uint64_t>(context);
    auto gp_context = pimpl_->gp_manager.getContext(0, context_reference);
    if (!gp_context) {
        ALOGE("No context.");
        return TEEC_ERROR_BAD_STATE;
    }

    auto session_reference = reinterpret_cast<uint64_t>(session);
    auto gp_session = std::make_shared<GpSession>(
                          pimpl_->service, gp_context, session_reference);

    TeeServiceGpOpenSessionIn params_in;
    params_in.context = gp_context->reference();
    params_in.session = gp_session->reference();
    // Copy UUID
    params_in.uuid.resize(sizeof(*destination));
    ::memcpy(&params_in.uuid[0], destination, sizeof(*destination));
    params_in.login_type = connection_method;
    if (connection_data) {
        params_in.login_data.resize(16);
        ::memcpy(&params_in.login_data[0], connection_data,
                params_in.login_data.size());
    }

    // We need to keep the temporary buffers for the duration of the operation
    std::vector<std::shared_ptr<IBuffer>> buffers;
    auto result = Impl::fromOperation(
                      gp_context, operation, &params_in.operation, buffers);
    if (result != TEEC_SUCCESS) {
        return result;
    }

    TeeServiceGpOpenSessionOut params_out;
    params_out.operation = params_in.operation;
    int32_t aidl_return;
    // Make sure session can be searched for, in case of cancellation
    gp_context->addSession(gp_session);
    gp_session->setOperation(params_in.operation.reference, operation);
    ALOGH("Operation %jx set for cancellation", params_in.operation.reference);
    auto status = gp_session->getService()->get()->TEEC_OpenSession(
                      params_in, &params_out, &aidl_return);
    gp_session->resetOperation();
    ALOGH("Operation %jx reset for cancellation", params_in.operation.reference);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        if (return_origin) {
            *return_origin = TEEC_ORIGIN_COMMS;
        }

        gp_context->removeSession(gp_session->reference());
        return TEEC_ERROR_COMMUNICATION;
    }

    ALOGH("%s got aidl_return=%x origin=%u id=%x", __func__,
          aidl_return, params_out.origin, params_out.id);
    Impl::toOperation(params_out.operation, operation, buffers);
    if (aidl_return != TEEC_SUCCESS) {
        gp_context->removeSession(gp_session->reference());
    } else {
        session->imp.sessionId = params_out.id;
    }

    if (return_origin) {
        *return_origin = params_out.origin;
    }

    return aidl_return;
}

void TeeServiceClient::TEEC_CloseSession(
        TEEC_Session*           session) {
    ALOGH("%s called", __func__);
    auto gp_client = pimpl_->gp_manager.getClient(0);
    if (!gp_client) {
        ALOGE("No client.");
        return;
    }

    auto session_reference = reinterpret_cast<uint64_t>(session);
    auto gp_session = gp_client->getSession(session_reference);
    if (!gp_session) {
        ALOGE("Failed to find session %jx.", session_reference);
        return;
    }

    TeeServiceGpCloseSessionIn params_in;
    auto gp_context = gp_session->context();
    params_in.context = gp_context->reference();
    params_in.session = gp_session->reference();
    auto status = gp_session->getService()->get()->TEEC_CloseSession(params_in);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return;
    }

    gp_context->removeSession(gp_session->reference());
}

TEEC_Result TeeServiceClient::TEEC_InvokeCommand(
        TEEC_Session*           session,
        uint32_t                command_id,
        TEEC_Operation*         operation,
        uint32_t*               return_origin) {
    ALOGH("%s called", __func__);
    auto gp_client = pimpl_->gp_manager.getClient(0);
    if (!gp_client) {
        ALOGE("No client.");
        return TEEC_ERROR_BAD_STATE;
    }

    auto operation_reference = reinterpret_cast<uint64_t>(operation);
    if (operation_reference) {
        if (operation->started == 0) {
            ALOGH("Check operation %jx for cancellation", operation_reference);
            if (gp_client->isCancelledOperation(operation_reference)) {
                if (return_origin) {
                    *return_origin = TEEC_ORIGIN_COMMS;
                }

                return TEEC_ERROR_CANCEL;
            }

            operation->started = 1;
        }
    }

    if (return_origin) {
        *return_origin = TEEC_ORIGIN_API;
    }

    auto session_reference = reinterpret_cast<uint64_t>(session);
    auto gp_session = gp_client->getSession(session_reference);
    if (!gp_session) {
        ALOGE("Failed to find session %jx.", session_reference);
        return TEEC_ERROR_BAD_STATE;
    }

    TeeServiceGpInvokeCommandIn params_in;
    auto gp_context = gp_session->context();
    params_in.context = gp_context->reference();
    params_in.session = gp_session->reference();
    params_in.command_id = command_id;

    // We need to keep the temporary buffers for the duration of the operation
    std::vector<std::shared_ptr<IBuffer>> buffers;
    auto result = Impl::fromOperation(
                      gp_context, operation, &params_in.operation, buffers);
    if (result != TEEC_SUCCESS) {
        return result;
    }

    TeeServiceGpInvokeCommandOut params_out;
    params_out.operation = params_in.operation;
    int32_t aidl_return;
    gp_session->setOperation(params_in.operation.reference, operation);
    auto status = gp_session->getService()->get()->TEEC_InvokeCommand(
                      params_in, &params_out, &aidl_return);
    gp_session->resetOperation();
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        if (return_origin) {
            *return_origin = TEEC_ORIGIN_COMMS;
        }

        return TEEC_ERROR_COMMUNICATION;
    }

    ALOGH("%s got aidl_return=%x origin=%u", __func__,
          aidl_return, params_out.origin);
    Impl::toOperation(params_out.operation, operation, buffers);
    if (return_origin) {
        *return_origin = params_out.origin;
    }

    return aidl_return;
}

void TeeServiceClient::TEEC_RequestCancellation(
        TEEC_Operation*         operation) {
    ALOGH("%s called", __func__);
    auto gp_client = pimpl_->gp_manager.getClient(0);
    if (!gp_client) {
        ALOGE("No client.");
        return;
    }

    auto reference = reinterpret_cast<uint64_t>(operation);
    // FIMXE this is racey because:
    // * we check for reference then add to the list of cancelled
    // * messages to the lower layers could be re-ordered
    auto session = gp_client->getSessionByOperation(reference);
    if (!session) {
        ALOGH("Failed to find session for operation %jx, keep.", reference);
        // started will be 1 once operation has started
        if (operation->started == 0) {
            gp_client->addCancelledOperation(reference);
        }

        return;
    }

    TeeServiceGpRequestCancellationIn params_in;
    auto gp_context = session->context();
    params_in.context = gp_context->reference();
    params_in.session = session->reference();
    params_in.operation = reference;
    auto status = gp_context->getService()->get()->TEEC_RequestCancellation(
                      params_in);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return;
    }
}

// MobiCore
mcResult_t TeeServiceClient::mcOpenDevice(
        uint32_t                /*device_id*/) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->createDevice();
    if (!device) {
        ALOGE("Failed to create device.");
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    int32_t aidl_return;
    auto status = device->getService()->get()->mcOpenDevice(&aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    return aidl_return;
}

mcResult_t TeeServiceClient::mcCloseDevice(
        uint32_t                /*device_id*/) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    int32_t aidl_return;
    auto status = device->getService()->get()->mcCloseDevice(&aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    if (aidl_return == MC_DRV_OK) {
        pimpl_->destroyDevice();
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    return aidl_return;
}

mcResult_t TeeServiceClient::mcOpenSession(
        mcSessionHandle_t*      mc_session,
        const mcUuid_t*         uuid,
        uint8_t*                tci,
        uint32_t                tciLen) {
    ALOGH("%s called tci=%p tciLen=%u", __func__, tci, tciLen);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = std::make_shared<McSession>(pimpl_->service);

    ::vendor::trustonic::teeservice::TeeServiceMcOpenSessionIn params_in;
    // Copy UUID
    params_in.uuid.resize(sizeof(*uuid));
    ::memcpy(&params_in.uuid[0], uuid, sizeof(*uuid));
    // Create TCI
    auto buffer = device->getBuffer(tci);
    if (!buffer) {
        auto shadow_buffer = std::make_shared<Impl::ShadowBuffer>(tci, tciLen);
        if (!shadow_buffer->create()) {
            return MC_DRV_ERR_NO_FREE_MEMORY;
        }

        buffer = shadow_buffer;
    }

    session->addBuffer(buffer);
    params_in.tci = buffer->fd();
    params_in.tci_len = tciLen;
    ::vendor::trustonic::teeservice::TeeServiceMcOpenSessionOut params_out;
    int32_t aidl_return;
    session->updateDestination();
    auto status = session->getService()->get()->mcOpenSession(
                      params_in, &params_out, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    session->updateSource();
    ALOGH("%s got mc_result=%u id=%x", __func__,
          params_out.mc_result, params_out.id);
    if (params_out.mc_result == MC_DRV_OK) {
        mc_session->sessionId = params_out.id;
        session->setId(params_out.id);
        device->addSession(session);
    }

    return params_out.mc_result;
}

mcResult_t TeeServiceClient::mcOpenTrustlet(
        mcSessionHandle_t*      mc_session,
        mcSpid_t                spid,
        uint8_t*                trustedapp,
        uint32_t                tLen,
        uint8_t*                tci,
        uint32_t                tciLen) {
    ALOGH("%s called tci=%p tciLen=%u", __func__, tci, tciLen);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = std::make_shared<McSession>(pimpl_->service);

    ::vendor::trustonic::teeservice::TeeServiceMcOpenTrustletIn params_in;
    // Copy SPID
    params_in.spid = spid;
    // Copy TA
    auto ta_buffer = std::make_shared<Impl::RealBuffer>(tLen);
    if (!ta_buffer->create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::memcpy(ta_buffer->address(), trustedapp, tLen);
    params_in.ta = ta_buffer->fd();
    params_in.ta_len = tLen;
    // Create TCI
    auto buffer = device->getBuffer(tci);
    if (!buffer) {
        auto shadow_buffer = std::make_shared<Impl::ShadowBuffer>(tci, tciLen);
        if (!shadow_buffer->create()) {
            return MC_DRV_ERR_NO_FREE_MEMORY;
        }

        buffer = shadow_buffer;
    }

    session->addBuffer(buffer);
    params_in.tci = buffer->fd();
    params_in.tci_len = tciLen;
    ::vendor::trustonic::teeservice::TeeServiceMcOpenSessionOut params_out;
    int32_t aidl_return;
    session->updateDestination();
    auto status = session->getService()->get()->mcOpenTrustlet(
                      params_in, &params_out, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    session->updateSource();
    ALOGH("%s got mc_result=%u id=%x", __func__,
          params_out.mc_result, params_out.id);
    if (params_out.mc_result == MC_DRV_OK) {
        mc_session->sessionId = params_out.id;
        session->setId(params_out.id);
        device->addSession(session);
    }

    return params_out.mc_result;
}

mcResult_t TeeServiceClient::mcCloseSession(
        mcSessionHandle_t*      mc_session) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(mc_session->sessionId);
    if (!session) {
        ALOGE("No session ID %u.", mc_session->sessionId);
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    int32_t aidl_return;
    auto status = session->getService()->get()->mcCloseSession(
                      mc_session->sessionId, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    if (aidl_return != MC_DRV_OK) {
        return aidl_return;
    }

    device->removeSession(mc_session->sessionId);
    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcNotify(
        mcSessionHandle_t*      mc_session) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(mc_session->sessionId);
    if (!session) {
        ALOGE("No session ID %u.", mc_session->sessionId);
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    int32_t aidl_return;
    session->updateDestination();
    auto status = session->getService()->get()->mcNotify(
                      mc_session->sessionId, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    if (aidl_return != MC_DRV_OK) {
        return aidl_return;
    }

    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcWaitNotification(
        mcSessionHandle_t*      mc_session,
        int32_t                 timeout) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(mc_session->sessionId);
    if (!session) {
        ALOGE("No session ID %u.", mc_session->sessionId);
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    int32_t aidl_return;
    static const int32_t timeout_max = 1000; // 1s
    int32_t timeout_left = timeout;
    while (true) {
        int32_t partial_timeout;
        bool partial;
        // Do not block for a long time, retry every so often
        if (timeout < 0) {
            // Infinite
            partial_timeout = timeout_max;
            partial = true;
        } else if (timeout_left > timeout_max) {
            // Big
            partial_timeout = timeout_max;
            partial = true;
            timeout_left -= timeout_max;
        } else {
            // Small enough
            partial_timeout = timeout_left;
            partial = false;
            timeout_left = 0;
        }
        ALOGH("%s timeout: asked=%d left=%d set=%d", __func__,
              timeout, timeout_left, partial_timeout);

        auto status = session->getService()->get()->mcWaitNotification(
                          mc_session->sessionId, partial_timeout, partial,
                          &aidl_return);
        if (!status.isOk()) {
            ALOGE("Failed to call service: %d.", status.exceptionCode());
            return MC_DRV_ERR_DAEMON_UNREACHABLE;
        }

        ALOGH("%s returns %u", __func__, aidl_return);

        bool retry;
        switch (aidl_return) {
            case MC_DRV_ERR_TIMEOUT:
                if (partial) {
                    // Timed out but full requested timeout not expired
                    retry = true;
                } else {
                    // Timeout expired
                    retry = false;
                }
                break;
            case MC_DRV_ERR_INTERRUPTED_BY_SIGNAL:
                if (timeout == MC_INFINITE_TIMEOUT) {
                    // Interrupted, but timeout is 'hard' infinite
                    retry = true;
                } else {
                    // Signal received
                    retry = false;
                }
                break;
            default:
                // Other error: give up
                retry = false;
                break;
        }

        if (!retry) {
            break;
        }

        ALOGH("%s retry", __func__);
    }

    session->updateSource();
    return aidl_return;
}

mcResult_t TeeServiceClient::mcMallocWsm(
        uint32_t                /*deviceId*/,
        uint32_t                len,
        uint8_t**               wsm) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto buffer = std::make_shared<Impl::RealBuffer>(len);
    if (!wsm) {
        // Log'd in callee
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    *wsm = buffer->create();

    device->addBuffer(buffer);
    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcFreeWsm(
        uint32_t                /*deviceId*/,
        uint8_t*                wsm,
        uint32_t                /*len*/) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto buffer = device->getBuffer(wsm);
    if (!buffer) {
        return MC_DRV_ERR_BULK_UNMAPPING;
    }

    device->removeBuffer(buffer);
    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcMap(
        mcSessionHandle_t*      mc_session,
        void*                   buf,
        uint32_t                len,
        mcBulkMap_t*            mapInfo) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(mc_session->sessionId);
    if (!session) {
        ALOGE("Failed to find requested session.");
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    TeeServiceMcMapIn params_in;
    params_in.id = mc_session->sessionId;
    // Create buffer
    auto buffer = device->getBuffer(buf);
    if (!buffer) {
        auto shadow_buffer = std::make_shared<Impl::ShadowBuffer>(buf, len);
        if (!shadow_buffer->create()) {
            return MC_DRV_ERR_NO_FREE_MEMORY;
        }

        buffer = shadow_buffer;
    }

    params_in.buf = buffer->fd();
    params_in.buf_len = len;
    TeeServiceMcMapOut params_out;

    int32_t aidl_return;
    auto status = session->getService()->get()->mcMap(
                      params_in, &params_out, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    if (aidl_return != MC_DRV_OK) {
        return aidl_return;
    }

    buffer->setReference(params_out.reference);
    session->addBuffer(buffer);
    mapInfo->sVirtualLen = len;
#if ( __WORDSIZE == 64 )
    mapInfo->sVirtualAddr = static_cast<uint32_t>(params_out.reference);
#else
    mapInfo->sVirtualAddr = reinterpret_cast<void*>(params_out.reference);
#endif
    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcUnmap(
        mcSessionHandle_t*      mc_session,
        void*                   /*buf*/,
        mcBulkMap_t*            mapInfo) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(mc_session->sessionId);
    if (!session) {
        ALOGE("Failed to find requested session.");
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    TeeServiceMcUnmapIn params_in;
    params_in.id = mc_session->sessionId;
#if ( __WORDSIZE == 64 )
    params_in.reference = mapInfo->sVirtualAddr;
#else
    params_in.reference = reinterpret_cast<uint64_t>(mapInfo->sVirtualAddr);
#endif

    auto buffer = session->getBuffer(params_in.reference);
    if (!buffer) {
        ALOGE("Failed to find requested buffer in session %x.", session->id());
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    int32_t aidl_return;
    auto status = session->getService()->get()->mcUnmap(
                      params_in, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    if (aidl_return != MC_DRV_OK) {
        return aidl_return;
    }

    session->removeBuffer(buffer);
    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcGetSessionErrorCode(
        mcSessionHandle_t*      mc_session,
        int32_t*                lastErr) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    auto session = device->getSession(mc_session->sessionId);
    if (!session) {
        ALOGE("No session ID %u.", mc_session->sessionId);
        return MC_DRV_ERR_UNKNOWN_SESSION;
    }

    TeeServiceMcGetSessionErrorCodeOut params_out;

    int32_t aidl_return;
    auto status = session->getService()->get()->mcGetSessionErrorCode(
                      mc_session->sessionId, &params_out, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    if (aidl_return != MC_DRV_OK) {
        return aidl_return;
    }

    *lastErr = params_out.last_err;
    return MC_DRV_OK;
}

mcResult_t TeeServiceClient::mcGetMobiCoreVersion(
        uint32_t                /*deviceId*/,
        mcVersionInfo_t*        versionInfo) {
    ALOGH("%s called", __func__);
    auto device = pimpl_->getDevice();
    if (!device) {
        ALOGE("No device.");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    TeeServiceMcGetMobiCoreVersionOut params_out;

    int32_t aidl_return;
    auto status = device->getService()->get()->mcGetMobiCoreVersion(
                      &params_out, &aidl_return);
    if (!status.isOk()) {
        ALOGE("Failed to call service: %d.", status.exceptionCode());
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ALOGH("%s returns %u", __func__, aidl_return);
    if (aidl_return != MC_DRV_OK) {
        return aidl_return;
    }

    ::strncpy(versionInfo->productId, params_out.productId, sizeof(versionInfo->productId) - 1);
    versionInfo->productId[sizeof(versionInfo->productId) - 1] = '\0';
    versionInfo->versionMci = params_out.versionMci;
    versionInfo->versionSo = params_out.versionSo;
    versionInfo->versionMclf = params_out.versionMclf;
    versionInfo->versionContainer = params_out.versionContainer;
    versionInfo->versionMcConfig = params_out.versionMcConfig;
    versionInfo->versionTlApi = params_out.versionTlApi;
    versionInfo->versionDrApi = params_out.versionDrApi;
    versionInfo->versionCmp = params_out.versionCmp;
    ALOGH("%s: %s %x %x %x %x %x %x %x %x", __func__,
          versionInfo->productId,
          versionInfo->versionMci,
          versionInfo->versionSo,
          versionInfo->versionMclf,
          versionInfo->versionContainer,
          versionInfo->versionMcConfig,
          versionInfo->versionTlApi,
          versionInfo->versionDrApi,
          versionInfo->versionCmp);
    return MC_DRV_OK;
}

bool TeeServiceClient::getBindInformation(
        BindInformation*        bind_information) {
    ::strncpy(bind_information->package_name,
              "com.trustonic.teeservice",
              sizeof(bind_information->package_name) - 1);
    bind_information->package_name[sizeof(bind_information->package_name) - 1] = '\0';
    ::strncpy(bind_information->starter_class,
              "TeeService",
              sizeof(bind_information->starter_class) - 1);
    bind_information->starter_class[sizeof(bind_information->starter_class) - 1] = '\0';
    return true;
}

extern "C" IClient* factory() {
    return new TeeServiceClient;
}
