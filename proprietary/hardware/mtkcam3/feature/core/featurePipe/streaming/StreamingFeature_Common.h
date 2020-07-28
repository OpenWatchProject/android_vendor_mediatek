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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_COMMON_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_COMMON_H_

#include "MtkHeader.h"
//#include <mtkcam3/feature/featurePipe/IStreamingFeaturePipe.h>
//#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
//#include <mtkcam/iopipe/PostProc/INormalStream.h>

#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/FatImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <featurePipe/core/include/SharedBufferPool.h>
#include <featurePipe/core/include/SecureBufferPool.h>

#include "StreamingFeatureData.h"
#include "DebugControl.h"

#include <mtkcam3/feature/utils/p2/DIPStream.h>

#define SUPPORT_HAL3

#ifdef SUPPORT_HAL3
#define BOOL_SUPPORT_HAL3 (true)
#else
#define BOOL_SUPPORT_HAL3 (false)
#endif

#define UHD_VR_WIDTH      3840
#define UHD_VR_HEIGHT     2160
#define MAX_FULL_WIDTH    4608 //(3840x1.2)
#define MAX_FULL_HEIGHT   2612 //(2176x1.2)
#define DS_IMAGE_WIDTH    320
#define DS_IMAGE_HEIGHT   320
#define MAX_WARP_WIDTH    320
#define MAX_WARP_HEIGHT   320

#define MAX_FULL_SIZE (MSize(MAX_FULL_WIDTH, MAX_FULL_HEIGHT))
#define DS_IMAGE_SIZE (MSize(DS_IMAGE_WIDTH, DS_IMAGE_HEIGHT))
#define MAX_WARP_SIZE (MSize(MAX_WARP_WIDTH, MAX_WARP_HEIGHT))

#define IMG2O_CROP_GROUP 1
#define WDMAO_CROP_GROUP 2
#define WROTO_CROP_GROUP 3

#define PORTID_IN 0
#define PORTID_OUT 1

#define MPoint_STR "%d,%d"
#define MPoint_ARG(p) p.x, p.y
#define MSize_STR "%dx%d"
#define MSize_ARG(s) s.w, s.h
#define MRectF_STR "(%fx%f)@(%f,%f)"
#define MRectF_ARG(r) r.s.w, r.s.h, r.p.x, r.p.y
#define MRect_STR "(%dx%d)@(%d,%d)"
#define MRect_ARG(r) r.s.w, r.s.h, r.p.x, r.p.y

#define MCropRect_STR "(%dx%d)@(%d,%d)"
#define MCropRect_ARG(r) r.s.w, r.s.h, r.p_integral.x, r.p_integral.y
#define MCropF_STR "(%fx%f)@(%f,%f)"
#define MCropF_ARG(r) r.s.w, r.s.h, r.p.x, r.p.y
#define MTransF_STR "Offset(%f,%f) Scale(%f,%f)"
#define MTransF_ARG(o,s) o.x, o.y, s.w, s.h
#define MSizeF_STR "(%fx%f)"
#define MSizeF_ARG(s) s.w, s.h
#define MPointF_STR "(%f,%f)"
#define MPointF_ARG(p) p.x, p.y

#define SFP_ABS(x,y) ((x)>(y)?(x)-(y):(y)-(x))

using NSCam::NSIoPipe::EPortType_Memory;
using NSCam::NSIoPipe::PortID;
//using NSCam::NSIoPipe::NSPostProc::INormalStream;
using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::MCrpRsInfo;
using NSCam::NSIoPipe::MCropRect;
//using namespace NSCamHW;
//using namespace NSCam::NSIoPipe;
//using namespace NSCam::NSIoPipe::NSCamIOPipe;
//using namespace NSCam::NSIoPipe::NSPostProc;
using NSCam::Feature::P2Util::DMAConstrain;
using NSCam::Feature::P2Util::P2IO;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class FrameInInfo
{
public:
    MSize inSize;
    MINT64 timestamp = 0;
};

const char* Fmt2Name(MINT fmt);
MBOOL useMDPHardware();
MBOOL getFrameInInfo(FrameInInfo &info, const Feature::P2Util::DIPFrameParams &frame, const PortID &portID = NSIoPipe::PORT_IMGI);

MBOOL isTargetOutput(MUINT32 target, const NSCam::NSIoPipe::Output &output);
MBOOL isDisplayOutput(const NSCam::NSIoPipe::Output &output);
MBOOL isRecordOutput(const NSCam::NSIoPipe::Output &output);
MBOOL isExtraOutput(const NSCam::NSIoPipe::Output &output);
MBOOL isFDOutput(const NSCam::NSIoPipe::Output &output);

MBOOL findUnusedMDPPort(const Feature::P2Util::DIPParams &dipParam, unsigned &index);
MBOOL findUnusedMDPCropGroup(const Feature::P2Util::DIPParams &dipParam, unsigned &cropGroup);

MSize toValid(const MSize &size, const MSize &def);
NSCam::EImageFormat toValid(const NSCam::EImageFormat fmt, const NSCam::EImageFormat def);
const char* toName(NSCam::EImageFormat fmt);
MBOOL toDpColorFormat(const NSCam::EImageFormat fmt, DpColorFormat &dpFmt);
android_pixel_format_t toPixelFormat(NSCam::EImageFormat fmt);

const char* toPortName(unsigned index);

MBOOL copyImageBuffer(IImageBuffer *src, IImageBuffer *dst);

NB_SPTR getGraphicBufferAddr(IImageBuffer *imageBuffer);

MBOOL getOutBuffer(const Feature::P2Util::DIPParams &dipParam, IO_TYPE target, NSCam::NSIoPipe::Output &output);

// Buffer Description by SFPIOMap
MBOOL getOutBuffer(const SFPIOMap &ioMap, IO_TYPE target, P2IO &output);
MBOOL getOutBuffer(const SFPIOMap &ioMap, IO_TYPE target, std::vector<P2IO> &outList);
MUINT32 getOutBufferCount(const SFPIOMap &ioMap, IO_TYPE target);
MBOOL getMultiOutBuffer(const SFPIOMap &ioMap, IO_TYPE target, std::vector<P2IO> &multiOut);
MBOOL existOutBuffer(const std::vector<SFPIOMap> &sfpIOList, IO_TYPE target);
MBOOL existOutBuffer(const SFPIOMap &sfpIO, IO_TYPE target);
MBOOL isTypeMatch(PathType pathT, IO_TYPE ioType);
MBOOL isDisplayOutput(const SFPOutput &output);
MBOOL isRecordOutput(const SFPOutput &output);
MBOOL isExtraOutput(const SFPOutput &output);
MBOOL isFDOutput(const SFPOutput &output);
MBOOL isPhysicalOutput(const SFPOutput &output);

MSize calcDsImgSize(const MSize &src);

MUINT32 allocate(const char *prefix, const sp<IBufferPool> &pool, MUINT32 count);
MBOOL dumpToFile(IImageBuffer *buffer, const char *fmt, ...);

MBOOL is4K2K(const MSize &size);
MBOOL isValid(const MSize &size);
MBOOL isValid(const MSizeF &size);
MBOOL isValidCrop(const MSizeF &inputSize, const MRectF &crop);
MRectF toMRectF(const MRect &rect);

MVOID accumulateUnion(MRectF &dst, const MRectF &src);

MUINT32 align(MUINT32 val, MUINT32 bits);
MSize align(const MSize &size, MUINT32 bits);

MVOID moveAppend(std::vector<P2IO> &source, std::vector<P2IO> &dest);

MBOOL syncCache(const BasicImg &img, NSCam::eCacheCtrl ctrl);
MBOOL syncCache(const ImgBuffer &img, NSCam::eCacheCtrl ctrl);

MBOOL refineBoundaryF(const char *name, const MSizeF &size, MRectF &crop, MBOOL needPrintIO);

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_COMMON_H_
