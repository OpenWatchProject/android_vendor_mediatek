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

#ifndef GPUD_INCLUDE_GPUD_GPUD_GRALLOC2_H_
#define GPUD_INCLUDE_GPUD_GPUD_GRALLOC2_H_

#include <android/hardware/graphics/common/1.1/types.h>
#include <android/hardware/graphics/mapper/2.0/IMapper.h>
#include <android/hardware/graphics/mapper/2.1/IMapper.h>
#include <gpud/gpud_gralloc_mapper.h>

using namespace android;

class GPUDGralloc2Mapper : public GPUDGrallocMapper {
 public:
    GPUDGralloc2Mapper();
    bool isLoaded() const override;
    status_t lock(buffer_handle_t bufferHandle, uint64_t usage, int32_t left, int32_t top,
                  int32_t width, int32_t height, int acquireFence, void** outData,
                  int32_t* outBytesPerPixel, int32_t* outBytesPerStride) const override;
    status_t lock(buffer_handle_t bufferHandle, uint64_t usage, int32_t left, int32_t top,
                  int32_t width, int32_t height, int acquireFence, android_ycbcr* ycbcr) const override;
    int unlock(buffer_handle_t bufferHandle) const override;
 private:
    sp<hardware::graphics::mapper::V2_0::IMapper> mMapper;
    sp<hardware::graphics::mapper::V2_1::IMapper> mMapperV2_1;
};

#endif  // GPUD_INCLUDE_GPUD_GPUD_GRALLOC2_H_
