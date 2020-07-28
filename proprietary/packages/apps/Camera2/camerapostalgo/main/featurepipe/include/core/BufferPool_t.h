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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_BUFFER_POOL_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_BUFFER_POOL_T_H_

#include <core/common.h>

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

#include <algorithm>
#include <functional>
#include <list>
#include <queue>
#include <vector>

#include "BufferHandle.h"
#include "SmartBuffer.h"
#include "IIBuffer.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

class IBufferPool : public virtual android::RefBase
{
public:
  IBufferPool();
  virtual ~IBufferPool();
  virtual const char* getName() const = 0;
  virtual MUINT32 preAllocate(MUINT32 count) = 0;
  virtual MUINT32 allocate() = 0;
  virtual MUINT32 allocate(MUINT32 count) = 0;
  virtual MUINT32 peakPoolSize() const = 0;
  virtual MUINT32 peakAvailableSize() const = 0;
  virtual MVOID setAutoAllocate(MINT32 bound) = 0;
  virtual MVOID setAutoFree(MINT32 bound) = 0;

  virtual EImageFormat getImageFormat() const = 0;
  virtual MSize getImageSize() const = 0;
  virtual android::sp<IIBuffer> requestIIBuffer() = 0;

  static MVOID destroy(android::sp<IBufferPool> &pool);

protected:
  virtual MVOID releaseAll() = 0;
};

template <typename T>
class BufferPool : public virtual IBufferPool
{
public:
  typedef typename std::vector<android::sp<T> > CONTAINER_TYPE;

  BufferPool(const char *mName);
  virtual ~BufferPool();
  virtual MUINT32 preAllocate(MUINT32 count);
  virtual MUINT32 allocate();
  virtual MUINT32 allocate(MUINT32 count);
  sb<T> request();
  virtual MUINT32 peakPoolSize() const;
  virtual MUINT32 peakAvailableSize() const;
  virtual MVOID setAutoAllocate(MINT32 bound);
  virtual MVOID setAutoFree(MINT32 bound);
  virtual EImageFormat getImageFormat() const;
  virtual MSize getImageSize() const;
  virtual android::sp<IIBuffer> requestIIBuffer();

  const char* getName() const;
  // Get pool internal, be careful
  CONTAINER_TYPE getPoolContents() const;

protected:
  MVOID addToPool(const android::sp<T> &handle);
  virtual MVOID releaseAll();
  virtual android::sp<T> doAllocate() = 0;
  virtual MBOOL doRelease(T *handle) = 0;

protected:
  friend class BufferHandle<T>;
  virtual MVOID recycle(T *handle);

private:
  MBOOL freeFromPool_locked(android::sp<T> handle);
  MVOID autoFree_locked();

protected:
  const char *mName;

private:
  typedef typename std::list<android::sp<T> > POOL_TYPE;

  mutable android::Mutex mMutex;
  android::Condition mCondition;
  POOL_TYPE mPool;
  std::queue<android::sp<T> > mAvailable;
  MINT32 mAutoFree;
  MINT32 mAutoAllocate;
  MINT32 mAllocatingCount;
};

};
};
};
};

#endif  // _MTK_CAMERA_FEATURE_PIPE_CORE_BUFFER_POOL_T_H_
