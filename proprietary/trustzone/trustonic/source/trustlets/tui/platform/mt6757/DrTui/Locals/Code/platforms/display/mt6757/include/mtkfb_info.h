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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef __MTKFB_INFO_H__
#define __MTKFB_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif


	typedef enum {
		DISPIF_TYPE_DBI = 0,
		DISPIF_TYPE_DPI,
		DISPIF_TYPE_DSI,
		DISPIF_TYPE_DPI0,
		DISPIF_TYPE_DPI1,
		DISPIF_TYPE_DSI0,
		DISPIF_TYPE_DSI1,
		HDMI = 7,
		HDMI_SMARTBOOK,
		MHL,
		DISPIF_TYPE_EPD,
		SLIMPORT
	} MTKFB_DISPIF_TYPE;

	typedef enum {
		MTKFB_DISPIF_PRIMARY_LCD = 0,
		MTKFB_DISPIF_HDMI,
		MTKFB_DISPIF_EPD,
		MTKFB_MAX_DISPLAY_COUNT
	} MTKFB_DISPIF_DEVICE_TYPE;

	typedef enum {
		DISPIF_FORMAT_RGB565 = 0,
		DISPIF_FORMAT_RGB666,
		DISPIF_FORMAT_RGB888
	} MTKFB_DISPIF_FORMAT;


	typedef enum {
		DISPIF_MODE_VIDEO = 0,
		DISPIF_MODE_COMMAND
	} MTKFB_DISPIF_MODE;

	typedef struct mtk_dispif_info {
		unsigned int display_id;
		unsigned int isHwVsyncAvailable;
		MTKFB_DISPIF_TYPE displayType;
		unsigned int displayWidth;
		unsigned int displayHeight;
		unsigned int displayFormat;
		MTKFB_DISPIF_MODE displayMode;
		unsigned int vsyncFPS;
		unsigned int physicalWidth;
		unsigned int physicalHeight;
		unsigned int isConnected;
		unsigned int lcmOriginalWidth; /* this value is for DFO Multi-Resolution feature, which stores the original LCM Wdith */
		unsigned int lcmOriginalHeight; /* this value is for DFO Multi-Resolution feature, which stores the original LCM Height */
	} mtk_dispif_info_t;

#define MAKE_MTK_FB_FORMAT_ID(id, bpp)  (((id) << 8) | (bpp))

	typedef enum {
		MTK_FB_FORMAT_UNKNOWN = 0,

		MTK_FB_FORMAT_RGB565 = MAKE_MTK_FB_FORMAT_ID(1, 2),
		MTK_FB_FORMAT_RGB888 = MAKE_MTK_FB_FORMAT_ID(2, 3),
		MTK_FB_FORMAT_BGR888 = MAKE_MTK_FB_FORMAT_ID(3, 3),
		MTK_FB_FORMAT_ARGB8888 = MAKE_MTK_FB_FORMAT_ID(4, 4),
		MTK_FB_FORMAT_ABGR8888 = MAKE_MTK_FB_FORMAT_ID(5, 4),
		MTK_FB_FORMAT_YUV422 = MAKE_MTK_FB_FORMAT_ID(6, 2),
		MTK_FB_FORMAT_XRGB8888 = MAKE_MTK_FB_FORMAT_ID(7, 4),
		MTK_FB_FORMAT_XBGR8888 = MAKE_MTK_FB_FORMAT_ID(8, 4),
		MTK_FB_FORMAT_UYVY = MAKE_MTK_FB_FORMAT_ID(9, 2),
		MTK_FB_FORMAT_YUV420_P = MAKE_MTK_FB_FORMAT_ID(10, 2),
		MTK_FB_FORMAT_YUY2 = MAKE_MTK_FB_FORMAT_ID(11, 2),
		MTK_FB_FORMAT_RGBA8888 = MAKE_MTK_FB_FORMAT_ID(12, 4),
		MTK_FB_FORMAT_BGRA8888 = MAKE_MTK_FB_FORMAT_ID(13, 4),
		MTK_FB_FORMAT_BPP_MASK = 0xFF,
	} MTK_FB_FORMAT;

#define GET_MTK_FB_FORMAT_BPP(f)    ((f) & MTK_FB_FORMAT_BPP_MASK)


#ifdef __cplusplus
}
#endif
#endif				/* __DISP_DRV_H__ */
