/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _HDRDEFS_COMMON_H_
#define _HDRDEFS_COMMON_H_

#include <utils/def/BuiltinTypes.h>
#include <utils/def/UITypes.h>

//using namespace com::mediatek::campostalgo::NSFeaturePipe;

namespace NSCam {
namespace HDRCommon {

// ---------------------------------------------------------------------------
// Module versioning control

// The most significant bits (bit 24 ~ 31)
// store the information of major version number.
//
// The least significant bits (bit 16 ~ 23)
// store the information of minor version number.
//
// bit 0 ~ 15 are reserved for future use
#define CREATE_API_VERSION(major,minor) \
    ((((major) & 0xff) << 24) | (((minor) & 0xff) << 16))

#define GET_MAJOR_VERSION(version) \
    (((version) >> 24) & 0xff)

#define GET_MINOR_VERSION(version) \
    (((version) >> 16) & 0xff)

// All supported module versions
#define HDR_MODULE_API_VERSION_1_0 CREATE_API_VERSION(1, 0)
#define HDR_MODULE_API_VERSION_2_0 CREATE_API_VERSION(2, 0)
#define HDR_MODULE_API_VERSION_2_3 CREATE_API_VERSION(2, 3)
#define HDR_MODULE_API_VERSION_3_0 CREATE_API_VERSION(3, 0)

// ---------------------------------------------------------------------------
// HDR common definitions

#define L1_CACHE_BYTES 32

// The maximum of HDR input frame
#define HDR_MAX_INPUT_FRAME  3

// A handle that represents a HDR instance
typedef void* hdr_object_t;

struct HDRHandle
{
    // HDR instance
    hdr_object_t id;

    // The amount of HDR iput frames
    MINT32 inputFrameCount;

    // Used to check if exposure has been configured
    MBOOL isExposureConfigured;

#ifdef __cplusplus
    HDRHandle()
        : id(nullptr),
          inputFrameCount(0),
          isExposureConfigured(MFALSE)
    {}
#endif
};

struct HDR_AWB_Gain
{
    int gGain;
    int rGain;
    int bGain;
};

enum {
    YuvDomainHDR,
    RawDomainHDR,
};

enum {
    HDR_Dark,
    HDR_Normal,
    HDR_Bright,
};

enum {
    HDRProcParam_Begin = 0,

    HDRProcParam_Get_src_main_format,
    HDRProcParam_Get_src_main_size,
    HDRProcParam_Get_src_small_format,
    HDRProcParam_Get_src_small_size,

    HDRProcParam_Set_sensor_size,
    HDRProcParam_Set_sensor_type,

    HDRProcParam_Set_AOEMode,
    HDRProcParam_Set_MaxSensorAnalogGain,
    HDRProcParam_Set_MaxAEExpTimeInUS,
    HDRProcParam_Set_MinAEExpTimeInUS,
    HDRProcParam_Set_ShutterLineTime,
    HDRProcParam_Set_MaxAESensorGain,
    HDRProcParam_Set_MinAESensorGain,
    HDRProcParam_Set_ExpTimeInUS0EV,
    HDRProcParam_Set_SensorGain0EV,
    HDRProcParam_Set_FlareOffset0EV,
    HDRProcParam_Set_GainBase0EV,
    HDRProcParam_Set_LE_LowAvg,
    HDRProcParam_Set_SEDeltaEVx100,
    HDRProcParam_Set_Histogram,

    HDRProcParam_Set_DetectFace,
    HDRProcParam_Set_FlareHistogram,
    HDRProcParam_Set_PLineAETable,

    // for debug purpose
    // can be either
    // 1. a monotonic time since unspecified starting point
    // 2. or a unique key given from HDR's user
    HDRProcParam_Set_SequenceNumber,
    HDRProcParam_Set_AWB_Gain,
    HDRProcParam_Set_Debug_Name_Info,

    HDRProcParam_Num
};

struct HDRProc_ShotParam
{
    // The dimensions for captured pictures in pixels (width x height)
    MSize  pictureSize;

    // The dimensions for postview in pixels (width x height)
    MSize  postviewSize;

    // This control can be used to implement digital zoom
    MRect  scalerCropRegion;
};

enum HDROutputType {
    HDR_OUTPUT_JPEG_YUV,
    HDR_OUTPUT_JPEG_THUMBNAIL_YUV,
    HDR_OUTPUT_NUM
};

enum HDRBufferType {
    HDR_BUFFER_SOURCE,
    HDR_BUFFER_SMALL,
    HDR_BUFFER_SE,
    HDR_BUFFER_WORKING,
    HDR_BUFFER_ORI_WEIGHT_MAP,
    HDR_BUFFER_BLURRED_WEIGHT_MAP,
    HDR_BUFFER_DOWNSIZED_WEIGHT_MAP,
    HDR_BUFFER_BLENDING
};

} // namespace HDRCommon
} // namespace NSCam

#endif // _HDRDEFS_COMMON_H_
