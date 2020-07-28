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
#include <task.h>
#include <stdio.h>
#include <driver_api.h>
#include <interrupt.h>
#include <mt_reg_base.h>
#include <scp_sem.h>
#include <platform_mtk.h>

#include <scp_ipi.h>
#include <utils.h>
#include <sys/types.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <tinysys_config.h>
#include <wakelock.h>
#include <dvfs.h>
#include <dvfs_common.h>

#define DVFS_DEBUG         0

#if DVFS_DEBUG
#define DVFSLOG(fmt, arg...)        \
    do {            \
        PRINTF_D(DVFSTAG fmt, ##arg);   \
    } while (0)
#else
#define DVFSLOG(...)
#endif

#if DVFS_DEBUG
#define DVFSFUC(fmt, arg...)        \
    do {            \
        PRINTF_D(DVFSTAG "%s("fmt")\n", __func__, ##arg);  \
    } while (0)
#else
#define DVFSFUC(...)
#endif

#if DVFS_DEBUG
#define DVFSDBG(fmt, arg...)        \
    do {            \
        PRINTF_W(DVFSTAG fmt, ##arg);   \
    } while (0)
#else
#define DVFSDBG(fmt, arg...)
#endif

extern int g_scp_dvfs_debug;

 /*****************************************
 * ADB CMD Control APIs
 ****************************************/
void dvfs_debug_set(int id, void* data, unsigned int len)
{
    uint8_t *msg = (uint8_t *)data;

    g_scp_dvfs_debug = *msg;
    DVFSLOG("set g_scp_dvfs_debug = %d\n", *msg);
}

/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
void enable_clk_bus(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", dma_id);
    kal_taskENTER_CRITICAL();
    enable_infra(dma_id, NEED_WAIT);
    kal_taskEXIT_CRITICAL();
}

void disable_clk_bus(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", dma_id);
    kal_taskENTER_CRITICAL();
    disable_infra(dma_id);
    kal_taskEXIT_CRITICAL();
}

void enable_clk_bus_from_isr(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", dma_id);
    enable_infra(dma_id, NEED_WAIT);
}

void disable_clk_bus_from_isr(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", dma_id);
    disable_infra(dma_id);
}

void dvfs_disable_DRAM_resource(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", dma_id);
    kal_taskENTER_CRITICAL();
    disable_AP_resource(dma_id);
    disable_infra(dma_id);
    kal_taskEXIT_CRITICAL();
}

