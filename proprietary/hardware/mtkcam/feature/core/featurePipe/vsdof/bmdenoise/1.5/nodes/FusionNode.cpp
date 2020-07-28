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
#include "FusionNode.h"

#define PIPE_MODULE_TAG "MFHR"
#define PIPE_CLASS_TAG "FusionNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_COLORIZATION "doColorization"

#include <PipeLog.h>

#include "../util/vsdof_util.h"
#include "../exif/ExifWriter.h"

#include <camera_custom_nvram.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <mtkcam/aaa/IIspMgr.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;
using namespace NSIoPipe;
using namespace mfll;

/*******************************************************************************
 *
 ********************************************************************************/
FusionNode::
FusionNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mImgInfoRequests_PreProcess);
    this->addWaitQueue(&mImgInfoRequests_N3D);
    this->addWaitQueue(&mRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
FusionNode::
~FusionNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    MY_LOGD("onData(request) +");
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case ROOT_HR_TO_FUSION:
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
FusionNode::
onData(
    DataID id,
    ImgInfoMapPtr& pImgInfo)
{
    MY_LOGD("onData(ImgInfo) +");
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case MONO_PREPROCESS_TO_FUSION:
            mImgInfoRequests_PreProcess.enque(pImgInfo);
            ret = MTRUE;
            break;
        case WPE_TO_DPE_WARP_RESULT:
            mImgInfoRequests_N3D.enque(pImgInfo);
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
FusionNode::
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
FusionNode::
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
FusionNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(mp3AHal_Main1 != nullptr){
        mp3AHal_Main1->destroyInstance("BMDENOISE_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }
    if(mp3AHal_Main2 != nullptr){
        mp3AHal_Main2->destroyInstance("BMDENOISE_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }
    if(mpCDNHAL != nullptr){
        MY_LOGE("should call releaseALG() by BMDNHRPipe to ensure the relese order!");
    }
    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
onThreadStart()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x, Main2: %x", mp3AHal_Main1, mp3AHal_Main2);

    {
        AutoProfileUtil proflie(PIPE_LOG_TAG, "CDN_HAL createInstance");

        if( ::property_get_int32("vendor.cdn.dump.io", -1) == 1){
            char filepathAlg[1024];
            snprintf(filepathAlg, 1024, "/sdcard/bmdenoise/CModelData");
            VSDOF_LOGD("makePath: %s", filepathAlg);
            makePath(filepathAlg, 0660);
        }

        // init alg
        if(::property_get_int32("vendor.bmdenoise.pipe.cdn", 1) == 1){
            mpCDNHAL = CDN_HAL::createInstance();
            if(mpCDNHAL == nullptr){
                MY_LOGE("failed creating CDN instance");
                return MFALSE;
            }
        }else{
            MY_LOGD("by-pass CDN ALG init");
        }
    }

    initBufferPool();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    ImgInfoMapPtr ImgInfo = nullptr;
    PipeRequestPtr pipeRequest = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }

    CAM_TRACE_CALL();

    Mutex::Autolock _l(mLock);
    MY_LOGD("PreProcess: %d/%d", mImgInfoRequests_PreProcess.size(), getMFHRCaptureCnt());
    MY_LOGD("N3D: %d/%d", mImgInfoRequests_N3D.size(), 1);
    MY_LOGD("PipeRequest: %d/%d", mRequests.size(), 3);
    if(
        mImgInfoRequests_PreProcess.size() >= getMFHRCaptureCnt() &&
        mImgInfoRequests_N3D.size() >= 1 &&
        mRequests.size() >= getMFHRCaptureCnt()
       ){
        MY_LOGD("data ready, start fusion!");
        this->incExtThreadDependency();

        list<ImgInfoMapPtr>  rvImgInfo_HR;

        MBOOL ret = doColorization(rvImgInfo_HR);

        if(ret){
            PipeRequestPtr mainRequest= nullptr;
            while(!rvImgInfo_HR.empty()){
                ImgInfoMapPtr imgInfo = rvImgInfo_HR.front();
                rvImgInfo_HR.pop_front();

                handleData(FUSION_TO_MFNR, imgInfo->getRequestPtr());
                handleData(FUSION_TO_MFNR, imgInfo);

                MINT32 isMainFrame = imgInfo->getRequestPtr()->getParam(PID_MFHR_IS_MAIN);
                if(isMainFrame == 1){
                    mainRequest = imgInfo->getRequestPtr();
                }

                if(isMainFrame != 1){
                    // release non-main requests
                    MY_LOGD("release non-main frames:%d", imgInfo->getRequestPtr()->getRequestNo());
                    handleData(MFHR_EARLY_RELEASE, imgInfo->getRequestPtr());
                }
            }
            handleData(P2_DONE,       mainRequest);
            this->decExtThreadDependency();
            return MFALSE;
        }else{
            MY_LOGE("Failed doColorization, please check above errors!");
            this->decExtThreadDependency();
            return MFALSE;
        }
    }else{
        MY_LOGD("data not ready, keep waiting");
        return MTRUE;
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
getAffineMatrix(float* warpingMatrix, CDN_HAL_PARAMS &rInData)
{
    #if USE_DEFAULT_AMATRIX
        rInData.Trans[0] = 1;
        rInData.Trans[1] = 0;
        rInData.Trans[2] = 0;
        rInData.Trans[3] = 0;
        rInData.Trans[4] = 1;
        rInData.Trans[5] = 0;
        rInData.Trans[6] = 0;
        rInData.Trans[7] = 0;
        rInData.Trans[8] = 1;
        MY_LOGD("Test mode, use identity matrix");
    #else
        rInData.Trans[0] = warpingMatrix[0];
        rInData.Trans[1] = warpingMatrix[1];
        rInData.Trans[2] = warpingMatrix[2];
        rInData.Trans[3] = warpingMatrix[3];
        rInData.Trans[4] = warpingMatrix[4];
        rInData.Trans[5] = warpingMatrix[5];
        rInData.Trans[6] = warpingMatrix[6];
        rInData.Trans[7] = warpingMatrix[7];
        rInData.Trans[8] = warpingMatrix[8];
    #endif

    MY_LOGD("warpingMatrix: [%f,%f,%f][%f,%f,%f][%f,%f,%f]",
        rInData.Trans[0],
        rInData.Trans[1],
        rInData.Trans[2],
        rInData.Trans[3],
        rInData.Trans[4],
        rInData.Trans[5],
        rInData.Trans[6],
        rInData.Trans[7],
        rInData.Trans[8]
    );
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
FusionNode::
collectExifData(MINT32 reqId, map<MINT32, MINT32>& exifData, const CDN_HAL_PARAMS &rInParams)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    ExifWriter writer(PIPE_LOG_TAG);

    writer.sendData(reqId, BM_TAG_MFHR_VERSION, 1);
    writer.sendData(reqId, BM_TAG_DENOISE_TYPE, DN_TYPE_QUICK);
    writer.sendData(reqId, BM_TAG_DENOISE_COMPOSITION, COMP_BM_DENOISE_AND_SWNR);

    for(int i = 0 ; i < WARPING_MATRIX_SIZE ; i ++){
        // cast from float* to MINT32* is for getting the memory layout
        writer.sendData(reqId, BM_TAG_DENOISE_AMATRIX1 + i, *reinterpret_cast<const MINT32*>(&rInParams.Trans[i]));
    }

    for(int i = 0 ; i < SCENE_INFO_SIZE; i ++){
        writer.sendData(reqId, BM_TAG_SCENE_INFO1 + i, rInParams.sInfo[i]);
    }

    writer.sendData(reqId, BM_TAG_RULE_INFO13, rInParams.macroBoundThr);
    writer.sendData(reqId, BM_TAG_RULE_INFO14, rInParams.boundaryThr);
    writer.sendData(reqId, BM_TAG_RULE_INFO15, rInParams.varThr);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
doColorization(list<ImgInfoMapPtr>&  rvImgInfo_HR)
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MTRUE;

    ImgInfoMapPtr imgInfo_N3D = nullptr;
    mImgInfoRequests_N3D.deque(imgInfo_N3D);

    sp<IStopWatchCollection> stopWatchCollection = imgInfo_N3D->getRequestPtr()->getStopWatchCollection();
    sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_colorization");

    SmartImageBuffer smpBuf_warpingMatrix = imgInfo_N3D->getSmartBuffer(BID_N3D_OUT_WARPING_MATRIX);
    SmartImageBuffer smpBuf_sceneInfo = imgInfo_N3D->getSmartBuffer(BID_N3D_OUT_SCENEINFO);

    MINT32 requestCount = getMFHRCaptureCnt();
    MBOOL mainFrame = MTRUE;

    for(int i=0 ; i < getMFHRCaptureCnt() ; i++){
        MY_LOGD("doColorization %d/%d", i, getMFHRCaptureCnt() - 1);

        if(mImgInfoRequests_PreProcess.empty()){
            MY_LOGE("mImgInfoRequests_PreProcess is empty!");
            return MFALSE;
        }

        if(mRequests.empty()){
            MY_LOGE("mRequests is empty!");
            return MFALSE;
        }

        ImgInfoMapPtr  imgInfo_PreProcess = nullptr;
        PipeRequestPtr pipeRequest = nullptr;

        mImgInfoRequests_PreProcess.deque(imgInfo_PreProcess);
        mRequests.deque(pipeRequest);

        // input buffers
        SmartImageBuffer smpBuf_bayer_yuv = imgInfo_PreProcess->getSmartBuffer(BID_BAYER_PREPROCESS_YUV);
        SmartImageBuffer smpBuf_mono_yuv = imgInfo_PreProcess->getSmartBuffer(BID_MONO_PREPROCESS_YUV);
        sp<IImageBuffer> spLCSO_2 = imgInfo_PreProcess->getRequestPtr()->getImageBuffer(BID_LCS_2);//PORT_LCEI

#ifdef BIT_TRUE
        sp<IImageBuffer> spLCSO_1 = imgInfo_PreProcess->getRequestPtr()->getImageBuffer(BID_LCS_1);
#endif

        // output buffers
        sp<IMfllImageBuffer> mfllBuf_fusion_out = mBufPool.getMfllBuffer(BID_FUSION_OUT);
        IImageBuffer* pBuf_fusion_out = static_cast<IImageBuffer*>(mfllBuf_fusion_out->getImageBuffer());
        SmartImageBuffer smpBuf_lcso_2 = mBufPool.getBufPool(BID_LCSO_2_BUF)->request();

#ifdef BIT_TRUE
        SmartImageBuffer smpBuf_lcso_1 = mBufPool.getBufPool(BID_LCSO_1_BUF)->request();
#endif

        // input metadatas
        IMetadata* pMeta_InHal_main1 = imgInfo_PreProcess->getRequestPtr()->getMetadata(BID_META_IN_HAL);
        IMetadata* pMeta_inApp = imgInfo_PreProcess->getMetadata(BID_META_IN_APP);

        // input params
        CDN_HAL_PARAMS param;
        param.isRotate = (eRotate_90  == StereoSettingProvider::getModuleRotation() ||
                          eRotate_270 == StereoSettingProvider::getModuleRotation());
        param.mode = 0;
        param.var = 0;
        getAffineMatrix((float*)smpBuf_warpingMatrix->mImageBuffer->getBufVA(0), param);
        param.width = smpBuf_bayer_yuv->mImageBuffer->getImgSize().w;
        param.height = smpBuf_bayer_yuv->mImageBuffer->getImgSize().h;
        param.id = i;

        static const BMDeNoiseCriteriaSettings bmDenoiseCriteriaSettings;
        std::memcpy(&param.sInfo, (void*)smpBuf_sceneInfo->mImageBuffer->getBufVA(0), sizeof(param.sInfo));
        param.macroBoundThr = bmDenoiseCriteriaSettings.CUSTOMER_MACRO_BOUND_THR;
        param.boundaryThr = bmDenoiseCriteriaSettings.CUSTOMER_BOUNDARY_THR;
        param.varThr = bmDenoiseCriteriaSettings.CUSTOMER_VAR_THR;

        const MSize srcSize = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FUSION_OUT).size;
        const MSize dstSize = mSizePrvider->getBufferSize(E_BM_DENOISE_FINAL_RESULT).size;
        const MBOOL isCropMain = false;
        const MRect fovRatioCrop = getFovRatioCrop(srcSize, dstSize, isCropMain);
        param.cInfo[0] = fovRatioCrop.p.x;
        param.cInfo[1] = fovRatioCrop.p.y;
        param.cInfo[2] = fovRatioCrop.s.w;
        param.cInfo[3] = fovRatioCrop.s.h;


        // io buffers
        CDN_HAL_IO io;
        io.width = smpBuf_bayer_yuv->mImageBuffer->getImgSize().w;
        io.width = smpBuf_bayer_yuv->mImageBuffer->getImgSize().h;
        io.MonoYUV = (unsigned char*)smpBuf_mono_yuv->mImageBuffer->getBufVA(0);
        io.BayerYUV = (unsigned char*)smpBuf_bayer_yuv->mImageBuffer->getBufVA(0);
        io.output[0] = (unsigned char*)pBuf_fusion_out->getBufVA(0);
        io.output[1] = (unsigned char*)pBuf_fusion_out->getBufVA(1);
        io.output[2] = (unsigned char*)pBuf_fusion_out->getBufVA(2);

        // run
        if(mpCDNHAL != nullptr){
            sp<IStopWatch> tempStopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_colorization:cdn_hal_run");

            MET_START(DO_COLORIZATION);
            mpCDNHAL->CDNHALRun(param, io);
            MET_END(DO_COLORIZATION);
        }

        if(mainFrame){
            // update exif meta
            map<MINT32, MINT32> exifData;
            collectExifData(pipeRequest->getRequestNo(), exifData, param);
        }

        {
            sp<IStopWatch> tempStopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_colorization:memcpy_lcso_2");
            MY_LOGD("memcpy lcso 2 src:%d, dst%d", spLCSO_2->getBufSizeInBytes(0), smpBuf_lcso_2->mImageBuffer->getBufSizeInBytes(0));
            if(spLCSO_2->getBufSizeInBytes(0) != smpBuf_lcso_2->mImageBuffer->getBufSizeInBytes(0)){
                MY_LOGE("size not match!");
            }

            MSize poolImgSize = mBufPool.getBufPool(BID_LCSO_2_BUF)->getImageSize();
            MSize bufImgSize = smpBuf_lcso_2->mImageBuffer->getImgSize();
            if(poolImgSize != bufImgSize){
                MY_LOGD("lcso 2 buffer size is not the same whit pool, pool: %d x %d, buffer: %d x %d (%p), reset the buffer size",
                    poolImgSize.w, poolImgSize.h, bufImgSize.w, bufImgSize.h, smpBuf_lcso_2->mImageBuffer.get());
                smpBuf_lcso_2->mImageBuffer->setExtParam(poolImgSize, 0);
            }

            std::memcpy(
                reinterpret_cast<void*>(smpBuf_lcso_2->mImageBuffer->getBufVA(0)),
                reinterpret_cast<void*>(spLCSO_2->getBufVA(0)),
                spLCSO_2->getBufSizeInBytes(0)
            );
        }

#ifdef BIT_TRUE
        {
            sp<IStopWatch> tempStopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_colorization:memcpy_lcso_1");
            MY_LOGD("memcpy lcso 1 src:%d, dst%d", spLCSO_1->getBufSizeInBytes(0), smpBuf_lcso_1->mImageBuffer->getBufSizeInBytes(0));
            if(spLCSO_1->getBufSizeInBytes(0) != smpBuf_lcso_1->mImageBuffer->getBufSizeInBytes(0)){
                MY_LOGE("size not match!");
            }

            MSize poolImgSize = mBufPool.getBufPool(BID_LCSO_1_BUF)->getImageSize();
            MSize bufImgSize = smpBuf_lcso_1->mImageBuffer->getImgSize();
            if(poolImgSize != bufImgSize){
                MY_LOGD("lcso 1 buffer size is not the same whit pool, pool: %d x %d, buffer: %d x %d (%p), reset the buffer size",
                    poolImgSize.w, poolImgSize.h, bufImgSize.w, bufImgSize.h, smpBuf_lcso_1->mImageBuffer.get());
                smpBuf_lcso_1->mImageBuffer->setExtParam(poolImgSize, 0);
            }

            std::memcpy(
                reinterpret_cast<void*>(smpBuf_lcso_1->mImageBuffer->getBufVA(0)),
                reinterpret_cast<void*>(spLCSO_1->getBufVA(0)),
                spLCSO_1->getBufSizeInBytes(0)
            );
        }
#endif

        // appMetadata
        IMetadata newAppMeta;
        IMetadata newHalMeta;
        {
            sp<IStopWatch> tempStopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_colorization:cpy_appMeta");
            newAppMeta = (*pMeta_inApp);
            newHalMeta = (*pMeta_InHal_main1);
        }

        // pack result
        ImgInfoMapPtr ImgBufInfo = new ImageBufInfoMap(pipeRequest);

        ImgBufInfo->addMfllBuffer(
            BID_FUSION_OUT,
            mfllBuf_fusion_out
        );

        ImgBufInfo->addSmartBuffer(
            BID_LCSO_2_BUF,
            smpBuf_lcso_2
        );

#ifdef BIT_TRUE
        ImgBufInfo->addSmartBuffer(
            BID_LCSO_1_BUF,
            smpBuf_lcso_1
        );
#endif

        ImgBufInfo->addSolidMetadata(
            BID_META_IN_APP,
            newAppMeta
        );

        ImgBufInfo->addSolidMetadata(
            BID_META_IN_HAL,
            newHalMeta
        );

        if(imgInfo_PreProcess->getMfllBuffer(BID_MONO_PREPROCESS_SMALL_YUV) == nullptr){
            MY_LOGE("cant get BID_MONO_PREPROCESS_SMALL_YUV!");
            return MFALSE;
        }else{
            ImgBufInfo->addMfllBuffer(
                BID_MONO_PREPROCESS_SMALL_YUV,
                imgInfo_PreProcess->getMfllBuffer(BID_MONO_PREPROCESS_SMALL_YUV)
            );
        }

        rvImgInfo_HR.push_back(ImgBufInfo);
        mainFrame = MFALSE;
    }

    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
FusionNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = getMFHRCaptureCnt()*getMFHRCapturePhaseCnt();

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FUSION_OUT);
        NSBMDN::BufferConfig c = {
            "BID_FUSION_OUT",
            BID_FUSION_OUT,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_I422,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };

        c.mfllBufConf.isMfllBuffer = MTRUE;
        c.mfllBufConf.alignment = 16;

        vBufConfig.push_back(c);
    }

    {
        NS3Av3::LCSO_Param lcsoParam;
        if ( auto pIspMgr = MAKE_IspMgr() ) {
            pIspMgr->queryLCSOParams(lcsoParam);
        }
        //
        MY_LOGI("lcso num:%d-%d format:%d size:%dx%d, stride:%d",
            lcsoParam.format,
            lcsoParam.size.w, lcsoParam.size.h,
            lcsoParam.stride
        );

        NSBMDN::BufferConfig c2 = {
            "BID_LCSO_2_BUF",
            BID_LCSO_2_BUF,
            (MUINT32)lcsoParam.size.w,
            (MUINT32)lcsoParam.size.h,
            (EImageFormat)lcsoParam.format,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c2);

    #ifdef BIT_TRUE
        NSBMDN::BufferConfig c1 = {
            "BID_LCSO_1_BUF",
            BID_LCSO_1_BUF,
            (MUINT32)lcsoParam.size.w,
            (MUINT32)lcsoParam.size.h,
            (EImageFormat)lcsoParam.format,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c1);
    #endif
    }

    if(!mBufPool.init(vBufConfig, getMFHRCaptureCnt())){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
FusionNode::
releaseALG()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(mpCDNHAL != nullptr){
        mpCDNHAL->destroyInstance();
        mpCDNHAL = nullptr;
    }
    return MTRUE;
}
