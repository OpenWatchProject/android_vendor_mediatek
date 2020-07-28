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

#ifndef __TRUSTONIC_TEE_H__
#define __TRUSTONIC_TEE_H__

#include <memory>

#include "iclient.h"

namespace trustonic {

class TrustonicTEE: public IClient {
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;
public:
    TrustonicTEE();
    ~TrustonicTEE() override;

    // Proprietary
    void TEEC_TT_RegisterPlatformContext(
        void*                   globalContext,
        void*                   localContext) override;

    TEEC_Result TEEC_TT_TestEntry(
        void*                   buff,
        size_t                  len,
        uint32_t*               tag) override;

    // Global Platform
    TEEC_Result TEEC_InitializeContext(
        const char*             name,
        TEEC_Context*           context) override;

    void TEEC_FinalizeContext(
        TEEC_Context*           context) override;

    TEEC_Result TEEC_RegisterSharedMemory(
        TEEC_Context*           context,
        TEEC_SharedMemory*      sharedMem) override;

    TEEC_Result TEEC_AllocateSharedMemory(
        TEEC_Context*           context,
        TEEC_SharedMemory*      sharedMem) override;

    void TEEC_ReleaseSharedMemory(
        TEEC_SharedMemory*      sharedMem) override;

    TEEC_Result TEEC_OpenSession(
        TEEC_Context*           context,
        TEEC_Session*           session,
        const TEEC_UUID*        destination,
        uint32_t                connectionMethod,
        const void*             connectionData,
        TEEC_Operation*         operation,
        uint32_t*               returnOrigin) override;

    void TEEC_CloseSession(
        TEEC_Session*           session) override;

    TEEC_Result TEEC_InvokeCommand(
        TEEC_Session*           session,
        uint32_t                commandID,
        TEEC_Operation*         operation,
        uint32_t*               returnOrigin) override;

    void TEEC_RequestCancellation(
        TEEC_Operation*         operation) override;

    // MobiCore
    mcResult_t mcOpenDevice(
        uint32_t                deviceId) override;

    mcResult_t mcCloseDevice(
        uint32_t                deviceId) override;

    mcResult_t mcOpenSession(
        mcSessionHandle_t*      session,
        const mcUuid_t*         uuid,
        uint8_t*                tci,
        uint32_t                tciLen) override;

    mcResult_t mcOpenTrustlet(
        mcSessionHandle_t*      session,
        mcSpid_t                spid,
        uint8_t*                trustedapp,
        uint32_t                tLen,
        uint8_t*                tci,
        uint32_t                tciLen) override;

    mcResult_t mcCloseSession(
        mcSessionHandle_t*      session) override;

    mcResult_t mcNotify(
        mcSessionHandle_t*      session) override;

    mcResult_t mcWaitNotification(
        mcSessionHandle_t*      session,
        int32_t                 timeout) override;

    mcResult_t mcMallocWsm(
        uint32_t                deviceId,
        uint32_t                len,
        uint8_t**               wsm) override;

    mcResult_t mcFreeWsm(
        uint32_t                deviceId,
        uint8_t*                wsm,
        uint32_t                len) override;

    mcResult_t mcMap(
        mcSessionHandle_t*      session,
        void*                   buf,
        uint32_t                len,
        mcBulkMap_t*            mapInfo) override;

    mcResult_t mcUnmap(
        mcSessionHandle_t*      session,
        void*                   buf,
        mcBulkMap_t*            mapInfo) override;

    mcResult_t mcGetSessionErrorCode(
        mcSessionHandle_t*      session,
        int32_t*                lastErr) override;

    mcResult_t mcGetMobiCoreVersion(
        uint32_t                deviceId,
        mcVersionInfo_t*        versionInfo) override;
};

}

#endif // __TRUSTONIC_TEE_H__
