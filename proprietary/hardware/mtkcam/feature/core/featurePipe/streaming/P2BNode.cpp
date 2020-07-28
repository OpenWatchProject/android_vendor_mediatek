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

#include "P2BNode.h"

#define PIPE_CLASS_TAG "P2BNode"
#define PIPE_TRACE TRACE_P2B_NODE
#include <featurePipe/core/include/PipeLog.h>

using namespace NSCam::NSIoPipe::NSPostProc;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

P2BNode::P2BNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mFullImgDatas);
    this->addWaitQueue(&mVFBDatas);
    TRACE_FUNC_EXIT();
}

P2BNode::~P2BNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL P2BNode::onData(DataID id, const ImgBufferData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_P2B_FULLIMG ||
        id == ID_MDP_TO_P2B_FULLIMG )
    {
        mFullImgDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2BNode::onData(DataID id, const VFBData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_VFB_TO_P2B )
    {
        mVFBDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2BNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2BNode::onUninit()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2BNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2BNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitNormalStreamBaseDone();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2BNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    MUINT32 requestNo;
    RequestPtr request;
    ImgBufferData fullImg;
    VFBData vfb;

    if( !waitAllQueueSync(requestNo) )
    {
        return MFALSE;
    }
    if( !mFullImgDatas.deque(requestNo, fullImg) )
    {
        MY_LOGE("FullImgData deque out of sync: %d", requestNo);
        return MFALSE;
    }
    if( !mVFBDatas.deque(requestNo, vfb) )
    {
        MY_LOGE("P2B deque out of sync: %d", requestNo);
        return MFALSE;
    }
    if( fullImg.mRequest == NULL ||
        fullImg.mRequest->mRequestNo != requestNo ||
        fullImg.mRequest != vfb.mRequest )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();
    request = fullImg.mRequest;
    request->mTimer.startP2B();
    TRACE_FUNC("Frame %d in P2B", request->mRequestNo);
    processP2B(request, fullImg.mData, vfb.mData);
    request->mTimer.stopP2B();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2BNode::onNormalStreamBaseCB(const QParams &params, const P2BEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing P2ANode class members
    TRACE_FUNC_ENTER();
    RequestPtr request;
    request = data.mRequest;

    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }

    request->mTimer.stopEnqueP2B();
    MY_LOGD("Frame %d enque done in %d ms, result = %d", request->mRequestNo, request->mTimer.getElapsedEnqueP2B(), params.mDequeSuccess);

    if( !params.mDequeSuccess )
    {
        MY_LOGW("Frame %d enque result failed", request->mRequestNo);
    }

    request->updateResult(params.mDequeSuccess);
    request->doExtCallback(FeaturePipeParam::MSG_FRAME_DONE);
    TRACE_FUNC_EXIT();
}

MBOOL P2BNode::processP2B(const RequestPtr &request, const ImgBuffer &fullImg, const VFBResult &vfb)
{
    TRACE_FUNC_ENTER();
    (void)(vfb);
    // process here
    // QParams params;
    // P2BEnqueData store;
    // store.mGpu = gpu.mData;
    // store.mDsImg = p2b.mDsImg;
    // store.mAlphaCL = p2b.mAlphaCL;
    // store.mAlphaNR = p2b.mAlphaNR;
    // store.mPCA = p2b.mPCA;
    // store.mRequest = request;
    // request->mTimer.startEnqueP2B();
    // this->enqueNormalStreamBase(pipe, params, store);

#if !DEV_P2B_READY
    {
        IImageBuffer *out;
        out = findOutBuffer(request->mQParams, 0);
        if( out )
        {
            copyImageBuffer(fullImg->getImageBufferPtr(), out);
        }
    }
    request->updateResult(request->mP2A_QParams.mDequeSuccess);
    request->doExtCallback(FeaturePipeParam::MSG_FRAME_DONE);
#endif

    TRACE_FUNC_EXIT();
    return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
