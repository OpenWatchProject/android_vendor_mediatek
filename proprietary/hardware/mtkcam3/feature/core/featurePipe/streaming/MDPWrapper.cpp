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

#include "StreamingFeature_Common.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

#define DP_PORT_SRC -1

using NSCam::Feature::P2Util::P2IOClassfier;

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

MBOOL MDPWrapper::findUnusedMDPPort(const OUTPUT_ARRAY &outputs, unsigned &index)
{
    MBOOL ret = MFALSE;
    MBOOL wrotUsed = MFALSE;
    MBOOL wdmaUsed = MFALSE;

    for( int i = 0, n = outputs.size(); i < n; ++i )
    {
        unsigned outIndex = outputs[i].mOutput.mPortID.index;
        if( outIndex == NSImageio::NSIspio::EPortIndex_WROTO )
        {
            wrotUsed = MTRUE;
        }
        else if( outIndex == NSImageio::NSIspio::EPortIndex_WDMAO )
        {
            wdmaUsed = MTRUE;
        }
    }

    if( !wdmaUsed )
    {
        index = NSImageio::NSIspio::EPortIndex_WDMAO;
        ret = MTRUE;
    }
    else if( !wrotUsed )
    {
        index = NSImageio::NSIspio::EPortIndex_WROTO;
        ret = MTRUE;
    }

    return ret;
}

MDPWrapper::MDPWrapper(const std::string& name)
    : mDpIspStream(DpIspStream::ISP_ZSD_STREAM)
    , mMaxMDPDsts(DpIspStream::queryMultiPortSupport(DpIspStream::ISP_ZSD_STREAM))
    , mName(name)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("[%s] out port count=%d", mName.c_str(), mMaxMDPDsts);
    TRACE_FUNC_EXIT();
}

MUINT32 MDPWrapper::getNumOutPort() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mMaxMDPDsts;
}

MVOID MDPWrapper::pushMDPOut(OUTPUT_ARRAY &outArray, NSCam::NSIoPipe::PortID &portID, const P2IO &out)
{
    DpPqParam pqParam;
    NSCam::NSIoPipe::Output output;
    MCropRect cropRect;

    output = toOutput(out, portID);
    cropRect = toMCropRect(out);
    if(out.mDpPqParam != NULL)
    {
        pqParam = *(static_cast<DpPqParam*>(out.mDpPqParam));
    }
    else
    {
        pqParam.scenario = MEDIA_ISP_PREVIEW;
        pqParam.enable = false;
    }

    outArray.push_back(MDPWrapper::MDPOutput(output, cropRect, pqParam));
}

MVOID MDPWrapper::collectOneRun(P2IOClassfier &collect, OUTPUT_ARRAY &outArray, MBOOL allowDummy)
{
    // put in wdma port
    MUINT32 count = 0;
    NSCam::NSIoPipe::PortID target = NSIoPipe::PORT_WDMAO;
    if( collect.hasNonRotateIO())
    {
        pushMDPOut(outArray, target, collect.popNonRotate());
        count++;
    }

    // put in wrot port
    target = NSIoPipe::PORT_WROTO;
    if( collect.hasRotateIO())
    {
        pushMDPOut(outArray, target, collect.popRotate());
        count++;
    }
    else if(collect.hasNonRotateIO())
    {
        pushMDPOut(outArray, target, collect.popNonRotate());
        count++;
    }

    // put dummy if needed
    if(allowDummy && collect.hasIO() && mMaxMDPDsts > 1 && count < mMaxMDPDsts)
    {
        MDPOutput out;
        out.mDummy = MTRUE;
        outArray.push_back(out);
    }
}

MVOID MDPWrapper::generateOutArray(const P2IO_OUTPUT_ARRAY &inList, OUTPUT_ARRAY &outList, MBOOL allowDummy)
{
    TRACE_FUNC_ENTER();
    P2IOClassfier collection(inList);
    while( collection.hasIO() )
    {
        collectOneRun(collection, outList, allowDummy);
    }
    TRACE_FUNC_EXIT();
}

MBOOL MDPWrapper::process(IImageBuffer *src, const P2IO_OUTPUT_ARRAY &dst, MBOOL printIO, MUINT32 cycleTimeMs)
{
    TRACE_FUNC_ENTER();
    OUTPUT_ARRAY outArray;
    generateOutArray(dst, outArray, MTRUE);
    MBOOL ret = process(src, outArray, printIO, cycleTimeMs);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::process(IImageBuffer *src, const OUTPUT_ARRAY &dst, MBOOL printIO, MUINT32 cycleTimeMs)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( src && dst.size() )
    {
        OUTPUT_ARRAY::const_iterator start = dst.begin();
        OUTPUT_ARRAY subDst;
        subDst.reserve(mMaxMDPDsts);

        while( prepareSubDst(dst, start, mMaxMDPDsts, subDst) )
        {
            MBOOL subRet = MTRUE;
            subRet = prepareMDPSrc(src) &&
                     prepareMDPDst(subDst) &&
                     prepareMDPOut(subDst.size(), cycleTimeMs);

            if( !subRet )
            {
                MY_LOGW("MDP not processed");
                dumpInfo(src, subDst, cycleTimeMs);
            }

            if( printIO )
            {
                dumpInfo(src, subDst, cycleTimeMs);
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

MBOOL MDPWrapper::process(IImageBuffer *src, IImageBuffer *dst, MBOOL printIO, MUINT32 cycleTimeMs)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    OUTPUT_ARRAY outArray;
    Output output;
    if( src && dst )
    {
        MINT32 transform = 0;
        MDPOutput output(Output(NSIoPipe::PORT_WDMAO, dst, transform),
                         MCropRect(MPoint(), src->getImgSize()));
        outArray.push_back(output);
        ret = process(src, outArray, printIO, cycleTimeMs);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPWrapper::prepareSubDst(const OUTPUT_ARRAY &dst, OUTPUT_ARRAY::const_iterator &start, MUINT32 count, OUTPUT_ARRAY& subDst)
{
    TRACE_FUNC_ENTER();
    subDst.clear();
    MUINT32 itCnt = 0;
    while(start < dst.end() && itCnt < count)
    {
        itCnt++;
        if(!start->mDummy)
        {
            subDst.push_back(*start);
        }
        start++;
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
            cropRect.w_fractional,
            cropRect.s.h,
            cropRect.h_fractional) < 0 )
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
        else if( (pqParam.enable || pqParam.u.isp.enableDump) && mDpIspStream.setPQParameter(i, pqParam))
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

MBOOL MDPWrapper::prepareMDPOut(MUINT32 outSize, MUINT32 cycleTimeMs)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    struct timeval diff, curr, target;
    timeval *targetPtr = NULL;
    if(cycleTimeMs != 0)
    {
        diff.tv_sec = 0;
        diff.tv_usec = cycleTimeMs * 1000;
        gettimeofday(&curr, NULL);
        timeradd(&curr, &diff, &target);
        targetPtr = &target;
    }

    if( mDpIspStream.startStream(targetPtr) < 0 )
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
            void* va[3] = {0};
            if( mDpIspStream.dequeueDstBuffer(i, va) < 0 )
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
    void* va[3] = {0};
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
            va[i]   = (void*)buffer->getBufVA(i);
            mva[i]  = buffer->getBufPA(i);
            size[i] = buffer->getBufSizeInBytes(i);
        }
        if( port == DP_PORT_SRC )
        {
            if( mDpIspStream.queueSrcBuffer(va, mva, size, planeCount) < 0 )
            {
                MY_LOGE("DpIspStream->queueSrcBuffer failed");
                ret = MFALSE;
            }
        }
        else
        {
            if( mDpIspStream.queueDstBuffer(port, va, mva, size, planeCount) < 0 )
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

MVOID MDPWrapper::dumpInfo(IImageBuffer *src, const OUTPUT_ARRAY &dst, MUINT32 cycleTimeMs)
{
    TRACE_FUNC_ENTER();
    MY_LOGD("[%s] src IImageBuffer=%p, targetCycleTimeMs=%d, out port count=%d", mName.c_str(), src, cycleTimeMs, mMaxMDPDsts);
    if( src )
    {
        EImageFormat fmt = (EImageFormat)src->getImgFormat();
        MY_LOGD("src fmt=0x%x(%s) size=(%dx%d) plane:%zu", fmt, toName(fmt), src->getImgSize().w, src->getImgSize().h, src->getPlaneCount());
        for( unsigned p = 0, n = src->getPlaneCount(); p < n; ++p )
        {
            void* va = (void*)src->getBufVA(p);
            MUINT32 mva = src->getBufPA(p);
            MUINT32 size = src->getBufSizeInBytes(p);
            MY_LOGD("src plane[%d] va=%p mva=0x%x size=%d", p, va, mva, size);
        }
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
                EImageFormat fmt = (EImageFormat)buffer->getImgFormat();
                MY_LOGD("dst[%d] fmt=0x%x(%s) size=(%dx%d) plane:%zu ID(%d) cap(%d) crop=(%d,%d).(%d,%d),(%dx%d).(%dx%d)",
                    i, fmt, toName(fmt), buffer->getImgSize().w, buffer->getImgSize().h, buffer->getPlaneCount(), output.mPortID.index, output.mPortID.capbility,
                    cropRect.p_integral.x, cropRect.p_integral.y, cropRect.p_fractional.x, cropRect.p_fractional.y,
                    cropRect.s.w, cropRect.s.h, cropRect.w_fractional, cropRect.h_fractional);
                MY_LOGD("dst[%d] pq enable=%d iso:%d timestamp:%d frameNo:%d requestNo:%d lensID:%d CZ.captureShot:%d", i, pqParam.enable, ispParam.iso,
                    ispParam.timestamp, ispParam.frameNo, ispParam.requestNo, ispParam.lensId, ispParam.clearZoomParam.captureShot);
                for( unsigned p = 0, pn = src->getPlaneCount(); p < pn; ++p )
                {
                    void* va = (void*)src->getBufVA(p);
                    MUINT32 mva = src->getBufPA(p);
                    MUINT32 size = src->getBufSizeInBytes(p);
                    MY_LOGD("dst[%d]plane[%d] va=%p mva=0x%x size=%d", i, p, va, mva, size);
                }
            }
        }
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
