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

#include "TPI_AsyncNode.h"
#include "tpi/TPIMgr_Util.h"

#define PIPE_CLASS_TAG "TPIAsyncNode"
#define PIPE_TRACE TRACE_TPI_ASYNC_NODE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TPI_AsyncNode::TPI_AsyncNode(const char* name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mData);
    TRACE_FUNC_EXIT();
}

TPI_AsyncNode::~TPI_AsyncNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID TPI_AsyncNode::setTPIMgr(TPIMgr *tpiMgr)
{
    TRACE_FUNC_ENTER();
    mTPIMgr = tpiMgr;
    TRACE_FUNC_EXIT();
}

MBOOL TPI_AsyncNode::queryFrameEnable()
{
    TRACE_FUNC_ENTER();
    MBOOL run = MFALSE;
    if( mAsyncMode == AsyncMode::FIX_RATE )
    {
        if( ++mFixRateCounter >= mFixRate )
        {
            run = MTRUE;
            mFixRateCounter = 0;
        }
    }
    else if( mAsyncMode == AsyncMode::WAITING )
    {
        if( mWaitState == WaitState::IDLE )
        {
            mWaitState = WaitState::WAITING;
            run = MTRUE;
        }
    }
    else if( mAsyncMode == AsyncMode::POLLING )
    {
        run = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return run;
}

MBOOL TPI_AsyncNode::onData(DataID id, const BasicImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived async buf=%d", data.mRequest->mRequestNo, ID2Name(id), data.mData.mBuffer != NULL);
    MBOOL ret = MFALSE;
    if( id == ID_HELPER_TO_ASYNC )
    {
        mData.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType TPI_AsyncNode::getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return (stream == STREAMTYPE_ASYNC && reqInfo.isMaster())
           ? IOPOLICY_INOUT_EXCLUSIVE : IOPOLICY_BYPASS;
}

MBOOL TPI_AsyncNode::getInputBufferPool(const StreamingReqInfo &/*reqInfo*/, android::sp<IBufferPool> &pool, NextFullInfo &info)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    info.mResize = MSize(0, 0);
    info.mNeedCrop = MFALSE;
    info.mCanShare = MFALSE;
    if( mBufferPool != NULL )
    {
        pool = mBufferPool;
        // return MSize(0,0) to indicate using display size
        // resize = mInputInfo.mSize;
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_AsyncNode::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    StreamingFeatureNode::onInit();
    mEnable = mTPIMgr && mPipeUsage.supportTPI(TPIOEntry::ASYNC);

    if( mEnable )
    {
        mTPIUsage = mPipeUsage.getTPIUsage();
        mTPINodeIO = mTPIUsage.getTPIO(TPIOEntry::ASYNC, 0);
        mNumWorkingBuffer = mTPIUsage.getNumInBuffer(TPIOEntry::ASYNC);
        mInputInfo.mFormat = mTPIUsage.getCustomFormat(TPIOEntry::ASYNC, mPipeUsage.getFullImgFormat());
        mInputInfo.mSize = mTPIUsage.getCustomSize(TPIOEntry::ASYNC, mPipeUsage.getStreamingSize());
        mInputInfo.mStride = mInputInfo.mSize.w;
        mBufferPool = ImageBufferPool::create("fpipe.async", mInputInfo.mSize, mInputInfo.mFormat, ImageBufferPool::USAGE_HW);

        mAsyncMode = configAsyncMode();
        mExpectedMS = mTPINodeIO.mNodeInfo.mExpectMS;
        mFixRate = std::max<MUINT32>(1, ((mExpectedMS+32)/33));
        mFixRateCounter = mFixRate-1;
        ret = MTRUE;
    }
    MY_LOGI("initNode enable=%d mgr=%p ret=%d mode=%d(%s) ms=%d rate=%d size=(%dx%d)", mEnable, mTPIMgr, ret, mAsyncMode, toStr(mAsyncMode), mExpectedMS, mFixRate, mInputInfo.mSize.w, mInputInfo.mSize.h);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_AsyncNode::onUninit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MBOOL support = mPipeUsage.supportTPI(TPIOEntry::ASYNC);
    if( support && mTPIMgr )
    {
        ret = mTPIMgr->uninitNode(mTPINodeIO.mNodeID);
    }
    MY_LOGI("uninitNode support=%d mgr=%p ret=%d", support, mTPIMgr, ret);
    mTPIMgr = NULL;
    ImageBufferPool::destroy(mBufferPool);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_AsyncNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = initPlugin();
    if( mNumWorkingBuffer && mBufferPool != NULL )
    {
        Timer timer(true);
        EImageFormat fmt = mBufferPool->getImageFormat();
        MSize size = mBufferPool->getImageSize();
        mBufferPool->allocate(mNumWorkingBuffer);
        timer.stop();
        MY_LOGI("fpipe.async.in %s %d buf (0x%x/%s)(%dx%d) in %d ms", STR_ALLOCATE, mNumWorkingBuffer, fmt, toName(fmt), size.w, size.h, timer.getElapsed());
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_AsyncNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = uninitPlugin();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPI_AsyncNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    BasicImgData data;
    RequestPtr request;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mData.deque(data) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    if( data.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();
    request = data.mRequest;

    request->mTimer.startAsync();
    TRACE_FUNC("Frame %d in async", request->mRequestNo);

    MBOOL needRun = checkNeedRun(request, data);
    MBOOL needBypass = request->needTPIBypass();

    MY_LOGD("sensor(%d) Frame %d process start run:%d bypass:%d", mSensorIndex, request->mRequestNo, needRun, needBypass);

    MBOOL result = MFALSE;
    if( needRun )
    {
        result = process(request, data);
    }
    MY_LOGD("sensor(%d) Frame %d process done in %d ms, result:%d run:%d bypass:%d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueAsync(), result, needRun, needBypass);

    handleResultData(request);
    request->mTimer.stopAsync();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

TPI_AsyncNode::AsyncMode TPI_AsyncNode::configAsyncMode() const
{
    TRACE_FUNC_ENTER();
    AsyncMode mode = AsyncMode::NONE;

    MBOOL waiting = (mTPINodeIO.mNodeInfo.mCustomOption & TPI_MTK_OPT_ASYNC_WAITING);
    MBOOL polling = (mTPINodeIO.mNodeInfo.mCustomOption & TPI_MTK_OPT_ASYNC_POLLING);

    mode = waiting ? AsyncMode::WAITING :
           polling ? AsyncMode::POLLING :
                     AsyncMode::FIX_RATE;

    TRACE_FUNC_EXIT();
    return mode;
}

MBOOL TPI_AsyncNode::initPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI Async:initPlugin");
    if( mTPIMgr )
    {
        TPI_ConfigInfo config;
        config.mBufferInfo = mInputInfo;
        ret = mTPIMgr->initNode(mTPINodeIO.mNodeInfo.mNodeID, config);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_AsyncNode::uninitPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI Async:uninitPlugin");
    if( mTPIMgr )
    {
        ret = mTPIMgr->uninitNode(mTPINodeIO.mNodeID);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_AsyncNode::checkNeedRun(const RequestPtr &request, const BasicImgData &data)
{
    (void)request;
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mAsyncMode == AsyncMode::FIX_RATE )
    {
        ret = (data.mData.mBuffer != NULL);
    }
    else if( mAsyncMode == AsyncMode::WAITING )
    {
        ret = (data.mData.mBuffer != NULL);
    }
    else if( mAsyncMode == AsyncMode::POLLING )
    {
        ret = !this->peakAllQueue();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPI_AsyncNode::process(const RequestPtr &request, const BasicImgData &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    BasicImg yuv;
    TPIRes in, out;
    yuv = data.mData.mBuffer;
    yuv.syncCache(eCACHECTRL_INVALID);
    in.setSFP(TPI_BUFFER_ID_MTK_OUT_YUV, yuv);
    out.setTP(TPI_BUFFER_ID_MTK_YUV, yuv);

    TPI_Meta meta[32];
    TPI_Image img[32];
    unsigned metaCount = 0;
    unsigned imgCount = out.getImgArray(img, 32);
    dumpLog(request, meta, metaCount, img, imgCount);

    if( request->needTPIDump() )
    {
        dumpData(request, yuv, "tpi.async.in");
    }

    mWaitState = WaitState::RUNNING;
    request->mTimer.startEnqueAsync();
    P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "async plugin");
    if( !request->needTPIBypass() )
    {
        TPI_Data tpiData;
        ret = mTPIMgr->enqueNode(mTPINodeIO.mNodeInfo.mNodeID, request->mRequestNo, meta, metaCount, img, imgCount, tpiData);
    }
    P2_CAM_TRACE_END(TRACE_DEFAULT);
    request->mTimer.stopEnqueAsync();
    mWaitState = WaitState::IDLE;

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID TPI_AsyncNode::dumpLog(const RequestPtr &request, TPI_Meta meta[], unsigned metaCount, TPI_Image img[], unsigned imgCount)
{
    if( request->needTPILog() )
    {
        MY_LOGD("Async=0x%x imgCount = %d, metaCount = %d",
            mTPINodeIO.mNodeInfo.mNodeID, imgCount, metaCount);
        for( unsigned i = 0; i < imgCount; ++i )
        {
            unsigned stride = 0, bytes = 0;
            MSize size(0,0);
            IImageBuffer *ptr = img[i].mBufferPtr;
            EImageFormat fmt = eImgFmt_UNKNOWN;
            if( ptr )
            {
                stride = ptr->getBufStridesInBytes(0);
                bytes = ptr->getBufSizeInBytes(0);
                size = ptr->getImgSize();
                fmt = (EImageFormat)ptr->getImgFormat();
            }
            MY_LOGD("img[%d] id:0x%x buffer:%p (%dx%d) fmt=%d(%s) stride=%d bytes=%d",
                i, img[i].mBufferID, ptr, size.w, size.h, fmt, toName(fmt), stride, bytes);
        }
        for( unsigned i = 0; i < metaCount; ++i )
        {
            MY_LOGD("meta[%d] id:0x%x meta:%p",
                i, meta[i].mMetaID, meta[i].mMetaPtr);
        }
    }
}

MVOID TPI_AsyncNode::handleResultData(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    handleData(ID_ASYNC_TO_HELPER, HelperData(HelpReq(FeaturePipeParam::MSG_INVALID, HMSG_ASYNC_DONE), request));
    TRACE_FUNC_EXIT();
}

const char* TPI_AsyncNode::toStr(AsyncMode mode)
{
    switch(mode)
    {
    case AsyncMode::NONE:       return "none";
    case AsyncMode::FIX_RATE:   return "fix";
    case AsyncMode::WAITING:    return "wait";
    case AsyncMode::POLLING:    return "poll";
    default:                    return "unknown";
    };
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
