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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxVdecEx.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video Decoder component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#include "stdio.h"
#include "string.h"
#include <cutils/log.h>
#include <signal.h>
#include <sys/mman.h>
//#include "ColorConverter.h"
#include <ui/Rect.h>
#include "MtkOmxVdecEx.h"
#include "OMX_Index.h"

#include <media/stagefright/foundation/ADebug.h>
#include "DpBlitStream.h"
//#include "../MtkOmxVenc/MtkOmxMVAMgr.h"
#include "MtkOmxMVAMgr.h"

#if 1
#include <linux/svp_region.h>
#endif

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#if (ANDROID_VER >= ANDROID_ICS)
#include <android/native_window.h>
#include <HardwareAPI.h>
//#include <gralloc_priv.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/gralloc_extra.h>
#include <ion.h>
#include <linux/mtk_ion.h>
#include "graphics_mtk_defs.h"
#include <utils/threads.h>
#include <poll.h>
#include "gralloc_mtk_defs.h"  //for GRALLOC_USAGE_SECURE
#endif
#include <system/window.h>
#include "ged/ged.h"
#include "ged/ged_type.h"

#define MTK_SW_RESOLUTION_CHANGE_SUPPORT
#define MTK_OMX_H263_DECODER "OMX.MTK.VIDEO.DECODER.H263"
#define MTK_OMX_MPEG4_DECODER  "OMX.MTK.VIDEO.DECODER.MPEG4"
#define MTK_OMX_AVC_DECODER "OMX.MTK.VIDEO.DECODER.AVC"
#define MTK_OMX_AVC_SEC_DECODER "OMX.MTK.VIDEO.DECODER.AVC.secure"
#define MTK_OMX_AVC_l3_DECODER "OMX.MTK.VIDEO.DECODER.AVC.l3"
#define MTK_OMX_RV_DECODER "OMX.MTK.VIDEO.DECODER.RV"
#define MTK_OMX_VC1_DECODER "OMX.MTK.VIDEO.DECODER.VC1"
#define MTK_OMX_VPX_DECODER "OMX.MTK.VIDEO.DECODER.VPX"
#define MTK_OMX_VP9_DECODER "OMX.MTK.VIDEO.DECODER.VP9"
#define MTK_OMX_VP9_SEC_DECODER "OMX.MTK.VIDEO.DECODER.VP9.secure"
#define MTK_OMX_MPEG2_DECODER "OMX.MTK.VIDEO.DECODER.MPEG2"
#define MTK_OMX_DIVX_DECODER "OMX.MTK.VIDEO.DECODER.DIVX"
#define MTK_OMX_DIVX3_DECODER "OMX.MTK.VIDEO.DECODER.DIVX3"
#define MTK_OMX_XVID_DECODER "OMX.MTK.VIDEO.DECODER.XVID"
#define MTK_OMX_S263_DECODER "OMX.MTK.VIDEO.DECODER.S263"
#define MTK_OMX_HEVC_DECODER "OMX.MTK.VIDEO.DECODER.HEVC"
#define MTK_OMX_HEVC_SEC_DECODER "OMX.MTK.VIDEO.DECODER.HEVC.secure"  //HEVC.SEC.M0
#define MTK_OMX_MJPEG_DECODER "OMX.MTK.VIDEO.DECODER.MJPEG"

#define CACHE_LINE_SIZE 64 // LCM of all supported cache line size

MTK_VDEC_PROFILE_MAP_ENTRY HevcProfileMapTable[] =
{
    {OMX_VIDEO_HEVCProfileMain,      VDEC_DRV_H265_VIDEO_PROFILE_H265_MAIN},
    {OMX_VIDEO_HEVCProfileMain10,  VDEC_DRV_H265_VIDEO_PROFILE_H265_MAIN_10},
    {OMX_VIDEO_HEVCProfileMain10HDR10,  VDEC_DRV_H265_VIDEO_PROFILE_H265_HDR_10}
};

MTK_VDEC_LEVEL_MAP_ENTRY HevcLevelMapTable[] =
{
    {OMX_VIDEO_HEVCMainTierLevel1,   VDEC_DRV_VIDEO_LEVEL_1},
    {OMX_VIDEO_HEVCHighTierLevel1,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_1},
    {OMX_VIDEO_HEVCMainTierLevel2,  VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_HEVCHighTierLevel2,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_2},
    {OMX_VIDEO_HEVCMainTierLevel21,  VDEC_DRV_VIDEO_LEVEL_2_1},
    {OMX_VIDEO_HEVCHighTierLevel21,   VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_2_1},
    {OMX_VIDEO_HEVCMainTierLevel3,  VDEC_DRV_VIDEO_LEVEL_3},
    {OMX_VIDEO_HEVCHighTierLevel3,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_3},
    {OMX_VIDEO_HEVCMainTierLevel31,   VDEC_DRV_VIDEO_LEVEL_3_1},
    {OMX_VIDEO_HEVCHighTierLevel31,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_3_1},
    {OMX_VIDEO_HEVCMainTierLevel4,   VDEC_DRV_VIDEO_LEVEL_4},
    {OMX_VIDEO_HEVCHighTierLevel4,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_4},
    {OMX_VIDEO_HEVCMainTierLevel41,  VDEC_DRV_VIDEO_LEVEL_4_1},
    {OMX_VIDEO_HEVCHighTierLevel41,   VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_4_1},
    {OMX_VIDEO_HEVCMainTierLevel5,  VDEC_DRV_VIDEO_LEVEL_5},
    {OMX_VIDEO_HEVCHighTierLevel5,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_5},
    {OMX_VIDEO_HEVCMainTierLevel51,   VDEC_DRV_VIDEO_LEVEL_5_1},
    {OMX_VIDEO_HEVCHighTierLevel51,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_5_1},
    {OMX_VIDEO_HEVCMainTierLevel52,   VDEC_DRV_VIDEO_LEVEL_5_2},
    {OMX_VIDEO_HEVCHighTierLevel52,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_5_2},
    {OMX_VIDEO_HEVCMainTierLevel6,   VDEC_DRV_VIDEO_LEVEL_6},
    {OMX_VIDEO_HEVCHighTierLevel6,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_6},
    {OMX_VIDEO_HEVCMainTierLevel61,   VDEC_DRV_VIDEO_LEVEL_6_1},
    {OMX_VIDEO_HEVCHighTierLevel61,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_6_1},
    {OMX_VIDEO_HEVCMainTierLevel62,   VDEC_DRV_VIDEO_LEVEL_6_2},
    {OMX_VIDEO_HEVCHighTierLevel62,  VDEC_DRV_VIDEO_HIGH_TIER_LEVEL_6_2}
};



MTK_VDEC_PROFILE_MAP_ENTRY AvcProfileMapTable[] =
{
    /*
        // from OMX_VIDEO_AVCPROFILETYPE
        public static final int AVCProfileBaseline = 0x01;
        public static final int AVCProfileMain     = 0x02;
        public static final int AVCProfileExtended = 0x04;
        public static final int AVCProfileHigh     = 0x08;
        public static final int AVCProfileHigh10   = 0x10;
        public static final int AVCProfileHigh422  = 0x20;
        public static final int AVCProfileHigh444  = 0x40;
    */
    {OMX_VIDEO_AVCProfileBaseline,  VDEC_DRV_H264_VIDEO_PROFILE_H264_BASELINE},
    {OMX_VIDEO_AVCProfileMain,      VDEC_DRV_H264_VIDEO_PROFILE_H264_MAIN},
    {OMX_VIDEO_AVCProfileExtended,  VDEC_DRV_H264_VIDEO_PROFILE_H264_EXTENDED},
    {OMX_VIDEO_AVCProfileHigh,      VDEC_DRV_H264_VIDEO_PROFILE_H264_HIGH},
    {OMX_VIDEO_AVCProfileHigh10,    VDEC_DRV_H264_VIDEO_PROFILE_H264_HIGH_10},
    {OMX_VIDEO_AVCProfileHigh422,   VDEC_DRV_H264_VIDEO_PROFILE_H264_HIGH422},
    {OMX_VIDEO_AVCProfileHigh444,   VDEC_DRV_H264_VIDEO_PROFILE_H264_HIGH444},
};

MTK_VDEC_LEVEL_MAP_ENTRY AvcLevelMapTable[] =
{
    /*
        // from OMX_VIDEO_AVCLEVELTYPE
        public static final int AVCLevel1       = 0x01;
        public static final int AVCLevel1b      = 0x02;
        public static final int AVCLevel11      = 0x04;
        public static final int AVCLevel12      = 0x08;
        public static final int AVCLevel13      = 0x10;
        public static final int AVCLevel2       = 0x20;
        public static final int AVCLevel21      = 0x40;
        public static final int AVCLevel22      = 0x80;
        public static final int AVCLevel3       = 0x100;
        public static final int AVCLevel31      = 0x200;
        public static final int AVCLevel32      = 0x400;
        public static final int AVCLevel4       = 0x800;
        public static final int AVCLevel41      = 0x1000;
        public static final int AVCLevel42      = 0x2000;
        public static final int AVCLevel5       = 0x4000;
        public static final int AVCLevel51      = 0x8000;
    */
    {OMX_VIDEO_AVCLevel1,   VDEC_DRV_VIDEO_LEVEL_1},
    {OMX_VIDEO_AVCLevel1b,  VDEC_DRV_VIDEO_LEVEL_1b},
    {OMX_VIDEO_AVCLevel11,  VDEC_DRV_VIDEO_LEVEL_1_1},
    {OMX_VIDEO_AVCLevel12,  VDEC_DRV_VIDEO_LEVEL_1_2},
    {OMX_VIDEO_AVCLevel13,  VDEC_DRV_VIDEO_LEVEL_1_3},
    {OMX_VIDEO_AVCLevel2,   VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_AVCLevel21,  VDEC_DRV_VIDEO_LEVEL_2_1},
    {OMX_VIDEO_AVCLevel22,  VDEC_DRV_VIDEO_LEVEL_2_2},
    {OMX_VIDEO_AVCLevel3,   VDEC_DRV_VIDEO_LEVEL_3},
    {OMX_VIDEO_AVCLevel31,  VDEC_DRV_VIDEO_LEVEL_3_1},
    {OMX_VIDEO_AVCLevel32,  VDEC_DRV_VIDEO_LEVEL_3_2},
    {OMX_VIDEO_AVCLevel4,   VDEC_DRV_VIDEO_LEVEL_4},
    {OMX_VIDEO_AVCLevel41,  VDEC_DRV_VIDEO_LEVEL_4_1},
    {OMX_VIDEO_AVCLevel42,  VDEC_DRV_VIDEO_LEVEL_4_2},
    {OMX_VIDEO_AVCLevel5,   VDEC_DRV_VIDEO_LEVEL_5},
    {OMX_VIDEO_AVCLevel51,  VDEC_DRV_VIDEO_LEVEL_5_1},
};

MTK_VDEC_PROFILE_MAP_ENTRY H263ProfileMapTable[] =
{
    /*
        // from OMX_VIDEO_H263PROFILETYPE
        public static final int H263ProfileBaseline             = 0x01;
        public static final int H263ProfileH320Coding           = 0x02;
        public static final int H263ProfileBackwardCompatible   = 0x04;
        public static final int H263ProfileISWV2                = 0x08;
        public static final int H263ProfileISWV3                = 0x10;
        public static final int H263ProfileHighCompression      = 0x20;
        public static final int H263ProfileInternet             = 0x40;
        public static final int H263ProfileInterlace            = 0x80;
        public static final int H263ProfileHighLatency          = 0x100;
    */
    {OMX_VIDEO_H263ProfileBaseline,             VDEC_DRV_MPEG_VIDEO_PROFILE_H263_0},
    {OMX_VIDEO_H263ProfileH320Coding,           VDEC_DRV_MPEG_VIDEO_PROFILE_H263_1},
    {OMX_VIDEO_H263ProfileBackwardCompatible,   VDEC_DRV_MPEG_VIDEO_PROFILE_H263_2},
    {OMX_VIDEO_H263ProfileISWV2,                VDEC_DRV_MPEG_VIDEO_PROFILE_H263_3},
    {OMX_VIDEO_H263ProfileISWV3,                VDEC_DRV_MPEG_VIDEO_PROFILE_H263_4},
    {OMX_VIDEO_H263ProfileHighCompression,      VDEC_DRV_MPEG_VIDEO_PROFILE_H263_5},
    {OMX_VIDEO_H263ProfileInternet,             VDEC_DRV_MPEG_VIDEO_PROFILE_H263_6},
    {OMX_VIDEO_H263ProfileInterlace,            VDEC_DRV_MPEG_VIDEO_PROFILE_H263_7},
    {OMX_VIDEO_H263ProfileHighLatency,          VDEC_DRV_MPEG_VIDEO_PROFILE_H263_8},
};

MTK_VDEC_LEVEL_MAP_ENTRY H263LevelMapTable[] =
{
    /*
        // from OMX_VIDEO_H263LEVELTYPE
        public static final int H263Level10      = 0x01;
        public static final int H263Level20      = 0x02;
        public static final int H263Level30      = 0x04;
        public static final int H263Level40      = 0x08;
        public static final int H263Level45      = 0x10;
        public static final int H263Level50      = 0x20;
        public static final int H263Level60      = 0x40;
        public static final int H263Level70      = 0x80;
    */
    {OMX_VIDEO_H263Level10,    VDEC_DRV_VIDEO_LEVEL_1},
    {OMX_VIDEO_H263Level20,    VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_H263Level30,    VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_H263Level40,    VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_H263Level45,    VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_H263Level50,    VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_H263Level60,    VDEC_DRV_VIDEO_LEVEL_3},
    {OMX_VIDEO_H263Level70,    VDEC_DRV_VIDEO_LEVEL_4},
};

MTK_VDEC_PROFILE_MAP_ENTRY MPEG4ProfileMapTable[] =
{
    /*
        // from OMX_VIDEO_MPEG4PROFILETYPE
        public static final int MPEG4ProfileSimple              = 0x01;
        public static final int MPEG4ProfileSimpleScalable      = 0x02;
        public static final int MPEG4ProfileCore                = 0x04;
        public static final int MPEG4ProfileMain                = 0x08;
        public static final int MPEG4ProfileNbit                = 0x10;
        public static final int MPEG4ProfileScalableTexture     = 0x20;
        public static final int MPEG4ProfileSimpleFace          = 0x40;
        public static final int MPEG4ProfileSimpleFBA           = 0x80;
        public static final int MPEG4ProfileBasicAnimated       = 0x100;
        public static final int MPEG4ProfileHybrid              = 0x200;
        public static final int MPEG4ProfileAdvancedRealTime    = 0x400;
        public static final int MPEG4ProfileCoreScalable        = 0x800;
        public static final int MPEG4ProfileAdvancedCoding      = 0x1000;
        public static final int MPEG4ProfileAdvancedCore        = 0x2000;
        public static final int MPEG4ProfileAdvancedScalable    = 0x4000;
        public static final int MPEG4ProfileAdvancedSimple      = 0x8000;
    */
    {OMX_VIDEO_MPEG4ProfileSimple,              VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE},
    //{OMX_VIDEO_MPEG4ProfileSimpleScalable,      VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileCore,                VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileMain,                VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileNbit,                VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileScalableTexture,     VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileSimpleFace,          VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileSimpleFBA,           VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileBasicAnimated,       VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileHybrid,              VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileAdvancedRealTime,    VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileCoreScalable,        VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileAdvancedCoding,      VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileAdvancedCore,        VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_MPEG4ProfileAdvancedScalable,    VDEC_DRV_VIDEO_UNSUPPORTED},
    {OMX_VIDEO_MPEG4ProfileAdvancedSimple,      VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE},
};

MTK_VDEC_LEVEL_MAP_ENTRY MPEG4LevelMapTable[] =
{
    /*
        // from OMX_VIDEO_MPEG4LEVELTYPE
        public static final int MPEG4Level0      = 0x01;
        public static final int MPEG4Level0b     = 0x02;
        public static final int MPEG4Level1      = 0x04;
        public static final int MPEG4Level2      = 0x08;
        public static final int MPEG4Level3      = 0x10;
        public static final int MPEG4Level4      = 0x20;
        public static final int MPEG4Level4a     = 0x40;
        public static final int MPEG4Level5      = 0x80;
    */
    {OMX_VIDEO_MPEG4Level0,     VDEC_DRV_VIDEO_LEVEL_0},
    {OMX_VIDEO_MPEG4Level0b,    VDEC_DRV_VIDEO_LEVEL_1},
    {OMX_VIDEO_MPEG4Level1,     VDEC_DRV_VIDEO_LEVEL_1},
    {OMX_VIDEO_MPEG4Level2,     VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_MPEG4Level3,     VDEC_DRV_VIDEO_LEVEL_3},
    {OMX_VIDEO_MPEG4Level4,     VDEC_DRV_VIDEO_LEVEL_4},
    {OMX_VIDEO_MPEG4Level4a,    VDEC_DRV_VIDEO_LEVEL_5},
    {OMX_VIDEO_MPEG4Level5,     VDEC_DRV_VIDEO_LEVEL_5},
};

MTK_VDEC_PROFILE_MAP_ENTRY MPEG2ProfileMapTable[] =
{
    /*
        // from OMX_VIDEO_MPEG2PROFILETYPE
        public static final int MPEG2ProfileSimple    = 0x00;
        public static final int MPEG2ProfileMain      = 0x01;
        public static final int MPEG2Profile422       = 0x02;
        public static final int MPEG2ProfileSNR       = 0x03;
        public static final int MPEG2ProfileSpatial   = 0x04;
        public static final int MPEG2ProfileHigh      = 0x05;
    */
    {OMX_VIDEO_MPEG2ProfileSimple,    VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG2_SIMPLE},
    {OMX_VIDEO_MPEG2ProfileMain,      VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG2_MAIN},
    /*{OMX_VIDEO_MPEG2Profile422,     VDEC_DRV_VIDEO_UNSUPPORTED},*/
    {OMX_VIDEO_MPEG2ProfileSNR,       VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG2_SNR},
    {OMX_VIDEO_MPEG2ProfileSpatial,   VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG2_SPATIAL},
    {OMX_VIDEO_MPEG2ProfileHigh,      VDEC_DRV_MPEG_VIDEO_PROFILE_MPEG2_HIGH},
};

MTK_VDEC_LEVEL_MAP_ENTRY MPEG2LevelMapTable[] =
{
    /*
        from OMX_VIDEO_MPEG2PROFILETYPE
        public static final int MPEG2LevelLL    = 0x00;
        public static final int MPEG2LevelML    = 0x01;
        public static final int MPEG2LevelH14   = 0x02;
        public static final int MPEG2LevelHL    = 0x03;
        public static final int MPEG2LevelHP    = 0x04;
    */
    {OMX_VIDEO_MPEG2LevelLL,    VDEC_DRV_VIDEO_LEVEL_LOW},
    {OMX_VIDEO_MPEG2LevelML,    VDEC_DRV_VIDEO_LEVEL_MEDIUM},
    {OMX_VIDEO_MPEG2LevelH14,   VDEC_DRV_VIDEO_LEVEL_HIGH1440},
    {OMX_VIDEO_MPEG2LevelHL,    VDEC_DRV_VIDEO_LEVEL_HIGH},
};

MTK_VDEC_PROFILE_MAP_ENTRY VP8ProfileMapTable[] =
{
    /*
        // from OMX_VIDEO_VP8PROFILETYPE
        public static final int VP8ProfileMain = 0x01;
    */
    //{OMX_VIDEO_VP8ProfileMain,    VDEC_DRV_VIDEO_UNSUPPORTED},
};

MTK_VDEC_LEVEL_MAP_ENTRY VP8LevelMapTable[] =
{
    /*
        // from OMX_VIDEO_VP8LEVELTYPE
        public static final int VP8Level_Version0 = 0x01;
        public static final int VP8Level_Version1 = 0x02;
        public static final int VP8Level_Version2 = 0x04;
        public static final int VP8Level_Version3 = 0x08;
    */
    //{OMX_VIDEO_VP8Level_Version0,    VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_VP8Level_Version1,    VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_VP8Level_Version2,    VDEC_DRV_VIDEO_UNSUPPORTED},
    //{OMX_VIDEO_VP8Level_Version3,    VDEC_DRV_VIDEO_UNSUPPORTED},
};

/*
VP9:
Profile Bit depth SRGB Colorspace support                   Chroma subsampling
0        8            No                                    YUV 4:2:0
1        8            Yes                                   YUV 4:2:2, YUV 4:4:0 or YUV 4:4:4
2       10 or 12      No                                    UV 4:2:0
3       10 or 12      Yes                                   YUV 4:2:2, YUV 4:4:0 or YUV 4:4:4

Level    Example Frame Size @ Display Rate (fps)***
1             256x144@15
1.1           384x192@30
2             480x256@30
2.1           640x384@30
3            1080x512@30
3.1          1280x768@30
4           2048x1088@30
4.1         2048x1088@60
5           4096x2176@30
5.1         4096x2176@60
5.2        4096x2176@120
6           8192x4352@30
6.1         8192x4352@60
6.2        8192x4352@120
*/

MTK_VDEC_PROFILE_MAP_ENTRY VP9ProfileMapTable[] =
{
    /*
        OMX_VIDEO_VP9Profile0       = 0x1,
        OMX_VIDEO_VP9Profile1       = 0x2,
        OMX_VIDEO_VP9Profile2       = 0x4,
        OMX_VIDEO_VP9Profile3       = 0x8,
        // HDR profiles also support passing HDR metadata
        OMX_VIDEO_VP9Profile2HDR    = 0x1000,
        OMX_VIDEO_VP9Profile3HDR    = 0x2000,
        OMX_VIDEO_VP9ProfileUnknown = 0x6EFFFFFF,
        OMX_VIDEO_VP9ProfileMax     = 0x7FFFFFFF
    */
    {OMX_VIDEO_VP9Profile0,    VDEC_DRV_VP9_VIDEO_PROFILE_0},
    {OMX_VIDEO_VP9Profile1,    VDEC_DRV_VP9_VIDEO_PROFILE_1},
    {OMX_VIDEO_VP9Profile2,    VDEC_DRV_VP9_VIDEO_PROFILE_2},
    {OMX_VIDEO_VP9Profile3,    VDEC_DRV_VP9_VIDEO_PROFILE_3},

    // HDR profiles also support passing HDR metadata
    {OMX_VIDEO_VP9Profile2HDR,    VDEC_DRV_VP9_VIDEO_PROFILE_2_HDR},
    {OMX_VIDEO_VP9Profile3HDR,    VDEC_DRV_VP9_VIDEO_PROFILE_3_HDR},
};

MTK_VDEC_LEVEL_MAP_ENTRY VP9LevelMapTable[] =
{
    /*
        OMX_VIDEO_VP9Level1       = 0x1,
        OMX_VIDEO_VP9Level11      = 0x2,
        OMX_VIDEO_VP9Level2       = 0x4,
        OMX_VIDEO_VP9Level21      = 0x8,
        OMX_VIDEO_VP9Level3       = 0x10,
        OMX_VIDEO_VP9Level31      = 0x20,
        OMX_VIDEO_VP9Level4       = 0x40,
        OMX_VIDEO_VP9Level41      = 0x80,
        OMX_VIDEO_VP9Level5       = 0x100,
        OMX_VIDEO_VP9Level51      = 0x200,
        OMX_VIDEO_VP9Level52      = 0x400,
        OMX_VIDEO_VP9Level6       = 0x800,
        OMX_VIDEO_VP9Level61      = 0x1000,
        OMX_VIDEO_VP9Level62      = 0x2000,
        OMX_VIDEO_VP9LevelUnknown = 0x6EFFFFFF,
        OMX_VIDEO_VP9LevelMax     = 0x7FFFFFFF
    */
    {OMX_VIDEO_VP9Level1,    VDEC_DRV_VIDEO_LEVEL_1},
    {OMX_VIDEO_VP9Level11,    VDEC_DRV_VIDEO_LEVEL_1_1},
    {OMX_VIDEO_VP9Level2,    VDEC_DRV_VIDEO_LEVEL_2},
    {OMX_VIDEO_VP9Level21,    VDEC_DRV_VIDEO_LEVEL_2_1},
    {OMX_VIDEO_VP9Level3,    VDEC_DRV_VIDEO_LEVEL_3},
    {OMX_VIDEO_VP9Level31,    VDEC_DRV_VIDEO_LEVEL_3_1},
    {OMX_VIDEO_VP9Level4,    VDEC_DRV_VIDEO_LEVEL_4},
    {OMX_VIDEO_VP9Level41,    VDEC_DRV_VIDEO_LEVEL_4_1},
    {OMX_VIDEO_VP9Level5,    VDEC_DRV_VIDEO_LEVEL_5},
    {OMX_VIDEO_VP9Level51,    VDEC_DRV_VIDEO_LEVEL_5_1},
    {OMX_VIDEO_VP9Level52,    VDEC_DRV_VIDEO_LEVEL_5_2},
    {OMX_VIDEO_VP9Level6,    VDEC_DRV_VIDEO_LEVEL_6},
    {OMX_VIDEO_VP9Level61,    VDEC_DRV_VIDEO_LEVEL_6_1},
    {OMX_VIDEO_VP9Level62,    VDEC_DRV_VIDEO_LEVEL_6_2},
};

#define MAX_HEVC_PROFILE_MAP_TABLE_SIZE    (sizeof(HevcProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_HEVC_LEVEL_MAP_TABLE_SIZE    (sizeof(HevcLevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))
#define MAX_AVC_PROFILE_MAP_TABLE_SIZE    (sizeof(AvcProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_AVC_LEVEL_MAP_TABLE_SIZE    (sizeof(AvcLevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))
#define MAX_MPEG4_PROFILE_MAP_TABLE_SIZE    (sizeof(MPEG4ProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_MPEG4_LEVEL_MAP_TABLE_SIZE    (sizeof(MPEG4LevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))
#define MAX_H263_PROFILE_MAP_TABLE_SIZE    (sizeof(H263ProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_H263_LEVEL_MAP_TABLE_SIZE    (sizeof(H263LevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))
#define MAX_MPEG2_PROFILE_MAP_TABLE_SIZE    (sizeof(MPEG2ProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_MPEG2_LEVEL_MAP_TABLE_SIZE    (sizeof(MPEG2LevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))
#define MAX_VP8_PROFILE_MAP_TABLE_SIZE    (sizeof(VP8ProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_VP8_LEVEL_MAP_TABLE_SIZE    (sizeof(VP8LevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))
#define MAX_VP9_PROFILE_MAP_TABLE_SIZE    (sizeof(VP9ProfileMapTable)/sizeof(MTK_VDEC_PROFILE_MAP_ENTRY))
#define MAX_VP9_LEVEL_MAP_TABLE_SIZE    (sizeof(VP9LevelMapTable)/sizeof(MTK_VDEC_LEVEL_MAP_ENTRY))


HEVCColorPrimaries HDRColorPrimariesMap[] =
{
    HEVC_ColorPrimaries_Unspecified,  // PrimariesUnspecified
    HEVC_ColorPrimaries_BT709_5,      // PrimariesBT709_5
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesBT470_6M
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesBT601_6_625
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesBT601_6_525
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesGenericFilm
    HEVC_ColorPrimaries_BT2020        // PrimariesBT2020
};

HEVCTransfer HDRTransferMap[] =
{
    HEVC_Transfer_Unspecified,     // TransferUnspecified
    HEVC_Transfer_Reserved_1,      // TransferLinear
    HEVC_Transfer_Reserved_1,      // TransferSRGB
    HEVC_Transfer_BT709_5,         // TransferSMPTE170M
    HEVC_Transfer_Reserved_1,      // TransferGamma22
    HEVC_Transfer_Reserved_1,      // TransferGamma28
    HEVC_Transfer_SMPTE_ST_2048,   // TransferST2084
    HEVC_Transfer_HLG              // TransferHLG
};

HEVCMatrixCoeff HDRMatrixCoeffMap[] =
{
    HEVC_MatrixCoeff_Unspecified,   // MatrixUnspecified
    HEVC_MatrixCoeff_BT709_5,       // MatrixBT709_5
    HEVC_MatrixCoeff_Reserved,      // MatrixBT470_6M
    HEVC_MatrixCoeff_Reserved,      // MatrixBT601_6
    HEVC_MatrixCoeff_Reserved,      // MatrixSMPTE240M
    HEVC_MatrixCoeff_BT2020,        // MatrixBT2020
    HEVC_MatrixCoeff_BT2020Cons     // MatrixBT2020Constant
};

#define HDR_Color_Primaries_Map_SIZE    (sizeof(HDRColorPrimariesMap)/sizeof(HEVCColorPrimaries))
#define HDR_Transfer_Map_SIZE    (sizeof(HDRTransferMap)/sizeof(HEVCTransfer))
#define HDR_Matrix_Coeff_Map_SIZE    (sizeof(HDRMatrixCoeffMap)/sizeof(HEVCMatrixCoeff))


const uint32_t FPS_PROFILE_COUNT = 30;

#if PROFILING
//static FILE *fpVdoProfiling;

int64_t getTickCountMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}

static int64_t getTickCountUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000000LL + tv.tv_usec);
}

void show_uptime()
{
    char cmdline[1024];
    int r;

    //static fd = open("/proc/uptime", 0);  //O_RDONLY=0
    static int fd = 0;
    if (fd == 0)
    {
        r = 0;
        fd = open("/proc/uptime", 0);
    }
    else
    {
        lseek(fd, 0, SEEK_SET);
        r = read(fd, cmdline, 1023);
        //close(fd);
        if (r < 0) { r = 0; }
    }
    cmdline[r] = 0;
    ALOGD("uptime - %s", cmdline);
}
#endif

template<class T>
static OMX_BOOL checkOMXParams(T *params)
{
    if (params->nSize != sizeof(T)              ||
        params->nVersion.s.nVersionMajor != 1   ||
        params->nVersion.s.nVersionMinor != 0   ||
        params->nVersion.s.nRevision != 0       ||
        params->nVersion.s.nStep != 0) {
            return OMX_FALSE;
        }
    return OMX_TRUE;
}

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

void __setBufParameter(buffer_handle_t hnd, int32_t mask, int32_t value)
{
    gralloc_extra_ion_sf_info_t info;
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    gralloc_extra_sf_set_status(&info, mask, value);
    gralloc_extra_perform(hnd, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
}

void writeBufferToFile
(
    const char *filename,
    void       *buffer,
    uint32_t    length
)
{
    FILE *fp;
    ALOGD("writeBufferToFile: filename=%s, buffer=0x%p, length=%d bytes", filename, buffer, length);
    fp = fopen(filename, "ab");
    if (fp)
    {
        fwrite(buffer, 1, length, fp);
        fclose(fp);
    }
    else
    {
        ALOGE("writeBufferToFile: fopen failed.");
    }
}

#if (ANDROID_VER >= ANDROID_KK)
OMX_BOOL MtkOmxVdec::SetupMetaIonHandle(OMX_BUFFERHEADERTYPE *pBufHdr)
{
    OMX_U32 graphicBufHandle = 0;

    if (OMX_FALSE == GetMetaHandleFromOmxHeader(pBufHdr, &graphicBufHandle))
    {
        MTK_OMX_LOGE("SetupMetaIonHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE("SetupMetaIonHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    VBufInfo info;
    int ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
    if (ret >= 0)
    {
        // found handle
        //MTK_OMX_LOGD("SetupMetaIonHandle found handle, u4BuffHdr(0x%08x)", pBufHdr);
        return OMX_TRUE;
    }
    else
    {
        // cannot found handle, create a new entry
        //MTK_OMX_LOGD("SetupMetaIonHandle cannot find handle, create a new entry,LINE:%d", __LINE__);
    }

    int count = 0;

    //if (mIonOutputBufferCount < mOutputPortDef.nBufferCountActual)
    //{
        // cannot found handle, create a new entry
        //MTK_OMX_LOGD("SetupMetaIonHandle cannot found handle, create a new entry, LINE:%d", __LINE__);

        ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, (void *)pBufHdr);
        if (ret < 0)
        {
            MTK_OMX_LOGE("[ERROR]newOmxMVAwithHndl() failed");
        }

        OMX_U32 buffer, bufferSize = 0;
        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE("[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
            return OMX_FALSE;
        }

        buffer = BufInfo.u4VA;
        bufferSize = BufInfo.u4BuffSize;

        gralloc_extra_ion_sf_info_t sf_info;
        memset(&sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));
        gralloc_extra_query((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        if ((sf_info.status & GRALLOC_EXTRA_MASK_TYPE) != GRALLOC_EXTRA_BIT_TYPE_VIDEO)
        {
            OMX_U32 u4PicAllocSize = mOutputStrideBeforeReconfig * mOutputSliceHeightBeforeReconfig;
            MTK_OMX_LOGD("First allocated buffer memset to black u4PicAllocSize %d, buffSize %d", u4PicAllocSize, bufferSize);
            //set default color to black
            memset((void *)(buffer + u4PicAllocSize), 128, u4PicAllocSize / 2);
            memset((void *)(buffer), 0x10, u4PicAllocSize);
            mOutputMVAMgr->syncBufferCacheFrm((void*)BufInfo.u4VA, (unsigned int)ION_CACHE_FLUSH_BY_RANGE);
        }

        buffer_handle_t _handle = NULL;
        _handle = (buffer_handle_t)graphicBufHandle;
        //ret = mOutputMVAMgr->getMapHndlFromIndex(count, &_handle);

        if (_handle != NULL)
        {
            gralloc_extra_ion_sf_info_t sf_info;
            //MTK_OMX_LOGD ("gralloc_extra_query");
            memset(&sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));

            gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

            sf_info.pool_id = (int32_t)this; //  for PQ to identify bitstream instance.

            gralloc_extra_sf_set_status(&sf_info,
                                        GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
                                        GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);

            gralloc_extra_perform(_handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
            //MTK_OMX_LOGD("gralloc_extra_perform");
        }
        else
        {
            MTK_OMX_LOGE("SetupMetaIonHandle pool id not set , DC failed,graphicBufHandle value %d\n",graphicBufHandle);
        }


        mIonOutputBufferCount++;
        MTK_OMX_LOGD("pBuffer(0x%08x),VA(0x%08X), PA(0x%08X), size(%d) mIonOutputBufferCount(%d)", pBufHdr, BufInfo.u4VA, BufInfo.u4PA, bufferSize, mIonOutputBufferCount);

//#ifdef MTK_CLEARMOTION_SUPPORT
        if (mMJCEnable == OMX_TRUE)
        {
        //MTK_OMX_LOGD("(%d), (%d), (%d)", mUseClearMotion, mSeqInfoCompleteFlag, mInputPortFormat.eCompressionFormat);

        if (((mUseClearMotion && (mInputPortFormat.eCompressionFormat != OMX_VIDEO_CodingAVC) && (mInputPortFormat.eCompressionFormat != OMX_VIDEO_CodingHEVC))
            || (mUseClearMotion && (OMX_TRUE == mSeqInfoCompleteFlag) && ((mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) || (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC))))
            && mPortReconfigInProgress == OMX_FALSE)
        {
            if (mMJCBufferCount < MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT)
            {
                mMJCBufferInfo[mMJCBufferCount].pNativeHandle = BufInfo.pNativeHandle;
                mMJCBufferInfo[mMJCBufferCount].u4VA = BufInfo.u4VA;
                mMJCBufferInfo[mMJCBufferCount].u4PA = BufInfo.u4PA;
                mMJCBufferInfo[mMJCBufferCount].u4Size = BufInfo.u4BuffSize;
                mMJCBufferInfo[mMJCBufferCount].u4Hdr = (VAL_UINT32_T)pBufHdr;

                if (mMJCLog)
                {
                    MTK_OMX_LOGD("[MJC] [SetupMetaIonHandle][MJC BUFFER][%d] NativeHandle = 0x%08X, Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X",
                                 mMJCBufferCount,
                                 mMJCBufferInfo[mMJCBufferCount].pNativeHandle,
                                 mMJCBufferInfo[mMJCBufferCount].u4VA,
                                 mMJCBufferInfo[mMJCBufferCount].u4PA,
                                 mMJCBufferInfo[mMJCBufferCount].u4Size,
                                 mMJCBufferInfo[mMJCBufferCount].u4Hdr
                                );
                }

                mMJCBufferCount++;
            }
            else
            {
                // For Scaler ClearMotion +
                if ((mMJCBufferCount + mScalerBufferCount) < TOTAL_MJC_BUFFER_CNT)
                {
                    mScalerBufferInfo[mScalerBufferCount].pNativeHandle = BufInfo.pNativeHandle;
                    mScalerBufferInfo[mScalerBufferCount].u4VA = BufInfo.u4VA;
                    mScalerBufferInfo[mScalerBufferCount].u4PA = BufInfo.u4PA;
                    mScalerBufferInfo[mScalerBufferCount].u4Size = BufInfo.u4BuffSize;
                    mScalerBufferInfo[mScalerBufferCount].u4Hdr = (VAL_UINT32_T)pBufHdr;

                    if (mMJCLog)
                    {
                        MTK_OMX_LOGD("[MJC] [SetupMetaIonHandle][SCALAR BUFFER][%d] NativeHandle = 0x%08X, Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X",
                                     mScalerBufferCount,
                                     mScalerBufferInfo[mScalerBufferCount].pNativeHandle,
                                     mScalerBufferInfo[mScalerBufferCount].u4VA,
                                     mScalerBufferInfo[mScalerBufferCount].u4PA,
                                     mScalerBufferInfo[mScalerBufferCount].u4Size,
                                     mScalerBufferInfo[mScalerBufferCount].u4Hdr
                                    );
                    }

                    mScalerBufferCount++;
                }
                // For Scaler ClearMotion -
            }
        }
        }
//#endif
        //return OMX_TRUE;

    //}

    if (mIonOutputBufferCount >= mOutputPortDef.nBufferCountActual)
    {
        //MTK_OMX_LOGD("SetupMetaIonHandle ERROR: Cannot found empty entry");
        //MTK_OMX_LOGE("SetupMetaIonHandle Warning: mIonOutputBufferCount %d,u4BuffHdr(0x%08x),graphicBufHandle(0x%08x)" ,mIonOutputBufferCount, pBufHdr,graphicBufHandle );
        //return OMX_FALSE;
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::SetupMetaIonHandleAndGetFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *pBuffer)
{
    // check if we had this handle first
    OMX_U32 graphicBufHandle = 0;
    if (OMX_FALSE == GetMetaHandleFromBufferPtr(pBuffer, &graphicBufHandle))
    {
        MTK_OMX_LOGE("SetupMetaIonHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE("SetupMetaIonHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    OMX_U32 buffer, bufferSize = 0;
    VBufInfo BufInfo;
    int ret = 0;
    if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
    {
        MTK_OMX_LOGE("[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
        MTK_OMX_LOGE("SetupMetaIonHandleAndGetFrame failed, cannot found handle, LINE:%d", __LINE__);
        // Add new entry
        ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, NULL);

        if (ret < 0)
        {
            MTK_OMX_LOGE("[ERROR]newOmxMVAwithHndl() failed");
        }

        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE("[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
            return OMX_FALSE;
        }
    }

    aFrame->rBaseAddr.u4VA = BufInfo.u4VA;
    aFrame->rBaseAddr.u4PA = BufInfo.u4PA;
    aFrame->rBaseAddr.hMemHandle = BufInfo.ionBufHndl;

    // TODO: FREE ION related resource

    return OMX_TRUE;
}
#endif

OMX_BOOL MtkOmxVdec::GetM4UFrameandBitstreamBuffer(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf)
{
    MTK_OMX_LOGD("deprecated");
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::GetBitstreamBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    //MTK_OMX_LOGD("[M4U] mM4UBufferCount = %d\n", mM4UBufferCount);

    VAL_UINT32_T u4x, u4y;

    if (OMX_TRUE == mIsSecureInst)
    {
        mRingbuf.rSecMemHandle = (unsigned long)aInputBuf;
        if(mIsSecUsingNativeHandle)
        {
            int i;
            for(i = 0; i < mSecInputBufCount; i++)
            {
                if(aInputBuf == mSecInputBufInfo[i].pNativeHandle)
                {
                    mRingbuf.rSecMemHandle = mSecInputBufInfo[i].u4SecHandle;
                    break;
                }
            }

            if(mSecInputBufCount == i )
            {
                MTK_OMX_LOGE("[ERROR] aBuffer(0x%08X)Cannot find secure handle, LINE: %d\n", aInputBuf, __LINE__);
                return OMX_FALSE;
            }
        }
        mRingbuf.rBase.u4Size = aInputSize;
        mRingbuf.rBase.u4VA = 0;
        MTK_OMX_LOGD("[INFO] GetM4UFrameandBitstreamBuffer mRingbuf.rSecMemHandle(0x%08X), mRingbuf.rBase.u4Size(%d)", mRingbuf.rSecMemHandle, mRingbuf.rBase.u4Size);
    }
    else
    {
        VBufInfo bufInfo;
        int ret = mInputMVAMgr->getOmxInfoFromVA((void *)aInputBuf, &bufInfo);
        if (ret < 0)
        {
            MTK_OMX_LOGE("[ERROR][ION][Input][VideoDecode],line %d\n", __LINE__);
            return OMX_FALSE;
        }
        mRingbuf.rBase.u4VA = bufInfo.u4VA;
        mRingbuf.rBase.u4PA = bufInfo.u4PA;
        mRingbuf.rBase.u4Size = aInputSize;
        mRingbuf.rBase.hMemHandle = bufInfo.ionBufHndl;
        mRingbuf.u4Read = bufInfo.u4VA;
        mRingbuf.u4Write = bufInfo.u4VA + aInputSize;
    }

    /*
        MTK_OMX_LOGD("[M4U] mRingbuf.rBase.u4VA = 0x%x, mRingbuf.rBase.u4PA = 0x%x, mRingbuf.rBase.u4Size = %d, mRingbuf.u4Read = 0x%x, mRingbuf.u4Write = 0x%x",
            mRingbuf.rBase.u4VA, mRingbuf.rBase.u4PA, mRingbuf.rBase.u4Size, mRingbuf.u4Read, mRingbuf.u4Write);
        */
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::SetupMetaSecureHandle(OMX_BUFFERHEADERTYPE *pBufHdr)
{
    OMX_U32 graphicBufHandle = 0;

    if (OMX_FALSE == GetMetaHandleFromOmxHeader(pBufHdr, &graphicBufHandle))
    {
        MTK_OMX_LOGE("SetupMetaSecureHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE("SetupMetaSecureHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    int i = 0;
    for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        if ((void *)graphicBufHandle == mSecFrmBufInfo[i].pNativeHandle)
        {
            // found handle
            return OMX_TRUE;
        }
    }

    if (i == mOutputPortDef.nBufferCountActual)
    {
        // cannot found handle, create a new entry
        MTK_OMX_LOGD("SetupMetaSecureHandle cannot found handle, create a new entry, LINE:%d", __LINE__);
        int j = 0;
        int secure_buffer_handle;
        size_t bufferSize;

        for (j = 0; j < mOutputPortDef.nBufferCountActual; j++)
        {
            if ((void *)0xffffffff == mSecFrmBufInfo[j].pNativeHandle)
            {
                // found an empty slot
                mSecFrmBufInfo[j].pNativeHandle = (void *)graphicBufHandle;
                buffer_handle_t _handle = (buffer_handle_t)mSecFrmBufInfo[j].pNativeHandle;
#if SECURE_OUTPUT_USE_ION
                int ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, (void *)pBufHdr);

                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR]newOmxMVAwithHndl() failed");
                }

                VBufInfo BufInfo;
                if (mOutputMVAMgr->getOmxInfoFromHndl((void *)_handle, &BufInfo) < 0)
                {
                    MTK_OMX_LOGE("[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
                    return OMX_FALSE;
                }

                //get secure handle from ion
                secure_buffer_handle = BufInfo.u4PA;
                bufferSize = BufInfo.u4BuffSize;
                MTK_OMX_LOGD("child Physical address = 0x%x, len = %d\n", BufInfo.u4PA, BufInfo.u4BuffSize);

#else
                //===========retrieve secure handle===================//
                {
                    int buffer_type = 0;
                    GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;

                    err = gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &bufferSize);
                    if (err != GRALLOC_EXTRA_OK)   /* something wrong */
                    {
                        MTK_OMX_LOGE("[ERROR] gralloc_extra_query GRALLOC_EXTRA_GET_ALLOC_SIZE failed, LINE: %d", __LINE__);
                        return OMX_FALSE;
                    }
                    //gralloc_extra_getSecureBuffer(_handle, &buffer_type, &secure_buffer_handle);
                    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &secure_buffer_handle);
                }
#endif
                mSecFrmBufInfo[j].u4BuffId = secure_buffer_handle;
                mSecFrmBufInfo[j].u4SecHandle = secure_buffer_handle;
                mSecFrmBufInfo[j].u4BuffSize = bufferSize;
                MTK_OMX_LOGD("@@ SetupMetaSecureHandle(), _handle(0x%08X), secure_buffer_handle(0x%08X), bufferSize = (0x%08X)",
                     _handle, secure_buffer_handle, bufferSize);

                return OMX_TRUE;
            }
        }

        if (j == mOutputPortDef.nBufferCountActual)
        {
            MTK_OMX_LOGD("SetupMetaSecureHandle ERROR: Cannot found empty entry");
            return OMX_FALSE;
        }
    }

    MTK_OMX_LOGD("SetupMetaSecureHandle ERROR: LINE:%d", __LINE__);
    return OMX_FALSE;
}
//HEVC.SEC.M0
OMX_BOOL MtkOmxVdec::SetupMetaSecureHandleAndGetFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *pBuffer)
{
    // check if we had this handle first
    OMX_U32 graphicBufHandle = 0;
    if (OMX_FALSE == GetMetaHandleFromBufferPtr(pBuffer, &graphicBufHandle))
    {
        MTK_OMX_LOGE("SetupMetaSecureHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE("SetupMetaSecureHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    int i = 0;
    for (i = 0 ; i < mOutputPortDef.nBufferCountActual ; i++)
    {
        if ((void *)graphicBufHandle == mSecFrmBufInfo[i].pNativeHandle)
        {
            // found handle, setup VA/PA from table
            aFrame->rBaseAddr.u4VA = 0x200;
            aFrame->rBaseAddr.u4PA = 0x200;
            aFrame->rSecMemHandle = mSecFrmBufInfo[i].u4SecHandle;
            //MTK_OMX_LOGE("SetupMetaSecureHandleAndGetFrame@@ aFrame->rSecMemHandle(0x%08X)", aFrame->rSecMemHandle);
            return OMX_TRUE;
        }
    }

    if (i == mOutputPortDef.nBufferCountActual)
    {
        MTK_OMX_LOGE("SetupMetaSecureHandleAndGetFrame failed, cannot found handle, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    return OMX_TRUE;

}

OMX_BOOL MtkOmxVdec::GetM4UFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *aOutputBuf)
{
    VAL_UINT32_T u4x, u4y;

#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        // TODO: Extract buffer VA/PA from graphic buffer handle
        if(mIsSecureInst == OMX_TRUE) //HEVC.SEC.M0
        {
            return SetupMetaSecureHandleAndGetFrame(aFrame, aOutputBuf);
        }
        else
        {
            return SetupMetaIonHandleAndGetFrame(aFrame, aOutputBuf);
        }
    }
#endif

    if (aFrame == NULL)
    {
        return OMX_FALSE;
    }

    if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo bufInfo;
        int ret = mOutputMVAMgr->getOmxInfoFromVA((void *)aOutputBuf, &bufInfo);
        if (ret < 0)
        {
            MTK_OMX_LOGE("[ERROR][ION][Output][VideoDecode]Can't find Frm in MVAMgr,line %d\n", __LINE__);
            return OMX_FALSE;

        }
        aFrame->rBaseAddr.u4VA = bufInfo.u4VA;
        aFrame->rBaseAddr.u4PA = bufInfo.u4PA;
        aFrame->rBaseAddr.hMemHandle = bufInfo.ionBufHndl;
        aFrame->rBaseAddr.u4Size = bufInfo.u4BuffSize;

        if (OMX_TRUE == mIsSecureInst)
        {
            aFrame->rSecMemHandle = bufInfo.secure_handle;
            //aFrame->rFrameBufVaShareHandle = mIonBufferInfo[u4y].va_share_handle;
            aFrame->rFrameBufVaShareHandle = 0;
            MTK_OMX_LOGE("@@ aFrame->rSecMemHandle(0x%08X), aFrame->rFrameBufVaShareHandle(0x%08X)", aFrame->rSecMemHandle, aFrame->rFrameBufVaShareHandle);
        }
        // MTK_OMX_LOGD("[ION] frame->rBaseAddr.u4VA = 0x%x, frame->rBaseAddr.u4PA = 0x%x", aFrame->rBaseAddr.u4VA, aFrame->rBaseAddr.u4PA);
    }
    else if (OMX_TRUE == mIsSecureInst)
    {
        for (u4y = 0; u4y < mSecFrmBufCount; u4y++)
        {
            //MTK_OMX_LOGE ("@@ aOutputBuf(0x%08X), mSecFrmBufInfo[%d].u4BuffId(0x%08X)", aOutputBuf, u4y, mSecFrmBufInfo[u4y].u4BuffId);
            if (mSecFrmBufInfo[u4y].u4BuffId == (VAL_UINT32_T)aOutputBuf)
            {
                aFrame->rBaseAddr.u4VA = 0x200;
                aFrame->rBaseAddr.u4PA = 0x200;
                aFrame->rSecMemHandle = mSecFrmBufInfo[u4y].u4SecHandle;
                MTK_OMX_LOGE("@@ aFrame->rSecMemHandle(0x%08X)", aFrame->rSecMemHandle);
                break;
            }
        }

        if (u4y == mSecFrmBufCount)
        {
            MTK_OMX_LOGE("[ERROR][SECURE][output][VideoDecode]\n");
            return OMX_FALSE;
        }
    }
    else  // M4U
    {
        VBufInfo bufInfo;
        int ret = mOutputMVAMgr->getOmxInfoFromVA((void *)aOutputBuf, &bufInfo);
        if (ret < 0)
        {
            MTK_OMX_LOGE("[ERROR][M4U][Output][VideoDecode]Can't find Frm in MVAMgr,line %d\n", __LINE__);
            return OMX_FALSE;
        }
        aFrame->rBaseAddr.u4VA = bufInfo.u4VA;
        aFrame->rBaseAddr.u4PA = bufInfo.u4PA;
    }

    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::ConfigIonBuffer(int ion_fd, int handle)
{

    struct ion_mm_data mm_data;
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VDEC);
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &mm_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot configure buffer");
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

VAL_UINT32_T MtkOmxVdec::GetIonPhysicalAddress(int ion_fd, int handle)
{
    // query physical address
    struct ion_sys_data sys_data;
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
        return 0;
    }
    return (VAL_UINT32_T)sys_data.get_phys_param.phy_addr;
}

void thread_exit_handler(int sig)
{
    ALOGE("@@ this signal is %d, tid=%d", sig, gettid());
    pthread_exit(0);
}

void *MtkOmxVdecDecodeThread(void *pData)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVdecDecodeThread", 0, 0, 0);
#endif

    // register signal handler
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    sigaction(SIGUSR1, &actions, NULL);

    ALOGD("[0x%08x] MtkOmxVdecDecodeThread created pVdec=0x%08X, tid=%d", pVdec, (unsigned int)pVdec, gettid());

    pVdec->mVdecDecThreadTid = gettid();

    while (1)
    {
        if (pVdec->mOmxVdecLogEnable)
            ALOGD("[0x%08x] ## Wait to decode (%d) (input: %d, output: %d) ", pVdec, get_sem_value(&pVdec->mDecodeSem), pVdec->mEmptyThisBufQ.size() , pVdec->mFillThisBufQ.size());

        pVdec->mInputBuffInuse = OMX_FALSE;

        LOCK_T(pVdec->mWaitDecSemLock);
        WAIT_T(pVdec->mDecodeSem);
        UNLOCK(pVdec->mWaitDecSemLock);

        if (OMX_FALSE == pVdec->mIsComponentAlive)
        {
            break;
        }

        if (pVdec->mDecodeStarted == OMX_FALSE)
        {
            ALOGD("[0x%08x] Wait for decode start.....", pVdec);
            SLEEP_MS(2);
            continue;
        }

        LOCK_T(pVdec->mDecodeLock);

        if (pVdec->mPortReconfigInProgress)
        {
            if (pVdec->mOmxVdecLogEnable)
                ALOGD("[0x%08x] MtkOmxVdecDecodeThread cannot decode when port re-config is in progress", pVdec);
            pVdec->mNumSemaphoreCountForOutput++;
            UNLOCK(pVdec->mDecodeLock);
            continue;
        }

        if (pVdec->CheckBufferAvailability() == OMX_FALSE)
        {
            if (pVdec->mOmxVdecLogEnable)
                ALOGD("[0x%08x] No input avail...", pVdec);
            UNLOCK(pVdec->mDecodeLock);
            SLEEP_MS(1);
            continue;
        }

        pVdec->mInputBuffInuse = OMX_TRUE;

        // dequeue an input buffer
        int input_idx = pVdec->DequeueInputBuffer();

        // dequeue an output buffer
        //int output_idx = pVdec->DequeueOutputBuffer();

        if ((input_idx < 0))// || (output_idx < 0))
        {
            sched_yield();
            ALOGD("[0x%08x] No input buffer...", pVdec);
            UNLOCK(pVdec->mDecodeLock);
            continue;
        }

        //MTK_OMX_LOGD ("Decode [%d, %d] (0x%08X, 0x%08X)", input_idx, output_idx, (unsigned int)pVdec->mInputBufferHdrs[input_idx], (unsigned int)pVdec->mOutputBufferHdrs[output_idx]);

        // send the input/output buffers to decoder
        if (pVdec->DecodeVideoEx(pVdec->mInputBufferHdrs[input_idx]) == OMX_FALSE)
        {
            pVdec->mErrorInDecoding++;
            ALOGE("[0x%08x] something wrong when decoding....", pVdec);
            UNLOCK(pVdec->mDecodeLock);
            break;
        }

#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        //pVdec->PriorityAdjustment();
#endif

        UNLOCK(pVdec->mDecodeLock);

    }
    pVdec->mInputBuffInuse = OMX_FALSE;

    ALOGD("[0x%08x] MtkOmxVdecDecodeThread terminated pVdec=0x%08X", pVdec, (unsigned int)pVdec);
    pVdec->mVdecDecodeThread = NULL;
    return NULL;
}


void *MtkOmxVdecThread(void *pData)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVdecThread", 0, 0, 0);
#endif

    ALOGD("[0x%08x] MtkOmxVdecThread created pVdec=0x%08X, tid=%d", pVdec, (unsigned int)pVdec, gettid());

    pVdec->mVdecThreadTid = gettid();

    int status;
    ssize_t ret;

    OMX_COMMANDTYPE cmd;
    OMX_U32 cmdCat;
    OMX_U32 nParam1;
    OMX_PTR pCmdData;

    unsigned int buffer_type;

    struct pollfd PollFd;
    PollFd.fd = pVdec->mCmdPipe[MTK_OMX_PIPE_ID_READ];
    PollFd.events = POLLIN;

    while (1)
    {
        status = poll(&PollFd, 1, -1);
        // WaitForSingleObject
        if (-1 == status)
        {
            ALOGE("[0x%08x] poll error %d (%s), fd:%d", pVdec, errno, strerror(errno), pVdec->mCmdPipe[MTK_OMX_PIPE_ID_READ]);
            //dump fd
            ALOGE("[0x%08x] pipe: %d %d", pVdec, pVdec->mCmdPipe[MTK_OMX_PIPE_ID_READ], pVdec->mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);
            pVdec->FdDebugDump();
            if (errno == 4) // error 4 (Interrupted system call)
            {
            }
            else
            {
                abort();
            }
        }
        else if (0 == status)   // timeout
        {
        }
        else
        {
            if (PollFd.revents & POLLIN)
            {
                READ_PIPE(cmdCat, pVdec->mCmdPipe);
                if (cmdCat == MTK_OMX_GENERAL_COMMAND)
                {
                    READ_PIPE(cmd, pVdec->mCmdPipe);
                    READ_PIPE(nParam1, pVdec->mCmdPipe);
                    if (pVdec->mOmxVdecLogEnable)
                        ALOGD("[0x%08x] # Got general command (%s)", pVdec, CommandToString(cmd));
                    switch (cmd)
                    {
                        case OMX_CommandStateSet:
                            pVdec->HandleStateSet(nParam1);
                            break;

                        case OMX_CommandPortEnable:
                            pVdec->HandlePortEnable(nParam1);
                            break;

                        case OMX_CommandPortDisable:
                            pVdec->HandlePortDisable(nParam1);
                            break;

                        case OMX_CommandFlush:
                            pVdec->HandlePortFlush(nParam1);
                            break;

                        case OMX_CommandMarkBuffer:
                            READ_PIPE(pCmdData, pVdec->mCmdPipe);
                            pVdec->HandleMarkBuffer(nParam1, pCmdData);

                        default:
                            ALOGE("[0x%08x] Error unhandled command", pVdec);
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_BUFFER_COMMAND)
                {
                    OMX_BUFFERHEADERTYPE *pBufHead;
                    READ_PIPE(buffer_type, pVdec->mCmdPipe);
                    READ_PIPE(pBufHead, pVdec->mCmdPipe);
                    switch (buffer_type)
                    {
                        case MTK_OMX_EMPTY_THIS_BUFFER_TYPE:
                            //MTK_OMX_LOGD ("## EmptyThisBuffer pBufHead(0x%08X)", pBufHead);
                            //handle input buffer from IL client
                            pVdec->HandleEmptyThisBuffer(pBufHead);
                            break;
                        case MTK_OMX_FILL_THIS_BUFFER_TYPE:
                            //MTK_OMX_LOGD ("## FillThisBuffer pBufHead(0x%08X)", pBufHead);
                            // handle output buffer from IL client
                            pVdec->HandleFillThisBuffer(pBufHead);
                            break;
#if 0
                        case MTK_OMX_EMPTY_BUFFER_DONE_TYPE:
                            //MTK_OMX_LOGD ("## EmptyBufferDone pBufHead(0x%08X)", pBufHead);
                            // TODO: handle return input buffer
                            pVdec->HandleEmptyBufferDone(pBufHead);
                            break;
                        case MTK_OMX _FILL_BUFFER_DONE_TYPE:
                            //MTK_OMX_LOGD ("## FillBufferDone pBufHead(0x%08X)", pBufHead);
                            // TODO: handle return output buffer
                            pVdec->HandleFillBufferDone(pBufHead);
                            break;
#endif
                        default:
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_STOP_COMMAND)
                {
                    // terminate
                    break;
                }
            }
            else
            {
                ALOGE("[0x%08x] FD: %d %d, Poll get unsupported event:0x%x", pVdec, pVdec->mCmdPipe[0], pVdec->mCmdPipe[1], PollFd.revents);
                abort();
            }
        }

    }

EXIT:
    ALOGD("[0x%08x] MtkOmxVdecThread terminated", pVdec);
    pVdec->mVdecThread = NULL;
    return NULL;
}

void *MtkOmxVdecConvertThread(void *pData)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVdecConvertThread", 0, 0, 0);
#endif

    ALOGD("[0x%08x] MtkOmxVdecConvertThread created pVdec=0x%08X, tid=%d", pVdec, (unsigned int)pVdec, gettid());

    pVdec->mVdecConvertThreadTid = gettid();

    while (1)
    {
        WAIT(pVdec->mConvertSem);

        if (OMX_FALSE == pVdec->mIsComponentAlive)
        {
            break;
        }

        Mutex::Autolock autolock(pVdec->mFlushOrConvertLock);
        {
            pVdec->HandleColorConvertForFillBufferDone(MTK_OMX_OUTPUT_PORT, OMX_FALSE);

            int size_of_SrcQ = pVdec->mBufColorConvertSrcQ.size();
            int size_of_DstQ = pVdec->mBufColorConvertDstQ.size();
            //prepare additional output buffer in one of src/dst situation
            if ((0 != size_of_SrcQ) || (0 != size_of_DstQ))
            {
                int mReturnIndex = -1;

                mReturnIndex = pVdec->PrepareAvaliableColorConvertBuffer(mReturnIndex, OMX_TRUE);

                if (0 < mReturnIndex)
                {
                    ALOGD("[0x%08x] PrepareAvaliableColorConvertBuffer mReturnIndex %d", pVdec, mReturnIndex);
                }
            }
        }
    }

    ALOGD("[0x%08x] MtkOmxVdecConvertThread terminated", pVdec);
    pVdec->mVdecConvertThread = NULL;
    return NULL;
}
//#ifdef MTK_CLEARMOTION_SUPPORT
OMX_BUFFERHEADERTYPE *MtkMJC_GetOutputFrame(OMX_IN OMX_HANDLETYPE hComponent)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    return pVdec->MJCPPBufQRemove();
}

OMX_BUFFERHEADERTYPE *MtkMJC_GetInputFrame(OMX_IN OMX_HANDLETYPE hComponent)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    return pVdec->MJCVdoBufQRemove();
}

void MtkMJC_PutDispFrame(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    pVdec->MJCDispBufQInsert(pBuffHdr);

    while (1)
    {
        OMX_BUFFERHEADERTYPE *pDispBuffHdr;
        pDispBuffHdr = pVdec->MJCDispBufQRemove();

        if (pDispBuffHdr != NULL)
        {
            pVdec->MJCHandleFillBufferDone(pDispBuffHdr);
        }
        else
        {
            break;
        }
    }
}

void MtkMJC_SetRefFrame(OMX_IN OMX_HANDLETYPE hComponent, MJC_INT_BUF_INFO *pMJCBufInfo)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    if (NULL != pMJCBufInfo)
    {
       pVdec->ProtectedIncRef(BUFFER_OWNER_MJC, pMJCBufInfo->u4VA, pVdec->mIsSecureInst);
       pVdec->MJCRefBufQInsert(pMJCBufInfo);
    }
    else
    {
       ALOGE("[ERROR] MtkMJC_SetRefFrame pMJCBufInfo is NULL");
       CHECK(0);
    }
}

void MtkMJC_ClearRefFrame(OMX_IN OMX_HANDLETYPE hComponent, MJC_INT_BUF_INFO *pMJCBufInfo)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    if (NULL != pMJCBufInfo)
    {
       pVdec->ProtectedDecRef(BUFFER_OWNER_MJC, pMJCBufInfo->u4VA, pVdec->mIsSecureInst);
       pVdec->MJCRefBufQRemove(pMJCBufInfo);
    }
    else
    {
       ALOGE("[ERROR] MtkMJC_ClearRefFrame pMJCBufInfo is NULL");
       CHECK(0);
    }
}

void *MtkMJC_GetBufGraphicHndl(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    return pVdec->MJCGetBufGraphicHndl(pBuffHdr);
}

MJC_INT_BUF_INFO MtkMJC_GetBufInfo(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    return pVdec->MJCGetBufferInfo(pBuffHdr);
}

void *MtkMJC_GetBufVA(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    return pVdec->MJCGetBufVA(pBuffHdr);
}

void *MtkMJC_GetBufPA(OMX_IN OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    return pVdec->MJCGetBufPA(pBuffHdr);
}
void MtkMJC_EventHandler(OMX_HANDLETYPE hComponent, const MJC_EVENT_TYPE eEventtype, void *param)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)hComponent;
    pVdec->MJCEventHandler(eEventtype, param);
}

//#endif

MtkOmxVdec::MtkOmxVdec()
{
    MTK_OMX_LOGU("MtkOmxVdec::MtkOmxVdec this= 0x%08X", (unsigned int)this);

    if (VDEC_DRV_MRESULT_FAIL == eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_CHIP_NAME, VAL_NULL, &mChipName))
    {
        MTK_OMX_LOGE("[ERROR][MtkOmxVdec] Cannot get decoder property, VDEC_DRV_QUERY_TYPE_CHIP_NAME");
    }
    CheckLogEnable();

    MTK_OMX_LOGD("OmxVdecLogEnable = %d, bOmxVdecPerfLogEnable = %d\n", mOmxVdecLogEnable, mOmxVdecPerfLogEnable);

    // check device\mediatek\common\kernel-headers\Val_types_public.h
    MTK_OMX_LOGUD("VAL_CHIP_ID: %d", mChipName);

    MTK_OMX_MEMSET(&mCompHandle, 0x00, sizeof(OMX_COMPONENTTYPE));
    mCompHandle.nSize = sizeof(OMX_COMPONENTTYPE);
    mCompHandle.pComponentPrivate = this;
    mState = OMX_StateInvalid;

    mInputBufferHdrs = NULL;
    mOutputBufferHdrs = NULL;
    mInputBufferPopulatedCnt = 0;
    mOutputBufferPopulatedCnt = 0;
    mPendingStatus = 0;
    mDecodeStarted = OMX_FALSE;
    mPortReconfigInProgress = OMX_FALSE;

    mNumPendingInput = 0;
    mNumPendingOutput = 0;

    mErrorInDecoding = 0;

    mFramesDecoded = 0;
    mTotalDecodeTime = 0;

    mCodecId = MTK_VDEC_CODEC_ID_INVALID;
    mCurrentSchedPolicy = SCHED_OTHER;

    m_fnMJCCreateInstance = NULL;
    m_fnMJCDestroyInstance = NULL;
    mpMJCLib   = NULL;
    mpMJC      = NULL;
    mMJCEnable = OMX_FALSE; //Initial value

    int i4BuildWithMJC = 0;
    mMJCFakeEngine = OMX_FALSE;

    MTK_OMX_LOGUD("Build With MJC:%d. Fake Engine(%d)", i4BuildWithMJC, mMJCFakeEngine);

    if (i4BuildWithMJC != 0 || mMJCFakeEngine == OMX_TRUE)
    {
        MJCCreateInstance();
    }

    mSucessfullyRegisterMJC = OMX_FALSE;

    INIT_MUTEX(mCmdQLock);
    INIT_MUTEX(mFillThisConvertBufQLock);
    INIT_MUTEX(mEmptyThisBufQLock);
    INIT_MUTEX(mFillThisBufQLock);
    INIT_MUTEX(mDecodeLock);
    INIT_MUTEX(mWaitDecSemLock);
    INIT_MUTEX(mFillUsedLock);

//#ifdef MTK_CLEARMOTION_SUPPORT
    INIT_MUTEX(mMJCVdoBufQLock);
    INIT_MUTEX(mMJCFlushBufQLock);
    INIT_MUTEX(mMJCDispBufQLock);
    INIT_MUTEX(mMJCRefBufQLock);
    INIT_MUTEX(mMJCPPBufQLock);
//#endif

    INIT_SEMAPHORE(mInPortAllocDoneSem);
    INIT_SEMAPHORE(mOutPortAllocDoneSem);
    INIT_SEMAPHORE(mInPortFreeDoneSem);
    INIT_SEMAPHORE(mOutPortFreeDoneSem);
    INIT_SEMAPHORE(mDecodeSem);
    INIT_SEMAPHORE(mOutputBufferSem);

//#ifdef MTK_CLEARMOTION_SUPPORT
    INIT_SEMAPHORE(mPPBufQManagementSem);
    INIT_SEMAPHORE(mMJCFlushBufQDoneSem);
    INIT_SEMAPHORE(mMJCVdoBufQDrainedSem);
//#endif

    INIT_SEMAPHORE(mConvertSem);

    mDecoderInitCompleteFlag = OMX_FALSE;
    mDrvHandle = NULL;
    mBitStreamBufferAllocated = OMX_FALSE;
    mBitStreamBufferVa = 0;
    mBitStreamBufferPa = 0;
    mFrameBuf = NULL;
    mFrameBufInt = NULL;
    mFrameBufSize = 0;
    mInputBuf = NULL;
    mNumFreeAvailOutput = 0;
    mNumAllDispAvailOutput = 0;
    mNumNotDispAvailOutput = 0;
    mNumSemaphoreCountForInput = 0;
    mNumSemaphoreCountForOutput = 0;
    mInterlaceChkComplete = OMX_FALSE;
    mIsInterlacing = OMX_FALSE;
    mb16xSlowMotionMode = OMX_FALSE;

    //default is 1, 2 for crossMount that camera HAL return buffer N after receive N+1
    mMaxColorConvertOutputBufferCnt = MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
    mCrossMountSupportOn = OMX_FALSE;
    VAL_CHAR_T mMCCValue[PROPERTY_VALUE_MAX];
    if (property_get("ro.vendor.mtk_crossmount_support", mMCCValue, NULL))
    {
        int mCCvalue = atoi(mMCCValue);

        if (mCCvalue)
        {
            VAL_CHAR_T mMCCMaxValue[PROPERTY_VALUE_MAX];
            //mCrossMountSupportOn = OMX_TRUE;  //enable via setParameter now
            if (property_get("ro.vendor.mtk_crossmount.maxcount", mMCCMaxValue, NULL))
            {
                int mCCMaxvalue = atoi(mMCCMaxValue);
                mMaxColorConvertOutputBufferCnt = mCCMaxvalue;
                if( mMaxColorConvertOutputBufferCnt > (MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT/2) )
                {
                    mMaxColorConvertOutputBufferCnt = (MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT/2);
                }
            }
            else
            {
                mMaxColorConvertOutputBufferCnt = MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
            }
            MTK_OMX_LOGD("[CM] enable CrossMunt and config MaxCC buffer count in OMX component : %d", mMaxColorConvertOutputBufferCnt);
        }
        else
        {
            MTK_OMX_LOGD("keep orignal buffer count : %d", mMaxColorConvertOutputBufferCnt);
        }
    }
    else
    {
        MTK_OMX_LOGD("keep orignal buffer count : %d", mMaxColorConvertOutputBufferCnt);
    }

    TOTAL_MJC_BUFFER_CNT = 0;

    TOTAL_MJC_BUFFER_CNT = MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT + MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT;
    // For Scaler ClearMotion +
    for (mScalerBufferCount = 0; mScalerBufferCount < MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT; mScalerBufferCount++)
    {
        mScalerBufferInfo[mScalerBufferCount].u4VA = 0xffffffff;
        mScalerBufferInfo[mScalerBufferCount].u4PA = 0xffffffff;
        mScalerBufferInfo[mScalerBufferCount].u4Size = 0xffffffff;
        mScalerBufferInfo[mScalerBufferCount].u4Hdr = 0xffffffff;
    }
    // For Scaler ClearMotion -

    for (mMJCBufferCount = 0; mMJCBufferCount < MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT; mMJCBufferCount++)
    {
        mMJCBufferInfo[mMJCBufferCount].u4VA = 0xffffffff;
        mMJCBufferInfo[mMJCBufferCount].u4PA = 0xffffffff;
        mMJCBufferInfo[mMJCBufferCount].u4Size = 0xffffffff;
        mMJCBufferInfo[mMJCBufferCount].u4Hdr = 0xffffffff;
    }

    for (mMJCBufferCount = 0; mMJCBufferCount < MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM; mMJCBufferCount++)
    {
        mMJCVdoBufQ[mMJCBufferCount] = NULL;
    }

    for (mMJCBufferCount = 0; mMJCBufferCount < MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM; mMJCBufferCount++)
    {
        mMJCDispBufQ[mMJCBufferCount] = NULL;
    }

    for (mMJCBufferCount = 0; mMJCBufferCount < MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM; mMJCBufferCount++)
    {
        mMJCRefBufQ[mMJCBufferCount] = 0xffffffff;
    }

    for (mMJCBufferCount = 0; mMJCBufferCount < MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM; mMJCBufferCount++)
    {
        mMJCPPBufQ[mMJCBufferCount] = NULL;
    }

    // For Scaler ClearMotion +
    mScalerBufferCount = 0;
    mMJCScalerByPassFlag = OMX_FALSE;
    mOriFrameWidth = 0;
    mOriFrameHeight = 0;
    // For Scaler ClearMotion -

    mMJCBufferCount = 0;
    mMJCRefBufCount = 0;
    mMJCPPBufCount = 0;
    mMJCVdoBufCount = 0;
    mMJCLog = VAL_FALSE;

    mOutputBufferHdrsCnt = 0;
    VAL_CHAR_T mMJCValue[PROPERTY_VALUE_MAX];

    mMJCLog = VAL_FALSE;

    mMJCFlushBufQ = VAL_FALSE;
    mMJCCreated = VAL_FALSE;

    VAL_CHAR_T mDIValue[PROPERTY_VALUE_MAX];

    property_get("vendor.mtk_deinterlace_support", mDIValue, "1");
    mDeInterlaceEnable = (VAL_BOOL_T) atoi(mDIValue);

#if (ANDROID_VER >= ANDROID_KK)
    mAdaptivePlayback = VAL_TRUE;

    VAL_CHAR_T mAdaptivePlaybackValue[PROPERTY_VALUE_MAX];

    //remove 72 which config 0 previously
    property_get("vendor.mtk.omxvdec.ad.vp", mAdaptivePlaybackValue, "1");

    mAdaptivePlayback = (VAL_BOOL_T) atoi(mAdaptivePlaybackValue);
/*
    if (mAdaptivePlayback)
    {
        MTK_OMX_LOGU("Adaptive Playback Enable!");
    }
    else
    {
        MTK_OMX_LOGU("Adaptive Playback Disable!");
    }
*/
#endif

    mRealCallBackFillBufferDone = OMX_TRUE;
//#if  (defined MTK_CLEARMOTION_SUPPORT)
    if (mMJCEnable == OMX_TRUE)
    {
    mRealCallBackFillBufferDone = OMX_FALSE;
    }
//#else
    //mRealCallBackFillBufferDone = OMX_TRUE;
//#endif

    mM4UBufferHandle = VAL_NULL;
    mM4UBufferCount = 0;

    mDPBSize = 0;
    mNeedMoreOutputBuf = OMX_FALSE;
    mSeqInfoCompleteFlag = OMX_FALSE;
    mMJCReconfigFlag = OMX_FALSE;
    mUseClearMotion = OMX_FALSE;
    mInputBuffInuse = OMX_FALSE;

    mVdecConvertThread = NULL;
    mVdecConvertThreadCreated = OMX_FALSE;
    mVdecConvertThreadTid = 0;

    MTK_OMX_MEMSET(&mQInfoOut, 0, sizeof(mQInfoOut));

    mCodecProperty = 0;
    mEverCallback = OMX_FALSE;
    mpCurrInput = NULL;
    mCBTime = 0;

    eVideoInitMVA((VAL_VOID_T **)&mM4UBufferHandle);

    mIonInputBufferCount = 0;
    mIonOutputBufferCount = 0;
    mInputUseION = OMX_FALSE;
    mOutputUseION = OMX_FALSE;
    mIonDevFd = -1;

    mIsClientLocally = OMX_TRUE;
    mIsFromGralloc = OMX_FALSE;

    for (mSecFrmBufCount = 0; mSecFrmBufCount < VIDEO_ION_MAX_BUFFER; mSecFrmBufCount++)
    {
        mSecFrmBufInfo[mSecFrmBufCount].u4BuffId = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].u4BuffHdr = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].u4BuffSize = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].u4SecHandle = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].pNativeHandle = (void *)0xffffffff;
    }
    for (mSecFrmBufCount = 0; mSecFrmBufCount < VIDEO_ION_MAX_BUFFER; mSecFrmBufCount++)
    {
        mSecInputBufInfo[mSecFrmBufCount].u4IonShareFd = 0xffffffff;
        mSecInputBufInfo[mSecFrmBufCount].pIonHandle = 0xffffffff;
        mSecInputBufInfo[mSecFrmBufCount].pNativeHandle = (void *)0xffffffff;
        mSecInputBufInfo[mSecFrmBufCount].u4SecHandle = 0xffffffff;
    }
    mSecFrmBufCount = 0;
    mSecInputBufCount = 0;
    mIsSecUsingNativeHandle = OMX_FALSE;
    mPropFlags = 0;
    mIsSecTlcAllocOutput = OMX_FALSE;

#if PROFILING
    /*
        fpVdoProfiling = fopen("//data//VIDEO_DECODE_PROFILING.txt", "ab");

        if (fpVdoProfiling == VAL_NULL)
        {
            MTK_OMX_LOGE("[ERROR] cannot open VIDEO_PROFILING.txt\n");
        }
    */
#endif

#if defined(DYNAMIC_PRIORITY_ADJUSTMENT)
    mllLastDispTime = 0;
#endif
    mllLastUpdateTime = 0;

    //#ifdef MT6577
    mCodecTidInitialized = OMX_FALSE;
    mNumCodecThreads = 0;
    MTK_OMX_MEMSET(mCodecTids, 0x00, sizeof(pid_t) * 8);
    //#endif

    char value[PROPERTY_VALUE_MAX];
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
    char *delimiter = " ";
    property_get("vendor.mtk.omxvdec.enable.priadj", value, "1");
    char *pch = strtok(value, delimiter);
    bool _enable = atoi(pch);
    if (_enable)
    {
        mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
        pch = strtok(NULL, delimiter);
        if (pch == NULL)
        {
            mPendingOutputThreshold = 5;
        }
        else
        {
            mPendingOutputThreshold = atoi(pch);
        }

        //MTK_OMX_LOGU("Priority Adjustment enabled (mPendingOutputThreshold=%d)!!!", mPendingOutputThreshold);
    }
    property_get("vendor.mtk.omxvdec.enable.priadjts", value, "1");
    pch = strtok(value, delimiter);
    _enable = atoi(pch);
    if (_enable)
    {
        mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
        pch = strtok(NULL, delimiter);
        if (pch == NULL)
        {
            mTimeThreshold = 133000;
        }
        else
        {
            mTimeThreshold = atoi(pch);
        }

        //MTK_OMX_LOGU("Priority Adjustment enabled (mTimeThreshold=%lld)!!!", mTimeThreshold);
    }
    mErrorCount = 0;
#else

#endif
    char value2[PROPERTY_VALUE_MAX];

    property_get("vendor.mtk.omxvdec.dump", value2, "0");
    mDumpOutputFrame = (OMX_BOOL) atoi(value2);

    property_get("vendor.mtk.omxvdec.dumpProfiling", value2, "0");
    mDumpOutputProfling = (OMX_BOOL) atoi(value2);

    property_get("vendor.mtk.omxvdec.output.checksum", value2, "0");
    mOutputChecksum = (OMX_BOOL) atoi(value2);
    property_get("vendor.mtk.omxvdec.output.buf.count", value2, "0");
    mForceOutputBufferCount = atoi(value2);

    property_get("vendor.mtk.dumpCts", value2, "0");
    mDumpCts = (OMX_BOOL) atoi(value2);

    // for VC1
    mFrameTsInterval = 0;
    mCurrentFrameTs = 0;
    mFirstFrameRetrieved = OMX_FALSE;
    mResetFirstFrameTs = OMX_FALSE;
    mCorrectTsFromOMX = OMX_FALSE;

    // for H.264
    iTSIn = 0;
    DisplayTSArray[0] = 0;

#if (ANDROID_VER >= ANDROID_ICS)
    mIsUsingNativeBuffers = OMX_FALSE;
#endif

#if (ANDROID_VER >= ANDROID_KK)
    mStoreMetaDataInBuffers = OMX_FALSE;
    mEnableAdaptivePlayback = OMX_FALSE;
    mMaxWidth = 0;
    mMaxHeight = 0;
    mCropLeft = 0;
    mCropTop = 0;
    mCropWidth = 0;
    mCropHeight = 0;
    mEarlyEOS = OMX_FALSE;
#endif

    // for UI response improvement
    mRRSlidingWindowLength = 15;
    mRRSlidingWindowCnt = mRRSlidingWindowLength;
    mRRSlidingWindowLimit = mRRSlidingWindowLength;
    mRRCntCurWindow = 0;
    mLastCpuIdleTime = 0L;
    mLastSchedClock = 0L;

#if CPP_STL_SUPPORT
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif

#if ANDROID
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif
    FNum = 0;

    mBufColorConvertDstQ.clear();
    mBufColorConvertSrcQ.clear();

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE)
    {
    mUseClearMotion = OMX_FALSE;
    mClearMotionDemoMode = 0;
    }
//#endif
    mMinUndequeuedBufs = MIN_UNDEQUEUED_BUFS;
    mMinUndequeuedBufsDiff = 0;
    mMinUndequeuedBufsFlag = OMX_FALSE;
    mStarvationSize = 0;

    mThumbnailMode = OMX_FALSE;
    mSeekTargetTime = 0;
    mSeekMode = OMX_FALSE;
    mPrepareSeek = OMX_FALSE;

    mConcealLevel = 0;
    mbH263InMPEG4 = OMX_FALSE;
    mEOSFound = OMX_FALSE;
    mEOSTS = -1;
    mFATALError = OMX_FALSE;
    mKernelLogCntChg = OMX_FALSE;
    mFailInitCounter = 0;
    mInputZero = OMX_FALSE;
    mIs4kSwAvc = OMX_FALSE;
    mIsVp9Sw4k = OMX_FALSE;

    mCoreGlobal = NULL;
    mStreamingMode = OMX_FALSE;
    mACodecColorConvertMode = OMX_FALSE;
    m3DStereoMode = OMX_VIDEO_H264FPA_2D;
#if 0//def MTK S3D SUPPORT
    mFramesDisplay = 0;
    m3DStereoMode = OMX_VIDEO_H264FPA_NONE;
    s3dAsvd = NULL;
    asvdWorkingBuffer = NULL;
#endif
    mAspectRatioWidth = 1;
    mAspectRatioHeight = 1;

    mFrameInterval = 0;
    meDecodeType = VDEC_DRV_DECODER_MTK_HARDWARE;
    mLegacyMode = OMX_FALSE;

    GED_HANDLE OmxGEDHandle = ged_create();
    ged_hint_force_mdp(OmxGEDHandle, -1, &mConvertYV12);
    if (mConvertYV12 == 1)
        MTK_OMX_LOGU("Force convert to YV12 !! hdl %p mConvertYV12 %d", OmxGEDHandle, mConvertYV12);
    else
        mConvertYV12 = 0;
    ged_destroy(OmxGEDHandle);

#if 0 //// FIXME
    mGlobalInstData = NULL;
#endif

    mNoReorderMode = OMX_FALSE;

    mIsSecureInst = OMX_FALSE;
    mbShareableMemoryEnabled = OMX_FALSE;
    mTeeType = NONE_TEE;

    mSkipReferenceCheckMode = OMX_FALSE;
    mLowLatencyDecodeMode = OMX_FALSE;
    mFlushDecoderDoneInPortSettingChange = OMX_TRUE;

    //for supporting SVP NAL size prefix content
    mAssignNALSizeLength = OMX_FALSE;
    MTK_OMX_MEMSET(&mNALSizeLengthInfo, 0x00, sizeof(OMX_VIDEO_CONFIG_NALSIZE));

    mH264SecVdecTlcLib = NULL;
    mH265SecVdecTlcLib = NULL;
    mVP9SecVdecTlcLib  = NULL;
    mTlcHandle = NULL;

    mCommonVdecInHouseLib  = NULL;

    mInputAllocateBuffer = OMX_FALSE;
    mOutputAllocateBuffer = OMX_FALSE;

    mAVSyncTime = -1;
    mResetCurrTime = false;

    mGET_DISP_i = 0;
    mGET_DISP_tmp_frame_addr = 0;

    mbYUV420FlexibleMode = OMX_FALSE;

    mInputFlushALL = OMX_FALSE;

    mIonDevFd = eVideoOpenIonDevFd();
    mInputMVAMgr = new OmxMVAManager("ion", "MtkOmxVdec1", mIonDevFd);
    mOutputMVAMgr = new OmxMVAManager("ion", "MtkOmxVdec2", mIonDevFd);

    mbIs10Bit = VAL_FALSE;
    mIsHorizontalScaninLSB = VAL_FALSE;

    mCmdPipe[0] = -1;
    mCmdPipe[1] = -1;

    mFullSpeedOn = false;

    MTK_OMX_MEMSET(&mrErrorMBInfo, 0x00, sizeof(VDEC_DRV_ERROR_MB_INFO_T));
    mSLI.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mSLI.nSize = sizeof(OMX_CONFIG_SLICE_LOSS_INDICATION);
    mSLI.nTimeStamp = 0;
    mSLI.nSliceCount = 0;
    mContinuousSliceLoss = 0;
    mFIRSent = OMX_FALSE;

    property_get("vendor.mtk.omxvdec.avpf", value, "0");
    mAVPFEnable = (OMX_BOOL) atoi(value);

    property_get("vendor.mtk.omxvdec.avpf.plithres", value, "1");
    if (0 != atoi(value))
    {
        mPLIThres = atoi(value);
        MTK_OMX_LOGD("[debug] mPLIThres : %d", mPLIThres);
    }

    property_get("vendor.mtk.omxvdec.avpf.resendplithres", value, "10");
    if (0 != atoi(value))
    {
        mResendPLIThres = atoi(value);
        MTK_OMX_LOGD("[debug] mResendPLIThres : %d", mResendPLIThres);
    }

    mViLTESupportOn = OMX_FALSE;
    mANW_HWComposer = OMX_FALSE;

    property_get("ro.vendor.mtk_hdr_video_support", value, "0");
    mHDRVideoSupportOn = (OMX_BOOL) atoi(value);
    mIsHDRVideo = OMX_FALSE;
    mNativeWindowHDRInternalConvert = OMX_FALSE;

    property_get("vendor.mtk.omxvdec.fakeHDR", value, "0");
    OMX_BOOL propertyfakeHDR = (OMX_BOOL) atoi(value);
    if (propertyfakeHDR)
    {
        mIsHDRVideo = OMX_TRUE;
    }

    mFirstTimestamp = 0;
    mFPSProfilingCountDown = FPS_PROFILE_COUNT;

    //init color aspects and HDR members
    MTK_OMX_MEMSET(&mDescribeColorAspectsParams, 0x00, sizeof(mDescribeColorAspectsParams));
    MTK_OMX_MEMSET(&mDescribeHDRStaticInfoParams, 0x00, sizeof(mDescribeHDRStaticInfoParams));
    MTK_OMX_MEMSET(&mColorDesc, 0x00, sizeof(mColorDesc));
    MTK_OMX_MEMSET((void *)&mSeqInfo, 0x00, sizeof(VDEC_DRV_SEQINFO_T));

    mSeqInfo.bIsMinWorkBuffer = VAL_FALSE;
    mSeqInfo.bIsMinDpbStrategy = VAL_FALSE;

    mReconfigOutputPortBufferCount = 0;
    mReconfigOutputPortBufferSize = 0;
    mIsResChg = OMX_FALSE;

    mNeedDecodeWhenFlush = false;
    mRestorePortReconfigFlag = false;
    mEnableAVTaskGroup = OMX_FALSE;

    mInitRetryNum = 0;
}


MtkOmxVdec::~MtkOmxVdec()
{
    MTK_OMX_LOGU("~MtkOmxVdec this= 0x%08X", (unsigned int)this);

    eVideoDeInitMVA(mM4UBufferHandle);

#if PROFILING
    //fclose(fpVdoProfiling);
#endif

    if (mInputBufferHdrs)
    {
        MTK_OMX_FREE(mInputBufferHdrs);
    }

    if (mOutputBufferHdrs)
    {
        MTK_OMX_FREE(mOutputBufferHdrs);
    }

    if (mFrameBuf)
    {
        MTK_OMX_FREE(mFrameBuf);
    }

    if (mFrameBufInt)
    {
        MTK_OMX_FREE(mFrameBufInt);
    }
    mFrameBufSize = 0;
    delete mOutputMVAMgr;
    delete mInputMVAMgr;

    if (mInputBuf)
    {
        MTK_OMX_FREE(mInputBuf);
    }

    if (mBitStreamBufferVa)
    {
        mBitStreamBufferVa = 0;
    }

#if 0//def MTK S3D SUPPORT
    if (s3dAsvd)
    {
        s3dAsvd->AsvdReset();
        s3dAsvd->destroyInstance();
        MTK_OMX_FREE(asvdWorkingBuffer);
        DestroyMutex();
    }
#endif

    if (-1 != mIonDevFd)
    {
        eVideoCloseIonDevFd(mIonDevFd);
    }

    DESTROY_MUTEX(mEmptyThisBufQLock);
    DESTROY_MUTEX(mFillThisBufQLock);
    DESTROY_MUTEX(mDecodeLock);
    DESTROY_MUTEX(mWaitDecSemLock);
    DESTROY_MUTEX(mCmdQLock);
    DESTROY_MUTEX(mFillUsedLock);
    DESTROY_MUTEX(mFillThisConvertBufQLock);

//#ifdef MTK_CLEARMOTION_SUPPORT
    DESTROY_MUTEX(mMJCVdoBufQLock);
    DESTROY_MUTEX(mMJCFlushBufQLock);
    DESTROY_MUTEX(mMJCDispBufQLock);
    DESTROY_MUTEX(mMJCRefBufQLock);
    DESTROY_MUTEX(mMJCPPBufQLock);
//#endif

    DESTROY_SEMAPHORE(mInPortAllocDoneSem);
    DESTROY_SEMAPHORE(mOutPortAllocDoneSem);
    DESTROY_SEMAPHORE(mInPortFreeDoneSem);
    DESTROY_SEMAPHORE(mOutPortFreeDoneSem);
    DESTROY_SEMAPHORE(mDecodeSem);
    DESTROY_SEMAPHORE(mOutputBufferSem);

//#ifdef MTK_CLEARMOTION_SUPPORT
    DESTROY_SEMAPHORE(mPPBufQManagementSem);
    DESTROY_SEMAPHORE(mMJCFlushBufQDoneSem);
    DESTROY_SEMAPHORE(mMJCVdoBufQDrainedSem);
//#endif

    DESTROY_SEMAPHORE(mConvertSem);
    //#ifdef MTK_SEC_VIDEO_PATH_SUPPORT
    //#ifdef TRUSTONIC_TEE_SUPPORT
#if 0
    if (NULL != mTlcHandle)
    {
        MtkH264SecVdec_tlcClose_Ptr *pfnMtkH264SecVdec_tlcClose = (MtkH264SecVdec_tlcClose_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_TLC_CLOSE_NAME);
        if (NULL == pfnMtkH264SecVdec_tlcClose)
        {
            MTK_OMX_LOGE("cannot find MtkH264SecVdec_tlcClose, LINE: %d", __LINE__);
        }
        else
        {
            pfnMtkH264SecVdec_tlcClose(mTlcHandle);
            mTlcHandle = NULL;
        }
    }
#endif
    if (NULL != mH264SecVdecTlcLib)
    {
        dlclose(mH264SecVdecTlcLib);
    }
    if (NULL != mH265SecVdecTlcLib)
    {
        dlclose(mH265SecVdecTlcLib);
    }
    if (NULL != mVP9SecVdecTlcLib)
    {
        dlclose(mVP9SecVdecTlcLib);
    }
    if(NULL != mCommonVdecInHouseLib){
        dlclose(mCommonVdecInHouseLib);
        mCommonVdecInHouseLib = NULL;
    }
    //#endif
    //#endif
}

OMX_ERRORTYPE MtkOmxVdec::ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGU("MtkOmxVdec::ComponentInit (%s)", componentName);
    mState = OMX_StateLoaded;
    int ret;
    size_t arraySize = 0;

    InitOMXParams(&mInputPortDef);
    InitOMXParams(&mOutputPortDef);

    mVdecThread = pthread_self();
    mVdecDecodeThread = pthread_self();
    mVdecConvertThread = pthread_self();

    mVdecThreadCreated = false;
    mVdecDecodeThreadCreated = false;
    mVdecConvertThreadCreated = false;

    pthread_attr_t attr;

    if (!strcmp(componentName, MTK_OMX_H263_DECODER))
    {
        if (OMX_FALSE == InitH263Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_H263;
    }
    else if (!strcmp(componentName, MTK_OMX_MPEG4_DECODER))
    {
        if (OMX_FALSE == InitMpeg4Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_MPEG4;
    }
    else if (!strcmp(componentName, MTK_OMX_AVC_DECODER))
    {
        if (OMX_FALSE == InitAvcParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_AVC;
    }
    else if (!strcmp(componentName, MTK_OMX_RV_DECODER))
    {
        if (OMX_FALSE == InitRvParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_RV;
    }
    else if (!strcmp(componentName, MTK_OMX_VC1_DECODER))
    {
        if (OMX_FALSE == InitVc1Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VC1;
    }
    else if (!strcmp(componentName, MTK_OMX_VPX_DECODER))
    {
        if (OMX_FALSE == InitVpxParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VPX;
    }
    else if (!strcmp(componentName, MTK_OMX_VP9_DECODER))
    {
        if (OMX_FALSE == InitVp9Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VP9;
    }
    else if (!strcmp(componentName, MTK_OMX_MPEG2_DECODER))
    {
        if (OMX_FALSE == InitMpeg2Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_MPEG2;
    }
    else if (!strcmp(componentName, MTK_OMX_DIVX_DECODER))
    {
        if (OMX_FALSE == InitDivxParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_DIVX;
    }
    else if (!strcmp(componentName, MTK_OMX_DIVX3_DECODER))
    {
        if (OMX_FALSE == InitDivx3Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_DIVX3;
    }
    else if (!strcmp(componentName, MTK_OMX_XVID_DECODER))
    {
        if (OMX_FALSE == InitXvidParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_XVID;
    }
    else if (!strcmp(componentName, MTK_OMX_S263_DECODER))
    {
        if (OMX_FALSE == InitS263Params())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_S263;
    }
    else if (!strcmp(componentName, MTK_OMX_MJPEG_DECODER))
    {
        if (OMX_FALSE == InitMJpegParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_MJPEG;
    }
    else if (!strcmp(componentName, MTK_OMX_HEVC_DECODER))
    {
        if (OMX_FALSE == InitHEVCParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_HEVC;
    }
    else if (!strcmp(componentName, MTK_OMX_HEVC_SEC_DECODER)) //HEVC.SEC.M0
    {
        if (OMX_FALSE == InitHEVCSecParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_HEVC; // USE the same codec id
    }
    else if (!strcmp(componentName, MTK_OMX_AVC_l3_DECODER))
    {
        if (OMX_FALSE == InitAvcParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_AVC;
    }
    else if (!strcmp(componentName, MTK_OMX_AVC_SEC_DECODER))
    {

        if (OMX_FALSE == InitAvcSecParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_AVC;
    }
    else if (!strcmp(componentName, MTK_OMX_VP9_SEC_DECODER))
    {
        if (OMX_FALSE == InitVp9SecParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VP9;
    }
    else
    {
        MTK_OMX_LOGE("MtkOmxVdec::ComponentInit ERROR: Unknown component name");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    VDEC_DRV_MRESULT_T eResult;

    if (!strcmp(componentName, MTK_OMX_MJPEG_DECODER)) //is MJPEG
    {
        //don't create driver when MJPEG
        mOutputPortFormat.eColorFormat = OMX_COLOR_Format32bitARGB8888;
        mOutputPortDef.format.video.eColorFormat = OMX_COLOR_Format32bitARGB8888;
        mOutputPortDef.format.video.nStride = VDEC_ROUND_32(mOutputPortDef.format.video.nFrameWidth);
        mOutputPortDef.format.video.nSliceHeight = VDEC_ROUND_16(mOutputPortDef.format.video.nSliceHeight);
    }
    else
    {
        eResult = eVDecDrvCreate(&mDrvHandle, GetVdecFormat(mCodecId));
        if (VDEC_DRV_MRESULT_OK != eResult)
        {
            MTK_OMX_LOGE("Error!! Cannot create driver");
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }

        // query output color format and stride and sliceheigt
        MTK_OMX_MEMSET(&mQInfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
        MTK_OMX_MEMSET(&mQInfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
        if (OMX_TRUE == QueryDriverFormat(&mQInfoIn, &mQInfoOut))
        {
            switch (mQInfoOut.ePixelFormat)
            {
                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
                    break;
                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
                    mOutputPortFormat.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                    mOutputPortDef.format.video.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                    break;
                default:
                    break;
            }
            if (needLegacyMode())
            {
                mMaxWidth = mQInfoOut.u4Width;
                mMaxHeight = mQInfoOut.u4Height;

                if(OMX_TRUE == mIsSecureInst)
                {
                    mMaxWidth = 1920;//mQInfoOut.u4Width;
                    mMaxHeight = 1088;//mQInfoOut.u4Height;
                    mLegacyMode = OMX_TRUE;
                }

                MTK_OMX_LOGD("[Legacy Mode]mMaxWidth %d, mMaxHeight %d",mMaxWidth,mMaxHeight);
            }
            mOutputPortDef.format.video.nStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameWidth, mQInfoOut.u4StrideAlign);
            mOutputPortDef.format.video.nSliceHeight = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameHeight, mQInfoOut.u4SliceHeightAlign);
            meDecodeType = mQInfoOut.eDecodeType;
            if (meDecodeType == VDEC_DRV_DECODER_MTK_HARDWARE)
            {
                mPropFlags &= ~MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
                MTK_OMX_LOGU("MtkOmxVdec::SetConfig -> disable priority adjustment");
            }
        }
        else
        {
            MTK_OMX_LOGE("ERROR: query driver format failed: Video Resolution (%d, %d), MAX(%d, %d); Current Video Profile/Level (%x/%d), MAX(%x/%d)", mQInfoIn.u4Width, mQInfoIn.u4Height,\
                mQInfoOut.u4Width, mQInfoOut.u4Height, mQInfoIn.u4Profile, mQInfoIn.u4Level, mQInfoOut.u4Profile, mQInfoOut.u4Level);
        }
    }

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);

    // allocate output buffer headers address array
//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE)
    {
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
    mOutputBufferHdrsCnt = MAX_TOTAL_BUFFER_CNT;
    MTK_OMX_LOGUD("mOutputBufferHdrsCnt:%d, mOutputPortDef.nBufferCountActual:%d, MAX_MIN_UNDEQUEUED_BUFS:%d, FRAMEWORK_OVERHEAD:%d, TOTAL_MJC_BUFFER_CNT:%d, MAX_TOTAL_BUFFER_CNT:%d",
                 mOutputBufferHdrsCnt,
                 mOutputPortDef.nBufferCountActual,
                 MAX_MIN_UNDEQUEUED_BUFS,
                 FRAMEWORK_OVERHEAD,
                 TOTAL_MJC_BUFFER_CNT,
                 MAX_TOTAL_BUFFER_CNT);

    }
    else
    {
//#else
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
    mOutputBufferHdrsCnt = MAX_TOTAL_BUFFER_CNT;
    }
//#endif

    // allocate mFrameBuf
    arraySize = sizeof(FrmBufStruct) * MAX_TOTAL_BUFFER_CNT;
    mFrameBuf = (FrmBufStruct *)MTK_OMX_ALLOC(arraySize);
    MTK_OMX_MEMSET(mFrameBuf, 0x00, arraySize);
    mFrameBufInt = (FrmBufStruct *)MTK_OMX_ALLOC(arraySize);
    MTK_OMX_MEMSET(mFrameBufInt, 0x00, arraySize);

    MTK_OMX_LOGUD("allocate mFrameBuf: 0x%08x mOutputPortDef.nBufferCountActual:%d MAX_TOTAL_BUFFER_CNT:%d sizeof(FrmBufStruct):%d", mFrameBuf, mOutputPortDef.nBufferCountActual, MAX_TOTAL_BUFFER_CNT, sizeof(FrmBufStruct));

    // allocate mInputBuf
    mInputBuf = (InputBufStruct *)MTK_OMX_ALLOC(sizeof(InputBufStruct) * MAX_TOTAL_BUFFER_CNT);
    MTK_OMX_MEMSET(mInputBuf, 0x00, sizeof(InputBufStruct) * MAX_TOTAL_BUFFER_CNT);

    // create command pipe
    ret = pipe(mCmdPipe);
    if (ret)
    {
        MTK_OMX_LOGE("mCmdPipe creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    mIsComponentAlive = OMX_TRUE;

    //MTK_OMX_LOGU ("mCmdPipe[0] = %d", mCmdPipe[0]);
    // create vdec thread

    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mVdecThread, &attr, &MtkOmxVdecThread, (void *)this);

    pthread_attr_destroy(&attr);

    //ret = pthread_create(&mVdecThread, NULL, &MtkOmxVdecThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVdecThread creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    } else {
        mVdecThreadCreated = true;
    }

    // create video decoding thread

    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mVdecDecodeThread, &attr, &MtkOmxVdecDecodeThread, (void *)this);

    pthread_attr_destroy(&attr);

    //ret = pthread_create(&mVdecDecodeThread, NULL, &MtkOmxVdecDecodeThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVdecDecodeThread creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    } else {
        mVdecDecodeThreadCreated = true;
    }

    // create vdec convert thread
    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mVdecConvertThread, &attr, &MtkOmxVdecConvertThread, (void *)this);

    pthread_attr_destroy(&attr);

    //ret = pthread_create(&mVdecConvertThread, NULL, &MtkOmxVdecConvertThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("mVdecConvertThread creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    } else {
        mVdecConvertThreadCreated = true;
    }

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE)
    {
        m_fnMJCCreate(mpMJC, this);

        MJC_CALLBACKTYPE mMJCCallbacks;
        MJC_MODE mMode;

        mMJCCallbacks.pfnGetOutputFrame     = MtkMJC_GetOutputFrame;
        mMJCCallbacks.pfnGetInputFrame      = MtkMJC_GetInputFrame;
        mMJCCallbacks.pfnPutDispFrame       = MtkMJC_PutDispFrame;
        mMJCCallbacks.pfnSetRef             = MtkMJC_SetRefFrame;
        mMJCCallbacks.pfnClearRef           = MtkMJC_ClearRefFrame;
        mMJCCallbacks.pfnGetBufGraphicHndl  = MtkMJC_GetBufGraphicHndl;
        mMJCCallbacks.pfnGetBufInfo         = MtkMJC_GetBufInfo;
        mMJCCallbacks.pfnGetBufVA           = MtkMJC_GetBufVA;
        mMJCCallbacks.pfnGetBufPA           = MtkMJC_GetBufPA;
        mMJCCallbacks.pfnEventHandler       = MtkMJC_EventHandler;

        m_fnMJCSetParam(mpMJC, MJC_PARAM_CALLBACKS, &mMJCCallbacks);
        mMode = MJC_MODE_BYPASS;
        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
        m_fnMJCSetParam(mpMJC, MJC_PARAM_CODECTYPE, &mCodecId);
        m_fnMJCGetParam(mpMJC, MJC_PARAM_DRIVER_REGISTER, &mSucessfullyRegisterMJC);
        if(mSucessfullyRegisterMJC == OMX_FALSE)
        {
            MTK_OMX_LOGE("Fail to register MJC Driver");
        }
        MJCSetBufFormat();
        mMJCCreated = VAL_TRUE;
    }
//#endif

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    MTK_OMX_LOGU("+MtkOmxVdec::ComponentDeInit");
    OMX_ERRORTYPE err = OMX_ErrorNone;
    ssize_t ret = 0;

    if (mKernelLogCntChg == OMX_TRUE)
    {
        MTK_OMX_LOGE("Warning!! ComponentDeInit, but kernel log detect count not set to original");
        VAL_BOOL_T fgIncCount = VAL_FALSE;
        eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_KERNEL_LOG_COUNT, (VAL_VOID_T *)&fgIncCount, NULL);
        mKernelLogCntChg = OMX_FALSE;
    }

    if (mState == OMX_StateExecuting || mState == OMX_StatePause)
    {
        // component deinit when executing is freeNode without ACodec stop, maybe app crash
        // flush all ports
        mInputFlushALL = OMX_TRUE;
        SIGNAL(mOutputBufferSem); // for the driver callback to get output buffer
    }

    LOCK(mDecodeLock);
    if (MTK_VDEC_CODEC_ID_MJPEG != mCodecId && mDrvHandle != NULL)//handlesetstate DeInitVideoDecodeHW() fail will  cause mDrvHandle = NULL
    {
        if (mDecoderInitCompleteFlag == OMX_TRUE)
        {
            MTK_OMX_LOGE("Warning!! ComponentDeInit before DeInitVideoDecodeHW! De-Init video driver..");

            // return all buffers from decoder
            FlushDecoder(OMX_FALSE);

            if (VDEC_DRV_MRESULT_OK != eVDecDrvDeInit(mDrvHandle))
            {
                MTK_OMX_LOGE("Error!! DeInitVideoDecodeHW failed: eVDecDrvDeInit");
            }
            mDecoderInitCompleteFlag = OMX_FALSE;
        }
        if (VDEC_DRV_MRESULT_OK != eVDecDrvRelease(mDrvHandle))
        {
            MTK_OMX_LOGE("Error!! DeInitVideoDecodeHW failed: eVDecDrvRelease");
        }
    }
    mDrvHandle = NULL;
    UNLOCK(mDecodeLock);

#if 1
    if( mIsSecureInst && mbShareableMemoryEnabled )
    {
        // adb shell echo 1 > /dev/svp_region
        int fd = -1;
        fd = open("/proc/svp_region", O_RDONLY);
        if (fd == -1)
        {
            MTK_OMX_LOGE("[Info] fail to open /proc/svp_region");
        }
        else
        {
            //kernel would query TEE sec api to release mem from sec world
            char *share_mem_enable = "1";
            int vRet;
            int vRet2;

            vRet2 = ioctl(fd, SVP_REGION_RELEASE, &vRet);

            MTK_OMX_LOGU("Should Release Sec Memory %d %d %d", vRet, vRet2, errno);
            close(fd);
            mbShareableMemoryEnabled = OMX_FALSE;
        }
    }
#endif

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE && VAL_TRUE == mMJCCreated)
    {
        MJCFlushAllBufQ();
    }
//#endif

    // terminate decode thread
    mIsComponentAlive = OMX_FALSE;
	SIGNAL(mOutputBufferSem);
    SIGNAL(mDecodeSem);
    //terminate convert thread
    SIGNAL(mConvertSem);

//#ifdef MTK_CLEARMOTION_SUPPORT
    // terminate MJC thread
    // For Scaler ClearMotion +
    if (mMJCEnable == OMX_TRUE && VAL_TRUE == mMJCCreated)
    {
        SIGNAL(mpMJC->mMJCFrameworkSem);
        // For Scaler ClearMotion -
        if (mMJCLog)
        {
            MTK_OMX_LOGUD("[MJC] signal mMJCSem to terminate MJC thread");
        }
    }
//#endif

    // terminate command thread
    OMX_U32 CmdCat = MTK_OMX_STOP_COMMAND;
    if (mCmdPipe[MTK_OMX_PIPE_ID_WRITE] > -1)
    {
        WRITE_PIPE(CmdCat, mCmdPipe);
    }

    if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
    }
    if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
    {
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGUD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }
    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
        MTK_OMX_LOGUD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGUD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }

#if 1
    // ALPS02355777 pthread_join -> pthread_detach

    static int64_t _in_time_1 = 0;
    static int64_t _in_time_2 = 0;
    static int64_t _out_time = 0;
    _in_time_1 = getTickCountMs();

    while(1)
    {
        if(mVdecConvertThread != NULL && mVdecConvertThreadCreated)
        {
            _in_time_2 = getTickCountMs();
            _out_time = _in_time_2 - _in_time_1;
            if(_out_time > 5000)
            {
                MTK_OMX_LOGE("timeout wait for mVdecConvertThread terminated");
                abort();
                break;
            }
            else {
                SLEEP_MS(10);
            }
        }
        else
            break;
    }
    mVdecConvertThreadCreated = false;

    _in_time_1 = getTickCountMs();

    while(1)
    {
        if(mVdecDecodeThread != NULL && mVdecDecodeThreadCreated)
        {
            _in_time_2 = getTickCountMs();
            _out_time = _in_time_2 - _in_time_1;
            if(_out_time > 5000)
            {
                MTK_OMX_LOGE("timeout wait for mVdecDecodeThread terminated");
                abort();
                break;
            }
            else {
                SLEEP_MS(10);
            }
        }
        else
            break;
    }
    mVdecDecodeThreadCreated = false;

    _in_time_1 = getTickCountMs();

    while(1)
    {
        if(mVdecThread != NULL && mVdecThreadCreated)
        {
            _in_time_2 = getTickCountMs();
            _out_time = _in_time_2 - _in_time_1;
            if(_out_time > 5500)
            {
                MTK_OMX_LOGE("timeout wait for mVdecThread terminated");
                abort();
                break;
            }
            else {
                SLEEP_MS(10);
            }
        }
        else
            break;
    }
    mVdecThreadCreated = false;
#if 0
    if (!pthread_equal(pthread_self(), mVdecDecodeThread))
    {
        // wait for mVdecDecodeThread terminate
        pthread_join(mVdecDecodeThread, NULL);
    }

    if (!pthread_equal(pthread_self(), mVdecThread))
    {
        // wait for mVdecThread terminate
        pthread_join(mVdecThread, NULL);
    }

    if (!pthread_equal(pthread_self(), mVdecConvertThread))
    {
        // wait for mVdecConvertThread terminate
        pthread_join(mVdecConvertThread, NULL);
    }
#endif
//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE && VAL_TRUE == mMJCCreated)
    {
        if(mSucessfullyRegisterMJC == OMX_TRUE)
        {
            OMX_BOOL unRegisterDriver;
            m_fnMJCGetParam(mpMJC, MJC_PARAM_DRIVER_UNREGISTER, &unRegisterDriver);
            if(unRegisterDriver != OMX_TRUE) {
                MTK_OMX_LOGE("Fail to unregister MJC Driver");
            }
        }
        m_fnMJCDeInit(mpMJC);
    }

    MJCDestroyInstance();

//#endif
#endif

    if (NULL != mCoreGlobal)
    {
        ((mtk_omx_core_global *)mCoreGlobal)->video_instance_count--;
#if 0 // FIXME
        for (int i = 0 ; i < ((mtk_omx_core_global *)mCoreGlobal)->gInstanceList.size() ; i++)
        {
            const mtk_omx_instance_data *pInstanceData = ((mtk_omx_core_global *)mCoreGlobal)->gInstanceList.itemAt(i);
            if (pInstanceData == mGlobalInstData)
            {
                MTK_OMX_LOGE("@@ Remove instance op_thread(%d)", pInstanceData->op_thread);
                ((mtk_omx_core_global *)mCoreGlobal)->gInstanceList.removeAt(i);
            }
        }
#endif
    }

    if (mCmdPipe[MTK_OMX_PIPE_ID_READ] > -1)
    {
        close(mCmdPipe[MTK_OMX_PIPE_ID_READ]);
    }
    if (mCmdPipe[MTK_OMX_PIPE_ID_WRITE] > -1)
    {
        close(mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);
    }

    MTK_OMX_LOGD("-MtkOmxVdec::ComponentDeInit");

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_STRING componentName,
                                              OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                              OMX_OUT OMX_VERSIONTYPE *specVersion,
                                              OMX_OUT OMX_UUIDTYPE *componentUUID)

{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGU("MtkOmxVdec::GetComponentVersion");
    componentVersion->s.nVersionMajor = 1;
    componentVersion->s.nVersionMinor = 1;
    componentVersion->s.nRevision = 2;
    componentVersion->s.nStep = 0;
    specVersion->s.nVersionMajor = 1;
    specVersion->s.nVersionMinor = 1;
    specVersion->s.nRevision = 2;
    specVersion->s.nStep = 0;
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_COMMANDTYPE Cmd,
                                      OMX_IN OMX_U32 nParam1,
                                      OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVdec::SendCommand cmd=%s", CommandToString(Cmd));

    OMX_U32 CmdCat = MTK_OMX_GENERAL_COMMAND;

    ssize_t ret = 0;

    LOCK_T(mCmdQLock);

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (Cmd)
    {
        case OMX_CommandStateSet:   // write 8 bytes to pipe [cmd][nParam1]
            if (nParam1 == OMX_StateIdle)
            {
                MTK_OMX_LOGUD("set MTK_OMX_VDEC_IDLE_PENDING");
                SET_PENDING(MTK_OMX_IDLE_PENDING);
            }
            else if (nParam1 == OMX_StateLoaded)
            {
                MTK_OMX_LOGUD("set MTK_OMX_VDEC_LOADED_PENDING");
                SET_PENDING(MTK_OMX_LOADED_PENDING);
            }
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortDisable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            // mark the ports to be disabled first, p.84
            if (nParam1 == MTK_OMX_INPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mInputPortDef.bEnabled = OMX_FALSE;
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_FALSE;
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortEnable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            // mark the ports to be enabled first, p.85
            if (nParam1 == MTK_OMX_INPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mInputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_FALSE))
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }

                if ((mState == OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_FALSE))   // component is idle pending and port is not populated
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }
                if ((mState == OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_FALSE))   // component is idle pending and port is not populated
                {
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_FALSE))
                {
                    //MTK_OMX_LOGD ("SET_PENDING(MTK_OMX_VDEC_OUT_PORT_ENABLE_PENDING) mState(%d)", mState);
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandFlush:  // p.84
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandMarkBuffer:    // write 12 bytes to pipe [cmd][nParam1][pCmdData]
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            WRITE_PIPE(pCmdData, mCmdPipe);
            break;

        default:
            MTK_OMX_LOGE("[ERROR] Unknown command(0x%08X)", Cmd);
            break;
    }

EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                       OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::SetCallbacks");
    if (NULL == pCallBacks)
    {
        MTK_OMX_LOGE("[ERROR] MtkOmxVdec::SetCallbacks pCallBacks is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }
    mCallback = *pCallBacks;
    mAppData = pAppData;
    mCompHandle.pApplicationPrivate = mAppData;

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                       OMX_IN OMX_INDEXTYPE nParamIndex,
                                       OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("SP (0x%08X) mState %x", nParamIndex, mState);

#if (ANDROID_VER >= ANDROID_ICS)
    if ((mState != OMX_StateLoaded) && (mPortReconfigInProgress != OMX_TRUE))
    {
#else
    if (mState != OMX_StateLoaded)
    {
#endif
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

            if (!checkOMXParams(pPortDef)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nBufferCountActual > MAX_TOTAL_BUFFER_CNT) {
                MTK_OMX_LOGE("not support buffer count %u", pPortDef->nBufferCountActual);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(&mInputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                if(mInputPortDef.nBufferSize > MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K)
                {
                    char value[PROPERTY_VALUE_MAX];
                    property_get("ro.vendor.mtk_config_max_dram_size", value, "0x40000000");
                    if(mCodecId == MTK_VDEC_CODEC_ID_AVC && strtol(value, NULL, 0) <= 0x40000000)
                    {
                        //for testStagefright_cve_2016_3741, sts set an extremely big buffersize which cause process kill by lmk in low mem project
                        mInputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K;
                        MTK_OMX_LOGU("[Warning] Setting unreasonable buffersize, force max to avc default input buffer size 4K (3.5M)");
                    }
                }

                if (mInputPortDef.format.video.nFrameWidth * mInputPortDef.format.video.nFrameHeight >= FHD_AREA)
                {
                    if (mInputPortDef.nBufferSize < SIZE_512K)
                    {
                        mInputPortDef.nBufferSize = SIZE_512K;
                    }
                }

                if ((mCodecId == MTK_VDEC_CODEC_ID_VP9)&&(meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE))
                {
                    mInputPortDef.nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_SW;
                }

                if (mInputPortDef.format.video.nFrameWidth * mInputPortDef.format.video.nFrameHeight > FHD_AREA)
                {
                    if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
                    {
                        if (mIsSecureInst && INHOUSE_TEE == mTeeType)
                        {
                            mInputPortDef.nBufferSize = MTK_VDEC_AVCSEC_DEFAULT_INPUT_BUFFER_SIZE_4K;
                        }
                        else if (mInputPortDef.nBufferSize < MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K)
                        {
                            mInputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K;
                        }
                        //mInputPortDef.nBufferCountActual = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT_4K;
                        //mInputPortDef.nBufferCountMin = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT_4K;
                    }
                    if (mCodecId == MTK_VDEC_CODEC_ID_HEVC)
                    {
                        if (mInputPortDef.nBufferSize < MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE_4K)
                        {
                            mInputPortDef.nBufferSize = MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE_4K;
                        }
                    }
                    if (mCodecId == MTK_VDEC_CODEC_ID_VP9)
                    {
                        if (mIsSecureInst && INHOUSE_TEE == mTeeType)
                        {
                            mInputPortDef.nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE;
                        }
                        else if (mInputPortDef.nBufferSize < MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_4K)
                        {
                            mInputPortDef.nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_4K;
                        }
                    }
                } else if (mInputPortDef.format.video.nFrameWidth * mInputPortDef.format.video.nFrameHeight > HD_AREA) {
                    if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
                    {
                        mInputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE;
                    }
                } else {  // <= HD_AREA
                    if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
                    {
                        mInputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_HD;
                    }
                }
                MTK_OMX_LOGU("[INPUT] (%d)(%d)(%d)", mInputPortDef.format.video.nFrameWidth, mInputPortDef.format.video.nFrameHeight, mInputPortDef.nBufferSize);
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *) pCompParam;


                //                MTK_OMX_LOGE(" 1 OMX_IndexParamPortDefinition for output buffer error %d, %d, %d", pPortDef->nBufferCountActual,
                //                            mOutputPortDef.nBufferCountActual, mStarvationSize);

                if (pPortDef->format.video.nFrameWidth == 0 || pPortDef->format.video.nFrameHeight == 0)
                {
                    err = OMX_ErrorBadParameter;
                    MTK_OMX_LOGE("SetParameter bad parameter width/height is 0");
                    goto EXIT;
                }

                if (mOutputPortDef.format.video.nFrameWidth == pPortDef->format.video.nFrameWidth && mOutputPortDef.format.video.nFrameHeight == pPortDef->format.video.nFrameHeight)
                {
                    if (pPortDef->nBufferCountActual - mOutputPortDef.nBufferCountActual > mStarvationSize)
                    {
                        //MTK_OMX_LOGE("OMX_IndexParamPortDefinition for output buffer error %d, %d, %d", pPortDef->nBufferCountActual,
                        //mOutputPortDef.nBufferCountActual, mStarvationSize);
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                    }
                    else
                    {
                        memcpy(&mOutputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                        if (OMX_TRUE == mDecoderInitCompleteFlag)
                        {
                            return err; // return ok
                        }
                        else
                        {
                            //MTK_OMX_LOGU("keep going before decoder init");
                        }
                    }
                }
                else if (mOutputPortDef.format.video.nFrameWidth != pPortDef->format.video.nFrameWidth || mOutputPortDef.format.video.nFrameHeight != pPortDef->format.video.nFrameHeight)
                {
                    memcpy(&mOutputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                    mOutputPortDef.format.video.nFrameWidth = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameWidth,2);
                    mOutputPortDef.format.video.nFrameHeight = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameHeight,2);
                }
                else   // for error case
                {
                    MTK_OMX_LOGE("OMX_IndexParamPortDefinition for output buffer error");
                    err = OMX_ErrorBadParameter;
                    goto EXIT;
                }
                //MTK_OMX_LOGU ("mOutputPortDef.format.video.nFrameWidth=%d, mOutputPortDef.format.video.nFrameHeight=%d", mOutputPortDef.format.video.nFrameWidth, mOutputPortDef.format.video.nFrameHeight);
                // calculate new stride/slice height and new output buffer size
#if 1
                //mOutputPortDef.format.video.nStride = VDEC_ROUND_16(mOutputPortDef.format.video.nFrameWidth);
                //mOutputPortDef.format.video.nSliceHeight = VDEC_ROUND_32(mOutputPortDef.format.video.nFrameHeight);

                MTK_OMX_MEMSET(&mQInfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
                MTK_OMX_MEMSET(&mQInfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
                if (OMX_FALSE == QueryDriverFormat(&mQInfoIn, &mQInfoOut))
                {
                    mOutputPortDef.format.video.nStride = mQInfoOut.u4Width;
                    mOutputPortDef.format.video.nSliceHeight = mQInfoOut.u4Height;
                    MTK_OMX_LOGE("Unsupported Video Resolution (%d, %d), MAX(%d, %d); Current Video Profile/Level (%x/%d), MAX(%x/%d)", mQInfoIn.u4Width, mQInfoIn.u4Height,\
                        mQInfoOut.u4Width, mQInfoOut.u4Height, mQInfoIn.u4Profile, mQInfoIn.u4Level, mQInfoOut.u4Profile, mQInfoOut.u4Level);
                    err = OMX_ErrorBadParameter;
                    goto EXIT;
                }
                else
                {
                    //for MTK VIDEO 4KH264 SUPPORT  // //query output color format again avoid wrong format [
                    if (mCodecId == MTK_VDEC_CODEC_ID_AVC || mCodecId == MTK_VDEC_CODEC_ID_HEVC)
                    {
                        switch (mQInfoOut.ePixelFormat)
                        {
                            case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
                                mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                                mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                                break;

                            case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
                                if (mIsInterlacing)
                                {
                                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                                }
                                else
                                {
                                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                                }
                                MTK_OMX_LOGUD("SetParameter: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
                                break;

                            case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
                                mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
                                mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
                                MTK_OMX_LOGUD("SetParameter:VDEC_DRV_PIXEL_FORMAT_YUV_YV12");
                                break;

                            case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
                                mOutputPortFormat.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                                mOutputPortDef.format.video.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                                MTK_OMX_LOGUD("UFO format is set 2");
                                break;

                            case VDEC_DRV_PIXEL_FORMAT_YUV_NV12:
                                mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
                                mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
                                MTK_OMX_LOGUD("SetParameter:VDEC_DRV_PIXEL_FORMAT_YUV_NV12");
                                break;

                            default:
                                break;
                        }
                    }
                    //] 4KH264
                    if (mLegacyMode == OMX_TRUE)
                    {
                        mOutputPortDef.format.video.nFrameWidth = mMaxWidth;
                        mOutputPortDef.format.video.nFrameHeight = mMaxHeight;
                    }
                    mOutputPortDef.format.video.nStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameWidth, mQInfoOut.u4StrideAlign);
                    mOutputPortDef.format.video.nSliceHeight = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameHeight, mQInfoOut.u4SliceHeightAlign);
                    MTK_OMX_LOGD(" Video Resolution width/height(%d, %d), nStride/nSliceHeight(%d, %d)", mOutputPortDef.format.video.nFrameWidth, mOutputPortDef.format.video.nFrameHeight, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);
                }
                //mOutputPortDef.format.video.eColorFormat = MtkDriver2OmxColorFormatMap(qinfoOut.ePixelFormat);    //color format has handled in MtkOmxVdec::ComponentInit
#if (ANDROID_VER >= ANDROID_KK)
                VDEC_DRV_CROPINFO_T temp_ccop_info;

                memset(&temp_ccop_info, 0, sizeof(VDEC_DRV_CROPINFO_T));
                if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO, NULL, &temp_ccop_info))
                {
                    MTK_OMX_LOGD("[Info] Cannot get param: VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO");
                    mCropLeft = 0;
                    mCropTop = 0;
                    mCropWidth = mOutputPortDef.format.video.nFrameWidth;
                    mCropHeight = mOutputPortDef.format.video.nFrameHeight;
                }
                else
                {
                    mCropLeft = temp_ccop_info.u4CropLeft;
                    mCropTop = temp_ccop_info.u4CropTop;
                    mCropWidth = temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1;
                    mCropHeight = temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1;
                }
                MTK_OMX_LOGD("GetCropInfo() mCropLeft %d, mCropTop %d, mCropWidth %d, mCropHeight %d\n",
                        mCropLeft, mCropTop, mCropWidth, mCropHeight);
#endif

#if 0
                //mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1) + 16;
#else   // for SW codec YV12 16,16,16 stride buffer size (without native window)
                //mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight) +
                //                             (VDEC_ROUND_16(mOutputPortDef.format.video.nStride / 2) * mOutputPortDef.format.video.nSliceHeight);
                mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nStride * (mOutputPortDef.format.video.nSliceHeight + 1) * 3 >> 1);// + 16
#endif

                //for MTK SUPPORT MJPEG [
                if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingMJPEG)
                {
                    mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight) << 2;
                    if (mThumbnailMode == OMX_TRUE)
                    {
                        mOutputPortDef.nBufferCountActual = 2;
                        mOutputPortDef.nBufferCountMin = 2;
                    }
                }
                //] MJPEG
                MTK_OMX_LOGUD("nStride = %d,  nSliceHeight = %d, PicSize = %d, ori_nBufferCountActual = %d", mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight,
                             mOutputPortDef.nBufferSize, mOutputPortDef.nBufferCountActual);
                if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC)
                {
                    int32_t MaxDPBSize, PicSize, MaxDPBNumber, ActualOutBuffNums, ActualOutBuffSize;
                    if (mSeqInfoCompleteFlag == OMX_FALSE)
                    {
                        //MaxDPBSize = 6912000; // level 3.1
                        MaxDPBSize = 70778880; // level 5.1
                        PicSize = ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) * 3) >> 1;
                        MaxDPBNumber = MaxDPBSize / PicSize;
                    }
                    else
                    {
                        MaxDPBNumber = mDPBSize;
                    }
                    MaxDPBNumber = (((MaxDPBNumber) < (16)) ? MaxDPBNumber : 16);
                    if (mLegacyMode == OMX_TRUE)
                    {
                        MaxDPBNumber = 16;
                    }
                    ActualOutBuffNums = MaxDPBNumber + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD; // for some H.264 baseline with other nal headers

                    //if (mChipName == VAL_CHIP_NAME_MT6580)
                    //{
                    //    mIs4kSwAvc = OMX_TRUE;
                    //}
//#ifdef MTK_CLEARMOTION_SUPPORT
                    if (mMJCEnable == OMX_TRUE)
                    {
                    if (mThumbnailMode == OMX_TRUE)
                    {
                        mUseClearMotion = OMX_FALSE;
                        MJC_MODE mMode;
                        mMode = MJC_MODE_BYPASS;
                        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                    }
                    else
                    {
                        if ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) > (1920 * 1088) ||
                            (mOutputPortDef.format.video.nFrameWidth > 1920) || (mOutputPortDef.format.video.nFrameHeight > 1920))
                        {
                            if (mIs4kSwAvc)
                            {
                                ActualOutBuffNums += 2; // for sw 4k264
                            }
                            ActualOutBuffNums += (MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT + mMinUndequeuedBufs);
                            mUseClearMotion = OMX_FALSE;
                            MJC_MODE mMode;
                            mMode = MJC_MODE_BYPASS;
                            m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                        }
                        else
                        {
                            if ((OMX_FALSE == mLegacyMode) && (mUseClearMotion == OMX_TRUE) && ((ActualOutBuffNums + TOTAL_MJC_BUFFER_CNT) <= MAX_TOTAL_BUFFER_CNT))
                            {
                                ActualOutBuffNums += TOTAL_MJC_BUFFER_CNT;
                                mUseClearMotion = OMX_TRUE;
                                MJC_MODE mMode;
                                mMode = MJC_MODE_NORMAL;
                                m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                            }
                            else
                            {
                                mUseClearMotion = OMX_FALSE;
                                MJC_MODE mMode;
                                mMode = MJC_MODE_BYPASS;
                                m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                            }
                        }
                    }

                    MTK_OMX_LOGD("mUseClearMotion = %d\n", mUseClearMotion);
                    }
//#endif
                    mOutputPortDef.nBufferCountActual = ActualOutBuffNums;
                    mOutputPortDef.nBufferCountMin = ActualOutBuffNums - mMinUndequeuedBufs;

                    MTK_OMX_LOGD("mSeqInfoCompleteFlag %d, mLegacyMode = %d, mIsUsingNativeBuffers %d",
                                 mSeqInfoCompleteFlag, mLegacyMode, mIsUsingNativeBuffers);
                    if ((mSeqInfoCompleteFlag == OMX_FALSE) && (mLegacyMode == OMX_FALSE))
                    {
//#ifdef MTK_CLEARMOTION_SUPPORT
                        if (mMJCEnable == OMX_TRUE)
                        {
                        mUseClearMotion = OMX_FALSE;
                        MJC_MODE mMode;
                        mMode = MJC_MODE_BYPASS;
                        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                        }
//#endif
                        mOutputPortDef.nBufferCountActual = 3 + mMinUndequeuedBufs;  //For initHW get DPB size and free
                        mOutputPortDef.nBufferCountMin = 3;

                        /* Workaround to make a little change on the crop size, so that we won't fail
                                               in the formatHasNotablyChanged when format changed was sent.
                                          */
                        //mCropWidth --; // for output format change
                        //mCropWidth -= mOutputPortDef.format.video.nFrameWidth > 32 ? 32 : 1;
                        //MTK_OMX_LOGU("Force port re-config for actual output buffer size!!");
                    }

                    if (mThumbnailMode == OMX_TRUE)
                    {
                        mOutputPortDef.nBufferCountActual = 3;
                        mOutputPortDef.nBufferCountMin = 3;
                    }
                    MTK_OMX_LOGUD("MaxDPBNumber = %d,  OutputBuffers = %d (%d) ,PicSize=%d", MaxDPBNumber, ActualOutBuffNums, mOutputPortDef.nBufferCountActual, mOutputPortDef.nBufferSize);

                }
                else if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)
                {

                    int32_t PicSize, MaxDPBNumber, ActualOutBuffNums, ActualOutBuffSize;
                    PicSize = ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) * 3) >> 1;
                    MaxDPBNumber = mDPBSize;
                    //MaxDPBNumber = (((MaxDPBNumber) < (16)) ? MaxDPBNumber : 16);
                    ActualOutBuffNums = MaxDPBNumber + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD; // for some HEVC baseline with other nal headers

//#ifdef MTK_CLEARMOTION_SUPPORT
                    if (mMJCEnable == OMX_TRUE)
                    {
                    if (mThumbnailMode == OMX_TRUE)
                    {
                        mUseClearMotion = OMX_FALSE;
                        MJC_MODE mMode;
                        mMode = MJC_MODE_BYPASS;
                        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                    }
                    else
                    {
                        if ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) > (1920 * 1088))
                        {
                            if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
                            {
                                ActualOutBuffNums += (MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT);
                            }
                            mUseClearMotion = OMX_FALSE;
                            MJC_MODE mMode;
                            mMode = MJC_MODE_BYPASS;
                            m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                        }
                        else
                        {
                            if ((mUseClearMotion == OMX_TRUE) && ((ActualOutBuffNums + TOTAL_MJC_BUFFER_CNT) <= MAX_TOTAL_BUFFER_CNT))
                            {
                                ActualOutBuffNums += TOTAL_MJC_BUFFER_CNT;
                                mUseClearMotion = OMX_TRUE;
                                MJC_MODE mMode;
                                mMode = MJC_MODE_NORMAL;
                                m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                            }
                            else
                            {
                                mUseClearMotion = OMX_FALSE;
                                MJC_MODE mMode;
                                mMode = MJC_MODE_BYPASS;
                                m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                            }
                        }
                    }

                    if (mSeqInfoCompleteFlag == OMX_FALSE)
                    {
                        mUseClearMotion = OMX_FALSE;
                        MJC_MODE mMode;
                        mMode = MJC_MODE_BYPASS;
                        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
                    }
                    }
//#endif
                    mOutputPortDef.nBufferCountActual = ActualOutBuffNums;
                    mOutputPortDef.nBufferCountMin = ActualOutBuffNums - mMinUndequeuedBufs;

                    if (mSeqInfoCompleteFlag == OMX_FALSE)
                    {
                        {
                            mOutputPortDef.nBufferCountActual = 2 + mMinUndequeuedBufs;  //For initHW get DPB size and free
                            mOutputPortDef.nBufferCountMin = 2;
                        }
                        if (mThumbnailMode == OMX_FALSE)
                        {
                            MTK_OMX_LOGD("Force port re-config for actual output buffer size!!");
                        }
                    }

                    if (mThumbnailMode == OMX_TRUE)
                    {
                        mOutputPortDef.nBufferCountActual = 1;
                        mOutputPortDef.nBufferCountMin = 1;

//#ifdef MTK_CLEARMOTION_SUPPORT
                        if (mMJCEnable == OMX_TRUE && meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
                        {
                            mOutputPortDef.nBufferCountActual += (TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs);
                            mOutputPortDef.nBufferCountMin += (TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs);
                        }
//#endif
                    }

                    MTK_OMX_LOGUD("MaxDPBNumber = %d,  OutputBuffers = %d (%d) ,PicSize=%d", MaxDPBNumber, ActualOutBuffNums, mOutputPortDef.nBufferCountActual, mOutputPortDef.nBufferSize);
                    //mOutputPortDef.nBufferSize = ActualOutBuffSize;
                    //mOutputPortDef.nBufferSize += 16;
                }

                //force allocating buffer if MJC fake engine is enabled
                HandleVendorMtkOmxVdecUseClearMotion(OMX_TRUE);

#else
                mOutputPortDef.format.video.nStride = ((mOutputPortDef.format.video.nFrameWidth + 15) & (~15));
                mOutputPortDef.format.video.nSliceHeight = ((mOutputPortDef.format.video.nFrameHeight + 15) & (~15));
                mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1);
                mOutputPortDef.nBufferSize = ((mOutputPortDef.nBufferSize + 31) & (~31)) + 32;;
#endif
                if ( /*(OMX_FALSE == mIsUsingNativeBuffers) &&*/ (OMX_TRUE == mCrossMountSupportOn) )
                {
                    VAL_CHAR_T mMCCMaxValue[PROPERTY_VALUE_MAX];
                    if (property_get("ro.vendor.mtk_crossmount.maxcount", mMCCMaxValue, NULL))
                    {
                        //use set property, the maxcount
                    }
                    else
                    {
                        mMaxColorConvertOutputBufferCnt = mOutputPortDef.nBufferCountActual;
                    }
                    mOutputPortDef.nBufferCountActual += mMaxColorConvertOutputBufferCnt;
                    MTK_OMX_LOGUD("original nBufferCountActual after adjust = %d(+%d) ",
                        mOutputPortDef.nBufferCountActual, mMaxColorConvertOutputBufferCnt);
                }
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }
            break;
        }
        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam;

            if (!checkOMXParams(pPortFormat)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                // TODO: should we allow setting the input port param?
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGUD("Set Output eColorFormat before %x, after %x", mOutputPortFormat.eColorFormat, pPortFormat->eColorFormat);
                //keep original format for CTS Vp8EncoderTest and GTS Vp8CocecTest, sw decode YV12 -> I420 -> set to OMX component
                //L-MR1 change the flow to preference the flexible format if hasNativeWindow=true but component can not support flexible format
                //framework will do SW render(clear NW), OMX component will do color convert to I420 for this SW rendering
                if (mOutputPortFormat.eColorFormat != pPortFormat->eColorFormat)
                {
                    mOutputPortFormat.eColorFormat = pPortFormat->eColorFormat;
                }
                else
                {
                    //CTS DecodeEditEncodeTest, Surface for output config in mediacodec will cause flexibleYUV flow enable.
                    //acodec log: does not support color format 7f000789, err 80000000
                    if( OMX_TRUE == mbYUV420FlexibleMode )
                    {
                        MTK_OMX_LOGUD("Disable FlexibleYUV Flow for surface format, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface in CTS case");
                        mbYUV420FlexibleMode = OMX_FALSE;
                    }
                }
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }
            break;
        }
        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;

            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            strlcpy((char *)mCompRole, (char *)pRoleParams->cRole, sizeof(mCompRole));
            break;
        }

        case OMX_IndexParamVideoRv:
        {
            OMX_VIDEO_PARAM_RVTYPE *pRvType = (OMX_VIDEO_PARAM_RVTYPE *)pCompParam;

            if (!checkOMXParams(pRvType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            MTK_OMX_LOGUD("Set pRvType nPortIndex=%d", pRvType->nPortIndex);
            memcpy(&mRvType, pCompParam, sizeof(OMX_VIDEO_PARAM_RVTYPE));
            break;
        }

        case OMX_IndexVendorMtkOmxVdecThumbnailMode:
        {
            OMX_PARAM_U32TYPE *pThumbnailMode = (OMX_PARAM_U32TYPE *)pCompParam;
            MTK_OMX_LOGUD("Set thumbnail mode enable? %s", (pThumbnailMode->nU32==1?"yes":"no"));
            if( 1 == pThumbnailMode->nU32 )
            {
            mThumbnailMode = OMX_TRUE;
            if ((mCodecId == MTK_VDEC_CODEC_ID_VPX) ||
                (mCodecId == MTK_VDEC_CODEC_ID_VP9) ||
                (mCodecId == MTK_VDEC_CODEC_ID_VC1) ||
                (mCodecId == MTK_VDEC_CODEC_ID_MPEG2) ||
                (mCodecId == MTK_VDEC_CODEC_ID_MPEG4))
            {
                mOutputPortDef.nBufferCountActual = MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT;
                MTK_OMX_LOGUD("SetParameter OMX_IndexVendorMtkOmxVdecThumbnailMode mOutputPortDef.nBufferCountActual %d", mOutputPortDef.nBufferCountActual);
            }
            }

            break;
        }

//#ifdef MTK_CLEARMOTION_SUPPORT
        case OMX_IndexVendorMtkOmxVdecUseClearMotion:
        {
            OMX_PARAM_U32TYPE *pUseClearMotion = (OMX_PARAM_U32TYPE *)pCompParam;
            //MTK_OMX_LOGU("UseClearMotion? %s", ((pUseClearMotion->nU32 & 1)==1?"yes":"no"));
            mClearMotionDemoMode = (pUseClearMotion->nU32 & 2) >> 1;    // bit 0 for mUseClearMotion, bit 1 for mClearMotionDemoMode
            //MTK_OMX_LOGU("mClearMotionDemoMode %d", mClearMotionDemoMode);

            HandleVendorMtkOmxVdecUseClearMotion(OMX_FALSE);
            MTK_OMX_LOGUD("[MJC] UseClearMotion? %s, mClearMotionDemoMode %d, buf %d\n", ((pUseClearMotion->nU32 & 1)==1?"yes":"no"), mClearMotionDemoMode, mOutputPortDef.nBufferCountActual);
            break;
        }
//#endif
        case OMX_IndexVendorMtkOmxVdecGetMinUndequeuedBufs:
        {
            OMX_PARAM_U32TYPE *pUseClearMotion = (OMX_PARAM_U32TYPE *)pCompParam;
            mMinUndequeuedBufsDiff = pUseClearMotion->nU32 - mMinUndequeuedBufs;
            mMinUndequeuedBufs = pUseClearMotion->nU32;

            if (mMinUndequeuedBufs > MAX_MIN_UNDEQUEUED_BUFS)
            {
                //MTK_OMX_LOGU("[MJC][ERROR] Please Check!!\n");
                MTK_OMX_LOGE("[MJC][ERROR] mMinUndequeuedBufs : %d\n", mMinUndequeuedBufs);
                //MTK_OMX_LOGU("[MJC][ERROR] Please Check!!\n");
            }

            if ((mMinUndequeuedBufsFlag == OMX_FALSE) && (mOutputPortDef.nBufferCountActual + mMinUndequeuedBufsDiff <= MAX_TOTAL_BUFFER_CNT))
            {
                //mOutputPortDef.nBufferCountActual += mMinUndequeuedBufsDiff;
                mMinUndequeuedBufsFlag = OMX_TRUE;
            }

            MTK_OMX_LOGD("[MJC] mMinUndequeuedBufs : %d (%i, %d)\n", mMinUndequeuedBufs, mMinUndequeuedBufsDiff, mOutputPortDef.nBufferCountActual);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecConcealmentLevel:
        {
            mConcealLevel = *(OMX_S32 *)pCompParam;
            MTK_OMX_LOGUD("Set concealment level %d", mConcealLevel);
            break;
        }
#if 0//def MTK S3D SUPPORT
        case OMX_IndexVendorMtkOmxVdec3DVideoPlayback:
        {
            m3DStereoMode = *(OMX_VIDEO_H264FPATYPE *)pCompParam;
            MTK_OMX_LOGU("3D mode from parser, %d", m3DStereoMode);
            break;
        }
#endif

        case OMX_IndexVendorMtkOmxVdecStreamingMode:
        {
            OMX_PARAM_U32TYPE *pStreamingModeInfo = (OMX_PARAM_U32TYPE *)pCompParam;
            mStreamingMode = (OMX_BOOL)pStreamingModeInfo->nU32;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecACodecColorConvertMode:
        {
            mACodecColorConvertMode = *(OMX_U32 *)pCompParam;
            MTK_OMX_LOGUD("OMX_IndexVendorMtkOmxVdecACodecColorConvertMode mACodecColorConvertMode: %d", mACodecColorConvertMode);
            break;
        }
        case OMX_IndexVendorMtkOmxVdecFixedMaxBuffer:
        {
            MTK_OMX_LOGUD("deprecated, only turn this on when legacy mode");
            break;
        }

#if (ANDROID_VER >= ANDROID_ICS)
        case OMX_GoogleAndroidIndexEnableAndroidNativeBuffers:
        {
            EnableAndroidNativeBuffersParams *pEnableNativeBuffers = (EnableAndroidNativeBuffersParams *) pCompParam;
            if (pEnableNativeBuffers->nPortIndex != mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGE("@@ OMX_GoogleAndroidIndexEnableAndroidNativeBuffers: invalid port index");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (NULL != pEnableNativeBuffers)
            {
                MTK_OMX_LOGUD("OMX_GoogleAndroidIndexEnableAndroidNativeBuffers enable(%d)", pEnableNativeBuffers->enable);
                mIsUsingNativeBuffers = pEnableNativeBuffers->enable;
            }
            else
            {
                MTK_OMX_LOGE("@@ OMX_GoogleAndroidIndexEnableAndroidNativeBuffers: pEnableNativeBuffers is NULL");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            break;
        }

        case OMX_GoogleAndroidIndexUseAndroidNativeBuffer:
        {
            // Bruce Hsu 20120329 cancel workaround
            // Morris Yang 20111128 temp workaround for ANativeWindowBuffer
            UseAndroidNativeBufferParams *pUseNativeBufferParams = (UseAndroidNativeBufferParams *)pCompParam;
            //UseAndroidNativeBufferParams3* pUseNativeBufferParams = (UseAndroidNativeBufferParams3*)pCompParam;

            if (pUseNativeBufferParams->nPortIndex != mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGE("@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer: invalid port index");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (OMX_TRUE != mIsUsingNativeBuffers)
            {
                MTK_OMX_LOGE("@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer: we are not using native buffers");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            sp<android_native_buffer_t> nBuf = pUseNativeBufferParams->nativeBuffer;
            buffer_handle_t _handle = nBuf->handle;
            native_handle_t *pGrallocHandle = (native_handle_t *) _handle;
            int ion_buf_fd = -1;

            // TODO: check secure case
            int secure_buffer_handle;
            if (OMX_TRUE == mIsSecureInst)
            {
                size_t bufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3) >> 1;

                if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
                {
                    bufferSize = (mOutputPortDef.format.video.nStride * (mOutputPortDef.format.video.nSliceHeight + 1) * 3 >> 1);
                }
                //GraphicBufferMapper::getInstance().getSecureBuffer(_handle, &buffer_type, &secure_buffer_handle);
                mSecFrmBufInfo[mSecFrmBufCount].pNativeHandle = (void *)_handle;

#if SECURE_OUTPUT_USE_ION
                int ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);

                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR]newOmxMVAwithHndl() failed");
                }

                VBufInfo BufInfo;
                if (mOutputMVAMgr->getOmxInfoFromHndl((void *)_handle, &BufInfo) < 0)
                {
                    MTK_OMX_LOGE("[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                //get secure handle from ion
                secure_buffer_handle = BufInfo.u4PA;
                MTK_OMX_LOGD("child Physical address = 0x%x, len = %d\n", BufInfo.u4PA, BufInfo.u4BuffSize);

#else
                gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &secure_buffer_handle);
#endif
                MTK_OMX_LOGUD("@@ _handle(0x%08X), secure_buffer_handle(0x%08X)", _handle, secure_buffer_handle);
                mOutputUseION = OMX_FALSE;
                mSecFrmBufInfo[mSecFrmBufCount].u4BuffId = secure_buffer_handle;
                mSecFrmBufInfo[mSecFrmBufCount].u4SecHandle = secure_buffer_handle;
                mSecFrmBufInfo[mSecFrmBufCount].u4BuffSize = bufferSize;

                err = UseBuffer(hComp, pUseNativeBufferParams->bufferHeader, pUseNativeBufferParams->nPortIndex, pUseNativeBufferParams->pAppPrivate, bufferSize, (OMX_U8 *)secure_buffer_handle);

                if (err != OMX_ErrorNone)
                {
                    MTK_OMX_LOGE("[ERROR] UseBuffer failed, LINE:%d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                goto EXIT;
            }

            gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ION_FD, &ion_buf_fd);

            if ((-1 < ion_buf_fd) && (mCodecId != MTK_VDEC_CODEC_ID_MJPEG))
            {
                // fd from gralloc and no need to free it in FreeBuffer function
                mIsFromGralloc = OMX_TRUE;
                // use ION
                mOutputUseION = OMX_TRUE;
            }
            else
            {
                // use M4U
                mOutputUseION = OMX_FALSE;
                if (mOutputMVAMgr != NULL)
                {
                    delete mOutputMVAMgr;
                    mOutputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec2");
                }
            }

            if (OMX_TRUE == mOutputUseION)
            {
                MTK_OMX_LOGUD("buffer_handle_t(0x%08x), ionFd(0x%08X)", _handle, ion_buf_fd);

                if (OMX_TRUE != mIsUsingNativeBuffers)
                {
                    MTK_OMX_LOGE("[ERROR] OMX_GoogleAndroidIndexUseAndroidNativeBuffer: we are not using native buffers");
                    err = OMX_ErrorBadParameter;
                    goto EXIT;
                }
                // map virtual address
                size_t bufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1);
                //fore MTK VIDEO 4KH264 SUPPORT [
                if (MTK_VDEC_CODEC_ID_AVC == mCodecId || MTK_VDEC_CODEC_ID_HEVC == mCodecId)
                {
                    // query output color format and stride and sliceheigt
                    VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoIn;
                    VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;
                    MTK_OMX_MEMSET(&qinfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
                    MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
                    if (OMX_TRUE == QueryDriverFormat(&qinfoIn, &qinfoOut))
                    {
                        meDecodeType = qinfoOut.eDecodeType;
                        MTK_OMX_LOGUD(" AVC meDecodeType = %d", meDecodeType);
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] AVC QueryDriverFormat failed: Video Resolution (%d, %d), MAX(%d, %d); Current Video Profile/Level (%x/%d), MAX(%x/%d)", qinfoIn.u4Width, qinfoIn.u4Height,\
                            mQInfoOut.u4Width, mQInfoOut.u4Height, qinfoIn.u4Profile, qinfoIn.u4Level, mQInfoOut.u4Profile, mQInfoOut.u4Level);
                    }
                }
                //] 4KH264
                if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
                {
                    bufferSize = (mOutputPortDef.format.video.nStride * (mOutputPortDef.format.video.nSliceHeight + 1) * 3 >> 1);
                }
//#ifdef MTK_CLEARMOTION_SUPPORT
                if (mMJCEnable == OMX_TRUE)
                {
                size_t sz_stride = VDEC_ROUND_16(mOutputPortDef.format.video.nStride);
                size_t sz_vertical_stride = VDEC_ROUND_32(mOutputPortDef.format.video.nSliceHeight);
                size_t sz_stride_uv = VDEC_ROUND_16(sz_stride / 2);
                size_t sz_vertical_stride_uv = sz_vertical_stride / 2;
                bufferSize = (sz_stride * (sz_vertical_stride + 1)) + (2 * sz_stride_uv * (sz_vertical_stride_uv + 1));
                }
//#endif

                int size, ret;
                OMX_U8 *buffer;
                gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);

                if (bufferSize > size)
                {
                    MTK_OMX_LOGE("[ERROR] Gralloc buffer size(%d) < codec required(%d)", size, bufferSize);
                }
                ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);

                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR]newOmxMVAwithHndl() failed");
                }

                VBufInfo BufInfo;
                if (mOutputMVAMgr->getOmxInfoFromHndl((void *)_handle, &BufInfo) < 0)
                {
                    MTK_OMX_LOGE("[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                buffer = (OMX_U8 *)BufInfo.u4VA;

                if (OMX_TRUE == mIsSecureInst)
                {
                    unsigned long secHandle = 0;
                    unsigned long va_share_handle = 0;
                    // get secure buffer handle from gralloc (DEBUG ONLY)
                    if (INHOUSE_TEE == mTeeType)
                    {
                        MtkVideoAllocateSecureFrameBuffer_Ptr *pfnMtkVideoAllocateSecureFrameBuffer_Ptr = NULL;
                        pfnMtkVideoAllocateSecureFrameBuffer_Ptr =
                        (MtkVideoAllocateSecureFrameBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER);
                        if (NULL == pfnMtkVideoAllocateSecureFrameBuffer_Ptr)
                        {
                            MTK_OMX_LOGE("[ERROR] cannot find MtkVideoAllocateSecureFrameBuffer, LINE: %d", __LINE__);
                            err = OMX_ErrorUndefined;
                            goto EXIT;

                        }

                    MtkVideoRegisterSharedMemory_Ptr *pfnMtkVideoRegisterSharedMemory_Ptr = NULL;
                    pfnMtkVideoRegisterSharedMemory_Ptr =
                    (MtkVideoRegisterSharedMemory_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_REGISTER_SHARED_MEMORY);
                        if (NULL == pfnMtkVideoRegisterSharedMemory_Ptr)
                        {
                            MTK_OMX_LOGE("[ERROR] cannot find MtkVideoRegisterSharedMemory, LINE: %d", __LINE__);
                            err = OMX_ErrorUndefined;
                            goto EXIT;

                        }
                        secHandle = pfnMtkVideoAllocateSecureFrameBuffer_Ptr(bufferSize, 512, "svp_vdec_set_UseNativeBuf",1);
                        va_share_handle = pfnMtkVideoRegisterSharedMemory_Ptr(buffer, bufferSize, "svp_vdec_set_UseNativeBuf",1);
                    }
                    else //TRUSTONIC_TEE
                    {
                        // get secure buffer handle from gralloc (DEBUG ONLY)
                        if (NULL != mH264SecVdecTlcLib)
                        {
                            MtkH264SecVdec_secMemAllocate_Ptr *pfnMtkH264SecVdec_secMemAllocate = (MtkH264SecVdec_secMemAllocate_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);

                            if (NULL != pfnMtkH264SecVdec_secMemAllocate)
                            {
                                secHandle = pfnMtkH264SecVdec_secMemAllocate(1024, bufferSize);
                            }
                            else
                            {
                                MTK_OMX_LOGE("[ERROR] cannot find MtkH264SecVdec_secMemAllocate, LINE: %d", __LINE__);
                                err = OMX_ErrorUndefined;
                                goto EXIT;
                            }
                        }
                        else if (NULL != mH265SecVdecTlcLib)
                        {
                            MtkH265SecVdec_secMemAllocate_Ptr *pfnMtkH265SecVdec_secMemAllocate = (MtkH265SecVdec_secMemAllocate_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);

                            if (NULL != pfnMtkH265SecVdec_secMemAllocate)
                            {
                                secHandle = pfnMtkH265SecVdec_secMemAllocate(1024, bufferSize);
                            }
                            else
                            {
                                MTK_OMX_LOGE("[ERROR] cannot find MtkH265SecVdec_secMemAllocate, LINE: %d", __LINE__);
                                err = OMX_ErrorUndefined;
                                goto EXIT;
                            }
                        }
                        else if (NULL != mVP9SecVdecTlcLib)
                        {
                            MtkVP9SecVdec_secMemAllocate_Ptr *pfnMtkVP9SecVdec_secMemAllocate = (MtkVP9SecVdec_secMemAllocate_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);

                            if (NULL != pfnMtkVP9SecVdec_secMemAllocate)
                            {
                                secHandle = pfnMtkVP9SecVdec_secMemAllocate(1024, bufferSize);
                            }
                            else
                            {
                                MTK_OMX_LOGE("[ERROR] cannot find MtkVP9SecVdec_secMemAllocate, LINE: %d", __LINE__);
                                err = OMX_ErrorUndefined;
                                goto EXIT;
                            }
                        }
                        else
                        {
                            MTK_OMX_LOGE("[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                            err = OMX_ErrorUndefined;
                            goto EXIT;
                        }
                        MTK_OMX_LOGUD("mIonOutputBufferInfo[%d].secure_handle(0x%08X) %d", mIonOutputBufferCount, secHandle, mIonOutputBufferCount);
                    }
                    MTK_OMX_LOGU("mIonOutputBufferInfo[%d].secure_handle(0x%08X), mIonOutputBufferInfo[%d].va_share_handle(0x%08X)", mIonOutputBufferCount, secHandle, mIonOutputBufferCount, va_share_handle);
                }

                err = UseBuffer(hComp, pUseNativeBufferParams->bufferHeader, pUseNativeBufferParams->nPortIndex, pUseNativeBufferParams->pAppPrivate, bufferSize, (OMX_U8 *)buffer);

                if (err != OMX_ErrorNone)
                {
                    MTK_OMX_LOGE("[ERROR] UseBuffer failed");
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
            }
            else     // M4U
            {
                sp<android_native_buffer_t> nBuf = pUseNativeBufferParams->nativeBuffer;
                buffer_handle_t _handle = nBuf->handle;
                //MTK_OMX_LOGU ("@@ buffer_handle_t(0x%08x)", _handle);

                //IMG_native_handle_t *pGrallocHandle = (IMG_native_handle_t*) _handle;
                native_handle_t *pGrallocHandle = (native_handle_t *) _handle;
                //MTK_OMX_LOGU ("0x%08x iFormat(0x%08X)(%d)(%d)", _handle, pGrallocHandle->iFormat, pGrallocHandle->iWidth, pGrallocHandle->iHeight);
                //MTK_OMX_LOGU ("0x%08x width(%d), height(%d), stride(%d)", _handle, nBuf->width, nBuf->height, nBuf->stride);
                if (OMX_TRUE != mIsUsingNativeBuffers)
                {
                    MTK_OMX_LOGE("@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer: we are not using native buffers");
                    err = OMX_ErrorBadParameter;
                    goto EXIT;
                }

                //LOGD ("@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer");
                OMX_U8 *buffer;
                GraphicBufferMapper &gbm = GraphicBufferMapper::getInstance();
                int iRet;
                iRet = gbm.lock(_handle, GRALLOC_USAGE_SW_READ_OFTEN, Rect(nBuf->width, nBuf->height), (void **)&buffer);
                if (iRet != 0)
                {
                    MTK_OMX_LOGE("gbm->lock(...) failed: %d", iRet);
                    break;
                }

                //LOGD ("@@ buffer(0x%08x)", buffer);
                err = UseBuffer(hComp, pUseNativeBufferParams->bufferHeader, pUseNativeBufferParams->nPortIndex, pUseNativeBufferParams->pAppPrivate, mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1, buffer);

                iRet = gbm.unlock(_handle);
                if (iRet != 0)
                {
                    MTK_OMX_LOGE("gbm->unlock() failed: %d", iRet);
                    break;
                }
            }
            break;
        }
#endif

        case OMX_IndexVendorMtkOmxVideoUseIonBuffer:
        {
            //MTK_OMX_LOGU ("@@ OMX_IndexVendorMtkOmxVencUseIonBuffer");
            UseIonBufferParams *pUseIonBufferParams = (UseIonBufferParams *)pCompParam;

            if (pUseIonBufferParams->nPortIndex == mInputPortFormat.nPortIndex)
            {
                //MTK_OMX_LOGU ("@@ OMX_IndexVendorMtkOmxVideoUseIonBuffer for port[%d]", pUseIonBufferParams->nPortIndex);
                mInputUseION = OMX_TRUE;
            }
            if (pUseIonBufferParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                //MTK_OMX_LOGU ("@@ OMX_IndexVendorMtkOmxVideoUseIonBuffer for port[%d]", pUseIonBufferParams->nPortIndex);
                mOutputUseION = OMX_TRUE;
            }

            MTK_OMX_LOGUD("@@ mIsClientLocally(%d), mIsFromGralloc(%d), VA(0x%08X), FD(%d), size(%d)", mIsClientLocally, mIsFromGralloc, pUseIonBufferParams->virAddr, pUseIonBufferParams->Ionfd, pUseIonBufferParams->size);
            int ret;
            OMX_U8 *buffer;
            size_t bufferSize = pUseIonBufferParams->size;
            if (pUseIonBufferParams->nPortIndex == mInputPortFormat.nPortIndex)
            {
                ret = mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, pUseIonBufferParams->size, NULL, (void **)&buffer);
            }
            else
            {
                ret = mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, pUseIonBufferParams->size, NULL, (void **)&buffer);
            }

            if (ret < 0)
            {
                MTK_OMX_LOGE("[ERROR][ION]Allocate Node failed,line:%d\n", __LINE__);
                err = OMX_ErrorUndefined;
                goto EXIT;
            }

            err = UseBuffer(hComp, pUseIonBufferParams->bufferHeader, pUseIonBufferParams->nPortIndex, pUseIonBufferParams->pAppPrivate, bufferSize, buffer);

            if (err != OMX_ErrorNone)
            {
                MTK_OMX_LOGE("[ERROR] UseBuffer failed");
                err = OMX_ErrorUndefined;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexVendorMtkOmxVideoSetClientLocally:
        {
            OMX_CONFIG_BOOLEANTYPE *pClientLocallyInfo = (OMX_CONFIG_BOOLEANTYPE *)pCompParam;
            mIsClientLocally = pClientLocallyInfo->bEnabled;
            MTK_OMX_LOGUD("@@ mIsClientLocally(%d)", mIsClientLocally);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecNoReorderMode:
        {
            OMX_PARAM_U32TYPE *pVideoNoReorderInfo = (OMX_PARAM_U32TYPE *)pCompParam;
            MTK_OMX_LOGUD("Set No Reorder mode enable, %d", pVideoNoReorderInfo->nU32);
            mNoReorderMode = OMX_TRUE;
            break;
        }

#if (ANDROID_VER >= ANDROID_KK)
        case OMX_GoogleAndroidIndexStoreMetaDataInBuffers:
        {
            StoreMetaDataInBuffersParams *pStoreMetaDataInBuffersParams = (StoreMetaDataInBuffersParams *)pCompParam;
            if (pStoreMetaDataInBuffersParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                // return unsupport intentionally
                if (mAdaptivePlayback && !needLegacyMode())
                {

                }
                else
                {
                    err = OMX_ErrorUnsupportedIndex;
                    goto EXIT;
                }

                mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
                MTK_OMX_LOGUD("@@ mStoreMetaDataInBuffers(%d) for port(%d)", mStoreMetaDataInBuffers, pStoreMetaDataInBuffersParams->nPortIndex);
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
        case OMX_GoogleAndroidIndexPrepareForAdaptivePlayback:
        {
            PrepareForAdaptivePlaybackParams* pAdaptivePlaybackParams = (PrepareForAdaptivePlaybackParams*)pCompParam;
            if (pAdaptivePlaybackParams->nPortIndex == mOutputPortFormat.nPortIndex) {
                mEnableAdaptivePlayback = pAdaptivePlaybackParams->bEnable;
                if (mEnableAdaptivePlayback)
                {
                    //Don't trust pAdaptivePlaybackParams's max width and height.
                    //When codec don't support auto enlarge, mMaxWidth and mMaxHeight is set in ComponentInit()
                    if (supportAutoEnlarge())
                    {
                        if((pAdaptivePlaybackParams->nMaxFrameWidth > mMaxWidth) || (pAdaptivePlaybackParams->nMaxFrameHeight > mMaxHeight))
                        {
                            MTK_OMX_LOGD ("@@ keep original size, pAdaptivePlaybackParamsmMaxWidth(%d), mMaxHeight(%d), component: mMaxWidth(%d), mMaxHeight(%d)",
                                    pAdaptivePlaybackParams->nMaxFrameWidth, pAdaptivePlaybackParams->nMaxFrameHeight, mMaxWidth, mMaxHeight);
                        }
                        else
                        {
                            mMaxWidth = VDEC_ROUND_16(pAdaptivePlaybackParams->nMaxFrameWidth);
                            mMaxHeight = VDEC_ROUND_16(pAdaptivePlaybackParams->nMaxFrameHeight);
                        }
                    }
                    mLegacyMode = needLegacyMode();
                }
                MTK_OMX_LOGU ("@@ mEnableAdaptivePlayback(%d), mMaxWidth(%d), mMaxHeight(%d), Legacy(%d)",
                        mEnableAdaptivePlayback, mMaxWidth, mMaxHeight, mLegacyMode);
            }
            else {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
#endif
#if (ANDROID_VER >= ANDROID_M)
        case OMX_GoogleAndroidIndexstoreANWBufferInMetadata:
        {
            StoreMetaDataInBuffersParams *pStoreMetaDataInBuffersParams = (StoreMetaDataInBuffersParams *)pCompParam;
            if (pStoreMetaDataInBuffersParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                char value2[PROPERTY_VALUE_MAX];

                property_get("vendor.mtk.omxvdec.USANWInMetadata", value2, "0");
                OMX_BOOL  mDisableANWInMetadata = (OMX_BOOL) atoi(value2);
                if( 1 == mDisableANWInMetadata )
                {
                    MTK_OMX_LOGUD("@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata return un support by setting property");
                    err = OMX_ErrorUnsupportedIndex;
                    goto EXIT;

                }
                // return unsupport intentionally
                if (mAdaptivePlayback && !needLegacyMode())
                {

                }
                else
                {
                    err = OMX_ErrorUnsupportedIndex;
                    goto EXIT;
                }

                mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
                //MTK_OMX_LOGU("@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata");
                MTK_OMX_LOGUD("@@ mStoreMetaDataInBuffers(%d) for port(%d)", mStoreMetaDataInBuffers, pStoreMetaDataInBuffersParams->nPortIndex);
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
#endif

//ANDROID_N
        case OMX_GoogleAndroidIndexEnableAndroidNativeHandle:
        {
            EnableAndroidNativeBuffersParams *pEnableNativeBuffers = (EnableAndroidNativeBuffersParams *) pCompParam;
            //input secure only
            if (pEnableNativeBuffers->nPortIndex != mInputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGD("@@ OMX_GoogleAndroidIndexEnableAndroidNativeHandle: invalid port index");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (NULL != pEnableNativeBuffers)
            {
                MTK_OMX_LOGUD("OMX_GoogleAndroidIndexEnableAndroidNativeHandle enable(%d)", pEnableNativeBuffers->enable);
                //maybe

                mIsSecUsingNativeHandle = pEnableNativeBuffers->enable;

                if(!mIsSecUsingNativeHandle)
                {
                    err = OMX_ErrorUnsupportedSetting;
                    MTK_OMX_LOGUD("OMX_GoogleAndroidIndexEnableAndroidNativeHandle not support, enable(%d)", pEnableNativeBuffers->enable);
                    goto EXIT;
                }
            }
            else
            {
                MTK_OMX_LOGE("@@ OMX_GoogleAndroidIndexEnableAndroidNativeHandle: pEnableNativeBuffers is NULL");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            break;
        }
//endof ANDROID_N

        case OMX_IndexVendorMtkOmxVdecSkipReferenceCheckMode:
        {
            MTK_OMX_LOGUD("Sets skip reference check mode!");
            mSkipReferenceCheckMode = OMX_TRUE;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecLowLatencyDecode:
        {
            OMX_PARAM_U32TYPE *pLowLatencyDecodeInfo = (OMX_PARAM_U32TYPE *) pCompParam;
            MTK_OMX_LOGUD("Sets low latency decode mode! %d", pLowLatencyDecodeInfo->nU32);
            mLowLatencyDecodeMode = OMX_TRUE;
            break;
        }

        //#ifdef MTK_16X_SLOWMOTION_VIDEO_SUPPORT
        case OMX_IndexVendorMtkOmxVdecUse16xSlowMotion:
        {
            MTK_OMX_LOGUD("Set 16x slowmotion mode");
            mb16xSlowMotionMode = OMX_TRUE;
//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            m_fnMJCSetParam(mpMJC, MJC_PARAM_16XSLOWMOTION_MODE, &mb16xSlowMotionMode);
            }
//#endif
        }
        break;
        //#endif

        case OMX_IndexVendorMtkOmxVdecSetScenario:
        {
            OMX_PARAM_U32TYPE *pSetScenarioInfo = (OMX_PARAM_U32TYPE *) pCompParam;
            OMX_U32 tmpVal = pSetScenarioInfo->nU32;
            if (tmpVal == OMX_VIDEO_MTKSpecificScenario_ViLTE )
            {
                mViLTESupportOn = OMX_TRUE;
                SetInputPortViLTE();
                mSeqInfo.bIsMinDpbStrategy= VAL_TRUE;
                mSeqInfo.bIsMinWorkBuffer = VAL_TRUE;
            }
            else if (tmpVal == OMX_VIDEO_MTKSpecificScenario_CrossMount) { mCrossMountSupportOn = true; }
            MTK_OMX_LOGUD("@@ set vdec scenario %lu, mViLTESupportOn %d, mCrossMountSupportOn %d", tmpVal, mViLTESupportOn, mCrossMountSupportOn);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecANW_HWComposer:
        {
            if (mDecoderInitCompleteFlag == OMX_FALSE) {
                OMX_PARAM_U32TYPE *pVal = (OMX_PARAM_U32TYPE *)pCompParam;
                mANW_HWComposer = (OMX_BOOL) pVal->nU32;
                MTK_OMX_LOGUD("@@ set OMX_IndexVendorMtkOmxVdecANW_HWComposer %d", mANW_HWComposer);
            }
            break;
        }

        case OMX_IndexVendorMtkOmxVdecTaskGroup:
        {
            OMX_PARAM_U32TYPE *pTaskGroupEnable = (OMX_PARAM_U32TYPE *) pCompParam;
            OMX_U32 tmpVal = pTaskGroupEnable->nU32;
            mEnableAVTaskGroup = tmpVal > 0 ? OMX_TRUE : OMX_FALSE;
            MTK_OMX_LOGUD("@@ set av task group %u", mEnableAVTaskGroup);
            break;
        }

        default:
        {
            MTK_OMX_LOGE("MtkOmxVdec::SetParameter unsupported nParamIndex");
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVdec::GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_INDEXTYPE nParamIndex,
                                       OMX_INOUT OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("GP (0x%08X)", nParamIndex);
    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

            if (!checkOMXParams(pPortDef)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                MTK_OMX_LOGU("[INPUT] (%d)(%d)(%d)", mInputPortDef.format.video.nFrameWidth, mInputPortDef.format.video.nFrameHeight, mInputPortDef.nBufferSize);
                memcpy(pCompParam, &mInputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {

                if (mMinUndequeuedBufsFlag == OMX_TRUE)
                {
                    if (mOutputBufferPopulatedCnt == 0)
                    {
                        mOutputPortDef.nBufferCountActual += mMinUndequeuedBufsDiff;
                    }
                    else
                    {
                        mMinUndequeuedBufsDiff = 0;
                    }
                    MTK_OMX_LOGUD("nBufferCountActual %d (%i)", mOutputPortDef.nBufferCountActual, mMinUndequeuedBufsDiff);
                    mMinUndequeuedBufsFlag = OMX_FALSE;
                }

                EnableCodecDriverUFO();

                if (mForceOutputBufferCount != 0)
                {
                    mOutputPortDef.nBufferCountActual = mForceOutputBufferCount;
                }

                memcpy(pCompParam, &mOutputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

                MTK_OMX_LOGU("mOutputPortDef eColorFormat(%x), eColorFormat(%x), meDecodeType(%x), mForceOutputBufferCount(%d), mIsUsingNativeBuffers(%d)", mOutputPortDef.format.video.eColorFormat, mOutputPortFormat.eColorFormat, meDecodeType, mForceOutputBufferCount, mIsUsingNativeBuffers);

                //getParameter should get from component, should not check pPortDef and re-assign it.
                //so remove the pPortDef->format.video.eColorFormat check
                if (OMX_COLOR_FormatYUV420Planar == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        if( OMX_TRUE == mbYUV420FlexibleMode )
                        {
                            //pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_I420;
                        }
                        else
                        {
                        if (VDEC_DRV_DECODER_MTK_SOFTWARE == meDecodeType) //VPX VP9 sw
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            MTK_OMX_LOGU("[STANDARD][HAL_PIXEL_FORMAT_I420] -> HAL_PIXEL_FORMAT_YCbCr_420_888");

                            if( ( OMX_TRUE == mMJCEnable ) ||( OMX_TRUE == mDeInterlaceEnable )/*||( OMX_TRUE == mUsePPFW )*/ )
                            {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                            MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_I420] -> HAL_PIXEL_FORMAT_YUV_PRIVATE");
                            }
                        }
                        else
                        {
                            // Should not be here! MTK HW codec only output MTK YUV
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK;
//#if  ((defined MTK_CLEARMOTION_SUPPORT) || (defined MTK_POST_PROCESS_FRAMEWORK_SUPPORT) || (defined MTK_DEINTERLACE_SUPPORT))
                            if( ( OMX_TRUE == mMJCEnable ) ||( OMX_TRUE == mDeInterlaceEnable )/*||( OMX_TRUE == mUsePPFW )*/ )
                            {
                            if (mConvertYV12 == 1) {
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YV12;
                                MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12_BLK] -> HAL_PIXEL_FORMAT_YV12 1");
                            } else {
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                                MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12_BLK] -> HAL_PIXEL_FORMAT_YUV_PRIVATE");
                            }
                            }
                            else if (mQInfoOut.u4StrideAlign > 16 || mQInfoOut.u4SliceHeightAlign > 16)
                            {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            MTK_OMX_LOGU("[UNIQUE][HAL_PIXEL_FORMAT_I420] -> HAL_PIXEL_FORMAT_YCbCr_420_888");
                            }
//#endif
                        }
                    }
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        if (OMX_TRUE == mbYUV420FlexibleMode)
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_I420;
                        }
                        else
                        {

                            if (OMX_TRUE == mbIs10Bit && OMX_TRUE == mIsHorizontalScaninLSB)
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H;
                            else if (OMX_TRUE == mbIs10Bit && OMX_FALSE == mIsHorizontalScaninLSB)
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V;
                            else
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK;
//#if  ((defined MTK_CLEARMOTION_SUPPORT) || (defined MTK_POST_PROCESS_FRAMEWORK_SUPPORT) || (defined MTK_DEINTERLACE_SUPPORT))
//#ifdef MTK_SEC_VIDEO_PATH_SUPPORT
//#else
                            if( ( OMX_TRUE == mMJCEnable ) ||( OMX_TRUE == mDeInterlaceEnable )/*||( OMX_TRUE == mUsePPFW )*/ )
                            {
                                MtkVideoAllocateSecureFrameBuffer_Ptr *pfnMtkVideoAllocateSecureFrameBuffer_Ptr = NULL;
                                pfnMtkVideoAllocateSecureFrameBuffer_Ptr  =
                                (MtkVideoAllocateSecureFrameBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER);
                                if (NULL != pfnMtkVideoAllocateSecureFrameBuffer_Ptr)
                                {
                                }
                                else
                                {
                                    if (OMX_TRUE == mbIs10Bit)
                                    {
                                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT;
                                        MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12_BLK] -> HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT");
                                    }
                                    else
                                    {
                                        if (mConvertYV12 == 1) {
                                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YV12;
                                            MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12_BLK] -> HAL_PIXEL_FORMAT_YV12 2");
                                        } else {
                                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                                            MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12_BLK] -> HAL_PIXEL_FORMAT_YUV_PRIVATE");
                                        }
                                    }
                                }
                            }
//#endif
//#endif
                        }
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                        pPortDef->format.video.nStride = pPortDef->format.video.nFrameWidth;
                        pPortDef->format.video.nSliceHeight = pPortDef->format.video.nFrameHeight;
                        MTK_OMX_LOGU("UsingConverted, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV_FCM == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV_UFO == mOutputPortFormat.eColorFormat)
                {
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_UFO;
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO;
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H == mOutputPortFormat.eColorFormat)
                {
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_UFO_10BIT_H;
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V == mOutputPortFormat.eColorFormat)
                {
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_UFO_10BIT_V;
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
                    }
                }
                else if (OMX_MTK_COLOR_FormatYV12 == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        if (OMX_TRUE == mbYUV420FlexibleMode)
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_I420;
                            MTK_OMX_LOGU("[HAL_PIXEL_FORMAT_YV12] -> HAL_PIXEL_FORMAT_I420");
                        }
                        else if (VDEC_DRV_DECODER_MTK_SOFTWARE == meDecodeType) //VPX VP9 sw
                        {
                            // Video codec requires 16 align for vertical stride, not standard YV12
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            MTK_OMX_LOGU("[STANDARD][HAL_PIXEL_FORMAT_YV12] -> HAL_PIXEL_FORMAT_YCbCr_420_888");
                            if( ( OMX_TRUE == mMJCEnable ) ||( OMX_TRUE == mDeInterlaceEnable )/*||( OMX_TRUE == mUsePPFW )*/ )
                            {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                            MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_YV12] -> HAL_PIXEL_FORMAT_YUV_PRIVATE");
                            }
                        }
                        else //TBD
                        {
                            // Video codec requires 16 align for vertical stride, not standard YV12
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            MTK_OMX_LOGU("[STANDARD][HAL_PIXEL_FORMAT_YV12] -> HAL_PIXEL_FORMAT_YCbCr_420_888");
                            if( ( OMX_TRUE == mMJCEnable ) ||( OMX_TRUE == mDeInterlaceEnable )/*||( OMX_TRUE == mUsePPFW )*/ )
                            {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                            MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_YV12] -> HAL_PIXEL_FORMAT_YUV_PRIVATE");
                            }
                        }
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                        pPortDef->format.video.nStride = pPortDef->format.video.nFrameWidth;
                        pPortDef->format.video.nSliceHeight = pPortDef->format.video.nFrameHeight;
                        MTK_OMX_LOGU("UsingConverted, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                    }
                }
                else if (OMX_COLOR_FormatYUV420SemiPlanar == mOutputPortFormat.eColorFormat)  // for NV12
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers && mNativeWindowHDRInternalConvert == OMX_FALSE)
                    {
                        if (OMX_TRUE == mbYUV420FlexibleMode)
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_I420;
                            MTK_OMX_LOGU("[HAL_PIXEL_FORMAT_NV12] -> HAL_PIXEL_FORMAT_I420");
                        }
                        else if (VDEC_DRV_DECODER_MTK_SOFTWARE == meDecodeType)  // H.264
                        {
                            // Video codec requires 16 align for vertical stride, not standard YV12
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            MTK_OMX_LOGU("[STANDARD][HAL_PIXEL_FORMAT_NV12] -> HAL_PIXEL_FORMAT_YCbCr_420_888 -1");
                            if( (OMX_TRUE == mMJCEnable) || (OMX_TRUE == mDeInterlaceEnable) )
                            {
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                                MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12] -> HAL_PIXEL_FORMAT_YUV_PRIVATE -1");
                            }
                        }
                        else //TBD
                        {
                            // Video codec requires 16 align for vertical stride, not standard YV12
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                            MTK_OMX_LOGU("[STANDARD][HAL_PIXEL_FORMAT_NV12] -> HAL_PIXEL_FORMAT_YCbCr_420_888 -2");
                            if( (OMX_TRUE == mMJCEnable) || (OMX_TRUE == mDeInterlaceEnable) )
                            {
                                pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YUV_PRIVATE;
                                MTK_OMX_LOGD("[MJC][HAL_PIXEL_FORMAT_NV12] -> HAL_PIXEL_FORMAT_YUV_PRIVATE -2");
                            }
                        }
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                        pPortDef->format.video.nStride = pPortDef->format.video.nFrameWidth;
                        pPortDef->format.video.nSliceHeight = pPortDef->format.video.nFrameHeight;
                        MTK_OMX_LOGU("UsingConverted, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                    }
                }
                else if (OMX_COLOR_Format32bitARGB8888 == mOutputPortFormat.eColorFormat)
                {
                    pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_RGBA_8888;
                }
                else
                {
                    pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_I420;
                }

                if (0)//OMX_TRUE == mIsUsingNativeBuffers)
                {
                    pPortDef->format.video.nFrameWidth = pPortDef->format.video.nStride;
                    pPortDef->format.video.nFrameHeight = pPortDef->format.video.nSliceHeight;
                    MTK_OMX_LOGU("UsingNativeBuffers, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                }

#if 0 //def MTK_BASIC_PACKAGE
                MTK_OMX_LOGU("In BASIC_PACKAGE");
                if (OMX_COLOR_FormatYUV420Planar == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers)
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK;
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers)
                    {
                        if (OMX_TRUE == mbYUV420FlexibleMode)
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888;
                        }
                        else
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK;
                        }
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                        pPortDef->format.video.nStride = pPortDef->format.video.nFrameWidth;
                        pPortDef->format.video.nSliceHeight = pPortDef->format.video.nFrameHeight;
                        MTK_OMX_LOGU("UsingConverted, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                    }
                }
                else if (OMX_COLOR_FormatVendorMTKYUV_FCM == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers)
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    }
                }
                else if (OMX_MTK_COLOR_FormatYV12 == mOutputPortFormat.eColorFormat)
                {
                    // FIXME: reflect real codec output format
                    if (mIsUsingNativeBuffers)
                    {
                        if (VDEC_DRV_DECODER_MTK_SOFTWARE == meDecodeType) //VPX VP9 sw
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YV12;
                        }
                        else //TBD
                        {
                            pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YV12;
                        }
                    }
                    else
                    {
                        pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                        pPortDef->format.video.nStride = pPortDef->format.video.nFrameWidth;
                        pPortDef->format.video.nSliceHeight = pPortDef->format.video.nFrameHeight;
                        MTK_OMX_LOGU("UsingConverted, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                    }
                }
                else if (OMX_COLOR_Format32bitARGB8888 == mOutputPortFormat.eColorFormat)
                {
                    pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_RGBA_8888;
                }
                else
                {
                    pPortDef->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_I420;
                }

                if (OMX_TRUE == mIsUsingNativeBuffers)
                {
                    pPortDef->format.video.nFrameWidth = pPortDef->format.video.nStride;
                    pPortDef->format.video.nFrameHeight = pPortDef->format.video.nSliceHeight;
                    MTK_OMX_LOGU("UsingNativeBuffers, FrameWidth(%d), FrameHeight(%d)", pPortDef->format.video.nFrameWidth, pPortDef->format.video.nFrameHeight);
                }
#endif // MTK_BASIC_PACKAGE

                // Use alignment of 32x32
                unsigned int srcWStride;
                unsigned int srcHStride;
                VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoIn;
                VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;
                MTK_OMX_MEMSET(&qinfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
                MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));

                QueryDriverFormat(&qinfoIn, &qinfoOut);
                srcWStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameWidth, qinfoOut.u4StrideAlign);
                srcHStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameHeight, qinfoOut.u4SliceHeightAlign);

                if (HAL_PIXEL_FORMAT_RGBA_8888 == pPortDef->format.video.eColorFormat)
                {
                    //for MJPEG, output is RGBA
                    ((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam)->nBufferSize = (VDEC_ROUND_32(srcWStride) * VDEC_ROUND_32(srcHStride)) << 2;
                }
                else
                {
                    if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
                    {
                        ((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam)->nBufferSize = (VDEC_ROUND_32(srcWStride) * (VDEC_ROUND_32(srcHStride) + 1) * 3 >> 1);// + 16;
                    }
                    else
                    {
                        if (OMX_TRUE == mbIs10Bit)
                            ((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam)->nBufferSize = ((VDEC_ROUND_32(srcWStride) * VDEC_ROUND_32(srcHStride) * 3 >> 1) + 16)*1.25;
                        else
                            ((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam)->nBufferSize = (VDEC_ROUND_32(srcWStride) * VDEC_ROUND_32(srcHStride) * 3 >> 1) + 16;
                    }
                }

                if( meDecodeType != VDEC_DRV_DECODER_MTK_HARDWARE )
                { //for SW decoder
                    OMX_U32 totalOutputBufCnt = mOutputPortDef.nBufferCountMin;
                    eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_TOTAL_OUTPUT_BUF_SIZE, &totalOutputBufCnt, NULL);
                }

                MTK_OMX_LOGU("[OUTPUT] 32x32 Aligned! mOutputPortDef.nBufferSize(%d), nStride(%d), nSliceHeight(%d) nBufferCountActual(%d)",
                             ((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam)->nBufferSize, VDEC_ROUND_32(srcWStride), VDEC_ROUND_32(srcHStride), mOutputPortDef.nBufferCountActual);
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }
            break;
        }
        case OMX_IndexParamVideoInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = MTK_OMX_INPUT_PORT;
            pPortParam->nPorts = 2;
            break;
        }
        case OMX_IndexParamAudioInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }
        case OMX_IndexParamImageInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }
        case OMX_IndexParamOtherInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }
        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam;
            if (!checkOMXParams(pPortFormat)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                if (pPortFormat->nIndex == 0)   // note: each component only plays one role and support only one format on each input port
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_FormatUnused;
                    pPortFormat->eCompressionFormat = mInputPortFormat.eCompressionFormat;
                }
                else
                {
                    err = OMX_ErrorNoMore;
                }
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                if (pPortFormat->nIndex == 0)
                {

                    EnableCodecDriverUFO();

                    pPortFormat->eColorFormat = mOutputPortFormat.eColorFormat;
                    pPortFormat->eCompressionFormat =  mOutputPortFormat.eCompressionFormat; // must be OMX_VIDEO_CodingUnused;
                }
                //for CTS case "VideoEncoderDecoderTest"
                //push one more format(YUV420) if there is under the decoding and with MTKBLK or MTKYV12 format
                else if (pPortFormat->nIndex == 1)
                {
                    pPortFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;//mOutputPortFormat.eColorFormat;
                    pPortFormat->eCompressionFormat =  mOutputPortFormat.eCompressionFormat; // must be OMX_VIDEO_CodingUnused;
                }
                else
                {
                    err = OMX_ErrorNoMore;
                }
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }
            break;
        }
        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            strlcpy((char *)mCompRole, (char *)pRoleParams->cRole, sizeof(mCompRole));
            break;
        }

        case OMX_IndexParamVideoRv:
        {
            OMX_VIDEO_PARAM_RVTYPE *pRvType = (OMX_VIDEO_PARAM_RVTYPE *)pCompParam;
            if (!checkOMXParams(pRvType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            MTK_OMX_LOGUD("Get pRvType nPortIndex=%d", pRvType->nPortIndex);
            memcpy(pCompParam, &mRvType, sizeof(OMX_VIDEO_PARAM_RVTYPE));
            break;
        }

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pCompParam;
            if (!checkOMXParams(pProfileLevel)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            //query whether there is divx license
            if(pProfileLevel->nProfileIndex == -1) // special handling that nProfileIndex = -1 for query divx license
            {
                err = QueryDivxLicense(pProfileLevel);

                if(OMX_ErrorNone == err)
                {
                    MTK_OMX_LOGE("divx license is found for extractor\n");
                }
                else if(OMX_ErrorNoMore == err)
                {
                     MTK_OMX_LOGE("no divx license for extractor\n");
                }
                return err;
            }

            if (pProfileLevel->nPortIndex == mInputPortFormat.nPortIndex)
            {

                switch (mInputPortFormat.eCompressionFormat)
                {
                    case OMX_VIDEO_CodingHEVC:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_H265, pProfileLevel,
                                                     HevcProfileMapTable, MAX_HEVC_PROFILE_MAP_TABLE_SIZE,
                                                     HevcLevelMapTable, MAX_HEVC_LEVEL_MAP_TABLE_SIZE);
                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    case OMX_VIDEO_CodingAVC:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_H264, pProfileLevel,
                                                     AvcProfileMapTable, MAX_AVC_PROFILE_MAP_TABLE_SIZE,
                                                     AvcLevelMapTable, MAX_AVC_LEVEL_MAP_TABLE_SIZE);
                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    case OMX_VIDEO_CodingDIVX:
                    case OMX_VIDEO_CodingDIVX3:
                    case OMX_VIDEO_CodingXVID:
                    case OMX_VIDEO_CodingS263:
                    case OMX_VIDEO_CodingMPEG4:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_MPEG4, pProfileLevel,
                                                     MPEG4ProfileMapTable, MAX_MPEG4_PROFILE_MAP_TABLE_SIZE,
                                                     MPEG4LevelMapTable, MAX_MPEG4_LEVEL_MAP_TABLE_SIZE);
                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    case OMX_VIDEO_CodingH263:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_H263, pProfileLevel,
                                                     H263ProfileMapTable, MAX_H263_PROFILE_MAP_TABLE_SIZE,
                                                     H263LevelMapTable, MAX_H263_LEVEL_MAP_TABLE_SIZE);
                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    case OMX_VIDEO_CodingMPEG2:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_MPEG2, pProfileLevel,
                                                     MPEG2ProfileMapTable, MAX_MPEG2_PROFILE_MAP_TABLE_SIZE,
                                                     MPEG2LevelMapTable, MAX_MPEG2_LEVEL_MAP_TABLE_SIZE);
                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    case OMX_VIDEO_CodingVP8:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_VP8, pProfileLevel,
                                                     VP8ProfileMapTable, MAX_VP8_PROFILE_MAP_TABLE_SIZE,
                                                     VP8LevelMapTable, MAX_VP8_LEVEL_MAP_TABLE_SIZE);
                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    case OMX_VIDEO_CodingVP9:
                        err = QureyVideoProfileLevel(VDEC_DRV_VIDEO_FORMAT_VP9, pProfileLevel,
                                                    VP9ProfileMapTable, MAX_VP9_PROFILE_MAP_TABLE_SIZE,
                                                    VP9LevelMapTable, MAX_VP9_LEVEL_MAP_TABLE_SIZE);

                        if (OMX_ErrorNoMore == err)
                        {
                            goto EXIT;
                        }
                        break;

                    default:
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                }
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexVendorMtkOmxVdecVideoSpecQuerySupported:
        {
            OMX_VIDEO_PARAM_SPEC_QUERY *pSpecQuery = (OMX_VIDEO_PARAM_SPEC_QUERY *)pCompParam;
            pSpecQuery->bSupported = OMX_FALSE;
            switch (mInputPortFormat.eCompressionFormat)
            {
                case OMX_VIDEO_CodingAVC:
                    if (pSpecQuery->nFrameWidth > 1280 || pSpecQuery->nFrameHeight > 720)
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                        break;
                    }
                    if (pSpecQuery->profile == OMX_VIDEO_AVCProfileBaseline)
                    {
                        if ((pSpecQuery->level <= OMX_VIDEO_AVCLevel31) && (pSpecQuery->nFps <= 30))
                        {
                            pSpecQuery->bSupported = OMX_TRUE;
                        }
                    }
                    else if (pSpecQuery->profile == OMX_VIDEO_AVCProfileMain)
                    {
                        if ((pSpecQuery->level <= OMX_VIDEO_AVCLevel31) && (pSpecQuery->nFps <= 25))
                        {
                            pSpecQuery->bSupported = OMX_TRUE;
                        }
                    }
                    else if (pSpecQuery->profile == OMX_VIDEO_AVCProfileHigh)
                    {
                        if ((pSpecQuery->level <= OMX_VIDEO_AVCLevel31) && (pSpecQuery->nFps <= 25))
                        {
                            pSpecQuery->bSupported = OMX_TRUE;
                        }
                    }
                    else
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                    }
                    break;

                case OMX_VIDEO_CodingDIVX:
                case OMX_VIDEO_CodingDIVX3:
                case OMX_VIDEO_CodingXVID:
                case OMX_VIDEO_CodingS263:
                case OMX_VIDEO_CodingMPEG4:
                    if (pSpecQuery->nFrameWidth > 1280 || pSpecQuery->nFrameHeight > 720)
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                        break;
                    }
                    if (pSpecQuery->profile == OMX_VIDEO_MPEG4ProfileSimple)
                    {
                        if ((pSpecQuery->level <= OMX_VIDEO_MPEG4Level5) && (pSpecQuery->nFps <= 30))
                        {
                            pSpecQuery->bSupported = OMX_TRUE;
                        }
                    }
                    else if (pSpecQuery->profile == OMX_VIDEO_MPEG4ProfileAdvancedSimple)
                    {
                        if ((pSpecQuery->level <= OMX_VIDEO_MPEG4Level5) && (pSpecQuery->nFps <= 30))
                        {
                            pSpecQuery->bSupported = OMX_TRUE;
                        }
                    }
                    else
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                    }
                    break;

                case OMX_VIDEO_CodingH263:
                    if (pSpecQuery->nFrameWidth > 1280 || pSpecQuery->nFrameHeight > 720)
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                        break;
                    }
                    if (pSpecQuery->profile == OMX_VIDEO_H263ProfileBaseline)
                    {
                        if ((pSpecQuery->level <= OMX_VIDEO_H263Level70) && (pSpecQuery->nFps <= 30))
                        {
                            pSpecQuery->bSupported = OMX_TRUE;
                        }
                    }
                    else
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                    }
                    break;

                case OMX_VIDEO_CodingHEVC:
                    break;

                case OMX_VIDEO_CodingRV:
                    break;

                case OMX_VIDEO_CodingWMV:
                    break;

                case OMX_VIDEO_CodingVP8:
                    if (pSpecQuery->nFrameWidth > 1280 || pSpecQuery->nFrameHeight > 720)
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                    }
                    if (pSpecQuery->nFps > 30)
                    {
                        pSpecQuery->bSupported = OMX_FALSE;
                    }
                    break;

                case OMX_VIDEO_CodingMPEG2:
                    break;

                default:
                    goto EXIT;
            }

            err = OMX_ErrorNone;
            break;
        }

#if (ANDROID_VER >= ANDROID_ICS)
        case OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage:
        {
            GetAndroidNativeBufferUsageParams *pNativeBuffersUsage = (GetAndroidNativeBufferUsageParams *)pCompParam;
            if (pNativeBuffersUsage->nPortIndex == mOutputPortFormat.nPortIndex)
            {
#if 0//def MTK S3D SUPPORT
                switch (m3DStereoMode)
                {
                    case OMX_VIDEO_H264FPA_SIDEBYSIDE:
                        pNativeBuffersUsage->nUsage = GRALLOC_USAGE_S3D_SIDE_BY_SIDE;
                        break;
                    case OMX_VIDEO_H264FPA_TOPANDBOTTOM:
                        pNativeBuffersUsage->nUsage = GRALLOC_USAGE_S3D_TOP_AND_BOTTOM;
                        break;
                    default:
                        pNativeBuffersUsage->nUsage = 0;
                        break;
                }
#else
                pNativeBuffersUsage->nUsage = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
                if(OMX_TRUE == mIsSecureInst) {
                    pNativeBuffersUsage->nUsage |= GRALLOC_USAGE_SECURE;
                    MTK_OMX_LOGUD("OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage %d, %x", mIsSecureInst, pNativeBuffersUsage->nUsage);
                }
#endif
            }
            else
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            break;
        }
#endif

        case OMX_IndexVendorMtkOmxPartialFrameQuerySupported:
        {
            OMX_PARAM_U32TYPE *pSupportPartialFrameInfo = (OMX_PARAM_U32TYPE *)pCompParam;
            pSupportPartialFrameInfo->nU32= OMX_FALSE;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecGetColorFormat:
        {
            OMX_PARAM_U32TYPE *colorFormat = (OMX_PARAM_U32TYPE *)pCompParam;
            colorFormat->nU32= mOutputPortFormat.eColorFormat;
            //MTK_OMX_LOGU("colorFormat %lx",*colorFormat);
            break;
        }

        //alps\frameworks\base\media\jni\android_media_ImageReader.cpp
        //ImageReader_imageSetup() need YV12
        case OMX_GoogleAndroidIndexDescribeColorFormat:
        {
            DescribeColorFormatParams *describeParams = (DescribeColorFormatParams *)pCompParam;
            //OMX_COLOR_FORMATTYPE *colorFormat = (OMX_COLOR_FORMATTYPE *)pCompParam;
            //*colorFormat = describeParams.eColorFormat;
            MTK_OMX_LOGUD("DescribeColorFormat %lx, bUsingNativeBuffers %d, mbYUV420FlexibleMode %d", describeParams->eColorFormat,
                         describeParams->bUsingNativeBuffers, mbYUV420FlexibleMode);
            if ((OMX_TRUE == describeParams->bUsingNativeBuffers) &&
                ((OMX_COLOR_FormatYUV420Planar == describeParams->eColorFormat)|| (HAL_PIXEL_FORMAT_I420 == describeParams->eColorFormat)||
                (HAL_PIXEL_FORMAT_YCbCr_420_888 == describeParams->eColorFormat) || (OMX_COLOR_FormatYUV420Flexible == describeParams->eColorFormat)) )
            {
                bool err_return = 0;
                mbYUV420FlexibleMode = OMX_TRUE;
                err_return = DescribeFlexibleColorFormat((DescribeColorFormatParams *)describeParams);

                MTK_OMX_LOGUD("client query OMX_COLOR_FormatYUV420Flexible mbYUV420FlexibleMode %d, ret: %d", mbYUV420FlexibleMode, err_return);
            }
            else
            {
                //treat the framework to push YUVFlexible format in codeccodec::queryCodecs()
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }

//ANDROID_N
        case OMX_GoogleAndroidIndexDescribeColorFormat2:
        {
            DescribeColorFormat2Params *describeParams = (DescribeColorFormat2Params *)pCompParam;
            MTK_OMX_LOGUD("DescribeColorFormat2 %lx, bUsingNativeBuffers %d, mbYUV420FlexibleMode %d",
                describeParams->eColorFormat, describeParams->bUsingNativeBuffers, mbYUV420FlexibleMode);

            err = OMX_ErrorUnsupportedIndex;

            break;
        }
//ANDROID_N

        case OMX_IndexVendorMtkOmxHandle:
        {
            OMX_PARAM_U32TYPE *handleParams = (OMX_PARAM_U32TYPE *)pCompParam;
            handleParams->nU32 = (OMX_U32)this;
            break;
        }
        default:
        {
            MTK_OMX_LOGUD("MtkOmxVdec::GetParameter unsupported nParamIndex");
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGU ("MtkOmxVdec::SetConfig");

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nConfigIndex)
    {
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        case OMX_IndexVendorMtkOmxVdecPriorityAdjustment:
        {
            if (*((OMX_BOOL *)ComponentConfigStructure) == OMX_TRUE)
            {
                mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
                MTK_OMX_LOGUD("MtkOmxVdec::SetConfig -> enable priority adjustment");
            }
            else
            {
                mPropFlags &= ~MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
                MTK_OMX_LOGUD("MtkOmxVdec::SetConfig -> disable priority adjustment");
            }
            break;
        }
#endif
        case OMX_IndexVendorMtkOmxVdecSeekMode:
        {
            OMX_PARAM_S64TYPE *pSeekTargetInfo = (OMX_PARAM_S64TYPE *)ComponentConfigStructure;
            mSeekTargetTime = pSeekTargetInfo->nS64;
            MTK_OMX_LOGUD("Set seek mode enable, %lld", mSeekTargetTime);
            if (mStreamingMode == OMX_FALSE && mSeekTargetTime > 0 && mDecoderInitCompleteFlag == OMX_TRUE)
            {
                mPrepareSeek = OMX_TRUE;
                mllLastUpdateTime = mSeekTargetTime;

                if (mMJCEnable == OMX_TRUE)
                {
                    bool mSeek;
                    mSeek = true;
                    m_fnMJCSetParam(mpMJC, MJC_PARAM_SEEK, &mSeek);
                }

            }
            else
            {
                mSeekMode = OMX_FALSE;

                VDEC_DRV_MRESULT_T rResult = VDEC_DRV_MRESULT_OK;
                OMX_U32 bNeedResetCurrentTime = VAL_FALSE;
                rResult = eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_NEED_RESET_CURRENT_TIME, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&bNeedResetCurrentTime);
                MTK_OMX_LOGUD("bNeedResetCurrentTime %d", bNeedResetCurrentTime);

                if (mSeekTargetTime == 0 &&
                    bNeedResetCurrentTime && mCodecId == MTK_VDEC_CODEC_ID_AVC)
                {
                    //for 6570/80 AVC, reset current time when loop (seek back to 0). (avoid speedy mode)
                    mAVSyncTime = mSeekTargetTime;
                    mResetCurrTime = true;
                }


                if (mMJCEnable == OMX_TRUE)
                {
                    bool mSeek;
                    mSeek = false;
                    m_fnMJCSetParam(mpMJC, MJC_PARAM_SEEK, &mSeek);
                }

                if (mStreamingMode == OMX_TRUE)
                {
                    mSeekTargetTime = 0;
                }
            }
            break;
        }

        case OMX_IndexVendorMtkOmxVdecSwitchBwTVout:
        {
            VAL_BOOL_T bEnable;
            if (*((OMX_BOOL *)ComponentConfigStructure) == OMX_TRUE)
            {
                MTK_OMX_LOGUD("MtkOmxVdec::SetConfig -> Switch TV out TRUE");
                bEnable = VAL_TRUE;
                eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_SWITCH_TVOUT, (VAL_BOOL_T *)&bEnable, NULL);
            }
            else
            {
                MTK_OMX_LOGUD("MtkOmxVdec::SetConfig -> Switch TV out FALSE");
                bEnable = VAL_FALSE;
                eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_SWITCH_TVOUT, (VAL_BOOL_T *)&bEnable, NULL);
            }
            break;
        }
        case OMX_IndexVendorMtkOmxVdecAVSyncTime:
        {
            OMX_PARAM_S64TYPE *pTimeInfo = (OMX_PARAM_S64TYPE *)ComponentConfigStructure;
            int64_t time = pTimeInfo->nS64;
            //MTK_OMX_LOGUD("MtkOmxVdec::SetConfig set avsync time %lld", time);
            mAVSyncTime = time;
            break;
        }

        //#ifdef MTK_16X_SLOWMOTION_VIDEO_SUPPORT
        case OMX_IndexVendorMtkOmxVdecSlowMotionSpeed:
        {
            // Todo: Pass the slowmotion speed to MJC
            OMX_PARAM_S32TYPE *paramInfo = (OMX_PARAM_S32TYPE *)ComponentConfigStructure;
            MTK_OMX_LOGUD("Set 16x slowmotion speed(%d)", paramInfo->nS32);
            unsigned int param = paramInfo->nS32;
//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            m_fnMJCSetParam(mpMJC, MJC_PARAM_SLOWMOTION_SPEED, &param);
            }
//#endif
            break;
        }

        case OMX_IndexVendorMtkOmxVdecSlowMotionSection:
        {
            // Todo: Pass the slowmotion speed to MJC
            OMX_MTK_SLOWMOTION_SECTION *pParam = (OMX_MTK_SLOWMOTION_SECTION *)ComponentConfigStructure;
            MTK_OMX_LOGUD("Sets 16x slowmotion section(%lld ~ %lld)", pParam->StartTime, pParam->EndTime);
//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            m_fnMJCSetParam(mpMJC, MJC_PARAM_SLOWMOTION_SECTION, pParam);
            }
//#endif
            break;
        }
        //#endif
        case OMX_IndexConfigVideoNalSize:
        {
            if (ComponentConfigStructure != NULL) {
                mAssignNALSizeLength = OMX_TRUE;
                memcpy(&mNALSizeLengthInfo, (OMX_VIDEO_CONFIG_NALSIZE *)ComponentConfigStructure, sizeof(OMX_VIDEO_CONFIG_NALSIZE));
                MTK_OMX_LOGD("[MtkOmxVdec::SetConfig] nal size length is set %d", mNALSizeLengthInfo.nNaluBytes);
            }
            break;
        }

//ANDROID_N
           /*
                    // Note: the size of sAspects may increase in the future by additional fields.
                    // Implementations SHOULD NOT require a certain size.
                    struct DescribeColorAspectsParams {
                        OMX_U32 nSize;                 // IN
                        OMX_VERSIONTYPE nVersion;      // IN
                        OMX_U32 nPortIndex;            // IN
                        OMX_BOOL bRequestingDataSpace; // IN
                        OMX_BOOL bDataSpaceChanged;    // IN
                        OMX_U32 nPixelFormat;          // IN
                        OMX_U32 nDataSpace;            // OUT
                        ColorAspects sAspects;         // IN/OUT
                    };
                    ColorAspects{
                        Range mRange;                // IN/OUT
                        Primaries mPrimaries;        // IN/OUT
                        Transfer mTransfer;          // IN/OUT
                        MatrixCoeffs mMatrixCoeffs;  // IN/OUT
                    };
                    */
        case OMX_GoogleAndroidIndexDescribeColorAspects:
        {
            DescribeColorAspectsParams *pColorAspectsParams = (DescribeColorAspectsParams *)ComponentConfigStructure;

            mDescribeColorAspectsParams.nSize = pColorAspectsParams->nSize;
            mDescribeColorAspectsParams.nVersion = pColorAspectsParams->nVersion;
            mDescribeColorAspectsParams.nPortIndex = pColorAspectsParams->nPortIndex;
            mDescribeColorAspectsParams.bRequestingDataSpace = pColorAspectsParams->bRequestingDataSpace;
            mDescribeColorAspectsParams.bDataSpaceChanged = pColorAspectsParams->bDataSpaceChanged;
            mDescribeColorAspectsParams.nPixelFormat = pColorAspectsParams->nPixelFormat;
            mDescribeColorAspectsParams.nDataSpace = pColorAspectsParams->nDataSpace;
            if(OMX_TRUE == ColorAspectsDiffer(mDescribeColorAspectsParams.sAspects, pColorAspectsParams->sAspects))
            {
                HandleColorAspectChange(pColorAspectsParams->sAspects);
            }


            MTK_OMX_LOGUD("set ColorAspectsParams nSize %d, nVersion %d, nPortIndex %d, bRequestingDataSpace %d, bDataSpaceChanged %d, nPixelFormat %d, nDataSpace %d, sizeof(ColorAspects) (%d, %d, %d, %d), %d",
                mDescribeColorAspectsParams.nSize, mDescribeColorAspectsParams.nVersion,
                mDescribeColorAspectsParams.nPortIndex, mDescribeColorAspectsParams.bRequestingDataSpace,
                mDescribeColorAspectsParams.bDataSpaceChanged, mDescribeColorAspectsParams.nPixelFormat,
                mDescribeColorAspectsParams.nDataSpace,
                mDescribeColorAspectsParams.sAspects.mRange, mDescribeColorAspectsParams.sAspects.mPrimaries,
                mDescribeColorAspectsParams.sAspects.mTransfer, mDescribeColorAspectsParams.sAspects.mMatrixCoeffs,
                sizeof(ColorAspects));
            break;
        }

        /*
               struct DescribeHDRStaticInfoParams {
                    OMX_U32 nSize;                 // IN
                    OMX_VERSIONTYPE nVersion;      // IN
                    OMX_U32 nPortIndex;            // IN
                    HDRStaticInfo sInfo;           // IN/OUT
               };
               */
        case OMX_GoogleAndroidIndexDescribeHDRStaticInfo:
        {
            DescribeHDRStaticInfoParams *pHDRStaticInfoParams = (DescribeHDRStaticInfoParams *)ComponentConfigStructure;

            mDescribeHDRStaticInfoParams.nSize = pHDRStaticInfoParams->nSize;
            mDescribeHDRStaticInfoParams.nVersion = pHDRStaticInfoParams->nVersion;
            mDescribeHDRStaticInfoParams.nPortIndex = pHDRStaticInfoParams->nPortIndex;
            memcpy(&mDescribeHDRStaticInfoParams.sInfo, &pHDRStaticInfoParams->sInfo, sizeof(pHDRStaticInfoParams->sInfo));
            //mDescribeHDRStaticInfoParams.sInfo = pHDRStaticInfoParams->sInfo;

            MTK_OMX_LOGUD("set HDRStaticInfo nSize %d, nVersion %d, nPortIndex %d, sInfo %x",
                mDescribeHDRStaticInfoParams.nSize, mDescribeHDRStaticInfoParams.nVersion,
                mDescribeHDRStaticInfoParams.nPortIndex, mDescribeHDRStaticInfoParams.sInfo);
            break;
        }

//ANDROID_N

        default:
            MTK_OMX_LOGE("MtkOmxVdec::SetConfig Unknown config index: 0x%08X", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGU ("MtkOmxVdec::GetConfig");

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nConfigIndex)
    {
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        case OMX_IndexVendorMtkOmxVdecPriorityAdjustment:
        {
            OMX_BOOL *pEnableState = (OMX_BOOL *)ComponentConfigStructure;
            *pEnableState = (mPropFlags & MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT ? OMX_TRUE : OMX_FALSE);
            break;
        }
#endif

        case OMX_IndexVendorMtkOmxVdecGetAspectRatio:
        {
            OMX_PARAM_U32TYPE *pAspectRationInfo = (OMX_PARAM_U32TYPE *)ComponentConfigStructure;
            pAspectRationInfo->nU32 = (mAspectRatioWidth << 16) | mAspectRatioHeight;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecGetCropInfo:
        {
            VDEC_DRV_CROPINFO_T temp_ccop_info;
            OMX_CONFIG_RECTTYPE *pCropInfo = (OMX_CONFIG_RECTTYPE *)ComponentConfigStructure;

            memset(&temp_ccop_info, 0, sizeof(VDEC_DRV_CROPINFO_T));
//#ifdef MTK_CLEARMOTION_SUPPORT
            //mMJCScalerByPassFlag = OMX_TRUE;
            if ((mMJCEnable == OMX_TRUE) && (mUseClearMotion == OMX_TRUE) && (mMJCScalerByPassFlag == OMX_FALSE))
            {
                pCropInfo->nLeft = mCropLeft;
                pCropInfo->nTop = mCropTop;
                pCropInfo->nWidth = mCropWidth;
                pCropInfo->nHeight = mCropHeight;

                MTK_OMX_LOGUD("[HEVC][MJC] from MJC, GetCropInfo() u4CropLeft %d, u4CropRight %d, u4CropTop %d, u4CropBottom %d\n",
                             pCropInfo->nLeft, pCropInfo->nTop, pCropInfo->nWidth, pCropInfo->nHeight);
            }
            else if (mLegacyMode)
            {
                pCropInfo->nLeft = mCropLeft;
                pCropInfo->nTop = mCropTop;
                pCropInfo->nWidth = mCropWidth;
                pCropInfo->nHeight = mCropHeight;

                MTK_OMX_LOGD("MTK crop info (%d)(%d)(%d)(%d)",
                        pCropInfo->nLeft, pCropInfo->nTop, pCropInfo->nWidth, pCropInfo->nHeight);
            }
            else
//#endif
            {
                if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO, NULL, &temp_ccop_info))
                {
                    MTK_OMX_LOGUD("[WARNING] Cannot get param: VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO");
                    err = OMX_ErrorBadParameter;
                    break;
                }
                pCropInfo->nLeft = temp_ccop_info.u4CropLeft;
                pCropInfo->nTop = temp_ccop_info.u4CropTop;
                pCropInfo->nWidth = temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1;
                pCropInfo->nHeight = temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1;

                mCropLeft = pCropInfo->nLeft ;
                mCropTop = pCropInfo->nTop;
                mCropWidth = pCropInfo->nWidth;
                mCropHeight = pCropInfo->nHeight;

                MTK_OMX_LOGUD("OMX_IndexVendorMtkOmxVdecGetCropInfo GetCropInfo() from Codec, u4CropLeft %d, u4CropRight %d, u4CropTop %d, u4CropBottom %d\n",
                             temp_ccop_info.u4CropLeft, temp_ccop_info.u4CropRight, temp_ccop_info.u4CropTop, temp_ccop_info.u4CropBottom);
            }
            break;
        }

#if (ANDROID_VER >= ANDROID_KK)
        case OMX_IndexConfigCommonOutputCrop:
        {
            VDEC_DRV_CROPINFO_T temp_ccop_info;
            OMX_CONFIG_RECTTYPE *pCropInfo = (OMX_CONFIG_RECTTYPE *)ComponentConfigStructure;

            memset(&temp_ccop_info, 0, sizeof(VDEC_DRV_CROPINFO_T));
            if (pCropInfo->nPortIndex != mOutputPortDef.nPortIndex)
            {
                return OMX_ErrorUndefined;
            }
            if (mCropLeft == 0 && mCropTop == 0 && mCropWidth == 0 && mCropHeight == 0)
            {
                MTK_OMX_LOGUD("mCropWidth : %d , mCropHeight : %d", mCropWidth, mCropHeight);
                return OMX_ErrorUndefined;
            }

            if ((mMJCEnable == OMX_TRUE) && (mUseClearMotion == OMX_TRUE) && (mMJCScalerByPassFlag == OMX_FALSE))
            {
                pCropInfo->nLeft = mCropLeft;
                pCropInfo->nTop = mCropTop;
                pCropInfo->nWidth = mCropWidth;
                pCropInfo->nHeight = mCropHeight;

                MTK_OMX_LOGUD("[HEVC][MJC] from MJC, GetCropInfo() u4CropLeft %d, u4CropRight %d, u4CropTop %d, u4CropBottom %d\n",
                             pCropInfo->nLeft, pCropInfo->nTop, pCropInfo->nWidth, pCropInfo->nHeight);
            }
            else if (mLegacyMode)
            {
                pCropInfo->nLeft = mCropLeft;
                pCropInfo->nTop = mCropTop;
                pCropInfo->nWidth = mCropWidth;
                pCropInfo->nHeight = mCropHeight;

                MTK_OMX_LOGD("MTK crop info (%d)(%d)(%d)(%d)",
                        pCropInfo->nLeft, pCropInfo->nTop, pCropInfo->nWidth, pCropInfo->nHeight);
            }
            else
            {
                if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO, NULL, &temp_ccop_info))
                {
                    MTK_OMX_LOGUD("[WARNING] Cannot get param: VDEC_DRV_GET_TYPE_GET_FRAME_CROP_INFO");
                    err = OMX_ErrorBadParameter;
                    break;
                }
                pCropInfo->nLeft = temp_ccop_info.u4CropLeft;
                pCropInfo->nTop = temp_ccop_info.u4CropTop;
                pCropInfo->nWidth = temp_ccop_info.u4CropRight - temp_ccop_info.u4CropLeft + 1;
                pCropInfo->nHeight = temp_ccop_info.u4CropBottom - temp_ccop_info.u4CropTop + 1;

                mCropLeft = pCropInfo->nLeft ;
                mCropTop = pCropInfo->nTop;
                mCropWidth = pCropInfo->nWidth;
                mCropHeight = pCropInfo->nHeight;

                MTK_OMX_LOGUD("GetCropInfo() from Codec, u4CropLeft %d, u4CropRight %d, u4CropTop %d, u4CropBottom %d\n",
                             temp_ccop_info.u4CropLeft, temp_ccop_info.u4CropRight, temp_ccop_info.u4CropTop, temp_ccop_info.u4CropBottom);
            }
            break;
        }
#endif

//ANDROID_N
       /*
            // Note: the size of sAspects may increase in the future by additional fields.
            // Implementations SHOULD NOT require a certain size.
            struct DescribeColorAspectsParams {
                OMX_U32 nSize;                 // IN
                OMX_VERSIONTYPE nVersion;      // IN
                OMX_U32 nPortIndex;            // IN
                OMX_BOOL bRequestingDataSpace; // IN
                OMX_BOOL bDataSpaceChanged;    // IN
                OMX_U32 nPixelFormat;          // IN
                OMX_U32 nDataSpace;            // OUT
                ColorAspects sAspects;         // IN/OUT
            };
            */
        case OMX_GoogleAndroidIndexDescribeColorAspects:
        {
            DescribeColorAspectsParams *pColorAspectsParams = (DescribeColorAspectsParams *)ComponentConfigStructure;

            //update codec ColorAspectsParams to framework
            pColorAspectsParams->nSize = mDescribeColorAspectsParams.nSize;
            pColorAspectsParams->nVersion = mDescribeColorAspectsParams.nVersion;
            pColorAspectsParams->nPortIndex = mDescribeColorAspectsParams.nPortIndex;
            pColorAspectsParams->bRequestingDataSpace = mDescribeColorAspectsParams.bRequestingDataSpace;
            pColorAspectsParams->bDataSpaceChanged = mDescribeColorAspectsParams.bDataSpaceChanged;
            pColorAspectsParams->nPixelFormat = mDescribeColorAspectsParams.nPixelFormat;
            pColorAspectsParams->nDataSpace = mDescribeColorAspectsParams.nDataSpace;
            pColorAspectsParams->sAspects = mDescribeColorAspectsParams.sAspects;
            memcpy(&pColorAspectsParams->sAspects, &mDescribeColorAspectsParams.sAspects, sizeof(mDescribeColorAspectsParams.sAspects));

            MTK_OMX_LOGUD("get ColorAspectsParams nSize %d, nVersion %d, nPortIndex %d, bRequestingDataSpace %d, bDataSpaceChanged %d, nPixelFormat %d, nDataSpace %d, (%d, %d, %d, %d)",
                pColorAspectsParams->nSize, pColorAspectsParams->nVersion,
                pColorAspectsParams->nPortIndex, pColorAspectsParams->bRequestingDataSpace,
                pColorAspectsParams->bDataSpaceChanged, pColorAspectsParams->nPixelFormat,
                pColorAspectsParams->nDataSpace,
                pColorAspectsParams->sAspects.mRange, pColorAspectsParams->sAspects.mPrimaries,
                pColorAspectsParams->sAspects.mTransfer, pColorAspectsParams->sAspects.mMatrixCoeffs);
            break;
        }

        /*
               struct DescribeHDRStaticInfoParams {
                    OMX_U32 nSize;                 // IN
                    OMX_VERSIONTYPE nVersion;      // IN
                    OMX_U32 nPortIndex;            // IN
                    HDRStaticInfo sInfo;           // IN/OUT
               };
               */
        case OMX_GoogleAndroidIndexDescribeHDRStaticInfo:
        {

            DescribeHDRStaticInfoParams *pHDRStaticInfoParams = (DescribeHDRStaticInfoParams *)ComponentConfigStructure;

            //update codec HDRStaticInfo to framework
            pHDRStaticInfoParams->nSize = mDescribeHDRStaticInfoParams.nSize;
            pHDRStaticInfoParams->nVersion = mDescribeHDRStaticInfoParams.nVersion;
            pHDRStaticInfoParams->nPortIndex = mDescribeHDRStaticInfoParams.nPortIndex;
            memcpy(&pHDRStaticInfoParams->sInfo, &mDescribeHDRStaticInfoParams.sInfo, sizeof(mDescribeHDRStaticInfoParams.sInfo));

            MTK_OMX_LOGUD("get HDRStaticInfo nSize %d, nVersion %d, nPortIndex %d, sInfo %x",
                pHDRStaticInfoParams->nSize, pHDRStaticInfoParams->nVersion,
                pHDRStaticInfoParams->nPortIndex, pHDRStaticInfoParams->sInfo);
            break;
        }

//ANDROID_N

        case OMX_IndexVendorMtkOmxVdecComponentColorConvert:
        {
            OMX_PARAM_U32TYPE *pConvertInfo = (OMX_PARAM_U32TYPE *)ComponentConfigStructure;

            if (OMX_TRUE == mOutputAllocateBuffer ||OMX_TRUE == needColorConvertWithNativeWindow())
            {
                pConvertInfo->nU32 = OMX_TRUE;
            }
            else
            {
                pConvertInfo->nU32 = OMX_FALSE;
            }

            MTK_OMX_LOGD("ComponentColorConvert? %s", (pConvertInfo->nU32==OMX_TRUE?"yes":"no") );
            break;
        }

        case OMX_IndexVendorMtkOmxSliceLossIndication:
        {
            OMX_CONFIG_SLICE_LOSS_INDICATION *pSLI = (OMX_CONFIG_SLICE_LOSS_INDICATION *) ComponentConfigStructure;
            memcpy(pSLI, &mSLI, sizeof(OMX_CONFIG_SLICE_LOSS_INDICATION));
            break;
        }

        default:
            MTK_OMX_LOGUD("MtkOmxVdec::GetConfig Unknown config index: 0x%08X", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING parameterName,
                                            OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::GetExtensionIndex");

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_PRIORITY_ADJUSTMENT, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_PRIORITY_ADJUSTMENT)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecPriorityAdjustment;
    }
#if 0//def MTK S3D SUPPORT
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_3D_STEREO_PLAYBACK, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_3D_STEREO_PLAYBACK)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdec3DVideoPlayback;
    }
#endif
#if (ANDROID_VER >= ANDROID_ICS)
    else if (!strncmp(parameterName, "OMX.google.android.index.enableAndroidNativeBuffers", strlen("OMX.google.android.index.enableAndroidNativeBuffers")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeBuffers;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.useAndroidNativeBuffer", strlen(parameterName)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexUseAndroidNativeBuffer;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.getAndroidNativeBufferUsage", strlen("OMX.google.android.index.getAndroidNativeBufferUsage")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_STREAMING_MODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_STREAMING_MODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecStreamingMode;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorFormat", strlen("OMX.google.android.index.describeColorFormat")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexDescribeColorFormat;
    }
#endif
    else if (!strncmp(parameterName, MTK_OMX_EXTENSION_INDEX_PARTIAL_FRAME_QUERY_SUPPORTED, strlen(MTK_OMX_EXTENSION_INDEX_PARTIAL_FRAME_QUERY_SUPPORTED)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxPartialFrameQuerySupported;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SWITCH_BW_TVOUT, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SWITCH_BW_TVOUT)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecSwitchBwTVout;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_NO_REORDER_MODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_NO_REORDER_MODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecNoReorderMode;
    }
    else if (!strncmp(parameterName, "OMX.MTK.VIDEO.index.useIonBuffer", strlen("OMX.MTK.VIDEO.index.useIonBuffer")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVideoUseIonBuffer;  // Morris Yang 20130709 ION
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_FIXED_MAX_BUFFER, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_FIXED_MAX_BUFFER)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecFixedMaxBuffer;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SKIP_REFERENCE_CHECK_MODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SKIP_REFERENCE_CHECK_MODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecSkipReferenceCheckMode;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_TASK_GROUP, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_TASK_GROUP)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecTaskGroup;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.prepareForAdaptivePlayback", strlen("OMX.google.android.index.prepareForAdaptivePlayback")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexPrepareForAdaptivePlayback;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.storeMetaDataInBuffers", strlen("OMX.google.android.index.storeMetaDataInBuffers")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexStoreMetaDataInBuffers;
        // Mark for AdaptivePlayback +
        if (mAdaptivePlayback)
        {

        }
        else
        {
            MTK_OMX_LOGD("only support Legacy mode, not support storeMetaDataInBuffers");
            err = OMX_ErrorUnsupportedIndex;
        }
        // Mark for AdaptivePlayback -
    }
#if (ANDROID_VER >= ANDROID_M)
    else if (!strncmp(parameterName, "OMX.google.android.index.storeANWBufferInMetadata", strlen("OMX.google.android.index.storeANWBufferInMetadata")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexstoreANWBufferInMetadata;
        // Mark for AdaptivePlayback +
        if (mAdaptivePlayback)
        {

        }
        else
        {
            MTK_OMX_LOGD("only support Legacy mode, not support storeANWBufferInMetadata");
            err = OMX_ErrorUnsupportedIndex;
        }
        // Mark for AdaptivePlayback -
    }
#endif
//ANDROID_N
    //add in ANDROID_N
    //from frameworks\native\include\media\hardware\HardwareAPI.h
    // A pointer to this struct is passed to OMX_GetParameter when the extension
    // index for the 'OMX.google.android.index.describeColorFormat2'
    // extension is given. This is operationally the same as DescribeColorFormatParams
    // but can be used for HDR and RGBA/YUVA formats.
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorFormat2", strlen("OMX.google.android.index.describeColorFormat2")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexDescribeColorFormat2;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.allocateNativeHandle", strlen("OMX.google.android.index.allocateNativeHandle")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeHandle;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorAspects", strlen("OMX.google.android.index.describeColorAspects")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexDescribeColorAspects;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeHDRStaticInfo", strlen("OMX.google.android.index.describeHDRStaticInfo")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexDescribeHDRStaticInfo;
    }

//ANDROID_N
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_LOW_LATENCY_DECODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_LOW_LATENCY_DECODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecLowLatencyDecode;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.SetVdecScenario",
                      strlen("OMX.MTK.index.param.video.SetVdecScenario")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecSetScenario;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.ANW_HWComposer", strlen("OMX.MTK.index.param.video.ANW_HWComposer"))) {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecANW_HWComposer;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SLICE_LOSS_INDICATION, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SLICE_LOSS_INDICATION)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxSliceLossIndication;
    }
    else
    {
        MTK_OMX_LOGUD("MtkOmxVdec::GetExtensionIndex Unknown parameter name: %s", parameterName);
        err = OMX_ErrorUnsupportedIndex;
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVdec::GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (NULL == pState)
    {
        MTK_OMX_LOGE("[ERROR] MtkOmxVdec::GetState pState is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }
    *pState = mState;

    MTK_OMX_LOGUD("MtkOmxVdec::GetState (mState=%s)", StateToString(mState));

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                         OMX_IN OMX_U32 nPortIndex,
                                         OMX_IN OMX_PTR pAppPrivate,
                                         OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int ret = 0;
#if 1
    if (OMX_TRUE == mIsSecureInst && OMX_FALSE == mbShareableMemoryEnabled)
    {
        if (mTeeType == TRUSTONIC_TEE || mTeeType == BLOWFISH_TEE || mTeeType == MICROTRUST_TEE)
        {
                int fd = -1;
                fd = open("/proc/svp_region", O_RDONLY);

                if (fd == -1)
                {
                    MTK_OMX_LOGE("[ERROR] fail to open /proc/svp_region");
                }
                else
                {
                    //kernel reserve mem for sec. and tell TEE the size and address(Tehsin.Lin)
                    char *share_mem_enable = "0";
                    int vRet;
                    int vRet2;
                    vRet2 = ioctl(fd, SVP_REGION_ACQUIRE, &vRet);
                    MTK_OMX_LOGD("Need Sec Memory %d %d %d", vRet, vRet2, errno);
                    close(fd);
                    mbShareableMemoryEnabled = OMX_TRUE;
                }
        }
    }
#endif

    if (nPortIndex == mInputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mInputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mInputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("[ERROR] MtkOmxVdec::AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        mInputAllocateBuffer = OMX_TRUE;

        if (mIsSecureInst == OMX_FALSE)
        {
            mInputUseION = OMX_TRUE;
        }

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        if (OMX_TRUE == mIsSecureInst)
        {
            if (mIsSecUsingNativeHandle)//for N, Use native handle & ION
            {
                native_handle_t* native_handle = NULL;
                ion_user_handle_t ion_handle;
                int ion_share_fd;
                OMX_U32 flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC | ION_FLAG_MM_HEAP_INIT_ZERO;

                mIonDevFd = ((mIonDevFd < 0) ? eVideoOpenIonDevFd(): mIonDevFd);
                if (mIonDevFd < 0)
                {
                    MTK_OMX_LOGE("[ERROR] cannot open ION device. LINE:%d", __LINE__);
                    mIonDevFd = -1;
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }


                if (ion_alloc(mIonDevFd, nSizeBytes, 1024, ION_HEAP_MULTIMEDIA_SEC_MASK, flags, &ion_handle) != 0)
                {
                    MTK_OMX_LOGE("[ERROR] Failed to ion_alloc (%d) from mIonDevFd(%d)!\n", nSizeBytes, mIonDevFd);
                    err = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }

                ret = ion_share( mIonDevFd, ion_handle, &ion_share_fd );
                if ( ret != 0 )
                {
                    MTK_OMX_LOGE("[ERROR] ion_share(ion fd = %d) failed(%d), LINE:%d",mIonDevFd, ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                /* must config buffer before get PA or MVA */
                {
                    struct ion_mm_data mm_data;
                    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
                    mm_data.config_buffer_param.handle = ion_handle;
                    mm_data.config_buffer_param.eModuleID = 1;
                    mm_data.config_buffer_param.security = 0;
                    mm_data.config_buffer_param.coherent = 1;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &mm_data))
                    {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] Config Buffer failed!\n");
                        err = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                }

                /* get PA or MVA  */
                {
                    struct ion_sys_data sys_data;
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
                    sys_data.get_phys_param.handle = ion_handle;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] Get Phys failed!\n");
                        err = OMX_ErrorInsufficientResources;
                        goto EXIT;

                    }

                    mSecInputBufInfo[mSecInputBufCount].u4SecHandle = sys_data.get_phys_param.phy_addr;

                    if( mSecInputBufInfo[mSecInputBufCount].u4SecHandle == 0 || nSizeBytes > sys_data.get_phys_param.len)
                    {
                        MTK_OMX_LOGE("MtkH264SecVdec_AllocateIon Fail!! Allocate(%d) but get(%d)",
                             nSizeBytes, sys_data.get_phys_param.len);
                    }
                }
                native_handle = (native_handle_t*)MTK_OMX_ALLOC(sizeof(native_handle_t)+sizeof(int));
                native_handle->version = sizeof(native_handle_t);
                native_handle->numFds = 1;
                native_handle->numInts = 0;
                native_handle->data[0] = ion_share_fd;
                (*ppBufferHdr)->pBuffer = (OMX_U8 *)native_handle;

                mSecInputBufInfo[mSecInputBufCount].pNativeHandle = native_handle;
                mSecInputBufInfo[mSecInputBufCount].u4IonShareFd = ion_share_fd;
                mSecInputBufInfo[mSecInputBufCount].pIonHandle = ion_handle;
                mSecInputBufCount++;

                MTK_OMX_LOGUD("AllocateBuffer: secure pBuffer(0x%08X), secure native_handle(0x%08X), secure handle(0x%08X), IonHandle(%d), ion_share_fd(%d), mSecInputBufCount(%d)",
                 (*ppBufferHdr)->pBuffer, native_handle, mSecInputBufInfo[mSecInputBufCount-1].u4SecHandle, ion_handle, ion_share_fd, mSecInputBufCount);

            }
            else if(INHOUSE_TEE == mTeeType)
            {
                //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
                MtkVideoAllocateSecureBuffer_Ptr *pfnMtkVideoAllocateSecureBuffer_Ptr = NULL;

                pfnMtkVideoAllocateSecureBuffer_Ptr =
                (MtkVideoAllocateSecureBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER);

                if (NULL == pfnMtkVideoAllocateSecureBuffer_Ptr)
                {
                    MTK_OMX_LOGE("[ERROR] cannot find MtkVideoAllocateSecureFrameBuffer, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVideoAllocateSecureBuffer_Ptr(nSizeBytes, 512, "svp_vdec_in_AllocBuffer",1);  // allocate secure buffer from TEE
                //#endif
            }
            else
            {
                if (NULL != mH264SecVdecTlcLib)
                {
                    MtkH264SecVdec_secMemAllocateTBL_Ptr *pfnMtkH264SecVdec_secMemAllocateTBL = (MtkH264SecVdec_secMemAllocateTBL_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME);
                    if (NULL != pfnMtkH264SecVdec_secMemAllocateTBL)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH264SecVdec_secMemAllocateTBL(1024, nSizeBytes); // for t-play
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkH264SecVdec_secMemAllocateTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mH265SecVdecTlcLib)
                {
                    MtkH265SecVdec_secMemAllocateTBL_Ptr *pfnMtkH265SecVdec_secMemAllocateTBL = (MtkH265SecVdec_secMemAllocateTBL_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME);
                    if (NULL != pfnMtkH265SecVdec_secMemAllocateTBL)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH265SecVdec_secMemAllocateTBL(1024, nSizeBytes); // for t-play
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkH265SecVdec_secMemAllocateTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mVP9SecVdecTlcLib)
                {
                    MtkVP9SecVdec_secMemAllocateTBL_Ptr *pfnMtkVP9SecVdec_secMemAllocateTBL = (MtkVP9SecVdec_secMemAllocateTBL_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME);
                    if (NULL != pfnMtkVP9SecVdec_secMemAllocateTBL)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVP9SecVdec_secMemAllocateTBL(1024, nSizeBytes); // for t-play
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkVP9SecVdec_secMemAllocateTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else
                {
                    MTK_OMX_LOGE("[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
            }
            MTK_OMX_LOGUD("AllocateBuffer hSecureHandle = 0x%08X", (*ppBufferHdr)->pBuffer);
        }
        else
        {
            if (OMX_FALSE == mInputUseION)
            {
                (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_MEMALIGN(MEM_ALIGN_64, nSizeBytes); //(OMX_U8*)MTK_OMX_ALLOC(nSizeBytes);  // allocate input from dram
            }
        }

        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //for ACodec color convert
        (*ppBufferHdr)->pPlatformPrivate = NULL;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        if (OMX_FALSE == mIsSecureInst)
        {

            if (OMX_TRUE == mInputUseION)
            {
                ret = mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                                   (void *)(*ppBufferHdr), (void **)(&(*ppBufferHdr)->pBuffer));
                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR] Allocate ION Buffer failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
                mIonInputBufferCount++;
            }
            else    // M4U
            {
                if (strncmp("m4u", mInputMVAMgr->getType(), strlen("m4u")))
                {
                    //if not m4u map
                    delete mInputMVAMgr;
                    mInputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec1");
                }
                ret = mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                                   (void *)(*ppBufferHdr), (void **)(&(*ppBufferHdr)->pBuffer));
                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR] Allocate M4U failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
                mM4UBufferCount++;
            }

        }


        //MTK_OMX_LOGU ("UB port (0x%X), idx[%d] (0x%08X)(0x%08X)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)pBuffer);
        MTK_OMX_LOGUD("MtkOmxVdec::AllocateBuffer In port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), mInputUseION(%d)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)((*ppBufferHdr)->pBuffer), mInputUseION);


        InsertInputBuf(*ppBufferHdr);

        mInputBufferPopulatedCnt++;

        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            CLEAR_SEMAPHORE(mInPortFreeDoneSem);
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            MTK_OMX_LOGD("AllocateBuffer:: input port populated");
        }
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mOutputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mOutputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("[ERROR] MtkOmxVdec::AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        mOutputAllocateBuffer = OMX_TRUE;

        if (mIsSecureInst == OMX_FALSE)
        {
            mOutputUseION = OMX_TRUE;
        }

        if (0 == mOutputBufferPopulatedCnt)
        {
            size_t arraySize = sizeof(FrmBufStruct) * MAX_TOTAL_BUFFER_CNT;
            MTK_OMX_MEMSET(mFrameBuf, 0x00, arraySize);
            MTK_OMX_MEMSET(mFrameBufInt, 0x00, arraySize);
            MTK_OMX_LOGUD("AllocateBuffer:: clear mFrameBuf");
        }

        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        OMX_U32 u4BufferVa;
        OMX_U32 u4BufferPa;

        if (OMX_TRUE == mIsSecureInst)
        {

            if (INHOUSE_TEE == mTeeType)
            {
                //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
                //MtkVideoAllocateSecureBuffer_Ptr *pfnMtkVideoAllocateSecureBuffer_Ptr = (MtkVideoAllocateSecureBuffer_Ptr *) dlsym(mH264SecVdecInHouseLib, MTK_H264_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_BUFFER);

                MtkVideoAllocateSecureBuffer_Ptr *pfnMtkVideoAllocateSecureBuffer_Ptr = NULL;

                pfnMtkVideoAllocateSecureBuffer_Ptr =
                (MtkVideoAllocateSecureBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER);

                if (NULL == pfnMtkVideoAllocateSecureBuffer_Ptr)
                {
                    MTK_OMX_LOGE("[ERROR] cannot find MtkVideoAllocateSecureFrameBuffer, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVideoAllocateSecureBuffer_Ptr(nSizeBytes, 512, "svp_vdec_out_AllocBuffer",1);  // allocate secure buffer from TEE
                //#endif
            }
            else
            {
                if (NULL != mH264SecVdecTlcLib)
                {
                    MtkH264SecVdec_secMemAllocate_Ptr *pfnMtkH264SecVdec_secMemAllocate = (MtkH264SecVdec_secMemAllocate_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);
                    if (NULL != pfnMtkH264SecVdec_secMemAllocate)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH264SecVdec_secMemAllocate(1024, nSizeBytes); //MtkVdecAllocateSecureFrameBuffer(nSizeBytes, 512);   // allocate secure buffer from TEE
                        mIsSecTlcAllocOutput = OMX_TRUE;
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkH264SecVdec_secMemAllocate, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mH265SecVdecTlcLib)
                {
                    MtkH265SecVdec_secMemAllocate_Ptr *pfnMtkH265SecVdec_secMemAllocate = (MtkH265SecVdec_secMemAllocate_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);
                    if (NULL != pfnMtkH265SecVdec_secMemAllocate)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH265SecVdec_secMemAllocate(1024, nSizeBytes); //MtkVdecAllocateSecureFrameBuffer(nSizeBytes, 512);   // allocate secure buffer from TEE
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkH265SecVdec_secMemAllocate, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mVP9SecVdecTlcLib)
                {
                    MtkVP9SecVdec_secMemAllocate_Ptr *pfnMtkVP9SecVdec_secMemAllocate = (MtkVP9SecVdec_secMemAllocate_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);
                    if (NULL != pfnMtkVP9SecVdec_secMemAllocate)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVP9SecVdec_secMemAllocate(1024, nSizeBytes); //MtkVdecAllocateSecureFrameBuffer(nSizeBytes, 512);   // allocate secure buffer from TEE
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkVP9SecVdec_secMemAllocate, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else
                {
                    MTK_OMX_LOGE("[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
            }

            MTK_OMX_LOGUD("AllocateBuffer hSecureHandle = 0x%08X", (*ppBufferHdr)->pBuffer);
        }
        else
        {
            if (OMX_FALSE == mStoreMetaDataInBuffers && nSizeBytes < (mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight * 3 / 2))
            {
                MTK_OMX_LOGE ("Error in MtkOmxVdec::AllocateBuffer, output size incorrect %d, LINE:%d", nSizeBytes, __LINE__);
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (OMX_TRUE == mStoreMetaDataInBuffers)
            {
                u4BufferVa = (OMX_U32)MTK_OMX_MEMALIGN(MEM_ALIGN_512, nSizeBytes);//(OMX_U32)MTK_OMX_ALLOC(nSizeBytes);  // allocate input from dram
            }

        }

        (*ppBufferHdr)->pBuffer = (OMX_U8 *)u4BufferVa;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        (*ppBufferHdr)->nOffset = 0;
        //for ACodec color convert
        (*ppBufferHdr)->pPlatformPrivate = NULL;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD

#if (ANDROID_VER >= ANDROID_KK)
        if (OMX_FALSE == mStoreMetaDataInBuffers && OMX_FALSE == mIsSecureInst)
        {
#endif
            if (OMX_TRUE == mOutputUseION)
            {
                ret = mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                                    (void *)*ppBufferHdr, (void **)(&(*ppBufferHdr)->pBuffer));
                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR] Allocate ION Buffer failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
                mIonOutputBufferCount++;
                MTK_OMX_LOGD("[AllocateBuffer] mIonOutputBufferCount (%d), u4BuffHdr(0x%08x), LINE:%d", mIonOutputBufferCount, (*ppBufferHdr), __LINE__);
            }
            else // M4U
            {
                if (strncmp("m4u", mOutputMVAMgr->getType(), strlen("m4u")))
                {
                    //if not m4u map
                    delete mOutputMVAMgr;
                    mOutputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec1");
                }
                ret = mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                                    (void *)(*ppBufferHdr), (void **)(&(*ppBufferHdr)->pBuffer));

                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR] Allocate M4U failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorBadParameter;
                    goto EXIT;
                }

                mM4UBufferCount++;
            }

            MTK_OMX_LOGUD("MtkOmxVdec::AllocateBuffer Out port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), mOutputUseION(%d)",
                         (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt], (unsigned int)((*ppBufferHdr)->pBuffer), mOutputUseION);

            InsertFrmBuf(*ppBufferHdr);

            // reset all buffer to black
            if (mOutputUseION == OMX_FALSE)
            {
                OMX_U32 u4PicAllocSize = mOutputPortDef.format.video.nSliceHeight * mOutputPortDef.format.video.nStride;
                //MTK_OMX_LOGU ("mOutputUseION:false, u4PicAllocSize %d", u4PicAllocSize);
                memset((*ppBufferHdr)->pBuffer + u4PicAllocSize, 128, u4PicAllocSize / 2);
                memset((*ppBufferHdr)->pBuffer, 0x10, u4PicAllocSize);
            }
            else
            {
                OMX_U32 u4PicAllocSize = mOutputPortDef.format.video.nSliceHeight * mOutputPortDef.format.video.nStride;
                //MTK_OMX_LOGU ("mOutputUseION:true, u4PicAllocSize %d", u4PicAllocSize);
                memset((*ppBufferHdr)->pBuffer + u4PicAllocSize, 128, u4PicAllocSize / 2);
                memset((*ppBufferHdr)->pBuffer, 0x10, u4PicAllocSize);
            }

#if (ANDROID_VER >= ANDROID_KK)
        }
        else
        {
            InsertFrmBuf(*ppBufferHdr);
        }
#endif

        mOutputBufferPopulatedCnt++;
        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            CLEAR_SEMAPHORE(mOutPortFreeDoneSem);
            mOutputStrideBeforeReconfig = mOutputPortDef.format.video.nStride;
            mOutputSliceHeightBeforeReconfig = mOutputPortDef.format.video.nSliceHeight;
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            MTK_OMX_LOGD("AllocateBuffer:: output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes,
                                    OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int ret = 0;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mInputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mInputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("[ERROR] MtkOmxVdec::UseBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //for ACodec color convert
        (*ppBufferHdr)->pPlatformPrivate = NULL;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        if (OMX_TRUE == mInputUseION)
        {
            mIonInputBufferCount++;
        }
        else    // M4U
        {
            if (strncmp("m4u", mInputMVAMgr->getType(), strlen("m4u")))
            {
                //if not m4u map
                delete mInputMVAMgr;
                mInputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec1");
            }
            ret = mInputMVAMgr->newOmxMVAwithVA((void *)pBuffer, (int)nSizeBytes, (void *)(*ppBufferHdr));
            if (ret < 0)
            {
                MTK_OMX_LOGE("[ERROR] Allocate M4U failed (%d), LINE:%d", ret, __LINE__);
            }
        }

        //MTK_OMX_LOGU ("UB port (0x%X), idx[%d] (0x%08X)(0x%08X)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)pBuffer);
        MTK_OMX_LOGUD("UB port (0x%X), idx[%d] (0x%08X)(0x%08X), mInputUseION(%d)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)pBuffer, mInputUseION);


        InsertInputBuf(*ppBufferHdr);

        mInputBufferPopulatedCnt++;
        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            CLEAR_SEMAPHORE(mInPortFreeDoneSem);
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            MTK_OMX_LOGU("input port populated");
        }
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mOutputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mOutputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("[ERROR] MtkOmxVdec::UseBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        if (0 == mOutputBufferPopulatedCnt)
        {
            size_t arraySize = sizeof(FrmBufStruct) * MAX_TOTAL_BUFFER_CNT;
            MTK_OMX_MEMSET(mFrameBuf, 0x00, arraySize);
            MTK_OMX_MEMSET(mFrameBufInt, 0x00, arraySize);
            MTK_OMX_LOGUD("UseBuffer:: clear mFrameBuf");
        }
        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        (*ppBufferHdr)->pPlatformPrivate = NULL;
#if (ANDROID_VER >= ANDROID_KK)
        if (OMX_FALSE == mStoreMetaDataInBuffers)
        {
#endif
            if (OMX_TRUE == mOutputUseION)
            {
                mIonOutputBufferCount++;
                MTK_OMX_LOGUD("[UseBuffer] mIonOutputBufferCount (%d), u4BuffHdr(0x%08x), LINE:%d", mIonOutputBufferCount, (*ppBufferHdr), __LINE__);
            }

            else if (OMX_TRUE == mIsSecureInst)
            {
                mSecFrmBufInfo[mSecFrmBufCount].u4BuffHdr = (VAL_UINT32_T)(*ppBufferHdr);
                mSecFrmBufCount++;
            }

            else // M4U
            {
                if (strncmp("m4u", mOutputMVAMgr->getType(), strlen("m4u")))
                {
                    //if not m4u map
                    delete mOutputMVAMgr;
                    mOutputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec2");
                }
                ret = mOutputMVAMgr->newOmxMVAwithVA((void *)pBuffer, (int)nSizeBytes, (void *)(*ppBufferHdr));
                if (ret < 0)
                {
                    MTK_OMX_LOGE("[ERROR] Allocate M4U failed (%d), LINE:%d", ret, __LINE__);
                }
            }

            //MTK_OMX_LOGU ("MtkOmxVdec::UseBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt], (unsigned int)pBuffer);
            MTK_OMX_LOGUD("UB port (0x%X), idx[%d] (0x%08X)(0x%08X), mOutputUseION(%d)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt], (unsigned int)pBuffer, mOutputUseION);

            InsertFrmBuf(*ppBufferHdr);

            //suggest checking the picSize and nSizeBytes first
            if (mIsSecureInst == OMX_TRUE)    // don't do this
            {
            }
            else
            {
                OMX_U32 u4PicAllocSize = mOutputPortDef.format.video.nSliceHeight * mOutputPortDef.format.video.nStride;
                MTK_OMX_LOGUD("mOutputUseION: %d, u4PicAllocSize %d", mOutputUseION, u4PicAllocSize);
                memset((*ppBufferHdr)->pBuffer + u4PicAllocSize, 128, u4PicAllocSize / 2);
                memset((*ppBufferHdr)->pBuffer, 0x10, u4PicAllocSize);
            }

#if (ANDROID_VER >= ANDROID_KK)
//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            if ((mUseClearMotion && (mInputPortFormat.eCompressionFormat != OMX_VIDEO_CodingAVC) && (mInputPortFormat.eCompressionFormat != OMX_VIDEO_CodingHEVC))
                || (mUseClearMotion && (OMX_TRUE == mSeqInfoCompleteFlag) && ((mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) || (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC))))
            {
                if (mOutputBufferPopulatedCnt < MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT)
                {
                    VAL_UINT32_T u4I, u4BufferCount;

                    u4BufferCount = mIonOutputBufferCount;
                    VBufInfo BufInfo;
                    if (mOutputMVAMgr->getOmxInfoFromVA((void *)(*ppBufferHdr)->pBuffer, &BufInfo) < 0)
                    {
                        MTK_OMX_LOGE("[MJC] [UseBuffer][MJC BUFFER][ERROR]\n");
                        MTK_OMX_LOGE("[ERROR] Can't find Frm in mOutputMVAMgr");
                        //return OMX_FALSE;
                    }
                    else
                    {
                        mMJCBufferInfo[mMJCBufferCount].u4VA = BufInfo.u4VA;
                        mMJCBufferInfo[mMJCBufferCount].u4PA = BufInfo.u4PA;
                        mMJCBufferInfo[mMJCBufferCount].u4Size = BufInfo.u4BuffSize;
                        mMJCBufferInfo[mMJCBufferCount].u4Hdr = (VAL_UINT32_T)(*ppBufferHdr);
                        if (mMJCLog)
                        {
                            MTK_OMX_LOGUD("[MJC] [UseBuffer][MJC BUFFER][%d] Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X",
                                         mMJCBufferCount,
                                         mMJCBufferInfo[mMJCBufferCount].u4VA,
                                         mMJCBufferInfo[mMJCBufferCount].u4PA,
                                         mMJCBufferInfo[mMJCBufferCount].u4Size,
                                         mMJCBufferInfo[mMJCBufferCount].u4Hdr
                                        );
                        }

                        mMJCBufferCount++;
                    }
                }
                else
                {
                    // For Scaler ClearMotion +
                    if (mOutputBufferPopulatedCnt < TOTAL_MJC_BUFFER_CNT)
                    {
                        VAL_UINT32_T u4I, u4BufferCount;

                        u4BufferCount = mIonOutputBufferCount;
                        VBufInfo BufInfo;
                        if (mOutputMVAMgr->getOmxInfoFromVA((void *)(*ppBufferHdr)->pBuffer, &BufInfo) < 0)
                        {
                            MTK_OMX_LOGE("[MJC] [UseBuffer][MJC BUFFER][ERROR]\n");
                            MTK_OMX_LOGE("[ERROR] Can't find Frm in mOutputMVAMgr");
                            //return OMX_FALSE;
                        }
                        else
                        {
                            mScalerBufferInfo[mScalerBufferCount].u4VA = BufInfo.u4VA;
                            mScalerBufferInfo[mScalerBufferCount].u4PA = BufInfo.u4PA;
                            mScalerBufferInfo[mScalerBufferCount].u4Size = BufInfo.u4BuffSize;
                            mScalerBufferInfo[mScalerBufferCount].u4Hdr = (VAL_UINT32_T)(*ppBufferHdr);
                            if (mMJCLog)
                            {
                                MTK_OMX_LOGUD("[MJC] [UseBuffer][SCALAR BUFFER][%d] Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X",
                                             mScalerBufferCount,
                                             mScalerBufferInfo[mScalerBufferCount].u4VA,
                                             mScalerBufferInfo[mScalerBufferCount].u4PA,
                                             mScalerBufferInfo[mScalerBufferCount].u4Size,
                                             mScalerBufferInfo[mScalerBufferCount].u4Hdr
                                            );
                            }

                            mScalerBufferCount++;

                        }
                    }
                    // For Scaler ClearMotion -
                }
            }
            }
//#endif // CLEAR_MOTION
        }
        else
        {
            InsertFrmBuf(*ppBufferHdr);
        }
#endif

        mOutputBufferPopulatedCnt++;

        {
            buffer_handle_t _handle = NULL;
            //ret = mOutputMVAMgr->getMapHndlFromIndex(mOutputBufferPopulatedCnt - 1, &_handle);
            VBufInfo bufInfo;
            ret = mOutputMVAMgr->getOmxInfoFromVA(pBuffer, &bufInfo);
            _handle = (buffer_handle_t)bufInfo.pNativeHandle;

            if ((ret > 0) && (_handle != NULL))
            {
                gralloc_extra_ion_sf_info_t sf_info;
                //MTK_OMX_LOGU ("gralloc_extra_query");
                memset(&sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));

                gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

                sf_info.pool_id = (int32_t)this;  //  for PQ to identify bitstream instance.

                gralloc_extra_sf_set_status(&sf_info,
                                            GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
                                            GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);

                gralloc_extra_perform(_handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
                //MTK_OMX_LOGU ("gralloc_extra_perform");
            }
        }

        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            CLEAR_SEMAPHORE(mOutPortFreeDoneSem);
            mOutputStrideBeforeReconfig = mOutputPortDef.format.video.nStride;
            mOutputSliceHeightBeforeReconfig = mOutputPortDef.format.video.nSliceHeight;
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGUD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            MTK_OMX_LOGU("output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }
    return err;

EXIT:
    MTK_OMX_LOGE("UseBuffer return err %x", err);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGUD ("MtkOmxVdec::FreeBuffer nPortIndex(%d)", nPortIndex);
    OMX_BOOL bAllowFreeBuffer = OMX_FALSE;

    //MTK_OMX_LOGU ("@@ mState=%d, Is LOADED PENDING(%d), Is IDLE PENDING(%d)", mState, IS_PENDING (MTK_OMX_LOADED_PENDING), IS_PENDING (MTK_OMX_IDLE_PENDING));
    if (mState == OMX_StateExecuting || mState == OMX_StateIdle || mState == OMX_StatePause)
    {
        if (((nPortIndex == MTK_OMX_INPUT_PORT) && (mInputPortDef.bEnabled == OMX_FALSE)) ||
            ((nPortIndex == MTK_OMX_OUTPUT_PORT) && (mOutputPortDef.bEnabled == OMX_FALSE)))      // in port disabled case, p.99
        {
            bAllowFreeBuffer = OMX_TRUE;
        }
        else if ((mState == OMX_StateIdle) && (IS_PENDING(MTK_OMX_LOADED_PENDING)))        // de-initialization, p.128
        {
            bAllowFreeBuffer = OMX_TRUE;
        }
        else
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   NULL,
                                   NULL);
            err = OMX_ErrorPortUnpopulated;
            goto EXIT;
        }
    }
    else if ((mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        bAllowFreeBuffer = OMX_TRUE;
    }

    if ((nPortIndex == MTK_OMX_INPUT_PORT) && bAllowFreeBuffer)
    {

        if (OMX_TRUE == mIsSecureInst)
        {
            MTK_OMX_LOGUD("FreeBuffer: hSecureHandle(0x%08X)", pBuffHead->pBuffer);
            OMX_U32 i = 0;

            if(mIsSecUsingNativeHandle)
            {
                if (mIonDevFd < 0)
                {
                    MTK_OMX_LOGE("[ERROR] cannot open ION device. LINE:%d", __LINE__);
                    mIonDevFd = -1;
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                for(i = 0; i < mSecInputBufCount; i++)
                {

                    if(pBuffHead->pBuffer == mSecInputBufInfo[i].pNativeHandle)
                    {
                        ion_share_close(mIonDevFd, mSecInputBufInfo[i].u4IonShareFd);

                        if (ion_free(mIonDevFd, mSecInputBufInfo[i].pIonHandle))
                        {

                            MTK_OMX_LOGE("[ION_FREE] Failed: hSecureHandle(0x%08X), secure native_handle(0x%08X), IonHandle(%d), ion_share_fd(%d)\n",
                              pBuffHead->pBuffer, mSecInputBufInfo[i].pNativeHandle, mSecInputBufInfo[i].pIonHandle, mSecInputBufInfo[i].u4IonShareFd);
                            err = OMX_ErrorUndefined;
                            goto EXIT;

                        }
                        MTK_OMX_LOGD("FreeBuffer: hSecureHandle(0x%08X), secure native_handle(0x%08X), IonHandle(%d), ion_share_fd(%d))",
                             pBuffHead->pBuffer, mSecInputBufInfo[i].pNativeHandle, mSecInputBufInfo[i].pIonHandle, mSecInputBufInfo[i].u4IonShareFd);

                        MTK_OMX_FREE(mSecInputBufInfo[i].pNativeHandle);
                        mSecInputBufInfo[i].u4IonShareFd = 0xffffffff;
                        mSecInputBufInfo[i].pIonHandle = 0xffffffff;
                        mSecInputBufInfo[i].pNativeHandle = (void *)0xffffffff;
                        mSecInputBufInfo[i].u4SecHandle = 0xffffffff;

                        break;
                    }
                }

                if(mSecInputBufCount == i)
                {
                    MTK_OMX_LOGE("[FreeBuffer][Error]: Cannot found hSecureHandle(0x%08X)\n", pBuffHead->pBuffer);
                    err = OMX_ErrorUndefined;

                }

            }
            else if (INHOUSE_TEE == mTeeType)
            {
                //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
                //MtkVideoFreeSecureBuffer_Ptr *pfnMtkVideoFreeSecureBuffer_Ptr = (MtkVideoFreeSecureBuffer_Ptr *) dlsym(mH264SecVdecInHouseLib, MTK_H264_SEC_VDEC_IN_HOUSE_FREE_SECURE_BUFFER);

                MtkVideoFreeSecureBuffer_Ptr *pfnMtkVideoFreeSecureBuffer_Ptr = NULL;

                pfnMtkVideoFreeSecureBuffer_Ptr =
                (MtkVideoFreeSecureBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_FREE_SECURE_FRAME_BUFFER);

                if (NULL == pfnMtkVideoFreeSecureBuffer_Ptr)
                {
                    MTK_OMX_LOGE("[ERROR] cannot find MtkVideoFreeSecureFrameBuffer, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                if (MTK_SECURE_AL_SUCCESS != pfnMtkVideoFreeSecureBuffer_Ptr((OMX_U32)(pBuffHead->pBuffer),1))
                {
                    MTK_OMX_LOGE("MtkVideoFreeSecureBuffer failed, line:%d\n", __LINE__);
                }
                //#endif
            }
            else
            {

                if (NULL != mH264SecVdecTlcLib)
                {
                    MtkH264SecVdec_secMemFreeTBL_Ptr *pfnMtkH264SecVdec_secMemFreeTBL = (MtkH264SecVdec_secMemFreeTBL_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_FREE_TBL_NAME);
                    if (NULL != pfnMtkH264SecVdec_secMemFreeTBL)
                    {
                        if (pfnMtkH264SecVdec_secMemFreeTBL((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                        {
                            MTK_OMX_LOGE("MtkH264SecVdec_secMemFreeTBL failed, line:%d\n", __LINE__);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkH264SecVdec_secMemFreeTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mH265SecVdecTlcLib)
                {
                    MtkH265SecVdec_secMemFreeTBL_Ptr *pfnMtkH265SecVdec_secMemFreeTBL = (MtkH265SecVdec_secMemFreeTBL_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_FREE_TBL_NAME);
                    if (NULL != pfnMtkH265SecVdec_secMemFreeTBL)
                    {
                        if (pfnMtkH265SecVdec_secMemFreeTBL((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                        {
                            MTK_OMX_LOGE("MtkH265SecVdec_secMemFreeTBL failed, line:%d\n", __LINE__);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkH265SecVdec_secMemFreeTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mVP9SecVdecTlcLib)
                {
                    MtkVP9SecVdec_secMemFreeTBL_Ptr *pfnMtkVP9SecVdec_secMemFreeTBL = (MtkVP9SecVdec_secMemFreeTBL_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_FREE_TBL_NAME);
                    if (NULL != pfnMtkVP9SecVdec_secMemFreeTBL)
                    {
                        if (pfnMtkVP9SecVdec_secMemFreeTBL((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                        {
                            MTK_OMX_LOGE("MtkVP9SecVdec_secMemFreeTBL failed, line:%d\n", __LINE__);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE("[ERROR] cannot find MtkVP9SecVdec_secMemFreeTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else
                {
                    MTK_OMX_LOGE("[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

            }
        }
        else
        {
            int ret = mInputMVAMgr->freeOmxMVAByVa((void *)pBuffHead->pBuffer);
            if (ret < 0)
            {
                MTK_OMX_LOGE("[ERROR][Input][FreeBuffer]\n");
            }
        }

        RemoveInputBuf(pBuffHead);
        // free input buffers
        for (OMX_U32 i = 0 ; i < mInputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mInputBufferHdrs[i])
            {
                MTK_OMX_LOGD("FB in (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mInputBufferHdrs[i]);
                mInputBufferHdrs[i] = NULL;
                mInputBufferPopulatedCnt--;
            }
        }

        if (mInputBufferPopulatedCnt == 0)       // all input buffers have been freed
        {
            if ((OMX_TRUE == mInputUseION))
            {
                mIonInputBufferCount = 0;
            }
            mInputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mInPortFreeDoneSem);
            MTK_OMX_LOGU("MtkOmxVdec::FreeBuffer all input buffers have been freed!!! signal mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
        }

        if ((mInputPortDef.bEnabled == OMX_TRUE) && (mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   NULL,
                                   NULL);
        }
    }

    if ((nPortIndex == MTK_OMX_OUTPUT_PORT) && bAllowFreeBuffer)
    {

#if (ANDROID_VER >= ANDROID_KK)
        if (OMX_FALSE == mStoreMetaDataInBuffers)
        {
#endif
            if(mIsSecureInst == OMX_TRUE && mIsSecTlcAllocOutput == OMX_TRUE && NULL != mH264SecVdecTlcLib)
            {
                MtkH264SecVdec_secMemFree_Ptr *pfnMtkH264SecVdec_secMemFree = (MtkH264SecVdec_secMemFree_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_FREE_NAME);
                if (NULL != pfnMtkH264SecVdec_secMemFree)
                {
                    if (pfnMtkH264SecVdec_secMemFree((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                    {
                        MTK_OMX_LOGE("MtkH264SecVdec_secMemFree failed, line:%d\n", __LINE__);
                    }
                }
                else
                {
                    MTK_OMX_LOGE("[ERROR] cannot find MtkH264SecVdec_secMemFree, LINE: %d", __LINE__);
                }
            }
            else
            {
                int ret = mOutputMVAMgr->freeOmxMVAByVa((void *)pBuffHead->pBuffer);
                if (ret < 0)
                {
                    MTK_OMX_LOGUD("[WARNING][Output][FreeBuffer], pBuffer %x, LINE: %d\n", pBuffHead->pBuffer, __LINE__);
                }
            }
//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            if (mUseClearMotion || mMJCBufferCount > 0 || mScalerBufferCount > 0)
            {
                if (mMJCBufferCount > 0 || mScalerBufferCount > 0)
                {
                    //MTK_OMX_LOGU("[MJC] [FreeBuffer][MJC BUFFER] mUseClearMotion (%d). free buffer(%d)(%d)", mUseClearMotion, mMJCBufferCount, mScalerBufferCount);
                }

                for (VAL_UINT32_T u4I = 0; u4I < MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT; u4I++)
                {
                    if (mMJCBufferInfo[u4I].u4Hdr == (VAL_UINT32_T)pBuffHead)
                    {
                        //if (mMJCLog)
                        {
                            MTK_OMX_LOGUD("[MJC] [FreeBuffer][MJC BUFFER][%d] Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X, mUseClearMotion (%d). free buffer(%d)(%d)",
                                         u4I,
                                         mMJCBufferInfo[u4I].u4VA,
                                         mMJCBufferInfo[u4I].u4PA,
                                         mMJCBufferInfo[u4I].u4Size,
                                         mMJCBufferInfo[u4I].u4Hdr,
                                         mUseClearMotion, mMJCBufferCount, mScalerBufferCount
                                        );
                        }

                        mMJCBufferInfo[u4I].u4VA = 0xffffffff;
                        mMJCBufferInfo[u4I].u4PA = 0xffffffff;
                        mMJCBufferInfo[u4I].u4Size = 0xffffffff;
                        mMJCBufferInfo[u4I].u4Hdr = 0xffffffff;
                        break;
                    }
                }

                mMJCBufferCount = 0;

                // For Scaler ClearMotion +
                for (VAL_UINT32_T u4I = 0; u4I < MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT; u4I++)
                {
                    if (mScalerBufferInfo[u4I].u4Hdr == (VAL_UINT32_T)pBuffHead)
                    {
                        //if (mMJCLog)
                        {
                            MTK_OMX_LOGUD("[MJC] [FreeBuffer][SCALAR BUFFER][%d] Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X",
                                         u4I,
                                         mScalerBufferInfo[u4I].u4VA,
                                         mScalerBufferInfo[u4I].u4PA,
                                         mScalerBufferInfo[u4I].u4Size,
                                         mScalerBufferInfo[u4I].u4Hdr
                                        );
                        }

                        mScalerBufferInfo[u4I].u4VA = 0xffffffff;
                        mScalerBufferInfo[u4I].u4PA = 0xffffffff;
                        mScalerBufferInfo[u4I].u4Size = 0xffffffff;
                        mScalerBufferInfo[u4I].u4Hdr = 0xffffffff;
                        break;
                    }
                }
                // For Scaler ClearMotion -

                mScalerBufferCount = 0;
            }
            }
//#endif

#if (ANDROID_VER >= ANDROID_KK)
        }
        else
        {
            // mStoreMetaDataInBuffers is OMX_TRUE
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(pBuffHead, &graphicBufHandle);
            VAL_UINT32_T u4Idx;

            if (OMX_TRUE == mPortReconfigInProgress)
            {
                int count = 0;
                while (OMX_FALSE == mFlushDecoderDoneInPortSettingChange)
                {
                    MTK_OMX_LOGUD("waiting flush decoder done...");
                    SLEEP_MS(5);
                    count++;
                    if (count == 100)
                    {
                        break;
                    }
                }
            }

            if(mIsSecureInst == OMX_TRUE)
            {
                for (u4Idx = 0 ; u4Idx < mOutputPortDef.nBufferCountActual ; u4Idx++)
                {
                    if (mSecFrmBufInfo[u4Idx].pNativeHandle == (void *)graphicBufHandle)
                    {
                        MTK_OMX_LOGD("=====Free Secure Handle in Meta Mode=========");
                        mSecFrmBufInfo[u4Idx].u4BuffId = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].u4BuffHdr = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].u4BuffSize = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].u4SecHandle = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].pNativeHandle = (void *)0xffffffff;
                    }
                }
#if SECURE_OUTPUT_USE_ION
                int ret = mOutputMVAMgr->freeOmxMVAByHndl((void *)graphicBufHandle);
                if (ret < 0)
                {
                    MTK_OMX_LOGD("[ERROR][Output][FreeBuffer], LINE: %d\n", __LINE__);
                }
#endif
            }
            else
            {
                int ret = mOutputMVAMgr->freeOmxMVAByHndl((void *)graphicBufHandle);
                if (ret < 0)
                {
                    MTK_OMX_LOGD("[ERROR][Output][FreeBuffer], LINE: %d\n", __LINE__);
                }
            }

//#ifdef MTK_CLEARMOTION_SUPPORT
                if (mMJCEnable == OMX_TRUE)
                {
                    if (mMJCBufferCount > 0 || mScalerBufferCount > 0)
                    {
                        //MTK_OMX_LOGU("[MJC] [FreeBuffer][MJC BUFFER] mUseClearMotion (%d). free buffer(%d)(%d)", mUseClearMotion, mMJCBufferCount, mScalerBufferCount);
                    }

                    for (VAL_UINT32_T u4I = 0; u4I < MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT; u4I++)
                    {
                    if (mMJCBufferInfo[u4I].u4Hdr == (VAL_UINT32_T)pBuffHead)
                        {
                            {
                            MTK_OMX_LOGUD("[MJC] [FreeBuffer][MJC BUFFER][%d] Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X, mUseClearMotion (%d). free buffer(%d)(%d)",
                                         u4I,
                                         mMJCBufferInfo[u4I].u4VA,
                                         mMJCBufferInfo[u4I].u4PA,
                                         mMJCBufferInfo[u4I].u4Size,
                                         mMJCBufferInfo[u4I].u4Hdr,
                                         mUseClearMotion, mMJCBufferCount, mScalerBufferCount
                                            );
                            }

                            mMJCBufferInfo[u4I].u4VA = 0xffffffff;
                            mMJCBufferInfo[u4I].u4PA = 0xffffffff;
                            mMJCBufferInfo[u4I].u4Size = 0xffffffff;
                        mMJCBufferInfo[u4I].u4Hdr = 0xffffffff;
                        mMJCBufferCount--;
                            break;
                        }
                    }

                    // For Scaler ClearMotion +
                for (VAL_UINT32_T u4I = 0; u4I < MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT; u4I++)
                    {
                        if (mScalerBufferInfo[u4I].u4Hdr == (VAL_UINT32_T)pBuffHead)
                        {
                        //if (mMJCLog)
                            {
                            MTK_OMX_LOGUD("[MJC] [FreeBuffer][SCALAR BUFFER][%d] Va = 0x%08X, Pa = 0x%08X, Size = 0x%08X, Hdr = 0x%08X",
                                             u4I,
                                             mScalerBufferInfo[u4I].u4VA,
                                             mScalerBufferInfo[u4I].u4PA,
                                         mScalerBufferInfo[u4I].u4Size,
                                         mScalerBufferInfo[u4I].u4Hdr
                                            );
                            }

                            mScalerBufferInfo[u4I].u4VA = 0xffffffff;
                            mScalerBufferInfo[u4I].u4PA = 0xffffffff;
                            mScalerBufferInfo[u4I].u4Size = 0xffffffff;
                            mScalerBufferInfo[u4I].u4Hdr = 0xffffffff;
                            mScalerBufferCount--;
                            break;
                        }
                    }
                    // For Scaler ClearMotion -
                }
        }
#endif
        RemoveFrmBuf(pBuffHead);

        // free output buffers
        for (OMX_U32 i = 0 ; i < mOutputBufferHdrsCnt ; i++)
        {
            if (pBuffHead == mOutputBufferHdrs[i])
            {
                MTK_OMX_LOGD("FB out (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mOutputBufferHdrs[i]);
                mOutputBufferHdrs[i] = NULL;
                mOutputBufferPopulatedCnt--;
            }
        }

        if (mOutputBufferPopulatedCnt == 0)      // all output buffers have been freed
        {
            if (OMX_TRUE == mOutputUseION || OMX_TRUE == mStoreMetaDataInBuffers)
            {
                mIonOutputBufferCount = 0;
                mM4UBufferCount = mInputBufferPopulatedCnt;
            }

            else if (OMX_TRUE == mIsSecureInst)
            {
                mSecFrmBufCount = 0;
            }

            else
            {
                mM4UBufferCount = mInputBufferPopulatedCnt;
            }
            mOutputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mOutPortFreeDoneSem);
            MTK_OMX_LOGU("MtkOmxVdec::FreeBuffer all output buffers have been freed!!! signal mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));

            mOutputMVAMgr->freeOmxMVAAll();
        }

        if ((mOutputPortDef.bEnabled == OMX_TRUE) && (mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   NULL,
                                   NULL);
        }
    }

    // TODO: free memory for AllocateBuffer case

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::EmptyThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHead, pBuffHead->pBuffer, pBuffHead->nFilledLen);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_EMPTY_THIS_BUFFER_TYPE;
    // write 8 bytes to mEmptyBufferPipe  [buffer_type][pBuffHead]
    LOCK_T(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::FillThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)", pBuffHead, pBuffHead->pBuffer, pBuffHead->nAllocLen);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_FILL_THIS_BUFFER_TYPE;
    // write 8 bytes to mFillBufferPipe  [bufId][pBuffHead]
    LOCK_T(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_OUT OMX_U8 *cRole,
                                            OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if ((0 == nIndex) && (NULL != cRole))
    {
        // Unused callback. enum set to 0
        *cRole = 0;
        MTK_OMX_LOGD("MtkOmxVdec::ComponentRoleEnum: Role[%d]", *cRole);
    }
    else
    {
        err = OMX_ErrorNoMore;
    }

    return err;
}

OMX_BOOL MtkOmxVdec::PortBuffersPopulated()
{
    if ((OMX_TRUE == mInputPortDef.bPopulated) && (OMX_TRUE == mOutputPortDef.bPopulated))
    {
        return OMX_TRUE;
    }
    else
    {
        return OMX_FALSE;
    }
}


OMX_ERRORTYPE MtkOmxVdec::HandleStateSet(OMX_U32 nNewState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //    MTK_OMX_LOGD ("MtkOmxVdec::HandleStateSet");
    switch (nNewState)
    {
        case OMX_StateIdle:
            if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
            {
                MTK_OMX_LOGU("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                if ((OMX_FALSE == mInputPortDef.bEnabled) || (OMX_FALSE == mOutputPortDef.bEnabled))
                {
                    break; // leave the flow to port enable
                }

                // wait until input/output buffers allocated
                MTK_OMX_LOGUD("wait on mInPortAllocDoneSem(%d), mOutPortAllocDoneSem(%d)!!", get_sem_value(&mInPortAllocDoneSem), get_sem_value(&mOutPortAllocDoneSem));
                WAIT_T(mInPortAllocDoneSem);
                WAIT_T(mOutPortAllocDoneSem);

                if ((OMX_TRUE == mInputPortDef.bEnabled) && (OMX_TRUE == mOutputPortDef.bEnabled) && (OMX_TRUE == PortBuffersPopulated()))
                {
                    mState = OMX_StateIdle;
                    CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           mState,
                                           NULL);
                }
            }
            else if ((mState == OMX_StateExecuting) || (mState == OMX_StatePause))
            {
                MTK_OMX_LOGU("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // flush all ports
                mInputFlushALL = OMX_TRUE;
                SIGNAL(mOutputBufferSem);

                LOCK_T(mDecodeLock);
                FlushInputPort();
                FlushOutputPort();
                if (mPortReconfigInProgress == OMX_TRUE)//Bruce, for setting IDLE when port reconfig
                {
                    MTK_OMX_LOGE("Set state when PortReconfigInProgress");
                    mPortReconfigInProgress = OMX_FALSE;
                }
                UNLOCK(mDecodeLock);

                // de-initialize decoder
                DeInitVideoDecodeHW();

                mState = OMX_StateIdle;
                CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateIdle)
            {
                MTK_OMX_LOGU("Request [%s]-> [OMX_StateIdle]", StateToString(mState));
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }

            break;

        case OMX_StateExecuting:
            MTK_OMX_LOGU("Request [%s]-> [OMX_StateExecuting]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StatePause)
            {
                mInputFlushALL = OMX_FALSE;
                // change state to executing
                mState = OMX_StateExecuting;

                // trigger decode start
                mDecodeStarted = OMX_TRUE;

                // send event complete to IL client
                //MTK_OMX_LOGU("state changes to OMX_StateExecuting");
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateExecuting)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }
            break;

        case OMX_StatePause:
            MTK_OMX_LOGU("Request [%s]-> [OMX_StatePause]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StateExecuting)
            {
                mState = OMX_StatePause;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StatePause)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }
            break;

        case OMX_StateLoaded:
            MTK_OMX_LOGU("Request [%s]-> [OMX_StateLoaded]", StateToString(mState));
            if (mState == OMX_StateIdle)    // IDLE  to LOADED
            {
                if (IS_PENDING(MTK_OMX_LOADED_PENDING))
                {

                    // wait until all input buffers are freed
                    MTK_OMX_LOGUD("wait on mInPortFreeDoneSem(%d) %d, mOutPortFreeDoneSem(%d) %d "
                        , get_sem_value(&mInPortFreeDoneSem), mInputBufferPopulatedCnt, get_sem_value(&mOutPortFreeDoneSem), mOutputBufferPopulatedCnt);
                    if (mInputBufferPopulatedCnt != 0)
                        WAIT_T(mInPortFreeDoneSem);

                    // wait until all output buffers are freed
                    if (mOutputBufferPopulatedCnt != 0)
                        WAIT_T(mOutPortFreeDoneSem);

                    mState = OMX_StateLoaded;
                    CLEAR_PENDING(MTK_OMX_LOADED_PENDING);
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           mState,
                                           NULL);
                }
            }
            else if (mState == OMX_StateWaitForResources)
            {
                mState = OMX_StateLoaded;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateLoaded)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }
            break;

        case OMX_StateWaitForResources:
            MTK_OMX_LOGU("Request [%s]-> [OMX_StateWaitForResources]", StateToString(mState));
            if (mState == OMX_StateLoaded)
            {
                mState = OMX_StateWaitForResources;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateWaitForResources)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }
            break;

        case OMX_StateInvalid:
            MTK_OMX_LOGU("Request [%s]-> [OMX_StateInvalid]", StateToString(mState));
            if (mState == OMX_StateInvalid)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mState = OMX_StateInvalid;

                // for conformance test <2,7> loaded -> invalid
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorInvalidState,
                                       NULL,
                                       NULL);
            }
            break;

        default:
            break;
    }
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandlePortEnable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGD("MtkOmxVdec::HandlePortEnable nPortIndex(0x%X)", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))       // p.86 component is not in LOADED state and the port is not populated
        {
            MTK_OMX_LOGUD("Wait on mInPortAllocDoneSem(%d)", get_sem_value(&mInPortAllocDoneSem));
            WAIT_T(mInPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
        }
        mInputFlushALL = OMX_FALSE;

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortEnable,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
        {
            MTK_OMX_LOGUD("Wait on mOutPortAllocDoneSem(%d)", get_sem_value(&mOutPortAllocDoneSem));
            WAIT_T(mOutPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);

            if (mState == OMX_StateExecuting && mPortReconfigInProgress == OMX_TRUE)
            {
                MTK_OMX_LOGUD("mPortReconfigInProgress as FALSE");
                mPortReconfigInProgress = OMX_FALSE;
            }
        }
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortEnable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
        {
            if ((OMX_TRUE == mInputPortDef.bEnabled) && (OMX_TRUE == mOutputPortDef.bEnabled) && (OMX_TRUE == PortBuffersPopulated()))
            {
                MTK_OMX_LOGU("@@ Change to IDLE in HandlePortEnable()");
                mState = OMX_StateIdle;
                CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
        }
    }

    return err;
}


OMX_BOOL MtkOmxVdec::CheckBufferAvailability()
{

#if CPP_STL_SUPPORT
    if (mEmptyThisBufQ.empty())// || mFillThisBufQ.empty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }
#endif


#if ANDROID
    if (mEmptyThisBufQ.isEmpty())// || mFillThisBufQ.isEmpty())
    {
        //if (mEmptyThisBufQ.isEmpty())
        {
            MTK_OMX_LOGD("0x%08x ETB empty", this);
            //SIGNAL (mDecodeSem); // output driven, if semaphore count is minus one by input, plus back.
            mNumSemaphoreCountForInput++;
            //MTK_OMX_LOGD("0x%08x input eat semaphore count: %d", this, mNumSemaphoreCountForInput);
        }
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }
#endif
}


int MtkOmxVdec::DequeueInputBuffer()
{
    int input_idx = -1;
    LOCK_T(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    input_idx = *(mEmptyThisBufQ.begin());
    mEmptyThisBufQ.erase(mEmptyThisBufQ.begin());
#endif

#if ANDROID
    input_idx = mEmptyThisBufQ[0];
    mEmptyThisBufQ.removeAt(0);
#endif

    UNLOCK(mEmptyThisBufQLock);

    return input_idx;
}

void MtkOmxVdec::CheckOutputBuffer()
{
    unsigned int i;
    int index;

    LOCK_T(mFillThisBufQLock);
    for (i = 0; i < mFillThisBufQ.size(); i++)
    {
        index = mFillThisBufQ[i];
        if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[index]))
        {
            MTK_OMX_LOGD("Output[%d] [0x%08X] is not free. pFrameBufArray", index, mOutputBufferHdrs[index]);
        }
        else
        {
            MTK_OMX_LOGD("Output[%d] [0x%08X] is free. pFrameBufArray", index, mOutputBufferHdrs[index]);
        }
    }
    UNLOCK(mFillThisBufQLock);
}

int MtkOmxVdec::DequeueOutputBuffer()
{
    int output_idx = -1, i;
    LOCK_T(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    output_idx = *(mFillThisBufQ.begin());
    mFillThisBufQ.erase(mFillThisBufQ.begin());
#endif

#if ANDROID
    for (i = 0; i < mFillThisBufQ.size(); i++)
    {
        output_idx = mFillThisBufQ[i];
        if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[output_idx]))
        {
            MTK_OMX_LOGD("DequeueOutputBuffer(), mOutputBufferHdrs[%d] 0x%08x is not free", output_idx, mOutputBufferHdrs[output_idx]);
        }
        else
        {
            //MTK_OMX_LOGE("DequeueOutputBuffer(), mOutputBufferHdrs[%d] 0x%08x is free", output_idx, mOutputBufferHdrs[output_idx]);
            break;
        }
    }

    if (0 == mFillThisBufQ.size())
    {
        MTK_OMX_LOGD("DequeueOutputBuffer(), mFillThisBufQ.size() is 0, return original idx %d", output_idx);
        UNLOCK(mFillThisBufQLock);
        output_idx = -1;
#ifdef HAVE_AEE_FEATURE
        //aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_DEFAULT, "\nOmxVdec should no be here will push dummy output buffer!");
#endif //HAVE_AEE_FEATURE
        return output_idx;
    }

    if (i == mFillThisBufQ.size())
    {
        output_idx = -1;
    }
    else
    {
        output_idx = mFillThisBufQ[i];
        mFillThisBufQ.removeAt(i);
    }
#endif

    UNLOCK(mFillThisBufQLock);

    return output_idx;
}

int MtkOmxVdec::FindQueueOutputBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    // ATTENTION, please lock this from caller function.
    int output_idx = -1, i;
    //LOCK_T(mFillThisBufQLock);

#if ANDROID
    for (i = 0; i < mFillThisBufQ.size(); i++)
    {
        output_idx = mFillThisBufQ[i];
        if (pBuffHdr == mOutputBufferHdrs[output_idx])
        {
            MTK_OMX_LOGD("FQOB %d (%d,%d)", output_idx, i, mFillThisBufQ.size());
            break;
        }
    }
    if (i == mFillThisBufQ.size())
    {
        MTK_OMX_LOGE("FindQueueOutputBuffer not found, 0x%08X", pBuffHdr);
        i = -1;
    }
#endif

    //UNLOCK(mFillThisBufQLock);

    return i;
}

void MtkOmxVdec::QueueInputBuffer(int index)
{
    LOCK_T(mEmptyThisBufQLock);

    //MTK_OMX_LOGD ("@@ QueueInputBuffer (%d)", index);

#if CPP_STL_SUPPORT
    //mEmptyThisBufQ.push_front(index);
#endif

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif

    UNLOCK(mEmptyThisBufQLock);
}

void MtkOmxVdec::QueueOutputBuffer(int index)
{
    // Caller is responsible for lock protection
    //LOCK_T(mFillThisBufQLock);

    //MTK_OMX_LOGD ("@@ QueueOutputBuffer");
#if CPP_STL_SUPPORT
    mFillThisBufQ.push_back(index);
#endif

#if ANDROID
    mFillThisBufQ.push(index);
#endif

    //UNLOCK(mFillThisBufQLock);

    if (mInputZero == OMX_TRUE)
    {
        MTK_OMX_LOGD("@@ mInputZero, QueueOutputBuffer SIGNAL (mDecodeSem)");
        mInputZero = OMX_FALSE;
        SIGNAL(mDecodeSem);
    }

}


OMX_BOOL MtkOmxVdec::FlushInputPort()
{
    // in this function ,  ******* mDecodeLock is LOCKED ********
    int cnt = 0;
    MTK_OMX_LOGU("+FlushInputPort");
    mInputFlushALL = OMX_TRUE;
    SIGNAL(mOutputBufferSem);

    DumpETBQ();

    // return all input buffers currently we have
    FlushDecoder(OMX_FALSE);
    ReturnPendingInputBuffers();

    MTK_OMX_LOGUD("FlushInputPort -> mNumPendingInput(%d)", (int)mNumPendingInput);
    while (mNumPendingInput > 0)
    {
        MTK_OMX_LOGUD("Wait input buffer release....%d", mNumPendingInput);
        SLEEP_MS(1);
        cnt ++;
        if (cnt > 2000)
        {
            MTK_OMX_LOGE("Wait input buffer release timeout mNumPendingInput %d", mNumPendingInput);
            abort();
            break;
        }
    }

    //MTK_OMX_LOGU("-FlushInputPort");
    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::FlushOutputPort()
{
    // in this function ,  ******* mDecodeLock is LOCKED ********
    MTK_OMX_LOGU("+FlushOutputPort");
    Mutex::Autolock autolock(mFlushOrConvertLock);
    //MTK_OMX_LOGU("++FlushOutputPort");

    VAL_UINT32_T u4SemCount = get_sem_value(&mOutputBufferSem);

    while (u4SemCount > 0 && get_sem_value(&mOutputBufferSem) > 0)
    {
        WAIT_T(mOutputBufferSem);
        u4SemCount--;
    }

    DumpFTBQ();
    // return all output buffers from decoder
    FlushDecoder(OMX_FALSE);
    // return all output buffers currently we have
    ReturnPendingOutputBuffers();

    mNumFreeAvailOutput = 0;
    mNumAllDispAvailOutput = 0;
    mNumNotDispAvailOutput = 0;
    mEOSFound = OMX_FALSE;
    mEOSTS = -1;

    MTK_OMX_LOGUD("-FlushOutputPort -> mNumPendingOutput(%d)", (int)mNumPendingOutput);

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxVdec::HandlePortDisable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGU("MtkOmxVdec::HandlePortDisable nPortIndex=0x%X", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    // TODO: should we hold mDecodeLock here??

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        if (mInputPortDef.bPopulated == OMX_TRUE)
        {
            //if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_TRUE)) {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                mInputFlushALL = OMX_TRUE;
                SIGNAL(mOutputBufferSem);

                LOCK_T(mDecodeLock);
                FlushInputPort();
                UNLOCK(mDecodeLock);
            }

            // wait until the input buffers are freed
            WAIT_T(mInPortFreeDoneSem);
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        mOutputPortDef.bEnabled = OMX_FALSE;

        MTK_OMX_LOGUD("MtkOmxVdec::HandlePortDisable mOutputPortDef.bPopulated(%d)", mOutputPortDef.bPopulated);

        if (mOutputPortDef.bPopulated == OMX_TRUE)
        {
            //if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_TRUE)) {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                // flush output port
                LOCK_T(mDecodeLock);
                FlushOutputPort();
                UNLOCK(mDecodeLock);

                mFlushDecoderDoneInPortSettingChange = OMX_TRUE;
            }

            // wait until the output buffers are freed
            //WAIT_T(mOutPortFreeDoneSem);
            if (0 != WAIT_TIMEOUT(mOutPortFreeDoneSem,LOCK_TIMEOUT_S)){
                ALOGE("## [ERROR, %d] %s() line: %d WAIT timeout...", errno, __FUNCTION__,__LINE__);
                mOutputMVAMgr->freeOmxMVAAll();
            }
        }
        else
        {
            if (get_sem_value(&mOutPortFreeDoneSem) > 0)
            {
                MTK_OMX_LOGUD("@@ OutSem ++");
                int retVal = TRY_WAIT(mOutPortFreeDoneSem);
                if (0 == retVal)
                {
                    MTK_OMX_LOGUD("@@ OutSem -- (OK)");
                }
                else if (-1 == retVal)
                {
                    MTK_OMX_LOGUD("@@ OutSem errno %d-- (EAGAIN %d)", errno, EAGAIN);
                }
            }
        }

        if (OMX_TRUE == mPortReconfigInProgress)
        {
            mOutputPortDef.nBufferCountActual = mReconfigOutputPortBufferCount;
            mOutputPortDef.nBufferSize = mReconfigOutputPortBufferSize;

            if ( (OMX_TRUE == mCrossMountSupportOn) )
            {
                VAL_CHAR_T mMCCMaxValue[PROPERTY_VALUE_MAX];
                if (property_get("ro.vendor.mtk_crossmount.maxcount", mMCCMaxValue, NULL))
                {
                    //use set property, the maxcount
                }
                else
                {
                    mMaxColorConvertOutputBufferCnt = mOutputPortDef.nBufferCountActual;
                }

                mReconfigOutputPortBufferCount += mMaxColorConvertOutputBufferCnt;
                mOutputPortDef.nBufferCountActual = mReconfigOutputPortBufferCount;
                MTK_OMX_LOGUD("during OMX_EventPortSettingsChanged nBufferCountActual after adjust = %d(+%d) ",
                    mOutputPortDef.nBufferCountActual, mMaxColorConvertOutputBufferCnt);
            }

//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            MJCScalerCheckandSetBufInfo();
            MJCSetBufRes();
            }
//#endif
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandlePortFlush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGU("MtkOmxVdec::HandleFlush nPortIndex(0x%X)", (unsigned int)nPortIndex);

#if 1 // for cts testCodecResets<SPEC>WithSurface and testCodecResets<SPEC>WithoutSurface
    MTK_OMX_LOGUD("@@ mNumPendingInput(%d), mNumPendingOutput(%d), mPortReconfigInProgress(%d), mNeedDecodeWhenFlush %d", mNumPendingInput, mNumPendingOutput, mPortReconfigInProgress, mNeedDecodeWhenFlush);
    if ((nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT) &&
        (!mEmptyThisBufQ.empty() || (mDecoderInitCompleteFlag == OMX_FALSE && mInputBuffInuse == OMX_TRUE)) && (mCodecId == MTK_VDEC_CODEC_ID_AVC || mCodecId == MTK_VDEC_CODEC_ID_MPEG4 || mCodecId == MTK_VDEC_CODEC_ID_HEVC))
    {
        VAL_UINT32_T u4Count = 0;
        VAL_UINT32_T u4DecSemIncCount = 0;

        bool bIsHEVC_ETBQcontainsPPS  = (mFramesDecoded < 3 && mCodecId == MTK_VDEC_CODEC_ID_HEVC); //VPS SPS PPS
        bool bIsH264_ETBQcontainsPPS  = (mFramesDecoded < 2 && mCodecId == MTK_VDEC_CODEC_ID_AVC);  //SPS PPS
        bool bIsMP4_ETBQcontainsPPS = (mFramesDecoded < 1 && mCodecId == MTK_VDEC_CODEC_ID_MPEG4); //Hrd

        if (mCodecId == MTK_VDEC_CODEC_ID_HEVC){
            u4DecSemIncCount = 3;
        } else if (mCodecId == MTK_VDEC_CODEC_ID_AVC) {
            u4DecSemIncCount = 2;
        } else if (mCodecId == MTK_VDEC_CODEC_ID_MPEG4) {
            u4DecSemIncCount = 1;
        }
        while ((!mEmptyThisBufQ.empty() || (mDecoderInitCompleteFlag == OMX_FALSE && mInputBuffInuse == OMX_TRUE) || mNeedDecodeWhenFlush) && (!mFillThisBufQ.empty()) && (bIsHEVC_ETBQcontainsPPS || bIsH264_ETBQcontainsPPS || bIsMP4_ETBQcontainsPPS))  // etb queue may contain pps
        {
            if (0 < mErrorInDecoding)
            {
                MTK_OMX_LOGUD("@@ ignore pending check when decoding error %d, PendingI %d, PendingO %d", mErrorInDecoding, mNumPendingInput, mNumPendingOutput);
                break;
            }
            if (u4Count > 20)
            {
                if (OMX_TRUE == mPortReconfigInProgress)
                {
                    MTK_OMX_LOGUD("@@ timeout(%d) skip pending check in PortReconfigInProgress, mNeedDecodeWhenFlush %d", u4Count, mNeedDecodeWhenFlush);

                    // allow decoding thread to consume next buffer
                    if (mRestorePortReconfigFlag == OMX_FALSE)
                    {
                        mPortReconfigInProgress = OMX_FALSE;
                        MTK_OMX_LOGUD("@@ allow decoding thread to consume next buffer, mFramesDecoded %d, mNeedDecodeWhenFlush %d", mFramesDecoded, mNeedDecodeWhenFlush);
                        mRestorePortReconfigFlag = OMX_TRUE;
                        u4Count = 0;
                        continue;
                    }

                    break;
                }
            }
            if (u4DecSemIncCount > 0) {
                SIGNAL(mDecodeSem);
            }
            sched_yield();
            SLEEP_MS(1);
            u4Count++;
            MTK_OMX_LOGUD("@@ waiting for input being consumed, mNumPendingInput(%d) mNeedDecodeWhenFlush %d", mNumPendingInput, mNeedDecodeWhenFlush);

            u4DecSemIncCount--;
            bIsHEVC_ETBQcontainsPPS  = (mFramesDecoded < 3 && mCodecId == MTK_VDEC_CODEC_ID_HEVC); //VPS SPS PPS
            bIsH264_ETBQcontainsPPS  = (mFramesDecoded < 2 && mCodecId == MTK_VDEC_CODEC_ID_AVC);  //SPS PPS
            bIsMP4_ETBQcontainsPPS = (mFramesDecoded < 1 && mCodecId == MTK_VDEC_CODEC_ID_MPEG4); //Hrd
        }
        if (mRestorePortReconfigFlag == OMX_TRUE)
        {
            mPortReconfigInProgress = OMX_TRUE;
            mRestorePortReconfigFlag = OMX_FALSE;
        }
    }
#endif



    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        mInputFlushALL = OMX_TRUE;
        SIGNAL(mOutputBufferSem); // one for the driver callback to get output buffer
        SIGNAL(mOutputBufferSem); // the other one is if one of input buffer is EOS and no output buffer is available

        MTK_OMX_LOGUD("get lock before FlushInputPort\n");
        LOCK_T(mDecodeLock);
        FlushInputPort();
        mInputFlushALL = OMX_FALSE;
        UNLOCK(mDecodeLock);

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        MTK_OMX_LOGUD("get lock before FlushOutputPort\n");
        LOCK_T(mDecodeLock);
        FlushOutputPort();
        UNLOCK(mDecodeLock);

        if (mCodecId == MTK_VDEC_CODEC_ID_HEVC && OMX_TRUE == mPortReconfigInProgress)
        {
            mOutputPortDef.nBufferCountActual = mReconfigOutputPortBufferCount;
            mOutputPortDef.nBufferSize = mReconfigOutputPortBufferSize;

            if ( (OMX_TRUE == mCrossMountSupportOn) )
            {
                VAL_CHAR_T mMCCMaxValue[PROPERTY_VALUE_MAX];
                if (property_get("ro.vendor.mtk_crossmount.maxcount", mMCCMaxValue, NULL))
                {
                    //use set property, the maxcount
                }
                else
                {
                    mMaxColorConvertOutputBufferCnt = mOutputPortDef.nBufferCountActual;
                }

                mReconfigOutputPortBufferCount += mMaxColorConvertOutputBufferCnt;
                mOutputPortDef.nBufferCountActual = mReconfigOutputPortBufferCount;
                MTK_OMX_LOGUD("during OMX_EventPortSettingsChanged nBufferCountActual after adjust = %d(+%d) ",
                    mOutputPortDef.nBufferCountActual, mMaxColorConvertOutputBufferCnt);
            }

//#ifdef MTK_CLEARMOTION_SUPPORT
            if (mMJCEnable == OMX_TRUE)
            {
            MJCScalerCheckandSetBufInfo();
            MJCSetBufRes();
            }
//#endif
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVdec::HandleMarkBuffer");

    return err;
}

OMX_U64 MtkOmxVdec::GetInputBufferCheckSum(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_U64 InputCheckSum=0;
    OMX_U32 i = 0;
    OMX_U8* InputBufferPointer = NULL;

    InputBufferPointer = pBuffHdr->pBuffer;
    for (i=0; i < pBuffHdr->nFilledLen; i++)
    {
        InputCheckSum = InputCheckSum + *InputBufferPointer;
    	InputBufferPointer ++;
    }

    return InputCheckSum;
}


OMX_ERRORTYPE MtkOmxVdec::HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGUD("ETB (0x%08X) (0x%08X) (%u) (%d %d) AVSyncTime(%lld) (0x%08X) %lld ",
                 pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nFilledLen, mNumPendingInput, mEmptyThisBufQ.size(), mAVSyncTime, pBuffHdr->nFlags, pBuffHdr->nTimeStamp);

    int index = findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] ETB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("ETB idx(%d)", index);

    LOCK_T(mEmptyThisBufQLock);
    mNumPendingInput++;

#if CPP_STL_SUPPORT
    mEmptyThisBufQ.push_back(index);
    DumpETBQ();
#endif

#if ANDROID
    mEmptyThisBufQ.push(index);
    //DumpETBQ();
#endif
    UNLOCK(mEmptyThisBufQLock);

    // for INPUT_DRIVEN
    SIGNAL(mDecodeSem);

#if 0
    FILE *fp_output;
    OMX_U32 size_fp_output;
    char ucStringyuv[100];
    char *ptemp_buff = (char *)pBuffHdr->pBuffer;
    sprintf(ucStringyuv, "//sdcard//Vdec%4d.bin",  gettid());
    fp_output = fopen(ucStringyuv, "ab");
    if (fp_output != NULL)
    {
        char header[4] = { 0, 0, 0, 1};
        fwrite(header, 1, 4, fp_output);
        size_fp_output = pBuffHdr->nFilledLen;
        MTK_OMX_LOGD("input write size = %d\n", size_fp_output);
        size_fp_output = fwrite(ptemp_buff, 1, size_fp_output, fp_output);
        MTK_OMX_LOGD("input real write size = %d\n", size_fp_output);
        fclose(fp_output);
    }
    else
    {
        LOGE("sdcard/mfv_264.out file create error\n");
    }
#endif

    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_BOOL bDecodeBuffer = OMX_FALSE;

    if (mMJCEnable == OMX_TRUE && OMX_TRUE == MJCIsMJCRefBuf(pBuffHdr))
    {
        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] Output [0x%08X] is MJC ref buf.", pBuffHdr);
        }

        if (OMX_TRUE == MJCIsScalerBufandScalerEbable(pBuffHdr)){
            MTK_OMX_LOGUD("FTB (Scaler) (0x%08X) (0x%08X) (%u) AVSyncTime(%lld)",
                         pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, mAVSyncTime);
        } else {    // MJC ref is Scaler or Decoder output
            //MTK_OMX_LOGUD("FTB (Decoder) (0x%08X) (0x%08X) (%u) AVSyncTime(%lld)",
            //         pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, mAVSyncTime);
        }

        WaitFence(pBuffHdr, OMX_TRUE);
        pBuffHdr->nFilledLen = 0;
        pBuffHdr->nTimeStamp = 0;
        MTK_OMX_LOGUD("FBD (MJC ref buf) (0x%08X) (0x%08X) %lld (%u) flags(0x%08x)",
                     pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen,
                     pBuffHdr->nFlags);
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                         mAppData,
                         pBuffHdr);
        return err;
    }

    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr);
    int i;
    OMX_BOOL bFound = OMX_FALSE;
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] FTB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("FTB idx(%d)", index);


    LOCK_T(mFillThisBufQLock);
#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        if(mIsSecureInst == OMX_TRUE)
        {
            SetupMetaSecureHandle(pBuffHdr);
        }
        else
        {
        SetupMetaIonHandle(pBuffHdr);

        int ret = 0;
        int srcIdx = -1;
        int dstIdx = -1;
        VBufInfo info;
        OMX_U32 graphicBufHandle = 0;
        OMX_BOOL bFoundInt = OMX_FALSE;
        OMX_BOOL bFound = OMX_FALSE;
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
        ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
/*
        if (ret < 0)
        {
           // MVA entry may exist in SetupMetaIonHandle, but later freed by GetFreeBuffer, recreate it
           ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, (void *)pBuffHdr);
           if (ret < 0)
           {
               MTK_OMX_LOGE("[ERROR] HandleFillThisBuffer newOmxMVAwithHndl() retry failed");
           }
           else
           {
               ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
               if (ret < 0)
               {
                   MTK_OMX_LOGE("[ERROR] HandleFillThisBuffer getOmxInfoFromHndl() retry failed");
               }
           }
        }
*/
        if (1 == ret)
        {
            //MTK_OMX_LOGE();
            for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (info.ionBufHndl == mFrameBufInt[i].ionBufHandle)
                {
                    bFoundInt = OMX_TRUE;
                    break;
                }
            }

            if (OMX_TRUE == bFoundInt)
            {
               srcIdx = i;
            }

            for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                //MTK_OMX_LOGD("xxxxx %d, 0x%08x (0x%08x)", i, mFrameBuf[i].ipOutputBuffer, mOutputBufferHdrs[i]);
                if (mFrameBuf[i].ipOutputBuffer == pBuffHdr)
                {
                    bFound = OMX_TRUE;
                    break;
                }
            }

            if (OMX_TRUE == bFound)
            {
               dstIdx = i;
            }

            if (srcIdx > -1 && dstIdx > -1)
            {
                // Existing buffer still in OMX's control
                //MTK_OMX_LOGE("[TEST] HandleFillThisBuffer Case 1, mFrameBuf[%d] hdr = 0x%x copied from mFrameBufInt[%d]", dstIdx, pBuffHdr, srcIdx);
                mFrameBuf[dstIdx].bUsed = mFrameBufInt[srcIdx].bUsed;
                mFrameBuf[dstIdx].bDisplay = mFrameBufInt[srcIdx].bDisplay;
                mFrameBuf[dstIdx].bNonRealDisplay = mFrameBufInt[srcIdx].bNonRealDisplay;
                mFrameBuf[dstIdx].bFillThis = OMX_TRUE;
                memcpy(&mFrameBuf[dstIdx].frame_buffer, &mFrameBufInt[srcIdx].frame_buffer, sizeof(VDEC_DRV_FRAMEBUF_T));
                //mFrameBuf[dstIdx].iTimestamp = mFrameBufInt[srcIdx].iTimestamp;
                mFrameBuf[dstIdx].bGraphicBufHandle = mFrameBufInt[srcIdx].bGraphicBufHandle;
                mFrameBuf[dstIdx].ionBufHandle = mFrameBufInt[srcIdx].ionBufHandle;
                mFrameBuf[dstIdx].refCount = mFrameBufInt[srcIdx].refCount;
                IncRef(BUFFER_OWNER_COMPONENT, mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));

                mFrameBufInt[srcIdx].bUsed = OMX_FALSE;
                mFrameBufInt[srcIdx].bDisplay = OMX_FALSE;
                mFrameBufInt[srcIdx].bNonRealDisplay = OMX_FALSE;
                mFrameBufInt[srcIdx].bFillThis = OMX_FALSE;
                memset(&mFrameBufInt[srcIdx], 0, sizeof(mFrameBufInt[srcIdx]));
                mFrameBufInt[srcIdx].iTimestamp = 0;
                mFrameBufInt[srcIdx].bGraphicBufHandle = 0;
                mFrameBufInt[srcIdx].ionBufHandle = 0;
                mFrameBufInt[srcIdx].refCount = 0;
            }
            else if (dstIdx > -1)
            {
                // Not in OMX's control
                int buffer_size = 0;
                gralloc_extra_query((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &buffer_size);

                // MTK_OMX_LOGE("[TEST] HandleFillThisBuffer Case 2, new mFrameBuf[%d] hdr = 0x%x, ionHandle = %d, va = 0x%x graphicBufHandle = 0x%x buffer_size %d",
                //    dstIdx, pBuffHdr, info.ionBufHndl, info.u4VA, graphicBufHandle, buffer_size);

                mFrameBuf[dstIdx].bUsed = OMX_FALSE;
                mFrameBuf[dstIdx].bDisplay = OMX_FALSE;
                mFrameBuf[dstIdx].bNonRealDisplay = OMX_FALSE;
                mFrameBuf[dstIdx].bFillThis = OMX_TRUE;
                memset(&mFrameBuf[dstIdx].frame_buffer, 0, sizeof(mFrameBuf[dstIdx].frame_buffer));
                mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4VA = info.u4VA;
                mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4PA = info.u4PA;
                mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4Size = buffer_size;
                mFrameBuf[dstIdx].iTimestamp = 0;
                mFrameBuf[dstIdx].bGraphicBufHandle = graphicBufHandle;
                mFrameBuf[dstIdx].ionBufHandle = info.ionBufHndl;
                mFrameBuf[dstIdx].refCount = 0;
                IncRef(BUFFER_OWNER_COMPONENT, mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
            }
            else
            {
                MTK_OMX_LOGE("[ERROR] FTB OMX Buffer header not exist 0x%x", pBuffHdr);
            }
        }
        else
        {
            MTK_OMX_LOGE("[ERROR] HandleFillThisBuffer failed to map buffer");
        }
    }
    }
#endif


    mNumPendingOutput++;

    //MTK_OMX_LOGE ("[@#@] FTB idx %d, mNumPendingOutput(%d) / ( %d / %d )",
    //    index, mNumPendingOutput, mBufColorConvertDstQ.size(), mBufColorConvertSrcQ.size());

#if CPP_STL_SUPPORT
    mFillThisBufQ.push_back(index);
    //DumpFTBQ();
#endif

#if ANDROID

//#ifdef MTK_CLEARMOTION_SUPPORT
    if (mMJCEnable == OMX_TRUE)
    {
    if (OMX_TRUE == MJCIsMJCOutBuf(pBuffHdr))
    {
        //mFillThisBufQ.insertAt(index, 0);
        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] Output [0x%08X] is MJC out buf.", pBuffHdr);
        }

        MTK_OMX_LOGUD("FTB (MJC) (0x%08X) (0x%08X) (%u) AVSyncTime(%lld)",
                     pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, mAVSyncTime);

        //mNumFreeAvailOutput--;
        MJCSetGrallocExtra(pBuffHdr, OMX_TRUE);
        MJCSetBufFlagInterpolateFrame(pBuffHdr);
        MJCPPBufQInsert(pBuffHdr);
        // For Scaler ClearMotion +
        SIGNAL(mpMJC->mMJCFrameworkSem);
    }
    // For Scaler ClearMotion +
    else if (OMX_TRUE == MJCIsScalerBufandScalerEbable(pBuffHdr))
    {
        //mFillThisBufQ.insertAt(index, 0);
        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] Output [0x%08X] is Scaler buf.", pBuffHdr);
        }

        MTK_OMX_LOGUD("FTB (Scaler) (0x%08X) (0x%08X) (%u) AVSyncTime(%lld)",
                     pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, mAVSyncTime);

        //mNumFreeAvailOutput--;
        MJCSetGrallocExtra(pBuffHdr, OMX_FALSE);
        MJCSetBufFlagScalerFrame(pBuffHdr);
        MJCPPBufQInsert(pBuffHdr);
        // For Scaler ClearMotion +
        SIGNAL(mpMJC->mMJCFrameworkSem);
    }
    // For Scaler ClearMotion -
    else
    {
        MTK_OMX_LOGUD("FTB (Decoder) (0x%08X) (0x%08X) (%u) AVSyncTime(%lld)",
                     pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, mAVSyncTime);
        bDecodeBuffer = OMX_TRUE;
        mFillThisBufQ.push(index);
    }
    }
    else
    {
//#else
    MTK_OMX_LOGUD("FTB (Decoder) (0x%08X) (0x%08X) (%u) AVSyncTime(%lld)",
                 pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, mAVSyncTime);
    bDecodeBuffer = OMX_TRUE;
    mFillThisBufQ.push(index);
    }
//#endif //MTK_CLEARMOTION_SUPPORT
    //DumpFTBQ();
#endif //Android

    /**
         *  During port-reconfig, decode thread will keep "conuming mDecodeSem".
         *  This strategy enables decode thread to sleep after running out of mDecodeSem.
         *
         *  After port-reconfig, framework should send new output buffers if it wants to decode.
         *  So here we signal all decode semaphores consumed during port-reconfig.
         *
         *  Make sure mPortReconfigInProgress is false to prevent decode thread
         *  from keeping decrease and increase mNumSemaphoreCountForOutput during port-reconfig.
         */
    if (mPortReconfigInProgress == OMX_FALSE)
    {
        for(; mNumSemaphoreCountForOutput > 0; -- mNumSemaphoreCountForOutput)
        {
            SIGNAL(mDecodeSem);
        }
    }

    int mReturnIndex = -1;

    if (bDecodeBuffer == OMX_TRUE)
    {
        mReturnIndex = PrepareAvaliableColorConvertBuffer(index, OMX_FALSE);

        if (0 > mReturnIndex)
        {
            //ALOGE("PrepareAvaliableColorConvertBuffer retry");
        }
    }

    for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        //MTK_OMX_LOGD("xxxxx %d, 0x%08x (0x%08x)", i, mFrameBuf[i].ipOutputBuffer, mOutputBufferHdrs[i]);
        if (mFrameBuf[i].ipOutputBuffer == pBuffHdr && bDecodeBuffer == OMX_TRUE)
        {
            mFrameBuf[i].bFillThis = OMX_TRUE;
            if (OMX_FALSE == mFrameBuf[i].bUsed && mReturnIndex < 0)
            {
                mNumFreeAvailOutput++;
                //MTK_OMX_LOGD("SIGNAL mOutputBufferSem");
                SIGNAL(mOutputBufferSem);
            }
            else
            {
                MTK_OMX_LOGD("NO SIGNAL mOutputBufferSem (0x%08x not free)", pBuffHdr);
            }
            // always signal for racing issue
            //SIGNAL(mDecodeSem);
            bFound = OMX_TRUE;
        }
    }
    UNLOCK(mFillThisBufQLock);

    if (bFound == OMX_FALSE && bDecodeBuffer == OMX_TRUE)
    {
        mNumFreeAvailOutput++;
        //MTK_OMX_LOGD("0x%08x SIGNAL mDecodeSem from HandleFillThisBuffer() 2", this);
        //SIGNAL(mDecodeSem);
    }

    if (mNumAllDispAvailOutput > 0)
    {
        mNumAllDispAvailOutput--;
    }


    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGUD("EBD (0x%08X) (0x%08X) (%d) (%d %d)", pBuffHdr, pBuffHdr->pBuffer, mNumPendingInput, mNumPendingInput, mEmptyThisBufQ.size());

    LOCK_T(mEmptyThisBufQLock);
    if (mNumPendingInput > 0)
    {
        mNumPendingInput--;
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] mNumPendingInput == 0 and want to --");
    }
    UNLOCK(mEmptyThisBufQLock);

    mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                              mAppData,
                              pBuffHdr);

    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone)
{
    //MTK_OMX_LOGE("### mDeInterlaceEnable = %d mInterlaceChkComplete = %d mThumbnailMode = %d\n", mDeInterlaceEnable, mInterlaceChkComplete, mThumbnailMode);
    if (mDeInterlaceEnable && (pBuffHdr->nFilledLen > 0))
    {
        if ((mInterlaceChkComplete == OMX_FALSE) && (mThumbnailMode != OMX_TRUE))
        {
            VAL_UINT32_T u32VideoInteraceing  = 0;
            if (VDEC_DRV_MRESULT_OK == eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_QUERY_VIDEO_INTERLACING, NULL, &u32VideoInteraceing))
            {
                mIsInterlacing = (VAL_BOOL_T)u32VideoInteraceing;
                if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE && mIsInterlacing == OMX_TRUE)
                {
                    if (OMX_TRUE == mMJCEnable)
                    {
                        unsigned int mMJC_RunTimeDis = OMX_TRUE;
				        m_fnMJCSetParam(mpMJC, MJC_PARAM_RUNTIME_DISABLE, &mMJC_RunTimeDis);
                    }

                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;

                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                            mAppData,
                                            OMX_EventPortSettingsChanged,
                                            MTK_OMX_OUTPUT_PORT,
                                            OMX_IndexVendMtkOmxUpdateColorFormat,
                                            NULL);

                    mInterlaceChkComplete = OMX_TRUE;
                }
                else if ((meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE) && (mIsInterlacing == OMX_FALSE) && (mCodecId == MTK_VDEC_CODEC_ID_AVC)
                         && ((mOutputPortDef.format.video.nFrameWidth > 2048) || (mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) > (2048 * 1088)))
                        // for platform need 4K SW decoder
                {
                    mOutputPortFormat.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                    mInterlaceChkComplete = OMX_TRUE;
                }
            }
            else
            {
                //MTK_OMX_LOGD("VDEC_DRV_GET_TYPE_QUERY_VIDEO_INTERLACING not support");
                mIsInterlacing = OMX_FALSE;
            }
            //MTK_OMX_LOGD("mIsInterlacing %d", mIsInterlacing);
        }
    }

//#if  ((defined MTK_CLEARMOTION_SUPPORT))
    if (OMX_TRUE == mMJCEnable && OMX_FALSE == mRealCallBackFillBufferDone)
    {
//#endif //((defined MTK_CLEARMOTION_SUPPORT))
        if (OMX_TRUE == mOutputAllocateBuffer || OMX_TRUE == needColorConvertWithNativeWindow())
        {
            for (OMX_U32 i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (mFrameBuf[i].ipOutputBuffer == pBuffHdr)
                {
                    if (OMX_TRUE == mFrameBuf[i].bUsed)
                    {
                        MTK_OMX_LOGD("@@ GetFrmStructure frm=0x%x, omx=0x%x, i=%d, color= %x, type= %x", &mFrameBuf[i].frame_buffer, pBuffHdr, i,
                                     mOutputPortFormat.eColorFormat, mInputPortFormat.eCompressionFormat);
                        //todo: cancel?
                        if ((meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_HEVC) ||
                            (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_VPX))
                        {
                            MTK_OMX_LOGD("@Flush Cache Before MDP");
                            mOutputMVAMgr->syncBufferCacheFrm((void *)mFrameBuf[i].frame_buffer.rBaseAddr.u4VA, (unsigned int)ION_CACHE_FLUSH_BY_RANGE);
                            sched_yield();
                            usleep(1000);       //For CTS checksum fail to make sure flush cache to dram
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGD("GetFrmStructure is not in used for convert, flag %x", mFrameBuf[i].ipOutputBuffer->nFlags);
                    }
                }
            }
        }
//#if  ((defined MTK_CLEARMOTION_SUPPORT))
    }
//#endif //((defined MTK_CLEARMOTION_SUPPORT))

//#if  ((defined MTK_CLEARMOTION_SUPPORT))
    if (OMX_TRUE == mMJCEnable && mRealCallBackFillBufferDone == OMX_FALSE)
    {
        OMX_ERRORTYPE err = OMX_ErrorNone;

//#ifdef MTK_CLEARMOTION_SUPPORT
        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] FBD %06x FBD (0x%08X) (0x%08X) %lld (%u)",
                         this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
        }
//#endif

        //MTK_OMX_LOGD("pBuffHdr->nTimeStamp -> %lld, length -> %d", pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
        if (pBuffHdr->nTimeStamp != 0)
        {
            eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_LAST_DISPLAY_TIME, (VAL_VOID_T *)&pBuffHdr->nTimeStamp, NULL);
        }

//#ifdef MTK_CLEARMOTION_SUPPORT

        MJCVdoBufQInsert(pBuffHdr);
        if (mUseClearMotion == OMX_TRUE && mMJCBufferCount == 0 && mScalerBufferCount == 0 && mDecoderInitCompleteFlag == OMX_TRUE && pBuffHdr->nFilledLen != 0)
        {
            //Bypass MJC if MJC/Scaler output buffers are not marked
            MJC_MODE mMode;
            mUseClearMotion = OMX_FALSE;
            mMode = MJC_MODE_BYPASS;
            m_fnMJCSetParam(mpMJC, MJC_PARAM_RUNTIME_DISABLE, &mMode);
            //mpMJC->mScaler.SetParameter(MJCScaler_PARAM_MODE, &mMode);
            MTK_OMX_LOGD("No MJC buffer or Scaler buffer (%d)(%d)", mMJCBufferCount, mScalerBufferCount);
        }

        // For Scaler ClearMotion +
        SIGNAL(mpMJC->mMJCFrameworkSem);
        // For Scaler ClearMotion -
//#endif
        return err;
    }
    else    // mRealCallBackFillBufferDone == OMX_TRUE
    {
//#endif
        OMX_ERRORTYPE err = OMX_ErrorNone;

        if (!(OMX_TRUE == mOutputAllocateBuffer || needColorConvertWithNativeWindow()))  // non color convert case will real callback
        {
            MTK_OMX_LOGUD("FBD (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d) frm_buf(0x%08X), flags(0x%08x)",
                         pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen,
                         mGET_DISP_i, mGET_DISP_tmp_frame_addr, pBuffHdr->nFlags);
            mGET_DISP_i = 0;
            mGET_DISP_tmp_frame_addr = 0;
        }

        if ((OMX_TRUE == mStoreMetaDataInBuffers) && (pBuffHdr->nFilledLen != 0))
        {
            OMX_U32 bufferType = *((OMX_U32 *)pBuffHdr->pBuffer);
            //MTK_OMX_LOGD("bufferType %d, %d, %d", bufferType, sizeof(VideoGrallocMetadata),
            //    sizeof(VideoNativeMetadata));
            // check buffer type
            if(kMetadataBufferTypeGrallocSource == bufferType)
            {
                pBuffHdr->nFilledLen = sizeof(VideoGrallocMetadata);//8
            }
            else if(kMetadataBufferTypeANWBuffer == bufferType)
            {
                pBuffHdr->nFilledLen = sizeof(VideoNativeMetadata);//12 in 32 bit
            }
        }

//#ifndef MTK_CLEARMOTION_SUPPORT
        if (mMJCEnable != OMX_TRUE)
        {
            //MTK_OMX_LOGD("pBuffHdr->nTimeStamp -> %lld, length -> %d", pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
            if (pBuffHdr->nTimeStamp != 0)
            {
                eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_LAST_DISPLAY_TIME, (VAL_VOID_T *)&pBuffHdr->nTimeStamp, NULL);
            }
        }
//#endif
        UpdateVideoInfo(pBuffHdr->nTimeStamp);
        HandleGrallocExtra(pBuffHdr);

#if (ANDROID_VER >= ANDROID_M)
        WaitFence(pBuffHdr, OMX_FALSE);
#endif

        if (OMX_TRUE == mOutputAllocateBuffer || OMX_TRUE == needColorConvertWithNativeWindow())
        {
            OMX_U32 i = 0;
            for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (mFrameBuf[i].ipOutputBuffer == pBuffHdr)
                {
                    MTK_OMX_LOGUD("@@ QueueOutputColorConvertSrcBuffer (0x%08X) (0x%08X) frm=0x%x, i=%d, pBuffHdr->nFlags = %x, used %d, EOS %d, t: %lld",
                                 pBuffHdr, pBuffHdr->pBuffer, &mFrameBuf[i].frame_buffer, i,
                                 pBuffHdr->nFlags, mFrameBuf[i].bUsed, mEOSFound, pBuffHdr->nTimeStamp);
                    QueueOutputColorConvertSrcBuffer(i);

                    break;
                }
                else
                {
                    //MTK_OMX_LOGE(" %d/%d, __LINE__ %d", i, mOutputPortDef.nBufferCountActual, __LINE__);
                }
            }
            if (mOutputPortDef.nBufferCountActual == i)
            {
                MTK_OMX_LOGD("[ERROR] QueueOutputColorConvertSrcBuffer out of range %d", i);
            }

        }
        else
        {
//#ifdef MTK_CROSSMOUNT_SUPPORT
            if( OMX_TRUE == mCrossMountSupportOn )
            {
            //VBufInfo info; //mBufInfo
            int ret = 0;
            buffer_handle_t _handle;
            if (OMX_TRUE == mStoreMetaDataInBuffers)
            {
                OMX_U32 graphicBufHandle = 0;
                GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
                ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &mBufInfo);
            }
            else
            {
                ret = mOutputMVAMgr->getOmxInfoFromVA((void *)pBuffHdr->pBuffer, &mBufInfo);
            }

            if (ret < 0)
            {
                MTK_OMX_LOGD("HandleGrallocExtra cannot find buffer info, LINE: %d", __LINE__);
            }
            else
            {
                MTK_OMX_LOGD("mBufInfo u4VA %x, u4PA %x, iIonFd %d", mBufInfo.u4VA,
                    mBufInfo.u4PA, mBufInfo.iIonFd);
                pBuffHdr->pPlatformPrivate = (OMX_U8 *)&mBufInfo;
            }

            pBuffHdr->nFlags |= OMX_BUFFERFLAG_VDEC_OUTPRIVATE;
            }
//#endif //MTK_CROSSMOUNT_SUPPORT

            LOCK_T(mFillThisBufQLock);
            mNumPendingOutput--;
            //MTK_OMX_LOGD ("FBD mNumPendingOutput(%d)", mNumPendingOutput);
            if (OMX_TRUE == mStoreMetaDataInBuffers && !mIsSecureInst)
            {
                int srcIdx = -1;
                int dstIdx = -1;
                int idx = 0;
                for (idx = 0; idx < mOutputPortDef.nBufferCountActual; idx++)
                {
                    if (0 == mFrameBufInt[idx].ionBufHandle)
                    {
                        dstIdx = idx;
                        break;
                    }
                }

                for (idx = 0; idx < mOutputPortDef.nBufferCountActual; idx++)
                {
                    if (mFrameBuf[idx].ipOutputBuffer == pBuffHdr)
                    {
                        srcIdx = idx;
                        break;
                    }
                }

                if (srcIdx > -1 && dstIdx > -1)
                {
                    //MTK_OMX_LOGE("[TEST] FBD copy mFrameBuf[%d] to mFrameBufInt[%d], hdr = 0x%x", srcIdx, dstIdx, pBuffHdr);
                    mFrameBufInt[dstIdx].bUsed = mFrameBuf[srcIdx].bUsed;
                    mFrameBufInt[dstIdx].bDisplay = mFrameBuf[srcIdx].bDisplay;
                    mFrameBufInt[dstIdx].bNonRealDisplay = mFrameBuf[srcIdx].bNonRealDisplay;
                    mFrameBufInt[dstIdx].bFillThis = mFrameBuf[srcIdx].bFillThis;
                    memcpy(&mFrameBufInt[dstIdx].frame_buffer, &mFrameBuf[srcIdx].frame_buffer, sizeof(mFrameBufInt[dstIdx].frame_buffer));
                    mFrameBufInt[dstIdx].iTimestamp = mFrameBuf[srcIdx].iTimestamp;
                    mFrameBufInt[dstIdx].bGraphicBufHandle = mFrameBuf[srcIdx].bGraphicBufHandle;
                    mFrameBufInt[dstIdx].ionBufHandle = mFrameBuf[srcIdx].ionBufHandle;
                    mFrameBufInt[dstIdx].ipOutputBuffer = mFrameBuf[srcIdx].ipOutputBuffer;
                    mFrameBufInt[dstIdx].refCount = mFrameBuf[srcIdx].refCount;
                    DecRef(BUFFER_OWNER_COMPONENT, mFrameBufInt[dstIdx].frame_buffer.rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));

                    mFrameBuf[srcIdx].bUsed = OMX_FALSE;
                    mFrameBuf[srcIdx].bDisplay = OMX_FALSE;
                    mFrameBuf[srcIdx].bNonRealDisplay = OMX_FALSE;
                    mFrameBuf[srcIdx].bFillThis = OMX_FALSE;
                    memset(&mFrameBuf[srcIdx].frame_buffer, 0, sizeof(VDEC_DRV_FRAMEBUF_T));
                    mFrameBuf[srcIdx].iTimestamp = 0;
                    mFrameBuf[srcIdx].bGraphicBufHandle = 0;
                    mFrameBuf[srcIdx].ionBufHandle = 0;
                    mFrameBuf[srcIdx].refCount = 0;
                }
                else
                {
                    MTK_OMX_LOGE("[ERROR] frame buffer array out of bound - src %d dst %d", srcIdx, dstIdx);
                }
            }
            UNLOCK(mFillThisBufQLock);
            mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                     mAppData,
                                     pBuffHdr);

            if (pBuffHdr->nFlags & OMX_BUFFERFLAG_EOS)
            {
                MTK_OMX_LOGD("[Info] %s callback event OMX_EventBufferFlag %d %d %d", __func__, mOutputAllocateBuffer, mThumbnailMode, needColorConvertWithNativeWindow());
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventBufferFlag,
                                       MTK_OMX_OUTPUT_PORT,
                                       pBuffHdr->nFlags,
                                       NULL);
            }
        }

        return err;
//#if  ((defined MTK_CLEARMOTION_SUPPORT))
    }
//#endif
}

OMX_BOOL MtkOmxVdec::GrallocExtraSetBufParameter(buffer_handle_t _handle,
                                                 VAL_UINT32_T gralloc_masks, VAL_UINT32_T gralloc_bits, OMX_TICKS nTimeStamp,
                                                 VAL_BOOL_T bIsMJCOutputBuffer, VAL_BOOL_T bIsScalerOutputBuffer)
{
    GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;
    gralloc_extra_ion_sf_info_t sf_info;
    VAL_UINT32_T uYAlign = 0;
    VAL_UINT32_T uCbCrAlign = 0;
    VAL_UINT32_T uHeightAlign = 0;

    err = gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
    if (GRALLOC_EXTRA_OK != err)
    {
        VAL_UINT32_T u4I;
        MTK_OMX_LOGE("GrallocExtraSetBufParameter(), gralloc_extra_query error:0x%x", err);
        return OMX_FALSE;
    }

    // buffer parameter
    gralloc_extra_sf_set_status(&sf_info, gralloc_masks, gralloc_bits);
    sf_info.videobuffer_status = 0;

    uYAlign = mQInfoOut.u4StrideAlign;
    uCbCrAlign = mQInfoOut.u4StrideAlign / 2;
    uHeightAlign = mQInfoOut.u4SliceHeightAlign;

    // Flexible YUV format, need to specify layout
    // I420 is always used for flexible yuv output with native window, no need to specify
    if ( !bIsMJCOutputBuffer && VAL_FALSE == mbYUV420FlexibleMode)
    {
        if (OMX_COLOR_FormatYUV420Planar == mOutputPortFormat.eColorFormat)
        {
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_I420);
        }
        else if (OMX_MTK_COLOR_FormatYV12 == mOutputPortFormat.eColorFormat)
        {
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
        }
        else if (OMX_COLOR_FormatYUV420SemiPlanar == mOutputPortFormat.eColorFormat)
        {
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12);
        }
    }

    if (bIsMJCOutputBuffer)
    {
        // MTK BLK alignment for MJC processed buffer
        uYAlign = 16;
        uCbCrAlign = 8;
        uHeightAlign = 32;
    }
    else if(bIsScalerOutputBuffer)
    {
        uYAlign = 16;
        uCbCrAlign = 16;
        uHeightAlign = 16;
    }
    else
    {
        // Android YV12 has 16/16/16 align, different from other YUV planar format
        if (OMX_MTK_COLOR_FormatYV12 == mOutputPortFormat.eColorFormat && uCbCrAlign < 16)
        {
            uCbCrAlign = 16;
        }
        else if (VAL_TRUE == mbYUV420FlexibleMode) // Internal color converted to fit standard I420
        {
            uYAlign = 2;
            uCbCrAlign = 1;
            uHeightAlign = 2;
        }

        if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_UFO)
        {
            if (mCodecId == MTK_VDEC_CODEC_ID_HEVC)
            {
                if ((mOutputPortDef.format.video.nFrameWidth / 64) == (mOutputPortDef.format.video.nStride / 64) &&
                    (mOutputPortDef.format.video.nFrameWidth / 64) == (mOutputPortDef.format.video.nSliceHeight / 64))
                {
                    MTK_OMX_LOGD("@@ UFO HandleGrallocExtra 64x64(0x%08X)", _handle);
                    uYAlign = 64;
                    uCbCrAlign = 32;
                    uHeightAlign = 64;
                }
                else if ((mOutputPortDef.format.video.nFrameWidth / 32) == (mOutputPortDef.format.video.nStride / 32) &&
                         (mOutputPortDef.format.video.nFrameWidth / 32) == (mOutputPortDef.format.video.nSliceHeight / 32))
                {
                    MTK_OMX_LOGD("@@ UFO HandleGrallocExtra 32x32(0x%08X)", _handle);
                    uYAlign = 32;
                    uCbCrAlign = 16;
                    uHeightAlign = 32;
                }
                else
                {
                    MTK_OMX_LOGD("@@ UFO HandleGrallocExtra 16x32(0x%08X)", _handle);
                }
            }
            else
            {
                MTK_OMX_LOGD("@@ UFO HandleGrallocExtra 16x32(0x%08X)", _handle);
            }
        }
    }

    //MTK_OMX_LOGD("@@ Video buffer status Y/Cb/Cr alignment =  %u/%u/%u, Height alignment = %u, Deinterlace = %u",
    //             uYAlign, uCbCrAlign, uCbCrAlign, uHeightAlign, (1u & mIsInterlacing));
    sf_info.videobuffer_status = (1 << 31) |
                                 ((uYAlign >> 1) << 25) |
                                 ((uCbCrAlign >> 1) << 19) |
                                 ((uHeightAlign >> 1) << 13) |
                                 ((mIsInterlacing & 1) << 12);
    //MTK_OMX_LOGD("@@ Video buffer status 0x%x", sf_info.videobuffer_status);

    // timestamp, u32
    sf_info.timestamp = nTimeStamp / 1000;
    //MTK_OMX_LOGD("GrallocExtraSetBufParameter(), timestamp: %d", sf_info.timestamp);


    err = gralloc_extra_perform(_handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
    if (GRALLOC_EXTRA_OK != err)
    {
        MTK_OMX_LOGE("GrallocExtraSetBufParameter(), gralloc_extra_perform error:0x%x", err);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::HandleGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    VAL_UINT32_T u4I;

    VAL_UINT32_T gralloc_masks = GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY;
    VAL_UINT32_T gralloc_bits = GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY;

    if (NULL == pBuffHdr)
    {
        return OMX_FALSE;
    }

    if (mOutputPortDef.format.video.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
    {
        gralloc_masks |= GRALLOC_EXTRA_MASK_CM;
        gralloc_bits |= GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM;
    }

    gralloc_masks |= GRALLOC_EXTRA_MASK_YUV_COLORSPACE;

    if (meDecodeType == VDEC_DRV_DECODER_MTK_HARDWARE)
    {
        gralloc_masks |= GRALLOC_EXTRA_MASK_FLUSH;
        gralloc_bits |= GRALLOC_EXTRA_BIT_NOFLUSH;
    }

    if (OMX_COLOR_PRIMARIES_BT601 == pBuffHdr->eColourPrimaries)
    {
        if (pBuffHdr->bVideoRangeExist && 1 == pBuffHdr->u4VideoRange)
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
        }
        else
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
        }
    }
    else if (OMX_COLOR_PRIMARIES_BT709 == pBuffHdr->eColourPrimaries)
    {
        if (pBuffHdr->bVideoRangeExist && 1 == pBuffHdr->u4VideoRange)
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT709_FULL;
        }
        else
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT709_NARROW;
        }
    }
    else if (OMX_COLOR_PRIMARIES_BT2020 == pBuffHdr->eColourPrimaries)
    {
        if (pBuffHdr->bVideoRangeExist && 1 == pBuffHdr->u4VideoRange)
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT2020_FULL;
        }
        else
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW;
        }
    }
    else
    {
        gralloc_bits |= GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
    }

    //MTK_OMX_LOGD("bColourPrimariesExist: %d, eColourPrimaries: %d, bVideoRangeExits: %d, u4VideoRange: %d, gralloc_bits 0x%x",
    //    pBuffHdr->bColourPrimariesExist, pBuffHdr->eColourPrimaries ,pBuffHdr->bVideoRangeExist, pBuffHdr->u4VideoRange, gralloc_bits);
    if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo info;
        int ret = 0;
        buffer_handle_t _handle;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)pBuffHdr->pBuffer, &info);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD("HandleGrallocExtra cannot find buffer info, LINE: %d", __LINE__);
        }

        _handle = (buffer_handle_t)info.pNativeHandle;


                if (m3DStereoMode == OMX_VIDEO_H264FPA_SIDEBYSIDE) {
                    gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
                    gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_SBS;
                }
                else if (m3DStereoMode == OMX_VIDEO_H264FPA_TOPANDBOTTOM) {
                    gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
                    gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_TAB;
                }
                else {
                    gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
                    gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_2D;
                }



        VAL_BOOL_T bIsMJCOutputBuffer =(mMJCEnable == OMX_TRUE && OMX_TRUE == MJCIsMJCOutBuf(pBuffHdr));
        VAL_BOOL_T bIsScalerOutputBuffer =(mMJCEnable == OMX_TRUE && OMX_TRUE == MJCIsScalerBufandScalerEbable(pBuffHdr));
        if( NULL != _handle)
        {
            GrallocExtraSetBufParameter(_handle, gralloc_masks, gralloc_bits, pBuffHdr->nTimeStamp, bIsMJCOutputBuffer, bIsScalerOutputBuffer);
        }
        else
        {
            //it should be handle the NULL case in non-meta mode
            MTK_OMX_LOGD ("GrallocExtraSetBufParameter handle is null, skip once");
        }
        //__setBufParameter(_handle, gralloc_masks, gralloc_bits);

    }

    if (OMX_TRUE == mIsSecureInst)
    {
        for (u4I = 0; u4I < mSecFrmBufCount; u4I++)
        {
            if (((VAL_UINT32_T)mSecFrmBufInfo[u4I].u4BuffHdr == (VAL_UINT32_T)pBuffHdr) && (((VAL_UINT32_T)0xffffffff) != (VAL_UINT32_T)mSecFrmBufInfo[u4I].pNativeHandle))
            {
                buffer_handle_t _handle = (buffer_handle_t)mSecFrmBufInfo[u4I].pNativeHandle;
                MTK_OMX_LOGD("@@ HandleGrallocExtra(secure) -  (0x%08X)", _handle);
                GrallocExtraSetBufParameter(_handle,
                                            GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
                                            GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY,
                                            pBuffHdr->nTimeStamp, VAL_FALSE, VAL_FALSE);
                __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK); // Use MTK BLK format
                //__setBufParameter(_handle, GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY, GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);
                break;
            }
        }
    }

    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        OMX_U32 graphicBufHandle = 0;
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

        if (m3DStereoMode == OMX_VIDEO_H264FPA_SIDEBYSIDE) {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_SBS;
        }
        else if (m3DStereoMode == OMX_VIDEO_H264FPA_TOPANDBOTTOM) {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_TAB;
        }
        else {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_2D;
        }

        VAL_BOOL_T bIsMJCOutputBuffer =(mMJCEnable == OMX_TRUE && OMX_TRUE == MJCIsMJCOutBuf(pBuffHdr));
        VAL_BOOL_T bIsScalerOutputBuffer =(mMJCEnable == OMX_TRUE && OMX_TRUE == MJCIsScalerBufandScalerEbable(pBuffHdr));
        GrallocExtraSetBufParameter((buffer_handle_t)graphicBufHandle, gralloc_masks, gralloc_bits, pBuffHdr->nTimeStamp, bIsMJCOutputBuffer, bIsScalerOutputBuffer);
        //__setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY, GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);
    }else{
        MTK_OMX_LOGD("pBuffHdr = %#x",(VAL_UINT32_T)pBuffHdr);
    }

    return OMX_TRUE;
}

void MtkOmxVdec::ReturnPendingInputBuffers()
{
    LOCK_T(mEmptyThisBufQLock);

    if (mNeedMoreOutputBuf == OMX_TRUE
        && ((mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC
             && meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)      //For SW H265 to avoid free input buffer twice
            ||
            (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC
             && meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE))   //For Hybrid/SW H264 to avoid free input buffer twice
       )
    {
        MTK_OMX_LOGE("[ReturnPendingInputBuffers] Don't free this buffer!!");
#if CPP_STL_SUPPORT
        mEmptyThisBufQ.erase(mEmptyThisBufQ.begin());
#endif

#if ANDROID
        mEmptyThisBufQ.removeAt(0);
#endif
    }


#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        int input_idx = (*iter);
        if (mNumPendingInput > 0)
        {
            mNumPendingInput--;
        }
        else
        {
            MTK_OMX_LOGE("[ERROR] mNumPendingInput == 0 and want to --");
        }
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ[input_idx]]);
    }
    mEmptyThisBufQ.clear();
#endif

#if ANDROID
    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        if (mNumPendingInput > 0)
        {
            mNumPendingInput--;
        }
        else
        {
            MTK_OMX_LOGE("[ERROR] mNumPendingInput == 0 (%d)(0x%08X)", i, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
        }
        MTK_OMX_LOGUD("EBD (Pending_ETQ) (0x%08X) (0x%08X) (%d) (%d %d)", mInputBufferHdrs[mEmptyThisBufQ[i]], mInputBufferHdrs[mEmptyThisBufQ[i]]->pBuffer, mNumPendingInput, mNumPendingInput, mEmptyThisBufQ.size());
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
    mEmptyThisBufQ.clear();
#endif

    UNLOCK(mEmptyThisBufQLock);
}


void MtkOmxVdec::ReturnPendingOutputBuffers()
{
    LOCK_T(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mFillThisBufQ.begin(); iter != mFillThisBufQ.end(); iter++)
    {
        int output_idx = (*iter);
        mNumPendingOutput--;
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mFillThisBufQ[output_idx]]);
    }
    mFillThisBufQ.clear();
#endif

#if ANDROID
    OMX_ERRORTYPE err = OMX_ErrorNone;

    for (size_t i = 0 ; i < mBufColorConvertDstQ.size() ; i++)
    {

#if (ANDROID_VER >= ANDROID_M)
        WaitFence(mOutputBufferHdrs[mBufColorConvertDstQ[i]], OMX_FALSE);
#endif
        mNumPendingOutput--;

        mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nFilledLen = 0;
        mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nTimeStamp = 0;
        MTK_OMX_LOGUD("FBD (Pending_CCDQ) (0x%08X) (0x%08X) %lld (%u) flags(0x%08x)",
                     mOutputBufferHdrs[mBufColorConvertDstQ[i]], mOutputBufferHdrs[mBufColorConvertDstQ[i]]->pBuffer, mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nTimeStamp, mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nFilledLen,
                     mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nFlags);
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mBufColorConvertDstQ[i]]);
    }
    mBufColorConvertDstQ.clear();
    for (size_t i = 0 ; i < mBufColorConvertSrcQ.size() ; i++)
    {

#if (ANDROID_VER >= ANDROID_M)
        WaitFence(mOutputBufferHdrs[mBufColorConvertSrcQ[i]], OMX_FALSE);
#endif
        mNumPendingOutput--;

        mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nFilledLen = 0;
        mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nTimeStamp = 0;
        MTK_OMX_LOGUD("FBD (Pending_CCSQ) (0x%08X) (0x%08X) %lld (%u) flags(0x%08x)",
                     mOutputBufferHdrs[mBufColorConvertSrcQ[i]], mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->pBuffer, mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nTimeStamp, mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nFilledLen,
                     mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nFlags);
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mBufColorConvertSrcQ[i]]);
    }
    mBufColorConvertSrcQ.clear();

    for (size_t i = 0 ; i < mFillThisBufQ.size() ; i++)
    {

#if (ANDROID_VER >= ANDROID_M)
        WaitFence(mOutputBufferHdrs[mFillThisBufQ[i]], OMX_FALSE);
#endif
        mNumPendingOutput--;
        mOutputBufferHdrs[mFillThisBufQ[i]]->nFilledLen = 0;
        mOutputBufferHdrs[mFillThisBufQ[i]]->nTimeStamp = 0;

        if (OMX_TRUE ==  mStoreMetaDataInBuffers)
        {
            for (size_t j = 0; j < mOutputPortDef.nBufferCountActual; j++)
            {
                if (mFrameBuf[j].ipOutputBuffer == mOutputBufferHdrs[mFillThisBufQ[i]] && mFrameBuf[j].ionBufHandle > 0)
                {
                    //MTK_OMX_LOGE("[TEST] ReturnPendingOutputBuffers release MVA index = %d, orig hdr = 0x%x", j, mFrameBufInt[j].ipOutputBuffer);
                    DecRef(BUFFER_OWNER_COMPONENT, mFrameBuf[j].frame_buffer.rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
                }
            }
        }


        MTK_OMX_LOGD("FBD (Pending_FTQ) (0x%08X) (0x%08X) %lld (%u) flags(0x%08x)",
                     mOutputBufferHdrs[mFillThisBufQ[i]], mOutputBufferHdrs[mFillThisBufQ[i]]->pBuffer, mOutputBufferHdrs[mFillThisBufQ[i]]->nTimeStamp, mOutputBufferHdrs[mFillThisBufQ[i]]->nFilledLen,
                     mOutputBufferHdrs[mFillThisBufQ[i]]->nFlags);
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mFillThisBufQ[i]]);
    }
    mFillThisBufQ.clear();
#endif

    UNLOCK(mFillThisBufQLock);
}


void MtkOmxVdec::DumpETBQ()
{
    MTK_OMX_LOGD("--- ETBQ: mNumPendingInput %d; mEmptyThisBufQ.size() %d", (int)mNumPendingInput, mEmptyThisBufQ.size());

    /*
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        MTK_OMX_LOGD("[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
#endif


#if ANDROID
    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        MTK_OMX_LOGD("[%d] - pBuffHead(0x%08X)", mEmptyThisBufQ[i], (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
#endif
    */
}




void MtkOmxVdec::DumpFTBQ()
{
    MTK_OMX_LOGD("--- FTBQ: mNumPendingOutput %d; mFillThisBufQ.size() %d,  CCDst: %d, CCSrc: %d",
                 (int)mNumPendingOutput, mFillThisBufQ.size(), mBufColorConvertDstQ.size(),
                 mBufColorConvertSrcQ.size());
    /*
    LOCK_T(mFillThisBufQLock);
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mFillThisBufQ.begin(); iter != mFillThisBufQ.end(); iter++)
    {
        MTK_OMX_LOGU("[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }
#endif

#if ANDROID
    for (size_t i = 0 ; i < mFillThisBufQ.size() ; i++)
    {
        MTK_OMX_LOGU("[%d] - pBuffHead(0x%08X)", mFillThisBufQ[i], (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }

    for (size_t i = 0 ; i < mBufColorConvertDstQ.size() ; i++)
    {
        MTK_OMX_LOGU("[%d] - pCCDstBuffHead(0x%08X)", mBufColorConvertDstQ[i], (unsigned int)mOutputBufferHdrs[mBufColorConvertDstQ[i]]);
    }
    for (size_t i = 0 ; i < mBufColorConvertSrcQ.size() ; i++)
    {
        MTK_OMX_LOGU("[%d] - pCCSrcBuffHead(0x%08X)", mBufColorConvertSrcQ[i], (unsigned int)mOutputBufferHdrs[mBufColorConvertSrcQ[i]]);
    }

#endif
    UNLOCK(mFillThisBufQLock);
    */
}



int MtkOmxVdec::findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_BUFFERHEADERTYPE **pBufHdrPool = NULL;
    int bufCount;

    if (nPortIndex == MTK_OMX_INPUT_PORT)
    {
        pBufHdrPool = mInputBufferHdrs;
        bufCount = mInputPortDef.nBufferCountActual;
    }
    else if (nPortIndex == MTK_OMX_OUTPUT_PORT)
    {
        pBufHdrPool = mOutputBufferHdrs;
        bufCount = mOutputPortDef.nBufferCountActual;
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] findBufferHeaderIndex invalid index(0x%X)", (unsigned int)nPortIndex);
        return -1;
    }

    for (int i = 0 ; i < bufCount ; i++)
    {
        if (pBuffHdr == pBufHdrPool[i])
        {
            // index found
            return i;
        }
    }

    MTK_OMX_LOGE("[ERROR] findBufferHeaderIndex not found. Port = %u, BufferHeader = 0x%x", nPortIndex, pBuffHdr);
    return -1; // nothing found
}

void MtkOmxVdec::printOmxColorFormat(OMX_COLOR_FORMATTYPE format){

    switch(format){
    case OMX_COLOR_FormatUnused:
        MTK_OMX_LOGD( "COLOR_FormatUnused");
        break;
    case OMX_COLOR_FormatMonochrome:
        MTK_OMX_LOGD( "OMX_COLOR_FormatMonochrome");
        break;
    case OMX_COLOR_Format8bitRGB332:
        MTK_OMX_LOGD( "OMX_COLOR_Format8bitRGB332");
        break;
    case OMX_COLOR_Format12bitRGB444:
        MTK_OMX_LOGD( "OMX_COLOR_Format12bitRGB444");
        break;
    case OMX_COLOR_Format16bitARGB4444:
        MTK_OMX_LOGD( "OMX_COLOR_Format16bitARGB4444");
        break;
    case OMX_COLOR_Format16bitARGB1555:
        MTK_OMX_LOGD( "OMX_COLOR_Format16bitARGB1555");
        break;
    case OMX_COLOR_Format16bitRGB565:
        MTK_OMX_LOGD( "OMX_COLOR_Format16bitRGB565");
        break;
    case OMX_COLOR_Format16bitBGR565:
        MTK_OMX_LOGD( "OMX_COLOR_Format16bitBGR565");
        break;
    case OMX_COLOR_Format18bitRGB666:
        MTK_OMX_LOGD( "OMX_COLOR_Format18bitRGB666");
        break;
    case OMX_COLOR_Format18bitARGB1665:
        MTK_OMX_LOGD( "OMX_COLOR_Format18bitARGB1665");
        break;
    case OMX_COLOR_Format19bitARGB1666:
        MTK_OMX_LOGD( "OMX_COLOR_Format19bitARGB1666");
        break;
    case OMX_COLOR_Format24bitRGB888:
        MTK_OMX_LOGD( "OMX_COLOR_Format24bitRGB888");
        break;
    case OMX_COLOR_Format24bitBGR888:
        MTK_OMX_LOGD( "OMX_COLOR_Format24bitBGR888");
        break;
    case OMX_COLOR_Format24bitARGB1887:
        MTK_OMX_LOGD( "OMX_COLOR_Format24bitARGB1887");
        break;
    case OMX_COLOR_Format25bitARGB1888:
        MTK_OMX_LOGD( "OMX_COLOR_Format25bitARGB1888");
        break;
    case OMX_COLOR_Format32bitBGRA8888:
        MTK_OMX_LOGD( "OMX_COLOR_Format32bitBGRA8888");
        break;
    case OMX_COLOR_Format32bitARGB8888:
        MTK_OMX_LOGD( "OMX_COLOR_Format32bitARGB8888");
        break;
    case OMX_COLOR_FormatYUV411Planar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV411Planar");
        break;
    case OMX_COLOR_FormatYUV411PackedPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV411PackedPlanar");
        break;
    case OMX_COLOR_FormatYUV420Planar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV420Planar");
        break;
    case OMX_COLOR_FormatYUV420PackedPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV420PackedPlanar");
        break;
    case OMX_COLOR_FormatYUV420SemiPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV420SemiPlanar");
        break;
    case OMX_COLOR_FormatYUV422Planar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV422Planar");
        break;
    case OMX_COLOR_FormatYUV422PackedPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV422PackedPlanar");
        break;
    case OMX_COLOR_FormatYUV422SemiPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV422SemiPlanar");
        break;
    case OMX_COLOR_FormatYCbYCr:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYCbYCr");
        break;
    case OMX_COLOR_FormatYCrYCb:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYCrYCb");
        break;
    case OMX_COLOR_FormatCbYCrY:
        MTK_OMX_LOGD( "OMX_COLOR_FormatCbYCrY");
        break;
    case OMX_COLOR_FormatCrYCbY:
        MTK_OMX_LOGD( "OMX_COLOR_FormatCrYCbY");
        break;
    case OMX_COLOR_FormatYUV444Interleaved:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV444Interleaved");
        break;
    case OMX_COLOR_FormatRawBayer8bit:
        MTK_OMX_LOGD( "OMX_COLOR_FormatRawBayer8bit");
        break;
    case OMX_COLOR_FormatRawBayer10bit:
        MTK_OMX_LOGD( "OMX_COLOR_FormatRawBayer10bit");
        break;
    case OMX_COLOR_FormatRawBayer8bitcompressed:
        MTK_OMX_LOGD( "OMX_COLOR_FormatRawBayer8bitcompressed");
        break;
    case OMX_COLOR_FormatL2:
        MTK_OMX_LOGD( "OMX_COLOR_FormatL2");
        break;
    case OMX_COLOR_FormatL4:
        MTK_OMX_LOGD( "OMX_COLOR_FormatL4");
        break;
    case OMX_COLOR_FormatL8:
        MTK_OMX_LOGD( "OMX_COLOR_FormatL8");
        break;
    case OMX_COLOR_FormatL16:
        MTK_OMX_LOGD( "OMX_COLOR_FormatL16");
        break;
    case OMX_COLOR_FormatL24:
        MTK_OMX_LOGD( "OMX_COLOR_FormatL24");
        break;
    case OMX_COLOR_FormatL32:
        MTK_OMX_LOGD( "OMX_COLOR_FormatL32");
        break;
    case OMX_COLOR_FormatYUV420PackedSemiPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV420PackedSemiPlanar");
        break;
    case OMX_COLOR_FormatYUV422PackedSemiPlanar:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV422PackedSemiPlanar");
        break;
    case OMX_COLOR_Format18BitBGR666:
        MTK_OMX_LOGD( "OMX_COLOR_Format18BitBGR666");
        break;
    case OMX_COLOR_Format24BitARGB6666:
        MTK_OMX_LOGD( "OMX_COLOR_Format24BitARGB6666");
        break;
    case OMX_COLOR_Format24BitABGR6666:
        MTK_OMX_LOGD( "OMX_COLOR_Format24BitABGR6666");
        break;
    case OMX_COLOR_FormatAndroidOpaque:
        MTK_OMX_LOGD( "OMX_COLOR_FormatAndroidOpaque");
        break;
    case OMX_COLOR_FormatYUV420Flexible:
        MTK_OMX_LOGD( "OMX_COLOR_FormatYUV420Flexible");
        break;
    case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
        MTK_OMX_LOGD( "OMX_TI_COLOR_FormatYUV420PackedSemiPlanar");
        break;
    case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
        MTK_OMX_LOGD( "OMX_QCOM_COLOR_FormatYVU420SemiPlanar");
        break;
    case OMX_COLOR_FormatVendorMTKYUV:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV");
        break;
    case OMX_COLOR_FormatVendorMTKYUV_FCM:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV_FCM");
        break;
    case OMX_COLOR_FormatVendorMTKYUV_UFO:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV_UFO");
        break;
    case OMX_COLOR_FormatVendorMTKYUV_10BIT_H:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV_10BIT_H");
        break;
    case OMX_COLOR_FormatVendorMTKYUV_10BIT_V:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV_10BIT_V");
        break;
    case OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H");
        break;
    case OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V:
        MTK_OMX_LOGD( "OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V");
        break;
    default:
        MTK_OMX_LOGD( "COLOR_FormatUnused");
        break;
    }

}

OMX_ERRORTYPE MtkOmxVdec::QureyVideoProfileLevel
(
    VAL_UINT32_T                        u4VideoFormat,
    OMX_VIDEO_PARAM_PROFILELEVELTYPE     *pProfileLevel,
    MTK_VDEC_PROFILE_MAP_ENTRY           *pProfileMapTable,
    VAL_UINT32_T                        nProfileMapTableSize,
    MTK_VDEC_LEVEL_MAP_ENTRY             *pLevelMapTable,
    VAL_UINT32_T                        nLevelMapTableSize
)
{
    VAL_UINT32_T nProfileLevelMapTableSize = nProfileMapTableSize * nLevelMapTableSize;

    /* nProfileIndex == -1 for query divx license for DIVX related decoder.
       reset nProfileIndex for following loop*/
    if(pProfileLevel->nProfileIndex == -1)
        pProfileLevel->nProfileIndex = 0;

    if (pProfileLevel->nProfileIndex >= nProfileLevelMapTableSize)
    {
        return OMX_ErrorNoMore;
    }
    else
    {
        VDEC_DRV_QUERY_VIDEO_FORMAT_T qInfo;
        VDEC_DRV_MRESULT_T nDrvRet;
        VAL_UINT32_T nProfileMapIndex;
        VAL_UINT32_T nLevelMapIndex;

        if( 0 >= nLevelMapTableSize )
        {
            MTK_OMX_LOGE("[Error] nLevelMapTableSize(%d) <= 0 ",nLevelMapTableSize);
            return OMX_ErrorNoMore;
        }
        // Loop until the supported profile-level found, or reach the end of table
        while (pProfileLevel->nProfileIndex < nProfileLevelMapTableSize)
        {

            nProfileMapIndex = pProfileLevel->nProfileIndex / nLevelMapTableSize;
            nLevelMapIndex = pProfileLevel->nProfileIndex % nLevelMapTableSize;
            memset(&qInfo, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));

            // Query driver to see if supported
            qInfo.u4VideoFormat = u4VideoFormat;
            qInfo.u4Profile = pProfileMapTable[nProfileMapIndex].u4Profile;
            qInfo.u4Level = pLevelMapTable[nLevelMapIndex].u4Level;
            qInfo.u4Width = 16;
            qInfo.u4Height = 16;
            nDrvRet = eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_VIDEO_FORMAT, &qInfo, 0);

            if (VDEC_DRV_MRESULT_OK == nDrvRet)
            {
                // If supported, return immediately
                pProfileLevel->eProfile = pProfileMapTable[nProfileMapIndex].profile;
                pProfileLevel->eLevel = pLevelMapTable[nLevelMapIndex].level;
/*
                MTK_OMX_LOGU("Supported nProfileIndex %d, eProfile 0x%x, eLevel 0x%x",
                             pProfileLevel->nProfileIndex,
                             pProfileLevel->eProfile,
                             pProfileLevel->eLevel);
*/
                return OMX_ErrorNone;
            }
            else if (pProfileLevel->nProfileIndex + 1 >= nProfileLevelMapTableSize)
            {
                // Reach the end of table ?
                return OMX_ErrorNoMore;
            }

            // If not supported, continue checking the rest of table ...
            pProfileLevel->nProfileIndex++;
        }
    }

    return OMX_ErrorNoMore;
}

OMX_ERRORTYPE MtkOmxVdec::QueryDivxLicense(OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pProfileLevel)
{
    OMX_ERRORTYPE nDrvRet;

    nDrvRet = QureyVideoProfileLevel(MTK_VDEC_CODEC_ID_DIVX, pProfileLevel,
                                 MPEG4ProfileMapTable, MAX_MPEG4_PROFILE_MAP_TABLE_SIZE,
                                 MPEG4LevelMapTable, MAX_MPEG4_LEVEL_MAP_TABLE_SIZE);

    if(nDrvRet == OMX_ErrorNone)
    {
        return OMX_ErrorNone;
    }
    else
    {
        MTK_OMX_LOGE("QueryDivxLicense err:%d\n", nDrvRet);
        return OMX_ErrorNoMore;
    }

}

OMX_BOOL MtkOmxVdec::NotInETBQ(OMX_BUFFERHEADERTYPE *ipInputBuf)
{

    LOCK_T(mEmptyThisBufQLock);
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        MTK_OMX_LOGD("[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
#endif


#if ANDROID
    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        if (ipInputBuf == mInputBufferHdrs[mEmptyThisBufQ[i]])
        {
            //MTK_OMX_LOGD ("[%d] - pBuffHead(0x%08X)", mEmptyThisBufQ[i], (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
            UNLOCK(mEmptyThisBufQLock);
            return OMX_TRUE;
        }
    }
#endif
    UNLOCK(mEmptyThisBufQLock);
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::IsFreeBuffer(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    OMX_U32 i;

#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers && (NULL != ipOutputBuffer))
    {
        OMX_BOOL bHeaderExists = OMX_FALSE;
        OMX_BOOL bBufferExists = OMX_FALSE;
        OMX_BOOL bUsed = OMX_TRUE;
        for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
        {
            if (mIsSecureInst)
            {
                if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer && OMX_FALSE == mFrameBuf[i].bUsed)
                {
                    return OMX_TRUE;
                }
            }
            else
            {
            if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
            {
                bHeaderExists = OMX_TRUE;
                if (mFrameBuf[i].ionBufHandle > 0)
                {
                    bBufferExists = OMX_TRUE;
                    bUsed = mFrameBuf[i].bUsed;
                }
            }
            if (mFrameBuf[i].ionBufHandle > 0 && mFrameBuf[i].ipOutputBuffer == ipOutputBuffer && OMX_FALSE == mFrameBuf[i].bUsed)
            {
                return OMX_TRUE;
                }
            }
        }
        //MTK_OMX_LOGE("[ERROR] IsFreeBuffer Hdr = 0x%x, hdr/buf/used = %d/%d/%d", ipOutputBuffer, bHeaderExists, bBufferExists, bUsed);
        return OMX_FALSE;
    }

#endif

    if (NULL != ipOutputBuffer)
    {
        for (i = 0; i < mOutputPortDef.nBufferCountActual; i++)
        {
            if (OMX_TRUE == mFrameBuf[i].bUsed)
            {
                if (ipOutputBuffer == (OMX_BUFFERHEADERTYPE *)mFrameBuf[i].ipOutputBuffer)
                {
                    return OMX_FALSE;
                }
            }
        }
    }
    return OMX_TRUE;
}

void MtkOmxVdec::InsertInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf)
{
    OMX_U32 i = 0;
    for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
    {
        if (mInputBuf[i].ipInputBuffer == NULL)
        {
            mInputBuf[i].ipInputBuffer = ipInputBuf;
            mInputBuf[i].bUsed = OMX_FALSE;
            //MTK_OMX_LOGD("InsertInputBuf() (%d 0x%08x)", i, mInputBuf[i].ipInputBuffer);
            break;
        }
    }
}

void MtkOmxVdec::RemoveInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf)
{
    OMX_U32 i = 0;
    for (i = 0; i < mInputPortDef.nBufferCountActual; i++)
    {
        if (mInputBuf[i].ipInputBuffer == ipInputBuf)
        {
            mInputBuf[i].ipInputBuffer = NULL;
            mInputBuf[i].bUsed = OMX_FALSE;
            MTK_OMX_LOGD("RemoveInputBuf frm=0x%x, omx=0x%x, i=%d", &mInputBuf[i].InputBuf, ipInputBuf, i);
            return;
        }
    }
    MTK_OMX_LOGE("[ERROR] RemoveInputBuf not found");
}

#if (ANDROID_VER >= ANDROID_KK)
OMX_BOOL MtkOmxVdec::GetMetaHandleFromOmxHeader(OMX_BUFFERHEADERTYPE *pBufHdr, OMX_U32 *pBufferHandle)
{
    OMX_U32 bufferType = *((OMX_U32 *)pBufHdr->pBuffer);
    // check buffer type
    if(kMetadataBufferTypeGrallocSource == bufferType)
    {
    //buffer_handle_t _handle = *((buffer_handle_t*)(pBufHdr->pBuffer + 4));
    *pBufferHandle = *((OMX_U32 *)(pBufHdr->pBuffer + 4));
    // MTK_OMX_LOGE("GetMetaHandleFromOmxHeader 0x%x", *pBufferHandle);
    }
#if (ANDROID_VER >= ANDROID_M)
    else if(kMetadataBufferTypeANWBuffer == bufferType)
    {
        ANativeWindowBuffer* pNWBuffer = *((ANativeWindowBuffer**)(pBufHdr->pBuffer + 4));
        *pBufferHandle = (OMX_U32)pNWBuffer->handle;
    }
#endif
    else
    {
        MTK_OMX_LOGD("Warning: BufferType is not Gralloc Source !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}
OMX_BOOL MtkOmxVdec::GetMetaHandleFromBufferPtr(OMX_U8 *pBuffer, OMX_U32 *pBufferHandle)
{
    OMX_U32 bufferType = *((OMX_U32 *)pBuffer);
    // check buffer type
    if(kMetadataBufferTypeGrallocSource == bufferType)
    {
        //buffer_handle_t _handle = *((buffer_handle_t*)(pBufHdr->pBuffer + 4));
        *pBufferHandle = *((OMX_U32 *)(pBuffer + 4));
    }
#if (ANDROID_VER >= ANDROID_M)
    else if(kMetadataBufferTypeANWBuffer == bufferType)
    {
        ANativeWindowBuffer* pNWBuffer = *((ANativeWindowBuffer**)(pBuffer + 4));
        *pBufferHandle = (OMX_U32)pNWBuffer->handle;
        //OMX_U32 pNWBuffer = *((OMX_U32 *)(pBuffer + 4));
        //*pBufferHandle = (ANativeWindowBuffer*)pNWBuffer->handle;
    }
#endif
    else
    {
        MTK_OMX_LOGD("Warning: BufferType is not Gralloc Source !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }

    return OMX_TRUE;
}
#endif

OMX_BOOL MtkOmxVdec::GetIonHandleFromGraphicHandle(OMX_U32 *pBufferHandle, int *pIonHandle)
{
    int ionFd = -1;
    ion_user_handle_t ionHandle;
    gralloc_extra_query((buffer_handle_t)*pBufferHandle, GRALLOC_EXTRA_GET_ION_FD, &ionFd);
    if (-1 == mIonDevFd)
    {
        mIonDevFd = eVideoOpenIonDevFd();
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE("[ERROR] cannot open ION device. LINE: %d", __LINE__);
            return OMX_FALSE;
        }
    }

    if (ionFd > 0)
    {
        if (ion_import(mIonDevFd, ionFd, &ionHandle))
        {
            MTK_OMX_LOGE("[ERROR] ion_import failed, LINE: %d", __LINE__);
            return OMX_FALSE;
        }
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] query ion fd failed(%d), LINE: %d", ionFd, __LINE__);
        return OMX_FALSE;
    }

    *pIonHandle = ionHandle;
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::FreeIonHandle(int ionHandle)
{
    if (-1 == mIonDevFd)
    {
        mIonDevFd = eVideoOpenIonDevFd();
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE("[ERROR] cannot open ION device. LINE: %d", __LINE__);
            return OMX_FALSE;
        }
    }

    if (ion_free(mIonDevFd, ionHandle))
    {
        MTK_OMX_LOGE("[ERROR] cannot free ion handle(%d). LINE: %d", ionHandle, __LINE__);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

void MtkOmxVdec::InsertFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    //MTK_OMX_LOGE(" output buffer count %d", mOutputPortDef.nBufferCountActual);
    LOCK_T(mFillThisBufQLock);
    for (OMX_U32 i = 0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        if (mFrameBuf[i].ipOutputBuffer == NULL)
        {
            mFrameBuf[i].ipOutputBuffer = ipOutputBuffer;
            mFrameBuf[i].bUsed = OMX_FALSE;
            MTK_OMX_LOGD("InsertFrmBuf , omx=0x%X, i=%d", ipOutputBuffer, i);
            break;
        }
    }
    UNLOCK(mFillThisBufQLock);
}

void MtkOmxVdec::RemoveFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    LOCK_T(mFillThisBufQLock);
    for (OMX_U32 i = 0; i < mOutputBufferHdrsCnt; i++)
    {
        if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
        {
            mFrameBuf[i].ipOutputBuffer = NULL;
            mFrameBuf[i].bUsed = OMX_FALSE;
            MTK_OMX_LOGD("RemoveFrmBuf frm=0x%x, omx=0x%x, i=%d", &mFrameBuf[i].frame_buffer, ipOutputBuffer, i);
            UNLOCK(mFillThisBufQLock);
            return;
        }
    }
    UNLOCK(mFillThisBufQLock);
    MTK_OMX_LOGE("[ERROR] RemoveFrmBuf not found");
}

OMX_BOOL MtkOmxVdec::PutFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    LOCK_T(mFillThisBufQLock);
    for (OMX_U32 i = 0; i < mOutputPortDef.nBufferCountActual; i++)
    {
        if (OMX_TRUE == mFrameBuf[i].bUsed)
        {
            if (ipOutputBuffer == mFrameBuf[i].ipOutputBuffer)
            {
                mFrameBuf[i].bUsed = OMX_FALSE;
                UNLOCK(mFillThisBufQLock);
                return OMX_TRUE;
            }
        }
    }
    UNLOCK(mFillThisBufQLock);
    MTK_OMX_LOGE("[ERROR] PutFrmBuf not found");
    return OMX_FALSE;
}

VDEC_DRV_VIDEO_FORMAT_T MtkOmxVdec::GetVdecFormat(MTK_VDEC_CODEC_ID codecId)
{
    switch (codecId)
    {

        case MTK_VDEC_CODEC_ID_HEVC:
            if (OMX_TRUE == mIsSecureInst)
            {
                return VDEC_DRV_VIDEO_FORMAT_H265SEC;
            }
            else
            {
                return VDEC_DRV_VIDEO_FORMAT_H265;
            }
        case MTK_VDEC_CODEC_ID_H263:
            return VDEC_DRV_VIDEO_FORMAT_H263;
        case MTK_VDEC_CODEC_ID_MPEG4:
            return VDEC_DRV_VIDEO_FORMAT_MPEG4;

        case MTK_VDEC_CODEC_ID_DIVX:
            return VDEC_DRV_VIDEO_FORMAT_DIVX4;

        case MTK_VDEC_CODEC_ID_DIVX3:
            return VDEC_DRV_VIDEO_FORMAT_DIVX311;

        case MTK_VDEC_CODEC_ID_XVID:
            return VDEC_DRV_VIDEO_FORMAT_XVID;

        case MTK_VDEC_CODEC_ID_S263:
            return VDEC_DRV_VIDEO_FORMAT_S263;

        case MTK_VDEC_CODEC_ID_AVC:
            if (OMX_TRUE == mIsSecureInst)
            {
                return VDEC_DRV_VIDEO_FORMAT_H264SEC;
            }
            else
            {
                return VDEC_DRV_VIDEO_FORMAT_H264;
            }
        case MTK_VDEC_CODEC_ID_RV:
            return VDEC_DRV_VIDEO_FORMAT_REALVIDEO9;

        case MTK_VDEC_CODEC_ID_VC1:
            return VDEC_DRV_VIDEO_FORMAT_VC1;

        case MTK_VDEC_CODEC_ID_VPX:
            return VDEC_DRV_VIDEO_FORMAT_VP8;
        case MTK_VDEC_CODEC_ID_VP9:
            if (OMX_TRUE == mIsSecureInst)
            {
                 return VDEC_DRV_VIDEO_FORMAT_VP9SEC;
            }
            else
            {
                 return VDEC_DRV_VIDEO_FORMAT_VP9;
            }
        case MTK_VDEC_CODEC_ID_MPEG2:
            return VDEC_DRV_VIDEO_FORMAT_MPEG2;
        case MTK_VDEC_CODEC_ID_MJPEG:
            return VDEC_DRV_VIDEO_FORMAT_MJPEG;
        default:
            return VDEC_DRV_VIDEO_FORMAT_UNKNOWN_VIDEO_FORMAT;
    }
}

OMX_U32 MtkOmxVdec::GetBufferCheckSum(char* u4VA, OMX_U32 u4Length)
{
    OMX_U32 i = 0;
    OMX_U32 u4Sum = 0;
    for (i = 0; i < u4Length; ++i)
    {
        u4Sum += u4VA[i];
    }
    return u4Sum;
}
OMX_BOOL MtkOmxVdec::ConvertFrameToYUV420(FrmBufStruct *pFrameBuf, FrmBufStruct *pFrameBufOut, OMX_BOOL bGetResolution)
{
    DpBlitStream blitStream;
    OMX_COLOR_FORMATTYPE mTempSrcColorFormat = mOutputPortFormat.eColorFormat;
    OMX_VIDEO_CODINGTYPE mTempSrcCompressionFormat = mInputPortFormat.eCompressionFormat;
    DP_PROFILE_ENUM srcColourPrimaries = DP_PROFILE_BT601;
    unsigned int srcWStride;
    unsigned int srcHStride;
    unsigned int CSrcSize =0;
    unsigned int CDstSize =0;

    VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoIn;
    VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;

    MTK_OMX_MEMSET(&qinfoIn, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
    MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
    QueryDriverFormat(&qinfoIn, &qinfoOut);

    srcWStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameWidth, qinfoOut.u4StrideAlign);
    srcHStride = VDEC_ROUND_N(mOutputPortDef.format.video.nFrameHeight, qinfoOut.u4SliceHeightAlign);

    //MTK_OMX_LOGD("mTempSrcColorFormat %x, mTempSrcCompressionFormat %x,, srcWStride %d, srcHStride %d",
    //    mTempSrcColorFormat, mTempSrcCompressionFormat, srcWStride, srcHStride);

    unsigned int dstWStride = mOutputPortDef.format.video.nFrameWidth;
    unsigned int dstHStride = (mOutputPortDef.format.video.nFrameHeight);

    if (OMX_TRUE == bGetResolution || mLegacyMode == OMX_TRUE)
    {
        srcWStride = VDEC_ROUND_16(pFrameBuf->ipOutputBuffer->nWidth);
        srcHStride = VDEC_ROUND_32(pFrameBuf->ipOutputBuffer->nHeight);
        dstWStride = srcWStride;
        dstHStride = (pFrameBuf->ipOutputBuffer->nHeight);
    }

    /* output of non-16 alignment resolution in new interace
        UV != (Y/2) size will cause MDP abnormal in previously.
        1. h264_hybrid_dec_init_ex()
        //Assume YV12 format
        //for new interface integration
        pH264Handle->codecOpenSetting.stride.u4YStride  = VDEC_ROUND_16(pH264Handle->rVideoDecYUVBufferParameter.u4Width);
        pH264Handle->codecOpenSetting.stride.u4UVStride = VDEC_ROUND_16(pH264Handle->codecOpenSetting.stride.u4YStride / 2) ;


        2. h264_hybrid_dec_decode()
        Height = VDEC_ROUND_16(pH264Handle->rVideoDecYUVBufferParameter.u4Height);
        YSize = pH264Handle->codecOpenSetting.stride.u4YStride * Height;
        CSize = pH264Handle->codecOpenSetting.stride.u4UVStride * (Height >> 1);

        example:
        144x136
        Y    size is 144x144
        UV  size is (160/2)x(144/2)
       */
    if( (OMX_MTK_COLOR_FormatYV12 == mTempSrcColorFormat) )
    {
        CSrcSize = VDEC_ROUND_16(srcWStride / 2) * (srcHStride / 2);
        MTK_OMX_LOGD("CSrcSize %d", CSrcSize);
    }
    else
    {
        CSrcSize = 0;
    }

    // Source MTKYUV
    DpRect srcRoi;
    srcRoi.x = 0;
    srcRoi.y = 0;
    srcRoi.w = mOutputPortDef.format.video.nFrameWidth;
    srcRoi.h = mOutputPortDef.format.video.nFrameHeight;
    if (OMX_TRUE == bGetResolution || mLegacyMode == OMX_TRUE)
    {
        srcRoi.w = pFrameBuf->ipOutputBuffer->nWidth;
        srcRoi.h = pFrameBuf->ipOutputBuffer->nHeight;
    }

    char *srcPlanar[3];
    char *srcMVAPlanar[3];
    unsigned int srcLength[3];
    VBufInfo bufInfo_src;
    mOutputMVAMgr->getOmxInfoFromVA((void *)pFrameBuf->frame_buffer.rBaseAddr.u4VA, &bufInfo_src);

    srcPlanar[0] = (char *)pFrameBuf->frame_buffer.rBaseAddr.u4VA;
    srcLength[0] = srcWStride * srcHStride;

    switch (pFrameBuf->frame_buffer.rColorPriInfo.eColourPrimaries)
    {
        case COLOR_PRIMARIES_BT601:
            if (pFrameBuf->frame_buffer.rColorPriInfo.u4VideoRange) {
                srcColourPrimaries = DP_PROFILE_FULL_BT601;
            } else {
                srcColourPrimaries = DP_PROFILE_BT601;
            }
            break;
        case COLOR_PRIMARIES_BT709:
            if (pFrameBuf->frame_buffer.rColorPriInfo.u4VideoRange) {
                srcColourPrimaries = DP_PROFILE_FULL_BT709;
            } else {
                srcColourPrimaries = DP_PROFILE_BT709;
            }
            break;
        case COLOR_PRIMARIES_BT2020:
            if (pFrameBuf->frame_buffer.rColorPriInfo.u4VideoRange) {
                srcColourPrimaries = DP_PROFILE_FULL_BT2020;
            } else {
                srcColourPrimaries = DP_PROFILE_BT2020;
            }
            break;
        default:
            srcColourPrimaries = DP_PROFILE_BT601;
            break;
    }

    if ((OMX_COLOR_FormatYUV420Planar == mTempSrcColorFormat) || (OMX_MTK_COLOR_FormatYV12 == mTempSrcColorFormat))
    {
        if (OMX_TRUE == mOutputUseION)
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = (CSrcSize == 0) ? srcWStride * srcHStride / 4 : CSrcSize;
            srcPlanar[2] = srcPlanar[1] + srcLength[1];
            srcLength[2] = (CSrcSize == 0) ? srcWStride * srcHStride / 4 : CSrcSize;

            srcMVAPlanar[0] = (char *)pFrameBuf->frame_buffer.rBaseAddr.u4PA;
            srcMVAPlanar[1] = srcMVAPlanar[0] + srcLength[0];
            srcMVAPlanar[2] = srcMVAPlanar[1] + srcLength[1];
        }
        else
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = (CSrcSize == 0) ? srcWStride * srcHStride / 4 : CSrcSize;

            srcPlanar[2] = srcPlanar[1] + srcLength[1];
            srcLength[2] = (CSrcSize == 0) ? srcWStride * srcHStride / 4 : CSrcSize;
        }

        blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 3);

        if (OMX_COLOR_FormatYUV420Planar == mTempSrcColorFormat)
        {
            blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride, (CSrcSize == 0) ? (srcWStride / 2) : (VDEC_ROUND_16(srcWStride / 2)), eI420, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
        else
        {
            blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride, (CSrcSize == 0) ? (srcWStride / 2) : (VDEC_ROUND_16(srcWStride / 2)), eYV12, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
    }
    else if (OMX_COLOR_FormatYUV420SemiPlanar == mTempSrcColorFormat)  // for NV12
    {
        if (OMX_TRUE == mOutputUseION)
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = srcWStride * (srcHStride / 2);

            srcMVAPlanar[0] = (char *)pFrameBuf->frame_buffer.rBaseAddr.u4PA;
            srcMVAPlanar[1] = srcMVAPlanar[0] + srcLength[0];
        }
        else
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = srcWStride * (srcHStride / 2);
        }

        blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 2);

        blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride, srcWStride, eNV12, srcColourPrimaries, eInterlace_None, &srcRoi);
    }
    else if (OMX_COLOR_FormatVendorMTKYUV_FCM == mTempSrcColorFormat)
    {
        if (OMX_TRUE == mOutputUseION)
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = srcWStride * srcHStride / 2;

            srcMVAPlanar[0] = (char *)pFrameBuf->frame_buffer.rBaseAddr.u4PA;
            srcMVAPlanar[1] = srcMVAPlanar[0] + srcLength[0];
        }
        else
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = srcWStride * srcHStride / 2;
        }

        blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 2);
        blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 32, srcWStride * 16, eNV12_BLK_FCM, srcColourPrimaries, eInterlace_None, &srcRoi);
    }
    else
    {

        if(mbIs10Bit)
        {
            srcLength[0] *= 1.25;
        }

        if (OMX_TRUE == mOutputUseION)
        {
            srcPlanar[1] = srcPlanar[0] + VDEC_ROUND_N(srcLength[0],512);
            srcLength[1] = srcWStride * srcHStride / 2;
            if (mbIs10Bit) {
                srcLength[1] *= 1.25;
            }
            srcMVAPlanar[0] = (char *)pFrameBuf->frame_buffer.rBaseAddr.u4PA;
            srcMVAPlanar[1] = srcMVAPlanar[0] + VDEC_ROUND_N(srcLength[0],512);

        }
        else
        {
            srcPlanar[1] = srcPlanar[0] + VDEC_ROUND_N(srcLength[0],512);
            srcLength[1] = srcWStride * srcHStride / 2;
            if (mbIs10Bit) {
                srcLength[1] *= 1.25;
            }
        }

        blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 2);
        if(mbIs10Bit)
        {
            if (mIsHorizontalScaninLSB)
                blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 40, srcWStride * 20, DP_COLOR_420_BLKP_10_H, srcColourPrimaries, eInterlace_None, &srcRoi);
            else
                blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 40, srcWStride * 20, DP_COLOR_420_BLKP_10_V, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
        else
        {
            blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 32, srcWStride * 16, eNV12_BLK, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
    }

    if (mIsHDRVideo == OMX_TRUE)
    {
        DpPqParam dppq_param;

        if (OMX_TRUE == mOutputAllocateBuffer)
        {
            dppq_param.enable = false;                      // thumbnail do not apply MDP sharpness/Color prevent gallery apply twice
        } else {
            dppq_param.enable = true;
        }

        dppq_param.scenario = MEDIA_VIDEO_CODEC;
        dppq_param.u.video.id = (unsigned int)this;
        dppq_param.u.video.timeStamp = pFrameBuf->iTimestamp;
        dppq_param.u.video.grallocExtraHandle = (buffer_handle_t)pFrameBuf->bGraphicBufHandle;

        if (sizeof(dppq_param.u.video.HDRinfo) != sizeof(mColorDesc)) {
            MTK_OMX_LOGE("[ERROR] HDR color desc info size not sync!!dppq_param.u.video.HDRinfo(%d) != mColorDesc(%d)", sizeof(dppq_param.u.video.HDRinfo), sizeof(mColorDesc));
        } else {
            memcpy(&dppq_param.u.video.HDRinfo, &mColorDesc, sizeof(mColorDesc));
        }

        MTK_OMX_LOGD("[Debug][Convert] mThumbnailMode %d, pFrameBuf->bGraphicBufHandle 0x%x,LINE:%d", mThumbnailMode, pFrameBuf->bGraphicBufHandle, __LINE__);
        blitStream.setPQParameter(dppq_param);
    }

    // Target Android YV12
    DpRect dstRoi;
    dstRoi.x = 0;
    dstRoi.y = 0;
    dstRoi.w = srcRoi.w;
    dstRoi.h = srcRoi.h;

    char *dstPlanar[3];
    char *dstMVAPlanar[3];
    unsigned int dstLength[3];

    /*reference from hardware\gpu_mali\mali_midgard\r5p0-eac\gralloc\src\Gralloc_module.cpp
            In gralloc_mtk_lock_ycbcr(), I420 case,
            case HAL_PIXEL_FORMAT_I420:
            int ysize = ycbcr->ystride * hnd->height;
            ycbcr->chroma_step = 1;
            ycbcr->cstride = GRALLOC_ALIGN(hnd->stride / 2, 16) * ycbcr->chroma_step;
            {
                int csize = ycbcr->cstride * hnd->height / 2;
                ycbcr->cb = (void *)((char *)ycbcr->y + ysize);
                ycbcr->cr = (void *)((char *)ycbcr->y + ysize + csize);
            }
        */
#if 0
    if ((OMX_TRUE == mStoreMetaDataInBuffers) && (OMX_TRUE == mbYUV420FlexibleMode))
    {
        CDstSize = VDEC_ROUND_16(dstWStride / 2) * (dstHStride / 2);
        MTK_OMX_LOGD("CDstSize %d", CDstSize);
    }
    else
    {
        CDstSize = 0;
    }
#endif
    VBufInfo bufInfo_dst;
    if(mOutputMVAMgr->getOmxInfoFromVA((void *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA, &bufInfo_dst) < 0)
    {
        MTK_OMX_LOGE("[ERROR][Convert] Can't find info in mOutputMVAMgr,LINE:%d", __LINE__);
    }

    if (OMX_TRUE == needColorConvertWithMetaMode())
    {
        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void*)pFrameBuf->bGraphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE("[ERROR][Convert] Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
            return OMX_FALSE;
        }
        else
        {
            dstPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA;

            if (OMX_TRUE == mOutputUseION)
            {
                dstMVAPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4PA;
            }
        }
    }
    else
    {
        dstPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA;
        if (OMX_TRUE == mOutputUseION)
        {
            dstMVAPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4PA;
        }
    }

        dstLength[0] = dstWStride * dstHStride;

#if 0
    if (OMX_TRUE == mOutputUseION)
    {
        dstPlanar[1] = dstPlanar[0] + dstLength[0];
        dstLength[1] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;
        dstPlanar[2] = dstPlanar[1] + dstLength[1];
        dstLength[2] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;

        dstMVAPlanar[1] = dstMVAPlanar[0] + dstLength[0];
        dstMVAPlanar[2] = dstMVAPlanar[1] + dstLength[1];
        blitStream.setDstBuffer((void **)dstPlanar, (void **)dstMVAPlanar, (unsigned int *)dstLength, 3);
    }
    else
    {
        dstPlanar[1] = dstPlanar[0] + dstLength[0];
        dstLength[1] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;
        dstPlanar[2] = dstPlanar[1] + dstLength[1];
        dstLength[2] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;

        blitStream.setDstBuffer((void **)dstPlanar, (unsigned int *)dstLength, 3);
    }
    blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, (CDstSize == 0) ? (dstWStride / 2) : (VDEC_ROUND_16(dstWStride / 2)), eI420, DP_PROFILE_BT601, eInterlace_None, &dstRoi);
#else
    if (OMX_TRUE == mOutputUseION)
    {
        dstPlanar[1] = dstPlanar[0] + dstLength[0];
        dstLength[1] = dstWStride * dstHStride / 4;
        dstPlanar[2] = dstPlanar[1] + dstLength[1];
        dstLength[2] = dstWStride * dstHStride / 4;

        dstMVAPlanar[1] = dstMVAPlanar[0] + dstLength[0];
        dstMVAPlanar[2] = dstMVAPlanar[1] + dstLength[1];
    }
    else
    {
        dstPlanar[1] = dstPlanar[0] + dstLength[0];
        dstLength[1] = dstWStride * dstHStride / 4;
        dstPlanar[2] = dstPlanar[1] + dstLength[1];
        dstLength[2] = dstWStride * dstHStride / 4;
    }
    blitStream.setDstBuffer(bufInfo_dst.iIonFd, (unsigned int *)dstLength, 3);

    if (mConvertYV12 == 1)
        blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, dstWStride / 2, eYV12, srcColourPrimaries, eInterlace_None, &dstRoi);
    else
        blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, dstWStride / 2, eI420, srcColourPrimaries, eInterlace_None, &dstRoi);

#endif

#if PROFILING
    static int64_t _in_time_1 = 0;
    static int64_t _in_time_2 = 0;
    static int64_t _out_time = 0;
    _in_time_1 = getTickCountMs();
#endif

    // Blit
    MTK_OMX_LOGD("Internal blitStream+ Src Va=0x%x, Size=%d, Dst Va=0x%x, Size=%d, Px %x",
                 srcPlanar[0], srcLength[0] * 3 / 2,
                 dstPlanar[0], dstLength[0] * 3 / 2, (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4PA);

#ifdef MTK_QOS_SUPPORT
    struct timeval target_end;
    gettimeofday(&target_end, NULL);
    target_end.tv_usec += 1600;
    if (target_end.tv_usec > 1000000)
    {
        target_end.tv_sec++;
        target_end.tv_usec -= 1000000;
    }

    int iRet = blitStream.invalidate(&target_end);
#else
    int iRet = blitStream.invalidate();
#endif

    MTK_OMX_LOGD("@Invalidate Cache After MDP va %p 0x%x", pFrameBufOut->frame_buffer.rBaseAddr.u4VA, pFrameBufOut->ionBufHandle);
    mOutputMVAMgr->syncBufferCacheFrm((void *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA, (unsigned int)ION_CACHE_INVALID_BY_RANGE);

#if PROFILING
    _out_time = getTickCountMs();
    _in_time_2 = _out_time - _in_time_1;
#endif
    //MTK_OMX_LOGD("Internal blitStream- iRet=%d, %lld ms", iRet, _in_time_2);

    if (0 != iRet)
    {
        MTK_OMX_LOGE("MDP iRet=%d, %lld ms", iRet, _in_time_2);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

void MtkOmxVdec::FdDebugDump()
{
    int x, inlen = 0, outlen = 0;
    char *IonInFdBuf = (char *)malloc(VDEC_ROUND_16(VIDEO_ION_MAX_BUFFER * 10 + 16));
    char *IonOutFdBuf = (char *)malloc(VDEC_ROUND_16(VIDEO_ION_MAX_BUFFER * 10 + 16));
    for (x = 0; x < VIDEO_ION_MAX_BUFFER; ++x)
    {
        if (IonInFdBuf == NULL) { break; }
        if (IonOutFdBuf == NULL) { break; }
        if (x == 0)
        {
            inlen += sprintf(IonInFdBuf + inlen, "IonInFd %d:", (int)mIonInputBufferCount);
            outlen += sprintf(IonOutFdBuf + outlen, "IonOutFd %d:", (int)mIonOutputBufferCount);
        }
    }
    MTK_OMX_LOGE("%s", IonInFdBuf);
    MTK_OMX_LOGE("%s", IonOutFdBuf);
    if (IonInFdBuf != NULL) { free(IonInFdBuf); }
    if (IonOutFdBuf != NULL) { free(IonOutFdBuf); }
}

OMX_U32 MtkOmxVdec::AllocateIonBuffer(int IonFd, OMX_U32 Size, VdecIonBufInfo *IonBufInfo)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (-1 == IonFd)
    {
        mIonDevFd = eVideoOpenIonDevFd();
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE("[ERROR] cannot open ION device. LINE:%d", __LINE__);
            err = OMX_ErrorUndefined;
            return err;
        }
    }
    int ret = ion_alloc_mm(mIonDevFd, Size, MEM_ALIGN_512, ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC, &IonBufInfo->pIonBufhandle);
    if (0 != ret)
    {
        MTK_OMX_LOGE("[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
        err = OMX_ErrorUndefined;
        return err;
    }
    int share_fd;
    if (ion_share(mIonDevFd, IonBufInfo->pIonBufhandle, &share_fd))
    {
        MTK_OMX_LOGE("[ERROR] ion_share failed, LINE:%d", __LINE__);
        err = OMX_ErrorUndefined;
        return err;
    }
    // map virtual address
    OMX_U8 *buffer = (OMX_U8 *) ion_mmap(mIonDevFd, NULL, Size, PROT_READ | PROT_WRITE, MAP_SHARED, share_fd, 0);
    if ((buffer == NULL) || (buffer == (void *) - 1))
    {
        MTK_OMX_LOGE("[ERROR] ion_mmap failed, LINE:%d", __LINE__);
        err = OMX_ErrorUndefined;
        return err;
    }

    // configure buffer
    ConfigIonBuffer(mIonDevFd, IonBufInfo->pIonBufhandle);
    IonBufInfo->u4OriVA = (VAL_UINT32_T)buffer;
    IonBufInfo->fd = share_fd;
    IonBufInfo->u4VA = (VAL_UINT32_T)buffer;
    IonBufInfo->u4PA = GetIonPhysicalAddress(mIonDevFd, IonBufInfo->pIonBufhandle);
    IonBufInfo->u4BuffSize = Size;

    MTK_OMX_LOGUD("ION allocate Size (%d), u4VA(0x%08X), share_fd(%d), VA(0x%08X), PA(0x%08X)",
                 Size, buffer, share_fd, IonBufInfo->u4VA, IonBufInfo->u4PA);

    return err;
}

OMX_BOOL MtkOmxVdec::DescribeFlexibleColorFormat(DescribeColorFormatParams *params)
{
    MediaImage &imageInfo = params->sMediaImage;
    memset(&imageInfo, 0, sizeof(imageInfo));

    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_UNKNOWN;
    imageInfo.mNumPlanes = 0;

    const OMX_COLOR_FORMATTYPE fmt = params->eColorFormat;
    imageInfo.mWidth = params->nFrameWidth;
    imageInfo.mHeight = params->nFrameHeight;

    MTK_OMX_LOGUD("DescribeFlexibleColorFormat %d fmt %x, W/H(%d, %d), WS/HS(%d, %d), (%d, %d)", sizeof(size_t), fmt, imageInfo.mWidth, imageInfo.mHeight,
        params->nStride, params->nSliceHeight, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);

    // only supporting YUV420
    if (fmt != OMX_COLOR_FormatYUV420Planar &&
        fmt != OMX_COLOR_FormatYUV420PackedPlanar &&
        fmt != OMX_COLOR_FormatYUV420SemiPlanar &&
        fmt != HAL_PIXEL_FORMAT_I420 &&
        fmt != OMX_COLOR_FormatYUV420PackedSemiPlanar)
    {
        ALOGW("do not know color format 0x%x = %d", fmt, fmt);
        return OMX_FALSE;
    }

    // set-up YUV format
    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_YUV;
    imageInfo.mNumPlanes = 3;
    imageInfo.mBitDepth = 8;
    imageInfo.mPlane[imageInfo.Y].mOffset = 0;
    imageInfo.mPlane[imageInfo.Y].mColInc = 1;
    imageInfo.mPlane[imageInfo.Y].mRowInc = params->nFrameWidth;
    imageInfo.mPlane[imageInfo.Y].mHorizSubsampling = 1;
    imageInfo.mPlane[imageInfo.Y].mVertSubsampling = 1;

    switch (fmt)
    {
        case OMX_COLOR_FormatYUV420Planar: // used for YV12
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case HAL_PIXEL_FORMAT_I420:
            imageInfo.mPlane[imageInfo.U].mOffset = params->nFrameWidth * params->nFrameHeight;
            imageInfo.mPlane[imageInfo.U].mColInc = 1;
            imageInfo.mPlane[imageInfo.U].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[imageInfo.U].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.U].mVertSubsampling = 2;

            imageInfo.mPlane[imageInfo.V].mOffset = imageInfo.mPlane[imageInfo.U].mOffset
                                            + (params->nFrameWidth * params->nFrameHeight / 4);
            imageInfo.mPlane[imageInfo.V].mColInc = 1;
            imageInfo.mPlane[imageInfo.V].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[imageInfo.V].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.V].mVertSubsampling = 2;
            break;

        case OMX_COLOR_FormatYUV420SemiPlanar:
            // FIXME: NV21 for sw-encoder, NV12 for decoder and hw-encoder
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            // NV12
            imageInfo.mPlane[imageInfo.U].mOffset = params->nStride * params->nSliceHeight;
            imageInfo.mPlane[imageInfo.U].mColInc = 2;
            imageInfo.mPlane[imageInfo.U].mRowInc = params->nStride;
            imageInfo.mPlane[imageInfo.U].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.U].mVertSubsampling = 2;

            imageInfo.mPlane[imageInfo.V].mOffset = imageInfo.mPlane[imageInfo.U].mOffset + 1;
            imageInfo.mPlane[imageInfo.V].mColInc = 2;
            imageInfo.mPlane[imageInfo.V].mRowInc = params->nStride;
            imageInfo.mPlane[imageInfo.V].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.V].mVertSubsampling = 2;
            break;

        default:
            MTK_OMX_LOGE("default %x", fmt);
    }
    return OMX_TRUE;
}

int MtkOmxVdec::PrepareAvaliableColorConvertBuffer(int output_idx, OMX_BOOL direct_dequeue)
{

    // dequeue an output buffer for color convert
    int output_color_convert_idx = -1;
    if (OMX_TRUE == mOutputAllocateBuffer || OMX_TRUE == needColorConvertWithNativeWindow())
    {
        int mCCQSize = mBufColorConvertDstQ.size();

        if( (mMaxColorConvertOutputBufferCnt > mCCQSize) && (OMX_TRUE == direct_dequeue) )
        {
            #if 1
            if (get_sem_value(&mOutputBufferSem) > 0)
            {
                MTK_OMX_LOGD("#ConvertTh# Wait for output buffer (%d)", get_sem_value(&mOutputBufferSem));
                int retVal = TRY_WAIT(mOutputBufferSem);
                if (0 == retVal)
                {
                    MTK_OMX_LOGD("@@ getSemOutputBufferSem -- (OK)");
                    //WAIT_T(mOutputBufferSem);
                    output_color_convert_idx = DequeueOutputBuffer();

                    if(-1 == output_color_convert_idx)
                    {
                        //get index = -1 means can not convert frame,
                        //signal semaphore for decoder thread during the state change
                       // MTK_OMX_LOGD("SIGNAL mOutputBufferSem");
                        SIGNAL(mOutputBufferSem);
                    }
                }
                else if (-1 == retVal)
                {
                    MTK_OMX_LOGD("@@ getSemOutputBufferSem errno %d-- (EAGAIN %d)", errno, EAGAIN);
                }

                MTK_OMX_LOGD("PrepareAvaliableColorConvertBuffer output_color_convert_idx %d", output_color_convert_idx);
            }
            if( output_color_convert_idx >= 0 )
            {
                QueueOutputColorConvertDstBuffer(output_color_convert_idx);
                MTK_OMX_LOGD("%d, output_idx = %d, size %d",
                  mNumFreeAvailOutput, output_color_convert_idx, CheckColorConvertBufferSize());
            }
            #endif
        }
        else if (((mCCQSize < mBufColorConvertSrcQ.size()) || (mMaxColorConvertOutputBufferCnt > mCCQSize))&&
            (OMX_FALSE == direct_dequeue))
        {
            output_color_convert_idx = output_idx;
            OMX_BOOL mIsFree = IsFreeBuffer(mOutputBufferHdrs[output_color_convert_idx]);
            if (OMX_FALSE == mIsFree)
            {
            }
            else
            {
                int i = FindQueueOutputBuffer(mOutputBufferHdrs[output_color_convert_idx]);
                OMX_BUFFERHEADERTYPE *pBuffHdr = mOutputBufferHdrs[output_color_convert_idx];
                if(i >= 0)
                {
                    mFillThisBufQ.removeAt(i);
                }
                MTK_OMX_LOGUD("@@ QueueOutputColorConvertDstBuffer (0x%08X) (0x%08X) index %d, mNumFreeAvailOutput %d, mIsFree %d, CCBsize %d",
                    pBuffHdr, pBuffHdr->pBuffer, output_color_convert_idx, mNumFreeAvailOutput, mIsFree, CheckColorConvertBufferSize());
                QueueOutputColorConvertDstBuffer(output_color_convert_idx);
                mNumFreeAvailOutput--;
            }
        }
        else
        {
            //MTK_OMX_LOGD("CheckColorConvertBufferSize %d", CheckColorConvertBufferSize());
        }
    }
    return output_color_convert_idx;//OMX_TRUE;
}

void MtkOmxVdec::QueueOutputColorConvertSrcBuffer(int index)
{
    LOCK_T(mFillThisBufQLock);
    VAL_UINT32_T u4y;
    OMX_BUFFERHEADERTYPE *ipOutputBuffer = mOutputBufferHdrs[index];

    //MTK_OMX_LOGD ("@@ QueueOutputColorConvertSrcBuffer index %d, %x, %x",
    //    index,
    //    ipOutputBuffer->pBuffer,
    //    ipOutputBuffer->pOutputPortPrivate);
    if(OMX_TRUE == needColorConvertWithMetaMode())
    {
        OMX_U32 graphicBufHandle = 0;
        int mIndex = 0;
        if (OMX_FALSE == GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle))
        {
            MTK_OMX_LOGE("SetupMetaIonHandle failed, LINE:%d", __LINE__);
        }

        if (0 == graphicBufHandle)
        {
            MTK_OMX_LOGE("GetMetaHandleFromOmxHeader failed, LINE:%d", __LINE__);
        }

        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGD("QueueOutputColorConvertSrcBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }
        else
        {
            mFrameBuf[index].bGraphicBufHandle = graphicBufHandle;
            mFrameBuf[index].ionBufHandle = BufInfo.ionBufHndl;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = BufInfo.u4VA;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = BufInfo.u4PA;
        }
    }
    else if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo info;
        int ret = 0;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
            MTK_OMX_LOGE("[Warning] it's not Meta mode, Should not be here. line %d", __LINE__);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)ipOutputBuffer->pBuffer, &info);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD("QueueOutputColorConvertSrcBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }

        mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = info.u4VA;
        mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = info.u4PA;
        if (OMX_TRUE == mIsSecureInst)
        {
            mFrameBuf[index].frame_buffer.rSecMemHandle = info.secure_handle;
            mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle = 0;
            MTK_OMX_LOGE("@@ aFrame->rSecMemHandle(0x%08X), aFrame->rFrameBufVaShareHandle(0x%08X)", mFrameBuf[index].frame_buffer.rSecMemHandle, mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle);
        }
    }

    mBufColorConvertSrcQ.push(index);
    UNLOCK(mFillThisBufQLock);
    SIGNAL(mConvertSem);
    return;

}
void MtkOmxVdec::QueueOutputColorConvertDstBuffer(int index)
{
    LOCK_T(mFillThisConvertBufQLock);
    VAL_UINT32_T u4y;
    OMX_BUFFERHEADERTYPE *ipOutputBuffer = mOutputBufferHdrs[index];

    //MTK_OMX_LOGD ("@@ QueueOutputColorConvertDstBuffer index %d, %x, %x",
    //    index,
    //    ipOutputBuffer->pBuffer,
    //    ipOutputBuffer->pOutputPortPrivate);

    if(OMX_TRUE == needColorConvertWithMetaMode())
    {
        OMX_U32 graphicBufHandle = 0;
        int mIndex = 0;
        if (OMX_FALSE == GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle))
        {
            MTK_OMX_LOGE("GetMetaHandleFromOmxHeader failed, LINE:%d", __LINE__);
        }

        if (0 == graphicBufHandle)
        {
            MTK_OMX_LOGE("GetMetaHandleFromOmxHeader failed, LINE:%d", __LINE__);
        }

        MTK_OMX_LOGD ("DstBuffer graphicBufHandle %p", graphicBufHandle);

        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGD("QueueOutputColorConvertDstBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }
        else
        {
            mFrameBuf[index].bGraphicBufHandle = graphicBufHandle;
            mFrameBuf[index].ionBufHandle = BufInfo.ionBufHndl;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = BufInfo.u4VA;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = BufInfo.u4PA;
        }
    }
    else if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo info;
        int ret = 0;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
            MTK_OMX_LOGE("[Warning] it's not Meta mode, Should not be here. line %d", __LINE__);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)ipOutputBuffer->pBuffer, &info);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD("QueueOutputColorConvertSrcBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }

        mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = info.u4VA;
        mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = info.u4PA;

        if (OMX_TRUE == mIsSecureInst)
        {
            mFrameBuf[index].frame_buffer.rSecMemHandle = info.secure_handle;
            mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle = 0;
            MTK_OMX_LOGE("@@ aFrame->rSecMemHandle(0x%08X), aFrame->rFrameBufVaShareHandle(0x%08X)", mFrameBuf[index].frame_buffer.rSecMemHandle, mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle);
        }

        //MTK_OMX_LOGD("[ION] id %x, frame->rBaseAddr.u4VA = 0x%x, frame->rBaseAddr.u4PA = 0x%x", u4y, mFrameBuf[u4y].frame_buffer.rBaseAddr.u4VA, mFrameBuf[u4y].frame_buffer.rBaseAddr.u4PA);
    }

    //MTK_OMX_LOGD("QueueOutputColorConvertDstBuffer %d", index);
    mBufColorConvertDstQ.push(index);
    UNLOCK(mFillThisConvertBufQLock);
    SIGNAL(mConvertSem);
    return;
}

int MtkOmxVdec::DeQueueOutputColorConvertSrcBuffer()
{
    int output_idx = -1, i;
    LOCK_T(mFillThisBufQLock);
    //MTK_OMX_LOGD("DeQueueOutputColorConvertSrcBuffer() size (%d, %d, %d)",
    //mBufColorConvertSrcQ.size(), mBufColorConvertDstQ.size(), mFillThisBufQ.size());
#if ANDROID
    if (0 == mBufColorConvertSrcQ.size())
    {
        MTK_OMX_LOGE("DeQueueOutputColorConvertSrcBuffer(), mFillThisBufQ.size() is 0, return original idx %d", output_idx);
        output_idx = -1;
#ifdef HAVE_AEE_FEATURE
        aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_DEFAULT, "\nOmxVdec should no be here will push dummy output buffer!");
#endif //HAVE_AEE_FEATURE
        UNLOCK(mFillThisBufQLock);
        return output_idx;
    }

    output_idx = mBufColorConvertSrcQ[0];
    mBufColorConvertSrcQ.removeAt(0);
#endif
    UNLOCK(mFillThisBufQLock);
    return output_idx;
}


int MtkOmxVdec::DeQueueOutputColorConvertDstBuffer()
{
    int output_idx = -1, i;
    LOCK_T(mFillThisConvertBufQLock);
    //MTK_OMX_LOGD("DeQueueOutputColorConvertDstBuffer() size (max-%d, %d, %d)",
    //mMaxColorConvertOutputBufferCnt, mBufColorConvertDstQ.size(), mFillThisBufQ.size());
#if ANDROID
    for (i = 0; i < mBufColorConvertDstQ.size(); i++)
    {
        output_idx = mBufColorConvertDstQ[i];
        if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[output_idx]))
        {
            MTK_OMX_LOGE("DeQueueOutputColorConvertDstBuffer(), mOutputBufferHdrs[%d] is not free (0x%08X)", output_idx, mOutputBufferHdrs[output_idx]);
        }
        else
        {
            //MTK_OMX_LOGE("DeQueueOutputColorConvertDstBuffer(), mOutputBufferHdrs[%d] is free", output_idx, mOutputBufferHdrs[output_idx]);
            break;
        }
    }

    if (0 == mBufColorConvertDstQ.size())
    {
        MTK_OMX_LOGE("DeQueueOutputColorConvertDstBuffer(), mFillThisBufQ.size() is 0, return original idx %d", output_idx);
        output_idx = -1;
        UNLOCK(mFillThisConvertBufQLock);
#ifdef HAVE_AEE_FEATURE
        aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_DEFAULT, "\nOmxVdec should no be here will push dummy output buffer!");
#endif //HAVE_AEE_FEATURE
        return output_idx;
    }

    if (i == mBufColorConvertDstQ.size())
    {
        output_idx = mBufColorConvertDstQ[0];
        mBufColorConvertDstQ.removeAt(0);
    }
    else
    {
        output_idx = mBufColorConvertDstQ[i];
        mBufColorConvertDstQ.removeAt(i);
    }
#endif
    UNLOCK(mFillThisConvertBufQLock);
    return output_idx;
}

OMX_U32 MtkOmxVdec::CheckColorConvertBufferSize()
{
    return mBufColorConvertDstQ.size();
}

OMX_ERRORTYPE MtkOmxVdec::HandleColorConvertForFillBufferDone(OMX_U32 nPortIndex, OMX_BOOL fromDecodet)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD("MtkOmxVdec::HandleColorConvertForFillBufferDone nPortIndex(0x%X)", (unsigned int)nPortIndex);

    int size_of_SrcQ = mBufColorConvertSrcQ.size();
    int size_of_DstQ = mBufColorConvertDstQ.size();
    int in_index = 0;
    if ((0 != size_of_SrcQ) && (0 != size_of_DstQ))
    {
        //ALOGD("size_of_SrcQ %d, DstQ %d", size_of_SrcQ, size_of_DstQ);
        //const int minSize = ((size_of_FBDQ >= size_of_CCQ) ? size_of_CCQ : size_of_FBDQ);
        //for (int i = 0 ; i < minSize; i++)
        {
            //MTK_OMX_LOGD("DeQueue i: %d", i);
            in_index = DeQueueOutputColorConvertSrcBuffer();
            if( 0 > in_index )
            {
                MTK_OMX_LOGE("[%s]: DeQueueOutputColorConvertSrcBuffer fail", __func__ );
                return OMX_ErrorBadParameter;
            }
            HandleColorConvertForFillBufferDone_1(in_index, OMX_FALSE);
            //MTK_OMX_LOGD("FBDQ.size %d, CCQ.size %d", mBufColorConvertSrcQ.size(), mBufColorConvertDstQ.size());
        }
    }
    MTK_OMX_LOGD("HandleColorConvertForFillBufferDone exit");
    return err;
}

OMX_ERRORTYPE MtkOmxVdec::HandleColorConvertForFillBufferDone_1(OMX_U32 input_index, OMX_BOOL fromDecodet)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (0 > input_index)
    {
        MTK_OMX_LOGE("@@ input_index = %d", input_index);
        return OMX_ErrorUndefined;
    }

    MTK_OMX_LOGD("@@ GetFrmStructure frm=0x%x, i=%d, color= %x, type= %x", &mFrameBuf[input_index].frame_buffer, input_index,
                 mOutputPortFormat.eColorFormat, mInputPortFormat.eCompressionFormat);
    MTK_OMX_LOGD("@@ nFlags = %x",
                 mFrameBuf[input_index].ipOutputBuffer->nFlags);

    // dequeue an output buffer
    int output_idx = DeQueueOutputColorConvertDstBuffer();
    int mNomoreColorConvertQ = 0;
    //MTK_OMX_LOGD("output_idx = %d", output_idx);
    if (-1 == output_idx)
    {
        ALOGE("OOPS! fix me");
        output_idx = input_index;
        mNomoreColorConvertQ = 1;
    }
    // check if this buffer is really "FREED"
    if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[output_idx]))
    {
        MTK_OMX_LOGD("Output [0x%08X] is not free, mNumFreeAvailOutput %d", mOutputBufferHdrs[output_idx],
              mNumFreeAvailOutput);
    }
    else
    {
        MTK_OMX_LOGD("now NumFreeAvailOutput = %d (%d %d), s:%x, d:%x, %d, %d, %lld",
              mNumFreeAvailOutput, input_index, output_idx, mFrameBuf[input_index].ipOutputBuffer, mFrameBuf[output_idx].ipOutputBuffer,
              mFrameBuf[input_index].ipOutputBuffer->nFlags,
              mFrameBuf[input_index].ipOutputBuffer->nFilledLen,
              mFrameBuf[input_index].ipOutputBuffer->nTimeStamp);
        //clone source buffer status
        mFrameBuf[output_idx].ipOutputBuffer->nFlags = mFrameBuf[input_index].ipOutputBuffer->nFlags;
        mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = mFrameBuf[input_index].ipOutputBuffer->nFilledLen;
        mFrameBuf[output_idx].ipOutputBuffer->nTimeStamp = mFrameBuf[input_index].ipOutputBuffer->nTimeStamp;
    }

    if ((meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_HEVC) ||
        (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_VPX))
    {
        MTK_OMX_LOGD("@Flush Cache Before MDP");
        mOutputMVAMgr->syncBufferCacheFrm((void *)mFrameBuf[input_index].frame_buffer.rBaseAddr.u4VA, (unsigned int)ION_CACHE_FLUSH_BY_RANGE);
        sched_yield();
        usleep(1000);       //For CTS checksum fail to make sure flush cache to dram
    }

    if (OMX_TRUE == mOutputChecksum)
    {
        MTK_OMX_LOGD("Before convert getid: %d, frame_num: %d, checksum1: 0x%x, length: %d, mLegacyMode: %d\n",
            gettid(), input_index,
            GetBufferCheckSum((char*)(mFrameBuf[input_index].frame_buffer.rBaseAddr.u4VA),mFrameBuf[input_index].ipOutputBuffer->nFilledLen),
            mFrameBuf[input_index].ipOutputBuffer->nFilledLen, mLegacyMode);
    }
    // MtkOmxVdec::DescribeFlexibleColorFormat() assumes all frame would be converted
    // Any modification skip this convertion here please revise the function above as well
    OMX_BOOL converted = ConvertFrameToYUV420((FrmBufStruct *)&mFrameBuf[input_index], (FrmBufStruct *)&mFrameBuf[output_idx], (mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE);

    // set gralloc_extra info from input buffer to output buffer
    //if (OMX_TRUE == mStoreMetaDataInBuffers)
    if (mFrameBuf[input_index].bGraphicBufHandle != 0)
    {
        GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;
        gralloc_extra_ion_sf_info_t sf_info;
        err = gralloc_extra_query((buffer_handle_t)mFrameBuf[input_index].bGraphicBufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
        if (GRALLOC_EXTRA_OK != err)
        {
            MTK_OMX_LOGE("HandleColorConvertForFillBufferDone_1(), after ConvertFrame gralloc_extra_query sf_info error:0x%x", err);
        }
        else
        {
            err = gralloc_extra_perform((buffer_handle_t)mFrameBuf[output_idx].bGraphicBufHandle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
            if (GRALLOC_EXTRA_OK != err)
            {
                MTK_OMX_LOGE("HandleColorConvertForFillBufferDone_1(), after ConvertFrame gralloc_extra_query sf_info error:0x%x", err);
            }
        }
    }

    if (OMX_FALSE == converted)
    {
        MTK_OMX_LOGE("Internal color conversion not complete");
    }

    if (mFrameBuf[input_index].ipOutputBuffer->nFilledLen != 0)
    {
        //update buffer filled length after converted if it is non zero
        if( OMX_TRUE == needColorConvertWithNativeWindow() )
        {
            OMX_U32 bufferType = *((OMX_U32 *)mFrameBuf[output_idx].ipOutputBuffer->pBuffer);
            //MTK_OMX_LOGD("bufferType %d, %d, %d", bufferType, sizeof(VideoGrallocMetadata),
            //    sizeof(VideoNativeMetadata));
            // check buffer type
            if(kMetadataBufferTypeGrallocSource == bufferType)
            {
                mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = sizeof(VideoGrallocMetadata);//8
            }
            else if(kMetadataBufferTypeANWBuffer == bufferType)
            {
                mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = sizeof(VideoNativeMetadata);//12 in 32 bit
            }
        }
        else
        {
            mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = (mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight) * 3 >> 1;
        }
    }

    if (OMX_TRUE == mOutputChecksum)
    {
        OMX_U32 u4convertLen = mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight * 3 >> 1;
        if (0 == mFrameBuf[output_idx].ipOutputBuffer->nFilledLen)
        {
            u4convertLen = 0;
        }
        MTK_OMX_LOGD("After convert getid: %d, frame_num: %d, checksum: 0x%x, length: %d\n",
           gettid(), output_idx,
           GetBufferCheckSum((char*)(mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA),u4convertLen), u4convertLen);
    }

    if (mDumpOutputFrame == OMX_TRUE)
    {
        // dump converted frames
        char filename[256];
        if (mDumpCts == OMX_FALSE) {
            sprintf(filename, "/sdcard/VdecOutFrm_w%d_h%d_t%d.yuv",
                    mOutputPortDef.format.video.nStride,
                    mOutputPortDef.format.video.nSliceHeight,
                    gettid());
        } else { // dump to specific folder
            sprintf(filename, "/sdcard/vdump/VdecOutFrm_w%d_h%d_t%05d.yuv",
                    mOutputPortDef.format.video.nStride,
                    mOutputPortDef.format.video.nSliceHeight,
                    gettid());
        }
        //(char *)mFrameBuf[i].frame_buffer.rBaseAddr.u4VA
        //mFrameBuf[output_idx].ipOutputBuffer->pBuffer
        writeBufferToFile(filename, (char *)mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA,
            (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight) * 3 >> 1);
        //dump converted frames in data/vdec
        FILE *fp;
        fp = fopen(filename, "ab");
        if (NULL == fp)
        {
            sprintf(filename, "/data/vdec/VdecOutConvertFrm_w%d_h%d_t%d.yuv",
                    mOutputPortDef.format.video.nStride,
                    mOutputPortDef.format.video.nSliceHeight,
                    gettid());
            writeBufferToFile(filename, (char *)mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA,
                (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight) * 3 >> 1);
        }
        else
        {
            fclose(fp);
        }
        //
    }

    LOCK_T(mFillThisBufQLock);
    if (0 == mNomoreColorConvertQ)
    {

        OMX_BUFFERHEADERTYPE *pBuffHdr = mOutputBufferHdrs[input_index];
        pBuffHdr->nFlags = 0; // clear converted EOS
        MTK_OMX_LOGUD("@@ QueueCC Src Buffer to FTBQ for decoder (0x%08X) (0x%08X) index %d, mNumFreeAvailOutput %d",
            pBuffHdr, pBuffHdr->pBuffer, input_index, mNumFreeAvailOutput);
        QueueOutputBuffer(input_index);
        mNumFreeAvailOutput++;
    }

//#ifdef MTK_CROSSMOUNT_SUPPORT
    if( OMX_TRUE == mCrossMountSupportOn )
    {
    //VBufInfo info; //mBufInfo
    int ret = 0;
    buffer_handle_t _handle;
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        OMX_U32 graphicBufHandle = 0;
        GetMetaHandleFromOmxHeader(mFrameBuf[output_idx].ipOutputBuffer, &graphicBufHandle);
        ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &mBufInfo);
    }
    else
    {
        ret = mOutputMVAMgr->getOmxInfoFromVA((void *)mFrameBuf[output_idx].ipOutputBuffer->pBuffer, &mBufInfo);
    }

    if (ret < 0)
    {
        MTK_OMX_LOGD("HandleGrallocExtra cannot find buffer info, LINE: %d", __LINE__);
    }
    else
    {
        MTK_OMX_LOGD("mBufInfo u4VA %x, u4PA %x, iIonFd %d", mBufInfo.u4VA,
            mBufInfo.u4PA, mBufInfo.iIonFd);
        mFrameBuf[output_idx].ipOutputBuffer->pPlatformPrivate = (OMX_U8 *)&mBufInfo;
    }
    mFrameBuf[output_idx].ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_VDEC_OUTPRIVATE;
    }
//#endif //MTK_CROSSMOUNT_SUPPORT

    mNumPendingOutput--;

    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        DecRef(BUFFER_OWNER_COMPONENT, mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA, (OMX_TRUE == mIsSecureInst));
    }

    if (mFrameBuf[input_index].bUsed == OMX_TRUE)
    {
        mFrameBuf[input_index].bFillThis = OMX_TRUE;
    }
    else
    {
        MTK_OMX_LOGD("0x%08x SIGNAL mDecodeSem from HandleColorConvertForFillBufferDone_1()", this);
        SIGNAL(mOutputBufferSem);
    }
    UNLOCK(mFillThisBufQLock);

#if (ANDROID_VER >= ANDROID_M)
    WaitFence(mFrameBuf[output_idx].ipOutputBuffer, OMX_FALSE);
#endif

    //MTK_OMX_LOGD ("FBD mNumPendingOutput(%d), line: %d", mNumPendingOutput, __LINE__);

    MTK_OMX_LOGUD("FBD (ColorConvert) (0x%08X) (0x%08X) bUsed %d, mNumPendingOutput(%d), input_index = %d, output_index = %d (%d, %d) ",
                                mFrameBuf[output_idx].ipOutputBuffer, mFrameBuf[output_idx].ipOutputBuffer->pBuffer, mFrameBuf[input_index].bUsed, mNumPendingOutput, input_index, output_idx, mBufColorConvertDstQ.size(), mFillThisBufQ.size());
    mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                             mAppData,
                             mFrameBuf[output_idx].ipOutputBuffer);

    if (mFrameBuf[output_idx].ipOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventBufferFlag,
                               MTK_OMX_OUTPUT_PORT,
                               mFrameBuf[output_idx].ipOutputBuffer->nFlags,
                               NULL);
    }

    return err;
}

void MtkOmxVdec::DISetGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    VAL_UINT32_T u4I;

    //MTK_OMX_LOGD("[DI] DISetGrallocExtra +");

    if (NULL == pBuffHdr)
    {
        MTK_OMX_LOGE("[DI][ERROR] %s, pBuffHdr is NULL", __func__);
        return;
    }

    if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo  bufInfo;
        int ret;
        if(OMX_TRUE == mStoreMetaDataInBuffers)//should not be here
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &bufInfo);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void*)pBuffHdr->pBuffer, &bufInfo);
        }

        if(ret < 0)
        {
            MTK_OMX_LOGE("[DI][ERROR] DISetGrallocExtra() cannot find Buffer Handle from BufHdr ");
        }
        else
        {
            buffer_handle_t _handle = (buffer_handle_t)bufInfo.pNativeHandle;
            if( NULL != _handle)
            {
            if (mOutputPortFormat.eColorFormat == OMX_MTK_COLOR_FormatYV12)
            {
                __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_YV12 mIsInterlacing %d", mIsInterlacing);
            }
            else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV)
            {
                if(mbIs10Bit)
                {
                    if(mIsHorizontalScaninLSB)
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H);
                        MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H mIsInterlacing %d", mIsInterlacing);
                    }
                    else
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V);
                        MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V mIsInterlacing %d", mIsInterlacing);
                    }
                }
                else
                {
                    if (mConvertYV12 == 1) {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                        MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_YV12 mIsInterlacing %d", mIsInterlacing);
                    } else {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                        MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK mIsInterlacing %d", mIsInterlacing);
                    }
                }
            }
            else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
            {
                __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM);
                MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM mIsInterlacing %d", mIsInterlacing);
            }
            else
            {
                //MTK_OMX_LOGE ("[DI] DISetGrallocExtra eColorFormat = %d", mOutputPortDef.format.video.eColorFormat);
                //__setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                //MTK_OMX_LOGD ("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK");
            }
            }
            else
            {
                //it should be handle the NULL case in non-meta mode
                //MTK_OMX_LOGD ("GrallocExtraSetBufParameter handle is null, skip once");
            }

        }
    }

    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        OMX_U32 graphicBufHandle = 0;
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

        if (mOutputPortFormat.eColorFormat == OMX_MTK_COLOR_FormatYV12)
        {
            __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
            MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_YV12 mIsInterlacing %d", mIsInterlacing);
        }
        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV)
        {
            if(mbIs10Bit)
            {
                if(mIsHorizontalScaninLSB)
                {
                    __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H);
                    MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H mIsInterlacing %d", mIsInterlacing);
                }
                else
                {
                    __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V);
                    MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V mIsInterlacing %d", mIsInterlacing);
                }
            }
            else
            {
                if (mConvertYV12 == 1) {
                    __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                    MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_YV12 mIsInterlacing %d", mIsInterlacing);
                } else {
                    __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                    MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK mIsInterlacing %d", mIsInterlacing);
                }
            }

        }
        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
        {
            __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM);
            MTK_OMX_LOGD("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM mIsInterlacing %d", mIsInterlacing);
        }
        else
        {
            //MTK_OMX_LOGE ("[DI] DISetGrallocExtra eColorFormat = %d", mOutputPortDef.format.video.eColorFormat);
            //__setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
            //MTK_OMX_LOGD ("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK");
        }
    }

    //MTK_OMX_LOGD("[DI] DISetGrallocExtra -");
}

OMX_BOOL MtkOmxVdec::WaitFence(OMX_BUFFERHEADERTYPE *mBufHdrType, OMX_BOOL mWaitFence)
{
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(mBufHdrType->pBuffer);

        if(kMetadataBufferTypeANWBuffer == nativeMeta.eType)
        {
            if( 0 <= nativeMeta.nFenceFd )
            {
                MTK_OMX_LOGD(" %s for fence %d", (OMX_TRUE == mWaitFence?"wait":"noWait"), nativeMeta.nFenceFd);

                //OMX_FLASE for flush and other FBD without getFrmBuffer case
                //should close FD directly
                if(OMX_TRUE == mWaitFence)
                {
                    sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                    int64_t startTime = getTickCountUs();
                    status_t ret = fence->wait(MTK_OMX_FENCE_TIMEOUT_MS);
                    int64_t duration = getTickCountUs() - startTime;
                    //Log waning on long duration. 10ms is an empirical value.
                    if (duration >= 10000){
                        MTK_OMX_LOGE("ret %x, wait fence %d took %lld us", ret, nativeMeta.nFenceFd, (long long)duration);
                    }
                }
                else
                {
                    //Fence::~Fence() would close fd automatically so decoder should not close
                    close(nativeMeta.nFenceFd);
                }
                //client need close and set -1 after waiting fence
                nativeMeta.nFenceFd = -1;
            }
        }
    }
    return OMX_TRUE;
}


// Need to access it within mFillThisBufQLock
int MtkOmxVdec::IncRef(int owner, VAL_ULONG_T va, bool isSecure)
{
    // Increase reference - should only be in mFrameBuf
    // Point of calling - GetFrmBuf (decoder gets output buffer), MJCRefBufQRemove (MJC gets input buffer)
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {

        int refCount = 0;
        if (VAL_FALSE == isSecure)
        {
            if (0 == va)
            {
                MTK_OMX_LOGE("[ERROR] IncRef null va");
                return REF_COUNT_ERROR_NOT_EXIST;
            }
            for (int i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (va == mFrameBuf[i].frame_buffer.rBaseAddr.u4VA)
                {
                    mFrameBuf[i].refCount++;
                    refCount = mFrameBuf[i].refCount;
                    //MTK_OMX_LOGD("[TEST-REF] %d IncRef 0x%x idx %d to refCount %d", owner, va, i, refCount);
                    if (refCount > 3 || (owner == 1 && refCount > 2))
                    {
                        MTK_OMX_LOGE("[TEST-REF][ERROR] IncRef Array 1 Index %d reference count over limit (3), print array 1", i);
                        for (int j = 0; j < mOutputPortDef.nBufferCountActual; j++)
                        {
                            MTK_OMX_LOGE("[TEST-REF][ERRROR] Array 1 Index %d, pBufHdr = 0x%x, va = 0x%x, ionBufHandle = %d, refCount = %d",
                                         j, mFrameBuf[j].ipOutputBuffer, va, mFrameBuf[j].ionBufHandle, mFrameBuf[j].refCount);
                        }
                    }
                    return refCount;
                }
            }
            MTK_OMX_LOGE("[ERROR] IncRef from %d, VA = 0x%x is not found", owner, va);
        }
        else
        {
            // Check secure handle instead of VA
        }

    }
    else
    {
        //MTK_OMX_LOGD("[INFO] IncRef not meta mode, ignore and return -1");
        return REF_COUNT_ERROR_NOT_META_MODE;
    }
    return 0;
}

// Need to access it within mFillThisBufQLock
int MtkOmxVdec::DecRef(int owner, VAL_ULONG_T va, bool isSecure)
{
    // Decrease reference - should only free if in mFrameBufInt
    // Point of calling - GetFreeFrame (free decoder ref), MJCRefBufQInsert (MJC free input reference)

    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        if (0 == va)
        {
            MTK_OMX_LOGE("[ERROR] DecRef null va");
            return REF_COUNT_ERROR_NOT_EXIST;
        }

        int refCount = 0;
        if (VAL_FALSE == isSecure)
        {
            for (int i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (va == mFrameBufInt[i].frame_buffer.rBaseAddr.u4VA)
                {
                    mFrameBufInt[i].refCount--;
                    refCount = mFrameBufInt[i].refCount;
                    if (refCount == 0)
                    {
                        mOutputMVAMgr->freeOmxMVAByVa((void *)mFrameBufInt[i].frame_buffer.rBaseAddr.u4VA);
                        mFrameBufInt[i].bDisplay = OMX_FALSE;
                        mFrameBufInt[i].bNonRealDisplay = OMX_FALSE;
                        mFrameBufInt[i].bUsed = OMX_FALSE;
                        mFrameBufInt[i].bFillThis = OMX_FALSE;
                        memset(&mFrameBufInt[i].frame_buffer, 0, sizeof(mFrameBufInt[i].frame_buffer));
                        mFrameBufInt[i].iTimestamp = 0;
                        mFrameBufInt[i].bGraphicBufHandle = 0;
                        mFrameBufInt[i].ionBufHandle = 0;
                    }
                    //MTK_OMX_LOGD("[TEST-REF] %d DecRef 0x%x idx %d array 2 to refCount %d", owner, va, i, refCount);
                    if (refCount < 0)
                    {
                        MTK_OMX_LOGE("[TEST-REF][ERROR] DecRef Array 2 Index %d reference count under limit (0), print array 2", i);
                        for (int j = 0; j < mOutputPortDef.nBufferCountActual; j++)
                        {
                            MTK_OMX_LOGE("[TEST-REF][ERRROR] Array 2 Index %d, pBufHdr = 0x%x, va = 0x%x, ionBufHandle = %d, refCount = %d",
                                         j, mFrameBufInt[j].ipOutputBuffer, va, mFrameBufInt[j].ionBufHandle, mFrameBufInt[j].refCount);
                        }
                    }
                    return refCount;
                }
            }
            for (int i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if (va == mFrameBuf[i].frame_buffer.rBaseAddr.u4VA)
                {
                    mFrameBuf[i].refCount--;
                    refCount = mFrameBuf[i].refCount;
                    if (refCount == 0)
                    {
                        MTK_OMX_LOGUD("[TEST-REF][Flush case] %d DecRef 0x%x in array 1 and ref count 0", owner, va);

                        mOutputMVAMgr->freeOmxMVAByVa((void *)mFrameBufInt[i].frame_buffer.rBaseAddr.u4VA);
                        mFrameBuf[i].bDisplay = OMX_FALSE;
                        mFrameBuf[i].bNonRealDisplay = OMX_FALSE;
                        mFrameBuf[i].bUsed = OMX_FALSE;
                        mFrameBuf[i].bFillThis = OMX_FALSE;
                        memset(&mFrameBufInt[i].frame_buffer, 0, sizeof(mFrameBufInt[i].frame_buffer));
                        mFrameBuf[i].iTimestamp = 0;
                        mFrameBuf[i].bGraphicBufHandle = 0;
                        mFrameBuf[i].ionBufHandle = 0;
                    }
                    //MTK_OMX_LOGD("[TEST-REF] %d DecRef 0x%x idx %d array 1 to refCount %d", owner, va, i, refCount);
                    if (refCount < 0)
                    {
                        MTK_OMX_LOGE("[TEST-REF][ERROR] DecRef Array 1 Index %d reference count under limit (0), print array 2", i);
                        for (int j = 0; j < mOutputPortDef.nBufferCountActual; j++)
                        {
                            MTK_OMX_LOGE("[TEST-REF][ERRROR] Array 1 Index %d, pBufHdr = 0x%x, va = 0x%x, ionBufHandle = %d, refCount = %d",
                                         j, mFrameBuf[j].ipOutputBuffer, va, mFrameBuf[j].ionBufHandle, mFrameBuf[j].refCount);
                        }
                    }
                    return refCount;
                }
            }
            MTK_OMX_LOGE("[ERROR] DecRef from %d, VA = 0x%x is not found", owner, va);
        }
        else
        {
            // Check secure handle instead of VA
        }

    }
    else
    {
        //MTK_OMX_LOGD("[INFO] IncRef not meta mode, ignore and return -1");
        return REF_COUNT_ERROR_NOT_META_MODE;
    }
    return 0;
}

int MtkOmxVdec::ProtectedIncRef(int owner, VAL_ULONG_T va, bool isSecure)
{
    int res = 0;
    LOCK_T(mFillThisBufQLock);
    res = IncRef(owner, va, isSecure);
    UNLOCK(mFillThisBufQLock);
    return res;
}


int MtkOmxVdec::ProtectedDecRef(int owner, VAL_ULONG_T va, bool isSecure)
{
    int res = 0;
    LOCK_T(mFillThisBufQLock);
    res = DecRef(owner, va, isSecure);
    UNLOCK(mFillThisBufQLock);
    return res;
}

void MtkOmxVdec::UpdateVideoInfo(OMX_TICKS Timestamp)
{
    if (0 != mFPSProfilingCountDown)
    {
        mFirstTimestamp = (mFirstTimestamp == 0) ? Timestamp: mFirstTimestamp;
        if (0 == (--mFPSProfilingCountDown))
        {
            VDEC_DRV_SET_VIDEO_INFO_T rVideoInfo;
            int Duration = (Timestamp - mFirstTimestamp);
            if (Duration > 0)
            {
                rVideoInfo.u4fps = (FPS_PROFILE_COUNT) * 1000000 / Duration;
                rVideoInfo.u4Width = mOutputPortDef.format.video.nStride;
                rVideoInfo.u4Height = mOutputPortDef.format.video.nSliceHeight;
                //MTK_OMX_LOGE("[UpdateVideoInfo] %lld - %lld = %lld", Timestamp, mFirstTimestamp, (Timestamp - mFirstTimestamp));
                eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_VIDEO_INFO, &rVideoInfo, NULL);
            }
            else
            {
                MTK_OMX_LOGE("[ERROR] UpdateVideoInfo fails due to wrong timestamp");
            }
        }
    }
}
OMX_BOOL MtkOmxVdec::needLegacyMode(void)
{
    if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
    {
        VDEC_DRV_MRESULT_T rResult = VDEC_DRV_MRESULT_OK;
        if(mIsSecureInst == OMX_TRUE)
        {
            //support svp legacy: D3/55/57/59/63/71/75/97/99
            OMX_U64 bIsSupportSVPPlatform = VAL_FALSE;
            rResult = eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_SUPPORT_SVP_LEGACY, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&bIsSupportSVPPlatform);
            if(bIsSupportSVPPlatform == VAL_TRUE)
            {
                return OMX_TRUE;
            }
        }
        else
        {
            //legacy: 70/80/D2
            OMX_U64 bIsNeedLegacyMode = VAL_FALSE;
            rResult = eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_NEED_LEGACY_MODE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&bIsNeedLegacyMode);
            if(bIsNeedLegacyMode == VAL_TRUE)
            {
                return OMX_TRUE;
            }
        }
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::needColorConvertWithNativeWindow(void)
{
    if (OMX_TRUE == needColorConvertWithMetaMode() || OMX_TRUE == needColorConvertWithoutMetaMode() ||
        OMX_TRUE == mNativeWindowHDRInternalConvert || mConvertYV12 == 1)
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;

}

OMX_BOOL MtkOmxVdec::needColorConvertWithMetaMode(void)
{
    if ((OMX_TRUE == mStoreMetaDataInBuffers && OMX_TRUE == mbYUV420FlexibleMode))
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::needColorConvertWithoutMetaMode(void)
{
    if ((OMX_TRUE == needLegacyMode() && OMX_TRUE == mbYUV420FlexibleMode))
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

bool MtkOmxVdec::supportAutoEnlarge(void)
{
    if ((mCodecId == MTK_VDEC_CODEC_ID_AVC) && (OMX_FALSE == mIsSecureInst))
    {
        return true;
    }
    return false;
}

void MtkOmxVdec::HandleVendorMtkOmxVdecUseClearMotion(OMX_BOOL mForceAllocateBuffer)
{
    if (mMJCEnable == OMX_TRUE)
    {
    if (OMX_TRUE == mIsSecureInst)
    {
        MTK_OMX_LOGUD("BYPASS MJC for SVP");
        mUseClearMotion = OMX_FALSE;
        mMJCReconfigFlag = OMX_FALSE;
        MJC_MODE mMode;
        mMode = MJC_MODE_BYPASS;
        m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
        return;
    }

    bool mNormVendorUseClearMotion = (mMJCFakeEngine == OMX_FALSE && mForceAllocateBuffer == OMX_FALSE);
    bool mFakeEngineForceAlloc = (mMJCFakeEngine == OMX_TRUE && mForceAllocateBuffer == OMX_TRUE);
    if (mUseClearMotion == OMX_FALSE && mSucessfullyRegisterMJC == OMX_TRUE && (mNormVendorUseClearMotion || mFakeEngineForceAlloc))
    {
        if ((mOutputPortDef.nBufferCountActual + FRAMEWORK_OVERHEAD + mMinUndequeuedBufs + TOTAL_MJC_BUFFER_CNT) <= MAX_TOTAL_BUFFER_CNT) // Jimmy temp
        {
            MTK_OMX_LOGUD("[MJC] (1)\n");

            MJC_MODE mMode;
            if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC || mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)
            {
                mMJCReconfigFlag = OMX_TRUE;    //MJCReconfigFlag for port reconfig can reopen MJC
                mUseClearMotion = OMX_TRUE;
                mMode = MJC_MODE_NORMAL;
            }
            else if(mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight > 1920*1088)
            {
                mOutputPortDef.nBufferCountActual += (FRAMEWORK_OVERHEAD + mMinUndequeuedBufs);
                mOutputPortDef.nBufferCountMin += (FRAMEWORK_OVERHEAD + mMinUndequeuedBufs);
                mMJCReconfigFlag = OMX_FALSE;
                mUseClearMotion = OMX_FALSE;
                mMode = MJC_MODE_BYPASS;
            }
            else
            {
                mOutputPortDef.nBufferCountActual += (FRAMEWORK_OVERHEAD + TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs); // Jimmy temp
                mOutputPortDef.nBufferCountMin += (FRAMEWORK_OVERHEAD + TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs); // Jimmy temp
                mMJCReconfigFlag = OMX_FALSE;
                mUseClearMotion = OMX_TRUE;
                mMode = MJC_MODE_NORMAL;
            }

            m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
            m_fnMJCSetParam(mpMJC, MJC_PARAM_SET_DEMO_MODE, &mClearMotionDemoMode);
        }
        else
        {
            MTK_OMX_LOGUD("[MJC] (0)\n");
            mUseClearMotion = OMX_FALSE;
            mMJCReconfigFlag = OMX_FALSE;
            MJC_MODE mMode;
            mMode = MJC_MODE_BYPASS;
            m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
        }
    }
    else
    {
        MTK_OMX_LOGUD("[MJC] (2)\n");
    }
    }
}

OMX_BOOL MtkOmxVdec::IsGPUSupportBlock()
{
    VDEC_DRV_MRESULT_T rResult = VDEC_DRV_MRESULT_OK;
    OMX_U32 bIsGPUSupportBlock = VAL_FALSE;
    rResult = eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_GPU_SUPPORT_BLOCK, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&bIsGPUSupportBlock);
    MTK_OMX_LOGUD("bIsGPUSupportBlock %d", bIsGPUSupportBlock);
    //97/99
    if (bIsGPUSupportBlock) {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::IsHDRSetByFramework()
{
    if (mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxDisplayLuminance != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mMinDisplayLuminance != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mW.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mW.y != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mR.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mR.y != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mG.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mG.y != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mB.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mB.y != 0 )
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::CopyHDRColorDesc()
{
    if (mDescribeColorAspectsParams.sAspects.mPrimaries >= HDR_Color_Primaries_Map_SIZE ||
        mDescribeColorAspectsParams.sAspects.mTransfer >= HDR_Transfer_Map_SIZE  ||
        mDescribeColorAspectsParams.sAspects.mMatrixCoeffs >= HDR_Matrix_Coeff_Map_SIZE )
    {
        MTK_OMX_LOGE("[ERROR] DescribeColorAspectsParams aspects out of range");
        return OMX_FALSE;
    }

    mColorDesc.u4ColorPrimaries = HDRColorPrimariesMap[mDescribeColorAspectsParams.sAspects.mPrimaries];
    mColorDesc.u4TransformCharacter = HDRTransferMap[mDescribeColorAspectsParams.sAspects.mTransfer];
    mColorDesc.u4MatrixCoeffs = HDRMatrixCoeffMap[mDescribeColorAspectsParams.sAspects.mMatrixCoeffs];

    mColorDesc.u4MaxDisplayMasteringLuminance = mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxDisplayLuminance;
    mColorDesc.u4MinDisplayMasteringLuminance = mDescribeHDRStaticInfoParams.sInfo.sType1.mMinDisplayLuminance;
    mColorDesc.u4WhitePointX = mDescribeHDRStaticInfoParams.sInfo.sType1.mW.x;
    mColorDesc.u4WhitePointY = mDescribeHDRStaticInfoParams.sInfo.sType1.mW.y;
    mColorDesc.u4DisplayPrimariesX[0] = mDescribeHDRStaticInfoParams.sInfo.sType1.mG.x;
    mColorDesc.u4DisplayPrimariesX[1] = mDescribeHDRStaticInfoParams.sInfo.sType1.mB.x;
    mColorDesc.u4DisplayPrimariesX[2] = mDescribeHDRStaticInfoParams.sInfo.sType1.mR.x;
    mColorDesc.u4DisplayPrimariesY[0] = mDescribeHDRStaticInfoParams.sInfo.sType1.mG.y;
    mColorDesc.u4DisplayPrimariesY[1] = mDescribeHDRStaticInfoParams.sInfo.sType1.mB.y;
    mColorDesc.u4DisplayPrimariesY[2] = mDescribeHDRStaticInfoParams.sInfo.sType1.mR.y;
    mColorDesc.u4MaxContentLightLevel = mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxContentLightLevel;
    mColorDesc.u4MaxPicAverageLightLevel = mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxFrameAverageLightLevel;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::EnableCodecDriverUFO()
{
    VAL_BOOL_T bUFOsupport = VAL_FALSE;

    if (VDEC_DRV_MRESULT_FAIL != eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_UFO_SUPPORT, VAL_NULL, &bUFOsupport))
    {
        MTK_OMX_LOGD("[MtkOmxVdec] Get decoder property, VDEC_DRV_QUERY_TYPE_UFO_SUPPORT: %d, %d", bUFOsupport, __LINE__);
    }

    if ((mCodecId == MTK_VDEC_CODEC_ID_HEVC || mCodecId == MTK_VDEC_CODEC_ID_AVC || mCodecId == MTK_VDEC_CODEC_ID_VP9)
         && (mANW_HWComposer == OMX_TRUE || (!IsGPUSupportBlock()))
         && mIsUsingNativeBuffers == OMX_TRUE
         && bUFOsupport == OMX_TRUE
         && mIsSecureInst == OMX_FALSE
         && ((mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight > 1920 * 1088) || ((mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight == 1920 * 1088))))
    {
        VAL_CHAR_T UFOSetting[PROPERTY_VALUE_MAX];
        OMX_BOOL UFOEnable = OMX_TRUE;
        VDEC_DRV_MRESULT_T rResult = VDEC_DRV_MRESULT_FAIL;

        property_get("vendor.mtk.omxvdec.ufo", UFOSetting, "1");
        UFOEnable = (OMX_BOOL) atoi(UFOSetting);
        if (UFOEnable == OMX_TRUE) {
            rResult = eVDecDrvSetParam(mDrvHandle, VDEC_DRV_SET_TYPE_SET_UFO_DECODE, NULL, NULL);
        }

        if (UFOEnable == OMX_TRUE && rResult == VDEC_DRV_MRESULT_OK)
        {
            if (OMX_TRUE == mbIs10Bit)
            {
                if (OMX_TRUE == mIsHorizontalScaninLSB)
                {
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
                }
                else
                {
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
                    mOutputPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
                }
            }
            else
            {
                mOutputPortFormat.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                mOutputPortDef.format.video.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            }
            MTK_OMX_LOGU("Enable UFO (OMX_IndexParamVideoPortFormat)");
            return OMX_TRUE;
        }
    }

    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::CheckLogEnable()
{
    OMX_BOOL nResult = OMX_TRUE;
    char BuildType[PROPERTY_VALUE_MAX];
    char OmxVdecLogValue[PROPERTY_VALUE_MAX];
    char OmxVdecPerfLogValue[PROPERTY_VALUE_MAX];

    property_get("ro.build.type", BuildType, "eng");
    if (!strcmp(BuildType,"eng")){
        property_get("vendor.mtk.omx.vdec.log", OmxVdecLogValue, "0");
        mOmxVdecLogEnable = (OMX_BOOL) atoi(OmxVdecLogValue);
        property_get("vendor.mtk.omx.vdec.perf.log", OmxVdecPerfLogValue, "0");
        mOmxVdecPerfLogEnable = (OMX_BOOL) atoi(OmxVdecPerfLogValue);
        if (mMJCEnable == OMX_TRUE) {
            property_get("vendor.mtk.omx.vdec.perf.log", OmxVdecPerfLogValue, "0");
            mOmxVdecPerfLogEnable = (OMX_BOOL) atoi(OmxVdecPerfLogValue);
        }
    } else if (!strcmp(BuildType,"userdebug") || !strcmp(BuildType,"user")) {
        property_get("vendor.mtk.omx.vdec.log", OmxVdecLogValue, "0");
        mOmxVdecLogEnable = (OMX_BOOL) atoi(OmxVdecLogValue);
        property_get("vendor.mtk.omx.vdec.perf.log", OmxVdecPerfLogValue, "0");
        mOmxVdecPerfLogEnable = (OMX_BOOL) atoi(OmxVdecPerfLogValue);
    }
    return nResult;
}

void MtkOmxVdec::SetInputPortViLTE()
{
    if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
    {
        mInputPortDef.nBufferCountActual = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT_VILTE;
        mInputPortDef.nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_HD;
    }
}

void MtkOmxVdec::HandleColorAspectChange(ColorAspects defaultAspects)
{
    ColorAspects tempAspects;
    tempAspects.mRange = mDescribeColorAspectsParams.sAspects.mRange != ColorAspects::RangeUnspecified ?
        mDescribeColorAspectsParams.sAspects.mRange : defaultAspects.mRange;
    tempAspects.mPrimaries = mDescribeColorAspectsParams.sAspects.mPrimaries != ColorAspects::PrimariesUnspecified ?
        mDescribeColorAspectsParams.sAspects.mPrimaries : defaultAspects.mPrimaries;
    tempAspects.mTransfer = mDescribeColorAspectsParams.sAspects.mTransfer != ColorAspects::TransferUnspecified ?
        mDescribeColorAspectsParams.sAspects.mTransfer : defaultAspects.mTransfer;
    tempAspects.mMatrixCoeffs = mDescribeColorAspectsParams.sAspects.mMatrixCoeffs != ColorAspects::MatrixUnspecified ?
        mDescribeColorAspectsParams.sAspects.mMatrixCoeffs : defaultAspects.mMatrixCoeffs;
    if(OMX_TRUE == ColorAspectsDiffer(tempAspects, mDescribeColorAspectsParams.sAspects))
    {
        mDescribeColorAspectsParams.sAspects = tempAspects;
    }
}

OMX_BOOL MtkOmxVdec::ColorAspectsDiffer(ColorAspects a, ColorAspects b)
{
    if (a.mRange != b.mRange || a.mPrimaries != b.mPrimaries || a.mTransfer != b.mTransfer || a.mMatrixCoeffs != b.mMatrixCoeffs)
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

/////////////////////////// -------------------   globalc functions -----------------------------------------///////////
OMX_ERRORTYPE MtkVdec_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_ComponentInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentInit(hComponent, componentName);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                   OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SetCallbacks");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetCallbacks(hComponent, pCallBacks, pAppData);
    }

    return err;
}


OMX_ERRORTYPE MtkVdec_ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_ComponentDeInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentDeInit(hComponent);
        delete(MtkOmxBase *)pHandle->pComponentPrivate;
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_STRING componentName,
                                          OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                          OMX_OUT OMX_VERSIONTYPE *specVersion,
                                          OMX_OUT OMX_UUIDTYPE *componentUUID)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetComponentVersion");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetComponentVersion(hComponent, componentName, componentVersion, specVersion, componentUUID);
    }
    return err;
}

OMX_ERRORTYPE MtkVdec_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_COMMANDTYPE Cmd,
                                  OMX_IN OMX_U32 nParam1,
                                  OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SendCommand");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SendCommand(hComponent, Cmd, nParam1, pCmdData);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_INDEXTYPE nParamIndex,
                                   OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetParameter(hComponent, nParamIndex, pCompParam);
    }
    return err;
}

OMX_ERRORTYPE MtkVdec_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN  OMX_INDEXTYPE nParamIndex,
                                   OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING parameterName,
                                        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetExtensionIndex");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetExtensionIndex(hComponent, parameterName, pIndexType);
    }
    return err;
}

OMX_ERRORTYPE MtkVdec_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                               OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetState");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetState(hComponent, pState);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_AllocateBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->AllocateBuffer(hComponent, pBuffHead, nPortIndex, pAppPrivate, nSizeBytes);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                OMX_IN OMX_U32 nPortIndex,
                                OMX_IN OMX_PTR pAppPrivate,
                                OMX_IN OMX_U32 nSizeBytes,
                                OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    // MTK_OMX_LOGD ("MtkVdec_UseBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_IN OMX_U32 nPortIndex,
                                 OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_FreeBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FreeBuffer(hComponent, nPortIndex, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_EmptyThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->EmptyThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_FillThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FillThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U8 *cRole,
                                        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_ComponentRoleEnum");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentRoleEnum(hComponent, cRole, nIndex);
    }
    return err;
}


// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{

    MtkOmxBase *pVdec  = new MtkOmxVdec;

    if (NULL == pVdec)
    {
        ALOGE("[0x%08x] MtkOmxComponentCreate out of memory!!!", pVdec);
        return NULL;
    }

    OMX_COMPONENTTYPE *pHandle = pVdec->GetComponentHandle();
    ALOGD("[0x%08x] MtkOmxComponentCreate mCompHandle(0x%08X)", pVdec, (unsigned int)pHandle);

    pHandle->SetCallbacks                  = MtkVdec_SetCallbacks;
    pHandle->ComponentDeInit               = MtkVdec_ComponentDeInit;
    pHandle->SendCommand                   = MtkVdec_SendCommand;
    pHandle->SetParameter                  = MtkVdec_SetParameter;
    pHandle->GetParameter                  = MtkVdec_GetParameter;
    pHandle->GetExtensionIndex        = MtkVdec_GetExtensionIndex;
    pHandle->GetState                      = MtkVdec_GetState;
    pHandle->SetConfig                     = MtkVdec_SetConfig;
    pHandle->GetConfig                     = MtkVdec_GetConfig;
    pHandle->AllocateBuffer                = MtkVdec_AllocateBuffer;
    pHandle->UseBuffer                     = MtkVdec_UseBuffer;
    pHandle->FreeBuffer                    = MtkVdec_FreeBuffer;
    pHandle->GetComponentVersion           = MtkVdec_GetComponentVersion;
    pHandle->EmptyThisBuffer            = MtkVdec_EmptyThisBuffer;
    pHandle->FillThisBuffer                 = MtkVdec_FillThisBuffer;

    OMX_ERRORTYPE err = MtkVdec_ComponentInit((OMX_HANDLETYPE)pHandle, componentName);
    if (err != OMX_ErrorNone)
    {
        ALOGE("[0x%08x] MtkOmxComponentCreate init failed, error = 0x%x", pVdec, err);
        MtkVdec_ComponentDeInit((OMX_HANDLETYPE)pHandle);
        pHandle = NULL;
    }

    return pHandle;
}

extern "C" void MtkOmxSetCoreGlobal(OMX_COMPONENTTYPE *pHandle, void *data)
{
    ((mtk_omx_core_global *)data)->video_instance_count++;
    ((MtkOmxBase *)(pHandle->pComponentPrivate))->SetCoreGlobal(data);
}
