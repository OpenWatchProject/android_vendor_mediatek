#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include <console.h>
#include <tkcore.h>
#include <uuid.h>
#include "plat_private.h"
#include "tkcored_private.h"
#include "tkcore.h"
#include "teesmc_tkcored.h"

#if (defined(MACH_TYPE_MT6735) || defined(MACH_TYPE_MT6735M) || \
	defined(MACH_TYPE_MT6753) || defined(MACH_TYPE_MT8173))
#define TKCORE_BOOT_ARG_ADDR	(((struct atf_arg_t *)\
	(uintptr_t) TEE_BOOT_INFO_ADDR)->tee_boot_arg_addr)
#else
#define TKCORE_BOOT_ARG_ADDR	(((struct atf_arg_t *)\
	(uintptr_t) (&gteearg))->tee_boot_arg_addr)
#endif

struct tkcore_vectors *tkcore_vectors;

struct tkcore_context tkcored_sp_context[TKCORED_CORE_COUNT];
uint32_t tkcored_rw;

static int32_t tkcored_init(void);

static uint64_t tkcored_sel1_interrupt_handler(uint32_t id,
		uint32_t flags,
		void *handle,
		void *cookie)
{
	unsigned int linear_id;
	struct tkcore_context *tkcore_ctx;

	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* Save the non-secure context before entering the TKCORE */
	cm_el1_sysregs_context_save(NON_SECURE);

	linear_id = plat_my_core_pos();
	/* Get a reference to this cpu's TKCORE context */
	tkcore_ctx = &tkcored_sp_context[linear_id];
	assert(&tkcore_ctx->cpu_ctx == cm_get_context(SECURE));

	SMC_SET_EL3(&tkcore_ctx->cpu_ctx,
		    CTX_ELR_EL3, (uint64_t) &tkcore_vectors->fiq_entry);

	cm_el1_sysregs_context_restore(SECURE);

	cm_set_next_eret_context(SECURE);

	SMC_RET2(&tkcore_ctx->cpu_ctx,
		TKCORE_HANDLE_FIQ_AND_RETURN, read_elr_el3());
	return 0;
}

void tkcored_init_core(uint64_t mpidr)
{
	(void) mpidr;
}

int32_t tkcored_setup(void)
{
	entry_point_info_t *image_info;
	int32_t rc;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id;

	linear_id = platform_get_core_pos(mpidr);

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.  TODO: Add support to
	 * conditionally include the SPD service
	 */
	image_info = bl31_plat_get_next_image_ep_info(SECURE);
	assert(image_info);

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!image_info->pc)
		return 1;

	/*
	 * We could inspect the SP image and determine it's execution
	 * state i.e whether AArch32 or AArch64. Assuming it's AArch32
	 * for the time being.
	 */
	tkcored_rw = TKCORE_AARCH32;
	rc = tkcored_init_secure_context(image_info->pc, tkcored_rw,
					 mpidr, &tkcored_sp_context[linear_id]);
	assert(rc == 0);

	/*
	 * All TKCORED initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&tkcored_init);

	return rc;
}

static int32_t tkcored_init(void)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr), flags;
	uint64_t rc;
	struct tkcore_context *tkcore_ctx = &tkcored_sp_context[linear_id];

	/* Save el1 registers in case non-secure world
	 * has already been set up.
	 */
	cm_el1_sysregs_context_save(NON_SECURE);

	/*
	 * Arrange for an entry into the test secure payload. We expect an array
	 * of vectors in return
	 */

	write_ctx_reg(get_gpregs_ctx(&(tkcore_ctx->cpu_ctx)), CTX_GPREG_X0,
		      (int64_t) TKCORE_BOOT_ARG_ADDR);
	tf_printf("TKCore Entry %016lx\n",
		read_ctx_reg(get_el3state_ctx(&(tkcore_ctx->cpu_ctx)),
		CTX_ELR_EL3));
	rc = tkcored_synchronous_sp_entry(tkcore_ctx);
	tf_printf("TKCore Exit\n");
	assert(rc != 0);
	if (rc) {
		set_tkcore_pstate(tkcore_ctx->state, TKCORE_PSTATE_ON);

		/*
		 * TKCORE has been successfully initialized. Register power
		 * managemnt hooks with PSCI
		 */
		psci_register_spd_pm_hook(&tkcored_pm);
	}

	/*
	 * Register an interrupt handler for S-EL1 interrupts when generated
	 * during code executing in the non-secure state.
	 */
	flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
		tkcored_sel1_interrupt_handler, flags);
	if (rc)
		panic();

	cm_el1_sysregs_context_restore(NON_SECURE);

	return rc;
}

uint64_t tkcored_smc_handler(uint32_t smc_fid,
			     uint64_t x1,
			     uint64_t x2,
			     uint64_t x3,
			     uint64_t x4,
			     void *cookie,
			     void *handle,
			     uint64_t flags)
{
	cpu_context_t *ns_cpu_context;
	unsigned long mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tkcore_context *tkcore_ctx = &tkcored_sp_context[linear_id];

	/*
	 * Determine which security state this SMC originated from
	 */

	if (is_caller_non_secure(flags)) {
		assert(handle ==
			cm_get_context_by_index(linear_id, NON_SECURE));

		cm_el1_sysregs_context_save(NON_SECURE);

		/*
		 * Verify if there is a valid context to use, copy the
		 * operation type and parameters to the secure context
		 * and jump to the fast smc entry point in the secure
		 * payload. Entry into S-EL1 will take place upon exit
		 * from this function.
		 */
		assert(&tkcore_ctx->cpu_ctx ==
		       cm_get_context_by_index(linear_id, SECURE));

		/* Set appropriate entry for SMC.
		 * We expect TKCORE to manage the PSTATE.I and PSTATE.F
		 * flags as appropriate.
		 */
		if (GET_SMC_TYPE(smc_fid) == SMC_TYPE_FAST) {
			cm_set_elr_el3(SECURE, (uint64_t)
				       &tkcore_vectors->fast_smc_entry);
		} else {
			cm_set_elr_el3(SECURE, (uint64_t)
				       &tkcore_vectors->std_smc_entry);
		}

		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);

		/* Propagate hypervisor client ID */
		write_ctx_reg(get_gpregs_ctx(&tkcore_ctx->cpu_ctx),
			      CTX_GPREG_X7, read_ctx_reg(get_gpregs_ctx(handle),
					      CTX_GPREG_X7));

		SMC_RET4(&tkcore_ctx->cpu_ctx, smc_fid, x1, x2, x3);
	}

	/*
	 * Returning from TKCORE
	 */

	switch (smc_fid) {
	/*
	 * TKCORE has finished initialising itself after a cold boot
	 */
	case TEESMC32_TKCORED_RETURN_ENTRY_DONE:
	case TEESMC64_TKCORED_RETURN_ENTRY_DONE:
		/*
		* Stash the TKCORE entry points information. This is done
		* only once on the primary cpu
		*/
		assert(tkcore_vectors == NULL);
		tkcore_vectors = (struct tkcore_vectors *) x1;

		/*
		 * TKCORE reports completion. The TKCORED must have initiated
		 * the original request through a synchronous entry into
		 * the SP. Jump back to the original C runtime context.
		 */
		tkcored_synchronous_sp_exit(tkcore_ctx, x1);
		break;
	/*
	 * TKCORE has finished turning itself on in response to an earlier
	 * psci cpu_on request
	 */
	case TEESMC32_TKCORED_RETURN_ON_DONE:
	case TEESMC64_TKCORED_RETURN_ON_DONE:

	/*
	 * TKCORE has finished turning itself off in response to an earlier
	 * psci cpu_off request.
	 */
	case TEESMC32_TKCORED_RETURN_OFF_DONE:
	case TEESMC64_TKCORED_RETURN_OFF_DONE:
	/*
	 * TKCORE has finished resuming itself after an earlier psci
	 * cpu_suspend request.
	 */
	case TEESMC32_TKCORED_RETURN_RESUME_DONE:
	case TEESMC64_TKCORED_RETURN_RESUME_DONE:
	/*
	 * TKCORE has finished suspending itself after an earlier psci
	 * cpu_suspend request.
	 */
	case TEESMC32_TKCORED_RETURN_SUSPEND_DONE:
	case TEESMC64_TKCORED_RETURN_SUSPEND_DONE:

		/*
		 * TKCORE reports completion. The TKCORED must have
		 * initiated the original request through a synchronous
		 * entry into TKCORE. Jump back to the original C runtime
		 * context, and pass x1 as return value to the caller
		 */
		tkcored_synchronous_sp_exit(tkcore_ctx, x1);
		break;

	/*
	 * TKCORE is returning from a call or being preemted from a call, in
	 * either case execution should resume in the normal world.
	 */
	case TEESMC32_TKCORED_RETURN_CALL_DONE:
	case TEESMC64_TKCORED_RETURN_CALL_DONE:
		/*
		 * This is the result from the secure client of an
		 * earlier request. The results are in x0-x3. Copy it
		 * into the non-secure context, save the secure state
		 * and return to the non-secure state.
		 */
		assert(handle == cm_get_context_by_index(linear_id, SECURE));

		cm_el1_sysregs_context_save(SECURE);

		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context_by_index(linear_id, NON_SECURE);
		assert(ns_cpu_context);

		/* Restore non-secure state */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);

		x1 &= ((1UL << 32) - 1);
		x2 &= ((1UL << 32) - 1);
		x3 &= ((1UL << 32) - 1);
		x4 &= ((1UL << 32) - 1);

		SMC_RET4(ns_cpu_context, x1, x2, x3, x4);
		break;

	/*
	 * TKCORE has finished handling a S-EL1 FIQ interrupt. Execution
	 * should resume in the normal world.
	 */
	case TEESMC32_TKCORED_RETURN_FIQ_DONE:
	case TEESMC64_TKCORED_RETURN_FIQ_DONE:
		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context_by_index(linear_id, NON_SECURE);
		assert(ns_cpu_context);

		/*
		 * Restore non-secure state. There is no need to save the
		 * secure system register context since TKCORE was supposed
		 * to preserve it during S-EL1 interrupt handling.
		 */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);

		SMC_RET0((uint64_t) ns_cpu_context);
		break;

	case TEESMC32_TKCORED_SECURE_SERVICE_REQUEST:
	case TEESMC64_TKCORED_SECURE_SERVICE_REQUEST:

		smc_fid = plat_tkcore_secure_service_request(x1, &x2, &x3);

		SMC_RET4(&tkcore_ctx->cpu_ctx, smc_fid, x1, x2, x3);
		break;

	default:
		assert(0);
	}
}

/* Define an TKCORED runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tkcored_fast,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	tkcored_setup,
	tkcored_smc_handler
);

/* Define an TKCORED runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	tkcored_std,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_STD,
	NULL,
	tkcored_smc_handler
);
