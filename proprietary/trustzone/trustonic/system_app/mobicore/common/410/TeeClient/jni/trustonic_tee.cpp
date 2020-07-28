/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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

#include <functional>
#include <mutex>
#include <vector>

#include <dlfcn.h>

#include "tee_bind_jni.h"
#include "MobiCoreDriverApi.h"
#include "tee_client_api.h"
#include "dynamic_log.h"
#include "dummy.h"
#include "system.h"
#include "proxy.h"
#include "trustonic_tee.h"

using namespace trustonic;

struct TrustonicTEE::Impl {
    std::unique_ptr<IClient>     client;
    enum class OpenMode {
        AUTO,                       // Same as MC_DEVICE_ID_DEFAULT
        DRIVER    = 0x44525652,     // Force use of the driver
        PROXY_TDP = 0x50544450,     // Force use of the downloadable proxy
        SERVICE   = 0x53525643,     // Force use of the Android service
    }                            open_mode = OpenMode::AUTO;
    std::mutex                   init_mutex;
    int                          init_context_count = 0;
    int                          open_device_count = 0;
    JavaVM*                      jvm = nullptr;
    jobject                      application_context = nullptr;
    std::unique_ptr<JavaProcess> tee_service;
    std::unique_ptr<JavaProcess> tui_service;

    Impl(): client(new Dummy) {}

    // For test purpose
    bool setOpenMode(uint32_t mode) {
        switch (mode) {
            case static_cast<int>(OpenMode::AUTO):
                open_mode = OpenMode::AUTO;
                break;
            case static_cast<int>(OpenMode::DRIVER):
                open_mode = OpenMode::DRIVER;
                break;
            case static_cast<int>(OpenMode::PROXY_TDP):
            case 0x4e4f4e45: // For old SDKs
                open_mode = OpenMode::PROXY_TDP;
                break;
            case static_cast<int>(OpenMode::SERVICE):
                open_mode = OpenMode::SERVICE;
                break;
            default:
                open_mode = OpenMode::AUTO;
                return false;
        }

        return true;
    }

    class IClientFactory {
        std::string name_;
    public:
        IClientFactory(const char* name): name_(name) {}

        virtual ~IClientFactory() {}

        const std::string& name() const {
            return name_;
        }

        virtual bool mayRequireTuiService() const {
            return false;
        }

        virtual std::unique_ptr<IClient> create() = 0;
    };

    class SystemClientFactory: public IClientFactory {
    public:
        SystemClientFactory(): IClientFactory("system library") {}

        bool mayRequireTuiService() const override {
            return true;
        }

        std::unique_ptr<IClient> create() override {
            std::unique_ptr<System> client(new System);
            return std::move(client);
        }
    };

    class ProxyClientFactory: public IClientFactory {
    public:
        ProxyClientFactory(): IClientFactory("proxy v3 TDP") {}

        bool mayRequireTuiService() const override {
            return true;
        }

        std::unique_ptr<IClient> create() override {
            std::unique_ptr<Proxy> client(new Proxy);
            return std::move(client);
        }
    };

    class PluginClientFactory: public IClientFactory {
        std::string lib_name_;
        std::mutex  dl_mutex_;
        void*       dl_lib_handle_ = nullptr;
    public:
        PluginClientFactory(const char* name, const char* lib_name):
            IClientFactory(name), lib_name_(lib_name) {}

        ~PluginClientFactory() override {
            if (dl_lib_handle_) {
                ::dlclose(dl_lib_handle_);
            }
        }

        std::unique_ptr<IClient> create() override {
            std::lock_guard<std::mutex> lock(dl_mutex_);
            dl_lib_handle_ = ::dlopen(lib_name_.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            if (!dl_lib_handle_) {
                LOG_I("Library '%s' not found on this system",
                      lib_name_.c_str());
                return nullptr;
            }

            auto dl_factory = ::dlsym(dl_lib_handle_, "factory");
            if (!dl_factory) {
                LOG_E("Library '%s' found, but does not provide a factory",
                      lib_name_.c_str());
                return nullptr;
            }

            auto factory = reinterpret_cast<IClient*(*)()>(dl_factory);
            auto client = factory();
            if (!client) {
                LOG_E("Library '%s' found, but factory fails to create clients",
                      lib_name_.c_str());
                return nullptr;
            }

            return std::unique_ptr<IClient>(client);
        }
    };

    std::vector<std::unique_ptr<IClientFactory>> clientFactories() {
        std::vector<std::unique_ptr<IClientFactory>> client_factories;
        switch (open_mode) {
            case OpenMode::DRIVER:
                LOG_I("Test open mode %08x detected, force McClient",
                      static_cast<unsigned int>(open_mode));
                client_factories.emplace_back(new SystemClientFactory);
                break;
            case OpenMode::PROXY_TDP:
                LOG_I("Test open mode %08x detected, force TDP",
                      static_cast<unsigned int>(open_mode));
                client_factories.emplace_back(new ProxyClientFactory);
                break;
            case OpenMode::SERVICE:
                LOG_I("Test open mode %08x detected, force Android service",
                      static_cast<unsigned int>(open_mode));
                client_factories.emplace_back(
                            new PluginClientFactory(
                                "framework library", "libteeservice_client.trustonic.so"));
                break;
            case OpenMode::AUTO:
                client_factories.emplace_back(
                            new PluginClientFactory(
                                "framework library", "libteeservice_client.trustonic.so"));
                client_factories.emplace_back(new SystemClientFactory);
                client_factories.emplace_back(new ProxyClientFactory);
                break;
        }
        return client_factories;
    }

    std::unique_ptr<JavaProcess> bindService(
            const std::string& package_name,
            const std::string& starter_class,
            bool send_restart = false) {
        if (!jvm || package_name.empty() || starter_class.empty()) {
            return nullptr;
        }

        std::unique_ptr<JavaProcess> service(
                    new JavaProcess(package_name, starter_class));
        if (!service->bind(jvm, application_context, send_restart)) {
            return nullptr;
        }

        return service;
    }

    void bindTuiService() {
        tui_service = bindService(
                          "com.trustonic.tuiservice", "TuiService", true);
        if (!tui_service) {
            LOG_W("TUI service not accessible");
        }
    }

    void unbindServices() {
        tee_service.reset();
        tui_service.reset();
    }
};

// Missing std::make_unique
TrustonicTEE::TrustonicTEE(): pimpl_(new Impl) {}

// Destructor needs the size of Impl
TrustonicTEE::~TrustonicTEE() {}

// Proprietary
void TrustonicTEE::TEEC_TT_RegisterPlatformContext(
    void*                   globalContext,
    void*                   localContext) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    pimpl_->jvm = static_cast<JavaVM*>(globalContext);
    pimpl_->application_context = static_cast<jobject>(localContext);
    // Update client data, if any
    pimpl_->client->TEEC_TT_RegisterPlatformContext(pimpl_->jvm,
            pimpl_->application_context);
}

TEEC_Result TrustonicTEE::TEEC_TT_TestEntry(
    void*                   buff,
    size_t                  len,
    uint32_t*               tag) {
    return pimpl_->client->TEEC_TT_TestEntry(buff, len, tag);
}

// Global Platform
TEEC_Result TrustonicTEE::TEEC_InitializeContext(
    const char*             name,
    TEEC_Context*           context) {
    if (name && ::strcmp(name, "kinibi")) {
        LOG_E("Name not supported '%s'", name);
        return TEEC_ERROR_NOT_SUPPORTED;
    }

    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    TEEC_Result teec_result = TEEC_ERROR_NOT_IMPLEMENTED;
    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
    if ((pimpl_->init_context_count == 0) && (pimpl_->open_device_count == 0)) {
        pimpl_->setOpenMode(context->imp.open_mode);
        for (auto& client_factory: pimpl_->clientFactories()) {
            LOG_I("Try to access Kinibi through the %s",
                  client_factory->name().c_str());
            auto client = client_factory->create();
            if (!client) {
                teec_result = TEEC_ERROR_NOT_IMPLEMENTED;
                continue;
            }

            client->TEEC_TT_RegisterPlatformContext(
                        pimpl_->jvm, pimpl_->application_context);
            std::unique_ptr<JavaProcess> service;
            BindInformation bind_info;
            if (client->getBindInformation(&bind_info)) {
                if (pimpl_->jvm) {
                    service = pimpl_->bindService(
                                bind_info.package_name, bind_info.starter_class);
                    if (!service) {
                        LOG_E("Service not supported '%s'",
                            bind_info.package_name);
                        teec_result = TEEC_ERROR_NOT_SUPPORTED;
                        continue;
                    }
                } else {
                    LOG_I("No Java VM to bind service '%s'",
                        bind_info.package_name);
                    // Open mode is only set for testing purpose, in which case
                    // we want to keep going
                    if (pimpl_->open_mode == Impl::OpenMode::AUTO) {
                        teec_result = TEEC_ERROR_COMMUNICATION;
                        continue;
                    }
                }
            }

            teec_result = client->TEEC_InitializeContext(name, context);
            if (teec_result == TEEC_SUCCESS) {
                pimpl_->client = std::move(client);
                pimpl_->tee_service = std::move(service);
                if (client_factory->mayRequireTuiService()) {
                    pimpl_->bindTuiService();
                }

                break;
            }

            LOG_I("Cannot access Kinibi through the %s",
                  client_factory->name().c_str());
        }

        if (teec_result != TEEC_SUCCESS) {
            // Error logged in the upper layer
            pimpl_->unbindServices();
        }
    } else {
        teec_result = pimpl_->client->TEEC_InitializeContext(name, context);
    }

    if (teec_result == TEEC_SUCCESS) {
        pimpl_->init_context_count++;
        LOG_D("init_context_count: %d open_device_count: %d",
              pimpl_->init_context_count, pimpl_->open_device_count);
    }

    return teec_result;
}

void TrustonicTEE::TEEC_FinalizeContext(
    TEEC_Context*           context) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
    if (pimpl_->init_context_count == 0) {
        LOG_E("init counter cannot go negative");
        return;
    }

    pimpl_->init_context_count--;
    if ((pimpl_->init_context_count == 0) && (pimpl_->open_device_count == 0)) {
        pimpl_->unbindServices();
    }

    pimpl_->client->TEEC_FinalizeContext(context);
    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
}

TEEC_Result TrustonicTEE::TEEC_RegisterSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    return pimpl_->client->TEEC_RegisterSharedMemory(context, sharedMem);
}

TEEC_Result TrustonicTEE::TEEC_AllocateSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    return pimpl_->client->TEEC_AllocateSharedMemory(context, sharedMem);
}

void TrustonicTEE::TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory*      sharedMem) {
    pimpl_->client->TEEC_ReleaseSharedMemory(sharedMem);
}

TEEC_Result TrustonicTEE::TEEC_OpenSession(
    TEEC_Context*           context,
    TEEC_Session*           session,
    const TEEC_UUID*        destination,
    uint32_t                connectionMethod,
    const void*             connectionData,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    return pimpl_->client->TEEC_OpenSession(context, session, destination,
                                            connectionMethod, connectionData,
                                            operation, returnOrigin);
}

void TrustonicTEE::TEEC_CloseSession(
    TEEC_Session*           session) {
    pimpl_->client->TEEC_CloseSession(session);
}

TEEC_Result TrustonicTEE::TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    return pimpl_->client->TEEC_InvokeCommand(session, commandID, operation,
            returnOrigin);
}

void TrustonicTEE::TEEC_RequestCancellation(
    TEEC_Operation*         operation) {
    pimpl_->client->TEEC_RequestCancellation(operation);
}

// MobiCore
mcResult_t TrustonicTEE::mcOpenDevice(
    uint32_t                deviceId) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    if (!pimpl_->setOpenMode(deviceId)) {
        LOG_E("%s invalid device ID %x", __FUNCTION__, deviceId);
        return MC_DRV_ERR_UNKNOWN_DEVICE;
    }

    mcResult_t mc_result = MC_DRV_ERR_NOT_IMPLEMENTED;
    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
    if (pimpl_->open_device_count > 0) {
        mc_result = MC_DRV_OK;
    } else if (pimpl_->init_context_count > 0) {
        mc_result = pimpl_->client->mcOpenDevice(deviceId);
    } else {
        for (auto& client_factory: pimpl_->clientFactories()) {
            LOG_I("Try to access Kinibi through the %s",
                  client_factory->name().c_str());
            auto client = client_factory->create();
            if (!client) {
                mc_result = MC_DRV_ERR_NOT_IMPLEMENTED;
                continue;
            }

            client->TEEC_TT_RegisterPlatformContext(
                        pimpl_->jvm, pimpl_->application_context);
            std::unique_ptr<JavaProcess> service;
            BindInformation bind_info;
            if (client->getBindInformation(&bind_info)) {
                if (pimpl_->jvm) {
                    service = pimpl_->bindService(
                                bind_info.package_name, bind_info.starter_class);
                    if (!service) {
                        LOG_E("Service not supported '%s'",
                            bind_info.package_name);
                        mc_result = MC_DRV_ERR_NOT_IMPLEMENTED;
                        continue;
                    }
                } else {
                    LOG_I("No Java VM to bind service '%s'",
                        bind_info.package_name);
                    // Open mode is only set for testing purpose, in which case
                    // we want to keep going
                    if (pimpl_->open_mode == Impl::OpenMode::AUTO) {
                        mc_result = MC_DRV_ERR_DAEMON_UNREACHABLE;
                        continue;
                    }
                }
            }

            mc_result = client->mcOpenDevice(deviceId);
            if (mc_result == MC_DRV_OK) {
                pimpl_->client = std::move(client);
                pimpl_->tee_service = std::move(service);
                if (client_factory->mayRequireTuiService()) {
                    pimpl_->bindTuiService();
                }

                break;
            }

            LOG_I("Cannot access Kinibi through the %s",
                  client_factory->name().c_str());
        }

        if (mc_result != MC_DRV_OK) {
            // Error logged in the upper layer
            pimpl_->unbindServices();
        }
    }

    if (mc_result == MC_DRV_OK) {
        pimpl_->open_device_count++;
    }

    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
    return mc_result;
}

mcResult_t TrustonicTEE::mcCloseDevice(
    uint32_t                deviceId) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    if (pimpl_->open_device_count == 0) {
        LOG_E("init counter cannot go negative");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
    mcResult_t mc_result = MC_DRV_OK;
    if (pimpl_->open_device_count == 1) {
        mc_result = pimpl_->client->mcCloseDevice(deviceId);
    }

    if (mc_result == MC_DRV_OK) {
        pimpl_->open_device_count--;
    } else if (mc_result != MC_DRV_ERR_SESSION_PENDING) {
        // Connection broken
        pimpl_->open_device_count = 0;
    }

    LOG_D("init_context_count: %d open_device_count: %d",
          pimpl_->init_context_count, pimpl_->open_device_count);
    if ((pimpl_->init_context_count == 0) && (pimpl_->open_device_count == 0)) {
        pimpl_->unbindServices();
    }

    return mc_result;
}

mcResult_t TrustonicTEE::mcOpenSession(
    mcSessionHandle_t*      session,
    const mcUuid_t*         uuid,
    uint8_t*                tci,
    uint32_t                tciLen) {
    return pimpl_->client->mcOpenSession(session, uuid, tci, tciLen);
}

mcResult_t TrustonicTEE::mcOpenTrustlet(
    mcSessionHandle_t*      session,
    mcSpid_t                spid,
    uint8_t*                trustedapp,
    uint32_t                tLen,
    uint8_t*                tci,
    uint32_t                tciLen) {
    return pimpl_->client->mcOpenTrustlet(session, spid, trustedapp, tLen, tci,
                                          tciLen);
}

mcResult_t TrustonicTEE::mcCloseSession(
    mcSessionHandle_t*      session) {
    return pimpl_->client->mcCloseSession(session);
}

mcResult_t TrustonicTEE::mcNotify(
    mcSessionHandle_t*      session) {
    return pimpl_->client->mcNotify(session);
}

mcResult_t TrustonicTEE::mcWaitNotification(
    mcSessionHandle_t*      session,
    int32_t                 timeout) {
    return pimpl_->client->mcWaitNotification(session, timeout);
}

mcResult_t TrustonicTEE::mcMallocWsm(
    uint32_t                deviceId,
    uint32_t                len,
    uint8_t**               wsm) {
    return pimpl_->client->mcMallocWsm(deviceId, len, wsm);
}

mcResult_t TrustonicTEE::mcFreeWsm(
    uint32_t                deviceId,
    uint8_t*                wsm,
    uint32_t                len) {
    return pimpl_->client->mcFreeWsm(deviceId, wsm, len);
}

mcResult_t TrustonicTEE::mcMap(
    mcSessionHandle_t*      session,
    void*                   buf,
    uint32_t                len,
    mcBulkMap_t*            mapInfo) {
    return pimpl_->client->mcMap(session, buf, len, mapInfo);
}

mcResult_t TrustonicTEE::mcUnmap(
    mcSessionHandle_t*      session,
    void*                   buf,
    mcBulkMap_t*            mapInfo) {
    return pimpl_->client->mcUnmap(session, buf, mapInfo);
}

mcResult_t TrustonicTEE::mcGetSessionErrorCode(
    mcSessionHandle_t*      session,
    int32_t*                lastErr) {
    return pimpl_->client->mcGetSessionErrorCode(session, lastErr);
}

mcResult_t TrustonicTEE::mcGetMobiCoreVersion(
    uint32_t                deviceId,
    mcVersionInfo_t*        versionInfo) {
    return pimpl_->client->mcGetMobiCoreVersion(deviceId, versionInfo);
}
