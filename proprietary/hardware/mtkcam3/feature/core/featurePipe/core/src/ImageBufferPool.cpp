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

#include "../include/MtkHeader.h"
//#include <mtkcam/common.h>
//#include <mtkcam/utils/common.h>
//#include <mtkcam/utils/ImageBufferHeap.h>
//#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>

#include "../include/ImageBufferPool.h"

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_IMAGE_BUFFER_POOL
#define PIPE_CLASS_TAG "ImageBufferPool"
#include "../include/PipeLog.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

// use one single contiguous memory for all buffer planes
#define USE_SINGLE_BUFFER MTRUE

using android::sp;
using namespace NSCam::Utils::Format;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const MUINT32 ImageBufferPool::USAGE_HW = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY;
const MUINT32 ImageBufferPool::USAGE_SW = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN;
const MUINT32 ImageBufferPool::USAGE_HW_AND_SW = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
const MBOOL ImageBufferPool::SEPARATE_BUFFER = MFALSE;

static MUINT32 queryPlanePixel(MUINT32 fmt, MUINT32 i, MUINT32 width, MUINT32 height)
{
    TRACE_FUNC_ENTER();
    MUINT32 pixel;
    pixel = (queryPlaneWidthInPixels(fmt,i, width) * queryPlaneBitsPerPixel(fmt,i) / 8) * queryPlaneHeightInPixels(fmt, i, height);
    TRACE_FUNC_EXIT();
    return pixel;
}

static MUINT32 queryStrideInPixels(MUINT32 fmt, MUINT32 i, MUINT32 width)
{
    TRACE_FUNC_ENTER();
    MUINT32 pixel;
    pixel = queryPlaneWidthInPixels(fmt, i, width) * queryPlaneBitsPerPixel(fmt, i) / 8;
    TRACE_FUNC_EXIT();
    return pixel;
}

ImageBufferHandle::ImageBufferHandle(const android::sp<BufferPool<ImageBufferHandle> > &pool)
    : BufferHandle(pool)
    , mType(ImageBufferHandle::ALLOCATE)
    , mUsage(0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

ImageBufferHandle::~ImageBufferHandle()
{
}

#if 0
MBOOL ImageBufferHandle::resize(MUINT32 width, MUINT32 height)
{
    TRACE_FUNC_ENTER();

    if( mType != ImageBufferHandle::ALLOCATE )
    {
        MY_LOGE("Non-allocated buffer does not support resize");
        return MFALSE;
    }

    MUINT32 fmt = mImageBuffer->getImgFormat();
    MUINT32 plane = mImageBuffer->getPlaneCount();
    MUINT32 size = 0;

    for( unsigned i = 0; i < plane; ++i )
    {
        size += queryPlanePixel(fmt, i, width, height);
    }
    if( size > mMemBuf.size )
    {
        MY_LOGE("Resizing to a size (%dx%d) larger than originally allocated (%d bytes)", width, height, mMemBuf.size);
        return MFALSE;
    }

    /* Delete original mapping */
    if( !mImageBuffer->unlockBuf( LOG_TAG ) )
    {
        MY_LOGE("unlock Buffer failed\n");
        return MFALSE;
    }
    mImageBuffer = NULL;
    mImageBuffer = createImageBuffer(width, height, plane, fmt, mUsage, mMemBuf);

    /* Map to new ImageBuffer */
    if( mImageBuffer == NULL )
    {
        MY_LOGE("NULL Buffer\n");
        return MFALSE;
    }


    TRACE_FUNC_EXIT();
    return MTRUE;
}
#endif

android::sp<ImageBufferPool> ImageBufferPool::create(const char *name, MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, MBOOL singleBuffer)
{
  TRACE_FUNC_ENTER();
  sp<ImageBufferPool> pool = new ImageBufferPool(name);
  if( pool == NULL )
  {
    MY_LOGE("OOM: %s: Cannot create ImageBufferPool", name);
  }
  else if( !pool->init(width, height, format, usage, singleBuffer) )
  {
    MY_LOGE("%s: ImageBufferPool init failed", name);
    pool = NULL;
  }
  TRACE_FUNC_EXIT();
  return pool;
}

android::sp<ImageBufferPool> ImageBufferPool::create(const char *name, const MSize &size, EImageFormat format, MUINT32 usage, MBOOL singleBuffer)
{
  TRACE_FUNC_ENTER();
  sp<ImageBufferPool> pool;
  pool = ImageBufferPool::create(name, size.w, size.h, format, usage, singleBuffer);
  TRACE_FUNC_EXIT();
  return pool;
}

MVOID ImageBufferPool::destroy(android::sp<ImageBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    if( pool != NULL )
    {
        pool->releaseAll();
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
}

ImageBufferPool::ImageBufferPool(const char *name)
    : BufferPool<ImageBufferHandle>(name)
    , mReady(MFALSE)
    , mWidth(0)
    , mHeight(0)
    , mFormat(eImgFmt_YV12)
    , mUsage(0)
    , mPlane(0)
    , mUseSingleBuffer(MTRUE)
    , mBufferSize(0)
    , mAllocatorParam(0, 0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

ImageBufferPool::~ImageBufferPool()
{
    TRACE_FUNC_ENTER();
    uninit();
    TRACE_FUNC_EXIT();
}

EImageFormat ImageBufferPool::getImageFormat() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mFormat;
}

MSize ImageBufferPool::getImageSize() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MSize(mWidth, mHeight);
}

MBOOL ImageBufferPool::init(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, MBOOL singleBuffer)
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;

    if( mReady )
    {
        MY_LOGE("%s: Already init", mName);
    }
    else if( initConfig(width, height, format, usage, singleBuffer) )
    {
      mReady = MTRUE;
      ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL ImageBufferPool::initConfig(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, MBOOL singleBuffer)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  if( !usage )
  {
    MY_LOGE("%s: Should specify image buffer usage", mName);
  }
  else if( !width || !height )
  {
    MY_LOGE("%s: Erronuous dimension (%dx%d)", mName, width, height);
  }
  else
  {
    MY_LOGD("%s: %dx%d, fmt(0x%x)", mName, width, height, format);
    mWidth = width;
    mHeight = height;
    mFormat = format;
    mUsage = usage;
    mPlane = queryPlaneCount(format);

    if( mPlane > 3 )
    {
      MY_LOGE("%s: plane counter larger than 3, not supported", mName);
    }
    else if(mPlane > 0)
    {
      memset(mStride, 0, sizeof(mStride));
      memset(mBoundary, 0, sizeof(mBoundary));
      mBufferSize = 0;
      for( unsigned i = 0; i < mPlane; ++i )
      {
        mStride[i] = queryStrideInPixels(mFormat, i, mWidth);
        mBufferSize += queryPlanePixel(mFormat, i, mWidth, mHeight);
      }
      mUseSingleBuffer = singleBuffer;
      ret = initAllocatorParam();
    }
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL ImageBufferPool::initAllocatorParam()
{
  TRACE_FUNC_ENTER();
  if( mUseSingleBuffer )
  {
    mAllocatorParam = IImageBufferAllocator::ImgParam(mBufferSize, 0);
  }
  else
  {
    mAllocatorParam = IImageBufferAllocator::ImgParam(mFormat, MSize(mWidth, mHeight), mStride, mBoundary, mPlane);
  }
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MVOID ImageBufferPool::uninit()
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

MBOOL ImageBufferPool::add(const android::sp<IImageBuffer> &image)
{
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    android::Mutex::Autolock lock(mMutex);
    sp<ImageBufferHandle> handle;

    if( !mReady )
    {
        MY_LOGE("%s: pool need init first", mName);
    }
    else if( image == NULL )
    {
        MY_LOGE("%s: invalid buffer handle", mName);
    }
    else if( (handle = new ImageBufferHandle(this)) == NULL )
    {
        MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    }
    else
    {
        //image->lockBuf(mName, mUsage);
        handle->mImageBuffer = image;
        handle->mType = ImageBufferHandle::REGISTER;
        addToPool(handle);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

sp<ImageBufferHandle> ImageBufferPool::doAllocate()
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);

  sp<ImageBufferHandle> bufferHandle;
  sp<IImageBufferHeap> heap;

  if( !mReady )
  {
    MY_LOGE("%s: pool need init first", mName);
    return NULL;
  }

  if( (bufferHandle = new ImageBufferHandle(this)) == NULL )
  {
    MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    return NULL;
  }

  heap = IIonImageBufferHeap::create(mName, mAllocatorParam);
  if( heap == NULL )
  {
    MY_LOGE("%s: IIonImageBufferHeap create failed", mName);
    return MFALSE;
  }

  bufferHandle->mImageBuffer = createImageBuffer(heap);
  if( bufferHandle->mImageBuffer == NULL )
  {
    MY_LOGE("%s: heap->createImageBuffer failed", mName);
    return NULL;
  }

  if( !bufferHandle->mImageBuffer->lockBuf(mName, mUsage) )
  {
    MY_LOGE("%s: mImageBuffer->lockBuf failed", mName);
    return NULL;
  }
  //bufferHandle->mImageBuffer->syncCache(eCACHECTRL_INVALID);

  bufferHandle->mUsage = mUsage;
  bufferHandle->mType = ImageBufferHandle::ALLOCATE;

  TRACE_FUNC_EXIT();
  return bufferHandle;
}

IImageBuffer* ImageBufferPool::createImageBuffer(const sp<IImageBufferHeap> &heap)
{
  TRACE_FUNC_ENTER();
  IImageBuffer *imageBuffer = NULL;
  if( heap != NULL )
  {
    if( mUseSingleBuffer )
    {
      imageBuffer = heap->createImageBuffer_FromBlobHeap(0, mFormat, MSize(mWidth, mHeight), mStride);
    }
    else
    {
      imageBuffer = heap->createImageBuffer();
    }
  }
  TRACE_FUNC_EXIT();
  return imageBuffer;
}

MBOOL ImageBufferPool::doRelease(ImageBufferHandle *handle)
{
  TRACE_FUNC_ENTER();

  // release should not need lock(mMutex)
  // becuare only BufferPool::releaseAll and
  // BufferPool::recycle calls release for handles for the pool,
  // and no handle can exist when IMemDrv is invalid

  MBOOL ret = MTRUE;

  if( !handle )
  {
    MY_LOGE("%s: ImageBufferHandle missing", mName);
    ret = MFALSE;
  }
  else
  {
    if( handle->mImageBuffer == NULL )
    {
      MY_LOGE("%s: ImageBufferHandle::mImageBuffer missing", mName);
      ret = MFALSE;
    }
    else if( handle->mType == ImageBufferHandle::ALLOCATE )
    {
      if( !handle->mImageBuffer->unlockBuf(mName) )
      {
        MY_LOGE("%s: ImageBufferHandle unlockBuf failed", mName);
        ret = MFALSE;
      }
    }
    handle->mImageBuffer = NULL;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

class IIBuffer_ImageBufferHandle : public IIBuffer
{
public:
  IIBuffer_ImageBufferHandle(sb<ImageBufferHandle> handle)
    : mHandle(handle)
  {
  }

  virtual ~IIBuffer_ImageBufferHandle()
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

private:
  sb<ImageBufferHandle> mHandle;
};

sp<IIBuffer> ImageBufferPool::requestIIBuffer()
{
  TRACE_FUNC_ENTER();
  sb<ImageBufferHandle> handle;
  sp<IIBuffer> buffer;
  handle = this->request();
  buffer = new IIBuffer_ImageBufferHandle(handle);
  TRACE_FUNC_EXIT();
  return buffer;
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
