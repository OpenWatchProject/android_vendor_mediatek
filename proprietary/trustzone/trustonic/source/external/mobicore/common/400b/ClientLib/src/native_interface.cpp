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

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "dynamic_log.h"

#include "common.h"

#define BUFFER_LENGTH_MAX 0x40000000

using namespace trustonic;

// Open/close count
static std::mutex open_close_lock;
static int open_count;

static Common& common = Common::getInstance();

// Proprietary

void TEEC_TT_RegisterPlatformContext(
    void*                   globalContext,
    void*                   localContext) {
    ENTER();
    if (!globalContext || !localContext) {
        return;
    }
    common.TEEC_TT_RegisterPlatformContext(globalContext, localContext);
    EXIT_NORETURN();
}


// Global Platform

static const char * GP_errorToString(uint32_t error) {
    switch (error) {
        case TEEC_SUCCESS:
            return "SUCCESS";
        case TEEC_ERROR_GENERIC:
            return "GENERIC";
        case TEEC_ERROR_ACCESS_DENIED:
            return "ACCESS_DENIED";
        case TEEC_ERROR_CANCEL:
            return "CANCEL";
        case TEEC_ERROR_ACCESS_CONFLICT:
            return "ACCESS_CONFLICT";
        case TEEC_ERROR_EXCESS_DATA:
            return "EXCESS_DATA";
        case TEEC_ERROR_BAD_FORMAT:
            return "BAD_FORMAT";
        case TEEC_ERROR_BAD_PARAMETERS:
            return "BAD_PARAMETERS";
        case TEEC_ERROR_BAD_STATE:
            return "BAD_STATE";
        case TEEC_ERROR_ITEM_NOT_FOUND:
            return "ITEM_NOT_FOUND";
        case TEEC_ERROR_NOT_IMPLEMENTED:
            return "NOT_IMPLEMENTED";
        case TEEC_ERROR_NOT_SUPPORTED:
            return "NOT_SUPPORTED";
        case TEEC_ERROR_NO_DATA:
            return "NO_DATA";
        case TEEC_ERROR_OUT_OF_MEMORY:
            return "OUT_OF_MEMORY";
        case TEEC_ERROR_BUSY:
            return "BUSY";
        case TEEC_ERROR_COMMUNICATION:
            return "COMMUNICATION";
        case TEEC_ERROR_SECURITY:
            return "SECURITY";
        case TEEC_ERROR_SHORT_BUFFER:
            return "SHORT_BUFFER";
        case TEEC_ERROR_TARGET_DEAD:
            return "TARGET_DEAD";
        case TEEC_ERROR_STORAGE_NO_SPACE:
            return "STORAGE_NO_SPACE";
    }
    return "???";
}

static const char * GP_originToString(uint32_t* error) {
    switch (*error) {
        case TEEC_ORIGIN_API:
            return "API";
        case TEEC_ORIGIN_COMMS:
            return "COMMS";
        case TEEC_ORIGIN_TEE:
            return "TEE";
        case TEEC_ORIGIN_TRUSTED_APP:
            return "TRUSTED_APP";
    }
    return "???";
}

#define GP_EXIT_ORIG(err__, orig_ptr__) \
    do { \
        auto rc__ = (err__); \
        if (rc__) { \
            if (orig_ptr__) { \
                LOG_E("%s returned %s (rc 0x%x) from %s", __FUNCTION__, GP_errorToString(rc__), rc__, GP_originToString(orig_ptr__)); \
            } else { \
                LOG_E("%s returned %s (rc 0x%x)", __FUNCTION__, GP_errorToString(rc__), rc__); \
            } \
        } else { \
            LOG_D("%s returned %s (rc 0x%x)", __func__, GP_errorToString(rc__), rc__); \
        } \
        return rc__; \
    } while (0)

#define GP_EXIT(err__) \
    GP_EXIT_ORIG(err__, nullptr)

// Proprietary
TEEC_Result TEEC_TT_TestEntry(
    void*                   buff,
    size_t                  len,
    uint32_t*               tag) {
    ENTER();
    GP_EXIT(common.TEEC_TT_TestEntry(buff, len, tag));
}

TEEC_Result TEEC_InitializeContext(
    const char*             name,
    TEEC_Context*           context) {
    ENTER();
    std::lock_guard<std::mutex> lock(open_close_lock);
    // Check parameters
    if (!context) {
        LOG_E("context is null");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }

    TEEC_Result teec_result = common.TEEC_InitializeContext(name, context);
    if (teec_result == TEEC_SUCCESS) {
        open_count++;
    }

    GP_EXIT(teec_result);
}

void TEEC_FinalizeContext(
    TEEC_Context*           context) {
    ENTER();
    std::lock_guard<std::mutex> lock(open_close_lock);
    // Check parameters
    if (!context) {
        LOG_E("context is null");
        EXIT_NORETURN();
    }

    common.TEEC_FinalizeContext(context);
    open_count--;
    EXIT_NORETURN();
}

TEEC_Result TEEC_RegisterSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    // Check parameters
    if (!context) {
        LOG_E("context is null");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if (!sharedMem) {
        LOG_E("sharedMem is null");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if (!sharedMem->buffer) {
        LOG_E("sharedMem->buffer is null");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if ((static_cast<int>(sharedMem->flags) & ~TEEC_MEM_FLAGS_MASK)) {
        LOG_E("sharedMem->flags is incorrect");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if (sharedMem->flags == 0) {
        LOG_E("sharedMem->flags is incorrect");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    GP_EXIT(common.TEEC_RegisterSharedMemory(context, sharedMem));
}

TEEC_Result TEEC_AllocateSharedMemory(
    TEEC_Context*           context,
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    // Check parameters
    if (!context) {
        LOG_E("context is null");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if (!sharedMem) {
        LOG_E("sharedMem is null");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if (static_cast<int>(sharedMem->flags) & ~TEEC_MEM_INOUT) {
        LOG_E("sharedMem->flags is incorrect");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    if (sharedMem->flags == 0) {
        LOG_E("sharedMem->flags is incorrect");
        GP_EXIT(TEEC_ERROR_BAD_PARAMETERS);
    }
    GP_EXIT(common.TEEC_AllocateSharedMemory(context, sharedMem));
}

void TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory*      sharedMem) {
    ENTER();
    // Check parameters
    if (!sharedMem) {
        LOG_E("sharedMem is null");
        EXIT_NORETURN();
    }
    if (!sharedMem->buffer) {
        LOG_E("sharedMem->buffer is null");
        EXIT_NORETURN();
    }
    // Release shared memory
    common.TEEC_ReleaseSharedMemory(sharedMem);
    EXIT_NORETURN();
}

TEEC_Result TEEC_OpenSession(
    TEEC_Context*           context,
    TEEC_Session*           session,
    const TEEC_UUID*        destination,
    uint32_t                connectionMethod,
    const void*             connectionData,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    // Set default origin
    if (returnOrigin) {
        *returnOrigin = TEEC_ORIGIN_API;
    }
    // Check parameters
    if (!context) {
        LOG_E("context is null");
        GP_EXIT_ORIG(TEEC_ERROR_BAD_PARAMETERS, returnOrigin);
    }
    if (!session) {
        LOG_E("session is null");
        GP_EXIT_ORIG(TEEC_ERROR_BAD_PARAMETERS, returnOrigin);
    }
    if ((connectionMethod != TEEC_LOGIN_PUBLIC) &&
            (connectionMethod != TEEC_LOGIN_USER) &&
            (connectionMethod != TEEC_LOGIN_GROUP) &&
            (connectionMethod != TEEC_LOGIN_APPLICATION) &&
            (connectionMethod != TEEC_LOGIN_USER_APPLICATION) &&
            (connectionMethod != TEEC_LOGIN_GROUP_APPLICATION)) {
        LOG_E("connectionMethod not supported");
        GP_EXIT_ORIG(TEEC_ERROR_NOT_IMPLEMENTED, returnOrigin);
    }
    if ((TEEC_LOGIN_GROUP == connectionMethod) ||
            (TEEC_LOGIN_GROUP_APPLICATION == connectionMethod)) {
        if (!connectionData) {
            LOG_E("connectionData is null");
            GP_EXIT_ORIG(TEEC_ERROR_BAD_PARAMETERS, returnOrigin);
        }
    }

    TEEC_Result res = common.TEEC_OpenSession(context, session, destination,
                      connectionMethod, connectionData,
                      operation, returnOrigin);
    GP_EXIT_ORIG(res, returnOrigin);
}

void TEEC_CloseSession(
    TEEC_Session*           session) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session is null");
        EXIT_NORETURN();
    }

    common.TEEC_CloseSession(session);
    EXIT_NORETURN();
}

TEEC_Result TEEC_InvokeCommand(
    TEEC_Session*           session,
    uint32_t                commandID,
    TEEC_Operation*         operation,
    uint32_t*               returnOrigin) {
    ENTER();
    // Set default origin
    if (returnOrigin) {
        *returnOrigin = TEEC_ORIGIN_API;
    }
    // Check parameters
    if (!session) {
        LOG_E("session is null");
        GP_EXIT_ORIG(TEEC_ERROR_BAD_PARAMETERS, returnOrigin);
    }

    TEEC_Result res = common.TEEC_InvokeCommand(session, commandID, operation,
                      returnOrigin);
    GP_EXIT_ORIG(res, returnOrigin);
}

/*
 * This function ensures that an operation is on the way before proceeding, so
 * the layers below can block waiting for the call to either TEEC_OpenSession or
 * TEEC_InvokeCommand.
 */
void TEEC_RequestCancellation(
    TEEC_Operation*         operation) {
    ENTER();
    if (!operation) {
        LOG_E("operation is null");
    }

    common.TEEC_RequestCancellation(operation);
    EXIT_NORETURN();
}


// MobiCore
static const char * MC_errorToString(uint32_t error) {
    switch (error) {
        case MC_DRV_OK:
            return "MC_DRV_OK";
        case MC_DRV_NO_NOTIFICATION :
            return "NO_NOTIFICATION";
        case MC_DRV_ERR_NOTIFICATION :
            return "ERR_NOTIFICATION";
        case MC_DRV_ERR_NOT_IMPLEMENTED :
            return "NOT_IMPLEMENTED";
        case MC_DRV_ERR_OUT_OF_RESOURCES :
            return "OUT_OF_RESOURCES";
        case MC_DRV_ERR_INIT :
            return "INIT";
        case MC_DRV_ERR_UNKNOWN :
            return "UNKNOWN";
        case MC_DRV_ERR_UNKNOWN_DEVICE :
            return "UNKNOWN_DEVICE";
        case MC_DRV_ERR_UNKNOWN_SESSION :
            return "UNKNOWN_SESSION";
        case MC_DRV_ERR_INVALID_OPERATION :
            return "INVALID_OPERATION";
        case MC_DRV_ERR_INVALID_RESPONSE :
            return "INVALID_RESPONSE";
        case MC_DRV_ERR_TIMEOUT :
            return "TIMEOUT";
        case MC_DRV_ERR_NO_FREE_MEMORY :
            return "NO_FREE_MEMORY";
        case MC_DRV_ERR_FREE_MEMORY_FAILED :
            return "FREE_MEMORY_FAILED";
        case MC_DRV_ERR_SESSION_PENDING :
            return "SESSION_PENDING";
        case MC_DRV_ERR_DAEMON_UNREACHABLE :
            return "DAEMON_UNREACHABLE";
        case MC_DRV_ERR_INVALID_DEVICE_FILE :
            return "INVALID_DEVICE_FILE";
        case MC_DRV_ERR_INVALID_PARAMETER :
            return "INVALID_PARAMETER";
        case MC_DRV_ERR_KERNEL_MODULE :
            return "KERNEL_MODULE";
        case MC_DRV_ERR_BULK_MAPPING :
            return "BULK_MAPPING";
        case MC_DRV_ERR_BULK_UNMAPPING :
            return "BULK_UNMAPPING";
        case MC_DRV_INFO_NOTIFICATION :
            return "INFO_NOTIFICATION";
        case MC_DRV_ERR_NQ_FAILED :
            return "NQ_FAILED";
        case MC_DRV_ERR_DAEMON_VERSION :
            return "DAEMON_VERSION";
        case MC_DRV_ERR_CONTAINER_VERSION :
            return "CONTAINER_VERSION";
        case MC_DRV_ERR_WRONG_PUBLIC_KEY :
            return "WRONG_PUBLIC_KEY";
        case MC_DRV_ERR_CONTAINER_TYPE_MISMATCH :
            return "CONTAINER_TYPE_MISMATCH";
        case MC_DRV_ERR_CONTAINER_LOCKED :
            return "CONTAINER_LOCKED";
        case MC_DRV_ERR_SP_NO_CHILD :
            return "SP_NO_CHILD";
        case MC_DRV_ERR_TL_NO_CHILD :
            return "TL_NO_CHILD";
        case MC_DRV_ERR_UNWRAP_ROOT_FAILED :
            return "UNWRAP_ROOT_FAILED";
        case MC_DRV_ERR_UNWRAP_SP_FAILED :
            return "UNWRAP_SP_FAILED";
        case MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED :
            return "UNWRAP_TRUSTLET_FAILED";
        case MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN :
            return "DAEMON_DEVICE_NOT_OPEN";
        case MC_DRV_ERR_TA_ATTESTATION_ERROR:
            return "TA_ATTESTATION_ERROR";
        case MC_DRV_ERR_TA_HEADER_ERROR:
            return "TA_HEADER_ERROR";
        case MC_DRV_ERR_SERVICE_BLOCKED:
            return "SERVICE_BLOCKED";
        case MC_DRV_ERR_SERVICE_LOCKED:
            return "SERVICE_LOCKED";
        case MC_DRV_ERR_SERVICE_KILLED:
            return "SERVICE_KILLED";
        case MC_DRV_ERR_NO_FREE_INSTANCES:
            return "NO_FREE_INSTANCES";
    }
    return "???";
}

#define MC_EXIT(err__) \
    do { \
        auto rc__ = (err__); \
        if (rc__) { \
            LOG_E("%s returned %s (rc 0x%x)", __FUNCTION__, MC_errorToString(rc__), rc__); \
        } else { \
            LOG_D("%s returned %s (rc 0x%x)", __func__, MC_errorToString(rc__), rc__); \
        } \
        return rc__; \
    } while (0)

/*
 * On mcCloseDevice(), all mappings need to be removed, which means we need to
 * keep them somewhere.  We keep them in an array, resized when needed.
 * Both map and length must be kept in user-space, as after a fork another
 * mapping with same address may have appeared from the other process(es).
 */
class WSMs {
    struct Map {
        uint8_t* wsm;
        uint32_t len;
    };
    std::vector<Map> maps_;
    std::mutex mutex_;
public:
    void add(uint8_t* wsm, uint32_t len) {
        LOG_D("WSMs::add %p %d", wsm, len);
        std::lock_guard<std::mutex> lock(mutex_);
        maps_.push_back({ wsm, len });
    }
    void remove(uint8_t* wsm) {
        std::lock_guard<std::mutex> lock(mutex_);
        /* Find position of WSM */
        for (auto i = maps_.begin(); i != maps_.end(); i++) {
            if (i->wsm == wsm) {
                LOG_D("WSMs::remove %p %d", wsm, i->len);
                maps_.erase(i);
                break;
            }
        }
    }
    int len(uint8_t* wsm) {
        std::lock_guard<std::mutex> lock(mutex_);
        int len = -1;
        for (auto i = maps_.begin(); i != maps_.end(); i++) {
            if (i->wsm == wsm) {
                len = i->len;
                LOG_D("WSMs::len %p %d", wsm, len);
                break;
            }
        }
        return len;
    }
    void flush() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!maps_.empty()) {
            Map wsm = maps_.back();
            LOG_D("WSMs::flush %p %d", wsm.wsm, wsm.len);
            common.mcFreeWsm(MC_DEVICE_ID_DEFAULT, wsm.wsm, wsm.len);
            maps_.pop_back();
        }
    }
};

static WSMs wsms;

/* Only 1 device is supported */
static inline bool isValidDevice(uint32_t deviceId) {
    return (MC_DEVICE_ID_DEFAULT == deviceId);
}

mcResult_t mcOpenDevice(
    uint32_t                deviceId) {
    ENTER();
    std::lock_guard<std::mutex> lock(open_close_lock);
    // deviceID is checked at lower levels so we can use non-default values
    mcResult_t mc_result = common.mcOpenDevice(deviceId);
    if (mc_result == MC_DRV_OK) {
        open_count++;
    }

    MC_EXIT(mc_result);
}

mcResult_t mcCloseDevice(
    uint32_t                deviceId) {
    ENTER();
    std::lock_guard<std::mutex> lock(open_close_lock);
    // Check parameters
    if (!isValidDevice(deviceId)) {
        LOG_E("invalid device ID %x", deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }

    auto mc_result = common.mcCloseDevice(deviceId);
    if (mc_result == MC_DRV_OK) {
        if (!--open_count) {
            // Free all remaining WSMs
            wsms.flush();
        }
    }
    MC_EXIT(mc_result);
}

mcResult_t mcOpenSession(
    mcSessionHandle_t*      session,
    const mcUuid_t*         uuid,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    // Check parameters
    if (!session) {
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!uuid) {
        LOG_E("uuid is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    LOG_D("tci %p len %u", tci, tciLen);
    if ((tci && !tciLen) || (!tci && tciLen)) {
        LOG_E("TCI and its length are inconsistent");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    // So we return the correct error code
    if (tciLen > MC_MAX_TCI_LEN) {
        LOG_E("TCI length too big: %u", tciLen);
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    MC_EXIT(common.mcOpenSession(session, uuid, tci, tciLen));
}

mcResult_t mcOpenTrustlet(
    mcSessionHandle_t*      session,
    mcSpid_t                spid,
    uint8_t*                trustedapp,
    uint32_t                tLen,
    uint8_t*                tci,
    uint32_t                tciLen) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    LOG_D("tci %p len %u", tci, tciLen);
    if ((tci && !tciLen) || (!tci && tciLen)) {
        LOG_E("TCI and its length are inconsistent");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    // So we return the correct error code
    if (tciLen > MC_MAX_TCI_LEN) {
        LOG_E("TCI length too big: %u", tciLen);
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    MC_EXIT(common.mcOpenTrustlet(session, spid, trustedapp, tLen, tci, tciLen));
}

mcResult_t mcCloseSession(
    mcSessionHandle_t*      session) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }

    MC_EXIT(common.mcCloseSession(session));
}

mcResult_t mcNotify(
    mcSessionHandle_t*      session) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }

    MC_EXIT(common.mcNotify(session));
}

mcResult_t mcWaitNotification(
    mcSessionHandle_t*      session,
    int32_t                 timeout) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }

    int mc_result;
    do {
        mc_result = common.mcWaitNotification(session, timeout);
    } while ((mc_result == MC_DRV_ERR_INTERRUPTED_BY_SIGNAL) &&
             (timeout == MC_INFINITE_TIMEOUT));

    MC_EXIT(mc_result);
}

mcResult_t mcMallocWsm(
    uint32_t                deviceId,
    uint32_t                /*align*/,
    uint32_t                len,
    uint8_t**               wsm,
    uint32_t                /*wsmFlags*/) {
    ENTER();
    // Check parameters
    if (!isValidDevice(deviceId)) {
        LOG_E("invalid device ID %x", deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!wsm) {
        LOG_E("wsm pointer is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    // Check length here to make sure we are consistent
    if ((len == 0) || (len > BUFFER_LENGTH_MAX)) {
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    auto mc_result = common.mcMallocWsm(deviceId, len, wsm);
    if (mc_result == MC_DRV_OK) {
        wsms.add(*wsm, len);
    }

    MC_EXIT(mc_result);
}

mcResult_t mcFreeWsm(
    uint32_t                deviceId,
    uint8_t*                wsm) {
    ENTER();
    // Check parameters
    if (!isValidDevice(deviceId)) {
        LOG_E("invalid device ID %x", deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!wsm) {
        LOG_E("wsm pointer is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    int len = wsms.len(wsm);
    if (len == -1) {
        LOG_E("wsm %p buffer not available", wsm);
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    auto mc_result = common.mcFreeWsm(deviceId, wsm, len);
    if (mc_result == MC_DRV_OK) {
        wsms.remove(wsm);
    }

    MC_EXIT(mc_result);
}

mcResult_t mcMap(
    mcSessionHandle_t*      session,
    void*                   buf,
    uint32_t                len,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!mapInfo) {
        LOG_E("mapInfo pointer is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (len > BUFFER_LENGTH_MAX) {
        LOG_E("Incorrect len: %u", len);
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (len == 0) {
        LOG_E("Incorrect len: %u", len);
        MC_EXIT(MC_DRV_ERR_INVALID_OPERATION);
    }

    MC_EXIT(common.mcMap(session, buf, len, mapInfo));
}

mcResult_t mcUnmap(
    mcSessionHandle_t*      session,
    void*                   buf,
    mcBulkMap_t*            mapInfo) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!buf) {
        LOG_E("buf pointer is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!mapInfo) {
        LOG_E("mapInfo pointer is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!mapInfo->sVirtualAddr) {
        LOG_E("mapInfo SVA is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    MC_EXIT(common.mcUnmap(session, buf, mapInfo));
}

mcResult_t mcGetSessionErrorCode(
    mcSessionHandle_t*      session,
    int32_t*                lastErr) {
    ENTER();
    // Check parameters
    if (!session) {
        LOG_E("session handle is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }
    if (!isValidDevice(session->deviceId)) {
        LOG_E("invalid device ID %x", session->deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!lastErr) {
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    MC_EXIT(common.mcGetSessionErrorCode(session, lastErr));
}

mcResult_t mcGetMobiCoreVersion(
    uint32_t                deviceId,
    mcVersionInfo_t*        versionInfo) {
    ENTER();
    // Check parameters
    if (!isValidDevice(deviceId)) {
        LOG_E("invalid device ID %x", deviceId);
        MC_EXIT(MC_DRV_ERR_UNKNOWN_DEVICE);
    }
    if (!versionInfo) {
        LOG_E("versionInfo pointer is null");
        MC_EXIT(MC_DRV_ERR_INVALID_PARAMETER);
    }

    MC_EXIT(common.mcGetMobiCoreVersion(deviceId, versionInfo));
}
