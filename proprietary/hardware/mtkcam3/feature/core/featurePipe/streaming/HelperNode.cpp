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

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_HELPER);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

HelperNode::HelperNode(const char *name)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_HELPER)
    , StreamingFeatureNode(name)
    , mMDP("fpipe.helper")
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

MVOID HelperNode::setTPIMgr(TPIMgr *mgr)
{
    TRACE_FUNC_ENTER();
    mTPIMgr = mgr;
    TRACE_FUNC_EXIT();
}

MBOOL HelperNode::onData(DataID id, const HelperData &data)
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
        id == ID_DISP_TO_HELPER ||
        0
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

    if( mPipeUsage.supportP2AP2() )
    {
        mWaitSet.insert(HMSG_PMDP_DONE);
        mCheckSet.insert(HMSG_DISPLAY_DONE);
        mCheckSet.insert(HMSG_FRAME_DONE);

        mCheckList.push_back(HMSG_DISPLAY_DONE);
        mCheckList.push_back(HMSG_FRAME_DONE);
    }

    if( mTPIMgr && mPipeUsage.supportTPI(TPIOEntry::META) )
    {
        mEnableMeta = MTRUE;
        mTPIMetaNode = mPipeUsage.getTPIUsage().getTPIO(TPIOEntry::META, 0);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mTPIMgr = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    if( mEnableMeta )
    {
        MBOOL ret = MFALSE;
        ret = initPlugin();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    if( mEnableMeta )
    {
        MBOOL ret = MFALSE;
        ret = uninitPlugin();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::onThreadLoop()
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    TRACE_FUNC("Waitloop");
    {
        HelperData cbRequest;
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
        processHelper(request, cbRequest.mData);
        request->mTimer.stopHelper();
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "release datas");
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::initPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI Meta:initPlugin");
    if( mTPIMgr )
    {
        TPI_ConfigInfo config;
        ret = mTPIMgr->initNode(mTPIMetaNode.mNodeID, config);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL HelperNode::uninitPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI Meta:uninitPlugin");
    if( mTPIMgr )
    {
        ret = mTPIMgr->uninitNode(mTPIMetaNode.mNodeID);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL HelperNode::processHelper(const RequestPtr &request, const HelpReq &helpReq)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    HelperMsg msg = helpReq.toHelperMsg();

    processTPIMeta(request, helpReq.mCBMsg);
    if( !mCheckSet.count(msg) && !mWaitSet.count(msg) )
    {
        processAsync(request, helpReq.mCBMsg);
        processCB(request, helpReq.mCBMsg);
    }
    else
    {
        request->mHelperNodeData.markReady(msg);
        if( request->mHelperNodeData.isReadyOrDone(mWaitSet) )
        {
            for( const HelperMsg &m : mCheckList ) // use list to confirm order
            {
                if( request->mHelperNodeData.isReady(m) )
                {
                    processAsync(request, toFPPMsg(m));
                    processCB(request, toFPPMsg(m));
                    request->mHelperNodeData.markDone(m);
                }
            }
        }
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL HelperNode::needMetaAsync(const RequestPtr &request, FeaturePipeParam::MSG_TYPE msg) const
{
    (void)request;
    // Check same msg for TPI_META and TPI_ASYNC
    // to ensure TPI_META always precedecs TPI_ASYNC
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return (msg == FeaturePipeParam::MSG_DISPLAY_DONE ||
            msg == FeaturePipeParam::MSG_FRAME_DONE);
}

MVOID HelperNode::processTPIMeta(const RequestPtr &request, FeaturePipeParam::MSG_TYPE msg)
{
    TRACE_FUNC_ENTER();
    if( mPipeUsage.supportTPI(TPIOEntry::META) && mTPIMgr )
    {
        MBOOL needCall;
        needCall = !request->mHelperNodeData.isReady(HMSG_META_DONE) &&
                   needMetaAsync(request, msg);

        if( needCall )
        {
            MBOOL ret = MFALSE;
            TPIRes res;
            TPI_Meta meta[32];
            TPI_Image img[32];
            unsigned imgCount = 0, metaCount = 0;

            request->getTPIMeta(res);
            metaCount = res.getMetaArray(meta, 32);
            request->mTimer.startTPIMeta();
            MY_LOGD("sensor(%d) Frame %d TPI meta plugin process start", mSensorIndex, request->mRequestNo);
            P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "3rdParty meta plugin");
            if( !request->needTPIBypass() )
            {
                TPI_Data tpiData;
                ret = mTPIMgr->enqueNode(mTPIMetaNode.mNodeID, request->mRequestNo, meta, metaCount, img, imgCount, tpiData);
            }
            MY_LOGD("sensor(%d) Frame %d TPI meta plugin process done in %d ms, result:%d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedTPIMeta(), ret);
            P2_CAM_TRACE_END(TRACE_DEFAULT);
            request->mTimer.stopTPIMeta();
            request->mHelperNodeData.markReady(HMSG_META_DONE);
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID HelperNode::processAsync(const RequestPtr &request, FeaturePipeParam::MSG_TYPE msg)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    if( mPipeUsage.supportTPI(TPIOEntry::ASYNC) )
    {
        if( needMetaAsync(request, msg) &&
            !request->mHelperNodeData.isReady(HMSG_ASYNC_DONE) )
        {
            ImgBuffer asyncImg, popImg;
            IImageBuffer *display = NULL, *copy = NULL;
            MBOOL mdp = MFALSE;
            MBOOL needAsync = request->needTPIAsync();
            if( needAsync )
            {
                popImg = request->popAsyncImg(this);
                if( popImg != NULL )
                {
                    request->mTimer.startHelperMDP();
                    display = request->getDisplayOutputBuffer();
                    copy = popImg->getImageBufferPtr();
                    if( display && copy )
                    {
                        mdp = mMDP.process(display, copy);
                        asyncImg = mdp ? popImg : NULL;
                    }
                    request->mTimer.stopHelperMDP();
                }
                else
                {
                    asyncImg = request->getAsyncImg(this);
                }
            }
            MY_LOGD("Frame %d async=%d img=%p pop=%p mdp=%d disp=%p copy=%p", request->mRequestNo, needAsync, asyncImg.get(), popImg.get(), mdp, display, copy);
            request->mHelperNodeData.markReady(HMSG_ASYNC_DONE);
            handleData(ID_HELPER_TO_ASYNC, BasicImgData(asyncImg, request));
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL HelperNode::processCB(const RequestPtr &request, FeaturePipeParam::MSG_TYPE msg)
{
    TRACE_FUNC_ENTER();
    if(msg != FeaturePipeParam::MSG_INVALID)
    {
        prepareCB(request, msg);
        CAM_NODE_ULOG_EXIT(this, request, getULogRequestSerial(request));
        request->doExtCallback(msg);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID HelperNode::prepareCB(const RequestPtr &request, FeaturePipeParam::MSG_TYPE msg)
{
    TRACE_FUNC_ENTER();
    switch( msg )
    {
    case FeaturePipeParam::MSG_DISPLAY_DONE:
        request->mTimer.markDisplayDone();
        break;
    case FeaturePipeParam::MSG_FRAME_DONE:
        request->mTimer.markDisplayDone();
        request->mTimer.markFrameDone();
        request->checkBufferHoldByReq();
        break;
    case FeaturePipeParam::MSG_RSSO_DONE:
        break;
    default:
        break;
    }
    TRACE_FUNC_EXIT();
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
