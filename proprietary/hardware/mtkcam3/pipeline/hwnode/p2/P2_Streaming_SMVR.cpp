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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include <cutils/properties.h>
#include "P2_StreamingProcessor.h"

#include <mtkcam3/feature/smvr/SMVRData.h>

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Streaming_SMVR
#define P2_TRACE        TRACE_STREAMING_SMVR
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

#define SMVR_BURST_LIMIT  960

namespace P2
{

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe;
using NSCam::Feature::SMVR::SMVRResult;

MBOOL StreamingProcessor::prepareSMVR(const sp<P2RequestPack> &reqPack, const ILog &log)
{
    TRACE_S_FUNC_ENTER(log);
    P2Util::SimpleIn &in = reqPack->mInputs[0];
    FeaturePipeParam &featureParam = in.mFeatureParam;

    if( isBatchSMVR() )
    {
        MUINT32 reqCount = getSMVRCount(log, reqPack);
        prepareSMVRQueue(log, in, reqCount);
        prepareSMVRVar(log, featureParam, reqCount);
    }

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MBOOL StreamingProcessor::processSMVRQ(const FeaturePipeParam &param, const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mLog);
    if( isBatchSMVR() )
    {
        SMVRResult result = param.getVar<SMVRResult>(SFP_VAR::SMVR_RESULT, SMVRResult());
        MY_S_LOGD(payload->mLog, "SMVR out=%d drop=%d q=%d/%zu", result.mRunCount, result.mDropCount, result.mQueueCount, mSMVRQueue.size());
        releaseSMVRQueue(result.mRunCount, result.mDropCount);
        updateSMVRTimestamp(payload, result.mRunCount);
        updateSMVRMeta(payload, result.mQueueCount);
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
    return MTRUE;
}

MVOID StreamingProcessor::flushSMVRQ()
{
    TRACE_FUNC_ENTER();
    mSMVRQueue.clear();
    TRACE_FUNC_EXIT();
}

MBOOL StreamingProcessor::isBatchSMVR() const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = (mPipeUsageHint.mMode == IStreamingFeaturePipe::USAGE_BATCH_SMVR);
    TRACE_FUNC("ret=%d mMode=0x%x", ret, mPipeUsageHint.mMode);
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 StreamingProcessor::getSMVRCount(const ILog &log, const sp<P2RequestPack> &reqPack)
{
    TRACE_S_FUNC_ENTER(log);
    MBOOL isRec = MFALSE;
    MUINT32 count = 0;
    MUINT32 fps = 0, tsCount = 0, rrzoCount = 0, lcsoCount = 0;
    MUINT32 displayCount = 0, recordCount = 0;

    isRec = reqPack->mMainRequest->mP2Pack.getFrameData().mIsRecording;
    fps = getSMVRFPS(log, reqPack->mMainRequest->mMeta[IN_P1_HAL]);
    tsCount = reqPack->mMainRequest->mP2Pack.getSensorData().mP1TSVector.size();
    rrzoCount = reqPack->mInputs[0].mIMGI->getIImageBufferPtrsCount();
    lcsoCount = reqPack->mInputs[0].mLCEI->getIImageBufferPtrsCount();

    displayCount = reqPack->getNumDisplay();
    recordCount = reqPack->getNumRecord();

    if( recordCount )
    {
        count = fps/30;
        if( count > rrzoCount )
        {
            count = rrzoCount;
            MY_S_LOGW(log, "#rrzo(%d) < fps(%d)/30: #ts(%d) #record(%d), reset count to #rrzo",
                            rrzoCount, fps, tsCount, recordCount);
        }
    }

    if( SMVR_BURST_LIMIT )
    {
        MUINT32 queueSize = mSMVRQueue.size();
        mSMVROverBurst = (queueSize > SMVR_BURST_LIMIT) ||
                         (mSMVROverBurst && queueSize);
        count = mSMVROverBurst ? std::min<MUINT32>(1, count) : count;
        if( mSMVROverBurst )
        {
            MY_S_LOGW(log, "SMVR over burst: speed=%d size=%d limit=%d", mPipeUsageHint.mSMVRSpeed, queueSize, SMVR_BURST_LIMIT);
        }
    }

    MY_S_LOGD(log, "fps=%d #req=%d isRec=%d cd=%d #out(record)=%d #out(display)=%d #ts=%d #rrzo=%d #lcso=%d", fps, count, isRec, mSMVROverBurst, recordCount, displayCount, tsCount, rrzoCount, lcsoCount);
    TRACE_S_FUNC_EXIT(log);
    return count;
}

MUINT32 StreamingProcessor::getSMVRFPS(const ILog &log, const sp<P2Meta> &inHal) const
{
    TRACE_S_FUNC_ENTER(log);
    MUINT32 fps = 0;
    MUINT8 flag = getMeta<MUINT8>(inHal, MTK_HAL_REQUEST_SMVR_FPS, MTK_SMVR_FPS_30);
    switch( flag )
    {
    case MTK_SMVR_FPS_30:   fps = 30;   break;
    case MTK_SMVR_FPS_120:  fps = 120;  break;
    case MTK_SMVR_FPS_240:  fps = 240;  break;
    case MTK_SMVR_FPS_480:  fps = 480;  break;
    case MTK_SMVR_FPS_960:  fps = 960;  break;
    default:                fps = 0;    break;
    }
    TRACE_S_FUNC_EXIT(log);
    return fps;
}

MVOID StreamingProcessor::prepareSMVRQueue(const ILog &log, const P2Util::SimpleIn &in, MUINT32 reqCount)
{
    TRACE_S_FUNC_ENTER(log);
    mSMVRQueue.push(reqCount, in);
    TRACE_S_FUNC_EXIT(log);
}

MVOID StreamingProcessor::prepareSMVRVar(const ILog &log, FeaturePipeParam &featureParam, MUINT32 reqCount) const
{
    TRACE_S_FUNC_ENTER(log);
    featureParam.setVar<MUINT32>(SFP_VAR::SMVR_REQ_COUNT, reqCount);
    TRACE_S_FUNC_EXIT(log);
}

MVOID StreamingProcessor::releaseSMVRQueue(MUINT32 run, MUINT32 drop)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "release from SMVR queue");
    mSMVRQueue.pop(run);
    mSMVRQueue.pop(drop);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

MVOID StreamingProcessor::updateSMVRTimestamp(const sp<Payload> &payload, MUINT32 run)
{
    TRACE_FUNC_ENTER();
    if( payload->mPartialPayloads.size() == 1 )
    {
        payload->mPartialPayloads[0]->mRequestPack->updateVRTimestamp(run);
    }
    else
    {
        MY_S_LOGE(payload->mLog, "Invalid payload number(%zu) for SMVR", payload->mPartialPayloads.size());
    }
    TRACE_FUNC_EXIT();
}

MVOID StreamingProcessor::updateSMVRMeta(const sp<Payload> &payload, MUINT32 queueCount)
{
    const ILog &log = payload->mLog;
    TRACE_S_FUNC_ENTER(log);
    sp<P2Meta> appOut = payload->getMainRequest()->mMeta[OUT_APP];
    MINT32 result = queueCount ? 1 : 0;
    TRACE_S_FUNC(log, "queue=%d, result=%d appOut=%p",
                 queueCount, result, appOut.get());
    if( isValid(appOut) )
    {
        appOut->trySet<MINT32>(MTK_SMVR_FEATURE_SMVR_RESULT, result);
    }
    TRACE_S_FUNC_EXIT(log);
}

} // namespace P2
