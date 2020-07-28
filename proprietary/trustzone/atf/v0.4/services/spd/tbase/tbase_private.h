/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2014 TRUSTONIC LIMITED
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TBASE_PRIVATE_H__
#define __TBASE_PRIVATE_H__

#include <context.h>
#include <arch.h>
#include <psci.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>

#include <tbase.h>

/*******************************************************************************
 * Secure Payload PM state information e.g. SP is suspended, uninitialised etc
 ******************************************************************************/
#define TBASE_STATE_OFF		0
#define TBASE_STATE_ON		1
#define TBASE_STATE_SUSPEND	2

/*******************************************************************************
 * Secure Payload execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define TBASE_AARCH32		MODE_RW_32
#define TBASE_AARCH64		MODE_RW_64

/*******************************************************************************
 * The SPD should know the type of Secure Payload.
 ******************************************************************************/
#define TBASE_TYPE_UP		PSCI_TOS_NOT_UP_MIG_CAP
#define TBASE_TYPE_UPM		PSCI_TOS_UP_MIG_CAP
#define TBASE_TYPE_MP		PSCI_TOS_NOT_PRESENT_MP

/*******************************************************************************
 * Secure Payload migrate type information as known to the SPD. We assume that
 * the SPD is dealing with an MP Secure Payload.
 ******************************************************************************/
#define TBASE_MIGRATE_INFO		TBASE_TYPE_MP

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define TBASE_CORE_COUNT		PLATFORM_CORE_COUNT

/*******************************************************************************
 * Constants used to set and clear flags (or features) in tbase
 ******************************************************************************/
#define TBASE_FLAG_SET                  1
#define TBASE_FLAG_CLEAR                0

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define TSPD_C_RT_CTX_X19		0x0
#define TSPD_C_RT_CTX_X20		0x8
#define TSPD_C_RT_CTX_X21		0x10
#define TSPD_C_RT_CTX_X22		0x18
#define TSPD_C_RT_CTX_X23		0x20
#define TSPD_C_RT_CTX_X24		0x28
#define TSPD_C_RT_CTX_X25		0x30
#define TSPD_C_RT_CTX_X26		0x38
#define TSPD_C_RT_CTX_X27		0x40
#define TSPD_C_RT_CTX_X28		0x48
#define TSPD_C_RT_CTX_X29		0x50
#define TSPD_C_RT_CTX_X30		0x58
#define TSPD_C_RT_CTX_SIZE		0x60
#define TSPD_C_RT_CTX_ENTRIES		(TSPD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, TSPD_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(TSPD_C_RT_CTX_SIZE == sizeof(c_rt_regs_t),	\
	assert_spd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'state'    - collection of flags to track SP state e.g. on/off
 * 'mpidr'    - mpidr to associate a context with a cpu
 * 'c_rt_ctx' - stack address to restore C runtime context from after returning
 *              from a synchronous entry into the SP.
 * 'cpu_ctx'  - space to maintain SP architectural state
 * 'monitorCallRegs' - area for monitor to tbase call parameter passing.
 ******************************************************************************/

typedef struct {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	uint64_t monitorCallRegs[TBASE_MAX_MONITOR_CALL_REGS];
        uint64_t tbase_input_fastcall[4];
} tbase_context;

typedef struct {
    uint32_t magic;        // magic value from information 
    uint32_t length;       // size of struct in bytes.
    uint64_t version;      // Version of structure
    uint64_t dRamBase;     // NonSecure DRAM start address
    uint64_t dRamSize;     // NonSecure DRAM size
    uint64_t secDRamBase;  // Secure DRAM start address
    uint64_t secDRamSize;  // Secure DRAM size
    uint64_t secIRamBase;  // Secure IRAM base
    uint64_t secIRamSize;  // Secure IRam size
    uint64_t conf_mair_el3;// MAIR_EL3 for memory attributes sharing
    uint32_t RFU1;
    uint32_t MSMPteCount;  // Number of MMU entries for MSM
    uint64_t MSMBase;      // MMU entries for MSM
    uint64_t gic_distributor_base;
    uint64_t gic_cpuinterface_base;
    uint32_t gic_version;
    uint32_t total_number_spi;
    uint32_t ssiq_number;
    uint32_t RFU2;
    uint64_t flags;
} bootCfg_t;


 // Magic for interface
#define TBASE_BOOTCFG_MAGIC (0x434d4254) // String TBMC in little-endian

// fastcall origin IDs for tbase
#define TBASE_SMC_NWD 0
#define TBASE_SMC_MONITOR 2

// tbase entrypoint offsets
#define ENTRY_OFFSET_FASTCALL 0x2C
#define ENTRY_OFFSET_FIQ 0x28
#define ENTRY_OFFSET_SMC 0x24

/*******************************************************************************
 * Tbase specific SMC ids 
 ******************************************************************************/

// Fastcall ids
#define TBASE_SMC_FASTCALL_RETURN               (0xB2000001)

#define TBASE_SMC_FASTCALL_CONFIG_OK            (0xB2000002)
#define TBASE_SMC_FASTCALL_CONFIG_VECTOR        1
#define TBASE_SMC_FASTCALL_CONFIG_DEBUG         2

#define TBASE_SMC_FASTCALL_OUTPUT               (0xB2000003)
#define TBASE_SMC_FASTCALL_OUTPUT_PUTC          1

#define TBASE_SMC_FASTCALL_STATUS               (0xB2000004)
#define TBASE_SMC_FASTCALL_STATUS_EXECUTION     1
#define TBASE_STATUS_NORMAL_BIT                 0x01
#define TBASE_STATUS_FASTCALL_OK_BIT            0x02
#define TBASE_STATUS_SMC_OK_BIT                 0x04

#define TBASE_STATUS_UNINIT                     0x00
#define TBASE_STATUS_NORMAL                     (TBASE_STATUS_NORMAL_BIT|TBASE_STATUS_FASTCALL_OK_BIT|TBASE_STATUS_SMC_OK_BIT)

#define TBASE_SMC_FASTCALL_INPUT                (0xB2000005)
#define TBASE_INPUT_HWIDENTITY                  0x01
#define TBASE_INPUT_HWKEY                       0x02
#define TBASE_INPUT_RNG                         0x03

#define TBASE_SMC_FASTCALL_DUMP		        (0xB2000006)

#define TBASE_SMC_FASTCALL_FORWARD_FIQ	        (0xB2000007)
#define TBASE_SMC_FASTCALL_FIQFORWARD_CONFIG	(0xFF000038)


#define TBASE_SMC_AEE_DUMP			(0xB200AEED)

// MSM area definition
// This structure is SPD owned area mapped as part of MSM
struct msm_area_t {
  tbase_context secure_context[TBASE_CORE_COUNT];
};

extern struct msm_area_t msm_area;

// Context for each core. gp registers not used by SPD.
extern tbase_context *secure_context;

/* tbase power management handlers */
extern const spd_pm_ops_t tbase_pm;

// ************************************************************************************

// secure_context for secodary cores are initialized 
// when primary core returns from initialization call.
#define TBASE_INIT_NONE 0
#define TBASE_INIT_CONFIG_OK 1
#define TBASE_INIT_SYSREGS_OK 2

// Current status of initialization
extern uint64_t tbaseInitStatus;

// Current status of execution
extern uint64_t tbaseExecutionStatus;

// Entry vector start address in tbase
extern uint64_t tbaseEntryBase;
// Tbase SPSR for SMC and FIQ handling.
extern uint32_t tbaseEntrySpsr;



// ************************************************************************************
// Shared tbase monitor memory
// ************************************************************************************

// Page aligned start addresses for memory windows between tbase and 
#define REGISTER_FILE_COUNT 2
#define REGISTER_FILE_NWD 0
#define REGISTER_FILE_MONITOR 1

extern uint64_t registerFileStart[REGISTER_FILE_COUNT];
extern uint64_t registerFileEnd[REGISTER_FILE_COUNT];


/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
extern void tbase_setup_entry( cpu_context_t *ns_context, uint32_t call_offset, uint32_t regfileNro);
extern uint64_t tbase_fiq_handler(uint32_t id, uint32_t flags, void *handle, void *cookie);


extern uint64_t tbase_enter_sp(uint64_t *c_rt_ctx);
extern void __dead2 tbase_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
extern uint64_t tbase_synchronous_sp_entry(tbase_context *tsp_ctx);
extern void __dead2 tbase_synchronous_sp_exit(tbase_context *tbase_ctx, uint64_t ret, uint32_t save_sysregs);

extern uint64_t maskSWdRegister(uint64_t x);

extern int32_t tbase_fastcall_setup(void);
extern void save_sysregs_allcore();
extern void tbase_init_core(uint64_t mpidr);
extern void configure_tbase(uint64_t x1, uint64_t x2);

extern void tbase_CoreDump(uint32_t coreId);
extern void tbase_PlatformDump(void);

extern void tbase_fiqforward_configure(uint32_t intrNo, uint32_t enable);

#if DEBUG
  #define DBG_PRINTF(...) printf(__VA_ARGS__)
#else 
  #define DBG_PRINTF(...)
#endif

#endif /*__ASSEMBLY__*/

#endif /* __TBASE_PRIVATE_H__ */
