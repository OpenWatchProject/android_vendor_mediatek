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
#include <PipeLog.h>

#include <DpBlitStream.h>
#include <camera_custom_nvram.h>
#include <mtkcam/aaa/IIspMgr.h>
#include "../exif/ExifWriter.h"

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/feature/common/vsdof/hal/1.5/inc/stereo_dp_util.h>

using NSCam::NSIoPipe::NSSImager::IImageTransform;


using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NS3Av3;

#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

#include "../../../util/vsdof_util.h"

#define THREAD_NAME       ("P2DoneThread@PostProcessNode")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

// MET tags
#define DO_P2_GGM "doGGM"
#define DO_YUV_MERGE "doYUVMerge"
#define DO_FINAL_PROCESS "doPostProcess"

// debug settings
#define USE_DEFAULT_ISP 0
#define USE_DEFAULT_SHADING_GAIN 0
#define USE_FS_RAW1_FOR_GGM 0
// #define GGM_INPUT_FORMAT eImgFmt_RGB48
#define GGM_INPUT_FORMAT eImgFmt_BAYER12_UNPAK

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
    mpP2DoneTasker->requestExit();
    mpP2DoneTasker->join();

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


    TaskQueue<PostProcessNode>::TaskTable taskTable({{DO_GGM, &PostProcessNode::onGGMDone},
                                                     {FINAL_PROCESS, &PostProcessNode::onFinalProcessDone}});
    mpP2DoneTasker = make_unique<TaskQueue<PostProcessNode>>(this, THREAD_NAME, taskTable);
    mpP2DoneTasker->run();

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
onP2SuccessCallback(QParams& rParams)
{
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PostProcessNode* pNode = (PostProcessNode*) (pEnqueData->mpNode);

    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("reqID: %d, Data: 0x%x, ProcessId: %s(%d)", reqNo, pEnqueData, getProcessName(pEnqueData->mProcessId), pEnqueData->mProcessId);
    pNode->mpP2DoneTasker->addTaskData(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PostProcessNode::
onP2FailedCallback(QParams& rParams)
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
    QParamTemplate qParamTemplate;
    // prepare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    SmartGraphicBuffer smgpBMDN_HAL_out = imgInfo->getGraphicBuffer(BID_DENOISE_HAL_OUT);
    SmartImageBuffer smpBuf_lcso = imgInfo->getSmartBuffer(BID_LCSO_1_BUF);
    IMetadata* pMeta_InHal_main1 = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_inApp = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_APP);
    IMetadata pMeta_InHal_Mod = (*pMeta_InHal_main1);
    TuningParam rTuningParam;

    smgpBMDN_HAL_out->mImageBuffer->syncCache(eCACHECTRL_FLUSH);

    // convert to unpak12
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


    SmartImageBuffer smpBuf_lcso_rot;
    {
        MSize lcsoBufferSize(smpBuf_lcso->mImageBuffer->getImgSize());
        MSize lcsoContentSize(imgInfo->getRequestPtr()->getParam(PID_LCSO_CONTENT_WIDTH),imgInfo->getRequestPtr()->getParam(PID_LCSO_CONTENT_HEIGHT));
        MY_LOGD("lcso buffer size: (%d, %d), contentSize: (%d, %d), moduleRotation: %d", lcsoBufferSize.w, lcsoBufferSize.h, lcsoContentSize.w, lcsoContentSize.h, mModuleTrans);

        if(lcsoContentSize.w == -1 || lcsoContentSize.h == -1)
        {
            MY_LOGE("invalidate lcso content size: (%d, %d)", lcsoContentSize.w, lcsoContentSize.h);
            return MFALSE;
        }

        if(lcsoBufferSize != lcsoContentSize)
        {
            smpBuf_lcso->mImageBuffer->setExtParam(lcsoContentSize, 0);
            MY_LOGD("set lcso size as content size, org: (%d, %d), content: (%d, %d)", lcsoBufferSize.w, lcsoBufferSize.h, lcsoContentSize.w, lcsoContentSize.h);
        }

        if(mModuleTrans)
        {
            smpBuf_lcso_rot = mBufPool.getBufPool(BID_LCSO_ROT_BUF)->request();

            MSize rotLcsoContentSize(lcsoContentSize);
            if(mModuleTrans == eTransform_ROT_90 || mModuleTrans == eTransform_ROT_270)
            {
                swap(rotLcsoContentSize.w, rotLcsoContentSize.h);
            }

            MY_LOGD("lcso size: (%d, %d), rot lcso size: (%d, %d)", lcsoBufferSize.w, lcsoBufferSize.h, rotLcsoContentSize.w, rotLcsoContentSize.h);
            smpBuf_lcso_rot->mImageBuffer->setExtParam(rotLcsoContentSize, 0);
            MBOOL ret = StereoDpUtil::transformImage(smpBuf_lcso->mImageBuffer.get(), smpBuf_lcso_rot->mImageBuffer.get(), eTransformToRotation(mModuleTrans));

            if(!ret)
            {
                MY_LOGE("failed doIImageTransform LCSO!");
                return MFALSE;
            }
        }
        else
        {
            smpBuf_lcso_rot = smpBuf_lcso;
        }
    }

    // applyISP
    {
        trySetMetadata<MUINT8>(&pMeta_InHal_Mod, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise_toGGM);
        if(mModuleTrans == eTransform_ROT_90 || mModuleTrans == eTransform_ROT_270){
            trySetMetadata<MBOOL>(&pMeta_InHal_Mod, MTK_3A_ISP_RAW_ROTATION, MTRUE);
        }
        ISPTuningConfig ispConfig = {pMeta_inApp, &pMeta_InHal_Mod, mp3AHal_Main1, MFALSE, reqNo, smpBuf_lcso_rot->mImageBuffer.get()};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP);
    }

    const MRect noCrop = {{0, 0}, spBuf_hal_out_bayer->getImgSize()};


    // setup qParamTemplate
    MINT32 revTrans = getRevTrans(mModuleTrans);

    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main1, ENormalStreamTag_DeNoise) // frame 0
            .addInput(PORT_IMGI)
            #ifndef GTEST
            .addInput(PORT_LCEI)
            #endif
            .addCrop(eCROP_WROT, noCrop.p, noCrop.s, smpBuf_rot_back->mImageBuffer->getImgSize()) // WROTO
            .addOutput(PORT_WROTO, revTrans)
            #ifdef BIT_TRUE
            .addOutput(PORT_IMG3O, 0)
            #endif
            .generate(qParamTemplate);
    if(!bSuccess){
        MY_LOGE("failed to create qParamTemplate template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParamTemplate.mTemplate, qParamTemplate.mStats);
        #ifndef GTEST
        qParamFiller.insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
        #else
        qParamFiller.insertTuningBuf(iFrameNum,             nullptr)
        #endif
        #ifndef GTEST
        .insertInputBuf(iFrameNum,  PORT_LCEI,  smpBuf_lcso_rot->mImageBuffer.get())
        #endif
        .insertInputBuf(iFrameNum,  PORT_IMGI,  spBuf_hal_out_bayer.get())
        #ifdef BIT_TRUE
        .insertOutputBuf(iFrameNum, PORT_IMG3O, smpBuf_temp_2->mImageBuffer.get())
        #endif
        .insertOutputBuf(iFrameNum, PORT_WROTO, smpBuf_rot_back->mImageBuffer.get());

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParamTemplate!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool *pEnqueData= new EnquedBufPool(this, imgInfo->getRequestPtr(), imgInfo, BMDeNoiseProcessId::DO_GGM);
    pEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK, smpBuf_rot_back);
    pEnqueData->addBuffData(BID_LCSO_ROT_BUF, smpBuf_lcso_rot);
    #ifdef BIT_TRUE
    pEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O, smpBuf_temp_2);
    #endif
    pEnqueData->addIImageBuffData(BID_TEMP_BUFFER, spBuf_hal_out_bayer);
    pEnqueData->start();

    imgInfo->getRequestPtr()->getStopWatchCollection()->BeginStopWatch("do_ggm", (void*)pEnqueData);

    // callbacks
    qParamTemplate.mTemplate.mpfnCallback = onP2SuccessCallback;
    qParamTemplate.mTemplate.mpfnEnQFailCallback = onP2FailedCallback;
    qParamTemplate.mTemplate.mpCookie = (MVOID*) pEnqueData;
    // p2 enque
    sp<P2Operator> p2Op = mwpP2Operator.promote();
    p2Op->enque(qParamTemplate.mTemplate, PIPE_LOG_TAG);
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

    sp<IImageBuffer> spBuf_hal_out_bayer = pEnqueData->mEnquedIImgBufMap[BID_TEMP_BUFFER];
    spBuf_hal_out_bayer->unlockBuf(PIPE_LOG_TAG);

    // reset lcso buffer size
    {
        SmartImageBuffer smpBuf_lcso_rot = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_LCSO_ROT_BUF);
        MSize poolImgSize = mBufPool.getBufPool(BID_LCSO_ROT_BUF)->getImageSize();
        MSize bufImgSize = smpBuf_lcso_rot->mImageBuffer->getImgSize();
        if(poolImgSize != bufImgSize)
        {
            MY_LOGD("reset the lcso buffer size, pool size: (%d, %d), buffer size: (%d, %d), buffer address: (%p)",
                poolImgSize.w, poolImgSize.h, bufImgSize.w, bufImgSize.h, smpBuf_lcso_rot->mImageBuffer.get());
            smpBuf_lcso_rot->mImageBuffer->setExtParam(poolImgSize, 0);
        }
    }

    pEnqueData->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );

    pEnqueData->mPImgInfo->getRequestPtr()->getStopWatchCollection()->EndStopWatch((void*)pEnqueData); //do_ggm

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
        sp<IStopWatch> pStopWatch = pEnqueData->mPImgInfo->getRequestPtr()->getStopWatchCollection()->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_yuv_merge");
        VSDOF::util::sMDP_Config config;
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
    QParamTemplate qParamTemplate;
    // prpare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    SmartImageBuffer smpBuf_YUV = imgInfo->getSmartBuffer(BID_PRE_PROCESS_OUT_YUV);
    IMetadata* pMeta_InHal_main1 = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_inApp = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_APP);
    TuningParam rTuningParam;

    // output
    SmartImageBuffer smpBuf_result = mBufPool.getBufPool(BID_DENOISE_FINAL_RESULT)->request();
    sp<IImageBuffer> smpBuf_result_thumb = imgInfo->getRequestPtr()->getImageBuffer(BID_THUMB_YUV);

    // applyISP
    {
        trySetMetadata<MUINT8>(pMeta_InHal_main1, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise);
        ISPTuningConfig ispConfig = {pMeta_inApp, pMeta_InHal_main1, mp3AHal_Main1, MFALSE, reqNo};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP, MTRUE);
    }


    const MBOOL isCropMain = true;
    const MRect fovRatioCrop = getFovRatioCrop(smpBuf_YUV->mImageBuffer->getImgSize(), smpBuf_result->mImageBuffer->getImgSize(), isCropMain);
    const MRect ratioCrop ={{0, 0}, smpBuf_YUV->mImageBuffer->getImgSize()};
    {
            ExifWriter writer(PIPE_LOG_TAG);
            // ratop crop
            writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_X, ratioCrop.p.x);
            writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_Y, ratioCrop.p.y);
            writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_W, ratioCrop.s.w);
            writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_H, ratioCrop.s.h);
            // fov crop
            writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_X, fovRatioCrop.p.x);
            writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_Y, fovRatioCrop.p.y);
            writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_W, fovRatioCrop.s.w);
            writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_H, fovRatioCrop.s.h);
            // update FOV crop to exif
            writer.makeExifFromCollectedData(imgInfo->getRequestPtr(), BMDeNoiseFeatureType::TYPE_BMDN);
    }
    const MRect thumbFovRatioCrop = getFovRatioCrop(smpBuf_YUV->mImageBuffer->getImgSize(), smpBuf_result_thumb->getImgSize(), isCropMain);


    // setup qParamTemplate
    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main1, ENormalStreamTag_Normal) // frame 0
            .addInput(PORT_IMGI)
            // WROTO
            .addCrop(eCROP_WROT, fovRatioCrop.p, fovRatioCrop.s, smpBuf_result->mImageBuffer->getImgSize())
            .addOutput(PORT_WROTO, 0)
            // WDMAO
            .addCrop(eCROP_WDMA, thumbFovRatioCrop.p, thumbFovRatioCrop.s, smpBuf_result_thumb->getImgSize())
            .addOutput(PORT_WDMAO, 0)
            .generate(qParamTemplate);
    if(!bSuccess){
        MY_LOGE("failed to create qParamTemplate!");
        return MFALSE;
    }
    QParamTemplateFiller qParamFiller(qParamTemplate.mTemplate, qParamTemplate.mStats);
    qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, smpBuf_YUV->mImageBuffer.get())
                #ifndef GTEST
                .insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
                #else
                .insertTuningBuf(iFrameNum,             nullptr)
                #endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, smpBuf_result->mImageBuffer.get())
                .insertOutputBuf(iFrameNum, PORT_WDMAO, smpBuf_result_thumb.get());

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParamTemplate!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool* newEnqueData = new EnquedBufPool(pEnqueData, FINAL_PROCESS);
    newEnqueData->addTuningData(tuningBuf);
    newEnqueData->addBuffData(BID_DENOISE_FINAL_RESULT,     smpBuf_result);
    newEnqueData->addBuffData(BID_POST_PROCESS_TEMP_I422,   smpBuf_YUV);
    newEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK, pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK));
    newEnqueData->addBuffData(BID_LCSO_ROT_BUF,             pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_LCSO_ROT_BUF));
    #ifdef BIT_TRUE
    newEnqueData->addBuffData(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O,   pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O));
    #endif
    newEnqueData->start();

    delete pEnqueData;

    // callbacks
    qParamTemplate.mTemplate.mpfnCallback = onP2SuccessCallback;
    qParamTemplate.mTemplate.mpfnEnQFailCallback = onP2FailedCallback;
    qParamTemplate.mTemplate.mpCookie = (MVOID*) newEnqueData;
    // p2 enque
    sp<P2Operator> p2Op = mwpP2Operator.promote();
    p2Op->enque(qParamTemplate.mTemplate, PIPE_LOG_TAG);
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
        BID_POST_PROCESS_TEMP_I422,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_POST_PROCESS_TEMP_I422)
    );
    ImgBufInfo->addSmartBuffer(
        BID_LCSO_ROT_BUF,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_LCSO_ROT_BUF)
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

    int allocateSize = 1;
    int tuningSize = 1;

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);
        NSBMDN::BufferConfig c = {
            "BID_DENOISE_HAL_OUT_ROT_BACK",
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
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK_IMG3O);

        MSize size(area.size);
        if(mModuleTrans == eTransform_ROT_90 || mModuleTrans == eTransform_ROT_270){
            swap(size.w, size.h);
        }

        NSBMDN::BufferConfig c = {
            "BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O",
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
        const MSize srcSize = mSizePrvider->getBufferSize(E_BM_PREPROCESS_OUT_YUV).size;
        const MSize dstSize = mSizePrvider->getBufferSize(E_BM_DENOISE_FINAL_RESULT).size;
        const MBOOL isCropMain = true;
        const MRect fovRatioCrop = getFovRatioCrop(srcSize, dstSize, isCropMain);

        // allocate a crop
        NSBMDN::BufferConfig c = {
            "BID_DENOISE_FINAL_RESULT",
            BID_DENOISE_FINAL_RESULT,
            (MUINT32)fovRatioCrop.width(),
            (MUINT32)fovRatioCrop.height(),
            eImgFmt_I422,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    {
        NS3Av3::LCSO_Param lcsoParam;
        if ( auto pIspMgr = MAKE_IspMgr() ) {
            pIspMgr->queryLCSOParams(lcsoParam);
        }
        MY_LOGI("lcso format:%d size:%dx%d, stride:%d", lcsoParam.format, lcsoParam.size.w, lcsoParam.size.h, lcsoParam.stride);

        MSize size(lcsoParam.size);
        if(mModuleTrans == eTransform_ROT_90 || mModuleTrans == eTransform_ROT_270){
            swap(size.w, size.h);
        }

        NSBMDN::BufferConfig c = {
            "BID_LCSO_ROT_BUF",
            BID_LCSO_ROT_BUF,
            (MUINT32)lcsoParam.size.w,
            (MUINT32)lcsoParam.size.h,
            (EImageFormat)lcsoParam.format,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)get_stereo_bmdenoise_capture_buffer_cnt()
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