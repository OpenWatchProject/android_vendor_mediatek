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
#define LOG_TAG "test"

#include <utils/StrongPointer.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/UITypes.h>
#include <mtkcam/def/ImageFormat.h>

#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include "ImageBufferManager.h"
//
#define ALIGN(w, a) (((w + (a-1)) / a) * a)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
using namespace NSCam;
using namespace NSCam::Utils::Format;
#define BUFFER_USAGE_SW (eBUFFER_USAGE_SW_READ_OFTEN | \
                         eBUFFER_USAGE_SW_WRITE_OFTEN | \
                         eBUFFER_USAGE_HW_CAMERA_READWRITE)
//
MBOOL
ImageBufferManager::
allocBuffer(
    sp<IImageBuffer>& imageBuf,
    MUINT32 w,
    MUINT32 h,
    MUINT32 format,
    MBOOL isContinuous
)
{
    MBOOL ret = MTRUE;

    sp<IImageBuffer> pBuf;

    MUINT32 planeCount = queryPlaneCount(format);

    MINT32 bufBoundaryInBytes[3] = {0};
    MUINT32 strideInBytes[3] = {0};
    for (MUINT32 i = 0; i < planeCount; i++)
    {
        strideInBytes[i] =
            (queryPlaneWidthInPixels(format, i, w) *
             queryPlaneBitsPerPixel(format, i) + 7) / 8;

        MY_LOGD("allocBuffer strideInBytes[%d] strideInBytes(%d)",
                i, strideInBytes[i]);
    }

    if (isContinuous)
    {
        // to avoid non-continuous multi-plane memory,
        // allocate memory in blob format and map it to ImageBuffer
        MUINT32 allPlaneSize = 0;
        for (MUINT32 i = 0; i < planeCount; i++)
        {
            allPlaneSize +=
                ((queryPlaneWidthInPixels(format, i, w) *
                  queryPlaneBitsPerPixel(format, i) + 7) / 8) *
                queryPlaneHeightInPixels(format, i, h);
        }
        MY_LOGD("allocBuffer all plane size(%d)", allPlaneSize);

        // allocate blob buffer
        IImageBufferAllocator::ImgParam blobParam(allPlaneSize, bufBoundaryInBytes[0]);

        IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
        sp<IImageBuffer> tmpImageBuffer = allocator->alloc(LOG_TAG, blobParam);
        if (tmpImageBuffer == NULL)
        {
            MY_LOGE("tmpImageBuffer is NULL");
            return MFALSE;
        }

        // NOTE: after sp holds the allocated buffer, free can be called anywhere
        allocator->free(tmpImageBuffer.get());

        if (!tmpImageBuffer->lockBuf(LOG_TAG, BUFFER_USAGE_SW))
        {
            MY_LOGE("lock Buffer failed");
            return MFALSE;
        }

        // encapsulate tmpImageBuffer into external ImageBuffer
        IImageBufferAllocator::ImgParam extParam(
                format, MSize(w,h), strideInBytes, bufBoundaryInBytes, planeCount);
        PortBufInfo_v1 portBufInfo =
            PortBufInfo_v1(
                    tmpImageBuffer->getFD(),
                    tmpImageBuffer->getBufVA(0),
                    0, 0, 0);

        sp<ImageBufferHeap> heap =
            ImageBufferHeap::create(LOG_TAG, extParam, portBufInfo);

        if (heap == NULL)
        {
            MY_LOGE("pHeap is NULL");
            return MFALSE;
        }

        pBuf = heap->createImageBuffer();

        // add into list for management
        {
            Mutex::Autolock _l(mInternalBufferListLock);
            mInternalBufferList.add(
                    reinterpret_cast<MINTPTR>(pBuf.get()), tmpImageBuffer);
        }
    }
    else
    {
        IIonImageBufferHeap::AllocImgParam_t imgParam(
                format, MSize(w,h), strideInBytes, bufBoundaryInBytes, planeCount);

        sp<IIonImageBufferHeap> heap =
            IIonImageBufferHeap::create(LOG_TAG,
                    imgParam, IIonImageBufferHeap::AllocExtraParam(), MFALSE);

        if (heap == NULL)
        {
            MY_LOGE("heap is NULL");
            return MFALSE;
        }

        pBuf = heap->createImageBuffer();
    }

    if (pBuf == NULL)
    {
        MY_LOGE("pBuf is NULL");
        return MFALSE;
    }

    if (!pBuf->lockBuf(LOG_TAG, BUFFER_USAGE_SW))
    {
        MY_LOGE("lock Buffer failed");
        ret = MFALSE;
    }
    else
    {
        // flush
        //pBuf->syncCache(eCACHECTRL_INVALID);  //hw->cpu
        MY_LOGD("allocBuffer addr(%p) width(%d) height(%d) format(0x%x)",
                pBuf.get(), w, h, format);

        for (MUINT32 i = 0; i < planeCount; i++)
        {
            MY_LOGD("allocBuffer getBufVA(%d)(%p)", i, pBuf->getBufVA(i));
        }
        imageBuf = pBuf;
    }

lbExit:
    return ret;
}
void
ImageBufferManager::
deallocBuffer(IImageBuffer* pBuf)
{
    if (pBuf == NULL)
    {
        MY_LOGD("pBuf is NULL, do nothing");
        return;
    }

    // unlock image buffer
    pBuf->unlockBuf(LOG_TAG);

    // unlock internal buffer
    {
        Mutex::Autolock _l(mInternalBufferListLock);
        const MINTPTR key = reinterpret_cast<MINTPTR>(pBuf);
        sp<IImageBuffer> tmpImageBuffer = mInternalBufferList.valueFor(key);
        if (tmpImageBuffer.get())
        {
            tmpImageBuffer->unlockBuf(LOG_TAG);
            mInternalBufferList.removeItem(key);
        }
    }
}

void
ImageBufferManager::
deallocBuffer(sp<IImageBuffer>& pBuf)
{
    deallocBuffer(pBuf.get());
}
MBOOL
ImageBufferManager::
removeBufferAlias(
        IImageBuffer *pOriginalBuf, IImageBuffer *pAliasBuf)
{
    MBOOL ret = MTRUE;

    if ((pOriginalBuf == NULL) || (pAliasBuf == NULL))
    {
        MY_LOGE("ImageBuffer is NULL: pOriginalBuf(%p) pAliasBuf(%p)",
                pOriginalBuf, pAliasBuf);
        ret = MFALSE;
        goto lbExit;
    }

    // destroy alias
    /*if (!pAliasBuf->unlockBuf(LOG_TAG))
    {
        MY_LOGE("can't unlock pAliasBuf");
        ret = MFALSE;
        goto lbExit;
    }*/
    pAliasBuf->decStrong(pAliasBuf);

    // re-lock the original buffer
    /*if (!pOriginalBuf->lockBuf(LOG_TAG, BUFFER_USAGE_SW))
    {
        MY_LOGE("can't lock pOriginalBuf");
        ret = MFALSE;
        goto lbExit;
    }*/

lbExit:
    return ret;
}
IImageBuffer*
ImageBufferManager::
createBufferAlias(
        IImageBuffer* pOriginalBuf, MUINT32 w, MUINT32 h, NSCam::EImageFormat format)
{
    MBOOL ret = MTRUE;

    MSize imgSize(w, h);
    size_t bufStridesInBytes[3] = {0};

    IImageBuffer *pBuf = NULL;
    IImageBufferHeap *pBufHeap = NULL;

    if (!pOriginalBuf)
    {
        MY_LOGE("pOriginalBuf is null");
        ret = MFALSE;
        goto lbExit;
    }

    MY_LOGD("pOriginalBuf is %p", pOriginalBuf);

    pBufHeap = pOriginalBuf->getImageBufferHeap();
    if (!pBufHeap)
    {
        MY_LOGE("pBufHeap is null");
        ret = MFALSE;
        goto lbExit;
    }

    if (pBufHeap->getImgFormat() != eImgFmt_BLOB)
    {
        MY_LOGE("heap buffer type must be BLOB=0x%x, this is 0x%x",
                eImgFmt_BLOB,
                pBufHeap->getImgFormat());
        ret = MFALSE;
        goto lbExit;
    }

    //@todo use those functions to replace this block
    //MINT32 const planeImgWidthStrideInPixels = Format::queryPlaneWidthInPixels(getImgFormat(), planeIndex, getImgSize().w);
    //MINT32 const planeImgHeightStrideInPixels= Format::queryPlaneHeightInPixels(getImgFormat(), planeIndex, getImgSize().h);
    #if 1
    switch (format)
    {
        case eImgFmt_Y8:
        case eImgFmt_JPEG:
            bufStridesInBytes[0] = w;
            break;
        case eImgFmt_I420:
            bufStridesInBytes[0] = w;
            bufStridesInBytes[1] = w / 2;
            bufStridesInBytes[2] = w / 2;
            break;
        case eImgFmt_YUY2:
            bufStridesInBytes[0] = w * 2;
            bufStridesInBytes[1] = w;
            bufStridesInBytes[2] = w;
            break;
        default:
            MY_LOGE("unsupported format(0x%x)", format);
            ret = MFALSE;
            goto lbExit;
    }
    #else
    GetStride(w, format, bufStridesInBytes);
    #endif

    // create new buffer
    pBuf = pBufHeap->createImageBuffer_FromBlobHeap(
            (size_t)0, format, imgSize, bufStridesInBytes);

    if (!pBuf)
    {
        MY_LOGE("can't makeBufferAlias size(%dx%d) format(0x%x)", w, h, format);
        ret = MFALSE;
        goto lbExit;
    }

    pBuf->incStrong(pBuf);

    // unlock old buffer
    if (!pOriginalBuf->unlockBuf(LOG_TAG))
    {
        MY_LOGE("can't unlock pOriginalBuf");
    };

    // lock new buffer
    /*if (!pBuf->lockBuf(LOG_TAG, BUFFER_USAGE_SW))
    {
        MY_LOGE("can't lock pBuf");
        ret = MFALSE;
        goto lbExit;
    }*/

lbExit:
    return pBuf;
}
