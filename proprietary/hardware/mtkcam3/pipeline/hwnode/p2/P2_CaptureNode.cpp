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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "P2_CaptureNode.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    CaptureNode
#define P2_TRACE        TRACE_P2_NODE
#include "P2_LogHeader.h"
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_CAP_NODE);

using namespace android;
using namespace NSCam;
using namespace NSCam::EIS;
using namespace NSCam::v3;
using namespace P2;
using namespace NSCam::Feature;
using namespace NSCam::Utils::ULog;


MBOOL parseBGPreRelease(const P2Common::Capture::UsageHint &usageHint)
{
    const MINT32 bgModeProp = ::property_get_int32("vendor.debug.camera.bgservice.mode", 0);
    MBOOL ret = usageHint.mIsSupportedBGPreRelease;
    switch(bgModeProp) {
        case 0:
            // align hint
            ret = usageHint.mIsSupportedBGPreRelease;
            break;
        case 1:
            // force enable
            ret = MTRUE;
        break;
        case 2:
            // force disable
            ret = MFALSE;
        break;
    }
    MY_LOGD("background service preRelease info, hintIsSupportedBG:%d, bgModeProp:%d, ret:%d",
        usageHint.mIsSupportedBGPreRelease, bgModeProp, ret);
    return ret;
}

sp<P2CaptureNode> P2CaptureNode::createInstance(P2CaptureNode::ePass2Type type, const P2Common::Capture::UsageHint &usage)
{
    if (type < 0 || type >= PASS2_TYPE_TOTAL) {
        MY_LOGE("not supported p2 type %d", type);
        return NULL;
    }

    return new P2CaptureNodeImp(type, usage);
}


P2CaptureNodeImp::P2CaptureNodeImp(const P2CaptureNode::ePass2Type pass2Type, const P2Common::Capture::UsageHint &usageHint)
        : BaseNode()
        , P2CaptureNode()
        , mRequestNo(0)
{
    MY_LOG_FUNC_ENTER("CaptureNode");
    mNodeName = "P2CaptureNode";//default name
    MUINT32 logLevel = property_get_int32(KEY_P2_LOG, VAL_P2_LOG);
    mLog = NSCam::Feature::makeLogger("", "P2C", logLevel);
    mP2Info = new P2InfoObj(mLog);
    mP2Info->mConfigInfo.mP2Type = toP2Type(pass2Type);
    mP2Info->mConfigInfo.mUsageHint =
    {
        .mTP = usageHint.mSupportedScenarioFeatures,
        .mBGPreRelease = parseBGPreRelease(usageHint),
        .mPluginUniqueKey = usageHint.mPluginUniqueKey,
        .mIsHidlIsp       = usageHint.mIsHidlIsp
    };
    mP2Info->mConfigInfo.mUsageHint.mDualMode = property_get_int32("vendor.debug.p2.dualMode", usageHint.mDualFeatureMode);
    MY_S_LOGI(mLog, "(%p) ctor, uniqueKey:%d, supportedScenarioFeatures:%#" PRIx64 ", bgPreRelease:%d, hidlIsp:%d",
        this,
        mP2Info->mConfigInfo.mUsageHint.mPluginUniqueKey,
        mP2Info->mConfigInfo.mUsageHint.mTP,
        mP2Info->mConfigInfo.mUsageHint.mBGPreRelease,
        mP2Info->mConfigInfo.mUsageHint.mIsHidlIsp);
    mP2Info->mConfigInfo.mLogLevel = logLevel;
    MY_LOG_FUNC_EXIT();
}

P2CaptureNodeImp::~P2CaptureNodeImp()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    MY_S_LOGI(mLog, "(%p) dtor", this);
    if (mStatus != STATUS_IDLE) {
        this->uninit();
    }
    MY_LOG_S_FUNC_EXIT(mLog);
}

MERROR P2CaptureNodeImp::init(const InitParams &rParams)
{
    ILog sensorLog = makeSensorLogger(mLog, rParams.openId);
    MY_LOG_S_FUNC_ENTER(sensorLog);
    Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "CaptureNode::init");

    if (mStatus != STATUS_IDLE) {
        MY_S_LOGW(sensorLog, "cannot init: status[%d] != IDLE", mStatus);
        return INVALID_OPERATION;
    }

    MBOOL ret = MFALSE;

    mCaptureProcessor.setEnable(MTRUE);

    mP2Info->mConfigInfo.mMainSensorID = rParams.openId;
    mP2Info->mConfigInfo.mLog = sensorLog;
    mP2Info->mLog = sensorLog;

    mP2Info->addSensorInfo(sensorLog, rParams.openId);
    std::vector<MUINT32> subList;
    subList.reserve(rParams.subOpenIdList.size());
    for (MUINT32 id : rParams.subOpenIdList) {
        if ((MINT32)id != rParams.openId) {
            ILog log = makeSensorLogger(mLog, id);
            mP2Info->addSensorInfo(log, id);
            subList.push_back(id);
        }
    }

    mInIDMap = new P2InIDMap(rParams.openId, subList);
    mDrawIDPlugin = new P2DrawIDPlugin();

    ret = mCaptureProcessor.init(P2InitParam(P2Info(mP2Info, sensorLog, mP2Info->mConfigInfo.mMainSensorID)));

    if (ret) {
        mLog = sensorLog;
        mStatus = STATUS_READY;
        mOpenId = rParams.openId;
        mNodeId = rParams.nodeId;
        mNodeName = rParams.nodeName;

        MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName());
    }

    MY_LOG_S_FUNC_EXIT(sensorLog);
    return ret ? OK : UNKNOWN_ERROR;
}

MERROR P2CaptureNodeImp::uninit()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "CaptureNode::uninit");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mLog, "cannot uninit: status[%d] != READY", mStatus);
    }
    else
    {
        mCaptureProcessor.uninit();
        mStatus = STATUS_IDLE;
        ret = MTRUE;
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

MERROR P2CaptureNodeImp::config(const ConfigParams &configParam)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "CaptureNode::config");
    MBOOL ret = MFALSE;
    if (mStatus != STATUS_READY)
    {
        MY_S_LOGW(mLog, "cannot config: status[%d] != READY", mStatus);
    }
    else
    {
        ret = parseConfigParam(configParam) &&
              mCaptureProcessor.config(P2ConfigParam(P2Info(mP2Info, mP2Info->mLog, mP2Info->mConfigInfo.mMainSensorID)));
    }

    MY_LOG_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}


MERROR P2CaptureNodeImp::queue(sp<IPipelineFrame> frame)
{
    TRACE_S_FUNC_ENTER(mLog);
    Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "CaptureNode::queue");
    MBOOL ret = MFALSE;

    MY_S_LOGI(mLog, "(%p) R/F Num: %d/%d", this, frame->getRequestNo(), frame->getFrameNo());

    if (mStatus != STATUS_READY) {
        MY_S_LOGW(mLog, "cannot queue: status[%d] != READY", mStatus);
    } else if(frame == NULL) {
        MY_S_LOGW(mLog, "cannot queue: pipeline frame = NULL");
    } else {
        mRequestNo = frame->getRequestNo();
        MUINT32 uFrameNo = frame->getFrameNo();
        ILog frameLog = NSCam::Feature::makeFrameLogger(mLog, uFrameNo, mRequestNo, uFrameNo);
        sp<MWFrame> frameHolder = new MWFrame(frameLog, mNodeId, mNodeName, frame);

        // MWFrame will handle callback even if enque failed
        sp<P2FrameRequest> pFrameRequest = createFrameRequest(frameLog, frameHolder);
        if (pFrameRequest != NULL) {
            CAM_ULOG_SUBREQS(MOD_P2_CAP_NODE, REQ_PIPELINE_FRAME, frame->getFrameNo(), REQ_P2_CAP_REQUEST, frame->getFrameNo());
            CAM_ULOG_ENTER(MOD_P2_CAP_PROC, REQ_P2_CAP_REQUEST, frame->getFrameNo());
            mCaptureProcessor.enque(pFrameRequest);
            ret = MTRUE;
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

MUINT32 P2CaptureNodeImp::getULogModuleId()
{
        return MOD_P2_CAP_NODE;
}

MERROR P2CaptureNodeImp::flush()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    Mutex::Autolock _lock(mMutex);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "CaptureNode::flush");
    MBOOL ret = MTRUE;
    if (mStatus != STATUS_READY) {
        MY_S_LOGW(mLog, "cannot flush: status[%d] != READY", mStatus);
        ret = MFALSE;
    } else {
        mCaptureProcessor.flush();
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return ret ? OK : UNKNOWN_ERROR;
}

MERROR P2CaptureNodeImp::flush(const sp<IPipelineFrame> &frame)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    if (frame != NULL) {
#if 1
        MY_LOGI("(%p) queue frame while flush, R/F Num: %d/%d)",
            this, frame->getRequestNo(), frame->getFrameNo());
        queue(frame);
#else
        if (frame->getRequestNo() == mRequestNo) {
            // Ensure that multi-frame request doesn't drop a frame
            MY_LOGI("queue multi-frame request while flush, R/F Num: %d/%d)",
                frame->getRequestNo(), frame->getFrameNo());
            queue(frame);
        } else {
            Mutex::Autolock _lock(mMutex);
            P2_CAM_TRACE_NAME(TRACE_DEFAULT, "CaptureNode::flush");
            ILog log = NSCam::Feature::makeFrameLogger(mLog, frame->getFrameNo(), frame->getRequestNo(), 0);
            MWFrame::flushFrame(log, frame, mNodeId);
        }
#endif
    }
    MY_LOG_S_FUNC_EXIT(mLog);
    return OK;
}

std::string
P2CaptureNodeImp::
getStatus()
{
    return mCaptureProcessor.getStatus();
}

P2Type P2CaptureNodeImp::toP2Type(ePass2Type pass2type) const
{
    P2Type type = P2_UNKNOWN;
    if (pass2type == PASS2_TIMESHARING) {
        type = P2_TIMESHARE_CAPTURE;
    } else {
        type = P2_CAPTURE;
    }
    return type;
}


MBOOL P2CaptureNodeImp::parseConfigParam(const ConfigParams &configParam)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    sp<MWInfo> info = new MWInfo(configParam, mInIDMap);
    if (!info->isValid(mP2Info->mConfigInfo.mLog)) {
        MY_S_LOGW(mLog, "invalid config param, inIDMap exist(%d)", (mInIDMap != NULL));
    } else {
        mMWInfo = info;
        mP2Info = mP2Info->clone();
        updateConfigInfo(mP2Info, mMWInfo);
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID P2CaptureNodeImp::updateConfigInfo(const sp<P2InfoObj> &p2Info, const sp<MWInfo> &mwInfo)
{
    TRACE_S_FUNC_ENTER(mLog);
    p2Info->mConfigInfo.mCustomOption = mwInfo->getCustomOption();
    p2Info->mConfigInfo.mSupportClearZoom = mwInfo->supportClearZoom();
    p2Info->mConfigInfo.mSupportDRE = mwInfo->supportDRE();
    p2Info->mConfigInfo.mSupportHFG = mwInfo->supportHFG();
    TRACE_S_FUNC_EXIT(mLog);
}

sp<P2FrameRequest> P2CaptureNodeImp::createFrameRequest(const ILog &log, const sp<MWFrame> &frameHolder)
{
    TRACE_S_FUNC_ENTER(log);
    sp<P2DataObj> p2Data = new P2DataObj(log);
    p2Data->mFrameData.mP2FrameNo = log.getLogFrameID();
    P2Pack p2Pack(log, mP2Info, p2Data);
    sp<P2FrameRequest> request = new MWFrameRequest(log, p2Pack, p2Data, mMWInfo, frameHolder, mInIDMap);
    if( mDrawIDPlugin->isEnabled() )
    {
        request->registerImgPlugin(mDrawIDPlugin, MTRUE);
    }
    TRACE_S_FUNC_EXIT(log);
    return request;
}


