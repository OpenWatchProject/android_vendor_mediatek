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


// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#define PIPE_MODULE_TAG "Util"
#define PIPE_CLASS_TAG "MDPOperator"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <featurePipe/core/include/PipeLog.h>

// Local header file
#include "MdpOperator.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace std;
using namespace android;
using namespace NSCam;
using namespace VSDOF::util;
/*******************************************************************************
* Global Define
*******************************************************************************/
#define SCOPE_LOGGER          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_ULOGMD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_ULOGMD("[%s] -", p); });
}

#define SCOPE_TIMER(extInfo)          auto __scope_timer__ = create_scope_timer(__FUNCTION__, extInfo)
static std::shared_ptr<char> create_scope_timer(const char* funcName, const char* extInfo)
{
    std::string msg(funcName);
    msg = msg + "-" + extInfo;
    char* pText = const_cast<char*>(msg.c_str());
    CAM_ULOGMD("[%s] + ", pText);
    return std::shared_ptr<char>(
        pText, [](char* p){ CAM_ULOGMD("[%s] -", p); }
    );
}

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return UNKNOWN_ERROR;}}
#define CHECK_DP_RET(ret, x)                        \
    do{                                             \
        if( ret && ((x) < 0) )                      \
        {                                           \
            MY_LOGE("%s failed", #x); ret = MFALSE; \
        }                                           \
    }while(0)

#define DP_PORT_SRC   4
#define DP_PORT_DST0  0
#define DP_PORT_DST1  1
/*******************************************************************************
* External Function
********************************************************************************/


/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MdpOperator::MdpOperator(
    const char* creatorName,
    MINT32 openId
    )
  : mCreatorName(creatorName)
  , miOpenId(openId)
{
    MY_LOGD("OpenId(%d) CreatorName(%s)", miOpenId, mCreatorName);
}

MdpOperator::~MdpOperator()
{
    SCOPE_LOGGER;
}

MVOID
MdpOperator::
onLastStrongRef( const void* /*id*/)
{
    SCOPE_LOGGER;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
MdpOperator::
execute(MdpConfig& config)
{
    SCOPE_LOGGER;
    Mutex::Autolock _l(mLock);

    MBOOL ret = MTRUE;

    // (1). Create Instance
    if( mpStream == nullptr ){
        SCOPE_TIMER("create DpIspStream stream");
        mpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        CHECK_OBJECT( mpStream );
    }

    // (2). config/enque src
    MINT32 srcW = 0;
    MINT32 srcH = 0;
    if( config.pSrcBuffer )
    {
        MINT32 srcStride = 0;
        // config width/height
        if(config.srcCust.size.w != 0 || config.srcCust.size.h != 0){
            srcW = config.srcCust.size.w;
            srcH = config.srcCust.size.h;
        }else{
            srcW = config.pSrcBuffer->getImgSize().w;
            srcH = config.pSrcBuffer->getImgSize().h;
        }

        // config stride
        if(config.srcCust.custStride){
            srcStride = config.srcCust.size.w*(config.pSrcBuffer->getImgBitsPerPixel()/8);
        }else if(config.srcCust.planeIdx != -1){
            srcStride = config.pSrcBuffer->getBufStridesInBytes(config.srcCust.planeIdx);
        }else{
            srcStride = config.pSrcBuffer->getBufStridesInBytes(0);
        }

        ret = ret && configPort(
            DP_PORT_SRC,
            config.pSrcBuffer,
            srcW,
            srcH,
            srcStride,
            config.srcCust.planeIdx
            );
        ret = ret && enqueBuffer(DP_PORT_SRC, config.pSrcBuffer, config.srcCust.planeIdx);

        MY_LOGD("src(%dx%d_%zu) custom(%dx%d) useCusStride(%d) specPlane(%d) => config(%dx%d_%d)",
            config.pSrcBuffer->getImgSize().w, config.pSrcBuffer->getImgSize().h, config.pSrcBuffer->getBufStridesInBytes(0),
            config.srcCust.size.w, config.srcCust.size.h,
            config.srcCust.custStride,
            config.srcCust.planeIdx,
            srcW, srcH, srcStride
        );
    }else{
        MY_LOGE("config.pSrcBuffer is null!");
        return MFALSE;
    }

    // (3). config/enque dst
    if( config.pDstBuffer )
    {
        MINT32 dstW = 0;
        MINT32 dstH = 0;
        MINT32 dstStride = 0;
        MUINT32 u4Rotation = 0;
        MUINT32 u4Flip = 0;

        // config width/height
        if(config.dstCust.size.w != 0 || config.dstCust.size.h != 0){
            dstW = config.dstCust.size.w;
            dstH = config.dstCust.size.h;
        }else{
            dstW = config.pDstBuffer->getImgSize().w;
            dstH = config.pDstBuffer->getImgSize().h;
        }

        // config stride
        if(config.dstCust.custStride){
            dstStride = config.dstCust.size.w*(config.pDstBuffer->getImgBitsPerPixel()/8);
        }else if(config.dstCust.planeIdx != -1){
            dstStride = config.pDstBuffer->getBufStridesInBytes(config.dstCust.planeIdx);
        }else{
            dstStride = config.pDstBuffer->getBufStridesInBytes(0);
        }

        if(config.usePQParams){
            setPQParameters(/*DP_PORT_DST0, config.featureId, config.processId*/);
        }

        ret = ret && configPort(
            DP_PORT_DST0,
            config.pDstBuffer,
            dstW,
            dstH,
            dstStride,
            config.srcCust.planeIdx,
             // should use src buffer format for specified single plane operation, e.g. src Y8- > dst I422
            (config.dstCust.planeIdx == -1) ?
                (EImageFormat)config.pDstBuffer->getImgFormat() : (EImageFormat)config.pSrcBuffer->getImgFormat()
        );

        ret = ret && convertTransform(config.trans, u4Rotation, u4Flip);

        CHECK_DP_RET( ret, mpStream->setSrcCrop( DP_PORT_DST0,
                                                 0, 0,
                                                 0, 0,
                                                 srcW, srcH // scrCrop is applied before scale/rotate, so should be srcW/srcH
                                                 ) );

        CHECK_DP_RET( ret, mpStream->setRotation(DP_PORT_DST0, u4Rotation) );
        CHECK_DP_RET( ret, mpStream->setFlipStatus(DP_PORT_DST0, u4Flip) );

        ret = ret && enqueBuffer(DP_PORT_DST0, config.pDstBuffer, config.dstCust.planeIdx);

        MY_LOGD("dst(%dx%d_%zu) custom(%dx%d) useCusStride(%d) specPlane(%d) => config(%dx%d_%d) | trans(%d) => rot(%d) flip(%d) | setPQ(%d)",
            config.pDstBuffer->getImgSize().w, config.pDstBuffer->getImgSize().h, config.pDstBuffer->getBufStridesInBytes(0),
            config.dstCust.size.w, config.dstCust.size.h,
            config.dstCust.custStride,
            config.dstCust.planeIdx,
            dstW, dstH, dstStride,
            config.trans, u4Rotation, u4Flip,
            config.usePQParams
        );
    }else{
        MY_LOGE("config.pDstBuffer is null!");
        return MFALSE;
    }

    // (4). start
    //profile.print();
    CHECK_DP_RET( ret, mpStream->startStream() );

    // (5). deque
    CHECK_DP_RET( ret, mpStream->dequeueSrcBuffer() );

    ret = ret && dequeDstBuffer(DP_PORT_DST0, config.pDstBuffer , config.dstCust.planeIdx);

    // (6). stop
    CHECK_DP_RET( ret, mpStream->stopStream() );

    if( mpStream != nullptr ){
         delete mpStream;
         mpStream = nullptr;
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
MdpOperator::
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
MdpOperator::
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
        FMT_SUPPORT(eImgFmt_RGB565 , DP_COLOR_RGB565)
        FMT_SUPPORT(eImgFmt_RGB888 , DP_COLOR_RGB888)
        FMT_SUPPORT(eImgFmt_ARGB888, DP_COLOR_ARGB8888)
        FMT_SUPPORT(eImgFmt_RGBA8888, DP_COLOR_RGBA8888)
        FMT_SUPPORT(eImgFmt_STA_BYTE, DP_COLOR_GREY)
        FMT_SUPPORT(eImgFmt_Y8     , DP_COLOR_GREY)
        case eImgFmt_BAYER12_UNPAK:
            MY_LOGD("eImgFmt_BAYER12_UNPAK, return DP_COLOR_RGB565_RAW for special MDP path");
            *(dp_fmt) = DP_COLOR_RGB565_RAW;
            break;
        case eImgFmt_BAYER14_UNPAK:
            MY_LOGD("eImgFmt_BAYER14_UNPAK, return DP_COLOR_RGB565_RAW for special MDP path");
            *(dp_fmt) = DP_COLOR_RGB565_RAW;
            break;
        case eImgFmt_Y16:
            MY_LOGD("eImgFmt_Y16, return DP_COLOR_RGB565_RAW for special MDP path");
            *(dp_fmt) = DP_COLOR_RGB565_RAW;
            break;
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
MdpOperator::
configPort(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer,
    MINT32 width, MINT32 height, MINT32 stride,
    MINT32 specifiedPlane,
    EImageFormat specifiedFmt
)
{
    MBOOL ret = MTRUE;
    DpColorFormat fmt;

    if( specifiedFmt != eImgFmt_UNKNOWN){
        if( !mapDpFormat(specifiedFmt, &fmt) ){
            return MFALSE;
        }
    }else{
        if( !mapDpFormat((NSCam::EImageFormat)pImageBuffer->getImgFormat(), &fmt) ){
            return MFALSE;
        }
    }

    MUINT32 planenum = (specifiedPlane == -1) ? pImageBuffer->getPlaneCount() : 1;
    if( port == DP_PORT_SRC )
    {
        CHECK_DP_RET( ret, mpStream->setSrcConfig(
                                        width,
                                        height,
                                        stride,
                                        planenum > 1 ? pImageBuffer->getBufStridesInBytes(1) : 0,
                                        fmt,
                                        DP_PROFILE_FULL_BT601
                                        ) );
    }
    else
    {
        CHECK_DP_RET( ret, mpStream->setDstConfig(
                                        port,
                                        width,
                                        height,
                                        stride,
                                        planenum > 1 ? pImageBuffer->getBufStridesInBytes(1) : 0,
                                        fmt,
                                        DP_PROFILE_FULL_BT601
                                        ) );
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MdpOperator::
enqueBuffer(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer,
    MINT32 specifiedPlane
)
{
    MBOOL    ret     = MTRUE;
    uint32_t va[3]   = {0};
    uint32_t pa[3]   = {0};
    uint32_t size[3] = {0};

#define checkAddr( addr, i )                       \
    do{                                            \
        if( addr == 0 )                            \
        {                                          \
            MY_LOGE( "%s == 0, p(%d)", #addr, i ); \
            return MFALSE;                         \
        }                                          \
    }while(0)

    if(specifiedPlane == -1){
        for( MUINT i = 0 ; i < pImageBuffer->getPlaneCount() ; i++ )
        {
            va[i]   = (MUINT32)pImageBuffer->getBufVA(i);
            pa[i]   = pImageBuffer->getBufPA(i);
            size[i] = pImageBuffer->getBufSizeInBytes(i);

    //        checkAddr(va[i], i);
    //        checkAddr(pa[i], i);
        }
    }else{
        MY_LOGD("specifiedPlane(%d)", specifiedPlane);
        va[0]   = (MUINT32)pImageBuffer->getBufVA(specifiedPlane);
        pa[0]   = pImageBuffer->getBufPA(specifiedPlane);
        size[0] = pImageBuffer->getBufSizeInBytes(specifiedPlane);
    }


    if( port == DP_PORT_SRC )
    {
        CHECK_DP_RET( ret, mpStream->queueSrcBuffer( (void**)va,
                                                     pa,
                                                     size,
                                                     (specifiedPlane == -1) ? pImageBuffer->getPlaneCount() : 1
                                                     ) );
    }
    else
    {
        CHECK_DP_RET( ret, mpStream->queueDstBuffer( port,
                                                     (void**)va,
                                                     pa,
                                                     size,
                                                     (specifiedPlane == -1) ? pImageBuffer->getPlaneCount() : 1
                                                     ) );
    }

#undef checkAddr
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MdpOperator::
dequeDstBuffer(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer,
    MINT32 specifiedPlane
)
{
    MBOOL ret = MTRUE;

    uint32_t va[3];
    CHECK_DP_RET( ret, mpStream->dequeueDstBuffer( port, (void**)va, true ) );

    if(specifiedPlane != -1){
        if( va[specifiedPlane] != (MUINT32)pImageBuffer->getBufVA(specifiedPlane) ){
            MY_LOGE("deque wrong buffer va(0x%x) != 0x%zu",
                    va[specifiedPlane], pImageBuffer->getBufVA(specifiedPlane));
            ret = MFALSE;
        }
    }else{
        for( MUINT i = 0 ; i < pImageBuffer->getPlaneCount() ; i++ )
        {
            if( va[i] != (MUINT32)pImageBuffer->getBufVA(i) )
            {
                MY_LOGE("deque wrong buffer va(0x%x) != 0x%zu",
                        va[i], pImageBuffer->getBufVA(i));
                ret = MFALSE;
                break;
            }
        }
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MdpOperator::
setPQParameters(/*MUINT32 port, MINT32 featureID, MINT32 processId*/)
{
#if 0
    //***************************CZ config for MDP .bin dump****************************//
    DpPqParam               ISPParam;
    ISPParam.enable         = false;
    ISPParam.scenario       = MEDIA_ISP_CAPTURE;

    ISPParam.u.isp.iso      = 0; // dummy value because PQ should be disabled
    ISPParam.u.isp.lensId     = featureID;
    ISPParam.u.isp.timestamp  = processId;
    ISPParam.u.isp.frameNo    = processId;
    ISPParam.u.isp.requestNo  = processId;

    ISPParam.u.isp.clearZoomParam.captureShot = CAPTURE_SINGLE;

    MY_LOGD("PQ params: lensId(%d) captureShot(%d) timestamp(%d) frameNo(%d) requestNo(%d) enable(%d) scenario(%d) iso(%d)",
        ISPParam.u.isp.lensId,
        ISPParam.u.isp.clearZoomParam.captureShot,
        ISPParam.u.isp.timestamp,
        ISPParam.u.isp.frameNo,
        ISPParam.u.isp.requestNo,
        ISPParam.enable,
        ISPParam.scenario,
        ISPParam.u.isp.iso
    );

    mpStream->setPQParameter(port, ISPParam);
#endif
}
