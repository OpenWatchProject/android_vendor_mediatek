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
volatile unsigned long mipi_tx0_reg = 0;
volatile unsigned long mipi_tx1_reg = 0;
volatile unsigned long dsi_reg_va = 0;

/* from DTS, for debug */
unsigned long ddp_reg_pa_base[DISP_REG_NUM] = {
	0x14000000,	/* CONFIG */
	0x1400b000,	/* OVL0 */
	0x1400c000,	/* OVL0_2L */
	0x1400d000,	/* RDMA0 */
	0x1400e000,	/* WDMA0 */
	0x1400f000,	/* COLOR0 */
	0x14010000,	/* CCORR0 */
	0x14011000,	/* AAL0 */
	0x14012000,	/* GAMMA0 */
	0x14013000,	/* DITHER0 */
	0x14014000,	/* DSI0 */
	0x14016000,	/* DSI1 */
	0x14015000, /* RSZ0 */
	0x1100e000,	/* PWM0*/
	0x14001000,	/* MUTEX*/
	0x14003000,	/* SMI_LARB0 */
	0x16010000,	/* SMI_LARB1 */
	0x14002000,	/* SMI_COMMON */
	0x11c80000,	/* MIPITX0*/
	0x19000000	/*MIPITX1*/
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
				MAP_HARDWARE, (void **)&dispsys_reg[i]);

		if(ret != DRAPI_OK)
			DDPAEE("map reg fail: pa=0x%x, size=0x%x, flag=0x%x, ret=%d(0x%x)\n",
				ddp_reg_pa_base[i], 0x1000, MAP_HARDWARE, ret, ret);

        DDPMSG("reg_map%d module=%s, map_addr=%p, reg_pa=0x%x\n",
            i, ddp_get_reg_module_name(i), dispsys_reg[i], ddp_reg_pa_base[i]);
    }

	ddp_path_init();
	ddp_dsi_reg_init();
	DDPMSG("dispsys probe done.\n");
	return 0;
}

int disp_get_version(void)
{
	if (!rdma_is_mem_mode(DISP_MODULE_RDMA0)){
		return MULTI_WINDOWS_TUI;
	}

	return SINGLE_WINDOWS_TUI;
}
