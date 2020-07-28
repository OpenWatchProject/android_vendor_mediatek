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
namespace Cap_3rdParty
{
static const struct MetaStreamManager::metadata_info_setting gStereoMetaTbl_P2Cap[] =
{
    // Hal
    {"App:Meta:Control",            eSTREAMID_META_APP_CONTROL,             eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_APP},
    {"Hal:Meta:Control",            eSTREAMID_META_HAL_CONTROL,             eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_HAL},
    // StereoRootNode
    {"Hal:Meta:DynamicRoot",        eSTREAMID_META_HAL_DYNAMIC_STEREOROOT,  eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    // DualYUVNode - input
    {"Hal:Meta:P1:Dynamic",         eSTREAMID_META_HAL_DYNAMIC_P1,          eSTREAMTYPE_META_INOUT, 9, 9, eStreamType_META_HAL},
    // DualYUVNode - ouput
    {"Hal:Meta:DynamicDepth",       eSTREAMID_META_HAL_DYNAMIC_DUALYUV,     eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    {"APP:Meta:DynamicDepth",       eSTREAMID_META_APP_DYNAMIC_DUALYUV,     eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_APP},
    // End
    {"", 0, 0, 0, 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_P2Cap[] =
{
    // Hal
    // DualYUVNode - input
    {"Hal:Image:Pass2_FullRaw",            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(4176, 3088), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw",          eSTREAMID_IMAGE_PIPE_RAW_RESIZER,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(2304, 1296), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw_Main2",    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(1600, 1200), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_FullRaw_Main2",      eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,            eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_BAYER10, MSize(0, 0),       0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // DualYUVNode - output
    {"Hal:Image:Main1YUV",                 eSTREAMID_IMAGE_PIPE_DUALYUVNODE_MAIN1YUV,     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_YV12, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:Main2YUV",                 eSTREAMID_IMAGE_PIPE_DUALYUVNODE_MAIN2YUV,     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_YV12, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static const MINT32 gStereoP2ConnectSetting_P2Cap[] =
{
    eNODEID_StereoRootNode,  eNODEID_DualYuvNode,
    -1,
    -1,
};
};
};
};