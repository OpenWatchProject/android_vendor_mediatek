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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include <core/common.h>
#include <core/GraphicBufferPool.h>
#include <core/DebugControl.h>
#define PIPE_TRACE TRACE_GRAPHIC_BUFFER_POOL
#define PIPE_CLASS_TAG "GraphicBufferPool"
#include <core/PipeLog.h>


using namespace android;
using namespace NSCam::Utils::Format;

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

const MUINT32 GraphicBufferPool::USAGE_HW_TEXTURE = NativeBufferWrapper::USAGE_HW_TEXTURE;
const MUINT32 GraphicBufferPool::USAGE_HW_RENDER = NativeBufferWrapper::USAGE_HW_RENDER;

static MUINT32 queryStrideInPixels(MUINT32 fmt, MUINT32 i, MUINT32 width)
{
    TRACE_FUNC_ENTER();
    MUINT32 pixel;
    pixel = queryPlaneWidthInPixels(fmt, i, width) * queryPlaneBitsPerPixel(fmt, i) / 8;
    TRACE_FUNC_EXIT();
    return pixel;
}

GraphicBufferHandle::GraphicBufferHandle(const android::sp<BufferPool<GraphicBufferHandle> > &pool)
    : BufferHandle(pool)
    , mImageBuffer(NULL)
    , mGraphicBuffer(NULL)
    , mType(GraphicBufferHandle::ALLOCATE)
{
}

GraphicBufferHandle::~GraphicBufferHandle()
{
}

NB_SPTR GraphicBufferHandle::getGraphicBufferAddr()
{
    return mGraphicBuffer->getBuffer_SPTR();
}

#if 0
MBOOL GraphicBufferHandle::resize(MUINT32 width, MUINT32 height)
{
    MUINT32 fmt = this->mImageBuffer->getImgFormat();
    MUINT32 plane = this->mImageBuffer->getPlaneCount();

    MUINT32 size = 0;
    for( MUINT32 i = 0; i < plane; ++i )
    {
        size += (queryPlaneWidthInPixels(fmt,i, width) * queryPlaneBitsPerPixel(fmt,i) / 8) * queryPlaneHeightInPixels(fmt, i, height);
    }
    if (size > this->mMemBuf.size)
    {
        MY_LOGE("Resizing to a size (%dx%d) larger than originally allocated (%d bytes)\n", width, height, this->mMemBuf.size);
        return MFALSE;
    }

    return this->mImageBuffer->setExtParam(MSize(width, height));
}
#endif

#if 0
MBOOL GraphicBufferHandle::lock()
{
    return mGraphicBuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&mMemBuf.virtAddr));
}

MBOOL GraphicBufferHandle::unlock()
{
    return mGraphicBuffer->unlock();
}
#endif

android::sp<GraphicBufferPool> GraphicBufferPool::create(const char *name, MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT32 usage, NativeBufferWrapper::ColorSpace color)
{
  TRACE_FUNC_ENTER();
  sp<GraphicBufferPool> pool;
  pool = GraphicBufferPool::create(name, width, height, toImageFormat(format), usage, color);
  TRACE_FUNC_EXIT();
  return pool;
}

android::sp<GraphicBufferPool> GraphicBufferPool::create(const char *name, const MSize &size, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color)
{
  TRACE_FUNC_ENTER();
  sp<GraphicBufferPool> pool;
  pool = GraphicBufferPool::create(name, size.w, size.h, format, usage, color);
  TRACE_FUNC_EXIT();
  return pool;
}

android::sp<GraphicBufferPool> GraphicBufferPool::create(const char *name, MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color)
{
  TRACE_FUNC_ENTER();
  sp<GraphicBufferPool> pool = new GraphicBufferPool(name);
  if( pool == NULL )
  {
    MY_LOGE("OOM: %s: Cannot create GraphicBufferPool", name);
  }
  else if( !pool->init(width, height, format, usage, color) )
  {
    MY_LOGE("%s: GraphicBufferPool init failed", name);
    pool = NULL;
  }
  TRACE_FUNC_EXIT();
  return pool;
}

MVOID GraphicBufferPool::destroy(android::sp<GraphicBufferPool> &pool)
{
  TRACE_FUNC_ENTER();
  if( pool != NULL )
  {
    pool->releaseAll();
    pool = NULL;
  }
  TRACE_FUNC_EXIT();
}

GraphicBufferPool::GraphicBufferPool(const char *name)
    : BufferPool<GraphicBufferHandle>(name)
    , mReady(MFALSE)
    , mWidth(0)
    , mHeight(0)
    , mImageFormat(eImgFmt_YV12)
    , mGraphicFormat(HAL_PIXEL_FORMAT_YV12)
    , mImageUsage(0)
    , mGraphicUsage(0)
    , mPlane(0)
    , mColorSpace(NativeBufferWrapper::NOT_SET)
    , mAllocatorParam(0, 0)
    , mAllocatorExtraParam(0)
{
}

GraphicBufferPool::~GraphicBufferPool()
{
    uninit();
}

EImageFormat GraphicBufferPool::getImageFormat() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mImageFormat;
}

MSize GraphicBufferPool::getImageSize() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MSize(mWidth, mHeight);
}

MBOOL GraphicBufferPool::init(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color)
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);
  MBOOL ret = MFALSE;

  if( mReady )
  {
    MY_LOGE("%s: Already inited", mName);
  }
  else if( initConfig(width, height, format, usage, color) )
  {
    mReady = MTRUE;
    ret = MTRUE;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL GraphicBufferPool::initConfig(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;

  if( !width || !height )
  {
    MY_LOGE("%s: Erronuous dimension (%dx%d)", mName, width, height);
  }
  else
  {
    MY_LOGD("%s: %dx%d, fmt(0x%x/%s)", mName, width, height, format, toStr(format));
    mWidth = width;
    mHeight = height;
    mImageFormat = format;
    mGraphicFormat = toPixelFormat(format);
    mImageUsage = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY;
    mGraphicUsage = usage;
    mPlane = queryPlaneCount(mImageFormat);
    mColorSpace = color;

    if( mPlane > 3 )
    {
      MY_LOGE("%s: plane counter larger than 3, not supported", mName);
    }
    else
    {
      memset(mStride, 0, sizeof(mStride));
      memset(mBoundary, 0, sizeof(mBoundary));
      for( unsigned i = 0; i < mPlane; ++i )
      {
        mStride[i] = queryStrideInPixels(mImageFormat, i, mWidth);
      }
      mAllocatorParam = IImageBufferAllocator::ImgParam(mImageFormat, MSize(mWidth, mHeight), mStride, mBoundary, mPlane);
      mAllocatorExtraParam = IGrallocImageBufferHeap::AllocExtraParam(mGraphicUsage);
      ret = MTRUE;
    }
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MVOID GraphicBufferPool::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    if( mReady )
    {
        this->releaseAll();
        mReady = MFALSE;
    }
    TRACE_FUNC_EXIT();
}

#if 0
MBOOL GraphicBufferPool::add(const GraphicBufferPool::BufferInfo &info)
{
    return add(info.mGraphic, info.mImage, info.mSize);
}

MBOOL GraphicBufferPool::add(const NativeBufferWrapper &graphic, const android::sp<IImageBuffer> &image, MUINT32 size)
{
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    android::Mutex::Autolock lock(mMutex);
    sp<GraphicBufferHandle> handle;

    if( !mReady )
    {
        MY_LOGE("%s: pool need init first", mName);
    }
    else if( (handle = new GraphicBufferHandle(this)) == NULL )
    {
        MY_LOGE("OOM: %s: create GraphicBufferHandle failed", mName);
    }
    else if(image == NULL || graphic == NULL)
    {
        MY_LOGE("%s: invalid buffer passed", mName);
    }
    else
    {
        handle->mImageBuffer = image;
        handle->mGraphicBuffer = graphic;
        handle->mMemBuf.size = size;
        handle->mType = GraphicBufferHandle::REGISTER;
        addToPool(handle);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}
#endif

android::sp<GraphicBufferHandle> GraphicBufferPool::doAllocate()
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);

  sp<GraphicBufferHandle> bufferHandle;
  sp<IImageBufferHeap> heap;

  if( !mReady )
  {
    MY_LOGE("%s: pool need init first", mName);
    return NULL;
  }

  if( (bufferHandle = new GraphicBufferHandle(this)) == NULL )
  {
    MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    return NULL;
  }

  heap = IGrallocImageBufferHeap::create(mName, mAllocatorParam, mAllocatorExtraParam);
  if( heap == NULL )
  {
    MY_LOGE("%s: IGrallocImageBufferHeap create failed", mName);
    return NULL;
  }

  bufferHandle->mImageBuffer = heap->createImageBuffer();
  if( bufferHandle->mImageBuffer == NULL )
  {
    MY_LOGE("%s: heap->createImageBuffer failed", mName);
    return NULL;
  }

  NB_SPTR spPtr = (NB_SPTR)heap->getHWBuffer();
  if( spPtr == NULL )
  {
    MY_LOGE("%s: heap->getGraphicBuffer failed", mName);
    return NULL;
  }
  bufferHandle->mGraphicBuffer = new NativeBufferWrapper(getNativeBuffer(spPtr), mName);
  if( !bufferHandle->mGraphicBuffer->setGrallocExtraParam(mColorSpace) )
  {
    MY_LOGW("%s: Set Gralloc Extra Param failed", mName);
  }

  if( !bufferHandle->mImageBuffer->lockBuf(mName, mImageUsage) )
  {
    MY_LOGE("%s: mImageBuffer->lockBuf failed", mName);
    return NULL;
  }
  //bufferHandle->mImageBuffer->syncCache(eCACHECTRL_INVALID);
  bufferHandle->mType = GraphicBufferHandle::ALLOCATE;

  TRACE_FUNC_EXIT();
  return bufferHandle;
}

MBOOL GraphicBufferPool::doRelease(GraphicBufferHandle *handle)
{
  TRACE_FUNC_ENTER();

  MBOOL ret = MTRUE;
  if( !handle )
  {
    MY_LOGE("%s: GrpahicBufferHandle missing", mName);
    ret = MFALSE;
  }
  else
  {
    if( handle->mImageBuffer == NULL )
    {
      MY_LOGE("%s: GraphicBufferHandle::mImageBuffer missing", mName);
      ret = MFALSE;
    }
    else if( handle->mGraphicBuffer == NULL )
    {
      MY_LOGE("%s: GraphicBufferHandle::mImageBuffer missing", mName);
      ret = MFALSE;
    }
    else if( handle->mType == GraphicBufferHandle::ALLOCATE )
    {
      if( !handle->mImageBuffer->unlockBuf(mName) )
      {
        MY_LOGE("%s: Unlock buffer failed", mName);
        ret = MFALSE;
      }
    }

    handle->mGraphicBuffer = NULL;
    handle->mImageBuffer = NULL;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

EImageFormat GraphicBufferPool::toImageFormat(android_pixel_format_t graphFormat)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  switch(graphFormat)
  {
  case HAL_PIXEL_FORMAT_RGBA_8888:      return eImgFmt_RGBA8888;
  case HAL_PIXEL_FORMAT_YV12:           return eImgFmt_YV12;
  case HAL_PIXEL_FORMAT_RAW16:          return eImgFmt_RAW16;
  case HAL_PIXEL_FORMAT_RAW_OPAQUE:     return eImgFmt_RAW_OPAQUE;
  case HAL_PIXEL_FORMAT_BLOB:           return eImgFmt_BLOB;
  case HAL_PIXEL_FORMAT_RGBX_8888:      return eImgFmt_RGBX8888;
  case HAL_PIXEL_FORMAT_RGB_888:        return eImgFmt_RGB888;
  case HAL_PIXEL_FORMAT_RGB_565:        return eImgFmt_RGB565;
  case HAL_PIXEL_FORMAT_BGRA_8888:      return eImgFmt_BGRA8888;
  case HAL_PIXEL_FORMAT_YCbCr_422_I:    return eImgFmt_YUY2;
  case HAL_PIXEL_FORMAT_YCbCr_422_SP:   return eImgFmt_NV16;
  case HAL_PIXEL_FORMAT_YCrCb_420_SP:   return eImgFmt_NV21;
  case HAL_PIXEL_FORMAT_Y8:             return eImgFmt_Y8;
  case HAL_PIXEL_FORMAT_Y16:            return eImgFmt_Y16;
  default: return (EImageFormat)graphFormat;
  };
}

android_pixel_format_t GraphicBufferPool::toPixelFormat(EImageFormat format)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  switch(format)
  {
  case eImgFmt_RGBA8888:        return HAL_PIXEL_FORMAT_RGBA_8888;
  case eImgFmt_YV12:            return HAL_PIXEL_FORMAT_YV12;
  case eImgFmt_RAW16:           return HAL_PIXEL_FORMAT_RAW16;
  case eImgFmt_RAW_OPAQUE:      return HAL_PIXEL_FORMAT_RAW_OPAQUE;
  case eImgFmt_BLOB:            return HAL_PIXEL_FORMAT_BLOB;
  case eImgFmt_RGBX8888:        return HAL_PIXEL_FORMAT_RGBX_8888;
  case eImgFmt_RGB888:          return HAL_PIXEL_FORMAT_RGB_888;
  case eImgFmt_RGB565:          return HAL_PIXEL_FORMAT_RGB_565;
  case eImgFmt_BGRA8888:        return HAL_PIXEL_FORMAT_BGRA_8888;
  case eImgFmt_YUY2:            return HAL_PIXEL_FORMAT_YCbCr_422_I;
  case eImgFmt_NV16:            return HAL_PIXEL_FORMAT_YCbCr_422_SP;
  case eImgFmt_NV21:            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
  case eImgFmt_NV12:            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
  case eImgFmt_Y8:              return HAL_PIXEL_FORMAT_Y8;
  case eImgFmt_Y16:             return HAL_PIXEL_FORMAT_Y16;
  default:                      return (android_pixel_format)format;
  };
}

const char* GraphicBufferPool::toStr(EImageFormat format)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  switch(format)
  {
  case eImgFmt_RGBA8888:        return "rgba8888";
  case eImgFmt_YV12:            return "yv12";
  case eImgFmt_RAW16:           return "raw16";
  case eImgFmt_RAW_OPAQUE:      return "raw_opaque";
  case eImgFmt_BLOB:            return "blob";
  case eImgFmt_RGBX8888:        return "rgbx8888";
  case eImgFmt_RGB888:          return "rgb888";
  case eImgFmt_RGB565:          return "rgb565";
  case eImgFmt_BGRA8888:        return "bgra8888";
  case eImgFmt_YUY2:            return "yuy2";
  case eImgFmt_NV16:            return "nv16";
  case eImgFmt_NV21:            return "nv21";
  case eImgFmt_NV12:            return "nv12";
  case eImgFmt_Y8:              return "y8";
  case eImgFmt_Y16:             return "y16";
  default:                      return "unknown";
  };
}

class IIBuffer_GraphicBufferHandle : public IIBuffer
{
public:
  IIBuffer_GraphicBufferHandle(sb<GraphicBufferHandle> handle)
    : mHandle(handle)
  {
  }

  virtual ~IIBuffer_GraphicBufferHandle()
  {
  }

  virtual sp<IImageBuffer> getImageBuffer() const
  {
    sp<IImageBuffer> buffer;
    if( mHandle != NULL )
    {
      buffer = mHandle->mImageBuffer;
    }
    return buffer;
  }

  virtual NB_SPTR getGraphicBufferAddr()
  {
    NB_SPTR addr = NULL;
    if( mHandle != NULL )
    {
      addr = mHandle->getGraphicBufferAddr();
    }
    return addr;
  }

private:
  sb<GraphicBufferHandle> mHandle;
};

sp<IIBuffer> GraphicBufferPool::requestIIBuffer()
{
  TRACE_FUNC_ENTER();
  sb<GraphicBufferHandle> handle;
  sp<IIBuffer> buffer;
  handle = this->request();
  buffer = new IIBuffer_GraphicBufferHandle(handle);
  TRACE_FUNC_EXIT();
  return buffer;
}

}
}
}
}


