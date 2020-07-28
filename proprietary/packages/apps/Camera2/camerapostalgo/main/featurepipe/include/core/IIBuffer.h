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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_IIBUFFER_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_IIBUFFER_H_

#include <core/common.h>
#include <utils/imgbuf/IImageBuffer.h>

#include "NativeBufferWrapper.h"

#include <utils/RefBase.h>

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

class SharedBufferPool;

class IIBuffer : public virtual android::RefBase
{
public:
  IIBuffer();
  virtual ~IIBuffer();

  virtual android::sp<IImageBuffer> getImageBuffer() const = 0;

  IImageBuffer* getImageBufferPtr() const;
  MSize getImgSize() const;
  MBOOL setExtParam(const MSize &size);
  MINT64 getTimestamp() const;
  MVOID setTimestamp(MINT64 timestamp);
  MBOOL syncCache(NSCam::eCacheCtrl ctrl);
  MBOOL erase();
  virtual NB_SPTR getGraphicBufferAddr();

private:
  // SharedBuffer APIs
  friend class SharedBufferPool;
  virtual MBOOL getOwnerID(const SharedBufferPool *pool, unsigned &ownerID) const;
  virtual MBOOL changeOwnerID(const SharedBufferPool *pool, unsigned newID, unsigned &oldID);

};

class IIBuffer_IImageBuffer : public IIBuffer
{
public:
  IIBuffer_IImageBuffer(IImageBuffer *buffer);
  virtual ~IIBuffer_IImageBuffer();
  virtual android::sp<IImageBuffer> getImageBuffer() const;

private:
  IImageBuffer *mBuffer;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
}

#endif  // _MTK_CAMERA_FEATURE_PIPE_CORE_IIBUFFER_H_
