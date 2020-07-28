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


/* Required for configuration-dependent settings */
#include "xtensa_config.h"
#include <mt_printf.h>
/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * Note that the default heap size is deliberately kept small so that
 * the build is more likely to succeed for configurations with limited
 * memory.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *----------------------------------------------------------*/




#define configUSE_PREEMPTION             1
#define configUSE_IDLE_HOOK              1

#define configUSE_TICK_HOOK              1
#ifndef configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE          1
#endif

#define configCPU_CLOCK_HZ               ( 466666463 ) //check this : FPGA 10M
#define configTICK_RATE_HZ               ( 1000 )

/* Default clock rate for simulator */

/* This has impact on speed of search for highest priority */
#define configMAX_PRIORITIES             ( 5 )

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         1

/* Minimal stack size. This may need to be increased for your application */
/* NOTE: The FreeRTOS demos may not work reliably with stack size < 4KB.  */
/* The Xtensa-specific examples should be fine with XT_STACK_MIN_SIZE.    */
#if !(defined XT_STACK_MIN_SIZE)
#error XT_STACK_MIN_SIZE not defined, did you include xtensa_config.h ?
#endif

#define configMINIMAL_STACK_SIZE        (256) //(XT_STACK_MIN_SIZE > 1024 ? XT_STACK_MIN_SIZE : 1024)

/* The Xtensa port uses a separate interrupt stack. Adjust the stack size */
/* to suit the needs of your specific application.                        */
#ifndef configISR_STACK_SIZE
#define configISR_STACK_SIZE            2048
#endif

/* Minimal heap size to make sure examples can run on memory limited
   configs. Adjust this to suit your system. */
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE           ( ( size_t ) (256 * 1024) )
#endif

#define configMAX_TASK_NAME_LEN         ( 8 )
#define configUSE_TRACE_FACILITY        1       /* Used by vTaskList in main.c */
#define configUSE_TRACE_FACILITY_2      0       /* Provided by Xtensa port patch */
#define configBENCHMARK                 0       /* Provided by Xtensa port patch */
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1

#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       8
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configUSE_RECURSIVE_MUTEXES      1
#define configUSE_MALLOC_FAILED_HOOK     1
#define configUSE_APPLICATION_TASK_TAG   0
#define configUSE_COUNTING_SEMAPHORES   1
#define configGENERATE_RUN_TIME_STATS    0
#define configUSE_QUEUE_SETS                1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Test FreeRTOS timers (with timer task) and more. */
/* Some files don't compile if this flag is disabled */
#define configUSE_TIMERS                    1
#define configTIMER_TASK_PRIORITY           2
#define configTIMER_QUEUE_LENGTH            10
#define configTIMER_TASK_STACK_DEPTH        (configMINIMAL_STACK_SIZE * 3)
/* Set the following definitions to 1 to include the API function, or zero
   to exclude the API function. */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1   /* Used by vTaskList in main.c */
#define INCLUDE_pcTaskGetTaskName               1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskGetIdleTaskHandle          1

/* Extension API add by MTK */
#define INCLUDE_xTaskSetSchedulerRunning        1

/* The priority at which the tick interrupt runs.  This should probably be
   kept at 1. */
#define configKERNEL_INTERRUPT_PRIORITY     1

/* The maximum interrupt priority from which FreeRTOS.org API functions can
   be called.  Only API functions that end in ...FromISR() can be used within
   interrupts. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    XCHAL_EXCM_LEVEL

#ifdef CFG_ASSERT_SUPPORT
#ifndef __ASSEMBLER__
extern void platform_assert(char* file, unsigned int line);
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); platform_assert( __FILE__, __LINE__ ); }
#endif /*__ASSEMBLER__*/
#endif

/* XT_USE_THREAD_SAFE_CLIB is defined in xtensa_config.h and can be
   overridden from the compiler/make command line. The small test*/
#ifndef __ASSEMBLER__
extern void pre_sleep_process(unsigned int modify_time);
#ifndef configPRE_SLEEP_PROCESSING
#define configPRE_SLEEP_PROCESSING( x )    pre_sleep_process( x )
#endif
extern void post_sleep_process(unsigned int expect_time);
#ifndef configPOST_SLEEP_PROCESSING
#define configPOST_SLEEP_PROCESSING( x )   post_sleep_process( x )
#endif
#endif /*__ASSEMBLER__*/

/* Specific config for XTENSA (these can be deleted and they will take default values) */

#if (!defined XT_SIMULATOR) && (!defined XT_BOARD)
#define configXT_SIMULATOR                  0   /* Simulator mode */
#define configXT_BOARD                      1   /* Board mode */
#endif


#if (!defined XT_INTEXC_HOOKS)
#define configXT_INTEXC_HOOKS               1   /* Exception hooks used by certain tests */
#endif

#if configUSE_TRACE_FACILITY_2
#define configASSERT_2                      1   /* Specific to Xtensa port */
#endif

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 3
#endif

/* extend mtk memory management tool */
/* feature 1 : Monitor heap memory usage */
#define configUSE_MALLOC_MEMORY_USAGE
/* feature 2 : Detail information of allocate memory */
#define configUSE_MALLOC_DEBUG_MSG_LEN      32
#define kal_pvPortMalloc(sz) \
    ({ \
        void *__ptr = (uint8_t *)pvPortMalloc((sz)); \
        vPortMallocSetMsg(__ptr, __FILE__, __LINE__); \
        __ptr; \
    })
/* end extend mtk */

#define kal_xTaskCreate(x...) xTaskCreate(x)
#define kal_taskENTER_CRITICAL() taskENTER_CRITICAL()
#define kal_taskEXIT_CRITICAL() taskEXIT_CRITICAL()
#define kal_vTaskSuspendAll() vTaskSuspendAll()
#define kal_xTaskResumeAll() xTaskResumeAll()
//#define kal_pvPortMalloc(x) pvPortMalloc(x)
#define kal_vPortFree(x) vPortFree(x)
#endif /* FREERTOS_CONFIG_H */

