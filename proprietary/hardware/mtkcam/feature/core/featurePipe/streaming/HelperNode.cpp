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

#include "HelperNode.h"

#define PIPE_CLASS_TAG "HelperNode"
#define PIPE_TRACE TRACE_HELPER_NODE
#include <featurePipe/core/include/PipeLog.h>

static unsigned char FaceNum =0;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

HelperNode::HelperNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mCBRequests);
    TRACE_FUNC_EXIT();
}

HelperNode::~HelperNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL HelperNode::onData(DataID id, const CBMsgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_HELPER ||
        id == ID_MDP_TO_HELPER ||
        id == ID_WARP_TO_HELPER ||
        id == ID_VMDP_TO_HELPER ||
        id == ID_RSC_TO_HELPER ||
        id == ID_FOV_WARP_TO_HELPER ||
        id == ID_N3D_TO_HELPER
        )
    {
        mCBRequests.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL HelperNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onUninit()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    clearTSQ();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onThreadLoop()
{
    CAM_TRACE_CALL();
    TRACE_FUNC("Waitloop");
    CBMsgData cbRequest;
    RequestPtr request;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mCBRequests.deque(cbRequest) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    if( cbRequest.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();
    request = cbRequest.mRequest;
    request->mTimer.resumeHelper();
    TRACE_FUNC("Frame %d in Helper", request->mRequestNo);
    processHelper(cbRequest);
    request->mTimer.stopHelper();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::processHelper(const CBMsgData &cbRequest)
{
    TRACE_FUNC_ENTER();
    switch( cbRequest.mData )
    {
    case FeaturePipeParam::MSG_DISPLAY_DONE:
        cbRequest.mRequest->mTimer.markDisplayDone();
        break;
    case FeaturePipeParam::MSG_FRAME_DONE:
        cbRequest.mRequest->mTimer.markDisplayDone();
        cbRequest.mRequest->mTimer.markFrameDone();
        processEISQ(cbRequest.mRequest);
        break;
    case FeaturePipeParam::MSG_RSSO_DONE:
        break;
    default:
        break;
    }
    cbRequest.mRequest->doExtCallback(cbRequest.mData);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID HelperNode::processEISQ(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    EISQ_ACTION action = request->getEISQAction();
    if( mPipeUsage.supportEIS_TSQ() )
    {
        MINT64 ts = request->getVar<MINT64>(VAR_P1_TS, 0);
        ts = processEISQAction<MINT64>(action, ts, mTSQueue, ts);
        request->setVar<MINT64>(VAR_EIS_TSQ_TS, ts);
    }
    TRACE_FUNC_EXIT();
}

MVOID HelperNode::clearTSQ()
{
    TRACE_FUNC_ENTER();
    mTSQueue.clear();
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
