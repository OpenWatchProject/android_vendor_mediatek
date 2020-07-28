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

#ifndef AUX_INCLUDE_AUX_AUX_MAPPER_H_
#define AUX_INCLUDE_AUX_AUX_MAPPER_H_

#include <stdint.h>
#include <sys/types.h>
#include <memory>
#include <aux/aux_gralloc.h>
#include <utils/Singleton.h>
#include <nativebase/nativebase.h>

using android::status_t;
using android::Singleton;

class AuxGrallocMapper;

class AuxBufferMapper : public Singleton<AuxBufferMapper> {
 public:
    enum Version {
        GRALLOC_2 = 2,
        GRALLOC_3,
    };
    static void preloadHal();
    static inline AuxBufferMapper& get() { return getInstance(); }

    // The imported outHandle must be freed with freeBuffer when no longer needed.
    // rawHandle is owned by the caller.
    status_t importBuffer(buffer_handle_t rawHandle, uint32_t width, uint32_t height, uint32_t layerCount,
                          PixelFormat format, uint64_t usage, uint32_t stride, buffer_handle_t* outHandle);

    status_t freeBuffer(buffer_handle_t handle);

    void getTransportSize(buffer_handle_t handle, uint32_t* outTransportNumFds, uint32_t* outTransportNumInts);

    // MTK added function
    status_t lockBuffer(const ANativeWindowBuffer *buffer, void **vaddr);

    // MTK added function
    status_t unlockBuffer(const ANativeWindowBuffer *buffer);

    status_t lock(buffer_handle_t handle, uint32_t usage, const Rect& bounds, void** vaddr,
                  int32_t* outBytesPerPixel = nullptr, int32_t* outBytesPerStride = nullptr);

    status_t lockYCbCr(buffer_handle_t handle, uint32_t usage, const Rect& bounds, android_ycbcr *ycbcr);

    status_t unlock(buffer_handle_t handle);

    status_t lockAsync(buffer_handle_t handle, uint32_t usage, const Rect& bounds, void** vaddr,
                       int fenceFd, int32_t* outBytesPerPixel = nullptr,
                       int32_t* outBytesPerStride = nullptr);

    status_t lockAsync(buffer_handle_t handle, uint64_t producerUsage, uint64_t consumerUsage,
                       const Rect& bounds, void** vaddr, int fenceFd,
                       int32_t* outBytesPerPixel = nullptr, int32_t* outBytesPerStride = nullptr);

    status_t lockAsyncYCbCr(buffer_handle_t handle, uint32_t usage,
                            const Rect& bounds, android_ycbcr *ycbcr, int fenceFd);

    status_t unlockAsync(buffer_handle_t handle, int *fenceFd);

    status_t isSupported(uint32_t width, uint32_t height, PixelFormat format,
                         uint32_t layerCount, uint64_t usage, bool* outSupported);

    const AuxGrallocMapper& getGrallocMapper() const {
        return reinterpret_cast<const AuxGrallocMapper&>(*mMapper);
    }

    Version getMapperVersion() const { return mMapperVersion; }

 private:
    friend class Singleton<AuxBufferMapper>;

    AuxBufferMapper();

    std::unique_ptr<const AuxGrallocMapper> mMapper;

    Version mMapperVersion;
};

#endif  // AUX_INCLUDE_AUX_AUX_MAPPER_H_

