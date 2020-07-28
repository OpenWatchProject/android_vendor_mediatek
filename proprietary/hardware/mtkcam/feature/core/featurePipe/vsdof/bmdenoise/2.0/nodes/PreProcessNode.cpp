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
#include "PreProcessNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "PreProcessNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// version settings
#define VERSION_E1 1

// debug settings
#define USE_DEFAULT_ISP 0

#include <PipeLog.h>

#include <DpBlitStream.h>
#include "../util/vsdof_util.h"

#include <mtkcam/feature/stereo/hal/stereo_common.h>
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NSIoPipe;

#define DO_P1_PREPROCESS_YUV "do P1 pre-process YUV"
#define DO_P1_PREPROCESS_1 "do P1 pre-process 1"
#define DO_MFBO_MDP_1 "do MFBO MDP rotation 1"
#define DO_P1_PREPROCESS_2 "do P1 pre-process 2"
#define DO_MFBO_MDP_2 "do MFBO MDP rotation 2"

#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

#define THREAD_NAME       ("P2DoneThread@PreProcessNode")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

/*******************************************************************************
 *
 ********************************************************************************/
PreProcessNode::
PreProcessNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mImgInfoRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
PreProcessNode::
~PreProcessNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onData(
    DataID id,
    ImgInfoMapPtr &imgInfo)
{
    FUNC_START;
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    switch(id)
    {
        case ROOT_TO_PREPROCESS:
            mImgInfoRequests.enque(imgInfo);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    TRACE_FUNC_EXIT();
    FUNC_END;
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_NAME("PreProcessNode::onInit");
    TRACE_FUNC_ENTER();

    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }

    // MDP
    MY_LOGD("PreProcessNode::onInit=>new DpBlitStream");
    CAM_TRACE_BEGIN("PreProcessNode::onInit=>new DpBlitStream");
    mpDpStream = new DpBlitStream();
    CAM_TRACE_END();

    MY_LOGD("PreProcessNode::onInit=>create_3A_instance senosrIdx:(%d/%d)", mSensorIdx_Main1, mSensorIdx_Main2);
    CAM_TRACE_BEGIN("PreProcessNode::onInit=>create_3A_instance");
    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x Main2: %x", mp3AHal_Main1, mp3AHal_Main2);
    CAM_TRACE_END();

    mSizePrvider = StereoSizeProvider::getInstance();

    TRACE_FUNC_EXIT();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onUninit()
{
    CAM_TRACE_NAME("PreProcessNode::onUninit");
    FUNC_START;
    cleanUp();
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
cleanUp()
{
    FUNC_START;

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
    FUNC_END;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mpP2DoneThread = new P2DoneThread(this);

    initBufferPool();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onThreadLoop()
{
    FUNC_START;

    ImgInfoMapPtr imgInfo = nullptr;

    // block until queue ready, or flush() breaks the blocking state too.
    if( !waitAllQueue() ){
        return MFALSE;
    }
    if( !mImgInfoRequests.deque(imgInfo) ){
        MY_LOGD("mImgInfoRequests.deque() failed");
        return MFALSE;
    }
    CAM_TRACE_NAME("PreProcessNode::onThreadLoop");

    this->incExtThreadDependency();
    if(!doPreProcess(imgInfo)){
        MY_LOGE("failed doing preprocess , please check above errors!");
        this->decExtThreadDependency();
    }

    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onP2SuccessCallback(NSIoPipe::QParams& rParams)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PreProcessNode* pNode = (PreProcessNode*) (pEnqueData->mpNode);

    pNode->invokeP2DoneThread(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
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
PreProcessNode::
onP2FailedCallback(NSIoPipe::QParams& rParams)
{
    MY_LOGE("PreProcessNode operations failed!!Check above errors!");
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PreProcessNode* pNode = (PreProcessNode*) (pEnqueData->mpNode);
    pNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mPImgInfo);
    pNode->decExtThreadDependency();
    delete pEnqueData;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
doPreProcess(
    ImgInfoMapPtr imgInfo)
{
    return doBayerToYUV(imgInfo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
doBayerToYUV(
    ImgInfoMapPtr imgInfo)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_P1_PREPROCESS_YUV);

    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

#if VERSION_E1
    // prepare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    sp<IImageBuffer> pBuf_fullraw_1 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1);
    IMetadata* pMeta_InHal_main1 = imgInfo->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_inApp = imgInfo->getMetadata(BID_META_IN_APP);
    IMetadata pMeta_InHal_Mod = (*pMeta_InHal_main1);
    TuningParam rTuningParam;

    // output
    SmartImageBuffer smpBuf_yuv = mBufPool.getBufPool(BID_PRE_PROCESS_OUT_YUV)->request();
    #ifdef BIT_TRUE
    SmartImageBuffer smpBuf_yuv_img3o = mBufPool.getBufPool(BID_PRE_PROCESS_OUT_YUV_IMG3O)->request();
    #endif
    sp<IImageBuffer> spBuf_result_postview = imgInfo->getRequestPtr()->getImageBuffer(BID_POSTVIEW);
    MY_LOGD("postview:%dx%d fmt:%d", spBuf_result_postview->getImgSize().w, spBuf_result_postview->getImgSize().h, spBuf_result_postview->getImgFormat());

    // applyISP
    {
        trySetMetadata<MUINT8>(&pMeta_InHal_Mod, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise_toYUV);
        ISPTuningConfig ispConfig = {pMeta_inApp, &pMeta_InHal_Mod, mp3AHal_Main1, MFALSE, reqNo};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP);
    }

    // setup qParam
    StereoArea fullraw_crop;
    fullraw_crop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);

    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main1, ENormalStreamTag_Normal) // frame 0
            .addInput(PORT_IMGI)
            #ifndef GTEST
            .addInput(PORT_DEPI)
            #endif
            #ifdef BIT_TRUE
            // IMG3O
            .addOutput(PORT_IMG3O, 0)
            #endif
            // WROTO
            .addCrop(eCROP_WROT, fullraw_crop.startPt, fullraw_crop.size, fullraw_crop.size) // WROTO
            .addOutput(PORT_WROTO, 0)
            // WDMAO
            .addCrop(eCROP_WDMA, fullraw_crop.startPt, fullraw_crop.size, fullraw_crop.size)
            .addOutput(PORT_WDMAO, 0)
            .generate(qParam);

    if(!bSuccess){
        MY_LOGE("failed to create qParam template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParam);
    qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGI,  pBuf_fullraw_1.get())
                #ifndef GTEST
                .insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
                .insertInputBuf(iFrameNum,  PORT_DEPI,  static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf))
                #else
                .insertTuningBuf(iFrameNum,             nullptr)
                #endif
                #ifdef BIT_TRUE
                .insertOutputBuf(iFrameNum, PORT_IMG3O, smpBuf_yuv_img3o->mImageBuffer.get())
                #endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, smpBuf_yuv->mImageBuffer.get())
                .insertOutputBuf(iFrameNum, PORT_WDMAO, spBuf_result_postview.get());

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool *pEnqueData= new EnquedBufPool(this, imgInfo->getRequestPtr(), imgInfo, BMDeNoiseProcessId::BAYER_TO_YUV);
    pEnqueData->addBuffData(BID_PRE_PROCESS_OUT_YUV, smpBuf_yuv);
    #ifdef BIT_TRUE
    pEnqueData->addBuffData(BID_PRE_PROCESS_OUT_YUV_IMG3O, smpBuf_yuv_img3o);
    #endif
    pEnqueData->start();

    // callbacks
    qParam.mpfnCallback = onP2SuccessCallback;
    qParam.mpfnEnQFailCallback = onP2FailedCallback;
    qParam.mpCookie = (MVOID*) pEnqueData;

    // p2 enque
    sp<P2Operator> p2Op = mwpP2Operator.promote();
    p2Op->enque(qParam, PIPE_LOG_TAG);

#else
    MY_LOGE("not implemented yet!");
    return MFALSE;
#endif
    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onBayerToYUVDone(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_END(DO_P1_PREPROCESS_YUV);

    pEnqueData->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );

    pEnqueData->mPImgInfo->addSmartBuffer(
        BID_PRE_PROCESS_OUT_YUV,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_YUV)
    );

    #ifdef BIT_TRUE
    pEnqueData->mPImgInfo->addSmartBuffer(
        BID_PRE_PROCESS_OUT_YUV_IMG3O,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_YUV_IMG3O)
    );
    #endif

    MY_LOGD("-, reqID=%d", reqNo);
    doBayerPreProcess(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
doBayerPreProcess(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    ImgInfoMapPtr imgInfo = pEnqueData->mPImgInfo;
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_P1_PREPROCESS_1);

    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

    // prepare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    sp<IImageBuffer> pBuf_fullraw_1 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1);
    IMetadata* pMeta_InHal_main1 = imgInfo->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_inApp = imgInfo->getMetadata(BID_META_IN_APP);
    IMetadata pMeta_InHal_Mod = (*pMeta_InHal_main1);
    TuningParam rTuningParam;

    // output
    SmartGraphicBuffer smgpBuf_w1 = mBufPool.getGraphicBufPool(BID_PRE_PROCESS_OUT_W_1)->request();
    SmartImageBuffer smpBuf_mfbo = mBufPool.getBufPool(BID_PRE_PROCESS_OUT_MFBO)->request();

    // convert W1 to Y8
    StereoArea area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_W_1);
    sp<IImageBufferHeap> spHeap_W1_Y8 = createImageBufferHeapInFormat(smgpBuf_w1->mImageBuffer, eImgFmt_Y8, area.contentSize());
    sp<IImageBuffer> spBuf_W1_Y8 = spHeap_W1_Y8->createImageBuffer();
    spBuf_W1_Y8->lockBuf(PIPE_LOG_TAG, ImageBufferPool::USAGE_HW);

    #ifdef BIT_TRUE
    SmartImageBuffer smpBuf_W1_img3o = mBufPool.getBufPool(BID_PRE_PROCESS_OUT_W_1_IMG3O)->request();
    #endif

    // applyISP
    {
        trySetMetadata<MUINT8>(&pMeta_InHal_Mod, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise_toW);
        ISPTuningConfig ispConfig = {pMeta_inApp, &pMeta_InHal_Mod, mp3AHal_Main1, MFALSE, reqNo};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP);
    }

    // setup qParam
    StereoArea fullraw_crop;
    fullraw_crop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);

    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main1, ENormalStreamTag_DeNoise) // frame 0
            .addInput(PORT_IMGI)
            #ifndef GTEST
            .addInput(PORT_DEPI)
            #endif
            // WROTO
            .addCrop(eCROP_WROT, fullraw_crop.startPt, fullraw_crop.size, fullraw_crop.size)
            .addOutput(PORT_WROTO, mModuleTrans)
            #ifdef BIT_TRUE
            // IMG3O
            .addOutput(PORT_IMG3O, 0)
            #endif
            .addOutput(PORT_MFBO)
            .generate(qParam);

    // A trick to make MFBO destination crop
    smpBuf_mfbo->mImageBuffer->setExtParam(
        fullraw_crop.size,
        fullraw_crop.size.w*smpBuf_mfbo->mImageBuffer->getPlaneBitsPerPixel(0)/8*fullraw_crop.startPt.y
    );

    if(!bSuccess){
        MY_LOGE("failed to create qParam template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParam);
    qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGI,  pBuf_fullraw_1.get())
                #ifndef GTEST
                .insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
                .insertInputBuf(iFrameNum,  PORT_DEPI,  static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf))
                #else
                .insertTuningBuf(iFrameNum,             nullptr)
                #endif
                #ifdef BIT_TRUE
                .insertOutputBuf(iFrameNum, PORT_IMG3O, smpBuf_W1_img3o->mImageBuffer.get())
                #endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, spBuf_W1_Y8.get())
                .insertOutputBuf(iFrameNum, PORT_MFBO,  smpBuf_mfbo->mImageBuffer.get());

    qParamFiller.setExtOffset(iFrameNum, PORT_MFBO,
    fullraw_crop.size.w*smpBuf_mfbo->mImageBuffer->getPlaneBitsPerPixel(0)/8*fullraw_crop.startPt.y);

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool* newEnqueData = new EnquedBufPool(pEnqueData, BAYER_PREPROCESS);
    newEnqueData->addTuningData(tuningBuf);
    newEnqueData->addGBuffData(BID_PRE_PROCESS_OUT_W_1,         smgpBuf_w1);
    newEnqueData->addBuffData(BID_PRE_PROCESS_OUT_MFBO,         smpBuf_mfbo);
    newEnqueData->addIImageBuffData(BID_TEMP_BUFFER,            spBuf_W1_Y8);
    #ifdef BIT_TRUE
    newEnqueData->addBuffData(BID_PRE_PROCESS_OUT_W_1_IMG3O,    smpBuf_W1_img3o);
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
PreProcessNode::
onBayerPreProcessDone(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_END(DO_P1_PREPROCESS_1);

    sp<IImageBuffer> spBuf_W1_Y8 = pEnqueData->mEnquedIImgBufMap.valueFor(BID_TEMP_BUFFER);
    spBuf_W1_Y8->unlockBuf(PIPE_LOG_TAG);

    pEnqueData->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );

    #ifdef BIT_TRUE
    {
        SmartImageBuffer smpBuf_mfbo = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_MFBO);

        char filepath[1024];
        snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", reqNo, getName());

        // make path
        MY_LOGD("makePath: %s", filepath);
        makePath(filepath, 0660);

        const char* fileName = "BID_PRE_PROCESS_OUT_MFBO_BAYER";
        const char* filePostfix = "raw";

        char writepath[1024];
        snprintf(writepath,
            1024, "%s/%s_%dx%d_%d.%s",
            filepath, fileName,
            smpBuf_mfbo->mImageBuffer->getImgSize().w, smpBuf_mfbo->mImageBuffer->getImgSize().h, smpBuf_mfbo->mImageBuffer->getBufStridesInBytes(0),
            filePostfix
        );

        smpBuf_mfbo->mImageBuffer->saveToFile(writepath);
    }
    #endif

    #ifdef BIT_TRUE
    {
        SmartImageBuffer smpBuf_mfbo = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_MFBO);

        char filepath[1024];
        snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", reqNo, getName());

        // make path
        MY_LOGD("makePath: %s", filepath);
        makePath(filepath, 0660);

        const char* fileName = "BID_PRE_PROCESS_OUT_MFBO_BAYER";
        const char* filePostfix = "raw";

        char writepath[1024];
        snprintf(writepath,
            1024, "%s/%s_%dx%d_%d.%s",
            filepath, fileName,
            smpBuf_mfbo->mImageBuffer->getImgSize().w, smpBuf_mfbo->mImageBuffer->getImgSize().h, smpBuf_mfbo->mImageBuffer->getBufStridesInBytes(0),
            filePostfix
        );

        smpBuf_mfbo->mImageBuffer->saveToFile(writepath);
    }
    #endif

    MBOOL ret = MFALSE;
    SmartGraphicBuffer smgpBuf_mfbo_final = nullptr;
    {
        AutoProfileUtil proflie(PIPE_LOG_TAG, __FUNCTION__);
        MET_START(DO_MFBO_MDP_1);
        // prepare data
        // input
        SmartImageBuffer smpBuf_mfbo = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_MFBO);

        // output
        smgpBuf_mfbo_final = mBufPool.getGraphicBufPool(BID_PRE_PROCESS_OUT_MFBO_FINAL_1)->request();

        // do MDP rotate for main1 mfbo
        StereoArea area_dst;
        area_dst = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_1);

        sMDP_Config config;
        config.pDpStream = mpDpStream;
        config.pSrcBuffer = smpBuf_mfbo->mImageBuffer.get();
        config.pDstBuffer = smgpBuf_mfbo_final->mImageBuffer.get();
        config.rotAngle = eTransformToDegree(mModuleTrans);
        config.customizedDstSize = area_dst.contentSize();
        ret = VSDOF::util::excuteMDPBayer12(config);
        MET_END(DO_MFBO_MDP_1);
    }

    if(ret){
        MY_LOGD("-, reqID=%d", reqNo);

        pEnqueData->mPImgInfo->addGraphicBuffer(
            BID_PRE_PROCESS_OUT_MFBO_FINAL_1,
            smgpBuf_mfbo_final
        );

        pEnqueData->mPImgInfo->addGraphicBuffer(
            BID_PRE_PROCESS_OUT_W_1,
            pEnqueData->mEnquedSmartGBufMap.valueFor(BID_PRE_PROCESS_OUT_W_1)
        );

        #ifdef BIT_TRUE
        pEnqueData->mPImgInfo->addSmartBuffer(
            BID_PRE_PROCESS_OUT_W_1_IMG3O,
            pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_W_1_IMG3O)
        );
        #endif

        doMonoPreProcess(pEnqueData);
    }else{
        MY_LOGE("excuteMDPBayer12 for main1 mfbo failed.");
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
doMonoPreProcess(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    ImgInfoMapPtr imgInfo = pEnqueData->mPImgInfo;
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_P1_PREPROCESS_2);

    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

    // prepare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    sp<IImageBuffer> pBuf_fullraw_2 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2);
    IMetadata* pMeta_InHal_main2 = imgInfo->getMetadata(BID_META_IN_HAL_MAIN2);
    IMetadata* pMeta_inApp = imgInfo->getMetadata(BID_META_IN_APP);
    IMetadata pMeta_InHal_Mod = (*pMeta_InHal_main2);
    TuningParam rTuningParam;

    // output
    SmartImageBuffer smpBuf_w2 = mBufPool.getBufPool(BID_PRE_PROCESS_OUT_W_2)->request();
    SmartImageBuffer smpBuf_mfbo = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_MFBO);

    // applyISP
    {
        trySetMetadata<MUINT8>(&pMeta_InHal_Mod, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Denoise_toW);
        ISPTuningConfig ispConfig = {pMeta_inApp, &pMeta_InHal_Mod, mp3AHal_Main2, MFALSE, reqNo};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP);
    }

    // setup qParam
    StereoArea fullraw_crop;
    fullraw_crop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);

    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main2, ENormalStreamTag_DeNoise) // frame 0
            .addInput(PORT_IMGI)
#ifndef GTEST
            .addInput(PORT_DEPI)
#endif
            .addCrop(eCROP_WROT, fullraw_crop.startPt, fullraw_crop.size, fullraw_crop.size) // WROTO
            .addOutput(PORT_WROTO, mModuleTrans)
            .addOutput(PORT_MFBO)
            .generate(qParam);

    // A trick to make MFBO destination crop
    smpBuf_mfbo->mImageBuffer->setExtParam(
        fullraw_crop.size,
        fullraw_crop.size.w*smpBuf_mfbo->mImageBuffer->getPlaneBitsPerPixel(0)/8*fullraw_crop.startPt.y
    );

    if(!bSuccess){
        MY_LOGE("failed to create qParam template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParam);
    qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGI,  pBuf_fullraw_2.get())
#ifndef GTEST
                .insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
                .insertInputBuf(iFrameNum,  PORT_DEPI,  static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf))
#else
                .insertTuningBuf(iFrameNum,             nullptr)
#endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, smpBuf_w2->mImageBuffer.get())
                .insertOutputBuf(iFrameNum, PORT_MFBO,  smpBuf_mfbo->mImageBuffer.get());

    qParamFiller.setExtOffset(iFrameNum, PORT_MFBO,
    fullraw_crop.size.w*smpBuf_mfbo->mImageBuffer->getPlaneBitsPerPixel(0)/8*fullraw_crop.startPt.y);

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool* newEnqueData = new EnquedBufPool(pEnqueData, MONO_PREPROCESS);
    newEnqueData->addTuningData(tuningBuf);
    newEnqueData->addBuffData(BID_PRE_PROCESS_OUT_W_2,     smpBuf_w2);
    newEnqueData->addBuffData(BID_PRE_PROCESS_OUT_MFBO,    smpBuf_mfbo);
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
PreProcessNode::
onMonoPreProcessDone(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_END(DO_P1_PREPROCESS_2);

    pEnqueData->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );

    #ifdef BIT_TRUE
    {
        SmartImageBuffer smpBuf_mfbo = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_MFBO);

        char filepath[1024];
        snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", reqNo, getName());

        // make path
        MY_LOGD("makePath: %s", filepath);
        makePath(filepath, 0660);

        const char* fileName = "BID_PRE_PROCESS_OUT_MFBO_MONO";
        const char* filePostfix = "raw";

        char writepath[1024];
        snprintf(writepath,
            1024, "%s/%s_%dx%d_%d.%s",
            filepath, fileName,
            smpBuf_mfbo->mImageBuffer->getImgSize().w, smpBuf_mfbo->mImageBuffer->getImgSize().h, smpBuf_mfbo->mImageBuffer->getBufStridesInBytes(0),
            filePostfix
        );

        smpBuf_mfbo->mImageBuffer->saveToFile(writepath);
    }
    #endif

    MBOOL ret = MFALSE;
    SmartGraphicBuffer smgpBuf_mfbo_final = nullptr;
    {
        AutoProfileUtil proflie(PIPE_LOG_TAG, __FUNCTION__);
        MET_START(DO_MFBO_MDP_2);
        // prepare data
        // input
        SmartImageBuffer smpBuf_mfbo = pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_PRE_PROCESS_OUT_MFBO);

        // output
        smgpBuf_mfbo_final = mBufPool.getGraphicBufPool(BID_PRE_PROCESS_OUT_MFBO_FINAL_2)->request();

        // do MDP rotate for main2 mfbo
        StereoArea area_dst;
        area_dst = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_2);

        sMDP_Config config;
        config.pDpStream = mpDpStream;
        config.pSrcBuffer = smpBuf_mfbo->mImageBuffer.get();
        config.pDstBuffer = smgpBuf_mfbo_final->mImageBuffer.get();
        config.rotAngle = eTransformToDegree(mModuleTrans);
        config.customizedDstSize = area_dst.contentSize();
        ret = VSDOF::util::excuteMDPBayer12(config);
        MET_END(DO_MFBO_MDP_2);
    }

    if(ret){
        MY_LOGD("-, reqID=%d", reqNo);

        pEnqueData->mPImgInfo->addGraphicBuffer(
            BID_PRE_PROCESS_OUT_MFBO_FINAL_2,
            smgpBuf_mfbo_final
        );

        handleFinish(pEnqueData);
    }else{
        MY_LOGE("excuteMDPBayer12 for main2 mfbo failed.");
    }

    MY_LOGD("-, reqID=%d", reqNo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
handleFinish(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    ImgInfoMapPtr imgInfo = pEnqueData->mPImgInfo;
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);

    delete pEnqueData;

    // handle data
    dumpRequestData(BID_POSTVIEW, imgInfo->getRequestPtr());
    handleData(PREPROCESS_TO_DENOISE, imgInfo);
    this->decExtThreadDependency();

    MY_LOGD("-, reqID=%d", reqNo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = get_stereo_bmdenoise_capture_buffer_cnt();
    int tuningSize = 3;

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);
        NSBMDN::BufferConfig c = {
            "mpMain1_yuv_bufPool",
            BID_PRE_PROCESS_OUT_YUV,
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
    #ifdef BIT_TRUE
    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_1);
        NSBMDN::BufferConfig c = {
            "mpMain1_yuv_img3o_bufPool",
            BID_PRE_PROCESS_OUT_YUV_IMG3O,
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
    #endif
    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_W_1);
        NSBMDN::BufferConfig c = {
            "mpMain1_w_bufPool",
            BID_PRE_PROCESS_OUT_W_1,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_Y16,
            GraphicBufferPool::USAGE_HW_RENDER,
            MTRUE,
            MTRUE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    #ifdef BIT_TRUE
    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_1);
        NSBMDN::BufferConfig c = {
            "mpMain1_w_img3o_bufPool",
            BID_PRE_PROCESS_OUT_W_1_IMG3O,
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
    #endif

    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_W_2);
        NSBMDN::BufferConfig c = {
            "mpMain2_w_bufPool",
            BID_PRE_PROCESS_OUT_W_2,
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

    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_1);
        NSBMDN::BufferConfig c = {
            "mpMfbo_bufPool",
            BID_PRE_PROCESS_OUT_MFBO,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_BAYER14_UNPAK,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_1);
        NSBMDN::BufferConfig c = {
            "mpMain1_mfbo_final_bufPool",
            BID_PRE_PROCESS_OUT_MFBO_FINAL_1,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_Y16,
            GraphicBufferPool::USAGE_HW_RENDER,
            MTRUE,
            MTRUE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_2);
        NSBMDN::BufferConfig c = {
            "mpMain2_mfbo_final_bufPool",
            BID_PRE_PROCESS_OUT_MFBO_FINAL_2,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_Y16,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MTRUE,
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
PreProcessNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}
/******************************************************************************
 *
 ******************************************************************************/
void
PreProcessNode::P2DoneThread::
requestExit()
{
    FUNC_START;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
PreProcessNode::P2DoneThread::
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
PreProcessNode::P2DoneThread::
threadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    EnquedBufPool* pEnqueData = mpNode->mpP2DonePendingData;

    MY_LOGD("ProcessId:%d", pEnqueData->mProcessId);

    switch(pEnqueData->mProcessId){
        case BAYER_TO_YUV:
            mpNode->onBayerToYUVDone(pEnqueData);
            break;
        case BAYER_PREPROCESS:
            mpNode->onBayerPreProcessDone(pEnqueData);
            break;
        case MONO_PREPROCESS:
            mpNode->onMonoPreProcessDone(pEnqueData);
            break;
        default:
            MY_LOGE("unknown processId:%d", pEnqueData->mProcessId);
            return false;
    }
    return  false;
}