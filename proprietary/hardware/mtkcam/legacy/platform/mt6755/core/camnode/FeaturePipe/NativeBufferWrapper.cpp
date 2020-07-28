/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


#define LOG_TAG "MtkCam/NativeBufferWrapper"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>


//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg)


#if 0
#define FUNC_START() MY_LOGD("+")
#define FUNC_END()   MY_LOGD("-")
#define FUNC_NAME()  MY_LOGD("")
#else
#define FUNC_START()
#define FUNC_END()
#define FUNC_NAME()
#endif

#define MODULE_NAME "NativeBufferWrapper"
#include "NativeBufferWrapper.h"


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


#if SUPPORT_AHARDWAREBUFFER
const MUINT64 NativeBufferWrapper::USAGE_HW_TEXTURE = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_HW_RENDER  = AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT  | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_SW = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN  | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
#else // TODO: remove after stable
using android::GraphicBuffer;
const MUINT64 NativeBufferWrapper::USAGE_HW_TEXTURE = GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_HW_RENDER  = GraphicBuffer::USAGE_HW_RENDER  | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_SW = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;
#endif // SUPPORT_AHARDWAREBUFFER


NativeBufferWrapper::NativeBufferWrapper(NativeBuffer_PTR buffer, const std::string &name)
    : mName(name)
    , mBuffer(buffer)
{
    FUNC_START();

    if( mBuffer != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        AHardwareBuffer_acquire(mBuffer);
#endif
    }
    else
    {
        MY_LOGE("Invalid nativeBuffer (%p)", getWarp_SPTR());
    }

    FUNC_END();
}

NativeBufferWrapper::NativeBufferWrapper(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage, const std::string &name)
    : mName(name)
    , mBuffer(NULL)
{
    FUNC_START();

    this->allocate(width, height, format, usage);

    FUNC_END();
}

NativeBufferWrapper::~NativeBufferWrapper()
{
    FUNC_START();

    MY_LOGD("Release buffer %s", mName.c_str());
#if SUPPORT_AHARDWAREBUFFER
    if( mBuffer != NULL )
    {
        AHardwareBuffer_release(mBuffer);
    }
#endif
    mBuffer = NULL;

    FUNC_END();
}

buffer_handle_t NativeBufferWrapper::getHandle()
{
    FUNC_START();

    buffer_handle_t handle = NULL;
#if SUPPORT_AHARDWAREBUFFER
    handle = AHardwareBuffer_getNativeHandle(mBuffer);
#else
    handle = mBuffer->handle;
#endif

    FUNC_END();

    return handle;
}

NativeBuffer_PTR NativeBufferWrapper::getBuffer()
{
    FUNC_START();
    FUNC_END();

    return mBuffer;
}

Warp_SPTR NativeBufferWrapper::getWarp_SPTR()
{
    FUNC_START();
    FUNC_END();
#if SUPPORT_AHARDWAREBUFFER
    return mBuffer;
#else
    return &mBuffer;
#endif
}

MBOOL NativeBufferWrapper::lock(MUINT64 usage, void** vaddr)
{
    FUNC_START();
    FUNC_END();
    return this->lock(getWarp_SPTR(), usage, vaddr);
}

MBOOL NativeBufferWrapper::unlock()
{
    FUNC_START();
    FUNC_END();
    return this->unlock(getWarp_SPTR());
}

#if SUPPORT_AHARDWAREBUFFER
MUINT32 NativeBufferWrapper::toNativeFormat(android_pixel_format_t format)
{
    FUNC_START();
    FUNC_END();

    switch( format )
    {
    case HAL_PIXEL_FORMAT_RGB_565:                return AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
    case HAL_PIXEL_FORMAT_BGRA_8888:              return AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM;
    case HAL_PIXEL_FORMAT_RGBA_1010102:           return AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM;
    case HAL_PIXEL_FORMAT_RGBA_FP16:              return AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT;
    case HAL_PIXEL_FORMAT_BLOB:                   return AHARDWAREBUFFER_FORMAT_BLOB;
    case HAL_PIXEL_FORMAT_YV12:                   return AHARDWAREBUFFER_FORMAT_YV12;
    case HAL_PIXEL_FORMAT_Y8:                     return AHARDWAREBUFFER_FORMAT_Y8;
    case HAL_PIXEL_FORMAT_Y16:                    return AHARDWAREBUFFER_FORMAT_Y16;
    case HAL_PIXEL_FORMAT_RAW16:                  return AHARDWAREBUFFER_FORMAT_RAW16;
    case HAL_PIXEL_FORMAT_RAW10:                  return AHARDWAREBUFFER_FORMAT_RAW10;
    case HAL_PIXEL_FORMAT_RAW12:                  return AHARDWAREBUFFER_FORMAT_RAW12;
    case HAL_PIXEL_FORMAT_RAW_OPAQUE:             return AHARDWAREBUFFER_FORMAT_RAW_OPAQUE;
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED: return AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED;
    case HAL_PIXEL_FORMAT_YCBCR_420_888:          return AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
    case HAL_PIXEL_FORMAT_YCBCR_422_SP:           return AHARDWAREBUFFER_FORMAT_YCbCr_422_SP;
    case HAL_PIXEL_FORMAT_YCRCB_420_SP:           return AHARDWAREBUFFER_FORMAT_YCrCb_420_SP;
    case HAL_PIXEL_FORMAT_YCBCR_422_I:            return AHARDWAREBUFFER_FORMAT_YCbCr_422_I;
    default:
        MY_LOGE("Pixel format(%d) is not supported, use native format(%d) as default", format, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
        return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    };

}
#endif

MBOOL NativeBufferWrapper::lock(Warp_SPTR bufferPtr, MUINT64 usage, void** vaddr)
{
    FUNC_START();

    int ret = 0;
    if( bufferPtr != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        ret = AHardwareBuffer_lock(bufferPtr, usage, -1, NULL, vaddr);
#else
        ret = (*bufferPtr)->lock(usage, vaddr);
#endif
    }

    FUNC_END();

    if( ret || bufferPtr == NULL )
    {
        MY_LOGE("Lock failed: %d or invalid nativeBuffer (%p) ", ret, bufferPtr);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL NativeBufferWrapper::unlock(Warp_SPTR bufferPtr)
{
    FUNC_START();

    int ret = 0;

    if( bufferPtr != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        int *fence = nullptr;
        ret = AHardwareBuffer_unlock(bufferPtr, fence);
#else
        ret = (*bufferPtr)->unlock();
#endif
    }

    FUNC_END();

    if( ret || bufferPtr == NULL )
    {
        MY_LOGE("Unlock failed: %d or invalid nativeBuffer (%p) ", ret, bufferPtr);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL NativeBufferWrapper::allocate(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage)
{
    FUNC_START();

    int ret = 0;
    MY_LOGD("Allocate Buffer = %s, w(%d), h(%d), format(%d), usage(%" PRIi64 ")",
            mName.c_str(), width, height, format, usage);

    if( mBuffer == NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        AHardwareBuffer_Desc desc =
        {
            width, height, 1, toNativeFormat(format), usage, 0, 0, 0
        };
        ret = AHardwareBuffer_allocate(&desc, &mBuffer);

#else
        mBuffer = new GraphicBuffer(width, height, format, usage);
        ret = mBuffer->initCheck();
#endif
    }
    else
    {
        MY_LOGW("Buffer %s has exsit!", mName.c_str());
    }

    FUNC_END();

    if( ret || mBuffer == NULL )
    {
        MY_LOGE("Allocate nativeBuffer failed: %d, buffer(%p)", ret, getWarp_SPTR());
        return MFALSE;
    }

    return MTRUE;
}



}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

