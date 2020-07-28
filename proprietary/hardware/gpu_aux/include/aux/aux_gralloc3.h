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

#ifndef AUX_INCLUDE_AUX_AUX_GRALLOC3_H_
#define AUX_INCLUDE_AUX_AUX_GRALLOC3_H_

#include <string>
#include <android/hardware/graphics/common/1.1/types.h>
#include <android/hardware/graphics/mapper/3.0/IMapper.h>
#include <aux/aux_gralloc.h>
#include <utils/StrongPointer.h>

using android::hardware::hidl_handle;
using android::status_t;
using android::sp;

class AuxGralloc3Mapper : public AuxGrallocMapper {
 public:
    static void preload();

    AuxGralloc3Mapper();

    bool isLoaded() const override;

    status_t createDescriptor(void* bufferDescriptorInfo, void* outBufferDescriptor) const override;

    status_t importBuffer(const hidl_handle& rawHandle, buffer_handle_t* outBufferHandle) const override;

    void freeBuffer(buffer_handle_t bufferHandle) const override;

    status_t validateBufferSize(buffer_handle_t bufferHandle, uint32_t width, uint32_t height,
                                PixelFormat format, uint32_t layerCount, uint64_t usage,
                                uint32_t stride) const override;

    void getTransportSize(buffer_handle_t bufferHandle, uint32_t* outNumFds, uint32_t* outNumInts) const override;

    status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                  int acquireFence, void** outData, int32_t* outBytesPerPixel,
                  int32_t* outBytesPerStride) const override;

    status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                  int acquireFence, android_ycbcr* ycbcr) const override;

    int unlock(buffer_handle_t bufferHandle) const override;

    status_t isSupported(uint32_t width, uint32_t height, PixelFormat format,
                         uint32_t layerCount, uint64_t usage, bool* outSupported) const override;

 private:
    // Determines whether the passed info is compatible with the mapper.
    status_t validateBufferDescriptorInfo(
            android::hardware::graphics::mapper::V3_0::IMapper::BufferDescriptorInfo* descriptorInfo) const;

    sp<android::hardware::graphics::mapper::V3_0::IMapper> mMapper;
};

#endif  // AUX_INCLUDE_AUX_AUX_GRALLOC3_H_
