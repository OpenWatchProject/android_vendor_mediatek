/*
 * Copyright (c) 2015-2018 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#ifndef MC_DEFINES_H_
#define MC_DEFINES_H_

////////////////////////////////////////////////////////////////////////////////
// Below this line the file needs to be cleaned up
// In fact those types and functions are the duplication of mobicore definitions
// They need to be imported from mobicore.
// Some headers reorganisation has to be done in order to avoid arm dependencies
// eg : get only the MACRO LOG_SOURCE_TASK_SHIFT of syscall-api/threadid.h and
// not all the dependencies which are comming with it and cause the compilation
// failure
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdint.h>
#include "mcLoadFormat.h" //SERVICE_TYPE_LAST_ENTRY
#include "mcifc.h"

#define LOG_SOURCE_TASK_SHIFT       8
#define THREADID_TASKID_MASK        ((1u << THREADID_THREADNO_SHIFT) - 1)
// Locals/Code/MTK/public/syscall-api/threadid.h:#define LOG_SOURCE_TASK_SHIFT      8

typedef enum {
    SESSION_STATE_UNUSED          = 0, /**< Session object is unused */
    SESSION_STATE_INITIALIZING    = 1, /**< Session is going up  */
    SESSION_STATE_ACTIVE          = 2, /**< Session is up and running (open) */
    SESSION_STATE_CLOSING         = 3, /**< Session closing has been started by MSH */
    SESSION_STATE_CLOSED          = 4, /**< Session closing has been finished by EXCH */
    SESSION_STATE_ENDPOINT_DIED   = 5, /**< Task of a session died, state set by EXCH */
} sessionState_t;
// Locals/Code/MC/RTM/SMGNT/public/smgnt.h:} sessionState_t;

typedef enum {
    TRAP_UNKNOWN        =  0,
    TRAP_SYSCALL        =  1,
    TRAP_SEGMENTATION   =  2,
    TRAP_ALIGNMENT      =  3,
    TRAP_UNDEF_INSTR    =  4,
    TRAP_BREAKPOINT     =  5,
    TRAP_ARITHMETIC     =  6,
    TRAP_INSTR_FETCH    =  7,
    TRAP_INTERRUPT      = 16,
    TRAP_TIMEFAULT      = 17,
    TRAP_TASK_TERMINATE = 18,
    TRAP_TASK_ACTIVATE  = 19,
    TRAP_TASK_START     = 20,
    TRAP_SUSPEND        = 21,
    TRAP_RESUME         = 22,
} trapTypes_t;
// Locals/Code/MTK/public/syscall-api/user.h:#define TRAP_UNKNOWN ( 0)                                      /!\ as define

static __inline__ uint16_t threadid_get_dbgId(uint32_t threadId) {
    return (uint16_t)(
                    ((uint8_t)(threadId & 0xFFFF)<<LOG_SOURCE_TASK_SHIFT) |
                    (uint8_t)(threadId>>16));
}

static __inline__ uint32_t threadid_get_taskid(uint32_t threadid) {
    return (threadid & 0x0000FFFF);
}

static inline const char *fcTypeToString(uint32_t fc){
    switch( fc ) {
        case MC_FC_INIT:                return "FC init"; break;
        case MC_FC_INFO:                return "FC info"; break;
        case MC_FC_CMD_SAVE:            return "FC cmd save"; break;
        case MC_FC_CMD_SHUTDOWN:        return "FC cmd shutdown"; break;
        case MC_FC_L2X0_CTRL:           return "FC L2X0 ctrl"; break;
        case MC_FC_L2X0_SETUP1:         return "FC L2X0 setup 1"; break;
        case MC_FC_L2X0_SETUP2:         return "FC L2X0 setup 2"; break;
        case MC_FC_L2X0_INVALL:         return "FC L2X0 invall"; break;
        case MC_FC_L2X0_DEBUG:          return "FC L2X0 debug"; break;
        case MC_FC_MEM_TRACE:           return "FC mem trace"; break;
        case MC_FC_STAT_COUNTER:        return "FC stat counter"; break;
        case MC_FC_PM_SUSPEND:          return "FC PM suspend"; break;
        case MC_FC_PM_SUSPEND_CANCEL:   return "FC PM suspend cancel"; break;
        case MC_FC_PM_QUERY:            return "FC PM quesry"; break;
        case MC_FC_CP15_REG:            return "FC CP15 reg"; break;
        case MC_FC_STORE_BINFO:         return "FC store info"; break;
        case MC_FC_LOAD_BINFO:          return "FC load info"; break;
        case MC_FC_SWAP_CPU:            return "FC swap CPU"; break;
    #if defined(__AARCH32__)
        case MC_FC_FIQ_FORWARD_CONFIG:  return "FC FIQ forward config"; break;
    #endif
        default:                        return "FC unknown"; break;
    }
}

// MTK Syscalls
#define SYSCALLNO_TASK_DONATE            0
#define SYSCALLNO_TASK_ACTIVATE          1
#define SYSCALLNO_IPCRIGHT_ATTACH        2
#define SYSCALLNO_IPCRIGHT_DETACH        3
#define SYSCALLNO_TASK_ATTR              4
#define SYSCALLNO_TASK_START             5
#define SYSCALLNO_TASK_TERMINATE         6
#define SYSCALLNO_MAP_PHYS               7
#define SYSCALLNO_UNMAP                  8
#define SYSCALLNO_THREAD_START           9
#define SYSCALLNO_THREAD_STOP           10
#define SYSCALLNO_TIME_DONATE           11
#define SYSCALLNO_THREAD_EX_REGS        12
#define SYSCALLNO_MAP                   13
#define SYSCALLNO_THREAD_SUSPEND        14
#define SYSCALLNO_THREAD_GET_ATTR       15
#define SYSCALLNO_THREAD_SET_ATTR       16
#define SYSCALLNO_IPC                   17
#define SYSCALLNO_INTR_ATTACH           18
#define SYSCALLNO_INTR_DETACH           19
#define SYSCALLNO_SIGNAL_WAIT           20
#define SYSCALLNO_SIGNAL_SET            21
#define SYSCALLNO_SIGNAL_CLEAR          22
#define SYSCALLNO_PLAT_CONTROL          23
#define SYSCALLNO_LOG                   24
#define SYSCALLNO_TIME                  25
#define SYSCALLNO_CONTROL               26
#define SYSCALLNO_ATF                   27
#define SYSCALLNO_VIRT_INTR_ALLOC       28

#define CTRL_MAP_PHYS_L2_TABLE          129
#define CTRL_MAP_PHYS_L1                133
#define CTRL_MAP_L1                     130
#define CTRL_UNMAP_L1                   131
#define CTRL_COPY_WITH_OVERWRITE_L1     132
#define CTRL_MAP_PHYS_L2_SECTION        134
#define CTRL_KMAP_SECTION               135
#define CTRL_MC                         143
#define CTRL_DBG                        144
#define CTRL_VA2PA                      145
#define CTRL_CACHE                      146
#define CTRL_SET_FASTCALL_HOOK          148
#define CTRL_CALL_FASTCALL              152
#define CTRL_GET_MEM_TYPE               150
#define CTRL_REMAP_AND_RELEASE_L2_TABLE 151
#define CTRL_PLAT_DRIVERS_SUSPENDED     251
#define CTRL_PLAT_DRIVERS_RESUMED       252

static inline const char *syscallToString(uint32_t syscall) {
    switch(syscall) {
        case SYSCALLNO_TASK_DONATE :           return "TASK_DONATE";
        case SYSCALLNO_TASK_ACTIVATE :         return "TASK_ACTIVATE";
        case SYSCALLNO_IPCRIGHT_ATTACH :       return "IPCRIGHT_ATTACH";
        case SYSCALLNO_IPCRIGHT_DETACH :       return "IPCRIGHT_DETACH";
        case SYSCALLNO_TASK_ATTR :             return "TASK_ATTR";
        case SYSCALLNO_TASK_START :            return "TASK_START";
        case SYSCALLNO_TASK_TERMINATE :        return "TASK_TERMINATE";
        case SYSCALLNO_MAP_PHYS :              return "MAP_PHYS";
        case SYSCALLNO_UNMAP :                 return "UNMAP";
        case SYSCALLNO_THREAD_START :          return "THREAD_START";
        case SYSCALLNO_THREAD_STOP :           return "THREAD_STOP";
        case SYSCALLNO_TIME_DONATE :           return "TIME_DONATE";
        case SYSCALLNO_THREAD_EX_REGS :        return "THREAD_EX_REGS";
        case SYSCALLNO_MAP :                   return "MAP";
        case SYSCALLNO_THREAD_SUSPEND :        return "THREAD_SUSPEND";
        case SYSCALLNO_THREAD_GET_ATTR :       return "THREAD_GET_ATTR";
        case SYSCALLNO_THREAD_SET_ATTR :       return "THREAD_SET_ATTR";
        case SYSCALLNO_IPC :                   return "IPC";
        case SYSCALLNO_INTR_ATTACH :           return "INTR_ATTACH";
        case SYSCALLNO_INTR_DETACH :           return "INTR_DETACH";
        case SYSCALLNO_SIGNAL_WAIT :           return "SIGNAL_WAIT";
        case SYSCALLNO_SIGNAL_SET :            return "SIGNAL_SET";
        case SYSCALLNO_SIGNAL_CLEAR :          return "SIGNAL_CLEAR";
        case SYSCALLNO_PLAT_CONTROL :          return "PLAT_CONTROL";
        case SYSCALLNO_LOG :                   return "LOG";
        case SYSCALLNO_TIME :                  return "TIME";
        case SYSCALLNO_CONTROL :               return "CONTROL";
        case SYSCALLNO_ATF :                   return "ATF";
        case SYSCALLNO_VIRT_INTR_ALLOC :       return "VIRT_INTR_ALLOC";
        default :                              return "Unknown syscall";
    }
}

static inline const char *sysCtrlToString(uint32_t sysCtrl) {
    switch(sysCtrl) {
        case CTRL_MAP_PHYS_L2_TABLE :          return "CTRL_MAP_PHYS_L2_TABLE";
        case CTRL_MAP_PHYS_L1 :                return "CTRL_MAP_PHYS_L1";
        case CTRL_MAP_L1 :                     return "CTRL_MAP_L1";
        case CTRL_UNMAP_L1 :                   return "CTRL_UNMAP_L1";
        case CTRL_COPY_WITH_OVERWRITE_L1 :     return "CTRL_COPY_WITH_OVERWRITE_L1";
        case CTRL_MAP_PHYS_L2_SECTION :        return "CTRL_MAP_PHYS_L2_SECTION";
        case CTRL_KMAP_SECTION :               return "CTRL_KMAP_SECTION";
        case CTRL_MC :                         return "CTRL_MC";
        case CTRL_DBG :                        return "CTRL_DBG";
        case CTRL_VA2PA :                      return "CTRL_VA2PA";
        case CTRL_CACHE :                      return "CTRL_CACHE";
        case CTRL_SET_FASTCALL_HOOK :          return "CTRL_SET_FASTCALL_HOOK";
        case CTRL_CALL_FASTCALL :              return "CTRL_CALL_FASTCALL";
        case CTRL_GET_MEM_TYPE :               return "CTRL_GET_MEM_TYPE";
        case CTRL_REMAP_AND_RELEASE_L2_TABLE : return "CTRL_REMAP_AND_RELEASE_L2_TABLE";
        case CTRL_PLAT_DRIVERS_SUSPENDED :     return "CTRL_PLAT_DRIVERS_SUSPENDED";
        case CTRL_PLAT_DRIVERS_RESUMED :       return "CTRL_PLAT_DRIVERS_RESUMED";
        default :                              return "Unknown sysCtrl";
    }
}

static inline const char *smgntTypeToString(uint32_t t) {
    const char * sTypes[] = {
            "Illegal",
            "Driver",
            "SP TA",
            "System TA",
            "Middleware"
    };
    int i = (t >= SERVICE_TYPE_LAST_ENTRY) ? 0 : t;
    return sTypes[i];
}
// Locals/Code/MC/RTM/SMGNT/public/smgnt.c:} sessionState_t;

static inline const char *smgntStateToString(sessionState_t t) {
    const char * sStates[] = {
            "Unused",
            "Initializing",
            "Active",
            "Closing",
            "Closed",
            "Dead"
    };
    int i = (t > SESSION_STATE_ENDPOINT_DIED) ? 0 : t;
    return sStates[i];
}
// Locals/Code/MC/RTM/SMGNT/public/smgnt.c:} sessionState_t;

static inline const char *exchTypeToString(trapTypes_t type) {
    const char *sTypes[] = {
        "Unknown",
        "Syscall",
        "Segmentation",
        "Alignment",
        "Undef instr",
        "Breakpoint",
        "Arithmetic",
        "Instr fetch",
        "Interrupt",
        "Timefault",
        "Task terminate",
        "Task activate",
        "Task start"
    };

    if (type >= TRAP_INTERRUPT) return sTypes[type-8];
    if (type > TRAP_INSTR_FETCH) return sTypes[0];

    return sTypes[type];
}

#endif //(MC_DEFINES_H_)
