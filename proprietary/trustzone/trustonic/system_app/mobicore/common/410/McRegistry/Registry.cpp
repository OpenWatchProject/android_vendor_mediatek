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
/** Mobicore Driver Registry Interface
 *
 * Implements the MobiCore registry interface for the ROOT-PA
 *
 * @file
 * @ingroup MCD_MCDIMPL_DAEMON_REG
 */

#define LOG_TAG "LibMcRegistry"
//#define LOG_NDEBUG 0

//#define TEESERVICE_DEBUG
//#define TEESERVICE_MEMLEAK_CHECK

#include <stdint.h>
#include <string.h>
#include <sys/uio.h>
#include <fcntl.h>

#include <utils/Log.h>

#include "mcSpid.h"
#include "MobiCoreRegistry.h"

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>
#include "vendor/trustonic/teeregistry/1.0/ITeeRegistry.h"

#define MAX_DATA_SIZE (512)

#undef ALOGH
#ifdef TEESERVICE_DEBUG
#define ALOGH ALOGI
#else
#define ALOGH(...) do {} while(0)
#endif

#undef ALOGM
#ifdef TEESERVICE_MEMLEAK_CHECK
#include <malloc.h>
#define ALOGM(fmt, ...) \
    ALOGI(fmt " (MEMLEAK_CHECK %zu) [%s:%d:%s]", ##__VA_ARGS__, \
          static_cast<size_t>(mallinfo().uordblks), \
          __FILE__, __LINE__, __func__)
#else
#define ALOGM(...) do {} while(0)
#endif

using namespace vendor::trustonic::teeregistry::V1_0;

class AllocatorWrapper {
    std::mutex allocator_mutex_;
    ::android::sp<::android::hidl::allocator::V1_0::IAllocator> allocator_;
    AllocatorWrapper() {}
public:
    static AllocatorWrapper& getInstance() {
        static AllocatorWrapper instance;
        return instance;
    }

    ::android::sp<::android::hidl::allocator::V1_0::IAllocator> get() {
        std::lock_guard<std::mutex> lock(allocator_mutex_);
        if (!allocator_) {
            allocator_ = ::android::hidl::allocator::V1_0::IAllocator::getService("ashmem");
            if (!allocator_) {
                ALOGE("Failed to create the allocator");
                return nullptr;
            }
        }

        return allocator_;
    }
};

class Buffer {
    ::android::hardware::hidl_memory hidl_memory_;
    ::android::sp<::android::hidl::memory::V1_0::IMemory> memory_;
    void* data_ = nullptr;
    uint32_t size_ = 0;
public:
    Buffer(
            uint32_t size):
        size_(size) {}

    bool create() {
        // Allocate
        auto allocator = AllocatorWrapper::getInstance().get();
        if (!allocator) {
            ALOGE("Failed to get allocator.");
            return false;
        }

        bool success = false;
        auto result = allocator->allocate(
                          size_, [&](bool s, const ::android::hardware::hidl_memory& m) {
            success = s;
            if (s) {
                hidl_memory_ = m;
            }
        });
        if (!success) {
            ALOGE("Failed to allocate buffer.");
            return false;
        }

        // Map
        memory_ = ::android::hardware::mapMemory(hidl_memory_);
        data_ = memory_->getPointer();
        return true;
    }

    auto mem() {
        memory_->commit();
        return hidl_memory_;
    }

    void* address() {
        memory_->update();
        return data_;
    }

    uint32_t size() const {
        return size_;
    }
};

struct Receiver {
    uint32_t result;
    uint32_t size;

    void callback(uint32_t result_in, uint32_t size_in) {
        result = result_in;
        size = size_in;
        ALOGH("%s: result=%u size=%u", __func__, result, size);
    }
};

mcResult_t mcRegistryStoreAuthToken(
        void* so,
        uint32_t size) {
    ALOGH("%s called with size %u", __func__, size);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(size);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::memcpy(buffer.address(), so, size);
    return service->mcRegistryStoreAuthToken(buffer.mem());
}

mcResult_t mcRegistryReadAuthToken(
        void* so,
        uint32_t* size) {
    ALOGH("%s called", __func__);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    *size = sizeof(mcSoAuthTokenCont_t);
    Buffer buffer(*size);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    auto result = service->mcRegistryReadAuthToken(buffer.mem());
    *size = buffer.size();
    ::memcpy(so, buffer.address(), *size);
    return result;
}

mcResult_t mcRegistryDeleteAuthToken(void) {
    ALOGH("%s called", __func__);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    return service->mcRegistryDeleteAuthToken();
}

mcResult_t mcRegistryStoreRoot(
        void* so,
        uint32_t size) {
    ALOGH("%s called with size %u", __func__, size);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(size);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::memcpy(buffer.address(), so, size);
    return service->mcRegistryStoreRoot(buffer.mem());
}

mcResult_t mcRegistryReadRoot(
        void* so,
        uint32_t* size) {
    ALOGH("%s called", __func__);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(MAX_DATA_SIZE);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    Receiver receiver;
    service->mcRegistryReadRoot(
                buffer.mem(),
                std::bind(&Receiver::callback, &receiver,
                          std::placeholders::_1, std::placeholders::_2));
    if (receiver.result != MC_DRV_OK) {
        ALOGE("%s: result %x", __func__, receiver.result);
        return receiver.result;
    }

    *size = receiver.size;
    ::memcpy(so, buffer.address(), *size);
    return MC_DRV_OK;
}

mcResult_t mcRegistryCleanupRoot(void) {
    ALOGH("%s called", __func__);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    return service->mcRegistryCleanupRoot();
}

mcResult_t mcRegistryStoreSp(
        mcSpid_t spid,
        void* so,
        uint32_t size) {
    ALOGH("%s called with size %u", __func__, size);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(size);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::memcpy(buffer.address(), so, size);
    return service->mcRegistryStoreSp(spid, buffer.mem());
}

mcResult_t mcRegistryReadSp(
        mcSpid_t spid,
        void* so,
        uint32_t* size) {
    ALOGH("%s called with spid %u", __func__, spid);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(MAX_DATA_SIZE);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    Receiver receiver;
    service->mcRegistryReadSp(
                spid, buffer.mem(),
                std::bind(&Receiver::callback, &receiver,
                          std::placeholders::_1, std::placeholders::_2));
    if (receiver.result != MC_DRV_OK) {
        ALOGE("%s: result %x", __func__, receiver.result);
        return receiver.result;
    }

    *size = receiver.size;
    ::memcpy(so, buffer.address(), *size);
    return MC_DRV_OK;
}

mcResult_t mcRegistryCleanupSp(
        mcSpid_t spid) {
    ALOGH("%s called", __func__);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    return service->mcRegistryCleanupSp(spid);
}

mcResult_t mcRegistryStoreTrustletCon(
        const mcUuid_t* uuid,
        mcSpid_t spid,
        void* so,
        uint32_t size) {
    ALOGH("%s called with spid %u size %u", __func__, spid, size);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(size);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::memcpy(buffer.address(), so, size);
    ::android::hardware::hidl_vec<uint8_t> uuid_hidl;
    uuid_hidl.resize(sizeof(mcUuid_t));
    ::memcpy(&uuid_hidl[0], uuid, sizeof(mcUuid_t));
    return service->mcRegistryStoreTrustletCon(uuid_hidl, spid, buffer.mem());
}

mcResult_t mcRegistryStoreTABlob(
        mcSpid_t spid,
        void* blob,
        uint32_t size) {
    ALOGH("%s called with spid %u size %u", __func__, spid, size);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(size);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::memcpy(buffer.address(), blob, size);
    return service->mcRegistryStoreTABlob(spid, buffer.mem());
}

mcResult_t mcRegistryReadTrustletCon(
        const mcUuid_t* uuid,
        mcSpid_t spid,
        void* so,
        uint32_t* size) {
    ALOGH("%s called with spid %u", __func__, spid);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    Buffer buffer(MAX_DATA_SIZE);
    if (!buffer.create()) {
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    ::android::hardware::hidl_vec<uint8_t> uuid_hidl;
    uuid_hidl.resize(sizeof(mcUuid_t));
    ::memcpy(&uuid_hidl[0], uuid, sizeof(mcUuid_t));
    Receiver receiver;
    service->mcRegistryReadTrustletCon(
                uuid_hidl, spid, buffer.mem(),
                std::bind(&Receiver::callback, &receiver,
                          std::placeholders::_1, std::placeholders::_2));
    if (receiver.result != MC_DRV_OK) {
        ALOGE("%s: result %x", __func__, receiver.result);
        return receiver.result;
    }

    *size = receiver.size;
    ::memcpy(so, buffer.address(), *size);
    return MC_DRV_OK;
}

mcResult_t mcRegistryCleanupTrustlet(
        const mcUuid_t* uuid,
        const mcSpid_t spid) {
    ALOGH("%s called with spid %u", __func__, spid);
    if (!uuid) {
        ALOGE("%s: null uuid", __func__);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ::android::hardware::hidl_vec<uint8_t> uuid_hidl;
    uuid_hidl.resize(sizeof(mcUuid_t));
    ::memcpy(&uuid_hidl[0], uuid, sizeof(mcUuid_t));
    return service->mcRegistryCleanupTrustlet(uuid_hidl, spid);
}

mcResult_t mcRegistryCleanupTA(
        const mcUuid_t* uuid) {
    ALOGH("%s called", __func__);
    auto service = ITeeRegistry::getService();
    if (!service) {
        ALOGE("%s: failed to get service", __func__);
        return MC_DRV_ERR_DAEMON_UNREACHABLE;
    }

    ::android::hardware::hidl_vec<uint8_t> uuid_hidl;
    uuid_hidl.resize(sizeof(mcUuid_t));
    ::memcpy(&uuid_hidl[0], uuid, sizeof(mcUuid_t));
    return service->mcRegistryCleanupTA(uuid_hidl);
}

mcResult_t mcRegistryStoreData(
        void*, uint32_t) {
    return MC_DRV_ERR_INVALID_PARAMETER;
}

mcResult_t mcRegistryReadData(
        uint32_t,
        const mcCid_t*,
        mcPid_t,
        mcSoDataCont_t*,
        uint32_t) {
    return MC_DRV_ERR_INVALID_PARAMETER;
}
