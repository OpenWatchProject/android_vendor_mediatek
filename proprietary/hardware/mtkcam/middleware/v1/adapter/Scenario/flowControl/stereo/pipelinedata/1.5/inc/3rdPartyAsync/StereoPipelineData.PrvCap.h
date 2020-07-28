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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
namespace PrvCap_3rdParty_Async
{
static const struct MetaStreamManager::metadata_info_setting gStereoMetaTbl_P2PrvCap[] =
{
    // Hal
    {"App:Meta:Control",            eSTREAMID_META_APP_CONTROL,             eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_APP},
    {"Hal:Meta:Control",            eSTREAMID_META_HAL_CONTROL,             eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_HAL},
    // StereoRootNode - in/out
    {"Hal:Meta:DynamicRoot",        eSTREAMID_META_HAL_DYNAMIC_P1,          eSTREAMTYPE_META_INOUT,   9, 9, eStreamType_META_HAL},
    {"Hal:Meta:DynamicRoot_01",     eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2,    eSTREAMTYPE_META_INOUT,   9, 9, eStreamType_META_HAL},
    {"App:Meta:RootOut",            eSTREAMID_META_HAL_DYNAMIC_STEREOROOT,  eSTREAMTYPE_META_OUT,    0, 0, eStreamType_META_APP},
    // Dual3rdParty - out
    {"App:Meta:Dynamic3rd",         eSTREAMID_META_APP_DYNAMIC_DUALYUV,     eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    {"Hal:Meta:Dynamic3rd",         eSTREAMID_META_HAL_DYNAMIC_DUALYUV,     eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    //p2 - output
    {"App:Meta:P2Out",              eSTREAMID_META_APP_DYNAMIC_P2,          eSTREAMTYPE_META_OUT,    9, 9, eStreamType_META_APP},
    {"App:Meta:P2Out_01",           eSTREAMID_META_APP_DYNAMIC_P2_MAIN2,    eSTREAMTYPE_META_OUT,    9, 9, eStreamType_META_APP},
    {"Hal:Meta:P2Out",              eSTREAMID_META_HAL_DYNAMIC_P2,          eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    {"Hal:Meta:P2Out_01",           eSTREAMID_META_HAL_DYNAMIC_P2_MAIN2,    eSTREAMTYPE_META_OUT,   9, 9, eStreamType_META_HAL},
    // End
    {"", 0, 0, 0, 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_P2PrvCap[] =
{
    // Hal
    // Dual3rdPartyNode - input/output
    {"Hal:Image:Pass2_FullRaw",            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(4176, 3088), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw",          eSTREAMID_IMAGE_PIPE_RAW_RESIZER,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(2304, 1296), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw_Main2",    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(1600, 1200), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_FullRaw_Main2",      eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,            eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_BAYER10, MSize(1, 1),    0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:Pass2_Lcso",               eSTREAMID_IMAGE_PIPE_RAW_LCSO,                 eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_STA_2BYTE, MSize(384, 384), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_Lcso_Main2",         eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_STA_2BYTE, MSize(384, 384), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run

    // p2 capture output
    {"Hal:Image:FullYuv",                  eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0, eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:FullYuvMain2",             eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1, eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:ResizeYuv",                eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0,   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:ResizeYuvMain2",           eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1,   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},

    // Dual3rdPartyNode - preview output
    {"Hal:Image:Preview",                  eSTREAMID_IMAGE_PIPE_YUV_00,                   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_YUY2, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:PreviewCB",                eSTREAMID_IMAGE_PIPE_YUV_01,                   eSTREAMTYPE_IMAGE_INOUT, 9, 9, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,                               eImgFmt_YV12, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:FD",                       eSTREAMID_IMAGE_YUV_FD,                        eSTREAMTYPE_IMAGE_OUT,   5, 5, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,                               eImgFmt_YUY2, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // PostView
    {"Hal:Image:PostView",                 eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,        eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,eImgFmt_YUY2, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_P2PrvCap_wo_Main2_Full[] =
{
    // Hal
    // Dual3rdPartyNode - input/output
    {"Hal:Image:Pass2_FullRaw",            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(4176, 3088), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw",          eSTREAMID_IMAGE_PIPE_RAW_RESIZER,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(2304, 1296), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw_Main2",    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_YV12, MSize(1600, 1200), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    //{"Hal:Image:Pass2_FullRaw_Main2",      eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,            eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_BAYER10, MSize(1, 1),    0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:Pass2_Lcso",               eSTREAMID_IMAGE_PIPE_RAW_LCSO,                 eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_STA_2BYTE, MSize(384, 384), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    //{"Hal:Image:Pass2_Lcso_Main2",         eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READ,                              eImgFmt_STA_2BYTE, MSize(384, 384), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run

    // p2 capture output
    {"Hal:Image:FullYuv",                  eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0, eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //{"Hal:Image:FullYuvMain2",             eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1, eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:ResizeYuv",                eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0,   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:ResizeYuvMain2",           eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1,   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_NV21, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},

    // Dual3rdPartyNode - preview output
    {"Hal:Image:Preview",                  eSTREAMID_IMAGE_PIPE_YUV_00,                   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_YUY2, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:PreviewCB",                eSTREAMID_IMAGE_PIPE_YUV_01,                   eSTREAMTYPE_IMAGE_INOUT, 9, 9, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,                               eImgFmt_YV12, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:FD",                       eSTREAMID_IMAGE_YUV_FD,                        eSTREAMTYPE_IMAGE_OUT,   5, 5, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,                               eImgFmt_YUY2, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // PostView
    {"Hal:Image:PostView",                 eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,        eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,eImgFmt_YUY2, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static const MINT32 gStereoP2ConnectSetting_P2PrvCap[] =
{
    eNODEID_StereoRootNode,     eNODEID_RAW16Out,
    eNODEID_StereoRootNode,     eNODEID_Dual3rdPartyNode,
    eNODEID_StereoRootNode,     eNODEID_P2Node,
    eNODEID_StereoRootNode,     eNODEID_P2Node_main2,
    -1,
    -1,
};
};
};
};