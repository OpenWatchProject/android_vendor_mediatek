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

#include "TPI_DispNode.h"

#define PIPE_CLASS_TAG "TPIDispNode"
#define PIPE_TRACE TRACE_TPI_DISP_NODE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TPI_DispNode::TPI_DispNode(const char *name)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_TPI_DISP)
    , StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mHelperRequests);
    TRACE_FUNC_EXIT();
}

TPI_DispNode::~TPI_DispNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID TPI_DispNode::setTPIMgr(TPIMgr *mgr)
{
    TRACE_FUNC_ENTER();
    mTPIMgr = mgr;
    TRACE_FUNC_EXIT();
}

MBOOL TPI_DispNode::onData(DataID id, const HelperData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_HELPER ||
        id == ID_P2SM_TO_HELPER ||
        id == ID_PMDP_TO_HELPER ||
        id == ID_BOKEH_TO_HELPER ||
        id == ID_WARP_TO_HELPER ||
        id == ID_VMDP_TO_HELPER ||
        id == ID_RSC_TO_HELPER ||
        id == ID_ASYNC_TO_HELPER ||
        0
        )
    {
        mHelperRequests.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_DispNode::onInit()
{
    MBOOL ret = MFALSE;
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();

    if( mTPIMgr && mPipeUsage.supportTPI(TPIOEntry::DISP) )
    {
        mTPIDispNode = mPipeUsage.getTPIUsage().getTPIO(TPIOEntry::DISP, 0);
        mEnable = MTRUE;
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_DispNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mTPIMgr = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_DispNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    if( mEnable )
    {
        MBOOL ret = MFALSE;
        ret = initPlugin();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_DispNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    if( mEnable )
    {
        MBOOL ret = MFALSE;
        ret = uninitPlugin();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_DispNode::onThreadLoop()
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    TRACE_FUNC("Waitloop");
    HelperData helperData;
    RequestPtr request;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mHelperRequests.deque(helperData) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    if( helperData.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();
    request = helperData.mRequest;
    request->mTimer.resumeTPIDisp();
    TRACE_FUNC("Frame %d in TPI_Disp", request->mRequestNo);
    if( checkRun(request, helperData.mData) )
    {
        processDisp(request);
    }
    handleHelperData(helperData);
    request->mTimer.stopTPIDisp();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_DispNode::initPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI Disp:initPlugin");
    if( mTPIMgr )
    {
        TPI_ConfigInfo config;
        ret = mTPIMgr->initNode(mTPIDispNode.mNodeID, config);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_DispNode::uninitPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI Disp:uninitPlugin");
    if( mTPIMgr )
    {
        ret = mTPIMgr->uninitNode(mTPIDispNode.mNodeID);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_DispNode::checkRun(const RequestPtr &request, const HelpReq &helpReq)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    HelperMsg msg = helpReq.toHelperMsg();
    if( msg == HMSG_DISPLAY_DONE ||
        msg == HMSG_FRAME_DONE )
    {
        ret = request->mDispNodeData;
        request->mDispNodeData = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID TPI_DispNode::processDisp(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    if( mPipeUsage.supportTPI(TPIOEntry::DISP) && mTPIMgr )
    {
        MBOOL ret = MFALSE;
        TPIRes res;
        TPI_Meta meta[32];
        TPI_Image img[32];

        request->getTPIMeta(res);
        IImageBuffer *display = request->getDisplayOutputBuffer();
        BasicImg disp(new IIBuffer_IImageBuffer(display));
        res.setTP(TPI_BUFFER_ID_MTK_YUV, disp);
        unsigned metaCount = res.getMetaArray(meta, 32);
        unsigned imgCount = res.getImgArray(img, 32);

        request->mTimer.startEnqueTPIDisp();
        MY_LOGD("sensor(%d) Frame %d TPI disp plugin process start", mSensorIndex, request->mRequestNo);
        P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "3rdParty disp plugin");
        if( !request->needTPIBypass() )
        {
            TPI_Data tpiData;
            ret = mTPIMgr->enqueNode(mTPIDispNode.mNodeID, request->mRequestNo, meta, metaCount, img, imgCount, tpiData);
        }
        MY_LOGD("sensor(%d) Frame %d TPI disp plugin process done in %d ms, result:%d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueTPIDisp(), ret);
        P2_CAM_TRACE_END(TRACE_DEFAULT);
        request->mTimer.stopEnqueTPIDisp();
    }
    TRACE_FUNC_EXIT();
}

MVOID TPI_DispNode::handleHelperData(const HelperData &helperData)
{
    TRACE_FUNC_ENTER();
    handleData(ID_DISP_TO_HELPER, helperData);
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
