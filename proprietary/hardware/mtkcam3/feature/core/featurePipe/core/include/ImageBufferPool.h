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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_IMAGE_BUFFER_POOL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_IMAGE_BUFFER_POOL_H_

#include "MtkHeader.h"
//#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>

#include <utils/Mutex.h>

#include "BufferPool.h"
#include "IIBuffer.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class ImageBufferPool;

class ImageBufferHandle : public BufferHandle<ImageBufferHandle>
{
public:
  ImageBufferHandle(const android::sp<BufferPool<ImageBufferHandle> > &pool);
  virtual ~ImageBufferHandle();
  #if 0
  MBOOL resize(MUINT32 width, MUINT32 height);
  #endif

public:
  android::sp<IImageBuffer> mImageBuffer;

private:
  friend class ImageBufferPool;
  enum Type { ALLOCATE, REGISTER };
  Type mType;
  MINT mUsage;
};
typedef sb<ImageBufferHandle> SmartImageBuffer;

class ImageBufferPool : public BufferPool<ImageBufferHandle>
{
public:
  static const MUINT32 USAGE_HW;
  static const MUINT32 USAGE_SW;
  static const MUINT32 USAGE_HW_AND_SW;
  static const MBOOL   SEPARATE_BUFFER;

public:
  static android::sp<ImageBufferPool> create(const char* name, MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, MBOOL singleBuffer = MTRUE);
  static android::sp<ImageBufferPool> create(const char* name, const MSize &size, EImageFormat format, MUINT32 usage, MBOOL singleBuffer = MTRUE);
  static MVOID destroy(android::sp<ImageBufferPool> &pool);
  virtual ~ImageBufferPool();

  virtual EImageFormat getImageFormat() const;
  virtual MSize getImageSize() const;
  virtual android::sp<IIBuffer> requestIIBuffer();
  MBOOL add(const android::sp<IImageBuffer> &image);

protected:
  ImageBufferPool(const char* name);
  MBOOL init(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, MBOOL singleBuffer);
  MVOID uninit();
  virtual android::sp<ImageBufferHandle> doAllocate();
  virtual MBOOL doRelease(ImageBufferHandle *handle);

private:
  MBOOL initConfig(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, MBOOL singleBuffer);
  MBOOL initAllocatorParam();
  IImageBuffer* createImageBuffer(const android::sp<IImageBufferHeap> &heap);

private:
  android::Mutex mMutex;
  MBOOL mReady;

private:
  MUINT32 mWidth;
  MUINT32 mHeight;
  EImageFormat mFormat;
  MUINT32 mUsage;
  MUINT32 mPlane;
  size_t mStride[3] = {0};
  size_t mBoundary[3];
  MBOOL mUseSingleBuffer;
  MUINT32 mBufferSize;
  IImageBufferAllocator::ImgParam mAllocatorParam;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
#endif  // _MTK_CAMERA_FEATURE_PIPE_CORE_IMAGE_BUFFER_POOL_H_
