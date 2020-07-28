/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

 /* Includes -----------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>
#include "exception_handler.h"
#include <interrupt.h>
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#ifdef CFG_CACHE_SUPPORT
#include <hal_cache.h>
#endif
#ifdef CFG_BUS_MONITOR_SUPPORT
#include "bus_monitor.h"
#endif  /* CFG_BUS_MONITOR_SUPPORT */
#ifdef CFG_WDT_SUPPORT
#include <wdt.h>
#include <mt_reg_base.h>
#endif

/* Private define ------------------------------------------------------------*/
#define EXCEPTION_SIGNEL_CORE           1
#define EXCEPTION_STACK_WORDS           256
#define WINDOW_ENTRY_SIZE               16 // 16-entry AR in register file
#define MAX_LOG_SIZE                    512

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* exception handler's stack */
__attribute__ ((aligned (16))) static unsigned int xExceptionStack[EXCEPTION_STACK_WORDS] = {0};
unsigned int *pxExceptionStack = &xExceptionStack[EXCEPTION_STACK_WORDS - 4];

/* exception context area */
__attribute__ ((aligned (4))) static ExceptionContext exceptionContext;
ExceptionContext *pExceptionContext = &exceptionContext;
static char *log_assert = NULL;

/* Private functions ---------------------------------------------------------*/
void platform_assert(char *file, unsigned int line)
{
    PRINTF_E("[ASSERT]task:%s,file:%s,line:%u\r\n", pcTaskGetTaskName(NULL), file, line);
    if (log_assert)
        sprintf(log_assert, "[ASSERT]task:%s,file:%s,line:%u\n",
                pcTaskGetTaskName(NULL), file, line);
    __asm__ (
        "movi.n  a7,0\n\t"
        "quos    a6,a6,a7\n\t"
    );
}

char *get_name_exception(uint32_t exccause)
{
    switch (exccause) {
        case EXCCAUSE_ILLEGAL:
            return "EXCCAUSE_ILLEGAL";
        case EXCCAUSE_SYSCALL:
            return "EXCCAUSE_SYSCALL";
        case EXCCAUSE_DIVIDE_BY_ZERO:
            return "EXCCAUSE_DIVIDE_BY_ZERO";
        case EXCCAUSE_LOAD_PROHIBITED:
            return "EXCCAUSE_LOAD_PROHIBITED";
        case EXCCAUSE_STORE_PROHIBITED:
            return "EXCCAUSE_STORE_PROHIBITED";
        case EXCCAUSE_UNALIGNED:
            return "EXCCAUSE_UNALIGNED";
        default :
            return " ";
    }
}

void stackDump(void)
{
    int i = 0, index = 0;
    char *current_task_name = NULL;
    ExceptionContext *pEC = pExceptionContext;
    int wb = pEC ->windowbase;
    printf("==========================EXCEPTION==========================\n");
    printf("Stack Dump address: %p\n", pEC);

    if (pEC->pc == pEC->epc)
        printf("Exception happend, epc = 0x%x\n", pEC->epc);
    else
        printf("Double Exception happend, depc = 0x%x, epc = 0x%x\n", pEC->depc, pEC->epc);

    current_task_name = pcTaskGetTaskName(NULL);
    if (current_task_name)
        strncpy(pEC->task_name, current_task_name, DEFAULT_TASK_NAME_LEN);

    printf("Last Running Task: %s\n", pEC->task_name);
    printf("special register:\n");
    printf("\texccause=0x%x %s\n", pEC->exccause, get_name_exception(pEC->exccause));
    printf("\texcvaddr=0x%x\n", pEC->excvaddr);
    printf("\tpc=0x%x\n", pEC->pc);
    printf("\tps=0x%x\n", pEC->ps);
    printf("\twindowbase=0x%x\n", pEC->windowbase);
    printf("\twindowstart=0x%x\n", pEC->windowstart);
    printf("\tsar=0x%x\n", pEC->sar);
    printf("\tlbeg=0x%x\n", pEC->lbeg);
    printf("\tlend=0x%x\n", pEC->lend);
    printf("\tlcount=0x%x\n", pEC->lcount);

    printf("register a0-a15:\n");
    for (i = 0; i < WINDOW_ENTRY_SIZE; i = i + 4) {
        printf("\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n",
               pEC->ar[i+0], pEC->ar[i+1], pEC->ar[i+2], pEC->ar[i+3]);
    }
    printf("register file ar0-ar63:\n");
    for (i = 0; i < XCHAL_NUM_AREGS; i = i + 4) {
        index = (XCHAL_NUM_AREGS - wb * 4 + i) % XCHAL_NUM_AREGS;
        printf("\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n",
               pEC->ar[index+0], pEC->ar[index+1], pEC->ar[index+2], pEC->ar[index+3]);
    }
}

__attribute__((optimize ("-O0"))) void DSP_Fault_handler(void)
{
#ifdef CFG_BUS_MONITOR_SUPPORT
    if (is_bus_monitor_triggered()) {
        bus_monitor_2nd_stage_handler();
        goto ITCM_EXCEPTION_CODE;
    }
#endif  /* CFG_BUS_MONITOR_SUPPORT */
    stackDump();

#ifdef CFG_CACHE_SUPPORT
    hal_cache_all_operation(HAL_CACHE_TYPE_INSTRUCTION,
                            HAL_CACHE_OPERATION_INVALIDATE);
    hal_cache_all_operation(HAL_CACHE_TYPE_DATA,
                            HAL_CACHE_OPERATION_FLUSH_INVALIDATE);
#endif
#ifdef CFG_BUS_MONITOR_SUPPORT
ITCM_EXCEPTION_CODE:
#endif  /* CFG_BUS_MONITOR_SUPPORT */
#ifdef CFG_VCORE_DVFS_SUPPORT
    adsp_update_sys_status(ADSP_STATUS_RESET);
#endif
#ifdef CFG_WDT_SUPPORT
    /* wake up APMCU */
    ADSP_SPM_WAKEUPSRC = 1;
#ifdef TCM_LAYOUT_VERSION
    mask_irq(HIFI3_WDT_IRQn);
#endif
    mtk_wdt_set_time_out_value(10);/*assign a small value to make ee sooner*/
    mtk_wdt_restart();
#endif

    /*put hifi3 in memw & idle*/
    while (1) {
        __asm(
            "memw;\n"
            "WAITI 15;\n"
        );
    }
}

void set_exception_context_pointer(void* addr, unsigned int size)
{
   if (size < sizeof(ExceptionContext) + MAX_LOG_SIZE) {
       printf("set Stack Dump fail: size(%d) < sizeof(ExceptionContext) + log_assert(%d byte)\n",
              size, MAX_LOG_SIZE);
       return;
   }
   pExceptionContext = (ExceptionContext *)addr;
   log_assert = (void *)addr + size - MAX_LOG_SIZE;

   memset(log_assert, '\0', MAX_LOG_SIZE);
   printf("set Stack Dump: exceptionContext %p, log_assert %p\n", pExceptionContext, log_assert);
}

