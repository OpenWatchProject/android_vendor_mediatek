
#ifndef __TKCORED_PRIVATE_H__
#define __TKCORED_PRIVATE_H__

#include <arch.h>
#include <context.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>
#include <psci.h>

/*******************************************************************************
 * TKCORE PM state information e.g. TKCORE is suspended, uninitialised etc
 * and macros to access the state information in the per-cpu 'state' flags
 ******************************************************************************/
#define TKCORE_PSTATE_OFF		0
#define TKCORE_PSTATE_ON			1
#define TKCORE_PSTATE_SUSPEND	2
#define TKCORE_PSTATE_ON_PENDING	4

#define TKCORE_PSTATE_SHIFT		0
#define TKCORE_PSTATE_MASK		0x7

#define get_tkcore_pstate(state) \
	((state >> TKCORE_PSTATE_SHIFT) & TKCORE_PSTATE_MASK)

#define clr_tkcore_pstate(state) \
	(state &= ~(TKCORE_PSTATE_MASK << TKCORE_PSTATE_SHIFT))

#define set_tkcore_pstate(st, pst) do { \
		clr_tkcore_pstate(st); \
		st |= (pst & TKCORE_PSTATE_MASK) << \
		TKCORE_PSTATE_SHIFT; \
	} while (0)


#define TKCORE_AARCH32		MODE_RW_32
#define TKCORE_AARCH64		MODE_RW_64

#define TKCORE_TYPE_UP		PSCI_TOS_NOT_UP_MIG_CAP
#define TKCORE_TYPE_UPM		PSCI_TOS_UP_MIG_CAP
#define TKCORE_TYPE_MP		PSCI_TOS_NOT_PRESENT_MP

#define TKCORE_MIGRATE_INFO		TKCORE_TYPE_MP

#define TKCORED_CORE_COUNT		PLATFORM_CORE_COUNT

#define TKCORED_C_RT_CTX_X19		0x0
#define TKCORED_C_RT_CTX_X20		0x8
#define TKCORED_C_RT_CTX_X21		0x10
#define TKCORED_C_RT_CTX_X22		0x18
#define TKCORED_C_RT_CTX_X23		0x20
#define TKCORED_C_RT_CTX_X24		0x28
#define TKCORED_C_RT_CTX_X25		0x30
#define TKCORED_C_RT_CTX_X26		0x38
#define TKCORED_C_RT_CTX_X27		0x40
#define TKCORED_C_RT_CTX_X28		0x48
#define TKCORED_C_RT_CTX_X29		0x50
#define TKCORED_C_RT_CTX_X30		0x58
#define TKCORED_C_RT_CTX_SCR		0x60
#define TKCORED_C_RT_CTX_SIZE		0x70
#define TKCORED_C_RT_CTX_ENTRIES \
	(TKCORED_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

#define TKCORE_NUM_ARGS	0x2

DEFINE_REG_STRUCT(c_rt_regs, TKCORED_C_RT_CTX_ENTRIES);

CASSERT(sizeof(c_rt_regs_t) == TKCORED_C_RT_CTX_SIZE,
	assert_spd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the TKCORED to maintain the per-cpu state of TKCORE.
 * 'state'          - collection of flags to track TKCORE state e.g. on/off
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into TKCORE.
 * 'cpu_ctx'        - space to maintain TKCORE architectural state
 ******************************************************************************/
struct tkcore_context {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
};

/* TKCORED power management handlers */
extern const spd_pm_ops_t tkcored_pm;

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct tkcore_vectors;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t tkcored_enter_sp(uint64_t *c_rt_ctx);
void __dead2 tkcored_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t tkcored_synchronous_sp_entry(struct tkcore_context *tkcore_ctx);
void __dead2 tkcored_synchronous_sp_exit(struct tkcore_context *tkcore_ctx,
		uint64_t ret);
int32_t tkcored_init_secure_context(uint64_t entrypoint,
				    uint32_t rw,
				    uint64_t mpidr,
				    struct tkcore_context *tkcore_ctx);
extern struct tkcore_context tkcored_sp_context[TKCORED_CORE_COUNT];
extern uint32_t tkcored_rw;
extern struct tkcore_vectors *tkcore_vectors;

#if DEBUG
#define DBG_PRINTF(...) tf_printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

#endif /*__ASSEMBLY__*/

#endif /* __TKCORED_PRIVATE_H__ */
