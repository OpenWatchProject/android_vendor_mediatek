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

#define LOG_TAG "MtkCam/BaseNode"
//
#include <signal.h>
#include <unistd.h>
//
#include <string>
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Sync.h>
#include <mtkcam/utils/std/CallStackLogger.h>
#include "BaseNode.h"
#include <cutils/properties.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

#include <mtkcam3/pipeline/hwnode/NodeId.h>
using namespace NSCam::Utils::ULog;

CAM_ULOG_DECLARE_MODULE_ID(MOD_BASE_NODE);

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT((1), "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1<=mLogLevel, "%d:%#" PRIxPTR ":%s +", getOpenId(), getNodeId(), getNodeName());
#define FUNCTION_OUT            MY_LOGD_IF(1<=mLogLevel, "%d:%#" PRIxPTR ":%s -", getOpenId(), getNodeId(), getNodeName());


/******************************************************************************
 *
 ******************************************************************************/
BaseNode::
BaseNode()
    : mOpenId(-1L)
    , mNodeId(NodeId_T(-1))
    , mNodeName()
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("vendor.debug.camera.log.basenode", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
BaseNode::
getOpenId() const
{
    return mOpenId;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineNode::NodeId_T
BaseNode::
getNodeId() const
{
    return mNodeId;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
BaseNode::
getNodeName() const
{
    return mNodeName.string();
}


/******************************************************************************
 *
 ******************************************************************************/
ModuleId
BaseNode::
getULogModuleId()
{
    switch (getNodeId())
    {
        case eNODEID_P1Node:
            return MOD_P1_NODE;
        case eNODEID_P1Node_main2:
            return MOD_P1_NODE_2;
        case eNODEID_P1Node_main3:
            return MOD_P1_NODE_3;
        case eNODEID_PDENode:
            return MOD_PDE_NODE;
        case eNODEID_P2CaptureNode:
            return MOD_P2_CAP_NODE;
        case eNODEID_JpegNode:
            return MOD_JPEG_NODE;
        case eNODEID_P2Node:
            return MOD_P2_STR_NODE;
        case eNODEID_FDNode:
            return MOD_FD_NODE;
        case eNODEID_RAW16:
            return MOD_RAW16_NODE;
        case eNODEID_RAW16_main2:
            return MOD_RAW16_NODE_2;
        case eNODEID_JpsNode:
            return MOD_JPS_NODE;
        default:
            MY_LOGD("undefined node id(0x%zX) -> ULog node id", mNodeId);
    }
    return MOD_HW_NODE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
ensureMetaBufferAvailable_(
    MUINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IMetaStreamBuffer>& rpStreamBuffer,
    MBOOL acquire
)
{
    FUNCTION_IN

    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGD("[frame:%u node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", frameNo, getNodeId(), streamId);
        return NAME_NOT_FOUND;
    }
    //
    if (acquire) {
        //
        //  Wait acquire_fence.
        sp<IFence> acquire_fence = IFence::create(rpStreamBuffer->createAcquireFence(getNodeId()));
        MERROR const err = acquire_fence->waitForever(getNodeName());
        MY_LOGE_IF(OK != err, "[frame:%u node:%#" PRIxPTR "][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", frameNo, getNodeId(), rpStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
        //
        //  Mark this buffer as ACQUIRED by this user.
        rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
    }
    //
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d", frameNo, getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }

    //
    FUNCTION_OUT

    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
ensureImageBufferAvailable_(
    MUINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    MBOOL acquire
)
{
    FUNCTION_IN

    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGD("[frame:%d node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", frameNo, getNodeId(), streamId);
        return NAME_NOT_FOUND;
    }
    //
    if (acquire) {
        //
        //  Wait acquire_fence.
        sp<IFence> acquire_fence = IFence::create(rpStreamBuffer->createAcquireFence(getNodeId()));
        MERROR const err = acquire_fence->waitForever(getNodeName());
        MY_LOGE_IF(OK != err, "[frame:%d node:%#" PRIxPTR "][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", frameNo, getNodeId(), rpStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
        //
        //  Mark this buffer as ACQUIRED by this user.
        rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
    }
    //
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        //  The producer ahead of this user may fail to render this buffer's content.
        MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d", frameNo, getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }

    FUNCTION_OUT
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onDispatchFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL) {
        cb->onDispatchFrame(pFrame, getNodeId());
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onEarlyCallback(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    IMetadata const& rMetaData,
    MBOOL error
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL) {
        cb->onEarlyCallback(pFrame->getRequestNo(), getNodeId(), streamId,
            rMetaData, error);
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onCtrlSetting(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const metaAppStreamId,
    IMetadata& rAppMetaData,
    StreamId_T const metaHalStreamId,
    IMetadata& rHalMetaData,
    MBOOL& rIsChanged
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL && cb->needCtrlCb(IPipelineNodeCallback::eCtrl_Setting)) {
        cb->onCtrlSetting(pFrame->getRequestNo(), getNodeId(),
            metaAppStreamId, rAppMetaData,
            metaHalStreamId, rHalMetaData,
            rIsChanged);
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onCtrlSync(
    android::sp<IPipelineFrame> const& pFrame,
    MUINT32 index,
    MUINT32 type,
    MINT64 duration
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL && cb->needCtrlCb(IPipelineNodeCallback::eCtrl_Sync)) {
        cb->onCtrlSync(pFrame->getRequestNo(), getNodeId(),
            index, type, duration);
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onCtrlResize(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const metaAppStreamId,
    IMetadata& rAppMetaData,
    StreamId_T const metaHalStreamId,
    IMetadata& rHalMetaData,
    MBOOL& rIsChanged
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL && cb->needCtrlCb(IPipelineNodeCallback::eCtrl_Resize)) {
        cb->onCtrlResize(pFrame->getRequestNo(), getNodeId(),
            metaAppStreamId, rAppMetaData,
            metaHalStreamId, rHalMetaData,
            rIsChanged);
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onCtrlReadout(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const metaAppStreamId,
    IMetadata& rAppMetaData,
    StreamId_T const metaHalStreamId,
    IMetadata& rHalMetaData,
    MBOOL& rIsChanged
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL && cb->needCtrlCb(IPipelineNodeCallback::eCtrl_Readout)) {
        cb->onCtrlReadout(pFrame->getRequestNo(), getNodeId(),
            metaAppStreamId, rAppMetaData,
            metaHalStreamId, rHalMetaData,
            rIsChanged);
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseNode::
needCtrlCb(
    android::sp<IPipelineFrame> const& pFrame,
    IPipelineNodeCallback::eCtrlType eType
)
{
    FUNCTION_IN

    MBOOL en = MFALSE;
    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL) {
        en = cb->needCtrlCb(eType);
    }

    FUNCTION_OUT
    return en;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onNextCaptureCallBack(
    android::sp<IPipelineFrame> const& pFrame,
    MUINT32   requestCnt,
    MBOOL     bSkipCheck
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
    if (cb != NULL) {
        cb->onNextCaptureCallBack(pFrame->getRequestNo(), getNodeId(), requestCnt, bSkipCheck);
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
setNodeCallBack(android::wp<INodeCallbackToPipeline> /*pCallback*/)
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
kick()
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
std::string
BaseNode::
getStatus()
{
    return std::string();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
triggerdb(TriggerDB const& arg)
{
    MY_LOGE("err:%d(%s) msg:%s", arg.err, ::strerror(-arg.err), arg.msg);
    #if (MTKCAM_HAVE_AEE_FEATURE == 1)
    aee_switch_ftrace(0);
    #endif

    if ( arg.needDumpCallstack ) {
        NSCam::Utils::CallStackLogger csl;
        csl.logProcess(LOG_TAG, ANDROID_LOG_WARN);
    }

    #if (MTKCAM_HAVE_AEE_FEATURE == 1)
    aee_system_exception(LOG_TAG, NULL, DB_OPT_DEFAULT|DB_OPT_FTRACE, arg.msg);
    #endif

    if ( arg.needTerminateCurrentProcess ) {
        MY_LOGE("commit suicide - raise(SIGTERM)");
        ::raise(SIGTERM);
        ::sleep(5);
        MY_LOGE("commit suicide - raise(SIGKILL)");
        ::raise(SIGKILL);
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    FUNCTION_IN;
    //
    MERROR err = OK;

    ////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////
    //
    //  Note:
    //  1. Don't mark ACQUIRE if never waiting on its acquire fence.
    //  2. Don't mark IN_FLIGHT (so we know the producer has not touched the buffer, and its content is ERROR).
    //  3. (Producer) users Needn't set its buffer status to ERROR.

    ////////////////////////////////////////////////////////////////////////////
    //  Mark buffers as RELEASE.
    ////////////////////////////////////////////////////////////////////////////

    sp<IStreamInfoSet const> pIStreams, pOStreams;
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    //
    err = pFrame->queryIOStreamInfoSet(getNodeId(), pIStreams, pOStreams);
    if  ( OK != err ) {
        MY_LOGE("nodeId:%#" PRIxPTR " frameNo:%d queryIOStreamInfoSet", getNodeId(), pFrame->getFrameNo());
    }
    //
    if  ( IStreamInfoSet const* pStreams = pIStreams.get() ) {
        {// I:Meta
            sp<IStreamInfoSet::IMap<IMetaStreamInfo> > pMap = pStreams->getMetaInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
        {// I:Image
            sp<IStreamInfoSet::IMap<IImageStreamInfo> > pMap = pStreams->getImageInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
    }
    else {
        MY_LOGE("nodeId:%#" PRIxPTR " frameNo:%d NULL IStreams", getNodeId(), pFrame->getFrameNo());
    }
    //
    if  ( IStreamInfoSet const* pStreams = pOStreams.get() ) {
        {// O:Meta
            sp<IStreamInfoSet::IMap<IMetaStreamInfo> > pMap = pStreams->getMetaInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
        {// O:Image
            sp<IStreamInfoSet::IMap<IImageStreamInfo> > pMap = pStreams->getImageInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
    }
    else {
        MY_LOGE("nodeId:%#" PRIxPTR " frameNo:%d NULL OStreams", getNodeId(), pFrame->getFrameNo());
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    rStreamBufferSet.applyRelease(getNodeId());

    ////////////////////////////////////////////////////////////////////////////
    //  Dispatch
    ////////////////////////////////////////////////////////////////////////////
    onDispatchFrame(pFrame);
    //
    FUNCTION_OUT;
    return OK;
}

