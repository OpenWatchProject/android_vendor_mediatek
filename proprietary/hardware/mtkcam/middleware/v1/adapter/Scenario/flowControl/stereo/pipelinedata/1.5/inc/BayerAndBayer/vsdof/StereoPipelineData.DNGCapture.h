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

/*******************************************************************************
*
* Capture Scenario
*
*******************************************************************************/
namespace StereoPipelineMgrData
{
namespace p2
{
namespace dngCap
{
static const struct MetaStreamManager::metadata_info_setting gStereoMetaTbl_P2DNGCap[] =
{
    // Hal
    {"App:Meta:Control",            eSTREAMID_META_APP_CONTROL,         eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_APP},
    {"Hal:Meta:Control",            eSTREAMID_META_HAL_CONTROL,         eSTREAMTYPE_META_IN,    0, 0, eStreamType_META_HAL},
    // StereoRootNode - in/out
    {"Hal:Meta:DynamicRoot",        eSTREAMID_META_HAL_DYNAMIC_P1,          eSTREAMTYPE_META_INOUT,   9, 9, eStreamType_META_HAL},
    {"Hal:Meta:DynamicRoot_01",     eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2,    eSTREAMTYPE_META_INOUT,   9, 9, eStreamType_META_HAL},
    {"App:Meta:RootOut",            eSTREAMID_META_HAL_DYNAMIC_STEREOROOT,  eSTREAMTYPE_META_OUT,    0, 0, eStreamType_META_APP},
    // DepthMapNode - ouput
    {"Hal:Meta:DynamicDepth",       eSTREAMID_META_HAL_DYNAMIC_DEPTH,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_HAL},
    {"APP:Meta:DynamicDepth",       eSTREAMID_META_APP_DYNAMIC_DEPTH,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    // BokehNode - Ouput
    {"Hal:Meta:DynamicBokeh",       eSTREAMID_META_HAL_DYNAMIC_BOKEH,    eSTREAMTYPE_META_INOUT, 1, 1, eStreamType_META_HAL},
    {"App:Meta:Bokeh",              eSTREAMID_META_APP_DYNAMIC_BOKEH,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    // DualImageTransform - output
    {"App:Meta:Dual_Out",           eSTREAMID_META_APP_DYNAMIC_DUALIT,    eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    {"Hal:Meta:P1_main2:Dynamic",   eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2,  eSTREAMTYPE_META_INOUT, 1, 1, eStreamType_META_HAL},
    // JpefEnc_BokehNode - output
    {"App:Meta:JpgEnc_Bokeh",       eSTREAMID_META_APP_DYNAMIC_BOKEH_JPG, eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    // JpefEnc_CleanImageNode - output
    {"App:Meta:JpgEnc_Clean",       eSTREAMID_META_APP_DYNAMIC_JPEG,      eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    // JpefEnc_JpsImageNode - output
    {"App:Meta:JpgEnc_JPS",         eSTREAMID_META_APP_DYNAMIC_JPS,  eSTREAMTYPE_META_OUT,   1, 1, eStreamType_META_APP},
    // End
    {"", 0, 0, 0, 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_P2DNGCap[] =
{
    // Hal
    // DepthMapNode - input
    {"Hal:Image:Pass2_FullRaw",            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw",          eSTREAMID_IMAGE_PIPE_RAW_RESIZER,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw_Main2",    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_Lcso",               eSTREAMID_IMAGE_PIPE_RAW_LCSO,                 eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_2BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_Lcso_Main2",         eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_2BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    // Raw 16
    {"Hal:Image:Raw16",                     eSTREAMID_IMAGE_PIPE_RAW16,                   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_RAW16, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // DepthMapNode - output
    // Hal
    {"Hal:Image:MainImage_Cap",     eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV,     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12,       MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:HalDepthWrapper",   eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER,      eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:LDCData",           eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC,                    eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:ExtraData",         eSTREAMID_IMAGE_PIPE_STEREO_DBG,                        eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:N3DDebug",          eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG,                  eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // Bokeh - output
    {"Hal:Image:CleanImage",        eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_NV21,       MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:BokehResult",       eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_NV21,       MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:Bokeh_Thumb",       eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,                     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, 0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:DepthWrapper",      eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_STA_BYTE,  MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"App:Image:ExtraData",         eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA,             eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_STA_BYTE, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // JpegEnc_BokehNode
    {"Hal:Image:JpegEnc_Bokeh",     eSTREAMID_IMAGE_PIPE_JPG_Bokeh,                         eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, 0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // JpegEnc_BokehNode
    {"Hal:Image:JpegEnc_Clean",     eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg,                  eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, 0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // PostView
    {"Hal:Image:PostView",          eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN,0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl_P2DNGCap_Full[] =
{
    // Hal
    // DepthMapNode - input
    {"Hal:Image:Pass2_FullRaw",            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw",          eSTREAMID_IMAGE_PIPE_RAW_RESIZER,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_FullRaw_Main2",      eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,            eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw_Main2",    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_Lcso",               eSTREAMID_IMAGE_PIPE_RAW_LCSO,                 eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_2BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    {"Hal:Image:Pass2_Lcso_Main2",         eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,              eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_2BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL}, // temp run
    // Raw 16
    {"Hal:Image:Raw16",                     eSTREAMID_IMAGE_PIPE_RAW16,                   eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_RAW16, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // DepthMapNode - output
    // Hal
    {"Hal:Image:MainImage_Cap",     eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV,     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_YV12,       MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:HalDepthWrapper",   eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER,      eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:LDCData",           eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC,                    eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:ExtraData",         eSTREAMID_IMAGE_PIPE_STEREO_DBG,                        eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"Hal:Image:N3DDebug",          eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG,                  eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_STA_BYTE,   MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // Bokeh - output
    {"Hal:Image:CleanImage",        eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_NV21,       MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:BokehResult",       eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, eImgFmt_NV21,       MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:Bokeh_Thumb",       eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,                     eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, 0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:DepthWrapper",      eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER,               eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_STA_BYTE,  MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    {"App:Image:ExtraData",         eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA,             eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_WRITE_OFTEN, eImgFmt_STA_BYTE, MSize(1, 1), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // JpegEnc_BokehNode
    {"Hal:Image:JpegEnc_Bokeh",     eSTREAMID_IMAGE_PIPE_JPG_Bokeh,                         eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, 0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // JpegEnc_BokehNode
    {"Hal:Image:JpegEnc_Clean",     eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg,                  eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN, 0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    // PostView
    {"Hal:Image:PostView",          eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,           eSTREAMTYPE_IMAGE_INOUT, 0, 0, eBUFFER_USAGE_SW_READ_OFTEN,0, MSize(0, 0), 0, 0, eStreamType_IMG_HAL_PROVIDER, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
static const MINT32 gStereoP2ConnectSetting_P2DNGCap[] =
{
    eNODEID_StereoRootNode,  eNODEID_RAW16Out,
    eNODEID_StereoRootNode,  eNODEID_DepthMapNode,
    eNODEID_DepthMapNode,    eNODEID_BokehNode,
    eNODEID_BokehNode,       eNODEID_JpegNode_Clean,
    eNODEID_BokehNode,       eNODEID_JpegNode_Bokeh,
    -1,
    -1,
};
};
};
};