/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2018
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef __PERF_MON_H__
#define __PERF_MON_H__

#include <stdint.h>

typedef enum {
    PERF_MON_STATUS_OK                    =  0, /* Normal */
    PERF_MON_STATUS_ERROR                 = -1, /* Error */
    PERF_MON_STATUS_NULL                  = -2, /* NULL pointer */
    PERF_MON_STATUS_INVALID_ORDER         = -3, /* Order out of the menu */
    PERF_MON_STATUS_MENU_MISMATCH         = -4, /* Order is on the menu but mismatch the counter configurations */
    PERF_MON_STATUS_INVALID_PARAMETER     = -5, /* Invalid function input parameter */
    PERF_MON_STATUS_INVALID_CONFIGURATION = -6, /* Invalid counter configuration */
    PERF_MON_STATUS_REENTRY               = -7, /* Function re-entry */
    PERF_MON_STATUS_CALL_STOP             = -8  /* Need to call perf_mon_stop() before using this API */
} perf_mon_status_t;

typedef enum {
    PERF_MON_ICACHE_MISS_COUNT = 0,
    PERF_MON_ICACHE_HIT_COUNT,
    PERF_MON_ICACHE_ACCESS_COUNT,
    PERF_MON_DCACHE_MISS_COUNT,
    PERF_MON_DCACHE_HIT_COUNT,
    PERF_MON_DCACHE_ACCESS_COUNT,
    PERF_MON_DCACHE_LOAD_MISS_COUNT,
    PERF_MON_DCACHE_LOAD_HIT_COUNT,
    PERF_MON_DCACHE_LOAD_ACCESS_COUNT,
    PERF_MON_DCACHE_STORE_MISS_COUNT,
    PERF_MON_DCACHE_STORE_HIT_COUNT,
    PERF_MON_DCACHE_STORE_ACCESS_COUNT,
    PERF_MON_IPREFETCH_MISS_COUNT,
    PERF_MON_IPREFETCH_HIT_COUNT,
    PERF_MON_IPREFETCH_ACCESS_COUNT,
    PERF_MON_DPREFETCH_MISS_COUNT,
    PERF_MON_DPREFETCH_HIT_COUNT,
    PERF_MON_DPREFETCH_ACCESS_COUNT,
    PERF_MON_MENU_TOTAL
} perf_mon_menu_t;

#define PMSEL_CYCLES          0 /* Count cycles */
#define PMSEL_OVERFLOW        1 /* Overflow of counter n-1 (assuming this is counter n) */
#define PMSEL_INSN            2 /* Successfully completed instructions */
#define PMSEL_D_STALL         3 /* Data-related global stall cycles */
#define PMSEL_I_STALL         4 /* Instruction-related & other global stall cycles */
#define PMSEL_EXR             5 /* Exceptions & pipeline replays */
#define PMSEL_BUBBLES         6 /* Hold & other bubble cycles */
#define PMSEL_I_TLB           7 /* Instruction TLB    accesses (per instruction retiring) */
#define PMSEL_I_MEM           8 /* Instruction memory accesses (per instruction retiring) */
#define PMSEL_D_TLB           9 /* Data TLB accesses */
#define PMSEL_D_LOAD_U1      10 /* Data memory load  instruction (load-store unit 1) */
#define PMSEL_D_STORE_U1     11 /* Data memory store instruction (load-store unit 1) */
#define PMSEL_D_ACCESS_U1    12 /* Data memory accesses          (load-store unit 1) */
#define PMSEL_D_LOAD_U2      13 /* Data memory load  instruction (load-store unit 2) */
#define PMSEL_D_STORE_U2     14 /* Data memory store instruction (load-store unit 2) */
#define PMSEL_D_ACCESS_U2    15 /* Data memory accesses          (load-store unit 2) */
#define PMSEL_D_LOAD_U3      16 /* Data memory load  instruction (load-store unit 3) */
#define PMSEL_D_STORE_U3     17 /* Data memory store instruction (load-store unit 3) */
#define PMSEL_D_ACCESS_U3    18 /* Data memory accesses          (load-store unit 3) */
#define PMSEL_MULTIPLE_LS    22 /* Multiple load / store */
#define PMSEL_OUTBOUND_PIF   23 /* Outbound PIF transactions */
#define PMSEL_INBOUND_PIF    24 /* Inbound  PIF transactions */
#define PMSEL_PREFETCH       26 /* Prefetch events */

#define PMMSK_INSN_ALL                      0x8DFF
#define PMMSK_INSN_JX                       0x0001 /* JX */
#define PMMSK_INSN_CALLX                    0x0002 /* CALLXn */
#define PMMSK_INSN_RET                      0x0004 /* call return i.e. RET, RETW */
#define PMMSK_INSN_RF                       0x0008 /* supervisor return i.e. RFDE, RFE, RFI, RFWO, RFWU */
#define PMMSK_INSN_BRANCH_TAKEN             0x0010 /* Conditional branch taken, or loopgtz/loopnez skips loop */
#define PMMSK_INSN_J                        0x0020 /* J */
#define PMMSK_INSN_CALL                     0x0040 /* CALLn */
#define PMMSK_INSN_BRANCH_NOT_TAKEN         0x0080 /* Conditional branch fall through (aka. not-taken branch) */
#define PMMSK_INSN_LOOP_TAKEN               0x0100 /* Loop instr falls into loop (aka. taken loop) */
#define PMMSK_INSN_LOOP_BEG                 0x0400 /* Loopback taken to LBEG */
#define PMMSK_INSN_LOOP_END                 0x0800 /* Loopback falls through to LEND */
#define PMMSK_INSN_NON_BRANCH               0x8000 /* Non-branch instruction (aka. non-CTI) */

#define PMMSK_D_STALL_ALL                   0x01FE
#define PMMSK_D_STALL_STORE_BUF_FULL        0x0002 /* Store buffer full stall */
#define PMMSK_D_STALL_STORE_BUF_CONFLICT    0x0004 /* Store buffer conflict stall */
#define PMMSK_D_STALL_CACHE_MISS            0x0008 /* DCache-miss stall */
#define PMMSK_D_STALL_BUSY                  0x0010 /* Data RAM/ROM/XLMI busy stall */
#define PMMSK_D_STALL_IN_PIF                0x0020 /* Data inbound-PIF request stall (incl s32c1i) */
#define PMMSK_D_STALL_MHT_LOOKUP            0x0040 /* MHT lookup stall */
#define PMMSK_D_STALL_UNCACHED_LOAD         0x0080 /* Uncached load stall (included in MHT lookup stall) */
#define PMMSK_D_STALL_BANK_CONFLICT         0x0100 /* Bank-conflict stall */

#define PMMSK_I_STALL_ALL                   0x01FF
#define PMMSK_I_STALL_CACHE_MISS            0x0001 /* ICache-miss stall */
#define PMMSK_I_STALL_BUSY                  0x0002 /* Instruction RAM/ROM busy stall */
#define PMMSK_I_STALL_IN_PIF                0x0004 /* Instruction RAM inbound-PIF request stall */
#define PMMSK_I_STALL_TIE_PORT              0x0008 /* TIE port stall */
#define PMMSK_I_STALL_EXTERNAL_SIGNAL       0x0010 /* External RunStall signal status */
#define PMMSK_I_STALL_UNCACHED_FETCH        0x0020 /* Uncached fetch stall */
#define PMMSK_I_STALL_FAST_L32R             0x0040 /* FastL32R stall */
#define PMMSK_I_STALL_ITERATIVE_MUL         0x0080 /* Iterative multiply stall */
#define PMMSK_I_STALL_ITERATIVE_DIV         0x0100 /* Iterative divide stall */

#define PMMSK_EXR_ALL                       0x01FF
#define PMMSK_EXR_REPLAYS                   0x0001 /* Other Pipeline Replay (i.e. excludes $ miss etc.) */
#define PMMSK_EXR_LEVEL1_INT                0x0002 /* Level-1 interrupt */
#define PMMSK_EXR_LEVELH_INT                0x0004 /* Greater-than-level-1 interrupt */
#define PMMSK_EXR_DEBUG                     0x0008 /* Debug exception */
#define PMMSK_EXR_NMI                       0x0010 /* NMI */
#define PMMSK_EXR_WINDOW                    0x0020 /* Window exception */
#define PMMSK_EXR_ALLOCA                    0x0040 /* Alloca exception */
#define PMMSK_EXR_OTHER                     0x0080 /* Other exceptions */
#define PMMSK_EXR_MEM_ERR                   0x0100 /* HW-corrected memory error */

#define PMMSK_BUBBLES_ALL                   0x01FD
#define PMMSK_BUBBLES_PSO                   0x0001 /* Processor domain PSO bubble */
#define PMMSK_BUBBLES_R_HOLD_D_CACHE_MISS   0x0004 /* R hold caused by DCache miss */
#define PMMSK_BUBBLES_R_HOLD_STORE_RELEASE  0x0008 /* R hold caused by Store release */
#define PMMSK_BUBBLES_R_HOLD_REG_DEP        0x0010 /* R hold caused by register dependency */
#define PMMSK_BUBBLES_R_HOLD_WAIT           0x0020 /* R hold caused by MEMW, EXTW or EXCW */
#define PMMSK_BUBBLES_R_HOLD_HALT           0x0040 /* R hold caused by Halt instruction (TX only) */
#define PMMSK_BUBBLES_CTI                   0x0080 /* CTI bubble (e.g. branch delay slot) */
#define PMMSK_BUBBLES_WAITI                 0x0100 /* WAITI bubble */

#define PMMSK_I_TLB_ALL                     0x000F
#define PMMSK_I_TLB_HITS                    0x0001 /* Hit */
#define PMMSK_I_TLB_REPLAYS                 0x0002 /* Replay of instruction due to ITLB miss */
#define PMMSK_I_TLB_REFILLS                 0x0004 /* HW-assisted TLB Refill completes */
#define PMMSK_I_TLB_MISSES                  0x0008 /* ITLB Miss Exception */

#define PMMSK_I_MEM_ALL                     0x000F
#define PMMSK_I_MEM_CACHE_HITS              0x0001 /* ICache Hit */
#define PMMSK_I_MEM_CACHE_MISSES            0x0002 /* ICache Miss (includes uncached) */
#define PMMSK_I_MEM_IRAM                    0x0004 /* InstRAM or InstROM */
#define PMMSK_I_MEM_BYPASS                  0x0008 /* Bypass (i.e. uncached) fetch */

#define PMMSK_D_TLB_ALL                     0x000F
#define PMMSK_D_TLB_HITS                    0x0001 /* Hit */
#define PMMSK_D_TLB_REPLAYS                 0x0002 /* Replay of instruction due to DTLB miss */
#define PMMSK_D_TLB_REFILLS                 0x0004 /* HW-assisted TLB Refill completes */
#define PMMSK_D_TLB_MISSES                  0x0008 /* DTLB Miss Exception */

#define PMMSK_D_LOAD_ALL                    0x000F
#define PMMSK_D_LOAD_CACHE_HITS             0x0001 /* Cache Hit */
#define PMMSK_D_LOAD_CACHE_MISSES           0x0002 /* Cache Miss */
#define PMMSK_D_LOAD_LOCAL_MEM              0x0004 /* Local memory hit */
#define PMMSK_D_LOAD_BYPASS                 0x0008 /* Bypass (i.e. uncached) load */

#define PMMSK_D_STORE_ALL                   0x000F
#define PMMSK_D_STORE_CACHE_HITS            0x0001 /* DCache Hit */
#define PMMSK_D_STORE_CACHE_MISSES          0x0002 /* DCache Miss */
#define PMMSK_D_STORE_LOCAL_MEM             0x0004 /* Local memory hit */
#define PMMSK_D_STORE_PIF                   0x0008 /* PIF Store */

#define PMMSK_D_ACCESS_ALL                  0x000F
#define PMMSK_D_ACCESS_CACHE_MISSES         0x0001 /* DCache Miss */
#define PMMSK_D_ACCESS_HITS_SHARED          0x0002 /* Hit Shared */
#define PMMSK_D_ACCESS_HITS_EXCLUSIVE       0x0004 /* Hit Exclusive */
#define PMMSK_D_ACCESS_HITS_MODIFIED        0x0008 /* Hit Modified */

#define PMMSK_MULTIPLE_LS_ALL               0x003F
#define PMMSK_MULTIPLE_LS_0S_0L             0x0001 /* 0 stores and 0 loads */
#define PMMSK_MULTIPLE_LS_0S_1L             0x0002 /* 0 stores and 1 loads */
#define PMMSK_MULTIPLE_LS_1S_0L             0x0004 /* 1 stores and 0 loads */
#define PMMSK_MULTIPLE_LS_1S_1L             0x0008 /* 1 stores and 1 loads */
#define PMMSK_MULTIPLE_LS_0S_2L             0x0010 /* 0 stores and 2 loads */
#define PMMSK_MULTIPLE_LS_2S_0L             0x0020 /* 2 stores and 0 loads */

#define PMMSK_OUTBOUND_PIF_ALL              0x0003
#define PMMSK_OUTBOUND_PIF_CASTOUT          0x0001 /* Castout */
#define PMMSK_OUTBOUND_PIF_PREFETCH         0x0002 /* Prefetch */

#define PMMSK_INBOUND_PIF_ALL               0x0003
#define PMMSK_INBOUND_PIF_I_DMA             0x0001 /* Instruction DMA */
#define PMMSK_INBOUND_PIF_D_DMA             0x0002 /* Data DMA */

#define PMMSK_PREFETCH_ALL                  0x002F
#define PMMSK_PREFETCH_I_HIT                0x0001 /* I prefetch-buffer-lookup hit */
#define PMMSK_PREFETCH_D_HIT                0x0002 /* D prefetch-buffer-lookup hit */
#define PMMSK_PREFETCH_I_MISS               0x0004 /* I prefetch-buffer-lookup miss */
#define PMMSK_PREFETCH_D_MISS               0x0008 /* D prefetch-buffer-lookup miss */
#define PMMSK_PREFETCH_D_L1_FILL            0x0020 /* Fill directly to DCache L1 */

#define PMCTRL_INT          (0x1)
#define PMCTRL_SEL_MASK     (0x1F)
#define PMCTRL_MSK_MASK     (0xFFFF)
#define PMCTRL_TRL_MASK     (0xF)
#define PMCTRL_KNC_MASK     (0x1)
#define PMCTRL_SEL_SHIFT    (8)
#define PMCTRL_MSK_SHIFT    (16)
#define PMCTRL_TRL_SHIFT    (4)
#define PMCTRL_KNC_SHIFT    (3)

#define SET_PMCTRL_SEL(val)     (((val) & PMCTRL_SEL_MASK) << PMCTRL_SEL_SHIFT)
#define SET_PMCTRL_MSK(val)     (((val) & PMCTRL_MSK_MASK) << PMCTRL_MSK_SHIFT)
#define SET_PMCTRL_TRL(val)     (((val) & PMCTRL_TRL_MASK) << PMCTRL_TRL_SHIFT)
#define SET_PMCTRL_KNC(val)     (((val) & PMCTRL_KNC_MASK) << PMCTRL_KNC_SHIFT)
#define SET_PMCTRL(sel, msk, trl, knc)  (SET_PMCTRL_SEL(sel) | SET_PMCTRL_MSK(msk) | SET_PMCTRL_TRL(trl) | SET_PMCTRL_KNC(knc))

#define PM_PRESET_BIT_I_MEM_CACHE_MISSES   (0x1 << 0)
#define PM_PRESET_BIT_I_MEM_CACHE_HITS     (0x1 << 1)
#define PM_PRESET_BIT_D_LOAD_CACHE_MISSES  (0x1 << 2)
#define PM_PRESET_BIT_D_LOAD_CACHE_HITS    (0x1 << 3)
#define PM_PRESET_BIT_D_STORE_CACHE_MISSES (0x1 << 4)
#define PM_PRESET_BIT_D_STORE_CACHE_HITS   (0x1 << 5)
#define PM_PRESET_BIT_PREFETCH_I_MISS      (0x1 << 6)
#define PM_PRESET_BIT_PREFETCH_D_MISS      (0x1 << 7)

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_init                                                         |
|                                                                             |
|   Description:                                                              |
|       Initialize the performance monitor driver.                            |
|       Setup the performance monitor counters by the default configurations. |
|       Reset all the performance monitor counters.                           |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_init (void);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_start                                                        |
|                                                                             |
|   Description:                                                              |
|       Start the performance monitor counters.                               |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_start (void);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_stop                                                         |
|                                                                             |
|   Description:                                                              |
|       Stop the performance monitor counters.                                |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_stop (void);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_get_value                                                    |
|                                                                             |
|   Description:                                                              |
|       Get the value of the given performance monitor order.                 |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_get_value (perf_mon_menu_t order, uint64_t *p_value);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_reset_counters                                               |
|                                                                             |
|   Description:                                                              |
|       Reset all the performance monitor counters.                           |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_reset_counters (void);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_get_number_of_counters                                       |
|                                                                             |
|   Description:                                                              |
|       Get the total number of hardware performance counters.                |
|                                                                             |
|   Return Values:                                                            |
|       Total number of hardware performance counters                         |
|                                                                             |
\*****************************************************************************/
uint32_t perf_mon_get_number_of_counters (void);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_set_configuration                                            |
|                                                                             |
|   Description:                                                              |
|       Setup the performance monitor counters by the given configurations.   |
|       Reset all the performance monitor counters.                           |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_set_configuration (uint32_t *p_config, uint32_t num);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_select_preset_bits                                           |
|                                                                             |
|   Description:                                                              |
|       Setup the performance monitor counters by the given preset bits.      |
|       The raised preset bits should not exceed the number of counters.      |
|       Example: Assume the number of counters equals to 8.                   |
|           ok     --> 0x000000FF                                             |
|           not ok --> 0x000001FF                                             |
|       Reset all the performance monitor counters.                           |
|       This API will start to set counters from lsb of the preset bits.      |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_select_preset_bits (uint32_t preset_bits);

/*****************************************************************************\
|   Function:                                                                 |
|       perf_mon_get_counters                                                 |
|                                                                             |
|   Description:                                                              |
|       Get the performance monitor counter values.                           |
|                                                                             |
|   Return Values:                                                            |
|       PERF_MON_STATUS_OK --> normal                                         |
|       Others             --> error                                          |
|                                                                             |
\*****************************************************************************/
perf_mon_status_t perf_mon_get_counters (uint64_t *p_value, uint32_t num);

#endif /* __PERF_MON_H__ */
