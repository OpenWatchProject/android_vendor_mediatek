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

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "mt_typedefs.h"
#include "dapc.h"
#define TAG "[dapc]"
#include "log.h"


unsigned char *devapc_ao_vaddr = 0;

static inline unsigned int readl(volatile unsigned int* addr)
{
    return *(volatile unsigned int*)addr;
}

static inline void writel(unsigned int val, volatile unsigned int* addr)
{
    *(volatile unsigned int*)addr = val;
}

int set_master_transaction(unsigned int master_index ,unsigned int transaction_type)
{
    volatile unsigned int* base = 0;
    unsigned int set_bit = 0;

    base = (unsigned int*) ((size_t)DEVAPC_MAS_SEC);

    if(master_index > 31)
        return -1;

    if (transaction_type == 0) {
        set_bit = ~(1 << master_index);
        writel(readl(base) & set_bit, base);
    } else if(transaction_type == 1) {
        set_bit = 1 << master_index;
        writel(readl(base) | set_bit, base);
    } else {
        return -2;
    }

    return 0;
}

void set_module_apc(unsigned int module, unsigned int domain_num , unsigned int permission_control)
{
    volatile unsigned int* base = 0;

    unsigned int clr_bit = 0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2);
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    if(module >= DEVAPC_DEVICE_NUMBER) {
        drDbgPrintLnf("[DEVAPC] set_module_apc : device number %d exceeds the max number!\n", module);
        return;
    }

    if (domain_num == 0)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D0_APC_0 + (module/16) *4);
    }
    else if (domain_num == 1)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D1_APC_0 + (module/16) *4);
    }
    else if (domain_num == 2)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D2_APC_0 + (module/16) *4);
    }
    else if (domain_num == 3)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D3_APC_0 + (module/16) *4);
    }
    else if (domain_num == 4)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D4_APC_0 + (module/16) *4);
    }
    else if (domain_num == 5)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D5_APC_0 + (module/16) *4);
    }
    else if (domain_num == 6)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D6_APC_0 + (module/16) *4);
    }
    else if (domain_num == 7)
    {
        base = (unsigned int*) ((unsigned int)DEVAPC_D7_APC_0 + (module/16) *4);
    }

     writel(readl(base) & ~clr_bit, base);
     writel(readl(base) | set_bit, base);

}

static int map_DAPC_Register()
{
    drApiResult_t ret;

    if((ret = drApiMapPhysicalBuffer((uint64_t)DEVAPC_AO_BASE, SIZE_4KB, MAP_HARDWARE, (void **)&DEVAPC_AO_VA_BUFFER)) != DRAPI_OK){
        drDbgPrintLnf("[DEVAPC] map DEVAPC_AO_BASE failed! ERROR: %d, phy addr : 0x%llx\n", ret, (uint64_t)DEVAPC_AO_BASE);
    }

    return 0;
}

int has_started=0;

int start_devapc()
{
    int module_index = 0;

    if (!has_started) {
        map_DAPC_Register();

        /*Enable Devapc*/

        writel(readl(DEVAPC_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC_APC_CON);
        has_started = 1;
    }
}
