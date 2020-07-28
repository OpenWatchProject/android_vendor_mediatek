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

#define LOG_TAG "mtkcam-vsdof"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)
//
#include <mtkcam/def/ImageFormat.h>
#include <DpBlitStream.h>
#include <DpIspStream.h>
#include <DpDataType.h>
//
#include "vsdof_util.h"
//
using namespace NSCam;
//
namespace VSDOF
{
namespace util
{
DpColorFormat
getDpColorFormat(MINT32 format)
{
    switch(format)
    {
        case eImgFmt_RGBA8888:
            LOG_DBG("eImgFmt_RGBA8888");
            return DP_COLOR_RGBA8888;
        case eImgFmt_YUY2:
            LOG_DBG("eImgFmt_YUY2");
            return DP_COLOR_YUYV;
        case eImgFmt_NV16:
            LOG_DBG("eImgFmt_NV16");
            return DP_COLOR_NV16;
        case eImgFmt_NV21:
            LOG_DBG("eImgFmt_NV21");
            return DP_COLOR_NV21;
        case eImgFmt_YV12:
            LOG_DBG("eImgFmt_YV12");
            return DP_COLOR_YV12;
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y8:
            LOG_DBG("eImgFmt_Y8 eImgFmt_STA_BYTE");
            return DP_COLOR_GREY;
        case eImgFmt_NV12:
            LOG_DBG("eImgFmt_NV12");
            return DP_COLOR_NV12;
        case eImgFmt_YV16:
            LOG_DBG("eImgFmt_YV16");
            return DP_COLOR_YV16;
        case eImgFmt_RGB48:
            LOG_DBG("eImgFmt_RGB48, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        case eImgFmt_BAYER12_UNPAK:
            LOG_DBG("eImgFmt_BAYER12_UNPAK, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        case eImgFmt_BAYER14_UNPAK:
            LOG_DBG("eImgFmt_BAYER14_UNPAK, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        case eImgFmt_Y16:
            LOG_DBG("eImgFmt_Y16, return DP_COLOR_RGB565_RAW for special MDP path");
            return DP_COLOR_RGB565_RAW;
        default:
            LOG_ERR("unsupported(%x)", format);
    }
    return DP_COLOR_UNKNOWN;
}

//
MBOOL
excuteMDP(sMDP_Config config)
{
    if(config.pDpStream == nullptr)
        return MFALSE;
    //***************************src****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    size_t planeCount = config.pSrcBuffer->getPlaneCount();
    LOG_DBG("src planeCount: %d", planeCount);
    for(size_t i=0;i<planeCount;++i)
    {
        src_addr_list[i] = config.pSrcBuffer->getBufVA(i);
        src_size_list[i] = config.pSrcBuffer->getBufSizeInBytes(i);
    }
    MINT32 uvPitch = 0;
    if(planeCount>1)
    {
        uvPitch = config.pSrcBuffer->getBufStridesInBytes(1);
    }
    config.pDpStream->setSrcBuffer((void **)src_addr_list, src_size_list, planeCount);
    config.pDpStream->setSrcConfig(
                            config.pSrcBuffer->getImgSize().w,
                            config.pSrcBuffer->getImgSize().h,
                            config.pSrcBuffer->getBufStridesInBytes(0),
                            uvPitch,
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            0,
                            DP_SECURE_NONE,
                            false);
    LOG_DBG("config.pSrcBuffer->getBufStridesInBytes(0) %d", config.pSrcBuffer->getBufStridesInBytes(0));
    LOG_DBG("uvPitch %d", uvPitch);
    if(config.pDpPqParam != nullptr)
        config.pDpStream->setPQParameter(*config.pDpPqParam);
    //***************************dst********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    planeCount = config.pDstBuffer->getPlaneCount();
    LOG_DBG("dst planeCount: %d", planeCount);
    for(size_t i=0;i<planeCount;++i)
    {
        dst_addr_list[i] = config.pDstBuffer->getBufVA(i);
        dst_size_list[i] = config.pDstBuffer->getBufSizeInBytes(i);
    }
    uvPitch = 0;
    if(planeCount>1)
    {
        uvPitch = config.pDstBuffer->getBufStridesInBytes(1);
    }
    config.pDpStream->setDstBuffer((void**)dst_addr_list, dst_size_list, planeCount);
    config.pDpStream->setDstConfig(config.pDstBuffer->getImgSize().w,
                                    config.pDstBuffer->getImgSize().h,
                                    config.pDstBuffer->getBufStridesInBytes(0),
                                    uvPitch,
                                    getDpColorFormat(config.pDstBuffer->getImgFormat()),
                                    DP_PROFILE_FULL_BT601,
                                    eInterlace_None,
                                    0,
                                    DP_SECURE_NONE,
                                    false);
    config.pDpStream->setRotate(config.rotAngle);
    LOG_DBG("config.pDstBuffer->getBufStridesInBytes(0) %d", config.pDstBuffer->getBufStridesInBytes(0));
    LOG_DBG("uvPitch %d", uvPitch);
    LOG_DBG("config.rotAngle %d", config.rotAngle);
    //*******************************************************************//
    if (config.pDpStream->invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
          return MFALSE;
    }
    return MTRUE;
}

MBOOL
calcDstImageCrop(
    MSize const &srcSize,
    MSize const &dstSize,
    MSize &result,
    MPoint &retStartPoint
)
{
    LOG_DBG("srcSize=%dx%d dstSize=%dx%d",
                srcSize.w, srcSize.h, dstSize.w, dstSize.h);

    if(srcSize.h*dstSize.w>dstSize.h*srcSize.w)
    {
        // align width
        int crop_height = (1.0*dstSize.h/dstSize.w) * srcSize.w;
        crop_height = (crop_height >> 1) << 1;
        result = MSize(srcSize.w, crop_height);
        retStartPoint = MPoint(0, abs(srcSize.h-crop_height)/2);
        LOG_DBG("Align width: crop_height=%d, crop=(%d,%d), size=%dx%d"
                , crop_height, retStartPoint.x, retStartPoint.y, result.w, result.h);
    }
    else
    {
        // align height
        int crop_width = (1.0*dstSize.w/dstSize.h) * srcSize.h;
        crop_width = (crop_width >> 1) << 1;
        result = MSize(crop_width, srcSize.h);
        retStartPoint = MPoint(abs(srcSize.w-crop_width)/2, 0);
        LOG_DBG("Align height: crop_width=%d, crop=(%d,%d), size=%dx%d"
                , crop_width, retStartPoint.x, retStartPoint.y, result.w, result.h);
    }

    return MTRUE;
}

MBOOL
excuteMDPWithCrop(sMDP_Config config)
{
    if(config.pDpStream == nullptr)
    {
        return MFALSE;
    }

    // crop variable
    MSize newDstCropSize;
    MSize srcSize = config.pSrcBuffer->getImgSize();
    MSize dstSize = config.pDstBuffer->getImgSize();
    MPoint startPoint;
    if( (srcSize.w < srcSize.h) && (config.pDstBuffer->getImgSize().w > config.pDstBuffer->getImgSize().h ) )
    {
        dstSize = MSize(config.pDstBuffer->getImgSize().h, config.pDstBuffer->getImgSize().w);
    }
    if(!calcDstImageCrop(srcSize, dstSize, newDstCropSize, startPoint))
    {
        LOG_ERR("Failed to calc crop");
        return MFALSE;
    }
    DpRect srcROI(startPoint.x, startPoint.y, newDstCropSize.w, newDstCropSize.h);
    //DpRect dstROI(0, 0, newDstCropSize.w, newDstCropSize.h);

    LOG_DBG("config.pSrcBuffer->getImgSize().w: %d, config.pSrcBuffer->getImgSize().h: %d",
                config.pSrcBuffer->getImgSize().w, config.pSrcBuffer->getImgSize().h);
    LOG_DBG("config.pDstBuffer->getImgSize().w: %d, config.pDstBuffer->getImgSize().h: %d",
                config.pDstBuffer->getImgSize().w, config.pDstBuffer->getImgSize().h);
    LOG_DBG("srcSize.w: %d, srcSize.h: %d",
                srcSize.w, srcSize.h);
    LOG_DBG("dstSize.w: %d, dstSize.h: %d",
                dstSize.w, dstSize.h);
    LOG_DBG("newDstCropSize.w: %d, newDstCropSize.h: %d",
                newDstCropSize.w, newDstCropSize.h);

    //***************************src****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    size_t planeCount = config.pSrcBuffer->getPlaneCount();
    LOG_DBG("src planeCount: %d", planeCount);
    for(size_t i=0;i<planeCount;++i)
    {
        src_addr_list[i] = config.pSrcBuffer->getBufVA(i);
        src_size_list[i] = config.pSrcBuffer->getBufSizeInBytes(i);
    }
    MINT32 uvPitch = 0;
    if(planeCount>1)
    {
        uvPitch = config.pSrcBuffer->getBufStridesInBytes(1);
    }

    config.pDpStream->setSrcBuffer((void **)src_addr_list, src_size_list, planeCount);
    config.pDpStream->setSrcConfig(
                            config.pSrcBuffer->getImgSize().w,
                            config.pSrcBuffer->getImgSize().h,
                            config.pSrcBuffer->getBufStridesInBytes(0),
                            uvPitch,
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            &srcROI,  // crop
                            DP_SECURE_NONE,
                            false);
    LOG_DBG("config.pSrcBuffer->getBufStridesInBytes(0) %d", config.pSrcBuffer->getBufStridesInBytes(0));
    LOG_DBG("uvPitch %d", uvPitch);

    //***************************dst********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    planeCount = config.pDstBuffer->getPlaneCount();
    LOG_DBG("dst planeCount: %d", planeCount);
    for(size_t i=0;i<planeCount;++i)
    {
        dst_addr_list[i] = config.pDstBuffer->getBufVA(i);
        dst_size_list[i] = config.pDstBuffer->getBufSizeInBytes(i);
    }
    uvPitch = 0;
    if(planeCount>1)
    {
        uvPitch = config.pDstBuffer->getBufStridesInBytes(1);
    }
    config.pDpStream->setDstBuffer((void**)dst_addr_list, dst_size_list, planeCount);
    config.pDpStream->setDstConfig(config.pDstBuffer->getImgSize().w,
                                    config.pDstBuffer->getImgSize().h,
                                    config.pDstBuffer->getBufStridesInBytes(0),
                                    uvPitch,
                                    getDpColorFormat(config.pDstBuffer->getImgFormat()),
                                    DP_PROFILE_FULL_BT601,
                                    eInterlace_None,
                                    0,
                                    DP_SECURE_NONE,
                                    false);
    config.pDpStream->setRotate(config.rotAngle);
    LOG_DBG("config.pDstBuffer->getBufStridesInBytes(0) %d", config.pDstBuffer->getBufStridesInBytes(0));
    LOG_DBG("uvPitch %d", uvPitch);
    LOG_DBG("config.rotAngle %d", config.rotAngle);
    //*******************************************************************//
    if (config.pDpStream->invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
          return MFALSE;
    }
    return MTRUE;
}

//
MBOOL
excuteMDPBayer12(sMDP_Config config)
{
    if(config.pDpStream == nullptr)
        return MFALSE;
    //***************************src****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    size_t planeCount = config.pSrcBuffer->getPlaneCount();

    for(size_t i=0;i<planeCount;++i)
    {
        src_addr_list[i] = config.pSrcBuffer->getBufVA(i);
        src_size_list[i] = config.pSrcBuffer->getBufSizeInBytes(i);
    }

    MINT32 yPitch = config.pSrcBuffer->getBufStridesInBytes(0);
    if(config.useSrcCustomizedStride){
        LOG_DBG("src use customized stride");
        yPitch = config.customizedSrcSize.w*(config.pSrcBuffer->getImgBitsPerPixel()/8);
    }
    MINT32 uvPitch = 0;

    LOG_DBG("src settings: yPitch(%d) bufSize(%dx%d) BPP(%d) plane(%d) addr(%p) addrSize(%d) customizedBufSize(%dx%d)",
        yPitch,
        config.pSrcBuffer->getImgSize().w, config.pSrcBuffer->getImgSize().h,
        config.pSrcBuffer->getImgBitsPerPixel(),
        planeCount,
        src_addr_list[0], src_size_list[0],
        config.customizedSrcSize.w, config.customizedSrcSize.h
    );

    config.pDpStream->setSrcBuffer((void **)src_addr_list, src_size_list, planeCount);
    config.pDpStream->setSrcConfig(
                            config.pSrcBuffer->getImgSize().w,
                            config.pSrcBuffer->getImgSize().h,
                            yPitch,
                            uvPitch,
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            0,
                            DP_SECURE_NONE,
                            false);
    //***************************dst********************************//
    size_t dst_planeCount = config.pDstBuffer->getPlaneCount();
    MSize dstBufferSize = config.pDstBuffer->getImgSize();

    yPitch = config.pDstBuffer->getBufStridesInBytes(0);
    if(config.useDstCustomizedStride){
        LOG_DBG("dst use customized stride");
        yPitch = config.customizedDstSize.w*(config.pDstBuffer->getImgBitsPerPixel()/8);
    }
    uvPitch = 0;

    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    for(size_t i=0;i<dst_planeCount;++i)
    {
        dst_addr_list[i] = config.pDstBuffer->getBufVA(i);
        dst_size_list[i] = config.pDstBuffer->getBufSizeInBytes(i);
        if(config.useDstCustomizedStride){
            dst_size_list[i] = config.customizedDstSize.w*config.customizedDstSize.h*(config.pDstBuffer->getImgBitsPerPixel()/8);
        }
    }

    LOG_DBG("dst settings: yPitch(%d) bufSize(%dx%d) BPP(%d) plane(%d) addr(%p) addrSize(%d) customizedBufSize(%dx%d) rot(%d)",
        yPitch,
        dstBufferSize.w, dstBufferSize.h,
        config.pDstBuffer->getImgBitsPerPixel(),
        dst_planeCount,
        dst_addr_list[0], dst_size_list[0],
        config.customizedDstSize.w, config.customizedDstSize.h,
        config.rotAngle
    );

    config.pDpStream->setDstBuffer((void**)dst_addr_list, dst_size_list, dst_planeCount);
    config.pDpStream->setDstConfig(
                                config.customizedDstSize.w,
                                config.customizedDstSize.h,
                                yPitch,
                                uvPitch,
                                getDpColorFormat(config.pDstBuffer->getImgFormat()),
                                DP_PROFILE_FULL_BT601,
                                eInterlace_None,
                                0,
                                DP_SECURE_NONE,
                                false);
    config.pDpStream->setRotate(config.rotAngle);
    //*******************************************************************//
    if (config.pDpStream->invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
          return MFALSE;
    }
    return MTRUE;
}
//
MBOOL
excuteMDPSubProcess( sMDP_Config config, sMDP_extConfig extConfig )
{
    if(config.pDpStream == nullptr)
        return MFALSE;

    LOG_DBG("excuteMDPSubProcess: getSrcBufSize:%d getDstBufSize:%d",
        config.pSrcBuffer->getBufSizeInBytes(0),
        config.pDstBuffer->getBufSizeInBytes(0)
    );
    //***************************src****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    MINT32 uvPitch = 0;
    size_t planeCount = config.pSrcBuffer->getPlaneCount();

    MINT32 srcWidth, srcHeight, srcYPitch, srcBufferSize;
    srcWidth = extConfig.srcSize.w;
    srcHeight = extConfig.srcSize.h;
    srcYPitch = extConfig.srcStride;
    srcBufferSize = extConfig.srcBufSize;

    for(size_t i=0;i<planeCount;++i)
    {
        src_addr_list[i] = config.pSrcBuffer->getBufVA(i) + extConfig.srcOffset;
        src_size_list[i] = srcBufferSize;
    }

     LOG_DBG("excuteMDPSubProcess: src:%p src:%dx%d, srcYPitch:%d, bufSize:%d, planeCount:%d",
        src_addr_list[0],
        srcWidth,
        srcHeight,
        srcYPitch,
        srcBufferSize,
        planeCount
    );

    config.pDpStream->setSrcBuffer((void **)src_addr_list, src_size_list, planeCount);
    config.pDpStream->setSrcConfig(
                            srcWidth,
                            srcHeight,
                            srcYPitch,
                            uvPitch,
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            0,
                            DP_SECURE_NONE,
                            false);
    //***************************dst********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    planeCount = config.pDstBuffer->getPlaneCount();

    MINT32 dstWidth, dstHeight, dstYPitch, dstBufferSize;
    dstWidth = extConfig.dstSize.w;
    dstHeight = extConfig.dstSize.h;
    dstYPitch = extConfig.dstStride;
    dstBufferSize = extConfig.dstBufSize;

    for(size_t i=0;i<planeCount;++i)
    {
        dst_addr_list[i] = config.pDstBuffer->getBufVA(i) + extConfig.dstOffset;
        dst_size_list[i] = dstBufferSize;
    }

    LOG_DBG("excuteMDPSubProcess: dst:%p dst:%dx%d, dstYPitch:%d, bufSize:%d, planeCount:%d, config.rotAngle:%d",
        dst_addr_list[0],
        dstWidth,
        dstHeight,
        dstYPitch,
        dstBufferSize,
        planeCount,
        config.rotAngle
    );

    config.pDpStream->setDstBuffer((void**)dst_addr_list, dst_size_list, planeCount);
    config.pDpStream->setDstConfig(
                                    dstWidth,
                                    dstHeight,
                                    dstYPitch,
                                    uvPitch,
                                    getDpColorFormat(config.pDstBuffer->getImgFormat()),
                                    DP_PROFILE_FULL_BT601,
                                    eInterlace_None,
                                    0,
                                    DP_SECURE_NONE,
                                    false);
    config.pDpStream->setRotate(config.rotAngle);
    //*******************************************************************//
    if (config.pDpStream->invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
          return MFALSE;
    }
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MBOOL excuteMDP_toSpecificPlane(
                        sMDP_Config config,
                        MINT32 specifiedPlaneSrc,
                        MINT32 specifiedPlaneDst)
{
    if(config.pDpStream == nullptr)
        return MFALSE;


    // source settings
    if(config.pSrcBuffer->getPlaneCount() < specifiedPlaneSrc + 1){
        LOG_ERR("src planeCount:%d < specifiedPlaneSrc:%d",
            config.pSrcBuffer->getPlaneCount(),
            specifiedPlaneSrc
        );
        return MFALSE;
    }

    MINTPTR src_addr_list[1] = {0};
    unsigned int src_size_list[1] = {0};
    src_addr_list[0] = config.pSrcBuffer->getBufVA(specifiedPlaneSrc);
    src_size_list[0] = config.pSrcBuffer->getBufSizeInBytes(specifiedPlaneSrc);

    config.pDpStream->setSrcBuffer((void **)src_addr_list, src_size_list, 1);

    config.pDpStream->setSrcConfig(
                            config.pSrcBuffer->getImgSize().w,
                            config.pSrcBuffer->getImgSize().h,
                            config.pSrcBuffer->getBufStridesInBytes(specifiedPlaneSrc),
                            0,
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            0,
                            DP_SECURE_NONE,
                            false);

    // desination settings
    if(config.pDstBuffer->getPlaneCount() < specifiedPlaneDst + 1){
        LOG_ERR("dst planeCount:%d < specifiedPlaneDst:%d",
            config.pDstBuffer->getPlaneCount(),
            specifiedPlaneDst
        );
        return MFALSE;
    }

    MINTPTR dst_addr_list[1] = {0};
    unsigned int dst_size_list[1] = {0};
    dst_addr_list[0] = config.pDstBuffer->getBufVA(specifiedPlaneDst);
    dst_size_list[0] = config.pDstBuffer->getBufSizeInBytes(specifiedPlaneDst);

    config.pDpStream->setDstBuffer((void **)dst_addr_list, dst_size_list, 1);

    config.pDpStream->setDstConfig(
                            config.pDstBuffer->getImgSize().w,
                            config.pDstBuffer->getImgSize().h,
                            config.pDstBuffer->getBufStridesInBytes(specifiedPlaneDst),
                            0,
                            // use the same format as src buffer
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            0,
                            DP_SECURE_NONE,
                            false);

    LOG_DBG("src:%dx%d @ plane[%d],  dst:%dx%d @ plane[%d], rotAngle:%d",
        config.pSrcBuffer->getImgSize().w,
        config.pSrcBuffer->getImgSize().h,
        specifiedPlaneSrc,
        config.pDstBuffer->getImgSize().w,
        config.pDstBuffer->getImgSize().h,
        specifiedPlaneDst,
        config.rotAngle
    );

     //trigger HW
    if (config.pDpStream->invalidate())
    {
          LOG_ERR("FDstream invalidate failed");
          return MFALSE;
    }
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
android::sp<IImageBuffer>
createEmptyImageBuffer(
    const IImageBufferAllocator::ImgParam imgParam,
    const char* name,
    MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    android::sp<IImageBuffer> pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}

//
MBOOL
excuteDpIspStream(sDpIspConfig config)
{
    if(config.pDpIspStream == nullptr)
        return MFALSE;
    //***************************src****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    size_t planeCount = config.pSrcBuffer->getPlaneCount();
    for(size_t i=0;i<planeCount;++i)
    {
        src_addr_list[i] = config.pSrcBuffer->getBufVA(i);
        src_size_list[i] = config.pSrcBuffer->getBufSizeInBytes(i);
    }
    MINT32 uvPitch = (planeCount > 1) ? config.pSrcBuffer->getBufStridesInBytes(1) : 0;
    config.pDpIspStream->queueSrcBuffer((void **)src_addr_list, src_size_list, planeCount);
    config.pDpIspStream->setSrcConfig(
                            config.pSrcBuffer->getImgSize().w,
                            config.pSrcBuffer->getImgSize().h,
                            config.pSrcBuffer->getBufStridesInBytes(0),
                            uvPitch,
                            getDpColorFormat(config.pSrcBuffer->getImgFormat()),
                            DP_PROFILE_FULL_BT601,
                            eInterlace_None,
                            0,
                            false);
    LOG_DBG("src buffer=%dx%d", config.pSrcBuffer->getImgSize().w, config.pSrcBuffer->getImgSize().h);
    //***************************dst********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    planeCount = config.pDstBuffer->getPlaneCount();
    for(size_t i=0;i<planeCount;++i)
    {
        dst_addr_list[i] = config.pDstBuffer->getBufVA(i);
        dst_size_list[i] = config.pDstBuffer->getBufSizeInBytes(i);
    }
    uvPitch = (planeCount > 1) ? config.pDstBuffer->getBufStridesInBytes(1) : 0;
    // currently only support
    int portIndex = 0;
    config.pDpIspStream->queueDstBuffer(portIndex, (void**)dst_addr_list, dst_size_list, planeCount);
    config.pDpIspStream->setDstConfig(portIndex, config.pDstBuffer->getImgSize().w,
                                    config.pDstBuffer->getImgSize().h,
                                    config.pDstBuffer->getBufStridesInBytes(0),
                                    uvPitch,
                                    getDpColorFormat(config.pDstBuffer->getImgFormat()),
                                    DP_PROFILE_FULL_BT601,
                                    eInterlace_None,
                                    0,
                                    false);
    config.pDpIspStream->setRotation(portIndex, config.rotAngle);
    if(config.pDpPqParam != nullptr)
        config.pDpIspStream->setPQParameter(portIndex, *config.pDpPqParam);
    LOG_DBG("dst buffer=%dx%d", config.pDstBuffer->getImgSize().w, config.pDstBuffer->getImgSize().h);
    //*******************************************************************//
    DP_STATUS_ENUM status = config.pDpIspStream->startStream();
    if(status != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("start DpIspStream failed.");
        return MFALSE;
    }
    void* pOuBuf;
    status = config.pDpIspStream->dequeueDstBuffer(portIndex, (void**)dst_addr_list, true);
    if(status != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("deque dst buffer failed.");
        return MFALSE;
    }
    status = config.pDpIspStream->dequeueFrameEnd();
    if(status != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("dequeueFrameEnd failed.");
        return MFALSE;
    }
    status = config.pDpIspStream->stopStream();
    if(status != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("stopStream failed.");
        return MFALSE;
    }

    return MTRUE;
}

}
}
