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

// buffer alloc size

// debug settings
#define USE_DEFAULT_ISP 0
#define USE_DEFAULT_SHADING_GAIN 0
#define USE_DEFAULT_AMATRIX 0

#include <PipeLog.h>

#include "../util/vsdof_util.h"

#include <camera_custom_nvram.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;
using namespace NSIoPipe;

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
        AutoProfileUtil profile(LOG_TAG, "CDN_HAL destroyInstance");
        mpCDNHAL->destroyInstance();
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

    Mutex::Autolock _l(mLock);
    MY_LOGD("PreProcess: %d/%d", mImgInfoRequests_PreProcess.size(), getMFHRCaptureCnt());
    MY_LOGD("N3D: %d/%d", mImgInfoRequests_N3D.size(), 1);
    MY_LOGD("PipeRequest: %d/%d", mRequests.size(), 3);
    if(
        mImgInfoRequests_PreProcess.size() == getMFHRCaptureCnt() &&
        mImgInfoRequests_N3D.size() == 1 &&
        mRequests.size() == getMFHRCaptureCnt()
       ){
        MY_LOGD("data ready, start fusion!");
        this->incExtThreadDependency();

        list<ImgInfoMapPtr>  rvImgInfo_HR;

        MBOOL ret = doColorization(rvImgInfo_HR);

        if(ret){
            while(!rvImgInfo_HR.empty()){
                ImgInfoMapPtr imgInfo = rvImgInfo_HR.front();
                rvImgInfo_HR.pop_front();
                handleData(FUSION_TO_MFNR, imgInfo->getRequestPtr());
                handleData(FUSION_TO_MFNR, imgInfo);
            }
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
MBOOL
FusionNode::
doColorization(list<ImgInfoMapPtr>&  rvImgInfo_HR)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MTRUE;

    ImgInfoMapPtr imgInfo_N3D = nullptr;
    mImgInfoRequests_N3D.deque(imgInfo_N3D);

    SmartImageBuffer smpBuf_warpingMatrix = imgInfo_N3D->getSmartBuffer(BID_N3D_OUT_WARPING_MATRIX);

    MINT32 requestCount = mImgInfoRequests_PreProcess.size();
    while( !mImgInfoRequests_PreProcess.empty()){
        MY_LOGD("doColorization %d/%d", (requestCount - mImgInfoRequests_PreProcess.size()) + 1, requestCount);

        ImgInfoMapPtr  imgInfo_PreProcess = nullptr;
        PipeRequestPtr pipeRequest = nullptr;

        mImgInfoRequests_PreProcess.deque(imgInfo_PreProcess);
        mRequests.deque(pipeRequest);

        // input buffers
        SmartImageBuffer smpBuf_bayer_yuv = imgInfo_PreProcess->getSmartBuffer(BID_BAYER_PREPROCESS_YUV);
        SmartImageBuffer smpBuf_mono_yuv = imgInfo_PreProcess->getSmartBuffer(BID_MONO_PREPROCESS_YUV);

        // output buffers
        SmartImageBuffer smpBuf_fusion_out = mBufPool.getBufPool(BID_FUSION_OUT)->request();

        // input params
        CDN_HAL_PARAMS param;
        param.isRotate = (eRotate_90  == StereoSettingProvider::getModuleRotation() ||
                          eRotate_270 == StereoSettingProvider::getModuleRotation());
        param.mode = 0;
        param.var = 0;
        getAffineMatrix((float*)smpBuf_warpingMatrix->mImageBuffer->getBufVA(0), param);
        param.width = smpBuf_bayer_yuv->mImageBuffer->getImgSize().w;
        param.height = smpBuf_bayer_yuv->mImageBuffer->getImgSize().h;

        // io buffers
        CDN_HAL_IO io;
        io.width = smpBuf_bayer_yuv->mImageBuffer->getImgSize().w;
        io.width = smpBuf_bayer_yuv->mImageBuffer->getImgSize().h;
        io.MonoYUV = (unsigned char*)smpBuf_mono_yuv->mImageBuffer->getBufVA(0);
        io.BayerYUV = (unsigned char*)smpBuf_bayer_yuv->mImageBuffer->getBufVA(0);
        io.output = (unsigned char*)smpBuf_fusion_out->mImageBuffer->getBufVA(0);

        // run
        if(mpCDNHAL != nullptr){
            AutoProfileUtil profile(LOG_TAG, "CDN_HAL run");
            MET_START(DO_COLORIZATION);
            mpCDNHAL->CDNHALRun(param, io);
            MET_END(DO_COLORIZATION);
        }

        // pack result
        ImgInfoMapPtr ImgBufInfo = new ImageBufInfoMap(pipeRequest);

        ImgBufInfo->addSmartBuffer(
            BID_FUSION_OUT,
            smpBuf_fusion_out
        );

        rvImgInfo_HR.push_back(ImgBufInfo);
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

    int allocateSize = getMFHRCaptureCnt();

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_1);
        NSBMDN::BufferConfig c = {
            "mpMFHR_fusion_result_bufPool",
            BID_FUSION_OUT,
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
