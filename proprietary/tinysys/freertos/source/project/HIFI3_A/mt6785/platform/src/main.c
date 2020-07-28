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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "interrupt.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <event_groups.h>
#include <driver_api.h>
#ifdef CFG_UART_SUPPORT
#include <mt_uart.h>
#endif
#ifdef CFG_ADSP_LOGGER_SUPPORT
#include <adsp_logger.h>
#endif
#ifdef CFG_IPC_SUPPORT
#include <adsp_ipi.h>
#endif
/* timestamp */
#ifdef CFG_WDT_SUPPORT
#include "wdt.h"
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#include <sleep.h>
#endif
#ifdef CFG_DMA_SUPPORT
#include <dma.h>
#endif
#ifdef CFG_XGPT_SUPPORT
#include "mt_gpt.h"
#endif
#ifdef CFG_MPU_SUPPORT
#include "mpu_mtk.h"
#endif
#ifdef CFG_TRAX_SUPPORT
#include <adsp_trax.h>
#endif
#ifdef CFG_CACHE_SUPPORT
#include "hal_cache.h"
#endif
#ifdef CFG_WAKELOCK_SUPPORT
#include "wakelock.h"
#include "ap_awake.h"
#endif  /* CFG_WAKELOCK_SUPPORT */
#ifdef CFG_BUS_MONITOR_SUPPORT
#include "bus_monitor.h"
#endif  /* CFG_BUS_MONITOR_SUPPORT */
#ifdef CFG_PERFORMANCE_MONITOR_SUPPORT
#include "perf_mon.h"
#endif  /* CFG_PERFORMANCE_MONITOR_SUPPORT */
#ifdef CFG_UT_TESTSUITE_SUPPORT
#include "test_main.h"
#endif  /* CFG_AP_AWAKE_SUPPORT */

/* Private Macro ---------------------------------------------------------*/
#define CREG_BOOTUP_MARK   (ADSP_CFGREG_RSV_RW_REG0)  /* the reg must sync with ctrl-boards.S */
#define MAGIC_PATTERN      0xfafafafa   /* if equal, bypass clear bss and some init*/
#define IS_HW_REBOOT      (DRV_Reg32(CREG_BOOTUP_MARK) != MAGIC_PATTERN)

static StaticTask_t xIdleTaskTCB INTERNAL_NON_INITIAL_DATA;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ] INTERNAL_NON_INITIAL_DATA;

static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

/* Private functions ---------------------------------------------------------*/
static void prvSetupHardware(boot_mode_t boot_mode);
static void adsp_suspend_wait_sw_rst(void);
static int main_lite(void);
__inline__ void loop_buffer_patch(void);

/* Extern functions ---------------------------------------------------------*/
extern void platform_init(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    if (!IS_HW_REBOOT) {
        main_lite(); /* lite version: Never return */
    } else {
        DRV_WriteReg32(CREG_BOOTUP_MARK, MAGIC_PATTERN);
    }

#ifdef CFG_IPC_SUPPORT
    uint32_t buf = (CFG_HIFI3_IRAM_SIZE + CFG_HIFI3_DRAM_SIZE); //TCM total size
#endif
    /* init irq handler */
    irq_init();

    /* Configure the hardware, after this point, the UART can be used*/
    prvSetupHardware(HW_BOOT);

    /* start to use UART */
    PRINTF_E("\n\rFreeRTOS %s %x \n\r", tskKERNEL_VERSION_NUMBER,
             XCHAL_INTLEVEL1_MASK);

    platform_init();
    //project_init();
#ifdef CFG_WAKELOCK_SUPPORT
    wakelocks_init();
    ap_awake_init();
#endif  /* CFG_WAKELOCK_SUPPORT */
#ifdef CFG_IPC_SUPPORT
    PRINTF_E("send ready IPI\n");
    while (adsp_ipi_send(IPI_ADSP_A_READY, &buf, sizeof(buf), 1, IPI_ADSP2AP) != DONE);
    PRINTF_E("send ready IPI done\n");
#endif
#ifdef CFG_ADSP_LOGGER_SUPPORT
    /* init logger */
    logger_init();
#endif
#ifdef CFG_TRAX_SUPPORT
    // if trax feature enable
    adsp_trax_init();
#endif
#ifdef CFG_WDT_SUPPORT
    /*put wdt init just before insterrupt enabled in vTaskStartScheduler()*/
    mtk_wdt_init();
#endif
#ifdef CFG_IRQ_MONITOR_SUPPORT
    set_irq_limit(1);
#endif
#ifdef CFG_UT_TESTSUITE_SUPPORT
    testsuite_init();
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
    adsp_update_sys_status(ADSP_STATUS_ACTIVE);
#endif
    PRINTF_E("Scheduler start...\n");

    /* Start the scheduler. After this point, the interrupt is enabled*/
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
      will never be reached.  If the following line does execute, then there was
      insufficient FreeRTOS heap memory available for the idle and/or timer tasks
      to be created.  See the memory management section on the FreeRTOS web site
      for more details. */
    adsp_suspend_wait_sw_rst(); /* if end scheduler, back here */
    for (;;);
}

/**
  * @brief  Main lite program
  * @param  None
  * @retval None
  */
int main_lite(void)
{
#ifdef CFG_IPC_SUPPORT
    uint32_t buf = (CFG_HIFI3_IRAM_SIZE + CFG_HIFI3_DRAM_SIZE); //TCM total size
#endif
    irq_sync();
    prvSetupHardware(ADSP_REBOOT);
#ifdef CFG_IPC_SUPPORT
    adsp_ipi_send(IPI_ADSP_A_READY, &buf, sizeof(buf), 1, IPI_ADSP2AP);
#endif
#ifdef CFG_WDT_SUPPORT
    /*put wdt init just before insterrupt enabled in vTaskStartScheduler()*/
    mtk_wdt_init();
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
    adsp_update_sys_status(ADSP_STATUS_ACTIVE);
#endif
    PRINTF_E("Scheduler re-start...\n");
    xPortStartScheduler();

    adsp_suspend_wait_sw_rst(); /* if end scheduler, back here */
    for (;;);
}

void adsp_suspend_wait_sw_rst(void)
{
    PRINTF_D("[%s] going to WFI\n", __FUNCTION__);

#ifdef CFG_CACHE_SUPPORT
    hal_cache_all_operation(HAL_CACHE_TYPE_INSTRUCTION,
                            HAL_CACHE_OPERATION_INVALIDATE);
    hal_cache_all_operation(HAL_CACHE_TYPE_DATA,
                            HAL_CACHE_OPERATION_FLUSH_INVALIDATE);
#endif
#ifdef CFG_WDT_SUPPORT
    mtk_wdt_disable();
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
    adsp_disable_dram_resource();
    adsp_update_sys_status(ADSP_STATUS_SUSPEND);
#endif

    /*put hifi3 in memw & idle*/
    while (1) {
        __asm(
            "memw;\n"
            "WAITI 15;\n"
        );
    }
}

/**
  * @brief  Hardware init
  * @param  None
  * @retval None
  */
static void prvSetupHardware(boot_mode_t boot_mode)
{
#ifdef CFG_UART_SUPPORT
    /* init UART before any printf function all */
    uart_init(UART_LOG_PORT, UART_LOG_BAUD_RATE, 0);
#endif
#ifdef CFG_MPU_SUPPORT
    /*set MPU*/
    mpu_init();
#endif

#ifdef CFG_CACHE_SUPPORT
    hal_cache_init();
#endif

    if (boot_mode == HW_BOOT) {
#ifdef CFG_BUS_MONITOR_SUPPORT
        bus_monitor_init();
#endif
#ifdef CFG_IPC_SUPPORT
        adsp_ipi_init();
#endif
#ifdef CFG_DMA_SUPPORT
        mt_init_dma();
#ifdef CFG_MPU_SUPPORT
        set_scp_addr_map(MEM_ID_HIFI3_SRAM, CFG_HIFI3_SRAM_ADDRESS,
                         CFG_HIFI3_SRAM_ADDRESS, mpu_get_adsp_sram_prot_size());
#endif
#endif
    }
#ifdef CFG_XGPT_SUPPORT
    mt_platform_timer_init();
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
   dvfs_init(boot_mode);
#endif

#ifdef CFG_PERFORMANCE_MONITOR_SUPPORT
    perf_mon_init();
#endif

    loop_buffer_patch();    /* Patch for loop buffer hardware issue */
    DRV_WriteReg32(ADSP_DCM_CTRL, 0x1);
}

/*-----------------------------------------------------------*/
__inline__ unsigned int read_ccount(void)
{
    unsigned int ccount;
    __asm__ __volatile__ ( "rsr %0, ccount": "=a" (ccount) : : "memory" );
    return ccount;
}
/*-----------------------------------------------------------*/
__inline__ void loop_buffer_patch(void)
{
    uint32_t memctl = 0;
    __asm__ __volatile__("rsr %0, MEMCTL" : "=a"(memctl));
    memctl &= ~0x1;
    //PRINTF_E("MEMCTL = 0x%08X\n", memctl);
    __asm__ __volatile__("wsr %0, MEMCTL" : : "a" (memctl) : "memory");
    __asm__ __volatile__("esync");
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
    /* If configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2 then this
    function will automatically get called if a task overflows its stack. */
    (void) pxTask;
    (void) pcTaskName;
    PRINTF_E("\n\rtask: %s stack overflow\n", pcTaskName);
    configASSERT(0);
    for (;;);
}
/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook(void)
{
    /* If configUSE_MALLOC_FAILED_HOOK is set to 1 then this function will
    be called automatically if a call to pvPortMalloc() fails.  pvPortMalloc()
    is called automatically when a task, queue or semaphore is created. */
    PRINTF_E("\n\rmalloc fail\n\r");
    PRINTF_D("Heap:free size/total size:%d/%d\n", xPortGetFreeHeapSize(),
             configTOTAL_HEAP_SIZE);
#if defined(configUSE_MALLOC_MEMORY_USAGE) && defined(configUSE_MALLOC_DEBUG_MSG_LEN)
    vPortHeapDump(NULL, 0);
#endif
    configASSERT(0);
    for (;;);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
#ifdef CFG_SYSTEM_OFF_SUPPORT
    EventBits_t uxBits;

    uxBits = xEventGroupClearBits(xSysEventGroup, EVENTBIT_SUSPEND);

    if ((uxBits & EVENTBIT_SUSPEND) == EVENTBIT_SUSPEND) {
        vTaskPrioritySet(xTaskGetIdleTaskHandle(), configMAX_PRIORITIES - 1);
        vTaskEndScheduler();
        vTaskPrioritySet(xTaskGetIdleTaskHandle(), 0);
    }
#endif
}

void vApplicationTickHook(void)
{
    //printf("\n\r Enter TickHook \n\r");
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
