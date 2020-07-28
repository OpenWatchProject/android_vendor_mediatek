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

#include "FreeRTOS.h"
#include <interrupt.h>
#include <task.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <limits.h>

void __real_vPortEnterCritical();
void __real_vPortExitCritical();
static uint64_t start_cs_time = 0;
static uint64_t end_cs_time = 0;
static uint64_t max_cs_duration_time = 0;
static uint64_t cs_duration_time = 0;
static uint64_t max_cs_limit = 0;
static uint64_t max_irq_limit = 0;
static uint32_t stamp_count_enter = 0;
static uint32_t stamp_count_exit = 0;
extern UBaseType_t uxCriticalNesting;
extern uint32_t scp_current_ipi_id;
extern uint32_t scp_max_duration_ipc_id;

/* os timer reg value * 77ns
* 1  ms: 12987
* 1.5ms: 19436
* 2  ms: 25974
* 3  ms: 38961
*/
#define IRQ_DURATION_LIMIT      38960  //3ms

struct irq_desc_t irq_desc[IRQ_MAX_CHANNEL];

/** get the status of isr context in this section of code
*
*  @returns
*    1 for isr context; 0 for task context.
*/
uint32_t is_in_isr(void)
{
    #define portVECTACTIVE_MASK ( 0xFFUL )

    /*  Masks off all bits but the VECTACTIVE bits in the ICSR register. */
    if ((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK) != 0)
        return 1;
    else
        return 0;
}

/** get irq mask status
*
*  @param irq interrupt number
*
*  @returns
*    irq mask status
*/
uint32_t get_mask_irq(IRQn_Type irq)
{
    return NVIC->ISER[((uint32_t)((int32_t)irq) >> 5)] & (1 << ((uint32_t)((int32_t)irq) & 0x1F));
}

/** enable irq
*
*  @param irq interrupt number
*
*  @returns
*    no return
*/
void unmask_irq(IRQn_Type irq)
{
    if (irq < IRQ_MAX_CHANNEL && irq_desc[irq].handler != NULL) {
        NVIC_EnableIRQ(irq);
    } else {
        PRINTF_E("ERR IRQ%d\n", irq);
    }
}

/** disable irq
*
*  @param irq interrupt number
*
*  @returns
*    no return
*/
void mask_irq(IRQn_Type irq)
{
    if (irq < IRQ_MAX_CHANNEL && irq_desc[irq].handler != NULL) {
        NVIC_DisableIRQ(irq);
    } else {
        PRINTF_E("ERR IRQ%d\n", irq);
    }
}

void wakeup_source_count(uint32_t wakeup_src)
{
    int32_t id;
    for (id = 0; id < IRQ_MAX_CHANNEL; id++) {
        if (wakeup_src & (1 << id))
            irq_desc[id].wakeup_count ++;
    }

}

/** register a irq handler
*
*  @param irq interrupt number
*  @param irq interrupt handler
*  @param irq interrupt name
*
*  @returns
*    no return
*/
void request_irq(uint32_t irq, irq_handler_t handler, const char *name)
{
    if (irq < IRQ_MAX_CHANNEL) {
        NVIC_DisableIRQ(irq);
        irq_desc[irq].name = name;
        irq_desc[irq].handler = handler;
        irq_desc[irq].priority = DEFAULT_IRQ_PRIORITY;
        //irq_desc[irq].irq_count = 0;
        //irq_desc[irq].wakeup_count = 0;
        irq_desc[irq].init_count ++;
        //irq_desc[irq].last_enter = 0;
        //irq_desc[irq].last_exit = 0;
        //irq_desc[irq].max_duration = 0;
        NVIC_SetPriority(irq, irq_desc[irq].priority);
        NVIC_ClearPendingIRQ(irq); /*clear pending IRQ to avoid old CIRQ setting before CIRQ init */
        NVIC_EnableIRQ(irq);
    } else {
        PRINTF("ERR IRQ%lu\n", irq);
    }
}

/** free a irq handler
*
*  @param irq interrupt number
*
*  @returns
*    no return
*/
void free_irq(uint32_t irq)
{
    if (irq < IRQ_MAX_CHANNEL) {
        NVIC_DisableIRQ(irq);
        irq_desc[irq].handler = NULL;
        if (irq_desc[irq].last_exit < irq_desc[irq].last_enter) {
            PRINTF_E("Free IRQ%lu\n", irq);
        }

    } else {
        PRINTF_E("ERR: Free IRQ%lu\n", irq);
    }
}

/** init irq handler
*  @returns
*    no return
*/
void irq_init(void)
{
    uint32_t irq;
    for (irq = 0; irq < IRQ_MAX_CHANNEL; irq++) {
        NVIC_DisableIRQ(irq);
        irq_desc[irq].name = "";
        irq_desc[irq].handler = NULL;
        irq_desc[irq].priority = DEFAULT_IRQ_PRIORITY;
        irq_desc[irq].irq_count = 0;
        irq_desc[irq].wakeup_count = 0;
        irq_desc[irq].init_count = 0;
        irq_desc[irq].last_enter = 0;
        irq_desc[irq].last_exit = 0;
        irq_desc[irq].max_duration = 0;
        NVIC_SetPriority(irq, irq_desc[irq].priority);
        /*clear pending interrupt*/
        DRV_WriteReg32(0xE000E280, 0xFFFFFFFF);
        DRV_WriteReg32(0xE000E284, 0xFFFFFFFF);
    }
}


/** interrupt handler entry & dispatcher
*
*  @param
*
*  @returns
*    no return
*/

void hw_isr_dispatch(void)
{
    uint32_t ulCurrentInterrupt;
    uint64_t duration;

    /* Obtain the number of the currently executing interrupt. */
    __asm volatile("mrs %0, ipsr" : "=r"(ulCurrentInterrupt));
    /* skip the CM4 built-in interrupts:16 */
    traceISR_ENTER();
    ulCurrentInterrupt = ulCurrentInterrupt - 16;
    //PRINTF_E("IRQ%lu\n", ulCurrentInterrupt);
    if (ulCurrentInterrupt < IRQ_MAX_CHANNEL) {
        if (irq_desc[ulCurrentInterrupt].handler) {

            irq_desc[ulCurrentInterrupt].irq_count ++;
            /* record the last handled interrupt duration, unit: (ns)*/
#ifdef CFG_XGPT_SUPPORT
            irq_desc[ulCurrentInterrupt].last_enter = timer_get_global_timer_tick();
#endif
            stamp_count_enter++;

            irq_desc[ulCurrentInterrupt].handler();

#ifdef CFG_XGPT_SUPPORT
            irq_desc[ulCurrentInterrupt].last_exit = timer_get_global_timer_tick();
#endif
            stamp_count_exit++;
            duration = irq_desc[ulCurrentInterrupt].last_exit - irq_desc[ulCurrentInterrupt].last_enter;
            /* handle the xgpt overflow case
            * discard the duration time when exit time < enter time
            * */
            if (irq_desc[ulCurrentInterrupt].last_exit > irq_desc[ulCurrentInterrupt].last_enter)
                if (duration > irq_desc[ulCurrentInterrupt].max_duration)
                    irq_desc[ulCurrentInterrupt].max_duration = duration;

#ifdef CFG_XGPT_SUPPORT
            /* monitor irq duration over limit
            *  timer_get_global_timer_tick() return OS timer register value
            *  OS timer register value * 77ns = time acuracy
            */
            if (irq_desc[ulCurrentInterrupt].last_exit > irq_desc[ulCurrentInterrupt].last_enter) {
                if ((duration > max_irq_limit) && (max_irq_limit != 0)) {
                    PRINTF_E("ERR IRQ%lu: duration %llu < %llu, ipi%ld ipc%ld\n"
                    , ulCurrentInterrupt, max_irq_limit, duration, scp_current_ipi_id, scp_max_duration_ipc_id);
#ifdef CFG_IRQ_MONITOR_SUPPORT
                    configASSERT(0);
#endif
                }
            }
#endif
        } else {
            PRINTF_E("IRQ%lu handler null\n", ulCurrentInterrupt);
        }
    } else {
        PRINTF_E("ERR IRQ%lu\n", ulCurrentInterrupt);
    }
    traceISR_EXIT();
}

/** interrupt status query function
*
*  @param
*
*  @returns
*    no return
*/
void irq_status_dump(void)
{
    int32_t id;

    PRINTF_E("id\tname\tpriority(HW)\tcount\tlast\tenable\tpending\tactive\n");
    for (id = NonMaskableInt_IRQn; id < 0; id++) {
        PRINTF_E("%d\t%s\t%d(%d)\t\t%d\t%d\t%s\t%s\t%s\n",
                 (int)id,
                 "n/a",
                 0,
                 (int)NVIC_GetPriority(id),
                 0,
                 0,
                 get_mask_irq(id) ? "enable" : "disable",
                 "n/a",
                 "n/a");
    }
    for (id = 0; id < IRQ_MAX_CHANNEL; id++) {
        PRINTF_E("%d\t%s\t%d(%d)\t\t%d\t%llu\t%s\t%s\t%s\n",
                 (int)id,
                 (irq_desc[id].name) ? irq_desc[id].name : "n/a",
                 (int)irq_desc[id].priority,
                 (int)NVIC_GetPriority(id),
                 (int)irq_desc[id].irq_count,
                 irq_desc[id].last_exit,
                 get_mask_irq(id) ? "enable" : "disable",
                 NVIC_GetPendingIRQ(id) ? "enable" : "no",
                 NVIC_GetActive(id) ? "enable" : "inactive");
    }
}

uint32_t irq_total_times()
{
    int32_t id;
    uint32_t num = 0;

    for (id = 0; id < IRQ_MAX_CHANNEL; id++)
        num += (int)irq_desc[id].irq_count;

    return num;
}

void set_max_cs_limit(uint64_t limit_time)
{
    if (max_cs_limit == 0)
        max_cs_limit = limit_time;
    else {
        PRINTF_D("set_max_cs_limit(%llu) failed\n", limit_time);
        PRINTF_D("Already set max context switch limit:%llu,\nPlease run disable_cs_limit() to disiable it first\n",
                 max_cs_limit);
    }
}

void disable_cs_limit(void)
{
    max_cs_limit = 0;
}

void __wrap_vPortEnterCritical()
{
    __real_vPortEnterCritical();
    if (uxCriticalNesting == 1 && max_cs_limit != 0) {
#ifdef CFG_XGPT_SUPPORT
        start_cs_time = read_xgpt_stamp_ns() ;
#endif
    }
}

void __wrap_vPortExitCritical()
{
    if ((uxCriticalNesting - 1) == 0 && max_cs_limit != 0) {
#ifdef CFG_XGPT_SUPPORT
        end_cs_time = read_xgpt_stamp_ns();
#endif
        if (end_cs_time > start_cs_time) {
            cs_duration_time = end_cs_time - start_cs_time;
            if (cs_duration_time > max_cs_duration_time)
                max_cs_duration_time = cs_duration_time;
            if (max_cs_duration_time > max_cs_limit) {
                PRINTF_D("violate the critical section time limit:%llu>%llu\n", max_cs_duration_time, max_cs_limit);
                configASSERT(0);
            }
        }

    }
    __real_vPortExitCritical();
}

uint64_t get_max_cs_duration_time(void)
{
    return max_cs_duration_time;
}

uint64_t get_max_cs_limit(void)
{
    return max_cs_limit;
}


void set_irq_limit(uint32_t irq_limit_time)
{
    if (irq_limit_time == 0)
        max_irq_limit = 0;
    else {
        max_irq_limit = CFG_IRQ_DURATION_LIMIT;
    }
}


