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

#define LOG_TAG "ddp_drv"
#include "ddp_drv.h"

/* device and driver */
/*volatile unsigned int dispsys_irq[DISP_REG_NUM] = {0};*/
volatile unsigned long dispsys_reg[DISP_REG_NUM] = {0};
/*volatile unsigned long mipi_tx0_reg = 0;*/
/*volatile unsigned long mipi_tx1_reg = 0;*/
/*volatile unsigned long dsi_reg_va = 0;*/
/*volatile unsigned long dsi_reg_va[2] = { 0 };*/

/* from DTS, for debug */
unsigned long ddp_reg_pa_base[DISP_REG_NUM] = {
	0x14000000,	/* CONFIG, mmsys_config_base */
	0x1400b000,	/* OVL0, disp_ovl0 */
	0x1400c000,	/* OVL1, disp_ovl1 */
	0x1400d000,	/* OVL0_2L, disp_ovl0_2l */
	0x1400e000,	/* OVL1_2L, disp_ovl1_2l */
	0x1400f000,	/* RDMA0, disp_rdma0 */
	0x14010000,	/* RDMA1, disp_rdma1 */
	0x14011000,	/* RDMA2, disp_rdma2 */
	0x14012000,	/* WDMA0, disp_wdma0 */
	0x14013000,	/* WDMA1, disp_wdma1 */
	0x14014000,	/* COLOR0, disp_color0*/
	0x14015000,	/* COLOR1, disp_color1*/
	0x14016000,	/* CCORR0, disp_ccorr0 */
	0x14017000,	/* CCORR1, disp_ccorr1 */
	0x14018000,	/* AAL0, disp_aal0*/
	0x14019000,	/* AAL1, disp_aal1*/
	0x1401a000,	/* GAMMA0, disp_gamma0 */
	0x1401b000,	/* GAMMA1, disp_gamma1 */
	0x1401c000,	/* OD, disp_od */
	0x1401d000,	/* DITHER0, disp_dither0 */
	0x1401e000,	/* DITHER1, disp_dither1 */
	0x1401f000,	/* UFOE, dsi_ufoe */
	0x14020000,	/* DSC, disp_dsc */
	0x14021000,	/* SPLIT0, dsi_split */
	0x14022000,	/* DSI0, dsi0 */
	0x14023000,	/* DSI1, dsi1 */
	0x14024000,	/* DPI0, dpi0 */
	0x1100e000,	/* PWM, disp_pwm */
	0x14025000,	/* MUTEX, mm_mutex */
	0x14026000,	/* SMI_LARB0 */
	0x14027000,	/* SMI_LARB4 */
	0x14022000,	/* SMI_COMMON, disp_smi_commom */
	0x10215000,	/* MIPITX0*/
	0x10216000	/*MIPITX1*/
};

int disp_reg_init(void)
{
	int ret;
	int i;
    static unsigned int disp_probe_cnt = 0;

    if(disp_probe_cnt!=0)
    {
		disp_probe_cnt = 1;
        return 0;
    }

    /* iomap registers */
    for(i=0;i<DISP_REG_NUM;i++)
    {
		drApiResult_t ret;
		/* remap registers */

		ret = drApiMapPhysicalBuffer(ddp_reg_pa_base[i], 0x1000,
				MAP_HARDWARE, &dispsys_reg[i]);

		if(ret != DRAPI_OK)
			DDPAEE("map reg fail: pa=0x%x, size=0x%x, flag=0x%x, ret=%d(0x%x)\n",
				ddp_reg_pa_base[i], 0x1000, MAP_HARDWARE, ret, ret);

        DDPDBG("reg_map%d module=%s, map_addr=%p, reg_pa=0x%x\n",
            i, ddp_get_reg_module_name(i), dispsys_reg[i], ddp_reg_pa_base[i]);
    }

	ddp_path_init();
	ddp_dsi_init();
	DDPDBG("dispsys probe done.\n");
	return 0;
}


