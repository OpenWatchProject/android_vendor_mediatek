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

#include "VendorMDPNode.h"

#define PIPE_CLASS_TAG "VMDPNode"
#define PIPE_TRACE TRACE_VMDP_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam3/feature/eis/eis_ext.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_VENDOR_MDP);

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

VendorMDPNode::VendorMDPNode(const char *name)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_VENDOR_MDP)
    , StreamingFeatureNode(name)
    , mMDP(PIPE_CLASS_TAG)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mData);
    mNumMDPPort = std::max<MUINT32>(1, mMDP.getNumOutPort());
    mForceExpectMS = getPropertyValue("vendor.debug.fpipe.vmdp.expect", 0);
    TRACE_FUNC_EXIT();
}

VendorMDPNode::~VendorMDPNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL VendorMDPNode::onData(DataID id, const VMDPReqData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    if( id == ID_VENDOR_TO_NEXT )
    {
        this->mData.enque(data);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onData(DataID id, const DualBasicImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    if( id == ID_VNR_TO_NEXT_FULLIMG )
    {
        this->mData.enque(toVMDPReqData(data));
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

IOPolicyType VendorMDPNode::getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const
{
    (void)stream;
    IOPolicyType policy = IOPOLICY_BYPASS;
    if( reqInfo.isMaster() &&
        ( HAS_TPI_YUV(reqInfo.mFeatureMask) ||
        ( HAS_DSDN20(reqInfo.mFeatureMask) && (!HAS_EIS(reqInfo.mFeatureMask) || !mPipeUsage.supportPreviewEIS()) )
        ))
    {
        policy = IOPOLICY_INOUT;
    }

    return policy;
}

MBOOL VendorMDPNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    enableDumpMask(0, sFilterTable);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onUninit()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    VMDPReqData data;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mData.deque(data) )
    {
        MY_LOGE("Data deque out of sync");
        return MFALSE;
    }

    if( data.mRequest == NULL)
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();
    request = data.mRequest;
    request->mTimer.startVMDP();
    TRACE_FUNC("Frame %d in MDP", request->mRequestNo);
    processVendorMDP(request, data.mData);
    request->mTimer.stopVMDP();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::processVendorMDP(const RequestPtr &request, const VMDPReq &mdpReq)
{
    TRACE_FUNC_ENTER();
    MBOOL result = MFALSE;

    BasicImg nextFullImg;
    ImgBuffer asyncImg;

    if( request->needVendorMDP() && request->hasGeneralOutput() )
    {
        VMDPReq::SRC_TYPE recSrc= mdpReq.getRecSrcType();
        MUINT32 maxMdpCycle = getMaxMDPCycleTimeMs(request, (recSrc == VMDPReq::SRC_TYPE::RECORD));
        result = processGeneralMDP(request, mdpReq.mGeneralInput, nextFullImg, asyncImg, (recSrc == VMDPReq::SRC_TYPE::GENERAL), maxMdpCycle);
        if( recSrc == VMDPReq::SRC_TYPE::RECORD )
        {
            result = result && processRecordMDP(request, mdpReq.mRecordInput, mdpReq.mRecordInfo, maxMdpCycle);
        }
        else if( recSrc == VMDPReq::SRC_TYPE::DROP )
        {
            processRecordDrop(request);
        }
        request->updateResult(result);
    }

    TRACE_FUNC("frame %d need=%d result=%d", request->mRequestNo, request->needVendorMDP(), result);

    FeaturePipeParam::MSG_TYPE msg = mPipeUsage.supportWarpNode() ? FeaturePipeParam::MSG_DISPLAY_DONE : FeaturePipeParam::MSG_FRAME_DONE;
    MBOOL needSkip = (msg == FeaturePipeParam::MSG_DISPLAY_DONE) && !request->needDisplayOutput(this);
    if (!needSkip)
    {
        handleData(ID_VMDP_TO_HELPER, HelperData(HelpReq(msg), request));
    }

    if( mPipeUsage.supportWarpNode() )
    {
        handleData(ID_VMDP_TO_NEXT_FULLIMG, BasicImgData(nextFullImg.mBuffer != NULL ? nextFullImg : mdpReq.mGeneralInput, request));
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::processGeneralMDP(const RequestPtr &request, const BasicImg &fullImg, BasicImg &nextFullImg, ImgBuffer &asyncImg,  MBOOL allowRecord, MUINT32 mdpCycleTimeMs)
{
    TRACE_FUNC_ENTER();
    MDPWrapper::P2IO_OUTPUT_ARRAY outputs;
    MBOOL result = MFALSE;

    if( prepareGeneralMDPOut(request, fullImg, outputs, nextFullImg, asyncImg, allowRecord) )
    {
        result = processMDP("general", request, fullImg, outputs, mdpCycleTimeMs);
    }
    TRACE_FUNC_EXIT();
    return result;
}

MBOOL VendorMDPNode::processRecordMDP(const RequestPtr &request, const BasicImg &fullImg, const VMDPReq::RecordInfo &recordInfo, MUINT32 mdpCycleTimeMs)
{
    TRACE_FUNC_ENTER();
    MDPWrapper::P2IO_OUTPUT_ARRAY outputs;
    P2IO out;
    DpPqParam pq;

    if( request->popRecordOutput(this, out) )
    {
        overrideRecordInfo(fullImg, recordInfo, out, &pq);
        refineCrop(request, fullImg, out);
        outputs.push_back(out);
    }

    MBOOL result = processMDP("record", request, fullImg, outputs, mdpCycleTimeMs);
    TRACE_FUNC_EXIT();
    return result;

}

MBOOL VendorMDPNode::processMDP(const char* name, const RequestPtr &request, const BasicImg &fullImg, const MDPWrapper::P2IO_OUTPUT_ARRAY& outputs, MUINT32 mdpCycleTimeMs)
{
    MBOOL result = MFALSE;
    if( fullImg.mBuffer == NULL )
    {
        MY_LOGW("sensor(%d) Frame %d fullImg is NULL in %s MDP run", mSensorIndex, request->mRequestNo, name);
    }
    else if(outputs.empty())
    {
        MY_LOGW("sensor(%d) Frame %d has no Output in %s MDP run", mSensorIndex, request->mRequestNo, name);
    }
    else
    {
        result = mMDP.process(fullImg.mBuffer->getImageBufferPtr(), outputs, request->needPrintIO(), mdpCycleTimeMs);

        if( request->needDump() && allowDump(MASK_ALL))
        {
            for( unsigned i = 0; i < outputs.size(); ++i )
            {
                dumpData(request, outputs[i].mBuffer, "vmdp_%s_%d", name, i);
            }
        }
    }
    return result;
}

MVOID VendorMDPNode::processRecordDrop(const RequestPtr &request)
{
    request->setVar<MBOOL>(SFP_VAR::EIS_SKIP_RECORD, MTRUE);
}

MBOOL VendorMDPNode::prepareGeneralMDPOut(const RequestPtr &request, const BasicImg &fullImg, MDPWrapper::P2IO_OUTPUT_ARRAY& outputs, BasicImg &nextFullImg, ImgBuffer &asyncImg, MBOOL allowRecord)
{
    TRACE_FUNC_ENTER();
    P2IO out;
    // display
    if( request->popDisplayOutput(this, out) )
    {
        refineCrop(request, fullImg, out);
        outputs.push_back(out);

        if( request->needTPIAsync() )
        {
            asyncImg = request->popAsyncImg(this);
            if( asyncImg != NULL )
            {
                P2IO async;
                async.mBuffer = asyncImg->getImageBufferPtr();
                async.mCropDstSize = asyncImg->getImgSize();
                async.mCropRect = out.mCropRect;
                outputs.push_back(async);
            }
        }
    }

    // record
    if( allowRecord && request->popRecordOutput(this, out) )
    {
        refineCrop(request, fullImg, out);
        outputs.push_back(out);
    }

    // Extra data
    std::vector<P2IO> extraOuts;
    if( request->popExtraOutputs(this, extraOuts))
    {
        for(P2IO out : extraOuts)
        {
            refineCrop(request, fullImg, out);
            outputs.push_back(out);
        }
    }

    // next FullImg
    const MUINT32 masterID = request->mMasterID;
    if( request->needNextFullImg(this, masterID) )
    {
        MSize inSize = fullImg.mBuffer->getImageBuffer()->getImgSize();
        MSize outSize = inSize;
        NextFullInfo info;
        nextFullImg.mBuffer = request->requestNextFullImg(this, masterID, info);
        nextFullImg.setAllInfo(fullImg);
        if( info.mResize.w && info.mResize.h )
        {
            outSize = info.mResize;
        }

        P2IO out;
        out.mBuffer = nextFullImg.mBuffer->getImageBufferPtr();
        out.mBuffer->setExtParam(outSize);
        out.mCropRect = MRect(MPoint(0,0), fullImg.mBuffer->getImageBuffer()->getImgSize());
        out.mCropDstSize = outSize;
        outputs.push_back(out);

        nextFullImg.accumulate("vmdpNFull", request->mLog, inSize, out.mCropRect, outSize);
    }

    TRACE_FUNC_EXIT();

    return !outputs.empty();
}


MVOID VendorMDPNode::refineCrop(const RequestPtr &request, const BasicImg &fullImg, P2IO &out)
{
    (void)request;
    MRectF cropRect = fullImg.mTransform.applyTo(out.mCropRect);

    TRACE_FUNC("No(%d)," MTransF_STR ",source=" MCropF_STR "->result=" MCropF_STR,
            request->mRequestNo, MTransF_ARG(fullImg.mTransform.mOffset, fullImg.mTransform.mScale),
            MCropF_ARG(out.mCropRect), MCropF_ARG(cropRect));

    MSizeF sourceSize = fullImg.mBuffer->getImageBuffer()->getImgSize();
    refineBoundaryF("VendorMDPNode", sourceSize, cropRect, request->needPrintIO());
    out.mCropRect = cropRect;
}

MVOID VendorMDPNode::overrideRecordInfo(const BasicImg &fullImg, const VMDPReq::RecordInfo &recordInfo, P2IO &out, DpPqParam *dpPqParam)
{
    if(recordInfo.mOverride)
    {
        Feature::P2Util::MDPObjPtr mdpObj;
        mdpObj.isExtraRecSrc = true;

        out.mCropRect = recordInfo.mOverrideCrop;
        out.mPqParam = NULL;
        out.mDpPqParam = Feature::P2Util::makeDpPqParam(dpPqParam, recordInfo.mOverrideP2Pack, NSIoPipe::EPortCapbility_Rcrd, mdpObj);
        out.mBuffer->setTimestamp(fullImg.mBuffer->getImageBuffer()->getTimestamp());
    }
}

MUINT32 VendorMDPNode::getMaxMDPCycleTimeMs(const RequestPtr &request, MBOOL needExtraRecordRun)
{
    MUINT32 runOut = 0;
    if( request->needNextFullImg(this, request->mMasterID) ) runOut += 1;
    if( request->needDisplayOutput(this) )
    {
        runOut += 1;
        if( request->needTPIAsync() ) runOut += 1;
    }
    if( request->needRecordOutput(this) && !needExtraRecordRun) runOut += 1;
    if( request->needExtraOutput(this) ) runOut += request->getExtraOutputCount();

    MUINT32 cycle = (runOut+mNumMDPPort-1) / mNumMDPPort;
    if( needExtraRecordRun ) cycle += 1;
    MUINT32 expectMs = (mForceExpectMS > 0) ? mForceExpectMS : request->getNodeCycleTimeMs();
    MUINT32 maxTimeMs = expectMs / std::max<MUINT32>(1, cycle);
    MY_S_LOGD_IF(request->mLog.getLogLevel(), request->mLog, "mNumPort(%d) expect(%d) runOut(%d) cycle(%d) cycleMs(%d)", mNumMDPPort, expectMs, runOut, cycle, maxTimeMs);
    return maxTimeMs;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
