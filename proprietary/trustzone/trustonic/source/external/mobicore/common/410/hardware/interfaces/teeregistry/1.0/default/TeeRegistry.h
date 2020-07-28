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

#ifndef VENDOR_TRUSTONIC_TEEREGISTRY_V1_0_TEEREGISTRY_H
#define VENDOR_TRUSTONIC_TEEREGISTRY_V1_0_TEEREGISTRY_H

#include <string>

#include <vendor/trustonic/teeregistry/1.0/ITeeRegistry.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace trustonic {
namespace teeregistry {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct TeeRegistry : public ITeeRegistry {
    TeeRegistry(const std::string& registry_path);
    ~TeeRegistry() override;

    Return<uint32_t> mcRegistryStoreAuthToken(
            const ::android::hardware::hidl_memory& so) override;
    Return<uint32_t> mcRegistryReadAuthToken(
            const ::android::hardware::hidl_memory& so) override;
    Return<uint32_t> mcRegistryReadAuthTokenBackup(
            const ::android::hardware::hidl_memory& so) override;
    Return<uint32_t> mcRegistryDeleteAuthToken() override;
    Return<uint32_t> mcRegistryStoreRoot(
            const ::android::hardware::hidl_memory& so) override;
    Return<void> mcRegistryReadRoot(
            const ::android::hardware::hidl_memory& so,
            mcRegistryReadRoot_cb _hidl_cb) override;
    Return<uint32_t> mcRegistryStoreSp(
            uint32_t spid,
            const ::android::hardware::hidl_memory& so) override;
    Return<void> mcRegistryReadSp(
            uint32_t spid,
            const ::android::hardware::hidl_memory& so,
            mcRegistryReadSp_cb _hidl_cb) override;
    Return<uint32_t> mcRegistryCleanupSp(
            uint32_t spid) override;
    Return<uint32_t> mcRegistryStoreTrustletCon(
            const ::android::hardware::hidl_vec<uint8_t>& uuid,
            uint32_t spid,
            const ::android::hardware::hidl_memory& so) override;
    Return<void> mcRegistryReadTrustletCon(
            const ::android::hardware::hidl_vec<uint8_t>& uuid,
            uint32_t spid,
            const ::android::hardware::hidl_memory& so,
            mcRegistryReadTrustletCon_cb _hidl_cb) override;
    Return<uint32_t> mcRegistryCleanupTrustlet(
            const ::android::hardware::hidl_vec<uint8_t>& uuid,
            uint32_t spid) override;
    Return<uint32_t> mcRegistryCleanupTA(
            const ::android::hardware::hidl_vec<uint8_t>& uuid) override;
    Return<uint32_t> mcRegistryCleanupRoot() override;
    Return<uint32_t> mcRegistryStoreTABlob(
            uint32_t spid,
            const ::android::hardware::hidl_memory& blob) override;
private:
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace teeregistry
}  // namespace trustonic
}  // namespace vendor

#endif  // VENDOR_TRUSTONIC_TEEREGISTRY_V1_0_TEEREGISTRY_H
