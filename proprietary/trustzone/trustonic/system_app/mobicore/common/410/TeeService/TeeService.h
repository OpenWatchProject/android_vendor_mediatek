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

#ifndef __TEE_FWK_SERVICE_H__
#define __TEE_FWK_SERVICE_H__

#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include "vendor/trustonic/teeservice/BnTeeService.h"

namespace vendor {

namespace trustonic {

namespace teeservice {

class TeeService: public BnTeeService {
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;

    ::android::binder::Status registerClient(
            int32_t client_version,
            const ::android::sp<::vendor::trustonic::teeservice::ITeeServiceListener>& client,
            int32_t* _aidl_return) override;

    ::android::binder::Status TEEC_InitializeContext(
            const ::vendor::trustonic::teeservice::TeeServiceGpContextIn& params_in,
            int32_t* _aidl_return) override;

    ::android::binder::Status TEEC_FinalizeContext(
            const ::vendor::trustonic::teeservice::TeeServiceGpContextIn& params_in) override;

    ::android::binder::Status TEEC_RegisterSharedMemory(
            const ::vendor::trustonic::teeservice::TeeServiceGpSharedMemoryIn& params_in,
            int32_t* _aidl_return) override;

    ::android::binder::Status TEEC_ReleaseSharedMemory(
            const ::vendor::trustonic::teeservice::TeeServiceGpSharedMemoryIn& params_in) override;

    ::android::binder::Status TEEC_OpenSession(
            const ::vendor::trustonic::teeservice::TeeServiceGpOpenSessionIn& params_in,
            ::vendor::trustonic::teeservice::TeeServiceGpOpenSessionOut* params_out,
            int32_t* _aidl_return) override;

    ::android::binder::Status TEEC_CloseSession(
            const ::vendor::trustonic::teeservice::TeeServiceGpCloseSessionIn& params_in) override;

    ::android::binder::Status TEEC_InvokeCommand(
            const ::vendor::trustonic::teeservice::TeeServiceGpInvokeCommandIn& params_in,
            ::vendor::trustonic::teeservice::TeeServiceGpInvokeCommandOut* params_out,
            int32_t* _aidl_return) override;

    ::android::binder::Status TEEC_RequestCancellation(
            const ::vendor::trustonic::teeservice::TeeServiceGpRequestCancellationIn& params_in) override;

    ::android::binder::Status mcOpenDevice(
            int32_t* _aidl_return) override;

    ::android::binder::Status mcCloseDevice(
            int32_t* _aidl_return) override;

    ::android::binder::Status mcOpenSession(
            const ::vendor::trustonic::teeservice::TeeServiceMcOpenSessionIn& params_in,
            ::vendor::trustonic::teeservice::TeeServiceMcOpenSessionOut* params_out,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcOpenTrustlet(
            const ::vendor::trustonic::teeservice::TeeServiceMcOpenTrustletIn& params_in, ::vendor::trustonic::teeservice::TeeServiceMcOpenSessionOut* params_out,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcCloseSession(
            int32_t id,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcNotify(
            int32_t id,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcWaitNotification(
            int32_t id,
            int32_t timeout,
            bool partial,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcMap(
            const ::vendor::trustonic::teeservice::TeeServiceMcMapIn& params_in,
            ::vendor::trustonic::teeservice::TeeServiceMcMapOut* params_out,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcUnmap(
            const ::vendor::trustonic::teeservice::TeeServiceMcUnmapIn& params_in,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcGetSessionErrorCode(
            int32_t id,
            ::vendor::trustonic::teeservice::TeeServiceMcGetSessionErrorCodeOut* params_out,
            int32_t* _aidl_return) override;

    ::android::binder::Status mcGetMobiCoreVersion(
            ::vendor::trustonic::teeservice::TeeServiceMcGetMobiCoreVersionOut* params_out,
            int32_t* _aidl_return) override;
public:
    static void instantiate(void* java_env, void* java_service_object);

    TeeService();
    TeeService(void* java_env, void* java_service_object);
    ~TeeService() override;
};

}

}

}

#endif // __TEE_FWK_SERVICE_H__

