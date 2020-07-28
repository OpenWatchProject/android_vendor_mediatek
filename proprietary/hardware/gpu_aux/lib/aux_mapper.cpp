/*
 * Copyright (C) 2018-2019 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <grallocusage/GrallocUsageConversion.h>
#include <sync/sync.h>
#include <utils/Log.h>
#include <aux/aux_mapper.h>
#include <aux/aux_gralloc.h>
#include <aux/aux_gralloc2.h>
#include <aux/aux_gralloc3.h>

using android::hardware::hidl_handle;
using android::status_t;

using android::hardware::graphics::common::V1_0::BufferUsage;

ANDROID_SINGLETON_STATIC_INSTANCE(AuxBufferMapper)

void AuxBufferMapper::preloadHal() {
    AuxGralloc2Mapper::preload();
    AuxGralloc3Mapper::preload();
}

AuxBufferMapper::AuxBufferMapper() {
    mMapper = std::make_unique<const AuxGralloc3Mapper>();
    if (!mMapper->isLoaded()) {
        mMapper = std::make_unique<const AuxGralloc2Mapper>();
        mMapperVersion = Version::GRALLOC_2;
    } else {
        mMapperVersion = Version::GRALLOC_3;
    }
    if (!mMapper->isLoaded()) {
        ALOGE("@%s: gralloc-mapper is missing", __func__);
    } else {
        ALOGI("@%s: gralloc-mapper's version is %d.0", __func__, mMapperVersion);
    }
}

status_t AuxBufferMapper::importBuffer(buffer_handle_t rawHandle,
        uint32_t width, uint32_t height, uint32_t layerCount,
        PixelFormat format, uint64_t usage, uint32_t stride,
        buffer_handle_t* outHandle) {
    buffer_handle_t bufferHandle;
    status_t error = mMapper->importBuffer(hidl_handle(rawHandle), &bufferHandle);
    if (error != android::NO_ERROR) {
        ALOGW("@%s: importBuffer(%p) failed: %d", __func__, rawHandle, error);
        return error;
    }

    error = mMapper->validateBufferSize(bufferHandle, width, height, format, layerCount, usage,
                                        stride);
    if (error != android::NO_ERROR) {
        ALOGE("@%s: validateBufferSize(%p) failed: %d", __func__, rawHandle, error);
        freeBuffer(bufferHandle);
        return static_cast<status_t>(error);
    }

    *outHandle = bufferHandle;

    return android::NO_ERROR;
}

void AuxBufferMapper::getTransportSize(buffer_handle_t handle,
            uint32_t* outTransportNumFds, uint32_t* outTransportNumInts) {
    mMapper->getTransportSize(handle, outTransportNumFds, outTransportNumInts);
}

status_t AuxBufferMapper::freeBuffer(buffer_handle_t handle) {
    mMapper->freeBuffer(handle);

    return android::NO_ERROR;
}

// MTK added function
status_t AuxBufferMapper::lockBuffer(const ANativeWindowBuffer *buffer, void **vaddr) {
    status_t error;
    Rect bounds(0, 0, buffer->width, buffer->height);
    uint64_t usage = static_cast<uint64_t>(BufferUsage::CPU_READ_OFTEN);
    if (buffer->format == HAL_PIXEL_FORMAT_YCbCr_420_888) {
        struct android_ycbcr ycbcr = {};
        error = mMapper->lock(buffer->handle, usage, bounds, -1, &ycbcr);
        if (ycbcr.y != nullptr) {
            /*
             * This is only valid because we know that MTK's
             * YCbCr_420_888 is really contiguous NV21/YV12 under the hood.
             */
            *vaddr = static_cast<void*>(ycbcr.y);
        }
        ALOGI("@%s: format is HAL_PIXEL_FORMAT_YCbCr_420_888", __func__);
    } else {
        error = mMapper->lock(buffer->handle, usage, bounds, -1, vaddr, nullptr, nullptr);
    }
    return error;
}

// MTK added function
status_t AuxBufferMapper::unlockBuffer(const ANativeWindowBuffer *buffer) {
    return mMapper->unlock(buffer->handle);
}

status_t AuxBufferMapper::lock(buffer_handle_t handle, uint32_t usage, const Rect& bounds,
                                   void** vaddr, int32_t* outBytesPerPixel,
                                   int32_t* outBytesPerStride) {
    return lockAsync(handle, usage, bounds, vaddr, -1, outBytesPerPixel, outBytesPerStride);
}

status_t AuxBufferMapper::lockYCbCr(buffer_handle_t handle, uint32_t usage,
        const Rect& bounds, android_ycbcr *ycbcr) {
    return lockAsyncYCbCr(handle, usage, bounds, ycbcr, -1);
}

status_t AuxBufferMapper::unlock(buffer_handle_t handle) {
    int32_t fenceFd = -1;
    status_t error = unlockAsync(handle, &fenceFd);
    if (error == android::NO_ERROR && fenceFd >= 0) {
        sync_wait(fenceFd, -1);
        close(fenceFd);
    }
    return error;
}

status_t AuxBufferMapper::lockAsync(buffer_handle_t handle, uint32_t usage, const Rect& bounds,
                                        void** vaddr, int fenceFd, int32_t* outBytesPerPixel,
                                        int32_t* outBytesPerStride) {
    return lockAsync(handle, usage, usage, bounds, vaddr, fenceFd, outBytesPerPixel,
                     outBytesPerStride);
}

status_t AuxBufferMapper::lockAsync(buffer_handle_t handle, uint64_t producerUsage,
                                        uint64_t consumerUsage, const Rect& bounds, void** vaddr,
                                        int fenceFd, int32_t* outBytesPerPixel,
                                        int32_t* outBytesPerStride) {
    const uint64_t usage = static_cast<uint64_t>(
            android_convertGralloc1To0Usage(producerUsage, consumerUsage));
    return mMapper->lock(handle, usage, bounds, fenceFd, vaddr, outBytesPerPixel,
                         outBytesPerStride);
}

status_t AuxBufferMapper::lockAsyncYCbCr(buffer_handle_t handle,
        uint32_t usage, const Rect& bounds, android_ycbcr *ycbcr, int fenceFd) {
    return mMapper->lock(handle, usage, bounds, fenceFd, ycbcr);
}

status_t AuxBufferMapper::unlockAsync(buffer_handle_t handle, int *fenceFd) {
    *fenceFd = mMapper->unlock(handle);

    return android::NO_ERROR;
}

status_t AuxBufferMapper::isSupported(uint32_t width, uint32_t height,
                                          PixelFormat format, uint32_t layerCount,
                                          uint64_t usage, bool* outSupported) {
    return mMapper->isSupported(width, height, format, layerCount, usage, outSupported);
}
