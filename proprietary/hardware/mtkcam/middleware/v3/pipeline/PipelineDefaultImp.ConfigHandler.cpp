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

#define LOG_TAG "MtkCam/HwPipeline/ConfigHandler"
//
#include "PipelineDefaultImp.h"
#include "PipelineUtility.h"
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/pipeline/hwnode/FDNode.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>
#include <mtkcam/pipeline/hwnode/RAW16Node.h>
#include <mtkcam/pipeline/extension/MFNR.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/hw/IScenarioControl.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/aaa/IIspMgr.h>
//
#include <cutils/compiler.h>

#include <camera_custom_eis.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::EIS;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::NSPipelineContext;


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configureLocked(
    PipeConfigParams const& rConfigParams,
    android::sp<IPipelineModel> pOldPipeline
)
{
    CAM_TRACE_NAME("PipelineDefaultImp::configure");
    FUNC_START;
    // RWLock::AutoWLock _l(mRWLock);
    //
    CHECK_ERROR(checkPermission());
    //

    //
    mCommonInfo->mCameraSetting->generateCamSetting(rConfigParams, mParams);
    //
    MY_LOGD_IF(mCommonInfo->mpPipelineContext.get(), "strong count %d", mCommonInfo->mpPipelineContext->getStrongCount());
    mCommonInfo->mpPipelineContext = PipelineContext::create("DefaultPipeline");
    //
    if ( pOldPipeline.get() )
        mpOldCtx = pOldPipeline->getContext();
    MY_LOGD_IF(  mpOldCtx.get(), "old strong count %d", mpOldCtx->getStrongCount());
    //
    if ( mpOldCtx.get() )
    {
#if 1
        CHECK_ERROR(mpOldCtx->waitUntilNodeDrained(0x01));
#else
        CHECK_ERROR(mpOldCtx->waitUntilDrained());
#endif
    }
    //
    CHECK_ERROR(mCommonInfo->mpPipelineContext->beginConfigure(mpOldCtx));
    //
    CHECK_ERROR(configScenarioCtrlLocked());
    // create IStreamInfos
    CHECK_ERROR(setupAppStreamsLocked(rConfigParams));
    CHECK_ERROR(setupHalStreamsLocked(rConfigParams));
    //
    // config stream
    CHECK_ERROR(configContextLocked_Streams(mCommonInfo->mpPipelineContext));
    // config node
    CHECK_ERROR(configContextLocked_Nodes(mCommonInfo->mpPipelineContext, MFALSE));
    // config pipeline
    CHECK_ERROR(configContextLocked_Pipeline(mCommonInfo->mpPipelineContext));
    //
    CHECK_ERROR(mCommonInfo->mpPipelineContext->endConfigure(true));
    mpOldCtx = NULL;
    //
    for (MINT32 i = 0; i < mParams->mPipelineConfigParams.mbUseP1Node.size(); i++)
    {
        CHECK_ERROR(configRequestRulesLocked(i));
    }
    //
    mpDeviceHelper->configDone();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
P2Common::UsageHint
PipelineDefaultImp::ConfigHandler::
prepareP2Usage(
    P2Node::ePass2Type type
) const
{
    P2Common::UsageHint usage;

    usage.mP2NodeType = P2Common::P2_NODE_COMMON;
    usage.m3DNRMode = 0;
    usage.mUseTSQ = 0;

    usage.mPackedEisInfo = mParams->mPipelineConfigParams.mEisInfo.toPackedData();
    if( mParams->mPipelineConfigParams.mbHasRecording )
    {
        usage.mAppMode = P2Common::APP_MODE_VIDEO;
    }

    if( mParams->mStreamSet.mpHalImage_P1_ResizerRaw[0] != NULL )
    {
        usage.mStreamingSize = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[0]->getImgSize();
    }
    MY_LOGI("operation_mode=%d p2_type=%d p2_node_type=%d app_mode=%d 3dnr_mode=0x%x eis_mode=0x%x eis_factor=%d stream_size=%dx%d",
            mParams->mPipelineConfigParams.mOperation_mode, type, usage.mP2NodeType, usage.mAppMode,
            usage.m3DNRMode, mParams->mPipelineConfigParams.mEisInfo.mode,
            mParams->mPipelineConfigParams.mEisInfo.factor, usage.mStreamingSize.w, usage.mStreamingSize.h);
    return usage;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::ConfigHandler::
evaluatePreviewSize(
    PipeConfigParams const& rConfigParams,
    MSize &rSize
)
{
    sp<IImageStreamInfo> pStreamInfo;
    int consumer_usage = 0;
    int allocate_usage = 0;
    int maxheight = rSize.h;
    int prevwidth = 0;
    int prevheight = 0;
    for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
        if  ( (pStreamInfo = rConfigParams.vImage_Yuv_NonStall[i]) != 0 ) {
            consumer_usage = pStreamInfo->getUsageForConsumer();
            allocate_usage = pStreamInfo->getUsageForAllocator();
            MY_LOGD("consumer : %X, allocate : %X", consumer_usage, allocate_usage);
            if(consumer_usage & GRALLOC_USAGE_HW_TEXTURE) {
                prevwidth = pStreamInfo->getImgSize().w;
                prevheight = pStreamInfo->getImgSize().h;
                break;
            }
            if(consumer_usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) {
                continue;
            }
            prevwidth = pStreamInfo->getImgSize().w;
            prevheight = pStreamInfo->getImgSize().h;
        }
    }
    if(prevwidth == 0 || prevheight == 0)
        return ;
    rSize.h = prevheight * rSize.w / prevwidth;
    if(maxheight < rSize.h) {
        MY_LOGW("Warning!!,  scaled preview height(%d) is larger than max height(%d)", rSize.h, maxheight);
        rSize.h = maxheight;
    }
    MY_LOGD("evaluate preview size : %dx%d", prevwidth, prevheight);
    MY_LOGD("FD buffer size : %dx%d", rSize.w, rSize.h);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
setupAppStreamsLocked(
    PipeConfigParams const& rConfigParams
)
{
    mParams->mStreamSet.mvStreamDurations.clear();
    mParams->mStreamSet.mpAppMeta_DynamicP1.clear();

    MINT32 iBurstNum = mParams->mCHSvrParams.mDefaultBusrstNum;
    //App:Meta:Control
    {
        sp<IMetaStreamInfo> pStreamInfo;
        if  ( (pStreamInfo = rConfigParams.pMeta_Control) != 0 )
        {
            mParams->mStreamSet.mpAppMeta_Control = pStreamInfo;
            pStreamInfo->setMaxBufNum(10*iBurstNum);
        }
    }

    //App:dynamic
    if( mParams->mPipelineConfigParams.mbUseP1Node.size() > 0 && mParams->mPipelineConfigParams.mbUseP1Node[0] )
    {
        //App:Meta:01
        //   pass1 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP1",
                    eSTREAMID_META_APP_DYNAMIC_01,
                    eSTREAMTYPE_META_OUT,
                    10*iBurstNum, 1
                    );
        mParams->mStreamSet.mpAppMeta_DynamicP1.push_back(pStreamInfo);
    }

    if( mParams->mPipelineConfigParams.mbUseP1Node.size() > 1 && mParams->mPipelineConfigParams.mbUseP1Node[1] )
    {
        //App:Meta:01_MAIN2
        //   pass1 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP1_Main2",
                    eSTREAMID_META_APP_DYNAMIC_01_MAIN2,
                    eSTREAMTYPE_META_OUT,
                    10*iBurstNum, 1
                    );
        mParams->mStreamSet.mpAppMeta_DynamicP1.push_back(pStreamInfo);
    }

    if( mParams->mPipelineConfigParams.mbUseP2Node )
    {
        //App:Meta:P2
        //   pass2 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP2",
                    eSTREAMID_META_APP_DYNAMIC_02,
                    eSTREAMTYPE_META_OUT,
                    10*iBurstNum, 1
                    );
        mParams->mStreamSet.mpAppMeta_DynamicP2 = pStreamInfo;
    }

    if( mParams->mPipelineConfigParams.mbUseP2CapNode )
    {
        //App:Meta:P2
        //   pass2 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP2Cap",
                    eSTREAMID_META_APP_DYNAMIC_02_CAP,
                    eSTREAMTYPE_META_OUT,
                    10*iBurstNum, 1
                    );
        mParams->mStreamSet.mpAppMeta_DynamicP2Capture = pStreamInfo;
        MY_LOGD("WillDBG8 eSTREAMID_META_APP_DYNAMIC_02_CAP : %d", eSTREAMID_META_APP_DYNAMIC_02_CAP);
    }

    if( mParams->mPipelineConfigParams.mbUseFDNode )
    {
        //App:Meta:FD
        //   FD result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:FD",
                    eSTREAMID_META_APP_DYNAMIC_FD,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mParams->mStreamSet.mpAppMeta_DynamicFD = pStreamInfo;
    }

    if( mParams->mPipelineConfigParams.mbUseJpegNode )
    {
        //App:Meta:Jpeg
        //   Jpeg result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:Jpeg",
                    eSTREAMID_META_APP_DYNAMIC_JPEG,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mParams->mStreamSet.mpAppMeta_DynamicJpeg = pStreamInfo;
    }


    //App:Image
    {
        sp<IImageStreamInfo> pStreamInfo;
        //
        //App:Image:Raw
        if  ( (pStreamInfo = rConfigParams.pImage_Raw) != 0
                 &&  eImgFmt_RAW16 == pStreamInfo->getImgFormat()
        ) {
            // RAW16 = rConfigParams.pImage_Raw set this stream
            mParams->mStreamSet.mpAppImage_RAW16 = pStreamInfo;
            pStreamInfo->setMaxBufNum(1);
            //
            mParams->mStreamSet.mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Raw_Duration.minDuration
                    );
        }
        //
        //App:Image:Jpeg:Stall
        if  ( (pStreamInfo = rConfigParams.pImage_Jpeg_Stall) != 0 ) {
            mParams->mStreamSet.mpAppImage_Jpeg = pStreamInfo;
            pStreamInfo->setMaxBufNum(1);
            //
            mParams->mStreamSet.mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Jpeg_Duration.minDuration
                    );
        }
        //App:Image:Yuv:In
        if  ( (pStreamInfo = rConfigParams.pImage_Yuv_In) != 0 ) {
            mParams->mStreamSet.mpAppImage_Yuv_In = pStreamInfo;
            pStreamInfo->setMaxBufNum(2);
            //
            mParams->mStreamSet.mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Yuv_In_Duration.minDuration
                    );
        }
        //App:Image:Opaque:In
        if  ( (pStreamInfo = rConfigParams.pImage_Opaque_In) != 0 ) {
            mParams->mStreamSet.mpAppImage_Opaque_In = pStreamInfo;
            pStreamInfo->setMaxBufNum(2);
            //
            mParams->mStreamSet.mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Opaque_In_Duration.minDuration
                    );
        }
        //App:Image:Opaque:Out
        if  ( (pStreamInfo = rConfigParams.pImage_Opaque_Out) != 0 ) {
            mParams->mStreamSet.mpAppImage_Opaque_Out = pStreamInfo;
            pStreamInfo->setMaxBufNum(6);
            //
            mParams->mStreamSet.mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Opaque_Out_Duration.minDuration
                    );
        }
        //
        //App:Image:Yuv:NotStall

        for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  ( (pStreamInfo = rConfigParams.vImage_Yuv_NonStall[i]) != 0 ) {
                mParams->mStreamSet.mvAppYuvImage.add(pStreamInfo->getStreamId(), pStreamInfo);
                if ( mParams->mPipelineConfigParams.mOperation_mode )
                    if ( pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                        pStreamInfo->setMaxBufNum(52);
                    else
                        pStreamInfo->setMaxBufNum(12);
                else{
                    if ( (pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER)){
                        pStreamInfo->setMaxBufNum(8 + mParams->mPipelineConfigParams.mEisExtraBufNum);
                    }else{
                        pStreamInfo->setMaxBufNum(8);
                    }
                }
                //
                if( i >= rConfigParams.vImage_Yuv_Duration.size() ) {
                    MY_LOGE("not enough yuv duration for streams");
                    continue;
                }
                mParams->mStreamSet.mvStreamDurations.add(
                        pStreamInfo->getStreamId(),
                        rConfigParams.vImage_Yuv_Duration[i].minDuration
                        );
            }
        }
        //
        // dump durations
        String8 durations = String8("durations:");
        for( size_t i = 0; i < mParams->mStreamSet.mvStreamDurations.size(); i++) {
            durations += String8::format("(stream %#" PRIx64 ": %lld) ",
                    mParams->mStreamSet.mvStreamDurations.keyAt(i), (long long int)mParams->mStreamSet.mvStreamDurations.valueAt(i));
        }
        MY_LOGD("%s", durations.string());
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
setupHalStreamsLocked(
    PipeConfigParams const& rConfigParams
)
{
    MINT32 iBurstNum = mParams->mCHSvrParams.mDefaultBusrstNum;
    //Hal:Meta
    //
    mParams->mStreamSet.mpHalMeta_Control.clear();
    mParams->mStreamSet.mpHalMeta_DynamicP1.clear();
    mParams->mStreamSet.mpHalImage_P1_Raw.clear();
    mParams->mStreamSet.mpHalImage_P1_ResizerRaw.clear();
    mParams->mStreamSet.mpHalImage_P1_Lcso.clear();
    mParams->mStreamSet.mpHalImage_P1_Rsso.clear();
    if( mParams->mPipelineConfigParams.mbUseP1Node.size() > 0 && mParams->mPipelineConfigParams.mbUseP1Node[0] )
    {
        {
            //Hal:Meta:Control
            sp<IMetaStreamInfo> pStreamInfo =
                new MetaStreamInfo(
                        "Hal:Meta:Control_Main1",
                        eSTREAMID_META_PIPE_CONTROL,
                        eSTREAMTYPE_META_IN,
                        10*iBurstNum, 1
                        );
            mParams->mStreamSet.mpHalMeta_Control.push_back(pStreamInfo);
        }
        {
            //Hal:Meta:01
            sp<IMetaStreamInfo> pStreamInfo =
                new MetaStreamInfo(
                        "Hal:Meta:P1:Dynamic_Main1",
                        eSTREAMID_META_PIPE_DYNAMIC_01,
                        eSTREAMTYPE_META_INOUT,
                        10*iBurstNum, 1
                        );
            mParams->mStreamSet.mpHalMeta_DynamicP1.push_back(pStreamInfo);
        }
    }
    //
    if( mParams->mPipelineConfigParams.mbUseP1Node.size() > 1 && mParams->mPipelineConfigParams.mbUseP1Node[1] )
    {
        {
            //Hal:Meta:Control
            sp<IMetaStreamInfo> pStreamInfo =
                new MetaStreamInfo(
                        "Hal:Meta:Control_Main2",
                        eSTREAMID_META_PIPE_CONTROL_MAIN2,
                        eSTREAMTYPE_META_IN,
                        10*iBurstNum, 1
                        );
            mParams->mStreamSet.mpHalMeta_Control.push_back(pStreamInfo);
        }
        {
            //Hal:Meta:01
            sp<IMetaStreamInfo> pStreamInfo =
                new MetaStreamInfo(
                        "Hal:Meta:P1:Dynamic_Main2",
                        eSTREAMID_META_PIPE_DYNAMIC_01_MAIN2,
                        eSTREAMTYPE_META_INOUT,
                        10*iBurstNum, 1
                        );
            mParams->mStreamSet.mpHalMeta_DynamicP1.push_back(pStreamInfo);
        }
    }
    //
    if( mParams->mPipelineConfigParams.mbUseP2Node )
    {
        //Hal:Meta:01
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:P2:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_02,
                    eSTREAMTYPE_META_INOUT,
                    10*iBurstNum, 1
                    );
        mParams->mStreamSet.mpHalMeta_DynamicP2 = pStreamInfo;
    }
    if( mParams->mPipelineConfigParams.mbUseP2CapNode )
    {
        //Hal:Meta:01
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:P2C:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_02_CAP,
                    eSTREAMTYPE_META_INOUT,
                    10*iBurstNum, 1
                    );
        mParams->mStreamSet.mpHalMeta_DynamicP2Capture = pStreamInfo;
    }
    mvPass1Resource.clear();
    for (MINT32 i = 0; i < mParams->mPipelineConfigParams.mbUseP1Node.size(); i++)
    {
        // p1node image port: imgo/rrzo/lcso/rsso
        pass1Resource Pass1Resource;
        // need modify to use table to config multi p1 node and stream
        int64_t ImgoId;
        int64_t RrzoId;
        int64_t LcsoId;
        int64_t RssoId;
        switch (i)
        {
            case 0:
                ImgoId = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00;
                RrzoId = eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00;
                LcsoId = eSTREAMID_IMAGE_PIPE_RAW_LCSO_00;
                RssoId = eSTREAMID_IMAGE_PIPE_RAW_RSSO_00;
                break;
            case 1:
                ImgoId = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01;
                RrzoId = eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01;
                LcsoId = eSTREAMID_IMAGE_PIPE_RAW_LCSO_01;
                RssoId = eSTREAMID_IMAGE_PIPE_RAW_RSSO_01;
                break;
            default:
                MY_LOGE("not support p1 node number large than 2");
                break;
        }
        //
        Pass1Resource.setStreamCapacity(4);
        //Hal:Image
        if ( mParams->mPipelineConfigParams.mbUseP1Node[i] &&
                !! mParams->mHwParams[i]->mFullrawSize.size() )
        {
            sp<IImageStreamInfo> pStreamInfo;
            // p1: fullsize
            MSize const& size = mParams->mHwParams[i]->mFullrawSize;
            MINT const format = mParams->mHwParams[i]->mFullrawFormat;
            size_t const stride = mParams->mHwParams[i]->mFullrawStride;
            MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
            //
            MBOOL bDone = MFALSE;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(ImgoId, pStreamInfo) &&
                 pStreamInfo.get() )
            {
                if ( pStreamInfo->getImgSize().w == size.w &&
                     pStreamInfo->getImgSize().h == size.h &&
                     pStreamInfo->getImgFormat() == format &&
                     pStreamInfo->getBufPlanes().itemAt(0).rowStrideInBytes == stride &&
                     pStreamInfo->getUsageForAllocator() == usage )
                {
                    MY_LOGD_IF( 1, "stream could be reused:%#" PRIx64 , ImgoId);
                    Pass1Resource.updateStreamStatus( ImgoId,
                                                       pass1Resource::StreamStatus::eStatus_Reuse);
                    bDone = MTRUE;
                }
            }
            //
            if ( !bDone )
            {
                pStreamInfo = createRawImageStreamInfo(
                        "Hal:Image:P1:Fullraw",
                        ImgoId,
                        eSTREAMTYPE_IMAGE_INOUT,
                        (6+2)*iBurstNum, 0,
                        usage, format, size, stride,/*, mParams->mHwInfoHelper*/
                        i
                );
                //
                if( pStreamInfo == NULL ) {
                    return BAD_VALUE;
                }
                Pass1Resource.updateStreamStatus( ImgoId,
                                                   pass1Resource::StreamStatus::eStatus_Inited);
            }
            //
            MY_LOGD_IF( 1, "setup streamid(%#" PRIx64 "):%p",
                        ImgoId, pStreamInfo.get() );
            mParams->mStreamSet.mpHalImage_P1_Raw.push_back(pStreamInfo);
        } else if ( mParams->mPipelineConfigParams.mbUseP1Node[i] &&
                    ! mParams->mHwParams[i]->mFullrawSize.size() )
        {
            sp<IImageStreamInfo> pStreamInfo;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(ImgoId, pStreamInfo) &&
                 ! pStreamInfo.get() )
            {
                MY_LOGD_IF( 1, "case of previous and current pipeline have no streamid:%#" PRIx64 ,
                            ImgoId);
                Pass1Resource.updateStreamStatus( ImgoId,
                                                   pass1Resource::StreamStatus::eStatus_NoNeed);
                mParams->mStreamSet.mpHalImage_P1_Raw.push_back(pStreamInfo);
            }
            else
            {
                mParams->mStreamSet.mpHalImage_P1_Raw.push_back(NULL);
            }
        }
        else
        {
            mParams->mStreamSet.mpHalImage_P1_Raw.push_back(NULL);
        }

        //if (
        //        mConfigProfile.mbConfigP1 &&
        //        !! mConfigProfile.mResizedSize
        if ( mParams->mPipelineConfigParams.mbUseP1Node[i] &&
                !! mParams->mHwParams[i]->mResizedrawSize.size() )
        {
            sp<IImageStreamInfo> pStreamInfo;
            // p1: resize
            MSize const& size = mParams->mHwParams[i]->mResizedrawSize;
            MINT const format = mParams->mHwParams[i]->mResizedrawFormat;
            size_t const stride = mParams->mHwParams[i]->mResizedrawStride;
            MUINT const usage = 0;
            //
            MBOOL bDone = MFALSE;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(RrzoId, pStreamInfo) &&
                 pStreamInfo.get() )
            {
                if ( pStreamInfo->getImgSize().w == size.w &&
                     pStreamInfo->getImgSize().h == size.h &&
                     pStreamInfo->getImgFormat() == format &&
                     pStreamInfo->getBufPlanes().itemAt(0).rowStrideInBytes == stride &&
                     pStreamInfo->getUsageForAllocator() == usage )
                {
                    MY_LOGD_IF( 1, "stream could be reused:%#" PRIx64 , RrzoId);
                    Pass1Resource.updateStreamStatus( RrzoId,
                                                       pass1Resource::StreamStatus::eStatus_Reuse);
                    bDone = MTRUE;
                }
            }
            //
            if ( !bDone )
            {
                pStreamInfo = createRawImageStreamInfo(
                        "Hal:Image:P1:ResizerawI",
                        RrzoId,
                        eSTREAMTYPE_IMAGE_INOUT,
                        (6+2)*iBurstNum, 0,
                        usage, format, size, stride,/*, mParams->mHwInfoHelper*/
                        i
                );
                //
                if( pStreamInfo == NULL ) {
                    return BAD_VALUE;
                }
                Pass1Resource.updateStreamStatus( RrzoId,
                                                   pass1Resource::StreamStatus::eStatus_Inited);
            }
            //
            MY_LOGD_IF( 1, "setup streamid(%#" PRIx64 "):%p",
                        RrzoId, pStreamInfo.get() );
            mParams->mStreamSet.mpHalImage_P1_ResizerRaw.push_back(pStreamInfo);
        } else if ( mParams->mPipelineConfigParams.mbUseP1Node[i] &&
                    ! mParams->mHwParams[i]->mResizedrawSize.size() )
        {
            sp<IImageStreamInfo> pStreamInfo;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(RrzoId, pStreamInfo) &&
                 ! pStreamInfo.get() )
            {
                MY_LOGD_IF( 1, "case of previous and current pipeline have no streamid:%#" PRIx64 ,
                            RrzoId);
                Pass1Resource.updateStreamStatus( RrzoId,
                                                   pass1Resource::StreamStatus::eStatus_NoNeed);
                mParams->mStreamSet.mpHalImage_P1_ResizerRaw.push_back(pStreamInfo);
            }
            else
            {
                mParams->mStreamSet.mpHalImage_P1_ResizerRaw.push_back(NULL);
            }
        }
        else
        {
            mParams->mStreamSet.mpHalImage_P1_ResizerRaw.push_back(NULL);
        }

        //Hal:Image Lcso
        if ( mParams->mPipelineConfigParams.mbUseP1Node[i] && mParams->mPipelineConfigParams.mbHasLcso[i])
        {
            sp<IImageStreamInfo> pStreamInfo;
            // p1: lcso size
            MUINT const usage = 0;
            NS3Av3::LCSO_Param lcsoParam;
            NS3Av3::IIspMgr* ispMgr = MAKE_IspMgr();
            if (ispMgr)
                ispMgr->queryLCSOParams(lcsoParam);
            else
                MY_LOGE("Query IIspMgr FAILED!");
            //
            MBOOL bDone = MFALSE;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(LcsoId, pStreamInfo) &&
                 pStreamInfo.get() )
            {

                MY_LOGD_IF( 1, "stream could be reused:%#" PRIx64 , LcsoId);
                Pass1Resource.updateStreamStatus( LcsoId,
                                                       pass1Resource::StreamStatus::eStatus_Reuse);
                bDone = MTRUE;
            }
            //
            if ( !bDone )
            {
                pStreamInfo = createRawImageStreamInfo(
                    "Hal:Image:LCSraw",
                    LcsoId,
                    eSTREAMTYPE_IMAGE_INOUT,
                    (6+2)*iBurstNum, 1,
                    usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride,/*, mParams->mHwInfoHelper*/
                    i
                );
                //
                if( pStreamInfo == NULL ) {
                    return BAD_VALUE;
                }
                Pass1Resource.updateStreamStatus( LcsoId,
                                                   pass1Resource::StreamStatus::eStatus_Inited);
            }
            //
            MY_LOGD_IF( 1, "setup streamid(%#" PRIx64 "):%p",
                        LcsoId, pStreamInfo.get() );
            mParams->mStreamSet.mpHalImage_P1_Lcso.push_back(pStreamInfo);
        } else if ( mParams->mPipelineConfigParams.mbUseP1Node[i] && ! mParams->mPipelineConfigParams.mbHasLcso[i] )
        {
            sp<IImageStreamInfo> pStreamInfo;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(LcsoId, pStreamInfo) &&
                 ! pStreamInfo.get() )
            {
                MY_LOGD_IF( 1, "case of previous and current pipeline have no streamid:%#" PRIx64 ,
                            LcsoId);
                Pass1Resource.updateStreamStatus( LcsoId,
                                                   pass1Resource::StreamStatus::eStatus_NoNeed);
                mParams->mStreamSet.mpHalImage_P1_Lcso.push_back(pStreamInfo);
            }
            else
            {
                mParams->mStreamSet.mpHalImage_P1_Lcso.push_back(NULL);
            }
        }
        else
        {
            mParams->mStreamSet.mpHalImage_P1_Lcso.push_back(NULL);
        }

        //Hal:Image Rsso
        if ( mParams->mPipelineConfigParams.mbUseP1Node[i] && mParams->mPipelineConfigParams.mbHasRsso[i])
        {
            sp<IImageStreamInfo> pStreamInfo;
            // p1: rsso size
            MUINT const usage = 0;
            NS3Av3::LCSO_Param lcsoParam;
            NS3Av3::IIspMgr* ispMgr = MAKE_IspMgr();
            MBOOL bDone = MFALSE;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(RssoId, pStreamInfo) &&
                 pStreamInfo.get() )
            {

                MY_LOGD_IF( 1, "stream could be reused:%#" PRIx64 , RssoId);
                Pass1Resource.updateStreamStatus(RssoId,
                                                  pass1Resource::StreamStatus::eStatus_Reuse);
            }
            else
            {
                MUINT usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
                MINT format = eImgFmt_STA_BYTE;
                MSize size(288, 512);
                size_t stride = size.w;
                pStreamInfo = createRawImageStreamInfo(
                    "Hal:Image:RSSO",
                    RssoId,
                    eSTREAMTYPE_IMAGE_INOUT,
                    (6+2)*iBurstNum, 1,
                    usage, format, size, stride,/*, mParams->mHwInfoHelper*/
                    i
                );
                //
                if( pStreamInfo == NULL ) {
                    return BAD_VALUE;
                }
                Pass1Resource.updateStreamStatus( RssoId,
                                                   pass1Resource::StreamStatus::eStatus_Inited);
            }
            //
            MY_LOGD_IF( 1, "setup streamid(%#" PRIx64 "):%p",
                        RssoId, pStreamInfo.get() );
            mParams->mStreamSet.mpHalImage_P1_Rsso.push_back(pStreamInfo);
        }
        else if ( mParams->mPipelineConfigParams.mbUseP1Node[i] && ! mParams->mPipelineConfigParams.mbHasRsso[i] )
        {
            sp<IImageStreamInfo> pStreamInfo;
            if ( mpOldCtx.get() &&
                 OK == mpOldCtx->queryStream(RssoId, pStreamInfo) &&
                 ! pStreamInfo.get() )
            {
                MY_LOGD_IF( 1, "case of previous and current pipeline have no streamid:%#" PRIx64 ,
                            RssoId);
                Pass1Resource.updateStreamStatus( RssoId,
                                                   pass1Resource::StreamStatus::eStatus_NoNeed);
                mParams->mStreamSet.mpHalImage_P1_Rsso.push_back(pStreamInfo);
            }
            else
            {
                mParams->mStreamSet.mpHalImage_P1_Rsso.push_back(NULL);
}
        }
        else
        {
            mParams->mStreamSet.mpHalImage_P1_Rsso.push_back(NULL);
         }
        mvPass1Resource.push_back(Pass1Resource);
    }

    //Hal:Image:FD
    if ( mParams->mPipelineConfigParams.mbUseFDNode )
    {
        //MSize const size(640, 480); //FIXME: hard-code here?
        MSize size(640, 480);
        // evaluate preview size
        evaluatePreviewSize(rConfigParams, size);

        // mPrevFDEn = MFALSE;

        MY_LOGD("evaluate FD buffer size : %dx%d", size.w, size.h);

        MINT const format = eImgFmt_YUY2;//eImgFmt_YV12;
        MUINT const usage = 0;

        sp<ImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                "Hal:Image:FD",
                eSTREAMID_IMAGE_FD,
                eSTREAMTYPE_IMAGE_INOUT,
                5, 1,
                usage, format, size
            );
        if( pStreamInfo == NULL ) {
            return BAD_VALUE;
        }
        //
        mParams->mStreamSet.mpHalImage_FD_YUV = pStreamInfo;
    }

    //Hal:Image:YUY2 for jpeg & thumbnail
    if ( mParams->mPipelineConfigParams.mbUseJpegNode )
    {
        //Hal:Image:YUY2 for jpeg
        {
            MSize const& size = rConfigParams.pImage_Jpeg_Stall->getImgSize();
            MINT const format = eImgFmt_YUY2;
            MUINT const usage = 0;
            sp<ImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                    "Hal:Image:YuvJpeg",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 0,
                    usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mParams->mStreamSet.mpHalImage_Jpeg_YUV = pStreamInfo;
        }
        //
        //Hal:Image:YUY2 for thumbnail
        {
            MSize const size(-1L, -1L); //unknown now
            MINT const format = eImgFmt_YUY2;
            MUINT const usage = 0;
            sp<ImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                    "Hal:Image:YuvThumbnail",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 0,
                    usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mParams->mStreamSet.mpHalImage_Thumbnail_YUV = pStreamInfo;
            MY_LOGD("streamId:%#" PRIx64 " %s %p", pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pStreamInfo.get());
        }
    }
    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_Streams(sp<PipelineContext> pContext)
{
    MY_LOGD("++");
    CAM_TRACE_CALL();
#define BuildStream(_type_, _IStreamInfo_)                                     \
    do {                                                                       \
        if( _IStreamInfo_.get() ) {                                            \
            MERROR err;                                                        \
            if ( OK != (err = StreamBuilder(_type_, _IStreamInfo_)             \
                    .build(pContext)) )                                        \
            {                                                                  \
                MY_LOGE("StreamBuilder fail stream %#" PRIx64 " of type %d",  \
                    _IStreamInfo_->getStreamId(), _type_);                     \
                return err;                                                    \
            }                                                                  \
        }                                                                      \
    } while(0)

    BuildStream(eStreamType_META_HAL, mParams->mStreamSet.mpHalMeta_DynamicP2);
    BuildStream(eStreamType_META_HAL, mParams->mStreamSet.mpHalMeta_DynamicP2Capture);
    //

    BuildStream(eStreamType_META_APP, mParams->mStreamSet.mpAppMeta_DynamicP2);
    BuildStream(eStreamType_META_APP, mParams->mStreamSet.mpAppMeta_DynamicP2Capture);
    BuildStream(eStreamType_META_APP, mParams->mStreamSet.mpAppMeta_DynamicFD);
    BuildStream(eStreamType_META_APP, mParams->mStreamSet.mpAppMeta_DynamicJpeg);
    BuildStream(eStreamType_META_APP, mParams->mStreamSet.mpAppMeta_Control);
    //
    BuildStream(eStreamType_IMG_HAL_POOL   , mParams->mStreamSet.mpHalImage_FD_YUV);
    //
    for (MINT32 i = 0; i < mvPass1Resource.size(); i++)
    {
        MY_LOGD("index : %d", i);
        BuildStream(eStreamType_META_HAL, mParams->mStreamSet.mpHalMeta_Control[i]);
        BuildStream(eStreamType_META_HAL, mParams->mStreamSet.mpHalMeta_DynamicP1[i]);
        BuildStream(eStreamType_META_APP, mParams->mStreamSet.mpAppMeta_DynamicP1[i]);
        MY_LOGD("Build P1 stream");
        if ( mvPass1Resource[i].checkStreamsReusable() )
        {
            if ( mParams->mStreamSet.mpHalImage_P1_Raw[i].get() )
                CHECK_ERROR( pContext->reuseStream(mParams->mStreamSet.mpHalImage_P1_Raw[i]) );
            if ( mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i].get() )
                CHECK_ERROR( pContext->reuseStream(mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i]) );
            if ( mParams->mStreamSet.mpHalImage_P1_Lcso[i].get() )
                CHECK_ERROR( pContext->reuseStream(mParams->mStreamSet.mpHalImage_P1_Lcso[i]) );
            if ( mParams->mStreamSet.mpHalImage_P1_Rsso[i].get() )
                CHECK_ERROR( pContext->reuseStream(mParams->mStreamSet.mpHalImage_P1_Rsso[i]) );
            //
            MY_LOGD_IF( 1, "Reuse:  p1 full raw(%p); resized raw(%p)",
                        mParams->mStreamSet.mpHalImage_P1_Raw[i].get(), mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i].get());
            mvPass1Resource[i].setReuseFlag(MTRUE);
        }
        else
        {
            BuildStream(eStreamType_IMG_HAL_POOL   , mParams->mStreamSet.mpHalImage_P1_Raw[i]);
            BuildStream(eStreamType_IMG_HAL_POOL   , mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i]);
            BuildStream(eStreamType_IMG_HAL_POOL   , mParams->mStreamSet.mpHalImage_P1_Lcso[i]);
            BuildStream(eStreamType_IMG_HAL_POOL   , mParams->mStreamSet.mpHalImage_P1_Rsso[i]);
            MY_LOGD_IF( 1, "New: p1 full raw(%p); resized raw(%p)",
                        mParams->mStreamSet.mpHalImage_P1_Raw[i].get(), mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i].get());
            mvPass1Resource[i].setReuseFlag(MFALSE);
        }
    }
    if ( mParams->mStreamSet.mpAppImage_Yuv_In.get() )
        BuildStream(eStreamType_IMG_APP, mParams->mStreamSet.mpAppImage_Yuv_In);
    if ( mParams->mStreamSet.mpAppImage_Opaque_In.get() )
        BuildStream(eStreamType_IMG_APP, mParams->mStreamSet.mpAppImage_Opaque_In);
    if ( mParams->mStreamSet.mpAppImage_Opaque_Out.get() )
        BuildStream(eStreamType_IMG_APP, mParams->mStreamSet.mpAppImage_Opaque_Out);
    //
    if ( ! mCommonInfo->mJpegRotationEnable )
        BuildStream(eStreamType_IMG_HAL_POOL   , mParams->mStreamSet.mpHalImage_Jpeg_YUV);
    else
        BuildStream(eStreamType_IMG_HAL_RUNTIME   , mParams->mStreamSet.mpHalImage_Jpeg_YUV);
    BuildStream(eStreamType_IMG_HAL_RUNTIME, mParams->mStreamSet.mpHalImage_Thumbnail_YUV);
    //
    for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++ )
    {
        BuildStream(eStreamType_IMG_APP, mParams->mStreamSet.mvAppYuvImage[i]);
    }
    BuildStream(eStreamType_IMG_APP, mParams->mStreamSet.mpAppImage_Jpeg);
    BuildStream(eStreamType_IMG_APP, mParams->mStreamSet.mpAppImage_RAW16);
#undef BuildStream
    MY_LOGD("--");

    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_Nodes(sp<PipelineContext> pContext, MBOOL isReConfig)
{
    CAM_TRACE_CALL();
    for (int i = 0; i < mParams->mPipelineConfigParams.mbUseP1Node.size(); i++)
    {
        CHECK_ERROR( configContextLocked_P1Node(pContext, isReConfig, i) );
    }
    if( mParams->mPipelineConfigParams.mbUseP2Node )
        CHECK_ERROR( configContextLocked_P2SNode(pContext, isReConfig) );
    if( mParams->mPipelineConfigParams.mbUseP2CapNode )
        CHECK_ERROR( configContextLocked_P2CNode(pContext, isReConfig) );
    if( mParams->mPipelineConfigParams.mbUseFDNode )
        CHECK_ERROR( configContextLocked_FdNode(pContext, isReConfig) );
    if( mParams->mPipelineConfigParams.mbUseJpegNode )
        CHECK_ERROR( configContextLocked_JpegNode(pContext, isReConfig) );
    if( mParams->mPipelineConfigParams.mbUseRaw16Node )
        CHECK_ERROR( configContextLocked_Raw16Node(pContext, isReConfig) );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_Pipeline(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();
    NodeSet roots;
    {
        roots.add(eNODEID_P1Node);
    }
    NodeEdgeSet edges;
    {
        // in:p1 -> out:
        for (int i = 0; i < mParams->mPipelineConfigParams.mbUseP1Node.size(); i++)
        {
            if( mParams->mPipelineConfigParams.mbUseP1Node[i] )
            {
                NodeId_T id = eNODEID_P1Node;
                if (i == 1)
                {
                    id = eNODEID_P1Node_main2;
                }
                if ( mParams->mPipelineConfigParams.mbUseP2Node )
                    edges.addEdge(id, eNODEID_P2Node);
                if ( mParams->mPipelineConfigParams.mbUseP2CapNode )
                    edges.addEdge(id, eNODEID_P2Node_VSS);
                if ( mParams->mPipelineConfigParams.mbUseRaw16Node )
                    edges.addEdge(id, eNODEID_RAW16Out);
            }
        }
        // in:p2 -> out:
        if ( mParams->mPipelineConfigParams.mbUseP2Node )
        {
            if ( mParams->mPipelineConfigParams.mbUseFDNode )
                edges.addEdge(eNODEID_P2Node, eNODEID_FDNode);
            if ( mParams->mPipelineConfigParams.mbUseJpegNode )
                edges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);
        }
        // in:p2ts -> out:
        if ( mParams->mPipelineConfigParams.mbUseP2CapNode )
        {
            if ( mParams->mPipelineConfigParams.mbUseJpegNode )
                edges.addEdge(eNODEID_P2Node_VSS, eNODEID_JpegNode);
        }
    }
    //
    CHECK_ERROR(
            PipelineBuilder()
            .setRootNode(roots)
            .setNodeEdges(edges)
            .build(pContext)
            );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
#define add_stream_to_set(_set_, _IStreamInfo_)                                \
    do {                                                                       \
        if( _IStreamInfo_.get() ) { _set_.add(_IStreamInfo_->getStreamId()); } \
    } while(0)
//
#define setImageUsage( _IStreamInfo_, _usg_ )                                   \
    do {                                                                        \
        if( _IStreamInfo_.get() ) {                                             \
            builder.setImageStreamUsage( _IStreamInfo_->getStreamId(), _usg_ ); \
        }                                                                       \
    } while(0)
/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_P1Node(sp<PipelineContext> pContext, MBOOL isReConfig, MINT32 idx)
{
    typedef P1Node                  NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T nodeId = eNODEID_P1Node;
    if (idx == 1)
    {
        nodeId = eNODEID_P1Node_main2;
    }
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = mParams->mSensorId[idx];
        initParam.nodeId = nodeId;
        initParam.nodeName = "P1Node";
    }

    NodeT::ConfigParams cfgParam;
    {
        NodeT::SensorParams sensorParam(
        /*mode     : */mParams->mHwParams[idx]->mSensorMode,
        /*size     : */mParams->mHwParams[idx]->mSensorSize,
        /*fps      : */mParams->mHwParams[idx]->mSensorFps,
        /*pixelMode: */mParams->mHwParams[idx]->mPixelMode,
        /*vhdrMode : */mParams->mPipelineConfigParams.mVhdrMode
        );
        //
        cfgParam.pInAppMeta        = mParams->mStreamSet.mpAppMeta_Control;
        cfgParam.pInHalMeta        = mParams->mStreamSet.mpHalMeta_Control[idx];
        cfgParam.pOutAppMeta       = mParams->mStreamSet.mpAppMeta_DynamicP1[idx];
        cfgParam.pOutHalMeta       = mParams->mStreamSet.mpHalMeta_DynamicP1[idx];
        cfgParam.pOutImage_resizer = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx];

        cfgParam.pOutImage_lcso    = mParams->mStreamSet.mpHalImage_P1_Lcso[idx];
        cfgParam.pOutImage_rsso    = mParams->mStreamSet.mpHalImage_P1_Rsso[idx];
        if( mParams->mStreamSet.mpHalImage_P1_Raw[idx].get() )
            cfgParam.pvOutImage_full.push_back(mParams->mStreamSet.mpHalImage_P1_Raw[idx]);
        cfgParam.enableLCS          = mParams->mPipelineConfigParams.mbHasLcso[idx];
        cfgParam.enableRSS          = mParams->mPipelineConfigParams.mbHasRsso[idx];
        cfgParam.sensorParams        = sensorParam;
        cfgParam.pStreamPool_resizer = NULL;
        cfgParam.pStreamPool_full    = NULL;
        // cfgParam.pStreamPool_resizer = mpHalImage_P1_ResizerRaw.get() ?
        //    pContext->queryImageStreamPool(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00) : NULL;
        // cfgParam.pStreamPool_full = mpHalImage_P1_Raw.get() ?
        //    pContext->queryImageStreamPool(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00) : NULL;
        if( mParams->mStreamSet.mpAppImage_Yuv_In.get() )
            cfgParam.pInImage_yuv = mParams->mStreamSet.mpAppImage_Yuv_In;
        if (idx == 0)
        {
            if( mParams->mStreamSet.mpAppImage_Opaque_In.get() )
                cfgParam.pInImage_opaque = mParams->mStreamSet.mpAppImage_Opaque_In;
            if( mParams->mStreamSet.mpAppImage_Opaque_Out.get() )
                cfgParam.pOutImage_opaque = mParams->mStreamSet.mpAppImage_Opaque_Out;
        }
        cfgParam.sensorParams      = sensorParam;
        {
            int64_t ImgoId;
            int64_t RrzoId;
            int64_t LcsoId;
            int64_t RssoId;
            switch (idx)
            {
                case 0:
                    ImgoId = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00;
                    RrzoId = eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00;
                    LcsoId = eSTREAMID_IMAGE_PIPE_RAW_LCSO_00;
                    RssoId = eSTREAMID_IMAGE_PIPE_RAW_RSSO_00;
                    break;
                case 1:
                    ImgoId = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01;
                    RrzoId = eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01;
                    LcsoId = eSTREAMID_IMAGE_PIPE_RAW_LCSO_01;
                    RssoId = eSTREAMID_IMAGE_PIPE_RAW_RSSO_01;
                    break;
                default:
                    MY_LOGE("not support p1 node number large than 2");
                    break;
            }
            cfgParam.pStreamPool_resizer = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx].get() ?
                pContext->queryImageStreamPool(RrzoId) : NULL;
            cfgParam.pStreamPool_lcso = mParams->mStreamSet.mpHalImage_P1_Lcso[idx].get() ?
                pContext->queryImageStreamPool(LcsoId) : NULL;
            cfgParam.pStreamPool_rsso = mParams->mStreamSet.mpHalImage_P1_Rsso[idx].get() ?
                pContext->queryImageStreamPool(RssoId) : NULL;
            cfgParam.pStreamPool_full = mParams->mStreamSet.mpHalImage_P1_Raw[idx].get() ?
                pContext->queryImageStreamPool(ImgoId) : NULL;
        }
        MBOOL needLMV = (mParams->mPipelineConfigParams.mbHasRecording && !mParams->mPipelineConfigParams.mOperation_mode)
                        || mParams->mPipelineConfigParams.mNeedLMV;
        cfgParam.enableEIS = needLMV ? mpDeviceHelper->isFirstUsingDevice() : MFALSE;
        cfgParam.packedEisInfo = mParams->mPipelineConfigParams.mEisInfo.toPackedData();
        // cfgParam.eisMode = mParams->mEISMode;
        //
        if ( mParams->mPipelineConfigParams.mb4KRecording )
            cfgParam.receiveMode = NodeT::REV_MODE::REV_MODE_CONSERVATIVE;
        //
        if ( mParams->mPipelineConfigParams.mOperation_mode == IPipelineModelMgr::OperationMode::C_HIGH_SPEED_VIDEO_MODE )
        {
            MINT32 batch = mParams->mCHSvrParams.mAeTargetFpsMin/30;
            cfgParam.burstNum     = (mParams->mCHSvrParams.mDefaultBusrstNum==batch)?
                                    mParams->mCHSvrParams.mDefaultBusrstNum : batch;
        }
        //
        if ( mParams->mHwInfoHelper[idx]->getDualPDAFSupported(mParams->mHwParams[idx]->mSensorMode) ) {
            cfgParam.enableDualPD = MTRUE;
            if ( ! mpCamMgr->isMultiDevice() )
                cfgParam.disableFrontalBinning = MTRUE;
            MY_LOGD("PDAF supported for sensor mode:%d - enableDualPD:%d disableFrontalBinning:%d",
                    mParams->mHwParams[idx]->mSensorMode, cfgParam.enableDualPD, cfgParam.disableFrontalBinning);
        }
    }
    //
    sp<NodeActorT> pNode;

    if (isReConfig) {
        mCommonInfo->mpPipelineContext->queryNodeActor(nodeId, pNode);
        pNode->getNodeImpl()->config(cfgParam);
    }else{
        sp<NodeActorT> pOldNode;
        MBOOL bHasOldNode = (mpOldCtx.get() && OK == mpOldCtx->queryNodeActor(nodeId, pOldNode));
        if ( MTRUE == mvPass1Resource[idx].getReuseFlag() && bHasOldNode )
        {
            NodeT::InitParams oldInitParam;
            pOldNode->getInitParam(oldInitParam);
            NodeT::ConfigParams oldCfgParam;
            pOldNode->getConfigParam(oldCfgParam);
            //
            if ( compareParamsLocked_P1Node(initParam, oldInitParam, cfgParam, oldCfgParam) )
            {
                MERROR err = pContext->reuseNode(nodeId);
                MY_LOGD_IF( 1, "check p1 state Old[%d] New[%d]",
                            mpOldCtx->queryINodeActor(nodeId)->getStatus(),
                            pContext->queryINodeActor(nodeId)->getStatus() );
                return err;
            }
        }
        if(bHasOldNode)
            pOldNode->getNodeImpl()->uninit(); // must uninit old P1 before call new P1 config
        //
        pNode = new NodeActorT( NodeT::createInstance() );

    }
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_Control);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalMeta_Control[idx]);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Yuv_In);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Opaque_In);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_P1_Raw[idx]);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx]);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_P1_Lcso[idx]);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_P1_Rsso[idx]);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppMeta_DynamicP1[idx]);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalMeta_DynamicP1[idx]);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppImage_Opaque_Out);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mParams->mStreamSet.mpAppImage_Yuv_In        , eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mParams->mStreamSet.mpAppImage_Opaque_In     , eBUFFER_USAGE_SW_READ_OFTEN);
    //
    setImageUsage(mParams->mStreamSet.mpHalImage_P1_Raw[idx]        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx] , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpHalImage_P1_Lcso[idx]       , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpHalImage_P1_Rsso[idx]       , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpAppImage_Opaque_Out    , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
template <class INITPARAM_T, class CONFIGPARAM_T>
MERROR
PipelineDefaultImp::ConfigHandler::
compareParamsLocked_P1Node(
    INITPARAM_T const& initParam1,  INITPARAM_T const& initParam2,
    CONFIGPARAM_T const& cfgParam1, CONFIGPARAM_T const& cfgParam2
) const
{
    FUNC_START;
    if ( initParam1.openId != initParam2.openId ||
         initParam1.nodeId != initParam2.nodeId ||
         strcmp(initParam1.nodeName, initParam2.nodeName) )
        return MFALSE;
    //
    if ( cfgParam1.sensorParams.mode         != cfgParam2.sensorParams.mode ||
         cfgParam1.sensorParams.size         != cfgParam2.sensorParams.size ||
         cfgParam1.sensorParams.fps          != cfgParam2.sensorParams.fps ||
         cfgParam1.sensorParams.pixelMode    != cfgParam2.sensorParams.pixelMode ||
         cfgParam1.sensorParams.vhdrMode    != cfgParam2.sensorParams.vhdrMode )
        return MFALSE;
    //
    if ( ! cfgParam1.pInAppMeta.get()  || ! cfgParam2.pInAppMeta.get() ||
         ! cfgParam1.pOutAppMeta.get() || ! cfgParam2.pOutAppMeta.get() ||
         ! cfgParam1.pOutHalMeta.get() || ! cfgParam2.pOutHalMeta.get() ||
         cfgParam1.pInAppMeta->getStreamId()  != cfgParam2.pInAppMeta->getStreamId() ||
         cfgParam1.pOutAppMeta->getStreamId() != cfgParam2.pOutAppMeta->getStreamId() ||
         cfgParam1.pOutHalMeta->getStreamId() != cfgParam2.pOutHalMeta->getStreamId() )
        return MFALSE;
    //
    if ( ! cfgParam1.pOutImage_resizer.get() || ! cfgParam2.pOutImage_resizer.get() ||
        cfgParam1.pOutImage_resizer->getStreamId() != cfgParam2.pOutImage_resizer->getStreamId() )
        return MFALSE;
    //
    if ( cfgParam1.pOutImage_lcso.get() != cfgParam2.pOutImage_lcso.get()
        || cfgParam1.enableLCS != cfgParam2.enableLCS){
        return MFALSE;
    }
    //
    if ( cfgParam1.pOutImage_rsso.get() != cfgParam2.pOutImage_rsso.get()
        || cfgParam1.enableRSS != cfgParam2.enableRSS){
        return MFALSE;
    }
    //
    if ( cfgParam1.pvOutImage_full.size() != cfgParam2.pvOutImage_full.size() )
        return MFALSE;
    //
    for ( size_t i=0; i<cfgParam1.pvOutImage_full.size(); i++ ) {
        MBOOL bMatch = MFALSE;
        for ( size_t j=0; j<cfgParam2.pvOutImage_full.size(); j++ ) {
            if ( cfgParam1.pvOutImage_full.itemAt(i)->getStreamId() == cfgParam2.pvOutImage_full.itemAt(i)->getStreamId() )
            {
                bMatch = MTRUE;
                break;
            }
        }
        if ( !bMatch )
            return MFALSE;
    }
    //
    FUNC_END;
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_P2SNode(sp<PipelineContext> pContext, MBOOL isReConfig)
{
    typedef P2Node                  NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_P2Node;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = mParams->mSensorId[0];
        initParam.nodeId = nodeId;
        initParam.nodeName = "P2StreamNode";
        for (int i = 1; i < mParams->mSensorId.size(); i++)
        {
            initParam.subOpenIdList.push_back(mParams->mSensorId[i]);
        }
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta    = mParams->mStreamSet.mpAppMeta_Control;
        cfgParam.pInAppRetMeta = mParams->mStreamSet.mpAppMeta_DynamicP1[0];
        cfgParam.pInHalMeta    = mParams->mStreamSet.mpHalMeta_DynamicP1[0];
        cfgParam.pOutAppMeta   = mParams->mStreamSet.mpAppMeta_DynamicP2;
        cfgParam.pOutHalMeta   = mParams->mStreamSet.mpHalMeta_DynamicP2;
        //
        if( mParams->mStreamSet.mpHalImage_P1_Raw[0].get() )
            cfgParam.pvInFullRaw.push_back(mParams->mStreamSet.mpHalImage_P1_Raw[0]);
        //
        cfgParam.pInResizedRaw = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[0];
        //
        cfgParam.pInLcsoRaw = mParams->mStreamSet.mpHalImage_P1_Lcso[0];
        //
        cfgParam.pInRssoRaw = mParams->mStreamSet.mpHalImage_P1_Rsso[0];
        //
        if (mParams->mHwParams.size() > 1)
        {
            cfgParam.pInAppRetMeta_Sub = mParams->mStreamSet.mpAppMeta_DynamicP1[1];
            cfgParam.pInHalMeta_Sub    = mParams->mStreamSet.mpHalMeta_DynamicP1[1];
            if( mParams->mStreamSet.mpHalImage_P1_Raw[1].get() )
                cfgParam.pvInFullRaw_Sub.push_back(mParams->mStreamSet.mpHalImage_P1_Raw[1]);
            //
            cfgParam.pInResizedRaw_Sub = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[1];
            //
            cfgParam.pInLcsoRaw_Sub = mParams->mStreamSet.mpHalImage_P1_Lcso[1];
            //
            cfgParam.pInRssoRaw_Sub = mParams->mStreamSet.mpHalImage_P1_Rsso[1];
        }
        //
        if( mParams->mStreamSet.mpAppImage_Yuv_In.get() )
            cfgParam.pInYuvImage = mParams->mStreamSet.mpAppImage_Yuv_In;
        //
        if( mParams->mStreamSet.mpAppImage_Opaque_In.get() )
            cfgParam.pvInOpaque.push_back(mParams->mStreamSet.mpAppImage_Opaque_In);
        //
        if( mParams->mStreamSet.mpAppImage_Opaque_Out.get() )
            cfgParam.pvInOpaque.push_back(mParams->mStreamSet.mpAppImage_Opaque_Out);
        //
        for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++)
            cfgParam.vOutImage.push_back(mParams->mStreamSet.mvAppYuvImage[i]);
        //
        /*if( mParams->mStreamSet.mpHalImage_Jpeg_YUV.get() )
            cfgParam.vOutImage.push_back(mParams->mStreamSet.mpHalImage_Jpeg_YUV);
        if( mParams->mStreamSet.mpHalImage_Thumbnail_YUV.get() )
            cfgParam.vOutImage.push_back(mParams->mStreamSet.mpHalImage_Thumbnail_YUV);*/
        //
        cfgParam.pOutFDImage = mParams->mStreamSet.mpHalImage_FD_YUV;
        //
        if ( mParams->mPipelineConfigParams.mOperation_mode == IPipelineModelMgr::OperationMode::C_HIGH_SPEED_VIDEO_MODE )
        {
            MINT32 batch = mParams->mCHSvrParams.mAeTargetFpsMin/30;
            cfgParam.burstNum     = (mParams->mCHSvrParams.mDefaultBusrstNum==batch)?
                                    mParams->mCHSvrParams.mDefaultBusrstNum : batch;
        }
    }
    P2Common::UsageHint p2Usage = prepareP2Usage(P2Node::PASS2_STREAM);
    cfgParam.mUsageHint = p2Usage;

    //
    sp<NodeActorT> pNode;
    if (isReConfig) {
        mCommonInfo->mpPipelineContext->queryNodeActor(nodeId, pNode);
        pNode->getNodeImpl()->config(cfgParam);
    } else {
        pNode = new NodeActorT( NodeT::createInstance(P2Node::PASS2_STREAM, p2Usage) );
    }
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_Control);
    for (int i = 0; i < mParams->mHwParams.size(); i++)
    {
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_DynamicP1[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalMeta_DynamicP1[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_Raw[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_Lcso[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_Rsso[i]);
    }
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Yuv_In);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Opaque_In);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Opaque_Out);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppMeta_DynamicP2);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalMeta_DynamicP2);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_Jpeg_YUV);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_Thumbnail_YUV);
    //
    for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++)
        add_stream_to_set(outStreamSet, mParams->mStreamSet.mvAppYuvImage[i]);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_FD_YUV);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mParams->mStreamSet.mpAppImage_Yuv_In        , eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_SW_READ_OFTEN);
    setImageUsage(mParams->mStreamSet.mpAppImage_Opaque_In     , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mParams->mStreamSet.mpAppImage_Opaque_Out    , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    for (int i = 0; i < mParams->mPipelineConfigParams.mbUseP1Node.size(); i++)
    {
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_Raw[i]        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i] , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_Lcso[i]       , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_Rsso[i]       , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    }
    //
    for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++)
        setImageUsage(mParams->mStreamSet.mvAppYuvImage[i], eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    setImageUsage(mParams->mStreamSet.mpHalImage_Jpeg_YUV      , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpHalImage_Thumbnail_YUV , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpHalImage_FD_YUV        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_P2CNode(sp<PipelineContext> pContext, MBOOL isReConfig)
{
#if 0

    typedef P2CaptureNode           NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_P2Node_VSS;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = mParams->mSensorId[0];
        initParam.nodeId = nodeId;
        initParam.nodeName = "P2CaptureNode";
        for (int i = 1; i < mParams->mSensorId.size(); i++)
        {
            initParam.subOpenIdList.push_back(mParams->mSensorId[i]);
        }
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta    = mParams->mStreamSet.mpAppMeta_Control;
        cfgParam.pInAppRetMeta = mParams->mStreamSet.mpAppMeta_DynamicP1[0];
        cfgParam.pInHalMeta    = mParams->mStreamSet.mpHalMeta_DynamicP1[0];
        cfgParam.pOutAppMeta   = mParams->mStreamSet.mpAppMeta_DynamicP2Capture;
        cfgParam.pOutHalMeta   = mParams->mStreamSet.mpHalMeta_DynamicP2Capture;
        //
        if( mParams->mStreamSet.mpHalImage_P1_Raw[0].get() )
            cfgParam.pInFullRaw = mParams->mStreamSet.mpHalImage_P1_Raw[0];
        //
        cfgParam.pInResizedRaw = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[0];
        //
        cfgParam.pInLcsoRaw = mParams->mStreamSet.mpHalImage_P1_Lcso[0];
        //
        #if 1 // capture node not support main2 yet
        if (mParams->mHwParams.size() > 1)
        {
            cfgParam.pInAppRetMeta2 = mParams->mStreamSet.mpAppMeta_DynamicP1[1];
            cfgParam.pInHalMeta2    = mParams->mStreamSet.mpHalMeta_DynamicP1[1];
            if( mParams->mStreamSet.mpHalImage_P1_Raw[1].get() )
                cfgParam.pInFullRaw2 = mParams->mStreamSet.mpHalImage_P1_Raw[1];
            //
            cfgParam.pInResizedRaw2 = mParams->mStreamSet.mpHalImage_P1_ResizerRaw[1];
            //
            cfgParam.pInLcsoRaw2 = mParams->mStreamSet.mpHalImage_P1_Lcso[1];
        }
        #endif
        //
        if( mParams->mStreamSet.mpAppImage_Yuv_In.get() )
            cfgParam.pInFullYuv = mParams->mStreamSet.mpAppImage_Yuv_In;
        //
        if( mParams->mStreamSet.mpAppImage_Opaque_In.get() )
            cfgParam.vpInOpaqueRaws.push_back(mParams->mStreamSet.mpAppImage_Opaque_In);
        //
        if( mParams->mStreamSet.mpAppImage_Opaque_Out.get() )
            cfgParam.vpInOpaqueRaws.push_back(mParams->mStreamSet.mpAppImage_Opaque_Out);
        //
        for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++)
            cfgParam.vpOutImages.push_back(mParams->mStreamSet.mvAppYuvImage[i]);
        //
        if( mParams->mStreamSet.mpHalImage_Jpeg_YUV.get() )
            cfgParam.pOutJpegYuv = mParams->mStreamSet.mpHalImage_Jpeg_YUV;
        if( mParams->mStreamSet.mpHalImage_Thumbnail_YUV.get() )
            cfgParam.pOutThumbnailYuv = mParams->mStreamSet.mpHalImage_Thumbnail_YUV;
    }

    //
    sp<NodeActorT> pNode;
    if (isReConfig) {
        mCommonInfo->mpPipelineContext->queryNodeActor(nodeId, pNode);
        pNode->getNodeImpl()->config(cfgParam);
    } else {
        pNode = new NodeActorT( NodeT::createInstance() );
    }
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_Control);
    for (int i = 0; i < mParams->mHwParams.size(); i++)
    {
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_DynamicP1[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalMeta_DynamicP1[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_Raw[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i]);
        add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_Lcso[i]);
    }
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Yuv_In);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Opaque_In);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppImage_Opaque_Out);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppMeta_DynamicP2Capture);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalMeta_DynamicP2Capture);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_Jpeg_YUV);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpHalImage_Thumbnail_YUV);
    //
    for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++)
        add_stream_to_set(outStreamSet, mParams->mStreamSet.mvAppYuvImage[i]);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mParams->mStreamSet.mpAppImage_Yuv_In        , eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_SW_READ_OFTEN);
    setImageUsage(mParams->mStreamSet.mpAppImage_Opaque_In     , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mParams->mStreamSet.mpAppImage_Opaque_Out    , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    for (int i = 0; i < mParams->mPipelineConfigParams.mbUseP1Node.size(); i++)
    {
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_Raw[i]        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_ResizerRaw[i] , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(mParams->mStreamSet.mpHalImage_P1_Lcso[i]       , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    }
    //
    for (size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++)
        setImageUsage(mParams->mStreamSet.mvAppYuvImage[i], eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    setImageUsage(mParams->mStreamSet.mpHalImage_Jpeg_YUV      , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mParams->mStreamSet.mpHalImage_Thumbnail_YUV , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
#else
    return OK;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_FdNode(sp<PipelineContext> pContext, MBOOL isReConfig)
{
    typedef FdNode                  NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_FDNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = mCommonInfo->mOpenId;
        initParam.nodeId = nodeId;
        initParam.nodeName = "FDNode";
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta    = mParams->mStreamSet.mpAppMeta_Control;
        cfgParam.pOutAppMeta   = mParams->mStreamSet.mpAppMeta_DynamicFD;
        cfgParam.vInImage      = mParams->mStreamSet.mpHalImage_FD_YUV;
    }
    //
    sp<NodeActorT> pNode;
    if (isReConfig) {
        mCommonInfo->mpPipelineContext->queryNodeActor(nodeId, pNode);
        pNode->getNodeImpl()->config(cfgParam);
    } else {
        pNode = new NodeActorT( NodeT::createInstance() );
    }
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_Control);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_FD_YUV);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppMeta_DynamicFD);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mParams->mStreamSet.mpHalImage_FD_YUV , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_JpegNode(sp<PipelineContext> pContext, MBOOL isReConfig)
{
    typedef JpegNode                NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_JpegNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = mCommonInfo->mOpenId;
        initParam.nodeId = nodeId;
        initParam.nodeName = "JpegNode";
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta        = mParams->mStreamSet.mpAppMeta_Control;
        cfgParam.pInHalMeta        = mParams->mStreamSet.mpHalMeta_DynamicP2Capture;
        cfgParam.pOutAppMeta       = mParams->mStreamSet.mpAppMeta_DynamicJpeg;
        cfgParam.pInYuv_Main       = mParams->mStreamSet.mpHalImage_Jpeg_YUV;
        cfgParam.pInYuv_Thumbnail  = mParams->mStreamSet.mpHalImage_Thumbnail_YUV;
        cfgParam.pOutJpeg          = mParams->mStreamSet.mpAppImage_Jpeg;
    }
    //
    sp<NodeActorT> pNode;
    if (isReConfig) {
        mCommonInfo->mpPipelineContext->queryNodeActor(nodeId, pNode);
        pNode->getNodeImpl()->config(cfgParam);
    } else {
        pNode = new NodeActorT( NodeT::createInstance() );
    }
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpAppMeta_Control);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalMeta_DynamicP2Capture);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_Jpeg_YUV);
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_Thumbnail_YUV);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppMeta_DynamicJpeg);
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppImage_Jpeg);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mParams->mStreamSet.mpHalImage_Jpeg_YUV, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mParams->mStreamSet.mpHalImage_Thumbnail_YUV, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mParams->mStreamSet.mpAppImage_Jpeg, eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configContextLocked_Raw16Node(sp<PipelineContext> pContext, MBOOL isReConfig)
{
    typedef RAW16Node               NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_RAW16Out;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = mCommonInfo->mOpenId;
        initParam.nodeId = nodeId;
        initParam.nodeName = "Raw16Node";
    }
    NodeT::ConfigParams cfgParam;
    {
    }
    //
    sp<NodeActorT> pNode;
    if (isReConfig) {
        mCommonInfo->mpPipelineContext->queryNodeActor(nodeId, pNode);
        pNode->getNodeImpl()->config(cfgParam);
    } else {
        pNode = new NodeActorT( NodeT::createInstance() );
    }
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mParams->mStreamSet.mpHalImage_P1_Raw[0]);
    //
    add_stream_to_set(outStreamSet, mParams->mStreamSet.mpAppImage_RAW16);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mParams->mStreamSet.mpHalImage_P1_Raw[0], eBUFFER_USAGE_SW_READ_OFTEN);
    setImageUsage(mParams->mStreamSet.mpAppImage_RAW16, eBUFFER_USAGE_SW_WRITE_OFTEN);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;

}
/******************************************************************************
 *
 ******************************************************************************/
#undef add_stream_to_set
#undef setImageUsage

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configScenarioCtrlLocked()
{
    CAM_TRACE_NAME("PipelineDefaultImp::BWC");
    sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(mCommonInfo->mOpenId);
    IScenarioControl::ControlParam param;

    // invoke setScenarioCtrl in component list
#if 1
    // basic component
    param.scenario   =
        (mParams->mPipelineConfigParams.mOperation_mode!=0) ?                  IScenarioControl::Scenario_HighSpeedVideo :
        (mParams->mPipelineConfigParams.mbHasRecording && mParams->mPipelineConfigParams.mbHasJpeg) ? IScenarioControl::Scenario_VSS :
        (mParams->mPipelineConfigParams.mbHasRecording) ?                      IScenarioControl::Scenario_VideoRecord:
        (mParams->mPipelineConfigParams.mbHasJpeg) ?                           IScenarioControl::Scenario_Capture :
        IScenarioControl::Scenario_NormalPreivew;
    param.sensorSize = mParams->mHwParams[0]->mSensorSize;
    param.sensorFps  = mParams->mHwParams[0]->mSensorFps;

    // streamingpipe component -> eis
    if( mpDeviceHelper->isFirstUsingDevice() )
    {
        MBOOL advEIS = EIS_MODE_IS_EIS_22_ENABLED(mParams->mPipelineConfigParams.mEisInfo.mode) ||
                       EIS_MODE_IS_EIS_25_ENABLED(mParams->mPipelineConfigParams.mEisInfo.mode) ||
                       EIS_MODE_IS_EIS_30_ENABLED(mParams->mPipelineConfigParams.mEisInfo.mode);
        if( advEIS && mParams->mPipelineConfigParams.mb4KRecording )
        {
            FEATURE_CFG_ENABLE_MASK(param.featureFlag, IScenarioControl::FEATURE_ADV_EIS_4K);
        }
        else if( advEIS || mParams->mPipelineConfigParams.mb4KRecording )
        {
            FEATURE_CFG_ENABLE_MASK(param.featureFlag, IScenarioControl::FEATURE_ADV_EIS);
        }
    }

    // streamingpipe component -> vhdr
    if( mParams->mPipelineConfigParams.mVhdrMode == SENSOR_VHDR_MODE_IVHDR )
        FEATURE_CFG_ENABLE_MASK(param.featureFlag, IScenarioControl::FEATURE_IVHDR);
    if( mParams->mPipelineConfigParams.mVhdrMode == SENSOR_VHDR_MODE_MVHDR )
        FEATURE_CFG_ENABLE_MASK(param.featureFlag, IScenarioControl::FEATURE_MVHDR);
    if( mParams->mPipelineConfigParams.mVhdrMode == SENSOR_VHDR_MODE_ZVHDR )
        FEATURE_CFG_ENABLE_MASK(param.featureFlag, IScenarioControl::FEATURE_ZVHDR);
#endif

    CHECK_ERROR(pScenarioCtrl->exitScenario());
    CHECK_ERROR(pScenarioCtrl->enterScenario(param));
    CHECK_ERROR(mCommonInfo->mpPipelineContext->setScenarioControl(pScenarioCtrl));
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
checkPermission()
{
    if ( ! mpCamMgr->getPermission() )
    {
        MY_LOGD("cannot config pipeline ... Permission denied");
        return PERMISSION_DENIED;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
configRequestRulesLocked(MINT32 idx)
{
    struct categorize_yuv_stream
    {
        MVOID   operator() (
                    sp<const IImageStreamInfo> const pInfo,
                    MSize const& thres,
                    StreamSet& vLarge, StreamSet& vSmall
                )
                {
                    if( ! pInfo.get() ) return;
                    //
                    MSize const size = pInfo->getImgSize();
                    if ( size.w > thres.w || size.h > thres.h )
                        vLarge.add(pInfo->getStreamId());
                    else
                        vSmall.add(pInfo->getStreamId());
                }
    };
    //
    mParams->mStreamSet.mvYuvStreams_Fullraw.clear();
    mParams->mStreamSet.mvYuvStreams_Resizedraw.clear();
    //
    if( ! mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx].get() && ! mParams->mStreamSet.mpHalImage_P1_Raw[idx].get() &&
        ! mParams->mStreamSet.mpAppImage_Opaque_In.get() && ! mParams->mStreamSet.mpAppImage_Yuv_In.get() ) {
        MY_LOGE("no available raw stream");
        return UNKNOWN_ERROR;
    }
    //
    MSize const threshold =
       mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx].get() ? mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx]->getImgSize() : MSize(0,0);
    //
    StreamSet& vLarge =
        mParams->mStreamSet.mpHalImage_P1_Raw[idx].get() || mParams->mStreamSet.mpAppImage_Opaque_Out.get() ? mParams->mStreamSet.mvYuvStreams_Fullraw : mParams->mStreamSet.mvYuvStreams_Resizedraw;
    StreamSet& vSmall =
        mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx].get() ? mParams->mStreamSet.mvYuvStreams_Resizedraw : mParams->mStreamSet.mvYuvStreams_Fullraw;
    //
    //bool haveFullraw = mpHalImage_P1_Raw.get();
    //
    for( size_t i = 0; i < mParams->mStreamSet.mvAppYuvImage.size(); i++ ) {
        sp<const IImageStreamInfo> pStreamInfo = mParams->mStreamSet.mvAppYuvImage.valueAt(i);
        if( (pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER) &&
             mParams->mPipelineConfigParams.mb4KRecording )
        {
            // 4K recording only
            // Avoid to use IMGO & RRZO in the same time
            // for reaching 30 fps performance
            categorize_yuv_stream()(pStreamInfo, threshold, vSmall, vSmall);
        }
        else
        {
            categorize_yuv_stream()(pStreamInfo, threshold, vLarge, vSmall);
        }
    }
    categorize_yuv_stream()(mParams->mStreamSet.mpHalImage_FD_YUV, threshold, vLarge, vSmall);
    //
    categorize_yuv_stream()(mParams->mStreamSet.mpHalImage_Jpeg_YUV, MSize(0,0), vLarge, vSmall);
    categorize_yuv_stream()(mParams->mStreamSet.mpHalImage_Thumbnail_YUV, MSize(0,0), vLarge, vSmall);
    //
#if 1
    // dump raw stream dispatch rule
    StreamId_T fullStream = mParams->mStreamSet.mpHalImage_P1_Raw[idx].get() ? mParams->mStreamSet.mpHalImage_P1_Raw[idx]->getStreamId() :
                            mParams->mStreamSet.mpAppImage_Opaque_Out.get() ? mParams->mStreamSet.mpAppImage_Opaque_Out->getStreamId() :
                            -1;
    for( size_t i = 0; i < mParams->mStreamSet.mvYuvStreams_Fullraw.size(); i++ ) {
        MY_LOGD("full raw streamId:%#" PRIx64 " -> yuv streamId:%#" PRIx64,
                fullStream, mParams->mStreamSet.mvYuvStreams_Fullraw[i]);
    }
    for( size_t i = 0; i < mParams->mStreamSet.mvYuvStreams_Resizedraw.size(); i++ ) {
        MY_LOGD("resized raw streamId:%#" PRIx64 " -> yuv streamId:%#" PRIx64,
                mParams->mStreamSet.mpHalImage_P1_ResizerRaw[idx]->getStreamId(), mParams->mStreamSet.mvYuvStreams_Resizedraw[i]);
    }
#endif
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
PipelineDefaultImp::ConfigHandler::
createRawImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    size_t const        stride,
    MINT32              p1Index
) const
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
    //
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_BAYER8: // LCSO
        case eImgFmt_STA_BYTE:
        case eImgFmt_STA_2BYTE: // LCSO with LCE3.0
            addBufPlane(bufPlanes , imgSize.h, stride);
            break;
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
        case eImgFmt_UFO_FG_BAYER8:
        case eImgFmt_UFO_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER12:
        case eImgFmt_UFO_FG_BAYER14:
            {
                size_t ufoStride[3] = {0};
                addBufPlane(bufPlanes , imgSize.h, stride);
                mParams->mHwInfoHelper[p1Index]->queryUFOStride(imgFormat, imgSize, ufoStride);
                addBufPlane(bufPlanes , imgSize.h, ufoStride[1]);
                addBufPlane(bufPlanes , imgSize.h, ufoStride[2]);
                break;
            }
        default:
            MY_LOGE("format not support yet %d", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}


/*******************************************************************************
 *
 ********************************************************************************/
PipelineDefaultImp::ConfigHandler::
ConfigHandler(
    std::shared_ptr<CommonInfo> pCommonInfo,
    std::shared_ptr<MyProcessedParams> pParams
)
    : mCommonInfo(pCommonInfo)
    , mParams(pParams)
    , mpCamMgr(CamManager::getInstance())
    , mpDeviceHelper(new CamManager::UsingDeviceHelper(mCommonInfo->mOpenId))
{
    FUNC_START;
    FUNC_END;
}


/*******************************************************************************
 *
 ********************************************************************************/
PipelineDefaultImp::ConfigHandler::
~ConfigHandler()
{
    FUNC_START;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::ConfigHandler::
reconfigPipelineLocked(parsedAppRequest const& request)
{
#if 0
    CAM_TRACE_NAME(__FUNCTION__);
    //
    IMetadata* pAppMetaControl = request.pRequest->vIMetaBuffers[0]->tryReadLock(__FUNCTION__);
    if( ! pAppMetaControl ) {
        MY_LOGE("cannot get control meta");
        return UNKNOWN_ERROR;
    }

    if ( mParams->mPipelineConfigParams.mOperation_mode )
    {
        // smvr
        IMetadata::IEntry const& entry = pAppMetaControl->entryFor(MTK_CONTROL_AE_TARGET_FPS_RANGE);
        MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
        MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
        if ( mParams->mCHSvrParams.mAeTargetFpsMin != i4MinFps )
        {
            MY_LOGI("do re-configure pipeline. min fps: %d -> %d ", mParams->mCHSvrParams.mAeTargetFpsMin, i4MinFps);
            mParams->mCHSvrParams.mAeTargetFpsMin_Req = i4MinFps;
            mParams->mCHSvrParams.mResetAeTargetFps_Req = MTRUE;
            CAM_TRACE_BEGIN("reconfigure: waitUntilDrained");
            mCommonInfo->mpPipelineContext->waitUntilDrained();
            CAM_TRACE_END();
            MY_LOGD("wait drained done");
            //
            mParams->update(mParams->mConfigParams, MFALSE);
            CHECK_ERROR(configScenarioCtrlLocked());
            CHECK_ERROR(setupHalStreamsLocked(mParams->mConfigParams));
            CHECK_ERROR(configContextLocked_Streams(mCommonInfo->mpPipelineContext));
            CHECK_ERROR(configContextLocked_Nodes(mCommonInfo->mpPipelineContext, MTRUE));
            MY_LOGI("end re-configure pipeline");
        }
    }
    //
    if( pAppMetaControl )
        request.pRequest->vIMetaBuffers[0]->unlock(
                __FUNCTION__, pAppMetaControl
            );
#endif
    return OK;
}
