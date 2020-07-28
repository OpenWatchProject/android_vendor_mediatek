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
//#include <mtkcam/utils/SecureBufferHeap.h>
//#include <mtkcam/utils/imagebuf/IIonSecureBufferHeap.h>

#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>
#include "../include/SecureBufferPool.h"

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_SECURE_BUFFER_POOL
#define PIPE_CLASS_TAG "SecureBufferPool"
#include "../include/PipeLog.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

using android::sp;
using namespace NSCam::Utils::Format;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const MUINT32 SecureBufferPool::USAGE_HW = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY;
const MUINT32 SecureBufferPool::USAGE_SW = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN;
const MUINT32 SecureBufferPool::USAGE_HW_AND_SW = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
const MBOOL SecureBufferPool::SEPARATE_BUFFER = MFALSE;

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

SecureBufferHandle::SecureBufferHandle(const android::sp<BufferPool<SecureBufferHandle> > &pool)
    : BufferHandle(pool)
    , mType(SecureBufferHandle::ALLOCATE)
    , mUsage(0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

SecureBufferHandle::~SecureBufferHandle()
{
}

#if 0
MBOOL SecureBufferHandle::resize(MUINT32 width, MUINT32 height)
{
    TRACE_FUNC_ENTER();

    if( mType != SecureBufferHandle::ALLOCATE )
    {
        MY_LOGE("Non-allocated buffer does not support resize");
        return MFALSE;
    }

    MUINT32 fmt = mSecureBuffer->getImgFormat();
    MUINT32 plane = mSecureBuffer->getPlaneCount();
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
    if( !mSecureBuffer->unlockBuf( LOG_TAG ) )
    {
        MY_LOGE("unlock Buffer failed\n");
        return MFALSE;
    }
    mSecureBuffer = NULL;
    mSecureBuffer = createSecureBuffer(width, height, plane, fmt, mUsage, mMemBuf);

    /* Map to new SecureBuffer */
    if( mSecureBuffer == NULL )
    {
        MY_LOGE("NULL Buffer\n");
        return MFALSE;
    }


    TRACE_FUNC_EXIT();
    return MTRUE;
}
#endif

android::sp<SecureBufferPool> SecureBufferPool::create(const char *name, MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NSCam::SecType secType)
{
  TRACE_FUNC_ENTER();
  sp<SecureBufferPool> pool = new SecureBufferPool(name);
  if( pool == NULL )
  {
    MY_LOGE("OOM: %s: Cannot create SecureBufferPool", name);
  }
  else if( !pool->init(width, height, format, usage, secType) )
  {
    MY_LOGE("%s: SecureBufferPool init failed", name);
    pool = NULL;
  }
  TRACE_FUNC_EXIT();
  return pool;
}

android::sp<SecureBufferPool> SecureBufferPool::create(const char *name, const MSize &size, EImageFormat format, MUINT32 usage, NSCam::SecType secType)
{
  TRACE_FUNC_ENTER();
  sp<SecureBufferPool> pool;
  pool = SecureBufferPool::create(name, size.w, size.h, format, usage, secType);
  TRACE_FUNC_EXIT();
  return pool;
}

MVOID SecureBufferPool::destroy(android::sp<SecureBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    if( pool != NULL )
    {
        pool->releaseAll();
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
}

SecureBufferPool::SecureBufferPool(const char *name)
    : BufferPool<SecureBufferHandle>(name)
    , mReady(MFALSE)
    , mWidth(0)
    , mHeight(0)
    , mFormat(eImgFmt_YV12)
    , mUsage(0)
    , mPlane(0)
    , mSecType(SecType::mem_protected)
    , mBufferSize(0)
    , mAllocatorParam(0, 0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

SecureBufferPool::~SecureBufferPool()
{
    TRACE_FUNC_ENTER();
    uninit();
    TRACE_FUNC_EXIT();
}

EImageFormat SecureBufferPool::getImageFormat() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mFormat;
}

MSize SecureBufferPool::getImageSize() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT(); return MSize(mWidth, mHeight);
}

MBOOL SecureBufferPool::init(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NSCam::SecType secType)
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;

    if( mReady )
    {
        MY_LOGE("%s: Already init", mName);
    }
    else if( initConfig(width, height, format, usage, secType) )
    {
      mReady = MTRUE;
      ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL SecureBufferPool::initConfig(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NSCam::SecType secType)
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
    mSecType = secType;

    if( mPlane > 3 )
    {
      MY_LOGE("%s: plane counter larger than 3, not supported", mName);
    }
    else
    {
      memset(mStride, 0, sizeof(mStride));
      memset(mBoundary, 0, sizeof(mBoundary));
      mBufferSize = 0;
      for( unsigned i = 0; i < mPlane; ++i )
      {
        mStride[i] = queryStrideInPixels(mFormat, i, mWidth);
        mBufferSize += queryPlanePixel(mFormat, i, mWidth, mHeight);
      }
      ret = initAllocatorParam();
    }
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL SecureBufferPool::initAllocatorParam()
{
  TRACE_FUNC_ENTER();

//  mAllocatorParam = IImageBufferAllocator::ImgParam(mBufferSize, 0);
    mAllocatorParam = IImageBufferAllocator::ImgParam(mFormat, MSize(mWidth, mHeight), mStride, mBoundary, mPlane);

  TRACE_FUNC_EXIT();
  return MTRUE;
}

MVOID SecureBufferPool::uninit()
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

MBOOL SecureBufferPool::add(const android::sp<IImageBuffer> &image)
{
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    android::Mutex::Autolock lock(mMutex);
    sp<SecureBufferHandle> handle;

    if( !mReady )
    {
        MY_LOGE("%s: pool need init first", mName);
    }
    else if( image == NULL )
    {
        MY_LOGE("%s: invalid buffer handle", mName);
    }
    else if( (handle = new SecureBufferHandle(this)) == NULL )
    {
        MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    }
    else
    {
        //image->lockBuf(mName, mUsage);
        handle->mSecureBuffer = image;
        handle->mType = SecureBufferHandle::REGISTER;
        addToPool(handle);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

sp<SecureBufferHandle> SecureBufferPool::doAllocate()
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);

  sp<SecureBufferHandle> bufferHandle;

  if( !mReady )
  {
    MY_LOGE("%s: pool need init first", mName);
    return NULL;
  }

  if( (bufferHandle = new SecureBufferHandle(this)) == NULL )
  {
    MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    return NULL;
  }

  android::sp<ISecureImageBufferHeap> heap =
      ISecureImageBufferHeap::create(LOG_TAG,
            mAllocatorParam,
            ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MTRUE, mSecType));
  if (heap == NULL) {
      MY_LOGE("Allocate secure tuning buffer failed!");
      return NULL;
  }


#if 0 // mktodo
  heap = IIonSecureBufferHeap::create(mName, mAllocatorParam);
  if( heap == NULL )
  {
    MY_LOGE("%s: IIonSecureBufferHeap create failed", mName);
    return MFALSE;
  }
#endif

  bufferHandle->mSecureBuffer = createSecureBuffer(heap);
  if( bufferHandle->mSecureBuffer == NULL )
  {
    MY_LOGE("%s: heap->createSecureBuffer failed", mName);
    return NULL;
  }

  if( !bufferHandle->mSecureBuffer->lockBuf(mName, mUsage) )
  {
    MY_LOGE("%s: mSecureBuffer->lockBuf failed", mName);
    return NULL;
  }
  //bufferHandle->mSecureBuffer->syncCache(eCACHECTRL_INVALID);

  bufferHandle->mUsage = mUsage;
  bufferHandle->mType = SecureBufferHandle::ALLOCATE;

  TRACE_FUNC_EXIT();
  return bufferHandle;
}

IImageBuffer* SecureBufferPool::createSecureBuffer(const sp<IImageBufferHeap> &heap)
{
  TRACE_FUNC_ENTER();
  IImageBuffer *SecureBuffer = NULL;
  if( heap != NULL )
  {
    SecureBuffer = heap->createImageBuffer();
  }


  TRACE_FUNC_EXIT();
  return SecureBuffer;
}

MBOOL SecureBufferPool::doRelease(SecureBufferHandle *handle)
{
  TRACE_FUNC_ENTER();

  // release should not need lock(mMutex)
  // becuare only BufferPool::releaseAll and
  // BufferPool::recycle calls release for handles for the pool,
  // and no handle can exist when IMemDrv is invalid

  MBOOL ret = MTRUE;

  if( !handle )
  {
    MY_LOGE("%s: SecureBufferHandle missing", mName);
    ret = MFALSE;
  }
  else
  {
    if( handle->mSecureBuffer == NULL )
    {
      MY_LOGE("%s: SecureBufferHandle::mSecureBuffer missing", mName);
      ret = MFALSE;
    }
    else if( handle->mType == SecureBufferHandle::ALLOCATE )
    {
      if( !handle->mSecureBuffer->unlockBuf(mName) )
      {
        MY_LOGE("%s: SecureBufferHandle unlockBuf failed", mName);
        ret = MFALSE;
      }
    }
    handle->mSecureBuffer = NULL;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

class IIBuffer_SecureBufferHandle : public IIBuffer
{
public:
  IIBuffer_SecureBufferHandle(sb<SecureBufferHandle> handle)
    : mHandle(handle)
  {
  }

  virtual ~IIBuffer_SecureBufferHandle()
  {
  }

  virtual sp<IImageBuffer> getImageBuffer() const
  {
    sp<IImageBuffer> buffer;
    if( mHandle != NULL )
    {
      buffer = mHandle->mSecureBuffer;
    }
    return buffer;
  }

private:
  sb<SecureBufferHandle> mHandle;
};

sp<IIBuffer> SecureBufferPool::requestIIBuffer()
{
  TRACE_FUNC_ENTER();
  sb<SecureBufferHandle> handle;
  sp<IIBuffer> buffer;
  handle = this->request();
  buffer = new IIBuffer_SecureBufferHandle(handle);
  TRACE_FUNC_EXIT();
  return buffer;
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

