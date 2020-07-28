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
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/feature/fsc/fsc_util.h>

#define FLOAT_IGNORE_ROUNDING_ERROR 0.001f
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

static void domainTrans(MRectF &cropRectF, MSizeF const &domainOffset, MSizeF const &from, MSizeF const &to)
{
    cropRectF.p.x = max( cropRectF.p.x - domainOffset.w, 0.0f );
    cropRectF.p.y = max( cropRectF.p.y - domainOffset.h, 0.0f );
    float ratioW = to.w / from.w;
    float ratioH = to.h / from.h;

    cropRectF.p.x *= ratioW;
    cropRectF.p.y *= ratioH;
    cropRectF.s.w *= ratioW;
    cropRectF.s.h *= ratioH;

    if( cropRectF.p.x < FLOAT_IGNORE_ROUNDING_ERROR ) cropRectF.p.x = 0.0f;
    if( cropRectF.p.y < FLOAT_IGNORE_ROUNDING_ERROR ) cropRectF.p.y = 0.0f;
    if( abs(cropRectF.s.w-to.w) < FLOAT_IGNORE_ROUNDING_ERROR ) cropRectF.s.w = to.w;
    if( abs(cropRectF.s.h-to.h) < FLOAT_IGNORE_ROUNDING_ERROR ) cropRectF.s.h = to.h;
}

static
MBOOL refineBoundary(
    MSizeF const& bufSize,
    MCropRect& crop
)
{
    MBOOL isRefined = MFALSE;
    MCropRect refined = crop;
    if( crop.p_integral.x < 0 ) {
        refined.p_integral.x = 0;
        isRefined = MTRUE;
    }
    if( crop.p_integral.y < 0 ) {
        refined.p_integral.y = 0;
        isRefined = MTRUE;
    }
    //
    int const carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
    if( (refined.p_integral.x + crop.s.w + carry_x) > bufSize.w ) {
        refined.s.w = bufSize.w - refined.p_integral.x - carry_x;
        isRefined = MTRUE;
    }
    int const carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
    if( (refined.p_integral.y + crop.s.h + carry_y) > bufSize.h ) {
        refined.s.h = bufSize.h - refined.p_integral.y - carry_y;
        isRefined = MTRUE;
    }
    //
    if( isRefined ) {

        MY_LOGD("buf size %.0fx%.0f, crop(%d/%d, %d/%d, %dx%d)->crop(%d/%d, %d/%d, %dx%d)",
                bufSize.w, bufSize.h,
                crop.p_integral.x,
                crop.p_integral.y,
                crop.p_fractional.x,
                crop.p_fractional.y,
                crop.s.w,
                crop.s.h,
                refined.p_integral.x,
                refined.p_integral.y,
                refined.p_fractional.x,
                refined.p_fractional.y,
                refined.s.w,
                refined.s.h
                );
        crop = refined;
    }
    return isRefined;
}

VendorMDPNode::VendorMDPNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mData);
    TRACE_FUNC_EXIT();
}

VendorMDPNode::~VendorMDPNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL VendorMDPNode::onData(DataID id, const BasicIOImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    if( id == ID_VENDOR_TO_VMDP_FULLIMG ||
        id == ID_VFOV_TO_VMDP_FULLIMG
      )
    {
        this->mData.enque(data);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", pRequest->mRequestNo, ID2Name(id));
    if( id == ID_N3D_TO_VMDP )
    {
        this->mN3DReadyData.enque(pRequest);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    if( mPipeUsage.supportN3D() )
    {
        this->addWaitQueue(&mN3DReadyData);
    }
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
    MBOOL isLastNode;
    RequestPtr request;
    BasicIOImgData data;

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

    if( mPipeUsage.supportN3D() )
    {
        if( !mN3DReadyData.deque(request) )
        {
            MY_LOGE("mN3DReadyData deque out of sync");
            return MFALSE;
        }
        if(request->mRequestNo != data.mRequest->mRequestNo)
        {
            MY_LOGE("Request out of sync");
            return MFALSE;
        }
    }

    TRACE_FUNC_ENTER();
    request = data.mRequest;
    request->mTimer.startVMDP();
    TRACE_FUNC("Frame %d in MDP", request->mRequestNo);
    processMDP(request, data.mData);
    request->mTimer.stopVMDP();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::processMDP(const RequestPtr &request, const BasicIOImg &fullIOImg)
{
    TRACE_FUNC_ENTER();

    if( request->needVendorMDP() )
    {
        MDPWrapper::OUTPUT_ARRAY outputs;
        if( prepareMDPOut(request, fullIOImg, outputs) )
        {
            if( fullIOImg.mIn.mBuffer != NULL )
            {
                MBOOL result = mMDP.process(fullIOImg.mIn.mBuffer->getImageBufferPtr(), outputs);
                request->updateResult(result);

                if( request->needDump() )
                {
                    for( unsigned i = 0; i < outputs.size(); ++i )
                    {
                        dumpData(request, outputs[i].mOutput.mBuffer, "vmdp_%d", i);
                    }
                }
            }
            else
            {
                MY_LOGW("sensor(%d) Frame %d mIn is NULL", mSensorIndex, request->mRequestNo);
            }
        }
    }

    FeaturePipeParam::MSG_TYPE msg = mPipeUsage.supportWarpNode() ? FeaturePipeParam::MSG_DISPLAY_DONE : FeaturePipeParam::MSG_FRAME_DONE;
    handleData(ID_VMDP_TO_HELPER, CBMsgData(msg, request));

    if( mPipeUsage.supportWarpNode() )
    {
        handleData(ID_VMDP_TO_NEXT_FULLIMG, BasicImgData(fullIOImg.mOut, request));
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorMDPNode::prepareMDPOut(const RequestPtr &request, const BasicIOImg &fullIOImg, MDPWrapper::OUTPUT_ARRAY& outputs)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    MBOOL skipRecord = mPipeUsage.supportWarpNode() && request->needWarp();
    MCrpRsInfo cropInfo;

    if( request->mQParams.mvFrameParams.size() )
    {
        for( int i = 0, n = request->mQParams.mvFrameParams[0].mvOut.size(); i < n; ++i )
        {
            MRectF resultF;
            if( isDisplayOutput(request->mQParams.mvFrameParams[0].mvOut[i]) &&
                request->getDisplayCrop(cropInfo, RRZO_DOMAIN, 0, &resultF) )
            {

            }
            else if( isRecordOutput(request->mQParams.mvFrameParams[0].mvOut[i]) &&
                     request->getRecordCrop(cropInfo, RRZO_DOMAIN, 0, &resultF) &&
                     !skipRecord )
            {

            }
            else if( isExtraOutput(request->mQParams.mvFrameParams[0].mvOut[i]) &&
                     request->getExtraCrop(cropInfo, RRZO_DOMAIN, 0, &resultF))
            {

            }
            else
            {
                continue;
            }

            DpPqParam pqParam;
            NSCam::NSIoPipe::PortID portID = request->mQParams.mvFrameParams[0].mvOut[i].mPortID;
            if( portID.index == NSImageio::NSIspio::EPortIndex_WDMAO )
            {
                request->getWDMADpPqParam(pqParam);
            }
            else if( portID.index == NSImageio::NSIspio::EPortIndex_WROTO )
            {
                request->getWROTDpPqParam(pqParam);
            }

            MRectF cropRectF;
            MCropRect cropRect;
            MSizeF from, to;
            if( request->needEarlyFSCVendorFullImg() || mPipeUsage.supportVendorCusSize())
            {
                if( request->needEarlyFSCVendorFullImg() )
                {
                    MRectF FSCRRZOCrop = request->getFSCCropRegion();
                    from = FSCRRZOCrop.s;
                }
                else
                {
                    from = request->mFullImgSize;
                }

                if( mPipeUsage.supportVendorCusSize() )
                {
                    to = mPipeUsage.getVendorCusSize();
                }
                else
                {
                    to = request->mFullImgSize;
                }

                cropRectF = resultF;
                domainTrans(cropRectF, fullIOImg.mIn.mDomainOffset, from, to);

                cropRect = getCropRect(cropRectF);
                if( FSCUtil::isFSCSubpixelEnabled(mPipeUsage.getFSCMode()) != MTRUE)
                {
                    cropRect.p_fractional.x = 0;
                    cropRect.p_fractional.y = 0;
                    cropRect.w_fractional = 0;
                    cropRect.h_fractional = 0;
                }

                refineBoundary(to, cropRect);

                if( FSCUtil::getFSCDebugLevel(mPipeUsage.getFSCMode()) && ::property_get_int32(FSC_VMDP_MAXCROP_PROPERTY, 0) )
                {
                    cropRect.p_integral.x = 0;
                    cropRect.p_integral.y = 0;
                    cropRect.p_fractional.x = 0;
                    cropRect.p_fractional.y = 0;
                    cropRect.s.w = to.w;
                    cropRect.s.h = to.h;
                    cropRect.w_fractional = 0;
                    cropRect.h_fractional = 0;
                }

                TRACE_FUNC("from:%dx%d to:%dx%d result:%dx%d", from.w, from.h, to.w, to.h, cropRect.s.w, cropRect.s.h);
            }
            else
            {
                cropRect = cropInfo.mCropRect;
                cropRect.p_integral.x = max(cropRect.p_integral.x - (int)fullIOImg.mIn.mDomainOffset.w, 0);
                cropRect.p_integral.y = max(cropRect.p_integral.y - (int)fullIOImg.mIn.mDomainOffset.h, 0);
            }

            if( mPipeUsage.supportFSC() )
            {
                MY_LOGD("f(%d) early fsc(%d) org(%f,%f,%f,%f) trans(%f,%f,%f,%f)->vcrop(%d,%d,0x%x,0x%x,%d,%d), domain offset(%f,%f), from:%fx%f to:%.0fx%.0f)",
                    request->mRequestNo, request->needEarlyFSCVendorFullImg(),
                    resultF.p.x, resultF.p.y, resultF.s.w, resultF.s.h,
                    cropRectF.p.x, cropRectF.p.y, cropRectF.s.w, cropRectF.s.h,
                    cropRect.p_integral.x, cropRect.p_integral.y, cropRect.p_fractional.x, cropRect.p_fractional.y, cropRect.s.w, cropRect.s.h,
                    fullIOImg.mIn.mDomainOffset.w, fullIOImg.mIn.mDomainOffset.h,
                    from.w, from.h, to.w, to.h);
            }

            outputs.push_back(MDPWrapper::MDPOutput(request->mQParams.mvFrameParams[0].mvOut[i], cropRect, pqParam));
        }
    }

    if( fullIOImg.mOut.mBuffer != NULL && !fullIOImg.mOut.mIsReady )
    {
        TRACE_FUNC("sensor(%d) Frame %d prepare VendorMDP extra FullImg", mSensorIndex, request->mRequestNo);

        NSIoPipe::Output output;
        output.mPortID = PortID(EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, PORTID_OUT);
        output.mBuffer = fullIOImg.mOut.mBuffer->getImageBufferPtr();

        MCropRect cropRect(MPoint(0, 0), fullIOImg.mIn.mBuffer->getImageBufferPtr()->getImgSize());
        DpPqParam pqParam;
        outputs.push_back(MDPWrapper::MDPOutput(output, cropRect, pqParam));
    }

    TRACE_FUNC_EXIT();
    return !outputs.empty();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
