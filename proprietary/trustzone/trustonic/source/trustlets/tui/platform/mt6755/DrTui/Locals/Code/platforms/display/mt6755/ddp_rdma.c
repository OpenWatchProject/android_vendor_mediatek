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

#define LOG_TAG "RDMA"
#include "ddp_log.h"
#include "ddp_dsi.h"
//#include "ddp_irq.h"

#include "ddp_reg.h"
#include "ddp_rdma.h"
#include "ddp_color_format.h"
#include "ddp_info.h"
#include "display_tui.h"

#if 0
enum RDMA_INPUT_FORMAT {
    RDMA_INPUT_FORMAT_BGR565    = 0,
    RDMA_INPUT_FORMAT_RGB888    = 1,
    RDMA_INPUT_FORMAT_RGBA8888  = 2,
    RDMA_INPUT_FORMAT_ARGB8888  = 3,
    RDMA_INPUT_FORMAT_VYUY      = 4,
    RDMA_INPUT_FORMAT_YVYU      = 5,

    RDMA_INPUT_FORMAT_RGB565    = 6,
    RDMA_INPUT_FORMAT_BGR888    = 7,
    RDMA_INPUT_FORMAT_BGRA8888  = 8,
    RDMA_INPUT_FORMAT_ABGR8888  = 9,
    RDMA_INPUT_FORMAT_UYVY      = 10,
    RDMA_INPUT_FORMAT_YUYV      = 11,

    RDMA_INPUT_FORMAT_UNKNOW    = 32,
};

#endif

#define MMSYS_CLK_LOW (0)
#define MMSYS_CLK_HIGH (1)
#define MMSYS_CLK_MEDIUM (2)

static unsigned int rdma_fps[RDMA_INSTANCES] = { 60, 60 };
static golden_setting_context *rdma_golden_setting;
#if 0
static enum RDMA_INPUT_FORMAT rdma_input_format_convert(DpColorFormat fmt)
{
   enum RDMA_INPUT_FORMAT rdma_fmt = RDMA_INPUT_FORMAT_RGB565;
   switch(fmt)
   {
       case eBGR565           :
           rdma_fmt = RDMA_INPUT_FORMAT_BGR565    ;  break;
       case eRGB888           :
           rdma_fmt = RDMA_INPUT_FORMAT_RGB888    ;  break;
       case eRGBA8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_RGBA8888  ;  break;
       case eARGB8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_ARGB8888  ;  break;
       case eVYUY             :
           rdma_fmt = RDMA_INPUT_FORMAT_VYUY      ;  break;
       case eYVYU             :
           rdma_fmt = RDMA_INPUT_FORMAT_YVYU      ;  break;
       case eRGB565           :
           rdma_fmt = RDMA_INPUT_FORMAT_RGB565    ;  break;
       case eBGR888           :
           rdma_fmt = RDMA_INPUT_FORMAT_BGR888    ;  break;
       case eBGRA8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_BGRA8888  ;  break;
       case eABGR8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_ABGR8888  ;  break;
       case eUYVY             :
           rdma_fmt = RDMA_INPUT_FORMAT_UYVY      ;  break;
       case eYUY2             :
           rdma_fmt = RDMA_INPUT_FORMAT_YUYV      ;  break;
       default:
           DDPERR("rdma_fmt_convert fmt=%d, rdma_fmt=%d \n", fmt, rdma_fmt);
   }
   return rdma_fmt;
}
#endif

#if 0
static unsigned int rdma_input_format_byte_swap(enum RDMA_INPUT_FORMAT inputFormat)
{
    int input_swap = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_YVYU:
        	input_swap = 1;
        	break;
        case RDMA_INPUT_FORMAT_RGB565:
        case RDMA_INPUT_FORMAT_BGR888:
        case RDMA_INPUT_FORMAT_BGRA8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
        case RDMA_INPUT_FORMAT_UYVY:
        case RDMA_INPUT_FORMAT_YUYV:
        	input_swap = 0;
        	break;
        default:
            DDPERR("unknow RDMA input format is %d\n", inputFormat);
            ASSERT(0);
    }
    return input_swap;
}
#endif


#if 0
static unsigned int rdma_input_format_bpp(enum RDMA_INPUT_FORMAT inputFormat)
{
    int bpp = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB565:
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_UYVY:
        case RDMA_INPUT_FORMAT_YVYU:
        case RDMA_INPUT_FORMAT_YUYV:
            bpp = 2;
            break;
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_BGR888:
            bpp = 3;
            break;
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_BGRA8888:
            bpp = 4;
            break;
        default:
            DDPERR("unknown RDMA input format = %d\n", inputFormat);
            ASSERT(0);
    }
    return  bpp;
}
#endif

#if 0
static unsigned int rdma_input_format_color_space(enum RDMA_INPUT_FORMAT inputFormat)
{
    int space = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB565:
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_BGR888:
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_BGRA8888:
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
            space = 0;
            break;
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_UYVY:
        case RDMA_INPUT_FORMAT_YVYU:
        case RDMA_INPUT_FORMAT_YUYV:
            space = 1;
            break;
        default:
            DDPERR("unknown RDMA input format = %d\n", inputFormat);
            ASSERT(0);
    }
    return space;
}


static unsigned int rdma_input_format_reg_value(enum RDMA_INPUT_FORMAT inputFormat)
{
    int reg_value = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB565:
            reg_value = 0x0;
        	break;
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_BGR888:
            reg_value = 0x1;
        	break;
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_BGRA8888:
            reg_value = 0x2;
        	break;
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
            reg_value = 0x3;
        	break;
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_UYVY:
          	reg_value = 0x4;
        	break;
        case RDMA_INPUT_FORMAT_YVYU:
        case RDMA_INPUT_FORMAT_YUYV:
        	reg_value = 0x5;
        	break;
        default:
            DDPERR("unknow RDMA input format is %d\n", inputFormat);
            ASSERT(0);
    }
    return reg_value;
}
#endif

#if 0
static char*  rdma_intput_format_name(enum RDMA_INPUT_FORMAT fmt, int swap)
{
    switch(fmt)
    {
        case RDMA_INPUT_FORMAT_BGR565   :
	    {
			return swap ? "eBGR565" : "eRGB565";
    	}
        case RDMA_INPUT_FORMAT_RGB565   :
	    {
			return "eRGB565";
    	}
        case RDMA_INPUT_FORMAT_RGB888   :
		{
            return swap ? "eRGB888" : "eBGR888";
		}
        case RDMA_INPUT_FORMAT_BGR888   :
		{
            return "eBGR888";
		}
        case RDMA_INPUT_FORMAT_RGBA8888 :
		{
			return swap ? "eRGBA888" : "eBGRA888";
		}
        case RDMA_INPUT_FORMAT_BGRA8888 :
		{
			return "eBGRA888";
		}
        case RDMA_INPUT_FORMAT_ARGB8888 :
		{
			return swap ? "eARGB8888" : "eABGR8888";
		}
        case RDMA_INPUT_FORMAT_ABGR8888 :
		{
			return "eABGR8888";
		}
        case RDMA_INPUT_FORMAT_VYUY :
		{
			return swap ? "eVYUY" : "eUYVY";
		}
        case RDMA_INPUT_FORMAT_UYVY :
		{
			return "eUYVY";
		}
        case RDMA_INPUT_FORMAT_YVYU :
		{
			return swap ? "eYVYU" : "eYUY2";
		}
        case RDMA_INPUT_FORMAT_YUYV :
		{
			return "eYUY2";
		}
        default:
		{
			DDPERR("rdma_intput_format_name unknow fmt=%d, swap=%d \n", fmt, swap);
			break;
    	}
    }
    return "unknow";
}

#endif

static unsigned int rdma_index(DISP_MODULE_ENUM module)
{
	int idx = 0;
	switch (module) {
	case DISP_MODULE_RDMA0:
		idx = 0;
		break;
	case DISP_MODULE_RDMA1:
		idx = 1;
		break;
	default:
		DDPERR("invalid rdma module=%d\n", module);	/* invalid module */
		ASSERT(0);
	}
	return idx;
}

#if 0
int rdma_enable_irq(DISP_MODULE_ENUM module, void *handle, DDP_IRQ_LEVEL irq_level)
{
	unsigned int idx = rdma_index(module);
	ASSERT(idx <= RDMA_INSTANCES);

	switch (irq_level) {
	case DDP_IRQ_LEVEL_ALL:
		DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0x1E);
		break;
	case DDP_IRQ_LEVEL_ERROR:
		DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0x18);
		break;
	case DDP_IRQ_LEVEL_NONE:
		DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0x0);
		break;
	default:
		break;
	}

	return 0;
}
#endif


unsigned int rdma_disable_irq_backup(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int idx = rdma_index(module);
    ASSERT(idx <= 2);
	unsigned int old_val;

	old_val = DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE);
	DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, 0x0);
	DDPDBG("%s, rdma%d,irq_enable:old_val =%d\n", __func__, idx, old_val);

    return old_val;
}

int rdma_irq_restore(DISP_MODULE_ENUM module, void *handle, unsigned int irq_val)
{
	unsigned int idx = rdma_index(module);
    ASSERT(idx <= 2);
	DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, irq_val);

    return 0;
}

#if 0

int rdma_start(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int idx = rdma_index(module);
	unsigned int regval;
	ASSERT(idx <= RDMA_INSTANCES);

	regval = REG_FLD_VAL(INT_STATUS_FLD_REG_UPDATE_INT_FLAG, 0) |
	    REG_FLD_VAL(INT_STATUS_FLD_FRAME_START_INT_FLAG, 1) |
	    REG_FLD_VAL(INT_STATUS_FLD_FRAME_END_INT_FLAG, 1) |
	    REG_FLD_VAL(INT_STATUS_FLD_EOF_ABNORMAL_INT_FLAG, 1) |
	    REG_FLD_VAL(INT_STATUS_FLD_FIFO_UNDERFLOW_INT_FLAG, 1) |
	    REG_FLD_VAL(INT_STATUS_FLD_TARGET_LINE_INT_FLAG, 0) |
	    REG_FLD_VAL(INT_STATUS_FLD_FIFO_EMPTY_INT_FLAG, 0);

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, regval);
	DISP_REG_SET_FIELD(handle, GLOBAL_CON_FLD_ENGINE_EN,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 1);

	return 0;
}


int rdma_stop(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int idx = rdma_index(module);
	ASSERT(idx <= RDMA_INSTANCES);

	DISP_REG_SET_FIELD(handle, GLOBAL_CON_FLD_ENGINE_EN,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 0);
	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0);
	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS, 0);
	return 0;
}
#endif


#if 0
static void udelay(int us)
{
	ASSERT(0);
}
#endif

#if 0

int rdma_reset(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int delay_cnt = 0;
	int ret = 0;
	unsigned int idx = rdma_index(module);

	ASSERT(idx <= RDMA_INSTANCES);

	DISP_REG_SET_FIELD(handle, GLOBAL_CON_FLD_SOFT_RESET,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 1);
	while ((DISP_REG_GET(idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON) & 0x700) ==
	       0x100) {
		delay_cnt++;
		udelay(10);
		if (delay_cnt > 10000) {
			ret = -1;
			DDPERR("rdma%d_reset timeout, stage 1! DISP_REG_RDMA_GLOBAL_CON=0x%x\n",
			       idx,
			       DISP_REG_GET(idx * DISP_RDMA_INDEX_OFFSET +
					    DISP_REG_RDMA_GLOBAL_CON));
			break;
		}
	}
	DISP_REG_SET_FIELD(handle, GLOBAL_CON_FLD_SOFT_RESET,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 0);
	delay_cnt = 0;
	while ((DISP_REG_GET(idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON) & 0x700) !=
	       0x100) {
		delay_cnt++;
		udelay(10);
		if (delay_cnt > 10000) {
			ret = -1;
			DDPERR("rdma%d_reset timeout, stage 2! DISP_REG_RDMA_GLOBAL_CON=0x%x\n",
			       idx,
			       DISP_REG_GET(idx * DISP_RDMA_INDEX_OFFSET +
					    DISP_REG_RDMA_GLOBAL_CON));
			break;
		}
	}
	return ret;
}

#endif

int rdma_reg_backup(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned long offset = idx*DISP_RDMA_INDEX_OFFSET;

	disp_tui_reg_backup(offset+DISP_REG_RDMA_GLOBAL_CON);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_MEM_CON);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_MEM_START_ADDR);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_MEM_SRC_PITCH);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_INT_ENABLE);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_SIZE_CON_0);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_SIZE_CON_1);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_FIFO_CON);

	return 0;
}

/* set ultra registers */
void rdma_set_ultra_l(unsigned int idx, unsigned int bpp, void *handle, golden_setting_context *p_golden_setting)
{

	/* rdma golden setting variables */
	unsigned int mmsysclk = 286;
	unsigned int is_wrot_sram = 0;
	unsigned int fifo_mode = 1;

	unsigned int ultra_low_us = 4;
	unsigned int ultra_high_us = 6;
	unsigned int preultra_low_us = ultra_high_us;
	unsigned int preultra_high_us = 7;

	unsigned long long fill_rate = 0;
	unsigned long long consume_rate = 0;

	unsigned int fifo_valid_size = 512;

	/* working variables */
	unsigned int ultra_low;
	unsigned int preultra_low;
	unsigned int preultra_high;
	unsigned int ultra_high;

	unsigned int issue_req_threshold;
	unsigned int output_valid_fifo_threshold;

	unsigned int sodi_threshold_high;
	unsigned int sodi_threshold_low;
	unsigned int dvfs_threshold_high;
	unsigned int dvfs_threshold_low;

	golden_setting_context temp_golden_setting;
	unsigned int frame_rate;
	long long temp;
	DDPMSG("%s, start1!\n", (__func__));

	if (p_golden_setting == NULL) {
		DDPDUMP("[disp_lowpower]p_golden_setting is NULL\n");

		/* default setting */
		temp_golden_setting.fps = 60;
		temp_golden_setting.is_dc = 0;
		temp_golden_setting.is_display_idle = 0;
		temp_golden_setting.is_wrot_sram = 0;
		temp_golden_setting.mmsys_clk = MMSYS_CLK_MEDIUM;/* 286: defalut ; 182: low ; 364: high */

		/* primary_display */
		temp_golden_setting.dst_width	= 1080;
		temp_golden_setting.dst_height	= 1920;
		temp_golden_setting.hrt_magicnum = 2;

		/* set hrtnum max */
		temp_golden_setting.hrt_num = temp_golden_setting.hrt_magicnum + 1;

		/* fifo mode : 0/1/2 */
		if (temp_golden_setting.is_display_idle)
			temp_golden_setting.fifo_mode = 0;
		else if (temp_golden_setting.hrt_num > temp_golden_setting.hrt_magicnum)
			temp_golden_setting.fifo_mode = 2;
		else
			temp_golden_setting.fifo_mode = 1;

		/* ext_display */
		temp_golden_setting.ext_dst_width = temp_golden_setting.dst_width;
		temp_golden_setting.ext_dst_height = temp_golden_setting.dst_height;
		temp_golden_setting.ext_hrt_magicnum = temp_golden_setting.hrt_magicnum;
		temp_golden_setting.ext_hrt_num = temp_golden_setting.hrt_num;

		rdma_golden_setting = &temp_golden_setting;
	} else
		rdma_golden_setting = p_golden_setting;

	frame_rate = rdma_golden_setting->fps;
	if (idx == 1) {
		/* hardcode bpp & frame_rate for rdma1 */
		bpp = 24;
		frame_rate = 60;
	}

	/* get fifo parameters */
	switch (rdma_golden_setting->mmsys_clk) {
	case MMSYS_CLK_LOW:
		mmsysclk = 182;
		break;
	case MMSYS_CLK_MEDIUM:
		mmsysclk = 286;
		break;
	case MMSYS_CLK_HIGH:
		mmsysclk = 364;
		break;
	}

	is_wrot_sram = rdma_golden_setting->is_wrot_sram;
	fifo_mode = rdma_golden_setting->fifo_mode;

	if (idx == 0) {
		if (fifo_mode == 0) {
			ultra_low_us = 1;
			ultra_high_us = 3;
			preultra_low_us = ultra_high_us;
			preultra_high_us = 4;
		} else if (fifo_mode == 1) {
			ultra_low_us = 4;
			ultra_high_us = 6;
			preultra_low_us = ultra_high_us;
			preultra_high_us = 7;
		} else {
			ultra_low_us = 8;
			ultra_high_us = 12;
			preultra_low_us = ultra_high_us;
			preultra_high_us = 15;
		}
	} else {
		if (rdma_golden_setting->ext_dst_width  == 1920 && rdma_golden_setting->ext_dst_height == 1080) {
			ultra_low_us = 4;
			ultra_high_us = 6;
			preultra_low_us = ultra_high_us;
			preultra_high_us = 7;
		} else if ((rdma_golden_setting->ext_dst_width == 1280 && rdma_golden_setting->ext_dst_height == 720)
		|| (rdma_golden_setting->ext_dst_width == 720 && rdma_golden_setting->ext_dst_height == 480)) {
			if (rdma_golden_setting->ext_hrt_num > rdma_golden_setting->hrt_magicnum) {
				ultra_low_us = 8;
				ultra_high_us = 12;
				preultra_low_us = ultra_high_us;
				preultra_high_us = 15;
			} else {
				ultra_low_us = 4;
				ultra_high_us = 6;
				preultra_low_us = ultra_high_us;
				preultra_high_us = 7;
			}
		} else {
				ultra_low_us = 8;
				ultra_high_us = 12;
				preultra_low_us = ultra_high_us;
				preultra_high_us = 15;
		}


	}

	if (rdma_golden_setting->is_dc)
		fill_rate = 960*mmsysclk; /* FIFO depth / us  */
	else
		fill_rate = 960*mmsysclk*3/16; /* FIFO depth / us  */

	if (idx == 0)
		consume_rate = rdma_golden_setting->dst_width*rdma_golden_setting->dst_height*frame_rate*bpp/(8*1000);
	else
		consume_rate = rdma_golden_setting->ext_dst_width
		*rdma_golden_setting->ext_dst_height*frame_rate*bpp/(8*1000);
	consume_rate = 1200*consume_rate/(16*1000);

	preultra_low = preultra_low_us * consume_rate;
	if (preultra_low%1000)
		preultra_low = preultra_low / 1000 + 1;
	else
		preultra_low = preultra_low / 1000;


	preultra_high = preultra_high_us * consume_rate;
	if (preultra_high%1000)
		preultra_high = preultra_high / 1000 + 1;
	else
		preultra_high = preultra_high / 1000;


	ultra_low = ultra_low_us * consume_rate;
	if (ultra_low%1000)
		ultra_low = ultra_low / 1000 + 1;
	else
		ultra_low = ultra_low / 1000;

	ultra_high = preultra_low;
	if (idx == 0) {
		/* only rdma0 can share sram */
		if (is_wrot_sram)
			fifo_valid_size = 2048;
		else
			fifo_valid_size = 512;
	} else
		fifo_valid_size = 256;


	issue_req_threshold = (fifo_valid_size - preultra_low) < 255  ? (fifo_valid_size - preultra_low) : 255;
	output_valid_fifo_threshold = preultra_low;

	temp = fifo_valid_size - 1200 * (fill_rate - consume_rate)/1000000;
	if (temp < 0)
		sodi_threshold_high = preultra_high;
	else
		sodi_threshold_high = preultra_high > (fifo_valid_size - 1200 * (fill_rate - consume_rate)/1000000)
			? preultra_high : (fifo_valid_size - 1200 * (fill_rate - consume_rate)/1000000);


	sodi_threshold_low = (ultra_low_us*10 + 4) * consume_rate / 10;
	if (sodi_threshold_low % 1000)
		sodi_threshold_low = sodi_threshold_low/1000 + 1;
	else
		sodi_threshold_low = sodi_threshold_low/1000;


	dvfs_threshold_low = preultra_low;
	dvfs_threshold_high = preultra_low+1;


	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_GMC_SETTING_0,
		preultra_low | (preultra_high << 16));

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_GMC_SETTING_1,
		ultra_low | (ultra_high << 16));

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_GMC_SETTING_2,
		issue_req_threshold);

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_FIFO_CON,
		output_valid_fifo_threshold | (fifo_valid_size << 16));

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_THRESHOLD_FOR_SODI,
		sodi_threshold_low | (sodi_threshold_high << 16));

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_THRESHOLD_FOR_DVFS,
		dvfs_threshold_low | (dvfs_threshold_high << 16));

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SRAM_SEL,
		is_wrot_sram);

	DISP_REG_SET_FIELD(handle, FIFO_CON_FLD_FIFO_UNDERFLOW_EN,
		idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_FIFO_CON, 1);
/*
	if (idx == 0)
		rdma_dump_golden_setting_context(DISP_MODULE_RDMA0);
	else
		rdma_dump_golden_setting_context(DISP_MODULE_RDMA1);
*/
	if (rdma_golden_setting->dst_width == 0 || rdma_golden_setting->dst_height == 0
		|| bpp == 0 || frame_rate == 0) {
		DDPDUMP("==RDMA Golden Setting Value=============\n");

		DDPDUMP("width		= %d\n", rdma_golden_setting->dst_width);
		DDPDUMP("height		= %d\n", rdma_golden_setting->dst_height);
		DDPDUMP("bpp		= %d\n", bpp);
		DDPDUMP("frame_rate	= %d\n", frame_rate);

		DDPDUMP("fill_rate	= %lld\n", fill_rate);
		DDPDUMP("consume_rate	= %lld\n", consume_rate);
		DDPDUMP("ultra_low_us	= %d\n", ultra_low_us);
		DDPDUMP("ultra_high_us	= %d\n", ultra_high_us);
		DDPDUMP("preultra_high_us= %d\n", preultra_high_us);

		DDPDUMP("preultra_low	= %d\n", preultra_low);
		DDPDUMP("preultra_high	= %d\n", preultra_high);
		DDPDUMP("ultra_low	= %d\n", ultra_low);
		DDPDUMP("issue_req_threshold		= %d\n", issue_req_threshold);
		DDPDUMP("output_valid_fifo_threshold	= %d\n", output_valid_fifo_threshold);
		DDPDUMP("sodi_threshold_low	= %d\n", sodi_threshold_low);
		DDPDUMP("sodi_threshold_high	= %d\n", sodi_threshold_high);
		DDPDUMP("dvfs_threshold_low	= %d\n", dvfs_threshold_low);
		DDPDUMP("dvfs_threshold_high	= %d\n", dvfs_threshold_high);
	}

}




int rdma_config(DISP_MODULE_ENUM module,
		       enum RDMA_MODE mode,
		       unsigned long address,
		       enum UNIFIED_COLOR_FMT inFormat,
		       unsigned pitch,
		       unsigned width,
		       unsigned height,
		       unsigned ufoe_enable,
		       DISP_BUFFER_TYPE sec,
		       unsigned int yuv_range, struct rdma_bg_ctrl_t *bg_ctrl, void *handle,
		       golden_setting_context *p_golden_setting, unsigned int bpp)
{

	unsigned int output_is_yuv = 0;
	unsigned int input_is_yuv = !UFMT_GET_RGB(inFormat);
	unsigned int input_swap = UFMT_GET_SWAP(inFormat);
	unsigned int input_format_reg = UFMT_GET_FORMAT(inFormat);
	unsigned int idx = rdma_index(module);
	unsigned int color_matrix;
	unsigned int regval;

	DDPMSG("%s, dxs rdma_config()start1!\n", (__func__));

	DDPMSG("dxs RDMAConfig idx %d, mode %d, address 0x%lx, pitch %u, width %u, height %u,sec%d\n",
	     idx, mode, address, pitch, width, height, sec);
	DDPMSG("%s, dxs rdma_config()start2!\n", (__func__));

	/*DDPMSG("RDMAConfig idx %d, mode %d, address 0x%lx, inputformat %s, pitch %u, width %u, height %u,sec%d\n",
	    idx, mode, address, unified_color_fmt_name(inFormat), pitch, width, height, sec);*/
	ASSERT(idx <= RDMA_INSTANCES);
	if ((width > RDMA_MAX_WIDTH) || (height > RDMA_MAX_HEIGHT))
		DDPERR("RDMA input overflow, w=%d, h=%d, max_w=%d, max_h=%d\n", width, height,
		       RDMA_MAX_WIDTH, RDMA_MAX_HEIGHT);

	if (input_is_yuv == 1 && output_is_yuv == 0) {
		switch (yuv_range) {
		case 0:
			color_matrix = 4;
			break;	/* BT601_full */
		case 1:
			color_matrix = 6;
			break;	/* BT601 */
		case 2:
			color_matrix = 7;
			break;	/* BT709 */
		default:
			DDPERR("%s,un-recognized yuv_range=%d!\n", __func__, yuv_range);
			color_matrix = 4;
		}
		DDPMSG("%s, dxs rdma_config()start3!\n", (__func__));

		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_ENABLE,
				   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 1);
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL,
				   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0,
				   color_matrix);
	} else if (input_is_yuv == 0 && output_is_yuv == 1) {
		color_matrix = 0x2;	/* 0x0010, RGB_TO_BT601 */
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_ENABLE,
				   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 1);
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL,
				   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0,
				   color_matrix);
	} else {
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_ENABLE,
				   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0);
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL,
				   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0);
	}
	DDPMSG("%s, dxs rdma_config()start4!\n", (__func__));

	DISP_REG_SET_FIELD(handle, GLOBAL_CON_FLD_MODE_SEL,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, mode);
	/* FORMAT & SWAP only works when RDMA memory mode, set both to 0 when RDMA direct link mode. */
	DISP_REG_SET_FIELD(handle, MEM_CON_FLD_MEM_MODE_INPUT_FORMAT,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_CON,
			   ((mode == RDMA_MODE_DIRECT_LINK) ? 0 : input_format_reg & 0xf));
	DISP_REG_SET_FIELD(handle, MEM_CON_FLD_MEM_MODE_INPUT_SWAP,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_CON,
			   ((mode == RDMA_MODE_DIRECT_LINK) ? 0 : input_swap));

	if (sec != DISP_SECURE_BUFFER) {
		DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_START_ADDR,
			     address);
	} else {
		ASSERT(0);
	}
	DDPMSG("%s, dxs rdma_config()start5!\n", (__func__));

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_MEM_SRC_PITCH, pitch);
	/* DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, 0x3F); */
	DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, width);
	DISP_REG_SET_FIELD(handle, SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT,
			   idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_1, height);

	/* rdma bg control */
#if 0
	regval = REG_FLD_VAL(RDMA_BG_CON_0_LEFT, bg_ctrl->left);
	regval |= REG_FLD_VAL(RDMA_BG_CON_0_RIGHT, bg_ctrl->right);
	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_BG_CON_0, regval);

	regval = REG_FLD_VAL(RDMA_BG_CON_1_TOP, bg_ctrl->top);
	regval |= REG_FLD_VAL(RDMA_BG_CON_1_BOTTOM, bg_ctrl->bottom);
	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_BG_CON_1, regval);
#endif
	DDPMSG("%s, dxs rdma_config()start6!\n", (__func__));

	rdma_set_ultra_l(idx, bpp, handle, p_golden_setting);
	DDPMSG("%s, dxs rdma_config()start7!\n", (__func__));

	return 0;
}

int rdma_is_mem_mode(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
    enum RDMA_MODE mode;

    ASSERT(idx <= 2);
    mode = DISP_REG_GET_FIELD(GLOBAL_CON_FLD_MODE_SEL, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON);
	return mode == RDMA_MODE_MEMORY;

}

/*** !!   rdma wait frame done by polling / interrupt / cmdq_event      (   +  )    !!***/

#define RDMA_INT_FRAME_START_BIT 1
#define RDMA_INT_FRAME_DONE_BIT 2
#define RDMA_INT_FRAME_ABNORMAL_BIT 3
#define RDMA_INT_FRAME_UNDERFLOW_BIT 4

int rdma_wait_frame_done_by_polling(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	/* clear interrupt first*/
    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, 0x0);

	while(1) {

		regval = DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS);
	    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, ~regval);

		if(regval & (1<<RDMA_INT_FRAME_ABNORMAL_BIT)) {
			DDPMSG("error: rdma frame abnormal int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_UNDERFLOW_BIT)) {
			DDPMSG("error: rdma frame underflow int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_DONE_BIT)) {
			return 0;
		}
	}
}



static int rdma_irq_num[3] = {222, 223, 224};
static int rdma_irq_attached[3] = {0, 0, 0};

int rdma_irq_attach(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(!rdma_irq_attached[idx]) {
		ret = drApiIntrAttach(irq_num, INTR_MODE_LOW_LEVEL);
		if(ret != DRAPI_OK){
			DDPMSG("error to attach irq %d flag=%d, ret=%d\n",
				irq_num, INTR_MODE_LOW_LEVEL, ret);
			return -1;
		}
		rdma_irq_attached[idx] = 1;
	}
	DDPDBG("%s, rdma%d,to attach irq %d flag=%d\n", __func__, idx, irq_num, INTR_MODE_LOW_LEVEL);
	return 0;
}

int rdma_irq_dettach(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(rdma_irq_attached[idx]) {
		ret = drApiIntrDetach(irq_num);
		if(ret != DRAPI_OK) {
			DDPMSG("error to detach irq %d ret=%d\n",
				irq_num, ret);
			return -1;
		}
		rdma_irq_attached[idx] = 0;
	}
	return 0;
}

int rdma_wait_frame_done_by_interrupt(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(!rdma_irq_attached[idx]) {
		ret = rdma_irq_attach(module);
		if(ret)
			goto err1;
	}

    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, 0x0);
	rdma_irq_restore(module, NULL, 0x1C);

	while(1) {
		ret = drApiWaitForIntr(irq_num, TIME_INFINITE, NULL);
		if(ret != DRAPI_OK){
			DDPMSG("error to wait irq %d time=%d, ret=%d\n",
				irq_num, TIME_INFINITE, ret);
			goto err1;
		}

		regval = DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS);
	    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, ~regval);

		if(regval & (1<<RDMA_INT_FRAME_ABNORMAL_BIT)) {
			DDPMSG("error: rdma frame abnormal int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_UNDERFLOW_BIT)) {
			DDPMSG("error: rdma frame underflow int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_DONE_BIT)) {
			rdma_irq_restore(module, NULL, 0);
			return 0;
		}
	}

	return 0;

err1:
	rdma_irq_restore(module, NULL, 0);
	return rdma_wait_frame_done_by_polling(module);

}

CMDQ_EVENT_ENUM rdma_get_EOF_cmdq_event(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	CMDQ_EVENT_ENUM event;

	if(idx == 0)
		event = CMDQ_EVENT_DISP_RDMA0_EOF;
	else if(idx == 1)
		event = CMDQ_EVENT_DISP_RDMA1_EOF;
	else if(idx == 2)
		event = CMDQ_EVENT_DISP_RDMA2_EOF;
	else
		ASSERT(0);

	return event;
}

int rdma_wait_frame_done_by_cmdq(DISP_MODULE_ENUM module, cmdqRecHandle cmdq_handle,
			int clear_before_wait, int clear_after_wait)
{
	CMDQ_EVENT_ENUM event;
	int ret = 0;
	ASSERT(cmdq_handle != NULL);
	event = rdma_get_EOF_cmdq_event(module);

	if(clear_before_wait)
		ret |= cmdqRecClearEventToken(cmdq_handle, event);

	if(clear_after_wait)
		ret |= cmdqRecWait(cmdq_handle, event);
	else
		ret |= cmdqRecWaitNoClear(cmdq_handle, event);

	if(ret)
		DDPERR("%s error ret=%d\n", __func__);

	return ret;
}

/*** !!     rdma wait frame done by polling / interrupt / cmdq_event     (   -  )    !!***/


#if 0

void rdma_set_target_line(DISP_MODULE_ENUM module, unsigned int line, void *handle)
{
	unsigned int idx = rdma_index(module);

	DISP_REG_SET(handle, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_TARGET_LINE, line);
}



static int rdma_clock_on(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int idx = rdma_index(module);
#ifdef ENABLE_CLK_MGR
#ifdef CONFIG_MTK_CLKMGR
	if (idx == 0)
		enable_clock(MT_CG_DISP0_DISP_RDMA0, "RDMA0");
	else
		enable_clock(MT_CG_DISP0_DISP_RDMA1, "RDMA1");
#else
	if (idx == 0)
		ddp_clk_enable(DISP0_DISP_RDMA0);
	else
		ddp_clk_enable(DISP0_DISP_RDMA1);
#endif
#endif
	DDPMSG("rdma_%d_clock_on CG 0x%x\n", idx, DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
	return 0;
}

static int rdma_clock_off(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int idx = rdma_index(module);
#ifdef ENABLE_CLK_MGR
#ifdef CONFIG_MTK_CLKMGR
	if (idx == 0)
		disable_clock(MT_CG_DISP0_DISP_RDMA0, "RDMA0");
	else
		disable_clock(MT_CG_DISP0_DISP_RDMA1, "RDMA1");
#else
	if (idx == 0)
		ddp_clk_disable(DISP0_DISP_RDMA0);
	else
		ddp_clk_disable(DISP0_DISP_RDMA1);

#endif
#endif
	DDPMSG("rdma_%d_clock_off CG 0x%x\n", idx, DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
	return 0;
}



static int rdma_init(DISP_MODULE_ENUM module, void *handle)
{
	return rdma_clock_on(module, handle);
}

static int rdma_deinit(DISP_MODULE_ENUM module, void *handle)
{
	return rdma_clock_off(module, handle);
}

#endif


void rdma_dump_golden_setting_context(DISP_MODULE_ENUM module)
{
	if (rdma_golden_setting) {
		DDPDUMP("==RDMA Golden Setting Context=============\n");
		DDPDUMP("fifo_mode	= %d\n", rdma_golden_setting->fifo_mode);
		DDPDUMP("hrt_num	= %d\n", rdma_golden_setting->hrt_num);
		DDPDUMP("is_display_idle	= %d\n", rdma_golden_setting->is_display_idle);
		DDPDUMP("is_wrot_sram	= %d\n", rdma_golden_setting->is_wrot_sram);
		DDPDUMP("is_dc		= %d\n", rdma_golden_setting->is_dc);
		DDPDUMP("mmsys_clk	= %d\n", rdma_golden_setting->mmsys_clk);
		DDPDUMP("fps		= %d\n", rdma_golden_setting->fps);
	}
}

void rdma_dump_reg(DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	DDPDUMP("== DISP RDMA%d REGS ==\n", idx);
	DDPDUMP("(0x000)R_INTEN       =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x004)R_INTS        =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_INT_STATUS + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x010)R_CON         =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x014)R_SIZE0       =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x018)R_SIZE1       =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x01c)R_TAR_LINE    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_TARGET_LINE + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x024)R_M_CON       =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_CON + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0xf00)R_M_S_ADDR    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x02c)R_M_SRC_PITCH =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x030)R_M_GMC_SET0  =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x034)R_M_GMC_SET1  =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x038)R_M_SLOW_CON  =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x03c)R_M_GMC_SET2  =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_2 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x040)R_FIFO_CON    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_FIFO_CON + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x044)R_FIFO_LOG    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x078)R_PRE_ADD0    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_0 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x07c)R_PRE_ADD1    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_1 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x080)R_PRE_ADD2    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_2 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x084)R_POST_ADD0   =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_POST_ADD_0 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x088)R_POST_ADD1   =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_POST_ADD_1 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x08c)R_POST_ADD2   =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_POST_ADD_2 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x090)R_DUMMY       =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_DUMMY + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x094)R_OUT_SEL     =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x094)R_M_START     =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0a0)R_BG_CON_0    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_BG_CON_0 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0a4)R_BG_CON_1    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_BG_CON_1 + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0a8)R_FOR_SODI    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_THRESHOLD_FOR_SODI + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0ac)R_FOR_DVFS    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_THRESHOLD_FOR_DVFS + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0b0)R_FOR_SRAM    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SRAM_SEL + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0f0)R_IN_PXL_CNT  =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_IN_P_CNT + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0f4)R_IN_LINE_CNT =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_IN_LINE_CNT + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0f8)R_OUT_PXL_CNT =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_OUT_P_CNT + DISP_RDMA_INDEX_OFFSET * idx));
	DDPDUMP("(0x0fc)R_OUT_LINE_CNT=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_OUT_LINE_CNT + DISP_RDMA_INDEX_OFFSET * idx));

	return;
}


void rdma_dump_analysis(DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	unsigned int global_ctrl =
	    DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON + DISP_RDMA_INDEX_OFFSET * idx);
	unsigned int bg0 = DISP_REG_GET(idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_BG_CON_0);
	unsigned int bg1 = DISP_REG_GET(idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_BG_CON_1);

	DDPDUMP("==DISP RDMA%d ANALYSIS==\n", idx);

	DDPDUMP("rdma %d : en=%d,memory_mode=%d,smi_busy=%d,w=%d,h=%d,pitch=%d,addr=0x%x,fmt=%s,fifo_min=%d \n",
		idx, REG_FLD_VAL_GET(GLOBAL_CON_FLD_ENGINE_EN, global_ctrl),
		REG_FLD_VAL_GET(GLOBAL_CON_FLD_MODE_SEL, global_ctrl),
		REG_FLD_VAL_GET(GLOBAL_CON_FLD_SMI_BUSY, global_ctrl),
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + DISP_RDMA_INDEX_OFFSET * idx) & 0xfff,
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1 + DISP_RDMA_INDEX_OFFSET * idx) & 0xfffff,
		DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH + DISP_RDMA_INDEX_OFFSET * idx),
		DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + DISP_RDMA_INDEX_OFFSET * idx),
		unified_color_fmt_name(display_fmt_reg_to_unified_fmt
				    ((DISP_REG_GET
				      (DISP_REG_RDMA_MEM_CON +
				       DISP_RDMA_INDEX_OFFSET * idx) >> 4) & 0xf,
				     (DISP_REG_GET
				      (DISP_REG_RDMA_MEM_CON +
				       DISP_RDMA_INDEX_OFFSET * idx) >> 8) & 0x1)),
		DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG + DISP_RDMA_INDEX_OFFSET * idx)
		);

	DDPDUMP("in_p=%d,in_l=%d,out_p=%d,out_l=%d,bg(t%d,b%d,l%d,r%d)\n",
		DISP_REG_GET(DISP_REG_RDMA_IN_P_CNT + DISP_RDMA_INDEX_OFFSET * idx),
		DISP_REG_GET(DISP_REG_RDMA_IN_LINE_CNT + DISP_RDMA_INDEX_OFFSET * idx),
		DISP_REG_GET(DISP_REG_RDMA_OUT_P_CNT + DISP_RDMA_INDEX_OFFSET * idx),
		DISP_REG_GET(DISP_REG_RDMA_OUT_LINE_CNT + DISP_RDMA_INDEX_OFFSET * idx),
		REG_FLD_VAL_GET(RDMA_BG_CON_1_TOP, bg1),
		REG_FLD_VAL_GET(RDMA_BG_CON_1_BOTTOM, bg1),
		REG_FLD_VAL_GET(RDMA_BG_CON_0_LEFT, bg0),
		REG_FLD_VAL_GET(RDMA_BG_CON_0_RIGHT, bg0)
	    );

	return;
}

static int rdma_dump(DISP_MODULE_ENUM module, int level)
{
	rdma_dump_analysis(module);
	rdma_dump_reg(module);
	rdma_dump_golden_setting_context(module);

	return 0;
}


#if 0
void rdma_get_address(DISP_MODULE_ENUM module, unsigned long *addr)
{
	unsigned int idx = rdma_index(module);
	*addr = DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + DISP_RDMA_INDEX_OFFSET * idx);
	return;
}
#endif

void rdma_get_info(int idx, RDMA_BASIC_STRUCT *info)
{
	RDMA_BASIC_STRUCT *p = info;
	p->addr = DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + DISP_RDMA_INDEX_OFFSET * idx);
	p->src_w = DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + DISP_RDMA_INDEX_OFFSET * idx) & 0xfff;
	p->src_h = DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1 + DISP_RDMA_INDEX_OFFSET * idx) & 0xfffff,
	    p->bpp = UFMT_GET_bpp(display_fmt_reg_to_unified_fmt((DISP_REG_GET
								  (DISP_REG_RDMA_MEM_CON +
								   DISP_RDMA_INDEX_OFFSET *
								   idx) >> 4) & 0xf,
								 (DISP_REG_GET
								  (DISP_REG_RDMA_MEM_CON +
								   DISP_RDMA_INDEX_OFFSET *
								   idx) >> 8) & 0x1)) / 8;

	return;
}

#if 0

static inline enum RDMA_MODE rdma_config_mode(unsigned long address)
{
	return address ? RDMA_MODE_MEMORY : RDMA_MODE_DIRECT_LINK;
}


static int do_rdma_config_l(DISP_MODULE_ENUM module, disp_ddp_path_config *pConfig, void *handle)
{
	RDMA_CONFIG_STRUCT *r_config = &pConfig->rdma_config;
	enum RDMA_MODE mode = rdma_config_mode(r_config->address);
	LCM_PARAMS *lcm_param = &(pConfig->dispif_config);
	unsigned int width = pConfig->dst_dirty ? pConfig->dst_w : r_config->width;
	unsigned int height = pConfig->dst_dirty ? pConfig->dst_h : r_config->height;
	golden_setting_context *p_golden_setting = pConfig->p_golden_setting_context;

	if (pConfig->fps)
		rdma_fps[rdma_index(module)] = pConfig->fps / 100;

	if (mode == RDMA_MODE_DIRECT_LINK && r_config->security != DISP_NORMAL_BUFFER)
		DDPERR("%s: rdma directlink BUT is sec ??!!\n", __func__);

	if (mode == RDMA_MODE_DIRECT_LINK) {
		pConfig->rdma_config.bg_ctrl.top = 0;
		pConfig->rdma_config.bg_ctrl.bottom = 0;
		pConfig->rdma_config.bg_ctrl.left = 0;
		pConfig->rdma_config.bg_ctrl.right = 0;
	} else if (mode == RDMA_MODE_MEMORY) {
		pConfig->rdma_config.bg_ctrl.top = r_config->dst_y;
		pConfig->rdma_config.bg_ctrl.bottom = r_config->dst_h -
			r_config->dst_y - height;
		pConfig->rdma_config.bg_ctrl.left = r_config->dst_x;
		pConfig->rdma_config.bg_ctrl.right = r_config->dst_w -
			r_config->dst_x - width;
	}
	DDPDBG("top=%d,bottom=%d,left=%d,right=%d,r.dst_x=%d,r.dst_y=%d,r.dst_w=%d,r.dst_h=%d,width=%d,height=%d\n",
		pConfig->rdma_config.bg_ctrl.top, pConfig->rdma_config.bg_ctrl.bottom,
		pConfig->rdma_config.bg_ctrl.left, pConfig->rdma_config.bg_ctrl.right,
		r_config->dst_x, r_config->dst_y, r_config->dst_w,
		r_config->dst_h, width, height);
	rdma_config(module,
		    mode,
		    (mode == RDMA_MODE_DIRECT_LINK) ? 0 : r_config->address,
		    (mode == RDMA_MODE_DIRECT_LINK) ? UFMT_RGB888 : r_config->inputFormat,
		    (mode == RDMA_MODE_DIRECT_LINK) ? 0 : r_config->pitch,
		    width,
		    height,
		    lcm_param->dsi.ufoe_enable,
		    r_config->security, r_config->yuv_range, &(r_config->bg_ctrl), handle, p_golden_setting, pConfig->lcm_bpp);

	return 0;
}
#endif


#if 0
static int setup_rdma_sec(DISP_MODULE_ENUM module, disp_ddp_path_config *pConfig, void *handle)
{
	static int rdma_is_sec[2];
	CMDQ_ENG_ENUM cmdq_engine;
	int rdma_idx = rdma_index(module);
	DISP_BUFFER_TYPE security = pConfig->rdma_config.security;
	enum RDMA_MODE mode = rdma_config_mode(pConfig->rdma_config.address);

	cmdq_engine = rdma_idx == 0 ? CMDQ_ENG_DISP_RDMA0 : CMDQ_ENG_DISP_RDMA1;

	if (!handle) {
		DDPMSG("[SVP] bypass rdma sec setting sec=%d,handle=NULL\n", security);
		return 0;
	}
	/* sec setting make sence only in memory mode ! */
	if (mode == RDMA_MODE_MEMORY) {
		if (security == DISP_SECURE_BUFFER) {
			cmdqRecSetSecure(handle, 1);
			/* set engine as sec */
			cmdqRecSecureEnablePortSecurity(handle, (1LL << cmdq_engine));
			/* cmdqRecSecureEnableDAPC(handle, (1LL << cmdq_engine)); */

			if (rdma_is_sec[rdma_idx] == 0)
				DDPMSG("[SVP] switch rdma%d to sec\n", rdma_idx);
			rdma_is_sec[rdma_idx] = 1;
		} else {
			if (rdma_is_sec[rdma_idx]) {
				/* rdma is in sec stat, we need to switch it to nonsec */
				cmdqRecHandle nonsec_switch_handle;
				int ret;
				ret =
				    cmdqRecCreate(CMDQ_SCENARIO_DISP_PRIMARY_DISABLE_SECURE_PATH,
						  &(nonsec_switch_handle));
				if (ret)
					DDPAEE("[SVP]fail to create disable handle %s ret=%d\n",
					       __func__, ret);

				cmdqRecReset(nonsec_switch_handle);
				_cmdq_insert_wait_frame_done_token_mira(nonsec_switch_handle);
				cmdqRecSetSecure(nonsec_switch_handle, 1);

				/*ugly work around by kzhang !!. will remove when cmdq delete disable scenario.
				 * To avoid translation fault like ovl (see notes in ovl.c)*/
				do_rdma_config_l(module, pConfig, nonsec_switch_handle);

				/*in fact, dapc/port_sec will be disabled by cmdq */
				cmdqRecSecureEnablePortSecurity(nonsec_switch_handle,
								(1LL << cmdq_engine));
				/* cmdqRecSecureEnableDAPC(nonsec_switch_handle, (1LL << cmdq_engine)); */

				cmdqRecFlush(nonsec_switch_handle);
				cmdqRecDestroy(nonsec_switch_handle);
				DDPMSG("[SVP] switch rdma%d to nonsec done\n", rdma_idx);
			}
			rdma_is_sec[rdma_idx] = 0;
		}
	}
	return 0;
}

#endif

#if 0
static int rdma_config_l(DISP_MODULE_ENUM module, disp_ddp_path_config *pConfig, void *handle)
{
	if (pConfig->dst_dirty || pConfig->rdma_dirty) {

		setup_rdma_sec(module, pConfig, handle);

		do_rdma_config_l(module, pConfig, handle);
	}
	return 0;
}
#endif

#if 0
void rdma_enable_color_transform(DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	UINT32 value = DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + DISP_RDMA_INDEX_OFFSET * idx);
	value = value | REG_FLD_VAL((SIZE_CON_0_FLD_MATRIX_EXT_MTX_EN), 1) |
	    REG_FLD_VAL((SIZE_CON_0_FLD_MATRIX_ENABLE), 1);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, value);
}

void rdma_disable_color_transform(DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	UINT32 value = DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + DISP_RDMA_INDEX_OFFSET * idx);
	value = value | REG_FLD_VAL((SIZE_CON_0_FLD_MATRIX_EXT_MTX_EN), 0) |
	    REG_FLD_VAL((SIZE_CON_0_FLD_MATRIX_ENABLE), 0);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, value);
}
#endif

#if 0
void rdma_set_color_matrix(DISP_MODULE_ENUM module,
 			   rdma_color_matrix *matrix, rdma_color_pre *pre, rdma_color_post *post)
{
	unsigned int idx = rdma_index(module);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C00, matrix->C00);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C01, matrix->C01);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C02, matrix->C02);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C10, matrix->C10);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C11, matrix->C11);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C12, matrix->C12);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C20, matrix->C20);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C21, matrix->C21);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_C22, matrix->C22);

	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_PRE_ADD_0, pre->ADD0);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_PRE_ADD_1, pre->ADD1);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_PRE_ADD_2, pre->ADD2);

	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_POST_ADD_0, post->ADD0);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_POST_ADD_1, post->ADD1);
	DISP_REG_SET(NULL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_POST_ADD_2, post->ADD2);
}
#endif

#if 0

int rdma_ioctl(DISP_MODULE_ENUM module, void *cmdq_handle, unsigned int ioctl_cmd, unsigned long *params)
{
	int ret = 0;
	DDP_IOCTL_NAME ioctl = (DDP_IOCTL_NAME) ioctl_cmd;
	unsigned int idx = rdma_index(module);
	disp_ddp_path_config *pConfig = (disp_ddp_path_config *)params;
	golden_setting_context *p_golden_setting = pConfig->p_golden_setting_context;

	switch (ioctl) {
	case DDP_RDMA_GOLDEN_SETTING:
		rdma_set_ultra_l(idx, pConfig->lcm_bpp, cmdq_handle, p_golden_setting);
		break;
	default:
		break;
	}

	return ret;
}

DDP_MODULE_DRIVER ddp_driver_rdma = {
	.init = rdma_init,
	.deinit = rdma_deinit,
	.config = rdma_config_l,
	.start = rdma_start,
	.trigger = NULL,
	.stop = rdma_stop,
	.reset = rdma_reset,
	.power_on = rdma_clock_on,
	.power_off = rdma_clock_off,
	.is_idle = NULL,
	.is_busy = NULL,
	.dump_info = rdma_dump,
	.bypass = NULL,
	.build_cmdq = NULL,
	.set_lcm_utils = NULL,
	.enable_irq = rdma_enable_irq,
	.ioctl = rdma_ioctl,
};

#endif

