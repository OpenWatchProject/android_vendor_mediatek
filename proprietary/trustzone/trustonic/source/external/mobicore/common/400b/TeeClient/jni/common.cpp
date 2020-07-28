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

#include <mutex>

#include "jni.h"
#include "MobiCoreDriverApi.h"
#include "tee_client_api.h"
#include "dynamic_log.h"
#include "dummy.h"
#include "trustonic_tee.h"
#include "common.h"

using namespace trustonic;

struct Common::Impl {
    Dummy           dummy;
    TrustonicTEE    trustonic_tee;
    IClient*        client = &dummy;
    std::mutex      init_mutex;
    JavaVM*         jvm = nullptr;
    jobject         application_context = nullptr;

    void manageApplicationContext(jobject newApplicationContext) {
        if (!jvm) {
            LOG_E("No JVM");
            return;
        }
        JNIEnv* env = nullptr;
        bool thread_attached = false;
        switch (jvm->GetEnv((void**) &env, JNI_VERSION_1_6)) {
            case JNI_OK:
                break;
            case JNI_EDETACHED:
                // Thread not attached, attach it
                if (jvm->AttachCurrentThread(&env, NULL) != 0) {
                    LOG_E("Failed to attach");
                    return;
                }
                thread_attached = true;
                break;
            default:
                return;
        }

        if (application_context) {
            LOG_D("Updating the application context global reference");
            env->DeleteGlobalRef(application_context);
        }
        application_context = (jobject)env->NewGlobalRef(newApplicationContext);

        if (thread_attached) {
            jvm->DetachCurrentThread();
        }
    }
};

// Missing std::make_unique
Common::Common(): pimpl_(new Impl) {
}

// Destructor needs the size of Impl
Common::~Common() {}

// Proprietary
void Common::TEEC_TT_RegisterPlatformContext(
    void*                   globalContext,
    void*                   localContext) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    pimpl_->jvm = static_cast<JavaVM*>(globalContext);
    pimpl_->manageApplicationContext(static_cast<jobject>(localContext));
    // Update client data, if any
    pimpl_->client->TEEC_TT_RegisterPlatformContext(pimpl_->jvm, pimpl_->application_context);
}

struct TestData {
    bool        using_proxy = false;
    bool        using_proxy_protocol_v3 = false;
};

TEEC_Result Common::TEEC_TT_TestEntry(
    void*                   buff,
    size_t                  len,
    uint32_t*               tag) {
    if (tag && (!buff || len == 0)) {
        // Return the version
        *tag = 0x1;
        LOG_D("test interface version 0x%x", *tag);
        return TEEC_SUCCESS;
    }

    LOG_D("test buf %p len %zu", buff, len);
    if (!buff && len != sizeof(TestData)) {
        LOG_W("test data not consistent; this api is for internal testing only");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    return pimpl_->client->TEEC_TT_TestEntry(buff, len, tag);
}

// Global Platform
TEEC_Result Common::TEEC_InitializeContext(
    const char*             name,
    TEEC_Context*           context) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    if (pimpl_->client != &pimpl_->dummy) {
        return pimpl_->client->TEEC_InitializeContext(name, context);
    }

    pimpl_->trustonic_tee.TEEC_TT_RegisterPlatformContext(pimpl_->jvm, pimpl_->application_context);
    if (pimpl_->trustonic_tee.TEEC_InitializeContext(name, context) == TEEC_SUCCESS) {
        pimpl_->client = &pimpl_->trustonic_tee;
        return TEEC_SUCCESS;
    }

    LOG_E("Failed to open lower layers: %s", strerror(errno));
    switch (errno) {
        case EINVAL:
            return TEEC_ERROR_BAD_PARAMETERS;
        case ENOENT:
        case ECONNREFUSED:
            return TEEC_ERROR_COMMUNICATION;
    }

    return TEEC_ERROR_GENERIC;
}

void Common::TEEC_FinalizeContext(
    TEEC_Context*           context) {
    pimpl_->client->TEEC_FinalizeContext(context);
}

TEEC_Result Common::TEEC_RegisterSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    return pimpl_->client->TEEC_RegisterSharedMemory(context, sharedMem);
}

TEEC_Result Common::TEEC_AllocateSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    return pimpl_->client->TEEC_AllocateSharedMemory(context, sharedMem);
}

void Common::TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory*      sharedMem) {
    pimpl_->client->TEEC_ReleaseSharedMemory(sharedMem);
}

TEEC_Result Common::TEEC_OpenSession(
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

void Common::TEEC_CloseSession(
    TEEC_Session*           session) {
    pimpl_->client->TEEC_CloseSession(session);
}

TEEC_Result Common::TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    return pimpl_->client->TEEC_InvokeCommand(session, commandID, operation,
                                              returnOrigin);
}

void Common::TEEC_RequestCancellation(
    TEEC_Operation*         operation) {
    pimpl_->client->TEEC_RequestCancellation(operation);
}

// MobiCore
mcResult_t Common::mcOpenDevice(
    uint32_t                deviceId) {
    std::lock_guard<std::mutex> lock(pimpl_->init_mutex);
    if (pimpl_->client != &pimpl_->dummy) {
        return pimpl_->client->mcOpenDevice(deviceId);
    }

    pimpl_->trustonic_tee.TEEC_TT_RegisterPlatformContext(pimpl_->jvm, pimpl_->application_context);
    mcResult_t res = pimpl_->trustonic_tee.mcOpenDevice(deviceId);
    if (res == MC_DRV_OK) {
        pimpl_->client = &pimpl_->trustonic_tee;
        return res;
    }

    LOG_E("failed open lower layers: %x", res);
    return res;
}

mcResult_t Common::mcCloseDevice(
    uint32_t                deviceId) {
    return pimpl_->client->mcCloseDevice(deviceId);
}

mcResult_t Common::mcOpenSession(
    mcSessionHandle_t*      session,
    const mcUuid_t*         uuid,
    uint8_t*                tci,
    uint32_t                tciLen) {
    return pimpl_->client->mcOpenSession(session, uuid, tci, tciLen);
}

mcResult_t Common::mcOpenTrustlet(
    mcSessionHandle_t*      session,
    mcSpid_t                spid,
    uint8_t*                trustedapp,
    uint32_t                tLen,
    uint8_t*                tci,
    uint32_t                tciLen) {
    return pimpl_->client->mcOpenTrustlet(session, spid, trustedapp, tLen, tci,
                                          tciLen);
}

mcResult_t Common::mcCloseSession(
    mcSessionHandle_t*      session) {
    return pimpl_->client->mcCloseSession(session);
}

mcResult_t Common::mcNotify(
    mcSessionHandle_t*      session) {
    return pimpl_->client->mcNotify(session);
}

mcResult_t Common::mcWaitNotification(
    mcSessionHandle_t*      session,
    int32_t                 timeout) {
    return pimpl_->client->mcWaitNotification(session, timeout);
}

mcResult_t Common::mcMallocWsm(
    uint32_t                deviceId,
    uint32_t                len,
    uint8_t**               wsm) {
    return pimpl_->client->mcMallocWsm(deviceId, len, wsm);
}

mcResult_t Common::mcFreeWsm(
    uint32_t                deviceId,
    uint8_t*                wsm,
    uint32_t                len) {
    return pimpl_->client->mcFreeWsm(deviceId, wsm, len);
}

mcResult_t Common::mcMap(
    mcSessionHandle_t*      session,
    void*                   buf,
    uint32_t                len,
    mcBulkMap_t*            mapInfo) {
    return pimpl_->client->mcMap(session, buf, len, mapInfo);
}

mcResult_t Common::mcUnmap(
    mcSessionHandle_t*      session,
    void*                   buf,
    mcBulkMap_t*            mapInfo) {
    return pimpl_->client->mcUnmap(session, buf, mapInfo);
}

mcResult_t Common::mcGetSessionErrorCode(
    mcSessionHandle_t*      session,
    int32_t*                lastErr) {
    return pimpl_->client->mcGetSessionErrorCode(session, lastErr);
}

mcResult_t Common::mcGetMobiCoreVersion(
    uint32_t                deviceId,
    mcVersionInfo_t*        versionInfo) {
    return pimpl_->client->mcGetMobiCoreVersion(deviceId, versionInfo);
}
