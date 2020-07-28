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
#include <sys/stat.h>
#include <mtkcam3/feature/eis/eis_hal.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_EIS_WARP);

using NSCam::NSIoPipe::Output;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum DumpMaskIndex
{
    MASK_REC_WARP_IN,
    MASK_REC_WARP_OUT,
    MASK_REC_WARP_MAP,
    MASK_REC_MDP_OUT,
    MASK_DISP_WARP_IN,
    MASK_DISP_WARP_OUT,
    MASK_DISP_WARP_MAP,
    MASK_DISP_MDP_OUT,

    MASK_UNKNOWN = 31,
};

const std::vector<DumpFilter> sFilterTable =
{
    DumpFilter( MASK_REC_WARP_IN,       "rec_warp_in" ),
    DumpFilter( MASK_REC_WARP_OUT,      "rec_warp_out"),
    DumpFilter( MASK_REC_WARP_MAP,      "rec_warpmap"),
    DumpFilter( MASK_REC_MDP_OUT,       "rec_mdp_out"),
    DumpFilter( MASK_DISP_WARP_IN,      "disp_warp_in"),
    DumpFilter( MASK_DISP_WARP_OUT,     "disp_warp_out"),
    DumpFilter( MASK_DISP_WARP_MAP,     "disp_warpmap"),
    DumpFilter( MASK_DISP_MDP_OUT,      "disp_mdp_out")
};

enum WARP_FILTER
{
    FILTER_WARP_IN,
    FILTER_WARP_OUT,
    FILTER_WARP_MAP,
    FILTER_MDP_OUT,
};

DumpMaskIndex getMaskIndex(WARP_FILTER myFilter, MBOOL isDisplay)
{
    switch(myFilter)
    {
        case FILTER_WARP_IN:
            return (isDisplay) ? MASK_DISP_WARP_IN : MASK_REC_WARP_IN;
        case FILTER_WARP_OUT:
            return (isDisplay) ? MASK_DISP_WARP_OUT : MASK_REC_WARP_OUT;
        case FILTER_WARP_MAP:
            return (isDisplay) ? MASK_DISP_WARP_MAP : MASK_REC_WARP_MAP;
        case FILTER_MDP_OUT:
            return (isDisplay) ? MASK_DISP_MDP_OUT : MASK_REC_MDP_OUT;
        default:
            MY_LOGF("unsupported Dump Warp Filter %d", myFilter);
            return MASK_UNKNOWN;
    }
}

const char* getDumpPostfix(MBOOL isDisplay)
{
    return (isDisplay) ? "disp" : "rec";
}

WarpNode::WarpNode(const char *name)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_EIS_WARP)
    , StreamingFeatureNode(name)
    , mWarpStream(NULL)
    , mJpegEncodeThread(NULL)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mInputImgDatas);
    this->addWaitQueue(&mWarpController);
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
        id == ID_VMDP_TO_NEXT_FULLIMG )
    {
        this->mInputImgDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpNode::onData(DataID id, const DualBasicImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_VNR_TO_NEXT_FULLIMG )
    {
        this->mInputImgDatas.enque(toBasicImgData(data));
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpNode::onData(DataID id, const WarpControlData &warpData)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", warpData.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_EIS_TO_WARP )
    {
        this->mWarpController.enque(warpData);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType WarpNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &reqInfo) const
{
    IOPolicyType policy = IOPOLICY_BYPASS;

    if( HAS_EIS(reqInfo.mFeatureMask) && reqInfo.isMaster() )
    {
        policy = IOPOLICY_INOUT_QUEUE;
    }

    return policy;
}

MBOOL WarpNode::getInputBufferPool(const StreamingReqInfo &reqInfo, android::sp<IBufferPool>& pool, NextFullInfo &info)
{
    MBOOL ret = MFALSE;
    info.mResize = reqInfo.mRRZOsize;
    info.mNeedCrop = MFALSE;
    info.mCanShare = MFALSE;
    if( mInputBufferPool != NULL )
    {
        pool = mInputBufferPool;
        info.mCanShare = mUseSharedInput;
        ret = MTRUE;
    }

    return ret;
}

MBOOL WarpNode::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    StreamingFeatureNode::onInit();
    enableDumpMask(0xffff, sFilterTable);

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
    mForceExpectMS = getPropertyValue("vendor.debug.fpipe.warp.expect", 0);
    mEncodePreviewBuf = property_get_int32("vendor.debug.eis.encodepreview", 0);
    mDumpDisplay = property_get_int32("vendor.debug.eis.dumpdisplay", 0);
    mUseSharedInput = mPipeUsage.supportSharedInput();
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mInputBufferPool = NULL;
    mOutputBufferPool = NULL;
    mJpegEncodeThread = NULL;
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
        MUINT32 inputPoolSize = mPipeUsage.getNumWarpInBuffer() + mPipeUsage.getNumDSDNBuffer();
        MUINT32 outputPoolSize = mPipeUsage.getNumWarpOutBuffer();
        mExtraInBufferNeeded = mPipeUsage.getNumExtraWarpInBuffer();
        Timer timer(MTRUE);

        mInputBufferPool->allocate(inputPoolSize);
        mOutputBufferPool->allocate(outputPoolSize);
        mNodeSignal->setSignal(NodeSignal::SIGNAL_GPU_READY);
        MY_LOGD("WarpNode %s (%d+%d) buf in %d ms", STR_ALLOCATE, inputPoolSize, outputPoolSize, timer.getNow());

        MY_LOGI("init WarpStream ++");
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "WarpStream Init");
        ret = mWarpStream->init(mSensorIndex, mInputBufferPool->getImageSize(), MAX_WARP_SIZE);
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        MY_LOGI("init WarpStream --");
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
    BasicImgData inBuffer;
    std::queue<BasicImgData> warpMapQueue;
    WarpControlData warpData;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mInputImgDatas.deque(inBuffer) )
    {
        MY_LOGE("InputImgData deque out of sync");
        return MFALSE;
    }
    else if (!mWarpController.deque(warpData))
    {
        MY_LOGE("WarpMapData deque out of sync");
        return MFALSE;
    }
    if( warpData.mRequest != inBuffer.mRequest )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();
    request = warpData.mRequest;
    request->mTimer.startWarp();
    TRACE_FUNC("Frame %d in WarpNode needWarp=%d", request->mRequestNo, request->needWarp());

    MUINT32 cycleMs = getMaxCycleTimeMs(request, warpData.mData);
    WarpSrcInfo dispSrcInfo = extractSrc(request, inBuffer.mData);
    WarpSrcInfo recSrcInfo = prepareRecordSrc(request, dispSrcInfo);
    if (warpData.mData.hasWarpmap(WarpController::TYPE_DISPLAY))
    {
        processWarp(request, warpData.mData, dispSrcInfo, MTRUE, cycleMs);
    }
    processWarp(request, warpData.mData, recSrcInfo, MFALSE, cycleMs);

    request->mTimer.stopWarp();

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

ImgBuffer WarpNode::prepareOutBuffer(const WarpSrcInfo &src, const WarpDstInfo &dst, const MSizeF &warpOutSize)
{
    TRACE_FUNC_ENTER();
    ImgBuffer outBuffer;

    if( dst.mNeedTempOut )
    {
        MSize size = mPipeUsage.supportWarpCrop() ? toWarpOutSize(warpOutSize) : src.mInput.mBuffer->getImageBuffer()->getImgSize();
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

#if 0
MVOID WarpNode::applyDZ(const RequestPtr &request, WarpParam &param)
{
    MRectF cropRect;
    OutputInfo info;
    if( request->getOutputInfo(IO_TYPE_RECORD, info) ||
        request->getOutputInfo(IO_TYPE_DISPLAY, info) )
    {
        cropRect = info.mCropRect;
    }
    else
    {
        cropRect.s = MSizeF(param.mWarpOut->getImageBuffer()->getImgSize().w,
                            param.mWarpOut->getImageBuffer()->getImgSize().h);
    }

    TRACE_FUNC("mIn=%dx%d, OutSize=(%dx%d), rect=(%d,%d)(%dx%d), offset=(%fx%f)",
        param.mInSize.w, param.mInSize.h,
        param.mOutSize.w, param.mOutSize.h,
        cropRect.p.x, cropRect.p.y,
        cropRect.s.w, cropRect.s.h,
        param.mWarpMap.mDomainOffset.x, param.mWarpMap.mDomainOffset.y );

    WarpBase::applyDZWarp(param.mWarpMap.mBuffer, param.mInSize, param.mOutSize,
                          cropRect, MSizeF(param.mWarpMap.mDomainOffset.x, param.mWarpMap.mDomainOffset.y)/*MPointF is better?*/);
}
#endif

MVOID WarpNode::enqueWarpStream(const WarpParam &param, const WarpEnqueData &data)
{
    TRACE_FUNC_ENTER();
    String8 str;
    str += String8::format("in(%dx%d),out(%dx%d),warpout(%fx%f),mdp(size=%zu):",
                           param.mInSize.w, param.mInSize.h,
                           param.mOutSize.w, param.mOutSize.h,
                           param.mWarpOutSize.w, param.mWarpOutSize.h,
                           param.mMDPOut.size());
    for( auto &out : param.mMDPOut )
    {
        str += String8::format("crop(%f,%f)(%fx%f).",
            out.mCropRect.p.x, out.mCropRect.p.y, out.mCropRect.s.w, out.mCropRect.s.h);
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
    // avoid accessing WarpNode class members
    TRACE_FUNC_ENTER();

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopEnqueWarp();
        MY_LOGD("sensor(%d) Frame %d warp enque done in %d ms, isDisplay=%d result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueWarp(),
            param.mIsDisplay, param.mResult);
        if( !param.mResult )
        {
            MY_LOGW("Frame %d warp enque result failed", request->mRequestNo);
        }
        request->updateResult(param.mResult);

        if( data.mNeedDump || request->needDump() )
        {
            dump(data, param.mMDPOut, param.mIsDisplay);
        }

        handleResultData(request, param.mMsgType);

        request->mTimer.stopWarp();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::handleResultData(const RequestPtr &request, FeaturePipeParam::MSG_TYPE msgType)
{
    TRACE_FUNC_ENTER();
    handleData(ID_WARP_TO_HELPER, HelperData(msgType, request));
    TRACE_FUNC_EXIT();
}

MVOID WarpNode::dump(const WarpEnqueData &data, const MDPWrapper::P2IO_OUTPUT_ARRAY &mdpout, MBOOL isDisplay) const
{
    TRACE_FUNC_ENTER();
    if( allowDump(getMaskIndex(FILTER_WARP_MAP, isDisplay)) )
    {
        dumpData(data.mRequest, data.mWarpMap.mBuffer, "warp_map-%s", getDumpPostfix(isDisplay));
    }
    if( allowDump(getMaskIndex(FILTER_WARP_IN, isDisplay)) )
    {
        dumpData(data.mRequest, data.mSrc, "warp_in-%s", getDumpPostfix(isDisplay));
    }
    if( allowDump(getMaskIndex(FILTER_WARP_OUT, isDisplay)) )
    {
        dumpData(data.mRequest, data.mDst, "warp_out-%s", getDumpPostfix(isDisplay));
    }
    if( allowDump(getMaskIndex(FILTER_MDP_OUT, isDisplay)) )
    {
        unsigned i = 0;
        for( const auto &out : mdpout )
        {
            dumpData(data.mRequest, out.mBuffer, "warp_mdp_out_%d-%s", i, getDumpPostfix(isDisplay));
            ++i;
        }
    }
    TRACE_FUNC_EXIT();
}

WarpSrcInfo WarpNode::prepareRecordSrc(const RequestPtr &request, const WarpSrcInfo &dispSrc)
{
    TRACE_FUNC_ENTER();
    WarpSrcInfo src = dispSrc;

    if( mPipeUsage.supportEIS_TSQ() )
    {
        EISQ_ACTION action = request->getEISQAction();
        src = processEISQAction<WarpSrcInfo>(action, dispSrc, mWarpSrcInfoQueue);
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
    src.mP2Pack = request->mP2Pack;

    if( src.mNeedWarp )
    {
        src.mInput = inputImg;
        OutputInfo info;
        if( request->getOutputInfo(IO_TYPE_RECORD, info) || request->getOutputInfo(IO_TYPE_DISPLAY, info) )
        {
            src.mCrop = info.mCropRect;
        }
        else
        {
            MY_S_LOGW(request->mLog, "Can not find display & record crop !!");
        }

        TRACE_FUNC("Source crop = (%f,%f)(%fx%f)",
                   src.mCrop.p.x, src.mCrop.p.y, src.mCrop.s.w, src.mCrop.s.h);
    }

    TRACE_FUNC_EXIT();
    return src;
}

WarpDstInfo WarpNode::prepareDst(const RequestPtr &request, MBOOL isDisplay)
{
    TRACE_FUNC_ENTER();
    WarpDstInfo dst;
    dst.mID = request->mRequestNo;
    dst.mNeedTempOut = needTempOutBuffer(request);
    dst.mOutputBuffer = request->getRecordOutputBuffer();
    dst.mIsDisplay = isDisplay;

    TRACE_FUNC_EXIT();
    return dst;
}

MDPWrapper::P2IO_OUTPUT_ARRAY WarpNode::prepareMDPParam(const RequestPtr &request, const WarpSrcInfo &src,
    const WarpDstInfo &dst, const WarpImg &warpMap, WarpEnqueData &data, const DomainTransform &transform)
{

    MBOOL skipDisplay = request->skipMDPDisplay();
    MBOOL useWarpRecord = request->useDirectGpuOut();
    MDPWrapper::P2IO_OUTPUT_ARRAY outList;
    P2IO out;
    std::vector<P2IO> extOuts;
    MRectF displayCrop;

    if(dst.mIsDisplay
        && !skipDisplay)
    {
        if (request->popDisplayOutput(this, out) && out.isValid())
        {
            MRectF orignalCrop = out.mCropRect;
            MRectF newCrop = transform.applyTo(orignalCrop);
            out.mCropRect = refineMDPCrop(newCrop, warpMap.mInputCrop.s, request);
            displayCrop = out.mCropRect;
            outList.push_back(out);
            out.mBuffer->setTimestamp(src.mInput.mBuffer->getImageBuffer()->getTimestamp());

            TRACE_FUNC("[Display]MDP Crop before transform=" MCropF_STR ",after=" MCropF_STR,
                       MCropF_ARG(orignalCrop), MCropF_ARG(out.mCropRect));
        }

        if (request->popExtraOutputs(this, extOuts))
        {
            for (size_t i = 0; i < extOuts.size(); i++)
            {
                P2IO extOut = extOuts[i];
                MRectF orignalCrop = extOut.mCropRect;
                MRectF newCrop = transform.applyTo(orignalCrop);
                extOut.mCropRect = refineMDPCrop(newCrop, warpMap.mInputCrop.s, request);
                outList.push_back(extOut);
                if(extOut.isValid())
                {
                    extOut.mBuffer->setTimestamp(src.mInput.mBuffer->getImageBuffer()->getTimestamp());
                }
                else
                {
                    MY_LOGF("Ext output %zu is NULL!", i);
                }

                TRACE_FUNC("[Extra]MDP Crop before transform=" MCropF_STR ",after=" MCropF_STR,
                           MCropF_ARG(orignalCrop), MCropF_ARG(extOut.mCropRect));
            }
        }

        //Workaround for preview source dump
        if (mEncodePreviewBuf)
        {
            if (request->popRecordOutput(this, out) && out.isValid())
            {
                out.mCropRect = displayCrop;
                outList.push_back(out);
                out.mBuffer->setTimestamp(src.mInput.mBuffer->getImageBuffer()->getTimestamp());
            }
        }
    }

    if (!mEncodePreviewBuf)
    {
        if(!dst.mIsDisplay
            && !useWarpRecord
            && request->popRecordOutput(this, out) && out.isValid())
        {
            Feature::P2Util::MDPObjPtr mdpObj;
            mdpObj.isExtraRecSrc = true;

            MRectF newCrop = transform.applyTo(src.mCrop); // Get queued crop from srcInfo
            out.mCropRect = refineMDPCrop(newCrop, warpMap.mInputCrop.s, request);
            out.mDpPqParam = Feature::P2Util::makeDpPqParam(&(data.mRunData->mP2Obj.pqWDMA), src.mP2Pack, NSIoPipe::EPortCapbility_Rcrd, mdpObj);
            out.mBuffer->setTimestamp(src.mInput.mBuffer->getImageBuffer()->getTimestamp());
            outList.push_back(out);

            TRACE_FUNC("[Record]MDP Crop before transform=" MCropF_STR ",after=" MCropF_STR,
                       MCropF_ARG(src.mCrop), MCropF_ARG(out.mCropRect));
        }
    }

    if( outList.empty() )
    {
        MY_LOGD("No avalible MDP buffer");
    }

    return outList;
}

WarpParam WarpNode::prepareWarpParam(const RequestPtr &request, const WarpSrcInfo &src, WarpDstInfo &dst,
    const WarpController &warpData, WarpEnqueData &data, MBOOL isDisplay, MUINT32 cycleMs, MBOOL bypass)
{
    TRACE_FUNC_ENTER();
    WarpImg warpMap = isDisplay ? warpData.getWarpmap(WarpController::TYPE_DISPLAY) : warpData.getWarpmap(WarpController::TYPE_RECORD);
    if(MSizeF(src.mInput.mBuffer->getImgSize()) != warpMap.mInputSize)
    {
        MSize inSize = src.mInput.mBuffer->getImgSize();
        MY_S_LOGE(request->mLog, "WarpMap ERROR!, warpTargetInput(" MSizeF_STR ") != input(" MSize_STR "), warpCrop(" MCropF_STR ")",
                    MSizeF_ARG(warpMap.mInputSize), MSize_ARG(inSize), MCropF_ARG(warpMap.mInputCrop));
    }

    WarpParam param;
    param.mIn = src.mInput.mBuffer;
    param.mInSize = param.mIn->getImageBuffer()->getImgSize();

    param.mWarpOutSize = warpMap.mInputCrop.s;
    param.mWarpOut = prepareOutBuffer(src, dst, param.mWarpOutSize);
    param.mOutSize = param.mWarpOut->getImageBuffer()->getImgSize();

    param.mByPass = bypass;
    param.mIsDisplay = isDisplay;
    param.mWarpMap = warpMap;
    param.mEndTimeMs = cycleMs;
    if (isDisplay)
    {
        param.mMsgType = warpData.hasWarpmap(WarpController::TYPE_RECORD) ? FeaturePipeParam::MSG_DISPLAY_DONE : FeaturePipeParam::MSG_FRAME_DONE;
        if (mEncodePreviewBuf)
        {
            MY_LOGD("Store warped preview output in record buffer, sent MSG_FRAME_DONE for display");
            param.mMsgType = FeaturePipeParam::MSG_FRAME_DONE;
        }
    }
    else
    {
        param.mMsgType = FeaturePipeParam::MSG_FRAME_DONE;
    }

    DomainTransform transform = src.mInput.mTransform;
    if(!bypass)
    {
        const char* str = isDisplay ? "warp-dis" : "warp-rec";
        transform.accumulate(str, request->mLog, warpMap.mInputSize, warpMap.mInputCrop, warpMap.mInputCrop.s);
    }

    param.mMDPOut = prepareMDPParam(request, src, dst, warpMap, data, transform);

    TRACE_FUNC("In(%dx%d),Out(%dx%d),WarpOut(%fx%f),transform=" MTransF_STR,
               param.mInSize.w, param.mInSize.h,
               param.mOutSize.w, param.mOutSize.h,
               param.mWarpOutSize.w, param.mWarpOutSize.h,
               MTransF_ARG(transform.mOffset, transform.mScale));
    TRACE_FUNC_EXIT();
    return param;
}

MBOOL WarpNode::processWarp(const RequestPtr &request, const WarpController &warpData, const WarpSrcInfo &srcInfo,
    MBOOL isDisplay, MUINT32 cycleMs)
{
    TRACE_FUNC_ENTER();
    WarpEnqueData data;
    WarpParam param;
    WarpImg warpMap = isDisplay ? warpData.getWarpmap(WarpController::TYPE_DISPLAY) : warpData.getWarpmap(WarpController::TYPE_RECORD);

    TRACE_FUNC("#%d src=%d isDisp=%d needWarp=%d valid=%d warpMap=%p", request->mRequestNo, srcInfo.mID, isDisplay, srcInfo.mNeedWarp, srcInfo.mValid, warpMap.mBuffer.get());

    if( !srcInfo.mValid ||
        (srcInfo.mNeedWarp && warpMap.mBuffer == NULL) )
    {
        MY_LOGD("frame %d/%d: drop record valid=%d needWarp=%d warpMap=%p", request->mRequestNo, request->mRecordNo, srcInfo.mValid, srcInfo.mNeedWarp, warpMap.mBuffer.get());

        if (!mEncodePreviewBuf)
        {
            request->setVar<MBOOL>(SFP_VAR::EIS_SKIP_RECORD, MTRUE);
        }

        if (!warpData.hasWarpmap(WarpController::TYPE_DISPLAY))
        {
            handleResultData(request, FeaturePipeParam::MSG_FRAME_DONE);
        }
    }
    else if( !srcInfo.mNeedWarp )
    {
        MY_LOGD("frame %d/%d: bypass", request->mRequestNo, request->mRecordNo);
        handleResultData(request, FeaturePipeParam::MSG_FRAME_DONE);
    }
    else
    {
        //Workaround for preview source dump
        if ( !mEncodePreviewBuf || isDisplay )
        {
            // zzz warpDomain should be also queue
            data.mRunData = new WarpRunData();
            WarpDstInfo dstInfo = prepareDst(request, isDisplay);
            param = prepareWarpParam(request, srcInfo, dstInfo, warpData, data, isDisplay, cycleMs, request->useWarpPassThrough());
            param.mRequest = data.mRequest = request;
            data.mSrc = param.mIn;
            data.mDst = param.mWarpOut;
            data.mWarpMap = param.mWarpMap;
            MBOOL needDump = mDumpDisplay ? isDisplay : !isDisplay;

            if (request->getEISDumpInfo().dumpIdx >= 1 && needDump)
            {
                EISSourceDumpInfo info = request->getEISDumpInfo();
                if (request->getEISDumpInfo().dumpIdx == 1)
                {
                    mJpegEncodeThread = NULL;
                    mJpegEncodeThread = JpegEncodeThread::getInstance(param.mMDPOut[0].mCropRect.s, info.filePath.c_str());
                }
                if (info.markFrame)
                {
                    mJpegEncodeThread->compressJpeg(srcInfo.mInput.mBuffer, true);
                }
                else
                {
                    mJpegEncodeThread->compressJpeg(srcInfo.mInput.mBuffer, false);
                }
            }

            if( param.mByPass )
            {
                WarpBase::makePassThroughWarp(param.mWarpMap.mBuffer, param.mInSize, param.mOutSize);
            }

            if( request->useDirectGpuOut() )
            {
                //applyDZ(request, param);
                MY_LOGE("Currently not support warp direct GPU out !!!!! ERROR!!!");
            }

            enqueWarpStream(param, data);
        }
        else
        {
            //Do not process record warp
        }
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MRectF WarpNode::refineMDPCrop(const MRectF &crop, const MSizeF &warpOutSize, const RequestPtr &request)
{
    MRectF outCrop = crop;
    if( mPipeUsage.supportWarpCrop() )
    {
        // Refine int-aligned difference
        MSizeF largeSize = toWarpOutSize(warpOutSize);
        outCrop.s += largeSize - warpOutSize;
    }

    refineBoundaryF("WarpNode", warpOutSize, outCrop, request->needPrintIO());
    return outCrop;
}

MUINT32 WarpNode::getMaxCycleTimeMs(const RequestPtr &request, const WarpController &warpData)
{
    MUINT32 cycle = 0;
    if( warpData.hasWarpmap(WarpController::TYPE_DISPLAY) ) cycle += 1;
    if( warpData.hasWarpmap(WarpController::TYPE_RECORD) ) cycle += 1;

    MUINT32 expectMs = (mForceExpectMS > 0) ? mForceExpectMS : request->getNodeCycleTimeMs();
    MUINT32 maxTimeMs = expectMs / std::max<MUINT32>(1, cycle);
    MY_S_LOGD_IF(request->mLog.getLogLevel(), request->mLog, "expect(%d) cycle(%d) cycleMs(%d)", expectMs, cycle, maxTimeMs);
    return maxTimeMs;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
