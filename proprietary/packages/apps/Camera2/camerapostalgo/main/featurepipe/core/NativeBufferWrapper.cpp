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

#include <core/NativeBufferWrapper.h>
#include <core/DebugControl.h>
#include <ui/gralloc_extra.h>

#define PIPE_CLASS_TAG "NativeBufferWrapper"
#define PIPE_TRACE TRACE_NATIVEBUFFER_WRAPPER
#include <core/PipeLog.h>




namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

#if SUPPORT_AHARDWAREBUFFER
const MUINT64 NativeBufferWrapper::USAGE_HW_TEXTURE = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_HW_RENDER  = AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT  | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_SW = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
#else //remove after stable
using android::GraphicBuffer;
const MUINT64 NativeBufferWrapper::USAGE_HW_TEXTURE = GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_HW_RENDER  = GraphicBuffer::USAGE_HW_RENDER  | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN;
const MUINT64 NativeBufferWrapper::USAGE_SW = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;
#endif // SUPPORT_AHARDWAREBUFFER


NativeBufferWrapper::NativeBufferWrapper(NB_PTR buffer, const std::string &name)
    : mName(name)
    , mBuffer(buffer)
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Acquire buffer %s", mName.c_str());
    if( mBuffer != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        AHardwareBuffer_acquire(mBuffer);
#endif
    }
    else
    {
        MY_LOGE("Invalid nativeBuffer (%p)", getBuffer_SPTR());
    }

    TRACE_FUNC_EXIT();
}

NativeBufferWrapper::NativeBufferWrapper(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage, const std::string &name)
    : mName(name)
    , mBuffer(NULL)
{
    TRACE_FUNC_ENTER();

    this->allocate(width, height, format, usage);

    TRACE_FUNC_EXIT();
}

NativeBufferWrapper::~NativeBufferWrapper()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Release buffer %s", mName.c_str());
#if SUPPORT_AHARDWAREBUFFER
    if( mBuffer != NULL )
    {
        AHardwareBuffer_release(mBuffer);
    }
#endif
    mBuffer = NULL;
    TRACE_FUNC_EXIT();
}

buffer_handle_t NativeBufferWrapper::getHandle()
{
    TRACE_FUNC_ENTER();

    buffer_handle_t handle = NULL;
#if SUPPORT_AHARDWAREBUFFER
    handle = AHardwareBuffer_getNativeHandle(mBuffer);
#else
    handle = mBuffer->handle;
#endif

    TRACE_FUNC_EXIT();

    return handle;
}

NB_PTR NativeBufferWrapper::getBuffer()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mBuffer;
}

NB_SPTR NativeBufferWrapper::getBuffer_SPTR()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
#if SUPPORT_AHARDWAREBUFFER
    return mBuffer;
#else
    return &mBuffer;
#endif
}

MBOOL NativeBufferWrapper::lock(MUINT64 usage, void** vaddr)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return lockNativeBuffer(getBuffer_SPTR(), usage, vaddr);
}

MBOOL NativeBufferWrapper::unlock()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return unlockNativeBuffer(getBuffer_SPTR());
}

#if SUPPORT_AHARDWAREBUFFER
MUINT32 NativeBufferWrapper::toNativeFormat(android_pixel_format_t format)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
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

MUINT32 NativeBufferWrapper::toGrallocExtraColor(ColorSpace color)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    switch( color )
    {
    case YUV_BT601_NARROW:  return GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
    case YUV_BT601_FULL:    return GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
    case YUV_BT709_NARROW:  return GRALLOC_EXTRA_BIT_YUV_BT709_NARROW;
    case YUV_BT709_FULL:    return GRALLOC_EXTRA_BIT_YUV_BT709_FULL;
    case YUV_BT2020_NARROW: return GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW;
    case YUV_BT2020_FULL:   return GRALLOC_EXTRA_BIT_YUV_BT2020_FULL;
    default:
        MY_LOGW("Color space(%d) is not supported, use color space(%d) as default", color, YUV_BT601_FULL);
        return GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
    }
}

MBOOL NativeBufferWrapper::allocate(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage)
{
    TRACE_FUNC_ENTER();

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

    TRACE_FUNC_EXIT();

    if( ret || mBuffer == NULL )
    {
        MY_LOGE("Allocate nativeBuffer failed: %d, buffer(%p)", ret, getBuffer_SPTR());
        return MFALSE;
    }
    return MTRUE;
}

MBOOL NativeBufferWrapper::setGrallocExtraParam(ColorSpace color)
{
    TRACE_FUNC_ENTER();
    if( color != NOT_SET )
    {
        MUINT32 colorSpace = toGrallocExtraColor(color);
        gralloc_extra_ion_sf_info_t info;
        if( gralloc_extra_query(getHandle(), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info) != GRALLOC_EXTRA_OK )
        {
            MY_LOGW("Set color space %s = gralloc_extra_query failed", mName.c_str());
            return MFALSE;
        }
        if( gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, colorSpace) != GRALLOC_EXTRA_OK )
        {
            MY_LOGW("Set color space %s = gralloc_extra_sf_set_status failed. color(%d)", mName.c_str(), colorSpace);
            return MFALSE;
        }
        if( gralloc_extra_perform(getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info) != GRALLOC_EXTRA_OK )
        {
            MY_LOGW("Set color space %s = gralloc_extra_perform failed", mName.c_str());
            return MFALSE;
        }
        TRACE_FUNC("Set color space %s = color(%d) successfully", mName.c_str(), colorSpace);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}


NB_PTR getNativeBuffer(NB_SPTR buffer)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    if( buffer != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        return buffer;
#else
        return *buffer;
#endif
    }
    else
    {
        MY_LOGE("NULL NativeBuffer");
    }
    return NULL;
}

NativeBuffer* getNativeBufferPtr(NB_SPTR buffer)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    if( buffer != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        return buffer;
#else
        return (*buffer).get();
#endif
    }
    else
    {
        MY_LOGE("NULL NativeBuffer");
    }
    return NULL;
}

MBOOL lockNativeBuffer(NB_SPTR buffer, MUINT64 usage, void** vaddr)
{
    TRACE_FUNC_ENTER();

    int ret = 0;
    if( buffer != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        ret = AHardwareBuffer_lock(buffer, usage, -1, NULL, vaddr);
#else
        ret = (*buffer)->lock(usage, vaddr);
#endif
    }
    TRACE_FUNC_EXIT();
    if( ret || buffer == NULL )
    {
        MY_LOGE("Lock failed: %d or invalid nativeBuffer (%p) ", ret, buffer);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL unlockNativeBuffer(NB_SPTR buffer)
{
    TRACE_FUNC_ENTER();

    int ret = 0;
    if( buffer != NULL )
    {
#if SUPPORT_AHARDWAREBUFFER
        int* fence = nullptr;
        ret = AHardwareBuffer_unlock(buffer, fence);
#else
        ret = (*buffer)->unlock();
#endif
    }

    TRACE_FUNC_EXIT();

    if( ret || buffer == NULL )
    {
        MY_LOGE("Unlock failed: %d or invalid nativeBuffer (%p) ", ret, buffer);
        return MFALSE;
    }
    return MTRUE;
}

}
}
}
}


