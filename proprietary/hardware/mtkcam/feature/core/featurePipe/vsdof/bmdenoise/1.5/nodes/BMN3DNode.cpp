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
#include "BMN3DNode.h"

#include <ui/gralloc_extra.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "BMN3DNode"
#include <featurePipe/core/include/PipeLog.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NS3Av3;

/*******************************************************************************
 *
 ********************************************************************************/
BMN3DNode::
BMN3DNode(const char* name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mJobQueue);
}

/*******************************************************************************
 *
 ********************************************************************************/
BMN3DNode::
~BMN3DNode()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMN3DNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    if(mpN3DHAL_CAP)
    {
        delete mpN3DHAL_CAP;
        mpN3DHAL_CAP = nullptr;
    }

    mJobQueue.clear();
    mBufPool.uninit();
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
onUninit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    cleanUp();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_NAME("BMN3DNode::onThreadStart");
    // BufferPool init
    VSDOF_LOGD("BufferPoolSet init");
    CAM_TRACE_BEGIN("Denoise::BMN3DNode::mBufPoolSet::init");
    initBufferPool();
    CAM_TRACE_END();
    // create N3DHAL instance for scenarios
    N3D_HAL_INIT_PARAM initParam_CAP;
    initParam_CAP.eScenario  = eSTEREO_SCENARIO_CAPTURE;
    initParam_CAP.fefmRound  = DENOISE_CONST_FE_EXEC_TIMES;
    mpN3DHAL_CAP = N3D_HAL::createInstance(initParam_CAP);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
onThreadStop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    // cleanUp(); //WHy?
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
onData(DataID data, ImgInfoMapPtr& P2ImgInfo)
{
    MBOOL ret = MTRUE;
    VSDOF_PRFLOG("+ : dataID=%d reqId=%d", data, P2ImgInfo->getRequestPtr()->getRequestNo());

    switch(data)
    {
        case P2AFM_TO_N3D_FEFM_CCin:
            mJobQueue.enque(P2ImgInfo);
            break;
        default:
            MY_LOGW("Unrecongnized DataID=%d", data);
            ret = MFALSE;
            break;
    }

    VSDOF_LOGD("-");
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    ImgInfoMapPtr P2ImgInfo;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mJobQueue.deque(P2ImgInfo) )
    {
        MY_LOGE("BMN3DNode mJobQueue.deque() failed");
        return MFALSE;
    }
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_PRFLOG("threadLoop start, reqID=%d", P2ImgInfo->getRequestNo());
    CAM_TRACE_NAME("BMN3DNode::onThreadLoop");

    MBOOL ret;
    {
        sp<IStopWatch> spStopWatch = P2ImgInfo->getRequestPtr()->getStopWatchCollection()->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_n3d");
        ret = performN3DALGO_CAP(P2ImgInfo);
    }

    VSDOF_PRFLOG("threadLoop end! reqID=%d", P2ImgInfo->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();

    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
performN3DALGO_CAP(ImgInfoMapPtr& rP2ImgInfo)
{
    CAM_TRACE_NAME("BMN3DNode::performN3DALGO_CAP");

    N3D_HAL_PARAM_CAPTURE n3dParams;
    N3D_HAL_OUTPUT_CAPTURE n3dOutput;
    PipeRequestPtr pPipeReq = rP2ImgInfo->getRequestPtr();
    // prepare input params
    MBOOL bRet = prepareN3DInputParam_CAP(rP2ImgInfo, n3dParams);
    //output ImgInfoMap
    ImgInfoMapPtr pToDPEImgInfo = new ImageBufInfoMap(pPipeReq);
    // prepare output params
    bRet &= prepareN3DOutputParam_CAP(rP2ImgInfo, n3dOutput, pToDPEImgInfo);
    //
    if(!bRet)
    {
        MY_LOGE("reqID=%d, failed to prepare CAP N3D params to run N3D Hal.", rP2ImgInfo->getRequestNo());
        return MFALSE;
    }

    SimpleTimer timer;
    VSDOF_PRFLOG("start N3D(CAP) ALGO, reqID=%d t=%d", rP2ImgInfo->getRequestNo(), timer.startTimer());

    CAM_TRACE_BEGIN("BMN3DNode::N3DHALRun");
    bool ret = mpN3DHAL_CAP->N3DHALRun(n3dParams, n3dOutput);
    CAM_TRACE_END();

    debugN3DParams_Cap(n3dParams, n3dOutput);

    if(ret)
    {
        VSDOF_PRFLOG("finsished N3D(CAP) ALGO, reqID=%d, exe-time=%f msec", rP2ImgInfo->getRequestNo(), timer.countTimer());

        // output to WPE
        handleData(WPE_TO_DPE_WARP_RESULT, pToDPEImgInfo);
    }
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
prepareN3DInputParam_CAP(ImgInfoMapPtr& rP2ImgInfo, N3D_HAL_PARAM_CAPTURE& rN3dParam)
{
    CAM_TRACE_NAME("prepareN3DInputParam_CAP");
    VSDOF_PRFLOG("prepareN3DInputParam_CAP +");
    // prepare FEFM input
    prepareFEFMData(rP2ImgInfo, rN3dParam.hwfefmData);

    // prepare Rect/CC input
    SmartImageBuffer smCCIn1 = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_CC_IN1);
    SmartImageBuffer smCCIn2 = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_CC_IN2);

    SmartImageBuffer   smRectIn1 = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_RECT_IN1_CAP);

    /* For using WPE Node in the future
    SmartImageBuffer smRectIn2 = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_RECT_IN2_CAP);
    smRectIn2->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    rN3dParam.rectifyImgMain2 = smRectIn2->mImageBuffer.get();
    */

    smCCIn1->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smCCIn2->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smRectIn1->mImageBuffer->syncCache(eCACHECTRL_INVALID);

    // set Rect/CC
    rN3dParam.ccImage[0] = smCCIn1->mImageBuffer.get();
    rN3dParam.ccImage[1] = smCCIn2->mImageBuffer.get();

    rN3dParam.rectifyImgMain1 = smRectIn1->mImageBuffer.get();

    // main2 graphic buffer for rectify (For using N3D warping)
    SmartGraphicBuffer smRectIn2 = rP2ImgInfo->getGraphicBuffer(BID_P2AFM_OUT_RECT_IN2_CAP);
    android::sp<NativeBufferWrapper> pRectInGraBuf = smRectIn2->mGraphicBuffer;
    // config graphic buffer to BT601_FULL (For using N3D warping)
    gralloc_extra_ion_sf_info_t info;
    gralloc_extra_query(pRectInGraBuf->getHandle(), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
    gralloc_extra_perform(pRectInGraBuf->getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
    rN3dParam.rectifyGBMain2 = smRectIn2->mImageBuffer.get();

    // input magic number/orientation
    MBOOL bRet;
    MINT32 magicNum = 0;
    MINT32 jpegOrientation = 0;

    bRet = tryGetMetadataInRequest<MINT32>(rP2ImgInfo->getRequestPtr(), FRAME_INPUT,
                                        BID_META_IN_HAL, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum);
    if(bRet) {
        rN3dParam.magicNumber[0] = magicNum;
    } else {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta(0)!");
    }

    bRet = tryGetMetadataInRequest<MINT32>(rP2ImgInfo->getRequestPtr(), FRAME_INPUT,
                                        BID_META_IN_HAL_MAIN2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum);
    if(bRet) {
        rN3dParam.magicNumber[1] = magicNum;
    } else {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta(1)!");
    }

    bRet = tryGetMetadataInRequest<MINT32>(rP2ImgInfo->getRequestPtr(), FRAME_INPUT,
                                        BID_META_IN_APP, MTK_JPEG_ORIENTATION, jpegOrientation);
    if(!bRet) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!");
    }

    rN3dParam.requestNumber = rP2ImgInfo->getRequestPtr()->getRequestNo();
    rN3dParam.captureOrientation = jpegOrientation;

    VSDOF_PRFLOG("prepareN3DInputParam_CAP -");

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMN3DNode::
debugN3DParams_Cap(N3D_HAL_PARAM_CAPTURE& inParam, N3D_HAL_OUTPUT_CAPTURE& output)
{
    if(!mbDebugLog)
        return;

    MY_LOGD("+");
    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataMain1[%d]=%x",i, inParam.hwfefmData.geoDataMain1[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataMain2[%d]=%x",i, inParam.hwfefmData.geoDataMain2[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataLeftToRight[%d]=%x",i, inParam.hwfefmData.geoDataLeftToRight[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataRightToLeft[%d]=%x",i, inParam.hwfefmData.geoDataRightToLeft[i]);
    }

    #define LOG_IMGBUFFER(imgbuf)\
        if(imgbuf!=NULL)\
        {\
            MY_LOGD(#imgbuf "=%p size=%dx%d format=%x",imgbuf, imgbuf->getImgSize().w \
                            , imgbuf->getImgSize().h, imgbuf->getImgFormat());\
        }

    #define LOG_IMGBUFFER_WARPING_BUFFER(imgbuf)\
    if(imgbuf!=NULL)\
    {\
        MY_LOGD(#imgbuf "=%p size=%dx%d format=%x",imgbuf.get(), imgbuf->getImgSize().w \
                        , imgbuf->getImgSize().h, imgbuf->getImgFormat());\
    }

    LOG_IMGBUFFER(inParam.rectifyImgMain1);
    LOG_IMGBUFFER(inParam.rectifyGBMain2);
    LOG_IMGBUFFER(inParam.ccImage[0]);
    LOG_IMGBUFFER(inParam.ccImage[1]);
    MY_LOGD("inParam.magicNumber=%d, %d", inParam.magicNumber[0], inParam.magicNumber[1]);
    MY_LOGD("inParam.captureOrientation=%x", inParam.captureOrientation);

    LOG_IMGBUFFER(output.rectifyImgMain1);
    LOG_IMGBUFFER(output.rectifyImgMain2);
    MY_LOGD("output.maskMain1=%x",output.maskMain1);
    MY_LOGD("output.maskMain2=%x",output.maskMain2);
    MY_LOGD("output.ldcMain1=%x",output.ldcMain1);
    MY_LOGD("output.warpingMatrix=%x",output.warpingMatrix);
    MY_LOGD("output.warpingMatrixSize=%d",output.warpingMatrixSize);

    if(output.sceneInfo != nullptr)
    {
        for(int i=0; i<SCENEINFO_CNT; i++)
        {
            MY_LOGD("SceneInfo[%d]=%d", i , output.sceneInfo[i]);
        }
    }
    else
    {
        MY_LOGD("no SceneInfo!");
    }

    /* For using WPE Node in the future
    LOG_IMGBUFFER(inParam.rectifyImgMain1);
    LOG_IMGBUFFER(inParam.rectifyImgMain2);
    LOG_IMGBUFFER(inParam.ccImage[0]);
    LOG_IMGBUFFER(inParam.ccImage[1]);
    MY_LOGD("inParam.magicNumber=%d, %d", inParam.magicNumber[0], inParam.magicNumber[1]);
    MY_LOGD("inParam.captureOrientation=%x", inParam.captureOrientation);

    LOG_IMGBUFFER(output.preMaskMain2);
    LOG_IMGBUFFER(output.ccImgMain2);
    LOG_IMGBUFFER_WARPING_BUFFER(output.warpingBuffer[0]->planeBuffer[0]);
    LOG_IMGBUFFER_WARPING_BUFFER(output.warpingBuffer[0]->planeBuffer[1]);
    if (mWarpingPlanNum_M == MAX_WARPING_PLANE_COUNT)
        LOG_IMGBUFFER_WARPING_BUFFER(output.warpingBuffer[0]->planeBuffer[2]);
    LOG_IMGBUFFER_WARPING_BUFFER(output.warpingBuffer[1]->planeBuffer[0]);
    LOG_IMGBUFFER_WARPING_BUFFER(output.warpingBuffer[1]->planeBuffer[1]);
    if (mWarpingPlanNum_S == MAX_WARPING_PLANE_COUNT)
        LOG_IMGBUFFER_WARPING_BUFFER(output.warpingBuffer[1]->planeBuffer[2]);
    MY_LOGD("output.maskMain1=%x",output.maskMain1);
    MY_LOGD("output.maskMain1=%x",output.warpingMatrix);
    MY_LOGD("output.preMaskMain2=%x",output.preMaskMain2);
    */

    MY_LOGD("-");
    #undef LOG_IMGBUFFER
}

/*******************************************************************************
 *
 ********************************************************************************/
const char*
BMN3DNode::
onDumpBIDToName(BMDeNoiseBufferID BID)
{
#define MAKE_NAME_CASE(name) \
    case name: return #name;
    switch(BID)
    {
        MAKE_NAME_CASE(BID_N3D_OUT_MV_Y);
        MAKE_NAME_CASE(BID_N3D_OUT_SV_Y);
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_S);
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_M);

        /* For WPE Node Only
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_M);
        MAKE_NAME_CASE(BID_N3D_OUT_PRE_MASK_S);
        MAKE_NAME_CASE(BID_N3D_OUT_SV_CC_Y);
        */

        default:
            MY_LOGW("unknown BID:%d", BID);
            return "unknown";
    }

#undef MAKE_NAME_CASE
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
prepareN3DOutputYUVMask(
    ImgInfoMapPtr pInfoMap,
    N3D_HAL_OUTPUT_CAPTURE& rN3dParam,
    ImgInfoMapPtr pToDPEImgInfo)
{
    // request output buffers
    SmartImageBuffer smImgBuf_MV_Y   = mBufPool.getBufPool(BID_N3D_OUT_MV_Y)->request();
    SmartImageBuffer smImgBuf_SV_Y   = mBufPool.getBufPool(BID_N3D_OUT_SV_Y)->request();
    SmartImageBuffer smImgBuf_MASK_S = mBufPool.getBufPool(BID_N3D_OUT_MASK_S)->request();
    SmartImageBuffer smImgBuf_MASK_M = mBufPool.getBufPool(BID_N3D_OUT_MASK_M)->request();
    SmartImageBuffer smImgBuf_WARP_MATRIX = mBufPool.getBufPool(BID_N3D_OUT_WARPING_MATRIX)->request();
    SmartImageBuffer smImgBuf_Sceneinfo   = mBufPool.getBufPool(BID_N3D_OUT_SCENEINFO)->request();

    // insert to OutputInfoMap and fill in the N3D output params
    pToDPEImgInfo->addSmartBuffer(BID_N3D_OUT_MV_Y, smImgBuf_MV_Y);
    rN3dParam.rectifyImgMain1 = smImgBuf_MV_Y->mImageBuffer.get();

    pToDPEImgInfo->addSmartBuffer(BID_N3D_OUT_SV_Y, smImgBuf_SV_Y);
    rN3dParam.rectifyImgMain2 = smImgBuf_SV_Y->mImageBuffer.get();

    pToDPEImgInfo->addSmartBuffer(BID_N3D_OUT_MASK_M, smImgBuf_MASK_M);
    rN3dParam.maskMain1 = (MUINT8*) smImgBuf_MASK_M->mImageBuffer->getBufVA(0);

    pToDPEImgInfo->addSmartBuffer(BID_N3D_OUT_MASK_S, smImgBuf_MASK_S);
    rN3dParam.maskMain2 = (MUINT8*) smImgBuf_MASK_S->mImageBuffer->getBufVA(0);

    pToDPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARPING_MATRIX, smImgBuf_WARP_MATRIX);
    rN3dParam.warpingMatrix = (MFLOAT*) smImgBuf_WARP_MATRIX->mImageBuffer->getBufVA(0);

    pToDPEImgInfo->addSmartBuffer(BID_N3D_OUT_SCENEINFO, smImgBuf_Sceneinfo);
    rN3dParam.sceneInfo = (MINT32*) smImgBuf_Sceneinfo->mImageBuffer->getBufVA(0);

    VSDOF_LOGD("rN3dParam.rectifyImgMain1 size=%dx%d format=%x",
                smImgBuf_MV_Y->mImageBuffer->getImgSize().w, smImgBuf_MV_Y->mImageBuffer->getImgSize().h,
                smImgBuf_MV_Y->mImageBuffer->getImgFormat());

    VSDOF_LOGD("rN3dParam.rectifyImgMain2 size=%dx%d format=%x",
                smImgBuf_SV_Y->mImageBuffer->getImgSize().w, smImgBuf_SV_Y->mImageBuffer->getImgSize().h,
                smImgBuf_SV_Y->mImageBuffer->getImgFormat());

    VSDOF_LOGD("rN3dParam.maskMain1 size=%dx%d format=%x", smImgBuf_MASK_M->mImageBuffer->getImgSize().w,
                smImgBuf_MASK_M->mImageBuffer->getImgSize().h, smImgBuf_MASK_M->mImageBuffer->getImgFormat());

    VSDOF_LOGD("rN3dParam.maskMain2 size=%dx%d format=%x", smImgBuf_MASK_S->mImageBuffer->getImgSize().w,
                smImgBuf_MASK_S->mImageBuffer->getImgSize().h, smImgBuf_MASK_S->mImageBuffer->getImgFormat());

    /* For using WPE Node in the future
    // init warpingBuffer main1 & main2
    WARPING_BUFFER_CONFIG_T warpingBuffConfigMain1 = StereoSettingProvider::getWarpingBufferConfig(StereoHAL::ENUM_STEREO_SENSOR::eSTEREO_SENSOR_MAIN1);
    WARPING_BUFFER_CONFIG_T warpingBuffConfigMain2 = StereoSettingProvider::getWarpingBufferConfig(StereoHAL::ENUM_STEREO_SENSOR::eSTEREO_SENSOR_MAIN2);
    WARPING_BUFFER_T warping_buf_1 = WARPING_BUFFER_T(warpingBuffConfigMain1);
    WARPING_BUFFER_T warping_buf_2 = WARPING_BUFFER_T(warpingBuffConfigMain2);
    rN3dParam.warpingBuffer[0] = &warping_buf_1;
    rN3dParam.warpingBuffer[1] = &warping_buf_2;

    // request output buffers
    SmartImageBuffer smImgBuf_MASK_S  = mBufPool.getBufPool(BID_N3D_OUT_PRE_MASK_S)->request();
    SmartImageBuffer smImgBuf_MASK_M  = mBufPool.getBufPool(BID_N3D_OUT_MASK_M)->request();
    SmartImageBuffer smImgBuf_IMG_CC  = mBufPool.getBufPool(BID_N3D_OUT_SV_CC_Y)->request();
    SmartImageBuffer smImgBuf_WARP_1X = mBufPool.getBufPool(BID_N3D_OUT_WARP_GRID_1_X)->request();
    SmartImageBuffer smImgBuf_WARP_1Y = mBufPool.getBufPool(BID_N3D_OUT_WARP_GRID_1_Y)->request();
    SmartImageBuffer smImgBuf_WARP_1Z = {nullptr};

    if (mWarpingPlanNum_M == MAX_WARPING_PLANE_COUNT)
    {
        smImgBuf_WARP_1Z = mBufPool.getBufPool(BID_N3D_OUT_WARP_GRID_1_Z)->request();
    }
    SmartImageBuffer smImgBuf_WARP_2X = mBufPool.getBufPool(BID_N3D_OUT_WARP_GRID_2_X)->request();
    SmartImageBuffer smImgBuf_WARP_2Y = mBufPool.getBufPool(BID_N3D_OUT_WARP_GRID_2_Y)->request();
    SmartImageBuffer smImgBuf_WARP_2Z = {nullptr};

    if (mWarpingPlanNum_S == MAX_WARPING_PLANE_COUNT)
    {
        smImgBuf_WARP_2Z = mBufPool.getBufPool(BID_N3D_OUT_WARP_GRID_2_Z)->request();
    }
    SmartImageBuffer smImgBuf_WARP_MATRIX = mBufPool.getBufPool(BID_N3D_OUT_WARPING_MATRIX)->request();

    // insert to OutputInfoMap and fill in the N3D output params
    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_MASK_M, smImgBuf_MASK_M);
    rN3dParam.maskMain1 = (MUINT8*) smImgBuf_MASK_M->mImageBuffer->getBufVA(0);

    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_PRE_MASK_S, smImgBuf_MASK_S);
    rN3dParam.preMaskMain2 = smImgBuf_MASK_S->mImageBuffer.get();

    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_SV_CC_Y, smImgBuf_IMG_CC);
    rN3dParam.ccImgMain2 = smImgBuf_MASK_S->mImageBuffer.get();

    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARP_GRID_1_X, smImgBuf_WARP_1X);
    rN3dParam.warpingBuffer[0]->planeBuffer[0] = smImgBuf_WARP_1X->mImageBuffer.get();
    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARP_GRID_1_Y, smImgBuf_WARP_1Y);
    rN3dParam.warpingBuffer[0]->planeBuffer[1] = smImgBuf_WARP_1Y->mImageBuffer.get();
    if (mWarpingPlanNum_M == MAX_WARPING_PLANE_COUNT)
    {
        pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARP_GRID_1_Z, smImgBuf_WARP_1Z);
        rN3dParam.warpingBuffer[0]->planeBuffer[2] = smImgBuf_WARP_1Z->mImageBuffer.get();
    }
    else
    {
        rN3dParam.warpingBuffer[0]->planeBuffer[2] = NULL;
    }

    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARP_GRID_2_X, smImgBuf_WARP_2X);
    rN3dParam.warpingBuffer[1]->planeBuffer[0] = smImgBuf_WARP_2X->mImageBuffer.get();
    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARP_GRID_2_Y, smImgBuf_WARP_2Y);
    rN3dParam.warpingBuffer[1]->planeBuffer[1] = smImgBuf_WARP_2Y->mImageBuffer.get();
    if (mWarpingPlanNum_S == MAX_WARPING_PLANE_COUNT)
    {
        pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARP_GRID_2_Z, smImgBuf_WARP_2Z);
        rN3dParam.warpingBuffer[1]->planeBuffer[2] = smImgBuf_WARP_2Z->mImageBuffer.get();
    }
    else
    {
        rN3dParam.warpingBuffer[1]->planeBuffer[2] = NULL;
    }

    pToWPEImgInfo->addSmartBuffer(BID_N3D_OUT_WARPING_MATRIX, smImgBuf_WARP_MATRIX);
    rN3dParam.warpingMatrix = (MFLOAT*) smImgBuf_WARP_MATRIX->mImageBuffer->getBufVA(0);

    //Use N3D`s input rectifyIn Main1 for the output for WPE node
    SmartImageBuffer rectIn1_cap = pInfoMap->getSmartBuffer(BID_P2AFM_OUT_RECT_IN1_CAP);
    pToWPEImgInfo->addSmartBuffer(BID_P2AFM_OUT_RECT_IN1_CAP, rectIn1_cap);

    //For debug
    VSDOF_LOGD("rN3dParam.maskMain1 size=%dx%d format=%x", smImgBuf_MASK_M->mImageBuffer->getImgSize().w,
                smImgBuf_MASK_M->mImageBuffer->getImgSize().h, smImgBuf_MASK_M->mImageBuffer->getImgFormat());

    VSDOF_LOGD("rN3dParam.premaskMain2 size=%dx%d format=%x", smImgBuf_MASK_S->mImageBuffer->getImgSize().w,
                smImgBuf_MASK_S->mImageBuffer->getImgSize().h, smImgBuf_MASK_S->mImageBuffer->getImgFormat());

    VSDOF_LOGD("rN3dParam.warpingBuffer[0]->planeBuffer[0] size=%dx%d format=%x", smImgBuf_WARP_1X->mImageBuffer->getImgSize().w,
                smImgBuf_WARP_1X->mImageBuffer->getImgSize().h, smImgBuf_WARP_1X->mImageBuffer->getImgFormat());
    */
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
prepareN3DOutputParam_CAP(
    ImgInfoMapPtr pInfoMap,
    N3D_HAL_OUTPUT_CAPTURE& rN3dParam,
    ImgInfoMapPtr pToDPEImgInfo)
{
    // prepare YUV/MASK
    prepareN3DOutputYUVMask(pInfoMap, rN3dParam, pToDPEImgInfo);

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
prepareFEFMData(ImgInfoMapPtr& rP2ImgInfo, HWFEFM_DATA& rFefmData)
{
    CAM_TRACE_NAME("prepareFEFMData");
    VSDOF_PRFLOG("prepareFEFMData +");
    // N3D input FEO/FMO data
    SmartImageBuffer smFe1bo, smFe2bo, smFe1co, smFe2co;
    SmartImageBuffer smFmboLR, smFmboRL, smFmcoLR, smFmcoRL;

    smFe1bo = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FE1BO);
    smFe2bo = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FE2BO);
    smFe1co = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FE1CO);
    smFe2co = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FE2CO);
    smFe1bo->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smFe2bo->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smFe1co->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smFe2co->mImageBuffer->syncCache(eCACHECTRL_INVALID);

    rFefmData.geoDataMain1[0] = (MUINT16*)smFe1bo->mImageBuffer->getBufVA(0);
    rFefmData.geoDataMain1[1] = (MUINT16*)smFe1co->mImageBuffer->getBufVA(0);
    rFefmData.geoDataMain1[2] = NULL;

    rFefmData.geoDataMain2[0] = (MUINT16*)smFe2bo->mImageBuffer->getBufVA(0);
    rFefmData.geoDataMain2[1] = (MUINT16*)smFe2co->mImageBuffer->getBufVA(0);
    rFefmData.geoDataMain2[2] = NULL;

    smFmboLR = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FMBO_LR);
    smFmboRL = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FMBO_RL);
    smFmcoLR = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FMCO_LR);
    smFmcoRL = rP2ImgInfo->getSmartBuffer(BID_P2AFM_OUT_FMCO_RL);
    smFmboLR->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smFmboRL->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smFmcoLR->mImageBuffer->syncCache(eCACHECTRL_INVALID);
    smFmcoRL->mImageBuffer->syncCache(eCACHECTRL_INVALID);

    rFefmData.geoDataLeftToRight[0] = (MUINT16*) smFmboLR->mImageBuffer->getBufVA(0);
    rFefmData.geoDataLeftToRight[1] = (MUINT16*) smFmcoLR->mImageBuffer->getBufVA(0);
    rFefmData.geoDataLeftToRight[2] = NULL;

    rFefmData.geoDataRightToLeft[0] = (MUINT16*) smFmboRL->mImageBuffer->getBufVA(0);
    rFefmData.geoDataRightToLeft[1] = (MUINT16*) smFmcoRL->mImageBuffer->getBufVA(0);
    rFefmData.geoDataRightToLeft[2] = NULL;

    VSDOF_PRFLOG("prepareFEFMData -");
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMN3DNode::
initBufferPool()
{

    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    MY_LOGD("+ : initBufferPool");

    StereoSizeProvider* pSizeProvder = StereoSizeProvider::getInstance();

    // Size Query
    // img size is independent to scenarios
    N3D_OUTPUT_IMG_SIZE = pSizeProvder->getBufferSize(E_MV_Y, eSTEREO_SCENARIO_CAPTURE);
    // mask size is independent to scenarios
    N3D_OUTPUT_MASK_SIZE = pSizeProvder->getBufferSize(E_MASK_M_Y, eSTEREO_SCENARIO_CAPTURE);
    // SceneInfo size
    N3D_OUTPUT_SCENEINFO_SIZE = MSize(StereoSettingProvider::getMaxSceneInfoBufferSizeInBytes(), 1);

    MSize wrappingmatrix_inputsize(StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes(), 1);

    // Regist buffer pool
    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        NSBMDN::BufferConfig c = {
            "N3DWarpOut_M",
            BID_N3D_OUT_MV_Y,
            (MUINT32) N3D_OUTPUT_IMG_SIZE.w,
            (MUINT32) N3D_OUTPUT_IMG_SIZE.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)N3D_WORKING_BUF_SET
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "N3DWarpOut_S",
            BID_N3D_OUT_SV_Y,
            (MUINT32) N3D_OUTPUT_IMG_SIZE.w,
            (MUINT32) N3D_OUTPUT_IMG_SIZE.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)N3D_WORKING_BUF_SET
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_MASK_S",
            BID_N3D_OUT_MASK_S,
            (MUINT32)N3D_OUTPUT_MASK_SIZE.w,
            (MUINT32)N3D_OUTPUT_MASK_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "mpRectInCapBufPool_Gra_Main2",
            BID_N3D_OUT_MASK_M,
            (MUINT32)N3D_OUTPUT_MASK_SIZE.w,
            (MUINT32)N3D_OUTPUT_MASK_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_WARPING_MATRIX",
            BID_N3D_OUT_WARPING_MATRIX,
            (MUINT32)wrappingmatrix_inputsize.w,
            (MUINT32)wrappingmatrix_inputsize.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET*get_stereo_bmdenoise_capture_buffer_cnt())
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "mN3DSceneinfoBufPool",
            BID_N3D_OUT_SCENEINFO,
            (MUINT32)N3D_OUTPUT_SCENEINFO_SIZE.w,
            (MUINT32)N3D_OUTPUT_SCENEINFO_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    // Size Query
    /* For WPE Warping use only
    WARPING_BUFFER_CONFIG_T warpingBuffConfigMain1 = StereoSettingProvider::getWarpingBufferConfig(StereoHAL::ENUM_STEREO_SENSOR::eSTEREO_SENSOR_MAIN1);
    WARPING_BUFFER_CONFIG_T warpingBuffConfigMain2 = StereoSettingProvider::getWarpingBufferConfig(StereoHAL::ENUM_STEREO_SENSOR::eSTEREO_SENSOR_MAIN2);
    mWarpingPlanNum_M = warpingBuffConfigMain1.PLANE_COUNT;
    mWarpingPlanNum_S = warpingBuffConfigMain2.PLANE_COUNT;

    N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE = warpingBuffConfigMain1.BUFFER_SIZE;
    N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE = warpingBuffConfigMain2.BUFFER_SIZE;
    N3D_OUTPUT_MASK_MASK1_SIZE = pSizeProvder->getBufferSize(E_MASK_M_Y, eSTEREO_SCENARIO_CAPTURE);
    N3D_OUTPUT_PRE_MASK_MAIN2_SIZE = pSizeProvder->getBufferSize(E_PRE_MASK_S_Y, eSTEREO_SCENARIO_CAPTURE);
    N3D_OUTPUT_IMG_CC_MAIN2SIZE = pSizeProvder->getBufferSize(E_SV_CC_Y, eSTEREO_SCENARIO_CAPTURE);

    MSize wrappingmatrix_inputsize(StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes(), 1);

    // Check Warping Plane Number
    if(mWarpingPlanNum_M > MAX_WARPING_PLANE_COUNT || mWarpingPlanNum_S > MAX_WARPING_PLANE_COUNT)
    {
        MY_LOGE("WARPING_PLANE_COUNT const error! WarpingPlan_1 = %d, WarpingPlan_2 = %d", mWarpingPlanNum_M, mWarpingPlanNum_S);
    }

    // Regist buffer pool
    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_WARP_GRID_1_X",
            BID_N3D_OUT_WARP_GRID_1_X,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE.w,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE.h,
            eImgFmt_RGBA8888,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_WARP_GRID_1_Y",
            BID_N3D_OUT_WARP_GRID_1_Y,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE.w,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE.h,
            eImgFmt_RGBA8888,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    if (mWarpingPlanNum_M == MAX_WARPING_PLANE_COUNT)
    {
        // Regist buffer pool
        {
            NSBMDN::BufferConfig c = {
                "BID_N3D_OUT_WARP_GRID_1_Z",
                BID_N3D_OUT_WARP_GRID_1_Z,
                (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE.w,
                (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE.h,
                eImgFmt_RGBA8888,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(N3D_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_WARP_GRID_2_X",
            BID_N3D_OUT_WARP_GRID_2_X,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE.w,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE.h,
            eImgFmt_RGBA8888,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_WARP_GRID_2_Y",
            BID_N3D_OUT_WARP_GRID_2_Y,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE.w,
            (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE.h,
            eImgFmt_RGBA8888,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    if (mWarpingPlanNum_S == MAX_WARPING_PLANE_COUNT)
    {
        {
            NSBMDN::BufferConfig c = {
                "BID_N3D_OUT_WARP_GRID_2_Z",
                BID_N3D_OUT_WARP_GRID_2_Z,
                (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE.w,
                (MUINT32)N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE.h,
                eImgFmt_RGBA8888,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(N3D_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_PRE_MASK_S",
            BID_N3D_OUT_PRE_MASK_S,
            (MUINT32)N3D_OUTPUT_PRE_MASK_MAIN2_SIZE.w,
            (MUINT32)N3D_OUTPUT_PRE_MASK_MAIN2_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "mpRectInCapBufPool_Gra_Main2",
            BID_N3D_OUT_MASK_M,
            (MUINT32)N3D_OUTPUT_MASK_MASK1_SIZE.w,
            (MUINT32)N3D_OUTPUT_MASK_MASK1_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_SV_CC_Y",
            BID_N3D_OUT_SV_CC_Y,
            (MUINT32)N3D_OUTPUT_IMG_CC_MAIN2SIZE.w,
            (MUINT32)N3D_OUTPUT_IMG_CC_MAIN2SIZE.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    {
        NSBMDN::BufferConfig c = {
            "BID_N3D_OUT_WARPING_MATRIX",
            BID_N3D_OUT_WARPING_MATRIX,
            (MUINT32)wrappingmatrix_inputsize.w,
            (MUINT32)wrappingmatrix_inputsize.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(N3D_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }
    */

    if(!mBufPool.init(vBufConfig)){
        MY_LOGE("Error! Please check above errors!");
    }

    MY_LOGD("- : initBufferPool ");
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMN3DNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}