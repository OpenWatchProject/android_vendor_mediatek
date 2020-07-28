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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "../include/SharedBufferPool.h"

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_SHARED_BUFFER_POOL
#define PIPE_CLASS_TAG "SharedBufferPool"
#include "../include/PipeLog.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

#define TRACE_QUOTA (TRACE_SHARED_BUFFER_POOL||TRACE_SHARED_BUFFER_POOL_QUOTA)

using Autolock = android::Mutex::Autolock;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

SharedBufferHandle::SharedBufferHandle(const android::sp<BufferPool<SharedBufferHandle>> &pool)
  : BufferHandle(pool)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

SharedBufferHandle::~SharedBufferHandle()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

android::sp<SharedBufferPool> SharedBufferPool::create(const char *name, const sp<IBufferPool> &bufferPool)
{
  TRACE_FUNC_ENTER();
  sp<SharedBufferPool> pool = new SharedBufferPool(name, bufferPool);
  TRACE_FUNC_EXIT();
  return pool;
}

MVOID SharedBufferPool::destroy(android::sp<SharedBufferPool> &pool)
{
  TRACE_FUNC_ENTER();
  if( pool != NULL )
  {
    pool->releaseAll();
    pool = NULL;
  }
  TRACE_FUNC_EXIT();
}

SharedBufferPool::SharedBufferPool(const char* name, const sp<IBufferPool> &pool)
  : BufferPool<SharedBufferHandle>(name)
  , mPool(pool)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

SharedBufferPool::~SharedBufferPool()
{
  TRACE_FUNC_ENTER();
  this->releaseAll();
  TRACE_FUNC_EXIT();
}

EImageFormat SharedBufferPool::getImageFormat() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mPool ? mPool->getImageFormat() : eImgFmt_UNKNOWN;
}

MSize SharedBufferPool::getImageSize() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mPool ? mPool->getImageSize() : MSize();
}

android::sp<IIBuffer> SharedBufferPool::requestIIBuffer()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return requestIIBuffer(0);
}

android::sp<IIBuffer> SharedBufferPool::requestIIBuffer(unsigned ownerID)
{
  TRACE_FUNC_ENTER();
  android::sp<IIBuffer> buffer;
  if( mPool != NULL )
  {
    {
      Autolock lock(mMutex);
      while( mQuotaMap[ownerID] <= 0 )
      {
        mCondition.wait(mMutex);
      }
      --mQuotaMap[ownerID];
      MY_LOGD_IF(TRACE_QUOTA, "quota[%d]-- =>%d", ownerID, mQuotaMap[ownerID]);
    }
    buffer = requestSharedIIBuffer(ownerID);
  }
  TRACE_FUNC_EXIT();
  return buffer;
}

MVOID SharedBufferPool::addQuota(unsigned ownerID, unsigned count)
{
  TRACE_FUNC_ENTER();
  Autolock lock(mMutex);
  mQuotaMap[ownerID] += count;
  MY_LOGD_IF(TRACE_QUOTA, "quota[%d]+%d =>%d", ownerID, count, mQuotaMap[ownerID]);
  mCondition.broadcast();
  TRACE_FUNC_EXIT();
}

MBOOL SharedBufferPool::changeOwner(unsigned newID, const android::sp<IIBuffer> &buffer)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  unsigned oldID;
  if( buffer != NULL && buffer->getOwnerID(this, oldID) )
  {
    ret = MTRUE;
    if( newID != oldID )
    {
      Autolock lock(mMutex);
      while( mQuotaMap[newID] <= 0 )
      {
        mCondition.wait(mMutex);
      }
      if( buffer->changeOwnerID(this, newID, oldID) )
      {
        --mQuotaMap[newID];
        MY_LOGD_IF(TRACE_QUOTA, "quota[%d]-- =>%d", newID, mQuotaMap[newID]);
        ++mQuotaMap[oldID];
        MY_LOGD_IF(TRACE_QUOTA, "quota[%d]++ =>%d", oldID, mQuotaMap[oldID]);
        mCondition.broadcast();
      }
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

std::map<unsigned, unsigned> SharedBufferPool::peakQuota() const
{
  TRACE_FUNC_ENTER();
  Autolock lock(mMutex);
  TRACE_FUNC_EXIT();
  return mQuotaMap;
}

unsigned SharedBufferPool::peakQuota(unsigned ownerID) const
{
  TRACE_FUNC_ENTER();
  Autolock lock(mMutex);
  unsigned count = 0;
  std::map<unsigned, unsigned>::const_iterator it = mQuotaMap.find(ownerID);
  if( it != mQuotaMap.end() )
  {
      count = it->second;
  }
  TRACE_FUNC_EXIT();
  return count;
}

sp<SharedBufferHandle> SharedBufferPool::doAllocate()
{
  TRACE_FUNC_ENTER();
  sp<SharedBufferHandle> handle;
  sp<IIBuffer> buffer;

  if( mPool == NULL )
  {
    MY_LOGE("%s: invalid: physical pool = NULL", mName);
    return NULL;
  }
  if( (handle = new SharedBufferHandle(this)) == NULL )
  {
    MY_LOGE("OOM: %s: create bufferHandle failed", mName);
    return NULL;
  }
  mPool->allocate();
  buffer = mPool->requestIIBuffer();
  if( buffer == NULL )
  {
    MY_LOGE("%s: mPool->requestIIBuffer failed", mName);
    return NULL;
  }
  handle->mBuffer = buffer;
  TRACE_FUNC_EXIT();
  return handle;
}

MBOOL SharedBufferPool::doRelease(SharedBufferHandle *handle)
{
  TRACE_FUNC_ENTER();

  // release should not need lock(mMutex)
  // because only BufferPool::releaseAll and
  // BufferPool::recycle calls release for handles for the pool,

  MBOOL ret = MTRUE;

  if( !handle )
  {
    MY_LOGE("%s: SharedBufferHandle missing", mName);
    ret = MFALSE;
  }
  else
  {
    if( handle->mBuffer == NULL )
    {
      MY_LOGE("%s: SharedBufferHandle::mBuffer missing", mName);
      ret = MFALSE;
    }
    handle->mBuffer = NULL;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MVOID SharedBufferPool::recycle(SharedBufferHandle *handle)
{
  TRACE_FUNC_ENTER();
  if( handle )
  {
    unsigned ownerID = handle->mOwnerID;
    {
      Autolock lock(mMutex);
      ++mQuotaMap[ownerID];
      MY_LOGD_IF(TRACE_QUOTA, "quota[%d]++ =>%d", ownerID, mQuotaMap[ownerID]);
      mCondition.broadcast();
    }
  }
  BufferPool<SharedBufferHandle>::recycle(handle);
  TRACE_FUNC_EXIT();
}

class IIBuffer_SharedBufferHandle : public IIBuffer
{
public:
  IIBuffer_SharedBufferHandle(sb<SharedBufferHandle> handle, unsigned ownerID)
    : mHandle(handle)
  {
    if( mHandle != NULL )
    {
      mHandle->mOwnerID = ownerID;
    }
  }

  virtual ~IIBuffer_SharedBufferHandle()
  {
  }

  virtual sp<IImageBuffer> getImageBuffer() const
  {
    sp<IImageBuffer> buffer;
    if( mHandle != NULL && mHandle->mBuffer != NULL )
    {
      buffer = mHandle->mBuffer->getImageBuffer();
    }
    return buffer;
  }

private:
  virtual MBOOL getOwnerID(const SharedBufferPool *pool, unsigned &ownerID) const
  {
    MBOOL ret = MFALSE;
    if( mHandle != NULL && mHandle->verifyPool(pool) )
    {
      ownerID = mHandle->mOwnerID;
      ret = MTRUE;
    }
    return ret;
  }

  virtual MBOOL changeOwnerID(const SharedBufferPool *pool, unsigned newID, unsigned &oldID)
  {
    MBOOL ret = MFALSE;
    if( mHandle != NULL && mHandle->verifyPool(pool) )
    {
      oldID = mHandle->mOwnerID;
      mHandle->mOwnerID = newID;
      ret = MTRUE;
    }
    return ret;
  }

private:
  sb<SharedBufferHandle> mHandle;
};

sp<IIBuffer> SharedBufferPool::requestSharedIIBuffer(unsigned ownerID)
{
  TRACE_FUNC_ENTER();
  sb<SharedBufferHandle> handle;
  sp<IIBuffer> buffer;
  handle = this->request();
  buffer = new IIBuffer_SharedBufferHandle(handle, ownerID);
  TRACE_FUNC_EXIT();
  return buffer;
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
