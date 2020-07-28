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

#ifndef __DISP_SESSION_H
#define __DISP_SESSION_H

#define DISP_SESSION_DEVICE	"mtk_disp_mgr"


#define DISP_NO_ION_FD                 ((int)(~0U>>1))
#define DISP_NO_USE_LAEYR_ID           ((int)(~0U>>1))

#define MAKE_DISP_FORMAT_ID(id, bpp)  (((id) << 8) | (bpp))
#define DISP_SESSION_MODE(id) (((id)>>24)&0xff)
#define DISP_SESSION_TYPE(id) (((id)>>16)&0xff)
#define DISP_SESSION_DEV(id) ((id)&0xff)
#define MAKE_DISP_SESSION(type, dev) (unsigned int)( (type)<<16 | (dev))



/* /============================================================================= */
/* structure declarations */
/* /=========================== */

typedef enum {
	DISP_IF_TYPE_DBI = 0,
	DISP_IF_TYPE_DPI,
	DISP_IF_TYPE_DSI0,
	DISP_IF_TYPE_DSI1,
	DISP_IF_TYPE_DSIDUAL,
	DISP_IF_HDMI = 7,
	DISP_IF_HDMI_SMARTBOOK,
	DISP_IF_MHL,
	DISP_IF_EPD,
	DISP_IF_SLIMPORT
} DISP_IF_TYPE;

typedef enum {
	DISP_IF_FORMAT_RGB565 = 0,
	DISP_IF_FORMAT_RGB666,
	DISP_IF_FORMAT_RGB888
} DISP_IF_FORMAT;

typedef enum {
	DISP_IF_MODE_VIDEO = 0,
	DISP_IF_MODE_COMMAND
} DISP_IF_MODE;


typedef enum {
	DISP_ORIENTATION_0 = 0,
	DISP_ORIENTATION_90 = 1,
	DISP_ORIENTATION_180 = 2,
	DISP_ORIENTATION_270 = 3,
} DISP_ORIENTATION;

typedef enum {
	DISP_FORMAT_UNKNOWN = 0,

	DISP_FORMAT_RGB565 = MAKE_DISP_FORMAT_ID(1, 2),
	DISP_FORMAT_RGB888 = MAKE_DISP_FORMAT_ID(2, 3),
	DISP_FORMAT_BGR888 = MAKE_DISP_FORMAT_ID(3, 3),
	DISP_FORMAT_ARGB8888 = MAKE_DISP_FORMAT_ID(4, 4),
	DISP_FORMAT_ABGR8888 = MAKE_DISP_FORMAT_ID(5, 4),
	DISP_FORMAT_RGBA8888 = MAKE_DISP_FORMAT_ID(6, 4),
	DISP_FORMAT_BGRA8888 = MAKE_DISP_FORMAT_ID(7, 4),
	DISP_FORMAT_YUV422 = MAKE_DISP_FORMAT_ID(8, 2),
	DISP_FORMAT_XRGB8888 = MAKE_DISP_FORMAT_ID(9, 4),
	DISP_FORMAT_XBGR8888 = MAKE_DISP_FORMAT_ID(10, 4),
	DISP_FORMAT_RGBX8888 = MAKE_DISP_FORMAT_ID(11, 4),
	DISP_FORMAT_BGRX8888 = MAKE_DISP_FORMAT_ID(12, 4),
	DISP_FORMAT_UYVY = MAKE_DISP_FORMAT_ID(13, 2),
	DISP_FORMAT_YUV420_P = MAKE_DISP_FORMAT_ID(14, 2),
	DISP_FORMAT_YV12 = MAKE_DISP_FORMAT_ID(16, 1),	/* BPP = 1.5 */
	DISP_FORMAT_PARGB8888 = MAKE_DISP_FORMAT_ID(17, 4),
	DISP_FORMAT_PABGR8888 = MAKE_DISP_FORMAT_ID(18, 4),
	DISP_FORMAT_PRGBA8888 = MAKE_DISP_FORMAT_ID(19, 4),
	DISP_FORMAT_PBGRA8888 = MAKE_DISP_FORMAT_ID(20, 4),
	DISP_FORMAT_DIM = MAKE_DISP_FORMAT_ID(21, 0),
	DISP_FORMAT_BPP_MASK = 0xFF,
} DISP_FORMAT;

typedef enum {
	DISP_LAYER_2D = 0,
	DISP_LAYER_3D_SBS_0 = 0x1,
	DISP_LAYER_3D_SBS_90 = 0x2,
	DISP_LAYER_3D_SBS_180 = 0x3,
	DISP_LAYER_3D_SBS_270 = 0x4,
	DISP_LAYER_3D_TAB_0 = 0x10,
	DISP_LAYER_3D_TAB_90 = 0x20,
	DISP_LAYER_3D_TAB_180 = 0x30,
	DISP_LAYER_3D_TAB_270 = 0x40,
} DISP_LAYER_TYPE;

typedef enum {
	/* normal memory */
	DISP_NORMAL_BUFFER = 0,
	/* normal memory but should not be dumpped within screenshot */
	DISP_PROTECT_BUFFER = 1,
	/* secure memory */
	DISP_SECURE_BUFFER = 2,
	DISP_SECURE_BUFFER_SHIFT = 0x10002
} DISP_BUFFER_TYPE;

typedef enum {
	/* ion buffer */
	DISP_BUFFER_ION = 0,
	/* dim layer, const alpha */
	DISP_BUFFER_ALPHA = 1,
	/* mva buffer */
	DISP_BUFFER_MVA = 2,
} DISP_BUFFER_SOURCE;

typedef enum {
	DISP_ALPHA_ONE = 0,
	DISP_ALPHA_SRC = 1,
	DISP_ALPHA_SRC_INVERT = 2,
	DISP_ALPHA_INVALID = 3,
} DISP_ALPHA_TYPE;

typedef enum {
	DISP_SESSION_PRIMARY = 1,
	DISP_SESSION_EXTERNAL = 2,
	DISP_SESSION_MEMORY = 3
} DISP_SESSION_TYPE;

typedef enum {
	DISP_YUV_BT601_FULL 	= 0,
	DISP_YUV_BT601	= 1,
	DISP_YUV_BT709  = 2
}DISP_YUV_RANGE_ENUM;


typedef enum {
	DISP_INVALID_SESSION_MODE = 0,
	/* single output */
	DISP_SESSION_DIRECT_LINK_MODE = 1,
	DISP_SESSION_DECOUPLE_MODE = 2,

	/* two ouputs */
	DISP_SESSION_DIRECT_LINK_MIRROR_MODE = 3,
	DISP_SESSION_DECOUPLE_MIRROR_MODE = 4,
	DISP_SESSION_RDMA_MODE,
	DISP_SESSION_MODE_NUM,
} DISP_MODE;

typedef enum {
    SESSION_USER_INVALID = -1,
	SESSION_USER_HWC 	 = 0,
	SESSION_USER_GUIEXT	 = 1,
	SESSION_USER_AEE	 = 2,
	SESSION_USER_PANDISP = 3,
	SESSION_USER_CNT,
}DISP_SESSION_USER;

typedef enum {
	DISP_OUTPUT_UNKNOWN				= 0,
	DISP_OUTPUT_MEMORY				= 1,
	DISP_OUTPUT_DECOUPLE			= 2,
	DISP_OUTPUT_DIRECT_LINK			= 3,
	DISP_OUTPUT_DIRECT_LINK_MIRROR	= 4,
}DISP_OUTPUT_TYPE;

typedef enum {
	TRIGGER_NORMAL,
	TRIGGER_SUSPEND,
	TRIGGER_RESUME,

	TRIGGER_MODE_MAX_NUM
} EXTD_TRIGGER_MODE;
typedef struct disp_session_config_t {
	DISP_SESSION_TYPE type;
	unsigned int device_id;
	DISP_MODE mode;
	unsigned int session_id;
	DISP_SESSION_USER user;
	unsigned int present_fence_idx;
	DISP_OUTPUT_TYPE output_type;
	int need_merge;
	EXTD_TRIGGER_MODE tigger_mode;
} disp_session_config;

typedef struct {
	unsigned int session_id;
	unsigned int vsync_cnt;
	unsigned long long vsync_ts;
    int lcm_fps;
} disp_session_vsync_config;

typedef struct disp_input_config_t {
	void *src_base_addr;
	void *src_phy_addr;
	DISP_BUFFER_SOURCE  buffer_source;
	DISP_BUFFER_TYPE security;
	
	DISP_FORMAT src_fmt;
	DISP_ALPHA_TYPE src_alpha;
	DISP_ALPHA_TYPE dst_alpha;
	DISP_YUV_RANGE_ENUM yuv_range;

	DISP_ORIENTATION layer_rotation;
	DISP_LAYER_TYPE layer_type;
	DISP_ORIENTATION video_rotation;
	unsigned int next_buff_idx;
	unsigned int src_fence_fd;	/* fence to be waited before using this buffer. -1 if invalid */
	void *src_fence_struct;	/* fence struct of src_fence_fd, used in kernel */
	unsigned int src_color_key;
	unsigned int frm_sequence;
	void *dirty_roi_addr;
	unsigned int dirty_roi_num;
	
	unsigned int src_pitch;
	unsigned int src_offset_x, src_offset_y;
	unsigned int src_width, src_height;

	unsigned int tgt_offset_x, tgt_offset_y;
	unsigned int tgt_width, tgt_height;
	
	unsigned int alpha_enable;
	unsigned int alpha;
	unsigned int sur_aen;
	unsigned int src_use_color_key;
	unsigned int layer_id;
	unsigned int layer_enable;
 	unsigned int src_direct_link;
	unsigned int isTdshp;	/* set to 1, will go through tdshp first, then layer blending, then to color */
	int identity;
	int connected_type;
	int ext_sel_layer;
} disp_input_config;

typedef struct disp_output_config_t {
	void *va;
	void *pa;
	DISP_FORMAT fmt;
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int pitchUV;
	DISP_BUFFER_TYPE security;
	unsigned int buff_idx;
	unsigned int interface_idx;
	unsigned int src_fence_fd;	/* fence to be waited before using this buffer. -1 if invalid */
	void *src_fence_struct;		/* fence struct of src_fence_fd, used in kernel */
	unsigned int frm_sequence;
} disp_output_config;

typedef struct disp_session_input_config_t {
	DISP_SESSION_USER setter;
	unsigned int session_id;
	unsigned int config_layer_num;
	disp_input_config config[8];
} disp_session_input_config;

typedef struct disp_session_output_config_t {
	unsigned int session_id;
	disp_output_config config;
} disp_session_output_config;

typedef struct disp_session_layer_num_config_t {
	unsigned int session_id;
	unsigned int max_layer_num;
} disp_session_layer_num_config;


#endif /* __DISP_SESSION_H */
