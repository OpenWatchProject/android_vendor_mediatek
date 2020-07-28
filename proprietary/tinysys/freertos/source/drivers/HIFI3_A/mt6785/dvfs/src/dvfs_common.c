/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <dvfs_common.h>

#define PLL_EN            (0x1 << 0)
#define PLL_PWR_ON        (0x1 << 0)
#define PLL_ISO_EN        (0x1 << 1)
#define ADSPPLL_DIV_RSTB  (0x1 << 23)

uint32_t g_adsppll_status INTERNAL_INITIAL_DATA = 1;
uint32_t g_adsppll_unlock INTERNAL_INITIAL_DATA = 0;

uint32_t g_force_adsppll_on INTERNAL_INITIAL_DATA = 0;
uint32_t g_sleep_tick_us INTERNAL_INITIAL_DATA = 0;

/* Turn on/ off ADSPPLL when AP suspend
 * @input freq: has no used in MT6779
 */
void adsp_setup_adsppll(uint32_t adsp_enable)
{
    if (adsp_enable) {
        /* enable ADSPPLL */
        drv_set_reg32(ADSPPLL_PWR_CON0, PLL_PWR_ON);
        udelay(30);
        drv_clr_reg32(ADSPPLL_PWR_CON0, PLL_ISO_EN);
        udelay(1);
        drv_set_reg32(ADSPPLL_CON0, PLL_EN);
        udelay(20);
        drv_set_reg32(ADSPPLL_CON0, ADSPPLL_DIV_RSTB);
        g_adsppll_status = 1;
    } else {
        drv_clr_reg32(ADSPPLL_CON0, ADSPPLL_DIV_RSTB);
        drv_clr_reg32(ADSPPLL_CON0, PLL_EN);
        drv_set_reg32(ADSPPLL_PWR_CON0, PLL_ISO_EN);
        drv_clr_reg32(ADSPPLL_PWR_CON0, PLL_PWR_ON);
        g_adsppll_status = 0;
    }
}
