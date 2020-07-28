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
//#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
//#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
//#include <mtkcam/iopipe/PostProc/INormalStream.h>

#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/FatImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>

#include "StreamingFeatureData.h"
#include "DebugControl.h"

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

#define EIS_WDMAO_CROP_GROUP 12
#define EIS_WROTO_CROP_GROUP 13

#define PORTID_IN 0
#define PORTID_OUT 1

#define MPoint_STR "%d,%d"
#define MPoint_ARG(p) p.x, p.y
#define MSize_STR "%dx%d"
#define MSize_ARG(s) s.w, s.h
#define MCrpRsInfo_STR "groupID=%d frameGroup=%d i(%dx%d) f(%dx%d) s(%dx%d) r(%dx%d)"
#define MCrpRsInfo_ARG(c) c.mGroupID, c.mFrameGroup, MPoint_ARG(c.mCropRect.p_integral), MPoint_ARG(c.mCropRect.p_fractional), MSize_ARG(c.mCropRect.s), MSize_ARG(c.mResizeDst)
#define ModuleInfo_STR "in(%dx%d) crop(%lux%lu) crop_start=(%d.%u, %d.%u) out(%dx%d)"
#define ModuleInfo_ARG(md) md->in_w, md->in_h, md->crop_w,  md->crop_h, md->crop_x, md->crop_floatX, md->crop_y, md->crop_floatY, md->out_w, md->out_h
#define ExtraParam_FE_STR "DSCR_SBIT=%d TH_C=%d TH_G=%d FLT_EN=%d PARAM=%d MODE=%d YIDX=%d XIDX=%d START_X=%d START_Y=%d IN_HT=%d IN_WD=%d"
#define ExtraParam_FE_ARG(ext) ext->mFEDSCR_SBIT, ext->mFETH_C, ext->mFETH_G, ext->mFEFLT_EN, ext->mFEPARAM, ext->mFEMODE, ext->mFEYIDX, ext->mFEXIDX, ext->mFESTART_X, ext->mFESTART_Y, ext->mFEIN_HT, ext->mFEIN_WD
#define ExtraParam_FM_STR "HEIGHT=%d WIDTH=%d SR_TYPE=%d OFFSET_X=%d OFFSET_Y=%d RES_TH=%d SAD_TH=%d MIN_RATIO=%d"
#define ExtraParam_FM_ARG(ext) ext->mFMHEIGHT, ext->mFMWIDTH, ext->mFMSR_TYPE, ext->mFMOFFSET_X, ext->mFMOFFSET_Y, ext->mFMRES_TH, ext->mFMSAD_TH, ext->mFMMIN_RATIO
#define DpPqParam_ClearZoomParam_STR "cz.captureShot=%d cz.p_customSetting=%p "
#define DpPqParam_ClearZoomParam_ARG(cz) cz.captureShot, cz.p_customSetting
#define DpPqParam_DREParam_STR "dre.cmd=%d dre.userId=%llu dre.buffer=%p dre.p_customSetting=%p dre.customIndex=%d "
#define DpPqParam_DREParam_ARG(dre) dre.cmd, dre.userId, dre.buffer, dre.p_customSetting, dre.customIndex
#define DpPqParam_ISPParam_STR "isp.iso=%d isp.LV=%d isp.p_faceInfor=%p "
#define DpPqParam_ISPParam_ARG(isp) isp.iso, isp.LV, isp.p_faceInfor
#define DpPqParam_STR "scenario=%d enable=%d " DpPqParam_ISPParam_STR DpPqParam_ClearZoomParam_STR DpPqParam_DREParam_STR
#define DpPqParam_ARG(pq) pq->scenario, pq->enable, DpPqParam_ISPParam_ARG(pq->u.isp), DpPqParam_ClearZoomParam_ARG(pq->u.isp.clearZoomParam), DpPqParam_DREParam_ARG(pq->u.isp.dpDREParam)
#define ExtraParam_PQ_STR "pqParam=%p pqParam.WDMA=%p pqParam.WROT=%p"
#define ExtraParam_PQ_ARG(ext) ext, ext->WDMAPQParam, ext->WROTPQParam
#define ExtraParam_CRSPINFO_STR "OFFSET_X=%d OFFSET_Y=%d WIDTH=%d HEIGHT=%d"
#define ExtraParam_CRSPINFO_ARG(ext) ext->m_CrspInfo.p_integral.x, ext->m_CrspInfo.p_integral.y, ext->m_CrspInfo.s.w, ext->m_CrspInfo.s.h

#define MCropRect_STR "(%dx%d)@(%d,%d)"
#define MCropRect_ARG(r) r.s.w, r.s.h, r.p_integral.x, r.p_integral.y

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

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


enum IO_TYPE
{
    IO_TYPE_DISPLAY,
    IO_TYPE_RECORD,
    IO_TYPE_EXTRA,
    IO_TYPE_FD,
};

MBOOL useMDPHardware();

MBOOL isTargetOutput(MUINT32 target, const NSCam::NSIoPipe::Output &output);
MBOOL isDisplayOutput(const NSCam::NSIoPipe::Output &output);
MBOOL isRecordOutput(const NSCam::NSIoPipe::Output &output);
MBOOL isExtraOutput(const NSCam::NSIoPipe::Output &output);
MBOOL isFDOutput(const NSCam::NSIoPipe::Output &output);

IImageBuffer* findInBuffer(const NSCam::NSIoPipe::QParams &qparam, MUINT32 port = NSImageio::NSIspio::EPortIndex_IMGI);
IImageBuffer* findInBuffer(const NSCam::NSIoPipe::FrameParams &param, MUINT32 port = NSImageio::NSIspio::EPortIndex_IMGI);
IImageBuffer* findOutBuffer(const NSCam::NSIoPipe::QParams &qparam, unsigned skip);
IImageBuffer* findOutBuffer(const NSCam::NSIoPipe::FrameParams &param, unsigned skip);

MBOOL toDpColorFormat(const NSCam::EImageFormat fmt, DpColorFormat &dpFmt);
MBOOL toPixelFormat(const NSCam::EImageFormat fmt, android_pixel_format_t &pixelFmt);

MBOOL copyImageBuffer(IImageBuffer *src, IImageBuffer *dst);

NB_SPTR getGraphicBufferAddr(IImageBuffer *imageBuffer);

MBOOL getOutBuffer(const NSCam::NSIoPipe::QParams &qparam, IO_TYPE target, NSCam::NSIoPipe::Output &output);

MSize calcDsImgSize(const MSize &src);

MBOOL dumpToFile(IImageBuffer *buffer, const char *fmt, ...);

MBOOL is4K2K(const MSize &size);

MUINT32 align(MUINT32 val, MUINT32 bits);

MCropRect getCropRect(const MRectF &rectF);


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_COMMON_H_
