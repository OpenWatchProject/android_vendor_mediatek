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

#define LOG_TAG "MtkCam/ppl_context"
//
#include <utility>

#include "Dispatcher.h"
#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::Utils::ULog;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
static NSCam::Utils::ULog::ModuleId getULogModuleId(Pipeline_NodeId_T nodeId)
{
    switch (nodeId)
    {
    case eNODEID_P1Node:
        return MOD_P1_NODE;
    case eNODEID_P1Node_main2:
        return MOD_P1_NODE_2;
    case eNODEID_P2CaptureNode:
        return MOD_P2_CAP_NODE;
    case eNODEID_JpegNode:
        return MOD_JPEG_NODE;
    case eNODEID_P2Node:
        return MOD_P2_STR_NODE;
    case eNODEID_FDNode:
        return MOD_FD_NODE;
    default:
        MY_LOGW("undefined nodeId:%#" PRIxPTR "", nodeId);
        break;
    }
    return MOD_HW_NODE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
DispatcherBase::
onMetaResultAvailable(
  MetaResultAvailable&& arg
) -> void
{
    {
        auto pDataCallback = mpDataCallback.promote();
        if  (CC_LIKELY( pDataCallback != nullptr )) {
            pDataCallback->onMetaResultAvailable(std::move(arg));
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DispatcherBase::
onEarlyCallback(
    MUINT32           requestNo,
    Pipeline_NodeId_T nodeId,
    StreamId_T        streamId,
    IMetadata const&  rMetaData,
    MBOOL             errorResult
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        pDataCallback->onMetaCallback(requestNo, nodeId, streamId, rMetaData, errorResult);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DispatcherBase::
onCtrlSetting(
    MUINT32           requestNo,
    Pipeline_NodeId_T nodeId,
    StreamId_T const  metaAppStreamId,
    IMetadata&        rAppMetaData,
    StreamId_T const  metaHalStreamId,
    IMetadata&        rHalMetaData,
    MBOOL&            rIsChanged
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        if (pDataCallback->isCtrlSetting()) {
            pDataCallback->onCtrlSetting(requestNo, nodeId,
                metaAppStreamId, rAppMetaData,
                metaHalStreamId, rHalMetaData,
                rIsChanged);
        }
        else {
            MY_LOGD("NOT Support ControlCallback - Setting");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DispatcherBase::
onCtrlSync(
    MUINT32           requestNo,
    Pipeline_NodeId_T nodeId,
    MUINT32           index,
    MUINT32           type,
    MINT64            duration
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        if (pDataCallback->isCtrlSync()) {
            pDataCallback->onCtrlSync(requestNo, nodeId,
                index, type, duration);
        }
        else {
            MY_LOGD("NOT Support ControlCallback - Sync");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DispatcherBase::
onCtrlResize(
    MUINT32           requestNo,
    Pipeline_NodeId_T nodeId,
    StreamId_T const  metaAppStreamId,
    IMetadata&        rAppMetaData,
    StreamId_T const  metaHalStreamId,
    IMetadata&        rHalMetaData,
    MBOOL&            rIsChanged
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        if (pDataCallback->isCtrlResize()) {
            pDataCallback->onCtrlResize(requestNo, nodeId,
                metaAppStreamId, rAppMetaData,
                metaHalStreamId, rHalMetaData,
                rIsChanged);
        }
        else {
            MY_LOGD("NOT Support ControlCallback - Resize");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DispatcherBase::
onCtrlReadout(
    MUINT32           requestNo,
    Pipeline_NodeId_T nodeId,
    StreamId_T const  metaAppStreamId,
    IMetadata&        rAppMetaData,
    StreamId_T const  metaHalStreamId,
    IMetadata&        rHalMetaData,
    MBOOL&            rIsChanged
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        if (pDataCallback->isCtrlSetting()) {
            pDataCallback->onCtrlSetting(requestNo, nodeId,
                metaAppStreamId, rAppMetaData,
                metaHalStreamId, rHalMetaData,
                rIsChanged);
        }
        else {
            MY_LOGD("NOT Support ControlCallback - Readout");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DispatcherBase::
needCtrlCb(
    eCtrlType eType
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    MBOOL res = MFALSE;
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        switch (eType) {
            case eCtrl_Setting:
                res = pDataCallback->isCtrlSetting();
                break;

            case eCtrl_Sync:
                res = pDataCallback->isCtrlSync();
                break;

            case eCtrl_Resize:
                res = pDataCallback->isCtrlResize();
                break;

            case eCtrl_Readout:
                res = pDataCallback->isCtrlReadout();
                break;

            default:
                break;
        };
    };
    return res;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DispatcherBase::
onNextCaptureCallBack(
    MUINT32           requestNo,
    Pipeline_NodeId_T nodeId,
    MUINT32           requestCnt,
    MBOOL             bSkipCheck
)
{
    android::sp<IDataCallback> pDataCallback = mpDataCallback.promote();
    if  (CC_LIKELY( pDataCallback != nullptr ))
    {
        pDataCallback->onNextCaptureCallBack(requestNo, nodeId, requestCnt, bSkipCheck);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultDispatcher::
onDispatchFrame(
    android::sp<IPipelineFrame> const& pFrame,
    Pipeline_NodeId_T nodeId
)
{
    CAM_TRACE_NAME(!ATRACE_ENABLED()?"":(
        std::string("onDispatchFrame|") +
        "requestNo:" + std::to_string(pFrame->getRequestNo()) +
        " frameNo:" + std::to_string(pFrame->getFrameNo()) +
        " nodeId:" + std::to_string(nodeId)
    ).c_str());

    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  (CC_UNLIKELY( pPipelineNodeMap == NULL || pPipelineNodeMap->isEmpty() )) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return;
    }
    sp<IPipelineNode> pCurrentNode = pPipelineNodeMap->nodeFor(nodeId);
    if (CC_LIKELY( pCurrentNode != nullptr )) {
        CAM_ULOG_EXIT(pCurrentNode, REQ_PIPELINE_FRAME, pFrame->getFrameNo());
    }
    //
    auto pPipelineBufferSetFrameControl = IPipelineBufferSetFrameControl::castFrom(pFrame.get());
    //
    IPipelineDAG::NodeObjSet_T nextNodes;
    MERROR err = pFrame->getPipelineDAG().getOutAdjacentNodes(nodeId, nextNodes);
    if  ( ! err && ! nextNodes.empty() )
    {
        for (size_t i = 0; i < nextNodes.size(); i++)
        {
            sp<IPipelineNode> pNextNode = pPipelineNodeMap->nodeAt(nextNodes[i].val);
            if (CC_LIKELY( pNextNode != nullptr ))
            {
                RWLock::AutoRLock _l(mFlushLock);
                Pipeline_NodeId_T nextNode_id = pNextNode->getNodeId();
                MUINT32 enqueue_count;
                IPipelineDAG::NodeObjSet_T InAdjacentNodes;

                // get Input Adjacent nodes of next node
                err = pFrame->getPipelineDAG().getInAdjacentNodes(nextNode_id, InAdjacentNodes);
                if(err)
                {
                    MY_LOGE("Get InAdjacentNodes of next node error (%d)", err);
                    return;
                }
                {
                    RWLock::AutoWLock _l(mRWLock);
                    // get In request counter of next node
                    err = pFrame->getPipelineDAGSp()->addInAdjacentNodesReqCnt(nextNode_id);
                    if(err)
                    {
                        MY_LOGE("Get InAdjacentNodes of next node error (%d)", err);
                        return;
                    }
                    err = pFrame->getPipelineDAG().getInAdjacentNodesReqCnt(nextNode_id, enqueue_count);
                    if(err)
                    {
                        MY_LOGE("Get InAdjacentNodes of next node error (%d)", err);
                        return;
                    }
                }
                // after next node receive all of requests, enqueue to next node
                if((size_t)enqueue_count >= InAdjacentNodes.size())
                {
                    auto text = android::String8::format("r%d frameNo:%d 0x%" PRIxPTR " -> 0x%" PRIxPTR " READY(%d/%zu)",
                        pFrame->getRequestNo(), pFrame->getFrameNo(), nodeId, nextNode_id, enqueue_count, InAdjacentNodes.size());
                    auto nextModuleId = getULogModuleId(nextNode_id);
                    CAM_ULOG_ENTER(nextModuleId, REQ_PIPELINE_FRAME, pFrame->getFrameNo());
                    if ( mInFlush || pFrame->isAborted() ) {
                        MY_LOGD("%s flush", text.c_str());
                        {
                            AutoLogDispatch(pPipelineBufferSetFrameControl, nodeId, nextNode_id, "flush");
                            pNextNode->flush(pFrame);
                        }
                    }
                    else {
                        MY_LOGD("%s", text.c_str());
                        {
                            AutoLogDispatch(pPipelineBufferSetFrameControl, nodeId, nextNode_id);
                            pNextNode->queue(pFrame);
                        }
                    }
                }
                else
                {
                    MY_LOGD("r%d frameNo:%d 0x%" PRIxPTR " -> 0x%" PRIxPTR " NOT READY(%d/%zu)",
                        pFrame->getRequestNo(), pFrame->getFrameNo(), nodeId, nextNode_id, enqueue_count, InAdjacentNodes.size());
                }
            }
        }
    }
    else {
        logDispatch(pPipelineBufferSetFrameControl, nodeId, eNODEID_UNKNOWN);
    }
}

