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

#ifndef __TEE_CLIENT_INTERFACE_H__
#define __TEE_CLIENT_INTERFACE_H__

#include <memory>
#include <string>
#include <tuple>

#include "MobiCoreDriverApi.h"
#include "tee_client_api.h"

// Debug macros
#define ENTER() \
    do { \
        LOG_D("entering %s", __func__); \
    } while (0)

#define EXIT_NORETURN() \
    do { \
        LOG_D("exiting %s", __func__); \
        return; \
    } while (0)

#define EXIT(a__) \
    do { \
        auto rc__ = (a__); \
        LOG_D("exiting %s rc 0x%x", __func__, rc__); \
        return rc__; \
    } while (0)

namespace trustonic {

class IClient {
public:
    virtual ~IClient() {}

    // Proprietary
    virtual void TEEC_TT_RegisterPlatformContext(
        void*                   globalContext,
        void*                   localContext) = 0;

    virtual TEEC_Result TEEC_TT_TestEntry(
        void*                   buff,
        size_t                  len,
        uint32_t*               tag) = 0;

    // Global Platform
    virtual TEEC_Result TEEC_InitializeContext(
        const char*             name,
        TEEC_Context*           context) = 0;

    virtual void TEEC_FinalizeContext(
        TEEC_Context*           context) = 0;

    virtual TEEC_Result TEEC_RegisterSharedMemory(
        TEEC_Context*           context,
        TEEC_SharedMemory*      sharedMem) = 0;

    virtual TEEC_Result TEEC_AllocateSharedMemory(
        TEEC_Context*           context,
        TEEC_SharedMemory*      sharedMem) = 0;

    virtual void TEEC_ReleaseSharedMemory(
        TEEC_SharedMemory*      sharedMem) = 0;

    virtual TEEC_Result TEEC_OpenSession(
        TEEC_Context*           context,
        TEEC_Session*           session,
        const TEEC_UUID*        destination,
        uint32_t                connectionMethod,
        const void*             connectionData,
        TEEC_Operation*         operation,
        uint32_t*               returnOrigin) = 0;

    virtual void TEEC_CloseSession(
        TEEC_Session*           session) = 0;

    virtual TEEC_Result TEEC_InvokeCommand(
        TEEC_Session*           session,
        uint32_t                commandID,
        TEEC_Operation*         operation,
        uint32_t*               returnOrigin) = 0;

    virtual void TEEC_RequestCancellation(
        TEEC_Operation*         operation) = 0;

    // MobiCore
    virtual mcResult_t mcOpenDevice(
        uint32_t                deviceId) = 0;

    virtual mcResult_t mcCloseDevice(
        uint32_t                deviceId) = 0;

    virtual mcResult_t mcOpenSession(
        mcSessionHandle_t*      session,
        const mcUuid_t*         uuid,
        uint8_t*                tci,
        uint32_t                tciLen) = 0;

    virtual mcResult_t mcOpenTrustlet(
        mcSessionHandle_t*      session,
        mcSpid_t                spid,
        uint8_t*                trustedapp,
        uint32_t                tLen,
        uint8_t*                tci,
        uint32_t                tciLen) = 0;

    virtual mcResult_t mcCloseSession(
        mcSessionHandle_t*      session) = 0;

    virtual mcResult_t mcNotify(
        mcSessionHandle_t*      session) = 0;

    virtual mcResult_t mcWaitNotification(
        mcSessionHandle_t*      session,
        int32_t                 timeout) = 0;

    virtual mcResult_t mcMallocWsm(
        uint32_t                deviceId,
        uint32_t                len,
        uint8_t**               wsm) = 0;

    virtual mcResult_t mcFreeWsm(
        uint32_t                deviceId,
        uint8_t*                wsm,
        uint32_t                len) = 0;

    virtual mcResult_t mcMap(
        mcSessionHandle_t*      session,
        void*                   buf,
        uint32_t                len,
        mcBulkMap_t*            mapInfo) = 0;

    virtual mcResult_t mcUnmap(
        mcSessionHandle_t*      session,
        void*                   buf,
        mcBulkMap_t*            mapInfo) = 0;

    virtual mcResult_t mcGetSessionErrorCode(
        mcSessionHandle_t*      session,
        int32_t*                lastErr) = 0;

    virtual mcResult_t mcGetMobiCoreVersion(
        uint32_t                deviceId,
        mcVersionInfo_t*        versionInfo) = 0;

    // Specific
    struct BindInformation {
        char package_name[128];
        char starter_class[32];
    };

    virtual bool getBindInformation(BindInformation* /*bind_information*/) {
        return false;
    }
};

}

#endif // __TEE_CLIENT_INTERFACE_H__
