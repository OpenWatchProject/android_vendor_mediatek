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
#include <semphr.h>
#include <utils.h>
#include <sys/types.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <feature_manager.h>
#include <tinysys_config.h>
#include <mt_uart.h>
#include <wakelock.h>
#include <dvfs_common.h>
#include <dvfs.h>
#include <sleep.h>
#ifdef CFG_CACHE_SUPPORT
#include <cache_dram_management.h>
#include <cache_internal.h>
#include <mpu_mtk.h>
#endif

#define DVFS_DEBUG  0

#if DVFS_DEBUG
#define DVFSLOG(fmt, arg...) PRINTF_W(DVFSTAG fmt, ##arg)
#else
#define DVFSLOG(...)
#endif

#if DVFS_DEBUG
#define DVFSFUC(fmt, arg...) PRINTF_W(DVFSTAG "%s("fmt")\n", __func__, ##arg)
#else
#define DVFSFUC(...)
#endif


static uint32_t ap_resource_ack_flag = 0;
static uint32_t ap_resource_req_cnt = 0;
static uint32_t dma_user_list[NUMS_MEM_ID];
static uint32_t infra_req_cnt = 0;
static uint32_t infra_user_list[NUMS_MEM_ID];
static uint32_t g_logger_enable_dram = 0;


/*****************************************
 * BUS Resource API
 ****************************************/
static void wait_bus_ack(void)
{
    int count = 0;

    do {
        if ((DRV_Reg32(BUS_RESOURCE) & 0x100) != 0)
            break;

        count++;
    } while (count < 1000000);

    if (count >= 1000000) {
        PRINTF_E("bus ack fail\n");
        configASSERT(0);
    }
}

void enable_infra(scp_reserve_mem_id_t dma_id, uint32_t wait_ack)
{
    DVFSFUC("%d, %d", dma_id, wait_ack);

    if (dma_id >= NUMS_MEM_ID) {
        PRINTF_E("err id %d\n", dma_id);
        configASSERT(0);
        return;
    }

    infra_user_list[dma_id]++;
    infra_req_cnt++;

    if (infra_req_cnt == 1) {
        turn_on_clk_sys_from_isr(NO_WAIT);
        DRV_SetReg32(BUS_RESOURCE, 0x1);

        if (wait_ack == NEED_WAIT) {
#ifdef CFG_XGPT_SUPPORT
            udelay(REQ_INFRA_WAIT_TIME_US);
#endif
            wait_bus_ack();
        }
    } else {
        DVFSLOG("infra is already ON.(infra_req_cnt=%d)\n", (int)infra_req_cnt);
    }
}

void disable_infra(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    if (dma_id >= NUMS_MEM_ID) {
        PRINTF_E("err id %d\n", dma_id);
        configASSERT(0);
        return;
    }

    if (infra_user_list[dma_id] == 0) {
        PRINTF_E("infra_user_list[%d]==0\n", dma_id);
        configASSERT(0);
        return;
    }

    if (infra_req_cnt == 0) {
        PRINTF_E("infra_req_cnt==0,id=%d\n", dma_id);
        configASSERT(0);
        return;
    }

    infra_user_list[dma_id]--;
    infra_req_cnt--;

    if (infra_req_cnt == 0) {
        DRV_ClrReg32(BUS_RESOURCE, 0x1);
        turn_off_clk_sys_from_isr();
    } else {
        DVFSLOG("keep clk_bus ON because infra_req_cnt %d != 0\n", (int)infra_req_cnt);
    }
}

/*****************************************
 * DRAM Resource API
 ****************************************/
int32_t get_dram_ack(void)
{
    return ((DRV_Reg32(AP_RESOURCE) & 0x100) ? 1 : 0);
}

void wait_dram_ack(void)
{
    int count = 0;

    do {
        if ((DRV_Reg32(AP_RESOURCE) & 0x100) != 0)
            break;

        count++;
    } while (count < 1000000);

    if (count >= 1000000) {
        PRINTF_E("DRAM ack fail\n");
        configASSERT(0);
    }
}

int32_t check_dram_ack_status(void)
{
    return ap_resource_ack_flag;
}

static void get_AP_ack_from_isr(scp_reserve_mem_id_t dma_id)
{
    if (ap_resource_ack_flag == 0) {
        wait_dram_ack();
        ap_resource_ack_flag = 1;

#ifdef CFG_CACHE_SUPPORT
        enable_mpu_region(MPU_ID_DRAM_0);
        enable_mpu_region(MPU_ID_DRAM_1);
        hal_cache_scp_sleep_protect_off();
        hal_cache_invalidate_all_cache_lines_from_isr(CACHE_ICACHE);
#endif
    } else
        DVFSLOG("bus already acked.\n");
}

void enable_AP_resource(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    if (dma_id >= NUMS_MEM_ID) {
        PRINTF_E("err id %d\n", dma_id);
        configASSERT(0);
        return;
    }

    dma_user_list[dma_id]++;
    ap_resource_req_cnt++;

    if (ap_resource_req_cnt == 1) {
        DRV_SetReg32(AP_RESOURCE, 0x1);
#if SCP_DDREN_AUTO_MODE == 0
        DRV_SetReg32(SYS_CTRL, 0x1 << DDREN_FIX_VALUE_BIT);
#endif
    } else
        DVFSLOG("DRAM is already ON.(ap_resource_req_cnt=%d)\n", (int)ap_resource_req_cnt);
}

void disable_AP_resource(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    if (dma_id >= NUMS_MEM_ID) {
        PRINTF_E("err id %d\n", dma_id);
        configASSERT(0);
        return;
    }

    if (dma_user_list[dma_id] == 0) {
        PRINTF_E("dma_user_list[%d]==0\n", dma_id);
        configASSERT(0);
        return;
    }

    if (ap_resource_req_cnt == 0) {
        PRINTF_E("ap_resource_req_cnt==0,id=%d\n", dma_id);
        configASSERT(0);
        return;
    }

    dma_user_list[dma_id]--;
    ap_resource_req_cnt--;

    if (ap_resource_req_cnt == 0) {
        DRV_ClrReg32(AP_RESOURCE, 0x1);
#if SCP_DDREN_AUTO_MODE == 0
        DRV_ClrReg32(SYS_CTRL, 0x1 << DDREN_FIX_VALUE_BIT);
#endif
        ap_resource_ack_flag = 0;
#ifdef CFG_CACHE_SUPPORT
        disable_mpu_region(MPU_ID_DRAM_0);
        disable_mpu_region(MPU_ID_DRAM_1);
        hal_cache_scp_sleep_protect_on();
#endif
    }
}

void dvfs_enable_DRAM_resource(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource(dma_id);
#ifdef CFG_XGPT_SUPPORT
    if (ap_resource_ack_flag == 0)
        udelay(REQ_DRAM_WAIT_TIME_US);
#endif
    get_AP_ack_from_isr(dma_id);
    kal_taskEXIT_CRITICAL();
}

void dvfs_enable_DRAM_resource_from_isr(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource(dma_id);
#ifdef CFG_XGPT_SUPPORT
    if (ap_resource_ack_flag == 0)
        udelay(REQ_DRAM_WAIT_TIME_US);
#endif
    get_AP_ack_from_isr(dma_id);
}

int32_t sshub_get_AP_ack_nonblock(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    get_AP_ack_from_isr(dma_id);
    kal_taskEXIT_CRITICAL();

    return 1;
}

void sshub_enable_AP_resource_nonblock(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource(dma_id);
    kal_taskEXIT_CRITICAL();
}

unsigned int dvfs_enable_DRAM_no_wait_for_logger(scp_reserve_mem_id_t dma_id)
{
    /* 0 go for rest of update, 1 go for postponding to next tick */
    unsigned int ret = 0;
#ifdef CFG_XGPT_SUPPORT
    static unsigned long long on_time = 0;
#endif

    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    if (g_logger_enable_dram == 0) {
        enable_infra(dma_id, NO_WAIT);
        enable_AP_resource(dma_id);
        g_logger_enable_dram = 1;
        ret = 1; /* postpone at least one tick before acking */
#ifdef CFG_XGPT_SUPPORT
        on_time = read_xgpt_stamp_ns();
#endif
    } else {
#ifdef CFG_XGPT_SUPPORT
        unsigned long long off_time = read_xgpt_stamp_ns();

        if((off_time > on_time) &&
            (off_time - on_time > REQ_DRAM_WAIT_TIME_US*1000)) {
            /* The next tick is past 500us and then go to ack dram. */
#endif

            get_AP_ack_from_isr(dma_id);
            ret = 0;

#ifdef CFG_XGPT_SUPPORT
        } else {
            /* The next tick happens in less than 500us,
               so postpones one tick agian.
             */
            ret = 1;
        }
#endif
    }
    kal_taskEXIT_CRITICAL();

    return ret;
}

void dvfs_disable_DRAM_no_wait_for_logger(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    disable_AP_resource(dma_id);
    disable_infra(dma_id);
    g_logger_enable_dram = 0;
    kal_taskEXIT_CRITICAL();
}

#if 0
void enable_dram_resource_for_dmgr(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource(dma_id);

#ifdef CFG_XGPT_SUPPORT
    if (ap_resource_ack_flag == 0) {
        kal_taskEXIT_CRITICAL();
        udelay(REQ_DRAM_WAIT_TIME_US);
    } else
        kal_taskEXIT_CRITICAL();
#else
    kal_taskEXIT_CRITICAL();
#endif
}

void get_dram_ack_from_isr_for_dmgr(void)
{
    if (ap_resource_ack_flag == 0) {
        if (get_dram_ack()) {
            ap_resource_ack_flag = 1;

    #ifdef CFG_CACHE_SUPPORT
            enable_mpu_region(MPU_ID_DRAM_0);
            enable_mpu_region(MPU_ID_DRAM_1);
            hal_cache_scp_sleep_protect_off();
            hal_cache_invalidate_all_cache_lines_from_isr(CACHE_ICACHE);
    #endif
        } else
            DVFSLOG("dram not ack\n");
    } else
        DVFSLOG("dram already acked.\n");
}
#endif

