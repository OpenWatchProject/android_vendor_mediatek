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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_GRAPHIC_BUFFER_POOL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_GRAPHIC_BUFFER_POOL_H_

#include <core/common.h>
//#include <mtkcam/common.h>
#include <utils/imgbuf/IGrallocImageBufferHeap.h>
//#include <mtkcam/utils/common.h>

#include <utils/Mutex.h>

#include "BufferPool.h"
#include "IIBuffer.h"
#include "NativeBufferWrapper.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

class GraphicBufferPool;

class GraphicBufferHandle : public BufferHandle<GraphicBufferHandle>
{
public:
  GraphicBufferHandle(const android::sp<BufferPool<GraphicBufferHandle> > &pool);
  virtual ~GraphicBufferHandle();
  NB_SPTR getGraphicBufferAddr();
  #if 0
  MBOOL resize(MUINT32 width, MUINT32 height);
  MBOOL lock();
  MBOOL unlock();
  #endif

public:
  android::sp<IImageBuffer> mImageBuffer;
  android::sp<NativeBufferWrapper> mGraphicBuffer;

private:
  enum Type { ALLOCATE, REGISTER };
  friend class GraphicBufferPool;
  Type mType;
};
typedef sb<GraphicBufferHandle> SmartGraphicBuffer;

class GraphicBufferPool : public BufferPool<GraphicBufferHandle>
{
public:
  static const MUINT32 USAGE_HW_TEXTURE;
  static const MUINT32 USAGE_HW_RENDER;

public:
  class BufferInfo
  {
  public:
    android::sp<NativeBufferWrapper> mGraphic;
    android::sp<IImageBuffer> mImage;
    MUINT32 mSize;

    BufferInfo()
    {}

    BufferInfo(android::sp<NativeBufferWrapper> graphic, android::sp<IImageBuffer> image, MUINT32 size) : mGraphic(graphic), mImage(image), mSize(size)
    {}
  };

public:
  static android::sp<GraphicBufferPool> create(const char *name, MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT32 usage, NativeBufferWrapper::ColorSpace color = NativeBufferWrapper::NOT_SET);
  static android::sp<GraphicBufferPool> create(const char *name, const MSize &size, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color = NativeBufferWrapper::NOT_SET);
  static android::sp<GraphicBufferPool> create(const char *name, MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color = NativeBufferWrapper::NOT_SET);
  static MVOID destroy(android::sp<GraphicBufferPool> &pool);
  virtual ~GraphicBufferPool();

  virtual EImageFormat getImageFormat() const;
  virtual MSize getImageSize() const;
  virtual android::sp<IIBuffer> requestIIBuffer();
  #if 0
    MBOOL add(const BufferInfo &info);
    MBOOL add(const android::sp<NativeBufferWrapper> &graphic, const android::sp<IImageBuffer> &image, MUINT32 size);
  #endif

protected:
  GraphicBufferPool(const char *name);
  MBOOL init(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color);
  MVOID uninit();
  virtual android::sp<GraphicBufferHandle> doAllocate();
  virtual MBOOL doRelease(GraphicBufferHandle *handle);

private:
  MBOOL initConfig(MUINT32 width, MUINT32 height, EImageFormat format, MUINT32 usage, NativeBufferWrapper::ColorSpace color);

private:
  static EImageFormat toImageFormat(android_pixel_format_t graphFormat);
  static android_pixel_format_t toPixelFormat(EImageFormat format);
  static const char* toStr(EImageFormat format);

private:
  android::Mutex mMutex;
  MBOOL mReady;

private:
  MUINT32 mWidth;
  MUINT32 mHeight;
  EImageFormat mImageFormat;
  android_pixel_format_t mGraphicFormat;
  MUINT32 mImageUsage;
  MUINT32 mGraphicUsage;
  MUINT32 mPlane;
  size_t mStride[3];
  size_t mBoundary[3];
  NativeBufferWrapper::ColorSpace mColorSpace;
  IImageBufferAllocator::ImgParam mAllocatorParam;
  IGrallocImageBufferHeap::AllocExtraParam mAllocatorExtraParam;
};

}
}
}
}
#endif  // _MTK_CAMERA_FEATURE_PIPE_CORE_GRAPHIC_BUFFER_POOL_H_
