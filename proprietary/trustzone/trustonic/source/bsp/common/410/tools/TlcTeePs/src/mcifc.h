/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
/**
 * Holds the functions for SIQ, YIELD and FastCall for switching to the secure world.
 */
#ifndef MCIFC_H_
#define MCIFC_H_

/** @name MobiCore FastCall Defines
 * Defines for the two different FastCall's.
 */
/** @{ */

// ARM SMC calling convention defines:
//   bit 31 is used to indicate Standard call/Fast Call (atomic)
//   bit 30 is used to indicate SMC32/64
//   bit 29-24 is the owning entity.
//     0x00:        ARM Architecture Calls
//     0x01:        CPU Service Calls
//     0x02:        SiP Service Calls
//     0x03:        OEM Service Calls
//     0x04:        Standard Secure Service Calls
//     0x05:        Standard Hypervisor Service Calls
//     0x06:        Vendor Specific Hypervisor Service Calls
//     0x07 - 0x2F: Reserved for future use
//     0x30 - 0x31: Trusted Application Calls
//     0x32 - 0x3F: Trusted OS Calls

// --- global ----
#define MC_FC_INVALID                ((uint32_t)  0 )  /**< Invalid FastCall ID */

// These are defined outside of the __AARCH32__ define so that new fastcalls
// that are compatible with both v7 and v8 can be defined.
// These are 32-bit FCs since TEE OS is 32-bit OS
#define FASTCALL_OWNER_TZOS          (0x3F000000)
#define FASTCALL_ATOMIC_MASK         (1U << 31)
#define MC_FC_STD32_BASE	     ((uint32_t)(FASTCALL_OWNER_TZOS | FASTCALL_ATOMIC_MASK))
#define MC_FC_STD32(x)               ((uint32_t)(MC_FC_STD32_BASE + (x)))

#define MC_FC_TRIGGER_INTERRUPT      MC_FC_STD32(61)


#if defined(__AARCH32__) || (defined(__aarch64__) && defined(__KERNEL__))

#define MC_FC_INIT                   MC_FC_STD32(1)  /**< Initializing FastCall. */
#define MC_FC_INFO                   MC_FC_STD32(2)  /**< Info FastCall. */

// --- MEM traces ---
#define MC_FC_MEM_TRACE              MC_FC_STD32(10)  /**< Enable SWd tracing via memory */

// --- store value in sDDRRAM ---
#define MC_FC_STORE_BINFO            MC_FC_STD32(20)  /**< write a 32bit value in secure DDRRAM in incremented art (max 2kB) */
#define MC_FC_LOAD_BINFO             MC_FC_STD32(21)  /**< load a 32bit value from secure DDRRAM using an offset */

// --- system settings ---
#define MC_FC_STAT_COUNTER           MC_FC_STD32(30)  /**< Require status counter */

// --- sleep modes ---
#define MC_FC_PM_SUSPEND             MC_FC_STD32(42) /**< put drivers to sleep, then enter power-sleep if r1 is not 0 */
#define MC_FC_PM_SUSPEND_CANCEL      MC_FC_STD32(43) /**< wake up drivers */
#define MC_FC_PM_QUERY               MC_FC_STD32(44) /**< query the state of drivers (for test) */

// --- wake-up access ---
#define MC_FC_CMD_SAVE               MC_FC_STD32(52)  /**< Save core context to CP15 table(r1 is core number) */
#define MC_FC_CMD_SHUTDOWN           MC_FC_STD32(53)  /**< Shutdown core(r1 is core number, cache flush is expected) */
#define MC_FC_SWAP_CPU               MC_FC_STD32(54)  /**< Change new active Core */

// --- forward fiq configuration ---
#define MC_FC_FIQ_FORWARD_CONFIG     MC_FC_STD32(56)  /**< Configure FIQ forwarding to ARM Trusted Firmware */

// --- blacklist memory management ---
#define TEE_FC_BLACKLIST_ADD         MC_FC_STD32(58)  /**< blacklist memory for TEE kernel */
#define TEE_FC_BLACKLIST_REMOVE      MC_FC_STD32(59)  /**< whitelist memory for TEE kernel */

#else

#define MC_FC_INIT                   ((uint32_t)(-1))  /**< Initializing FastCall. */
#define MC_FC_INFO                   ((uint32_t)(-2))  /**< Info FastCall. */
// following defines are currently frozen, so they will candidate for later big-change
// --- sleep modes ---
#define MC_FC_PM_SUSPEND             ((uint32_t)(-51)) /**< put drivers to sleep, then enter power-sleep if r1 is not 0 */
#define MC_FC_PM_SUSPEND_CANCEL      ((uint32_t)(-52)) /**< wake up drivers */
#define MC_FC_PM_QUERY               ((uint32_t)(-53)) /**< query the state of drivers (for test) */
// --- wake-up access ---
#define MC_FC_CMD_SAVE               ((uint32_t)(-6))  /**< Save core context to CP15 table(r1 is core number) */
#define MC_FC_CMD_SHUTDOWN           ((uint32_t)(-7))  /**< Shutdown core(r1 is core number, cache flush is expected) */
// --- L2 cache access ---
#define MC_FC_L2X0_CTRL             ((uint32_t)(-21))  /**< Write to L2X0 control register */
#define MC_FC_L2X0_SETUP1           ((uint32_t)(-22))  /**< Setup L2X0 register - part 1 */
#define MC_FC_L2X0_SETUP2           ((uint32_t)(-23))  /**< Setup L2X0 register - part 2 */
#define MC_FC_L2X0_INVALL           ((uint32_t)(-24))  /**< Invalidate all L2 cache */
#define MC_FC_L2X0_DEBUG            ((uint32_t)(-25))  /**< Write L2X0 debug register */

// --- MEM traces ---
#define MC_FC_MEM_TRACE             ((uint32_t)(-31))  /**< Enable SWd tracing via memory */
// --- system settings ---

#define MC_FC_STAT_COUNTER          ((uint32_t)(-41))  /**< Require status counter */
#define MC_FC_CP15_REG             ((uint32_t)(-101))  /**< general CP15/cache register update */
// --- store value in sDDRRAM ---
#define MC_FC_STORE_BINFO          ((uint32_t)(-201))  /**< write a 32bit value in secure DDRRAM in incremented art (max 2kB) */
#define MC_FC_LOAD_BINFO           ((uint32_t)(-202))  /**< load a 32bit value from secure DDRRAM using an offset */

#define MC_FC_SWAP_CPU             ((uint32_t)(0x84000005))  /**< Change new active Core */
// r1 is requested status (0,1,2), on return r2 holds this status value

#endif

/** @} */

/** @name MobiCore SMC Defines
 * Defines the different secure monitor calls (SMC) for world switching.
 * @{ */
#define MC_SMC_N_YIELD     0x3     /**< Yield to switch from NWd to SWd. */
#define MC_SMC_N_SIQ       0x4     /**< SIQ to switch from NWd to SWd. */
/** @} */

/** @name MobiCore status
 *  MobiCore status information.
 * @{ */
#define MC_STATUS_NOT_INITIALIZED  0   /**< MobiCore is not yet initialized. FastCall FcInit() has to be used function to set up MobiCore.*/
#define MC_STATUS_BAD_INIT         1   /**< Bad parameters have been passed in FcInit(). */
#define MC_STATUS_INITIALIZED      2   /**< MobiCore did initialize properly. */
#define MC_STATUS_HALT             3   /**< MobiCore kernel halted due to an unrecoverable exception. Further information is available extended info */
/** @} */

/** @name Extended Info Identifiers
 *  Extended info parameters for MC_FC_INFO to obtain further information depending on MobiCore state.
 * @{ */
#define MC_EXT_INFO_ID_MCI_VERSION      0 /**< Version of the MobiCore Control Interface (MCI) */
#define MC_EXT_INFO_ID_FLAGS            1 /**< MobiCore control flags */
#define MC_EXT_INFO_ID_HALT_CODE        2 /**< MobiCore halt condition code */
#define MC_EXT_INFO_ID_HALT_IP          3 /**< MobiCore halt condition instruction pointer */
#define MC_EXT_INFO_ID_FAULT_CNT        4 /**< MobiCore fault counter */
#define MC_EXT_INFO_ID_FAULT_CAUSE      5 /**< MobiCore last fault cause */
#define MC_EXT_INFO_ID_FAULT_META       6 /**< MobiCore last fault meta */
#define MC_EXT_INFO_ID_FAULT_THREAD     7 /**< MobiCore last fault threadid */
#define MC_EXT_INFO_ID_FAULT_IP         8 /**< MobiCore last fault instruction pointer */
#define MC_EXT_INFO_ID_FAULT_SP         9 /**< MobiCore last fault stack pointer */
#define MC_EXT_INFO_ID_FAULT_ARCH_DFSR  10 /**< MobiCore last fault ARM arch information */
#define MC_EXT_INFO_ID_FAULT_ARCH_ADFSR 11 /**< MobiCore last fault ARM arch information */
#define MC_EXT_INFO_ID_FAULT_ARCH_DFAR  12 /**< MobiCore last fault ARM arch information */
#define MC_EXT_INFO_ID_FAULT_ARCH_IFSR  13 /**< MobiCore last fault ARM arch information */
#define MC_EXT_INFO_ID_FAULT_ARCH_AIFSR 14 /**< MobiCore last fault ARM arch information */
#define MC_EXT_INFO_ID_FAULT_ARCH_IFAR  15 /**< MobiCore last fault ARM arch information */
#define MC_EXT_INFO_ID_MC_CONFIGURED    16 /**< MobiCore configured by Daemon via fc_init flag */
#define MC_EXT_INFO_ID_MC_SCHED_STATUS  17 /**< MobiCore scheduling status: idle/non-idle */
#define MC_EXT_INFO_ID_MC_STATUS        18 /**< MobiCore runtime status: initialized, halted */
#define MC_EXT_INFO_ID_MC_EXC_PARTNER   19 /**< MobiCore exception handler last partner */
#define MC_EXT_INFO_ID_MC_EXC_IPCPEER   20 /**< MobiCore exception handler last peer */
#define MC_EXT_INFO_ID_MC_EXC_IPCMSG    21 /**< MobiCore exception handler last IPC message */
#define MC_EXT_INFO_ID_MC_EXC_IPCDATA   22 /**< MobiCore exception handler last IPC data */
/** @} */

/** @name FastCall return values
 * Return values of the MobiCore FastCalls.
 * @{ */
#define MC_FC_RET_OK                       0     /**< No error. Everything worked fine. */
#define MC_FC_RET_ERR_INVALID              1     /**< FastCall was not successful. */
#define MC_FC_RET_ERR_ALREADY_INITIALIZED  5     /**< MobiCore has already been initialized. */
#define TEE_FC_RET_ERR_NOABILITY           6     /**< Call is not allowed. */
/** @} */

/** @name Init FastCall flags
 * Return flags of the Init FastCall.
 * @{ */
#define MC_FC_INIT_FLAG_LPAE       (1U << 0)     /**< SWd uses LPAE MMU table format. */
/** @} */

#endif /** MCIFC_H_ */

/** @} */
