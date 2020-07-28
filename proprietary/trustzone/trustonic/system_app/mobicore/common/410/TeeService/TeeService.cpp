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

#define LOG_TAG "teeserviceserver"
//#define LOG_NDEBUG 0

//#define TEESERVICE_DEBUG
//#define TEESERVICE_MEMLEAK_CHECK

#include <sys/mman.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <jni.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <cutils/ashmem.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>

#include "vendor/trustonic/tee/1.0/ITee.h"
#include "MobiCoreDriverApi.h"
#include "tee_client_api.h"
#include "gp_types.h"
#include "mc_types.h"
#include "TeeService.h"

static const int32_t server_version = 1;

using namespace vendor::trustonic;
using namespace vendor::trustonic::teeservice;
using namespace vendor::trustonic::tee::V1_0;

class ::vendor::trustonic::Service {
    ::android::sp<ITee> service_;
public:
    bool create() {
        if (!service_) {
            service_ = ITee::getService();
        }

        return service_.get();
    }

    auto get() const {
        return service_;
    }
};

struct TeeService::Impl {
    // Client death detection
    class ClientManager: public ::android::IBinder::DeathRecipient {
        ::android::sp<ITeeServiceListener> client_;
        GpManager& gp_manager_;
        McManager& mc_manager_;
        std::function<void(int)> remover_;
        int32_t version_ = 0;
        int pid_ = 0;
    public:
        ClientManager(
                int pid,
                const ::android::sp<ITeeServiceListener>& client,
                int32_t version,
                GpManager& gp_manager,
                McManager& mc_manager,
                std::function<void(int)> remover):
            client_(client), gp_manager_(gp_manager), mc_manager_(mc_manager),
            remover_(remover), version_(version), pid_(pid) {}

        int version() const {
            return version_;
        }

        int pid() const {
            return pid_;
        }

        void binderDied(
                const ::android::wp<::android::IBinder>& /*who*/) override {
            ALOGH("%s called for PID %d", __func__, pid_);
            gp_manager_.flushPid(pid_);
            mc_manager_.flushPid(pid_);

            auto service = std::make_shared<Service>();
            if (!service->create()) {
                ALOGW("Failed to connect to service.");
                return;
            }

            service->get()->flushPid(pid_);
            remover_(pid_);
        }
    };

    // Service death detection
    class HidlDeathNotifier:
            public ::android::hardware::hidl_death_recipient {
        std::mutex mutex_;
        std::condition_variable cond_;
        bool died_ = false;
        bool stop_ = false;
    public:
        bool waitForServiceDeath() {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!died_ && !stop_) {
                cond_.wait(lock);
            }

            bool died = died_;
            // Re-arm
            died_ = false;
            return died;
        }

        void stopWaiting() {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
            cond_.notify_one();
        }

        void serviceDied(
                uint64_t /*cookie*/,
                const ::android::wp<::android::hidl::base::V1_0::IBase>& /*who*/) override {
            std::lock_guard<std::mutex> lock(mutex_);
            died_ = true;
            cond_.notify_one();
        }
    };

    // MC sessions/buffers tracking
    class Buffer: public IBuffer {
        ::android::sp<::android::hardware::HidlMemory> hidl_memory_;
        ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
        uint32_t size_ = 0;
    public:
        Buffer(
                uint32_t size):
            size_(size) {}

        Buffer(
                const std::shared_ptr<GpContext>& context,
                uint64_t reference,
                uint32_t size,
                uint32_t flags):
            IBuffer(context, flags), size_(size) {
            setReference(reference);
        }

        bool create(
                int fd) {
            hidl_memory_ = ::android::hardware::HidlMemory::getInstance(
                               "ashmem", fd, ::ashmem_get_size_region(fd));
            if (!hidl_memory_.get()) {
                ALOGE("Failed to wrap ashmem buffer.");
                return false;
            }

            memory_ = ::android::hardware::mapMemory(*hidl_memory_.get());
            if (!memory_.get()) {
                ALOGE("Failed to map ashmem buffer.");
                return false;
            }

            return true;
        }

        const ::android::hardware::hidl_memory& mem() const {
            return *hidl_memory_.get();
        }

        int fd() const override {
            return -1;
        }

        void* address() const override {
            return nullptr;
        }

        uint32_t size() const override {
            return size_;
        }

        void updateDestination() override {
            memory_->commit();
        }

        void updateSource() override {
            if (flags() & TEEC_MEM_OUTPUT) {
                memory_->update();
            }
        }
    };

    struct TeeServiceGpOpenSessionOutWrapper;
    struct TeeServiceGpInvokeCommandOutWrapper;

    static TEEC_Result fromOperation(
            const std::shared_ptr<GpContext>& context,
            const TeeServiceGpOperation& operation_in,
            ITee::Operation& operation_out,
            std::vector<std::shared_ptr<IBuffer>>& buffers) {
        operation_out.reference = operation_in.reference;
        if (!operation_out.reference) {
            return TEEC_SUCCESS;
        }

        operation_out.started = operation_in.started;
        for (size_t i = 0; i < 4; i++) {
            const TeeServiceGpOperation::Param& param_in = operation_in.params[i];
            ITee::Operation::Param& param_out = operation_out.params[i];
            param_out.type = param_in.type;
            switch (param_out.type) {
                case TEEC_NONE:
                case TEEC_VALUE_OUTPUT:
                    break;
                case TEEC_VALUE_INPUT:
                case TEEC_VALUE_INOUT:
                    param_out.value_a = param_in.value_a;
                    param_out.value_b = param_in.value_b;
                    break;
                case TEEC_MEMREF_TEMP_OUTPUT:
                case TEEC_MEMREF_TEMP_INPUT:
                case TEEC_MEMREF_TEMP_INOUT:
                    // A Temporary Memory Reference may be null, which can be
                    // used to request the required output size.
                    if (param_in.buffer >= 0) {
                        auto buffer = std::make_shared<Buffer>(
                                          context,
                                          param_in.reference,
                                          param_in.size,
                                          param_in.flags);
                        if (!buffer->create(param_in.buffer)) {
                            // Log'd in callee
                            return TEEC_ERROR_OUT_OF_MEMORY;
                        }

                        buffers.push_back(buffer);
                        param_out.reference = buffer->reference();
                        param_out.buffer = buffer->mem();
                        param_out.has_buffer = true;
                        // Not part of GP protocol
                        param_out.flags = buffer->flags();
                    } else {
                        param_out.has_buffer = false;
                    }

                    param_out.size = param_in.size;
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
                    param_out.reference = buffer->reference();
                    param_out.size = param_in.size;
                    param_out.flags = param_in.flags;
                    param_out.window_offset = param_in.window_offset;
                    param_out.window_size = param_in.window_size;
                    break;
            }
        }

        for (const auto& buffer: buffers) {
            buffer->updateDestination();
        }

        return TEEC_SUCCESS;
    }

    static void toOperation(
            const ITee::Operation& operation_in,
            TeeServiceGpOperation* operation_out,
            std::vector<std::shared_ptr<IBuffer>>& buffers) {
        if (!operation_out->reference) {
            return;
        }

        operation_out->started = operation_in.started;
        for (size_t i = 0; i < 4; i++) {
            const ITee::Operation::Param& param_in = operation_in.params[i];
            TeeServiceGpOperation::Param& param_out = operation_out->params[i];
            switch (param_in.type) {
                case TEEC_NONE:
                case TEEC_VALUE_INPUT:
                    break;
                case TEEC_VALUE_OUTPUT:
                case TEEC_VALUE_INOUT:
                    param_out.value_a = param_in.value_a;
                    param_out.value_b = param_in.value_b;
                    break;
                case TEEC_MEMREF_TEMP_OUTPUT:
                case TEEC_MEMREF_TEMP_INPUT:
                case TEEC_MEMREF_TEMP_INOUT:
                    param_out.size = param_in.size;
                    break;
                case TEEC_MEMREF_WHOLE:
                case TEEC_MEMREF_PARTIAL_OUTPUT:
                case TEEC_MEMREF_PARTIAL_INPUT:
                case TEEC_MEMREF_PARTIAL_INOUT:
                    param_out.size = param_in.size;
                    break;
            }
        }

        for (const auto& buffer: buffers) {
            buffer->updateSource();
        }
    }

    void serviceMonitor() {
        while (true) {
            Service service;
            // Wait for service to appear (getService waits, so we just loop)
            while (!stop_service_monitor && !service.create());
            if (stop_service_monitor) {
                break;
            }

            // Make sure we start clean
            service.get()->flushAll();
            // Register for service death detection
            service.get()->linkToDeath(hidl_death_notifier, 0);
            if (!hidl_death_notifier->waitForServiceDeath()) {
                // Asked to stop
                break;
            }

            ALOGH("vendor service died, flush");
            gp_manager.flush();
            mc_manager.flush();
        }
    }

    Impl():
        hidl_death_notifier(new HidlDeathNotifier),
        service_monitor(std::bind(&Impl::serviceMonitor, this)) {}

    Impl(void* java_env_, void* java_service_object_):
        java_env(static_cast<JNIEnv*>(java_env_)),
        java_service_object(static_cast<jobject>(java_service_object_)) {
            Impl();
        }

    ~Impl() {
        stop_service_monitor = true;
        hidl_death_notifier->stopWaiting();
        service_monitor.join();
    }

    void removeDeathNotifier(int pid) {
        std::lock_guard<std::mutex> lock(death_notifiers_mutex_);
        auto it = std::find_if(
                      death_notifiers_.begin(),
                      death_notifiers_.end(),
                      [pid](const auto& death_notifier){
            return pid == death_notifier->pid();
        });
        if (it != death_notifiers_.end()) {
            death_notifiers_.erase(it);
            ALOGM("%zu death notifiers", death_notifiers_.size());
        }
    }

    std::vector<uint8_t> getApkIdentityHash(int uid) {
        ALOGD("uid=%i env=%p ctx=%p", uid, java_env, java_service_object);
        jclass clazz = java_env->GetObjectClass(java_service_object);

        // Find method from the java service
        jmethodID getApk_identity = java_env->GetMethodID(clazz,
                "getApkIdentityHash", "(I)[B");

        std::vector<uint8_t> apk_identity_hash;
        jint juid = uid;
        jbyteArray j_hash = static_cast<jbyteArray>(
                java_env->CallObjectMethod(java_service_object,
                    getApk_identity, juid));
        if (!j_hash ) {
            ALOGE("%s:%i Cannot get hash!\n", __func__, __LINE__);
            return apk_identity_hash;
        } else {

            jsize j_hashlen = java_env->GetArrayLength(j_hash);
            if (j_hashlen != 16) {
                ALOGE("%s:%i Hash size (%zu) is wrong!\n", __func__, __LINE__,
                        static_cast<size_t>(j_hashlen));
                return apk_identity_hash;
            } else {
                /*
                 * copy the data for the byte array (JAVA) to the uint8_t array (native)
                 */
                apk_identity_hash.resize(j_hashlen);
                java_env->GetByteArrayRegion(j_hash, 0, j_hashlen,
                        reinterpret_cast<jbyte*>(&apk_identity_hash[0]));

                ALOGH("%s: SUCCESS", __func__);
            }
        }

        return apk_identity_hash;
    }

    ::android::sp<HidlDeathNotifier> hidl_death_notifier;
    std::thread service_monitor;
    bool stop_service_monitor = false;
    GpManager gp_manager;
    McManager mc_manager;
    std::mutex death_notifiers_mutex_;
    std::vector<::android::sp<ClientManager>> death_notifiers_;
    JNIEnv *java_env = nullptr;
    jobject java_service_object = nullptr;
};

// Using new here because std::make_unique is not part of C++11
TeeService::TeeService(): pimpl_(new Impl) {
}

TeeService::TeeService(void* java_env, void* java_service_object):
    pimpl_(new Impl(java_env, java_service_object)) {
}

TeeService::~TeeService() {
}

void TeeService::instantiate(void* java_env, void* java_service_object) {
    ALOGH("%s called, start service vendor.trustonic.teeservice.ITeeService",
          __func__);
    ::android::defaultServiceManager()->addService(
                ::android::String16("vendor.trustonic.teeservice.ITeeService"),
                new TeeService(java_env, java_service_object));
}

::android::binder::Status TeeService::registerClient(
        int32_t client_version,
        const ::android::sp<ITeeServiceListener>& client,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d client %p", __func__, pid, client.get());
    *_aidl_return = server_version;
    std::lock_guard<std::mutex> lock(pimpl_->death_notifiers_mutex_);
    auto it = std::find_if(
                  pimpl_->death_notifiers_.begin(),
                  pimpl_->death_notifiers_.end(),
                  [pid](const auto& death_notifier){
        return pid == death_notifier->pid();
    });
    if (it != pimpl_->death_notifiers_.end()) {
        ALOGH("Already got a death notifier for client with PID %d.", pid);
        return ::android::binder::Status::ok();
    }

    ::android::sp<Impl::ClientManager> recipient =
            new Impl::ClientManager(
                pid, client, client_version,
                pimpl_->gp_manager, pimpl_->mc_manager,
                std::bind(&Impl::removeDeathNotifier, pimpl_.get(),
                          std::placeholders::_1));
    auto status = ::android::IInterface::asBinder(client)->linkToDeath(
                      recipient);
    if (status == ::android::NO_ERROR) {
        pimpl_->death_notifiers_.push_back(recipient);
        ALOGM("%zu death notifiers", pimpl_->death_notifiers_.size());
        ALOGI("Registered client with version %d.", client_version);
    } else {
        ALOGE("Failed to register client: %d.", status);
        *_aidl_return = -1;
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::TEEC_InitializeContext(
        const TeeServiceGpContextIn& params_in,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d", __func__, pid);
    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to connect to service.");
        *_aidl_return = TEEC_ERROR_COMMUNICATION;
        return ::android::binder::Status::ok();
    }

    *_aidl_return = service->get()->TEEC_InitializeContext(
                        pid,
                        params_in.context,
                        params_in.name.c_str(),
                        params_in.has_name);
    ALOGH("%s _aidl_return=%x", __func__, *_aidl_return);
    if (*_aidl_return == TEEC_SUCCESS) {
        pimpl_->gp_manager.createContext(service, pid, params_in.context);
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::TEEC_FinalizeContext(
        const TeeServiceGpContextIn& params_in) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d", __func__, pid);
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context.");
        return ::android::binder::Status::ok();
    }

    context->getService()->get()->TEEC_FinalizeContext(pid, params_in.context);
    pimpl_->gp_manager.removeContext(pid, params_in.context);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::TEEC_RegisterSharedMemory(
        const TeeServiceGpSharedMemoryIn& params_in,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d context %jx", __func__, pid, params_in.context);
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context.");
        return ::android::binder::Status::ok();
    }

    auto buffer = std::make_shared<Impl::Buffer>(
                      context, params_in.reference,
                      params_in.size, params_in.flags);
    if (!buffer->create(params_in.buffer)) {
        // Log'd in callee
        *_aidl_return = TEEC_ERROR_OUT_OF_MEMORY;
        return ::android::binder::Status::ok();
    }

    ITee::GpSharedMemory shr_mem;
    shr_mem.reference = buffer->reference();
    shr_mem.buffer = buffer->mem();
    shr_mem.size = buffer->size();
    shr_mem.flags = buffer->flags();
    *_aidl_return = context->getService()->get()->TEEC_RegisterSharedMemory(
                        pid, params_in.context, shr_mem);
    if (*_aidl_return == TEEC_SUCCESS) {
        context->addBuffer(buffer);
        ALOGH("%s added buffer %jx to context %jx", __func__,
              buffer->reference(), params_in.context);
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::TEEC_ReleaseSharedMemory(
        const TeeServiceGpSharedMemoryIn& params_in) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d context %jx buffer %jx",
          __func__, pid, params_in.context, params_in.reference);
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context.");
        return ::android::binder::Status::ok();
    }

    auto buffer = context->getBuffer(params_in.reference);
    if (!buffer) {
        ALOGE("No buffer %jx in PID %d context %jx.",
              params_in.reference, pid, params_in.context);
        return ::android::binder::Status::ok();
    }

    ITee::GpSharedMemory shr_mem;
    shr_mem.reference = buffer->reference();
    shr_mem.flags = buffer->flags();
    context->getService()->get()->TEEC_ReleaseSharedMemory(
                pid, params_in.context, shr_mem);
    ALOGH("%s removed buffer %jx from context %jx", __func__,
          buffer->reference(), params_in.context);
    context->removeBuffer(buffer);
    return ::android::binder::Status::ok();
}

struct TeeService::Impl::TeeServiceGpOpenSessionOutWrapper {
    TeeServiceGpOpenSessionOut* params_out;
    // We need to keep the temporary buffers for the duration of the operation
    std::vector<std::shared_ptr<IBuffer>> buffers;
    uint32_t result = TEEC_ERROR_BAD_STATE;

    void callback(
            uint32_t return_value,
            uint32_t return_origin,
            uint32_t id,
            const ITee::Operation& operation) {
        result = return_value;
        params_out->origin = return_origin;
        params_out->id = id;
        toOperation(operation, &params_out->operation, buffers);
    }
};

::android::binder::Status TeeService::TEEC_OpenSession(
        const TeeServiceGpOpenSessionIn& params_in,
        TeeServiceGpOpenSessionOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d context %jx", __func__, pid, params_in.context);
    params_out->origin = TEEC_ORIGIN_API;
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context %jx for PID %d.", params_in.context, pid);
        *_aidl_return = TEEC_ERROR_BAD_STATE;
        return ::android::binder::Status::ok();
    }

    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to connect to service.");
        *_aidl_return = TEEC_ERROR_COMMUNICATION;
        return ::android::binder::Status::ok();
    }

    auto session = std::make_shared<GpSession>(
                       service, context, params_in.session);
    auto login_type = static_cast<::ITee::LoginType>(params_in.login_type);

    /* default login methods */
    ::android::hardware::hidl_vec<uint8_t> login_data = params_in.login_data;

    /*
     * If the login method parameters is equal to LOGIN_APPLICATION or
     * LOGIN_USER_APPLICATION CA identity is computed as follows:
     * SHA1(PACKAGE_NAME + APP_DEV_KEY).
     */
    if ((login_type == ::ITee::LoginType::TEE_PROXY_LOGIN_APPLICATION) ||
        (login_type == ::ITee::LoginType::TEE_PROXY_LOGIN_USER_APPLICATION)) {
        /* retrieve client identity information */
        int uid = ::android::IPCThreadState::self()->getCallingUid();
        std::vector<uint8_t> apk_identity_hash = pimpl_->getApkIdentityHash(uid);
        if (apk_identity_hash.empty()) {
            *_aidl_return = TEEC_ERROR_NO_DATA;
            return ::android::binder::Status::ok();
        }
        login_data = apk_identity_hash;
    }

    Impl::TeeServiceGpOpenSessionOutWrapper wrapper;
    wrapper.params_out = params_out;
    ::ITee::Operation operation;
    *_aidl_return = Impl::fromOperation(
                        context, params_in.operation, operation,
                        wrapper.buffers);
    if (*_aidl_return != TEEC_SUCCESS) {
        return ::android::binder::Status::ok();
    }

    params_out->operation = params_in.operation;
    context->addSession(session);
    session->setOperation(params_in.operation.reference, nullptr);
    service->get()->TEEC_OpenSession(
                pid, params_in.context, params_in.session, params_in.uuid,
                login_type, login_data, operation,
                std::bind(&Impl::TeeServiceGpOpenSessionOutWrapper::callback,
                          &wrapper,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4));
    session->resetOperation();
    *_aidl_return = wrapper.result;
    ALOGH("%s _aidl_return=%x", __func__, *_aidl_return);
    // toOperation called by callback
    if (*_aidl_return != TEEC_SUCCESS) {
        context->removeSession(session->reference());
    }

    ALOGH("%s returns %x from %u id %x", __func__, *_aidl_return,
          params_out->origin, params_out->id);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::TEEC_CloseSession(
        const TeeServiceGpCloseSessionIn& params_in) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d context %jx session %jx", __func__,
          pid, params_in.context, params_in.session);
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context %jx for PID %d.", params_in.context, pid);
        return ::android::binder::Status::ok();
    }

    auto session = context->getSession(params_in.session);
    if (!session) {
        ALOGE("No session %jx in PID %d context %jx.",
              params_in.session, pid, params_in.context);
        return ::android::binder::Status::ok();
    }

    session->getService()->get()->TEEC_CloseSession(
                pid, params_in.context, params_in.session);
    context->removeSession(session->reference());
    return ::android::binder::Status::ok();
}

struct TeeService::Impl::TeeServiceGpInvokeCommandOutWrapper {
    TeeServiceGpInvokeCommandOut* params_out;
    // We need to keep the temporary buffers for the duration of the operation
    std::vector<std::shared_ptr<IBuffer>> buffers;
    uint32_t result = TEEC_ERROR_BAD_STATE;

    void callback(
            uint32_t return_value,
            uint32_t return_origin,
            const ITee::Operation& operation) {
        result = return_value;
        params_out->origin = return_origin;
        toOperation(operation, &params_out->operation, buffers);
    }
};

::android::binder::Status TeeService::TEEC_InvokeCommand(
        const ::vendor::trustonic::teeservice::TeeServiceGpInvokeCommandIn& params_in,
        ::vendor::trustonic::teeservice::TeeServiceGpInvokeCommandOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d context %jx session %jx", __func__,
          pid, params_in.context, params_in.session);
    params_out->origin = TEEC_ORIGIN_API;
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context %jx for PID %d.", params_in.context, pid);
        *_aidl_return = TEEC_ERROR_BAD_STATE;
        return ::android::binder::Status::ok();
    }

    auto session = context->getSession(params_in.session);
    if (!session) {
        ALOGE("No session %jx in PID %d context %jx.",
              params_in.session, pid, params_in.context);
        *_aidl_return = TEEC_ERROR_BAD_STATE;
        return ::android::binder::Status::ok();
    }

    Impl::TeeServiceGpInvokeCommandOutWrapper wrapper;
    wrapper.params_out = params_out;
    ::ITee::Operation operation;
    *_aidl_return = Impl::fromOperation(
                        context, params_in.operation, operation,
                        wrapper.buffers);
    if (*_aidl_return != TEEC_SUCCESS) {
        return ::android::binder::Status::ok();
    }

    params_out->operation = params_in.operation;
    session->setOperation(params_in.operation.reference, nullptr);
    session->getService()->get()->TEEC_InvokeCommand(
                pid, params_in.context, params_in.session, params_in.command_id,
                operation,
                std::bind(&Impl::TeeServiceGpInvokeCommandOutWrapper::callback,
                          &wrapper,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3));
    session->resetOperation();
    *_aidl_return = wrapper.result;
    ALOGH("%s returns %x from %u", __func__, *_aidl_return, params_out->origin);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::TEEC_RequestCancellation(
        const ::vendor::trustonic::teeservice::TeeServiceGpRequestCancellationIn& params_in) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d context %jx session %jx operation %jx", __func__,
          pid, params_in.context, params_in.session, params_in.operation);
    auto context = pimpl_->gp_manager.getContext(pid, params_in.context);
    if (!context) {
        ALOGE("No context %jx for PID %d.", params_in.context, pid);
        return ::android::binder::Status::ok();
    }

    auto session = context->getSession(params_in.session);
    if (!session) {
        ALOGE("No session %jx in PID %d context %jx.",
              params_in.session, pid, params_in.context);
        return ::android::binder::Status::ok();
    }

    // No check on operation reference here.

    context->getService()->get()->TEEC_RequestCancellation(
                pid, params_in.context, params_in.session, params_in.operation);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcOpenDevice(
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d", __func__, pid);
    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to connect to service.");
        *_aidl_return = MC_DRV_ERR_DAEMON_UNREACHABLE;
        return ::android::binder::Status::ok();
    }

    *_aidl_return = service->get()->mcOpenDevice(pid);
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    if (*_aidl_return == MC_DRV_OK) {
        pimpl_->mc_manager.addDevice(service, pid);
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcCloseDevice(
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d", __func__, pid);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        *_aidl_return = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
        return ::android::binder::Status::ok();
    }

    *_aidl_return = device->getService()->get()->mcCloseDevice(pid);
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    if (*_aidl_return == MC_DRV_OK) {
        pimpl_->mc_manager.removeDevice(pid);
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcOpenSession(
        const TeeServiceMcOpenSessionIn& params_in,
        TeeServiceMcOpenSessionOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d: tci=%d", __func__, pid, params_in.tci);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        *_aidl_return = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
        return ::android::binder::Status::ok();
    }

    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to create service for session.");
        *_aidl_return = MC_DRV_ERR_DAEMON_UNREACHABLE;
        return ::android::binder::Status::ok();
    }

    auto session = std::make_shared<McSession>(service);
    auto buffer =
            std::make_shared<Impl::Buffer>(params_in.tci_len);
    if (!buffer->create(params_in.tci)) {
        *_aidl_return = MC_DRV_ERR_NO_FREE_MEMORY;
        return ::android::binder::Status::ok();
    }

    session->addBuffer(buffer);
    session->updateDestination();
    session->getService()->get()->mcOpenSession(
                pid,
                params_in.uuid,
                buffer->mem(),
                buffer->size(),
                std::bind(&TeeServiceMcOpenSessionOut::callback, params_out,
                          std::placeholders::_1, std::placeholders::_2));
    session->updateSource();
    ALOGH("%s mc_result=%u id=%x", __func__,
          params_out->mc_result, params_out->id);
    *_aidl_return = params_out->mc_result;
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    if (*_aidl_return == MC_DRV_OK) {
        session->setId(params_out->id);
        device->addSession(session);
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcOpenTrustlet(
        const TeeServiceMcOpenTrustletIn& params_in,
        TeeServiceMcOpenSessionOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d: ta=%d tci=%d", __func__,
          pid, params_in.ta, params_in.tci);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        *_aidl_return = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
        return ::android::binder::Status::ok();
    }

    auto service = std::make_shared<Service>();
    if (!service->create()) {
        ALOGE("Failed to create service for session.");
        *_aidl_return = MC_DRV_ERR_DAEMON_UNREACHABLE;
        return ::android::binder::Status::ok();
    }

    auto ta = std::make_shared<Impl::Buffer>(params_in.ta_len);
    if (!ta->create(params_in.ta)) {
        *_aidl_return = MC_DRV_ERR_NO_FREE_MEMORY;
        return ::android::binder::Status::ok();
    }

    ta->updateDestination();
    auto session = std::make_shared<McSession>(service);
    auto buffer =
            std::make_shared<Impl::Buffer>(params_in.tci_len);
    if (!buffer->create(params_in.tci)) {
        *_aidl_return = MC_DRV_ERR_NO_FREE_MEMORY;
        return ::android::binder::Status::ok();
    }

    session->addBuffer(buffer);
    session->updateDestination();
    session->getService()->get()->mcOpenTrustlet(
                pid,
                params_in.spid,
                ta->mem(),
                ta->size(),
                buffer->mem(),
                buffer->size(),
                std::bind(&TeeServiceMcOpenSessionOut::callback, params_out,
                          std::placeholders::_1, std::placeholders::_2));
    session->updateSource();
    ALOGH("%s mc_result=%u id=%x", __func__,
          params_out->mc_result, params_out->id);
    *_aidl_return = params_out->mc_result;
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    if (*_aidl_return == MC_DRV_OK) {
        session->setId(params_out->id);
        device->addSession(session);
    }

    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcCloseSession(
        int32_t id,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        *_aidl_return = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
        return ::android::binder::Status::ok();
    }

    auto session = device->getSession(id);
    if (!session) {
        ALOGE("Failed to find requested session.");
        *_aidl_return = MC_DRV_ERR_UNKNOWN_SESSION;
        return ::android::binder::Status::ok();
    }

    *_aidl_return = session->getService()->get()->mcCloseSession(pid, id);
    if (*_aidl_return == MC_DRV_OK) {
        device->removeSession(static_cast<uint32_t>(id));
    }

    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcNotify(
        int32_t id,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        *_aidl_return = mc_result;
        return ::android::binder::Status::ok();
    }

    session->updateDestination();
    *_aidl_return = session->getService()->get()->mcNotify(pid, id);
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcWaitNotification(
        int32_t id,
        int32_t timeout,
        bool partial,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d id=%x timeout=%u", __func__, pid, id, timeout);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        *_aidl_return = mc_result;
        return ::android::binder::Status::ok();
    }

    *_aidl_return = session->getService()->get()->mcWaitNotification(
                        pid, id, timeout, partial);
    session->updateSource();
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcMap(
        const TeeServiceMcMapIn& params_in,
        TeeServiceMcMapOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d id=%x", __func__, pid, params_in.id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, params_in.id, &mc_result);
    if (!session) {
        *_aidl_return = mc_result;
        return ::android::binder::Status::ok();
    }

    auto buffer =
            std::make_shared<Impl::Buffer>(params_in.buf_len);
    if (!buffer->create(params_in.buf)) {
        *_aidl_return = MC_DRV_ERR_NO_FREE_MEMORY;
        return ::android::binder::Status::ok();
    }

    session->getService()->get()->mcMap(
                pid,
                params_in.id,
                buffer->mem(),
                buffer->size(),
                std::bind(&TeeServiceMcMapOut::callback, params_out,
                          std::placeholders::_1, std::placeholders::_2));
    ALOGH("%s mc_result=%u reference=%jx", __func__,
          params_out->mc_result, params_out->reference);
    *_aidl_return = params_out->mc_result;
    if (*_aidl_return == MC_DRV_OK) {
        buffer->setReference(params_out->reference);
        session->addBuffer(buffer);
    }

    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcUnmap(
        const TeeServiceMcUnmapIn& params_in,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d id=%x reference=%jx", __func__,
          pid, params_in.id, params_in.reference);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, params_in.id, &mc_result);
    if (!session) {
        *_aidl_return = mc_result;
        return ::android::binder::Status::ok();
    }

    auto buffer = session->getBuffer(params_in.reference);
    if (!buffer) {
        *_aidl_return = MC_DRV_ERR_INVALID_PARAMETER;
        return ::android::binder::Status::ok();
    }

    *_aidl_return = session->getService()->get()->mcUnmap(
                        pid, params_in.id, params_in.reference);
    if (*_aidl_return == MC_DRV_OK) {
        session->removeBuffer(buffer);
    }

    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}

::android::binder::Status TeeService::mcGetSessionErrorCode(
        int32_t id,
        TeeServiceMcGetSessionErrorCodeOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d id=%x", __func__, pid, id);
    mcResult_t mc_result;
    auto session = pimpl_->mc_manager.getSession(pid, id, &mc_result);
    if (!session) {
        *_aidl_return = mc_result;
        return ::android::binder::Status::ok();
    }

    session->getService()->get()->mcGetSessionErrorCode(
                pid,
                id,
                std::bind(&TeeServiceMcGetSessionErrorCodeOut::callback,
                          params_out,
                          std::placeholders::_1,
                          std::placeholders::_2));
    ALOGH("%s mc_result=%u last_err=%d", __func__,
          params_out->mc_result, params_out->last_err);
    *_aidl_return = params_out->mc_result;
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}

struct TeeServiceMcGetMobiCoreVersionOutWrapper{
    TeeServiceMcGetMobiCoreVersionOut* params_out;

    void callback(
            uint32_t mc_result,
            const ::android::hardware::hidl_string& product_id,
            uint32_t mci,
            uint32_t so,
            uint32_t mclf,
            uint32_t container,
            uint32_t mc_config,
            uint32_t tl_api,
            uint32_t dr_api,
            uint32_t cmp) {
        params_out->mc_result = mc_result;
        ::strncpy(params_out->productId, product_id.c_str(),
                  sizeof(params_out->productId) - 1);
        params_out->productId[sizeof(params_out->productId) - 1] = '\0';
        params_out->versionMci = mci;
        params_out->versionSo = so;
        params_out->versionMclf = mclf;
        params_out->versionContainer = container;
        params_out->versionMcConfig = mc_config;
        params_out->versionTlApi = tl_api;
        params_out->versionDrApi = dr_api;
        params_out->versionCmp = cmp;
    }
};

::android::binder::Status TeeService::mcGetMobiCoreVersion(
        TeeServiceMcGetMobiCoreVersionOut* params_out,
        int32_t* _aidl_return) {
    int pid = ::android::IPCThreadState::self()->getCallingPid();
    ALOGH("%s called by PID %d", __func__, pid);
    auto device = pimpl_->mc_manager.getDevice(pid);
    if (!device) {
        ALOGE("No device.");
        *_aidl_return = MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
        return ::android::binder::Status::ok();
    }

    TeeServiceMcGetMobiCoreVersionOutWrapper wrapper;
    wrapper.params_out = params_out;
    device->getService()->get()->mcGetMobiCoreVersion(
                pid,
                std::bind(&TeeServiceMcGetMobiCoreVersionOutWrapper::callback,
                          &wrapper,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4,
                          std::placeholders::_5, std::placeholders::_6,
                          std::placeholders::_7, std::placeholders::_8,
                          std::placeholders::_9, std::placeholders::_10));

    ALOGH("%s mc_result=%u", __func__, params_out->mc_result);
    *_aidl_return = params_out->mc_result;
    ALOGH("%s _aidl_return=%d", __func__, *_aidl_return);
    return ::android::binder::Status::ok();
}
