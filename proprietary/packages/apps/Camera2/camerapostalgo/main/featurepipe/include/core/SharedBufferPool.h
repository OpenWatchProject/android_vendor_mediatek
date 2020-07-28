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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_SHARED_BUFFER_POOL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_SHARED_BUFFER_POOL_H_

#include "BufferPool.h"
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <map>


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {


class SharedBufferPool;

class SharedBufferHandle : public BufferHandle<SharedBufferHandle>
{
public:
  SharedBufferHandle(const android::sp<BufferPool<SharedBufferHandle>> &pool);
  virtual ~SharedBufferHandle();

private:
  android::sp<IIBuffer> mBuffer;
  unsigned mOwnerID = 0;

private:
  friend class SharedBufferPool;
  friend class IIBuffer_SharedBufferHandle;
};

class SharedBufferPool : public BufferPool<SharedBufferHandle>
{
public:
  static android::sp<SharedBufferPool> create(const char *name, const sp<IBufferPool> &bufferPool);
  static MVOID destroy(android::sp<SharedBufferPool> &pool);
  virtual ~SharedBufferPool();

  virtual EImageFormat getImageFormat() const;
  virtual MSize getImageSize() const;
  virtual android::sp<IIBuffer> requestIIBuffer();
  virtual android::sp<IIBuffer> requestIIBuffer(unsigned ownerID);

public:
  MVOID addQuota(unsigned ownerID, unsigned count);
  MBOOL changeOwner(unsigned newID, const android::sp<IIBuffer> &buffer);
  std::map<unsigned, unsigned> peakQuota() const;
  unsigned peakQuota(unsigned ownerID) const;

protected:
  SharedBufferPool(const char *name, const sp<IBufferPool> &pool);
  virtual android::sp<SharedBufferHandle> doAllocate();
  virtual MBOOL doRelease(SharedBufferHandle *handle);

protected:
  virtual MVOID recycle(SharedBufferHandle *handle);

private:
  android::sp<IIBuffer> requestSharedIIBuffer(unsigned id);

private:
  using OWNER_ID_T = unsigned;

private:
  sp<IBufferPool> mPool;
  mutable android::Mutex mMutex;
  android::Condition mCondition;
  std::map<OWNER_ID_T, unsigned> mQuotaMap;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
}
#endif  // _MTK_CAMERA_FEATURE_PIPE_CORE_SHARED_BUFFER_POOL_H_
