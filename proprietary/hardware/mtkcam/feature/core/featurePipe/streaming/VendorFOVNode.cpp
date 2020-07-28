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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "VendorFOVNode.h"

#define PIPE_CLASS_TAG "VendorFOVNode"
#define PIPE_TRACE TRACE_VENDOR_FOV_NODE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

VendorFOVNode::VendorFOVNode(const char *name)
    : StreamingFeatureNode(name)
    , mInImgPoolAllocateNeed(0)
    , mOutImgPoolAllocateNeed(0)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mData);
    TRACE_FUNC_EXIT();
}

VendorFOVNode::~VendorFOVNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID VendorFOVNode::setInputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mInImgPool = pool;
    mInImgPoolAllocateNeed = allocate;
    TRACE_FUNC("allocate: %d", allocate);
    TRACE_FUNC_EXIT();
}

MVOID VendorFOVNode::setOutputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mOutImgPool = pool;
    mOutImgPoolAllocateNeed = allocate;
    TRACE_FUNC("allocate: %d", allocate);
    TRACE_FUNC_EXIT();
}

MBOOL VendorFOVNode::onData(DataID id, const DualBasicIOImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_VFOV_FULLIMG )
    {
        mData.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VendorFOVNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    mVendorCore = IVendorCore::getInstance();
    if (mVendorCore  != NULL) {
        mVendorCore->init();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorFOVNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mOutImgPool = NULL;
    mInImgPool = NULL;
    if (mVendorCore != NULL) {
        mVendorCore->uninit();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorFOVNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("vendorFOV in buffer need:%d instance:%d, out buffer need:%d instance: %d",
        mInImgPoolAllocateNeed, (mInImgPool.get()!=NULL), mOutImgPoolAllocateNeed, (mOutImgPool.get()!=NULL));
    if(mOutImgPoolAllocateNeed && mOutImgPool.get() && mInImgPoolAllocateNeed && mInImgPool.get())
    {
        Timer timer;
        timer.start();
        mOutImgPool->allocate(mOutImgPoolAllocateNeed);
        mInImgPool->allocate(mInImgPoolAllocateNeed);
        timer.stop();
        MY_LOGD("%s outPool %d buf, inPool %d buf in %d ms", STR_ALLOCATE,
            mOutImgPoolAllocateNeed, mInImgPoolAllocateNeed, timer.getElapsed());

    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorFOVNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorFOVNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    DualBasicIOImgData data;
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
    TRACE_FUNC("Frame %d in Vendor", request->mRequestNo);
    DualBasicIOImg dualData = data.mData;
    BasicImg out;
    BasicImg master = dualData.mMaster.mIn;
    BasicImg slave = dualData.mSlave.mIn;

    TRACE_FUNC("Frame %d fovImgPool=(%d/%d)", request->mRequestNo, mOutImgPool->peakAvailableSize(), mOutImgPool->peakPoolSize());

    MY_LOGD("VendorFOVNode featuremask:%d", request->mFeatureMask);
    if (slave.mBuffer.get() && slave.mBuffer->getImageBufferPtr()) {
        MY_LOGD("has slave");
    } else {
        MY_LOGD("has no slave");
    }

    MY_LOGD("sensor(%d) Frame %d process start", mSensorIndex, request->mRequestNo);
    if( request->needVendorFOV() )
    {
        MY_LOGD("need vendorFOV");
        request->mTimer.startVendorFOV();
        MY_LOGD("request buffer");
        out.mBuffer = mOutImgPool->requestIIBuffer();
        MY_LOGD("request buffer done");
        out.mBuffer->getImageBuffer()->setExtParam(master.mBuffer->getImageBufferPtr()->getImgSize());
        MBOOL result = processVendorFOV(request, master.mBuffer, slave.mBuffer, out.mBuffer);
        request->mTimer.stopVendorFOV();
        request->updateResult(result);
    }
    else
    {
        MY_LOGD("don't need vendorFOV");
        // bypass, return input as output
        request->mTimer.startVendorFOV();
        out = master;
        request->mTimer.stopVendorFOV();
    }
    MY_LOGD("dump");

    if (request->needDump())
    {
        if (master.mBuffer.get() && master.mBuffer->getImageBufferPtr())
        {
            dumpData(request, master.mBuffer->getImageBufferPtr(), "vvmaster");
        }
        if (slave.mBuffer.get() && slave.mBuffer->getImageBufferPtr())
        {
            dumpData(request, slave.mBuffer->getImageBufferPtr(), "vvslave");
        }
        if (out.mBuffer.get() && out.mBuffer->getImageBufferPtr())
        {
            dumpData(request, out.mBuffer->getImageBufferPtr(), "vvout");
        }
    }

    MY_LOGD("sensor(%d) Frame %d process done in %d ms", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedVendorFOV());
    if( mPipeUsage.supportVendor() )
    {
        handleData(ID_VFOV_TO_VENDOR, BasicIOImgData(BasicIOImg(out, dualData.mMaster.mOut), request));
    }
    else
    {
        handleData(ID_VFOV_TO_VMDP_FULLIMG, BasicIOImgData(BasicIOImg(out, dualData.mMaster.mOut), request));
    }

    request->mTimer.stopVendorFOV();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorFOVNode::processVendorFOV(const RequestPtr &request, const ImgBuffer &master, const ImgBuffer &slave, const ImgBuffer &out)
{
    TRACE_FUNC_ENTER();
    MBOOL result = MTRUE;

    if (mVendorCore.get() != NULL) {
        mVendorCore->processVendor(request, master, slave, out);
    } else {
        MY_LOGE("VendorCore is null");
    }
    TRACE_FUNC_EXIT();
    return result;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
