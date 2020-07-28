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

#include "MtkHeader.h"

#include <featurePipe/core/include/DebugUtil.h>

#include "StreamingFeature_Common.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "Util"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_COMMON
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include "MDPWrapper.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using NSImageio::NSIspio::EPortIndex;
using NSImageio::NSIspio::EPortIndex_BPCI;
using NSImageio::NSIspio::EPortIndex_DEPI;
using NSImageio::NSIspio::EPortIndex_DMGI;
using NSImageio::NSIspio::EPortIndex_LCEI;
using NSImageio::NSIspio::EPortIndex_LSCI;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_IMGBI;
using NSImageio::NSIspio::EPortIndex_IMGCI;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::VirDIPPortIdx_YNR_FACEI;
using NSImageio::NSIspio::VirDIPPortIdx_YNR_LCEI;

using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_DCESO;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const char* Fmt2Name(MINT fmt)
{
    switch(fmt)
    {
    case eImgFmt_BAYER8:            return "bayer8";
    case eImgFmt_BAYER10:           return "bayer10";
    case eImgFmt_BAYER12:           return "bayer12";
    case eImgFmt_BAYER14:           return "bayer14";
    case eImgFmt_FG_BAYER8:         return "fgbayer8";
    case eImgFmt_FG_BAYER10:        return "fgbayer10";
    case eImgFmt_FG_BAYER12:        return "fgbayer12";
    case eImgFmt_FG_BAYER14:        return "fgbayer14";
    case eImgFmt_RGBA8888:          return "rgba";
    case eImgFmt_RGB888:            return "rgb";
    case eImgFmt_YUY2:              return "yuy2";
    case eImgFmt_YV12:              return "yv12";
    case eImgFmt_NV21:              return "nv21";
    case eImgFmt_NV12:              return "nv12";
    default:                        return "unknown";
    };
}

MBOOL useMDPHardware()
{
    return MTRUE;
}

MBOOL isTargetOutput(IO_TYPE target, const SFPOutput &output)
{
    if( target == IO_TYPE_DISPLAY )
    {
        return isDisplayOutput(output);
    }
    else if( target == IO_TYPE_RECORD )
    {
        return isRecordOutput(output);
    }
    else if( target == IO_TYPE_EXTRA )
    {
        return isExtraOutput(output);
    }
    else if( target == IO_TYPE_FD )
    {
        return isFDOutput(output);
    }
    else if( target == IO_TYPE_PHYSICAL )
    {
        return isPhysicalOutput(output);
    }
    else
    {
        MY_LOGW("Invalid output target type = %d", target);
    }
    return MFALSE;
}

MBOOL isExtraOutput(const SFPOutput &output)
{
    return (output.mTargetType == SFPOutput::OUT_TARGET_UNKNOWN);
}

MBOOL isDisplayOutput(const SFPOutput &output)
{
    return (output.mTargetType == SFPOutput::OUT_TARGET_DISPLAY);
}

MBOOL isRecordOutput(const SFPOutput &output)
{
    return (output.mTargetType == SFPOutput::OUT_TARGET_RECORD);
}

MBOOL isFDOutput(const SFPOutput &output)
{
    return (output.mTargetType == SFPOutput::OUT_TARGET_FD);
}

MBOOL isPhysicalOutput(const SFPOutput &output)
{
    return (output.mTargetType == SFPOutput::OUT_TARGET_PHYSICAL);
}


MBOOL isTargetOutput(IO_TYPE target, const NSCam::NSIoPipe::Output &output)
{
    if( target == IO_TYPE_DISPLAY )
    {
        return isDisplayOutput(output);
    }
    else if( target == IO_TYPE_RECORD )
    {
        return isRecordOutput(output);
    }
    else if( target == IO_TYPE_EXTRA )
    {
        return isExtraOutput(output);
    }
    else if( target == IO_TYPE_FD )
    {
        return isFDOutput(output);
    }
    else
    {
        MY_LOGW("Invalid output target type = %d", target);
    }
    return MFALSE;
}

MBOOL isExtraOutput(const NSCam::NSIoPipe::Output &output)
{
    return (output.mPortID.capbility == NSCam::NSIoPipe::EPortCapbility_None && !isFDOutput(output)) ||
        (output.mPortID.capbility == NSCam::NSIoPipe::EPortCapbility_Rcrd && (getGraphicBufferAddr(output.mBuffer) == NULL));
}

MBOOL isDisplayOutput(const NSCam::NSIoPipe::Output &output)
{
    return output.mPortID.capbility == NSCam::NSIoPipe::EPortCapbility_Disp;
}

MBOOL isRecordOutput(const NSCam::NSIoPipe::Output &output)
{
    return (output.mPortID.capbility == NSCam::NSIoPipe::EPortCapbility_Rcrd) &&
           (getGraphicBufferAddr(output.mBuffer) != NULL );
}

MBOOL isFDOutput(const NSCam::NSIoPipe::Output &output)
{
    return output.mPortID.index == NSImageio::NSIspio::EPortIndex_IMG2O;
}

MBOOL getFrameInInfo(FrameInInfo &info, const Feature::P2Util::DIPFrameParams &frame, const PortID &portID)
{
    IImageBuffer *buffer = Feature::P2Util::findInputBuffer(frame, portID);
    if( buffer != NULL )
    {
        info.inSize = buffer->getImgSize();
        info.timestamp = buffer->getTimestamp();
        return MTRUE;
    }
    return MFALSE;
}

MBOOL findUnusedMDPPort(const Feature::P2Util::DIPParams &dipParam, unsigned &index)
{
    MBOOL ret = MFALSE;
    MBOOL wrotUsed = MFALSE;
    MBOOL wdmaUsed = MFALSE;

    if( dipParam.mvDIPFrameParams.size() > 0 )
    {
        for( unsigned i = 0, count = dipParam.mvDIPFrameParams[0].mvOut.size(); i < count; ++i )
        {
            unsigned outIndex = dipParam.mvDIPFrameParams[0].mvOut[i].mPortID.index;
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
    }

    return ret;
}

MBOOL findUnusedMDPCropGroup(const Feature::P2Util::DIPParams &dipParam, unsigned &cropGroup)
{
    MBOOL ret = MFALSE;
    MBOOL wrotUsed = MFALSE;
    MBOOL wdmaUsed = MFALSE;

    if( dipParam.mvDIPFrameParams.size() > 0 )
    {
        for( unsigned i = 0, count = dipParam.mvDIPFrameParams[0].mvCropRsInfo.size(); i < count; ++i )
        {
            unsigned outCropGroup = dipParam.mvDIPFrameParams[0].mvCropRsInfo[i].mGroupID;
            if( outCropGroup == WROTO_CROP_GROUP )
            {
                wrotUsed = MTRUE;
            }
            else if( outCropGroup == WDMAO_CROP_GROUP )
            {
                wdmaUsed = MTRUE;
            }
        }

        if( !wdmaUsed )
        {
            cropGroup = WDMAO_CROP_GROUP;
            ret = MTRUE;
        }
        else if( !wrotUsed )
        {
            cropGroup = WROTO_CROP_GROUP;
            ret = MTRUE;
        }
    }

    return ret;
}

MSize toValid(const MSize &size, const MSize &def)
{
    return (size.w && size.h) ? size : def;
}

NSCam::EImageFormat toValid(const NSCam::EImageFormat fmt, const NSCam::EImageFormat def)
{
    return (fmt == eImgFmt_UNKNOWN) ? def : fmt;
}

const char* toName(const NSCam::EImageFormat fmt)
{
    const char* name = "unknown";
    switch( fmt )
    {
    case eImgFmt_YUY2:      name = "YUY2";  break;
    case eImgFmt_UYVY:      name = "UYVY";  break;
    case eImgFmt_YVYU:      name = "YVYU";  break;
    case eImgFmt_VYUY:      name = "VYUY";  break;
    case eImgFmt_NV16:      name = "NV16";  break;
    case eImgFmt_NV61:      name = "NV61";  break;
    case eImgFmt_NV21:      name = "NV21";  break;
    case eImgFmt_NV12:      name = "NV12";  break;
    case eImgFmt_YV16:      name = "YV16";  break;
    case eImgFmt_I422:      name = "I422";  break;
    case eImgFmt_YV12:      name = "YV12";  break;
    case eImgFmt_I420:      name = "I420";  break;
    case eImgFmt_Y800:      name = "Y800";  break;
    case eImgFmt_STA_BYTE:  name = "BYTE";  break;
    case eImgFmt_RGB565:    name = "RGB565";  break;
    case eImgFmt_RGB888:    name = "RGB888";  break;
    case eImgFmt_ARGB888:   name = "ARGB888"; break;
    default:                name = "unknown"; break;
    }
    return name;
}

MBOOL toDpColorFormat(const NSCam::EImageFormat fmt, DpColorFormat &dpFmt)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    switch( fmt )
    {
    case eImgFmt_YUY2:    dpFmt = DP_COLOR_YUYV;      break;
    case eImgFmt_UYVY:    dpFmt = DP_COLOR_UYVY;      break;
    case eImgFmt_YVYU:    dpFmt = DP_COLOR_YVYU;      break;
    case eImgFmt_VYUY:    dpFmt = DP_COLOR_VYUY;      break;
    case eImgFmt_NV16:    dpFmt = DP_COLOR_NV16;      break;
    case eImgFmt_NV61:    dpFmt = DP_COLOR_NV61;      break;
    case eImgFmt_NV21:    dpFmt = DP_COLOR_NV21;      break;
    case eImgFmt_NV12:    dpFmt = DP_COLOR_NV12;      break;
    case eImgFmt_YV16:    dpFmt = DP_COLOR_YV16;      break;
    case eImgFmt_I422:    dpFmt = DP_COLOR_I422;      break;
    case eImgFmt_YV12:    dpFmt = DP_COLOR_YV12;      break;
    case eImgFmt_I420:    dpFmt = DP_COLOR_I420;      break;
    case eImgFmt_Y800:    dpFmt = DP_COLOR_GREY;      break;
    case eImgFmt_STA_BYTE: dpFmt = DP_COLOR_GREY;      break;
    case eImgFmt_RGB565:  dpFmt = DP_COLOR_RGB565;    break;
    case eImgFmt_RGB888:  dpFmt = DP_COLOR_RGB888;    break;
    case eImgFmt_ARGB888: dpFmt = DP_COLOR_ARGB8888;  break;
    default:
        MY_LOGE("fmt(0x%x) not support in DP", fmt);
        ret = MFALSE;
        break;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

android_pixel_format_t toPixelFormat(NSCam::EImageFormat fmt)
{
    TRACE_FUNC_ENTER();
    android_pixel_format_t pixelFmt = HAL_PIXEL_FORMAT_YV12;
    switch( fmt )
    {
    case eImgFmt_YUY2:    pixelFmt = HAL_PIXEL_FORMAT_YCbCr_422_I;      break;
    case eImgFmt_NV16:    pixelFmt = HAL_PIXEL_FORMAT_YCbCr_422_SP;     break;
    case eImgFmt_NV21:    pixelFmt = HAL_PIXEL_FORMAT_YCrCb_420_SP;     break;
    case eImgFmt_YV12:    pixelFmt = HAL_PIXEL_FORMAT_YV12;             break;
    case eImgFmt_RGB565:  pixelFmt = HAL_PIXEL_FORMAT_RGB_565;          break;
    case eImgFmt_RGB888:  pixelFmt = HAL_PIXEL_FORMAT_RGB_888;          break;
    case eImgFmt_RGBA8888:pixelFmt = HAL_PIXEL_FORMAT_RGBA_8888;        break;
    default:
        MY_LOGE("unknown fmt(0x%x), use eImgFmt_YV12", fmt);
        break;
    }
    TRACE_FUNC_EXIT();
    return pixelFmt;
}

const char* toPortName(unsigned index)
{
    switch( index )
    {
    case EPortIndex_BPCI:           return "bpci";
    case EPortIndex_DEPI:           return "depi";
    case EPortIndex_DMGI:           return "dmgi";
    case EPortIndex_LCEI:           return "lcei";
    case EPortIndex_LSCI:           return "lsci";
    case EPortIndex_IMGI:           return "imgi";
    case EPortIndex_IMGBI:          return "imgbi";
    case EPortIndex_IMGCI:          return "imgci";
    case EPortIndex_VIPI:           return "vipi";
    case VirDIPPortIdx_YNR_FACEI:   return "y_facei";
    case VirDIPPortIdx_YNR_LCEI:    return "y_lcei";
    case EPortIndex_IMG2O:          return "img2o";
    case EPortIndex_IMG3O:          return "img3o";
    case EPortIndex_WDMAO:          return "wdmao";
    case EPortIndex_WROTO:          return "wroto";
    case EPortIndex_DCESO:          return "dceso";
    default:                        return "unknown";
    }
}

MBOOL copyImageBuffer(IImageBuffer *src, IImageBuffer *dst)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( !src || !dst )
    {
        MY_LOGE("Invalid buffers src=%p dst=%p", src, dst);
        ret = MFALSE;
    }
    else if( src->getImgSize() != dst->getImgSize() )
    {
        MY_LOGE("Mismatch buffer size src(%dx%d) dst(%dx%d)",
                src->getImgSize().w, src->getImgSize().h,
                dst->getImgSize().w, dst->getImgSize().h);
        ret = MFALSE;
    }
    else
    {
        unsigned srcPlane = src->getPlaneCount();
        unsigned dstPlane = dst->getPlaneCount();

        if( !srcPlane || !dstPlane ||
            (srcPlane != dstPlane && srcPlane != 1 && dstPlane != 1) )
        {
            MY_LOGE("Mismatch buffer plane src(%d) dst(%d)", srcPlane, dstPlane);
            ret = MFALSE;
        }
        for( unsigned i = 0; i < srcPlane; ++i )
        {
            if( !src->getBufVA(i) )
            {
                MY_LOGE("Invalid src plane[%d] VA", i);
                ret = MFALSE;
            }
        }
        for( unsigned i = 0; i < dstPlane; ++i )
        {
            if( !dst->getBufVA(i) )
            {
                MY_LOGE("Invalid dst plane[%d] VA", i);
                ret = MFALSE;
            }
        }

        if( srcPlane == 1 )
        {
            MY_LOGD("src: plane=1 size=%zu stride=%zu",
                    src->getBufSizeInBytes(0), src->getBufStridesInBytes(0));
            ret = MFALSE;
        }
        if( dstPlane == 1 )
        {
            MY_LOGD("dst: plane=1 size=%zu stride=%zu",
                    dst->getBufSizeInBytes(0), dst->getBufStridesInBytes(0));
            ret = MFALSE;
        }

        if( ret )
        {
            char *srcVA = NULL, *dstVA = NULL;
            size_t srcSize = 0;
            size_t dstSize = 0;
            size_t srcStride = 0;
            size_t dstStride = 0;

            for( unsigned i = 0; i < srcPlane && i < dstPlane; ++i )
            {
                if( i < srcPlane )
                {
                    srcVA = (char*)src->getBufVA(i);
                }
                if( i < dstPlane )
                {
                    dstVA = (char*)dst->getBufVA(i);
                }

                srcSize = src->getBufSizeInBytes(i);
                dstSize = dst->getBufSizeInBytes(i);
                srcStride = src->getBufStridesInBytes(i);
                dstStride = dst->getBufStridesInBytes(i);
                MY_LOGD("plane[%d] memcpy %p(%zu)=>%p(%zu)",
                          i, srcVA, srcSize, dstVA, dstSize);
                if( srcStride == dstStride )
                {
                    memcpy((void*)dstVA, (void*)srcVA, (srcSize <= dstSize) ? srcSize : dstSize );
                }
                else
                {
                    MY_LOGD("Stride: src(%zu) dst(%zu)", srcStride, dstStride);
                    size_t stride = (srcStride < dstStride) ? srcStride : dstStride;
                    unsigned height = dstSize / dstStride;
                    for( unsigned j = 0; j < height; ++j )
                    {
                        memcpy((void*)dstVA, (void*)srcVA, stride);
                        srcVA += srcStride;
                        dstVA += dstStride;
                    }
                }
            }
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

NB_SPTR getGraphicBufferAddr(IImageBuffer *imageBuffer)
{
    TRACE_FUNC_ENTER();
    void *addr = NULL;
    if( !imageBuffer )
    {
        MY_LOGE("Invalid imageBuffer");
    }
    else if( !imageBuffer->getImageBufferHeap() )
    {
        MY_LOGW("Cannot get imageBufferHeap");
    }
    else
    {
        addr = imageBuffer->getImageBufferHeap()->getHWBuffer();
        // MY_LOGV_IF(!addr, "Cannot get graphic buffer addr");
    }
    TRACE_FUNC_EXIT();
    return (NB_SPTR)addr;
}

MBOOL isTypeMatch(PathType pathT, IO_TYPE ioType)
{
    if(ioType == IO_TYPE_FD) // FD may exist in any Path
        return MTRUE;
    if(pathT == PATH_GENERAL)
    {
        if(ioType == IO_TYPE_DISPLAY
            || ioType == IO_TYPE_RECORD
            || ioType == IO_TYPE_EXTRA)
        {
            return MTRUE;
        }
    }
    if(pathT == PATH_PHYSICAL)
    {
        if(ioType == IO_TYPE_PHYSICAL)
        {
            return MTRUE;
        }
    }
    return MFALSE;
}

MBOOL existOutBuffer(const SFPIOMap &sfpIO, IO_TYPE target)
{
    for(auto&& out : sfpIO.mOutList)
    {
        if(isTargetOutput(target, out))
            return MTRUE;
    }
    return MFALSE;
}

MBOOL existOutBuffer(const std::vector<SFPIOMap> &sfpIOList, IO_TYPE target)
{
    MBOOL exist = MFALSE;
    for(auto&& sfpIO : sfpIOList)
    {
        if(isTypeMatch(sfpIO.mPathType, target))
        {
            exist |= existOutBuffer(sfpIO, target);
        }
    }
    return exist;
}

MBOOL getOutBuffer(const Feature::P2Util::DIPParams &dipParam, IO_TYPE target, NSCam::NSIoPipe::Output &output)
{
    TRACE_FUNC_ENTER();
    unsigned count = 0;
    if( dipParam.mvDIPFrameParams.size() )
    {
        for( unsigned i = 0, size = dipParam.mvDIPFrameParams[0].mvOut.size(); i < size; ++i )
        {
            if( isTargetOutput(target, dipParam.mvDIPFrameParams[0].mvOut[i]) )
            {
                if( ++count == 1 )
                {
                    output = dipParam.mvDIPFrameParams[0].mvOut[i];
                }
            }
        }
    }
    if( count > 1 )
    {
        TRACE_FUNC("suspicious output number = %d, type = %d", count, target);
    }
    TRACE_FUNC_EXIT();
    return count >= 1;
}

MBOOL getOutBuffer(const SFPIOMap &ioMap, IO_TYPE target, P2IO &output)
{
    TRACE_FUNC_ENTER();
    if(!ioMap.isValid())
        return MFALSE;
    unsigned count = 0;
    for(const SFPOutput &out : ioMap.mOutList)
    {
        if(isTargetOutput(target, out))
        {
            if( ++count == 1 )
            {
                output = out.toP2IO();
            }
        }
    }
    if( count > 1 )
    {
        MY_LOGW("suspicious output number = %d, type = %d", count, target);
    }
    TRACE_FUNC_EXIT();
    return count >= 1;
}

MBOOL getOutBuffer(const SFPIOMap &ioMap, IO_TYPE target, std::vector<P2IO> &outList)
{
    TRACE_FUNC_ENTER();
    if(!ioMap.isValid())
        return MFALSE;
    unsigned count = 0;
    for(const SFPOutput &out : ioMap.mOutList)
    {
        if(isTargetOutput(target, out))
        {
            outList.push_back(out.toP2IO());
            count++;
        }
    }
    TRACE_FUNC_EXIT();
    return count >= 1;
}

MUINT32 getOutBufferCount(const SFPIOMap &ioMap, IO_TYPE target)
{
    TRACE_FUNC_ENTER();
    MUINT32 count = 0;
    if( ioMap.isValid() )
    {
        for( const SFPOutput &out : ioMap.mOutList )
        {
            if( isTargetOutput(target, out) )
            {
                ++count;
            }
        }
    }
    TRACE_FUNC_EXIT();
    return count;
}

MBOOL getMultiOutBuffer(const SFPIOMap &ioMap, IO_TYPE target, std::vector<P2IO> &multiOut)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( ioMap.isValid() )
    {
        for( const SFPOutput &out : ioMap.mOutList )
        {
            if( isTargetOutput(target, out) )
            {
                multiOut = out.toP2IOVector();
                ret = MTRUE;
                break;
            }
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MSize calcDsImgSize(const MSize &src)
{
    TRACE_FUNC_ENTER();
    MSize result;

    if( src.w*3 == src.h*4 )
    {
        result = MSize(320, 24);
    }
    else if( src.w*9 == src.h*16 )
    {
        result = MSize(320, 180);
    }
    else if( src.w*3 == src.h*5 )
    {
        result = MSize(320, 180);
    }
    else
    {
        result = MSize(320, 320*src.h/src.w);
    }

    TRACE_FUNC_EXIT();
    return result;
}

MUINT32 allocate(const char *prefix, const sp<IBufferPool> &pool, MUINT32 count)
{
    EImageFormat fmt = eImgFmt_UNKNOWN;;
    MSize size;
    Timer timer(true);
    if( pool != NULL )
    {
        fmt = pool->getImageFormat();
        size = pool->getImageSize();
        pool->allocate(count);
    }
    timer.stop();
    MY_LOGI("%s %s %d buf (0x%x/%s)(%dx%d) in %d ms", prefix, STR_ALLOCATE, count, fmt, toName(fmt), size.w, size.h, timer.getElapsed());
    return count;
}

MBOOL dumpToFile(IImageBuffer *buffer, const char *fmt, ...)
{
    MBOOL ret = MFALSE;
    if( buffer && fmt )
    {
        char filename[256];
        va_list arg;
        va_start(arg, fmt);
        vsprintf(filename, fmt, arg);
        va_end(arg);
        buffer->saveToFile(filename);
        ret = MTRUE;
    }
    return ret;
}

MBOOL is4K2K(const MSize &size)
{
    return (size.w >= UHD_VR_WIDTH && size.h >= UHD_VR_HEIGHT);
}

MBOOL isValid(const MSize &size)
{
    return size.w > 0 && size.h > 0;
}

MBOOL isValid(const MSizeF &size)
{
    return size.w > 0.0f && size.h > 0.0f;
}

MBOOL isValidCrop(const MSizeF &inputSize, const MRectF &crop)
{
    return isValid(crop.s) && crop.p.x >= 0.0f && crop.p.x <= inputSize.w && crop.p.y >= 0.0f && crop.p.y <= inputSize.h;
}

MRectF toMRectF(const MRect &rect)
{
    return MRectF(rect.p, rect.s);
}

MVOID accumulateUnion(MRectF &dst, const MRectF &src)
{
    MPointF m;
    m.x = std::max(dst.p.x + dst.s.w, src.p.x + src.s.w);
    m.y = std::max(dst.p.y + dst.s.h, src.p.y + src.s.h);
    dst.p.x = std::min(dst.p.x, src.p.x);
    dst.p.y = std::min(dst.p.y, src.p.y);
    dst.s.w = m.x - dst.p.x;
    dst.s.h = m.y - dst.p.y;
}


MUINT32 align(MUINT32 val, MUINT32 bits)
{
    // example: align 5 bits => align 32
    MUINT32 mask = (0x01 << bits) - 1;
    return (val + mask) & (~mask);
}

MSize align(const MSize &size, MUINT32 bits)
{
    // example: align 5 bits => align 32
    MUINT32 mask = (0x01 << bits) - 1;
    return MSize((size.w + mask) & (~mask),
                 (size.h + mask) & (~mask));
}

MVOID moveAppend(std::vector<P2IO> &source, std::vector<P2IO> &dest)
{
    if(dest.empty())
    {
        dest = std::move(source);
    }
    else
    {
        dest.reserve(dest.size() + source.size());
        std::move(std::begin(source), std::end(source), std::back_inserter(dest));
        source.clear();
    }
}

MBOOL syncCache(const BasicImg &img, NSCam::eCacheCtrl ctrl)
{
    return img.mBuffer != NULL ? img.mBuffer->syncCache(ctrl) : MFALSE;
}

MBOOL syncCache(const ImgBuffer &img, NSCam::eCacheCtrl ctrl)
{
    return img != NULL ? img->syncCache(ctrl) : MFALSE;
}

MBOOL refineBoundaryF(const char *name, const MSizeF &size, MRectF &crop, MBOOL needPrintIO)
{
    MBOOL isRefined = MFALSE;

    MRectF refined = crop;
    if( crop.p.x < 0 )
    {
        refined.p.x = 0;
        isRefined = MTRUE;
    }
    if( crop.p.y < 0 )
    {
        refined.p.y = 0;
        isRefined = MTRUE;
    }
    if( (refined.p.x + crop.s.w) > size.w )
    {
        refined.s.w = size.w - refined.p.x;
        isRefined = MTRUE;
    }
    if( (refined.p.y + crop.s.h) > size.h )
    {
        refined.s.h = size.h - refined.p.y;
        isRefined = MTRUE;
    }
    if( isRefined )
    {
        MY_LOGW_IF(needPrintIO, "name:%s, size:(%.0fx%.0f), crop:(%f,%f,%f,%f) -> crop:(%f,%f,%f,%f)",
            name, size.w, size.h,
            crop.p.x, crop.p.y, crop.s.w, crop.s.h,
            refined.p.x, refined.p.y, refined.s.w, refined.s.h);
        crop = refined;
    }

    return isRefined;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
