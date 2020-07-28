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

#include "MDPWrapper.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "MDPWrapper"
#define PIPE_TRACE TRACE_MDP_WRAPPER
#include <featurePipe/core/include/PipeLog.h>

#define DP_PORT_SRC -1

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

MBOOL MDPWrapper::toDpColorFormat(NSCam::EImageFormat fmt, DpColorFormat &dpFmt)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    switch( fmt )
    {
    case eImgFmt_YUY2:      dpFmt = DP_COLOR_YUYV;      break;
    case eImgFmt_UYVY:      dpFmt = DP_COLOR_UYVY;      break;
    case eImgFmt_YVYU:      dpFmt = DP_COLOR_YVYU;      break;
    case eImgFmt_VYUY:      dpFmt = DP_COLOR_VYUY;      break;
    case eImgFmt_NV16:      dpFmt = DP_COLOR_NV16;      break;
    case eImgFmt_NV61:      dpFmt = DP_COLOR_NV61;      break;
    case eImgFmt_NV21:      dpFmt = DP_COLOR_NV21;      break;
    case eImgFmt_NV12:      dpFmt = DP_COLOR_NV12;      break;
    case eImgFmt_YV16:      dpFmt = DP_COLOR_YV16;      break;
    case eImgFmt_I422:      dpFmt = DP_COLOR_I422;      break;
    case eImgFmt_YV12:      dpFmt = DP_COLOR_YV12;      break;
    case eImgFmt_I420:      dpFmt = DP_COLOR_I420;      break;
    case eImgFmt_Y800:      dpFmt = DP_COLOR_GREY;      break;
    case eImgFmt_STA_BYTE:  dpFmt = DP_COLOR_GREY;      break;
    case eImgFmt_RGB565:    dpFmt = DP_COLOR_RGB565;    break;
    case eImgFmt_RGB888:    dpFmt = DP_COLOR_RGB888;    break;
    case eImgFmt_ARGB888:   dpFmt = DP_COLOR_ARGB8888;  break;
    case eImgFmt_RGBA8888:  dpFmt = DP_COLOR_RGBA8888;  break;
    default:
        MY_LOGE("fmt(0x%x) not support in DP", fmt);
        ret = MFALSE;
        break;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::toDpColorFormat(MINT fmt, DpColorFormat &dpFmt)
{
    return toDpColorFormat((NSCam::EImageFormat)fmt, dpFmt);
}

MBOOL MDPWrapper::toDpTransform(MUINT32 transform, MUINT32 &rotation, MUINT32 &flip)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    switch (transform)
    {
#define TransCase(t, r, f)    \
    case (t):                 \
        rotation = (r);       \
        flip = (f);           \
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
        MY_LOGE("not supported transform(0x%x)", transform);
        ret = MFALSE;
        break;
#undef TransCase
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::isRecordPort(const Output &output)
{
    return output.mPortID.capbility == NSCam::NSIoPipe::EPortCapbility_Rcrd;
}

MDPWrapper::MDPWrapper()
    : mDpIspStream(DpIspStream::ISP_ZSD_STREAM)
    , mMaxMDPDsts(DpIspStream::queryMultiPortSupport(DpIspStream::ISP_ZSD_STREAM))
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL MDPWrapper::process(IImageBuffer *src, const OUTPUT_ARRAY &dst)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( src && dst.size() )
    {
        OUTPUT_ARRAY::const_iterator start = dst.begin();
        OUTPUT_ARRAY subDst;

        while( prepareSubDst(dst, start, mMaxMDPDsts, subDst) )
        {
            MBOOL subRet = MTRUE;
            subRet = prepareMDPSrc(src) &&
                     prepareMDPDst(subDst) &&
                     prepareMDPOut(subDst.size());

            if( !subRet )
            {
                MY_LOGW("MDP not processed");
                dumpInfo(src, subDst);
            }

            ret &= subRet;
        }
    }
    else
    {
        MY_LOGD("Invalid config src=%p, dst.size=%zu", src, dst.size());
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::prepareSubDst(const OUTPUT_ARRAY &dst, OUTPUT_ARRAY::const_iterator &start, MUINT32 count, OUTPUT_ARRAY& subDst)
{
    TRACE_FUNC_ENTER();

    if( start + count < dst.end() )
    {
        subDst = OUTPUT_ARRAY(start, start + count);
        start += count;
    }
    else
    {
        subDst = OUTPUT_ARRAY(start, dst.end());
        start = dst.end();
    }

    TRACE_FUNC_EXIT();

    return !subDst.empty();
}

MBOOL MDPWrapper::prepareMDPSrc(IImageBuffer *src)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    DpColorFormat dpFormat;

    if( !src )
    {
        MY_LOGE("Invalid src");
    }
    else if( !toDpColorFormat(src->getImgFormat(), dpFormat) )
    {
        MY_LOGE("Invalid src color format");
    }
    else if( mDpIspStream.setSrcConfig(
                src->getImgSize().w,
                src->getImgSize().h,
                src->getBufStridesInBytes(0),
                (src->getPlaneCount() > 1 ? src->getBufStridesInBytes(1) : 0),
                dpFormat,
                DP_PROFILE_FULL_BT601,
                eInterlace_None,
                NULL,
                false) < 0 )
    {
        MY_LOGE("DpIspStream->setSrcConfig failed");
    }
    else if( !queueMDPBuffer(DP_PORT_SRC, src) )
    {
        MY_LOGE("queue MDP src failed");
    }
    else
    {
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::prepareMDPDst(const OUTPUT_ARRAY &dst)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    DP_PROFILE_ENUM profile;
    MUINT32 rotate, flip;
    DpColorFormat colorFormat;
    for( int i = 0, n = dst.size(); ret && i < n; ++i )
    {
        ret = MFALSE;
        const Output &output = dst[i].mOutput;
        const MCropRect &cropRect = dst[i].mCropRect;
        const DpPqParam &pqParam = dst[i].mPqParam;
        IImageBuffer *buffer = output.mBuffer;
        if( !buffer || !prepareMDPDstConfig(i, output, rotate, flip, colorFormat, profile) )
        {
            MY_LOGE("Invalid output[%d] buffer=%p", i, buffer);
        }
        else if( mDpIspStream.setRotation(i, rotate) )
        {
            MY_LOGE("DpIspStream->setRotation(%d, %d) failed", i, rotate);
        }
        else if( mDpIspStream.setFlipStatus(i, flip) )
        {
            MY_LOGE("DpIspStream->setFlipStatus(%d, %d) failed", i, flip);
        }
        else if( mDpIspStream.setSrcCrop(
            i, //port
            cropRect.p_integral.x,
            cropRect.p_fractional.x,
            cropRect.p_integral.y,
            cropRect.p_fractional.y,
            cropRect.s.w,
            cropRect.s.h) < 0 )
        {
            MY_LOGE("DpIspStream->setSrcCrop[%d] failed", i);
        }
        else if( mDpIspStream.setDstConfig(i, //port
                    buffer->getImgSize().w,
                    buffer->getImgSize().h,
                    buffer->getBufStridesInBytes(0),
                    (buffer->getPlaneCount() > 1 ? buffer->getBufStridesInBytes(1) : 0),
                    colorFormat,
                    profile,
                    eInterlace_None,  //default
                    NULL, //default
                    false) < 0 )
        {
            MY_LOGE("DpIspStream->setDstConfig[%d] failed", i);
        }
        else if( pqParam.enable && mDpIspStream.setPQParameter(i, pqParam))
        {
            MY_LOGE("DpIspStream->setPQParameter[%d] failed", i);
        }
        else
        {
            if( !(ret = queueMDPBuffer(i, buffer)) )
            {
                MY_LOGE("queueMDPBuffer(%d, %p) failed", i, buffer);
            }
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::prepareMDPOut(MUINT32 outSize)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( mDpIspStream.startStream() < 0 )
    {
        MY_LOGE("DpIspStream->startStream failed");
        ret = MFALSE;
    }
    if( ret && mDpIspStream.dequeueSrcBuffer() < 0 )
    {
        MY_LOGE("DpIspStream->dequeueSrcBuffer failed");
        ret = MFALSE;
    }
    if( ret )
    {
        for ( MUINT32 i = 0; i < outSize; ++i )
        {
            MUINT32 va[3] = {0};
            if( mDpIspStream.dequeueDstBuffer(i, (void**)va) < 0 )
            {
                MY_LOGE("DpIspStream->dequeueDstBuffer[%d] failed", i);
                ret = MFALSE;
            }
        }
    }
    if( ret && mDpIspStream.stopStream() < 0 )
    {
        MY_LOGE("DpIspStream->stopStream failed");
        ret = MFALSE;
    }
    if( ret && mDpIspStream.dequeueFrameEnd() < 0 )
    {
        MY_LOGE("DpIspStream->dequeueFrameEnd failed");
        ret = MFALSE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::queueMDPBuffer(MINT32 port, const IImageBuffer *buffer)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    MUINT32 va[3] = {0};
    MUINT32 mva[3] = {0};
    MUINT32 size[3] = {0};
    MUINT   planeCount = 0;

    if( !buffer )
    {
        MY_LOGE("Invalid buffer for port=%d", port);
        ret = MFALSE;
    }
    else if( (planeCount = buffer->getPlaneCount()) > 3 )
    {
        MY_LOGE("Does not support port(%d) with planeCount(%d) > 3", port, planeCount);
        ret = MFALSE;
    }
    else
    {
        for( MUINT i = 0; i < planeCount; ++i )
        {
            va[i]   = (MUINT32)buffer->getBufVA(i);
            mva[i]  = buffer->getBufPA(i);
            size[i] = buffer->getBufSizeInBytes(i);
        }
        if( port == DP_PORT_SRC )
        {
            if( mDpIspStream.queueSrcBuffer((void**)va, mva, size, planeCount) < 0 )
            {
                MY_LOGE("DpIspStream->queueSrcBuffer failed");
                ret = MFALSE;
            }
        }
        else
        {
            if( mDpIspStream.queueDstBuffer(port, (void**)va, mva, size, planeCount) < 0 )
            {
                MY_LOGE("DpIspStream->queueDstBuffer port(%d) failed", port);
                ret = MFALSE;
            }
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::prepareMDPDstConfig(MUINT32 index, const Output &output, MUINT32 &rotate, MUINT32 &flip, DpColorFormat &colorFormat, DP_PROFILE_ENUM &profile)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    IImageBuffer *buffer = output.mBuffer;

    if( !buffer )
    {
        MY_LOGE("Invalid output[%d] buffer", index);
        ret = MFALSE;
    }
    else if( !toDpColorFormat(buffer->getImgFormat(), colorFormat) )
    {
        MY_LOGE("Invalid output[%d] color format=0x%x", index, buffer->getImgFormat());
        ret = MFALSE;
    }
    else
    {
        MUINT32 transform = output.mTransform;
        if( !toDpTransform(transform, rotate, flip) )
        {
            rotate = flip = 0;
        }
        profile = isRecordPort(output) ? DP_PROFILE_BT601 : DP_PROFILE_FULL_BT601;
        TRACE_FUNC("Output[%d] transformat=0x%x rotate=%d flip=%d colorFormat=0x%x profile=0x%x", index, transform, rotate, flip, colorFormat, profile);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID MDPWrapper::dumpInfo(IImageBuffer *src, const OUTPUT_ARRAY &dst)
{
    TRACE_FUNC_ENTER();
    MY_LOGD("src IImageBuffer=%p", src);
    if( src )
    {
        MY_LOGD("src fmt=%d size=(%dx%d)", src->getImgFormat(), src->getImgSize().w, src->getImgSize().h);
        for( int i = 0, n = dst.size(); i < n; ++i )
        {
            const Output &output = dst[i].mOutput;
            const MCropRect &cropRect = dst[i].mCropRect;
            const DpPqParam &pqParam = dst[i].mPqParam;
            const DpIspParam &ispParam = pqParam.u.isp;
            IImageBuffer *buffer = output.mBuffer;

            MY_LOGD("dst[%d] IImageBuffer=%p", i, buffer);
            if( buffer )
            {
                MY_LOGD("dst[%d] fmt=%d size=(%dx%d) crop=(%d,%d)(%dx%d)", i, buffer->getImgFormat(), buffer->getImgSize().w, buffer->getImgSize().h,
                    cropRect.p_integral.x, cropRect.p_integral.y, cropRect.s.w, cropRect.s.h);
                MY_LOGD("dst[%d] pq enable=%d iso:%d timestamp:%d frameNo:%d requestNo:%d lensId:%d CZ.captureShot:%d", i, pqParam.enable, ispParam.iso,
                    ispParam.timestamp, ispParam.frameNo, ispParam.requestNo, ispParam.lensId, ispParam.clearZoomParam.captureShot);
            }
        }
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
