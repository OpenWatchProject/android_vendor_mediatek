/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2017. All rights reserved.
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
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/


#include "FreeRTOS.h"
#include <xtensa_rtos.h>
#include <interrupt.h>
#include <task.h>
#include <stdio.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif

#ifdef LIANG_UNMARK
static uint64_t start_cs_time = 0;
static uint64_t end_cs_time = 0;
static uint64_t cs_duration_time = 0;
#endif
uint64_t max_cs_duration_time INTERNAL_NON_INITIAL_DATA = 0;
uint64_t max_cs_limit INTERNAL_NON_INITIAL_DATA = 0;
uint64_t max_irq_limit INTERNAL_NON_INITIAL_DATA = 0;
uint32_t stamp_count_enter INTERNAL_NON_INITIAL_DATA = 0;
uint32_t stamp_count_exit INTERNAL_NON_INITIAL_DATA = 0;
extern unsigned port_interruptNesting;  // Interrupt nesting level.

/* hw interrupt priority info*/
unsigned char   int_pri[32] INTERNAL_INITIAL_DATA = {
    XCHAL_INT0_LEVEL,  XCHAL_INT1_LEVEL,  XCHAL_INT2_LEVEL,  XCHAL_INT3_LEVEL,
    XCHAL_INT4_LEVEL,  XCHAL_INT5_LEVEL,  XCHAL_INT6_LEVEL,  XCHAL_INT7_LEVEL,
    XCHAL_INT8_LEVEL,  XCHAL_INT9_LEVEL,  XCHAL_INT10_LEVEL, XCHAL_INT11_LEVEL,
    XCHAL_INT12_LEVEL, XCHAL_INT13_LEVEL, XCHAL_INT14_LEVEL, XCHAL_INT15_LEVEL,
    XCHAL_INT16_LEVEL, XCHAL_INT17_LEVEL, XCHAL_INT18_LEVEL, XCHAL_INT19_LEVEL,
    XCHAL_INT20_LEVEL, XCHAL_INT21_LEVEL, XCHAL_INT22_LEVEL, XCHAL_INT23_LEVEL,
    XCHAL_INT24_LEVEL, XCHAL_INT25_LEVEL, XCHAL_INT26_LEVEL, XCHAL_INT27_LEVEL,
    XCHAL_INT28_LEVEL, XCHAL_INT29_LEVEL, XCHAL_INT30_LEVEL, XCHAL_INT31_LEVEL
};

struct irq_desc_t irq_desc[IRQ_MAX_CHANNEL] INTERNAL_NON_INITIAL_DATA;

/* os timer reg value * 77ns  //13M os timer
* 1  ms: 12987
* 1.5ms: 19436
* 2  ms: 25974
* 3  ms: 38961
*/
#define IRQ_DURATION_LIMIT      39000 //3ms
#define OSCYC2US(cyc) (((cyc) * 315) >> 12) // os-timer = 13Mhz

static uint32_t get_irq_status(IRQn_Type irq);
static uint32_t get_mask_irq(IRQn_Type irq);
uint32_t is_in_isr(void)
{
    return port_interruptNesting ? 1 : 0;
}

/** get irq mask status
*
*  @param irq interrupt number
*
*  @returns
*    irq interrupt status
*/
static uint32_t get_irq_status(IRQn_Type irq)
{
    // get irq status
    uint32_t irq_status;
    irq_status = drv_reg32(ADSP_HIFI3_INTR);
    return (irq_status & (1 << irq)) ? 1 : 0;
}

/** get irq mask status
*
*  @param irq interrupt number
*
*  @returns
*    irq mask status
*/
static uint32_t get_mask_irq(IRQn_Type irq)
{
    uint32_t irq_mask;
    irq_mask = drv_reg32(ADSP_IRQ_EN);
    return (irq_mask & (1 << irq)) ? 1 : 0;
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
        drv_set_reg32(ADSP_IRQ_EN, (1 << irq));
        xt_ints_on(1 << irq);
    }
    else {
        PRINTF("ERR Int ID%u\n\r", irq);
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
        xt_ints_off(1 << irq);
        drv_clr_reg32(ADSP_IRQ_EN, (1 << irq));
    }
    else {
        PRINTF("ERR IRQ ID%u\n\r", irq);
    }
}

void wakeup_source_count(uint32_t wakeup_src)
{
    int32_t id;
    for (id = 0; id < IRQ_MAX_CHANNEL; id++) {
        if (wakeup_src & (1 << id)) {
            irq_desc[id].wakeup_count ++;
        }
    }
}

/** mask all irq except of wakeup source IRQ
*   bit 23, edge trigger, pritority 3
*
*   @returns
*     no return
*/
void turn_off_non_wakeup_src_irq(void)
{
    xt_ints_off(wakeup_src_mask);
}

void turn_on_non_wakeup_src_irq(void)
{
    xt_ints_on(wakeup_src_mask & drv_reg32(ADSP_IRQ_EN));
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
        xt_ints_off(1 << irq);
        drv_clr_reg32(ADSP_IRQ_EN, (1 << irq)); //hw disable irq
        irq_desc[irq].name = name;
        irq_desc[irq].handler = handler;
        irq_desc[irq].init_count ++;
        drv_set_reg32(ADSP_IRQ_EN, (1 << irq)); //hw enable irq
        xt_ints_on(1 << irq);                //sw enable irq
    }
    else {
        PRINTF("ERR IRQ ID%u\n\r", irq);
    }
}

/** init irq handler
*  @returns
*    no return
*/
void irq_init(void)
{
    uint32_t irq;

    xt_ints_off(0xffffffff);                   //sw disable all irq
    drv_clr_reg32(ADSP_IRQ_EN, 0xffffffff);    //hw disable all irq

    for (irq = 0; irq < IRQ_MAX_CHANNEL; irq++) {
        irq_desc[irq].name = "";
        irq_desc[irq].handler = NULL;
        irq_desc[irq].priority = int_pri[irq];
        irq_desc[irq].irq_count = 0;
        irq_desc[irq].wakeup_count = 0;
        irq_desc[irq].init_count = 0;
        irq_desc[irq].last_enter = 0;
        irq_desc[irq].last_exit = 0;
        irq_desc[irq].max_duration = 0;
        xt_set_interrupt_handler(irq, hw_isr_dispatch,
                                 (void *)irq); //default go hw_isr_dispatcher
    }
}

void irq_sync(void)
{
    uint32_t irq;
    uint32_t already_enabled = 0;
    for (irq = 0; irq < IRQ_MAX_CHANNEL; irq++) {
        if (irq_desc[irq].init_count)
            already_enabled |= 1 << irq;
    }

    drv_write_reg32(ADSP_IRQ_EN, already_enabled);
}

/** interrupt handler entry & dispatcher
*
*  @param
*
*  @returns
*    no return
*/

#if configUSE_TICKLESS_IDLE == 1
extern void check_tickless_flag (uint32_t tick_compensation);
#endif  /* configUSE_TICKLESS_IDLE == 1 */

void hw_isr_dispatch(void *arg)
{
    uint32_t ulCurrentInterrupt;
    uint64_t duration;

    /* Obtain the number of the currently executing interrupt. */
    ulCurrentInterrupt = (uint32_t)arg;
#ifdef LIANG_UNMARK
    traceISR_ENTER();
#endif
    //PRINTF("Interrupt ID %lu\n", ulCurrentInterrupt);
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
            duration = irq_desc[ulCurrentInterrupt].last_exit -
                       irq_desc[ulCurrentInterrupt].last_enter;
            /* handle the xgpt overflow case
            * discard the duration time when exit time < enter time
            * */
            if (irq_desc[ulCurrentInterrupt].last_exit >
                irq_desc[ulCurrentInterrupt].last_enter)
                if (duration > irq_desc[ulCurrentInterrupt].max_duration) {
                    irq_desc[ulCurrentInterrupt].max_duration = duration;
                }

#ifdef CFG_XGPT_SUPPORT
            /* monitor irq duration over limit
            *  timer_get_global_timer_tick() return OS timer register value
            *  OS timer register value * 77ns = time acuracy
            */
            if (irq_desc[ulCurrentInterrupt].last_exit >
                irq_desc[ulCurrentInterrupt].last_enter) {
                if ((duration > max_irq_limit) && (max_irq_limit != 0)) {
                    PRINTF_D("IRQ ID:%u violate irq duration %llu us > max_irq_limit %llu us\n",
                             ulCurrentInterrupt, OSCYC2US(duration), OSCYC2US(max_irq_limit));
#ifdef CFG_IRQ_MONITOR_SUPPORT
                    configASSERT(0);
#endif
                }
            }
#endif
        }
        else {
            PRINTF("IRQ ID %u handler null\n\r", ulCurrentInterrupt);
        }
    }
    else {
        PRINTF("ERR IRQ ID %u\n\r", ulCurrentInterrupt);
    }
#ifdef LIANG_UNMARK
    traceISR_EXIT();
#endif
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

    PRINTF_E("id\tname\tpriority(HW)\tcount\tlast\tenable\tactive\n\r");
    for (id = 0; id < IRQ_MAX_CHANNEL; id++) {
        if (irq_desc[id].handler) {
            PRINTF_E("%d\t%s\t%d\t%d\t%llu\t%s\t%s\n\r",
                     id,
                     (irq_desc[id].name) ? irq_desc[id].name : "n/a",
                     irq_desc[id].priority,
                     irq_desc[id].irq_count,
                     irq_desc[id].last_exit,
                     get_mask_irq(id) ? "enable" : "disable",
                     get_irq_status(id) ? "enable" : "inactive");
        }
    }
}

void set_max_cs_limit(uint64_t limit_time)
{
    if (max_cs_limit == 0) {
        max_cs_limit = limit_time;
    }
    else {
        PRINTF_D("set_max_cs_limit(%llu) failed\n\r", limit_time);
        PRINTF_D("Already set max context switch limit:%llu,\n\rPlease run disable_cs_limit() to disiable it first\n",
                 max_cs_limit);
    }
}

void disable_cs_limit(void)
{
    max_cs_limit = 0;
}

#ifdef LIANG_UNMARK
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
            if (cs_duration_time > max_cs_duration_time) {
                max_cs_duration_time = cs_duration_time;
            }
            if (max_cs_duration_time > max_cs_limit) {
                PRINTF_D("violate the critical section time limit:%llu>%llu\n",
                         max_cs_duration_time, max_cs_limit);
                configASSERT(0);
            }
        }

    }
    __real_vPortExitCritical();
}
#endif
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
    if (irq_limit_time == 0) {
        max_irq_limit = 0;
    }
    else {
        max_irq_limit = IRQ_DURATION_LIMIT;
    }
}

