/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
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
 * @file drCommon.h
 * @brief Basic TUI driver functions error codes.
 */
#ifndef __TLAPITUIERROR_H__
#define __TLAPITUIERROR_H__

/* Priority definitions */
#define EXCH_PRIORITY           MAX_PRIORITY
#define IPCH_PRIORITY           (MAX_PRIORITY-1)
#define DCIH_PRIORITY           (MAX_PRIORITY-1)
#define COREH_PRIORITY          (MAX_PRIORITY-1)
#define IRQH_PRIORITY           (MAX_PRIORITY-1)

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


/* RTM specific definitions */
#define RTM_TASK             1
#define RTM_THREADNO_IPCH    3


/* Thread numbers */
#define DRIVER_THREAD_NO_EXCH      1
#define DRIVER_THREAD_NO_IPCH      2
#define DRIVER_THREAD_NO_COREH     3

/* Register ids for THREAD_EX_REGS syscall */
#define THREAD_EX_REGS_IP       (1U << 0)   /**<  Instruction pointer */
#define THREAD_EX_REGS_SP       (1U << 1)   /**<  Stack pointer */


#define THREADID_THREADNO_SHIFT     16
#define THREADID_TASKID_MASK        ((1 << THREADID_THREADNO_SHIFT) - 1)
#define THREADID_THREADNO_MASK      (~THREADID_TASKID_MASK)
#define GET_THREADNO(threadid)      (threadid >> THREADID_THREADNO_SHIFT)


#define CPSR_REG                    0x10
#define MAX_MEM_LENGTH              4096
#define LIFETIME_SEED_SIZE          16
#define STACK_DBG_INIT_VALUE        0xCC
#define fillStack(_name_,_val_) \
  memset(_STACK_ARRAY_VAR(_name_), _val_, getStackSize(_name_))
#define clearStack(_name_)      fillStack(_name_,STACK_DBG_INIT_VALUE)
#define TASKID_GET_THREADID(taskid,threadno) ((threadid_t)((taskid) | ((threadno) << THREADID_THREADNO_SHIFT)))
#define FUNC_PTR(func)              VAL2PTR( PTR2VAL( func ) )
#define getStackTop(_name_)         GET_STACK_TOP(_name_)

#define HEAP_DBG_INIT_VALUE        0xDD
#define fillHeap(_name_,_val_) \
  memset(_HEAP_ARRAY_VAR(_name_), _val_, getHeapSize(_name_))
#define clearHeap(_name_)      fillHeap(_name_,HEAP_DBG_INIT_VALUE)

/* stolen from MTK/kernel.h */
#define COMPILE_TIME_ASSERT(e) { typedef int __ASSERT_DUMMY[ (e) ? 0 : -1]; }

//----------------------------------------------------------------------------
// IMPROVEMENT-2012-01-17-heidera (hollyr, galkag) with RVCT3.1 Build 761 the inline function are not equal to macros.
//       A function call is generated and no inlining occurs. So we should
//       consider switching to macros again (axh, 2009-06-02)
#define PTR_OFFS(ptr,offset)   ( (addr_t)( (word_t)(ptr) + (offset) ) )
//static __inline__ __pure addr_t PTR_OFFS(
//    addr_t ptr,
//    word_t offset
//) {
//    return (addr_t)((word_t)ptr + offset);
//}

#ifdef MTK_FEATURE_LPAE_SUPPORT
#define PHYS_PTR_OFFS(p,o)    ((p) + (o))
#define PHYS_ALIGN_UP(x, n)   (((x)+ ((n)-1)) & ~((n)-1))
#define PHYS_ALIGN_DOWN(x, n) ((x) & ~((n)-1))
#else
#define PHYS_PTR_OFFS(p,o)    PTR_OFFS(p,o)
#define PHYS_ALIGN_UP(x, n)   _alignUp(x, n)
#define PHYS_ALIGN_DOWN(x, n) _alignDown(x, n)
#endif
//----------------------------------------------------------------------------
#define PTR_AND(ptr,mask)   ( (addr_t)( (word_t)(ptr) & (mask) ) )
//static __inline__ __pure addr_t PTR_AND(
//    addr_t ptr,
//    word_t mask
//) {
//    return (addr_t)((word_t)ptr & mask);
//}

#define PTR_MASK(p,m)       PTR_AND(p,m)

//----------------------------------------------------------------------------
#define PTR_OR(ptr,mask)   ( (addr_t)( (word_t)(ptr) | (mask) ) )
//static __inline__ __pure addr_t PTR_OR(
//    addr_t ptr,
//    word_t mask
//) {
//    return (addr_t)((word_t)ptr | ask);
//}

typedef uint64_t phys_addr_t;    /**< physical address datatype. */


#define PHYS_INVALID_ADDRESS ((phys_addr_t) -1)
#define PHYS_IS_ERROR(x) (PHYS_INVALID_ADDRESS <= ((phys_addr_t)x))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // __TLAPITUIERROR_H__

/** @} */
