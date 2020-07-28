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
//
#define ZSD_MAIN2_P1_IMGO_ENABLE    0

/*
 * pipeline usage 5
 * + capture buffer 9
 * + spare buffer 2
 * = 16
 */
#define ZSD_BUFFER_COUNT    16
/*******************************************************************************
*
* ZSD Scenario
*
*******************************************************************************/
namespace StereoPipelineMgrData
{
namespace p1
{
namespace zsd
{
static const struct MetaStreamManager::metadata_info_setting gStereoMetaTbl_ZsdP1[] =
{
    //
    {"App:Meta:Control", eSTREAMID_META_APP_CONTROL,    eSTREAMTYPE_META_IN,    ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_APP},
    {"Hal:Meta:Control", eSTREAMID_META_HAL_CONTROL,    eSTREAMTYPE_META_IN,    ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_HAL},
    //
    {"App:Meta:ResultP1", eSTREAMID_META_APP_DYNAMIC_P1, eSTREAMTYPE_META_OUT,   ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_APP},
    {"Hal:Meta:ResultP1", eSTREAMID_META_HAL_DYNAMIC_P1, eSTREAMTYPE_META_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_HAL},
    //
    {"", 0, 0, 0, 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_ZsdP1[] =
{
    { //RRZO. P1 output sizes will be updated later when constructing P1 pipelines
        "Hal:Image:ResizeRaw", eSTREAMID_IMAGE_PIPE_RAW_RESIZER, eSTREAMTYPE_IMAGE_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT,
        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
        eImgFmt_FG_BAYER10, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL
    },
    { //IMGO. P1 output sizes will be updated later when constructing P1 pipelines
        "Hal:Image:FullRaw", eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, eSTREAMTYPE_IMAGE_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT,
        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
        eImgFmt_BAYER10, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL
    },
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static const MINT32 gStereoConnectSetting_ZsdP1[] =
{
    eNODEID_P1Node,
    -1,
    -1,
};
//
static const struct MetaStreamManager::metadata_info_setting gStereoMetaTbl_ZsdP1Main2[] =
{
    //
    {"App:Meta:Control_Main2",  eSTREAMID_META_APP_CONTROL_MAIN2,   eSTREAMTYPE_META_IN,    ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_APP},
    {"Hal:Meta:Control_Main2",  eSTREAMID_META_HAL_CONTROL_MAIN2,   eSTREAMTYPE_META_IN,    ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_HAL},
    //
    {"App:Meta:ResultP1_Main2", eSTREAMID_META_APP_DYNAMIC_P1_MAIN2, eSTREAMTYPE_META_OUT,   ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_APP},
    {"Hal:Meta:ResultP1_Main2", eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2, eSTREAMTYPE_META_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT, eStreamType_META_HAL},
    //
    {"", 0, 0, 0, 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_ZsdP1Main2_RRZO[] =
{
    { //RRZO. P1 output sizes will be updated later when constructing P1 pipelines
        "Hal:Image:ResizeRaw_Main2", eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, eSTREAMTYPE_IMAGE_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT,
        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
        eImgFmt_FG_BAYER10, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL
    },
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_ZsdP1Main2_IMGO[] =
{
    { //IMGO. P1 output sizes will be updated later when constructing P1 pipelines
        "Hal:Image:FullRaw_Main2", eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, eSTREAMTYPE_IMAGE_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT,
        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
        eImgFmt_BAYER10, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL
    },
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_ZsdP1Main2_BOTH[] =
{
    { //RRZO. P1 output sizes will be updated later when constructing P1 pipelines
        "Hal:Image:ResizeRaw_Main2", eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, eSTREAMTYPE_IMAGE_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT,
        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
        eImgFmt_FG_BAYER10, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL
    },
    { //IMGO. P1 output sizes will be updated later when constructing P1 pipelines
        "Hal:Image:FullRaw_Main2", eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, eSTREAMTYPE_IMAGE_INOUT, ZSD_BUFFER_COUNT, ZSD_BUFFER_COUNT,
        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
        eImgFmt_BAYER10, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL
    },
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static const MINT32 gStereoConnectSetting_ZsdP1Main2[] =
{
    eNODEID_P1Node_main2,
    -1,
    -1,
};

}
}
}
