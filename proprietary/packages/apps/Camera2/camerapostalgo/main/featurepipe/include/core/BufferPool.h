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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_BUFFER_POOL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_BUFFER_POOL_H_

#include "BufferPool_t.h"
#include "BufferHandle.h"

#include <algorithm>
#include <functional>

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_BUFFER_POOL
#define PIPE_CLASS_TAG "BufferPool"
#include "PipeLog.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

template <typename T>
BufferPool<T>::BufferPool(const char *name)
  : mName(name)
  , mAutoFree(-1)
  , mAutoAllocate(-1)
  , mAllocatingCount(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("trackpool: create pool<%s=%p>", mName, this);
  TRACE_FUNC_EXIT();
}

template <typename T>
BufferPool<T>::~BufferPool()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("trackpool: destroy pool<%s=%p>", mName, this);
  TRACE_FUNC_EXIT();
}

template <typename T>
MUINT32 BufferPool<T>::preAllocate(MUINT32 count)
{
  TRACE_FUNC_ENTER();
  (void)count;
  TRACE_FUNC_EXIT();
  return 0;
}

template <typename T>
MUINT32 BufferPool<T>::allocate()
{
  TRACE_FUNC_ENTER();
  MUINT32 result = 0;
  android::sp<T> handle = this->doAllocate();
  if( handle != NULL )
  {
    this->addToPool(handle);
    result = 1;
  }
  TRACE_FUNC_EXIT();
  return result;
}

template <typename T>
MUINT32 BufferPool<T>::allocate(MUINT32 count)
{
  TRACE_FUNC_ENTER();
  MUINT32 result, i;
  result = 0;
  this->preAllocate(count);
  for( i = 0; i < count; ++i )
  {
    android::sp<T> handle = this->doAllocate();
    if( handle != NULL )
    {
      this->addToPool(handle);
      ++result;
    }
  }
  TRACE_FUNC_EXIT();
  return result;
}

template <typename T>
sb<T> BufferPool<T>::request()
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);
  sb<T> handle = NULL;

  do
  {
    if( !mAvailable.empty() )
    {
      handle = mAvailable.front();
      mAvailable.pop();
      break;
    }
    else if( mAutoAllocate > 0 &&
            ((mPool.size() + mAllocatingCount) < (MUINT32)mAutoAllocate) )
    {
      ++mAllocatingCount;
      mMutex.unlock();
      if( !this->allocate() )
      {
        MY_LOGE("%s: Auto allocate attemp failed", mName);
      }
      mMutex.lock();
      --mAllocatingCount;
    }
    else
    {
      mCondition.wait(mMutex);
    }
  }while(true);

  TRACE_FUNC_EXIT();
  return handle;
}

template <typename T>
MUINT32 BufferPool<T>::peakPoolSize() const
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  TRACE_FUNC_EXIT();
  return mPool.size();
}

template <typename T>
MUINT32 BufferPool<T>::peakAvailableSize() const
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  TRACE_FUNC_EXIT();
  return mAvailable.size();
}

template <typename T>
MVOID BufferPool<T>::setAutoAllocate(MINT32 bound)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mAutoAllocate = bound;
  TRACE_FUNC_EXIT();
}

template <typename T>
MVOID BufferPool<T>::setAutoFree(MINT32 bound)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mAutoFree = bound;
  autoFree_locked();
  TRACE_FUNC_EXIT();
}

template <typename T>
EImageFormat BufferPool<T>::getImageFormat() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return eImgFmt_UNKNOWN;
}

template <typename T>
MSize BufferPool<T>::getImageSize() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MSize(0, 0);
}

template <typename T>
sp<IIBuffer> BufferPool<T>::requestIIBuffer()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return NULL;
}

template <typename T>
typename BufferPool<T>::CONTAINER_TYPE BufferPool<T>::getPoolContents() const
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  BufferPool<T>::CONTAINER_TYPE temp(mPool.begin(), mPool.end());
  TRACE_FUNC_EXIT();
  return temp;
}

template <typename T>
MVOID BufferPool<T>::addToPool(const android::sp<T> &handle)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mPool.push_back(handle);
  mAvailable.push(handle);
  mCondition.broadcast();
  TRACE_FUNC_EXIT();
}

template <typename T>
MVOID BufferPool<T>::releaseAll()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  typename BufferPool<T>::POOL_TYPE::iterator it, end;

  TRACE_FUNC("trackpool: release pool<%s=%p>", mName, this);

  if( mAvailable.size() != mPool.size())
  {
    MY_LOGE("%s: Some buffers are not released before pool released",mName ? mName:"unknown pool");
  }

  for( it = mPool.begin(), end = mPool.end(); it != end; ++it )
  {
    (*it)->mTrack = MFALSE;
  }

  while(!mAvailable.empty())
  {
    this->doRelease(mAvailable.front().get());
    mAvailable.pop();
  }

  mPool.clear();
  TRACE_FUNC_EXIT();
}

template <typename T>
MVOID BufferPool<T>::recycle(T *handle)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  if( handle && handle->mTrack )
  {
    // TODO: check handle is from this pool
    if( mAutoFree >= 0 &&
        mPool.size() > (unsigned)mAutoFree )
    {
      freeFromPool_locked(handle);
    }
    else
    {
      mAvailable.push(handle);
    }
    mCondition.broadcast();
  }
  else
  {
    this->doRelease(handle);
  }
  TRACE_FUNC_EXIT();
}

template <typename T>
MBOOL BufferPool<T>::freeFromPool_locked(android::sp<T> handle)
{
  // mMutex must be locked
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  typename POOL_TYPE::iterator it;

  if( handle != NULL )
  {
    it = std::find(mPool.begin(), mPool.end(), handle);
    if( it != mPool.end() )
    {
      mPool.erase(it);
      this->doRelease(handle.get());
      ret = MTRUE;
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename T>
MVOID BufferPool<T>::autoFree_locked()
{
  // mMutex must be locked
  TRACE_FUNC_ENTER();
  MUINT32 count;
  if( mAutoFree >= 0 )
  {
    count = mPool.size();
    while( (count > (MUINT32)mAutoFree) &&
           !mAvailable.empty() )
    {
      freeFromPool_locked(mAvailable.front());
      mAvailable.pop();
      --count;
    }
  }
  TRACE_FUNC_EXIT();
}

template <typename T>
const char* BufferPool<T>::getName() const
{
    return mName;
}

};
};
};
};

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_BUFFER_POOL_H_
