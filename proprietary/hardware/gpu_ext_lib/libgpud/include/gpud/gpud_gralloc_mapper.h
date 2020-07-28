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

#ifndef GPUD_INCLUDE_GPUD_GPUD_GRALLOC_MAPPER_H_
#define GPUD_INCLUDE_GPUD_GPUD_GRALLOC_MAPPER_H_

#include <memory>
#include <utils/Singleton.h>
#include <system/graphics.h>
#include <nativebase/nativebase.h>

using namespace android;

class GPUDGrallocMapper {
 public:
    virtual ~GPUDGrallocMapper();
    virtual bool isLoaded() const = 0;
    // The ownership of acquireFence is always transferred to the callee, even on errors.
    virtual status_t lock(buffer_handle_t bufferHandle, uint64_t usage, int32_t left, int32_t top,
                          int32_t width, int32_t height, int acquireFence, void** outData,
                          int32_t* outBytesPerPixel, int32_t* outBytesPerStride) const = 0;
    // The ownership of acquireFence is always transferred to the callee, even on errors.
    virtual status_t lock(buffer_handle_t bufferHandle, uint64_t usage, int32_t left, int32_t top,
                          int32_t width, int32_t height, int acquireFence, android_ycbcr* ycbcr) const = 0;
    // unlock returns a fence sync object (or -1) and the fence sync object is owned by the caller
    virtual int unlock(buffer_handle_t bufferHandle) const = 0;
};

class GPUDBufferMapper : public Singleton<GPUDBufferMapper> {
 public:
    enum Version {
        GRALLOC_2 = 2,
        GRALLOC_3,
    };
    static inline GPUDBufferMapper& get() { return getInstance(); }
    status_t lockBuffer(const ANativeWindowBuffer *buffer, void **vaddr);
    status_t unlockBuffer(const ANativeWindowBuffer *buffer);
    const GPUDGrallocMapper& getGrallocMapper() const {
        return reinterpret_cast<const GPUDGrallocMapper&>(*mMapper);
    }
    Version getMapperVersion() const { return mMapperVersion; }
 private:
    friend class Singleton<GPUDBufferMapper>;
    GPUDBufferMapper();
    std::unique_ptr<const GPUDGrallocMapper> mMapper;
    Version mMapperVersion;
};

#endif  // GPUD_INCLUDE_GPUD_GPUD_GRALLOC_MAPPER_H_
