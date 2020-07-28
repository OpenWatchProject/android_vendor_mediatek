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

#include "WarpNode.h"

#include "GPUWarpStream.h"
#include "WPEWarpStream.h"

#define PIPE_CLASS_TAG "WarpNode"
#define PIPE_TRACE TRACE_WARP_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <utils/String8.h>

using NSCam::NSIoPipe::Output;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

WarpNode::WarpNode(const char *name)
    : StreamingFeatureNode(name)
    , mWarpStream(NULL)
    , mExtraInBufferNeeded(0)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mWarpMapDatas);
    this->addWaitQueue(&mInputImgDatas);
    TRACE_FUNC_EXIT();
}

WarpNode::~WarpNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::setInputBufferPool(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mInputBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::setOutputBufferPool(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mOutputBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::clearTSQ()
{
    TRACE_FUNC_ENTER();
    mWarpSrcInfoQueue.clear();
    TRACE_FUNC_EXIT();
}

MBOOL WarpNode::onData(DataID id, const BasicImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_WARP_FULLIMG ||
        id == ID_VMDP_TO_NEXT_FULLIMG ||
        id == ID_FOV_TO_EIS_FULLIMG)
    {
        this->mInputImgDatas.enque(data);
        ret = MTRUE;
    }
    else if( id == ID_EIS_TO_WARP )
    {
        this->mWarpMapDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpNode::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    StreamingFeatureNode::onInit();

    if( mPipeUsage.supportWPE() )
    {
        mWarpStream = WPEWarpStream::createInstance();
    }
    else
    {
        mWarpStream = GPUWarpStream::createInstance();
    }

    if( mWarpStream == NULL )
    {
        MY_LOGE("OOM: failed to create warp module");
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mInputBufferPool = NULL;
    mOutputBufferPool = NULL;
    if( mWarpStream )
    {
        mWarpStream->destroyInstance();
        mWarpStream = NULL;
    }
    mExtraInBufferNeeded = 0;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL WarpNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mWarpStream != NULL &&
        mInputBufferPool != NULL &&
        mOutputBufferPool != NULL )
    {
        MUINT32 inputPoolSize = mPipeUsage.getNumWarpInBuffer();
        MUINT32 outputPoolSize = mPipeUsage.getNumWarpOutBuffer();
        mExtraInBufferNeeded = mPipeUsage.getNumExtraWarpInBuffer();
        Timer timer(MTRUE);

        mInputBufferPool->allocate(inputPoolSize);
        mOutputBufferPool->allocate(outputPoolSize);
        mNodeSignal->setSignal(NodeSignal::SIGNAL_GPU_READY);
        MY_LOGD("WarpNode %s (%d+%d) buf in %d ms", STR_ALLOCATE, inputPoolSize, outputPoolSize, timer.getNow());

        ret = mWarpStream->init(mSensorIndex, mInputBufferPool->getImageSize(), MAX_WARP_SIZE);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitWarpStreamBaseDone();
    if( mWarpStream != NULL )
    {
        mWarpStream->uninit();
    }
    clearTSQ();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL WarpNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    BasicImgData warpMap;
    BasicImgData inBuffer;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mInputImgDatas.deque(inBuffer) )
    {
        MY_LOGE("InputImgData deque out of sync");
        return MFALSE;
    }
    else if( !mWarpMapDatas.deque(warpMap) )
    {
        MY_LOGE("WarpMapData deque out of sync");
        return MFALSE;
    }
    if( warpMap.mRequest != inBuffer.mRequest )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();
    request = warpMap.mRequest;
    request->mTimer.startWarp();
    TRACE_FUNC("Frame %d in WarpNode needWarp=%d", request->mRequestNo, request->needWarp());
    processWarp(request, inBuffer.mData, warpMap.mData);
    request->mTimer.stopWarp();

    warpMap.mData.mBuffer = NULL;
    inBuffer.mData.mBuffer = NULL;

    if( request->needWarp() )
    {
        tryAllocateExtraInBuffer();
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID WarpNode::tryAllocateExtraInBuffer()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("FullImgPool=(%d/%d) ExtraNeeded=%d", mInputBufferPool->peakAvailableSize(), mInputBufferPool->peakPoolSize(), mExtraInBufferNeeded);

    while( (mExtraInBufferNeeded > 0) &&
           (!this->peakAllQueue()) &&
           (!mNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH)) )
    {
        mInputBufferPool->allocate();
        mExtraInBufferNeeded--;
        MY_LOGD("Extra in buffer allocation remain = %d", mExtraInBufferNeeded);
    }
    TRACE_FUNC_EXIT();
}

MSize WarpNode::toWarpOutSize(const MSizeF &inSize)
{
    TRACE_FUNC_ENTER();
    MSize outSize = MSize(inSize.w + 0.5, inSize.h + 0.5);
    TRACE_FUNC("inSize(%f,%f),outSize(%dx%d)", inSize.w, inSize.h, outSize.w, outSize.h);
    TRACE_FUNC_EXIT();
    return outSize;
}

ImgBuffer WarpNode::prepareOutBuffer(const WarpSrcInfo &src, const WarpDstInfo &dst)
{
    TRACE_FUNC_ENTER();
    ImgBuffer outBuffer;

    if( dst.mNeedTempOut )
    {
        MSize size = mPipeUsage.supportWarpCrop() ? toWarpOutSize(src.mWarpDomain) : src.mInput.mBuffer->getImageBuffer()->getImgSize();
        outBuffer = mOutputBufferPool->requestIIBuffer();
        outBuffer->getImageBuffer()->setExtParam(size);
    }
    else
    {
        if( dst.mOutputBuffer )
        {
            outBuffer = new IIBuffer_IImageBuffer(dst.mOutputBuffer);
        }
        else
        {
            MY_LOGE("Missing record buffer");
        }
    }

    TRACE_FUNC_EXIT();
    return outBuffer;
}

MBOOL WarpNode::needTempOutBuffer(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( mPipeUsage.supportWPE() )
    {
        ret = MTRUE;
    }
    else if( request->useDirectGpuOut() )
    {
        IImageBuffer *recordBuffer = request->getRecordOutputBuffer();
        if( recordBuffer && getGraphicBufferAddr(recordBuffer) )
        {
            ret = MFALSE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MSizeF WarpNode::getWarpOutSize(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return request->getEISCropRegion().s;
}

MVOID WarpNode::applyDZ(const RequestPtr &request, WarpParam &param)
{
    MCrpRsInfo cropInfo;
    MCropRect cropRect;
    if( request->getRecordCrop(cropInfo, RRZO_DOMAIN) ||
        request->getDisplayCrop(cropInfo, RRZO_DOMAIN) )
    {
        cropRect = cropInfo.mCropRect;
    }
    else
    {
        cropRect.s = param.mWarpOut->getImageBuffer()->getImgSize();
    }

    TRACE_FUNC("mIn=%dx%d, OutSize=(%dx%d), rect=(%d,%d)(%dx%d), offset=(%fx%f)",
        param.mInSize.w, param.mInSize.h,
        param.mOutSize.w, param.mOutSize.h,
        cropRect.p_integral.x, cropRect.p_integral.y,
        cropRect.s.w, cropRect.s.h,
        param.mWarpMap.mDomainOffset.w, param.mWarpMap.mDomainOffset.h );

    WarpBase::applyDZWarp(param.mWarpMap.mBuffer, param.mInSize, param.mOutSize,
                          MRect(cropRect.p_integral, cropRect.s),
                          param.mWarpMap.mDomainOffset);
}

MVOID WarpNode::enqueWarpStream(const WarpParam &param, const WarpEnqueData &data)
{
    TRACE_FUNC_ENTER();

    String8 str;
    str += String8::format("in(%dx%d),out(%dx%d),warpout(%fx%f),mdp(size=%d):",
                           param.mInSize.w, param.mInSize.h,
                           param.mOutSize.w, param.mOutSize.h,
                           param.mWarpOutSize.w, param.mWarpOutSize.h,
                           param.mMDPOut.size());
    for( auto &out : param.mMDPOut )
    {
        str += String8::format("crop(%f,%f)(%fx%f).",
            out.mCropRect.p_integral.x, out.mCropRect.p_integral.y, out.mCropRect.s.w, out.mCropRect.s.h);
    }

    MY_LOGD("sensor(%d) Frame %d warp enque start: %s", mSensorIndex, data.mRequest->mRequestNo, str.string());
    data.mRequest->mTimer.startEnqueWarp();
    this->incExtThreadDependency();
    this->enqueWarpStreamBase(mWarpStream, param, data);
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::onWarpStreamBaseCB(const WarpParam &param, const WarpEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing RSCNode class members
    TRACE_FUNC_ENTER();

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopEnqueWarp();
        MY_LOGD("sensor(%d) Frame %d warp enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueWarp(), param.mResult);
        if( !param.mResult )
        {
            MY_LOGW("Frame %d warp enque result failed", request->mRequestNo);
        }
        request->updateResult(param.mResult);

        if( data.mNeedDump || request->needDump() )
        {
            dump(data, param.mMDPOut);
        }

        handleResultData(request);
        request->mTimer.stopWarp();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::handleResultData(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    handleData(ID_WARP_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_FRAME_DONE, request));
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::dump(const WarpEnqueData &data, const WarpParam::OUTPUT_ARRAY &mdpout) const
{
    TRACE_FUNC_ENTER();
    dumpData(data.mRequest, data.mWarpMap, "warp_map");
    dumpData(data.mRequest, data.mSrc, "warp_in");
    dumpData(data.mRequest, data.mDst, "warp_out");
    for( const auto &out : mdpout )
    {
        dumpData(data.mRequest, out.mOutput.mBuffer, "warp_mdp_out_%d", out.mOutput.mPortID.index);
    }
    TRACE_FUNC_EXIT();
}

WarpSrcInfo WarpNode::prepareSrc(const RequestPtr &request, const BasicImg &inputImg)
{
    TRACE_FUNC_ENTER();
    WarpSrcInfo src = extractSrc(request, inputImg);
    EISQ_ACTION action = request->getEISQAction();

    if( mPipeUsage.supportEIS_TSQ() )
    {
        src = processEISQAction<WarpSrcInfo>(action, src, mWarpSrcInfoQueue);
    }

    TRACE_FUNC("Frame %d: tsq=%d recNo=%d qAction=%d qCounter=%d qSize=%zu src=%d", request->mRequestNo, mPipeUsage.supportEIS_TSQ(), request->mRecordNo, request->getEISQAction(), request->getEISQCounter(), mWarpSrcInfoQueue.size(), src.mID);
    TRACE_FUNC_EXIT();
    return src;
}

WarpSrcInfo WarpNode::extractSrc(const RequestPtr &request, const BasicImg &inputImg)
{
    TRACE_FUNC_ENTER();
    WarpSrcInfo src;

    src.mID = request->mRequestNo;
    src.mValid = MTRUE;
    src.mNeedWarp = request->needWarp();

    if( src.mNeedWarp )
    {
        src.mInput = inputImg;
        src.mWarpDomain = getWarpOutSize(request);

        MCrpRsInfo cropInfo;
        MRectF cropInfoF;
        if( request->getRecordCrop(cropInfo, RRZO_DOMAIN, 0, &cropInfoF) )
        {
            src.mCrop = cropInfoF;
        }
        else if( request->getDisplayCrop(cropInfo, RRZO_DOMAIN, 0, &cropInfoF) )
        {
            src.mCrop = cropInfoF;
            MY_LOGD("Suspious no record, use display crop = (%f,%f)(%fx%f)",
                    src.mCrop.p.x, src.mCrop.p.y, src.mCrop.s.w, src.mCrop.s.h);
        }
        TRACE_FUNC("Source crop = (%f,%f)(%fx%f)",
                    src.mCrop.p.x, src.mCrop.p.y, src.mCrop.s.w, src.mCrop.s.h);
    }

    TRACE_FUNC_EXIT();
    return src;
}

WarpDstInfo WarpNode::prepareDst(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    WarpDstInfo dst;
    dst.mID = request->mRequestNo;
    dst.mNeedTempOut = needTempOutBuffer(request);
    dst.mOutputBuffer = request->getRecordOutputBuffer();

    TRACE_FUNC_EXIT();
    return dst;
}

WarpParam::OUTPUT_ARRAY WarpNode::prepareMDPParam(const RequestPtr &request, const WarpSrcInfo &src, MSizeF domainOffset)
{
    WarpParam::OUTPUT_ARRAY mdpOut;
    MRectF cropF = prepareMDPCrop(src, domainOffset);
    MCropRect crop = MCropRect(MPoint(cropF.p.x, cropF.p.y), MSize(cropF.s.w, cropF.s.h) );
    if( request->mQParams.mvFrameParams.size() )
    {
        MBOOL skipDisplay = request->skipMDPDisplay();
        MBOOL useWarpRecord = request->useDirectGpuOut();

        for( unsigned i = 0, n = request->mQParams.mvFrameParams[0].mvOut.size(); i < n; ++i )
        {
            Output out = request->mQParams.mvFrameParams[0].mvOut[i];
            if( isDisplayOutput(out) && !skipDisplay )
            {

            }
            else if( isRecordOutput(out) && !useWarpRecord )
            {

            }
            else
            {
                continue;
            }

            DpPqParam pqParam;
            if( out.mPortID.index == NSImageio::NSIspio::EPortIndex_WDMAO )
            {
                request->getWDMADpPqParam(pqParam);
            }
            else if( out.mPortID.index == NSImageio::NSIspio::EPortIndex_WROTO )
            {
                request->getWROTDpPqParam(pqParam);
            }
            else
            {
                pqParam.scenario = MEDIA_ISP_PREVIEW;
                pqParam.enable = false;
                MY_LOGW("Unknown port.index:%d. Use default PQParam", out.mPortID.index);
            }
            mdpOut.push_back(MDPWrapper::MDPOutput(out, crop, pqParam, cropF));
        }
    }

    if( mdpOut.size() < 1)
    {
        MY_LOGD("No avalible MDP buffer");
    }

    return mdpOut;
}

WarpParam WarpNode::prepareWarpParam(const RequestPtr &request, const WarpSrcInfo &src, WarpDstInfo &dst, const BasicImg &warpMap, MBOOL bypass)
{
    TRACE_FUNC_ENTER();
    WarpParam param;
    param.mIn = src.mInput.mBuffer;
    param.mInSize = param.mIn->getImageBuffer()->getImgSize();

    param.mWarpOut = prepareOutBuffer(src, dst);
    param.mOutSize = param.mWarpOut->getImageBuffer()->getImgSize();

    param.mByPass = bypass;
    param.mWarpMap = warpMap;

    MSizeF domainOffset = bypass ? src.mInput.mDomainOffset :
                                  src.mInput.mDomainOffset + warpMap.mDomainOffset;

    param.mWarpOutSize = getWarpOutSize(request);

    param.mMDPOut = prepareMDPParam(request, src, domainOffset);

    TRACE_FUNC("In(%dx%d),Out(%dx%d),WarpOut(%fx%f),domainOffset(%f,%f)",
               param.mInSize.w, param.mInSize.h,
               param.mOutSize.w, param.mOutSize.h,
               param.mWarpOutSize.w, param.mWarpOutSize.h,
               domainOffset.w, domainOffset.h);

    TRACE_FUNC_EXIT();
    return param;
}

MBOOL WarpNode::processWarp(const RequestPtr &request, const BasicImg &inputImg, const BasicImg &warpMap)
{
    TRACE_FUNC_ENTER();
    WarpEnqueData data;
    WarpParam param;

    WarpSrcInfo srcInfo = prepareSrc(request, inputImg);
    WarpDstInfo dstInfo = prepareDst(request);

    TRACE_FUNC("#%d src=%d dst=%d needWarp=%d valid=%d warpMap=%p", request->mRequestNo, srcInfo.mID, dstInfo.mID, srcInfo.mNeedWarp, srcInfo.mValid, warpMap.mBuffer.get());

    if( !srcInfo.mValid ||
        (srcInfo.mNeedWarp && warpMap.mBuffer == NULL) )
    {
        MY_LOGD("frame %d/%d: drop record valid=%d needWarp=%d warpMap=%p", request->mRequestNo, request->mRecordNo, srcInfo.mValid, srcInfo.mNeedWarp, warpMap.mBuffer.get());
        request->setVar<MBOOL>(VAR_EIS_SKIP_RECORD, MTRUE);
        handleResultData(request);
    }
    else if( !srcInfo.mNeedWarp )
    {
        handleResultData(request);
    }
    else
    {
        param = prepareWarpParam(request, srcInfo, dstInfo, warpMap, request->useWarpPassThrough());
        param.mRequest = data.mRequest = request;
        data.mSrc = param.mIn;
        data.mDst = param.mWarpOut;
        data.mWarpMap = param.mWarpMap;

        if( param.mByPass )
        {
            WarpBase::makePassThroughWarp(param.mWarpMap.mBuffer, param.mInSize, param.mOutSize);
        }

        if( request->useDirectGpuOut() )
        {
            applyDZ(request, param);
        }

        enqueWarpStream(param, data);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MRectF WarpNode::prepareMDPCrop(const WarpSrcInfo &src, const MSizeF &domainOffset)
{
    TRACE_FUNC_ENTER();
    MRectF crop;
    if( src.mCrop.s.w && src.mCrop.s.h )
    {
        crop = src.mCrop;

        if( mPipeUsage.supportWarpCrop() )
        {
            // Refine int-aligned difference
            MSizeF warpOutSize = toWarpOutSize(src.mWarpDomain);
            crop.s += warpOutSize - src.mWarpDomain;
            TRACE_FUNC("MDPCrop(%fx%f)=>(%fx%f)", src.mCrop.s.w, src.mCrop.s.h, crop.s.w, crop.s.h);
        }

        crop.p.x = max(crop.p.x - domainOffset.w, 0.0f);
        crop.p.y = max(crop.p.y - domainOffset.h, 0.0f);

        const double delta = 0.001;
        if( crop.s.w > src.mWarpDomain.w + delta || crop.s.h > src.mWarpDomain.h + delta)
        {
            MSizeF newCrop;
            newCrop.w = min(src.mWarpDomain.w, crop.s.w);
            newCrop.h = min(src.mWarpDomain.h, crop.s.h);
            MY_LOGD("Crop(%fx%f) > WarpDomain(%fx%f), use new Crop = (%fx%f)",
                    crop.s.w, crop.s.h, src.mWarpDomain.w, src.mWarpDomain.h, newCrop.w, newCrop.h);
            crop.s = newCrop;
        }
    }
    else
    {
        crop.s = src.mWarpDomain;
        MY_LOGD("Cannot get source buffer crop, use default crop=(%f,%f)(%fx%f)", crop.p.x, crop.p.y, crop.s.w, crop.s.h);
    }

    TRACE_FUNC("src=(%fx%f)@(%f,%f) offset=(%f,%f) inDomain=(%fx%f) result=(%fx%f)@(%f,%f)",
                src.mCrop.s.w, src.mCrop.s.h,
                src.mCrop.p.x, src.mCrop.p.y,
                domainOffset.w, domainOffset.h,
                src.mWarpDomain.w, src.mWarpDomain.h,
                crop.s.w, crop.s.h,
                crop.p.x, crop.p.y);

    TRACE_FUNC_EXIT();
    return crop;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
