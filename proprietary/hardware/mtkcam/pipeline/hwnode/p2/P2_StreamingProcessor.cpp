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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2_StreamingProcessor.h"
#include "P2_Util.h"

#define P2_STREAMING_THREAD_NAME "p2_streaming"
#define VAR_STREAMING_PAYLOAD "p2_streaming_payload"

#define IDLE_WAIT_TIME_MS 66

using NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe;
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;
using namespace NSCam::NR3D;

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    StreamingProcessor
#define P2_TRACE        TRACE_STREAMING_PROCESSOR
#include "P2_LogHeader.h"

StreamingProcessor::StreamingProcessor()
    : Processor(P2_STREAMING_THREAD_NAME)
    , mFeaturePipe(NULL)
    , mHal3A(NULL)
    , mConfigVencStream(MFALSE)
    , mTuningSize(0)
    , m3dnrDebugLevel(0)
{
    MY_LOG_FUNC_ENTER();
    this->setIdleWaitMS(IDLE_WAIT_TIME_MS);
    MY_LOG_FUNC_EXIT();
}

StreamingProcessor::~StreamingProcessor()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    this->uninit();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL StreamingProcessor::onInit(const P2InitParam &param)
{
    MY_LOG_S_FUNC_ENTER(param.mInfo);
    CAM_TRACE_NAME("P2_Streaming:init()");

    MBOOL ret = MFALSE;

    mP2Info = param.mInfo;
    mConfigParam = param.mConfig;
    if( mP2Info == NULL )
    {
        MY_S_LOGE(mP2Info, "Invalid P2Info = NULL");
    }
    else
    {
        ret = initFeaturePipe(mConfigParam) && init3A();
        if( ret )
        {
            mTuningSize = mFeaturePipe->getRegTableSize();
            init3DNR();
        }
        else
        {
            uninitFeaturePipe();
            uninit3A();
        }
    }

    MY_LOG_S_FUNC_EXIT(param.mInfo);
    return ret;
}

MVOID StreamingProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Streaming:uninit()");
    uninit3DNR();
    uninitFeaturePipe();
    uninit3A();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID StreamingProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Streaming:threadStart()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID StreamingProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Streaming:threadStop()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL StreamingProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MTRUE;
    CAM_TRACE_NAME("P2_Streaming:config()");
    if( needReConfig(mConfigParam, param) )
    {
        android::Mutex::Autolock _lock(mPayloadMutex);
        if( mPayloadList.size() )
        {
            MY_S_LOGE(mP2Info, "Config called before p2 is empty, size=%zu", mPayloadList.size());
            ret = MFALSE;
        }
        else
        {
            uninitFeaturePipe();
            ret = initFeaturePipe(param);
        }
    }
    if( ret )
    {
        mConfigParam = param;
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL StreamingProcessor::onEnque(const sp<P2Request> &request)
{
    TRACE_S_FUNC_ENTER(request);
    CAM_TRACE_NAME("P2_Streaming:enque()");
    MBOOL ret = MFALSE;

    if( request != NULL && request->hasInput() && request->hasOutput() )
    {
        if( request->hasInput() && request->hasOutput() )
        {
            sp<Payload> payload = new Payload(this, request);

            if( payload == NULL )
            {
                MY_S_LOGW(request, "cannot allocate Payload");
            }
            else
            {
                payload->mIO = P2Util::extractSimpleIO(request);
                payload->mIO.setUseLMV(isEIS12() ? MTRUE : MFALSE);

                if( payload->mIO.hasInput() &&
                    payload->mIO.hasOutput() )
                {
                    P2MetaSet metaSet;
                    metaSet = request->getMetaSet();
                    // Begin 3A related feature plug-in section
                    prepare3DNR(payload, metaSet, request->getLogger());
                    // End 3A related feature plug-in section
                    process3A(payload, metaSet);
                    prepareFeatureParam(payload);
                    payload->mRequest->releaseResource(P2Request::RES_IN_IMG);
                    ret = processP2(payload);
                }
            }
        }
        else
        {
            processTSQ(request);
        }
    }

    TRACE_S_FUNC_EXIT(request);
    return ret;
}

MVOID StreamingProcessor::onNotifyFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Streaming:notifyFlush()");
    if( mFeaturePipe )
    {
        mFeaturePipe->flush();
    }
    updateRsso(NULL);
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID StreamingProcessor::onWaitFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Streaming:waitFlush()");
    waitFeaturePipeDone();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID StreamingProcessor::onIdle()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    // release rsso & last frame if no more request
    updateRsso(NULL);
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

IStreamingFeaturePipe::UsageHint StreamingProcessor::getFeatureUsageHint(const P2ConfigParam &config)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    IStreamingFeaturePipe::UsageHint pipeUsage;
    switch( config.mP2Type )
    {
    case P2_PHOTO:
    case P2_PREVIEW:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_FEATURE;
        break;
    case P2_CAPTURE:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH;
        break;
    case P2_TIMESHARE_CAPTURE:
    #if NEW_CODE
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH_TIME_SHARING;
    #else
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH_TIME_SAHRING;
    #endif // NEW_CODE
        break;
    case P2_VIDEO:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_FULL;
        break;
    case P2_UNKNOWN:
    default:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_FULL;
        break;
    }

    pipeUsage.mStreamingSize = config.mUsageHint.mStreamingSize;
    if( pipeUsage.mStreamingSize.w == 0 || pipeUsage.mStreamingSize.h == 0 )
    {
        MY_S_LOGW(mP2Info, "no size in UsageHint");
    }
    // Set EIS Info
    pipeUsage.mEISInfo = config.mUsageHint.mEisInfo;
    pipeUsage.m3DNRMode = config.mUsageHint.m3DNRMode;
    pipeUsage.mUseTSQ = config.mUsageHint.mUseTSQ;

    TRACE_S_FUNC_EXIT(mP2Info);
    return pipeUsage;
}

MBOOL StreamingProcessor::needReConfig(const P2ConfigParam &oldConfig, const P2ConfigParam &newConfig)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    const P2UsageHint &oldHint = oldConfig.mUsageHint;
    const P2UsageHint &newHint = newConfig.mUsageHint;
    if( (newHint.mEisInfo.mode != oldHint.mEisInfo.mode) ||
        (newHint.mEisInfo.factor != oldHint.mEisInfo.factor) ||
        (newHint.mStreamingSize != oldHint.mStreamingSize) )
    {
        ret = MTRUE;
    }
    MY_S_LOGI(mP2Info, "re-config=%d, EISMode(%d=>%d), EISFactor(%d=>%d), Size(%dx%d=>%dx%d)", ret, oldHint.mEisInfo.mode, newHint.mEisInfo.mode, oldHint.mEisInfo.factor, newHint.mEisInfo.factor, oldHint.mStreamingSize.w, oldHint.mStreamingSize.h, newHint.mStreamingSize.w, newHint.mStreamingSize.h);
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL StreamingProcessor::initFeaturePipe(const P2ConfigParam &config)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;

    mPipeUsageHint = getFeatureUsageHint(config);
    CAM_TRACE_BEGIN("P2_Streaming:FeaturePipe create");
    mFeaturePipe = IStreamingFeaturePipe::createInstance(mP2Info->mSensorID, mPipeUsageHint);
    CAM_TRACE_END();
    if( mFeaturePipe == NULL )
    {
        MY_S_LOGE(mP2Info, "OOM: cannot create FeaturePipe");
    }
    else
    {
        CAM_TRACE_BEGIN("P2_Streaming:FeaturePipe init");
        ret = mFeaturePipe->init(getName());
        CAM_TRACE_END();
        if( !ret )
        {
            MY_S_LOGE(mP2Info, "FeaturePipe init failed");
        }
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MVOID StreamingProcessor::uninitFeaturePipe()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    if( mFeaturePipe )
    {
        if( mConfigVencStream )
        {
            mFeaturePipe->sendCommand(ESDCmd_RELEASE_VENC_DIRLK);
        }
        mFeaturePipe->uninit(getName());
        mFeaturePipe->destroyInstance();
        mFeaturePipe = NULL;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

MBOOL StreamingProcessor::init3A()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MTRUE;
    CAM_TRACE_BEGIN("P2_Streaming:3A create");
    mHal3A = MAKE_Hal3A(mP2Info->mSensorID, getName());
    CAM_TRACE_END();
    if( mHal3A == NULL )
    {
        MY_S_LOGE(mP2Info, "OOM: cannot create Hal3A");
        ret = MFALSE;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MVOID StreamingProcessor::uninit3A()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    if( mHal3A != NULL )
    {
        mHal3A->destroyInstance(getName());
        mHal3A = NULL;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

MVOID StreamingProcessor::process3A(const sp<Payload> &payload, P2MetaSet &metaSet)
{
    TRACE_S_FUNC_ENTER(payload);
    CAM_TRACE_NAME("P2_Streaming:process3A()");

    sp<P2Request> &request = payload->mRequest;
    const P2Util::SimpleIO &io = payload->mIO;
    TuningParam &tuning = payload->mTuning;

    P2Util::process3A(io, mTuningSize, mHal3A, tuning, metaSet, request->getLogger());

    P2Util::updateDebugExif(metaSet.mInHal, metaSet.mOutHal, request->getLogger());
    P2Util::updateExtraMeta(request->getExtraData(), metaSet.mOutHal, request->getLogger());

    prepareEISOutMeta(metaSet);
    request->updateMetaSet(metaSet);
    TRACE_S_FUNC_EXIT(payload);
}

MVOID StreamingProcessor::prepareFeatureParam(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);

    const sp<P2Request> &request = payload->mRequest;
    const P2Util::SimpleIO &io = payload->mIO;
    const TuningParam &tuning = payload->mTuning;
    FeaturePipeParam &featureParam = payload->mFeatureParam;

    // Begin feature plug-in section
    prepareCommon(payload);
    prepareEIS(payload);
    prepareRSSO(payload);
    // End feaure plug-in section

    featureParam.mQParams = P2Util::makeSimpleQParams(ENormalStreamTag_Prv, io, tuning, request->getCropper(), request->getLogger());
    P2Util::prepareExtraModule(featureParam.mQParams, request->getExtraData(), request->getLogger());

    TRACE_S_FUNC_EXIT(payload);
}

MBOOL StreamingProcessor::processP2(sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);
    MBOOL ret = MFALSE;
    //payload->mFeatureParam.mQParams.mpCookie = NULL;
    //payload->mFeatureParam.mQParams.mpfnCallback = NULL;
    //payload->mFeatureParam.mCallback = sFPipeCB;
    incPayload(payload);
    CAM_TRACE_NAME("P2_Streaming:drv enq");
    ret = mFeaturePipe->enque(payload->mFeatureParam);
    if( !ret )
    {
        MY_S_LOGW(payload->mRequest, "enque failed");
        payload->mIO.updateResult(MFALSE);
        decPayload(payload->mFeatureParam, payload, MFALSE);
    }
    TRACE_S_FUNC_EXIT(payload);
    return ret;
}

MVOID StreamingProcessor::onFPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param, const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);
    CAM_TRACE_NAME("P2_Streaming:onFPipeCB()");
    if( msg == FeaturePipeParam::MSG_FRAME_DONE )
    {
        payload->mIO.updateResult(param.mQParams.mDequeSuccess);
        if( param.getVar<MBOOL>(VAR_EIS_SKIP_RECORD, MFALSE) )
        {
            payload->mIO.dropRecord();
        }
        payload->mCurrRsso = NULL;
        payload->mPrevRsso = NULL;
        processTSQ(payload->mRequest, getTSQ(payload, param));
    }
    else if( msg == FeaturePipeParam::MSG_DISPLAY_DONE )
    {
        payload->mIO.earlyRelease(P2Util::RELEASE_DISP, param.mQParams.mDequeSuccess);
    }
    else if( msg == FeaturePipeParam::MSG_RSSO_DONE )
    {
        payload->mCurrRsso = NULL;
        payload->mPrevRsso = NULL;
    }
    else if( msg == FeaturePipeParam::MSG_FD_DONE )
    {
        payload->mIO.earlyRelease(P2Util::RELEASE_FD, param.mQParams.mDequeSuccess);
    }
    TRACE_S_FUNC_EXIT(payload);
}

MVOID StreamingProcessor::incPayloadCount(const Logger &logger)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mPayloadCountMutex);
    ++mPayloadCount;
    TRACE_S_FUNC(logger, "count=%d", mPayloadCount);
    TRACE_FUNC_EXIT();
}

MVOID StreamingProcessor::decPayloadCount(const Logger &logger)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mPayloadCountMutex);
    --mPayloadCount;
    TRACE_S_FUNC(logger, "count=%d", mPayloadCount);
    TRACE_FUNC_EXIT();
}

MBOOL StreamingProcessor::sFPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    sp<Payload> payload = param.getVar<sp<Payload>>(VAR_STREAMING_PAYLOAD, NULL);
    if( payload == NULL )
    {
        MY_LOGW("invalid payload = NULL");
        ret = MFALSE;
    }
    else if( payload->mParent == NULL )
    {
        MY_LOGW("invalid payload(%p), parent = NULL", payload.get());
        payload = NULL;
    }
    else
    {
        payload->mParent->onFPipeCB(msg, param, payload);
    }

    if( msg == FeaturePipeParam::MSG_FRAME_DONE &&
        payload != NULL )
    {
        payload->mParent->decPayload(param, payload, MTRUE);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

StreamingProcessor::Payload::Payload(StreamingProcessor *parent, const sp<P2Request> &request)
    : LoggerHolder(request)
    , mParent(parent)
    , mRequest(request)
    , mFeatureParam(sFPipeCB)
{
    mParent->incPayloadCount(mRequest->getLogger());
}

StreamingProcessor::Payload::~Payload()
{
    P2Util::releaseTuning(mTuning);
    P2Util::releaseExtraModule(mFeatureParam.mQParams);
    mParent->decPayloadCount(mRequest->getLogger());
}

MVOID StreamingProcessor::incPayload(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);
    android::Mutex::Autolock _lock(mPayloadMutex);
    mPayloadList.push_back(payload);
    payload->mFeatureParam.setVar<sp<Payload>>(VAR_STREAMING_PAYLOAD, payload);
    TRACE_S_FUNC_EXIT(payload);
}

MBOOL StreamingProcessor::decPayload(FeaturePipeParam &param, sp<Payload> &payload, MBOOL checkOrder)
{
    TRACE_S_FUNC_ENTER(payload);
    android::Mutex::Autolock _lock(mPayloadMutex);
    MBOOL ret = MFALSE;
    auto it = find(mPayloadList.begin(), mPayloadList.end(), payload);
    if( it != mPayloadList.end() )
    {
        if( checkOrder && it != mPayloadList.begin() )
        {
            MY_S_LOGW(payload, "callback out of order");
        }
        mPayloadList.erase(it);
        mPayloadCondition.broadcast();
        ret = MTRUE;
    }
    else
    {
        MY_S_LOGE(mP2Info, "Payload not released: invalid data=%p list=%zu", payload.get(), mPayloadList.size());
    }

    param.clearVar<sp<Payload>>(VAR_STREAMING_PAYLOAD);
    payload->mFeatureParam.clearVar<sp<Payload>>(VAR_STREAMING_PAYLOAD);

    TRACE_S_FUNC_EXIT(payload);
    return ret;
}

MVOID StreamingProcessor::waitFeaturePipeDone()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mPayloadMutex);
    while( mPayloadList.size() )
    {
        mPayloadCondition.wait(mPayloadMutex);
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

sp<P2Img> StreamingProcessor::updateRsso(const sp<P2Img> &rsso)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mRssoHolderMutex);
    sp<P2Img> holder;
    holder = mRssoHolder;
    mRssoHolder = rsso;
    TRACE_S_FUNC_EXIT(mP2Info);
    return holder;
}

MBOOL StreamingProcessor::prepareCommon(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);
    const sp<P2Request> &request = payload->mRequest;
    const P2Util::SimpleIO &io = payload->mIO;
    const Cropper &cropper = request->getCropper();
    FeaturePipeParam &featureParam = payload->mFeatureParam;
    IStreamingFeaturePipe::eAppMode mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;

    switch( request->getExtraData().mAppMode )
    {
    case MTK_FEATUREPIPE_PHOTO_PREVIEW:
        mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
        break;
    case MTK_FEATUREPIPE_VIDEO_PREVIEW:
        mode = IStreamingFeaturePipe::APP_VIDEO_PREVIEW;
        break;
    case MTK_FEATUREPIPE_VIDEO_RECORD:
        mode = IStreamingFeaturePipe::APP_VIDEO_RECORD;
        break;
    case MTK_FEATUREPIPE_VIDEO_STOP:
        mode = IStreamingFeaturePipe::APP_VIDEO_STOP;
        break;
    default:
        mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
        break;
    }

    featureParam.setVar<IStreamingFeaturePipe::eAppMode>(VAR_APP_MODE, mode);
    featureParam.setVar<MINT64>(VAR_P1_TS, request->getExtraData().mP1TS);
    TRACE_S_FUNC_ENTER(payload);
    return MTRUE;
}

MBOOL StreamingProcessor::isNeedRSSO(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);
    MBOOL ret = MFALSE;
    if( (payload->mRequest->getExtraData().mIsRecording &&
        EIS_MODE_IS_EIS_30_ENABLED(mPipeUsageHint.mEISInfo.mode) &&
        EIS_MODE_IS_EIS_IMAGE_ENABLED(mPipeUsageHint.mEISInfo.mode)) ||
        ((mPipeUsageHint.m3DNRMode & (E3DNR_MODE_MASK_HAL_FORCE_SUPPORT | E3DNR_MODE_MASK_UI_SUPPORT)) &&
        (mPipeUsageHint.m3DNRMode & E3DNR_MODE_MASK_RSC_EN)) )
    {
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingProcessor::prepareRSSO(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);
    const sp<P2Request> &request = payload->mRequest;
    FeaturePipeParam &featureParam = payload->mFeatureParam;

    if( !isNeedRSSO(payload) )
    {
        updateRsso(NULL);
    }
    else
    {
        payload->mCurrRsso = std::move(request->mImg[IN_RSSO]);
        payload->mPrevRsso = updateRsso(payload->mCurrRsso);
        if( isValid(payload->mCurrRsso) )
        {
            featureParam.setVar<IImageBuffer*>(VAR_CURR_RSSO, payload->mCurrRsso->getIImageBufferPtr());
        }
        if( isValid(payload->mPrevRsso) )
        {
            featureParam.setVar<IImageBuffer*>(VAR_PREV_RSSO, payload->mPrevRsso->getIImageBufferPtr());
        }
    }

    TRACE_S_FUNC_EXIT(payload);
    return MTRUE;
}

MINT64 StreamingProcessor::getTSQ(const sp<Payload> &payload, const FeaturePipeParam &param)
{
    TRACE_S_FUNC_ENTER(payload->mRequest);
    MINT64 ts = param.getVar<MINT64>(VAR_EIS_TSQ_TS, 0);
    TRACE_S_FUNC_EXIT(payload->mRequest);
    return ts;
}

MVOID StreamingProcessor::processTSQ(const sp<P2Request> &request, MINT64 ts)
{
    TRACE_S_FUNC_ENTER(request);
    sp<P2Meta> inHal = request->mMeta[IN_P1_HAL];
    sp<P2Meta> appOut = request->mMeta[OUT_APP];
    MBOOL updated = MFALSE;
    if( inHal != NULL && appOut != NULL )
    {
        MBOOL needOverride = getMeta<MBOOL>(inHal, MTK_EIS_NEED_OVERRIDE_TIMESTAMP, MFALSE);
        if( needOverride )
        {
            MINT64 timestamp;
            timestamp = ts ? ts : request->getExtraData().mP1TS;
            IMetadata::IEntry entry(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP);
            entry.push_back(1, Type2Type<MUINT8>());
            entry.push_back(1, Type2Type<MUINT8>());
            appOut->setEntry(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP, entry);
            appOut->trySet<MINT64>(MTK_SENSOR_TIMESTAMP, timestamp);
            appOut->trySet<MINT64>(MTK_EIS_FEATURE_NEW_SHUTTER_TIMESTAMP, timestamp);
            updated = MTRUE;
        }
    }
    TRACE_S_FUNC(request, "update=%d tsq=%" PRId64 " p1TS=%" PRId64, updated, ts, request->getExtraData().mP1TS);
    TRACE_S_FUNC_ENTER(request);
}

} // namespace P2
