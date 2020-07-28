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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "platform_mtk.h"
#include "stdarg.h"
#include "string.h"
#include <scp_ipi.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>        /* for mdelay() */
#endif
#ifdef CFG_WDT_SUPPORT
#include <wdt.h>           /* for mtk_wdt_isr() */
#endif
#ifdef CFG_CACHE_SUPPORT
#include <cache_dram_management.h>
#endif

enum { r0, r1, r2, r3, r12, lr, pc, psr};

typedef struct TaskContextType {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int sp;              /* after pop r0-r3, lr, pc, xpsr                   */
    unsigned int lr;              /* lr before exception                             */
    unsigned int pc;              /* pc before exception                             */
    unsigned int psr;             /* xpsr before exeption                            */
    unsigned int control;         /* nPRIV bit & FPCA bit meaningful, SPSEL bit = 0  */
    unsigned int exc_return;      /* current lr                                      */
    unsigned int msp;             /* msp                                             */
} TaskContext;

#define CRASH_SUMMARY_LENGTH 12
#define CRASH_MEMORY_LENGTH  (CFG_SCP_SRAM_SIZE / 4)
#define EIGHT_BYTE_ALIGN_IND (1 << 9)

typedef struct MemoryDumpType {
    char crash_summary[CRASH_SUMMARY_LENGTH];
    TaskContext core;
    int flash_length;
    int sram_length;
    int memory[CRASH_MEMORY_LENGTH];
} MemoryDump;

typedef struct {
    int task_context_addr;
    int dump_start;
    int length;
} RAM_DUMP_INFO;

#define FULL_MEMORY_DUMP 0
#define MINI_MEMORY_DUMP 0
#define EXCEPTION_HALT 1

static TaskContext taskCtxDump __attribute__((section(".share")));
TaskContext *pTaskContext = &taskCtxDump;

#ifdef CFG_RAMDUMP_SUPPORT
#if (FULL_MEMORY_DUMP)
static MemoryDump *pMemoryDump;
#endif

extern unsigned long __RTOS_segment_start__[];
extern unsigned long __RTOS_segment_end__[];

void mt_ram_dump_init(void)
{
    void* task_context_dump_addr = &taskCtxDump;
    struct scp_region_info_st* region_info_p = INFO_BASE_POINTER;
    memset(&taskCtxDump, 0, sizeof(taskCtxDump));
    region_info_p->TaskContext_ptr = (unsigned int)&taskCtxDump;
    PRINTF_E("ram dump: %p\n\r", task_context_dump_addr);
}
#endif

void triggerException(void)
{
    SCB->CCR |= 0x10; /* DIV_0_TRP */
    __ASM volatile("mov r0, #0");
    __ASM volatile("UDIV r0, r0");
    for (;;);
}

#if FULL_MEMORY_DUMP
static void memoryDumpAll(void)
{
    uint32_t current, end;
    uint32_t offset;
    current = (uint32_t)__RTOS_segment_start__;
    end     = (uint32_t)__RTOS_segment_end__;
    pMemoryDump->flash_length = end - current;

    PRINTF_E("flash len:0x%08x\n\r", pMemoryDump->flash_length);
    PRINTF_E("mem:0x%08x\n\r", pMemoryDump->memory);
    for (offset = 0; current < end; current += 16, offset += 4) {
        if ((*(uint32_t*) current) == 0xFFFFFFFF &&
                (*(uint32_t*)(current + 4)) == 0xFFFFFFFF &&
                (*(uint32_t*)(current + 8)) == 0xFFFFFFFF &&
                (*(uint32_t*)(current + 12)) == 0xFFFFFFFF)
            continue;

        if ((*(uint32_t*) current) == 0 &&
                (*(uint32_t*)(current + 4)) == 0 &&
                (*(uint32_t*)(current + 8)) == 0 &&
                (*(uint32_t*)(current + 12)) == 0)
            continue;

        pMemoryDump->memory[offset]    = *((unsigned int*)current);
        pMemoryDump->memory[offset + 1]  = *((unsigned int*)current + 1);
        pMemoryDump->memory[offset + 2]  = *((unsigned int*)current + 2);
        pMemoryDump->memory[offset + 3]  = *((unsigned int*)current + 3);
        PRINTF_E("0x%08x: %08x %08x %08x %08x\n\r", (unsigned int)current,
                 *((unsigned int*)current), *((unsigned int*)(current + 4)),
                 *((unsigned int*)(current + 8)), *((unsigned int*)(current + 12)));
    }
}
#endif

#if MINI_MEMORY_DUMP

#define NEAR_OFFSET 0x100

static uint32_t isValidMemory(uint32_t addr)
{
    addr = addr - addr % 4;

    if ((addr < (uint32_t)__RTOS_segment_start__) || (addr > (uint32_t)__RTOS_segment_end__))
        return 0;
    return 1;
}

void nearDump(uint32_t addr)
{
    uint32_t current, end;

    addr = addr - addr % 16;
    current = addr - NEAR_OFFSET / 2;
    end = addr + NEAR_OFFSET / 2;

    if (current < (uint32_t)__RTOS_segment_start__)
        current = (uint32_t)__RTOS_segment_start__;

    if (end > (uint32_t)__RTOS_segment_end__)
        end = (uint32_t)__RTOS_segment_end__;

    for (; current < end; current += 16) {
        PRINTF_E("0x%08x: %08x %08x %08x %08x\n\r", (unsigned int)current,
                 *((unsigned int*)current), *((unsigned int*)(current + 4)),
                 *((unsigned int*)(current + 8)), *((unsigned int*)(current + 12)));
    }
}

static void memoryDump(TaskContext *pTaskContext)
{
    const char *prompt = "Mem dump";

    if (isValidMemory(pTaskContext->r0)) {
        PRINTF_E("\n\r%s r0:\n\r", prompt);
        nearDump(pTaskContext->r0);
    }
    if (isValidMemory(pTaskContext->r1)) {
        PRINTF_E("\n\r%s r1:\n\r", prompt);
        nearDump(pTaskContext->r1);
    }
    if (isValidMemory(pTaskContext->r2)) {
        PRINTF_E("\n\r%s r2:\n\r", prompt);
        nearDump(pTaskContext->r2);
    }
    if (isValidMemory(pTaskContext->r3)) {
        PRINTF_E("\n\r%s r3:\n\r", prompt);
        nearDump(pTaskContext->r3);
    }
    if (isValidMemory(pTaskContext->r4)) {
        PRINTF_E("\n\r%s r4:\n\r", prompt);
        nearDump(pTaskContext->r4);
    }
    if (isValidMemory(pTaskContext->r5)) {
        PRINTF_E("\n\r%s r5:\n\r", prompt);
        nearDump(pTaskContext->r5);
    }
    if (isValidMemory(pTaskContext->r6)) {
        PRINTF_E("\n\r%s r6:\n\r", prompt);
        nearDump(pTaskContext->r6);
    }
    if (isValidMemory(pTaskContext->r7)) {
        PRINTF_E("\n\r%s r7:\n\r", prompt);
        nearDump(pTaskContext->r7);
    }
    if (isValidMemory(pTaskContext->r8)) {
        PRINTF_E("\n\r%s r8:\n\r", prompt);
        nearDump(pTaskContext->r8);
    }
    if (isValidMemory(pTaskContext->r9)) {
        PRINTF_E("\n\r%s r9:\n\r", prompt);
        nearDump(pTaskContext->r9);
    }
    if (isValidMemory(pTaskContext->r10)) {
        PRINTF_E("\n\r%s r10:\n\r", prompt);
        nearDump(pTaskContext->r10);
    }
    if (isValidMemory(pTaskContext->r11)) {
        PRINTF_E("\n\r%s r11:\n\r", prompt);
        nearDump(pTaskContext->r11);
    }
    if (isValidMemory(pTaskContext->r12)) {
        PRINTF_E("\n\r%s r12:\n\r", prompt);
        nearDump(pTaskContext->r12);
    }
    if (isValidMemory(pTaskContext->lr)) {
        PRINTF_E("\n\r%s lr:\n\r", prompt);
        nearDump(pTaskContext->lr);
    }
    if (isValidMemory(pTaskContext->pc)) {
        PRINTF_E("\n\r%s pc:\n\r", prompt);
        nearDump(pTaskContext->pc);
    }
    if (isValidMemory(pTaskContext->sp)) {
        PRINTF_E("\n\r%s sp:\n\r", prompt);
        nearDump(pTaskContext->sp);
        nearDump(pTaskContext->sp + NEAR_OFFSET);
    }
}
#endif

static void stackDump(uint32_t stack[])
{
#ifdef CFG_RAMDUMP_SUPPORT
    taskCtxDump.r0   = stack[r0];
    taskCtxDump.r1   = stack[r1];
    taskCtxDump.r2   = stack[r2];
    taskCtxDump.r3   = stack[r3];
    taskCtxDump.r12  = stack[r12];
    taskCtxDump.lr   = stack[lr];
    taskCtxDump.pc   = stack[pc];
    taskCtxDump.psr  = stack[psr];
    taskCtxDump.sp   = ((uint32_t)stack) + 0x20;
    if (taskCtxDump.psr & EIGHT_BYTE_ALIGN_IND)
        taskCtxDump.sp += 4;

    /* update CONTROL.SPSEL if returning to thread mode */
    if (taskCtxDump.exc_return & 0x4)
        taskCtxDump.control |= 0x2;
    else /* update msp if returning to handler mode */
        taskCtxDump.msp = taskCtxDump.sp;

    PRINTF_E("Core dump\n\r");
    PRINTF_E("r0 =0x%08x\n\r", taskCtxDump.r0);
    PRINTF_E("r1 =0x%08x\n\r", taskCtxDump.r1);
    PRINTF_E("r2 =0x%08x\n\r", taskCtxDump.r2);
    PRINTF_E("r3 =0x%08x\n\r", taskCtxDump.r3);
    PRINTF_E("r4 =0x%08x\n\r", taskCtxDump.r4);
    PRINTF_E("r5 =0x%08x\n\r", taskCtxDump.r5);
    PRINTF_E("r6 =0x%08x\n\r", taskCtxDump.r6);
    PRINTF_E("r7 =0x%08x\n\r", taskCtxDump.r7);
    PRINTF_E("r8 =0x%08x\n\r", taskCtxDump.r8);
    PRINTF_E("r9 =0x%08x\n\r", taskCtxDump.r9);
    PRINTF_E("r10=0x%08x\n\r", taskCtxDump.r10);
    PRINTF_E("r11=0x%08x\n\r", taskCtxDump.r11);
    PRINTF_E("r12=0x%08x\n\r", taskCtxDump.r12);
    PRINTF_E("lr =0x%08x\n\r", taskCtxDump.lr);
    PRINTF_E("pc =0x%08x\n\r", taskCtxDump.pc);
    PRINTF_E("psr=0x%08x\n\r", taskCtxDump.psr);
    PRINTF_E("EXC_RET=0x%08x\n\r", taskCtxDump.exc_return);
    PRINTF_E("CONTROL=0x%08x\n\r", taskCtxDump.control);
    PRINTF_E("MSP    =0x%08x\n\r", taskCtxDump.msp);
    PRINTF_E("sp     =0x%08x\n\r", taskCtxDump.sp);
#endif
}


/*****************************************************************************/
/*****************************************************************************/


void dump_dram(uint32_t addr, uint32_t len, char *str)
{
    uint32_t start = addr;
    uint32_t current = addr;
    uint32_t end = addr + len;  // size: 0x40000 = 256 KB
    int32_t count = 0;

    PRINTF_E("start of %s\n", str);
    for (current = start; current <= end - 16; current += 16, count += 16) {
        PRINTF_E("0x%08x: %08x %08x %08x %08x\n", (unsigned int)current,
                 *((unsigned int*)current), *((unsigned int*)(current + 4)),
                 *((unsigned int*)(current + 8)), *((unsigned int*)(current + 12)));
        if (count >= 256) {
#ifdef CFG_XGPT_SUPPORT
            mdelay(10);
#endif
            count = 0;
        }
    }
    PRINTF_E("End of %s\n", str);
}


void crash_dump(uint32_t stack[])
{
    stackDump(stack);

#if MINI_MEMORY_DUMP
    memoryDump(&taskCtxDump);
#endif

    /*
     * Backup for later merging.
     *
     * dump_dram(0, 0x40000, "SRAM");
     * dump_dram(0x80000, 0x40000, "DRAM");
     */



#if FULL_MEMORY_DUMP
    memoryDumpAll();
#endif

#if EXCEPTION_HALT
#if DEBUGGER_ON
    __ASM volatile("BKPT #01");
#else
#ifdef CFG_WDT_SUPPORT
    mtk_wdt_set_time_out_value(10); /*assign a small value to make ee sooner*/
    mtk_wdt_restart();
#endif /* CFG_WDT_SUPPORT */
    while (1);
#endif /* DEBUGGER_ON */
#else
    SCB->CFSR = SCB->CFSR;
#endif

}

void printUsageErrorMsg(uint32_t CFSRValue)
{
    PRINTF_E("Usage fault:");
    CFSRValue >>= 16;                  // right shift to lsb
    if ((CFSRValue & (1 << 9)) != 0) {
        PRINTF_E("Div by zero\n\r");
    }
    if ((CFSRValue & (1 << 8)) != 0) {
        PRINTF_E("Unalign access\n\r");
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        PRINTF_E("Undefined ins.\n\r");
    }
}

void printBusFaultErrorMsg(uint32_t CFSRValue)
{
    PRINTF_E("Bus fault: ");
    CFSRValue &= 0x0000FF00; // mask just bus faults
    CFSRValue >>= 8;
    if ((CFSRValue & (1 << 5)) != 0) {
        PRINTF_E("FP lazy state preservation\n\r");
    }
    if ((CFSRValue & (1 << 4)) != 0) {
        PRINTF_E("Excep. entry\n\r");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        PRINTF_E("Excep. return\n\r");
    }
    if ((CFSRValue & (1 << 2)) != 0) {
        PRINTF_E("Imprecise data access err\n\r");
    }
    if ((CFSRValue & (1 << 1)) != 0) { /* Need to check valid bit (bit 7 of CFSR)? */
        PRINTF_E("precise data access err0x%08x\n\r", (unsigned int)SCB->BFAR);
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        PRINTF_E("ins. prefetch has occurred\n\r");
    }
    if ((CFSRValue & (1 << 7)) != 0) { /* To review: remove this if redundant */
        PRINTF_E("SCB->BFAR=0x%08x\n\r", (unsigned int)SCB->BFAR);
    }
}

void printMemoryManagementErrorMsg(uint32_t CFSRValue)
{
    PRINTF_E("Mem Manag. fault:");
    CFSRValue &= 0x000000FF; // mask just mem faults
    if ((CFSRValue & (1 << 5)) != 0) {
        PRINTF_E("FP lazy state preservation\n\r");
    }
    if ((CFSRValue & (1 << 4)) != 0) {
        PRINTF_E("Excep. entry\n\r");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        PRINTF_E("Excep. return\n\r");
    }
    if ((CFSRValue & (1 << 1)) != 0) { /* Need to check valid bit (bit 7 of CFSR)? */
        PRINTF_E("Data access violation @0x%08x\n\r", (unsigned int)SCB->MMFAR);
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        PRINTF_E("MPU or Execute Never default mem map access violation\n\r");
    }
    if ((CFSRValue & (1 << 7)) != 0) { /* To review: remove this if redundant */
        PRINTF_E("SCB->MMFAR=0x%08x\n\r", (unsigned int)SCB->MMFAR);
    }
}

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

void Hard_Fault_Handler(uint32_t stack[])
{
    PRINTF_E("\n\rHard Fault\n\r");
    PRINTF_E("SCB->HFSR=0x%08x\n\r", (unsigned int)SCB->HFSR);
    PRINTF_E("Current xPSR=0x%08x, Previous xPSR=0x%08x\n\r",
             (unsigned int)__get_xPSR(), (unsigned int)stack[psr]);

    if ((SCB->HFSR & (1 << 30)) != 0) {
        PRINTF_E("Forced Hard Fault\n\r");
        PRINTF_E("SCB->CFSR=0x%08x\n\r", (unsigned int)SCB->CFSR);
        if ((SCB->CFSR & 0xFFFF0000) != 0) {
            printUsageErrorMsg(SCB->CFSR);
        }
        if ((SCB->CFSR & 0x0000FF00) != 0) {
            printBusFaultErrorMsg(SCB->CFSR);
        }
        if ((SCB->CFSR & 0x000000FF) != 0) {
            printMemoryManagementErrorMsg(SCB->CFSR);
        }
    }

    crash_dump(stack);
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    __asm volatile
    (
        "ldr r0, =pTaskContext         \n"
        "ldr r0, [r0]                  \n"
        "add r0, r0, #16               \n"     /* point to context.r4          */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                 */
        "add r0, #20                   \n"     /* point to context.control     */
        "mrs r1, control               \n"     /* move CONTROL to r1           */
        "str r1, [r0], #4              \n"     /* store CONTROL                */
        "str lr, [r0], #4              \n"     /* store EXC_RETURN             */
        "mrs r1, msp                   \n"     /* move MSP to r1               */
        "str r1, [r0]                  \n"     /* store MSP                    */
        "tst lr, #4                    \n"     /* thread or handler mode?      */
        "ite eq                        \n"
        "mrseq r0, msp                 \n"
        "mrsne r0, psp                 \n"
        "push {lr}                     \n"
        "bl Hard_Fault_Handler         \n"
        "pop {lr}                      \n"
        "bx lr                         \n"
    );
}

void Bus_Fault_Handler(uint32_t stack[])
{
    PRINTF_E("\n\rBus Fault\n\r");
    PRINTF_E("SCB->CFSR = 0x%08x\n\r", (unsigned int)SCB->CFSR);
    if ((SCB->CFSR & 0xFF00) != 0) {
        printBusFaultErrorMsg(SCB->CFSR);
    }

    crash_dump(stack);
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    __asm volatile
    (
        "ldr r0, =pTaskContext         \n"
        "ldr r0, [r0]                  \n"
        "add r0, r0, #16               \n"     /* point to context.r4          */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                 */
        "add r0, #20                   \n"     /* point to context.control     */
        "mrs r1, control               \n"     /* move CONTROL to r1           */
        "str r1, [r0], #4              \n"     /* store CONTROL                */
        "str lr, [r0], #4              \n"     /* store EXC_RETURN             */
        "mrs r1, msp                   \n"     /* move MSP to r1               */
        "str r1, [r0]                  \n"     /* store MSP                    */
        "tst lr, #4                    \n"     /* thread or handler mode?      */
        "ite eq                        \n"
        "mrseq r0, msp                 \n"
        "mrsne r0, psp                 \n"
        "push {lr}                     \n"
        "bl Bus_Fault_Handler          \n"
        "pop {lr}                      \n"
        "bx lr                         \n"
    );
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Fault_Handler(uint32_t stack[])
{

#ifdef CFG_CACHE_SUPPORT
    if (xTrySwitchOnDramPower(stack) == pdPASS) {
        /* clear the w1c status bits */
        SCB->CFSR = SCB->CFSR & 0xFF;
        return;
    }
#endif

    PRINTF_E("\n\rMemManage Fault\n\r");
    PRINTF_E("SCB->CFSR=0x%08x\n\r", (unsigned int)SCB->CFSR);

    if ((SCB->CFSR & 0xFF) != 0) {
        printMemoryManagementErrorMsg(SCB->CFSR);
    }

    crash_dump(stack);
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    __asm volatile
    (
        "ldr r0, =pTaskContext         \n"
        "ldr r0, [r0]                  \n"
        "add r0, r0, #16               \n"     /* point to context.r4          */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                 */
        "add r0, #20                   \n"     /* point to context.control     */
        "mrs r1, control               \n"     /* move CONTROL to r1           */
        "str r1, [r0], #4              \n"     /* store CONTROL                */
        "str lr, [r0], #4              \n"     /* store EXC_RETURN             */
        "mrs r1, msp                   \n"     /* move MSP to r1               */
        "str r1, [r0]                  \n"     /* store MSP                    */
        "tst lr, #4                    \n"     /* thread or handler mode?      */
        "ite eq                        \n"
        "mrseq r0, msp                 \n"
        "mrsne r0, psp                 \n"
        "push {lr}                     \n"
        "bl MemManage_Fault_Handler    \n"
        "pop {lr}                      \n"
        "bx lr                         \n"
    );
}
#if 0 /* not enable */
/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}
#endif

#ifdef CFG_DWT_SUPPORT
void Debug_Mon_Handler(uint32_t stack[])
{
    /* PRINTF_E("\n\rIn Debug Mon Fault Handler\n\r"); */
    PRINTF_E("SCB->CFSR = 0x%08x\n\r", (unsigned int)SCB->CFSR);

    if (SCB->DFSR & SCB_DFSR_DWTTRAP_Msk) {
        PRINTF_E("DWT debug event\n\r");
        /*
         PRINTF_D("COMP0: %8lx \t MASK0: %8lx \t FUNC0: %8lx \n\r", DWT->COMP0,DWT->MASK0,DWT->FUNCTION0);
         PRINTF_D("COMP1: %8lx \t MASK1: %8lx \t FUNC1: %8lx \n\r", DWT->COMP1,DWT->MASK1,DWT->FUNCTION1);
         PRINTF_D("COMP2: %8lx \t MASK2: %8lx \t FUNC2: %8lx \n\r", DWT->COMP2,DWT->MASK2,DWT->FUNCTION2);
         PRINTF_D("COMP3: %8lx \t MASK3: %8lx \t FUNC3: %8lx \n\r", DWT->COMP3,DWT->MASK3,DWT->FUNCTION3);
        */
    }

    crash_dump(stack);

    //PRINTF_D("LR:0x%x\n",drv_reg32(REG_LR));
    //PRINTF_D("PC:0x%x\n",drv_reg32(REG_PC));
    //PRINTF_D("PSP:0x%x\n",drv_reg32(REG_PSP));
    //PRINTF_D("SP:0x%x\n",drv_reg32(REG_SP));
}
/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
//TODO: implement here !!!
//    PRINTF_E("\n\rEnter Debug Monitor Handler\n\r");
//    PRINTF_E("SCB->DFSR = 0x%08x\n\r", (unsigned int)SCB->DFSR );
    __asm volatile
    (
        "ldr r0, =pTaskContext         \n"
        "ldr r0, [r0]                  \n"
        "add r0, r0, #16               \n"     /* point to context.r4          */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                 */
        "add r0, #20                   \n"     /* point to context.control     */
        "mrs r1, control               \n"     /* move CONTROL to r1           */
        "str r1, [r0], #4              \n"     /* store CONTROL                */
        "str lr, [r0], #4              \n"     /* store EXC_RETURN             */
        "mrs r1, msp                   \n"     /* move MSP to r1               */
        "str r1, [r0]                  \n"     /* store MSP                    */
        "tst lr, #4                    \n"     /* thread or handler mode?      */
        "ite eq                        \n"
        "mrseq r0, msp                 \n"
        "mrsne r0, psp                 \n"
        "push {lr}                     \n"
        "bl Debug_Mon_Handler         \n"
        "pop {lr}                      \n"
        "bx lr                         \n"
    );
}
#endif

void AP_Wdt_Irq_Handler(uint32_t stack[])
{
    struct scp_region_info_st* region_info_p = INFO_BASE_POINTER;
    PRINTF_E("\n\rAP_Wdt_Irq_Handler\n\r");
    stackDump(stack);
    /* set done flag 0x80000000 */
    region_info_p->TaskContext_ptr |= 0x80000000;
    while (1) {
        __asm volatile("dsb");
        __asm volatile("wfi");
    }
    return;
}

void APWdtIrq_Handler(void)
{
    __asm volatile
    (
        "ldr r0, =pTaskContext         \n"
        "ldr r0, [r0]                  \n"
        "add r0, r0, #16               \n"     /* point to context.r4          */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                 */
        "add r0, #20                   \n"     /* point to context.control     */
        "mrs r1, control               \n"     /* move CONTROL to r1           */
        "str r1, [r0], #4              \n"     /* store CONTROL                */
        "str lr, [r0], #4              \n"     /* store EXC_RETURN             */
        "mrs r1, msp                   \n"     /* move MSP to r1               */
        "str r1, [r0]                  \n"     /* store MSP                    */
        "tst lr, #4                    \n"     /* thread or handler mode?      */
        "ite eq                        \n"
        "mrseq r0, msp                 \n"
        "mrsne r0, psp                 \n"
        "push {lr}                     \n"
        "bl AP_Wdt_Irq_Handler         \n"
        "pop {lr}                      \n"
        "bx lr                         \n"
    );
}

