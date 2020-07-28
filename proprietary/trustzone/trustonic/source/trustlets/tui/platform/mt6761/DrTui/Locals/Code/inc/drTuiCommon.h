/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/** @addtogroup DRIVER_TUI
 * @{
 * @file drTuiCommon.h
 * @brief Common definitions and macros.
 */
#ifndef __DR_TUI_COMMON_H__
#define __DR_TUI_COMMON_H__

/* Priority definitions */
#define EXCH_PRIORITY           MAX_PRIORITY
#define SAFEKEEPERH_PRIORITY    (MAX_PRIORITY-1)
#define BLITTERH_PRIORITY       (MAX_PRIORITY-2)
#define IPCH_PRIORITY           (MAX_PRIORITY-3)
#define DCIH_PRIORITY           (MAX_PRIORITY-3)
#define COREH_PRIORITY          (MAX_PRIORITY-3)
#define IRQH_PRIORITY           (MAX_PRIORITY-3)

/* Kernel exceptions */
#define TRAP_UNKNOWN        ( 0)    /**< message was sent due to an unknown exception. */
#define TRAP_SYSCALL        ( 1)    /**< message was sent due to an invalid syscall number. */
#define TRAP_SEGMENTATION   ( 2)    /**< message was sent due to an illegal memory access. */
#define TRAP_ALIGNMENT      ( 3)    /**< message was sent due to a misaligned memory access. */
#define TRAP_UNDEF_INSTR    ( 4)    /**< message was sent due to an undefined instruction. */
#define TRAP_BREAKPOINT     ( 5)    /**< message was sent due to a breakpoint exception. */
#define TRAP_ARITHMETIC     ( 6)    /**< message was sent due to an arithmetic exception. */
#define TRAP_INSTR_FETCH    ( 7)    /**< message was sent due to instruction fetch failure. */
#define TRAP_INTERRUPT      (16)    /**< message was sent due to an interrupt. */
#define TRAP_TIMEFAULT      (17)    /**< message was sent due to a timefault exception. */
#define TRAP_TASK_TERMINATE (18)    /**< message was sent due to a child task termination. */
#define TRAP_TASK_ACTIVATE  (19)    /**< message was sent due to a task activation */
#define TRAP_TASK_START     (20)    /**< message was sent due to a task start */

/* Thread numbers */
#define DRIVER_THREAD_NO_EXCH      1
#define DRIVER_THREAD_NO_IPCH      2
#define DRIVER_THREAD_NO_COREH     3
#define DRIVER_THREAD_NO_SAFEKEEPERH    5
#define DRIVER_THREAD_NO_BLITTERH       6

#define THREADID_THREADNO_SHIFT     16
#define THREADID_TASKID_MASK        ((1 << THREADID_THREADNO_SHIFT) - 1)
#define THREADID_THREADNO_MASK      (~THREADID_TASKID_MASK)
#define GET_THREADNO(threadid)      (threadid >> THREADID_THREADNO_SHIFT)

#define PTR_OFFS(ptr,offset)   ( (addr_t)( (word_t)(ptr) + (offset) ) )
#define PTR_AND(ptr,mask)   ( (addr_t)( (word_t)(ptr) & (mask) ) )
#define PTR_MASK(p,m)       PTR_AND(p,m)

typedef uint64_t phys_addr_t;    /**< physical address datatype. */


#define PHYS_INVALID_ADDRESS ((phys_addr_t) -1)
#define PHYS_IS_ERROR(x) (PHYS_INVALID_ADDRESS <= ((phys_addr_t)x))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define IS_WORD_ALIGNED(ptr) (((uintptr_t)ptr % 4) == 0)

#define PANIC() (drApiExit(1))

//==============================================================================
// The helper function to allocate protected stack dynamically.
//==============================================================================
#if defined(DEGUG)
    void tuiAllocStack_debug (
        char const * const tag,
        uint32_t  const stackSize,
        uint8_t **const stackTop
        );
    #define tuiAllocStack(tag, stackSize, stackTop)\
            tuiAllocStack_debug(tag,stackSize, stackTop)
#else
    void tuiAllocStack_release (
        uint32_t  const stackSize,
        uint8_t **const stackTop
        );
    #define tuiAllocStack(tag, stackSize, stackTop)\
            tuiAllocStack_release(stackSize, stackTop)
#endif

#define PANIC() (drApiExit(1))

#endif // __DR_TUI_COMMON_H__

/** @} */
