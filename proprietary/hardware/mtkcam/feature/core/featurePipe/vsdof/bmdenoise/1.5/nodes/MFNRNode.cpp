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
#include "MFNRNode.h"

#define PIPE_MODULE_TAG "MFHR"
#define PIPE_CLASS_TAG "MFNRNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_MFLL "doMFLL"

// buffer alloc size
#define BUFFER_ALLOC_SIZE 1
#define TUNING_ALLOC_SIZE 2

#include <PipeLog.h>

#include "../util/vsdof_util.h"
#include "../exif/ExifWriter.h"

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;
using namespace NSIoPipe;
using namespace mfll;
using NSCam::NSIoPipe::NSSImager::IImageTransform;
using namespace mfll;

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

/*******************************************************************************
 *
 ********************************************************************************/
MFNRNode::
MFNRNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mImgInfoRequests);
    this->addWaitQueue(&mRequests);
    this->addWaitQueue(&mBSSResults);
}
/*******************************************************************************
 *
 ********************************************************************************/
MFNRNode::
~MFNRNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    MY_LOGD("onData(request) +");
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case FUSION_TO_MFNR:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    if(ret){
        mCondPushData.signal();
    }

    MY_LOGD("onData(request) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
onData(
    DataID id,
    ImgInfoMapPtr& pImgInfo)
{
    MY_LOGD("onData(ImgInfo) +");
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case FUSION_TO_MFNR:
            mImgInfoRequests.enque(pImgInfo);
            ret = MTRUE;
            break;
        default:
            MY_LOGE("onData(ImgInfoMapPtr) unknown data id :%d", id);
            ret = MFALSE;
            break;
    }

    if(ret){
        mCondPushData.signal();
    }

    MY_LOGD("onData(ImgInfo) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
onData(
    DataID id,
    android::Vector<BM_BSS_RESULT>& data)
{
    MY_LOGD("onData(Vector<BM_BSS_RESULT>) +");
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case BSS_RESULT:
            mBSSResults.enque(data);
            ret = MTRUE;
            break;
        default:
            MY_LOGE("onData(Vector<BM_BSS_RESULT>) unknown data id :%d", id);
            ret = MFALSE;
            break;
    }

    if(ret){
        mCondPushData.signal();
    }

    MY_LOGD("onData(Vector<BM_BSS_RESULT>) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
onInit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }

    mbUninit = MFALSE;

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
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
MBOOL
MFNRNode::
onThreadStart()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x, Main2: %x", mp3AHal_Main1, mp3AHal_Main2);

    createMFLLCore();

    initBufferPool();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
onThreadStop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    mbUninit = MTRUE;
    mCondPushData.signal();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
MFNRNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    destroyMFLLCore();

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
MFNRNode::
createMFLLCore()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    if(m_pMfb != nullptr || m_pCapturer != nullptr || m_pCore!= nullptr){
        MY_LOGE("Should not create without destroy!  %p,%p,%p",
            m_pMfb.get(),
            m_pCapturer.get(),
            m_pCore.get()
        );
        return MFALSE;
    }

    {
        AutoProfileUtil profile(LOG_TAG, "IMfllCore::createInstance");
        m_pCore = IMfllCore::createInstance(0, 0, IMfllCore::Type::MFHR);
    }

    if (m_pCore.get() == nullptr) {
        MY_LOGE("create MFNR Core Library failed");
        return MFALSE;
    }

    m_pCapturer = m_pCore->getCapturer();
    m_pMfb = m_pCore->getMfb();

    if (m_pCapturer.get() == nullptr) {
        MY_LOGE("create MFNR Capturer failed");
        return MFALSE;
    }

    if (m_pMfb.get() == nullptr) {
        MY_LOGE("create MFNR MFB failed");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
destroyMFLLCore()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    m_pMfb = nullptr;
    m_pCapturer = nullptr;
    m_pCore = nullptr;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
MFNRNode::
getRealIso(IMetadata* pAppMeta)
{
    MINT32 __iso = 100;
    auto b = IMetadata::getEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, __iso);

    if(b){
        MY_LOGD("MTK_SENSOR_SENSITIVITY:%d", __iso);
    }else{
        MY_LOGW("cant get MTK_SENSOR_SENSITIVITY from appMeta!");
    }
    return __iso;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
MFNRNode::
getFinalIso(IMetadata* pAppMeta)
{
    // the same as real iso
    return getRealIso(pAppMeta);
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT64
MFNRNode::
getRealShutterTime(IMetadata* pAppMeta)
{
    MINT64 __exp = 1;
    auto b = IMetadata::getEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, __exp);

    if(b){
        MY_LOGD("MTK_SENSOR_EXPOSURE_TIME:%d", __exp);
    }else{
        MY_LOGW("cant get MTK_SENSOR_EXPOSURE_TIME from appMeta!");
    }
    return __exp;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT64
MFNRNode::
getFinalShutterTime(IMetadata* pAppMeta)
{
    // the same as real exp
    return getRealShutterTime(pAppMeta);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }

    if(mbUninit){
        return MFALSE;
    }

    MBOOL canDoMFNR = MFALSE;
    WaitQueue<ImgInfoMapPtr>            todoImgInfoRequests;
    WaitQueue<PipeRequestPtr>           todoRequests;
    WaitQueue< Vector<BM_BSS_RESULT> >  todoBSSResults;

    {
        Mutex::Autolock _l(mLock);

        MY_LOGD("Fusion: %d/%d",        mImgInfoRequests.size(), getMFHRCaptureCnt());
        MY_LOGD("BSS: %d/%d",           mBSSResults.size(),      1);
        MY_LOGD("PipeRequest: %d/%d",   mRequests.size(),        getMFHRCaptureCnt());

        if(
            mImgInfoRequests.size() >= getMFHRCaptureCnt() &&
            mBSSResults.size() >= 1 &&
            mRequests.size() >= getMFHRCaptureCnt()
           ){
            MY_LOGD("data ready, prepare for MFNR!");

            Vector<BM_BSS_RESULT> bssResult;
            mBSSResults.deque(bssResult);
            todoBSSResults.enque(bssResult);

            for(int i=0 ; i<getMFHRCaptureCnt() ; i++){
                // imgInfo
                ImgInfoMapPtr ImgInfo = nullptr;
                mImgInfoRequests.deque(ImgInfo);
                todoImgInfoRequests.enque(ImgInfo);
                // PipeRequests
                PipeRequestPtr pipeRequest = nullptr;
                mRequests.deque(pipeRequest);
                todoRequests.enque(pipeRequest);
            }
            canDoMFNR = MTRUE;
        }else{
            MY_LOGD("data not ready, keep waiting");
            mCondPushData.wait(mLock);
        }
    }

    if(canDoMFNR){
        this->incExtThreadDependency();

        ImgInfoMapPtr rImgInfo_Main = nullptr;

        MBOOL ret = doMFNR(
                        rImgInfo_Main,
                        todoImgInfoRequests,
                        todoRequests,
                        todoBSSResults
                    );

        if(ret){
            handleData(BMDENOISE_RESULT_TO_SWNR, rImgInfo_Main->getRequestPtr());
            handleData(BMDENOISE_RESULT_TO_SWNR, rImgInfo_Main);
            this->decExtThreadDependency();
            return MFALSE;
        }else{
            MY_LOGE("Failed doMFNR, please check above errors!");
            this->decExtThreadDependency();
            return MFALSE;
        }
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
doMFNR(
    ImgInfoMapPtr& rImgInfo_Main,
    WaitQueue<ImgInfoMapPtr>& ImgInfoRequests,
    WaitQueue<PipeRequestPtr>& Requests,
    WaitQueue< Vector<BM_BSS_RESULT> >& BSSResults
){
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MTRUE;
    MET_START(DO_MFLL);

    if(m_pMfb == nullptr || m_pCapturer == nullptr || m_pCore == nullptr){
        createMFLLCore();
    }

    Vector<BM_BSS_RESULT> bssResult;
    BSSResults.deque(bssResult);

    // move requests from waitQueue to localQueue
    Vector<ImgInfoMapPtr> pendingImgInfos;
    for(int i=0 ; i<getMFHRCaptureCnt() ; i++){
        // imgInfo
        ImgInfoMapPtr ImgInfo = nullptr;
        ImgInfoRequests.deque(ImgInfo);
        CHECK_OBJECT(ImgInfo);
        pendingImgInfos.add(ImgInfo);
        // PipeRequests
        PipeRequestPtr pipeRequest = nullptr;
        Requests.deque(pipeRequest);
        CHECK_OBJECT(pipeRequest);
    }

    if(pendingImgInfos.size() > bssResult.size()){
        MY_LOGE("pendingImgInfos.size():%d vs bssResult.size():%d not illegal! getMFHRCaptureCnt():%d",
            pendingImgInfos.size(),
            bssResult.size(),
            getMFHRCaptureCnt()
        );
        return MFALSE;
    }

    // sort by BSSResult
    MY_LOGD("sort by BSSResult");
    Vector<ImgInfoMapPtr> sortedImgInfos;
    MINT32 mainFrame_idx = 0;
    for(int i=0 ; i<pendingImgInfos.size() ; i++){
        MY_LOGD("%d", i);
        sortedImgInfos.add(
            pendingImgInfos.editItemAt(bssResult[i].frameIdx)
        );

        if(bssResult[i].frameIdx == 0){
            mainFrame_idx = i;
            MY_LOGD("mainFrame_idx:%d", mainFrame_idx);
        }
    }

    sp<IStopWatchCollection> stopWatchCollection = sortedImgInfos[0]->getRequestPtr()->getStopWatchCollection();
    sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_mfhr");
    // init capturer
    {
        sp<IStopWatch> temp = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_mfhr:_init_capturer");
        m_pCapturer->setMfllCore(m_pCore.get());
        m_pCapturer->setShotMode(getMfbMode());
        m_pCapturer->setPostNrType(getPostNRMode());
    }
    // init mfb
    {
        sp<IStopWatch> temp = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_mfhr:_init_mfb");
        m_pMfb->init(miOpenId);
        m_pMfb->setMfllCore(m_pCore.get());
        m_pMfb->setShotMode(getMfbMode());
        m_pMfb->setPostNrType(getPostNRMode());
    }

    // init mfllCore
    {
        sp<IStopWatch> temp = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_mfhr:_init_mfllcore");
        ImgInfoMapPtr imgInfo = nullptr;
        imgInfo = sortedImgInfos.editItemAt(0);
        IMetadata meta_inApp = imgInfo->getSolidMetadata(BID_META_IN_APP);

        sp<IMfllImageBuffer> mfllImgBuf = imgInfo->getMfllBuffer(BID_FUSION_OUT);
        IImageBuffer* pBuf_fusion_yuv = static_cast<IImageBuffer*>(mfllImgBuf->getImageBuffer());

        CHECK_OBJECT(&meta_inApp);
        CHECK_OBJECT(mfllImgBuf);

        m_pCore->setCaptureResolution(
            pBuf_fusion_yuv->getImgSize().w,
            pBuf_fusion_yuv->getImgSize().h
        );

        MfllConfig cfg;
        cfg.sensor_id = miOpenId;
        cfg.capture_num = getMFHRCaptureCnt();
        cfg.blend_num = getMFHRCaptureCnt();
        cfg.mfll_mode = getMfbMode();
        cfg.post_nr_type = getPostNRMode();
        cfg.full_size_mc = getIsFullSizeMc();
        cfg.iso = getFinalIso(&meta_inApp);
        cfg.exp = getFinalShutterTime(&meta_inApp);
        cfg.original_iso = getRealIso(&meta_inApp);
        cfg.original_exp = getRealShutterTime(&meta_inApp);

        if (m_pCore->init(cfg) != MfllErr_Ok) {
            MY_LOGE("Init MFLL Core returns fail");
            return MFALSE;
        }
    }

    // run MFLL
    MY_LOGD("run MFLL");
    MINT32 i = 0;
    while(!sortedImgInfos.empty()){
        MY_LOGD("MFLL run %d/%d", i, (getMFHRCaptureCnt()-1));
        ImgInfoMapPtr imgInfo = nullptr;
        imgInfo = sortedImgInfos.editItemAt(0);
        sortedImgInfos.removeAt(0);

        // input
        IMetadata meta_InHal_main1 = imgInfo->getSolidMetadata(BID_META_IN_HAL);
        IMetadata meta_inApp = imgInfo->getSolidMetadata(BID_META_IN_APP);
        SmartImageBuffer smpBuf_LCSO_2 = imgInfo->getSmartBuffer(BID_LCSO_2_BUF);

        sp<IMfllImageBuffer> mfllImgBuf = imgInfo->getMfllBuffer(BID_FUSION_OUT);
        IImageBuffer* pBuf_fusion_yuv = static_cast<IImageBuffer*>(mfllImgBuf->getImageBuffer());

        sp<IMfllImageBuffer> mfllImgBuf_small = imgInfo->getMfllBuffer(BID_MONO_PREPROCESS_SMALL_YUV);

        CHECK_OBJECT(&meta_InHal_main1);
        CHECK_OBJECT(&meta_inApp);
        CHECK_OBJECT(smpBuf_LCSO_2);
        CHECK_OBJECT(mfllImgBuf);
        CHECK_OBJECT(mfllImgBuf_small);

        // frame i is main frame, provides extra data/info
        if(i == mainFrame_idx){
            rImgInfo_Main = new ImageBufInfoMap(imgInfo->getRequestPtr());

            sp<IImageBuffer> spBuf_result_postview = imgInfo->getRequestPtr()->getImageBuffer(BID_POSTVIEW);
            if(spBuf_result_postview != nullptr){
                MY_LOGD("postview:%dx%d fmt:%d", spBuf_result_postview->getImgSize().w, spBuf_result_postview->getImgSize().h, spBuf_result_postview->getImgFormat());
                sp<IMfllImageBuffer> mfllBuf_postview = IMfllImageBuffer::createInstance("postview");
                if (mfllBuf_postview.get() == NULL){
                    MY_LOGE("mfllBuf_postview for postview create failed!");
                    return MFALSE;
                }
                MfllRect_t __srcCropRgn;
                MRect appCropRgn;
                if (!tryGetMetadata<MRect>(&meta_inApp, MTK_SCALER_CROP_REGION, appCropRgn)){
                    __srcCropRgn.x = appCropRgn.p.x;
                    __srcCropRgn.y = appCropRgn.p.y;
                    __srcCropRgn.w = appCropRgn.s.w;
                    __srcCropRgn.h = appCropRgn.s.h;
                    MY_LOGD("MTK_SCALER_CROP_REGION (%d,%d,%dx%d)", __srcCropRgn.x, __srcCropRgn.y, __srcCropRgn.w, __srcCropRgn.h);
                }else{
                    StereoArea fullraw_crop;
                    fullraw_crop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);
                    __srcCropRgn.x = fullraw_crop.startPt.x;
                    __srcCropRgn.y = fullraw_crop.startPt.y;
                    __srcCropRgn.w = fullraw_crop.size.w;
                    __srcCropRgn.h = fullraw_crop.size.h;
                    MY_LOGW("cant retrieve MTK_SCALER_CROP_REGION for app meta in! use fullraw_crop (%d,%d,%dx%d)", __srcCropRgn.x, __srcCropRgn.y, __srcCropRgn.w, __srcCropRgn.h);
                }
            }else{
                MY_LOGW("spBuf_result_postview is nullptr!");
            }
        }
        // add data to MFLLCore
        std::deque<void*> metaset;
        metaset.push_back(static_cast<void*>(&meta_inApp));
        metaset.push_back(static_cast<void*>(&meta_InHal_main1));
        metaset.push_back(static_cast<void*>(smpBuf_LCSO_2->mImageBuffer.get()));
        m_pMfb->setSyncPrivateData(metaset);

        m_pCapturer->queueSourceImage(mfllImgBuf);
        m_pCapturer->queueSourceMonoImage(mfllImgBuf_small);

        MfllMotionVector mv;
        mv.x = bssResult[i].x;
        mv.y = bssResult[i].y;
        m_pCapturer->queueGmv(mv);
        i++;
    }

    {
        sp<IStopWatch> temp = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_mfhr:domfll");
        MY_LOGD("doMfll");
        m_pCore->doMfll();
    }

    {
        sp<IStopWatch> temp = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_mfhr:process_mixed_yuv");
        if(!processMixedYuv(rImgInfo_Main)){
            MY_LOGE("failed process mixed yuv!");
            return MFALSE;
        }
    }

    pendingImgInfos.clear();

    // 1. update MFHR exif
    ExifWriter writer(PIPE_LOG_TAG);
    writer.makeExifFromCollectedData(rImgInfo_Main->getRequestPtr(), BMDeNoiseFeatureType::TYPE_MFHR);

    // 2. update MFLL exif
    writer.doExifUpdate_MFNR(rImgInfo_Main->getRequestPtr(), m_pCore->getExifContainer()->getInfoMap());

    destroyMFLLCore();

    MET_END(DO_MFLL);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
processMixedYuv(ImgInfoMapPtr& ImgInfo_Main)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MINT32 reqNo = ImgInfo_Main->getRequestPtr()->getRequestNo();

    // input
    // retrieve mixed YUV image buffer
    auto mfllImgBuf = m_pCore->retrieveBuffer(MfllBuffer_MixedYuv);
    if (mfllImgBuf.get() == nullptr) {
        MY_LOGE("mixed output buffer is NULL");
        return MFALSE;
    }

    IImageBuffer* pResult = static_cast<IImageBuffer*>(mfllImgBuf->getImageBuffer());
    if (pResult == nullptr) {
        MY_LOGE("IMfllImageBuffer is not NULL but result in it is NULL");
        return MFALSE;
    }

    // output
    SmartImageBuffer smpBuf_MFNR_out_final = mBufPool.getBufPool(BID_MFNR_OUT_FINAL)->request();
    sp<IImageBuffer> spBuf_result_thumb = ImgInfo_Main->getRequestPtr()->getImageBuffer(BID_THUMB_YUV);

    if(spBuf_result_thumb == nullptr){
        MY_LOGE("cant get thumbnail!");
        return MFALSE;
    }

    {
        // use IImageTransform to handle image cropping
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
                IImageTransform::createInstance(), // constructor
                [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                );

        if (transform.get() == nullptr) {
            MY_LOGE("IImageTransform is NULL, cannot generate output");
            return MFALSE;
        }

        MRect fovRatioCrop;
        {
            const MSize srcSize = pResult->getImgSize();
            const MSize dstSize = smpBuf_MFNR_out_final->mImageBuffer.get()->getImgSize();
            const MBOOL isCropMain = false;

            fovRatioCrop = getFovRatioCrop(srcSize, dstSize, isCropMain);
            {
                // update FOV crop to exif
                ExifWriter writer(PIPE_LOG_TAG);
                // ratop crop
                writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_X, 0);
                writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_Y,0);
                writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_W, srcSize.w);
                writer.sendData(reqNo, BM_TAG_DENOISE_RATIO_CROP_H, srcSize.h);
                // fov crop
                writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_X, fovRatioCrop.p.x);
                writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_Y, fovRatioCrop.p.y);
                writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_W, fovRatioCrop.s.w);
                writer.sendData(reqNo, BM_TAG_DENOISE_FOV_CROP_H, fovRatioCrop.s.h);
            }
        }

        MRect fovRatioCropThumb;
        {
            FOVCropCalculator fOVCropCalculator(false);

            const MSize outImgSize = spBuf_result_thumb->getImgSize();
            fOVCropCalculator.setRatio(outImgSize);

            MSize imgRawSize = pResult->getImgSize();
            MY_LOGD("imgRawSize: (%d, %d), outImgSize: (%d, %d)", imgRawSize.w, imgRawSize.h, outImgSize.w, outImgSize.h);

            fovRatioCropThumb = fOVCropCalculator.getResultRegion(imgRawSize);
            MY_LOGD("fovRatioCropThumb: (%d, %d, %d, %d)",
                fovRatioCropThumb.p.x, fovRatioCropThumb.p.y, fovRatioCropThumb.width(), fovRatioCropThumb.height());
        }

        MBOOL ret = MTRUE;
        ret = transform->execute(
                pResult,
                smpBuf_MFNR_out_final->mImageBuffer.get(),
                spBuf_result_thumb.get(),
                fovRatioCrop, // ratio crop (dst1)
                fovRatioCropThumb, // ratio crop (dst2)
                0,    // no transform (dst1)
                0,    // no transform (dst2)
                3000  // timeout
            );

        if(!ret){
            MY_LOGE("Failed doIImageTransform!");
            return MFALSE;
        }
    }

    #ifdef BIT_TRUE
        if(shouldDumpRequest(ImgInfo_Main->getRequestPtr())){
            handleDump(pResult, BID_MFNR_OUT, reqNo);
        }
    #endif

    ImgInfo_Main->addSmartBuffer(
        BID_DENOISE_FINAL_RESULT,
        smpBuf_MFNR_out_final
    );

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
MFNRNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = 2;
    int tuningSize = 0;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        const MSize srcSize = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FUSION_OUT).size;
        const MSize dstSize = mSizePrvider->getBufferSize(E_BM_DENOISE_FINAL_RESULT).size;
        const MBOOL isCropMain = false;
        const MRect fovRatioCrop = getFovRatioCrop(srcSize, dstSize, isCropMain);

        NSBMDN::BufferConfig c = {
            "BID_MFNR_OUT_FINAL",
            BID_MFNR_OUT_FINAL,
            (MUINT32)fovRatioCrop.width(),
            (MUINT32)fovRatioCrop.height(),
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
MFNRNode::
doBufferPoolAllocation(MUINT32 count)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
#if 0
    StereoArea area;
    area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_1);

    for(int i=0 ; i<count ; i++){
        sp<IMfllImageBuffer> spMFNROut = IMfllImageBuffer::createInstance("MFNRout");

        if(spMFNROut->setAligned(16, 16) != MfllErr_Ok){
            MY_LOGE("failed setAligned for MFLL buffer!");
        }

        if(spMFNROut->setImageFormat(ImageFormat_I422) != MfllErr_Ok){
            MY_LOGE("failed setImageFormat for MFLL buffer!");
        }

        if(spMFNROut->setResolution(area.size.w, area.size.h) != MfllErr_Ok){
            MY_LOGE("failed setResolution for MFLL buffer!");
        }

        mvMFLLBufferPool.push_back(spMFNROut);
    }

    for(auto e:mvMFLLBufferPool){
        if(e->initBuffer() != MfllErr_Ok){
            MY_LOGE("failed init MFFL buffer!");
            return MFALSE;
        }
    }
    return MTRUE;
#else
    return mBufPool.doAllocate(count);
#endif
}
