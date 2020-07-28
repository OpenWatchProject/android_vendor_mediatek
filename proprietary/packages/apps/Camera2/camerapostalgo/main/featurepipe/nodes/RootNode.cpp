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


#include "core/DebugControl.h"
#define PIPE_CLASS_TAG "RootNode"
#define PIPE_TRACE TRACE_ROOT_NODE
#include <core/PipeLog.h>

#include "RootNode.h"


#define __DEBUG // enable debug

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

RootNode::RootNode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    :FeatureNode(nid, name, 0, policy, priority)
    , mDebugLevel(0)
    , mDebugDump(0)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);

    MY_LOGD("mDebugDump=%d", mDebugDump);
    TRACE_FUNC_EXIT();
}

RootNode::~RootNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL RootNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "request %d: %s arrived", pRequest->getRequestNo(), PathID2Name(id));

    MBOOL ret = MFALSE;
    Mutex::Autolock _l(mLock);
    switch(id)
    {
        case PID_ENQUE:
        {
            const MINT32 frameIndex = pRequest->getPipelineFrameIndex();
            const MBOOL isFirstFrame = pRequest->isPipelineFirstFrame();
            MY_LOGD_IF(mLogLevel, "updateFaceData info., reqNo:%d, frameIndex:%d, isFirstFrame:%d",
                pRequest->getRequestNo(), frameIndex, isFirstFrame);

            /*
             *   no need to enque to root thread, just dispatch to
             */
            if(1) {
                /*
                if (pRequest->hasDelayInference())
                    pRequest->startInference();
                 */

                dispatch(pRequest);
            }
            else {
                mRequests.enque(pRequest);
            }
            ret = MTRUE;
            break;
        }
        default:
        {
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RootNode::onInit()
{
    TRACE_FUNC_ENTER();
    FeatureNode::onInit();

    TRACE_FUNC_EXIT();
    return MTRUE;
}


MBOOL RootNode::onThreadStart()
{
    return MTRUE;
}

MBOOL RootNode::onThreadStop()
{
    return MTRUE;
}

MERROR RootNode::evaluate(NodeID_T nodeId, FeatureInferenceData& rInfer)
{
    (void) rInfer;
    (void) nodeId;
    return OK;
}

MBOOL RootNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr pRequest;

    //CAM_TRACE_CALL();

    if (!waitAllQueue()) {
        return MFALSE;
    }

    {
        Mutex::Autolock _l(mLock);
        if (!mRequests.deque(pRequest)) {
            MY_LOGE("Request deque out of sync");
            return MFALSE;
        } else if (pRequest == NULL) {
            MY_LOGE("Request out of sync");
            return MFALSE;
        }

        onRequestProcess(pRequest);

        if (mbWait && mRequests.size() == 0) {
            mbWait = false;
            mWaitCondition.broadcast();
        }
    }

    return MTRUE;
}

MBOOL RootNode::onAbort(RequestPtr& pRequest)
{
    Mutex::Autolock _l(mLock);

    if (mRequests.size() > 0) {
        mbWait = true;
        MY_LOGI("Wait+:deque R/T: %d/%d, request size %zu", pRequest->getRequestNo(), pRequest->getTimestamp(), mRequests.size());
        auto ret = mWaitCondition.waitRelative(mLock, 100000000); // 100msec
        if (ret != OK) {
            MY_LOGW("wait timeout!!");
        }
        MY_LOGI("Wait-");
    }
    onRequestFinish(pRequest);
    return MTRUE;
}

MBOOL RootNode::onRequestProcess(RequestPtr& pRequest)
{
    this->incExtThreadDependency();

    /*
    if (pRequest->isCancelled() && !mbWait) {
        MY_LOGD("Cancel, R/F Num: %d/%d", pRequest->getRequestNo(), pRequest->getFrameNo());
        onRequestFinish(pRequest);
        return MFALSE;
    }
    */

    onRequestFinish(pRequest);
    return MTRUE;
}

MVOID RootNode::onRequestFinish(const RequestPtr& pRequest)
{
    if (pRequest->hasDelayInference())
        pRequest->startInference();

    if (mDebugDump) {
        MINT32 requestNo = pRequest->getRequestNo();
        MINT32 timestamp = pRequest->getTimestamp();
        MY_LOGD("Dump R:%d/Timestamp:%d", requestNo,timestamp);

    }
    MY_LOGD_IF(mLogLevel, "dispatch  I/C:%d/%d R/T:%d/%d isCross:%d",
                pRequest->getActiveFrameIndex(), pRequest->getActiveFrameCount(),
                pRequest->getRequestNo(), pRequest->getTimestamp(), pRequest->isCross());
    dispatch(pRequest);
    this->decExtThreadDependency();
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

