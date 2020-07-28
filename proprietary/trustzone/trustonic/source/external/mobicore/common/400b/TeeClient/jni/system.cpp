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

// C++
#include <functional>
#include <mutex>
#include <string>
// C
#include <dlfcn.h>
// Local
#include "dynamic_log.h"
#include "system.h"

using namespace trustonic;

#define RESOLVE_FUNCTION(signature, error_code) \
    static std::function<signature> function; \
    if (!function) { \
        function = pimpl_->resolve<signature>(__FUNCTION__); \
        if (!function) { \
            return error_code; \
        } \
    } \
    LOG_D("call(%s)", __FUNCTION__)

struct System::Impl {
    std::mutex  dl_handle_mutex;
    void*       dl_handle = nullptr;
    bool        init_done = false;

    int init() {
        const char* path = "libMcClient.so";
        std::lock_guard<std::mutex> lock(dl_handle_mutex);
        if (!init_done) {
            init_done = true;
            dl_handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
            if (!dl_handle) {
                // Get error first, in case LOG_E leads to a dlopen() itself
                const char* error = dlerror();
                LOG_E("dlopen(%s) failed: %s", path, error);
                return -1;
            }
            LOG_I("dlopen(%s) succeeded", path);
            return 0;
        } else {
            return dl_handle ? 0 : -1;
        }
    }
    template <typename Signature>
    std::function<Signature> resolve(const char* name) {
        void* handle = dlsym(dl_handle, name);
        if (!handle) {
            LOG_E("dlsym(%s) failed", name);
            return std::function<Signature>();
        }
        LOG_D("dlsym(%s) succeeded", name);
        return reinterpret_cast<Signature*>(handle);
    }
};

// Missing std::make_unique
System::System(): pimpl_(new Impl) {}

// Destructor needs the size of Impl
System::~System() {}

// Proprietary
void System::TEEC_TT_RegisterPlatformContext(
    void*                   /*globalContext*/,
    void*                   /*localContext*/) {
    ENTER();
    EXIT_NORETURN();
}

struct TestData {
    bool        using_proxy = false;
    bool        using_proxy_protocol_v3 = false;
};

TEEC_Result System::TEEC_TT_TestEntry(
    void*                   buff,
    size_t                  /*len*/,
    uint32_t*               /*tag*/) {
    ENTER();
    TestData* test_data = static_cast<TestData*>(buff);
    test_data->using_proxy = false;
    test_data->using_proxy_protocol_v3 = false;
    // Do not propagate further in the libMcClient
    EXIT(TEEC_SUCCESS);
}

// Global Platform
TEEC_Result System::TEEC_InitializeContext(
    const char*             name,
    TEEC_Context*           context) {
    ENTER();
    if (pimpl_->init()) {
        return TEEC_ERROR_NOT_IMPLEMENTED;
    }
    RESOLVE_FUNCTION(TEEC_Result(const char*, TEEC_Context*),
                     TEEC_ERROR_NOT_IMPLEMENTED);
    EXIT(function(name, context));
}

void System::TEEC_FinalizeContext(
    TEEC_Context*           context) {
    ENTER();
    RESOLVE_FUNCTION(void(TEEC_Context*),);
    function(context);
    EXIT_NORETURN();
}

TEEC_Result System::TEEC_RegisterSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    RESOLVE_FUNCTION(TEEC_Result(TEEC_Context*, TEEC_SharedMemory*),
                     TEEC_ERROR_NOT_IMPLEMENTED);
    EXIT(function(context, sharedMem));
}

TEEC_Result System::TEEC_AllocateSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    RESOLVE_FUNCTION(TEEC_Result(TEEC_Context*, TEEC_SharedMemory*),
                     TEEC_ERROR_NOT_IMPLEMENTED);
    EXIT(function(context, sharedMem));
}

void System::TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    RESOLVE_FUNCTION(void(TEEC_SharedMemory*),);
    function(sharedMem);
    EXIT_NORETURN();
}

TEEC_Result System::TEEC_OpenSession(
    TEEC_Context*           context,
    TEEC_Session*           session,
    const TEEC_UUID*        destination,
    uint32_t                connectionMethod,
    const void*             connectionData,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    RESOLVE_FUNCTION(TEEC_Result(TEEC_Context*, TEEC_Session*, const TEEC_UUID*,
                                 uint32_t, const void*, TEEC_Operation*,
                                 uint32_t*),
                     TEEC_ERROR_NOT_IMPLEMENTED);
    EXIT(function(context, session, destination, connectionMethod,
                  connectionData, operation, returnOrigin));
}

void System::TEEC_CloseSession(
    TEEC_Session*           session) {
    ENTER();
    RESOLVE_FUNCTION(void(TEEC_Session*),);
    function(session);
    EXIT_NORETURN();
}

TEEC_Result System::TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    RESOLVE_FUNCTION(TEEC_Result(TEEC_Session*, uint32_t, TEEC_Operation*,
                                 uint32_t*),
                     TEEC_ERROR_NOT_IMPLEMENTED);
    EXIT(function(session, commandID, operation, returnOrigin));
}

void System::TEEC_RequestCancellation(
    TEEC_Operation*         operation) {
    ENTER();
    RESOLVE_FUNCTION(void(TEEC_Operation*),);
    function(operation);
    EXIT_NORETURN();
}

// MobiCore
mcResult_t System::mcOpenDevice(
    uint32_t                /*deviceId*/) {
    ENTER();
    if (pimpl_->init()) {
        return MC_DRV_ERR_NOT_IMPLEMENTED;
    }
    RESOLVE_FUNCTION(mcResult_t(uint32_t), MC_DRV_ERR_NOT_IMPLEMENTED);
    // deviceId has already been checked at this point. Only call McClient with
    // correct device ID.
    EXIT(function(MC_DEVICE_ID_DEFAULT));
}

mcResult_t System::mcCloseDevice(
    uint32_t                deviceId) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(uint32_t), MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(deviceId));
}

mcResult_t System::mcOpenSession(
    mcSessionHandle_t*      session,
    const mcUuid_t*         uuid,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*, const mcUuid_t*, uint8_t*,
                                uint32_t), MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session, uuid, tci, tciLen));
}

mcResult_t System::mcOpenTrustlet(
    mcSessionHandle_t*      session,
    mcSpid_t                spid,
    uint8_t*                trustedapp,
    uint32_t                tLen,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*, mcSpid_t, uint8_t*,
                                uint32_t, uint8_t*, uint32_t),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session, spid, trustedapp, tLen, tci, tciLen));
}

mcResult_t System::mcCloseSession(
    mcSessionHandle_t*      session) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session));
}

mcResult_t System::mcNotify(
    mcSessionHandle_t*      session) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session));
}

mcResult_t System::mcWaitNotification(
    mcSessionHandle_t*      session,
    int32_t                 timeout) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*, int32_t),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session, timeout));
}

mcResult_t System::mcMallocWsm(
    uint32_t                deviceId,
    uint32_t                len,
    uint8_t**               wsm) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(uint32_t, uint32_t, uint32_t, uint8_t**,
                                uint32_t), MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(deviceId, 0, len, wsm, 0));
}

mcResult_t System::mcFreeWsm(
    uint32_t                deviceId,
    uint8_t*                wsm,
    uint32_t                /*len*/) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(uint32_t, uint8_t*),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(deviceId, wsm));
}

mcResult_t System::mcMap(
    mcSessionHandle_t*      session,
    void*                   buf,
    uint32_t                len,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*, void*, uint32_t,
                                mcBulkMap_t*), MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session, buf, len, mapInfo));
}

mcResult_t System::mcUnmap(
    mcSessionHandle_t*      session,
    void*                   buf,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*, void*, mcBulkMap_t*),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session, buf, mapInfo));
}

mcResult_t System::mcGetSessionErrorCode(
    mcSessionHandle_t*      session,
    int32_t*                lastErr) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(mcSessionHandle_t*, int32_t*),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(session, lastErr));
}

mcResult_t System::mcGetMobiCoreVersion(
    uint32_t                deviceId,
    mcVersionInfo_t*        versionInfo) {
    ENTER();
    RESOLVE_FUNCTION(mcResult_t(uint32_t, mcVersionInfo_t*),
                     MC_DRV_ERR_NOT_IMPLEMENTED);
    EXIT(function(deviceId, versionInfo));
}
