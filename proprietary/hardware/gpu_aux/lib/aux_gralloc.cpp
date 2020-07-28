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

#include <log/log.h>
#include <utils/Singleton.h>
#include <ui/gralloc_extra.h>
#include <ui/GraphicBuffer.h>
#include <vndk/hardware_buffer.h>
#include <aux/aux_gralloc.h>
#include <android/hardware/graphics/common/1.0/types.h>

using android::status_t;

using android::hardware::graphics::common::V1_0::BufferUsage;

namespace {

static uint32_t AHWBuffer_convertFromPixelFormat(uint32_t fmt) {
    switch (fmt) {
    case HAL_PIXEL_FORMAT_YV12:
        return AHARDWAREBUFFER_FORMAT_YV12;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        return AHARDWAREBUFFER_FORMAT_YCrCb_420_SP;
    case HAL_PIXEL_FORMAT_RGBA_8888:
        return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    case HAL_PIXEL_FORMAT_RGB_888:
        return AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
    default:
        ALOGW("@%s: non-support gralloc fmt(0x%x)", __func__, fmt);
        break;
    }
    return AHARDWAREBUFFER_FORMAT_YV12;
}

}  // anonymous namespace

// --------------------------------------------------------------------------------------------------------------------

AuxGrallocMapper::~AuxGrallocMapper() {}

// --------------------------------------------------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(AuxGralloc)

AuxGralloc::AuxGralloc() {}
AuxGralloc::~AuxGralloc() {}

status_t AuxGralloc::allocateBuffer(uint32_t width, uint32_t height, PixelFormat format,
                                    ANativeWindowBuffer **anb, AHardwareBuffer **ahb) {
    // Ensure that layerCount is valid.
    uint32_t layerCount = 1;
    uint64_t usage = static_cast<uint64_t>(BufferUsage::CPU_READ_OFTEN | BufferUsage::GPU_TEXTURE);

    // make sure to not allocate a N x 0 or 0 x N buffer, since this is
    // allowed from an API stand-point allocate a 1x1 buffer instead.
    if (!width || !height)
        width = height = 1;

    AHardwareBuffer_Desc desc;
    desc.width = width;
    desc.height= height;
    desc.layers= 1;
    desc.format= AHWBuffer_convertFromPixelFormat(format);
    desc.usage = usage;
    desc.rfu0 = 0;
    desc.rfu1 = 0;
    status_t error = AHardwareBuffer_allocate(&desc, ahb);
    if (ahb == NULL){
        ALOGE("@%s: Failed to allocate (%u x %u) format %d error because of null_pointer(ahb)", __func__, width, height, format);
        return android::UNEXPECTED_NULL;
    }
    buffer_handle_t buffer_hnd = AHardwareBuffer_getNativeHandle(*ahb);
    if (ahb && error == android::NO_ERROR) {
        AHardwareBuffer_Desc outDesc;
        AHardwareBuffer_describe(*ahb, &outDesc);
        (*anb)->handle = buffer_hnd;
        (*anb)->width  = outDesc.width;
        (*anb)->height = outDesc.height;
        (*anb)->stride = outDesc.stride;
        (*anb)->format = outDesc.format;
        (*anb)->usage  = outDesc.usage;
        return android::NO_ERROR;
    } else {
        ahb = NULL;
        ALOGE("@%s: Failed to allocate (%u x %u) format %d error %d", __func__, width, height, format, error);
        return android::NO_MEMORY;
    }
}

status_t AuxGralloc::freeBuffer(ANativeWindowBuffer *anb, AHardwareBuffer *ahb) {
    if (ahb) {
        AHardwareBuffer_release(ahb);
        ahb = NULL;
        anb->handle = NULL;
        anb->width  = 0;
        anb->height = 0;
        anb->stride = 0;
        anb->format = 0;
        anb->usage  = 0;
        return android::NO_ERROR;
    } else {
        ALOGE("@%s: AHardwareBuffer is NULL", __func__);
        return android::NO_MEMORY;
    }
}

AuxBufferInfo AuxGralloc::getBufferInfo(const ANativeWindowBuffer *buffer) {
    int err = GRALLOC_EXTRA_OK;
    AuxBufferInfo info;
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_ION_FD, &info.ion_fd);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_WIDTH,  &info.width);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_HEIGHT, &info.height);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_FORMAT, &info.format);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_USAGE,  &info.usage);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_STRIDE, &info.stride);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &info.alloc_size);
    err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_VERTICAL_STRIDE, &info.vertical_stride);

    if (GRALLOC_EXTRA_OK == err) {
        gralloc_extra_ion_sf_info_t sf_info;
        err |= gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        info.status = sf_info.status;
        info.pool_id = sf_info.pool_id;
        info.timestamp = sf_info.timestamp;
        info.status2 = sf_info.status2;
        info.videobuffer_status = sf_info.videobuffer_status;
    } else {
        ALOGE("@%s: gralloc_extra_query failed, handle=%p", __func__, buffer->handle);
    }
    info.err = err;
    return info;
}

int32_t AuxGralloc::setBufferSFInfo(const ANativeWindowBuffer *buffer, uint32_t mask, uint32_t value) {

    gralloc_extra_ion_sf_info_t sf_info;
    gralloc_extra_query(buffer->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
    gralloc_extra_sf_set_status(&sf_info, mask, value);
    gralloc_extra_perform(buffer->handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);

    return 0;
}