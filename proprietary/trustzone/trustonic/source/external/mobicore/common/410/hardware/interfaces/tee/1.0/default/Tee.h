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

#ifndef VENDOR_TRUSTONIC_TEECLIENT_V1_0_TEECLIENT_H
#define VENDOR_TRUSTONIC_TEECLIENT_V1_0_TEECLIENT_H

#include <vendor/trustonic/tee/1.0/ITee.h>
#include <vendor/trustonic/tee/1.0/ITeeCallback.h>
#include <vendor/trustonic/tee/tui/1.0/ITui.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "TrustonicService.h"

namespace vendor {
namespace trustonic {
namespace tee {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Tee : public ITee {
    Tee(std::shared_ptr<TrustonicService> service_);
    ~Tee() override;

    Return<void> flushAll() override;
    Return<void> flushPid(
            int32_t client_pid) override;

    Return<uint32_t> TEEC_InitializeContext(
            int32_t client_pid,
            uint64_t context,
            const ::android::hardware::hidl_string& name,
            bool has_name) override;
    Return<void> TEEC_FinalizeContext(
            int32_t client_pid,
            uint64_t context) override;
    Return<uint32_t> TEEC_RegisterSharedMemory(
            int32_t client_pid,
            uint64_t context,
            const ::vendor::trustonic::tee::V1_0::ITee::GpSharedMemory& shr_mem) override;
    Return<void> TEEC_ReleaseSharedMemory(
            int32_t client_pid,
            uint64_t context,
            const ::vendor::trustonic::tee::V1_0::ITee::GpSharedMemory& shr_mem) override;
    Return<void> TEEC_OpenSession(
            int32_t client_pid,
            uint64_t context,
            uint64_t session,
            const hidl_vec<uint8_t>& uuid,
            ::vendor::trustonic::tee::V1_0::ITee::LoginType login_type,
            const hidl_vec<uint8_t>& login_data,
            const ::vendor::trustonic::tee::V1_0::ITee::Operation& operation,
            TEEC_OpenSession_cb _hidl_cb) override;
    Return<void> TEEC_CloseSession(
            int32_t client_pid,
            uint64_t context,
            uint64_t session) override;
    Return<void> TEEC_InvokeCommand(
            int32_t client_pid,
            uint64_t context,
            uint64_t session,
            uint32_t cmd_id,
            const ::vendor::trustonic::tee::V1_0::ITee::Operation& operation,
            TEEC_InvokeCommand_cb _hidl_cb) override;
    Return<void> TEEC_RequestCancellation(
            int32_t client_pid,
            uint64_t context,
            uint64_t session,
            uint64_t operation) override;

    Return<uint32_t> mcOpenDevice(
            int32_t client_pid) override;
    Return<uint32_t> mcCloseDevice(
            int32_t client_pid) override;
    Return<void> mcOpenSession(
            int32_t client_pid,
            const hidl_vec<uint8_t>& uuid,
            const ::android::hardware::hidl_memory& tci,
            uint32_t tci_len,
            mcOpenSession_cb _hidl_cb) override;
    Return<void> mcOpenTrustlet(
            int32_t client_pid,
            uint32_t spid,
            const ::android::hardware::hidl_memory& ta,
            uint32_t ta_len,
            const ::android::hardware::hidl_memory& tci,
            uint32_t tci_len,
            mcOpenTrustlet_cb _hidl_cb) override;
    Return<uint32_t> mcCloseSession(
            int32_t client_pid,
            uint32_t id) override;
    Return<uint32_t> mcNotify(
            int32_t client_pid,
            uint32_t id) override;
    Return<uint32_t> mcWaitNotification(
            int32_t client_pid,
            uint32_t id,
            int32_t timeout,
            bool partial) override;
    Return<void> mcMap(
            int32_t client_pid,
            uint32_t id,
            const ::android::hardware::hidl_memory& buf,
            uint32_t buf_len,
            mcMap_cb _hidl_cb) override;
    Return<uint32_t> mcUnmap(
            int32_t client_pid,
            uint32_t id,
            uint64_t reference) override;
    Return<void> mcGetSessionErrorCode(
            int32_t client_pid,
            uint32_t id,
            mcGetSessionErrorCode_cb _hidl_cb) override;
    Return<void> mcGetMobiCoreVersion(
            int32_t client_pid,
            mcGetMobiCoreVersion_cb _hidl_cb) override;

    Return<void> registerTeeCallback(
            const ::android::sp<::vendor::trustonic::tee::V1_0::ITeeCallback>& callback);

private:
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor

#endif  // VENDOR_TRUSTONIC_TEECLIENT_V1_0_TEECLIENT_H
