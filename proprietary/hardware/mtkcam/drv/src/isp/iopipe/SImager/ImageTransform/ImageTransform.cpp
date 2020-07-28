/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/ImageTranform"
//
#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"
//
CAM_ULOG_DECLARE_MODULE_ID(MOD_SIMAGER);
#define MY_LOGV(fmt, arg...)    CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");
//
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/drv/def/IPostProcDef.h>
#include <mtkcam/drv/iopipe/PortMap.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
//
#include <DpIspStream.h>
#include <cutils/properties.h>
//
#include "./inc/ImageTransform.h"
//
#include <cutils/compiler.h>
#include <stdlib.h>
#include  <limits.h>

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
#define CHECK_DP_RET(ret, x)                        \
    do{                                             \
        if( ret && ((x) < 0) )                      \
        {                                           \
            MY_LOGE("%s failed", #x); ret = MFALSE; \
        }                                           \
    }while(0)

using namespace android;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NSCam::NSIoPipe;
using namespace NSCam::Utils::Format;
/*******************************************************************************
*
********************************************************************************/

////////////////////////////////////////////////////////////////////////////////
#define DP_PORT_SRC   4
#define DP_PORT_DST0  0
#define DP_PORT_DST1  1
#define DIP_CROP_WMDA 2
#define DIP_CROP_WROT 3
#define USER_NAME "ImageTranform"

/**
 * Try to set metadata value
 *
 * @param[in]  pMetadata: IMetadata instance
 * @param[in]  tag: the metadata tag to set
 * @param[in]  val: the metadata value to be configured
 *
 *
 * @return
 *  -  true if successful; otherwise false.
 */
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        CAM_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


template <typename T>
inline bool
tryGetMetadata(IMetadata const *pMetadata, MUINT32 tag, T& rVal)
{
    if (pMetadata == nullptr) return MFALSE;

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty())
    {
        rVal = entry.itemAt(0,Type2Type<T>());
        return true;
    }
    else
    {
#define var(v) #v
#define type(t) #t
        MY_LOGW("no metadata %s in %s", var(tag), type(pMetadata));
#undef type
#undef var
    }
    return false;
}

MUINT32 queryPlanePixel(MUINT32 fmt, MUINT32 i, MUINT32 width, MUINT32 height)
{
    MUINT32 pixel;
    pixel = (queryPlaneWidthInPixels(fmt,i, width) * queryPlaneBitsPerPixel(fmt,i) / 8) * queryPlaneHeightInPixels(fmt, i, height);
    return pixel;
}

MUINT32 queryStrideInPixels(MUINT32 fmt, MUINT32 i, MUINT32 width)
{
    MUINT32 pixel;
    pixel = queryPlaneWidthInPixels(fmt, i, width) * queryPlaneBitsPerPixel(fmt, i) / 8;
    return pixel;
}
/*******************************************************************************
*
********************************************************************************/
std::atomic<int> ImageTransform::miTaskID;

ImageTransform::
ImageTransform(
    const char* sUserName,
    MINT32 iSensorIdx
)
    : mi4ErrorCode(0)
    , mpStream(NULL)
    , miSensorIndex(iSensorIdx)
    , msUserName(sUserName)
{
    static MBOOL sInit;
    static MBOOL sSupportCZ;
    static MBOOL sSupportDRE;
    static MBOOL sSupportHFG;

    if (!sInit) {

        std::map<DP_ISP_FEATURE_ENUM, bool> features;
        DpIspStream::queryISPFeatureSupport(features);
        sSupportCZ  = features[ISP_FEATURE_CLEARZOOM];
        sSupportDRE = features[ISP_FEATURE_DRE];
        sSupportHFG = features[ISP_FEATURE_HFG];
        sInit = MTRUE;
    }

    mSupportDRE = sSupportDRE;
    mSupportCZ = sSupportCZ;
    mSupportHFG = sSupportHFG;

    mpIspHal = nullptr;
    mpNormalStream = nullptr;
    // init semaphore
    sem_init(&mSemaphore, 0, 0);
    miUniqueKey = rand() % INT_MAX;
    mbAddIMG3ODump = ::property_get_int32("vendor.debug.camera.imgtrmdump", 0);
    std::replace(msUserName.begin(), msUserName.end(), '/', '_');
    mbEnableLog = ::property_get_int32("vendor.debug.camera.log.simager", 0);
    MY_LOGD("username:%s uniqueKey=%d mbEnableLog=%d", msUserName.c_str(), miUniqueKey, mbEnableLog);
}


/*******************************************************************************
*
********************************************************************************/
ImageTransform::
~ImageTransform(
)
{
    if( mpStream != NULL )
    {
         delete mpStream;
         mpStream = NULL;
    }

    if (mpIspHal != nullptr) {
        mpIspHal->destroyInstance(USER_NAME);
        mpIspHal = nullptr;
    }
    //
    sem_destroy(&mSemaphore);
    //
    if(mpNormalStream != nullptr)
    {
        mpNormalStream->uninit(USER_NAME);
        mpNormalStream->destroyInstance();
        mpNormalStream = nullptr;
    }

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
execute(
    IImageBuffer const * pSrcBuf,
    IImageBuffer const * pDstBuf_0,
    IImageBuffer const * pDstBuf_1,
    MRect const rROI,
    MUINT32 const u4Transform_0,
    MUINT32 const u4TimeOutInMs,
    MBOOL const bLastOut,
    struct timeval *endTime,
    MBOOL const bNeedDump __unused
)
{
    MY_LOGD_IF(mbEnableLog, "src %p, dst %p/%p, (%d, %d, %dx%d), trans %d +",
            pSrcBuf, pDstBuf_0, pDstBuf_1,
            rROI.p.x, rROI.p.y, rROI.s.w, rROI.s.h,
            u4Transform_0);
    MBOOL ret = MTRUE;

    if( !pSrcBuf || (!pDstBuf_0 && !pDstBuf_1) )
    {
        MY_LOGE("not config enough port, src(%p), dst0(%p), dst1(%p)",
                pSrcBuf, pDstBuf_0, pDstBuf_1);
        ret = MFALSE;
    }

    if (!lock(u4TimeOutInMs))
    {
        MY_LOGE("[execute] lock fail ");
        return MFALSE;
    }

    if (!bLastOut)
    {
        auto& cfg = mvPQParam[pDstBuf_0 ? DP_PORT_DST0 : DP_PORT_DST1];
        //shouldn't set dre command to "UnInitialize" for first out
        if (cfg.dre.type == IImageTransform::DREParam::HisType::His_One_Time)
            cfg.dre.type = IImageTransform::DREParam::HisType::His_Conti;
    }

    // (1). Create Instance
    if( mpStream == NULL )
    {
        mpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        CHECK_OBJECT( mpStream );
    }

    // (2). config/enque src
    if( pSrcBuf )
    {
        ret = ret && configPort(DP_PORT_SRC, pSrcBuf);
        CHECK_DP_RET( ret, mpStream->setSrcCrop( rROI.p.x, 0,
                                                 rROI.p.y, 0,
                                                 rROI.s.w, rROI.s.h
                                                 ) );
        ret = ret && enqueBuffer(DP_PORT_SRC, pSrcBuf);
    }

    // (3). config/enque dst
    MUINT32 u4Rotation = 0;
    MUINT32 u4Flip = 0;
    if( pDstBuf_0 )
    {
        ret = ret && configPort(DP_PORT_DST0, pDstBuf_0)
            && convertTransform(u4Transform_0, u4Rotation, u4Flip);

        CHECK_DP_RET( ret, mpStream->setRotation(DP_PORT_DST0, u4Rotation) );
        CHECK_DP_RET( ret, mpStream->setFlipStatus(DP_PORT_DST0, u4Flip) );

        ret = ret && enqueBuffer(DP_PORT_DST0, pDstBuf_0);
    }

    if( pDstBuf_1 )
    {
        ret = ret && configPort(DP_PORT_DST1, pDstBuf_1)
            && convertTransform(u4Transform_0, u4Rotation, u4Flip);

        CHECK_DP_RET( ret, mpStream->setRotation(DP_PORT_DST1, u4Rotation) );
        CHECK_DP_RET( ret, mpStream->setFlipStatus(DP_PORT_DST1, u4Flip) );

        ret = ret && enqueBuffer(DP_PORT_DST1, pDstBuf_1);
    }

    // (4). start
    //profile.print();
    if(endTime != nullptr){
        CHECK_DP_RET( ret, mpStream->startStream(endTime) );
    }
    else{
        CHECK_DP_RET( ret, mpStream->startStream() );
    }

    // (5). stop
    CHECK_DP_RET( ret, mpStream->stopStream() );

    // (6). deque
    CHECK_DP_RET( ret, mpStream->dequeueSrcBuffer() );

    if( pDstBuf_0 ) {
        ret = ret && dequeDstBuffer(DP_PORT_DST0, pDstBuf_0 );
    }

    if( pDstBuf_1 ) {
        ret = ret && dequeDstBuffer(DP_PORT_DST1, pDstBuf_1 );
    }

    CHECK_DP_RET( ret, mpStream->dequeueFrameEnd() );

    if( !ret && mpStream != NULL )
    {
        delete mpStream;
        mpStream = NULL;
    }
    //
    MY_LOGD_IF(mbEnableLog, "-");
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
setPQParameter(
    const IImageTransform::PQParam& p
)
{
    mvPQParam[p.portIdx] = p;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
setSensorIndex(
    MINT32 iSensorIdx
)
{
    miSensorIndex = iSensorIdx;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
const map<int,IImageTransform::PQParam>&
ImageTransform::
getPQParameter() const
{
    return mvPQParam;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
execute(
    IImageBuffer const *pSrcBuf,
    IImageBuffer const * pDstBuf_0,
    IImageBuffer const * pDstBuf_1,
    MRect const rROI_0,
    MRect const rROI_1,
    MUINT32 const u4Transform_0,
    MUINT32 const u4Transform_1,
    MUINT32 const u4TimeOutInMs,
    struct timeval *endTime,
    MBOOL const bNeedDump __unused
)
{
    MY_LOGD_IF(mbEnableLog, "src %p, dst1 %p (%d, %d, %dx%d) trans %d, dst2 %p (%d, %d, %dx%d) trans %d +",
            pSrcBuf,
            pDstBuf_0, rROI_0.p.x, rROI_0.p.y, rROI_0.s.w, rROI_0.s.h, u4Transform_0,
            pDstBuf_1, rROI_1.p.x, rROI_1.p.y, rROI_1.s.w, rROI_1.s.h, u4Transform_1);
    MBOOL ret = MTRUE;

    if( !pSrcBuf || (!pDstBuf_0 && !pDstBuf_1) )
    {
        MY_LOGE("not config enough port, src(%p), dst0(%p), dst1(%p)",
                pSrcBuf, pDstBuf_0, pDstBuf_1);
        ret = MFALSE;
    }

    if (!lock(u4TimeOutInMs))
    {
        MY_LOGE("[execute] lock fail ");
        return MFALSE;
    }

    // (1). Create Instance
    if( mpStream == NULL )
    {
        mpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        CHECK_OBJECT( mpStream );
    }

    // (2). config/enque src
    if( pSrcBuf )
    {
        ret = ret && configPort(DP_PORT_SRC, pSrcBuf);

        ret = ret && enqueBuffer(DP_PORT_SRC, pSrcBuf);
    }

    // (3). config/enque dst
    MUINT32 u4Rotation = 0;
    MUINT32 u4Flip = 0;
    if( pDstBuf_0 )
    {
        ret = ret && configPort(DP_PORT_DST0, pDstBuf_0)
            && convertTransform(u4Transform_0, u4Rotation, u4Flip);

        CHECK_DP_RET( ret, mpStream->setSrcCrop( DP_PORT_DST0,
                                                 rROI_0.p.x, 0,
                                                 rROI_0.p.y, 0,
                                                 rROI_0.s.w, rROI_0.s.h
                                                 ) );

        CHECK_DP_RET( ret, mpStream->setRotation(DP_PORT_DST0, u4Rotation) );
        CHECK_DP_RET( ret, mpStream->setFlipStatus(DP_PORT_DST0, u4Flip) );

        ret = ret && enqueBuffer(DP_PORT_DST0, pDstBuf_0);
    }

    if( pDstBuf_1 )
    {
        ret = ret && configPort(DP_PORT_DST1, pDstBuf_1)
            && convertTransform(u4Transform_1, u4Rotation, u4Flip);

        CHECK_DP_RET( ret, mpStream->setSrcCrop( DP_PORT_DST1,
                                                 rROI_1.p.x, 0,
                                                 rROI_1.p.y, 0,
                                                 rROI_1.s.w, rROI_1.s.h
                                                 ) );

        CHECK_DP_RET( ret, mpStream->setRotation(DP_PORT_DST1, u4Rotation) );
        CHECK_DP_RET( ret, mpStream->setFlipStatus(DP_PORT_DST1, u4Flip) );

        ret = ret && enqueBuffer(DP_PORT_DST1, pDstBuf_1);
    }

    // (4). start
    //profile.print();
    if(endTime != nullptr){
        CHECK_DP_RET( ret, mpStream->startStream(endTime) );
    }
    else{
        CHECK_DP_RET( ret, mpStream->startStream() );
    }

    // (5). deque
    CHECK_DP_RET( ret, mpStream->dequeueSrcBuffer() );

    if( pDstBuf_0 ) {
        ret = ret && dequeDstBuffer(DP_PORT_DST0, pDstBuf_0 );
    }

    if( pDstBuf_1 ) {
        ret = ret && dequeDstBuffer(DP_PORT_DST1, pDstBuf_1 );
    }

    CHECK_DP_RET( ret, mpStream->dequeueFrameEnd() );


    // (6). stop
    CHECK_DP_RET( ret, mpStream->stopStream() );

    if( !ret && mpStream != NULL )
    {
        delete mpStream;
        mpStream = NULL;
    }
    //
    MY_LOGD_IF(mbEnableLog, "-");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
lock(MUINT32 const u4TimeOutInMs __attribute__((unused)))
{
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
unlock()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
convertTransform(
    MUINT32 const u4Transform,
    MUINT32 & u4Rotation,
    MUINT32 & u4Flip
)
{
    MY_LOGV("u4Transform:0x%x", u4Transform);
    switch (u4Transform)
    {
#define TransCase( trans, rot, flip ) \
        case (trans):                 \
            u4Rotation = (rot);       \
            u4Flip = (flip);          \
            break;
        TransCase(0                  , 0   , 0)
        TransCase(eTransform_FLIP_H  , 0   , 1)
        TransCase(eTransform_FLIP_V  , 180 , 1)
        TransCase(eTransform_ROT_90  , 90  , 0)
        TransCase(eTransform_ROT_180 , 180 , 0)
        TransCase(eTransform_FLIP_H|eTransform_ROT_90 , 270 , 1)
        TransCase(eTransform_FLIP_V|eTransform_ROT_90 , 90  , 1)
        TransCase(eTransform_ROT_270 , 270 , 0)
        default:
            MY_LOGE("not supported transform(0x%x)", u4Transform);
            u4Rotation = 0;
            u4Flip = 0;
            return MFALSE;
            break;
#undef TransCase
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
mapDpFormat(
    NSCam::EImageFormat fmt,
    DpColorFormat* dp_fmt
)
{
    switch( fmt )
    {
#define FMT_SUPPORT( fmt, dpfmt ) \
        case fmt: \
            *(dp_fmt) = dpfmt; \
            break;
        FMT_SUPPORT(eImgFmt_YUY2   , DP_COLOR_YUYV)
        FMT_SUPPORT(eImgFmt_UYVY   , DP_COLOR_UYVY)
        FMT_SUPPORT(eImgFmt_YVYU   , DP_COLOR_YVYU)
        FMT_SUPPORT(eImgFmt_VYUY   , DP_COLOR_VYUY)
        FMT_SUPPORT(eImgFmt_NV16   , DP_COLOR_NV16)
        FMT_SUPPORT(eImgFmt_NV61   , DP_COLOR_NV61)
        FMT_SUPPORT(eImgFmt_NV21   , DP_COLOR_NV21)
        FMT_SUPPORT(eImgFmt_NV12   , DP_COLOR_NV12)
        FMT_SUPPORT(eImgFmt_YV16   , DP_COLOR_YV16)
        FMT_SUPPORT(eImgFmt_I422   , DP_COLOR_I422)
        FMT_SUPPORT(eImgFmt_YV12   , DP_COLOR_YV12)
        FMT_SUPPORT(eImgFmt_I420   , DP_COLOR_I420)
        FMT_SUPPORT(eImgFmt_Y800   , DP_COLOR_GREY)
        FMT_SUPPORT(eImgFmt_RGB565 , DP_COLOR_RGB565)
        FMT_SUPPORT(eImgFmt_RGB888 , DP_COLOR_RGB888)
        FMT_SUPPORT(eImgFmt_ARGB888, DP_COLOR_ARGB8888)
        FMT_SUPPORT(eImgFmt_RGBA8888, DP_COLOR_RGBA8888)
        // 10 bits
        FMT_SUPPORT(eImgFmt_YUV_P010    , DP_COLOR_NV12_10L)
        FMT_SUPPORT(eImgFmt_MTK_YUV_P010, DP_COLOR_NV12_10P)
        // not supported
        default:
            MY_LOGE(" fmt(0x%x) not support in DP", fmt);
            return MFALSE;
            break;
#undef FMT_SUPPORT
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
configPort(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer
)
{
    MBOOL ret = MTRUE;
    DpColorFormat fmt;

    if( !mapDpFormat((NSCam::EImageFormat)pImageBuffer->getImgFormat(), &fmt) )
    {
        return MFALSE;
    }

    if( port == DP_PORT_SRC )
    {
        MUINT32 planenum = pImageBuffer->getPlaneCount();
        CHECK_DP_RET( ret, mpStream->setSrcConfig(
                                        pImageBuffer->getImgSize().w,
                                        pImageBuffer->getImgSize().h,
                                        pImageBuffer->getBufStridesInBytes(0),
                                        planenum > 1 ? pImageBuffer->getBufStridesInBytes(1) : 0,
                                        fmt,
                                        (pImageBuffer->getColorProfile() == eCOLORPROFILE_BT601_LIMITED) ? DP_PROFILE_BT601 : DP_PROFILE_FULL_BT601
                                        ) );
    }
    else
    {
        // set clear zoom setting for output ports
        if(port == DP_PORT_DST0)
            enquePQParam(port, mvPQParam[DP_PORT_DST0]);
        if(port == DP_PORT_DST1)
            enquePQParam(port, mvPQParam[DP_PORT_DST1]);

        MUINT32 planenum = pImageBuffer->getPlaneCount();
        CHECK_DP_RET( ret, mpStream->setDstConfig(
                                        port,
                                        pImageBuffer->getImgSize().w,
                                        pImageBuffer->getImgSize().h,
                                        pImageBuffer->getBufStridesInBytes(0),
                                        planenum > 1 ? pImageBuffer->getBufStridesInBytes(1) : 0,
                                        fmt,
                                        (pImageBuffer->getColorProfile() == eCOLORPROFILE_BT601_LIMITED) ? DP_PROFILE_BT601 : DP_PROFILE_FULL_BT601
                                        ) );
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
enqueBuffer(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer
)
{
    MBOOL ret = MTRUE;
    MINTPTR va[3];
    uint32_t pa[3];
    uint32_t size[3];

#define checkAddr( addr, i )                       \
    do{                                            \
        if( addr == 0 )                            \
        {                                          \
            MY_LOGE( "%s == 0, p(%d)", #addr, i ); \
            return MFALSE;                         \
        }                                          \
    }while(0)

    String8 strEnqueLog;
    strEnqueLog += String8::format("enqueBuffer to DpIspStream: port(%d) Fmt(0x%x) planeCount(%d)", port, pImageBuffer->getImgFormat(), pImageBuffer->getPlaneCount());
    for( MUINT i = 0 ; i < pImageBuffer->getPlaneCount() ; i++ )
    {
        va[i]   = pImageBuffer->getBufVA(i);
        pa[i]   = pImageBuffer->getBufPA(i);
        size[i] = pImageBuffer->getBufSizeInBytes(i);

        strEnqueLog += String8::format(" size(%zu),VA(0x%" PRIxPTR "),PA(0x%" PRIxPTR ")", pImageBuffer->getBufSizeInBytes(i), pImageBuffer->getBufVA(i), pImageBuffer->getBufPA(i));
//        checkAddr(va[i], i);
//        checkAddr(pa[i], i);
    }

    MY_LOGD("%s", strEnqueLog.string());

    if( port == DP_PORT_SRC )
    {
        CHECK_DP_RET( ret, mpStream->queueSrcBuffer( (void**)va,
                                                     pa,
                                                     size,
                                                     pImageBuffer->getPlaneCount()
                                                     ) );
    }
    else
    {
        CHECK_DP_RET( ret, mpStream->queueDstBuffer( port,
                                                     (void**)va,
                                                     pa,
                                                     size,
                                                     pImageBuffer->getPlaneCount()
                                                     ) );
    }
#undef checkAddr
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
dequeDstBuffer(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer
)
{
    MBOOL ret = MTRUE;

    MINTPTR va[3];
    CHECK_DP_RET( ret, mpStream->dequeueDstBuffer( port, (void**)va, true ) );

    for( MUINT i = 0 ; i < pImageBuffer->getPlaneCount() ; i++ )
    {
        if( va[i] != pImageBuffer->getBufVA(i) )
        {
            MY_LOGE("deque wrong buffer va(%" PRIxPTR ") != 0x%zx",
                    va[i], pImageBuffer->getBufVA(i) );
            ret = MFALSE;
            break;
        }
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
ImageTransform::
setDumpInfo(const IImageTransform::DumpInfos& info)
{
    mDumpInfo = info;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
enquePQParam(
    MUINT32                          port,
    const IImageTransform::PQParam&  cfg
)
{
    // check port
    static_assert(DP_PORT_DST0 == 0, "DP_PORT_DST0 doesn't equal to 0, may have problem");
    static_assert(DP_PORT_DST1 == 1, "DP_PORT_DST1 doesn't equal to 1, may have problem");
    if (port != cfg.portIdx)
        return MFALSE;
    DpPqParam           dp_param;
    if(!parseDpPQParam(cfg, dp_param))
        return MFALSE;

    auto err = mpStream->setPQParameter(port, dp_param);

    if (CC_UNLIKELY(err != DP_STATUS_RETURN_SUCCESS)) {
        MY_LOGE("port(%d), setPQParameter fail(%d)", port, err);
        return MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
executeByPassISP(
    IImageBuffer const *pSrcBuf,
    IImageBuffer const * pDstBuf_0,
    IImageBuffer const * pDstBuf_1,
    MRect const rROI_0,
    MUINT32 const u4Transform_0,
    MUINT32 const u4TimeOutInMs,
    struct timeval *endTime,
    MBOOL const bNeedDump
)
{
    return this->executeByPassISP(pSrcBuf, pDstBuf_0, pDstBuf_1, rROI_0, rROI_0, u4Transform_0, u4Transform_0, u4TimeOutInMs, endTime, bNeedDump);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
executeByPassISP(
    IImageBuffer const *pSrcBuf,
    IImageBuffer const * pDstBuf_0,
    IImageBuffer const * pDstBuf_1,
    MRect const rROI_0,
    MRect const rROI_1,
    MUINT32 const u4Transform_0,
    MUINT32 const u4Transform_1,
    MUINT32 const u4TimeOutInMs,
    struct timeval *endTime,
    MBOOL const bNeedDump
)
{
    if(miSensorIndex == -1)
    {
        MY_LOGE("Please configure the correct sensor index before run the API");
        return MFALSE;
    }
    if (mpIspHal == nullptr)
    {
        mpIspHal = MAKE_HalISP(miSensorIndex, USER_NAME);
        if (mpIspHal == nullptr)
        {
            MY_LOGE("cannot make Isp Hal");
            return MFALSE;
        }
    }
    if(mpNormalStream == nullptr)
    {
        mpNormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIndex);
        mpNormalStream->init(USER_NAME);
    }

    IImageBuffer* pImgBuf_IMG3O = nullptr;
    if(mbAddIMG3ODump)
    {
        auto format = eImgFmt_MTK_YUV_P010;
        int plane = queryPlaneCount(format);
        size_t stride[3];
        size_t boundary[3] = {0, 0, 0};
        for( unsigned idx = 0; idx < plane; ++idx )
        {
            stride[idx] = queryStrideInPixels(format, idx, pSrcBuf->getImgSize().w);
        }
        IImageBufferAllocator::ImgParam imgParam(format, pSrcBuf->getImgSize(), stride, boundary, plane);
        pImgBuf_IMG3O = IImageBufferAllocator::getInstance()->alloc("ImgTransIMG3O", imgParam);
        pImgBuf_IMG3O->lockBuf("ImgTransIMG3O",  eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
    }
    //1. ISP Tuning
    void* pTuningData = ::malloc(INormalStream::getRegTableSize());
    if(!pTuningData)
    {
        MY_LOGE("Tuning data failed to malloc.");
        return MFALSE;
    }
    memset(pTuningData, 0, INormalStream::getRegTableSize());
    TuningParam tuningParam;
    tuningParam.pRegBuf = pTuningData;
    tuningParam.pLcsBuf = NULL;
    MetaSet_T inMetaSet;
    IMetadata metaApp;
    IMetadata metaHal;
    inMetaSet.appMeta = metaApp;
    inMetaSet.halMeta = metaHal;
    MY_LOGD_IF(mbEnableLog, "setP2ISP +");
    trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, 8);
    mpIspHal->setP2Isp(0, inMetaSet, &tuningParam, NULL);
    MY_LOGD_IF(mbEnableLog, "setP2ISP -");
    // Enque cookie
    EnqueCookie *pCookie = new EnqueCookie(this, pTuningData, miTaskID++);
    // 2. Fill FrameParams
    FrameParams frameParam;
    frameParam.mStreamTag = NSPostProc::ENormalStreamTag_Vss;
    frameParam.mSensorIdx = miSensorIndex;
    frameParam.mTuningData = pTuningData;
    if (endTime != nullptr){
        frameParam.ExpectedEndTime = *endTime;
    }
    if(mDumpInfo.FrameNo != -1)
    {
        frameParam.FrameNo = mDumpInfo.FrameNo;
        frameParam.RequestNo = mDumpInfo.RequestNo;
        frameParam.Timestamp = mDumpInfo.Timestamp;
    }
    else if(mvPQParam.size() > 0)
    {
        frameParam.FrameNo = mvPQParam.begin()->second.frameNo;
        frameParam.RequestNo = mvPQParam.begin()->second.requestNo;
        frameParam.Timestamp = mvPQParam.begin()->second.timestamp;
    }
    frameParam.UniqueKey = miUniqueKey;
    // 2.1 Input
    Input imgi;
    imgi.mPortID = PORT_IMGI;
    imgi.mBuffer = const_cast<IImageBuffer*>(pSrcBuf);
    frameParam.mvIn.push_back(imgi);
    // 2.2 Output + PQ Param
    MBOOL bNeedPQ = MFALSE;
    NSIoPipe::PQParam p2PQParam;
    if(pDstBuf_0 != nullptr)
    {
        Output wdmao;
        wdmao.mPortID = PORT_WDMAO;
        wdmao.mBuffer = const_cast<IImageBuffer*>(pDstBuf_0);
        wdmao.mTransform = u4Transform_0;
        frameParam.mvOut.push_back(wdmao);
        // CropInfo
        MCrpRsInfo cropInfo;
        cropInfo.mGroupID = DIP_CROP_WMDA;
        cropInfo.mCropRect = rROI_0;
        cropInfo.mResizeDst=pDstBuf_0->getImgSize();
        frameParam.mvCropRsInfo.push_back(cropInfo);
        // PqPqParam
        if(mvPQParam.count(DP_PORT_DST0)>0)
        {
            bNeedPQ = MTRUE;
            DpPqParam *pDpPqParam = new DpPqParam();
            pCookie->mvDpPqParam.push_back(pDpPqParam);
            if(!parseDpPQParam(mvPQParam[DP_PORT_DST0], *pDpPqParam))
            {
                MY_LOGE("Failed to parse DPPqParam.");
                if (pCookie->mpTuningData != nullptr)
                    free(pCookie->mpTuningData);
                for (auto item : pCookie->mvDpPqParam)
                    delete item;
                delete pCookie;
                return MFALSE;
            }
            p2PQParam.WDMAPQParam = (MVOID*) pDpPqParam;
        }
        MY_LOGD_IF(mbEnableLog, "add WDMA output");
    }
    if(pDstBuf_1 != nullptr)
    {
        Output wroto;
        wroto.mPortID = PORT_WROTO;
        wroto.mBuffer = const_cast<IImageBuffer*>(pDstBuf_1);
        wroto.mTransform = u4Transform_1;
        frameParam.mvOut.push_back(wroto);
        // CropInfo
        MCrpRsInfo cropInfo;
        cropInfo.mGroupID = DIP_CROP_WROT;
        cropInfo.mCropRect = rROI_1;
        cropInfo.mResizeDst=pDstBuf_1->getImgSize();
        frameParam.mvCropRsInfo.push_back(cropInfo);
        // PqPqParam
        if(mvPQParam.count(DP_PORT_DST1)>0)
        {
            bNeedPQ = MTRUE;
            DpPqParam *pDpPqParam = new DpPqParam();
            pCookie->mvDpPqParam.push_back(pDpPqParam);
            if(!parseDpPQParam(mvPQParam[DP_PORT_DST1], *pDpPqParam))
            {
                MY_LOGE("Failed to parse DPPqParam.");
                if (pCookie->mpTuningData != nullptr)
                    free(pCookie->mpTuningData);
                for (auto item : pCookie->mvDpPqParam)
                    delete item;
                delete pCookie;
                return MFALSE;
            }
            p2PQParam.WROTPQParam = (MVOID*) pDpPqParam;
        }
        MY_LOGD_IF(mbEnableLog, "add WROT output");
    }
    // img3o
    if(mbAddIMG3ODump)
    {
        Output img3o;
        img3o.mPortID = PORT_IMG3O;
        img3o.mBuffer = pImgBuf_IMG3O;
        frameParam.mvOut.push_back(img3o);
    }
    // config extra param
    ExtraParam extra;
    extra.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
    extra.moduleStruct = &p2PQParam;
    frameParam.mvExtraParam.push_back(extra);
    // 3.0 Fill QParam
    QParams enqueParam;
    enqueParam.mpfnCallback = onP2Callback;
    enqueParam.mpfnEnQFailCallback = onP2FailedCallback;
    enqueParam.mpCookie = (MVOID*)pCookie;
    enqueParam.mvFrameParams.add(frameParam);
    MY_LOGD_IF(mbEnableLog, "executeByPassISP: taskID=%d enque start! frameNO=%d reqNo=%d uniqueKey=%d",
    pCookie->miTaskID, frameParam.FrameNo, frameParam.RequestNo, frameParam.UniqueKey);
    if(!mpNormalStream->enque(enqueParam))
    {
        MY_LOGE("executeByPassISP: taskID=%d enque failed!", pCookie->miTaskID);
        if (pCookie->mpTuningData != nullptr)
            free(pCookie->mpTuningData);
        for (auto item : pCookie->mvDpPqParam)
            delete item;
        delete pCookie;
        return MFALSE;
    }
    MY_LOGD_IF(mbEnableLog, "executeByPassISP wait");
    sem_wait(&mSemaphore);
    MY_LOGD_IF(mbEnableLog, "executeByPassISP wait done!");
    //dump img3o
    if(mbAddIMG3ODump && bNeedDump)
    {
        TuningUtils::FILE_DUMP_NAMING_HINT hint;
        hint.UniqueKey          = miUniqueKey;
        hint.RequestNo          = mDumpInfo.RequestNo;
        hint.FrameNo            = mDumpInfo.FrameNo;
        extract_by_SensorOpenId(&hint, miSensorIndex);
        extract(&hint, pImgBuf_IMG3O);

        const int DUMP_FILE_NAME_SIZE = 1024;
        char writepath[DUMP_FILE_NAME_SIZE] = {0};
        char filename[DUMP_FILE_NAME_SIZE] = {0};
        snprintf(filename, DUMP_FILE_NAME_SIZE, "%s_IMG3O", msUserName.c_str());
        extract(&hint, pImgBuf_IMG3O);
        genFileName_YUV(writepath, DUMP_FILE_NAME_SIZE, &hint, TuningUtils::YUV_PORT_IMG3O, filename);
        pImgBuf_IMG3O->saveToFile(writepath);
        IImageBufferAllocator::getInstance()->free(pImgBuf_IMG3O);
    }
    //
    MBOOL bSuccess = pCookie->isSuccess();
    // free memory
    free(pCookie->mpTuningData);
    for(DpPqParam*& item: pCookie->mvDpPqParam)
    {
        delete item;
    }
    delete pCookie;

    return bSuccess;
}

MVOID
ImageTransform::
onP2Callback(QParams& rParams)
{
    EnqueCookie* pEnqueData = (EnqueCookie*) (rParams.mpCookie);
    MY_LOGD_IF(pEnqueData->mpUser->mbEnableLog, "onP2Callback: taskID=%d done!", pEnqueData->miTaskID);
    pEnqueData->updateResult(true);
    sem_post(&pEnqueData->mpUser->mSemaphore);
}

MVOID
ImageTransform::
onP2FailedCallback(QParams& rParams)
{
    EnqueCookie* pEnqueData = (EnqueCookie*) (rParams.mpCookie);
    MY_LOGE("onP2FailedCallback: taskID=%d dequed failed!", pEnqueData->miTaskID);
    pEnqueData->updateResult(false);
    sem_post(&pEnqueData->mpUser->mSemaphore);
}

MBOOL
ImageTransform::
parseDpPQParam(
    const IImageTransform::PQParam&  cfg,
    DpPqParam& rDpPqParam
)
{
    if (!cfg.enable)
        return MFALSE;

    if (!mSupportCZ && !mSupportDRE) {
        MY_LOGD_IF(mbEnableLog, "NOT SUPPORT BOTH CLEARZOOM(%d) DRE(%d)", mSupportCZ, mSupportDRE);
        return MFALSE;
    }

    static auto __getCaptureShot = [](IImageTransform::Mode m)
    {
        switch (m) {
            case IImageTransform::Mode::Capture_Single:
                return CAPTURE_SINGLE;

            case IImageTransform::Mode::Capture_Multi:
                return CAPTURE_MULTI;

            default:
                MY_LOGW("Not support Mode(%#x) for CaptureShot", m);
        }
        return CAPTURE_MULTI;
    };

    static auto __getScenario = [](IImageTransform::Mode m)
    {
        switch (m) {
            case IImageTransform::Mode::Capture_Single:
            case IImageTransform::Mode::Capture_Multi:
                return MEDIA_ISP_CAPTURE;

            case IImageTransform::Mode::Preview:
                return MEDIA_ISP_PREVIEW;

            default:
                MY_LOGW("Not support Mode(%#x) for Scenario", m);
        }
        return MEDIA_PICTURE;
    };
    ClearZoomParam&     cz_param  = rDpPqParam.u.isp.clearZoomParam;
    DpDREParam&         dre_param = rDpPqParam.u.isp.dpDREParam;
    DpHFGParam&         hfg_param = rDpPqParam.u.isp.dpHFGParam;
    rDpPqParam.scenario             = __getScenario(cfg.mode);

    if(strcmp(cfg.userString, "")==0)
        strncpy(rDpPqParam.u.isp.userString, "pure", sizeof(rDpPqParam.u.isp.userString)-1);
    else
        strncpy(rDpPqParam.u.isp.userString, cfg.userString, sizeof(rDpPqParam.u.isp.userString)-1);
    rDpPqParam.u.isp.userString[sizeof(rDpPqParam.u.isp.userString)-1] = '\0';
    // Clear Zoom
    if (mSupportCZ && (cfg.type & IImageTransform::PQType::ClearZoom))
    {
        cz_param.captureShot    = __getCaptureShot(cfg.mode);
        if (rDpPqParam.scenario == MEDIA_ISP_PREVIEW)
            rDpPqParam.enable = (cfg.enable == MTRUE ? (PQ_COLOR_EN | PQ_ULTRARES_EN) : false);
        else if (rDpPqParam.scenario == MEDIA_ISP_CAPTURE)
            rDpPqParam.enable = (cfg.enable == MTRUE ? (PQ_ULTRARES_EN) : false);
        cz_param.p_customSetting = cfg.cz.p_customSetting;
        MY_LOGD_IF(mbEnableLog, "CZ shot(%d) tuningSet(%p)",
                cz_param.captureShot, cz_param.p_customSetting );
    }

    // HFG
    if (mSupportHFG && (cfg.type & IImageTransform::PQType::HFG))
    {
        rDpPqParam.enable |= PQ_HFG_EN;

        hfg_param.p_upperSetting = cfg.hfg.p_upperSetting;
        hfg_param.p_lowerSetting = cfg.hfg.p_lowerSetting;
        hfg_param.upperISO       = cfg.iso;
        hfg_param.lowerISO       = cfg.iso;
    }

    // DRE
    if (mSupportDRE && (cfg.type & IImageTransform::PQType::DRE))
    {
        if(cfg.dre.cmd < IImageTransform::DREParam::CMD::DRE_OFF)
            rDpPqParam.enable |= PQ_DRE_EN;
        dre_param.cmd =  cfg.dre.cmd==IImageTransform::DREParam::CMD::DRE_Generate ? DpDREParam::Cmd::Generate :
                        (cfg.dre.cmd==IImageTransform::DREParam::CMD::DRE_Apply ?   DpDREParam::Cmd::Apply : DpDREParam::Cmd::Default) ;
        if(cfg.dre.type == IImageTransform::DREParam::HisType::His_One_Time){
            if(cfg.dre.cmd==IImageTransform::DREParam::CMD::DRE_Generate)
                dre_param.cmd |= DpDREParam::Cmd::Initialize;
            else
                dre_param.cmd |= DpDREParam::Cmd::UnInitialize;
        }
        dre_param.userId = cfg.dre.userId;
        dre_param.buffer =    cfg.dre.pBuffer;
        dre_param.p_customSetting = cfg.dre.p_customSetting;
        dre_param.customIndex = cfg.dre.customIdx;
        MY_LOGD_IF(mbEnableLog, "DRE Id(%lld) cmd(0x%x) buf(%p) tuningSet(%p) cusIdx(%d)",
                dre_param.userId, dre_param.cmd, dre_param.buffer,
                dre_param.p_customSetting, dre_param.customIndex);
    }

    rDpPqParam.u.isp.iso            = cfg.iso;
    rDpPqParam.u.isp.timestamp      = cfg.timestamp;
    rDpPqParam.u.isp.frameNo        = cfg.frameNo;
    rDpPqParam.u.isp.requestNo      = cfg.requestNo;
    rDpPqParam.u.isp.lensId         = cfg.sensorId;
    rDpPqParam.u.isp.p_mdpSetting   = (MDPSetting*)cfg.p_mdpSetting;
    rDpPqParam.u.isp.LV             = cfg.lv_value;
    rDpPqParam.u.isp.p_faceInfor    = cfg.p_faceInfor;
    MY_LOGD_IF(mbEnableLog, "scenario:%d cfg.type:0x%x PQenable:0x%x iso:%d timestamp:%d frameNo:%d requestNo:%d sensorId:%d, LV:%d, fd:%p",
             rDpPqParam.scenario, cfg.type, rDpPqParam.enable, cfg.iso, cfg.timestamp, cfg.frameNo,
             cfg.requestNo, cfg.sensorId, cfg.lv_value, cfg.p_faceInfor);

    return MTRUE;
}
