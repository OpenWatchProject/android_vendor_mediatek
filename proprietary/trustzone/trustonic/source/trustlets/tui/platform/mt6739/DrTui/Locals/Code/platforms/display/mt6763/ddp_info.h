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

#ifndef _H_DDP_INFO
#define _H_DDP_INFO

#include "ddp_hal.h"
#include "lcm_drv.h"
#include "disp_session.h"
#include "cmdq_sec_record.h"
#include "ddp_color_format.h"

struct disp_rect {
	int x;
	int y;
	int width;
	int height;
	int is_dual;
};

struct OVL_CONFIG_STRUCT {
	unsigned int ovl_index;
	unsigned int layer;
	unsigned int layer_en;
	enum OVL_LAYER_SOURCE source;
	enum UNIFIED_COLOR_FMT fmt;
	unsigned long addr;
	unsigned long vaddr;
	unsigned int src_x;
	unsigned int src_y;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int src_pitch;
	unsigned int dst_x;
	unsigned int dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int keyEn;
	unsigned int key;
	unsigned int aen;
	unsigned char alpha;

	unsigned int sur_aen;
	unsigned int src_alpha;
	unsigned int dst_alpha;

	unsigned int isTdshp;
	unsigned int isDirty;

	unsigned int buff_idx;
	unsigned int identity;
	unsigned int connected_type;
	enum DISP_BUFFER_TYPE security;
	unsigned int yuv_range;
	int is_configured;	/* is this layer configured to OVL HW, for multiply OVL sync */
	int const_bld;
	int ext_sel_layer;
	int ext_layer;
	int phy_layer;
};

struct OVL_BASIC_STRUCT {
	unsigned int layer;
	unsigned int layer_en;
	enum UNIFIED_COLOR_FMT fmt;
	unsigned long addr;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int src_pitch;
	unsigned int bpp;
	unsigned int gpu_mode;
	unsigned int adobe_mode;
	unsigned int ovl_gamma_out;
	unsigned int alpha;
};

struct rdma_bg_ctrl_t {
	unsigned int left;
	unsigned int right;
	unsigned int top;
	unsigned int bottom;
};

struct RDMA_CONFIG_STRUCT {
	unsigned idx;		/* instance index */
	enum UNIFIED_COLOR_FMT inputFormat;
	unsigned long address;
	unsigned pitch;
	unsigned width;
	unsigned height;
	unsigned dst_w;
	unsigned dst_h;
	unsigned dst_x;
	unsigned dst_y;
	enum DISP_BUFFER_TYPE security;
	unsigned int yuv_range;
	struct rdma_bg_ctrl_t bg_ctrl;
};

struct WDMA_CONFIG_STRUCT {
	unsigned srcWidth;
	unsigned srcHeight;	/* input */
	unsigned clipX;
	unsigned clipY;
	unsigned clipWidth;
	unsigned clipHeight;	/* clip */
	enum UNIFIED_COLOR_FMT outputFormat;
	unsigned long dstAddress;
	unsigned dstPitch;	/* output */
	unsigned int useSpecifiedAlpha;
	unsigned char alpha;
	enum DISP_BUFFER_TYPE security;
};

#define TOTAL_OVL_LAYER_NUM		(2)

typedef struct {
	/* for ovl */
	bool ovl_dirty;
	bool rdma_dirty;
	bool dst_dirty;
	int ovl_layer_scanned;	/*each bit reprsent one layer, used for ovl engines */
	struct OVL_CONFIG_STRUCT ovl_config[TOTAL_OVL_LAYER_NUM];
	struct RDMA_CONFIG_STRUCT rdma_config;
	unsigned int dst_w;
	unsigned int dst_h;
} disp_ddp_path_config;

struct golden_setting_context {
	unsigned int fifo_mode;
	unsigned int is_wrot_sram;
	unsigned int mmsys_clk;
	unsigned int hrt_num;
	unsigned int ext_hrt_num;
	unsigned int is_display_idle;
	unsigned int is_dc;
	unsigned int hrt_magicnum; /* by resolution */
	unsigned int ext_hrt_magicnum; /* by resolution */
	unsigned int dst_width;
	unsigned int dst_height;
	unsigned int ext_dst_width;
	unsigned int ext_dst_height;
	unsigned int fps;
	unsigned int is_one_layer;
	unsigned int rdma_width;
	unsigned int rdma_height;
	unsigned int is_dual_pipe;
};

#define TOTAL_OVL_LAYER_NUM		(12)

struct disp_ddp_path_config {
	/* for ovl */
	bool ovl_dirty;
	bool ovl_partial_dirty;
	bool rdma_dirty;
	bool wdma_dirty;
	bool dst_dirty;
	int ovl_layer_dirty;	/*each bit represent one layer */
	int ovl_layer_scanned;	/*each bit reprsent one layer, used for ovl engines */
	int overlap_layer_num;
	struct OVL_CONFIG_STRUCT ovl_config[TOTAL_OVL_LAYER_NUM];
	struct disp_rect ovl_partial_roi;
	struct RDMA_CONFIG_STRUCT rdma_config;
	struct WDMA_CONFIG_STRUCT wdma_config;
	LCM_PARAMS dispif_config;
	unsigned int lcm_bpp;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int fps;
	struct golden_setting_context *p_golden_setting_context;
	void *path_handle;
	bool is_dual;
};

/* dpmgr_ioctl cmd definition */
enum DDP_IOCTL_NAME {
/* DSI operation */
	DDP_SWITCH_DSI_MODE = 0,
	DDP_STOP_VIDEO_MODE = 1,
	DDP_BACK_LIGHT = 2,
	DDP_SWITCH_LCM_MODE = 3,
	DDP_DPI_FACTORY_TEST = 4,
	DDP_DSI_IDLE_CLK_CLOSED = 5,
	DDP_DSI_IDLE_CLK_OPEN = 6,
	DDP_DSI_PORCH_CHANGE = 7,
	DDP_PHY_CLK_CHANGE = 8,
	DDP_ENTER_ULPS = 9,
	DDP_EXIT_ULPS = 10,
	DDP_RDMA_GOLDEN_SETTING = 11,
	DDP_OVL_GOLDEN_SETTING,
	DDP_PARTIAL_UPDATE,
	DDP_UPDATE_PLL_CLK_ONLY,
	DDP_DPI_FACTORY_RESET,
	DDP_SWITCH_SINGLE_DUAL_PIPE,
};

struct ddp_io_golden_setting_arg {
	enum dst_module_type dst_mod_type;
	int is_decouple_mode;
	unsigned int dst_w;
	unsigned int dst_h;
};

enum DISP_TUI_VERSION
{
	SINGLE_WINDOWS_TUI = 0,
	MULTI_WINDOWS_TUI,
};

char *ddp_get_module_name(enum DISP_MODULE_ENUM module);
char *ddp_get_reg_module_name(enum DISP_REG_ENUM reg_module);

enum DISP_MODULE_ENUM ddp_get_reg_module(enum DISP_REG_ENUM reg_module);

#endif
