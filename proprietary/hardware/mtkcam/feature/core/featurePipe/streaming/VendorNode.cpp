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

#include "VendorNode.h"
#include "MDPWrapper.h"

#define PIPE_CLASS_TAG "VendorNode"
#define PIPE_TRACE TRACE_VENDOR_NODE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

VendorNode::VendorNode(const char *name)
    : StreamingFeatureNode(name)
    , mInImgPoolAllocateNeed(0)
    , mOutImgPoolAllocateNeed(0)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mData);
    TRACE_FUNC_EXIT();
}

VendorNode::~VendorNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID VendorNode::setInImgPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mInImgPool = pool;
    mInImgPoolAllocateNeed = allocate;
    TRACE_FUNC_EXIT();
}

MVOID VendorNode::setOutImgPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mOutImgPool = pool;
    mOutImgPoolAllocateNeed = allocate;
    TRACE_FUNC_EXIT();
}

MBOOL VendorNode::onData(DataID id, const BasicIOImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_VENDOR_FULLIMG ||
        id == ID_FOV_WARP_TO_VENDOR)
    {
        mData.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VendorNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mOutImgPool = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    if( mInImgPoolAllocateNeed && mInImgPool != NULL )
    {
        Timer timer;
        timer.start();
        mInImgPool->allocate(mInImgPoolAllocateNeed);
        timer.stop();
        MY_LOGD("fpipe.vendor.in %s %d buf in %d ms", STR_ALLOCATE, mInImgPoolAllocateNeed, timer.getElapsed());
    }
    if( mOutImgPoolAllocateNeed && mOutImgPool != NULL )
    {
        Timer timer;
        timer.start();
        mOutImgPool->allocate(mOutImgPoolAllocateNeed);
        timer.stop();
        MY_LOGD("fpipe.vendor.out %s %d buf in %d ms", STR_ALLOCATE, mOutImgPoolAllocateNeed, timer.getElapsed());
    }
    return MTRUE;
}

MBOOL VendorNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    BasicIOImgData data;
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
    request->mTimer.startVendor();
    TRACE_FUNC("Frame %d in Vendor", request->mRequestNo);

    BasicImg out;
    MY_LOGD("sensor(%d) Frame %d process start needVendor %d", mSensorIndex, request->mRequestNo, request->needVendor());
    request->mTimer.startVendorProcess();
    MBOOL result = MTRUE;
    if( request->needVendor() )
    {
        if( data.mData.mIn.mBuffer != NULL )
        {
            if( mPipeUsage.supportVendorInplace() )
            {
                out = data.mData.mIn;
            }
            else
            {
                MUINT32 inFormat = data.mData.mIn.mBuffer->getImageBuffer()->getImgFormat();
                if( data.mData.mOut.mBuffer != NULL &&  inFormat == data.mData.mOut.mBuffer->getImageBuffer()->getImgFormat() )
                {
                    data.mData.mOut.mIsReady = MTRUE;
                    out = data.mData.mOut;
                }
                else
                {
                    out.mBuffer = mOutImgPool->requestIIBuffer();
                }
            }

            if(out.mBuffer != NULL)
            {
                out.mBuffer->getImageBuffer()->setExtParam(data.mData.mIn.mBuffer->getImageBuffer()->getImgSize());
            }
            result = processVendor(request, data.mData.mIn.mBuffer, out.mBuffer);

            if( result )
            {
                out.mDomainOffset = data.mData.mIn.mDomainOffset;
                data.mData.mOut.mDomainOffset = data.mData.mIn.mDomainOffset;
            }
            else
            {
                MY_LOGW("sensor(%d) Frame %d processVendor fail", mSensorIndex, request->mRequestNo);
                data.mData.mOut.mIsReady = MFALSE;
                out = data.mData.mIn;
            }
        }
        else
        {
            MY_LOGW("sensor(%d) Frame %d mIn is NULL", mSensorIndex, request->mRequestNo);
        }
    }
    else
    {
        //return input as output
        out = data.mData.mIn;
    }
    request->mTimer.stopVendorProcess();
    request->updateResult(result);

    if( request->needDump() )
    {
        if( data.mData.mIn.mBuffer != NULL )
        {
            data.mData.mIn.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mData.mIn.mBuffer->getImageBufferPtr(), "vendor.in");
        }
        if( out.mBuffer!= NULL )
        {
            out.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, out.mBuffer->getImageBufferPtr(), "vendor.out");
        }
    }

    MY_LOGD("sensor(%d) Frame %d process done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedVendorProcess(), result);
    handleData(ID_VENDOR_TO_VMDP_FULLIMG, BasicIOImgData(BasicIOImg(out, data.mData.mOut), request));

    request->mTimer.stopVendor();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VendorNode::processVendor(const RequestPtr &request, const ImgBuffer &in, const ImgBuffer &out)
{
    TRACE_FUNC_ENTER();
    MBOOL result = MTRUE;

    in->getImageBuffer()->syncCache(eCACHECTRL_INVALID);

    if( mPipeUsage.supportVendorInplace() )
    {
        result = processVendorInplace(request, in);
    }
    else
    {
        result = processVendorInOut(request, in, out);
    }

    out->getImageBuffer()->syncCache(eCACHECTRL_FLUSH);

    TRACE_FUNC_EXIT();
    return result;
}

MBOOL VendorNode::processVendorInOut(const RequestPtr &request, const ImgBuffer &in, const ImgBuffer &out)
{
    TRACE_FUNC_ENTER();

    MBOOL result = MTRUE;

    // Add IN AND OUT vendor code here
    // and return process result
    result = copy(request, in, out);//this is example PLEASE replace it


    TRACE_FUNC_EXIT();
    return  result;
}

MBOOL VendorNode::processVendorInplace(const RequestPtr &request, const ImgBuffer &img)
{
    TRACE_FUNC_ENTER();

    (void)(request);
    (void)(img);

    MBOOL result = MTRUE;

    // Add INPLACE vendor code here
    // and return process result

    TRACE_FUNC_EXIT();
    return  result;
}

MBOOL VendorNode::copy(const RequestPtr &request, const ImgBuffer &in, const ImgBuffer &out)
{
    TRACE_FUNC_ENTER();

    Timer timer(MTRUE);

    MDPWrapper mdp;

    MDPWrapper::Output output;
    output.mPortID = PortID(EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = out->getImageBufferPtr();

    MDPWrapper::MCropRect crop;
    crop.s = out->getImageBuffer()->getImgSize();
    crop.p_integral.x = 0;
    crop.p_integral.y = 0;

    MDPWrapper::OUTPUT_ARRAY outputs;
    outputs.push_back(MDPWrapper::MDPOutput(output, crop));

    MBOOL result = mdp.process(in->getImageBufferPtr(), outputs);
    MY_LOGD("Frame %d MDP process result %d in %d ms", request->mRequestNo, result, timer.getNow());

    TRACE_FUNC_EXIT();
    return result;
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
