/*
 * FreeRTOS Kernel V10.0.1
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
 * copies or substantial portions of the Software.
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


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */
#include <stdint.h>
extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION             1
#define configUSE_IDLE_HOOK              1
#define configUSE_TICK_HOOK              1
#ifndef configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE          1
#endif
#define configCPU_CLOCK_HZ               ( SystemCoreClock )
#define configTICK_RATE_HZ               ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES             ( 5 )
#define configMINIMAL_STACK_SIZE         ( ( unsigned short ) 150 )

#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE ( ( size_t ) ( 80 * 1024 ) )
#endif

#define configMAX_TASK_NAME_LEN          ( 6 )
#define configUSE_TRACE_FACILITY         1
#define configUSE_16_BIT_TICKS           0
#define configIDLE_SHOULD_YIELD          1
#define configUSE_MUTEXES                1
#define configQUEUE_REGISTRY_SIZE        8
#define configCHECK_FOR_STACK_OVERFLOW   2
#define configUSE_RECURSIVE_MUTEXES      0
#define configUSE_MALLOC_FAILED_HOOK     1
#define configUSE_APPLICATION_TASK_TAG   0
#define configUSE_COUNTING_SEMAPHORES    1
#define configGENERATE_RUN_TIME_STATS    0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES            0
#define configMAX_CO_ROUTINE_PRIORITIES  ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                 1
#define configTIMER_TASK_PRIORITY        ( 2 )
#define configTIMER_QUEUE_LENGTH         15
#define configTIMER_TASK_STACK_DEPTH     ( configMINIMAL_STACK_SIZE * 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet         1
#define INCLUDE_uxTaskPriorityGet        1
#define INCLUDE_vTaskDelete              1
#define INCLUDE_vTaskCleanUpResources    1
#define INCLUDE_vTaskSuspend             1
#define INCLUDE_vTaskDelayUntil          1
#define INCLUDE_vTaskDelay               1
#define configUSE_STATS_FORMATTING_FUNCTIONS    2
#define INCLUDE_pcTaskGetTaskName               1
#define INCLUDE_xTimerPendFunctionCall          1
/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS               __NVIC_PRIO_BITS
#else
#define configPRIO_BITS             4 /* 15 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0xf

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
extern void vAssertCalled(char* file, uint32_t line);
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); vAssertCalled( __FILE__, __LINE__ ); }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_To1stTask
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

extern void pre_sleep_process(uint32_t);
#ifndef configPRE_SLEEP_PROCESSING
#define configPRE_SLEEP_PROCESSING( x )     pre_sleep_process( x )
#endif
extern void post_sleep_process(uint32_t);
#ifndef configPOST_SLEEP_PROCESSING
#define configPOST_SLEEP_PROCESSING( x )      post_sleep_process( x )
#endif

#ifdef CFG_FREERTOS_TRACE_SUPPORT
#include "met_freertos_trace.h"
#else
/* Trace Macro Definitions*/
#include "FreeRTOSConfigTraceMacro.h"
#ifndef traceISR_ENTER
#define traceISR_ENTER()
#endif
#ifndef traceISR_EXIT
#define traceISR_EXIT()
#endif
#ifndef traceINT_ON
#define traceINT_ON()
#endif
#ifndef traceINT_OFF
#define traceINT_OFF()
#endif

#endif

/****************************************************************************/
/* Constant Priority Assigned for Tasks                                     */
/****************************************************************************/
#define PRI_DMGR        (configMAX_PRIORITIES - 1) /* 5 */
#define PRI_DMA         (configMAX_PRIORITIES - 2) /* 4 */
#define PRI_CONSOLE     (configMAX_PRIORITIES - 2) /* 4 */
#define PRI_ADSP_IPI    (configMAX_PRIORITIES - 2) /* 4 */
#define PRI_TIMER       configTIMER_TASK_PRIORITY  /* 2 */
#define PRI_TEST        (configMAX_PRIORITIES - 5) /* 1 */
#define PRI_MONITOR     (configMAX_PRIORITIES - 6) /* 0 */
#define PRI_STRESS      (configMAX_PRIORITIES - 6) /* 0 */


/****************************************************************************/

#ifdef CFG_PBFR_SUPPORT
#undef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY    1
#define FREERTOS_MAX_TASKS 8
#define FREERTOS_MAX_QUEUES 22
#define FREERTOS_MAX_SWTIMER 10

#define PBFR_SUPPORT_PERF_BUDGET
//#define PBFR_SUPPORT_FLIGHT_REC
/* Performance Budget Support */
#ifdef PBFR_SUPPORT_PERF_BUDGET
#define PBFR_SUPPORT_POLLING
//#define PBFR_SUPPORT_WORKSLOT_MONITOR
//#define PBFR_SUPPORT_IOSTALL  /* move to platform.mk for asm definition */
#define PBFR_SUPPORT_CACHE_COUNT
//#define PBFR_SUPPORT_TASK_CHECKER
#define configMCU_LOADING_CRITERIA  30
#ifdef PBFR_SUPPORT_TASK_CHECKER
#define PBFR_SUPPORT_DELAY_API
#define configMCU_FREQ_MHz          26
#endif /* PBFR_SUPPORT_TASK_CHECKER */
#endif /* PBFR_SUPPORT_PERF_BUDGET */
/* Flight Recorder Support */
#ifdef PBFR_SUPPORT_FLIGHT_REC
#define PBFR_MAX_REC_EVENTS         32
#define PBFR_SUPPORT_REC_TASK
#define PBFR_SUPPORT_REC_QUEUE
#define PBFR_SUPPORT_REC_SWTIMER
#define PBFR_SUPPORT_REC_INT
#define PBFR_SUPPORT_REC_ISR
#define PBFR_SUPPORT_REC_OSTICK
//#define CFG_PBFR_FLIGHT_REC_DEBUG
#endif /* PBFR_SUPPORT_FLIGHT_REC */
#include "pbfr_cm4.h"
#endif /* CFG_PBFR_SUPPORT */

/* The MCU needs 1ms to go into and wake up from sleep.
 * 0.3ms to sleep, 0.7ms from sleep to wake up.
 */
#define portMCU_WAKEUP_COST                ( 1 )
#define portTIME_SKEW_FACTOR               ( 10 )

#ifdef TINYSYS_DEBUG_BUILD
#define DEBUGLEVEL 3
#else
#define DEBUGLEVEL 1
#endif

#define UNUSED(x)       (void)(x)

#include <mt_printf.h>

#define kal_xTaskCreate(x...) xTaskCreate(x)
#define kal_taskENTER_CRITICAL() taskENTER_CRITICAL()
#define kal_taskEXIT_CRITICAL() taskEXIT_CRITICAL()
#define kal_vTaskSuspendAll() vTaskSuspendAll()
#define kal_xTaskResumeAll() xTaskResumeAll()
#define kal_pvPortMalloc(x) pvPortMalloc(x)
#define kal_vPortFree(x) vPortFree(x)

#endif /* FREERTOS_CONFIG_H */
