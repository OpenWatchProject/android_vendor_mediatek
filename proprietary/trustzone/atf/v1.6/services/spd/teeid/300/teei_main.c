/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include "teei_private.h"
#include <plat_teei.h>
#include "utos_version.h"
#include "teei_atf.h"
#include "platform_def_teei.h"

/* SPSR for teei setup */
#define ISEE_FLAGS_SOTER_FEATURE_AARCH64 0x10
#define ISEE_FLAGS_SOTER_FEATURE_AARCH64_INDEX 1
#define TEEI_INIT_SPSR_AARCH32 0x1d3
#define TEEI_INIT_SPSR_AARCH64 0x0c5
#define NWD_INIT_SPSR 0x1d3

#define TEEI_ENTRY_SPSR 0x1d1

/* Initialization parameters for teei. */
static struct tee_arg_t *teeiBootCfg;
uint64_t teeiBootCoreMpidr;

static uint32_t teei_is_aarch64(void)
{
	/* flags not aligned to unsigned long long, so access it by unsigned char. */
	unsigned char *flag_ptr = (unsigned char *)(&(teeiBootCfg->flags));
	unsigned char *feature_ptr = flag_ptr + ISEE_FLAGS_SOTER_FEATURE_AARCH64_INDEX;
	unsigned char feature = *feature_ptr;

	return feature & ISEE_FLAGS_SOTER_FEATURE_AARCH64;
}

/******************************************************************************
 * Mask 32 bit software registers
 *******************************************************************************/
uint32_t maskSWdRegister(uint64_t x)
{
	return (uint32_t)x;
}

/*****************************************************************************
 * Initilzation a secure context for teei(secure OS)
 ******************************************************************************/
int32_t teei_init_secure_context(struct teei_context *teei_ctx)
{
	uint32_t sctlr = read_sctlr_el3();
	el1_sys_regs_t *el1_state;
	uint64_t mpidr = read_mpidr();

	assert(teei_ctx);
	memset(teei_ctx, 0, sizeof(*teei_ctx));
	el1_state = get_sysregs_ctx(&teei_ctx->cpu_ctx);

	/* Program the sctlr for S-EL1 execution with caches and mmu off.*/
	sctlr &= SCTLR_EE_BIT;
	sctlr |= SCTLR_EL1_RES1;
	write_ctx_reg(el1_state, CTX_SCTLR_EL1, sctlr);

	/* Setting SPSR for S-EL1 execution. */
	if (teei_is_aarch64())
		write_ctx_reg(el1_state, CTX_SPSR_EL1, TEEI_INIT_SPSR_AARCH64);
	else
		write_ctx_reg(el1_state, CTX_SPSR_EL1, TEEI_INIT_SPSR_AARCH32);

	write_ctx_reg(el1_state, CTX_ELR_EL1, teeiBootCfg->secDRamBase);

	/* Set up context for this core */
	teei_ctx->mpidr = mpidr;
	cm_set_context_by_mpidr(mpidr, &teei_ctx->cpu_ctx, SECURE);

	return 0;
}
/*******************************************************************************
 * This function populates 'cpu_context' pertaining to the given security state
 * with the entrypoint, SPSR and SCR values so that an ERET from this security
 * state correctly restores corresponding values to drop the CPU to the next
 * exception level
 ******************************************************************************/
static void cm_set_el3_eret_context(uint32_t security_state, uint64_t entrypoint,
		uint32_t spsr, uint32_t scr)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = CM_GET_CONTEXT(security_state);
	assert(ctx);

	/* Program the interrupt routing model for this security state */
	scr &= ~SCR_FIQ_BIT;
	scr &= ~SCR_IRQ_BIT;
	scr |= get_scr_el3_from_routing_model(security_state);

	/* Populate EL3 state so that we've the right context before doing ERET */
	state = get_el3state_ctx(ctx);
	write_ctx_reg(state, CTX_SPSR_EL3, spsr);
	write_ctx_reg(state, CTX_ELR_EL3, entrypoint);
	write_ctx_reg(state, CTX_SCR_EL3, scr);
}


/*******************************************************************************
 * Configure teei and EL3 registers for initial entry
 *******************************************************************************/
static void teei_init_eret(uint64_t entrypoint, uint32_t rw)
{
	uint32_t scr = read_scr();
	uint32_t spsr = 0;

	assert(rw == TEEI_AARCH32 || rw == TEEI_AARCH64);

	/* Set the right security state and register width for the SP*/
	scr &= ~SCR_NS_BIT;

	switch (rw) {
	case TEEI_AARCH32:
		spsr = TEEI_INIT_SPSR_AARCH32;
		scr &= ~SCR_RW_BIT;
		break;
	case TEEI_AARCH64:
		spsr = TEEI_INIT_SPSR_AARCH64;
		scr |= SCR_RW_BIT;
		break;
	}

	/* Also IRQ and FIQ handled in secure state*/
	scr &= ~(SCR_FIQ_BIT|SCR_IRQ_BIT);
	/* No execution from Non-secure memory*/
	scr |= SCR_SIF_BIT;

	cm_set_el3_eret_context(SECURE, entrypoint, spsr, scr);

	entry_point_info_t *image_info = bl31_plat_get_next_image_ep_info(SECURE);

	assert(image_info);

	image_info->spsr = spsr;

}


/*************************************************************************************
 * Initialize teei system for first entry to teei
 * This and initial entry should be done only once in cold boot.
 ************************************************************************************/
static int32_t teei_init_entry(void)
{

	DBG_PRINTF("[ATF--uTos] version [%s]\n\r", UTOS_VERSION);

	/* Save el1 registers in case non-secure world has already been set up.*/
	cm_el1_sysregs_context_save(NON_SECURE);

	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct teei_context *teei_ctx = &secure_context[linear_id];

	/* Set registers for teei initialization entry */
	cpu_context_t *s_entry_context = &teei_ctx->cpu_ctx;
	gp_regs_t *s_entry_gpregs = get_gpregs_ctx(s_entry_context);

	write_ctx_reg(s_entry_gpregs, CTX_GPREG_X1, 0);
	write_ctx_reg(s_entry_gpregs, CTX_GPREG_X1, (int64_t)TEEI_BOOT_PARAMS);

	/* Start teei */
	DBG_PRINTF("  mtee ST [%d]\n\r", linear_id);
	teei_synchronous_sp_entry(teei_ctx);

#ifdef TEEI_PM_ENABLE
	/* Register power managemnt hooks with PSCI */
	psci_register_spd_pm_hook(&teei_pm);
#endif
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	return 1;
}

/************************************************************************************
 * Setup teei SPD
 ************************************************************************************/
int32_t teei_fastcall_setup(void)
{

	entry_point_info_t *image_info;

	image_info = bl31_plat_get_next_image_ep_info(SECURE);

	assert(image_info);

	uint32_t linear_id = platform_get_core_pos(read_mpidr());

	teeiBootCoreMpidr = read_mpidr();

	teeiBootCfg = (struct tee_arg_t *)(uintptr_t)TEEI_BOOT_PARAMS;
	teei_gic_setup();
	teei_init_secure_context(&secure_context[linear_id]);

	if (teei_is_aarch64())
		teei_init_eret(image_info->pc, TEEI_AARCH64);
	else
		teei_init_eret(image_info->pc, TEEI_AARCH32);

	bl31_register_bl32_init(&teei_init_entry);

	return 0;
}



