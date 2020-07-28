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

#include <FreeRTOS.h>
#include <platform_mtk.h>
#include <task.h>
#include <mpu_mtk.h>
#ifdef CFG_CACHE_SUPPORT
#include <cache_internal.h>
#endif

extern UBaseType_t uxTaskGetEndOfStack(TaskHandle_t xTask);

mpu_region_t mpu_region_table_mtk[MAX_NUM_REGIONS] = {
    /*0: total memory0x20000000*/
    {
        0x0, MPU_SIZE_4GB, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_DISABLE, MPU_B_DISABLE,
        MPU_S_DISABLE, MPU_DISABLE
    },
    /*1: Out of SRAM*/
    {
        0x0, MPU_SIZE_512MB, MPU_AP_PRI_NO_ACCESS_UNPRI_NO_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE,
        MPU_S_DISABLE, MPU_DISABLE
    },
    /*2: MPU_ID_SRAM_0*/
    {
        0x0, MPU_SIZE_512KB, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE, MPU_S_DISABLE,
        MPU_DISABLE
    },
    /*3: MPU_ID_SRAM_1*/
    {
#if defined(MT6785)
        0x80000, MPU_SIZE_256KB, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE, MPU_S_DISABLE,
        MPU_DISABLE
#else
        0x0, 0, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE, MPU_S_DISABLE,
        MPU_DISABLE
#endif  // defined(MT6785)
    },
    /*4: MPU_ID_DRAM_0*/
    {
#ifdef CFG_CACHE_SUPPORT
        L1C_DRAM_ADDR_0, L1C_DRAM_SIZE_0, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE,
        MPU_S_ENABLE, MPU_DISABLE
#else
        0x0, 0, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE, MPU_S_ENABLE,
        MPU_DISABLE
#endif
    },
    /*5: MPU_ID_DRAM_1*/
    {
#ifdef CFG_CACHE_SUPPORT
        L1C_DRAM_ADDR_1, L1C_DRAM_SIZE_1, MPU_AP_PRI_RW_ACCESS_UNPRI_RW_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE,
        MPU_S_ENABLE, MPU_DISABLE
#else
        0x0, 0, MPU_AP_PRI_NO_ACCESS_UNPRI_NO_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE, MPU_S_ENABLE,
        MPU_DISABLE
#endif
    },
    /*6: MPU*/
    {
        0xE000ED80, MPU_SIZE_64B, MPU_AP_PRI_RO_ACCESS_UNPRI_NO_ACCESS, MPU_TEX, MPU_C_DISABLE, MPU_B_DISABLE,
        MPU_S_DISABLE, MPU_DISABLE
    },
    /*7: Loader*/
    {
#if defined(CFG_DVT_SUPPORT) || defined(CFG_SLT_SUPPORT)
        0x0, 0, MPU_AP_PRI_NO_ACCESS_UNPRI_NO_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE,
        MPU_S_DISABLE, MPU_DISABLE
#else
        0x0, MPU_SIZE_1KB, MPU_AP_PRI_NO_ACCESS_UNPRI_NO_ACCESS, MPU_TEX, MPU_C_ENABLE, MPU_B_DISABLE,
        MPU_S_DISABLE, MPU_DISABLE
#endif
    },
};

void enable_mpu_region(uint32_t region_num)
{
    if (region_num >= MAX_NUM_REGIONS) {
        PRINTF_E("Wrong region #%d\n", (int)region_num);
        return;
    }
    MPU->RNR = region_num;
    MPU->RASR |= MPU_RASR_ENABLE_Msk;
    mpu_region_table_mtk[region_num].enable = MPU_ENABLE;
    __DSB();
    __ISB();
}

void disable_mpu_region(uint32_t region_num)
{
    if (region_num >= MAX_NUM_REGIONS) {
        PRINTF_E("Wrong region #%d\n", (int)region_num);
        return;
    }
    MPU->RNR = region_num;
    MPU->RASR &= ~MPU_RASR_ENABLE_Msk;
    mpu_region_table_mtk[region_num].enable = MPU_DISABLE;
    __DSB();
    __ISB();
}

void setup_mpu_region(uint32_t region_num)
{
    uint32_t region_attribute;
    mpu_region_t region_setting = mpu_region_table_mtk[region_num];

    if (region_num >= MAX_NUM_REGIONS) {
        PRINTF_E("Wrong region #%d\n", (int)region_num);
        return;
    }

    if (region_setting.enable == MPU_ENABLE) {
        PRINTF_E("MPU-%d was enabled, disable it first.\n", (int)region_num);
        return;
    }

    MPU->RNR = region_num;
    MPU->RBAR = region_setting.base_address;
    region_attribute = ((region_setting.size << MPU_RASR_SIZE_Pos) & MPU_RASR_SIZE_Msk) | \
                       ((region_setting.access_permission << MPU_RASR_AP_Pos) & MPU_RASR_AP_Msk) | \
                       ((region_setting.TEX << MPU_RASR_TEX_Pos) & MPU_RASR_TEX_Msk) | \
                       ((region_setting.C << MPU_RASR_C_Pos) & MPU_RASR_C_Msk) | \
                       ((region_setting.B << MPU_RASR_B_Pos) & MPU_RASR_B_Msk) | \
                       ((region_setting.S << MPU_RASR_S_Pos) & MPU_RASR_S_Msk) ;

    MPU->RASR = region_attribute;
    PRINTF_D("MPU-%d region setting:0x%x, 0x%x\n", (int)region_num, (unsigned int)region_setting.base_address, (unsigned int)region_attribute);
}

void dump_mpu_status(void)
{
    uint32_t regions;
    uint32_t ix;

    regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
    PRINTF_D("TYPE\tCTRL\t\n");
    PRINTF_D("0x%x\t0x%x\t\n", (unsigned int)MPU->TYPE, (unsigned int)MPU->CTRL);
    PRINTF_D("RNR\tRBAR\t\tRASR\n");
    for (ix = 0; ix < regions; ix++) {
        MPU->RNR = ix;
        PRINTF_D("0x%x\t0x%08x\t0x%08x\n", (unsigned int)MPU->RNR, (unsigned int)MPU->RBAR, (unsigned int)MPU->RASR);
    }
}

int32_t request_mpu_region(void)
{
    uint32_t max_regions;
    uint32_t region_num = -1;
    uint32_t ix;

    max_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
    for (ix = 0; ix < max_regions; ix++) {
        if (mpu_region_table_mtk[ix].enable == MPU_DISABLE) {
            region_num = ix;
            break;
        }
    }
    return region_num;
}

void enable_mpu_ctrl(void)
{
    MPU->CTRL = 0x1;
    PRINTF_D("0x%x\t0x%x\t\n", (unsigned int)MPU->TYPE, (unsigned int)MPU->CTRL);
}

void disable_mpu_ctrl(void)
{
    MPU->CTRL = 0x0;
    PRINTF_D("0x%x\t0x%x\t\n", (unsigned int)MPU->TYPE, (unsigned int)MPU->CTRL);
}

void mpu_init(void)
{
    PRINTF_E("Init MPU\n");
    int i;

    /* set the settings of the mpu table
     * and then enable the corresponding regions
     */
    for (i = 0; i < MAX_NUM_REGIONS; i++) {
        if (mpu_region_table_mtk[i].size != 0 ) {
            setup_mpu_region(i);
            enable_mpu_region(i);
        }
    }

    /* enable mpu controller */
    enable_mpu_ctrl();
    dump_mpu_status();

    __DSB();
    __ISB();
}


/*
int32_t stack_protector(TaskHandle_t task_handler)
{
    uint32_t stack_end_address;
    uint32_t region_num;
    mpu_region_t region_setting;

    region_num = request_mpu_region();

    if (region_num < 0 || region_num >= MAX_NUM_REGIONS) {
        PRINTF_E("Wrong region #%d\n", region_num);
        return -1;
    }
    stack_end_address = uxTaskGetEndOfStack(task_handler);
    PRINTF_D("stack address:0x%x\n", stack_end_address);
    region_setting.base_address = stack_end_address & (~0x1fUL); //MPU has minimum 32byte limitation
    region_setting.size = MPU_SIZE_32B;
    region_setting.access_permission = MPU_AP_PRI_RO_ACCESS_UNPRI_RO_ACCESS;
    region_setting.TEX = MPU_TEX;
    region_setting.C = MPU_C_ENABLE;
    region_setting.B = MPU_B_DISABLE;
    region_setting.S = MPU_S_ENABLE;
    setup_mpu_region(region_num, region_setting);
    enable_mpu_region(region_num);

    return 0;
}
*/

void enable_dram_protector(void)
{
    /* depreciated API */
    return ;
}

void disable_dram_protector(void)
{
    /* depreciated API */
    return ;
}

