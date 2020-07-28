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

#include "VNRNode.h"

using NSImageio::NSIspio::EPortIndex_IMG3O;

#define PIPE_CLASS_TAG "VNRNode"
#define PIPE_TRACE TRACE_VNR_NODE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_VNR);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum DumpMaskIndex
{
    MASK_VPUO,
};


const std::vector<DumpFilter> sFilterTable =
{
    DumpFilter( MASK_VPUO,      "vpuo" )
};

VNRNode::VNRNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mDSDNDatas);
    TRACE_FUNC_EXIT();
}

VNRNode::~VNRNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID VNRNode::setInputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    (void)allocate;
    TRACE_FUNC_ENTER();
    mInputBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID VNRNode::setOutputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    (void)allocate;
    TRACE_FUNC_ENTER();
    mOutputBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL VNRNode::onData(DataID id, const DSDNData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2NR_TO_VNR )
    {
        mDSDNDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType VNRNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &reqInfo) const
{
    IOPolicyType policy = IOPOLICY_BYPASS;
    if( reqInfo.isMaster() && HAS_DSDN20(reqInfo.mFeatureMask) )
    {
        policy = IOPOLICY_INOUT_NEXT;
    }
    return policy;
}

MBOOL VNRNode::getInputBufferPool(const StreamingReqInfo &/*reqInfo*/, android::sp<IBufferPool> &pool, NextFullInfo &info)
{
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    info.mResize = MSize(0, 0);
    info.mNeedCrop = MFALSE;
    info.mCanShare = MFALSE;
    if( mInputBufferPool != NULL )
    {
        pool = mInputBufferPool;
        info.mResize = mCustomSize;
        info.mCanShare = mUseSharedInput;
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    enableDumpMask(0xffff, sFilterTable);
    MBOOL ret = MTRUE;
    mDSDNCfg = mPipeUsage.getDSDNCfg();
    mVNR.init(mDSDNCfg.getMaxSize(DSDNCfg::FULL));
    mCustomSize = mPipeUsage.getTPIUsage().getCustomSize(TPIOEntry::YUV, MSize(0, 0));
    mUseSharedInput = mPipeUsage.supportSharedInput();
    mSimulate = VNRHal::isSimulate() ||
                getPropertyValue("vendor.debug.fpipe.vnr.simulate");
    ret = (mInputBufferPool != NULL && mOutputBufferPool != NULL);

    MY_LOGI("VNR custSize=(%dx%d) simulate=%d",
            mCustomSize.w, mCustomSize.h, mSimulate);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mVNR.uninit();
    mInputBufferPool = NULL;
    mOutputBufferPool = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VNRNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    allocate("fpipe.vnr.in", mInputBufferPool, 6);
    allocate("fpipe.vnr.out", mOutputBufferPool, 3);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VNRNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VNRNode::onThreadLoop()
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    TRACE_FUNC("Waitloop");
    MBOOL need = MFALSE, result = MFALSE;
    DSDNData data;
    RequestPtr request;
    BasicImg out;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mDSDNDatas.deque(data) )
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
    request->mTimer.startVNR();
    TRACE_FUNC("Frame %d in VNR needDSDN20 %d", request->mRequestNo, request->needDSDN20());
    need = request->needDSDN20();
    MY_S_LOGD(request->mLog, "sensor(%d) Frame %d VNR process start need=%d simulate=%d", mSensorIndex, request->mRequestNo, need, mSimulate);
    if( need )
    {
        out = prepareOut(request, data.mData);
        result = processVNR(request, data.mData, out);
        if( result )
        {
            dump(request, out);
        }
    }
    MY_S_LOGD(request->mLog, "sensor(%d) Frame %d VNR process done in %d ms, need=%d processed=%d appFPS=%d simulate=%d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedVNREnque(), need, result, request->getAppFPS(), mSimulate);
    if( !result )
    {
        out = data.mData.mFullImg;
    }
    handleData(ID_VNR_TO_NEXT_FULLIMG, DualBasicImgData(DualBasicImg(out, data.mData.mSlaveImg), request));
    request->mTimer.stopVNR();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

BasicImg VNRNode::prepareOut(const RequestPtr &request, const DSDNImg &data)
{
    TRACE_FUNC_ENTER();
    BasicImg out;
    NextFullInfo nfInfo;
    MUINT32 masterID = request->mMasterID;
    if( request->needNextFullImg(this, masterID) )
    {
        out.mBuffer = request->requestNextFullImg(this, masterID, nfInfo);
    }
    else
    {
        out.mBuffer = mOutputBufferPool->requestIIBuffer();
    }
    out.setAllInfo(data.mFullImg, nfInfo.mResize);
    TRACE_FUNC_EXIT();
    return out;
}

MBOOL VNRNode::processVNR(const RequestPtr &request, const DSDNImg &data, BasicImg &out)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( checkInput(request, data) )
    {
        MUINT32 fps = request->getAppFPS();
        out.setAllInfo(data.mFullImg);
        request->mTimer.startVNREnque();
        ImgBuffer inFull = data.mFullImg.mBuffer;
        ImgBuffer inDS1 = data.mDS1Img.mBuffer;
        ImgBuffer inDS2 = data.mDS2Img;
        ImgBuffer outFull = out.mBuffer;
        ret = mSimulate ? simulate(fps, inFull, inDS1, inDS2, outFull) :
                          mVNR.process(fps, inFull, inDS1, inDS2, outFull);
        request->mTimer.stopVNREnque();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRNode::checkInput(const RequestPtr &request, const DSDNImg &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( !data.mFullImg.isValid() ||
        !data.mDS1Img.isValid() ||
        data.mDS2Img == NULL)
    {
        MY_LOGW("Frame %d: invalid input: full=%d ds1=%d ds2=%d",
                request->mRequestNo,
                data.mFullImg.isValid(), data.mDS1Img.isValid(), data.mDS2Img != NULL);
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRNode::simulate(MUINT32 fps, const ImgBuffer &inFull, const ImgBuffer &inDS1, const ImgBuffer &inDS2, const ImgBuffer &outFull)
{
    (void)fps;
    (void)inDS1;
    (void)inDS2;
    IImageBuffer *src = inFull->getImageBufferPtr();
    IImageBuffer *dst = outFull->getImageBufferPtr();
    copyImageBuffer(src, dst);
    usleep(7000);
    return MTRUE;
}

MVOID VNRNode::dump(const RequestPtr &request, const BasicImg &out) const
{
    TRACE_FUNC_ENTER();
    if( request->needNddDump() || request->needDump() )
    {
        syncCache(out, eCACHECTRL_INVALID);

        if( request->needNddDump() && allowDump(MASK_VPUO) )
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData().mNDDHint;
            dumpNddData(&hint, out, TuningUtils::YUV_PORT_VNRO);
        }
        if( request->needDump() && allowDump(MASK_VPUO) )
        {
            dumpData(request, out, "dsdn_full_out");
        }
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
