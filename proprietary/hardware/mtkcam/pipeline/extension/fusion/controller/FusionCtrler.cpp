/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "FusionCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

#include "FusionCtrler.h"
// log
#include <mtkcam/utils/std/Log.h>

// mtkcam
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/utils/std/Format.h>
#include "../utils/VendorUtils.h"
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/BaseImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

#include <faces.h>
#include <Hal3AAdapter3.h>

#include <mtkcam/aaa/IHal3A.h>

// stereo
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <camera_custom_dualzoom.h>


// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>

#define MY_LOGV(id, fmt, arg...)     CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)     CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)     CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)     CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)     CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)     CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)     CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)

#define MY__LOGV(fmt, arg...)        CAM_LOGV("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGD(fmt, arg...)        CAM_LOGD("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGI(fmt, arg...)        CAM_LOGI("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGW(fmt, arg...)        CAM_LOGW("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGE(fmt, arg...)        CAM_LOGE("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGA(fmt, arg...)        CAM_LOGA("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGF(fmt, arg...)        CAM_LOGF("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)

using namespace android;
using namespace NSCam::plugin;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCam::Utils::Format;
#define BUFFER_USAGE_SW (eBUFFER_USAGE_SW_READ_OFTEN | \
                         eBUFFER_USAGE_SW_WRITE_OFTEN | \
                         eBUFFER_USAGE_HW_CAMERA_READWRITE | \
                         eBUFFER_USAGE_SW_WRITE_RARELY)

#define dumpFrameReq(frame) \
for (int i = 0 ; i < RequestFrame::eRequestImg_Size ; i++) \
{ \
    IImageBuffer* ptr = frame->getImageBuffer((RequestFrame::eRequestImg)i); \
    if (ptr) \
    { \
        MSize size  = ptr->getImgSize(); \
        MINT format = ptr->getImgFormat(); \
        MY_LOGD(frame->getOpenId(), " [%02d] VA(%p) (%dx%d) f(0x%x)", \
            i, ptr->getBufVA(0), size.w, size.h, format); \
    } \
}


#define saveImage(id, img) \
{ \
    MSize size = img->getImgSize(); \
    char filename[256]; \
    snprintf(filename, sizeof(filename), \
            "/sdcard/dump/cam%d_%d_%dx%d_f(0x%x).raw", \
            id, \
            img->getBufOffsetInBytes(0), \
            size.w, size.h, img->getImgFormat()); \
    img->saveToFile(filename); \
}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
FusionCtrler::
FusionCtrler(
    MINT32 mode
) : BaseFusionCtrler(mode)
{
    setCaptureNum(1);
    setDelayFrameNum(0);
    MY__LOGD("CapNum(%d) DelayNum(%d)", getCaptureNum(), getDelayFrameNum());

    mpDpStream = new DpBlitStream();
}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
FusionCtrler::
~FusionCtrler()
{
    MY__LOGD("+");
    if (mpDpStream != nullptr)
    {
        delete mpDpStream;
        mpDpStream = nullptr;
    }
}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
FusionCtrler::
init()
{
    MY__LOGD("+");
    return true;
}

void
FusionCtrler::doCopy(
    ReqFrameSP mainFrame,
    ReqFrameSP main2Frame, MBOOL srcWide)
{
    IMetadata* appMetaIn        = mainFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* appMetaOut        = mainFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
    IImageBuffer* pSourceFrame  = mainFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pFrameYuvJpeg = mainFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
    IImageBuffer* pFrameYuvThub = mainFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
    IImageBuffer* pFramePostview = mainFrame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut);
    MUINT32 outOrientaion       = mainFrame->getOrientation(RequestFrame::eRequestImg_FullOut);

    if (appMetaIn == nullptr)
    {
        MY__LOGW("input metadata is NULL");
    }

    if (pFrameYuvJpeg == nullptr)
    {
        MY__LOGE("output YUV is NULL");
        return;
    }

    if (pFrameYuvThub == nullptr)
    {
        MY__LOGW("output thumbnail is NULL");
        mainFrame->markError(RequestFrame::eRequestImg_ThumbnailOut, true);
    }

    if (pFramePostview == nullptr)
    {
        MY__LOGW("output postview is NULL");
        mainFrame->markError(RequestFrame::eRequestImg_PostviewOut, true);
    }

    if (!srcWide)
    {
        pSourceFrame  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
        appMetaIn     = main2Frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    }

    if (pSourceFrame == nullptr)
    {
        MY__LOGE("input YUV is NULL");
        return;
    }


    // get input/output size, and crop size
    MSize dst1Size =
        (outOrientaion == eTransform_ROT_90 || outOrientaion == eTransform_ROT_270)
        ? MSize(pFrameYuvJpeg->getImgSize().h, pFrameYuvJpeg->getImgSize().w)
        : pFrameYuvJpeg->getImgSize();

    MINT32 mOpenId = srcWide ? mainFrame->getOpenId() : main2Frame->getOpenId();
    MRect rectDst1(MPoint(0, 0), dst1Size);
    MRect rectDst2;
    MRect rectSrc = calCropRegin(appMetaIn, pSourceFrame->getImgSize(), mOpenId);

    if (pFrameYuvThub)
    {
        rectDst2.p = MPoint(0, 0);
        rectDst2.s = pFrameYuvThub->getImgSize();
    }

    // create JPEG YUV and thumbnail YUV using MDP (IImageTransform)
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>>
            transform(IImageTransform::createInstance()// constructor
                      , [](IImageTransform * p)
    {
        if (p)
        {
            p->destroyInstance();    // deleter
        }
    });

    if (transform.get() == nullptr)
    {
        MY__LOGE("IImageTransform is NULL, cannot generate output");
        return;
    }

    MBOOL ret = MTRUE;
    if (pFrameYuvThub)
    {
        ret = transform->execute(
                  pSourceFrame,
                  pFrameYuvJpeg,
                  pFrameYuvThub,
                  calCrop(rectSrc, rectDst1),
                  calCrop(rectSrc, rectDst2),
                  outOrientaion,
                  0,
                  3000);
    }
    else
    {
        ret = transform->execute(
                  pSourceFrame,
                  pFrameYuvJpeg,
                  nullptr,
                  calCrop(rectSrc, rectDst1),
                  outOrientaion,
                  3000);
    }

    if (ret != MTRUE)
    {
        MY__LOGE("execute IImageTransform::execute returns fail");
    }
    else
    {
        MY__LOGD("Good Picture !");
        onShutter();
        onNextCaptureReady();
    }

    if (pFramePostview)
    {
        ret = transform->execute(
                  pSourceFrame,
                  pFramePostview,
                  nullptr,
                  calCrop(rectSrc, rectDst2),
                  0,
                  3000);
    }
}

DpColorFormat
FusionCtrler::
getDpColorFormat(MINT32 format)
{
    switch (format)
    {
        case eImgFmt_RGBA8888:
            MY__LOGD("eImgFmt_RGBA8888");
            return DP_COLOR_RGBA8888;
        case eImgFmt_YUY2:
            MY__LOGD("eImgFmt_YUY2");
            return DP_COLOR_YUYV;
        case eImgFmt_NV16:
            MY__LOGD("eImgFmt_NV16");
            return DP_COLOR_NV16;
        case eImgFmt_NV21:
            MY__LOGD("eImgFmt_NV21");
            return DP_COLOR_NV21;
        case eImgFmt_YV12:
            MY__LOGD("eImgFmt_YV12");
            return DP_COLOR_YV12;
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y8:
            MY__LOGD("eImgFmt_Y8 eImgFmt_STA_BYTE");
            return DP_COLOR_GREY;
        case eImgFmt_NV12:
            MY__LOGD("eImgFmt_NV12");
            return DP_COLOR_NV12;
        case eImgFmt_YV16:
            MY__LOGD("eImgFmt_YV16");
            return DP_COLOR_YV16;
        case eImgFmt_RGB48:
            MY__LOGD("eImgFmt_RGB48, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        case eImgFmt_BAYER12_UNPAK:
            MY__LOGD("eImgFmt_BAYER12_UNPAK, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        case eImgFmt_BAYER14_UNPAK:
            MY__LOGD("eImgFmt_BAYER14_UNPAK, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        case eImgFmt_Y16:
            MY__LOGD("eImgFmt_Y16, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        default:
            MY__LOGE("unsupported(%x)", format);
    }
    return DP_COLOR_UNKNOWN;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MBOOL
FusionCtrler::excuteMDP(sMDP_Config config)
{
    MY__LOGD("+");

    if (config.pDpStream == nullptr)
    {
        return MFALSE;
    }
    //***************************src****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    size_t planeCount = config.pSrcBuffer->getPlaneCount();
    for (size_t i = 0; i < planeCount; ++i)
    {
        src_addr_list[i] = config.pSrcBuffer->getBufVA(i);
        src_size_list[i] = config.pSrcBuffer->getBufSizeInBytes(i);
    }
    MINT32 uvPitch = 0;
    if (planeCount > 1)
    {
        uvPitch = config.pSrcBuffer->getBufStridesInBytes(1);
    }
    config.pDpStream->setSrcBuffer((void**)src_addr_list, src_size_list, planeCount);
    config.pDpStream->setSrcConfig(
        config.pSrcBuffer->getImgSize().w,
        config.pSrcBuffer->getImgSize().h,
        config.pSrcBuffer->getBufStridesInBytes(0),
        uvPitch,
        getDpColorFormat(config.pSrcBuffer->getImgFormat()),
        DP_PROFILE_FULL_BT601,
        eInterlace_None,
        (config.src_roi.w == 0 ) ? 0 : &(config.src_roi),
        DP_SECURE_NONE,
        false);
    MY__LOGD("config.src_roi: %d, %d, %d, %d", config.src_roi.x, config.src_roi.y, config.src_roi.w, config.src_roi.h);
    MY__LOGD("src:%dx%d (P:%zu,Strides:%zu,uvPitch:%d)",
        config.pSrcBuffer->getImgSize().w,
        config.pSrcBuffer->getImgSize().h, planeCount,
        config.pSrcBuffer->getBufStridesInBytes(0), uvPitch);


    //***************************dst********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    planeCount = config.pDstBuffer->getPlaneCount();
    for (size_t i = 0; i < planeCount; ++i)
    {
        dst_addr_list[i] = config.pDstBuffer->getBufVA(i);
        dst_size_list[i] = config.pDstBuffer->getBufSizeInBytes(i);
    }
    uvPitch = 0;
    if (planeCount > 1)
    {
        uvPitch = config.pDstBuffer->getBufStridesInBytes(1);
    }

    MINT32 w;
    MINT32 h;
    DpRect* rect = NULL;
    if (config.dst_roi.w != 0 && config.dst_roi.h != 0)
    {
        rect = &(config.dst_roi);
        w = config.dst_roi.w;
        h = config.dst_roi.h;
    }
    else
    {
        w = config.pDstBuffer->getImgSize().w;
        h = config.pDstBuffer->getImgSize().h;
    }

    config.pDpStream->setDstBuffer((void**)dst_addr_list, dst_size_list, planeCount);
    config.pDpStream->setDstConfig(w, h,
                                   config.pDstBuffer->getBufStridesInBytes(0),
                                   uvPitch,
                                   getDpColorFormat(config.pDstBuffer->getImgFormat()),
                                   DP_PROFILE_FULL_BT601,
                                   eInterlace_None,
                                   rect,
                                   DP_SECURE_NONE,
                                   false);
    config.pDpStream->setRotate(config.rotAngle);
    MY__LOGD("dst:%dx%d (P:%zu,Strides:%zu,uvPitch:%d,angle:%d)",
        config.pDstBuffer->getImgSize().w,
        config.pDstBuffer->getImgSize().h, planeCount,
        config.pDstBuffer->getBufStridesInBytes(0), uvPitch, config.rotAngle);

    //*******************************************************************//
    if (config.pDpStream->invalidate())  //trigger HW
    {
        MY__LOGD("FDstream invalidate failed");
        return MFALSE;
    }
    return MTRUE;
}

MINT32 FusionCtrler::getZoomRatio(std::shared_ptr<RequestFrame> main1Frame)
{
    MINT32 zoomRatio = 100;
    IMetadata* appMetaIn        = main1Frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* halMetaIn        = main1Frame->getMetadata(RequestFrame::eRequestMeta_InFullRaw);
    MRect origCropRegion;
    MRect activeRegionWide;
    retrieveScalerCropRgn(appMetaIn, origCropRegion);

    MINT32 id = main1Frame->getOpenId();
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(id);
    if (pMetadataProvider.get())
    {
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();

        IMetadata::IEntry active_array_entry = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        if (!active_array_entry.isEmpty())
        {
            activeRegionWide = active_array_entry.itemAt(0, Type2Type<MRect>());
            zoomRatio = ((activeRegionWide.s.w * 100)) / origCropRegion.s.w;

            MY_LOGD(id, "wide:   crop region (%d,%d) (%dx%d)",
                    origCropRegion.p.x, origCropRegion.p.y, origCropRegion.s.w, origCropRegion.s.h);
            MY_LOGD(id, "wide: active region (%d,%d) (%dx%d) r:%d",
                    activeRegionWide.p.x, activeRegionWide.p.y,
                    activeRegionWide.s.w, activeRegionWide.s.h, zoomRatio);

            IMetadata::IEntry tag(MTK_DUALZOOM_ZOOM_FACTOR);
            tag.push_back(zoomRatio, Type2Type<MINT32>());
            halMetaIn->update(MTK_DUALZOOM_ZOOM_FACTOR, tag);
        }
        else
        {
            MY_LOGE(id, "no MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION !");
        }
    }
    else
    {
        MY_LOGE(id, "no pMetadataProvider !");
    }

    return zoomRatio;
}

static MUINT32 queryPlanePixel(MUINT32 fmt, MUINT32 i, MUINT32 width, MUINT32 height)
{
    MUINT32 pixel;
    pixel = ((queryPlaneWidthInPixels(fmt,i, width) * queryPlaneBitsPerPixel(fmt,i)) / 8) * queryPlaneHeightInPixels(fmt, i, height);
    return pixel;
}

static MUINT32 queryStrideInPixels(MUINT32 fmt, MUINT32 i, MUINT32 width)
{
    MUINT32 pixel;
    pixel = (queryPlaneWidthInPixels(fmt, i, width) * queryPlaneBitsPerPixel(fmt, i)) / 8;
    return pixel;
}


MBOOL
FusionCtrler::
createEmptyImageBuffer(
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

    size_t bufBoundaryInBytes[3] = {0};
    size_t strideInBytes[3] = {0};
    for (MUINT32 i = 0; i < planeCount; i++)
    {
        strideInBytes[i] = queryStrideInPixels(format, i, w);
        MY__LOGD("strideInBytes[%d] strideInBytes(%d)", i, strideInBytes[i]);
    }

    if (isContinuous)
    {
        // to avoid non-continuous multi-plane memory,
        // allocate memory in blob format and map it to ImageBuffer
        MUINT32 allPlaneSize = 0;
        for (MUINT32 i = 0; i < planeCount; i++)
        {
            allPlaneSize += queryPlanePixel(format, i, w, h);
        }
        MY__LOGD("all plane size(%d)", allPlaneSize);

        // allocate blob buffer
        IImageBufferAllocator::ImgParam blobParam = IImageBufferAllocator::ImgParam(allPlaneSize, 0);

        sp<IImageBufferHeap> heap = IIonImageBufferHeap::create("fusion_working", blobParam);
        if( heap == NULL )
        {
            MY__LOGE("IIonImageBufferHeap create failed");
            return MFALSE;
        }

        pBuf = heap->createImageBuffer_FromBlobHeap((size_t)0, (MINT32)format, MSize(w, h), strideInBytes);
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
            MY__LOGE("heap is NULL");
            return MFALSE;
        }

        pBuf = heap->createImageBuffer();
    }

    if (pBuf == NULL)
    {
        MY__LOGE("pBuf is NULL");
        return MFALSE;
    }

    if (!pBuf->lockBuf(LOG_TAG, BUFFER_USAGE_SW))
    {
        MY__LOGE("lock Buffer failed");
        ret = MFALSE;
    }
    else
    {
        // flush
        //pBuf->syncCache(eCACHECTRL_INVALID);  //hw->cpu
        MY__LOGD("addr(%p) width(%d) height(%d) format(0x%x)",
                pBuf.get(), w, h, format);

        for (MUINT32 i = 0; i < planeCount; i++)
        {
            MY__LOGD("getBufVA(%d)(%p)", i, pBuf->getBufVA(i));
        }
        imageBuf = pBuf;
    }

lbExit:
    return ret;
}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
FusionCtrler::
doFusion(
    ReqFrameSP main1Frame,
    ReqFrameSP main2Frame)
{
    MY__LOGW("Wide(%p) Tele(%p)", main1Frame.get(), main2Frame.get());

    if (main1Frame.get() != NULL && main2Frame.get() != NULL)
    {
        dumpFrameReq(main1Frame);
        dumpFrameReq(main2Frame);

        // ************** ToDo - Fusion algorithm Start ***************

        IImageBuffer* pWideImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
        IImageBuffer* pTeleImg  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
        IImageBuffer* pOutputImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);

        char* wide_yuv_ptr = (char*)pWideImg->getBufVA(0);
        char* tele_yuv_ptr = (char*)pTeleImg->getBufVA(0);
        char* jpeg_yuv_ptr = (char*)pOutputImg->getBufVA(0);

        MSize wsize = pWideImg->getImgSize();
        MSize tsize = pTeleImg->getImgSize();
        MSize osize = pOutputImg->getImgSize();

        MY__LOGI("Wide(%p)(%p) %p",
            (char*)pWideImg->getBufVA(0),
            (char*)pWideImg->getBufVA(1),
            (char*)(pWideImg->getBufVA(0) + wsize.h*wsize.w));
        MY__LOGI("Tele(%p)(%p) %p",
            (char*)pTeleImg->getBufVA(0),
            (char*)pTeleImg->getBufVA(1),
            (char*)(pTeleImg->getBufVA(0) + tsize.h*tsize.w));
        MY__LOGI("Jpeg(%p)(%p) %p",
            (char*)pOutputImg->getBufVA(0),
            (char*)pOutputImg->getBufVA(1),
            (char*)(pOutputImg->getBufVA(0) + osize.h*osize.w));

        //pWideImg->saveToFile("/sdcard/wide.nv21");
        //pTeleImg->saveToFile("/sdcard/tele.nv21");

        // get ratio
        MINT32 zoomRatio = getZoomRatio(main1Frame);



        // ************************ warning ********************************************
        // jpeg_yuv_ptr is not continue jpeg yuv buffer
        // Y:  pOutputImg->getBufVA(0)
        // UV: pOutputImg->getBufVA(1)
        // (pOutputImg->getBufVA(1) != pOutputImg->getBufVA(0) + osize.h * osize.w)
        // ************************ warning ********************************************


#if 0   // Reference code for continue jpeg yuv buffer output access by algorithm
        // *** step 1. ***
        // create continue jpeg yuv buffer (img->getBufVA(1) == img->getBufVA(0) + osize.h * osize.w)
        sp<IImageBuffer> jpeg_continue_imageBuf;
        createEmptyImageBuffer(jpeg_continue_imageBuf, osize.w, osize.h, eImgFmt_NV21, true);

        if (nullptr != jpeg_continue_imageBuf.get())
        {

            // *** step 2. ***
            // call algorithm with these three continue yuv buffer

            // wide_yuv_ptr      = (char*)pWideImg->getBufVA(0);
            // tele_yuv_ptr      = (char*)pTeleImg->getBufVA(0);
            // jpeg_temp_yuv_ptr = (char*)jpeg_continue_imageBuf->getBufVA(0);
            // myFusion(wide_yuv_ptr, tele_yuv_ptr, jpeg_temp_yuv_ptr);


            // *** step 3. ***
            // copy continue jpeg yuv buffer back to jpeg buffer for output jpeg result
            // jpeg_continue_imageBuf ==> pOutputImg
            MY__LOGD("start copy back to jpeg buffer");
            sMDP_Config config;
            memset(&config, 0, sizeof(sMDP_Config));

            config.pDpStream = mpDpStream;
            config.pSrcBuffer = jpeg_continue_imageBuf.get();
            config.pDstBuffer = pOutputImg;
            config.rotAngle = 0;
            if (!excuteMDP(config))
            {
                MY__LOGE("excuteMDP fail. 1");
            }


            // *** step 4. ***
            // finish and notify
            MY__LOGD("Good Picture !");
            onShutter();
            onNextCaptureReady();
        }
        else
        {
            MY__LOGE("create continue jpeg yuv buffer error");
        }
#elif (MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT == 1)
        doFusion01(main1Frame, main2Frame);
#elif (MTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT == 1)
        doFusion02(main1Frame, main2Frame);
#elif (MTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT == 1)
        doFusion03(main1Frame, main2Frame);
#else
        // copy wide or tele to jpeg buffer as an example via IImageTransform utility
        doCopy(main1Frame, main2Frame, zoomRatio < DUALZOOM_SWICH_CAM_ZOOM_RATIO);
#endif

        // ************** ToDo - Fusion algorithm End ***************
    }


    // after main2 finish, mark it to error, it is mandatory
    if (main2Frame.get() != NULL)
    {
        // after main2 finish, mark it to error
        main2Frame->markError(RequestFrame::eRequestImg_All, true);
        main2Frame->markError(RequestFrame::eRequestMeta_All, true);
    }

    return true;
}


// ----------------------------------------------------------------------------
// 3rd party algorithm
// ----------------------------------------------------------------------------

#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT == 1)
void FusionCtrler::doFusion01_setIoTriplet(IO_triplet_t& io_triplet,
        int wideWidth, int wideHeight, int widePpln,
        int teleWidth, int teleHeight, int telePpln,
        int outWidth, int outHeight, int outPpln)
{
    io_triplet.wideFrameSize.width = wideWidth;
    io_triplet.wideFrameSize.height = wideHeight;
    io_triplet.wideFrameSize.ppln = widePpln;

    io_triplet.teleFrameSize.width = teleWidth;
    io_triplet.teleFrameSize.height = teleHeight;
    io_triplet.teleFrameSize.ppln = telePpln;

    io_triplet.fusedFrameSize.width = outWidth;
    io_triplet.fusedFrameSize.height = outHeight;
    io_triplet.fusedFrameSize.ppln = outPpln;


    MY__LOGD("Wide:%dx%d ppln:%d", wideWidth, wideHeight, widePpln);
    MY__LOGD("Tele:%dx%d ppln:%d", teleWidth, teleHeight, telePpln);
    MY__LOGD("Jpeg:%dx%d ppln:%d", outWidth, outHeight, outPpln);
}

void FusionCtrler::doFusion01_updateParamsGet3A(
    info_3a_t* inout, ReqFrameSP frame)
{

    IMetadata* AppMetaIn = frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* HalMetaIn = frame->getMetadata(RequestFrame::eRequestMeta_InFullRaw);
    IMetadata* HalMetaP1Out = frame->getMetadata(RequestFrame::eRequestMeta_InHalP1);

    const MINT32 id = frame->getOpenId();

    MINT32 magin_num = 0;
    if (tryGetMetadata<MINT32>(HalMetaP1Out, MTK_P1NODE_PROCESSOR_MAGICNUM, magin_num))
    {
        MY_LOGD(id, "magin num:%d", magin_num);
    }
    else
    {
        MY_LOGE(id, "no magin num");
        return;
    }

    MetaSet_T f3A;
    Hal3AAdapter3* hal3a = Hal3AAdapter3::createInstance(frame->getOpenId(), LOG_TAG);

#if 1
    if (hal3a->get(magin_num, f3A) == 0)
    {
        //f3A.appMeta.dump();
        //f3A.halMeta.dump();

        // AE
        {
            MINT64 exposureTime = 0;
            tryGetMetadata<MINT64>(HalMetaP1Out, MTK_SENSOR_EXPOSURE_TIME, exposureTime);
            inout->ae_info.exposure = (float)(exposureTime / 1000);
            MY_LOGD(id, "exposure: %" PRId64, exposureTime);


            MINT32 iso = 0;
            tryGetMetadata<MINT32>(HalMetaP1Out, MTK_SENSOR_SENSITIVITY, iso);
            inout->ae_info.gain = iso;
            MY_LOGD(id, "iso: %d", iso);

            inout->ae_info.status = AE_VALID;
        }


        // AF
        {
            inout->af_info.focus_PosInfo.far_field_cm = 4;
            inout->af_info.focus_PosInfo.lens_shift_um = 5;
            inout->af_info.focus_PosInfo.near_field_cm = 6;
            inout->af_info.focus_PosInfo.object_distance_cm = 7;

            MRect activeRegion;
            sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(id);
            if (pMetadataProvider.get())
            {
                IMetadata static_meta = pMetadataProvider->geMtktStaticCharacteristics();

                IMetadata::IEntry active_array_entry = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
                if (!active_array_entry.isEmpty())
                {
                    activeRegion = active_array_entry.itemAt(0, Type2Type<MRect>());
                    inout->af_info.roi.start_x = activeRegion.p.x;
                    inout->af_info.roi.start_y = activeRegion.p.y;
                    inout->af_info.roi.width   = activeRegion.s.w;
                    inout->af_info.roi.height  = activeRegion.s.h;
                    inout->af_info.status =  AF_VALID;
                }
            }
        }


        // AWB
        {
            inout->awb_info.ccm_size = 10;
            inout->awb_info.colorTemp = 11;

            IMetadata::IEntry entry = HalMetaP1Out->entryFor(MTK_COLOR_CORRECTION_GAINS);
            if (!entry.isEmpty() && entry.count() >= 4)
            {
                // [R, G_even, G_odd, B]
                inout->awb_info.gain_r = entry.itemAt(0, Type2Type<MFLOAT>());
                inout->awb_info.gain_g = entry.itemAt(1, Type2Type<MFLOAT>());
                inout->awb_info.gain_b = entry.itemAt(3, Type2Type<MFLOAT>());

                inout->awb_info.status = AWB_VALID;
            }
        }
    }
#endif
    hal3a->destroyInstance(LOG_TAG);
}


void FusionCtrler::doFusion01_updateParams(
    fusionInputParams_t& inParams,
    ReqFrameSP wide,
    ReqFrameSP tele)
{
    IMetadata* wideAppMetaIn = wide->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* wideHalMetaIn = wide->getMetadata(RequestFrame::eRequestMeta_InFullRaw);
    IMetadata* wideHalMetaP1Out = wide->getMetadata(RequestFrame::eRequestMeta_InHalP1);

    IMetadata* teleAppMetaIn = tele->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* teleHalMetaIn = tele->getMetadata(RequestFrame::eRequestMeta_InFullRaw);
    IMetadata* teleHalMetaP1Out = tele->getMetadata(RequestFrame::eRequestMeta_InHalP1);



    for (MUINT32 i = 1; i < 7; i++)
    {
        IMetadata* metaIn = wide->getMetadata((RequestFrame::eRequestMeta)i);
        MY__LOGD("wide[%d]: %p, MAGICNUM:0x%x", i, metaIn, MTK_P1NODE_PROCESSOR_MAGICNUM);
        if (metaIn != nullptr)
        {
            metaIn->dump();
        }
    }

    for (MUINT32 i = 1; i < 7; i++)
    {
        IMetadata* metaIn = tele->getMetadata((RequestFrame::eRequestMeta)i);
        MY__LOGD("tele[%d]: %p", i, metaIn);
        if (metaIn != nullptr)
        {
            metaIn->dump();
        }
    }


    inParams.sceneMode = CP_STANDARD;

#if 0
    inParams.runtime_params.status_3a.tele.ae_info.exposure = 1;
    inParams.runtime_params.status_3a.tele.ae_info.gain = 2;
    inParams.runtime_params.status_3a.tele.ae_info.status = AE_VALID;
    inParams.runtime_params.status_3a.tele.af_info.focus_PosInfo.far_field_cm = 4;
    inParams.runtime_params.status_3a.tele.af_info.focus_PosInfo.lens_shift_um = 5;
    inParams.runtime_params.status_3a.tele.af_info.focus_PosInfo.near_field_cm = 6;
    inParams.runtime_params.status_3a.tele.af_info.focus_PosInfo.object_distance_cm = 7;
    inParams.runtime_params.status_3a.tele.af_info.roi.height =  400.000000;
    inParams.runtime_params.status_3a.tele.af_info.roi.start_x =  -200.000000;
    inParams.runtime_params.status_3a.tele.af_info.roi.start_y =  -200.000000;
    inParams.runtime_params.status_3a.tele.af_info.roi.width =  400.000000;
    inParams.runtime_params.status_3a.tele.af_info.status =  AF_VALID;
    inParams.runtime_params.status_3a.tele.awb_info.ccm_size = 10;
    inParams.runtime_params.status_3a.tele.awb_info.colorTemp = 11;
    inParams.runtime_params.status_3a.tele.awb_info.gain_b = 12;
    inParams.runtime_params.status_3a.tele.awb_info.gain_g = 13;
    inParams.runtime_params.status_3a.tele.awb_info.gain_r = 14;
    inParams.runtime_params.status_3a.tele.awb_info.status = AWB_VALID;
    inParams.runtime_params.status_3a.wide.ae_info.exposure = 15;
    inParams.runtime_params.status_3a.wide.ae_info.gain =  1.0;
    inParams.runtime_params.status_3a.wide.ae_info.status = AE_VALID;
    inParams.runtime_params.status_3a.wide.af_info.focus_PosInfo.far_field_cm = 16;
    inParams.runtime_params.status_3a.wide.af_info.focus_PosInfo.lens_shift_um = 17;
    inParams.runtime_params.status_3a.wide.af_info.focus_PosInfo.near_field_cm = 18;
    inParams.runtime_params.status_3a.wide.af_info.focus_PosInfo.object_distance_cm = 19;
    inParams.runtime_params.status_3a.wide.af_info.roi.height = 20;
    inParams.runtime_params.status_3a.wide.af_info.roi.start_x = 21;
    inParams.runtime_params.status_3a.wide.af_info.roi.start_y = 22;
    inParams.runtime_params.status_3a.wide.af_info.roi.width = 23;
    inParams.runtime_params.status_3a.wide.af_info.status = AF_VALID;
    inParams.runtime_params.status_3a.wide.awb_info.ccm_size = 25;
    inParams.runtime_params.status_3a.wide.awb_info.colorTemp = 26;
    inParams.runtime_params.status_3a.wide.awb_info.gain_b = 27;
    inParams.runtime_params.status_3a.wide.awb_info.gain_g = 28;
    inParams.runtime_params.status_3a.wide.awb_info.gain_r = 29;
    inParams.runtime_params.status_3a.wide.awb_info.status = AWB_VALID;
#endif

    // get 3A
    doFusion01_updateParamsGet3A(&(inParams.runtime_params.status_3a.wide), wide);
    doFusion01_updateParamsGet3A(&(inParams.runtime_params.status_3a.tele), tele);


    inParams.runtime_params.zoom.tele_input.crop.cbcr_first_line = 31;
    inParams.runtime_params.zoom.tele_input.crop.cbcr_first_pixel = 32;
    inParams.runtime_params.zoom.tele_input.crop.cbcr_last_line = 33;
    inParams.runtime_params.zoom.tele_input.crop.cbcr_last_pixel = 34;
    inParams.runtime_params.zoom.tele_input.crop.y_first_line = 35;
    inParams.runtime_params.zoom.tele_input.crop.y_first_pixel = 36;
    inParams.runtime_params.zoom.tele_input.crop.y_last_line = 37;
    inParams.runtime_params.zoom.tele_input.crop.y_last_pixel = 38;
    inParams.runtime_params.zoom.tele_input.ois.offset_x = 39;
    inParams.runtime_params.zoom.tele_input.ois.offset_y = 40;
    inParams.runtime_params.zoom.tele_input.precrop.cbcr_first_line = 41;
    inParams.runtime_params.zoom.tele_input.precrop.cbcr_first_pixel = 42;
    inParams.runtime_params.zoom.tele_input.precrop.cbcr_last_line = 43;
    inParams.runtime_params.zoom.tele_input.precrop.cbcr_last_pixel = 44;
    inParams.runtime_params.zoom.tele_input.precrop.y_first_line = 45;
    inParams.runtime_params.zoom.tele_input.precrop.y_first_pixel = 46;
    inParams.runtime_params.zoom.tele_input.precrop.y_last_line =  47;
    inParams.runtime_params.zoom.tele_input.precrop.y_last_pixel = 48;
    inParams.runtime_params.zoom.tele_input.prescalar.y_horz_input = 49;
    inParams.runtime_params.zoom.tele_input.prescalar.y_horz_output = 50;
    inParams.runtime_params.zoom.tele_input.prescalar.y_vert_input = 51;
    inParams.runtime_params.zoom.tele_input.prescalar.y_vert_output = 52;
    inParams.runtime_params.zoom.tele_input.prescalar.cbcr_horz_input = 53;
    inParams.runtime_params.zoom.tele_input.prescalar.cbcr_horz_output = 54;
    inParams.runtime_params.zoom.tele_input.prescalar.cbcr_vert_input = 55;
    inParams.runtime_params.zoom.tele_input.prescalar.cbcr_vert_output = 56;
    inParams.runtime_params.zoom.tele_input.rotate = rotate_0;
    inParams.runtime_params.zoom.tele_input.scalar.y_horz_input = 57;
    inParams.runtime_params.zoom.tele_input.scalar.y_horz_output = 58;
    inParams.runtime_params.zoom.tele_input.scalar.y_vert_input = 59;
    inParams.runtime_params.zoom.tele_input.scalar.y_vert_output = 60;
    inParams.runtime_params.zoom.tele_input.scalar.cbcr_horz_input = 61;
    inParams.runtime_params.zoom.tele_input.scalar.cbcr_horz_output = 62;
    inParams.runtime_params.zoom.tele_input.scalar.cbcr_vert_input =  63;
    inParams.runtime_params.zoom.tele_input.scalar.cbcr_vert_output = 64;
    inParams.runtime_params.zoom.tele_input.start_of_frame_time = 65;
    inParams.runtime_params.zoom.user_zoom = 1;
    inParams.runtime_params.zoom.wide_input.crop.cbcr_first_line = 68;
    inParams.runtime_params.zoom.wide_input.crop.cbcr_first_pixel = 69;
    inParams.runtime_params.zoom.wide_input.crop.cbcr_last_line = 70;
    inParams.runtime_params.zoom.wide_input.crop.cbcr_last_pixel = 71;
    inParams.runtime_params.zoom.wide_input.crop.y_first_line = 72;
    inParams.runtime_params.zoom.wide_input.crop.y_first_pixel = 73;
    inParams.runtime_params.zoom.wide_input.crop.y_last_line = 74;
    inParams.runtime_params.zoom.wide_input.crop.y_last_pixel = 75;
    inParams.runtime_params.zoom.wide_input.ois.offset_x = 76;
    inParams.runtime_params.zoom.wide_input.ois.offset_y = 77;
    inParams.runtime_params.zoom.wide_input.precrop.cbcr_first_line = 78;
    inParams.runtime_params.zoom.wide_input.precrop.cbcr_first_pixel = 79;
    inParams.runtime_params.zoom.wide_input.precrop.cbcr_last_line = 80;
    inParams.runtime_params.zoom.wide_input.precrop.cbcr_last_pixel = 81;
    inParams.runtime_params.zoom.wide_input.precrop.y_first_line = 82;
    inParams.runtime_params.zoom.wide_input.precrop.y_first_pixel = 83;
    inParams.runtime_params.zoom.wide_input.precrop.y_last_line = 84;
    inParams.runtime_params.zoom.wide_input.precrop.y_last_pixel = 85;
    inParams.runtime_params.zoom.wide_input.prescalar.y_horz_input = 86;
    inParams.runtime_params.zoom.wide_input.prescalar.y_horz_output = 87;
    inParams.runtime_params.zoom.wide_input.prescalar.y_vert_input = 88;
    inParams.runtime_params.zoom.wide_input.prescalar.y_vert_output = 89;
    inParams.runtime_params.zoom.wide_input.prescalar.cbcr_horz_input = 90;
    inParams.runtime_params.zoom.wide_input.prescalar.cbcr_horz_output = 91;
    inParams.runtime_params.zoom.wide_input.prescalar.cbcr_vert_input = 92;
    inParams.runtime_params.zoom.wide_input.prescalar.cbcr_vert_output = 93;
    inParams.runtime_params.zoom.wide_input.rotate = rotate_0;
    inParams.runtime_params.zoom.wide_input.scalar.y_horz_input = 94;
    inParams.runtime_params.zoom.wide_input.scalar.y_horz_output = 95;
    inParams.runtime_params.zoom.wide_input.scalar.y_vert_input = 96;
    inParams.runtime_params.zoom.wide_input.scalar.y_vert_output = 97;
    inParams.runtime_params.zoom.wide_input.scalar.cbcr_horz_input = 98;
    inParams.runtime_params.zoom.wide_input.scalar.cbcr_horz_output = 99;
    inParams.runtime_params.zoom.wide_input.scalar.cbcr_vert_input = 100;
    inParams.runtime_params.zoom.wide_input.scalar.cbcr_vert_output = 101;
    inParams.runtime_params.zoom.wide_input.start_of_frame_time = 102;
}

MBOOL FusionCtrler::doFusion01_loadLib(my_fusion_t& my_fusion)
{
    do
    {
        //---------============ Open .so file ============---------
        MY__LOGI("Open .so file: libfusionLibrary.so (dlopen)\n");
        my_fusion.ptr = dlopen("libfusionLibrary.so", RTLD_NOW);
        if (!my_fusion.ptr)
        {
            MY__LOGE("Error during dlopen() of libfusionLibrary.so [%s]\n", dlerror());
            break;
        }

        *(void**) &(my_fusion.fusionInit) = dlsym(my_fusion.ptr, "fusionInit");
        if (!my_fusion.fusionInit)
        {
            MY__LOGE("Error during dlsym() of fusionInit, error: %s \n", dlerror());
            break;
        }

        *(void**) &(my_fusion.getSupportedZoomFactorRange) = dlsym(my_fusion.ptr,
                "getSupportedZoomFactorRange");
        if (!my_fusion.getSupportedZoomFactorRange)
        {
            MY__LOGE("Error during dlsym() of getSupportedZoomFactorRange, error: %s \n", dlerror());
            break;
        }


        *(void**) &(my_fusion.setIoSize) = dlsym(my_fusion.ptr, "setIoSize");
        if (!my_fusion.setIoSize)
        {
            MY__LOGE("Error during dlsym() of setIoSize, error: %s \n", dlerror());
            break;
        }


        *(void**) &(my_fusion.fusionProcess) = dlsym(my_fusion.ptr, "fusionProcess");
        if (!my_fusion.fusionProcess)
        {
            MY__LOGE("Error during dlsym() of fusionProcess\n, error: %s \n", dlerror());
            break;
        }


        *(void**) &(my_fusion.fusionDestroy) = dlsym(my_fusion.ptr, "fusionDestroy");
        if (!my_fusion.fusionDestroy)
        {
            MY__LOGE("Error during dlsym() of fusionDestroy\n, error: %s \n", dlerror());
            break;
        }

        return true;
    }
    while (false);

    dlclose(my_fusion.ptr);
    my_fusion.ptr = NULL;
    return false;
}

MINT32 FusionCtrler::doFusion01(
    ReqFrameSP main1Frame,
    ReqFrameSP main2Frame)
{
    MY__LOGD("+");
    char* strBinaryPath = "/sdcard/data/";

    InitResult initresult = CP_INIT_FAILED;
    ProcessResult processresult = CP_FUSION_FAILED;
    DestroyResult destroyresult = CP_DESTROY_FAILED;
    SetIoSizeResult setiosizeresult;
    zoomRange_t zoomrange;
    my_fusion_t my_fusion;

    uint8_t* imgWide;
    uint8_t* imgTele;
    uint8_t* imgFused;

    IO_triplet_t io_triplet;

    MY__LOGD("Load Calibration Data");
    calib_data_t calib_data;
    int calib_data_size = 264;
    uint8_t tmpArr[calib_data_size];
    calib_data.params = tmpArr;
    calib_data.size = calib_data_size;

#if 0
    FILE* fp;
    fp = fopen("/sdcard/FusionCalibrationParams.bin", "rb");
    MY__LOGD("Opening Calibration Data from file\n");
    fread(tmpArr, calib_data_size, sizeof(uint8_t), fp);
    fclose(fp);
#endif

    // ============================== In\Out Params - start ====================================
    IImageBuffer* pWideImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pTeleImg  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pOutputImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);

    MSize w_size = pWideImg->getImgSize();
    MSize t_size = pTeleImg->getImgSize();
    MSize j_size = pOutputImg->getImgSize();

    fusionInputParams_t inParams;
    fusionOutputParams_t outParams;

    doFusion01_setIoTriplet(io_triplet,
                            w_size.w, w_size.h, w_size.w,
                            t_size.w, t_size.h, t_size.w,
                            j_size.w, j_size.h, j_size.w);

    //doCPFusion_setIoTriplet(io_triplet1, 4192, 3120, 4192, 3200, 2400, 3200, 4192, 3120, 4192);
    //doCPFusion_setIoTriplet(io_triplet2, 4208, 3120, 4208, 4208, 3120, 4208, 4208, 3120, 4208);

    doFusion01_updateParams(inParams, main1Frame, main2Frame);


    outParams.fusedFrameSize.width = io_triplet.fusedFrameSize.width;
    outParams.fusedFrameSize.height = io_triplet.fusedFrameSize.height;
    outParams.fusedFrameSize.ppln = io_triplet.fusedFrameSize.ppln;

    //imgFused = new unsigned char[io_triplet.fusedFrameSize.width * io_triplet.fusedFrameSize.height * 3/ 2];


    // FusionInit
    MY__LOGD("calling fusionInit()");
    initresult = my_fusion.fusionInit(calib_data, strBinaryPath);
    if (initresult != CP_INIT_SUCCESSFUL)
    {
        MY__LOGE("fusionInit failed");
        return (initresult);
    }


    // getSupportedZoomFactorRange
    MY__LOGD("calling getSupportedZoomFactorRange()");
    zoomrange = my_fusion.getSupportedZoomFactorRange(io_triplet);
    MY__LOGD("Supported Zoom Factor Range is: %.2f-%.2f", zoomrange.minZoom, zoomrange.maxZoom);


    // setIoSize
    MY__LOGD("calling setIoSize()");
    setiosizeresult = my_fusion.setIoSize(io_triplet);
    if (setiosizeresult != CP_SUCCESS)
    {
        MY__LOGE("setIoSize failed");
        return (setiosizeresult);
    }


    // load images
    MY__LOGD("Loading Images");

    imgWide  = (uint8_t*)(pWideImg->getBufVA(0));
    imgTele  = (uint8_t*)(pTeleImg->getBufVA(0));
    imgFused = (uint8_t*)(pOutputImg->getBufVA(0));

    MY__LOGD("Loaded image params: \nWide: Width=%d,Height=%d,PPln=%d \nTele: Width=%d,Height=%d,PPln=%d \n",
             inParams.wideFrameSize.width, inParams.wideFrameSize.height, inParams.wideFrameSize.ppln,
             inParams.teleFrameSize.width, inParams.teleFrameSize.height, inParams.teleFrameSize.ppln);



    // fusion process
    MY__LOGD("calling fusionProcess()");

    processresult = my_fusion.fusionProcess(imgWide, imgTele, inParams, imgFused, outParams);
    if (processresult != CP_FUSION_SUCCESSFUL)
    {
        MY__LOGE("fusionProcess failed");
        return (processresult);
    }

    //saveImageRGB(imgFused, outParams.fusedFrameSize.width, outParams.fusedFrameSize.height,
    //    outParams.fusedFrameSize.ppln, Fused_name);



    // DeInit MainFusion
    MY__LOGD("calling fusionDestroy()");
    destroyresult = my_fusion.fusionDestroy();
    if (destroyresult != CP_DESTROY_SUCCESSFUL)
    {
        MY__LOGE("fusionDestroy failed");
        return (destroyresult);
    }

    MY__LOGI("Fusion successfully");

    dlclose(my_fusion.ptr);
    my_fusion.ptr = NULL;

    onShutter();
    onNextCaptureReady();

    return EXIT_SUCCESS;
}
#endif



#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT == 1)
#include "DZOOM_if.h"

void FusionCtrler::doFusion02(
    ReqFrameSP main1Frame,
    ReqFrameSP main2Frame)
{
    MY__LOGD("+");

    IImageBuffer* pWideImg_org  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pTeleImg_org  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pOutputImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);

    // *** step 1. ***
    // create continue jpeg working buffer for algorithm output
    sp<IImageBuffer> jpeg_continue_imageBuf;
    createEmptyImageBuffer(jpeg_continue_imageBuf, wsize.w, wsize.h, eImgFmt_NV21, true);

#if 1
    MY__LOGI("Process SS fusion +");

    // *** step 2. ***
    // call algorithm with these three continue yuv buffer
    char* wide_yuv_ptr = (char*)pWideImg_org->getBufVA(0);
    char* tele_yuv_ptr = (char*)pTeleImg_org->getBufVA(0);
    char* jpeg_yuv_ptr = (char*)jpeg_continue_imageBuf->getBufVA(0);

    MSize wsize = pWideImg_org->getImgSize();
    MSize tsize = pTeleImg_org->getImgSize();
    MSize osize = pOutputImg->getImgSize();

    DZOOM dzoom;
    ////////////////////////
    // Init setting
    ////////////////////////
    DZOOM_stat_params statParams;
    statParams.in_Config_path = "";
    statParams.in_MV_stat_path = "";
    statParams.in_Calibration_File = "/sdcard/MTK_Test.bin3";
    statParams.tele_w = tsize.w; //4160
    statParams.tele_h = tsize.h; //3120
    statParams.tele_stride = tsize.w; //4160
    statParams.tele_scanline = tsize.h; //3136
    statParams.wide_w = wsize.w; //4000
    statParams.wide_h = wsize.h; //3000
    statParams.wide_stride = wsize.w; //4032
    statParams.wide_scanline = wsize.h; //3008
    statParams.out_w = osize.w;
    statParams.out_h = osize.h;
    statParams.out_stride = osize.w;
    statParams.out_scanline = osize.h;

    MY__LOGD("Wide (%dx%d) +", wsize.w, wsize.h);
    MY__LOGD("Tele (%dx%d) +", tsize.w, tsize.h);
    MY__LOGD("Jpeg (%dx%d) +", osize.w, osize.h);

    dzoom.prepare(&statParams);

    IMetadata* appMetaIn = main1Frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);

    // get AF DAC
    NS3Av3::IHal3A* hal3A_Wide = MAKE_Hal3A(0, LOG_TAG);
    DualZoomInfo_T WideZoomInfo;
    hal3A_Wide->send3ACtrl(E3ACtrl_GetDualZoomInfo, reinterpret_cast<MINTPTR>(&WideZoomInfo), 0);

    NS3Av3::IHal3A* hal3A_Tele = MAKE_Hal3A(2, LOG_TAG);
    DualZoomInfo_T TeleZoomInfo;
    hal3A_Tele->send3ACtrl(E3ACtrl_GetDualZoomInfo, reinterpret_cast<MINTPTR>(&TeleZoomInfo), 0);

    MY__LOGD("W_AF_DAC=%d, T_AF_DAC:%d", WideZoomInfo.i4AFDAC, TeleZoomInfo.i4AFDAC);

    //////////////////////////////////
    // For each capture
    ////////////////////////
    DZOOM_dyn_params dynParams;

    MINT32 zoomRatio = getZoomRatio(main1Frame);

    dynParams.out_buffer = (unsigned char*)jpeg_yuv_ptr;
    dynParams.in_pTeleNV21 = (unsigned char*)tele_yuv_ptr;
    dynParams.in_pWideNV21 = (unsigned char*)wide_yuv_ptr;
    dynParams.Zoom_factor = ((float)zoomRatio) /
                            100.0; //(float)wsize.w / (float)origCropRegion.s.w; //1.65
    dynParams.HAL_LensPosition_wide = WideZoomInfo.i4AFDAC;
    dynParams.HAL_LensPosition_tele = TeleZoomInfo.i4AFDAC;
    dynParams.fusion_enbled;
    dzoom.setDynParams(&dynParams);

    int stat = dzoom.run();
    dzoom.finish();

#endif


    // *** step 3. ***
    // copy continue jpeg yuv buffer back to jpeg buffer for output jpeg result
    // jpeg_continue_imageBuf ==> pOutputImg

    MUINT32 outOrientaion = main1Frame->getOrientation(RequestFrame::eRequestImg_FullOut);

    MINT32 angle = 0;
    switch(outOrientaion)
    {
        case eTransform_ROT_90:
            angle = 90;
            break;
        case eTransform_ROT_180:
            angle = 180;
            break;
        case eTransform_ROT_270:
            angle = 270;
            break;
    }

    MY__LOGD("move to jpeg buffer angle:%d", angle);
    sMDP_Config config;
    memset(&config, 0, sizeof(sMDP_Config));

    config.pDpStream = mpDpStream;
    config.pSrcBuffer = jpeg_continue_imageBuf.get();
    config.pDstBuffer = pOutputImg;
    config.rotAngle = angle;
    if (!excuteMDP(config))
    {
        MY__LOGE("excuteMDP fail. 1");
    }


    // postview for fast UI display
    IImageBuffer* pFramePostview = main1Frame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut);
    if (NULL != pFramePostview)
    {
        MY__LOGD("move to post view");
        sMDP_Config config;
        memset(&config, 0, sizeof(sMDP_Config));

        config.pDpStream = mpDpStream;
        config.pSrcBuffer = jpeg_continue_imageBuf.get();
        config.pDstBuffer = pFramePostview;
        config.rotAngle = 0;
        if (!excuteMDP(config))
        {
            MY__LOGE("excuteMDP fail. 2");
        }
    }

    // thumbnail
    IImageBuffer* pFrameYuvThub = main1Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
    if (NULL != pFrameYuvThub)
    {
        MY__LOGD("move to thumbnail");
        sMDP_Config config;
        memset(&config, 0, sizeof(sMDP_Config));

        config.pDpStream = mpDpStream;
        config.pSrcBuffer = jpeg_continue_imageBuf.get();
        config.pDstBuffer = pFrameYuvThub;
        config.rotAngle = 0;
        if (!excuteMDP(config))
        {
            MY__LOGE("excuteMDP fail. 4");
        }
    }

    // *** step 4. ***
    // finish and notify
    MY__LOGW("Good Picture !");
    onShutter();
    onNextCaptureReady();

    MY__LOGD("-");
}
#endif



#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT == 1)
#define STRING(s)        #s
#define APPEND_S(p, x)   STRING(p##x)
#define APPEND(p, x)     p##x

// ********************************************************
// To-Do: modify VNAME to real vendor name (ex: Mediatek)
#define PREFIX(x)        APPEND(VNAME, x)
#define PREFIX_STRING(x) APPEND_S(VNAME, x)
#define LIB_SUCCESS      D_VNAME_LIB_SUCCESS
// To-Do: modify VENDOR_NAME to real vendor name (ex: Mediatek)
// ********************************************************

#define LOCAL_HEADER     PREFIX_STRING(DualCamZoomLib.h)

#include LOCAL_HEADER

void FusionCtrler::doFusion03(
    ReqFrameSP main1Frame,
    ReqFrameSP main2Frame)
{
    MY__LOGD("+");

    IImageBuffer* pWideImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pTeleImg  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
    IImageBuffer* pOutputImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);

    MSize wsize = pWideImg->getImgSize();
    MSize tsize = pTeleImg->getImgSize();
    MSize osize = pOutputImg->getImgSize();

    // get ratio
    MINT32 zoomRatio = getZoomRatio(main1Frame);

    // *** step 1. ***
    // create continue jpeg working buffer for algorithm output
    sp<IImageBuffer> jpeg_continue_imageBuf;
    createEmptyImageBuffer(jpeg_continue_imageBuf, wsize.w, wsize.h, eImgFmt_NV21, true);

    if (nullptr != jpeg_continue_imageBuf.get())
    {
        // *** step 2. ***
        // call algorithm with these three continue yuv buffer
        char* wide_yuv_ptr = (char*)pWideImg->getBufVA(0);
        char* tele_yuv_ptr = (char*)pTeleImg->getBufVA(0);
        char* jpeg_temp_yuv_ptr = (char*)jpeg_continue_imageBuf->getBufVA(0);

        while (true)
        {
            MY__LOGD("START of DualCamZoom Library");

            int ret = LIB_SUCCESS;

            /* parameter for DualCamZoomLibGetVersion */
            PREFIX(DualCamZoomLibVersion_t) version;

            /* parameters for DualCamZoomLibSetGlobalParam */
            PREFIX(DualCamZoomLibGlobalParam_t) globalParam;
            globalParam.posX   = 684;
            globalParam.posY   = 474;
            globalParam.ratioX = 0.615f;
            globalParam.ratioY = 0.615f;

            /* parameter for DualCamZoomLibSetFusionAreaRatio */
            float fusionAreaRatio = 0.8f;

            /* parameters for DualCamZoomLibExecute */
            PREFIX(DualCamZoomLibSZoomParam_t) zoom;
            zoom.zoomFactor     = zoomRatio / 100.0; //1.0f;
            zoom.wideAnalogGain = 0.0f;

            MY__LOGD("[DualCamZoom] zoom.zoomFactor : %f", zoom.zoomFactor);

            PREFIX(DualCamZoomLibImage_t) wide;
            PREFIX(DualCamZoomLibImage_t) tele;
            PREFIX(DualCamZoomLibImage_t) fusion;
            wide.width  = wsize.w;
            wide.height = wsize.h;
            wide.stride = wsize.w;
            wide.p_NV21 = (unsigned char*)wide_yuv_ptr;
            tele.width  = tsize.w;
            tele.height = tsize.h;
            tele.stride = tsize.w;
            tele.p_NV21 = (unsigned char*)tele_yuv_ptr;
            fusion.p_NV21 = (unsigned char*)jpeg_temp_yuv_ptr;


            ret = PREFIX(DualCamZoomLibGetVersion)(&version);
            if (ret != LIB_SUCCESS)
            {
                MY__LOGE("[DualCamZoom] DualCamZoomLibGetVersion return %d", ret);
                break;
            }
            MY__LOGD("[DualCamZoom] DualCamZoomLibGetVersion: %d %d", version.major, version.minor);

            ret = PREFIX(DualCamZoomLibInitialize)();
            if (ret != LIB_SUCCESS)
            {
                MY__LOGE("[DualCamZoom] DualCamZoomLibInitialize return %d", ret);
                break;
            }
            MY__LOGD("[DualCamZoom] DualCamZoomLibInitialize OK");

            ret = PREFIX(DualCamZoomLibSetGlobalParam)(&globalParam);
            if (ret != LIB_SUCCESS)
            {
                MY__LOGE("[DualCamZoom] DualCamZoomSetGlobalParam return %d", ret);
                break;
            }
            MY__LOGD("[DualCamZoom] DualCamZoomSetGlobalParam OK");

            ret = PREFIX(DualCamZoomLibSetFusionAreaRatio)(fusionAreaRatio);
            if (ret != LIB_SUCCESS)
            {
                MY__LOGE("[DualCamZoom] DualCamZoomLibSetFusionAreaRatio return %d", ret);
                break;
            }
            MY__LOGD("[DualCamZoom] DualCamZoomLibSetFusionAreaRatio OK");


            /* Execute DualCamZoom Library */
            struct timespec start;
            struct timespec end;

            MY__LOGD("[DualCamZoom] DualCamZoomExecute start");
            clock_gettime(CLOCK_REALTIME, &start);
            ret = PREFIX(DualCamZoomLibExecute)(&wide, &tele, &zoom, &fusion);
            MY__LOGD("[DualCamZoom] DualCamZoomExecute end");

            clock_gettime(CLOCK_REALTIME, &end);
            double time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
            MY__LOGD("[DualCamZoom] DualCamZoomExecuteTime %lf", time);

            if (ret != LIB_SUCCESS)
            {
                MY__LOGE("[DualCamZoom] DualCamZoomExecute return %d", ret);
                break;
            }
            MY__LOGD("[DualCamZoom] DualCamZoomExecute OK");


            /* Finalization */
            ret = PREFIX(DualCamZoomLibFinalize)();
            if (ret != LIB_SUCCESS)
            {
                MY__LOGE("[DualCamZoom] DualCamZoomLibFinalize return %d", ret);
                break;
            }
            MY__LOGD("[DualCamZoom] DualCamZoomLibFinalize OK");
            break;
        }

        // *** step 3. ***
        // copy continue jpeg yuv buffer back to jpeg buffer for output jpeg result
        // jpeg_continue_imageBuf ==> pOutputImg

        MUINT32 outOrientaion = main1Frame->getOrientation(RequestFrame::eRequestImg_FullOut);

        MINT32 angle = 0;
        switch(outOrientaion)
        {
            case eTransform_ROT_90:
                angle = 90;
                break;
            case eTransform_ROT_180:
                angle = 180;
                break;
            case eTransform_ROT_270:
                angle = 270;
                break;
        }

        MY__LOGD("move to jpeg buffer angle:%d", angle);
        sMDP_Config config;
        memset(&config, 0, sizeof(sMDP_Config));

        config.pDpStream = mpDpStream;
        config.pSrcBuffer = jpeg_continue_imageBuf.get();
        config.pDstBuffer = pOutputImg;
        MY__LOGD("[DualCamZoom] output w = %d", osize.w);
        MY__LOGD("[DualCamZoom] output h = %d", osize.h);

        MY__LOGD("[DualCamZoom] Crop");
#define aspect_judge 1.5f

        float aspect_ratio;
        if (osize.h > osize.w)
        {
            aspect_ratio = (float) osize.h/(float)osize.w;
        }
        else
        {
            aspect_ratio = (float) osize.w/(float)osize.h;
        }
        MY__LOGD("[DualCamZoom] aspect_ratio = %f", aspect_ratio);

        if (aspect_ratio < aspect_judge)
        {
            //Crop offest size 4:3
            config.src_roi.x = 0 ;
            config.src_roi.y = 0 ;
            config.src_roi.w = wsize.w ;
            config.src_roi.h = wsize.h ;
        }
        else
        {
            //Crop offest size 16 : 9
            config.src_roi.x = 0 ;
            config.src_roi.y =  (wsize.h - wsize.w*9/16)/2;
            config.src_roi.w = wsize.w ;
            config.src_roi.h = wsize.w*9/16 ;
            MY__LOGD("[DualCamZoom] 16 :9 offest = %d", config.src_roi.y );
            MY__LOGD("[DualCamZoom] 16 :9 h = %d", config.src_roi.h);
        }
        config.rotAngle = angle;
        if (!excuteMDP(config))
        {
            MY__LOGE("excuteMDP fail. 1");
        }


        // postview for fast UI display
        IImageBuffer* pFramePostview = main1Frame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut);
        if (NULL != pFramePostview)
        {
            MY__LOGD("move to post view");
            sMDP_Config config;
            memset(&config, 0, sizeof(sMDP_Config));

            config.pDpStream = mpDpStream;
            config.pSrcBuffer = jpeg_continue_imageBuf.get();
            config.pDstBuffer = pFramePostview;
            config.rotAngle = 0;
            if (!excuteMDP(config))
            {
                MY__LOGE("excuteMDP fail. 2");
            }
        }

        // thumbnail
        IImageBuffer* pFrameYuvThub = main1Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
        if (NULL != pFrameYuvThub)
        {
            MY__LOGD("move to thumbnail");
            sMDP_Config config;
            memset(&config, 0, sizeof(sMDP_Config));

            config.pDpStream = mpDpStream;
            config.pSrcBuffer = jpeg_continue_imageBuf.get();
            config.pDstBuffer = pFrameYuvThub;
            config.rotAngle = 0;
            if (!excuteMDP(config))
            {
                MY__LOGE("excuteMDP fail. 3");
            }
        }

        // *** step 4. ***
        // finish and notify
        MY__LOGW("Good Picture !");
        onShutter();
        onNextCaptureReady();
    }
    else
    {
        MY__LOGE("create continue jpeg yuv buffer error");
    }
}

#endif
