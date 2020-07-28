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

#include <adsp_ipi.h>
#include <semphr.h>
#include <utils.h>
#include <sys/types.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <tinysys_config.h>
#include <feature_manager.h>
#include <wakelock.h>
#include <dvfs_common.h>
#include <dvfs.h>
#include <sleep.h>
#include <dma.h>

static void adsp_enter_sleep_setup_freq(void) INTERNAL_FUNCTION;
static void adsp_leave_sleep_setup_freq(void) INTERNAL_FUNCTION;
uint32_t g_sleep_flag INTERNAL_INITIAL_DATA = 0;
uint32_t g_sleep_once INTERNAL_INITIAL_DATA = 1;
uint32_t g_active_adsp_mux_value INTERNAL_INITIAL_DATA = ADSPPLL_D6;

uint32_t wakeup_src_mask INTERNAL_INITIAL_DATA = 0x01331887; //mapping to wakeup_src_id bit

/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
int32_t get_resource_ack(uint32_t reg_name)
{
    return 1;
}

int32_t wait_for_ack(uint32_t reg_addr)
{
    return 1;
}


#if configUSE_WAKEUP_SOURCE == 1
extern uint32_t g_tickless_flag;
extern uint32_t g_sleep_debug_flag;

void wakeup_src_handler(void)
{
    UBaseType_t mask = portSET_INTERRUPT_MASK_FROM_ISR();
    g_tickless_flag = 0;

    /* clean wakeup source IRQ */
    drv_clr_reg32(ADSP_WAKEUPSRC_IRQ, 1);

    portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
}
#endif

void adsp_wakeup_src_setup(wakeup_src_id irq, uint32_t enable)
{
    taskENTER_CRITICAL();

    if (irq >= IRQ_MAX_CHANNEL) {
        DVFS_LOG_E("fail to set wakeup source\n");
        taskEXIT_CRITICAL();
        return;
    }

    if (enable) {
        wakeup_src_mask &= ~(1 << irq);
    } else {
        wakeup_src_mask |= (1 << irq);
    }
    drv_write_reg32(ADSP_WAKEUPSRC_MASK, wakeup_src_mask);
    DVFS_LOG_D("wakesrc mask=0x%x\n", drv_reg32(ADSP_WAKEUPSRC_MASK));
    taskEXIT_CRITICAL();
}

/*celine: use ipi to send sleep cmd from AP to ADSP*/
void adsp_sleep_ctrl(int id, void* data, unsigned int len)
{
}

/*celine: TODO, will implement for ADSP*/
#if AUTO_DUMP_SLEPP_STATUS
void dump_sleep_block_reason(void)
{
}
#endif

int can_enter_sleep(uint32_t modify_time)
{
    int reason = NR_REASONS;

    if (!wake_lock_is_empty()) {
        reason = BY_WAKELOCK;
        goto out;
    }

    if (is_ipi_busy()) {
        reason = BY_IPI_BUSY;
        goto out;
    }
    if (drv_reg32(DMA_GLBSTA)) { //dma is running
        reason = BY_DMA_BUSY;
        goto out;
    }

out:
    if (reason != NR_REASONS) {
        if (g_sleep_once) {
            DVFS_LOG_D("cannot sleep:reason=%d\n", reason);
            g_sleep_once = 0;
        }
        return 0;
    } else {
        DVFS_LOG_D("enter sleep=%d(ticks), last sleep dur=%d(us), sleep_debug_flag=0x%0x\n",
                   modify_time, g_sleep_tick_us, g_sleep_debug_flag);
        g_sleep_once = 1;
        return 1;
    }
}

void adsp_enable_dram_resource(void)
{
    int timeout = 200;

    /* enable SPM resource request */
    drv_set_reg32(ADSP_SSPM_REQ, ADSP_SPM_ALL_REQ);
    drv_set_reg32(ADSP_DDREN_REQ, ADSP_DDR_ENABLE);

    while(--timeout) {
        if (drv_reg32(ADSP_SSPM_ACK) == (ADSP_SPM_ALL_REQ | (ADSP_DDR_ENABLE << 4)))
            break;
        udelay(50);
    }
    if (timeout == 0)
        configASSERT(0);
}

void adsp_disable_dram_resource(void)
{
    /* disable SPM resource request */
    drv_clr_reg32(ADSP_DDREN_REQ, ADSP_DDR_ENABLE);
    drv_clr_reg32(ADSP_SSPM_REQ, ADSP_SPM_ALL_REQ);
}

static void adsp_enter_sleep_setup_freq(void)
{
    adsp_clock_mux_setup(TCK_26M_MX9_CK);
    adsp_disable_dram_resource();
}

static void adsp_leave_sleep_setup_freq(void)
{
    adsp_enable_dram_resource();
    adsp_clock_mux_setup(g_active_adsp_mux_value);
    g_sleep_debug_flag |= (adsp_get_clock_mux() <<DBG_ADSP_SEL_MUX_BIT);
}

void pre_sleep_process(unsigned int modify_time)
{
    /* Avoid race condition by interrupt masking */
    UBaseType_t mask = portSET_INTERRUPT_MASK_FROM_ISR();

    adsp_enter_sleep_setup_freq();
    g_sleep_flag = 1;
    portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
}
void post_sleep_process(unsigned int expect_time)
{
    if (g_sleep_flag) {
        UBaseType_t mask = portSET_INTERRUPT_MASK_FROM_ISR();

        g_sleep_flag = 0;
        adsp_leave_sleep_setup_freq();

        portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
    }
}
