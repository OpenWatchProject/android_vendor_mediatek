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
#include "CaptureBufferPool.h"

#define PIPE_CLASS_TAG "Pool"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_NODE

#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_CAPTURE);

using namespace NSCam::NSCamFeature::NSFeaturePipe;

//#define FORCE_GRALLOC MTRUE

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

android_pixel_format_t toPixelFormat(NSCam::EImageFormat fmt)
{
  switch(fmt)
  {
      case eImgFmt_RGBA8888:        return HAL_PIXEL_FORMAT_RGBA_8888;
      case eImgFmt_YV12:            return HAL_PIXEL_FORMAT_YV12;
      case eImgFmt_RAW16:           return HAL_PIXEL_FORMAT_RAW16;
      case eImgFmt_RAW_OPAQUE:      return HAL_PIXEL_FORMAT_RAW_OPAQUE;
      case eImgFmt_BLOB:            return HAL_PIXEL_FORMAT_BLOB;
      case eImgFmt_RGBX8888:        return HAL_PIXEL_FORMAT_RGBX_8888;
      case eImgFmt_RGB888:          return HAL_PIXEL_FORMAT_RGB_888;
      case eImgFmt_RGB565:          return HAL_PIXEL_FORMAT_RGB_565;
      case eImgFmt_BGRA8888:        return HAL_PIXEL_FORMAT_BGRA_8888;
      case eImgFmt_YUY2:            return HAL_PIXEL_FORMAT_YCbCr_422_I;
      case eImgFmt_NV16:            return HAL_PIXEL_FORMAT_YCbCr_422_SP;
      case eImgFmt_NV21:            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
      case eImgFmt_NV12:            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
      case eImgFmt_Y8:              return HAL_PIXEL_FORMAT_Y8;
      case eImgFmt_Y16:             return HAL_PIXEL_FORMAT_Y16;
      default:
        return (android_pixel_format)-1;
  };
}

TuningBufferHandle::TuningBufferHandle(const sp<BufferPool<TuningBufferHandle> > &pool)
    : BufferHandle(pool)
    , mpVA(0)
{
}

sp<TuningBufferPool> TuningBufferPool::create(const char *name, MUINT32 size)
{
    TRACE_FUNC_ENTER();
    sp<TuningBufferPool> pool = new TuningBufferPool(name);
    if (pool == NULL) {
        MY_LOGE("OOM: Cannot create TuningBufferPool!");
    } else if (!pool->init(size)) {
        MY_LOGE("Pool initialization failed!");
        pool = NULL;
    }

    TRACE_FUNC_EXIT();
    return pool;
}

MVOID TuningBufferPool::destroy(android::sp<TuningBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    if (pool != NULL) {
        pool->releaseAll();
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
}

TuningBufferPool::TuningBufferPool(const char* name)
    : BufferPool<TuningBufferHandle>(name)
    , muBufSize(0)
{
}

TuningBufferPool::~TuningBufferPool()
{
    uninit();
}

MBOOL TuningBufferPool::init(MUINT32 size)
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);

    if (size <= 0)
        return MFALSE;

    muBufSize = size;

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID TuningBufferPool::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    this->releaseAll();
    TRACE_FUNC_EXIT();
}

sp<TuningBufferHandle> TuningBufferPool::doAllocate()
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);
    sp<TuningBufferHandle> handle = new TuningBufferHandle(this);

    if (handle == NULL) {
        MY_LOGE("TuningBufferHandle create failed!");
        return NULL;
    }

    handle->mpVA = (MVOID*) malloc(muBufSize);

    if (handle->mpVA == NULL) {
        MY_LOGE("Out of memory!!");
        return NULL;
    }
    // clear memory
    memset((void*)handle->mpVA, 0, muBufSize);

    TRACE_FUNC_EXIT();

    return handle;
}

MBOOL TuningBufferPool::doRelease(TuningBufferHandle* handle)
{
    TRACE_FUNC_ENTER();

    if (handle->mpVA != NULL) {
        free(handle->mpVA);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

CaptureBufferPool::CaptureBufferPool(const char *name, MUINT32 uLogLevel)
    : mName(name)
    , mbInit(MFALSE)
    , muTuningBufSize(0)
    , mLogLevel(uLogLevel)

{
}

CaptureBufferPool::~CaptureBufferPool()
{
    uninit();
}

MBOOL CaptureBufferPool::init(Vector<BufferConfig> vBufConfig)
{
    Mutex::Autolock _l(mPoolLock);

    if (mbInit) {
        MY_LOGE("do init when it's already init!");
        return MFALSE;
    }

    mvImagePools.clear();

    for (auto& bufConf : vBufConfig) {

        MY_LOGD("[%s] s:%dx%d f:%d min:%d max:%d",
            bufConf.name,
            bufConf.width, bufConf.height, bufConf.format,
            bufConf.minCount,
            bufConf.maxCount
        );

        // SmartBuffer
        sp<IBufferPool> pImagePool;
#ifdef FORCE_GRALLOC
        MBOOL bGraphicBuffer = MTRUE;
#else
        MBOOL bGraphicBuffer = MFALSE;
#endif
        // internal format don't support Graphic buffer
        android_pixel_format_t pixelFmt;
        if (bGraphicBuffer && (MINT32)(pixelFmt = toPixelFormat((EImageFormat)bufConf.format)) == -1) {
            bGraphicBuffer = MFALSE;
        }
        if (bGraphicBuffer) {
            pImagePool = GraphicBufferPool::create(
                bufConf.name,
                bufConf.width,
                bufConf.height,
                pixelFmt,
                GraphicBufferPool::USAGE_HW_TEXTURE);
        } else {
            pImagePool = ImageBufferPool::create(
                bufConf.name,
                bufConf.width, bufConf.height,
                (EImageFormat)bufConf.format,
                ImageBufferPool::USAGE_HW_AND_SW, MFALSE);
        }

        if (pImagePool == nullptr) {
            MY_LOGE("create [%s] failed!", bufConf.name);
            return MFALSE;
        }
        mvImagePools.add(
            make_tuple(bufConf.width, bufConf.height, bufConf.format),
            pImagePool);
    }

    // tuning buffer pools
    {
        mvTuningBufferPools.clear();
        muTuningBufSize = NSCam::NSIoPipe::NSPostProc::INormalStream::getRegTableSize();
    }

    mbInit = MTRUE;
    return MTRUE;
}


MBOOL CaptureBufferPool::uninit()
{
    Mutex::Autolock _l(mPoolLock);

    if (!mbInit) {
        MY_LOGE("do uninit when it's not init yet!");
        return MFALSE;
    }

    // image buffers release
    for (size_t i = 0; i < mvImagePools.size(); i++) {
        IBufferPool::destroy(mvImagePools.editValueAt(i));
    }
    mvImagePools.clear();

    // tuning buffer pool
    for (size_t i = 0; i < mvTuningBufferPools.size(); i++) {
        TuningBufferPool::destroy(mvTuningBufferPools.editValueAt(i));
    }
    mvTuningBufferPools.clear();

    mbInit = MFALSE;
    return MTRUE;
}

MBOOL CaptureBufferPool::allocate()
{

    MBOOL ret = MTRUE;
    MY_LOGD("ret:%d", ret);
    return ret;
}

SmartTuningBuffer CaptureBufferPool::getTuningBuffer()
{
    return getTuningBuffer(muTuningBufSize);
}

SmartTuningBuffer CaptureBufferPool::getTuningBuffer(MUINT32 size)
{
    android::sp<TuningBufferPool> pPool;
    {
        Mutex::Autolock _l(mPoolLock);

        if (mvTuningBufferPools.indexOfKey(size) < 0) {
            auto pTuningBufferPool = TuningBufferPool::create(mName, size);
            pTuningBufferPool->setAutoAllocate(8);
        #if MTKCAM_LOW_MEM
            pTuningBufferPool->setAutoFree(0);
        #else
            pTuningBufferPool->setAutoFree(6);
        #endif
            mvTuningBufferPools.add(size, pTuningBufferPool);
        }
        pPool = mvTuningBufferPools.valueFor(size);
    }

    SmartTuningBuffer pBuf = pPool->request();
    memset((void*)pBuf->mpVA, 0, size);
    return pBuf;
}

android::sp<IIBuffer> CaptureBufferPool::getImageBuffer(const MSize& size, MUINT32 format, MSize align, MBOOL bGraphicBuffer)
{
    sp<IBufferPool> pImagePool;
    {
        Mutex::Autolock _l(mPoolLock);

        auto ceil = [](MUINT32 val, MUINT32 align) {
            if (align) {
                if (align & (align -1))
                    MY_LOGE("not valid alignment(%d) to value(%d)", align, val);
                else
                    return (val + align - 1) & ~(align - 1);
            }
            return val;
        };

#ifdef FORCE_GRALLOC
        bGraphicBuffer = MTRUE;
#endif
        // internal format don't support Graphic buffer
        android_pixel_format_t pixelFmt;
        if (bGraphicBuffer && (MINT32)(pixelFmt = toPixelFormat((EImageFormat)format)) == -1) {
            bGraphicBuffer = MFALSE;
        }

        PoolKey_T poolKey =  make_tuple(size.w << 16 | size.h, align.w << 16 | align.h, format << 1 | bGraphicBuffer);

        if (mvImagePools.indexOfKey(poolKey) < 0) {
            MBOOL bUseSingleBuffer = (format == eImgFmt_I422 || format == eImgFmt_NV12);
            MUINT32 w = ceil(size.w, align.w);
            MUINT32 h = ceil(size.h, align.h);

            if ((MUINT32)size.w != w || (MUINT32)size.h != h) {
                MY_LOGD("alloc buffer Pool: format(%d), (%d,%d)=>(%d,%d)", format, size.w, size.h, w, h);
            }

            android::sp<IBufferPool> pImagePool;

            if (bGraphicBuffer) {
                pImagePool = GraphicBufferPool::create(
                    "CapturePipe", w, h, pixelFmt,
                    GraphicBufferPool::USAGE_HW_TEXTURE);
            } else {
                pImagePool = ImageBufferPool::create(
                    "CapturePipe", w, h, (EImageFormat)format,
                    ImageBufferPool::USAGE_HW_AND_SW, bUseSingleBuffer);
            }

            if (pImagePool == nullptr) {
                MY_LOGE("create buffer pool failed!");
                return MFALSE;
            }

            pImagePool->setAutoAllocate(18);
    #if MTKCAM_LOW_MEM
            pImagePool->setAutoFree(0);
    #else
            pImagePool->setAutoFree(6);
    #endif
            mvImagePools.add(poolKey, pImagePool);
        }

        pImagePool = mvImagePools.valueFor(poolKey);
    }

    android::sp<IIBuffer> pBuf = pImagePool->requestIIBuffer();

    if (pBuf != NULL) {
        auto pImgBuf = pBuf->getImageBuffer();
        if (pImgBuf != NULL) {
            if (align.w != 0 || align.h != 0) {
                pImgBuf->setExtParam(size);
            }
            // invalidate working buffer before using it
            pImgBuf->syncCache(eCACHECTRL_INVALID);
        } else {
            MY_LOGE("get ImageBuffer fail!");
        }
    }

    MY_LOGD_IF(mLogLevel, "Buffer Pool: Size(%dx%d) Format(%d) Align(%dx%d) Peak(%d) Available(%d) bGraphicBuffer(%d)",
         size.w, size.h, format,
         align.w, align.h,
         pImagePool->peakPoolSize(),
         pImagePool->peakAvailableSize(),
         bGraphicBuffer);

    return pBuf;

}


} // NSCapture
} // NSFeaturePipe
} // NSCamFeature
} // NSCam
