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
/*******************************************************************************
*
* Capture Scenario
*
*******************************************************************************/
namespace StereoPipelineMgrData
{
namespace p2
{
namespace BM_Cap_Denoise
{
static const struct MetaStreamManager::metadata_info_setting gStereoMetaTbl_BM_Cap[] =
{
    // Hal
    {"App:Meta:Control",            eSTREAMID_META_APP_CONTROL,         eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_APP},
    {"Hal:Meta:Control",            eSTREAMID_META_HAL_CONTROL,         eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_HAL},
    // DepthMapNode - input
    {"Hal:Meta:P1:Dynamic",         eSTREAMID_META_HAL_DYNAMIC_P1,          eSTREAMTYPE_META_INOUT, 9, 9, eStreamType_META_HAL},
    // StereoRootNode
    {"Hal:Meta:DynamicRoot",       eSTREAMID_META_HAL_DYNAMIC_STEREOROOT,    eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    // DualYUVNode
    {"Hal:Meta:DynamicDualYUV",       eSTREAMID_META_HAL_DYNAMIC_DUALYUV,    eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    {"APP:Meta:DynamicDualYUV",       eSTREAMID_META_APP_DYNAMIC_DUALYUV,    eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_APP},
    // DepthMapNode - ouput
    {"Hal:Meta:DynamicDepth",       eSTREAMID_META_HAL_DYNAMIC_DEPTH,    eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    {"APP:Meta:DynamicDepth",       eSTREAMID_META_APP_DYNAMIC_DEPTH,    eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_APP},
    // BMPreProcessNode - output
    {"Hal:Meta:DynamicPreProcess",  eSTREAMID_META_HAL_DYNAMIC_BMPREPROCESS,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_HAL},
    {"APP:Meta:DynamicPreProcess",  eSTREAMID_META_APP_DYNAMIC_BMPREPROCESS,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_HAL},
    // BMDeNoiseNode - ouput
    {"Hal:Meta:DynamicDenoise",       eSTREAMID_META_HAL_DYNAMIC_BMDENOISE,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_HAL},
    {"APP:Meta:DynamicDenoise",       eSTREAMID_META_APP_DYNAMIC_BMDENOISE,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_HAL},
    // JpefEnc - output
    {"App:Meta:JpgEnc_Clean",       eSTREAMID_META_APP_DYNAMIC_JPEG,      eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    // End
    {"", 0, 0, 0, 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_BM_Cap[] =
{
    // Hal
    // DualYUVNode/DepthMapNode - input
    {"Hal:Image:Pass2_FullRaw",            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(4176, 3088), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw",          eSTREAMID_IMAGE_PIPE_RAW_RESIZER,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(2304, 1296), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw_Main2",    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(1600, 1200), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_FullRaw_Main2",      eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,            eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_BAYER10, MSize(0, 0),       0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // DualYUVNode - output
    {"Hal:Image:YUVThumb",                 eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // APP
    {"Hal:Image:ExtraData",                eSTREAMID_IMAGE_PIPE_STEREO_DBG,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,eImgFmt_STA_BYTE, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // DepthMapNode - output
    {"Hal:Image:Disparity_Left",           eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DISPARITY_L, eSTREAMTYPE_IMAGE_INOUT, 1, 1, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_Y16, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:Disparity_Right",          eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DISPARITY_R, eSTREAMTYPE_IMAGE_INOUT, 1, 1, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_Y16, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:WrappingMatrix",           eSTREAMID_IMAGE_PIPE_BMDENOISE_WAPING_MATRIX,    eSTREAMTYPE_IMAGE_INOUT, 1, 1, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_Y8, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:SceneInfo",                eSTREAMID_IMAGE_PIPE_BMDENOISE_SCENE_INFO,       eSTREAMTYPE_IMAGE_INOUT, 1, 1, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_Y8, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    // BMDeNoiseNode - output
    {"Hal:Image:DenoiseResult",            eSTREAMID_IMAGE_PIPE_YUV_JPEG,                 eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:DenoiseThumb",             eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,            eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // JpegEnc
    {"Hal:Image:JpegEnc",                  eSTREAMID_IMAGE_JPEG,                          eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,0, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static const MINT32 gStereoP2ConnectSetting_BM_Cap[] =
{
    eNODEID_StereoRootNode,   eNODEID_DualYuvNode,
    eNODEID_StereoRootNode,   eNODEID_DepthMapNode,
    eNODEID_DepthMapNode,   eNODEID_BMDeNoiseNode,
    eNODEID_BMDeNoiseNode,    eNODEID_JpegNode,

    -1,
    -1,
};
};
};
};