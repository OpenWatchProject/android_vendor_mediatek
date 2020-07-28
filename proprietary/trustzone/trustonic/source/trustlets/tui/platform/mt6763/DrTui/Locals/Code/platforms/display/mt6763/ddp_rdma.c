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

#include "ddp_reg.h"
#include "ddp_rdma.h"
#include "ddp_color_format.h"
#include "ddp_info.h"
#include "display_tui.h"

#define MMSYS_CLK_LOW (0)
#define MMSYS_CLK_HIGH (1)
#define MMSYS_CLK_MEDIUM (2)

static unsigned int rdma_fps[RDMA_INSTANCES] = { 60, 60 };
static struct golden_setting_context *rdma_golden_setting;

unsigned long rdma_base_addr(enum DISP_MODULE_ENUM module)
{
	switch (module) {
	case DISP_MODULE_RDMA0:
		return DDP_REG_BASE_DISP_RDMA0;
	case DISP_MODULE_RDMA1:
		return DDP_REG_BASE_DISP_RDMA1;
	default:
		DDPERR("invalid rdma module=%d\n", module);
	}
	return 0;
}

static unsigned int rdma_index(enum DISP_MODULE_ENUM module)
{
	int idx = 0;

	switch (module) {
	case DISP_MODULE_RDMA0:
		idx = 0;
		break;
	case DISP_MODULE_RDMA1:
		idx = 1;
		break;
	case DISP_MODULE_RDMA2:
		idx = 2;
		break;
	default:
		DDPERR("invalid rdma module=%d\n", module);	/* invalid module */
		ASSERT(0);
	}
	ASSERT((idx >= 0) && (idx < RDMA_INSTANCES));
	return idx;
}

unsigned int rdma_disable_irq_backup(enum DISP_MODULE_ENUM module, void *handle)
{
	unsigned long base_addr = rdma_base_addr(module);
	unsigned int idx = rdma_index(module);
	unsigned int old_val;

	old_val = DISP_REG_GET(base_addr + DISP_REG_RDMA_INT_ENABLE);
	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_INT_ENABLE, 0x0);
	DDPDBG("%s, rdma%d,irq_enable:old_val =%d\n", __func__, idx, old_val);

    return old_val;
}

int rdma_irq_restore(enum DISP_MODULE_ENUM module, void *handle, unsigned int irq_val)
{
	unsigned long base_addr = rdma_base_addr(module);

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_INT_ENABLE, irq_val);
	return 0;
}

int rdma_reg_backup(enum DISP_MODULE_ENUM module)
{
	unsigned long base_addr = rdma_base_addr(module);

	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_GLOBAL_CON);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_MEM_CON);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_MEM_START_ADDR);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_MEM_SRC_PITCH);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_INT_ENABLE);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_SIZE_CON_0);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_SIZE_CON_1);
	disp_tui_reg_backup(base_addr + DISP_REG_RDMA_FIFO_CON);

	return 0;
}

/* set ultra registers */
void rdma_set_ultra_l(enum DISP_MODULE_ENUM module, unsigned int bpp, void *handle,
	struct golden_setting_context *p_golden_setting)
{
	unsigned long base_addr = rdma_base_addr(module);
	/* rdma golden setting variables */
	unsigned int mmsysclk = 273;
	unsigned int is_wrot_sram = 0;
	unsigned int fifo_mode = 1;

	unsigned int ultra_low_us = 4;
	unsigned int ultra_high_us = 6;
	unsigned int preultra_low_us = ultra_high_us;
	unsigned int preultra_high_us = 7;

	unsigned long long fill_rate = 0;
	unsigned long long consume_rate = 0;

	unsigned int fifo_valid_size = 640;

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

	unsigned int frame_rate;
	unsigned int Bytes_per_sec;
	long long temp;
	long long temp_for_div;


	if (!p_golden_setting) {
		DDPERR("golden setting is null, %s,%d\n", __FILE__, __LINE__);
		ASSERT(0);
		return;
	}
	rdma_golden_setting = p_golden_setting;

	frame_rate = rdma_golden_setting->fps;
	if (module == DISP_MODULE_RDMA1) {
		/* hardcode bpp & frame_rate for rdma1 */
		bpp = 24;
		frame_rate = 60;

		if ((rdma_golden_setting->ext_dst_width == 3840) &&
				(rdma_golden_setting->ext_dst_height == 2160))
			frame_rate = 30;
	}

	/* get fifo parameters */
	switch (rdma_golden_setting->mmsys_clk) {
	case MMSYS_CLK_LOW:
		mmsysclk = 273;
		break;
	case MMSYS_CLK_HIGH:
		mmsysclk = 364;
		break;
	default:
		mmsysclk = 273; /* worse case */
		break;
	}

	Bytes_per_sec = bpp / 8;
	if (!Bytes_per_sec) {
		DDPERR("bpp is invalid, bpp=%d\n", bpp);
		return;
	}

	is_wrot_sram = rdma_golden_setting->is_wrot_sram;
	fifo_mode = rdma_golden_setting->fifo_mode;

	ultra_low_us = 4;
	ultra_high_us = 6;
	preultra_low_us = ultra_high_us;
	preultra_high_us = 7;

	if (rdma_golden_setting->is_dc)
		fill_rate = 960*mmsysclk; /* FIFO depth / us  */
	else
		fill_rate = 960*mmsysclk*3/16; /* FIFO depth / us  */

	if (module == DISP_MODULE_RDMA0) {
		consume_rate = rdma_golden_setting->dst_width * rdma_golden_setting->dst_height;
		consume_rate /= (8*1000);
		consume_rate *= frame_rate * bpp;
	} else {
		consume_rate = rdma_golden_setting->ext_dst_width
		* rdma_golden_setting->ext_dst_height * frame_rate  *bpp / (8 * 1000);
	}
	consume_rate *= 1250;
	consume_rate /= (16 * 1000);

	if (rdma_golden_setting->is_dual_pipe)
		consume_rate /= 2;

	preultra_low = preultra_low_us * consume_rate;
	if (preultra_low % 1000)
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
	if (module == DISP_MODULE_RDMA0) {
		/* only rdma0 can share sram */
		if (is_wrot_sram)
			fifo_valid_size = 2048;
		else
			fifo_valid_size = 640;
	} else {
		fifo_valid_size = 640;
	}

	issue_req_threshold = (fifo_valid_size - preultra_low) < 256  ? (fifo_valid_size - preultra_low) : 256;
	/* output valid should < total rdma data size, or hang will happen */
	temp = rdma_golden_setting->rdma_width * rdma_golden_setting->rdma_height * Bytes_per_sec;
	temp /= 16;
	temp -= 1;
	output_valid_fifo_threshold = preultra_low < temp ? preultra_low : temp;

	/* SODI threshold */
	sodi_threshold_low = ((ultra_low_us + 5)*10) * consume_rate;
	if (sodi_threshold_low % 1000)
		sodi_threshold_low = sodi_threshold_low/1000 + 1;
	else
		sodi_threshold_low = sodi_threshold_low/1000;

	temp_for_div = 1200 * (fill_rate - consume_rate);
	temp_for_div /= 1000000;
	temp = fifo_valid_size - temp_for_div;
	if (temp < 0)
		sodi_threshold_high = preultra_high;
	else
		sodi_threshold_high = preultra_high > temp ? preultra_high : temp;

	dvfs_threshold_low = preultra_low;
	dvfs_threshold_high = preultra_low+1;

	/* keep MEM_GMC_SETTING_0_FLD_RG_VALID_THRESHOLD_FORCE_PREULTRA[30] enable  */
	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_MEM_GMC_SETTING_0,
		preultra_low | (preultra_high << 16) | 0 << 30);

	/* keep MEM_GMC_SETTING_1_FLD_RG_VALID_THRESHOLD_BLOCK_ULTRA[30] enable */
	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_MEM_GMC_SETTING_1,
		ultra_low | (ultra_high << 16) | 0 << 30);

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_MEM_GMC_SETTING_2,
		issue_req_threshold);

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_FIFO_CON,
		REG_FLD_VAL(FIFO_CON_FLD_OUTPUT_VALID_FIFO_THRESHOLD, output_valid_fifo_threshold) |
		REG_FLD_VAL(FIFO_CON_FLD_FIFO_PSEUDO_SIZE, fifo_valid_size) |
		REG_FLD_VAL(FIFO_CON_FLD_FIFO_UNDERFLOW_EN, 1));

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_THRESHOLD_FOR_SODI,
		sodi_threshold_low | (sodi_threshold_high << 16));

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_THRESHOLD_FOR_DVFS,
		dvfs_threshold_low | (dvfs_threshold_high << 16));

	/* only config RDMA0 SRAM_SEL */
	if (module == DISP_MODULE_RDMA0)
		DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_SRAM_SEL, is_wrot_sram);

	/* DVFS_SETTING ULTRA, extend 2us for DVFS blocking time */
	preultra_low = (preultra_low_us + 2) * consume_rate;
	if (preultra_low % 1000)
		preultra_low = preultra_low / 1000 + 1;
	else
		preultra_low = preultra_low / 1000;

	preultra_high = (preultra_high_us + 2) * consume_rate;
	if (preultra_high % 1000)
		preultra_high = preultra_high / 1000 + 1;
	else
		preultra_high = preultra_high / 1000;


	ultra_low = (ultra_low_us + 2) * consume_rate;
	if (ultra_low % 1000)
		ultra_low = ultra_low / 1000 + 1;
	else
		ultra_low = ultra_low / 1000;


	ultra_high = preultra_low;

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_DVFS_SETTING_ULTRA,
		ultra_low | (ultra_high << 16));

	/* LEAVE DRS SETTING, extend 2us for blocking time */
	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_LEAVE_DRS_SETTING,
		ultra_low | (ultra_low << 16));

	/* ENTER DRS SETTING, extend 4us for blocking time */
	ultra_low = (ultra_low_us + 4) * consume_rate;
	if (ultra_low % 1000)
		ultra_low = ultra_low / 1000 + 1;
	else
		ultra_low = ultra_low / 1000;


	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_ENTER_DRS_SETTING,
		ultra_low | (ultra_low << 16));

/*if (idx == 0)*/
		/*rdma_dump_golden_setting_context(DISP_MODULE_RDMA0);*/
	/*else*/
		/*rdma_dump_golden_setting_context(DISP_MODULE_RDMA1);*/

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


int rdma_config(enum DISP_MODULE_ENUM module,
		       enum RDMA_MODE mode,
		       unsigned long address,
		       enum UNIFIED_COLOR_FMT inFormat,
		       unsigned pitch,
		       unsigned width,
		       unsigned height,
		       unsigned ufoe_enable,
		       enum DISP_BUFFER_TYPE sec,
		       unsigned int yuv_range, struct rdma_bg_ctrl_t *bg_ctrl, void *handle,
		       struct golden_setting_context *p_golden_setting, unsigned int bpp)
{
	unsigned long base_addr = rdma_base_addr(module);
	unsigned int output_is_yuv = 0;
	unsigned int input_is_yuv = !UFMT_GET_RGB(inFormat);
	unsigned int input_swap = UFMT_GET_BYTESWAP(inFormat);
	unsigned int input_format_reg = UFMT_GET_FORMAT(inFormat);
	unsigned int idx = rdma_index(module);
	unsigned int color_matrix;
	unsigned int regval;

	DDPDBG("RDMAConfig idx %d, mode %d, address 0x%x, inFormat 0x%x, pitch %u, width %u, height %u,sec%d\n",
	       idx, mode, address, inFormat, pitch, width, height, sec);

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

		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_ENABLE,
				   base_addr + DISP_REG_RDMA_SIZE_CON_0, 1);
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL,
				   base_addr + DISP_REG_RDMA_SIZE_CON_0,
				   color_matrix);
	} else if (input_is_yuv == 0 && output_is_yuv == 1) {
		color_matrix = 0x2;	/* 0x0010, RGB_TO_BT601 */
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_ENABLE,
				   base_addr + DISP_REG_RDMA_SIZE_CON_0, 1);
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL,
				   base_addr + DISP_REG_RDMA_SIZE_CON_0,
				   color_matrix);
	} else {
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_ENABLE,
				   base_addr + DISP_REG_RDMA_SIZE_CON_0, 0);
		DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL,
				   base_addr + DISP_REG_RDMA_SIZE_CON_0, 0);
	}

	DISP_REG_SET_FIELD(handle, GLOBAL_CON_FLD_MODE_SEL,
			   base_addr + DISP_REG_RDMA_GLOBAL_CON, mode);
	/* FORMAT & SWAP only works when RDMA memory mode, set both to 0 when RDMA direct link mode. */
	DISP_REG_SET_FIELD(handle, MEM_CON_FLD_MEM_MODE_INPUT_FORMAT,
			   base_addr + DISP_REG_RDMA_MEM_CON,
			   ((mode == RDMA_MODE_DIRECT_LINK) ? 0 : input_format_reg & 0xf));
	DISP_REG_SET_FIELD(handle, MEM_CON_FLD_MEM_MODE_INPUT_SWAP,
			   base_addr + DISP_REG_RDMA_MEM_CON,
			   ((mode == RDMA_MODE_DIRECT_LINK) ? 0 : input_swap));

	if (sec != DISP_SECURE_BUFFER) {
		DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_MEM_START_ADDR,
			     address);
	} else {
		ASSERT(0);
	}

	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_MEM_SRC_PITCH, pitch);
	DISP_REG_SET_FIELD(handle, SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH,
			   base_addr + DISP_REG_RDMA_SIZE_CON_0, width);
	DISP_REG_SET_FIELD(handle, SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT,
			   base_addr + DISP_REG_RDMA_SIZE_CON_1, height);

	/* rdma bg control */
	regval = REG_FLD_VAL(RDMA_BG_CON_0_LEFT, bg_ctrl->left);
	regval |= REG_FLD_VAL(RDMA_BG_CON_0_RIGHT, bg_ctrl->right);
	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_BG_CON_0, regval);

	regval = REG_FLD_VAL(RDMA_BG_CON_1_TOP, bg_ctrl->top);
	regval |= REG_FLD_VAL(RDMA_BG_CON_1_BOTTOM, bg_ctrl->bottom);
	DISP_REG_SET(handle, base_addr + DISP_REG_RDMA_BG_CON_1, regval);

	/* set_rdma_width_height(width, height);
	rdma_set_ultra_l(module, bpp, handle, p_golden_setting)*/;

	return 0;
}

int rdma_is_mem_mode(enum DISP_MODULE_ENUM module)
{
	unsigned long base_addr = rdma_base_addr(module);
	enum RDMA_MODE mode;

	mode = DISP_REG_GET_FIELD(GLOBAL_CON_FLD_MODE_SEL, base_addr + DISP_REG_RDMA_GLOBAL_CON);
	return mode == RDMA_MODE_MEMORY;
}

/*** !!   rdma wait frame done by polling / interrupt / cmdq_event      (   +  )    !!***/

#define RDMA_INT_FRAME_START_BIT 1
#define RDMA_INT_FRAME_DONE_BIT 2
#define RDMA_INT_FRAME_ABNORMAL_BIT 3
#define RDMA_INT_FRAME_UNDERFLOW_BIT 4

int rdma_wait_frame_done_by_polling(enum DISP_MODULE_ENUM module)
{
	unsigned long base_addr = rdma_base_addr(module);
	unsigned int regval;
	/* clear interrupt first*/
	DISP_REG_SET(NULL, base_addr + DISP_REG_RDMA_INT_STATUS, 0x0);

	while(1) {

		regval = DISP_REG_GET(base_addr + DISP_REG_RDMA_INT_STATUS);
	    DISP_REG_SET(NULL, base_addr + DISP_REG_RDMA_INT_STATUS, ~regval);

		if(regval & (1<<RDMA_INT_FRAME_ABNORMAL_BIT)) {
			DDPERR("error: rdma frame abnormal int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_UNDERFLOW_BIT)) {
			DDPERR("error: rdma frame underflow int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_DONE_BIT)) {
			return 0;
		}
	}
}


#if DISP_RDMA_INTERRUPT_ENABLE
static int rdma_irq_num[3] = {283, 284, 285};
static int rdma_irq_attached[3] = {0, 0, 0};

int rdma_irq_attach(enum DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(!rdma_irq_attached[idx]) {
		ret = drApiIntrAttach(irq_num, INTR_MODE_LOW_LEVEL);
		if(ret != DRAPI_OK){
			DDPERR("error to attach irq %d flag=%d, ret=%d!!\n",
				irq_num, INTR_MODE_LOW_LEVEL, ret);
			return -1;
		}
		rdma_irq_attached[idx] = 1;
	}
	DDPDBG("%s, rdma%d,to attach irq %d flag=%d\n", __func__, idx, irq_num, INTR_MODE_LOW_LEVEL);
	return 0;
}

int rdma_irq_dettach(enum DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(rdma_irq_attached[idx]) {
		ret = drApiIntrDetach(irq_num);
		if(ret != DRAPI_OK) {
			DDPERR("error to detach irq %d ret=%d!!\n",
				irq_num, ret);
			return -1;
		}
		rdma_irq_attached[idx] = 0;
	}
	return 0;
}

int rdma_wait_frame_done_by_interrupt(enum DISP_MODULE_ENUM module)
{
	unsigned long base_addr = rdma_base_addr(module);
	unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(!rdma_irq_attached[idx]) {
		ret = rdma_irq_attach(module);
		if(ret)
			goto err1;
	}

	DISP_REG_SET(NULL, base_addr + DISP_REG_RDMA_INT_STATUS, 0x0);
	rdma_irq_restore(module, NULL, 0x1C);

	while(1) {
		ret = drApiWaitForIntr(irq_num, TIME_INFINITE, NULL);
		if (ret != DRAPI_OK) {
			DDPERR("error to wait irq %d time=%d, ret=%d\n",
				irq_num, TIME_INFINITE, ret);
			goto err1;
		}

		regval = DISP_REG_GET(base_addr + DISP_REG_RDMA_INT_STATUS);
		DISP_REG_SET(NULL, base_addr + DISP_REG_RDMA_INT_STATUS, ~regval);

		if (regval & (1<<RDMA_INT_FRAME_ABNORMAL_BIT))
			DDPERR("error: rdma frame abnormal int happens 0x%x\n", regval);

		if (regval & (1<<RDMA_INT_FRAME_UNDERFLOW_BIT))
			DDPERR("error: rdma frame underflow int happens 0x%x\n", regval);

		if (regval & (1<<RDMA_INT_FRAME_DONE_BIT)) {
			rdma_irq_restore(module, NULL, 0);
			return 0;
		}
	}

	return 0;

err1:
	rdma_irq_restore(module, NULL, 0);
	return rdma_wait_frame_done_by_polling(module);
}
#endif

CMDQ_EVENT_ENUM rdma_get_EOF_cmdq_event(enum DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	CMDQ_EVENT_ENUM event;

	if (idx == 0)
		event = CMDQ_EVENT_DISP_RDMA0_EOF;
	else if (idx == 1)
		event = CMDQ_EVENT_DISP_RDMA1_EOF;
	else
		ASSERT(0);

	return event;
}

int rdma_wait_frame_done_by_cmdq(enum DISP_MODULE_ENUM module, cmdqRecHandle cmdq_handle,
			int clear_before_wait, int clear_after_wait)
{
	CMDQ_EVENT_ENUM event;
	int ret = 0;

	ASSERT(cmdq_handle != NULL);
	event = rdma_get_EOF_cmdq_event(module);

	if (clear_before_wait)
		ret |= cmdqRecClearEventToken(cmdq_handle, event);

	if (clear_after_wait)
		ret |= cmdqRecWait(cmdq_handle, event);
	else
		ret |= cmdqRecWaitNoClear(cmdq_handle, event);

	if (ret)
		DDPERR("%s error ret=%d\n", __func__);

	return ret;
}

void rdma_dump_golden_setting_context(enum DISP_MODULE_ENUM module)
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

void rdma_dump_reg(enum DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	unsigned long base_addr = rdma_base_addr(module);

	/* dump working register */
	DISP_REG_MASK(NULL, base_addr + DISP_REG_RDMA_SHADOW_UPDATE, 0x4, 0x4);
	DDPDUMP("== DISP RDMA%d REGS ==\n", idx);
	DDPDUMP("(0x000)R_INTEN=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE + base_addr));
	DDPDUMP("(0x004)R_INTS=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_INT_STATUS + base_addr));
	DDPDUMP("(0x010)R_CON=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON + base_addr));
	DDPDUMP("(0x014)R_SIZE0=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + base_addr));
	DDPDUMP("(0x018)R_SIZE1=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1 + base_addr));
	DDPDUMP("(0x01c)R_TAR_LINE=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_TARGET_LINE + base_addr));
	DDPDUMP("(0x024)R_M_CON=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_CON + base_addr));
	DDPDUMP("(0xf00)R_M_S_ADDR=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + base_addr));
	DDPDUMP("(0x02c)R_M_SRC_PITCH=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH + base_addr));
	DDPDUMP("(0x030)R_M_GMC_SET0=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0 + base_addr));
	DDPDUMP("(0x034)R_M_GMC_SET1=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1 + base_addr));
	DDPDUMP("(0x038)R_M_SLOW_CON=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON + base_addr));
	DDPDUMP("(0x03c)R_M_GMC_SET2=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_2 + base_addr));
	DDPDUMP("(0x040)R_FIFO_CON=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_FIFO_CON + base_addr));
	DDPDUMP("(0x044)R_FIFO_LOG=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG + base_addr));
	DDPDUMP("(0x078)R_PRE_ADD0=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_0 + base_addr));
	DDPDUMP("(0x07c)R_PRE_ADD1=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_1 + base_addr));
	DDPDUMP("(0x080)R_PRE_ADD2=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_2 + base_addr));
	DDPDUMP("(0x084)R_POST_ADD0=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_POST_ADD_0 + base_addr));
	DDPDUMP("(0x088)R_POST_ADD1=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_POST_ADD_1 + base_addr));
	DDPDUMP("(0x08c)R_POST_ADD2=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_POST_ADD_2 + base_addr));
	DDPDUMP("(0x090)R_DUMMY=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_DUMMY + base_addr));
	DDPDUMP("(0x094)R_OUT_SEL=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL + base_addr));
	DDPDUMP("(0x094)R_M_START=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + base_addr));
	DDPDUMP("(0x0a0)R_BG_CON_0=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_BG_CON_0 + base_addr));
	DDPDUMP("(0x0a4)R_BG_CON_1=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_BG_CON_1 + base_addr));
	DDPDUMP("(0x0a8)R_FOR_SODI=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_THRESHOLD_FOR_SODI + base_addr));
	DDPDUMP("(0x0ac)R_FOR_DVFS=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_THRESHOLD_FOR_DVFS + base_addr));
	DDPDUMP("(0x0b0)R_FOR_SRAM=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SRAM_SEL + base_addr));
	DDPDUMP("(0x0b4)DISP_REG_RDMA_STALL_CG_CON=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_STALL_CG_CON + base_addr));
	DDPDUMP("(0x0b8)DISP_REG_RDMA_SHADOW_UPDATE    =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_SHADOW_UPDATE + base_addr));
	DDPDUMP("(0x0f0)R_IN_PXL_CNT  =0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_IN_P_CNT + base_addr));
	DDPDUMP("(0x0f4)R_IN_LINE_CNT=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_IN_LINE_CNT + base_addr));
	DDPDUMP("(0x0f8)R_OUT_PXL_CNT=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_OUT_P_CNT + base_addr));
	DDPDUMP("(0x0fc)R_OUT_LINE_CNT=0x%x\n",
		DISP_REG_GET(DISP_REG_RDMA_OUT_LINE_CNT + base_addr));

	DISP_REG_MASK(NULL, base_addr + DISP_REG_RDMA_SHADOW_UPDATE, 0x0, 0x4);
}

void rdma_dump_analysis(enum DISP_MODULE_ENUM module)
{
	unsigned int idx = rdma_index(module);
	unsigned long base_addr = rdma_base_addr(module);
	unsigned int global_ctrl = DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON + base_addr);
	unsigned int bg0 = DISP_REG_GET(base_addr + DISP_REG_RDMA_BG_CON_0);
	unsigned int bg1 = DISP_REG_GET(base_addr + DISP_REG_RDMA_BG_CON_1);

	DDPDUMP("== DISP RDMA%d ANALYSIS ==\n", idx);
	DDPDUMP("rdma%d: en=%d,memory_mode=%d,smi_busy=%d,w=%d,h=%d,pitch=%d,addr=0x%x,fmt=%s,fifo_min=%d,\n",
		idx, REG_FLD_VAL_GET(GLOBAL_CON_FLD_ENGINE_EN, global_ctrl),
		REG_FLD_VAL_GET(GLOBAL_CON_FLD_MODE_SEL, global_ctrl),
		REG_FLD_VAL_GET(GLOBAL_CON_FLD_SMI_BUSY, global_ctrl),
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + base_addr) & 0xfff,
		DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1 + base_addr) & 0xfffff,
		DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH + base_addr),
		DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + base_addr),
		unified_color_fmt_name(
			display_fmt_reg_to_unified_fmt((DISP_REG_GET(DISP_REG_RDMA_MEM_CON +
								     base_addr) >> 4) & 0xf,
						       (DISP_REG_GET(DISP_REG_RDMA_MEM_CON +
								      base_addr) >> 8) & 0x1, 0)),
		DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG + base_addr));
}

static int rdma_dump(enum DISP_MODULE_ENUM module, int level)
{
	rdma_dump_analysis(module);
	rdma_dump_reg(module);
	rdma_dump_golden_setting_context(module);

	return 0;
}
