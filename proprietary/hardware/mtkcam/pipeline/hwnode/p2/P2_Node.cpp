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

#include "P2_Node.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2_Node
#define P2_TRACE        TRACE_P2_NODE
#include "P2_LogHeader.h"
using namespace NSCam::EIS;

namespace P2
{

P2_Node::P2_Node(const P2FeatureNode::ePass2Type pass2Type, const P2FeatureNode::UsageHint &usageHint)
    : mStatus(STATUS_IDLE)
    , mNodeID(INVALID_SENSOR_ID)
    , mNodeName("P2_Node")
    , mFrameCount(0)
    , mLogLevel(0)
    , mRedirectMode(REDIRECT_NEVER)
{
    MY_LOG_FUNC_ENTER("P2Node(P2F)");
    mLogLevel = property_get_int32(KEY_P2_LOG, VAL_P2_LOG);
    if( setupRedirect(MFALSE) )
    {
        mRedirectP2F = P2FeatureNode::createHal1Instance(pass2Type, usageHint);
    }
    mConfigParam.mP2Type = toP2Type(pass2Type, usageHint);
    mConfigParam.mUsageHint = toP2UsageHint(usageHint);
    MY_LOG_FUNC_EXIT();
}

P2_Node::P2_Node(const P2Node::ePass2Type pass2Type, const P2Common::UsageHint &usageHint)
    : mStatus(STATUS_IDLE)
    , mNodeID(INVALID_SENSOR_ID)
    , mNodeName("P2_Node")
    , mFrameCount(0)
    , mLogLevel(0)
    , mRedirectMode(REDIRECT_NEVER)
{
    MY_LOG_FUNC_ENTER("P2Node(P2)");
    mLogLevel = property_get_int32(KEY_P2_LOG, VAL_P2_LOG);
    if( setupRedirect(usageHint.mAppMode != P2Common::APP_MODE_VIDEO) )
    {
        mRedirectP2 = P2Node::createInstance(pass2Type);
    }
    mConfigParam.mP2Type = toP2Type(pass2Type, usageHint);
    mConfigParam.mUsageHint = toP2UsageHint(usageHint);
    MY_LOG_FUNC_EXIT();
}

P2_Node::~P2_Node()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    if( mStatus != STATUS_IDLE )
    {
        this->uninit();
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

NSCam::MERROR P2_Node::init(const IPipelineNode::InitParams &initParam)
{
    Logger logger = makeSensorLogger(mLogLevel, initParam.openId);
    MY_LOG_S_FUNC_ENTER(logger);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::init");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_IDLE )
    {
        MY_S_LOGW(logger, "cannot init: status[%d] != IDLE", mStatus);
    }
    else
    {
        ret = parseInitParam(initParam, logger) &&
              mDispatcher.init(P2InitParam(mP2Info, mConfigParam));
        if( ret )
        {
            mStatus = STATUS_READY;
        }
    }
    if( mRedirectP2 != NULL )
    {
        ret = (mRedirectP2->init(initParam) == OK) && ret;
    }
    if( mRedirectP2F != NULL )
    {
        ret = (mRedirectP2F->init(initParam) == OK) && ret;
    }
    MY_LOG_S_FUNC_EXIT(logger);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2_Node::uninit()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::uninit");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mP2Info, "cannot uninit: status[%d] != READY", mStatus);
    }
    else
    {
        mDispatcher.uninit();
        mStatus = STATUS_IDLE;
        ret = MTRUE;
    }
    if( mRedirectP2 != NULL )
    {
        ret = (mRedirectP2->uninit() == OK) && ret;
    }
    if( mRedirectP2F != NULL )
    {
        ret = (mRedirectP2F->uninit() == OK) && ret;
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2_Node::config(const P2FeatureNode::ConfigParams &configParam)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::config");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mP2Info, "cannot config: status[%d] != READY", mStatus);
    }
    else
    {
        ret = parseConfigParam(configParam) && updateConfig();
    }
    if( mRedirectP2F != NULL )
    {
        ret = (mRedirectP2F->config(configParam) == OK) && ret;
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2_Node::config(const P2Node::ConfigParams &configParam)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::config");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mP2Info, "cannot config: status[%d] != READY", mStatus);
    }
    else
    {
        ret = parseConfigParam(configParam) && updateConfig();
    }
    if( mRedirectP2 != NULL )
    {
        ret = (mRedirectP2->config(configParam) == OK) && ret;
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2_Node::queue(sp<IPipelineFrame> frame)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::queue");
    MBOOL ret = MFALSE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mP2Info, "cannot queue: status[%d] != READY", mStatus);
    }
    else if( frame == NULL )
    {
        MY_S_LOGW(mP2Info, "cannot queue: pipeline frame = NULL");
    }
    else if( needRedirect(frame) )
    {
        if( mRedirectP2 != NULL )
        {
            ret = (mRedirectP2->queue(frame) == OK);
        }
        else if( mRedirectP2F != NULL )
        {
            ret = (mRedirectP2F->queue(frame) == OK);
        }
    }
    else
    {
        sp<MWFrame> frameHolder;
        MUINT32 frameID = generateFrameID();
        //frameID = frame->getFrameNo();
        Logger logger = makeFrameLogger(mLogLevel, mP2Info->mSensorID, frameID);
        frameHolder = new MWFrame(mNodeID, mNodeName, frame, logger);
        if( frameHolder == NULL )
        {
            MY_S_LOGW(logger, "OOM: allocate MWFrame failed");
        }
        else
        {
            // MWFrame will handle callback even if enque failed
            ret = MTRUE;
            sp<P2FrameRequest> request;
            if( prepareFrameRequest(request, frameHolder, logger) )
            {
                mDispatcher.enque(request);
            }
        }
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2_Node::kick()
{
    return OK;
}

NSCam::MERROR P2_Node::flush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::flush");
    MBOOL ret = MTRUE;
    if( mStatus != STATUS_READY )
    {
        MY_S_LOGW(mP2Info, "cannot flush: status[%d] != READY", mStatus);
        ret = MFALSE;
    }
    else
    {
        mDispatcher.flush();
    }
    if( mRedirectP2 != NULL )
    {
        ret = (mRedirectP2->flush() == OK) && ret;
    }
    if( mRedirectP2F != NULL )
    {
        ret = (mRedirectP2F->flush() == OK) && ret;
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return ret ? OK : UNKNOWN_ERROR;
}

NSCam::MERROR P2_Node::flush(const sp<IPipelineFrame> &frame)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mMutex);
    CAM_TRACE_NAME("P2_Node::flush");
    if( frame != NULL )
    {
        Logger logger = makeFrameLogger(mLogLevel, getOpenId(), frame->getFrameNo());
        MWFrame::flushFrame(frame, mNodeID, logger);
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return OK;
}

MINT32 P2_Node::getOpenId() const
{
    TRACE_S_FUNC_ENTER(mP2Info);
    TRACE_S_FUNC_EXIT(mP2Info);
    return (mP2Info != NULL) ? mP2Info->mSensorID : INVALID_SENSOR_ID;
}

P2_Node::NodeId_T P2_Node::getNodeId() const
{
    TRACE_S_FUNC_ENTER(mP2Info);
    TRACE_S_FUNC_EXIT(mP2Info);
    return mNodeID;
}

char const* P2_Node::getNodeName() const
{
    TRACE_S_FUNC_ENTER(mP2Info);
    TRACE_S_FUNC_EXIT(mP2Info);
    return  mNodeName;
}

P2Type P2_Node::toP2Type(P2FeatureNode::ePass2Type pass2type, const P2FeatureNode::UsageHint &hint) const
{
    P2Type type = P2_UNKNOWN;
    if( pass2type == P2FeatureNode::PASS2_TIMESHARING )
    {
        type = P2_TIMESHARE_CAPTURE;
    }
    else
    {
        switch( hint.mUsageMode )
        {
        case P2FeatureNode::USAGE_PREVIEW:
            type = P2_PREVIEW;
            break;
        case P2FeatureNode::USAGE_CAPTURE:
            type = P2_CAPTURE;
            break;
        case P2FeatureNode::USAGE_TIMESHARE_CAPTURE:
            type = P2_TIMESHARE_CAPTURE;
            break;
        case P2FeatureNode::USAGE_RECORD:
            type = P2_VIDEO;
            break;
        default:
            type = P2_UNKNOWN;
            break;
        }
    }
    return type;
}

P2UsageHint P2_Node::toP2UsageHint(const P2FeatureNode::UsageHint &hint) const
{
    TRACE_S_FUNC_ENTER(mP2Info);
    P2UsageHint usage;

    usage.mStreamingSize = hint.mStreamingSize;
    usage.mEisInfo = hint.mEisInfo;
    usage.m3DNRMode = hint.m3DNRMode;
    usage.mUseTSQ = MFALSE;
    TRACE_S_FUNC_EXIT(mP2Info);
    return usage;
}

P2Type P2_Node::toP2Type(P2Node::ePass2Type pass2type, const P2Common::UsageHint &hint) const
{
    TRACE_S_FUNC_ENTER(mP2Info);
    P2Type type = P2_UNKNOWN;

    if( pass2type == P2Node::PASS2_TIMESHARING )
    {
        type = P2_TIMESHARE_CAPTURE;
    }
    else if( hint.mAppMode == P2Common::APP_MODE_VIDEO )
    {
        type = P2_VIDEO;
    }
    else
    {
        type = P2_PHOTO;
    }
    return type;
}

P2UsageHint P2_Node::toP2UsageHint(const P2Common::UsageHint &hint) const
{
    TRACE_S_FUNC_ENTER(mP2Info);
    P2UsageHint usage;

    usage.mStreamingSize = hint.mStreamingSize;
    usage.mEisInfo = EisInfo(hint.mPackedEisInfo);
    usage.m3DNRMode = hint.m3DNRMode;
    usage.mUseTSQ = hint.mUseTSQ;
    TRACE_S_FUNC_EXIT(mP2Info);
    return usage;
}

MUINT32 P2_Node::generateFrameID()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    TRACE_S_FUNC_EXIT(mP2Info);
    return ++mFrameCount;
}

MBOOL P2_Node::parseInitParam(const IPipelineNode::InitParams &initParam, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    MBOOL ret = MFALSE;
    MRect activeArray;
    if( P2Util::getActiveArrayRect(initParam.openId, activeArray, logger) )
    {
        this->mNodeID = initParam.nodeId;
        this->mNodeName = initParam.nodeName;
        sp<P2Info> info = new P2Info(initParam.openId, activeArray, mLogLevel);
        if( info == NULL )
        {
            MY_S_LOGE(logger, "OOM: create P2Info failed");
        }
        else
        {
            mP2Info = info;
            ret = MTRUE;
        }
        //param.mUsageHint = prepareUsageHint(this->mPass2Type, this->mUsageHint);
    }
    TRACE_S_FUNC_EXIT(logger);
    return ret;
}

MBOOL P2_Node::parseConfigParam(const P2FeatureNode::ConfigParams &configParam)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    sp<MWInfo> info = new MWInfo(configParam);
    if( info == NULL )
    {
        MY_S_LOGW(mP2Info, "OOM: allocate MWInfo failed");
    }
    else if( !info->isValid(getLogger(mP2Info)))
    {
        MY_S_LOGW(mP2Info, "invalid config param");
    }
    else
    {
        mMWInfo = info;
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL P2_Node::parseConfigParam(const P2Node::ConfigParams &configParam)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    sp<MWInfo> info = new MWInfo(configParam);
    if( info == NULL )
    {
        MY_S_LOGW(mP2Info, "OOM: allocate MWInfo failed");
    }
    else if( !info->isValid(getLogger(mP2Info)))
    {
        MY_S_LOGW(mP2Info, "invalid config param");
    }
    else
    {
        mMWInfo = info;
        mConfigParam.mUsageHint = toP2UsageHint(configParam.mUsageHint);
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL P2_Node::updateConfig()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = mDispatcher.config(mConfigParam);
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL P2_Node::prepareFrameRequest(sp<P2FrameRequest> &request, const sp<MWFrame> &frameHolder, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    MBOOL ret = MTRUE;
    request = new MWFrameRequest(mP2Info, mMWInfo, frameHolder);
    if( request == NULL )
    {
        MY_S_LOGW(logger, "OOM: allocate MWFrameRequest failed");
        ret = MFALSE;
    }
    TRACE_S_FUNC_EXIT(logger);
    return ret;
}

MBOOL P2_Node::setupRedirect(MBOOL wantRedirect)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    mRedirectMode = REDIRECT_NEVER;
    if( property_get_int32(KEY_P2_REDIRECT, VAL_P2_REDIRECT) )
    {
        ret = MTRUE;
        mRedirectMode = REDIRECT_ALWAYS;
    }
    else if( wantRedirect )
    {
        ret = MTRUE;
        mRedirectMode = REDIRECT_SELECT;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2_Node::needRedirect(const sp<IPipelineFrame> &frame)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    ret = (mRedirectMode != REDIRECT_NEVER) &&
          ((mRedirectMode == REDIRECT_ALWAYS) ||
           (mRedirectMode == REDIRECT_SELECT && isCapture(frame)));
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL hasMultiIOMap(IPipelineFrame::InfoIOMapSet &ioMap)
{
    return ioMap.mImageInfoIOMapSet.size() >= 2;
}

MBOOL hasRawIn(IPipelineFrame::InfoIOMapSet &ioMap, const sp<MWInfo> &mwInfo)
{
    MBOOL found = MFALSE;
    const IPipelineFrame::ImageInfoIOMapSet &imgSet = ioMap.mImageInfoIOMapSet;
    if( mwInfo != NULL )
    {
        for( unsigned i = 0, iMax = imgSet.size(); !found && i < iMax; ++i )
        {
            for( unsigned j = 0, jMax = imgSet[i].vIn.size(); !found && j < jMax; ++j )
            {
                found = mwInfo->isCaptureIn(imgSet[i].vIn.keyAt(j));
            }
        }
    }
    return found;
}

MBOOL P2_Node::isCapture(const sp<IPipelineFrame> &frame)
{
    MBOOL ret = MFALSE;
    MBOOL hasMulti = MFALSE, hasRaw = MFALSE;
    IPipelineFrame::InfoIOMapSet ioMap;
    if( OK == frame->queryInfoIOMapSet(mNodeID, ioMap) )
    {
        hasMulti = hasMultiIOMap(ioMap);
        hasRaw = hasRawIn(ioMap, mMWInfo);
        ret = hasMulti || hasRaw;
    }
    if( ret )
    {
        MY_S_LOGD(mP2Info, "MWFrame %d is capture: multi=%d raw=%d", frame->getFrameNo(), hasMulti, hasRaw);
    }
    return ret;
}

} // namespace P2
