#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <string.h>
#include "tkcored_private.h"

int32_t tkcored_init_secure_context(uint64_t entrypoint,
				    uint32_t rw,
				    uint64_t mpidr,
				    struct tkcore_context *tkcore_ctx)
{
	uint32_t scr, sctlr;
	el1_sys_regs_t *el1_state;
	el3_state_t *state;
	uint32_t spsr;

	/* Passing a NULL context is a critical programming error */
	assert(tkcore_ctx);

	/*
	 * This might look redundant if the context was statically
	 * allocated but this function cannot make that assumption.
	 */
	memset(tkcore_ctx, 0, sizeof(*tkcore_ctx));

	/*
	 * Set the right security state, register width and enable access to
	 * the secure physical timer for TKCORE.
	 */
	scr = read_scr();
	scr &= ~SCR_NS_BIT;
	scr &= ~SCR_RW_BIT;
	scr |= SCR_ST_BIT;
	if (rw == TKCORE_AARCH64)
		scr |= SCR_RW_BIT;

	/* Get a pointer to the S-EL1 context memory */
	el1_state = get_sysregs_ctx(&tkcore_ctx->cpu_ctx);

	/*
	 * Program the SCTLR_EL1 such that upon entry in S-EL1, caches and
	 * MMU are disabled and exception endianness is set to be the same
	 * as EL3
	 */
	sctlr = read_sctlr_el3();
	sctlr &= SCTLR_EE_BIT;
	sctlr |= SCTLR_EL1_RES1;
	write_ctx_reg(el1_state, CTX_SCTLR_EL1, sctlr);

	/* Set this context as ready to be initialised i.e OFF */
	set_tkcore_pstate(tkcore_ctx->state, TKCORE_PSTATE_OFF);

	/* Associate this context with the cpu specified */
	tkcore_ctx->mpidr = mpidr;

	cm_set_context_by_mpidr(mpidr, &tkcore_ctx->cpu_ctx, SECURE);
	if (rw == TKCORE_AARCH64)
		spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	else
		spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
				   DAIF_FIQ_BIT | DAIF_IRQ_BIT);

	scr &= ~SCR_FIQ_BIT;
	scr &= ~SCR_IRQ_BIT;
	scr |= get_scr_el3_from_routing_model(SECURE);
	state = get_el3state_ctx(&tkcore_ctx->cpu_ctx);
	write_ctx_reg(state, CTX_SCR_EL3, scr);

	cm_set_elr_spsr_el3(SECURE, entrypoint, spsr);

	return 0;
}

uint64_t tkcored_synchronous_sp_entry(struct tkcore_context *tkcore_ctx)
{
	uint64_t rc;

	assert(tkcore_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context_by_mpidr(read_mpidr(), SECURE)
		== &tkcore_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = tkcored_enter_sp(&tkcore_ctx->c_rt_ctx);
	tkcore_ctx->c_rt_ctx = 0;

	return rc;
}


void tkcored_synchronous_sp_exit(struct tkcore_context *tkcore_ctx,
								 uint64_t ret)
{
	/* Save the Secure EL1 system register context */
	assert(cm_get_context_by_mpidr(read_mpidr(), SECURE)
		== &tkcore_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(tkcore_ctx->c_rt_ctx != 0);
	tkcored_exit_sp(tkcore_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}
