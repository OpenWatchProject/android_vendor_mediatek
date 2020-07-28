
/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
 * Copyright (c) 2015-2019 Cadence Design Systems, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <xtensa/config/core.h>

#include "xtensa_rtos.h"
#include "xtensa_api.h"

#include "FreeRTOS.h"
#include "task.h"

#if configUSE_TICKLESS_IDLE == 1
#include "driver_api.h"
#ifdef CFG_XGPT_SUPPORT
#include "mt_gpt.h"
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
#include "dvfs.h"
#endif
#include "xtensa_api.h"
#include "xtensa_timer.h"
#include "interrupt.h"
#include "sleep.h"
#include "wdt.h"
#endif  /* configUSE_TICKLESS_IDLE == 1 */

#include "dvfs_common.h"
/* Defined in portasm.h */
extern void _frxt_tick_timer_init(void);

/* Defined in xtensa_context.S */
extern void _xt_coproc_init(void);


/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;
extern void vPortYieldStartScheduler(); /* from portasm.S, context switch to first task */
extern void vPortYieldEndScheduler();   /* from portasm.S, context switch to main */

unsigned int *pStackOfMain = NULL; /* main function stask point, it save sp when call vPortYieldStartScheduler */
unsigned port_xSchedulerRunning = 0; // Duplicate of inaccessible xSchedulerRunning; needed at startup to avoid counting nesting
unsigned port_interruptNesting = 0;  // Interrupt nesting level
UBaseType_t uxCriticalNesting = 0xaaaaaaaa;
/*-----------------------------------------------------------*/
#if configUSE_TICKLESS_IDLE == 1
#define SLEEP_CPU_CLOCK_IN_HZ       (26000000)
#define SLEEP_CYCLE_PER_TICK        (SLEEP_CPU_CLOCK_IN_HZ / configTICK_RATE_HZ)    /* = 26000  */
#define MAX_SUPPRESSED_TICK_COUNT   (0xFFFFFFFF / SLEEP_CYCLE_PER_TICK)             /* = 165191 */
#define OS_TIMER_CLOCK_IN_HZ        (13000000)
#define OS_TIMER_TICK_RATIO         (OS_TIMER_CLOCK_IN_HZ / configTICK_RATE_HZ)     /* = 13000                                                    */
#define TICK_THRESHOLD_DIVISOR      (2)                                             /* > half tick: skip this tick; <= half tick: apply this tick */
#define OS_TIMER_THRESHOLD          (OS_TIMER_TICK_RATIO / TICK_THRESHOLD_DIVISOR)  /* = 6500                                                     */
#define UPPER_BOUND(in,up)          ((in) > (up) ? (up) : (in))
#ifdef CFG_TASK_MONITOR
#define MAXIMUM_TICK_DIFFERENCE     (100 * configTICK_RATE_HZ)                      /* 100 sec --> 100000 ticks */
#endif

#ifdef CFG_WDT_SUPPORT
static unsigned int wdt_tickcount = 0;
#ifdef configUSE_WAKEUP_SOURCE
#define WDT_DELAY_TIME              (WDT_TOTAL_DELAY_US * (SLEEP_CYCLE_PER_TICK / 1000)) /* enter sleep delay * 26M */
#define TICK_TO_WDT_COUNT(tick)     (((tick) * WDT_COUNT_RATE + (WDT_COUNT_RATE / 2)) / configTICK_RATE_HZ)
#else
#define WDT_DELAY_TIME              (0)
#endif
#else
#define WDT_DELAY_TIME              (0)
#endif

extern unsigned _xt_tick_divisor;

uint64_t g_os_timer_base = 0;
uint64_t g_os_timer_prev = 0;
volatile uint32_t g_tickless_flag = 0;
volatile uint32_t g_sleep_debug_flag = 0;
static TickType_t g_expected_idle_tick_count = 0;

/* measure sleep duration */
static uint32_t g_cycle_pre_sleep = 0;
static uint32_t g_cycle_post_sleep = 0;
static uint32_t g_tick_sleep_dur = 0;

#ifdef CFG_XGPT_SUPPORT
static uint64_t g_os_timer_pre_sleep = 0;
static uint64_t g_os_timer_post_sleep = 0;
#endif
#endif
/*-----------------------------------------------------------*/

// User exception dispatcher when exiting
void _xt_user_exit(void);

/*
 * Stack initialization
 */
#if portUSING_MPU_WRAPPERS
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters, BaseType_t xRunPrivileged )
#else
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
#endif
{
	StackType_t *sp, *tp;
	XtExcFrame  *frame;
	#if XCHAL_CP_NUM > 0
	uint32_t *p;
	#endif

	/* Create interrupt stack frame aligned to 16 byte boundary */
	sp = (StackType_t *) (((UBaseType_t)(pxTopOfStack + 1) - XT_CP_SIZE - XT_STK_FRMSZ) & ~0xf);

	/* Clear the entire frame (do not use memset() because we don't depend on C library) */
	for (tp = sp; tp <= pxTopOfStack; ++tp)
		*tp = 0;

	frame = (XtExcFrame *) sp;

	/* Explicitly initialize certain saved registers */
	frame->pc   = (UBaseType_t) pxCode;             /* task entrypoint                */
	frame->a0   = 0;                                /* to terminate GDB backtrace     */
	frame->a1   = (UBaseType_t) sp + XT_STK_FRMSZ;  /* physical top of stack frame    */
	frame->exit = (UBaseType_t) _xt_user_exit;      /* user exception exit dispatcher */

	/* Set initial PS to int level 0, EXCM disabled ('rfe' will enable), user mode. */
	/* Also set entry point argument parameter. */
	#ifdef __XTENSA_CALL0_ABI__
	frame->a2 = (UBaseType_t) pvParameters;
	frame->ps = PS_UM | PS_EXCM;
	#else
	/* + for windowed ABI also set WOE and CALLINC (pretend task was 'call4'd). */
	frame->a6 = (UBaseType_t) pvParameters;
	frame->ps = PS_UM | PS_EXCM | PS_WOE | PS_CALLINC(1);
	#endif

	#ifdef XT_USE_SWPRI
	/* Set the initial virtual priority mask value to all 1's. */
	frame->vpri = 0xFFFFFFFF;
	#endif

	#if XCHAL_CP_NUM > 0
	/* Init the coprocessor save area (see xtensa_context.h) */
	/* No access to TCB here, so derive indirectly. Stack growth is top to bottom.
         * //p = (uint32_t *) xMPUSettings->coproc_area;
	 */
	p = (uint32_t *)(((uint32_t) pxTopOfStack - XT_CP_SIZE) & ~0xf);
	p[0] = 0;
	p[1] = 0;
	p[2] = (((uint32_t) p) + 12 + XCHAL_TOTAL_SA_ALIGN - 1) & -XCHAL_TOTAL_SA_ALIGN;
	#endif

	return sp;
}

/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the Xtensa port will get stopped.  If required simply
	disable the tick interrupt here. */
	xt_ints_off(XT_TIMER_INTEN);
	port_xSchedulerRunning = 0;
	xTaskSetSchedulerRunning(pdFALSE);
	vPortYieldEndScheduler();
}

/*-----------------------------------------------------------*/

#if configUSE_TICKLESS_IDLE == 1
uint32_t g_tick_base = 0;
#endif

BaseType_t xPortStartScheduler(void)
{
	// Interrupts are disabled at this point and stack contains PS with enabled interrupts when task context is restored

	#if XCHAL_CP_NUM > 0
	/* Initialize co-processor management for tasks. Leave CPENABLE alone. */
	_xt_coproc_init();
	#endif

	/* Init the tick divisor value */
	_xt_tick_divisor_init();

#if configUSE_TICKLESS_IDLE == 1
	g_tick_base = xTaskGetTickCount();
#endif

	/* Setup the hardware to generate the tick. */
	_frxt_tick_timer_init();

	#if XT_USE_THREAD_SAFE_CLIB
	// Init C library
	vPortClibInit();
	#endif

	port_xSchedulerRunning = 1;
	uxCriticalNesting = 0;
	xTaskSetSchedulerRunning(pdTRUE);

	// Cannot be directly called from C; never returns
	vPortYieldStartScheduler();
	//__asm__ volatile ("call0    _frxt_dispatch\n");

	/* Should not get here. */
	return pdTRUE;
}
/*-----------------------------------------------------------*/
#ifdef CFG_WDT_SUPPORT
extern void mtk_wdt_restart(void);
extern void mtk_wdt_restart_interval(unsigned long long interval);
#endif

BaseType_t xPortSysTickHandler( void )
{
	BaseType_t ret;
	uint32_t interruptMask;

#if configUSE_TICKLESS_IDLE == 1
#ifndef configUSE_WAKEUP_SOURCE
	check_tickless_flag(1);
#else
	/* wakeup from sleep*/
	if (g_tickless_flag) {
		g_tickless_flag = 0;
		return pdTRUE;
	}
#endif
#endif  /* configUSE_TICKLESS_IDLE == 1 */
	portbenchmarkIntLatency();
#ifdef CFG_WDT_SUPPORT
	wdt_tickcount++;
	if (wdt_tickcount == 30) { //26M , 40ms timeout
		mtk_wdt_restart();
		wdt_tickcount = 0;
	}
#endif

	/* Interrupts upto configMAX_SYSCALL_INTERRUPT_PRIORITY must be
	 * disabled before calling xTaskIncrementTick as it access the
	 * kernel lists. */
	interruptMask = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		ret = xTaskIncrementTick();
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( interruptMask );

	portYIELD_FROM_ISR( ret );

	return ret;
}
/*-----------------------------------------------------------*/
/* Let user to get this value but not allow to modify it */
UBaseType_t xGetCriticalNesting(void)
{
	return 0;
}
/*-----------------------------------------------------------*/

/*
 * Used to set coprocessor area in stack. Current hack is to reuse MPU pointer for coprocessor area.
 */
#if portUSING_MPU_WRAPPERS
void vPortStoreTaskMPUSettings( xMPU_SETTINGS *xMPUSettings, const struct xMEMORY_REGION * const xRegions, StackType_t *pxBottomOfStack, uint32_t ulStackDepth )
{
	#if XCHAL_CP_NUM > 0
	//xMPUSettings->coproc_area = (StackType_t*)((((uint32_t)(pxBottomOfStack + ulStackDepth - 1)) - XT_CP_SIZE ) & ~0xf);

	uint32_t tmp = ((uint32_t)(pxBottomOfStack + ulStackDepth - 1)) & ((portPOINTER_SIZE_TYPE) ~portBYTE_ALIGNMENT_MASK);
	tmp = ((tmp - XT_CP_SIZE) & ~0xf);
	xMPUSettings->coproc_area = (StackType_t*) tmp;

	/* NOTE: we cannot initialize the coprocessor save area here because FreeRTOS is going to
         * clear the stack area after we return. This is done in pxPortInitialiseStack().
	 */
	#endif
}
#endif

#if configUSE_TICKLESS_IDLE == 1
static void tickless_process_after_sleep (uint32_t tick_compensation)
{
    UBaseType_t mask = taskENTER_CRITICAL_FROM_ISR();
    {   /* Restart wdt */
#ifdef CFG_WDT_SUPPORT
#ifdef configUSE_WAKEUP_SOURCE
        mtk_wdt_disable();
        mtk_wdt_enable();
        mtk_wdt_restart();
#else
        mtk_wdt_restart();
        mtk_wdt_enable();
#endif
#endif
    }
    {   /* Disable wake-up timer */
        xt_ints_off(XT_TIMER_INTEN);
    }
    {   /* Post-sleep processing */
        configPOST_SLEEP_PROCESSING(g_expected_idle_tick_count);
        g_sleep_debug_flag |= DBG_SPM_SRC_REQUEST;
    }
    {   /* Compensate tick count & setup tick interrupt */
        uint32_t cycle_curr = 0, cycle_inc;
        uint32_t tick_expect, tick_residual;
        uint64_t ost_curr;
        {   /* Sync os timer & current cycle count */
            __asm__ __volatile__("rsr %0, CCOUNT" : "=a"(cycle_curr));
            {   /* Get os timer, unit: 1/13000000 sec */
#ifdef CFG_XGPT_SUPPORT
                uint32_t lo1 = DRV_Reg32(OSTIMER_CUR_L);
                uint32_t hi1 = DRV_Reg32(OSTIMER_CUR_H);
                uint32_t lo2 = DRV_Reg32(OSTIMER_CUR_L);
                uint32_t hi2 = DRV_Reg32(OSTIMER_CUR_H);
                if (hi1 == hi2) {
                    ost_curr = ((uint64_t)hi1 << 32) | (uint64_t)lo1;
                } else {
                    ost_curr = ((uint64_t)hi2 << 32) | (uint64_t)lo2;
                    PRINTF_W("hi1 0x%08X lo1 0x%08X hi2 0x%08X lo2 0x%08X\n", hi1, lo1, hi2, lo2);
                }
#else
                ost_curr = 0;
#endif
            configASSERT(ost_curr >= g_os_timer_prev);
            }
        }
        {   /* Calculate expected tick count, tick residual, & cycle increment */
            uint64_t ost_base = g_os_timer_base;    /* Unit: 1/13000000 sec */
            uint64_t ost_diff = ost_curr - ost_base;
            uint64_t ost_tick = ost_diff / OS_TIMER_TICK_RATIO;
            uint64_t ost_residual = ost_diff - ost_tick * OS_TIMER_TICK_RATIO;
            uint32_t cycle_threshold = _xt_tick_divisor / TICK_THRESHOLD_DIVISOR;
            uint64_t cycle_residual = ost_residual * (uint64_t)_xt_tick_divisor / OS_TIMER_TICK_RATIO;
            tick_expect = (uint32_t)(ost_tick & 0xFFFFFFFF);
//            SLOGD("base=%lld,curr=%lld\n", ost_base, ost_curr);
            cycle_residual = UPPER_BOUND(cycle_residual, (uint64_t)cycle_threshold);
            if (ost_residual > OS_TIMER_THRESHOLD) {
                tick_residual = 1;
                cycle_inc = _xt_tick_divisor * 2 - (uint32_t)cycle_residual;
            } else {
                tick_residual = 0;
                cycle_inc = _xt_tick_divisor - (uint32_t)cycle_residual;
            }
            tick_expect += tick_residual;
            tick_expect += g_tick_base;
            g_os_timer_prev = ost_curr;
        }
        {   /* Compensate tick count */
            uint32_t tick_curr = xTaskGetTickCountFromISR();
            if (tick_expect != tick_curr) { // tick_inc >= 1
                uint32_t tick_tail = 0xFFFFFFFF - tick_curr;
                uint32_t tick_diff;
                uint32_t tick_inc;
                if (tick_expect > tick_curr) {  // normal tick
                    tick_inc = tick_expect - tick_curr;
                } else if (tick_expect + 1 == tick_curr) {  // non-tick interrupt comes too fast
                    tick_inc = 0;
                    PRINTF_W("too fast curr 0x%08X exp 0x%08X ost_curr %lld base %lld\n", tick_curr, tick_expect, ost_curr, g_os_timer_base);
                } else {    // tick overflow
                    tick_inc = tick_tail + tick_expect + 1;
                }
                if (tick_inc > 0) {
                    tick_diff = tick_inc - tick_compensation;   // tick_diff >= 0
#ifdef CFG_TASK_MONITOR
                    if (tick_diff >= MAXIMUM_TICK_DIFFERENCE) {
                        PRINTF_E("tick diff > max %d, curr 0x%08X exp 0x%08X ost_curr %lld base %lld\n", MAXIMUM_TICK_DIFFERENCE, tick_curr, tick_expect, ost_curr, g_os_timer_base);
                    }
#endif
                    tick_diff = UPPER_BOUND(tick_diff, g_expected_idle_tick_count);
                    if (tick_diff > tick_tail) {    // tick overflow handling
                        vTaskStepTick(tick_tail);
                        xTaskIncrementTick();
                        vTaskStepTick(tick_diff - tick_tail - 1);   // tick_diff - tick_tail > 0
                    } else {
                        vTaskStepTick(tick_diff);
                    }
                    xTaskIncrementTick();
                }
            }
        }
        {   /* Setup tick interrupt */
            uint32_t cycle_next = cycle_curr + cycle_inc;
            __asm__ __volatile__("wsr %0, CCOMPARE0" : : "a" (cycle_next) : "memory");
            __asm__ __volatile__("esync");
        }
    }
    {   /* Enable tick interrupt */
        xt_ints_on(XT_TIMER_INTEN);
    }
#ifdef CFG_VCORE_DVFS_SUPPORT
    adsp_update_sys_status(ADSP_STATUS_ACTIVE);
#endif
    taskEXIT_CRITICAL_FROM_ISR(mask);
}

void check_tickless_flag (uint32_t tick_compensation)
{   /* Avoid race condition by interrupt masking */
    UBaseType_t mask = portSET_INTERRUPT_MASK_FROM_ISR();
    if (g_tickless_flag) {
        g_tickless_flag = 0;
        __asm__ __volatile__("rsr %0, CCOUNT" : "=a"(g_cycle_post_sleep));
        tickless_process_after_sleep(tick_compensation);
        g_tick_sleep_dur = (g_cycle_post_sleep - g_cycle_pre_sleep) / SLEEP_CYCLE_PER_TICK;
//        SLOGD("sleep dur(%d)%d tick,expected:%d tick\n", tick_compensation, g_tick_sleep_dur, g_expected_idle_tick_count);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    /* may be interrupted here */
    taskENTER_CRITICAL();
    xExpectedIdleTime = xGetExpectedIdleTime();
    if (xExpectedIdleTime >= configEXPECTED_IDLE_TIME_BEFORE_SLEEP && eTaskConfirmSleepModeStatus() != eAbortSleep && can_enter_sleep(xExpectedIdleTime)) {
        g_sleep_debug_flag = 0;
#ifdef CFG_XGPT_SUPPORT
        g_os_timer_pre_sleep = timer_get_global_timer_tick();
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
        adsp_update_sys_status(ADSP_STATUS_SLEEP);
#endif
        g_expected_idle_tick_count = UPPER_BOUND(xExpectedIdleTime, MAX_SUPPRESSED_TICK_COUNT);
        {   /* Disable tick interrupt */
            xt_ints_off(XT_TIMER_INTEN);
        }
#ifdef CFG_WDT_SUPPORT
#ifdef configUSE_WAKEUP_SOURCE
        {
            uint32_t wdt_idle_count = TICK_TO_WDT_COUNT(g_expected_idle_tick_count);
            mtk_wdt_set_time_out_value(wdt_idle_count + WDT_TIMEOUT_COUNT);
            mtk_wdt_restart();
        }
#else
        mtk_wdt_disable();
#endif
#endif
#ifdef configUSE_WAKEUP_SOURCE
        {
            turn_off_non_wakeup_src_irq();
            unmask_irq(WAKEUP_SRC_IRQn);
            drv_write_reg32(ADSP_WAKEUPSRC_MASK, wakeup_src_mask);
            g_sleep_debug_flag |= DBG_WAKEUP_SRC_UNMASK;
        }
#endif
        {   /* Pre-sleep processing */
            configPRE_SLEEP_PROCESSING(g_expected_idle_tick_count);
            g_sleep_debug_flag |= DBG_SPM_SRC_RELEASE;
        }
        {   /* Setup wake-up timer */
            uint32_t cycle_curr = 0;
            uint32_t cycle_next;
            uint32_t cycle_inc = g_expected_idle_tick_count * SLEEP_CYCLE_PER_TICK;
            __asm__ __volatile__("rsr %0, CCOUNT" : "=a"(cycle_curr));
            cycle_next = cycle_curr + cycle_inc - WDT_DELAY_TIME;
            g_cycle_pre_sleep = cycle_curr;
            __asm__ __volatile__("wsr %0, CCOMPARE0" : : "a" (cycle_next) : "memory");
            __asm__ __volatile__("esync");
        }
        {   /* Enable wake-up timer */
            xt_ints_on(XT_TIMER_INTEN);
        }
        {
            g_tickless_flag = 1;
            g_sleep_debug_flag |= DBG_ENTER_WFI;
            /* Sleep until interrupts */
            __asm__ __volatile__("waiti 0");
        }
#ifdef configUSE_WAKEUP_SOURCE
        g_sleep_debug_flag |= DBG_EXIT_WFI;
        tickless_process_after_sleep(1);
#ifdef CFG_XGPT_SUPPORT
        g_os_timer_post_sleep = timer_get_global_timer_tick();
#endif
        taskEXIT_CRITICAL();
        {   /* Mask wakeup source IRQ & turn on all IRQ */
            drv_write_reg32(ADSP_WAKEUPSRC_MASK, 0xFFFFFFFF);
            mask_irq(WAKEUP_SRC_IRQn);
            turn_on_non_wakeup_src_irq();
            g_sleep_debug_flag |= DBG_WAKEUP_SRC_MASK;
        }
#else
#ifdef CFG_XGPT_SUPPORT
        g_os_timer_post_sleep = timer_get_global_timer_tick();
#endif
        taskEXIT_CRITICAL();
#endif /* configUSE_WAKEUP_SOURCE */
#ifdef CFG_XGPT_SUPPORT
        g_sleep_tick_us = (uint32_t)((g_os_timer_post_sleep - g_os_timer_pre_sleep) / 13);
#endif
    } else {
        taskEXIT_CRITICAL();
    }
}

void print_tickless_status (void)
{
    uint32_t tick_curr = xTaskGetTickCountFromISR();
    uint64_t ost_curr;
    {   /* Get os timer, unit: 1/13000000 sec */
#ifdef CFG_XGPT_SUPPORT
        uint32_t lo1 = DRV_Reg32(OSTIMER_CUR_L);
        uint32_t hi1 = DRV_Reg32(OSTIMER_CUR_H);
        uint32_t lo2 = DRV_Reg32(OSTIMER_CUR_L);
        uint32_t hi2 = DRV_Reg32(OSTIMER_CUR_H);
        if (hi1 == hi2) {
            ost_curr = ((uint64_t)hi1 << 32) | (uint64_t)lo1;
        } else {
            ost_curr = ((uint64_t)hi2 << 32) | (uint64_t)lo2;
            PRINTF_W("hi1 0x%08X lo1 0x%08X hi2 0x%08X lo2 0x%08X\n", hi1, lo1, hi2, lo2);
        }
#else
        ost_curr = 0;
#endif
        PRINTF_D("tick_curr 0x%08X ost_curr %lld base %lld\n", tick_curr, ost_curr, g_os_timer_base);
    }
}

#endif  /* #if configUSE_TICKLESS_IDLE */
