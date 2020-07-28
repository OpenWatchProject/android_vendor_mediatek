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

#include "P2_StreamingNode.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    StreamingNode
#define P2_TRACE        TRACE_P2_NODE
#include "P2_LogHeader.h"
using namespace NSCam::EIS;
using namespace NSCam::v3;
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_NODE);
using namespace NSCam::Utils::ULog;

android::Mutex P2::P2StreamingNodeImp::sMutex;
android::Condition P2::P2StreamingNodeImp::sCondition;
MBOOL P2::P2StreamingNodeImp::sRunning[P2S_MAX_SENSOR_SUPPORT] = {MFALSE};

sp<P2StreamingNode> P2StreamingNode::createInstance(P2StreamingNode::ePass2Type const type, P2Common::UsageHint usage) {
    if (type < 0 || type >= PASS2_TYPE_TOTAL) {
        MY_LOGE("not supported p2 type %d", type);
        return NULL;
    }

    return new P2::P2StreamingNodeImp(type, usage);
}

namespace P2
{

P2StreamingNodeImp::P2StreamingNodeImp(const P2StreamingNode::ePass2Type pass2Type, const P2Common::UsageHint &usageHint): BaseNode(),P2StreamingNode()
{
//    MY_LOG_FUNC_ENTER("StreamingNode");
    MUINT32 logLevel = property_get_int32(KEY_P2_LOG, VAL_P2_LOG);
    mLog = NSCam::Feature::makeLogger("", "P2S", logLevel);
    mP2Info = new P2InfoObj(mLog);
    mP2Info->mConfigInfo.mP2Type = toP2Type(pass2Type, usageHint);
    mP2Info->mConfigInfo.mUsageHint = toP2UsageHint(usageHint);
    mP2Info->mConfigInfo.mLogLevel = logLevel;
    MY_LOG_FUNC_EXIT();
}

P2StreamingNodeImp::~P2StreamingNodeImp()
{
//    MY_LOG_S_FUNC_ENTER(mLog);
    if( mStatus != STATUS_IDLE )
    {
        this->uninit();
    }
    MY_LOG_S_FUNC_EXIT(mLog);
}

NSCam::MERROR P2StreamingNodeImp::init(const IPipelineNode::InitParams &initParam)
{
    MUINT32 openID = initParam.openId;
    ILog sensorLog = NSCam::Feature::makeSensorLogger(mLog, openID);
    MY_LOG_S_FUNC_ENTER(sensorLog);
    if(openID < P2S_MAX_SENSOR_SUPPORT)
    {
        P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "StreamingNode::initLock");
        MY_S_LOGI(sensorLog, "init lock +");
        android::Mutex::Autolock _lock(sMutex);
        while(sRunning[openID])
        {
            sCondition.wait(sMutex);
        }
        sRunning[openID] = MTRUE;
        MY_S_LOGI(sensorLog, "init lock -");
        P2_CAM_TRACE_END(TRACE_DEFAULT);
    }
    else
    {
        MY_S_LOGE(sensorLog, "sensor ID (%d) >= P2S_MAX_SENSOR(%d), no lock protect",
            openID, P2S_MAX_SENSOR_SUPPORT);
    }
    android::Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "StreamingNode::init");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_IDLE )
    {
        MY_S_LOGW(sensorLog, "cannot init: status[%d] != IDLE", mStatus);
    }
    else
    {
        mDispatcher.setNeedThread(MFALSE);
        ret = parseInitParam(sensorLog, initParam) &&
              mDispatcher.init(P2InitParam(P2Info(mP2Info, sensorLog, mP2Info->mConfigInfo.mMainSensorID)));
        if( ret )
        {
            mLog = sensorLog;
            mStatus = STATUS_READY;
        }
    }
    MY_LOG_S_FUNC_EXIT(sensorLog);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2StreamingNodeImp::uninit()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    {
        android::Mutex::Autolock _lock(mMutex);
        P2_CAM_TRACE_NAME(TRACE_DEFAULT, "StreamingNode::uninit");
        if( mStatus != STATUS_READY )
        {
            MY_S_LOGW(mLog, "cannot uninit: status[%d] != READY", mStatus);
        }
        else
        {
            mDispatcher.uninit();
            mStatus = STATUS_IDLE;
            ret = MTRUE;
        }
    }

    MUINT32 openID = mP2Info->mConfigInfo.mMainSensorID;
    if(ret && openID < P2S_MAX_SENSOR_SUPPORT)
    {
        P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "StreamingNode::uninitLock");
        MY_S_LOGI(mLog, "uninit lock +");
        android::Mutex::Autolock _lock(sMutex);
        sRunning[openID] = MFALSE;
        sCondition.broadcast();
        MY_S_LOGI(mLog, "uninit lock -");
        P2_CAM_TRACE_END(TRACE_DEFAULT);
    }
    else
    {
        MY_S_LOGE(mLog, "sensor ID (%d) >= P2S_MAX_SENSOR(%d) or ret(%d)=false, no reset init lock",
            openID, P2S_MAX_SENSOR_SUPPORT, ret);
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2StreamingNodeImp::config(const P2StreamingNode::ConfigParams &configParam)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "StreamingNode::config");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mLog, "cannot config: status[%d] != READY", mStatus);
    }
    else
    {
        ret = parseConfigParam(configParam) &&
              mDispatcher.config(P2ConfigParam(P2Info(mP2Info, mP2Info->mLog, mP2Info->mConfigInfo.mMainSensorID)));
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2StreamingNodeImp::queue(sp<IPipelineFrame> frame)
{
    TRACE_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mLog, "cannot queue: status[%d] != READY", mStatus);
    }
    else if( frame == NULL )
    {
        MY_S_LOGW(mLog, "cannot queue: pipeline frame = NULL");
    }
    else
    {
        sp<MWFrame> frameHolder;

        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "StreamingNode:queue->newMWFrame");
        MUINT32 frameID = generateFrameID();
        //frameID = frame->getFrameNo();
        ILog frameLog = NSCam::Feature::makeFrameLogger(mLog, frame->getFrameNo(), frame->getRequestNo(), frameID);
        frameHolder = new MWFrame(frameLog, mNodeID, mNodeName, frame);
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        if( frameHolder == NULL )
        {
            MY_S_LOGW(frameLog, "OOM: allocate MWFrame failed");
        }
        else
        {
            // MWFrame will handle callback even if enque failed
            ret = MTRUE;
            sp<P2FrameRequest> request;
            if( prepareFrameRequest(frameLog, request, frameHolder) )
            {
                CAM_ULOG_SUBREQS(MOD_P2_STR_NODE, REQ_PIPELINE_FRAME, frame->getFrameNo(), REQ_P2_STR_REQUEST, frameID);
                CAM_ULOG_ENTER(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, frameID);
                mDispatcher.enque(request);
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2StreamingNodeImp::kick()
{
    return OK;
}

NSCam::MERROR P2StreamingNodeImp::flush()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "StreamingNode::flush");
    MBOOL ret = MTRUE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mLog, "cannot flush: status[%d] != READY", mStatus);
        ret = MFALSE;
    }
    else
    {
        mDispatcher.flush();
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2StreamingNodeImp::flush(const sp<IPipelineFrame> &frame)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "StreamingNode::flush");
    if( frame != NULL )
    {
        ILog log = NSCam::Feature::makeFrameLogger(mLog, frame->getFrameNo(), frame->getRequestNo(), 0);
        MWFrame::flushFrame(log, frame, mNodeID);
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return OK;
}

MINT32 P2StreamingNodeImp::getOpenId() const
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return mP2Info->mConfigInfo.mMainSensorID;
}

P2StreamingNodeImp::NodeId_T P2StreamingNodeImp::getNodeId() const
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return mNodeID;
}

char const* P2StreamingNodeImp::getNodeName() const
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return  mNodeName;
}

P2Type P2StreamingNodeImp::toP2Type(P2StreamingNode::ePass2Type /*pass2type*/, const P2Common::UsageHint &hint) const
{
    TRACE_S_FUNC_ENTER(mLog);
    P2Type type = P2_UNKNOWN;

    switch( hint.mAppMode )
    {
    case P2Common::APP_MODE_VIDEO:
        type = P2_VIDEO; break;
    case P2Common::APP_MODE_HIGH_SPEED_VIDEO:
        type = P2_HS_VIDEO; break;
    case P2Common::APP_MODE_BATCH_SMVR:
        type = P2_BATCH_SMVR; break;
    default:
        type = P2_PHOTO; break;
    }

    // TODO: remove adb debug
    bool forceSMVR = property_get_bool("vendor.debug.p2s.smvr", false);
    if( forceSMVR )
    {
        type = P2_BATCH_SMVR;
    }
    MY_LOGD("SMVR=%d type=%d forceSMVR=%d", P2_BATCH_SMVR, type, forceSMVR);

    TRACE_S_FUNC_EXIT(mLog);
    return type;
}

MUINT32 P2StreamingNodeImp::getP2PQIndex(const P2UsageHint &usage, P2Type p2Type) const
{
    MUINT32 pqIndex = 0;
    // handle EIS flow
    const NSCam::EIS::EisInfo eisInfo = usage.mEisInfo;
    if (EIS_MODE_IS_EIS_30_ENABLED(eisInfo.mode)) // Adv EIS mode
    {
        if (eisInfo.previewEIS)
        {
            if (usage.mHasVideo)
            {
                pqIndex = P2_PQ_EIS35_VIDEO;
                if (eisInfo.videoConfig == VIDEO_CFG_4K2K)
                {
                    pqIndex = P2_PQ_EIS35_VIDEO_4k;
                }
            }
            else
            {
                pqIndex = P2_PQ_EIS35_NO_VIDEO;
            }
        }
        else
        {
            pqIndex = P2_PQ_EIS30_VIDEO;
            if (eisInfo.videoConfig == VIDEO_CFG_4K2K)
            {
                pqIndex = P2_PQ_EIS30_VIDEO_4k;
            }
        }
    }
    else if (p2Type == P2_BATCH_SMVR) // SMVRBatch
    {
        pqIndex = P2_PQ_SMVRBATCH;
    }
    else if (p2Type == P2_HS_VIDEO) // SMVRConstraint
    {
        pqIndex = P2_PQ_SMVRCONSTRAINT;
    }
    else if (EIS_MODE_IS_EIS_12_ENABLED(eisInfo.mode)) // EIS1.2
    {
        pqIndex = P2_PQ_NORMAL;
        if (eisInfo.videoConfig == VIDEO_CFG_4K2K)
        {
            pqIndex = P2_PQ_EIS12_VIDEO_4k;
        }
    }
    else // Normal flow
    {
        pqIndex = P2_PQ_NORMAL;
    }

    MY_LOGD("MDPPQ: FeaturePQIdx=%d(%s), hasVideo=%d, eisInfo(previewEIS=%d, mode=0x%x, videoConfig=%d)",
        pqIndex, P2Util::p2PQIdx2String(pqIndex), usage.mHasVideo,
        eisInfo.previewEIS, eisInfo.mode, eisInfo.videoConfig);

    return pqIndex;
}

P2UsageHint P2StreamingNodeImp::toP2UsageHint(const P2Common::UsageHint &hint) const
{
    TRACE_S_FUNC_ENTER(mLog);
    P2UsageHint usage;

    usage.mStreamingSize = hint.mStreamingSize;
    usage.mOutSizeVector = hint.mOutSizeVector;
    usage.mAppSessionMeta = hint.mAppSessionMeta;
    usage.mDsdnHint = hint.mDsdnHint;
    usage.mTPICustomConfig = hint.mTPICustomConfig;

    if( mP2Info->mConfigInfo.mP2Type != P2_BATCH_SMVR )
    {
        usage.mEisInfo = EisInfo(hint.mPackedEisInfo);
        usage.m3DNRMode = hint.m3DNRMode;
        usage.mDualMode = hint.mDualFeatureMode ? hint.mDualFeatureMode : hint.mDualMode;
        usage.mDualMode = property_get_int32("vendor.debug.p2.dualMode", usage.mDualMode);
        usage.mSecType = hint.mSecType;
        usage.mFSCMode = hint.mFSCMode;
        usage.mUseTSQ = hint.mUseTSQ;
        usage.mEnlargeRsso = hint.mEnlargeRsso;
        usage.mTP = hint.mTP;
        usage.mTPMarginRatio = hint.mTPMarginRatio;
        usage.mResizedRawMap = hint.mResizedRawMap;
        usage.mSensorModule = hint.mSensorModule;

        usage.mOutCfg.mMaxOutNum = hint.mOutCfg.mMaxOutNum;
        usage.mOutCfg.mHasPhysical = hint.mOutCfg.mHasPhysical;
        usage.mOutCfg.mHasLarge = hint.mOutCfg.mHasLarge;
        usage.mOutCfg.mFDSize = hint.mOutCfg.mFDSize;
        usage.mOutCfg.mVideoSize = hint.mOutCfg.mVideoSize;
    }

    if( mP2Info->mConfigInfo.mP2Type == P2_BATCH_SMVR )
    {
        usage.mSMVRSpeed = hint.mSMVRSpeed;
    }

    usage.mHasVideo = hint.mHasVideo;
    usage.mP2PQIndex = getP2PQIndex(usage, mP2Info->mConfigInfo.mP2Type);

    TRACE_S_FUNC_EXIT(mLog);
    return usage;
}

MUINT32 P2StreamingNodeImp::generateFrameID()
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return ++mFrameCount;
}

MBOOL P2StreamingNodeImp::parseInitParam(const ILog &log, const IPipelineNode::InitParams &initParam)
{
    TRACE_S_FUNC_ENTER(log);
    MBOOL ret = MTRUE;
    this->mNodeID = initParam.nodeId;
    this->mNodeName = initParam.nodeName;
    mP2Info->mConfigInfo.mMainSensorID = initParam.openId;
    mP2Info->mConfigInfo.mLog = log;
    mP2Info->mLog = log;

    mP2Info->addSensorInfo(log, initParam.openId);
    std::vector<MUINT32> subList;
    subList.reserve(initParam.subOpenIdList.size());
    for( MUINT32 id : initParam.subOpenIdList )
    {
        if( id != (MUINT32)initParam.openId )
        {
            ILog sensorLog = makeSensorLogger(log, id);
            mP2Info->addSensorInfo(sensorLog, id);
            subList.push_back(id);
        }
    }

    mInIDMap = new P2InIDMap(initParam.openId, subList);
    TRACE_S_FUNC_EXIT(log);
    return ret;
}

MBOOL P2StreamingNodeImp::parseConfigParam(const P2StreamingNode::ConfigParams &configParam)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    sp<MWInfo> info = new MWInfo(configParam, mInIDMap);
    if( info == NULL )
    {
        MY_S_LOGW(mLog, "OOM: allocate MWInfo failed");
    }
    else if( !info->isValid(mP2Info->mConfigInfo.mLog) )
    {
        MY_S_LOGW(mLog, "invalid config param, inIDMap exist(%d)", (mInIDMap != NULL));
    }
    else
    {
        mMWInfo = info;
        mP2Info = mP2Info->clone();
        mP2Info->mConfigInfo.mUsageHint = toP2UsageHint(configParam.mUsageHint);
        mP2Info->mConfigInfo.mAppStreamInfo = info->getAppStreamInfo();
        updateConfigInfo(mP2Info, mMWInfo);
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID P2StreamingNodeImp::updateConfigInfo(const sp<P2InfoObj> &p2Info, const sp<MWInfo> &mwInfo)
{
    TRACE_S_FUNC_ENTER(mLog);
    p2Info->mConfigInfo.mBurstNum = mwInfo->getBurstNum();
    p2Info->mConfigInfo.mSupportPQ = mwInfo->supportPQ();
    if( p2Info->mConfigInfo.mP2Type != P2_BATCH_SMVR )
    {
        p2Info->mConfigInfo.mCustomOption = mwInfo->getCustomOption();
        p2Info->mConfigInfo.mSupportClearZoom = mwInfo->supportClearZoom();
        p2Info->mConfigInfo.mSupportDRE = mwInfo->supportDRE();
        p2Info->mConfigInfo.mSupportHFG = mwInfo->supportHFG();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL P2StreamingNodeImp::prepareFrameRequest(const ILog &log, sp<P2FrameRequest> &request, const sp<MWFrame> &frameHolder)
{
    TRACE_S_FUNC_ENTER(log);
    MBOOL ret = MTRUE;
    sp<P2DataObj> p2Data = new P2DataObj(log);
    p2Data->mFrameData.mP2FrameNo = log.getLogFrameID();
    P2Pack p2Pack(log, mP2Info, p2Data);
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "new MWFrameRequest");
    mFPSCounter.update();
    request = new MWFrameRequest(log, p2Pack, p2Data, mMWInfo, frameHolder, mInIDMap, mFPSCounter.getFPS());
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( request == NULL )
    {
        MY_S_LOGW(log, "OOM: allocate MWFrameRequest failed");
        ret = MFALSE;
    }
    TRACE_S_FUNC_EXIT(log);
    return ret;
}

} // namespace P2
