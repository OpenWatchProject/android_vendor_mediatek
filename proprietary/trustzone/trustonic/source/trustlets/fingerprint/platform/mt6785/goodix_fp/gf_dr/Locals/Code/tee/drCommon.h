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

#ifndef __DR_COMMON_H__
#define __DR_COMMON_H__

#include "DrApi/DrApi.h"
#include "TlApi/TlApiError.h"

/* Priority definitions */
#define EXCH_PRIORITY       MAX_PRIORITY
#define IPCH_PRIORITY       (MAX_PRIORITY-1)

/* Driver version (used by the IPC handler) */
#define DRIVER_VERSION       1

/* Thread numbers */
#define DRIVER_THREAD_NO_EXCH      1
#define DRIVER_THREAD_NO_IPCH      2

/* Kernel exceptions */
#define TRAP_UNKNOWN        ( 0)    /* unknown exception. */
#define TRAP_SYSCALL        ( 1)    /* invalid syscall number. */
#define TRAP_SEGMENTATION   ( 2)    /* illegal memory access. */
#define TRAP_ALIGNMENT      ( 3)    /* misaligned memory access. */
#define TRAP_UNDEF_INSTR    ( 4)    /* undefined instruction. */

#define THREADID_THREADNO_SHIFT     16
#define THREADID_TASKID_MASK        ((1 << THREADID_THREADNO_SHIFT) - 1)
#define THREADID_THREADNO_MASK      (~THREADID_TASKID_MASK)
#define GET_THREADNO(threadid)      (threadid >> THREADID_THREADNO_SHIFT)

/* Macro and relevat definitions for stack */
#define STACK_DBG_INIT_VALUE        0xCC
#define fillStack(_name_,_val_) \
  memset(_STACK_ARRAY_VAR(_name_), _val_, getStackSize(_name_))
#define clearStack(_name_)      fillStack(_name_,STACK_DBG_INIT_VALUE)
#define TASKID_GET_THREADID(taskid,threadno) ((threadid_t)((taskid) | ((threadno) << THREADID_THREADNO_SHIFT)))
#define FUNC_PTR(func)              VAL2PTR( PTR2VAL( func ) )
#define getStackTop(_name_)         GET_STACK_TOP(_name_)

#endif // __DR_COMMON_H__
