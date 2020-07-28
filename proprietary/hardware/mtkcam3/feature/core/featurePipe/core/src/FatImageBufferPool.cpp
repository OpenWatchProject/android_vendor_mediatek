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
//#include <mtkcam/utils/imagebuf/IDummyImageBufferHeap.h>
//#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>

#include "../include/FatImageBufferPool.h"

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_FAT_IMAGE_BUFFER_POOL
#define PIPE_CLASS_TAG "FatImageBufferPool"
#include "../include/PipeLog.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

using android::sp;
using namespace NSCam::Utils::Format;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const MUINT32 FatImageBufferPool::USAGE_HW = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY;
const MUINT32 FatImageBufferPool::USAGE_SW = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN;
const MUINT32 FatImageBufferPool::USAGE_HW_AND_SW = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;

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

FatHeap::FatHeap(const char* name, MUINT32 size, MUINT32 count, MINT usage)
  : mName(name)
  , mUsage(usage)
  , mChunkSize(size)
  , mChunkCount(count)
  , mChunkUsed(0)
{
  TRACE_FUNC_ENTER();
  if( size <= 0 || count <= 0 )
  {
    MY_LOGE("%s: Invalid param: size=%d count=%d", mName, size, count);
    return;
  }

  IImageBufferAllocator::ImgParam param(mChunkSize * mChunkCount, 0);
  mHeap = IIonImageBufferHeap::create(mName, param);
  if( mHeap == NULL )
  {
    MY_LOGE("%s: IIonImageBufferHeap create failed", mName);
    return;
  }

  mBuffer = mHeap->createImageBuffer();
  if( mBuffer == NULL )
  {
    MY_LOGE("%s: createImageBuffer failed", mName);
    return;
  }

  if( !mBuffer->lockBuf(mName, usage) )
  {
    MY_LOGE("%s: mBuffer->lockBuf failed", mName);
    mBuffer = NULL;
    mHeap = NULL;
  }
  TRACE_FUNC_EXIT();
}

FatHeap::~FatHeap()
{
  TRACE_FUNC_ENTER();
  if( mBuffer != NULL )
  {
    mBuffer->unlockBuf(mName);
    mBuffer = NULL;
  }
  TRACE_FUNC_EXIT();
}

MBOOL FatHeap::isValid() const
{
  TRACE_FUNC_ENTER();
  MBOOL ret = (mBuffer != NULL) && mChunkCount && (mChunkUsed < mChunkCount);
  TRACE_FUNC_EXIT();
  return ret;
}

MUINT32 FatHeap::getCount() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mChunkCount;
}

sp<IImageBufferHeap> FatHeap::createDummyHeap()
{
  TRACE_FUNC_ENTER();
  sp<IImageBufferHeap> heap;
  if( this->isValid() )
  {
    MUINTPTR heapVA = mHeap->getBufVA(0);
    MUINTPTR heapPA = (mUsage & eBUFFER_USAGE_HW_MASK) ? mHeap->getBufPA(0) : 0;
    MINT32 heapID = mHeap->getHeapID();
    MUINTPTR bufVA[3] = { 0, 0, 0 };
    MUINTPTR bufPA[3] = { 0, 0, 0 };
    bufVA[0] = heapVA + (mChunkUsed*mChunkSize);
    bufPA[0] = (mUsage & eBUFFER_USAGE_HW_MASK) ? heapPA + (mChunkUsed*mChunkSize) : 0;
    IImageBufferAllocator::ImgParam param(mChunkSize, 0);
    PortBufInfo_dummy portBufInfo(heapID, bufVA, bufPA, 1);
    heap = IDummyImageBufferHeap::create(mName, param, portBufInfo, false);
    if( heap == NULL )
    {
      MY_LOGE("%s: dummy heap create failed", mName);
    }
    else
    {
      ++mChunkUsed;
    }
  }
  TRACE_FUNC_EXIT();
  return heap;
}

FatImageBufferHandle::FatImageBufferHandle(const android::sp<BufferPool<FatImageBufferHandle> > &pool)
    : BufferHandle(pool)
    , mType(FatImageBufferHandle::ALLOCATE)
    , mUsage(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

FatImageBufferHandle::~FatImageBufferHandle()
{
}

android::sp<FatImageBufferPool> FatImageBufferPool::create(const char *name, MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage)
{
  TRACE_FUNC_ENTER();
  sp<FatImageBufferPool> pool = new FatImageBufferPool(name);
  if( pool == NULL )
  {
    MY_LOGE("OOM: %s: Cannot create FatImageBufferPool", name);
  }
  else if( !pool->init(width, height, format, usage) )
  {
    MY_LOGE("%s: FatImageBufferPool init failed", name);
    pool = NULL;
  }
  TRACE_FUNC_EXIT();
  return pool;
}

android::sp<FatImageBufferPool> FatImageBufferPool::create(const char *name, const MSize &size, EImageFormat format, MUINT32 usage)
{
  TRACE_FUNC_ENTER();
  sp<FatImageBufferPool> pool;
  pool = FatImageBufferPool::create(name, size.w, size.h, format, usage);
  TRACE_FUNC_EXIT();
  return pool;
}

MVOID FatImageBufferPool::destroy(android::sp<FatImageBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    if( pool != NULL )
    {
        pool->releaseAll();
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
}

FatImageBufferPool::FatImageBufferPool(const char *name)
    : BufferPool<FatImageBufferHandle>(name)
    , mReady(MFALSE)
    , mWidth(0)
    , mHeight(0)
    , mFormat(eImgFmt_YV12)
    , mUsage(0)
    , mPlane(0)
    , mBufferSize(0)
    , mAllocatorParam(0, 0)
    , mFatHeapNeeded(0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

FatImageBufferPool::~FatImageBufferPool()
{
    TRACE_FUNC_ENTER();
    uninit();
    TRACE_FUNC_EXIT();
}

MUINT32 FatImageBufferPool::preAllocate(MUINT32 count)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mFatHeapNeeded += count;
  TRACE_FUNC_EXIT();
  return count;
}

EImageFormat FatImageBufferPool::getImageFormat() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mFormat;
}

MSize FatImageBufferPool::getImageSize() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MSize(mWidth, mHeight);
}

MBOOL FatImageBufferPool::init(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage)
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;

    if( mReady )
    {
        MY_LOGE("%s: Already init", mName);
    }
    else if( initConfig(width, height, format, usage) )
    {
      mReady = MTRUE;
      ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FatImageBufferPool::initConfig(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage)
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

MBOOL FatImageBufferPool::initAllocatorParam()
{
  TRACE_FUNC_ENTER();
  mAllocatorParam = IImageBufferAllocator::ImgParam(mBufferSize, 0);
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MVOID FatImageBufferPool::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    if( mReady )
    {
        this->releaseAll();
        mFatHeap = NULL;
        mReady = MFALSE;
    }
    TRACE_FUNC_EXIT();
}

sp<FatImageBufferHandle> FatImageBufferPool::doAllocate()
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);

  sp<FatImageBufferHandle> bufferHandle;
  sp<IImageBufferHeap> heap;

  if( !mReady )
  {
    MY_LOGE("%s: pool need init first", mName);
    return NULL;
  }

  if( (bufferHandle = new FatImageBufferHandle(this)) == NULL )
  {
    MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    return NULL;
  }

  if( mFatHeap == NULL || !mFatHeap->isValid() )
  {
    MUINT32 need = mFatHeapNeeded ? mFatHeapNeeded : 1;
    if( (mFatHeap = new FatHeap(mName, mBufferSize, need, mUsage)) == NULL )
    {
      MY_LOGE("OOM: %s: create FatHeap failed", mName);
      return NULL;
    }
    mFatHeapNeeded -= std::min(need, mFatHeap->getCount());
  }
  bufferHandle->mFatHeap = mFatHeap;

  heap = mFatHeap->createDummyHeap();
  if( heap == NULL )
  {
    MY_LOGE("%s: dummy heap create failed", mName);
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
  bufferHandle->mType = FatImageBufferHandle::ALLOCATE;

  TRACE_FUNC_EXIT();
  return bufferHandle;
}

IImageBuffer* FatImageBufferPool::createImageBuffer(const sp<IImageBufferHeap> &heap)
{
  TRACE_FUNC_ENTER();
  IImageBuffer *imageBuffer = NULL;
  if( heap != NULL )
  {
    imageBuffer = heap->createImageBuffer_FromBlobHeap(0, mFormat, MSize(mWidth, mHeight), mStride);
  }
  TRACE_FUNC_EXIT();
  return imageBuffer;
}

MBOOL FatImageBufferPool::doRelease(FatImageBufferHandle *handle)
{
  TRACE_FUNC_ENTER();

  // release should not need lock(mMutex)
  // becuare only BufferPool::releaseAll and
  // BufferPool::recycle calls release for handles for the pool,
  // and no handle can exist when IMemDrv is invalid

  MBOOL ret = MTRUE;

  if( !handle )
  {
    MY_LOGE("%s: FatImageBufferHandle missing", mName);
    ret = MFALSE;
  }
  else
  {
    if( handle->mImageBuffer == NULL )
    {
      MY_LOGE("%s: FatImageBufferHandle::mImageBuffer missing", mName);
      ret = MFALSE;
    }
    else if( handle->mType == FatImageBufferHandle::ALLOCATE )
    {
      if( !handle->mImageBuffer->unlockBuf(mName) )
      {
        MY_LOGE("%s: FatImageBufferHandle unlockBuf failed", mName);
        ret = MFALSE;
      }
    }
    handle->mImageBuffer = NULL;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

class IIBuffer_FatImageBufferHandle : public IIBuffer
{
public:
  IIBuffer_FatImageBufferHandle(sb<FatImageBufferHandle> handle)
    : mHandle(handle)
  {
  }

  virtual ~IIBuffer_FatImageBufferHandle()
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
  sb<FatImageBufferHandle> mHandle;
};

sp<IIBuffer> FatImageBufferPool::requestIIBuffer()
{
  TRACE_FUNC_ENTER();
  sb<FatImageBufferHandle> handle;
  sp<IIBuffer> buffer;
  handle = this->request();
  buffer = new IIBuffer_FatImageBufferHandle(handle);
  TRACE_FUNC_EXIT();
  return buffer;
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
