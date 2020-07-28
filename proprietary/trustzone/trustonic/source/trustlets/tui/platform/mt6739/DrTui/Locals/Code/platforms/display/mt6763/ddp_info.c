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

#define LOG_TAG "INFO"
#include"ddp_info.h"
#include"ddp_debug.h"
#include "ddp_log.h"

char *ddp_get_module_name(enum DISP_MODULE_ENUM module)
{
	switch (module) {
	case DISP_MODULE_OVL0:
		return "ovl0 ";
	case DISP_MODULE_OVL1:
		return "ovl1 ";
	case DISP_MODULE_OVL0_2L:
		return "ovl0_2l ";
	case DISP_MODULE_OVL1_2L:
		return "ovl1_2l ";
	case DISP_MODULE_RDMA0:
		return "rdma0 ";
	case DISP_MODULE_RDMA1:
		return "rdma1 ";
	case DISP_MODULE_RDMA2:
		return "rdma2 ";
	case DISP_MODULE_WDMA0:
		return "wdma0 ";
	case DISP_MODULE_WDMA1:
		return "wdma1 ";
	case DISP_MODULE_COLOR0:
		return "color0 ";
	case DISP_MODULE_COLOR1:
		return "color1 ";
	case DISP_MODULE_CCORR0:
		return "ccorr0 ";
	case DISP_MODULE_CCORR1:
		return "ccorr1 ";
	case DISP_MODULE_AAL0:
		return "aal0 ";
	case DISP_MODULE_AAL1:
		return "aal1 ";
	case DISP_MODULE_GAMMA0:
		return "gamma0 ";
	case DISP_MODULE_GAMMA1:
		return "gamma1 ";
	case DISP_MODULE_OD:
		return "od ";
	case DISP_MODULE_DITHER0:
		return "dither0 ";
	case DISP_MODULE_DITHER1:
		return "dither1 ";
	case DISP_MODULE_PATH0:
		return "path0 ";
	case DISP_MODULE_PATH1:
		return "path1 ";
	case DISP_MODULE_UFOE:
		return "ufoe ";
	case DISP_MODULE_DSC:
		return "dsc ";
	case DISP_MODULE_SPLIT0:
		return "split0 ";
	case DISP_MODULE_DPI:
		return "dpi ";
	case DISP_MODULE_DSI0:
		return "dsi0 ";
	case DISP_MODULE_DSI1:
		return "dsi1 ";
	case DISP_MODULE_DSIDUAL:
		return "dsidual ";
	case DISP_MODULE_PWM0:
		return "pwm0 ";
	case DISP_MODULE_PWM1:
		return "pwm1 ";
	case DISP_MODULE_CONFIG:
		return "config ";
	case DISP_MODULE_MUTEX:
		return "mutex ";
	case DISP_MODULE_SMI_COMMON:
		return "smi_common ";
	case DISP_MODULE_SMI_LARB0:
		return "smi_larb0 ";
	case DISP_MODULE_SMI_LARB1:
		return "smi_larb1 ";
	case DISP_MODULE_MIPI0:
		return "mipi0 ";
	case DISP_MODULE_MIPI1:
		return "mipi1";
	case DISP_MODULE_RSZ0:
		return "rsz0";
	case DISP_MODULE_RSZ1:
		return "rsz1";
	case DISP_MODULE_OVL0_VIRTUAL:
		return "ovl0_virtual ";
	case DISP_MODULE_OVL0_2L_VIRTUAL:
		return "ovl0_2l_virtual ";
	case DISP_MODULE_OVL1_2L_VIRTUAL:
		return "ovl1_2l_virtual ";
	default:
		DDPMSG("invalid module id=%d", module);
		return "unknown";
	}
}

enum DISP_MODULE_ENUM ddp_get_reg_module(enum DISP_REG_ENUM reg_module)
{
	switch (reg_module) {
	case DISP_REG_CONFIG:
		return DISP_MODULE_CONFIG;
	case DISP_REG_OVL0:
		return DISP_MODULE_OVL0;
	case DISP_REG_OVL0_2L:
		return DISP_MODULE_OVL0_2L;
	case DISP_REG_OVL1_2L:
		return DISP_MODULE_OVL1_2L;
	case DISP_REG_RDMA0:
		return DISP_MODULE_RDMA0;
	case DISP_REG_RDMA1:
		return DISP_MODULE_RDMA1;
	case DISP_REG_WDMA0:
		return DISP_MODULE_WDMA0;
	case DISP_REG_COLOR0:
		return DISP_MODULE_COLOR0;
	case DISP_REG_CCORR0:
		return DISP_MODULE_CCORR0;
	case DISP_REG_AAL0:
		return DISP_MODULE_AAL0;
	case DISP_REG_GAMMA0:
		return DISP_MODULE_GAMMA0;
	case DISP_REG_DITHER0:
		return DISP_MODULE_DITHER0;
	case DISP_REG_UFOE:
		return DISP_MODULE_UFOE;
	case DISP_REG_SPLIT0:
		return DISP_MODULE_SPLIT0;
	case DISP_REG_DSI0:
		return DISP_MODULE_DSI0;
	case DISP_REG_DPI0:
		return DISP_MODULE_DPI;
	case DISP_REG_PWM0:
		return DISP_MODULE_PWM0;
	case DISP_REG_MUTEX:
		return DISP_MODULE_MUTEX;
	case DISP_REG_SMI_LARB0:
		return DISP_MODULE_SMI_LARB0;
	case DISP_REG_SMI_LARB1:
		return DISP_MODULE_SMI_LARB1;
	case DISP_REG_SMI_COMMON:
		return DISP_MODULE_SMI_COMMON;
	case DISP_REG_RSZ0:
		return DISP_MODULE_RSZ0;
	case DISP_REG_MIPI0:
		return DISP_MODULE_MIPI0;
	case DISP_REG_MIPI1:
		return DISP_MODULE_MIPI1;
	default:
		DDPERR("%s: invalid reg module id=%d\n", __func__, reg_module);
		return DISP_MODULE_UNKNOWN;
	}

}

char *ddp_get_reg_module_name(enum DISP_REG_ENUM reg_module)
{
	return ddp_get_module_name(ddp_get_reg_module(reg_module));
}

unsigned int ddp_module_to_idx(int module)
{
	unsigned int id = 0;

	switch (module) {
	case DISP_MODULE_AAL0:
	case DISP_MODULE_COLOR0:
	case DISP_MODULE_RDMA0:
	case DISP_MODULE_WDMA0:
	case DISP_MODULE_OVL0:
	case DISP_MODULE_GAMMA0:
	case DISP_MODULE_PWM0:
	case DISP_MODULE_OD:
	case DISP_MODULE_SPLIT0:
	case DISP_MODULE_DSI0:
	case DISP_MODULE_DPI:
	case DISP_MODULE_DITHER0:
	case DISP_MODULE_CCORR0:
		id = 0;
		break;
	case DISP_MODULE_AAL1:
	case DISP_MODULE_COLOR1:
	case DISP_MODULE_GAMMA1:
	case DISP_MODULE_DITHER1:
	case DISP_MODULE_CCORR1:
	case DISP_MODULE_RDMA1:
	case DISP_MODULE_DSI1:
	case DISP_MODULE_OVL1:
	case DISP_MODULE_WDMA1:
	case DISP_MODULE_PWM1:
		id = 1;
		break;
	case DISP_MODULE_RDMA2:
	case DISP_MODULE_DSIDUAL:
		id = 2;
		break;
	default:
		DDPERR("ddp_module_to_idx, module=0x%x\n", module);
	}

	return id;
}

