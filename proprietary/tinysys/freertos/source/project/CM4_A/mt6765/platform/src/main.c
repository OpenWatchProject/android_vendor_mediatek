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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "interrupt.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include <scp_ipi.h>
#ifdef CFG_LOGGER_SUPPORT
#include <scp_logger.h>
#endif
#include <driver_api.h>
#include <mt_uart.h>
/* timestamp */
#include <utils.h>
/*  console includes. */
#ifdef CFG_TESTSUITE_SUPPORT
#include <console.h>
#endif
#ifdef CFG_WDT_SUPPORT
#include "wdt.h"
#endif
#ifdef CFG_EINT_SUPPORT
#include <eint.h>
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#ifdef CFG_DMA_SUPPORT
#include <dma.h>
#endif
#include <unwind.h> // to support pc back trace dump
#ifdef CFG_CCCI_SUPPORT
extern void ccci_init(void);
#endif
#ifdef CFG_XGPT_SUPPORT
#include "mt_gpt.h"
#endif
#ifdef CFG_DWT_SUPPORT
#include "dwt.h"
#endif
#ifdef CFG_SPI_SUPPORT
#ifdef CFG_CHRE_SUPPORT
#include <plat/inc/spichre.h>
#include "spichre-plat.h"
#endif
#endif
#ifdef CFG_I2C_SUPPORT
#ifdef CFG_CHRE_SUPPORT
#include "i2cchre-plat.h"
#endif
#endif
#ifdef CFG_MPU_DEBUG_SUPPORT
#include "mpu_mtk.h"
#endif
#ifdef CFG_PMIC_WRAP_SUPPORT
#include "pmic_wrap.h"
#endif
#ifdef CFG_OVERLAY_INIT_SUPPORT
#include "mtk_overlay_init.h"
#endif
#include <wakelock.h>

#ifdef CFG_DO_ENABLED
#include <dynamic_object.h>
#endif

#ifdef CFG_CACHE_SUPPORT
#include <cache_internal.h>
#include <cache_dram_management.h>
#endif

#include <scp_scpctl.h>

/* Private functions ---------------------------------------------------------*/
static void prvSetupHardware(void);
/* Extern functions ---------------------------------------------------------*/
extern void platform_init(void);
extern void platform_post_init();
extern void ap_wdt_irq_init(void);
extern void project_init(void);
extern void mt_init_dramc(void);
extern void scp_remap_init(void);
extern void mt_ram_dump_init(void);
#ifdef CFG_CACHE_SUPPORT
extern void l1c_remap(void);
extern void l1c_recovery(void);
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    mt_cleanup_dma_operations();

    /* init irq handler */
    irq_init();

#ifdef CFG_OVERLAY_INIT_SUPPORT
    /* overlay struct init*/
    tinysys_overlay_init();
#endif

#ifdef CFG_XGPT_SUPPORT
    /* init timestamp */
    timestamp_init();
#endif
#ifndef CFG_ONE_IPI_IRQ_DESIGN
    /* init awake lock before any awake IPI call */
    scp_awake_init();
#endif
    /* wakelock list init*/
    wakelocks_init();
    /* Configure the hardware, after this point, the UART can be used*/
    prvSetupHardware();

    /* start to use UART */
    PRINTF_E("\n\rFreeRTOS %s(build:%s %s) \n\r", tskKERNEL_VERSION_NUMBER, __DATE__, __TIME__);

#ifdef CFG_SLT_SUPPORT
    extern void vSCPSLTFunction(void);
    /*this function will not return, it will call scheduler in it.*/
    vSCPSLTFunction();
#endif

    platform_init();
    project_init();
#ifndef CFG_DVT_SUPPORT
    extern uint32_t _SCP_SRAM_SIZE;
    uint32_t buf = ((volatile unsigned int) & (_SCP_SRAM_SIZE));

    /**************************************************************************
     * It is important to call mt_ram_dump_init() before the IPI_SCP_A_READY.
     * The reason is that the TaskContext_ptr field in the region info can be
     * read when the IPI is received at AP side, .
     *************************************************************************/
#ifdef CFG_RAMDUMP_SUPPORT
    mt_ram_dump_init();
#endif

    /* send ready IPI when linux boot */
    PRINTF_E("send ready IPI\n");
    while (scp_ipi_send(IPI_SCP_A_READY, &buf, sizeof(buf), 1, IPI_SCP2AP) != DONE);
#endif

#ifdef CFG_LOGGER_SUPPORT
    /* init logger */
    logger_init();
#endif

#ifdef CFG_TESTSUITE_SUPPORT
    PRINTF_E("support TESTSUITE\n");
    console_start();
#endif

#ifdef CFG_WDT_SUPPORT
    /*put wdt init just before insterrupt enabled in vTaskStartScheduler()*/
    mtk_wdt_init();
#endif
#ifdef CFG_PMIC_WRAP_SUPPORT
    pmic_wrap_init();
#endif
#ifdef CFG_IRQ_MONITOR_SUPPORT
    set_irq_limit(1);
#endif

    platform_post_init();

    PRINTF_E("Scheduler start...\n");
    /* Start the scheduler. After this point, the interrupt is enabled*/
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}
/*-----------------------------------------------------------*/
/**
  * @brief  Hardware init
  * @param  None
  * @retval None
  */
static void prvSetupHardware(void)
{
#ifdef CFG_VCORE_DVFS_SUPPORT
    /* It had better do this reset as earlier as possible */
    /* becasue the APIs used by other modules will use them */
    reset_dvfs_global_variables();
#endif

#ifdef CFG_CM4_MODIFICATION
    DRV_WriteReg32(CM4_MOIFICATION_EMABLE, 0x3);
    DRV_WriteReg32(CM4_DCM_FEATURE, 0x3);
#endif
    /* CIRQ enable all interrrupt from peripheral to CM4 */
    DRV_WriteReg32(SCP_INTC_IRQ_ENABLE, 0xFFFFFFF5);
    DRV_WriteReg32(SCP_INTC_IRQ_ENABLE_MSB, 0xFFFFFFFF);

    /* enable scp clk gates*/
    DRV_WriteReg32(SCP_SET_CLK_CG, DRV_Reg32(SCP_SET_CLK_CG) | (1 << DMA_CH3_CG) | (1 << DMA_CH2_CG) |
                   (1 << DMA_CH1_CG) | (1 << DMA_CH0_CG) | (1 << I2C_MCLK_CG) | (1 << MAD_MCLK_CG));
    //DRV_WriteReg32(0xA02D03E0, (DRV_Reg32(0xA02D03E0)&0x00FFFFFF)|0x44000000);

#ifdef CFG_UART_SUPPORT
    /* init UART before any printf function all */
    uart_init(UART_LOG_PORT, UART_LOG_BAUD_RATE, 0);
#endif

#ifdef CFG_CACHE_SUPPORT
    l1c_remap();
#endif

    /* init remap register before any dram access*/
    scp_remap_init();
    /* init IPI before any IPI all */
    scp_ipi_init();
    ap_wdt_irq_init();
#ifdef CFG_VCORE_DVFS_SUPPORT
    /* enable plaform wakeup source */
    scp_wakeup_src_setup(IPC0_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(IPC1_WAKE_CLK_CTRL, 0);
    scp_wakeup_src_setup(IPC2_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(IPC3_WAKE_CLK_CTRL, 0);
    scp_wakeup_src_setup(CIRQ_WAKE_CLK_CTRL, 0);
    scp_wakeup_src_setup(EINT_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(DMA_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(INFRA_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(AP_WDT_WAKE_CLK_CTRL, 1);
#endif

#ifdef CFG_DMA_SUPPORT
    /* init DMA */
    mt_init_dma();
#endif

#ifdef CFG_EINT_SUPPORT
    /* init EINT */
    mt_eint_init();
#endif
#ifdef CFG_DRAMC_MONITOR_SUPPORT
    /* gating auto save  */
    mt_init_dramc();
#endif

#ifdef CFG_CCCI_SUPPORT
    ccci_init();
#endif

#ifdef CFG_XGPT_SUPPORT
    mt_platform_timer_init();
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_init();
#endif

#ifdef CFG_CACHE_SUPPORT
    /* recover dram image for backup section */
    l1c_recovery();
#endif

#ifdef CFG_MPU_DEBUG_SUPPORT
        /*set MPU*/
    mpu_init();
#endif

#ifdef CFG_DWT_SUPPORT
    /* init data watchpoint & trace */
    dwt_init();
#endif
#ifdef CFG_SPI_SUPPORT
#ifdef CFG_CHRE_SUPPORT
    spi_init();
#endif
#endif
#ifdef CFG_I2C_SUPPORT
#ifdef CFG_CHRE_SUPPORT
    i2c_init();
#endif
#endif
}

#ifdef CFG_DO_ENABLED
extern const uint32_t __do_ws_start;
extern const uint32_t __do_ws_end;
#endif
static uint32_t last_pc = 0x0;
static uint32_t current_pc = 0x0;
_Unwind_Reason_Code trace_func(_Unwind_Context *ctx, void *d)
{
    current_pc = _Unwind_GetIP(ctx);
    if (current_pc != last_pc) {
#ifdef CFG_DO_ENABLED
        if ((current_pc > (uint32_t)&__do_ws_start) && (current_pc < (uint32_t)(&__do_ws_end))) {
            char* current_do = mt_do_get_loaded_do();
            PRINTF_E("0x%08x [%s +0x%x]\n", current_pc, current_do, current_pc - (uint32_t)(&__do_ws_start));
        } else {
            PRINTF_E("0x%08x\n", (unsigned int)current_pc);
        }
#else
        PRINTF_E("0x%08x\n", (unsigned int)current_pc);
#endif
        last_pc = current_pc;
        return _URC_NO_REASON;
    } else {
        taskDISABLE_INTERRUPTS();
        /* disable INTC interrupt*/
        DRV_WriteReg32(SCP_INTC_IRQ_ENABLE, 0x00000000);
        DRV_WriteReg32(SCP_INTC_IRQ_ENABLE_MSB, 0x00000000 | DBG_IRQ_EN | WDT_IRQ_EN);

        PRINTF_E("==PC backtrace dump end==\n");
#ifdef CFG_WDT_SUPPORT
        mtk_wdt_set_time_out_value(10);/*assign a small value to make ee sooner*/
        mtk_wdt_restart();
#endif
        while (1);
    }
}

void print_backtrace(void)
{
    int depth = 0;

    _Unwind_Backtrace(&trace_func, &depth);
}
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void vAssertCalled(char* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    PRINTF_E("[ASSERT]task:%s,file:%s,line:%lu\r\n", pcTaskGetTaskName(NULL), file, line);
    PRINTF_E("=PC backtrace dump start=\n");
    print_backtrace();
    PRINTF_E("=PC backtrace dump end=\n");
    taskDISABLE_INTERRUPTS();
#ifdef CFG_WDT_SUPPORT
    mtk_wdt_set_time_out_value(10);/*assign a small value to make ee sooner*/
    mtk_wdt_restart();
#endif
    /* Halt CPU */
    while (1) {
        __asm volatile("dsb");
        __asm volatile("wfi");
    }
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
    /* If configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2 then this
    function will automatically get called if a task overflows its stack. */
    (void) pxTask;
    (void) pcTaskName;
    PRINTF_E("\n\rtask:%s stack overflow\n\r", pcTaskName);
    for (;;);
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* If configUSE_MALLOC_FAILED_HOOK is set to 1 then this function will
    be called automatically if a call to pvPortMalloc() fails.  pvPortMalloc()
    is called automatically when a task, queue or semaphore is created. */
    PRINTF_E("\n\rMalloc fail, free/all:%d/%d\n\r", xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
    configASSERT(0);
    for (;;);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
    scpctl_idlehook();
}

void vApplicationTickHook(void)
{
    //printf("\n\r Enter TickHook \n\r");
}
