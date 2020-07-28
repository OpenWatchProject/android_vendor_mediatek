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
#include "PostProcessNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "PostProcessNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_P2_GGM "doGGM"
#define DO_YUV_MERGE "doYUVMerge"
#define DO_FINAL_PROCESS "doPostProcess"

// debug settings
#define USE_DEFAULT_ISP 0
#define USE_DEFAULT_SHADING_GAIN 0
#define USE_FS_RAW1_FOR_GGM 0
// #define GGM_INPUT_FORMAT eImgFmt_RGB48
#define GGM_INPUT_FORMAT eImgFmt_BAYER14_UNPAK

#include <PipeLog.h>

#include <DpBlitStream.h>
#include "../../util/vsdof_util.h"

#include <camera_custom_nvram.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;

#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

#define THREAD_NAME       ("P2DoneThread@PostProcessNode")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

/*******************************************************************************
 *
 ********************************************************************************/
PostProcessNode::
PostProcessNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
PostProcessNode::
~PostProcessNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    MY_LOGD("onData(request) +");

    MBOOL ret = MFALSE;
    switch(id)
    {
        case DENOISE_TO_POSTPROCESS:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    MY_LOGD("onData(request) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onData(
    DataID id,
    ImgInfoMapPtr& pImgInfo)
{
    MY_LOGD("onData(ImgInfo) +");

    MBOOL ret = MFALSE;
    switch(id)
    {
        case DENOISE_TO_POSTPROCESS:
            mImgInfoRequests.enque(pImgInfo);
            ret = MTRUE;
            break;
        default:
            MY_LOGE("onData(ImgInfoMapPtr) unknown data id :%d", id);
            ret = MFALSE;
            break;
    }

    MY_LOGD("onData(ImgInfo) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onInit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }

    {
        MY_LOGD("PostProcessNode::onInit=>new DpBlitStream");
        CAM_TRACE_BEGIN("PostProcessNode::onInit=>new DpBlitStream");
        mpDpStream = new DpBlitStream();
        CAM_TRACE_END();
    }

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onUninit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    cleanUp();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    // exit threads
    mpP2DoneThread->requestExitAndWait();
    mpP2DoneThread = nullptr;

    if(mpDpStream!= nullptr)
        delete mpDpStream;
    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("BMDENOISE_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }
    if(mp3AHal_Main2)
    {
        mp3AHal_Main2->destroyInstance("BMDENOISE_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }
    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
PostProcessNode::
getRevTrans(MINT32 oriTrans)
{
    switch(oriTrans)
    {
        case 0:
            return 0;
        case eTransform_ROT_90:
            return eTransform_ROT_270;
        case eTransform_ROT_180:
            return eTransform_ROT_180;
        case eTransform_ROT_270:
            return eTransform_ROT_90;
        default:
            MY_LOGE("unknown oriTrans:%d", oriTrans);
            return -1;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onThreadStart()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x, Main2: %x", mp3AHal_Main1, mp3AHal_Main2);

    mpP2DoneThread = new P2DoneThread(this);

    mUseLCSO = ::property_get_int32("vendor.bmdenoise.pipe.lcso", -1);

    initBufferPool();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    ImgInfoMapPtr imgInfo = nullptr;
    PipeRequestPtr pipeRequest = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }
    if( !mImgInfoRequests.deque(imgInfo) )
    {
        MY_LOGD("mImgInfoRequests.deque() failed");
        return MFALSE;
    }
    if( !mRequests.deque(pipeRequest) )
    {
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }

    this->incExtThreadDependency();
    if(!doPostProcess(pipeRequest, imgInfo)){
        MY_LOGE("failed doing doPostProcess, please check above errors!");
        this->decExtThreadDependency();
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
onP2SuccessCallback(NSIoPipe::QParams& rParams)
{
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PostProcessNode* pNode = (PostProcessNode*) (pEnqueData->mpNode);

    pNode->invokeP2DoneThread(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
invokeP2DoneThread(EnquedBufPool* pEnqueData)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    MY_LOGD("ProcessId:%s", getProcessName(pEnqueData->mProcessId));

    mpP2DoneThread->requestExitAndWait();

    mpP2DonePendingData = pEnqueData;

    mpP2DoneThread->run(PIPE_LOG_TAG);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
onP2FailedCallback(NSIoPipe::QParams& rParams)
{
    MY_LOGE("PostProcessNode operations failed!!Check above errors!");
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PostProcessNode* pNode = (PostProcessNode*) (pEnqueData->mpNode);
    pNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mPImgInfo);
    pNode->decExtThreadDependency();
    delete pEnqueData;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
doPostProcess(PipeRequestPtr request, ImgInfoMapPtr imgInfo)
{
    return doGGM(imgInfo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
doGGM(ImgInfoMapPtr imgInfo)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_P2_GGM);

    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

    // prepare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    SmartGraphicBuffer smgpBMDN_HAL_out = imgInfo->getGraphicBuffer(BID_DENOISE_HAL_OUT);
    sp<IImageBuffer> spLCSO = imgInfo->getRequestPtr()->getImageBuffer(BID_LCS_1);//PORT_LCEI
    IMetadata* pMeta_InHal_main1 = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_inApp = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_APP);
    IMetadata pMeta_InHal_Mod = (*pMeta_InHal_main1);
    TuningParam rTuningParam;

    smgpBMDN_HAL_out->mImageBuffer->syncCache(eCACHECTRL_FLUSH);

    // convert to unpak14
    // Since ALG out have padding, we need the exact content size for ROI
    StereoArea hal_out_size;
    hal_out_size = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT);

    sp<IImageBufferHeap> spBufHeap_hal_out_bayer = createImageBufferHeapInFormat(smgpBMDN_HAL_out->mImageBuffer, GGM_INPUT_FORMAT, hal_out_size.contentSize());
    sp<IImageBuffer> spBuf_hal_out_bayer = spBufHeap_hal_out_bayer->createImageBuffer();
    spBuf_hal_out_bayer->lockBuf(PIPE_LOG_TAG, ImageBufferPool::USAGE_HW);


    // output
    SmartImageBuffer smpBuf_rot_back = mBufPool.getBufPool(BID_DENOISE_HAL_OUT_ROT_BACK)->request();
    #ifdef BIT_TRUE
    SmartImageBuffer smpBuf_temp_2 = mBufPool.getBufPool(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O)->request();
    #endif

    // applyISP
    {
        // Normally fullraw are not cropped before enque to pass2, but we DO crop the MFBO, so that we have to
        // tell 3A ISP the cropped region for correct shading link
        StereoArea fullraw_crop;
        fullraw_crop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);
        trySetMetadata<MRect>(&pMeta_InHal_Mod, MTK_P1NODE_SCALAR_CROP_REGION, MRect(fullraw_crop.startPt, fullraw_crop.size));
        trySetMetadata<MSize>(&pMeta_InHal_Mod, MTK_P1NODE_RESIZER_SIZE, fullraw_crop.size);

        trySetMetadata<MUINT8>(&pMeta_InHal_Mod, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise_toGGM);
        ISPTuningConfig ispConfig = {pMeta_inApp, &pMeta_InHal_Mod, mp3AHal_Main1, MFALSE, reqNo};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP);
    }

    // setup qParam
    MINT32 revTrans = getRevTrans(mModuleTrans);

    #define USE_LCSO mUseLCSO

    MY_LOGD("USE_LCSO:%d", USE_LCSO);
    MY_LOGD("don't use LSC buffer");

    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main1, ENormalStreamTag_DeNoise) // frame 0
            .addInput(PORT_IMGI)
            #if USE_LCSO == 1
            .addInput(PORT_LCEI)
            #endif
            .addCrop(eCROP_WROT,
                MPoint(0,0),
                #if USE_FS_RAW1_FOR_GGM == 1
                smpBuf_FS_1->getImgSize(),
                #else
                spBuf_hal_out_bayer->getImgSize(),
                #endif
                smpBuf_rot_back->mImageBuffer->getImgSize()) // WROTO
            .addOutput(PORT_WROTO, revTrans)
            #ifdef BIT_TRUE
            .addOutput(PORT_IMG3O, 0)
            #endif
            .generate(qParam);

    if(!bSuccess){
        MY_LOGE("failed to create qParam template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParam);
    #ifndef GTEST
    qParamFiller.insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
    #else
    qParamFiller.insertTuningBuf(iFrameNum,             nullptr)
    #endif
                #if USE_FS_RAW1_FOR_GGM == 1
                .insertInputBuf(iFrameNum,  PORT_IMGI,  smpBuf_FS_1.get())
                #else
                #if USE_LCSO == 1
                .insertInputBuf(iFrameNum,  PORT_LCEI,  spLCSO.get())
                #endif
                .insertInputBuf(iFrameNum,  PORT_IMGI,  spBuf_hal_out_bayer.get())
                #endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, smpBuf_rot_back->mImageBuffer.get())
                #ifdef BIT_TRUE
                .insertOutputBuf(iFrameNum, PORT_IMG3O, smpBuf_temp_2->mImageBuffer.get());
                #else
                ;
                #endif

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool *pEnqueData= new EnquedBufPool(this, imgInfo->getRequestPtr(), imgInfo, BMDeNoiseProcessId::DO_GGM);
    pEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK, smpBuf_rot_back);
    #ifdef BIT_TRUE
    pEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O, smpBuf_temp_2);
    #endif
    pEnqueData->addIImageBuffData(BID_TEMP_BUFFER, spBuf_hal_out_bayer);
    pEnqueData->start();

    // callbacks
    qParam.mpfnCallback = onP2SuccessCallback;
    qParam.mpfnEnQFailCallback = onP2FailedCallback;
    qParam.mpCookie = (MVOID*) pEnqueData;

    // p2 enque
    sp<P2Operator> p2Op = mwpP2Operator.promote();
    p2Op->enque(qParam, PIPE_LOG_TAG);

    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
onGGMDone(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_END(DO_P2_GGM);

    sp<IImageBuffer> spBuf_hal_out_bayer = pEnqueData->mEnquedIImgBufMap.valueFor(BID_TEMP_BUFFER);
    spBuf_hal_out_bayer->unlockBuf(PIPE_LOG_TAG);

    pEnqueData->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );

    MY_LOGD("-, reqID=%d", reqNo);
    doYUVMerge(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
doYUVMerge(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_YUV_MERGE);

    // prpare data
    // input
    SmartImageBuffer smpBuf_rot_back = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK);

    // output
    SmartImageBuffer smpBuf_YUV = pEnqueData->mPImgInfo->getSmartBuffer(BID_PRE_PROCESS_OUT_YUV);

    // do mdp
    MBOOL ret = MFALSE;
    {
        AutoProfileUtil proflie(PIPE_LOG_TAG, __FUNCTION__);
        sMDP_Config config;
        config.pDpStream = mpDpStream;
        config.pSrcBuffer = smpBuf_rot_back->mImageBuffer.get();
        config.pDstBuffer = smpBuf_YUV->mImageBuffer.get();
        config.rotAngle = 0;
        ret = VSDOF::util::excuteMDP_toSpecificPlane(config, 0, 0);
    }

    MET_END(DO_YUV_MERGE);
    if(ret){
        MY_LOGD("-, reqID=%d", reqNo);
        doFinalProcess(pEnqueData);
    }else{
        MY_LOGE("excuteMDP_toSpecificPlane failed.");
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
doFinalProcess(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    ImgInfoMapPtr imgInfo = pEnqueData->mPImgInfo;
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_FINAL_PROCESS);

    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

    // prpare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    SmartImageBuffer smpBuf_YUV = imgInfo->getSmartBuffer(BID_PRE_PROCESS_OUT_YUV);
    SmartImageBuffer smpBuf_temp_YUV = mBufPool.getBufPool(BID_POST_PROCESS_TEMP_YV12)->request();
    IMetadata* pMeta_InHal_main1 = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_inApp = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_APP);
    TuningParam rTuningParam;

    // output
    SmartImageBuffer smpBuf_result = mBufPool.getBufPool(BID_DENOISE_FINAL_RESULT)->request();
    sp<IImageBuffer> smpBuf_result_thumb = imgInfo->getRequestPtr()->getImageBuffer(BID_THUMB_YUV);

    // format transform
    {
        AutoProfileUtil proflie(PIPE_LOG_TAG, "doFinalProcess input format convert");
        if (!formatConverter(smpBuf_YUV->mImageBuffer.get(), smpBuf_temp_YUV->mImageBuffer.get())) {
            MY_LOGE("doFinalProcess input format convert failed");
            return MFALSE;
        }
    }

    // applyISP
    {
    // force 3DNR off
        MINT32 nr3DMode = -1;
        tryGetMetadata<MINT32>(pMeta_inApp, MTK_NR_FEATURE_3DNR_MODE, nr3DMode);
        MY_LOGD("nr3DMode was:%d, will be forced off in BMDNShot");
        trySetMetadata<MINT32>(pMeta_inApp, MTK_NR_FEATURE_3DNR_MODE, MTK_NR_FEATURE_3DNR_MODE_OFF);

        trySetMetadata<MUINT8>(pMeta_InHal_main1, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise);
        ISPTuningConfig ispConfig = {pMeta_inApp, pMeta_InHal_main1, mp3AHal_Main1, MFALSE, reqNo};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP, MTRUE);
    }

    // update thumb ratio
    MRect thumbCrop(smpBuf_temp_YUV->mImageBuffer->getImgSize().w, smpBuf_temp_YUV->mImageBuffer->getImgSize().h);
    updateSrcCropByOutputRatio(thumbCrop,smpBuf_temp_YUV->mImageBuffer, smpBuf_result_thumb);

    // setup qParam
    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main1, ENormalStreamTag_Normal) // frame 0
            .addInput(PORT_IMGI)
             // WROTO
            .addCrop(eCROP_WROT,
                MPoint(0,0),
                smpBuf_temp_YUV->mImageBuffer->getImgSize(),
                smpBuf_result->mImageBuffer->getImgSize())
            .addOutput(PORT_WROTO, 0)
            // WDMAO
            .addCrop(eCROP_WDMA,
                thumbCrop.p,
                thumbCrop.s,
                smpBuf_result_thumb->getImgSize())
            .addOutput(PORT_WDMAO, 0)
            .generate(qParam);

    if(!bSuccess){
        MY_LOGE("failed to create qParam template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParam);
    qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, smpBuf_temp_YUV->mImageBuffer.get())
                #ifndef GTEST
                .insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
                #else
                .insertTuningBuf(iFrameNum,             nullptr)
                #endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, smpBuf_result->mImageBuffer.get())
                .insertOutputBuf(iFrameNum, PORT_WDMAO, smpBuf_result_thumb.get());

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }



    // save working buffers to enqueBufferPool
    EnquedBufPool* newEnqueData = new EnquedBufPool(pEnqueData, FINAL_PROCESS);
    newEnqueData->addTuningData(tuningBuf);
    newEnqueData->addBuffData(BID_DENOISE_FINAL_RESULT,     smpBuf_result);
    newEnqueData->addBuffData(BID_POST_PROCESS_TEMP_YV12,   smpBuf_temp_YUV);
    newEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK, pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK));
    #ifdef BIT_TRUE
    newEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O,   pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O));
    #endif
    newEnqueData->start();

    delete pEnqueData;

    // callbacks
    qParam.mpfnCallback = onP2SuccessCallback;
    qParam.mpfnEnQFailCallback = onP2FailedCallback;
    qParam.mpCookie = (MVOID*) newEnqueData;

    // p2 enque
    sp<P2Operator> p2Op = mwpP2Operator.promote();
    p2Op->enque(qParam, PIPE_LOG_TAG);

    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
onFinalProcessDone(EnquedBufPool* pEnqueData)
{
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    pEnqueData->stop();
    MET_END(DO_FINAL_PROCESS);

    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );
    handleFinish(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
handleFinish(EnquedBufPool* pEnqueData)
{
    PipeRequestPtr request = pEnqueData->mPImgInfo->getRequestPtr();
    MINT32 reqNo = request->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);

    sp<ImageBufInfoMap> ImgBufInfo = new ImageBufInfoMap(request);
    ImgBufInfo->addSmartBuffer(
        BID_PRE_PROCESS_OUT_YUV,
        pEnqueData->mPImgInfo->getSmartBuffer(BID_PRE_PROCESS_OUT_YUV)
    );
    ImgBufInfo->addSmartBuffer(
        BID_DENOISE_HAL_OUT_ROT_BACK,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK)
    );
    ImgBufInfo->addSmartBuffer(
        BID_DENOISE_FINAL_RESULT,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_FINAL_RESULT)
    );
    ImgBufInfo->addSmartBuffer(
        BID_POST_PROCESS_TEMP_YV12,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_POST_PROCESS_TEMP_YV12)
    );

    #ifdef BIT_TRUE
    ImgBufInfo->addSmartBuffer(
        BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O)
    );
    #endif
    delete pEnqueData;

    // handle data
    handleData(BMDENOISE_RESULT_TO_SWNR, ImgBufInfo);
    handleData(BMDENOISE_RESULT_TO_SWNR, request);
    this->decExtThreadDependency();

    MY_LOGD("-, reqID=%d", reqNo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = get_stereo_bmdenoise_capture_buffer_cnt();
    int tuningSize = get_stereo_bmdenoise_capture_buffer_cnt();

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);
        NSBMDN::BufferConfig c = {
            "mpBMDeNoiseResult_Rot_Back_bufPool",
            BID_DENOISE_HAL_OUT_ROT_BACK,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }
    #ifdef BIT_TRUE
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);

        MSize size(area.size);
        if(mModuleTrans == eTransform_ROT_90 || mModuleTrans == eTransform_ROT_270){
            swap(size.w, size.h);
        }

        NSBMDN::BufferConfig c = {
            "mpBMDeNoiseResult_Rot_Back__img3o_bufPool",
            BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O,
            (MUINT32)size.w,
            (MUINT32)size.h,
            eImgFmt_I422,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }
    #endif
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);
        NSBMDN::BufferConfig c = {
            "mpBMDeNoiseResult_final_bufPool",
            BID_DENOISE_FINAL_RESULT,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_I422,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);
        NSBMDN::BufferConfig c = {
            "mpMain1_yuv_temp_bufPool",
            BID_POST_PROCESS_TEMP_YV12,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    if(!mBufPool.init(vBufConfig, tuningSize)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PostProcessNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}

/******************************************************************************
 *
 ******************************************************************************/
void
PostProcessNode::P2DoneThread::
requestExit()
{
    FUNC_START;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
PostProcessNode::P2DoneThread::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)THREAD_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, THREAD_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
PostProcessNode::P2DoneThread::
threadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    EnquedBufPool* pEnqueData = mpNode->mpP2DonePendingData;
    mpNode->mpP2DonePendingData = nullptr;

    MY_LOGD("ProcessId:%d", pEnqueData->mProcessId);

    switch(pEnqueData->mProcessId){
        case DO_GGM:
            mpNode->onGGMDone(pEnqueData);
            break;
        case FINAL_PROCESS:
            mpNode->onFinalProcessDone(pEnqueData);
            break;
        default:
            MY_LOGE("unknown processId:%d", pEnqueData->mProcessId);
    }
    return  false;
}

