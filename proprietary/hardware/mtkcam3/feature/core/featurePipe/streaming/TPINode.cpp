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

#include "TPINode.h"
#include "MDPWrapper.h"

#define PIPE_CLASS_TAG "TPINode"
#define PIPE_TRACE TRACE_TPI_NODE
#include <featurePipe/core/include/PipeLog.h>

#include "tpi/TPIMgr_Util.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_TPI);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
enum DumpMaskIndex
{
    MASK_SFP_INPUT,
    MASK_SFP_OUTPUT,
    MASK_TP_WORKING,
};


const std::vector<DumpFilter> sFilterTable =
{
    DumpFilter( MASK_SFP_INPUT,     "sfp_in" ),
    DumpFilter( MASK_SFP_OUTPUT,    "sfp_out"),
    DumpFilter( MASK_TP_WORKING,    "tp_working")
};

TPINode::TPINode(const char* name, MUINT32 index)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_TPI)
    , StreamingFeatureNode(name)
    , mTPINodeIndex(index)
    , mPoolID(mTPINodeIndex+1)
    , mForceErase(MFALSE)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mData);
    TRACE_FUNC_EXIT();
}

TPINode::~TPINode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID TPINode::setTPIMgr(TPIMgr *tpiMgr)
{
    TRACE_FUNC_ENTER();
    mTPIMgr = tpiMgr;
    TRACE_FUNC_EXIT();
}

MVOID TPINode::setSharedBufferPool(const sp<SharedBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mSharedBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL TPINode::onData(DataID id, const BasicImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived TP[%d]", data.mRequest->mRequestNo, ID2Name(id), mTPINodeIndex);
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_VENDOR_FULLIMG ||
        id == ID_BOKEH_TO_VENDOR_FULLIMG ||
        id == ID_VENDOR_TO_NEXT )
    {
        mData.enque(TPIData(data.mData, data.mRequest));
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::onData(DataID id, const DualBasicImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived TP[%d]", data.mRequest->mRequestNo, ID2Name(id), mTPINodeIndex);
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_VENDOR_FULLIMG ||
        id == ID_VNR_TO_NEXT_FULLIMG )
    {
        mData.enque(TPIData(data.mData, data.mRequest));
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::onData(DataID id, const TPIData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived TP[%d]", data.mRequest->mRequestNo, ID2Name(id), mTPINodeIndex);
    MBOOL ret = MFALSE;
    if( id == ID_VENDOR_TO_NEXT )
    {
        mData.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::onData(DataID id, const DepthImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived TP[%d]", data.mRequest->mRequestNo, ID2Name(id), mTPINodeIndex);
    MBOOL ret = MFALSE;
    if( id == ID_DEPTH_TO_VENDOR )
    {
        mData.enque(TPIData(data.mData.mCleanYuvImg, data.mRequest));
        mDepthDatas.enque(data);
        ret = MTRUE;
    }
    else if( id == ID_TOF_TO_NEXT )
    {
        mDepthDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType TPINode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &reqInfo) const
{
    IOPolicyType policy = IOPOLICY_BYPASS;
    if( HAS_TPI_YUV(reqInfo.mFeatureMask) )
    {
        policy = IOPOLICY_INOUT_EXCLUSIVE;
        if( mPipeUsage.supportDepthP2() && !reqInfo.isMaster() )
        {
            policy = IOPOLICY_BYPASS; // Currently Depth Node not support output Slave YUV
        }
    }
    return policy;
}

MBOOL TPINode::getInputBufferPool(const StreamingReqInfo &/*reqInfo*/, android::sp<IBufferPool> &pool, NextFullInfo &info)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    info.mResize = MSize(0, 0);
    info.mNeedCrop = MFALSE;
    info.mCanShare = MFALSE;
    if( mSharedBufferPool != NULL )
    {
        pool = mSharedBufferPool;
        info.mResize = mCustomSize;
        info.mNeedCrop = mIsInputCrop;
        info.mCanShare = mUseSharedInput;
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    StreamingFeatureNode::onInit();
    enableDumpMask((MASK_SFP_INPUT + MASK_SFP_OUTPUT), sFilterTable, "fpipe.tpi");

    ret = mTPIMgr &&
          (mSharedBufferPool != NULL) &&
          initSettings();

    if( mWaitDepthData )
    {
        this->addWaitQueue(&mDepthDatas);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::onUninit()
{
    TRACE_FUNC_ENTER();
    clearQueue();
    mTPIMgr = NULL;
    mSharedBufferPool = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = initPlugin();
    prepareBufferPool();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = uninitPlugin();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    TPIData data;
    DepthImgData depthData;
    RequestPtr request;
    MBOOL need = MFALSE, bypass = MFALSE;
    TPIRes in, out;
    VMDPReq::RecordInfo recInfo;
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
    if( mWaitDepthData )
    {
        if( !mDepthDatas.deque(depthData) )
        {
            MY_LOGE("DepthData deque out of sync");
            return MFALSE;
        }
        if( data.mRequest != depthData.mRequest )
        {
            MY_LOGE("DepthData out of sync");
            return MFALSE;
        }
        data.mData.add(depthData.mData);
    }

    TRACE_FUNC_ENTER();
    request = data.mRequest;
    in = data.mData;
    need = request->needTPIYuv();
    bypass = request->needTPIBypass();
    request->mTimer.startTPI(mTPINodeIndex);
    TRACE_FUNC("Frame %d in Vendor", request->mRequestNo);
    MY_LOGD("TP[%d] sensor(%d) Frame %d process start needTPI:%d div:%d inplace:%d bypass:%d", mTPINodeIndex, mSensorIndex, request->mRequestNo, need, mIsDivNode, mIsInplace, bypass);

    MBOOL result = MFALSE;
    initInRes(request, in);
    dumpInBuffer(request, in);
    if( need )
    {
        prepareInRes(request, in);
        prepareOutRes(request, in, out);
        result = process(request, in, out, recInfo);
    }
    MY_LOGD("TP[%d] sensor(%d) Frame %d process done in %d ms, result:%d div:%d inplace:%d bypass:%d", mTPINodeIndex, mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueTPI(mTPINodeIndex), result, mIsDivNode, mIsInplace, bypass);

    if( !result )
    {
        handleBypass(request, in, out);
    }
    dumpOutBuffer(request, out);
    handleResultData(request, out, recInfo);
    request->mTimer.stopTPI(mTPINodeIndex);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::initSettings()
{
    TRACE_FUNC_ENTER();
    mForceErase = mPipeUsage.supportVendorErase();
    mTPIUsage = mPipeUsage.getTPIUsage();
    mTPINodeIO = mTPIUsage.getTPIO(TPIOEntry::YUV, mTPINodeIndex);
    mIsFirstTPI = (mTPINodeIndex == 0);
    mIsLastTPI = (mTPINodeIndex+1) == mTPIUsage.getNodeCount(TPIOEntry::YUV);
    mIsInplace = !!(mTPINodeIO.mNodeInfo.mNodeOption & TPI_NODE_OPT_INPLACE);
    mIsDivNode = !!(mTPINodeIO.mNodeInfo.mNodeOption & TPI_NODE_OPT_DIV);
    mIsDivPath = mTPIUsage.use(TPIOEntry::YUV, TPIOUse::DIV);;
    mUseDivDisplay = mTPIUsage.use(TPIOEntry::YUV, TPIOUse::OUT_DISPLAY);
    mIsInputCrop = mTPIUsage.use(TPIOEntry::YUV, TPIOUse::INPUT_CROP);
    mQueueCount = mTPINodeIO.mNodeInfo.mQueueCount;
    mWaitDepthData = mIsFirstTPI &&
                     (mPipeUsage.supportTOF() ||
                     (mPipeUsage.supportDepthP2() && !mPipeUsage.supportBokeh()));
    mInputInfo.mFormat = mSharedBufferPool->getImageFormat();
    mInputInfo.mSize = mSharedBufferPool->getImageSize();
    mInputInfo.mStride = mInputInfo.mSize.w;
    mCustomSize = mTPIUsage.getCustomSize(TPIOEntry::YUV, MSize(0,0));
    mUseSharedInput = mIsFirstTPI && mPipeUsage.supportSharedInput();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::initPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI:initPlugin");
    if( mTPIMgr )
    {
        ret = mTPIMgr->initNode(mTPINodeIO.mNodeInfo.mNodeID, getTPIConfig());
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    MY_LOGI("TP[%d]:initNode ret=%d", mTPINodeIndex, ret);
    TRACE_FUNC_EXIT();
    return ret;
}

TPI_ConfigInfo TPINode::getTPIConfig() const
{
    TRACE_FUNC_ENTER();
    TPI_ConfigInfo config;
    config.mBufferInfo = mInputInfo;
    TRACE_FUNC_EXIT();
    return config;
}

MBOOL TPINode::prepareBufferPool()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    const MUINT32 BASE = 3;
    if( mSharedBufferPool != NULL )
    {
        MUINT32 maxSensorNum = std::min<MUINT32>(mPipeUsage.getNumSensor(), 2);
        if( mIsFirstTPI )
        {
            MUINT32 count = BASE;
            count = BASE * maxSensorNum;
            count += mPipeUsage.getNumDSDNBuffer();
            allocate("fpipe.tpi.in", mSharedBufferPool, count);
            mSharedBufferPool->addQuota(0, count);
        }

        MUINT32 inCount = 0, outCount = 0;
        inCount = maxSensorNum + mQueueCount;
        outCount = BASE + (mIsLastTPI ? 1 : 0);
        outCount = outCount * mTPINodeIO.mNodeInfo.mBufferInfoListCount;
        char str[256];
        snprintf(str, sizeof(str), "fpipe.tpi[%d].in", mTPINodeIndex);
        allocate(str, mSharedBufferPool, inCount);
        snprintf(str, sizeof(str), "fpipe.tpi[%d].out", mTPINodeIndex);
        allocate(str, mSharedBufferPool, outCount);
        mSharedBufferPool->addQuota(mPoolID, inCount + outCount);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::uninitPlugin()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI:uninitPlugin");
    if( mTPIMgr )
    {
        ret = mTPIMgr->uninitNode(mTPINodeIO.mNodeID);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    MY_LOGI("TP[%d]:uninitNode ret=%d mgr=%p", mTPINodeIndex, ret, mTPIMgr);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL TPINode::initInRes(const RequestPtr &request, TPIRes &in)
{
    TRACE_FUNC_ENTER();
    if( mIsFirstTPI )
    {
        BasicImg inYuv, inYuv2, inDepth, inDepthIntensity;
        inYuv = in.getSFP(TPI_BUFFER_ID_MTK_YUV);
        inYuv2 = in.getSFP(TPI_BUFFER_ID_MTK_YUV_2);
        inDepth = in.getSFP(TPI_BUFFER_ID_MTK_DEPTH);
        inDepthIntensity = in.getSFP(TPI_BUFFER_ID_MTK_DEPTH_INTENSITY);
        in.setTP(TPI_BUFFER_ID_MTK_YUV, inYuv); // for 1st node bypass
        in.setTP(TPI_BUFFER_ID_MTK_OUT_YUV, inYuv);
        in.setTP(TPI_BUFFER_ID_MTK_OUT_YUV_2, inYuv2);
        in.setTP(TPI_BUFFER_ID_MTK_OUT_DEPTH, inDepth);
        in.setTP(TPI_BUFFER_ID_MTK_OUT_DEPTH_INTENSITY, inDepthIntensity);
        request->getTPIMeta(in);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::prepareInRes(const RequestPtr &request, TPIRes &in)
{
    TRACE_FUNC_ENTER();
    if( mIsFirstTPI )
    {
        BasicImg inYuv, inYuv2, inDepth, inDepthIntensity;
        inYuv = in.getSFP(TPI_BUFFER_ID_MTK_YUV);
        inYuv2 = in.getSFP(TPI_BUFFER_ID_MTK_YUV_2);
        inYuv.syncCache(eCACHECTRL_INVALID);
        inYuv2.syncCache(eCACHECTRL_INVALID);
    }
    updateBufferOwner(request, in.mSFP);
    updateBufferOwner(request, in.mTP);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::prepareOutRes(const RequestPtr &request, const TPIRes &in, TPIRes &out)
{
    TRACE_FUNC_ENTER();
    out.mMeta = in.mMeta;
    out.mSFP = in.mSFP;
    for( auto &prev : mTPINodeIO.mPrevPortMap )
    {
        unsigned inID = prev.first;
        auto first = prev.second.begin();
        if( first != prev.second.end() )
        {
            unsigned outID = first->mPort;
            out.setTP(inID, in.getTP(outID));
        }
    }

    // use main1 input as output buffer initialize
    BasicImg inYuv = out.getTP(TPI_BUFFER_ID_MTK_YUV);
    out.setZoomROI(inYuv.mTransform.applyTo(request->getZoomROI()));
    for( unsigned i = 0; i < mTPINodeIO.mNodeInfo.mBufferInfoListCount; ++i )
    {
        BasicImg buffer;
        unsigned id = mTPINodeIO.mNodeInfo.mBufferInfoList[i].mBufferID;
        if( mIsInplace && id == TPI_BUFFER_ID_MTK_OUT_YUV )
        {
            out.setTP(TPI_BUFFER_ID_MTK_OUT_YUV, inYuv);
        }
        else if( id != TPI_BUFFER_ID_MTK_OUT_RECORD ||
                 needOutputRecord(request) )
        {
            buffer.mBuffer = mSharedBufferPool->requestIIBuffer(mPoolID);
            buffer.setAllInfo(inYuv);
            if( mForceErase )
            {
                buffer.mBuffer->erase();
            }
            out.setTP(id, buffer);
        }
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::updateBufferOwner(const RequestPtr &request, TPIRes::BasicImgMap &map)
{
    (void)request;
    TRACE_FUNC_ENTER();
    if( mSharedBufferPool != NULL )
    {
        for( auto &it : map )
        {
            mSharedBufferPool->changeOwner(mPoolID, it.second.mBuffer);
        }
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TPINode::process(const RequestPtr &request, const TPIRes &in, TPIRes &out, VMDPReq::RecordInfo &recInfo)
{
    (void)request;
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    BasicImg inputYuv = out.getTP(TPI_BUFFER_ID_MTK_YUV);

    TPI_Meta meta[32];
    TPI_Image img[32];
    TPI_Data tpiData;
    unsigned imgCount = out.getImgArray(img, 32);
    unsigned metaCount = out.getMetaArray(meta, 32);
    dumpLog(request, in, out, meta, metaCount, img, imgCount);

    request->mTimer.startEnqueTPI(mTPINodeIndex);
    P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "3rdParty plugin");
    if( !request->needTPIBypass(mTPINodeIO.mNodeInfo.mNodeID) )
    {
        ret = mTPIMgr->enqueNode(mTPINodeIO.mNodeInfo.mNodeID, request->mRequestNo, meta, metaCount, img, imgCount, tpiData);
    }
    P2_CAM_TRACE_END(TRACE_DEFAULT);
    request->mTimer.stopEnqueTPI(mTPINodeIndex);

    filterResult(request, img, imgCount, tpiData);
    processSrcInplace(request, img, imgCount, out);

    if( mIsDivNode )
    {
        processRecordQ(request, inputYuv, tpiData, recInfo, out);
    }

    out.updateViewInfo(getName(), request->mLog, img, imgCount);
    dumpLog(request, in, out, meta, metaCount, img, imgCount);

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID TPINode::filterResult(const RequestPtr &request, TPI_Image img[], unsigned imgCount, TPI_Data &result)
{
    TRACE_FUNC_ENTER();
    (void)request;
    (void)img;
    (void)imgCount;
    MBOOL useQueueCmd = mIsDivNode;
#if 0
    MBOOL useImageClip = mIsDivNode;
    MBOOL useROI = MFALSE;
    MBOOL useSrcImage = MFALSE;
#endif

    if( !useQueueCmd )
    {
        result.mQueueCmd = TPI_DATA_QUEUE_CMD_NONE;
    }
#if 0
    for( unsigned i = 0; i < imgCount; ++i )
    {
        unsigned id = img[i].mBufferID;
        if( (id != TPI_BUFFER_ID_MTK_OUT_DISPLAY &&
             id != TPI_BUFFER_ID_MTK_OUT_RECORD &&
             id != TPI_BUFFER_ID_MTK_OUT_YUV &&
             id != TPI_BUFFER_ID_MTK_OUT_YUV_2) ||
            (id == TPI_BUFFER_ID_MTK_OUT_RECORD &&
             result.mQueueCmd == TPI_DATA_QUEUE_CMD_PUSH) ||
            (id == TPI_BUFFER_ID_MTK_OUT_YUV && mIsDivNode) )
        {
            continue;
        }
        TPI_ViewInfo &v = img[i].mViewInfo;
        MBOOL validSrcClip = isValid(v.mSrcImageClip.s);
        MBOOL validDstClip = isValid(v.mDstImageClip.s);
        MBOOL validDstROI = isValid(v.mDstZoomROI.s);

        if( (useImageClip && !(validSrcClip && validDstClip)) ||
            (!useImageClip && (validSrcClip || validDstClip)) )
        {
            MY_LOGW("TPI[%d][%d/%s] Clip info not match: use=%d isDiv=%d srcClip=" MRectF_STR " dstClip=" MRectF_STR,
              mTPINodeIndex, id, toBufferName(id), useImageClip, mIsDivNode,
              MRectF_ARG(v.mSrcImageClip), MRectF_ARG(v.mDstImageClip));
            v.mSrcImageClip = MRectF();
            v.mDstImageClip = MRectF();
        }
        if( (useROI && !validDstROI) ||
            (!useROI && validDstROI) )
        {
            MY_LOGW("TPI[%d][%d/%s] ROI info not match: use=%d isDiv=%d DstROI=" MRectF_STR,
              mTPINodeIndex, id, toBufferName(id),
              useROI, mIsDivNode, MRectF_ARG(v.mDstZoomROI));
            v.mDstZoomROI = MRectF();
        }
        if( !useSrcImage )
        {
            v.mSrcImageID = TPI_BUFFER_ID_UNKNOWN;
        }
    }
#endif
    TRACE_FUNC_EXIT();
}

MVOID TPINode::processRecordQ(const RequestPtr &request, const BasicImg &input, const TPI_Data &tpiData, VMDPReq::RecordInfo &outRecordInfo, TPIRes &out)
{
    TRACE_FUNC_ENTER();
    const Feature::ILog &log = request->mLog;
    if( tpiData.mQueueCmd == TPI_DATA_QUEUE_CMD_PUSH ||
        tpiData.mQueueCmd == TPI_DATA_QUEUE_CMD_PUSH_POP )
    {
        OutputInfo info;
        if( !request->getOutputInfo(IO_TYPE_RECORD, info) )
        {
            MY_S_LOGE(log, "QueueCmd=%d(%s) but record buffer not exist", tpiData.mQueueCmd, toStr(tpiData.mQueueCmd));
        }

        TPIQData qData(input, request->mP2Pack, info.mCropRect);
        mTPIQueue.push(qData);

        if( tpiData.mQueueCmd == TPI_DATA_QUEUE_CMD_PUSH )
        {
            outRecordInfo.mSrcType = VMDPReq::SRC_TYPE::DROP;
        }
        else if( tpiData.mQueueCmd == TPI_DATA_QUEUE_CMD_PUSH_POP )
        {
            qData = mTPIQueue.front();
            mTPIQueue.pop();
            outRecordInfo.mSrcType = VMDPReq::SRC_TYPE::RECORD;
            outRecordInfo.mOverride = MTRUE;
            outRecordInfo.mOverrideP2Pack = qData.mP2Pack;
            outRecordInfo.mOverrideCrop = qData.mRecordCrop;

            BasicImg recOut = out.getTP(TPI_BUFFER_ID_MTK_OUT_RECORD);
            recOut.setAllInfo(qData.mInput);
            out.setTP(TPI_BUFFER_ID_MTK_OUT_RECORD, recOut);
        }
        out.setTP(TPI_BUFFER_ID_MTK_Q_RECORD_IN, qData.mInput);
    }
    else // TPI_DATA_QUEUE_CMD_NONE || TPI_DATA_QUEUE_CMD_POP_ALL
    {
        clearQueue();
    }

    MY_LOGW_IF(mTPIQueue.size() > mQueueCount, "QueueSize(%zu) exceeds mQueueCount(%d)", mTPIQueue.size(), mQueueCount);
    MY_S_LOGI_IF(log.getLogLevel(), log, "QueueCmd=%d(%s) QueueSize=%zu", tpiData.mQueueCmd, toStr(tpiData.mQueueCmd), mTPIQueue.size());
    TRACE_FUNC_EXIT();
}

MBOOL TPINode::needOutputRecord(const RequestPtr &request)
{
    MBOOL ret = mIsDivNode && request->hasRecordOutput();
    if( ret )
    {
        MUINT32 delayRecNo = TPI_RECORD_QUEUE_DELAY_CNT * request->getAppFPS() / 30;
        ret = request->mRecordNo > delayRecNo;
        MY_S_LOGD_IF(request->mLog.getLogLevel(), request->mLog, "canRec(%d), recNo(%d), delayNo(%d), fps(%d)", ret, request->mRecordNo, delayRecNo, request->getAppFPS());
    }
    return ret;
}

MVOID TPINode::processSrcInplace(const RequestPtr &request, TPI_Image img[], unsigned imgCount, TPIRes &out)
{
    for( unsigned i = 0; i < imgCount; ++i )
    {
        unsigned id = img[i].mBufferID;
        unsigned srcId = img[i].mUseSrcImageBuffer ? img[i].mViewInfo.mSrcImageID : TPI_BUFFER_ID_UNKNOWN;
        if( srcId != TPI_BUFFER_ID_UNKNOWN )
        {
            BasicImg src = out.getTP(srcId);
            if( src.isValid() )
            {
                out.setTP(id, src);
                img[i].mViewInfo.mSrcImageClip = MRectF(MPoint(), src.mBuffer->getImgSize());
            }
            else
            {
                MY_S_LOGW(request->mLog, "Buffer(%s) need inplace by Src(%s), but src is INVALID!!!", toBufferName(id), toBufferName(srcId));
            }
        }
    }
}

MVOID TPINode::clearQueue()
{
    TRACE_FUNC_ENTER();
    while( !mTPIQueue.empty() )
    {
        mTPIQueue.pop();
    }
    TRACE_FUNC_EXIT();
}

MVOID TPINode::handleBypass(const RequestPtr &request, const TPIRes &in, TPIRes &out)
{
    TRACE_FUNC_ENTER();
    (void)request;
    //unsigned inID = TPI_BUFFER_ID_MTK_YUV;
    //unsigned outID = TPI_BUFFER_ID_MTK_OUT_YUV;
    //out.setTP(outID, out.getTP(inID));
    out = in;
    TRACE_FUNC_EXIT();
}

MVOID TPINode::dumpLog(const RequestPtr &request, const TPIRes &in, const TPIRes &out, TPI_Meta meta[], unsigned metaCount, TPI_Image img[], unsigned imgCount)
{
    TRACE_FUNC_ENTER();
    (void)in;
    (void)out;

    if( request->needTPILog() )
    {
        MY_LOGD("TP[%d]=0x%x isFirst=%d isLast=%d prev=%zu next=%zu enque imgCount = %d, metaCount = %d",
            mTPINodeIndex, mTPINodeIO.mNodeInfo.mNodeID,
            mIsFirstTPI, mIsLastTPI,
            mTPINodeIO.mPrevPortMap.size(), mTPINodeIO.mNextPortMap.size(),
            imgCount, metaCount);
        for( unsigned i = 0; i < imgCount; ++i )
        {
            unsigned stride = 0, bytes = 0;
            MSize size(0,0);
            IImageBuffer *ptr = img[i].mBufferPtr;
            const TPI_ViewInfo &info = img[i].mViewInfo;
            EImageFormat fmt = eImgFmt_UNKNOWN;
            MINT64 timestamp = 0;
            if( ptr )
            {
                stride = ptr->getBufStridesInBytes(0);
                bytes = ptr->getBufSizeInBytes(0);
                size = ptr->getImgSize();
                fmt = (EImageFormat)ptr->getImgFormat();
                timestamp = ptr->getTimestamp();
            }
            MY_LOGD("TP[%d] img[%d] id:0x%x(%s) buffer:%p (%dx%d) fmt=%d(%s) stride=%d bytes=%d time=%ld, viewIn: \
sID(%d)/sCrop(" MCropF_STR ")/size(" MSize_STR ")/srcROI(" MCropF_STR "), viewOut: \
srcID(0x%x)/srcCrp(" MCropF_STR ")/dstCrp(" MCropF_STR ")/dstROI(" MCropF_STR ")",
                mTPINodeIndex, i, img[i].mBufferID, toBufferName(img[i].mBufferID), ptr, size.w, size.h, fmt, toName(fmt), stride, bytes, (long)timestamp,
                info.mSensorID, MCropF_ARG(info.mSensorClip), MSize_ARG(info.mSensorSize), MCropF_ARG(info.mSrcZoomROI),
                info.mSrcImageID, MCropF_ARG(info.mSrcImageClip), MCropF_ARG(info.mDstImageClip), MCropF_ARG(info.mDstZoomROI));
        }
        for( unsigned i = 0; i < metaCount; ++i )
        {
            MY_LOGD("TP[%d] meta[%d] id:0x%x meta:%p",
                mTPINodeIndex, i, meta[i].mMetaID, meta[i].mMetaPtr);
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID TPINode::dumpInBuffer(const RequestPtr &request, const TPIRes &in)
{
    TRACE_FUNC_ENTER();
    if( request->needTPIDump() || request->needDump() )
    {
        if( mIsFirstTPI && allowDump(MASK_SFP_INPUT))
        {
            dumpData(request, in.getSFP(TPI_BUFFER_ID_MTK_YUV), "tpi.in");
            dumpData(request, in.getSFP(TPI_BUFFER_ID_MTK_YUV_2), "tpi.in2");
            dumpData(request, in.getSFP(TPI_BUFFER_ID_MTK_DEPTH), "tpi.inDepth");
            dumpData(request, in.getSFP(TPI_BUFFER_ID_MTK_DEPTH_INTENSITY), "tpi.inDepthInt");
        }
    }

    TRACE_FUNC_EXIT();
}

MVOID TPINode::dumpOutBuffer(const RequestPtr &request, const TPIRes &out)
{
    TRACE_FUNC_ENTER();
    if( request->needTPIDump() || request->needDump() )
    {
        if( mIsLastTPI && allowDump(MASK_SFP_INPUT))
        {
            dumpData(request, out.getTP(TPI_BUFFER_ID_MTK_OUT_YUV), "tpi.out");
            if( mIsDivNode )
            {
                dumpData(request, out.getTP(TPI_BUFFER_ID_MTK_OUT_DISPLAY), "tpi.out.disp");
                dumpData(request, out.getTP(TPI_BUFFER_ID_MTK_OUT_RECORD), "tpi.out.rec");
            }
        }
        if(allowDump(MASK_TP_WORKING))
        {
            TPI_Image img[32];
            unsigned bufferCount = out.getImgArray(img, 32);
            for( unsigned i = 0; i < bufferCount; ++i )
            {
                dumpData(request, img[i].mBufferPtr, "tpi_%d_id_0x%x_%s", mTPINodeIndex, img[i].mBufferID, toBufferName(img[i].mBufferID));
            }
        }
    }

    TRACE_FUNC_EXIT();
}

MVOID TPINode::handleResultData(const RequestPtr &request, const TPIRes &out, const VMDPReq::RecordInfo &recInfo)
{
    TRACE_FUNC_ENTER();
    if( mIsLastTPI )
    {
        VMDPReq outReq;
        BasicImg display = out.getTP(TPI_BUFFER_ID_MTK_OUT_DISPLAY);
        BasicImg record = out.getTP(TPI_BUFFER_ID_MTK_OUT_RECORD);
        BasicImg inYuv = out.getTP(TPI_BUFFER_ID_MTK_YUV);
        BasicImg outYuv = out.getTP(TPI_BUFFER_ID_MTK_OUT_YUV);

        outReq.mGeneralInput = !mIsDivNode ? outYuv :
                               display.isValid() ? display : inYuv;
        if( record.isValid() )
        {
            outReq.mRecordInput = record;
            outReq.mRecordInfo = recInfo;
        }
        if( request->needTPIYuv() )
        {
            outReq.mGeneralInput.syncCache(eCACHECTRL_FLUSH);
            outReq.mRecordInput.syncCache(eCACHECTRL_FLUSH);
        }
        handleData(ID_VENDOR_TO_NEXT, VMDPReqData(outReq, request));
    }
    else
    {
        handleData(ID_VENDOR_TO_NEXT, TPIData(out, request));
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
