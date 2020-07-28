/*-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2009-2010 ARM Limited.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *
 *      Revision            : $Revision: #1 $
 *
 *      Release information : cortexm4_r0p1_00rel0
 *-----------------------------------------------------------------------------
 */

////////////////////////////////////////////////////////////////////////////////
//
// Cortex-M4 Integration Kit Configuration File
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  Processor configuration options.
//  These must match the expected hardware configuration of the processor.
//
//  - EXPECTED_BE        : Expected Endianness (0,1)
//  - EXPECTED_BKPT      : Expected number of Breakpoint Comparators (0,2,6)
//  - EXPECTED_TRACE_LVL : Expected Trace Config (0,1,2,3)
//  - EXPECTED_DBG_PORT  : Expected Debug config (0,1)
//  - EXPECTED_JTAGnSW   : Expected CORTEXM4-DAP Protocol (0,1)
//  - EXPECTED_NUMIRQ    : Expected number of interrupts (1..240)
//  - EXPECTED_WPT       : Expected number of Watchpoint Comparators (0,1,4)
//  - EXPECTED_DMATCH    : Expected Data Matching (0,1)
//  - EXPECTED_MPU       : Expected MPU configuration (0,1)
//  - EXPECTED_FPU       : Expected FPU configuration (0,1)
//  - EXPECTED_LVL_WIDTH : Expected number of priority levels (3..8)
//  - EXPECTED_BB        : Expected bit banding config (0,1)
//  - EXPECTED_WIC       : Expected WIC config (0,1)
//  - EXPECTED_WIC_LINES : Expected WIC lines (3..243)

// RealView MDK GUI Support
// <<< Use Configuration Wizard in Context Menu >>>
// <h> Processor configuration options
// <o> EXPECTED_BE: Expected Endianness <0=> Little Endian
//                                      <1=> Big Endian
#define EXPECTED_BE             0

// <o> EXPECTED_BKPT: Expected number of Breakpoint Comparators <0=> 0
//                                                              <2=> 2
//                                                              <6=> 6
#define EXPECTED_BKPT           6

// <o> EXPECTED_TRACE_LVL: Expected Trace level  <0=> No Trace (0)
//                                               <1=> ITM Trace (1)
//                                               <2=> ETM,ITM (2)
//                                               <3=> ETM,ITM,HTM (3)
#define EXPECTED_TRACE_LVL      2

// <o> EXPECTED_DBG_PORT: Debug Port Configuration <0=> Absent <1=> Present
#define EXPECTED_DBG_PORT       1

// <o> EXPECTED_JTAGnSW: DAP Protocol to use <0=> Serial Wire <1=> JTAG
#define EXPECTED_JTAGnSW        1

// <o> EXPECTED_NUMIRQ: Expected number of interrupts <1-240>
#define EXPECTED_NUMIRQ         16

// <o> EXPECTED_WPT: Expected number of Watchpoint Comparators <0=> No DWT <1=> DWT_COMP0 <4=> DWT_COMP0,1,2,3
#define EXPECTED_WPT            4

// <o> EXPECTED_DMATCH: Expected DWT Data Matching <0=> Absent <1=> Present
#define EXPECTED_DMATCH         1

// <o> EXPECTED_MPU: Expected MPU configuration <0=> Absent <1=> Present
#define EXPECTED_MPU            0

// <o> EXPECTED_FPU: Expected FPU configuration <0=> Absent <1=> Present
#define EXPECTED_FPU            0

// <o> EXPECTED_LVL_WIDTH: Expected number of priority levels <3-8>
#define EXPECTED_LVL_WIDTH      3

// <o> EXPECTED_BB: Expected bit banding config <0=> Absent <1=> Present
#define EXPECTED_BB             1

// <o> EXPECTED_WIC: Expected WIC config <0=> Absent <1=> Present
#define EXPECTED_WIC            0

// <o> EXPECTED_WIC_LINES: Expected WIC lines <3-243>
#define EXPECTED_WIC_LINES      3

// Processor port Tie-Offs.
//
// <o.0..25> EXPECTED_STCALIB: Expected value of STCALIB[25:0] at CORTEXM4INTEGRATION or CORTEXM4 <0x0-0x3FFFFFF>
#define EXPECTED_STCALIB        0x1028B0A

// <o> EXPECTED_BASEADDR: Expected value of RomTbl[31:0] at cm4_dap_ahb_ap_defs.v <0x0-0xFFFFFFFF>
#define EXPECTED_BASEADDR       0xE00FF003

// </h>
// <<< end of configuration section >>>



////////////////////////////////////////////////////////////////////////////////
//
// Integration Kit Test Configuration
//

//
// Define whether the IK tests and the DebugDriver code should display messages
// using the chracter output device in the testbench.
//
#define CM4IKMCU_PRINTF
//#define DEBUGDRIVER_PRINTF
//#define DEBUGTRACE_PRINTF

// These values are used for TRACE ID when testing trace.
// Allows the formatter output to be associated with the generating source

#define CM4IKMCU_ETM_ID 0x23
#define CM4IKMCU_ITM_ID 0x59

////////////////////////////////////////////////////////////////////////////////
//
// ID value revision codes PID2[7:4]
//
#define CORTEXM4_SCS_REV        0x0
#define CORTEXM4_ITM_REV        0x3
#define CORTEXM4_DWT_REV        0x3
#define CORTEXM4_FPB_REV        0x2
#define CORTEXM4_SCS_F_REV      0x0
#define CORTEXM4_ROM_REV        0x0
#define CORTEXM4_TPIU_REV       0x0
#define CORTEXM4_ETM_REV        0x0

// revand and customer modified fields for PID3
#define CORTEXM4_SCS_ECO        0x00
#define CORTEXM4_ITM_ECO        0x00
#define CORTEXM4_DWT_ECO        0x00
#define CORTEXM4_FPB_ECO        0x00
#define CORTEXM4_SCS_F_ECO      0x00
#define CORTEXM4_ROM_ECO        0x00
#define CORTEXM4_TPIU_ECO       0x00
#define CORTEXM4_ETM_ECO        0x00

////////////////////////////////////////////////////////////////////////////////
//
// ID values - DO NOT MODIFY
//
#define CORTEXM4DAP_JTAG_TAPID  0x0BA01477
// Note that DPIDR is not supported in this version of JTAG-DP
// Use TAPID instead
#define CORTEXM4DAP_JTAG_DPIDR  0x00000000
#define CORTEXM4DAP_SW_DPIDR    0x2BA01477
#define CORTEXM4DAP_AP_IDR      0x04770021
#define CORTEXM4_CPUID          0x410fc241
#define CORTEXM4_ROM_PID3       0x00


// Component identifier for peripheral classes
#define CORESIGHT_CID0           0x0D
#define CORESIGHT_CID2           0x05
#define CORESIGHT_CID3           0xB1
// CPU system peripherals
#define CS_CPU_PER_CID1          0xE0
// Debug/Trace Component
#define CS_CORESIGHT_ID1         0x90

// Component Part Numbers
//
// PID0 is PART[7:0]
// PID1 is [JEP[3:0]PART[11:8]
// PID2 is REV,1,JEP[6:4]
// PID3 is [revand][customer modified]
// PID4 id 4k count,JEP_CONT

#define ARM_JEP_ID               0x3B
#define ARM_JEP_CONT             0x4

#define CORTEXM4_SCS_PART        0x000
#define CORTEXM4_ITM_PART        0x001
#define CORTEXM4_DWT_PART        0x002
#define CORTEXM4_FPB_PART        0x003
#define CORTEXM4_SCS_F_PART      0x00C
#define CORTEXM4_ROM_PART        0x4C4
#define CS_TPIULITE_PART         0x941
#define CS_TPIU_PART             0x912
#define CS_CTI_PART              0x906
#define CS_ETB_PART              0x907
#define CS_FUNNEL_PART           0x908
#define CS_ITM_PART              0x913
#define CORTEXM4_TPIU_PART       0x9A1
#define CORTEXM4_ETM_PART        0x925

#define CORTEXM4_SCB_MVFR0      0x10110021
#define CORTEXM4_SCB_MVFR1      0x11000011


////////////////////////////////////////////////////////////////////////////////
