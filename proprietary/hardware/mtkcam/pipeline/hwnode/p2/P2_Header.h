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

#ifndef _MTKCAM_HWNODE_P2_HEADER_H_
#define _MTKCAM_HWNODE_P2_HEADER_H_

#include <list>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <cutils/properties.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>

#include <mtkcam/def/common.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/custom/ExifFactory.h>
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/DebugScanLine.h>
#include <mtkcam/utils/std/Trace.h>

#include <camera_custom_eis.h>

#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>

#include <DpDataType.h>

#pragma push_macro("LOG_TAG")
#undef LOG_TAG
#define LOG_TAG "MtkCam/P2"
#include "../hwnode_utilities.h"
#pragma pop_macro("LOG_TAG")

#include "P2_DebugControl.h"

using NSCam::NSIoPipe::PortID;

#if NEW_CODE
typedef NSCam::NSIoPipe::FrameParams QFRAME_T;
using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::FrameParams;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::MCropRect;
using NSCam::NSIoPipe::MCrpRsInfo;
using NSCam::NSIoPipe::ExtraParam;
#else
typedef NSCam::NSIoPipe::NSPostProc::QParams QFRAME_T;
using NSCam::NSIoPipe::NSPostProc::QParams;
using NSCam::NSIoPipe::NSPostProc::Input;
using NSCam::NSIoPipe::NSPostProc::Output;
using NSCam::NSIoPipe::NSPostProc::MCropRect;
using NSCam::NSIoPipe::NSPostProc::MCrpRsInfo;
#endif // NEW_CODE
using NSCam::NSIoPipe::NSPostProc::INormalStream;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Prv;
using NSCam::NSIoPipe::NSPostProc::ESDCmd_CONFIG_VENC_DIRLK;
using NSCam::NSIoPipe::NSPostProc::ESDCmd_RELEASE_VENC_DIRLK;

#ifndef NEW_NODE
#define EIS_REGION_INDEX_MAX_GMV (EIS_REGION_INDEX_LWTS+1)
#endif // NEW_NODE

using android::sp;
using android::Vector;

using NSCam::DebugExifUtils;
using NSCam::eTransform_ROT_90;
using NSCam::IImageBuffer;
using NSCam::IImageBufferHeap;
using NSCam::IMetadata;
using NSCam::IMetadataProvider;
using NSCam::IImageBuffer;
using NSCam::IImageBufferHeap;
using NSCam::IMetadata;
using NSCam::MPoint;
using NSCam::MRect;
using NSCam::MPointF;
using NSCam::MRectF;
using NSCam::MSize;
using NSCam::MSizeF;
using NSCam::SENSOR_VHDR_MODE_NONE;
using NSCam::Type2Type;

using NSCam::Utils::Sync::IFence;

using NSCam::v3::StreamId_T;
using NSCam::v3::IStreamInfo;
using NSCam::v3::IMetaStreamInfo;
using NSCam::v3::IImageStreamInfo;
using NSCam::v3::IPipelineFrame;
using NSCam::v3::IPipelineNode;
using NSCam::v3::IPipelineNodeCallback;
using NSCam::v3::IStreamBuffer;
using NSCam::v3::IStreamBufferSet;
using NSCam::v3::IMetaStreamBuffer;
using NSCam::v3::IImageStreamBuffer;
using NSCam::v3::vector_f;
using NSCam::v3::simpleTransform;
using NSCam::v3::IUsersManager;
using NSCam::v3::STREAM_BUFFER_STATUS;
using NSCam::v3::div_round;
using NSCam::v3::IStreamInfoSet;

using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_IMGCI;
using NSCam::NSIoPipe::PORT_IMGBI;
using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_LCEI;
using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WROTO;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::EPortCapbility;
using NSCam::NSIoPipe::EPortCapbility_None;
using NSCam::NSIoPipe::EPortCapbility_Disp;
using NSCam::NSIoPipe::EPortCapbility_Rcrd;
using NSCam::NSIoPipe::NSSImager::IImageTransform;

using NSIspTuning::RAWIspCamInfo;

using NSCamHW::HwMatrix;
using NSCamHW::HwTransHelper;

using NS3Av3::MetaSet_T;
using NS3Av3::TuningParam;


#define CROP_IMGO  1
#define CROP_IMG2O 1
#define CROP_IMG3O 1
#define CROP_WDMAO 2
#define CROP_WROTO 3

#endif // _MTKCAM_HWNODE_P2_HEADER_H_
