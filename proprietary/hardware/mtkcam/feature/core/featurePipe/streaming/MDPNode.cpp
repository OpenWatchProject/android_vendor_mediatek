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

#include "MDPNode.h"

#define PIPE_CLASS_TAG "MDPNode"
#define PIPE_TRACE TRACE_MDP_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/feature/eis/eis_ext.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

MDPNode::MDPNode(const char *name)
    : StreamingFeatureNode(name)
    , mDpIspStream(DpIspStream::ISP_ZSD_STREAM)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mGpuDatas);
    TRACE_FUNC_EXIT();
}

MDPNode::~MDPNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID MDPNode::setMDPOutputPool(const android::sp<ImageBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mMDPOutputBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL MDPNode::onData(DataID id, const ImgBufferData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    if( id == ID_WARP_TO_MDP_FULLIMG )
    {
        this->mGpuDatas.enque(data);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::convertRGBA8888(IImageBuffer *src, IImageBuffer *dst)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( !src || !dst )
    {
        MY_LOGE("Invalid buffer src(%p)/dst(%p)", src, dst);
        ret = MFALSE;
    }
    else
    {
        // src RGBA8888
        MUINT32 srcWidth = src->getImgSize().w;
        MUINT32 srcHeight = src->getImgSize().h;
        unsigned int srcVA[3] = {0, 0, 0};
        unsigned int srcPA[3] = {0, 0, 0};
        unsigned int srcSize[3] = {0, 0, 0};
        int numPlane = 1;
        srcVA[0] = (unsigned int)src->getBufVA(0);
        srcPA[0] = (unsigned int)src->getBufPA(0);
        srcSize[0] = src->getBufStridesInBytes(0) * src->getImgSize().h;

        mDpBlitStream.setSrcBuffer((void **)srcVA, (void **)srcPA, srcSize, numPlane);
        mDpBlitStream.setSrcConfig(srcWidth, srcHeight, src->getBufStridesInBytes(0), 0, DP_COLOR_RGBA8888, DP_PROFILE_FULL_BT601, eInterlace_None, 0, DP_SECURE_NONE, false);

        // dst YV12
        MUINT32 dstWidth = dst->getImgSize().w;
        MUINT32 dstHeight = dst->getImgSize().h;
        unsigned int dstVA[3] = {0, 0, 0};
        unsigned int dstPA[3] = {0, 0, 0};
        unsigned int dstSize[3] = {0, 0, 0};
        numPlane = 3;
        for( int i = 0; i < numPlane; ++i )
        {
            dstVA[i] = (unsigned int)dst->getBufVA(i);
            dstPA[i] = (unsigned int)dst->getBufPA(i);
            dstSize[i] = dst->getBufSizeInBytes(i);
        }
        mDpBlitStream.setDstBuffer((void**)dstVA, (void**)dstPA, dstSize, numPlane);
        mDpBlitStream.setDstConfig(dstWidth, dstHeight, dst->getBufStridesInBytes(0), (dst->getPlaneCount() > 1 ? dst->getBufStridesInBytes(1) : 0), DP_COLOR_YV12, DP_PROFILE_FULL_BT601, eInterlace_None, 0, DP_SECURE_NONE, false);
        mDpBlitStream.setRotate(0);

        // set & add pipe to stream
        // trigger HW
        ret = (mDpBlitStream.invalidate() == 0);
        if( !ret )
        {
            MY_LOGE("FDstream invalidate failed");
            ret = MFALSE;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::onUninit()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( mPipeUsage.supportVFB() )
    {
        if( mMDPOutputBufferPool != NULL )
        {
            mMDPOutputBufferPool->allocate(NUM_MDP_BUFFER);
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    MBOOL isLastNode;
    RequestPtr request;
    ImgBufferData gpuData;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mGpuDatas.deque(gpuData) )
    {
        MY_LOGE("GpuData deque out of sync");
        return MFALSE;
    }
    if( gpuData.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();

    request = gpuData.mRequest;
    request->mTimer.startMDP();
    TRACE_FUNC("Frame %d in MDP", request->mRequestNo);
    processMDP(request, gpuData.mData);
    request->mTimer.stopMDP();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::processMDP(const RequestPtr &request, const ImgBuffer &gpuOutput)
{
    TRACE_FUNC_ENTER();
    if( request->needVFB() )
    {
        ImgBuffer p2bBuffer;
        MBOOL result;
        result = prepareP2B(request, gpuOutput, p2bBuffer);
        request->updateResult(result);
        handleData(ID_MDP_TO_P2B_FULLIMG, ImgBufferData(p2bBuffer, request));
    }
    else
    {
        handleData(ID_MDP_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_FRAME_DONE, request));
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::prepareP2B(const RequestPtr &request, const ImgBuffer &gpuOutput, ImgBuffer &p2bBuffer)
{
    TRACE_FUNC_ENTER();
    (void)(request);
    MBOOL ret = MTRUE;

    if( useMDPHardware() )
    {
        ImgBuffer src, dst;
        src = gpuOutput;
        dst = mMDPOutputBufferPool->requestIIBuffer();
        if( src->getImageBuffer()->getImgSize() !=
            dst->getImageBuffer()->getImgSize() )
        {
            dst->getImageBuffer()->setExtParam(src->getImageBuffer()->getImgSize());
        }
        this->convertRGBA8888(src->getImageBufferPtr(), dst->getImageBufferPtr());
        p2bBuffer = dst;
    }
    else
    {
        p2bBuffer = gpuOutput;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPNode::convertRGBA8888(IImageBuffer *src, const std::vector<IImageBuffer*> dst, const MCropRect &crop)
{
    //No USE now, may be deprecated later
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    std::vector<IImageBuffer*>::const_iterator it, end;

    if( !src )
    {
        MY_LOGE("Invalid src");
        return MFALSE;
    }

    /* Copy result to PostProc output buffer */
    if( mDpIspStream.setSrcConfig(DP_COLOR_RGBA8888,
            src->getImgSize().w,
            src->getImgSize().h,
            src->getBufStridesInBytes(0),
            false) < 0)
    {
        MY_LOGE("DpIspStream->setSrcConfig failed");
        return MFALSE;
    }

    MUINT32 va[3] = {(MUINT32) src->getBufVA(0),0,0};
    MUINT32 mva[3] = {(MUINT32) src->getBufPA(0),0,0};
    MUINT32 size[3] = {(MUINT32) src->getBufStridesInBytes(0) * src->getImgSize().h, 0, 0};
    if( mDpIspStream.queueSrcBuffer((void**)va, mva, size, 1) < 0 )
    {
        MY_LOGE("DpIspStream->queueSrcBuffer failed");
        return MFALSE;
    }

    if( mDpIspStream.setSrcCrop(crop.p_integral.x, crop.p_fractional.x, crop.p_integral.y, crop.p_fractional.y, crop.s.w,crop.s.h) < 0 )
    {
        MY_LOGE("DpIspStream->setSrcCrop failed");
        return MFALSE;
    }

    for( unsigned i = 0, n = dst.size(); i < n; ++i )
    {
        DpColorFormat dpFmt;
        IImageBuffer* buffer = dst[i];

        if( !buffer )
        {
            MY_LOGE("Invalid output buffer");
            return MFALSE;
        }

        if( !toDpColorFormat((NSCam::EImageFormat) buffer->getImgFormat(), dpFmt) )
        {
            MY_LOGE("Not supported format");
            return MFALSE;
        }

        if( mDpIspStream.setDstConfig(i, //port
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                (buffer->getPlaneCount() > 1 ? buffer->getBufStridesInBytes(1) : 0),
                dpFmt,
                DP_PROFILE_FULL_BT601,
                eInterlace_None,  //default
                NULL, //default
                false) < 0)
        {
            MY_LOGE("DpIspStream->setDstConfig failed");
            return MFALSE;
        }

        MUINT32 va[3]   = {0,0,0};
        MUINT32 size[3] = {0,0,0};
        MUINT32 pa[3]   = {0,0,0};
        for( unsigned j = 0; j < buffer->getPlaneCount(); ++j )
        {
            va[j]   = buffer->getBufVA(j);
            pa[j]   = buffer->getBufPA(j);
            size[j] = buffer->getBufSizeInBytes(j);
        }

        if( mDpIspStream.queueDstBuffer(i, //port
                (void**)va,
                pa,
                size,
                buffer->getPlaneCount()) < 0)
        {
            MY_LOGE("queueDstBuffer failed");
            return MFALSE;
        }
    }

    if( mDpIspStream.startStream() < 0 )
    {
        MY_LOGE("startStream failed");
        return MFALSE;
    }

    if( mDpIspStream.dequeueSrcBuffer() < 0 )
    {
        MY_LOGE("dequeueSrcBuffer failed");
        return MFALSE;
    }

    for( unsigned i = 0, n = dst.size(); i < n; ++i )
    {
        MUINT32 va[3] = {0,0,0};

        if( mDpIspStream.dequeueDstBuffer(i,(void**)va) < 0 )
        {
            MY_LOGE("dequeueDstBuffer failed");
            return MFALSE;
        }
    }

    if( mDpIspStream.stopStream() < 0 )
    {
        MY_LOGE("stopStream failed");
        return MFALSE;
    }

    if( mDpIspStream.dequeueFrameEnd() < 0 )
    {
        MY_LOGE("dequeueFrameEnd failed");
        return MFALSE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPNode::convertRGBA8888(IImageBuffer *src,
                               const android::Vector<NSCam::NSIoPipe::Output> &mvOut,
                               const MCropRect &crop)
{
    MUINT32 i,n;
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    MINT32 result;
    std::vector<IImageBuffer*>::const_iterator it, end;

    if (!src )
    {
        MY_LOGE("Invalid src");
        return MFALSE;
    }
    if( mvOut.size() <= 0 )
    {
        MY_LOGW("skipping process: mvOut.size() = %zu", mvOut.size());
        return MTRUE;
    }

    DpColorFormat inFmt = SUPPORT_GPU_YV12 ? DP_COLOR_YV12 : DP_COLOR_RGBA8888;

    if( SUPPORT_GPU_YV12 )
    {
        result = mDpIspStream.setSrcConfig(src->getImgSize().w,
                                           src->getImgSize().h,
                                           src->getBufStridesInBytes(0),
                                           (src->getBufStridesInBytes(0) >> DP_COLOR_GET_H_SUBSAMPLE(inFmt)),
                                           inFmt,
                                           DP_PROFILE_FULL_BT601,
                                           eInterlace_None,
                                           NULL, false);
    }
    else
    {
        result = mDpIspStream.setSrcConfig(inFmt,
                                           src->getImgSize().w,
                                           src->getImgSize().h,
                                           src->getBufStridesInBytes(0),
                                           false);
    }
    if( result < 0 )
    {
        MY_LOGE("DpIspStream->setSrcConfig failed");
        return MFALSE;
    }

    MUINT32 va[3] = {(MUINT32) src->getBufVA(0),0,0};
    MUINT32 mva[3] = {(MUINT32) src->getBufPA(0),0,0};
    MUINT32 size[3] = {(MUINT32) src->getBufStridesInBytes(0) * src->getImgSize().h, 0, 0};
    MUINT32 planeCount = 1;

    if( SUPPORT_GPU_YV12 )
    {
        size[0] = src->getImgSize().w * src->getImgSize().h;
        for( unsigned i = 1; i < 3; ++i )
        {
            va[i] = (MUINT32)src->getBufVA(i);
            mva[i] = (MUINT32)src->getBufPA(i);
            size[i] = size[0] >> 2;
        }
        planeCount = DP_COLOR_GET_PLANE_COUNT(inFmt);
    }

    if (mDpIspStream.queueSrcBuffer((void**)va, mva, size, planeCount) < 0 )
    {
        MY_LOGE("DpIspStream->queueSrcBuffer failed");
        return MFALSE;
    }

    if (mDpIspStream.setSrcCrop(crop.p_integral.x, crop.p_fractional.x, crop.p_integral.y, crop.p_fractional.y, crop.s.w,crop.s.h) < 0 )
    {
        MY_LOGE("DpIspStream->setSrcCrop failed");
        return MFALSE;
    }

    for (i = 0, n = mvOut.size(); i < n; ++i )
    {
        DpColorFormat dpFmt;
        IImageBuffer* buffer = mvOut[i].mBuffer;

        if (!buffer )
        {
            MY_LOGE("Invalid output buffer");
            return MFALSE;
        }

        if (!toDpColorFormat((NSCam::EImageFormat) buffer->getImgFormat(), dpFmt) )
        {
            MY_LOGE("Not supported format");
            return MFALSE;
        }


        MUINT32 u4Rotation;
        MUINT32 u4Flip;
        MUINT32 u4Transform;

        u4Transform = mvOut[i].mTransform;

        switch (u4Transform)
        {
#define TransCase( trans, rot, flip ) \
        case (trans):                 \
            u4Rotation = (rot);       \
            u4Flip = (flip);          \
            break;
        TransCase(0                  , 0   , 0)
        TransCase(eTransform_FLIP_H  , 0   , 1)
        TransCase(eTransform_FLIP_V  , 180 , 1)
        TransCase(eTransform_ROT_90  , 90  , 0)
        TransCase(eTransform_ROT_180 , 180 , 0)
        TransCase(eTransform_FLIP_H|eTransform_ROT_90 , 270 , 1)
        TransCase(eTransform_FLIP_V|eTransform_ROT_90 , 90  , 1)
        TransCase(eTransform_ROT_270 , 270 , 0)
        default:
            MY_LOGE("not supported transform(0x%x)", u4Transform);
            u4Rotation = 0;
            u4Flip = 0;
            break;
#undef TransCase
        }

        MY_LOGD("u4Transform %d, Rotation %d, Flip %d", u4Transform, u4Rotation, u4Flip);

        if(mDpIspStream.setRotation(i, u4Rotation))
        {
            MY_LOGE("DpIspStream->setDstConfig failed");
            return MFALSE;
        }

        if(mDpIspStream.setFlipStatus(i, u4Flip))
        {
            MY_LOGE("DpIspStream->setDstConfig failed");
            return MFALSE;
        }

        DP_PROFILE_ENUM bProfile = DP_PROFILE_FULL_BT601;
        if (mvOut[i].mPortID.capbility == NSCam::NSIoPipe::EPortCapbility_Rcrd)
        {
            bProfile = DP_PROFILE_BT601;
            MY_LOGD("mvOut record port ID: %d", i);
        }

        if (mDpIspStream.setDstConfig(i, //port
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                (buffer->getPlaneCount() > 1 ? buffer->getBufStridesInBytes(1) : 0),
                dpFmt,
                bProfile,
                eInterlace_None,  //default
                NULL, //default
                false) < 0)
        {
            MY_LOGE("DpIspStream->setDstConfig failed");
            return MFALSE;
        }

        MUINT32 va[3]   = {0,0,0};
        MUINT32 size[3] = {0,0,0};
        MUINT32 pa[3]   = {0,0,0};
        for (unsigned j = 0; j < buffer->getPlaneCount(); ++j )
        {
            va[j]   = buffer->getBufVA(j);
            pa[j]   = buffer->getBufPA(j);
            size[j] = buffer->getBufSizeInBytes(j);
        }

        if (mDpIspStream.queueDstBuffer(i, //port
                (void**)va,
                pa,
                size,
                buffer->getPlaneCount()) < 0)
        {
            MY_LOGE("queueDstBuffer failed");
            return MFALSE;
        }
    }

    if (mDpIspStream.startStream() < 0 )
    {
        MY_LOGE("startStream failed");
        return MFALSE;
    }

    if (mDpIspStream.dequeueSrcBuffer() < 0 )
    {
        MY_LOGE("dequeueSrcBuffer failed");
        return MFALSE;
    }

    for ( i = 0, n = mvOut.size(); i < n; ++i )
    {
        MUINT32 va[3] = {0,0,0};

        if( mDpIspStream.dequeueDstBuffer(i,(void**)va) < 0 )
        {
            MY_LOGE("dequeueDstBuffer failed");
            return MFALSE;
        }
    }

    if (mDpIspStream.stopStream() < 0 )
    {
        MY_LOGE("stopStream failed");
        return MFALSE;
    }

    if (mDpIspStream.dequeueFrameEnd() < 0 )
    {
        MY_LOGE("dequeueFrameEnd failed");
        return MFALSE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}



} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
