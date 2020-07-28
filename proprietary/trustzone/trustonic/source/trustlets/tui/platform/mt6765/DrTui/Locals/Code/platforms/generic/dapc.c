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

#define TAG "[dapc]"

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "mt_typedefs.h"
#include "dapc.h"
#include "log.h"
#include "tlDapcIndex.h"

unsigned char *devapc_ao_infra_peri_vaddr = 0;
unsigned char *devapc_ao_mm_vaddr = 0;
unsigned char *devapc_ao_md_vaddr = 0;

static inline unsigned int readl(volatile unsigned int* addr)
{
    return *(volatile unsigned int*)addr;
}

static inline void writel(unsigned int val, volatile unsigned int* addr)
{
    *(volatile unsigned int*)addr = val;
}

int set_module_apc(unsigned int module, E_MASK_DOM domain_num, APC_ATTR permission_control)
{
    volatile unsigned int *base = NULL;
    unsigned int slave_type = (module & 0xF0000000);
    unsigned int real_module_index = (module & 0x0FFFFFFF);
    unsigned int clr_bit = 0xFFFFFFFF ^ (0x3 << ((real_module_index % MOD_NO_IN_1_DEVAPC) * 2));
    unsigned int set_bit = permission_control << ((real_module_index % MOD_NO_IN_1_DEVAPC) * 2);

    /* Do the domain boundary check */
    if ((permission_control >= E_MAX_APC_ATTR) || (permission_control < E_L0)) {
    	return -4;
    }

    if (slave_type == SLAVE_TYPE_PREFIX_INFRA_PERI) {
        if ((domain_num >= E_MAX_DOMAIN) || (domain_num < E_DOMAIN_0)) {
            return -1;
        }
    } else if ((slave_type == SLAVE_TYPE_PREFIX_MM) || (slave_type == SLAVE_TYPE_PREFIX_MD)) {
        if ((domain_num > E_DOMAIN_3) || (domain_num < E_DOMAIN_0)) {
            return -1;
        }
    } else {
        return -2;
    }

    if (slave_type == SLAVE_TYPE_PREFIX_INFRA_PERI) {
        if (real_module_index > SLAVE_INFRA_PERI_MAX_INDEX) {
            return -3;
        }
        base = (volatile unsigned int *) ((unsigned int)DEVAPC0_D0_APC_0 + (domain_num * 0x100) + ((real_module_index / MOD_NO_IN_1_DEVAPC) * 4));
    } else if (slave_type == SLAVE_TYPE_PREFIX_MM) {
        if (real_module_index > SLAVE_MM_MAX_INDEX) {
            return -3;
        }
        base = (volatile unsigned int *) ((unsigned int)DEVAPC0_MM_D0_APC_0 + (domain_num * 0x100) + ((real_module_index / MOD_NO_IN_1_DEVAPC) * 4));
    } else if (slave_type == SLAVE_TYPE_PREFIX_MD) {
        if (real_module_index > SLAVE_MD_MAX_INDEX) {
            return -3;
        }
        base = (volatile unsigned int *) ((unsigned int)DEVAPC0_MD_D0_APC_0 + (domain_num * 0x100) + ((real_module_index / MOD_NO_IN_1_DEVAPC) * 4));
    } else {
        return -2;
    }

    if (base != NULL) {
        writel(readl(base) & clr_bit, base);
        writel(readl(base) | set_bit, base);
    }

    return 0;
}

int set_master_transaction(unsigned int master_index, unsigned int transaction_type)
{
    volatile unsigned int *base = NULL;
    unsigned int set_bit = 0;
    unsigned int master_register_index = 0;
    unsigned int master_set_index = 0;
    unsigned int master_type = (master_index & 0xF0000000);
    unsigned int real_master_index = (master_index & 0x0FFFFFFF);

    master_register_index = real_master_index / (MOD_NO_IN_1_DEVAPC * 2);
    master_set_index = real_master_index % (MOD_NO_IN_1_DEVAPC * 2);

    if (master_type == MASTER_TYPE_PREFIX_INFRA_PERI) {
        if (real_master_index > MASTER_INFRA_PERI_MAX_INDEX) {
            return -1;
        }

        base = (volatile unsigned int *) ((unsigned int)DEVAPC0_MAS_SEC_0 + master_register_index * 4);
    } else {
        return -3;
    }

    if (base != NULL) {
        if (transaction_type == 0) {
            set_bit = 0xFFFFFFFF ^ (1 << master_set_index);
            writel(readl(base) & set_bit, base);
        } else if (transaction_type == 1) {
            set_bit = 1 << master_set_index;
            writel(readl(base) | set_bit, base);
        } else {
            return -2;
        }

        drDbgPrintLnf("T[DEVAPC] (Infra)MAS_SEC_0 = %x\n", readl(DEVAPC0_MAS_SEC_0));
    }

    return 0;
}

int map_DAPC_Register()
{
    drApiResult_t ret;

    if((ret = drApiMapPhysicalBuffer((uint64_t)DEVAPC_AO_INFRA_PERI_BASE, SIZE_4KB, MAP_HARDWARE, (void **)&DEVAPC_AO_INFRA_PERI_VA_BUFFER)) != DRAPI_OK){
#ifdef FIX_BUILD_ERROR
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_INFRA_PERI_BASE failed! ERROR: %d, phy addr : 0x%llx\n", ret, (uint64_t)DEVAPC_AO_INFRA_PERI_BASE);
#else
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_INFRA_PERI_BASE failed! ERROR: %d\n", ret);
#endif
    }

    if((ret = drApiMapPhysicalBuffer((uint64_t)DEVAPC_AO_MM_BASE, SIZE_4KB, MAP_HARDWARE, (void **)&DEVAPC_AO_MM_VA_BUFFER)) != DRAPI_OK){
#ifdef FIX_BUILD_ERROR
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_MM_BASE failed! ERROR: %d, phy addr : 0x%llx\n", ret, (uint64_t)DEVAPC_AO_MM_VA_BUFFER);
#else
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_MM_BASE failed! ERROR: %d\n", ret);
#endif
    }

    if((ret = drApiMapPhysicalBuffer((uint64_t)DEVAPC_AO_MD_BASE, SIZE_4KB, MAP_HARDWARE, (void **)&DEVAPC_AO_MD_VA_BUFFER)) != DRAPI_OK){
#ifdef FIX_BUILD_ERROR
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_MD_BASE failed! ERROR: %d, phy addr : 0x%llx\n", ret, (uint64_t)DEVAPC_AO_MD_VA_BUFFER);
#else
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_MD_BASE failed! ERROR: %d\n", ret);
#endif
    }

    return 0;
}

static void dump_dapc_registers()
{
    unsigned int i = 0;
    unsigned int d = 0;

    for (d = 0; d <= 1; d++) {
        for (i = 0; i <= (SLAVE_INFRA_PERI_MAX_INDEX / MOD_NO_IN_1_DEVAPC); i++) {
            drDbgPrintLnf("T[DEVAPC] (Infra)reg%d-%d = 0x%x \n",
                d, i,
                readl((volatile unsigned int *)((unsigned int)DEVAPC0_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    for (d = 0; d <= 1; d++) {
        for (i = 0; i <= (SLAVE_MM_MAX_INDEX / MOD_NO_IN_1_DEVAPC); i++) {
            drDbgPrintLnf("T[DEVAPC] (MM)reg%d-%d = 0x%x \n",
                d, i,
                readl((volatile unsigned int *)((unsigned int)DEVAPC0_MM_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    for (d = 0; d <= 1; d++) {
        for (i = 0; i <= (SLAVE_MD_MAX_INDEX / MOD_NO_IN_1_DEVAPC); i++) {
            drDbgPrintLnf("T[DEVAPC] (MD)reg%d-%d = 0x%x \n",
                d, i,
                readl((volatile unsigned int *)((unsigned int)DEVAPC0_MD_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    drDbgPrintLnf("T[DEVAPC] (Infra)MAS_SEC_0 = 0x%x \n", readl(DEVAPC0_MAS_SEC_0));
}

#if 0
void do_devapc_ut_test()
{

    /* Important Note: ALWAYS USE THE DEVICE APC MODULE INDEX IN "tlDapcIndex.h" HEADER */

    drDbgPrintLnf("[DEVAPC][TUI] test_before");

    dump_dapc_registers();

    drDbgPrintLnf("[DEVAPC][TUI] test_lock");
    set_module_apc(DAPC_INDEX_I2C3, DOMAIN_AP, SECURE_RW_ONLY);
    set_module_apc(DAPC_INDEX_SPI0, DOMAIN_AP, SECURE_RW_ONLY);
    set_module_apc(DAPC_INDEX_SPI7, DOMAIN_AP, SECURE_RW_ONLY);
    set_module_apc(DAPC_INDEX_IPUSYS_TOP, DOMAIN_AP, SECURE_RW_ONLY);
    set_master_transaction(DAPC_MASTER_INDEX_SPI2, SECURE_TRANSACTION);
    set_master_transaction(DAPC_MASTER_INDEX_SPI6, SECURE_TRANSACTION);

    dump_dapc_registers();

    drDbgPrintLnf("[DEVAPC][TUI] test_unlock");
    set_module_apc(DAPC_INDEX_I2C3, DOMAIN_AP, NO_PROTECTION);
    set_module_apc(DAPC_INDEX_SPI0, DOMAIN_AP, NO_PROTECTION);
    set_module_apc(DAPC_INDEX_SPI7, DOMAIN_AP, NO_PROTECTION);
    set_module_apc(DAPC_INDEX_IPUSYS_TOP, DOMAIN_AP, NO_PROTECTION);
    set_master_transaction(DAPC_MASTER_INDEX_SPI2, NON_SECURE_TRANSACTION);
    set_master_transaction(DAPC_MASTER_INDEX_SPI6, NON_SECURE_TRANSACTION);

    dump_dapc_registers();
}
#endif

int has_started = 0;

int start_devapc()
{
    int module_index = 0;

    if (!has_started) {
        map_DAPC_Register();

        /*Enable Devapc*/

#if 0
        do_devapc_ut_test();
#else
        dump_dapc_registers();
#endif

        has_started = 1;
    }
    return 0;
}
