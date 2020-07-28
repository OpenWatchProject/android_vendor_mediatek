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

#include "P2AMDPNode.h"

#define PIPE_CLASS_TAG "P2AMDPNode"
#define PIPE_TRACE TRACE_P2AMDP_NODE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_P2A_MDP);


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum DumpMaskIndex
{
    MASK_ALL,
};

const std::vector<DumpFilter> sFilterTable =
{
    DumpFilter( MASK_ALL,      "all" )
};


P2AMDPNode::P2AMDPNode(const char *name)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_P2A_MDP)
    , StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mMDPRequests);
    TRACE_FUNC_EXIT();
}

P2AMDPNode::~P2AMDPNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL P2AMDPNode::onData(DataID id, const P2AMDPReqData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_PMDP )
    {
        mMDPRequests.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2AMDPNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    enableDumpMask(0, sFilterTable);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2AMDPNode::onThreadLoop()
{
    TRACE_FUNC_ENTER();
    P2AMDPReqData mdpRequest;
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    if( !waitAllQueue() )
    {
        TRACE_FUNC("Wait all queue exit");
        return MFALSE;
    }
    if( !mMDPRequests.deque(mdpRequest) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    if( mdpRequest.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    handleRequest(mdpRequest.mData, mdpRequest.mRequest);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2AMDPNode::handleRequest(const P2AMDPReq &data, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    request->mTimer.startP2AMDP();
    processMDP(data.mMDPIn, data.mMDPOuts, request);
    handleData(ID_PMDP_TO_HELPER, HelperData(HelpReq(FeaturePipeParam::MSG_INVALID, HMSG_PMDP_DONE), request));
    request->mTimer.stopP2AMDP();
    TRACE_FUNC_EXIT();
}

MBOOL P2AMDPNode::processMDP(const BasicImg &mdpIn, const std::vector<P2IO> &mdpOuts, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    if(mdpOuts.empty())
    {
        if(mdpIn.mBuffer == NULL)
        {
            return MTRUE;
        }
        else
        {
            MY_LOGE("Frame %d mdp in fullImg not NULL w/ mdp out list size(%zu)", request->mRequestNo, mdpOuts.size());
            return MFALSE;
        }
    }
    MBOOL result = MFALSE;

    if( mdpIn.mBuffer != NULL )
    {
        if(request->needPrintIO())
        {
            MY_LOGD("Frame %d in Helper doMDP, in(%p) , out size(%zu)", request->mRequestNo,
                mdpIn.mBuffer->getImageBufferPtr(), mdpOuts.size());
        }

        result = mMDP.process(mdpIn.mBuffer->getImageBufferPtr(), mdpOuts, request->needPrintIO());
        request->updateResult(result);

        if( request->needDump() && allowDump(MASK_ALL))
        {
            const size_t size = mdpOuts.size();
            for( unsigned i = 0; i < size; ++i )
            {
                dumpData(request, mdpOuts[i].mBuffer, "p2amdp_%d", i);
            }
        }
    }
    else
    {
        MY_LOGE("Frame %d mdp in fullImg is NULL", request->mRequestNo);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
