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

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;
using namespace NSIoPipe;
using namespace mfll;
using NSCam::NSIoPipe::NSSImager::IImageTransform;

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
    android::Vector<MINT32>& data)
{
    MY_LOGD("onData(Vector<MINT32>) +");
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case BSS_RESULT:
            mBSSResults.enque(data);
            ret = MTRUE;
            break;
        default:
            MY_LOGE("onData(Vector<MINT32>) unknown data id :%d", id);
            ret = MFALSE;
            break;
    }

    MY_LOGD("onData(Vector<MINT32>) -");
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
MBOOL
MFNRNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }

    Mutex::Autolock _l(mLock);
    MY_LOGD("Fusion: %d/%d", mImgInfoRequests.size(), getMFHRCaptureCnt());
    MY_LOGD("BSS: %d/%d", mBSSResults.size(), 1);
    MY_LOGD("PipeRequest: %d/%d", mRequests.size(), getMFHRCaptureCnt());
    if(
        mImgInfoRequests.size() == getMFHRCaptureCnt() &&
        mBSSResults.size() == 1 &&
        mRequests.size() == getMFHRCaptureCnt()
       ){
        MY_LOGD("data ready, start MFNR!");
        this->incExtThreadDependency();

        ImgInfoMapPtr rImgInfo_MFNR = nullptr;

        MBOOL ret = doMFNR(rImgInfo_MFNR);

        if(ret){
            handleData(FUSION_TO_MFNR, rImgInfo_MFNR->getRequestPtr());
            handleData(FUSION_TO_MFNR, rImgInfo_MFNR);
            this->decExtThreadDependency();
            return MFALSE;
        }else{
            MY_LOGE("Failed doMFNR, please check above errors!");
            this->decExtThreadDependency();
            return MFALSE;
        }
    }else{
        MY_LOGD("data not ready, keep waiting");
        return MTRUE;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
doMFNR(ImgInfoMapPtr& rImgInfo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MTRUE;
    MET_START(DO_MFLL);

    if(m_pMfb == nullptr || m_pCapturer == nullptr || m_pCore == nullptr){
        createMFLLCore();
    }

    Vector<MINT32> bssResult;
    mBSSResults.deque(bssResult);

    Vector<ImgInfoMapPtr> pendingImgInfos;
    for(int i=0 ; i<getMFHRCaptureCnt() ; i++){
        ImgInfoMapPtr ImgInfo = nullptr;
        mImgInfoRequests.deque(ImgInfo);
        pendingImgInfos.add(ImgInfo);
    }

    if(pendingImgInfos.size() > bssResult.size()){
        MY_LOGE("pendingImgInfos.size():%d vs bssResult.size():%d not illegal! getMFHRCaptureCnt():%d",
            pendingImgInfos.size(),
            bssResult.size(),
            getMFHRCaptureCnt()
        );
        return MFALSE;
    }

    MY_LOGD("sort by BSSResult");
    Vector<ImgInfoMapPtr> sortedImgInfos;
    MINT32 mainFrame_idx = 0;
    for(int i=0 ; i<pendingImgInfos.size() ; i++){
        MY_LOGD("%d", i);
        sortedImgInfos.add(
            pendingImgInfos.editItemAt(bssResult[i])
        );

        if(bssResult[i] == 0){
            mainFrame_idx = i;
            MY_LOGD("mainFrame_idx:%d", mainFrame_idx);
        }
    }

    // run MFNR
    MY_LOGD("run MFNR");
    ImgInfoMapPtr mainFrame = nullptr;
    MINT32 i = 0;
    while(!sortedImgInfos.empty()){
        MY_LOGD("MFNR run %d/%d", i, (getMFHRCaptureCnt()-1));
        ImgInfoMapPtr imgInfo = nullptr;
        imgInfo = sortedImgInfos.editItemAt(0);
        sortedImgInfos.removeAt(0);

        // input
        IMetadata* pMeta_InHal_main1 = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_HAL);
        IMetadata* pMeta_inApp = imgInfo->getRequestPtr()->getMetadata(BID_META_IN_APP);
        SmartImageBuffer smpBuf_fusion_yuv = imgInfo->getSmartBuffer(BID_FUSION_OUT);
        sp<IImageBuffer> spLCSO = imgInfo->getRequestPtr()->getImageBuffer(BID_LCS_1);//PORT_LCEI

        if(spLCSO == nullptr){
            MY_LOGE("no LCSO !");
            return MFALSE;
        };

        // frame i is main frame, provides extra data/info
        if(i == mainFrame_idx){
            mainFrame = imgInfo;

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
                if (!tryGetMetadata<MRect>(pMeta_inApp, MTK_SCALER_CROP_REGION, appCropRgn)){
                    __srcCropRgn.x = appCropRgn.p.x;
                    __srcCropRgn.y = appCropRgn.p.y;
                    __srcCropRgn.w = appCropRgn.s.w;
                    __srcCropRgn.h = appCropRgn.s.h;
                    MY_LOGD("MTK_SCALER_CROP_REGION (%d,%d,%dx%d)", __srcCropRgn.x, __srcCropRgn.y, __srcCropRgn.w, __srcCropRgn.h);
                }else{
                    __srcCropRgn.x = 0;
                    __srcCropRgn.y = 0;
                    __srcCropRgn.w = smpBuf_fusion_yuv->mImageBuffer->getImgSize().w;
                    __srcCropRgn.h = smpBuf_fusion_yuv->mImageBuffer->getImgSize().h;
                    MY_LOGW("cant retrieve MTK_SCALER_CROP_REGION for app meta in! use src buf size (%d,%d,%dx%d)", __srcCropRgn.x, __srcCropRgn.y, __srcCropRgn.w, __srcCropRgn.h);
                }
                mfllBuf_postview->setImageBuffer(reinterpret_cast<void*>(spBuf_result_postview.get()));
                m_pCore->setPostviewBuffer(mfllBuf_postview, __srcCropRgn);
            }else{
                MY_LOGW("spBuf_result_postview is nullptr!");
            }
        }

        // other data/info porvided by each frame
        sp<IMfllImageBuffer> mfllImgBuf = IMfllImageBuffer::createInstance("muv");
        mfllImgBuf->setImageBuffer(smpBuf_fusion_yuv->mImageBuffer.get());

        // add data to MFLLCore
        std::deque<void*> metaset;
        metaset.push_back(static_cast<void*>(pMeta_inApp));
        metaset.push_back(static_cast<void*>(pMeta_InHal_main1));
        metaset.push_back(static_cast<void*>(spLCSO.get()));
        m_pMfb->setSyncPrivateData(metaset);
        m_pCapturer->queueSourceImage(mfllImgBuf);
        i++;
    }

    {
        AutoProfileUtil profile(LOG_TAG, "doMfll");
        MY_LOGD("doMfll");
        m_pCore->doMfll();
    }

    if(!processMixedYuv(mainFrame, rImgInfo)){
        MY_LOGE("failed process mixed yuv!");
        return MFALSE;
    }

    pendingImgInfos.clear();

    destroyMFLLCore();

    MET_END(DO_MFLL);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
MFNRNode::
processMixedYuv(ImgInfoMapPtr& ImgInfo, ImgInfoMapPtr& rImgInfo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    AutoProfileUtil profile(LOG_TAG, "processMixedYuv");

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
    SmartImageBuffer smpBuf_MFNR_out = mBufPool.getBufPool(BID_MFNR_OUT)->request();
    sp<IImageBuffer> spBuf_result_thumb = ImgInfo->getRequestPtr()->getImageBuffer(BID_THUMB_YUV);

    if(spBuf_result_thumb == nullptr){
        MY_LOGE("cant get thumbnail!");
        return MFALSE;
    }

    {
        // use IImageTransform to handle image cropping
        AutoProfileUtil profile(LOG_TAG, "doIImageTransform");
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
                IImageTransform::createInstance(), // constructor
                [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                );

        if (transform.get() == nullptr) {
            MY_LOGE("IImageTransform is NULL, cannot generate output");
            return MFALSE;
        }

        StereoArea fullraw_crop;
        fullraw_crop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);
        MRect crop(fullraw_crop.startPt, fullraw_crop.size);

        MBOOL ret = MTRUE;
        ret = transform->execute(
                pResult,
                smpBuf_MFNR_out->mImageBuffer.get(),
                spBuf_result_thumb.get(),
                crop, // ratio crop (dst1)
                crop, // ratio crop (dst2)
                0,    // no transform (dst1)
                0,    // no transform (dst2)
                3000  // timeout
            );

        if(!ret){
            MY_LOGE("Failed doIImageTransform!");
            return MFALSE;
        }
    }

    // pack result data back
    rImgInfo = new ImageBufInfoMap(ImgInfo->getRequestPtr());
    rImgInfo->addSmartBuffer(
        BID_MFNR_OUT,
        smpBuf_MFNR_out
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

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);
        NSBMDN::BufferConfig c = {
            "mpMFLLOut_bufPool",
            BID_MFNR_OUT,
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
