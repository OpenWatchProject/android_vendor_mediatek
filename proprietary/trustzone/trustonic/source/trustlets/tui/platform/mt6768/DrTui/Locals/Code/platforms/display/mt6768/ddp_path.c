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

#define LOG_TAG "ddp_path"
#include "ddp_log.h"
#include "ddp_reg.h"
#include "ddp_path.h"
#include "display_tui.h"

#pragma GCC optimize("O0")

#define BIT_NUM (8)
#define DDP_ENING_NUM (24)
#if 0
#define DDP_MOUT_NUM		9
#define DDP_SEL_OUT_NUM		13
#define DDP_SEL_IN_NUM		20
#endif
#define DDP_ENG_NUM   10
#define DDP_MUTEX_MAX    5

#define DISP_COLOR_ON
#define DDP_CLK_PREPARE_ENABLE    0x00010001
#define DDP_CLK_PREPARE_DISABLE    0x00010000
#define DDP_CLK_UNPREPARE_ENABLE    0x00000001
#define DDP_CLK_UNPREPARE_DISABLE    0x00000000

struct module_map_s {
	enum DISP_MODULE_ENUM module;
	int bit;
	int mod_num;
};

struct m_to_b {
	int m;
	int v;
};

struct m_to_hw_b {
	int m;
	int g;
	int b;
};

struct mout_s {
	int id;
	struct m_to_b out_id_bit_map[BIT_NUM];

	unsigned long *reg;
	unsigned int reg_val;
};

struct mout_hw_s {
	int id;
	struct m_to_hw_b out_id_bit_map[BIT_NUM];
};

struct sel_s {
	int id;
	int id_bit_map[BIT_NUM];

	unsigned long *reg;
	unsigned int reg_val;
};

unsigned int module_list_scenario[DDP_SCENARIO_MAX][DDP_ENING_NUM] = {
	/* DDP_SCENARIO_PRIMARY_DISP */
	{
		DISP_MODULE_OVL0, DISP_MODULE_OVL0_2L, DISP_MODULE_RDMA0,
		DISP_MODULE_RSZ0_VIRTUAL0, DISP_MODULE_RSZ0_VIRTUAL1,
#ifdef DISP_COLOR_ON
		DISP_MODULE_COLOR0,
#endif
		DISP_MODULE_CCORR0, DISP_MODULE_AAL0, DISP_MODULE_GAMMA0,
		DISP_MODULE_DITHER0,
		DISP_MODULE_PWM0, DISP_MODULE_DSI0,
		-1,
	},

	/* DDP_SCENARIO_PRIMARY_BYPASS_PQ_DISP */
	{
		DISP_MODULE_OVL0, DISP_MODULE_OVL0_2L, DISP_MODULE_RDMA0,
		DISP_MODULE_RSZ0_VIRTUAL0, DISP_MODULE_RSZ0_VIRTUAL1,
		DISP_MODULE_PWM0, DISP_MODULE_DSI0,
		-1,
	},

	/* DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP */
	{
		DISP_MODULE_RDMA0,
		DISP_MODULE_RSZ0_VIRTUAL0, DISP_MODULE_RSZ0_VIRTUAL1,
#ifdef DISP_COLOR_ON
		DISP_MODULE_COLOR0,
#endif
		DISP_MODULE_CCORR0, DISP_MODULE_AAL0, DISP_MODULE_GAMMA0,
		DISP_MODULE_DITHER0, DISP_MODULE_PWM0, DISP_MODULE_DSI0,
		-1,
	},

	/* DDP_SCENARIO_PRIMARY_RDMA0_DISP */
	{
		DISP_MODULE_RDMA0, DISP_MODULE_PWM0, DISP_MODULE_DSI0,
		DISP_MODULE_RSZ0_VIRTUAL0, DISP_MODULE_RSZ0_VIRTUAL1,
		-1,
	},

	/* DDP_SCENARIO_PRIMARY_OVL_MEMOUT */
	{
		DISP_MODULE_OVL0, DISP_MODULE_OVL0_2L,
		DISP_MODULE_WDMA0,
		-1,
	},

	/* DDP_SCENARIO_PRIMARY_ALL */
	{
		DISP_MODULE_OVL0, DISP_MODULE_OVL0_2L,
		DISP_MODULE_WDMA0,
		DISP_MODULE_RDMA0,
		DISP_MODULE_RSZ0_VIRTUAL0, DISP_MODULE_RSZ0_VIRTUAL0,
#ifdef DISP_COLOR_ON
		DISP_MODULE_COLOR0,
#endif
		DISP_MODULE_CCORR0, DISP_MODULE_AAL0, DISP_MODULE_GAMMA0,
		DISP_MODULE_DITHER0, DISP_MODULE_PWM0, DISP_MODULE_DSI0,
		-1,
	},

	/* DDP_SCENARIO_SUB_OVL_MEMOUT */
	{
		DISP_MODULE_OVL0_2L, DISP_MODULE_WDMA0,
		-1,
	},
};

/* 1st para is mout's input, 2nd para is mout's output */
static struct mout_s mout_map[] = {
	/* OVL_MOUT */
	{DISP_MODULE_OVL0,
		{{DISP_MODULE_RDMA0, 1 << 0},
		 {DISP_MODULE_OVL0_2L, 1 << 1},
		 {DISP_MODULE_WDMA0, 1 << 2},
		 {DISP_MODULE_RSZ0, 1 << 3}, {-1, 0} },
		0, 0},

	/* OVL0_2L_MOUT */
	{DISP_MODULE_OVL0_2L,
		{{DISP_MODULE_RDMA0, 1 << 0},
		 {DISP_MODULE_WDMA0, 1 << 1},
		 {DISP_MODULE_RSZ0, 1 << 2}, {-1, 0} },
		0, 0},

	/* DISP_RSZ0_MOUT_EN */
	{DISP_MODULE_RSZ0,
		{{DISP_MODULE_RDMA0, 1 << 0},
		 {DISP_MODULE_OVL0, 1 << 1},
		 {DISP_MODULE_OVL0_2L, 1 << 2},
		 {DISP_MODULE_RSZ0_VIRTUAL1, 1 << 3},
		 {DISP_MODULE_WDMA0, 1 << 4}, {-1, 0} },
		0, 0},

	/* DITHER0_MOUT */
	{DISP_MODULE_DITHER0,
		{{DISP_MODULE_DSI0, 1 << 0},
		 {DISP_MODULE_WDMA0, 1 << 1}, {-1, 0} },
		0, 0},
};

static struct sel_s sel_out_map[] = {
	/* DISP_RDMA0_RSZ0_IN_SOUT_SEL */
	{DISP_MODULE_RDMA0, {DISP_MODULE_RSZ0_VIRTUAL0,
		DISP_MODULE_RSZ0}, 0, 0},

	/* DISP_RDMA0_SOUT_SEL */
	{DISP_MODULE_RSZ0_VIRTUAL1, {DISP_MODULE_DSI0,
		DISP_MODULE_COLOR0, DISP_MODULE_CCORR0}, 0, 0},
};

/* 1st para is sout's output, 2nd para is sout's input */
static struct sel_s sel_in_map[] = {
	/* DISP_PATH0_SEL_IN */
	{DISP_MODULE_RDMA0, {DISP_MODULE_OVL0, DISP_MODULE_OVL0_2L,
	 DISP_MODULE_RSZ0, -1},
	 0, 0},

	/* DISP_RSZ0_SEL_IN */
	{DISP_MODULE_RSZ0, {DISP_MODULE_OVL0, DISP_MODULE_OVL0_2L,
	 DISP_MODULE_RDMA0, -1}, 0, 0},

	/* DISP_RDMA0_RSZ0_SEL_IN */
	{DISP_MODULE_RSZ0_VIRTUAL1, {DISP_MODULE_RSZ0_VIRTUAL0,
	 DISP_MODULE_RSZ0, -1}, 0, 0},

	/* DISP_COLOR0_OUT_SEL_IN */
	{DISP_MODULE_CCORR0, {DISP_MODULE_COLOR0,
	 DISP_MODULE_RSZ0_VIRTUAL1, -1}, 0, 0},

	/* DSI0_SEL_IN */
	{DISP_MODULE_DSI0, {DISP_MODULE_RSZ0_VIRTUAL1,
	 DISP_MODULE_DITHER0, -1},
	 0, 0},

	/* DISP_WDMA0_SEL_IN */
	{DISP_MODULE_WDMA0, {DISP_MODULE_DITHER0, DISP_MODULE_OVL0,
	 DISP_MODULE_OVL0_2L, DISP_MODULE_RSZ0, -1}, 0, 0},
};

static const int DDP_MOUT_NUM = sizeof(mout_map)/sizeof(struct mout_s);
static const int DDP_SEL_OUT_NUM = sizeof(sel_out_map)/sizeof(struct sel_s);
static const int DDP_SEL_IN_NUM = sizeof(sel_in_map)/sizeof(struct sel_s);

int ddp_path_init(void)
{
	/* mout */
	mout_map[0].reg = (unsigned long *)DISP_REG_CONFIG_DISP_OVL0_MOUT_EN;
	mout_map[1].reg = (unsigned long *)DISP_REG_CONFIG_DISP_OVL0_2L_MOUT_EN;
	mout_map[2].reg = (unsigned long *)DISP_REG_CONFIG_DISP_RSZ0_MOUT_EN;
	mout_map[3].reg = (unsigned long *)DISP_REG_CONFIG_DITHER0_MOUT_EN;

	/* sel_out */
	sel_out_map[0].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_RDMA0_RSZ0_IN_SOUT_SEL;
	sel_out_map[1].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_RDMA0_SOUT_SEL;

	/* sel_in */
	sel_in_map[0].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_PATH0_SEL_IN;
	sel_in_map[1].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_RSZ0_SEL_IN;
	sel_in_map[2].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_RDMA0_RSZ0_SEL_IN;
	sel_in_map[3].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_COLOR0_OUT_SEL_IN;
	sel_in_map[4].reg =
		(unsigned long *)DISP_REG_CONFIG_DSI0_SEL_IN;
	sel_in_map[5].reg =
		(unsigned long *)DISP_REG_CONFIG_DISP_WDMA0_SEL_IN;

	return 0;
}

static struct module_map_s module_mutex_map[DISP_MODULE_NUM] = {
	{DISP_MODULE_OVL0, 7, 0},
	{DISP_MODULE_OVL0_2L, 8, 0},
	{DISP_MODULE_RDMA0, 9, 0},
	{DISP_MODULE_WDMA0, 10, 0},
	{DISP_MODULE_WDMA_VIRTUAL0, -1, 0},
	{DISP_MODULE_WDMA_VIRTUAL1, -1, 0},
	{DISP_MODULE_COLOR0, 11, 0},
	{DISP_MODULE_CCORR0, 12, 0},
	{DISP_MODULE_AAL0, 13, 0},
	{DISP_MODULE_GAMMA0, 14, 0},
	{DISP_MODULE_DITHER0, 15, 0},
	{DISP_MODULE_DSI0, 16, 0},
	{DISP_MODULE_DSI1, -1, 0},
	{DISP_MODULE_DSIDUAL, -1, 0},
	{DISP_MODULE_PWM0, 19, 0},
	{DISP_MODULE_CONFIG, -1, 0},
	{DISP_MODULE_MUTEX, -1, 0},
	{DISP_MODULE_SMI_COMMON, -1, 0},
	{DISP_MODULE_SMI_LARB0, -1, 0},
	{DISP_MODULE_SMI_LARB1, -1, 0},
	{DISP_MODULE_MIPI0, -1, 0},
	{DISP_MODULE_MIPI1, -1, 0},
	{DISP_MODULE_RSZ0, 17, 0},
	{DISP_MODULE_RSZ0_VIRTUAL0, -1, 0},
	{DISP_MODULE_RSZ0_VIRTUAL1, -1, 0},
	{DISP_MODULE_UNKNOWN, -1, 0},
};

/* module can be connect if 1 */
static struct module_map_s module_can_connect[DISP_MODULE_NUM] = {
	{DISP_MODULE_OVL0, 1, 0},
	{DISP_MODULE_OVL0_2L, 1, 0},
	{DISP_MODULE_RDMA0, 1, 0},
	{DISP_MODULE_WDMA0, 1, 0},
	{DISP_MODULE_WDMA_VIRTUAL0, 0, 0},
	{DISP_MODULE_WDMA_VIRTUAL1, 0, 0},
	{DISP_MODULE_COLOR0, 1, 0},
	{DISP_MODULE_CCORR0, 1, 0},
	{DISP_MODULE_AAL0, 1, 0},
	{DISP_MODULE_GAMMA0, 1, 0},
	{DISP_MODULE_DITHER0, 1, 0},
	{DISP_MODULE_DSI0, 1, 0},
	{DISP_MODULE_DSI1, 0, 0},
	{DISP_MODULE_DSIDUAL, 0, 0},
	{DISP_MODULE_PWM0, 0, 0},
	{DISP_MODULE_CONFIG, 0, 0},
	{DISP_MODULE_MUTEX, 0, 0},
	{DISP_MODULE_SMI_COMMON, 0, 0},
	{DISP_MODULE_SMI_LARB0, 0, 0},
	{DISP_MODULE_SMI_LARB1, 0, 0},
	{DISP_MODULE_MIPI0, 0, 0},
	{DISP_MODULE_MIPI1, 0, 0},
	{DISP_MODULE_RSZ0, 1, 0},
	{DISP_MODULE_RSZ0_VIRTUAL0, 1, 0},
	{DISP_MODULE_RSZ0_VIRTUAL1, 1, 0},
	{DISP_MODULE_UNKNOWN, 0, 0},
};



char *ddp_get_scenario_name(enum DDP_SCENARIO_ENUM scenario)
{
	switch (scenario) {
	/* primary display */
	case DDP_SCENARIO_PRIMARY_DISP:
		return "primary_disp";
	case DDP_SCENARIO_PRIMARY_BYPASS_PQ_DISP:
		return "primary_bypass_pq";
	case DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP:
		return "primary_rdma0_color0_disp";
	case DDP_SCENARIO_PRIMARY_RDMA0_DISP:
		return "primary_rdma0_disp";
	case DDP_SCENARIO_PRIMARY_OVL_MEMOUT:
		return "primary_ovl_memout";
	case DDP_SCENARIO_PRIMARY_ALL:
		return "primary_all";

	/* sub display */
	case DDP_SCENARIO_SUB_DISP:
		return "sub_disp";
	case DDP_SCENARIO_SUB_RDMA1_DISP:
		return "sub_rdma1_disp";
	case DDP_SCENARIO_SUB_OVL_MEMOUT:
		return "sub_ovl_memout";
	case DDP_SCENARIO_SUB_ALL:
		return "sub_all";
	/* others */
	default:
		DDPMSG("invalid scenario id=%d\n", scenario);
		return "unknown";
	}
}


char *ddp_get_mutex_sof_name(unsigned int regval)
{
	if (regval == SOF_VAL_MUTEX0_SOF_SINGLE_MODE)
		return "single";
	else if (regval == SOF_VAL_MUTEX0_SOF_FROM_DSI0)
		return "dsi0";
	else if (regval == SOF_VAL_MUTEX0_SOF_FROM_DPI)
		return "dpi";

	DDPDUMP("%s, unknown reg=%d\n", __func__, regval);
	return "unknown";
}

char *ddp_get_mode_name(enum DDP_MODE ddp_mode)
{
	switch (ddp_mode) {
	case DDP_VIDEO_MODE:
		return "vido_mode";
	case DDP_CMD_MODE:
		return "cmd_mode";
	default:
		DDPMSG("invalid ddp mode =%d\n", ddp_mode);
		return "unknown";
	}
}

static int ddp_get_module_num_l(int *module_list)
{
	unsigned int num = 0;

	while (*(module_list + num) != -1) {
		num++;

		if (num == DDP_ENING_NUM)
			break;
	}
	return num;
}

static int _ddp_check_path_l(int *module_list, int strict)
{
	unsigned int i, j, k;
	int step = 0;
	int valid = 0;
	unsigned int mout;
	unsigned int path_error = 0;
	unsigned int module_num = ddp_get_module_num_l(module_list);

	DDPMSG("check_path: %s to %s\n", ddp_get_module_name(module_list[0])
		, ddp_get_module_name(module_list[module_num - 1]));
	/* check mout */
	for (i = 0; i < module_num - 1; i++) {
		for (j = 0; j < DDP_MOUT_NUM; j++) {
			if (module_list[i] == mout_map[j].id) {
				mout = 0;
				/* find next module which can be connected */
				step = i + 1;
				while (module_can_connect[module_list[step]].bit == 0
				       && step < module_num) {
					step++;
				}
				ASSERT(step < module_num);
				for (k = 0; k < BIT_NUM; k++) {
					if (mout_map[j].out_id_bit_map[k].m == -1)
						break;
					if (mout_map[j].out_id_bit_map[k].m == module_list[step]) {
						mout |= mout_map[j].out_id_bit_map[k].v;
						valid = 1;
						break;
					}
				}
				if (valid) {
					valid = 0;
					if ((DISP_REG_GET(mout_map[j].reg) & mout) == 0) {
						path_error += 1;
						DDPERR("error:%s mout, expect=0x%x, real=0x%x\n",
							ddp_get_module_name(module_list[i]),
							mout, DISP_REG_GET(mout_map[j].reg));
					} else if (DISP_REG_GET(mout_map[j].reg) != mout) {
						DDPERR
						    ("warning: %s mout expect=0x%x, real=0x%x\n",
						     ddp_get_module_name(module_list[i]), mout,
						     DISP_REG_GET(mout_map[j].reg));
					}
				}
				break;
			}
		}
	}
	/* check out select */
	for (i = 0; i < module_num - 1; i++) {
		for (j = 0; j < DDP_SEL_OUT_NUM; j++) {
			if (module_list[i] != sel_out_map[j].id)
				continue;
			/* find next module which can be connected */
			step = i + 1;
			while (module_can_connect[module_list[step]].bit == 0
			       && step < module_num) {
				step++;
			}
			ASSERT(step < module_num);
			for (k = 0; k < BIT_NUM; k++) {
				if (sel_out_map[j].id_bit_map[k] == -1)
					break;
				if (sel_out_map[j].id_bit_map[k] == module_list[step]) {
					if (DISP_REG_GET(sel_out_map[j].reg) != k) {
						path_error += 1;
						DDPERR
						    ("error:out_s %s not connect to %s, expect=0x%x, real=0x%x\n",
						     ddp_get_module_name(module_list[i]),
						     ddp_get_module_name(module_list[step]),
						     k, DISP_REG_GET(sel_out_map[j].reg));
					}
					break;
				}
			}
		}
	}
	/* check input select */
	for (i = 1; i < module_num; i++) {
		for (j = 0; j < DDP_SEL_IN_NUM; j++) {
			if (module_list[i] != sel_in_map[j].id)
				continue;
			/* find next module which can be connected */
			step = i - 1;
			while (module_can_connect[module_list[step]].bit == 0 && step > 0)
				step--;
			ASSERT(step >= 0);
			for (k = 0; k < BIT_NUM; k++) {
				if (sel_in_map[j].id_bit_map[k] == -1)
					break;
				if (sel_in_map[j].id_bit_map[k] == module_list[step]) {
					if (DISP_REG_GET(sel_in_map[j].reg) != k) {
						path_error += 1;
						DDPERR("error:in_s %s not conn to %s,expect0x%x,real0x%x\n",
						     ddp_get_module_name(module_list[step]),
						     ddp_get_module_name(module_list[i]), k,
						     DISP_REG_GET(sel_in_map[j].reg));
					}
					break;
				}
			}
		}
	}
	if (path_error == 0) {
		DDPMSG("path: %s to %s is connected\n", ddp_get_module_name(module_list[0]),
			ddp_get_module_name(module_list[module_num - 1]));
	} else {
		DDPERR("path: %s to %s not connected!!!, path_err=%d\n", ddp_get_module_name(module_list[0]),
			ddp_get_module_name(module_list[module_num - 1]), path_error);
	}
	return path_error;
}

static int ddp_check_path_l(int *module_list)
{
	return _ddp_check_path_l(module_list, 0);
}

static int ddp_check_path_l_strict(int *module_list)
{
	return _ddp_check_path_l(module_list, 1);
}

static int ddp_get_mutex_src(enum DISP_MODULE_ENUM dest_module, enum DDP_MODE ddp_mode,
			     unsigned int *SOF_src, unsigned int *EOF_src)
{
	unsigned int src_from_dst_module = 0;

	if (dest_module == DISP_MODULE_WDMA0/* || dest_module == DISP_MODULE_WDMA1 */) {

		if (ddp_mode == DDP_VIDEO_MODE)
			DISP_LOG_W("%s: dst_mode=%s, but is video mode !!\n", __func__,
				   ddp_get_module_name(dest_module));

		*SOF_src = SOF_VAL_MUTEX0_SOF_SINGLE_MODE;
		*EOF_src = SOF_VAL_MUTEX0_EOF_DISABLE;
		return 0;
	}

	/* CASE2: *** -> DSI | *** -> DPI */
	if (dest_module == DISP_MODULE_DSI0 || dest_module == DISP_MODULE_DSIDUAL) {
		src_from_dst_module = SOF_VAL_MUTEX0_SOF_FROM_DSI0;
	/* } else if (dest_module == DISP_MODULE_DPI) {
		src_from_dst_module = SOF_VAL_MUTEX0_SOF_FROM_DPI;*/
	} else {
		DDPERR("get mutex sof, invalid param dst module = %s(%d), dis mode %s\n",
		       ddp_get_module_name(dest_module), dest_module, ddp_get_mode_name(ddp_mode));
	}

	if (ddp_mode == DDP_CMD_MODE) {
		*SOF_src = SOF_VAL_MUTEX0_SOF_SINGLE_MODE;
		if (0 /*disp_helper_get_option(DISP_OPT_MUTEX_EOF_EN_FOR_CMD_MODE)*/)
			*EOF_src = src_from_dst_module;
		else
			*EOF_src = SOF_VAL_MUTEX0_EOF_DISABLE;

	} else {
		*SOF_src = *EOF_src = src_from_dst_module;
	}

	return 0;
}

/* id: mutex ID, 0~5 */
static int ddp_mutex_set_l(int mutex_id, int *module_list, enum DDP_MODE ddp_mode, void *handle)
{
	int i = 0;
	unsigned int value0 = 0;
	unsigned int value1 = 0;
	unsigned int sof_val;
	unsigned int sof_src, eof_src;
	int module_num = ddp_get_module_num_l(module_list);

	ddp_get_mutex_src(module_list[module_num - 1], ddp_mode, &sof_src, &eof_src);
	if (mutex_id < DISP_MUTEX_DDP_FIRST || mutex_id > DISP_MUTEX_DDP_LAST) {
		DDPERR("exceed mutex max (0 ~ %d)\n", DISP_MUTEX_DDP_LAST);
		return -1;
	}
	for (i = 0; i < module_num; i++) {
		if (module_mutex_map[module_list[i]].bit != -1) {
			DDPDBG("module %s added to mutex %d\n", ddp_get_module_name(module_list[i]),
			       mutex_id);
			if (module_mutex_map[module_list[i]].mod_num == 0) {
				value0 |= (1 << module_mutex_map[module_list[i]].bit);
			} else if (module_mutex_map[module_list[i]].mod_num == 1) {
				/* DISP_MODULE_DSIDUAL is special */
				if (module_mutex_map[module_list[i]].module == DISP_MODULE_DSIDUAL) {
					value1 |= (1 << module_mutex_map[DISP_MODULE_DSI0].bit);
					/* DISP MODULE enum must start from 0 */
					value1 |= (1 << module_mutex_map[DISP_MODULE_DSI1].bit);
				} else {
					value1 |= (1 << module_mutex_map[module_list[i]].bit);
				}
			}
		} else {
			DDPDBG("module %s not added to mutex %d\n",
			      ddp_get_module_name(module_list[i]), mutex_id);
		}
	}

	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_MOD0(mutex_id), value0);
	//DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_MOD1(mutex_id), value1);

	sof_val = REG_FLD_VAL(SOF_FLD_MUTEX0_SOF, sof_src);
	sof_val |= REG_FLD_VAL(SOF_FLD_MUTEX0_EOF, eof_src);
	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_SOF(mutex_id), sof_val);

	DDPDBG("mutex %d value=0x%x, sof=%s, eof=%s\n", mutex_id,
	       value0, ddp_get_mutex_sof_name(sof_src), ddp_get_mutex_sof_name(eof_src));
	return 0;
}

static void ddp_check_mutex_l(int mutex_id, int *module_list, enum DDP_MODE ddp_mode)
{
	int i = 0;
	uint32_t real_value0 = 0;
	uint32_t real_value1 = 0;
	uint32_t expect_value0 = 0;
	uint32_t expect_value1 = 0;
	unsigned int real_sof, real_eof, val;
	unsigned int expect_sof, expect_eof;
	int module_num = ddp_get_module_num_l(module_list);

	if (mutex_id < DISP_MUTEX_DDP_FIRST || mutex_id > DISP_MUTEX_DDP_LAST) {
		DDPDUMP("error:check mutex fail:exceed mutex max (0 ~ %d)\n", DISP_MUTEX_DDP_LAST);
		return;
	}
	real_value0 = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD0(mutex_id));
	//real_value1 = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD1(mutex_id));
	for (i = 0; i < module_num; i++) {
		if (module_mutex_map[module_list[i]].bit != -1) {
			if (module_mutex_map[module_list[i]].mod_num == 0) {
				expect_value0 |= (1 << module_mutex_map[module_list[i]].bit);
			} else if (module_mutex_map[module_list[i]].mod_num == 1) {
				if (module_mutex_map[module_list[i]].module == DISP_MODULE_DSIDUAL) {
					expect_value1 |= (1 << module_mutex_map[DISP_MODULE_DSI0].bit);
					/* DISP MODULE enum must start from 0 */

					expect_value1 |= (1 << module_mutex_map[DISP_MODULE_DSI1].bit);
				} else {
					expect_value1 |= (1 << module_mutex_map[module_list[i]].bit);
				}
			}
		}
	}
	if (expect_value0 != real_value0)
		DDPDUMP("error:mutex %d error: expect0 0x%x, real0 0x%x\n", mutex_id, expect_value0,
			real_value0);

	if (expect_value1 != real_value1)
		DDPDUMP("error:mutex %d error: expect1 0x%x, real1 0x%x\n", mutex_id, expect_value1,
			real_value1);

	val = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(mutex_id));
	real_sof = REG_FLD_VAL_GET(SOF_FLD_MUTEX0_SOF, val);
	real_eof = REG_FLD_VAL_GET(SOF_FLD_MUTEX0_EOF, val);
	ddp_get_mutex_src(module_list[module_num - 1], ddp_mode, &expect_sof, &expect_eof);
	if (expect_sof != real_sof)
		DDPDUMP("error:mutex %d sof error: expect %s, real %s\n", mutex_id,
			ddp_get_mutex_sof_name(expect_sof), ddp_get_mutex_sof_name(real_sof));
	if (expect_eof != real_eof)
		DDPDUMP("error:mutex %d eof error: expect %s, real %s\n", mutex_id,
			ddp_get_mutex_sof_name(expect_eof), ddp_get_mutex_sof_name(real_eof));

}

int ddp_mutex_enable_l(int mutex_idx, void *handle)
{
	DDPDBG("mutex %d enable\n", mutex_idx);
	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_CFG, 0);
	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_EN(mutex_idx), 1);
#if 0	
	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_EN(mutex_idx), 1);

	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_GET(mutex_idx), 1);
	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_GET(mutex_idx), 0);
#endif
	return 0;
}

int ddp_get_module_num(enum DDP_SCENARIO_ENUM scenario)
{
	return ddp_get_module_num_l(module_list_scenario[scenario]);
}

static void ddp_print_scenario(enum DDP_SCENARIO_ENUM scenario)
{
	int i = 0;
	char path[512] = { '\0' };
	int num = ddp_get_module_num(scenario);
#if 0
	for (i = 0; i < num; i++)
		strncat(path, ddp_get_module_name(module_list_scenario[scenario][i]), sizeof(path));
	DDPMSG("scenario %s have modules: %s\n", ddp_get_scenario_name(scenario), path);
#endif
}


static int ddp_find_module_index(enum DDP_SCENARIO_ENUM ddp_scenario, enum DISP_MODULE_ENUM module)
{
	int i = 0;

	for (i = 0; i < DDP_ENING_NUM; i++) {
		if (module_list_scenario[ddp_scenario][i] == module)
			return i;

	}
	DDPDBG("find module: can not find module %s on scenario %s\n", ddp_get_module_name(module),
	       ddp_get_scenario_name(ddp_scenario));
	return -1;
}

/* set display interface when kernel init */
int ddp_set_dst_module(enum DDP_SCENARIO_ENUM scenario, enum DISP_MODULE_ENUM dst_module)
{
	int i = 0;

	DDPMSG("ddp_set_dst_module, scenario=%s, dst_module=%s\n",
	       ddp_get_scenario_name(scenario), ddp_get_module_name(dst_module));
	if (ddp_find_module_index(scenario, dst_module) > 0) {
		DDPDBG("%s is already on path\n", ddp_get_module_name(dst_module));
		return 0;
	}
	i = ddp_get_module_num_l(module_list_scenario[scenario]) - 1;
	ASSERT(i >= 0);
	if (dst_module == DISP_MODULE_DSIDUAL) {
		if (i < (DDP_ENING_NUM - 1)) {
			/* module_list_scenario[scenario][i++] = DISP_MODULE_SPLIT0; */
		} else {
			DDPERR("set dst module over up bound\n");
			return -1;
		}
	} else {
		if (ddp_get_dst_module(scenario) == DISP_MODULE_DSIDUAL) {
			if (i >= 1) {
				module_list_scenario[scenario][i--] = -1;
			} else {
				DDPERR("set dst module over low bound\n");
				return -1;
			}
		}
	}
	module_list_scenario[scenario][i] = dst_module;
	if (scenario == DDP_SCENARIO_PRIMARY_ALL)
		ddp_set_dst_module(DDP_SCENARIO_PRIMARY_DISP, dst_module);
	else if (scenario == DDP_SCENARIO_SUB_ALL)
		ddp_set_dst_module(DDP_SCENARIO_SUB_RDMA1_DISP, dst_module);

	ddp_print_scenario(scenario);
	return 0;
}

enum DISP_MODULE_ENUM ddp_get_dst_module(enum DDP_SCENARIO_ENUM ddp_scenario)
{
	enum DISP_MODULE_ENUM module_name = DISP_MODULE_UNKNOWN;
	int module_num = ddp_get_module_num_l(module_list_scenario[ddp_scenario]) - 1;

	if (module_num >= 0)
		module_name = module_list_scenario[ddp_scenario][module_num];

	return module_name;
}

int *ddp_get_scenario_list(enum DDP_SCENARIO_ENUM ddp_scenario)
{
	return module_list_scenario[ddp_scenario];
}

int ddp_is_module_in_scenario(enum DDP_SCENARIO_ENUM ddp_scenario, enum DISP_MODULE_ENUM module)
{
	int i = 0;

	for (i = 0; i < DDP_ENING_NUM; i++) {
		if (module_list_scenario[ddp_scenario][i] == module)
			return 1;
	}
	return 0;
}

void ddp_check_path(enum DDP_SCENARIO_ENUM scenario)
{
	DDPDBG("path check path on scenario %s\n", ddp_get_scenario_name(scenario));

	if (scenario == DDP_SCENARIO_PRIMARY_ALL) {
		ddp_check_path_l(module_list_scenario[DDP_SCENARIO_PRIMARY_DISP]);
		ddp_check_path_l(module_list_scenario[DDP_SCENARIO_PRIMARY_OVL_MEMOUT]);
	} else if (scenario == DDP_SCENARIO_SUB_ALL) {
		ddp_check_path_l(module_list_scenario[DDP_SCENARIO_SUB_DISP]);
		ddp_check_path_l(module_list_scenario[DDP_SCENARIO_SUB_OVL_MEMOUT]);
	} else {
		ddp_check_path_l(module_list_scenario[scenario]);
	}
}

int ddp_check_path_strict(enum DDP_SCENARIO_ENUM scenario)
{
    DDPMSG("path check path on scenario %s\n", ddp_get_scenario_name(scenario));

    if(scenario == DDP_SCENARIO_PRIMARY_ALL)
    {
		DDPAEE("%s, not support primary_all\n", __func__);
		return -1;
    }
    else if(scenario == DDP_SCENARIO_SUB_ALL)
    {
		DDPAEE("%s, not support sub_all\n", __func__);
		return -1;
    }
    else
    {
        return ddp_check_path_l_strict(module_list_scenario[scenario]);
    }
}

void ddp_check_mutex(int mutex_id, enum DDP_SCENARIO_ENUM scenario, enum DDP_MODE mode)
{
	DDPDBG("check mutex %d on scenario %s\n", mutex_id, ddp_get_scenario_name(scenario));
	ddp_check_mutex_l(mutex_id, module_list_scenario[scenario], mode);
}

int ddp_mutex_reg_backup(int mutex_id)
{
	disp_tui_reg_backup((void*)DISP_REG_CONFIG_MUTEX_MOD0(mutex_id));
	disp_tui_reg_backup((void*)DISP_REG_CONFIG_MUTEX_SOF(mutex_id));
	return 0;
}

int ddp_mutex_set(int mutex_id, enum DDP_SCENARIO_ENUM scenario, enum DDP_MODE mode, void *handle)
{
	if (scenario < DDP_SCENARIO_MAX) {
		return ddp_mutex_set_l(mutex_id, module_list_scenario[scenario], mode, handle);
	} else {
		DDPERR("Invalid scenario %d when setting mutex\n", scenario);
		return -1;
	}
	return 0;
}

int ddp_is_moudule_in_mutex(int mutex_id, enum DISP_MODULE_ENUM module)
{
	int ret = 0;
	uint32_t real_value = 0;

	if (mutex_id < DISP_MUTEX_DDP_FIRST || mutex_id > DISP_MUTEX_DDP_LAST) {
		DDPDUMP("error:check_moudule_in_mute fail:exceed mutex max (0 ~ %d)\n",
			DISP_MUTEX_DDP_LAST);
		return ret;
	}

	if (module_mutex_map[module].mod_num == 0)
		real_value = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD0(mutex_id));
	else if (module_mutex_map[module].mod_num == 1)
		real_value = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD0(mutex_id));

	return ret;
}

int ddp_mutex_enable(int mutex_id, enum DDP_SCENARIO_ENUM scenario, void *handle)
{
	return ddp_mutex_enable_l(mutex_id, handle);
}

int ddp_mutex_disable(int mutex_id, enum DDP_SCENARIO_ENUM scenario, void *handle)
{
	DDPDBG("mutex %d disable\n", mutex_id);
	DISP_REG_SET(handle, DISP_REG_CONFIG_MUTEX_EN(mutex_id), 0);
	return 0;
}

int disp_get_dst_module(enum DDP_SCENARIO_ENUM scenario)
{
	return ddp_get_dst_module(scenario);
}
