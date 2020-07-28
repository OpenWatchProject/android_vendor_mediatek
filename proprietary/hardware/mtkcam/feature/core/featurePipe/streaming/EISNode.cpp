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


#include <system/thread_defs.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <utils/ThreadDefs.h>


#include "EISNode.h"
#include "WarpBase.h"
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/feature/eis/eis_hal.h>
#include <mtkcam/feature/fsc/fsc_ext.h>

#define PIPE_CLASS_TAG "EISNode"
#define PIPE_TRACE TRACE_EIS_NODE
#include <featurePipe/core/include/PipeLog.h>

#define EISNODE_THREAD_PRIORITY       (ANDROID_PRIORITY_FOREGROUND-3)

#define EISNODE_DEBUG_FSC_BUFFER      "vendor.debug.eis.fsc"
#define EISNODE_DUMP_FOLDER_PATH      "/data/vendor/dump"


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

EISNode::EISNode(const char *name)
    : StreamingFeatureNode(name)
    , mWarpMapBufferPool(NULL)
    , mpEisHal(NULL)
    , mWarpGridSize(2,2)
    , mInFlush(MFALSE)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);
    mEnableDump = (getPropertyValue(EISNODE_DEBUG_FSC_BUFFER, 0) == 1) ? MTRUE : MFALSE;
    TRACE_FUNC_EXIT();
}

EISNode::~EISNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL EISNode::onData(DataID id, const RequestPtr &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_EIS_P2DONE )
    {
        mRequests.enque(FMData(FMResult(), data));
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::onData(DataID id, const FMData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_EIS_P2DONE ||
        id == ID_P2A_TO_EIS_FM )
    {
        mRequests.enque(data);
        ret = MTRUE;
    }

    if (mpEisHal)
    {
        mpEisHal->ForcedDoEisPass2();
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::onData(DataID id, const RSCData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_RSC_TO_EIS )
    {
        mRSCDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::onData(DataID id, const FOVData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_FOV_TO_EIS_WARP)
    {
        mFOVDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();

    if( mPipeUsage.supportRSCNode() )
    {
        this->addWaitQueue(&mRSCDatas);
    }
    if( mPipeUsage.supportDual() )
    {
        this->addWaitQueue(&mFOVDatas);
    }

    initEIS();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::onUninit()
{
    TRACE_FUNC_ENTER();

    if (mpEisHal)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance("FeaturePipe_EisNode");
        mpEisHal = NULL;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    const int EIS_WARP_MAP_NUM = 6;
    prepareBufferPool();
    if( mWarpMapBufferPool != NULL )
    {
        Timer timer;
        timer.start();
        mWarpMapBufferPool->allocate(EIS_WARP_MAP_NUM);
        timer.stop();
        MY_LOGD("mWarpMap %s %d buf in %d ms", STR_ALLOCATE, EIS_WARP_MAP_NUM, timer.getElapsed());
        ret = MTRUE;
    }
    ::setpriority(PRIO_PROCESS, 0, EISNODE_THREAD_PRIORITY);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    FatImageBufferPool::destroy(mWarpMapBufferPool);
    mWarpMapBufferPool = NULL;

    if( mQueue.size() )
    {
        MY_LOGW("Queue not empty size=%zu", mQueue.size());
        mQueue.clear();
        mRefCount = 0;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    FMData p2aRequest;
    RSCData rscData;
    FOVData fovData;

    if( waitAllQueue() )
    {
        if( !mRequests.deque(p2aRequest) )
        {
            MY_LOGE("P2A done deque out of sync");
            return MFALSE;
        }
        if( p2aRequest.mRequest == NULL )
        {
            MY_LOGE("No p2aRequest data!");
            return MFALSE;
        }
        if( mPipeUsage.supportRSCNode() )
        {
            if( !mRSCDatas.deque(rscData) )
            {
                MY_LOGE("RSCData deque out of sync");
                return MFALSE;
            }
            if( p2aRequest.mRequest != rscData.mRequest )
            {
                MY_LOGE("RSCData out of sync");
                return MFALSE;
            }
        }
        if( mPipeUsage.supportDual() )
        {
            if( !mFOVDatas.deque(fovData) )
            {
                MY_LOGE("FOVData deque out of sync");
                return MFALSE;
            }
            if( p2aRequest.mRequest != fovData.mRequest )
            {
                MY_LOGE("FOVData out of sync");
                return MFALSE;
            }
        }
        TRACE_FUNC_ENTER();
        request = p2aRequest.mRequest;
        if( needFlushAll(request) )
        {
            this->flushAll();
        }
        request->mTimer.startEIS();
        TRACE_FUNC("Frame %d in EIS needEIS=%d queue size=%zu", request->mRequestNo, request->needEIS(), mQueue.size());
        if( request->needEIS() )
        {
            processEIS(request, p2aRequest.mData, rscData.mData, fovData.mData);
        }
        else
        {
            handleWarpResult(request, NULL, getDomainOffset(request));
        }
        request->mTimer.stopEIS();
    }

    mInFlush = mNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH);
    if( mInFlush )
    {
        MY_LOGI("Received flush signal");
        this->flushAll();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::initEIS()
{
    TRACE_FUNC_ENTER();

    mEisInfo.mode = mPipeUsage.getEISMode();
    mEisInfo.factor = mPipeUsage.getEISFactor();
    mEisInfo.videoConfig = mPipeUsage.getEISVideoConfig();
    mEisInfo.queueSize = mPipeUsage.getEISQueueSize();
    mEisInfo.startFrame = mPipeUsage.getEISStartFrame();

    if( mPipeUsage.supportFEFM() )
    {
        FMHal::getConfig(mPipeUsage.getStreamingSize(), mFEFMConfig);
        for( MUINT32 i = 0; i < MULTISCALE_FEFM; ++i )
        {
            mFEFMScale.MultiScale_width[i] = mFEFMConfig[i].mImageSize.w;
            mFEFMScale.MultiScale_height[i] = mFEFMConfig[i].mImageSize.h;
            mFEFMScale.MultiScale_blocksize[i] = mFEFMConfig[i].mBlockSize;
        }
    }

    if( mPipeUsage.supportFSC() )
    {
        mFSCInfo.isEnabled = MTRUE;
        mFSCInfo.numSlices = FSC_WARPING_SLICE_NUM;
    }
    mpEisHal = EisHal::CreateInstance("FeaturePipe_EisNode", mEisInfo, mSensorIndex, mPipeUsage.supportMultiSensor());
    if (mpEisHal == NULL)
    {
        MY_LOGE("FeaturePipe_EisNode: Create EIS Instance failed");
        return MFALSE;
    }

    if(EIS_RETURN_NO_ERROR != mpEisHal->Init())
    {
        MY_LOGE("FeaturePipe_EisNode: mpEisHal init failed");
        return MFALSE;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID EISNode::uninitEIS()
{
}

MVOID EISNode::prepareBufferPool()
{
    /* Determine WarpMap Dimension */
    if( mPipeUsage.supportEIS_22() ||
        (mPipeUsage.supportEIS_25() && !EIS_MODE_IS_EIS_DEJELLO_ENABLED(mEisInfo.mode)) )
    {
        mWarpGridSize = MSize(2,2);
    }
    else
    {
        mWarpGridSize = MSize(31,18);
    }
    MY_LOGD("EIS Warp Matrix = Grid W(%d), H(%d), EISMode(0x%x)",
            mWarpGridSize.w, mWarpGridSize.h, mEisInfo.mode);

    mWarpMapBufferPool = FatImageBufferPool::create("EISNode", mWarpGridSize, eImgFmt_WARP_2PLANE, FatImageBufferPool::USAGE_HW_AND_SW);
}

MVOID EISNode::processEIS(const RequestPtr &request, const FMResult &fm, const RSCResult &rsc, const FOVResult &fov)
{
    TRACE_FUNC_ENTER();
    EIS_HAL_CONFIG_DATA config;

    prepareEIS(request, config);
    if( mPipeUsage.supportEIS_30() )
    {
        processEIS30(request, config, rsc, fov);
    }
    else if( mPipeUsage.supportEIS_25() )
    {
        processEIS25(request, config, fm);
    }
    else if( mPipeUsage.supportEIS_22() )
    {
        processEIS22(request, config);
    }
    else
    {
        MY_LOGE("Frame %d missing EISMode", request->mRequestNo);
    }

    TRACE_FUNC_EXIT();
}

MBOOL EISNode::prepareEIS(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    extractConfig(request, mEISHalCfgData);
    ret = applyConfig(request, mEISHalCfgData);
    memcpy(&config, &mEISHalCfgData, sizeof(EIS_HAL_CONFIG_DATA));
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID EISNode::extractConfig(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    MSize sensorSize = request->getVar<MSize>(VAR_EIS_SENSOR_SIZE, MSize());
    MRect scalerCrop = request->getVar<MRect>(VAR_EIS_SCALER_CROP, MRect());
    MSize scalerOutSize = request->getVar<MSize>(VAR_EIS_SCALER_SIZE, MSize());
    MSizeF gpuTargetSize;

    config.gmv_X = request->getVar<MINT32>(VAR_EIS_GMV_X, 0);
    config.gmv_Y = request->getVar<MINT32>(VAR_EIS_GMV_Y, 0);
    config.confX = request->getVar<MINT32>(VAR_EIS_CONF_X, 0);
    config.confY = request->getVar<MINT32>(VAR_EIS_CONF_Y, 0);

    config.sensorIdx = request->getMasterID();
    config.vHDREnabled = request->needVHDR() ? 1 : 0;
    config.sensor_Width = sensorSize.w;
    config.sensor_Height = sensorSize.h;
    config.rrz_crop_Width = scalerCrop.s.w;
    config.rrz_crop_Height = scalerCrop.s.h;
    config.rrz_crop_X = scalerCrop.p.x;
    config.rrz_crop_Y = scalerCrop.p.y;
    config.rrz_scale_Width = scalerOutSize.w;
    config.rrz_scale_Height = scalerOutSize.h;

    MSize inSize = request->getEISInputSize();
    MSizeF inSizeF(inSize.w, inSize.h);
    if( mPipeUsage.supportDual() )
    {
        inSize = request->getFOVAlignSize();
        config.is_multiSensor = 1;
        config.fov_wide_idx = mPipeUsage.getDualSensorIndex_Wide();
        config.fov_tele_idx = mPipeUsage.getDualSensorIndex_Tele();
    }
    else
    {
        inSize = request->getEISInputSize();
    }
    gpuTargetSize = inSizeF - request->getEISMarginPixel()*2;

    if( mPipeUsage.supportFSC() )
    {
        // EIS margin pixel is relative to FSC max cropped region
        MSize margin = request->getFSCMaxMarginPixel();
        gpuTargetSize.w = ( inSizeF.w - margin.w*2 ) * 100 / mEisInfo.factor;
        gpuTargetSize.h = ( inSizeF.h - margin.h*2 ) * 100 / mEisInfo.factor;
    }

    config.gpuTargetW = gpuTargetSize.w;
    config.gpuTargetH = gpuTargetSize.h;
    config.crzOutW = request->getEISInputSize().w;
    config.crzOutH = request->getEISInputSize().h;
    config.srzOutW = request->getEISInputSize().w;
    config.srzOutH = request->getEISInputSize().h;
    config.feTargetW = request->getEISInputSize().w;
    config.feTargetH = request->getEISInputSize().h;
    config.cropX = 0; // No longer use CRZ crop
    config.cropY = 0; // No longer use CRZ crop
    config.imgiW = request->getEISInputSize().w;
    config.imgiH = request->getEISInputSize().h;
    config.warp_precision = mPipeUsage.getWarpPrecision();

    if( mEnableDump && mLogCount == 0 )
    {
        mLogCount = (request->getVar<MINT64>(VAR_EIS_TIMESTAMP, 0) / (MUINT64)1000000000LL);
        char path[256];
        snprintf(path, sizeof(path), EISNODE_DUMP_FOLDER_PATH"/%d", mLogCount);
        mkdir(EISNODE_DUMP_FOLDER_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    MY_LOGD("Frame:%d/%d(App=%d),eisMode:0x%x,factor:%d,sensor(%dx%d),scalerCrop:(%d,%d)(%dx%d)=>Out(%dx%d),imgi(%u,%u),crz(%u,%u),srz(%u,%u),feT(%u,%u),gpuT(%u,%u),crop(%u,%u),pri(%d),fsc(%d/%d).qSize=%zu,qAction=%d,qCounter=%d",
            request->mRequestNo, request->mRecordNo,
            request->getVar<IStreamingFeaturePipe::eAppMode>(VAR_APP_MODE, IStreamingFeaturePipe::APP_PHOTO_PREVIEW),
            mEisInfo.mode, mEisInfo.factor,
            config.sensor_Width, config.sensor_Height,
            config.rrz_crop_X , config.rrz_crop_Y,
            config.rrz_crop_Width, config.rrz_crop_Height,
            config.rrz_scale_Width, config.rrz_scale_Height,
            config.imgiW, config.imgiH,
            config.crzOutW, config.crzOutH,
            config.srzOutW, config.srzOutH,
            config.feTargetW, config.feTargetH,
            config.gpuTargetW, config.gpuTargetH,
            config.cropX, config.cropY,
            config.warp_precision,
            mFSCInfo.isEnabled, mFSCInfo.numSlices,
            mQueue.size(), request->getEISQAction(), request->getEISQCounter()
            );
    TRACE_FUNC_EXIT();
}

MBOOL EISNode::applyConfig(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MINT32 eisRet = 0;

    if( request->needEIS30() )
    {
        eisRet = request->needFSC() ?
                 mpEisHal->ConfigRSCMEEis(config, mEisInfo.mode, &mFSCInfo) :
                 mpEisHal->ConfigRSCMEEis(config, mEisInfo.mode);

        ret = (eisRet == EIS_RETURN_NO_ERROR);
        MY_LOGE_IF(!ret, "EISNode ConfigRSCMEEis fail (%d)", eisRet);
    }
    else if( request->needEIS25() )
    {
        // Only image-based version need config FEFM
        eisRet = request->needFEFM() ?
                 mpEisHal->ConfigFEFMEis(config, mEisInfo.mode, &mFEFMScale) :
                 mpEisHal->ConfigFEFMEis(config, mEisInfo.mode);
        ret = (eisRet == EIS_RETURN_NO_ERROR);
        MY_LOGE_IF(!ret, "EISNode ConfigFEFMEis fail (%d)", eisRet);
    }
    else
    {
        eisRet = mpEisHal->ConfigGis(config, mEisInfo.mode);
        ret = (eisRet == EIS_RETURN_NO_ERROR);
        MY_LOGE_IF(!ret, "EISNode ConfigGis fail (%d)", eisRet);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::processEIS22(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    ImgBuffer warpMap;
    MINT64 ts = request->getVar<MINT64>(VAR_EIS_TIMESTAMP, 0);
    MINT64 expTime = request->getVar<MINT32>(VAR_EIS_EXP_TIME, 0);

    warpMap = mWarpMapBufferPool->requestIIBuffer();
    mpEisHal->SetEisPlusWarpInfo((MINT32*)warpMap->getImageBuffer()->getBufVA(0),
                                 (MINT32*)warpMap->getImageBuffer()->getBufVA(1));
    if( request->is4K2K() )
    {
        WarpBase::makePassThroughWarp(warpMap->getImageBuffer(), mWarpGridSize, request->getEISInputSize());
    }
    else
    {
        mpEisHal->DoGis(&config, ts, expTime);
    }
    handleWarpResult(request, warpMap, getDomainOffset(request));
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::processEIS25(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config, const FMResult &fm)
{
    TRACE_FUNC_ENTER();
    ImgBuffer warpMap;
    FEFM_PACKAGE fefmCfg;
    IMAGE_BASED_DATA imgBaseData;
    MINT64 ts = request->getVar<MINT64>(VAR_EIS_TIMESTAMP, 0);
    MINT64 expTime = request->getVar<MINT32>(VAR_EIS_EXP_TIME, 0);

    warpMap = mWarpMapBufferPool->requestIIBuffer();
    prepareFEFM(fm, fefmCfg);

    EISQData qData = decideQData(request, config);
    imgBaseData.fefmData = &fefmCfg;

    configAdvEISConfig(request, warpMap, qData, qData.mConfig);
    mpEisHal->DoFEFMEis(&qData.mConfig, &imgBaseData, ts, expTime);
    handleQDataWarpResult(request, warpMap, qData);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::processEIS30(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config, const RSCResult &rsc, const FOVResult &fov)
{
    TRACE_FUNC_ENTER();

    ImgBuffer warpMap;
    FSC_WARPING_DATA_STRUCT fsc;

    IMAGE_BASED_DATA imgBaseData;
    RSCME_PACKAGE rscCfg;
    LMV_PACKAGE lmvCfg;
    FSC_PACKAGE fscCfg;

    MINT64 ts = request->getVar<MINT64>(VAR_EIS_TIMESTAMP, 0);
    MINT64 expTime = request->getVar<MINT32>(VAR_EIS_EXP_TIME, 0);
    MINT64 longExpTime = request->getVar<MINT32>(VAR_EIS_LONGEXP_TIME, 0);
    lmvCfg.enabled = request->tryGetVar<EIS_STATISTIC_STRUCT>(VAR_EIS_LMV_DATA, lmvCfg.data);

    warpMap = mWarpMapBufferPool->requestIIBuffer();

    lmvCfg.enabled = request->tryGetVar<EIS_STATISTIC_STRUCT>(VAR_EIS_LMV_DATA, lmvCfg.data);
    prepareRSC(rsc, rscCfg, config);
    prepareFOV(request, fov, config);
    prepareFSC(request, fsc, fscCfg);

    EISQData qData = decideQData(request, config);
    imgBaseData.rscData = &rscCfg;
    imgBaseData.lmvData = &lmvCfg;
    imgBaseData.fscData = &fscCfg;

    configAdvEISConfig(request, warpMap, qData, qData.mConfig);

    mpEisHal->DoRSCMEEis(&qData.mConfig, &imgBaseData, ts, expTime, longExpTime);
    handleQDataWarpResult(request, warpMap, qData);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MINTPTR getVA(const ImgBuffer &img)
{
    MINTPTR va = NULL;
    if( img != NULL )
    {
        IImageBuffer *ptr = NULL;
        ptr = img->getImageBufferPtr();
        ptr->syncCache(eCACHECTRL_INVALID);
        va = ptr->getBufVA(0);
    }
    return va;
}

template<typename T>
T getVal(const ImgBuffer &img)
{
    T val = 0;
    if( img != NULL )
    {
        IImageBuffer *ptr = NULL;
        MINTPTR va = NULL;
        ptr = img->getImageBufferPtr();
        ptr->syncCache(eCACHECTRL_INVALID);
        va = ptr->getBufVA(0);
        val = *((T*)va);
    }
    return val;
}

MVOID EISNode::prepareFEFM(const FMResult &fm, FEFM_PACKAGE &fefmCfg)
{
    TRACE_FUNC_ENTER();
    if( fm.FM_A.isValid() )
    {
        //Previous to Current
        fefmCfg.ForwardFM[0] = (MUINT16*)getVA(fm.FM_A.High);
        fefmCfg.ForwardFM[1] = (MUINT16*)getVA(fm.FM_A.Medium);
        fefmCfg.ForwardFM[2] = (MUINT16*)getVA(fm.FM_A.Low);
        fefmCfg.ForwardFMREG[0] = getVal<MUINT32>(fm.FM_A.Register_High);
        fefmCfg.ForwardFMREG[1] = getVal<MUINT32>(fm.FM_A.Register_Medium);
        fefmCfg.ForwardFMREG[2] = getVal<MUINT32>(fm.FM_A.Register_Low);
    }
    if( fm.FM_B.isValid() )
    {
        //Current to Previous
        fefmCfg.BackwardFM[0] = (MUINT16*)getVA(fm.FM_B.High);
        fefmCfg.BackwardFM[1] = (MUINT16*)getVA(fm.FM_B.Medium);
        fefmCfg.BackwardFM[2] = (MUINT16*)getVA(fm.FM_B.Low);
        fefmCfg.BackwardFMREG[0] = getVal<MUINT32>(fm.FM_A.Register_High);
        fefmCfg.BackwardFMREG[1] = getVal<MUINT32>(fm.FM_A.Register_Medium);
        fefmCfg.BackwardFMREG[2] = getVal<MUINT32>(fm.FM_A.Register_Low);
    }
    if( fm.FE.isValid() )
    {
        fefmCfg.FE[0] = (MUINT16*)getVA(fm.FE.High);
        fefmCfg.FE[1] = (MUINT16*)getVA(fm.FE.Medium);
        fefmCfg.FE[2] = (MUINT16*)getVA(fm.FE.Low);
    }
    if( fm.PrevFE.isValid() )
    {
        fefmCfg.LastFE[0] = (MUINT16*)getVA(fm.PrevFE.High);
        fefmCfg.LastFE[1] = (MUINT16*)getVA(fm.PrevFE.Medium);
        fefmCfg.LastFE[2] = (MUINT16*)getVA(fm.PrevFE.Low);
    }

    TRACE_FUNC_EXIT();
}

MVOID EISNode::prepareRSC(const RSCResult &rsc, RSCME_PACKAGE &rscCfg, EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    if( rsc.mIsValid )
    {
        rscCfg.RSCME_mv  = (MUINT8*)getVA(rsc.mMV);
        rscCfg.RSCME_var = (MUINT8*)getVA(rsc.mBV);
        config.rssoWidth  = rsc.mRssoSize.w;
        config.rssoHeight = rsc.mRssoSize.h;
    }
    TRACE_FUNC_EXIT();
}

MVOID EISNode::prepareFOV(const RequestPtr &request, const FOVResult &fov, EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    if( mPipeUsage.supportDual() )
    {
        config.fov_align_Width  = request->getFOVAlignSize().w;
        config.fov_align_Height = request->getFOVAlignSize().h;
        MINT32 *GridX = (MINT32*)fov.mWarpMap->getImageBuffer()->getBufVA(0);
        MINT32 *GridY = (MINT32*)fov.mWarpMap->getImageBuffer()->getBufVA(1);
        config.warp_grid[0].x = GridX[0];
        config.warp_grid[1].x = GridX[1];
        config.warp_grid[2].x = GridX[2];
        config.warp_grid[3].x = GridX[3];
        config.warp_grid[0].y = GridY[0];
        config.warp_grid[1].y = GridY[1];
        config.warp_grid[2].y = GridY[2];
        config.warp_grid[3].y = GridY[3];
        TRACE_FUNC("fov_align_Width(%d), fov_align_Height(%d)", config.fov_align_Width, config.fov_align_Height);
        TRACE_FUNC("GridX[0]=%f, GridX[1]=%f, GridX[2]=%f, GridX[3]=%f",
                   config.warp_grid[0].x, config.warp_grid[1].x, config.warp_grid[2].x, config.warp_grid[3].x);
        TRACE_FUNC("GridY[0]=%f, GridY[1]=%f, GridY[2]=%f, GridY[3]=%f",
                   config.warp_grid[0].y, config.warp_grid[1].y, config.warp_grid[2].y, config.warp_grid[3].y);
    }
    TRACE_FUNC_EXIT();
}

MVOID EISNode::prepareFSC(const RequestPtr &request, FSC_WARPING_DATA_STRUCT &fsc, FSC_PACKAGE &fscCfg)
{
    TRACE_FUNC_ENTER();
    if( mPipeUsage.supportFSC() )
    {
        MBOOL ret = request->tryGetVar<FSC_WARPING_DATA_STRUCT>(VAR_FSC_RRZO_WARP_DATA, fsc);
        if( ret )
        {
            fscCfg.procWidth     = request->getEISInputSize().w - request->getFSCMaxMarginPixel().w*2;
            fscCfg.procHeight    = request->getEISInputSize().h - request->getFSCMaxMarginPixel().h*2;
            fscCfg.scalingFactor = fsc.fsc_warp_result.scale_list;
        }
        else
        {
            MY_LOGW("Cannot get FSC data");
        }

        if( request->needDump() || mEnableDump )
        {
            MUINT32 size = sizeof(FSC_WARPING_RESULT_INFO_STRUCT);

            char path[256];
            snprintf(path, sizeof(path), EISNODE_DUMP_FOLDER_PATH"/%d/%04d_r%04d_fsc_warp_data_%d_%d.bin",
                mLogCount, request->mRequestNo, request->mRecordNo, size, request->needFSC());
            dumpData((char *)&fsc.fsc_warp_result, size, path);
        }

    }
    TRACE_FUNC_EXIT();
}

MSizeF EISNode::getDomainOffset(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    MSizeF domainOffset = request->getEISMarginPixel();
    if( mPipeUsage.supportFOVCombineEIS() )
    {
        domainOffset += request->getFOVMarginPixel();
    }
    TRACE_FUNC("frame(%d) EISNode domainoffset=(%fx%f)",
               request->mRequestNo, domainOffset.w, domainOffset.h);
    TRACE_FUNC_EXIT();
    return domainOffset;
}

EISQData EISNode::decideQData(const RequestPtr &request, const EIS_HAL_CONFIG_DATA &config)
{
    TRACE_FUNC_ENTER();
    EISQ_ACTION qAction = request->getEISQAction();
    MUINT32 qCounter = request->getEISQCounter();
    RequestPtr qDataRequest = mPipeUsage.supportEIS_TSQ() ? NULL : request;
    EISQData qData(qDataRequest, config, getDomainOffset(request));
    MBOOL noQ = MTRUE;

    if( qAction == EISQ_ACTION_PUSH )
    {
        mQueue.push_back(qData);
        ++mRefCount;
        noQ = MFALSE;
    }
    else if( qAction == EISQ_ACTION_RUN ||
             qAction == EISQ_ACTION_STOP ||
             qAction == EISQ_ACTION_NO )
    {
        mQueue.push_back(qData);
        qData = mQueue.front();
        mQueue.pop_front();
        if( qAction == EISQ_ACTION_STOP && mRefCount )
        {
            --mRefCount;
        }
        noQ = MFALSE;
    }
    else if( qAction == EISQ_ACTION_INIT ||
             qAction == EISQ_ACTION_READY )
    {
    }

    switch( qAction )
    {
    case EISQ_ACTION_NO:    qData.setAlg(EISALG_QUEUE_NONE, 0);             break;
    case EISQ_ACTION_INIT:  qData.setAlg(EISALG_QUEUE_INIT, 0);             break;
    case EISQ_ACTION_PUSH:  qData.setAlg(EISALG_QUEUE_WAIT, qCounter);      break;
    case EISQ_ACTION_RUN:   qData.setAlg(EISALG_QUEUE, mEisInfo.queueSize); break;
    case EISQ_ACTION_STOP:  qData.setAlg(EISALG_QUEUE_STOP, qCounter);      break;
    case EISQ_ACTION_READY: qData.setAlg(EISALG_QUEUE_NONE, 0);             break;
    default:                qData.setAlg(EISALG_QUEUE_NONE, 0);             break;
    }

    if( (noQ && mQueue.size()) || (qCounter != mRefCount) )
    {
        MY_LOGW("noQ=%d, QSize=%zu, action=%d, qCounter=%d, refCount=%d",
                noQ, mQueue.size(), qAction, qCounter, mRefCount);
    }

    TRACE_FUNC("Frame: %d, qReq=%d QSize=%zu qAction=%d qCounter=%d algMode=%d algCounter=%d", request->mRequestNo, qData.getReqNo(), mQueue.size(), qAction, qCounter, qData.mAlgMode, qData.mAlgCounter);
    TRACE_FUNC_EXIT();
    return qData;
}

MVOID EISNode::configAdvEISConfig(const RequestPtr &request, const ImgBuffer &warp, const EISQData &qData, EIS_HAL_CONFIG_DATA &config)
{
    (void)request;
    TRACE_FUNC_ENTER();
    mpEisHal->SetEisPlusWarpInfo((MINT32*)(warp->getImageBuffer())->getBufVA(0),
                                 (MINT32*)(warp->getImageBuffer())->getBufVA(1));

    config.process_mode = mInFlush ? EISALG_QUEUE_NONE : qData.mAlgMode;
    config.process_idx = mInFlush ? 0 : qData.mAlgCounter;

    TRACE_FUNC("Frame: %d, Queue size: %zu, _mode: %d, _idx: %d",
                request->mRequestNo, mQueue.size(), config.process_mode, config.process_idx);
    TRACE_FUNC_EXIT();
}

MBOOL EISNode::handleWarpResult(const RequestPtr &request, const ImgBuffer &warp, const MSizeF &domainOffset)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame: %d, warp=%p", request->mRequestNo, warp.get());
    MBOOL ret = MFALSE;
    ret = handleData(ID_EIS_TO_WARP, BasicImgData(BasicImg(warp, domainOffset), request));
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL EISNode::handleQDataWarpResult(const RequestPtr &request, const ImgBuffer &warp, const EISQData &qData)
{
    TRACE_FUNC_ENTER();
    EISQ_ACTION qAction = request->getEISQAction();

    if( mPipeUsage.supportEIS_TSQ() )
    {
        ImgBuffer warpOut = (qAction == EISQ_ACTION_PUSH) ? NULL : warp;
        handleWarpResult(request, warpOut, qData.mDomainOffset);
    }
    else
    {
        if( qAction != EISQ_ACTION_PUSH )
        {
            handleWarpResult(qData.mRequest, warp, qData.mDomainOffset);
        }
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL EISNode::needFlushAll(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mQueue.size() )
    {
        ret = !request->needEIS() ||
              (request->getEISQAction() == EISQ_ACTION_NO);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID EISNode::flushAll()
{
    TRACE_FUNC_ENTER();

    MY_LOGI("Trigger EIS flush queue: size=%zu", mQueue.size());

    while( mQueue.size() )
    {
        EISQData qData;
        qData = mQueue.front();
        mQueue.pop_front();
        if( mPipeUsage.supportEIS_TSQ() || qData.mRequest == NULL )
        {
            MY_LOGW("pop request=%p tsq=%d", qData.mRequest.get(), mPipeUsage.supportEIS_TSQ());
        }
        else
        {
            handleWarpResult(qData.mRequest, NULL, qData.mDomainOffset);
        }
    }
    mRefCount = 0;

    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
