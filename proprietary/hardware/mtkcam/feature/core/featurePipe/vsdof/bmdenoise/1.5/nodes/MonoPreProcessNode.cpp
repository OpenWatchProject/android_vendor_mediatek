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
#include "MonoPreProcessNode.h"

#define PIPE_MODULE_TAG "MFHR"
#define PIPE_CLASS_TAG "MonoPreProcessNode"
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
using namespace mfll;

#define DO_MONO_TO_YUV "do mono to yuv"

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

/*******************************************************************************
 *
 ********************************************************************************/
MonoPreProcessNode::
MonoPreProcessNode(const char *name,
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
MonoPreProcessNode::
~MonoPreProcessNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MonoPreProcessNode::
onData(
    DataID id,
    ImgInfoMapPtr &imgInfo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_CALL();

    MBOOL ret = MFALSE;
    switch(id)
    {
        case BAYER_TO_MONO_PREPROCESS:
            mImgInfoRequests.enque(imgInfo);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MonoPreProcessNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_CALL();

    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }

    MY_LOGD("MonoPreProcessNode::onInit=>create_3A_instance senosrIdx:(%d/%d)", mSensorIdx_Main1, mSensorIdx_Main2);
    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x Main2: %x", mp3AHal_Main1, mp3AHal_Main2);

    mSizePrvider = StereoSizeProvider::getInstance();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MonoPreProcessNode::
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
MonoPreProcessNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
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
MBOOL
MonoPreProcessNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    initBufferPool();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MonoPreProcessNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MonoPreProcessNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    ImgInfoMapPtr imgInfo = nullptr;
    // block until queue ready, or flush() breaks the blocking state too.
    if( !waitAllQueue() ){
        return MFALSE;
    }
    if( !mImgInfoRequests.deque(imgInfo) ){
        MY_LOGD("mImgInfoRequests.deque() failed");
        return MFALSE;
    }
    CAM_TRACE_NAME("BayerPreProcessNode::onThreadLoop");

    this->incExtThreadDependency();
    if(!doMonoToYUV(imgInfo)){
        MY_LOGE("failed doing doMonoToYUV, please check above errors!");
        this->decExtThreadDependency();
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
MonoPreProcessNode::
onP2SuccessCallback(NSIoPipe::QParams& rParams)
{
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    MonoPreProcessNode* pNode = (MonoPreProcessNode*) (pEnqueData->mpNode);

    MY_LOGD("ProcessId:%d", pEnqueData->mProcessId);

    switch(pEnqueData->mProcessId){
        case MONO_TO_YUV:
            pNode->onMonoToYUVDone(pEnqueData);
            break;
        default:
            MY_LOGE("unknown processId:%d", pEnqueData->mProcessId);
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
MonoPreProcessNode::
onP2FailedCallback(NSIoPipe::QParams& rParams)
{
    MY_LOGE("MonoPreProcessNode operations failed!!Check above errors!");
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    MonoPreProcessNode* pNode = (MonoPreProcessNode*) (pEnqueData->mpNode);
    pNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mPImgInfo);
    pNode->decExtThreadDependency();
    delete pEnqueData;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MonoPreProcessNode::
doMonoToYUV(
    ImgInfoMapPtr imgInfo)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_START(DO_MONO_TO_YUV);

    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

    // prepare data
    // input
    SmartTuningBuffer tuningBuf = mBufPool.getTuningBufPool()->request();
    sp<IImageBuffer> pBuf_fullraw_2 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2);
    sp<IImageBuffer> spLCSO = imgInfo->getRequestPtr()->getImageBuffer(BID_LCS_2);//PORT_LCEI
    IMetadata* pMeta_InHal_main2 = imgInfo->getMetadata(BID_META_IN_HAL_MAIN2);
    IMetadata* pMeta_inApp = imgInfo->getMetadata(BID_META_IN_APP);
    IMetadata pMeta_InHal_Mod = (*pMeta_InHal_main2);
    TuningParam rTuningParam;

    CHECK_OBJECT(spLCSO);

    // output
    SmartImageBuffer smpBuf_yuv = mBufPool.getBufPool(BID_MONO_PREPROCESS_YUV)->request();
    sp<IMfllImageBuffer> mfllBuf_small_yuv = mBufPool.getMfllBuffer(BID_MONO_PREPROCESS_SMALL_YUV);
    IImageBuffer* pBuf_small_yuv = static_cast<IImageBuffer*>(mfllBuf_small_yuv->getImageBuffer());

    MY_LOGD("small yuv mfll size(%dx%d) alignedSize(%dx%d) | IImageSize(%dx%d)",
        mfllBuf_small_yuv->getWidth(),          mfllBuf_small_yuv->getHeight(),
        mfllBuf_small_yuv->getAlignedWidth(),   mfllBuf_small_yuv->getAlignedHeight(),
        pBuf_small_yuv->getImgSize().w,         pBuf_small_yuv->getImgSize().h
    );

    // applyISP
    {
        trySetMetadata<MUINT8>(&pMeta_InHal_Mod, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_MFHR_Before_Blend);
        ISPTuningConfig ispConfig = {pMeta_inApp, &pMeta_InHal_Mod, mp3AHal_Main2, MFALSE, reqNo, spLCSO.get()};
        rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, USE_DEFAULT_ISP);
    }

    PQParam* framePQParam = new PQParam();
    fillInPQParam(framePQParam, BMDeNoiseProcessId::MONO_TO_YUV);

    // setup qParam
    bSuccess =
            QParamTemplateGenerator(iFrameNum, mSensorIdx_Main2, ENormalStreamTag_Normal) // frame 0
            .addInput(PORT_IMGI)
#ifndef GTEST
            .addInput(PORT_DEPI)
            .addInput(PORT_LCEI)
#endif
            .addOutput(PORT_WROTO, 0)
            .addCrop(eCROP_WROT, MPoint(0,0), pBuf_fullraw_2->getImgSize(), pBuf_small_yuv->getImgSize()) // WROTO
            .addOutput(PORT_IMG3O, 0)
            .addExtraParam(EPIPE_MDP_PQPARAM_CMD, (MVOID*)framePQParam)
            .generate(qParam);

    if(!bSuccess){
        MY_LOGE("failed to create qParam template!");
        return MFALSE;
    }

    QParamTemplateFiller qParamFiller(qParam);
                qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGI,  pBuf_fullraw_2.get())
#ifndef GTEST
                .insertTuningBuf(iFrameNum,             tuningBuf->mpVA)
                .insertInputBuf(iFrameNum,  PORT_DEPI,  static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf))
                .insertInputBuf(iFrameNum,  PORT_LCEI,  spLCSO.get())
#else
                .insertTuningBuf(iFrameNum,             nullptr)
#endif
                .insertOutputBuf(iFrameNum, PORT_WROTO, pBuf_small_yuv)
                .insertOutputBuf(iFrameNum, PORT_IMG3O, smpBuf_yuv->mImageBuffer.get());

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }

    // save working buffers to enqueBufferPool
    EnquedBufPool *pEnqueData= new EnquedBufPool(this, imgInfo->getRequestPtr(), imgInfo, BMDeNoiseProcessId::MONO_TO_YUV);
    pEnqueData->addBuffData(BID_MONO_PREPROCESS_YUV, smpBuf_yuv);
    pEnqueData->addIMfllImageBuffData(BID_MONO_PREPROCESS_SMALL_YUV, mfllBuf_small_yuv);
    pEnqueData->addPQData(framePQParam);
    pEnqueData->start();

    sp<IStopWatchCollection> pStopWatchCollection  = imgInfo->getRequestPtr()->getStopWatchCollection();
    pStopWatchCollection->BeginStopWatch("do_mono_to_yuv", (void*)pEnqueData);

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
MonoPreProcessNode::
onMonoToYUVDone(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MINT32 reqNo = pEnqueData->mPImgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);
    MET_END(DO_MONO_TO_YUV);

    pEnqueData->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueData->mProcessId),
        pEnqueData->getElapsed(),
        reqNo
    );

    pEnqueData->mPImgInfo->getRequestPtr()->getStopWatchCollection()->EndStopWatch((void*)pEnqueData); // do_mono_to_yuv

    pEnqueData->mPImgInfo->addSmartBuffer(
        BID_MONO_PREPROCESS_YUV,
        pEnqueData->mEnquedSmartImgBufMap.valueFor(BID_MONO_PREPROCESS_YUV)
    );

    pEnqueData->mPImgInfo->addMfllBuffer(
        BID_MONO_PREPROCESS_SMALL_YUV,
        pEnqueData->mEnquedIMfllImageBufMap[BID_MONO_PREPROCESS_SMALL_YUV]
    );

    MY_LOGD("-, reqID=%d", reqNo);
    handleFinish(pEnqueData);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
MonoPreProcessNode::
handleFinish(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    ImgInfoMapPtr imgInfo = pEnqueData->mPImgInfo;
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);

    if(imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1) != nullptr){
        imgInfo->delIImageBuffer(BID_INPUT_FSRAW_1);
    }
    if(imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2) != nullptr){
        imgInfo->delIImageBuffer(BID_INPUT_FSRAW_2);
    }

    delete pEnqueData;

    // handle data
    handleData(MONO_PREPROCESS_TO_FUSION, imgInfo);

    this->decExtThreadDependency();

    MY_LOGD("-, reqID=%d", reqNo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
MonoPreProcessNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = getMFHRCaptureCnt();
    int tuningSize = getMFHRCaptureCnt();

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_2);
        NSBMDN::BufferConfig c = {
            "mpMonoPreporcess_bufPool",
            BID_MONO_PREPROCESS_YUV,
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
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_SMALL_YUV);

        NSBMDN::BufferConfig c = {
            "mpMonoPreporcess_small_bufPool",
            BID_MONO_PREPROCESS_SMALL_YUV,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };

        c.mfllBufConf.isMfllBuffer = MTRUE;
        c.mfllBufConf.alignment = 16;

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
MonoPreProcessNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}
