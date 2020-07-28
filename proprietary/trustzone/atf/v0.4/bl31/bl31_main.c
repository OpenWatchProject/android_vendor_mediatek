/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stdio.h>
#include <string.h>
#include <log.h>
#include <console.h>    //for disable uart log when leaving ATF booting flow

/*******************************************************************************
 * This function pointer is used to initialise the BL32 image. It's initialized
 * by SPD calling bl31_register_bl32_init after setting up all things necessary
 * for SP execution. In cases where both SPD and SP are absent, or when SPD
 * finds it impossible to execute SP, this pointer is left as NULL
 ******************************************************************************/
static int32_t (*bl32_init)(unsigned int, unsigned int);

/*******************************************************************************
 * Variable to indicate whether next image to execute after BL31 is BL33
 * (non-secure & default) or BL32 (secure).
 ******************************************************************************/
static uint32_t next_image_type;

/*******************************************************************************
 * Simple function to initialise all BL31 helper libraries.
 ******************************************************************************/
void bl31_lib_init()
{
	cm_init();
}

/*******************************************************************************
 * BL31 is responsible for setting up the runtime services for the primary cpu
 * before passing control to the bootloader or an Operating System. This
 * function calls runtime_svc_init() which initializes all registered runtime
 * services. The run time services would setup enough context for the core to
 * swtich to the next exception level. When this function returns, the core will
 * switch to the programmed exception level via. an ERET.
 ******************************************************************************/
void bl31_main(void)
{
#if DEBUG
	unsigned long mpidr = read_mpidr();
#endif
	atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;
	 
	if(teearg->atf_log_buf_size != 0)
	{
	    teearg->atf_aee_debug_buf_size = ATF_AEE_BUFFER_SIZE;
        teearg->atf_aee_debug_buf_start = teearg->atf_log_buf_start + teearg->atf_log_buf_size - ATF_AEE_BUFFER_SIZE;
		mt_log_setup(teearg->atf_log_buf_start, teearg->atf_log_buf_size, teearg->atf_aee_debug_buf_size);
		printf("ATF log service is registered (0x%x, aee:0x%x)\n", teearg->atf_log_buf_start, teearg->atf_aee_debug_buf_start);
	}
	else
	{
		teearg->atf_aee_debug_buf_size = 0;
		teearg->atf_aee_debug_buf_start = 0;        
	}    

	/* Perform remaining generic architectural setup from EL3 */
	bl31_arch_setup();

	/* Perform platform setup in BL1 */
	bl31_platform_setup();

	printf("BL31 %s\n\r", build_message);

	/* Initialise helper libraries */
	bl31_lib_init();

	/* Initialize the runtime services e.g. psci */
	runtime_svc_init();

	/* Clean caches before re-entering normal world */
	dcsw_op_all(DCCSW);

	/*
	 * Use the more complex exception vectors now that context
	 * management is setup. SP_EL3 should point to a 'cpu_context'
	 * structure which has an exception stack allocated.  The PSCI
	 * service should have set the context.
	 */
	assert(cm_get_context(mpidr, NON_SECURE));
	cm_set_next_eret_context(NON_SECURE);
	cm_init_pcpu_ptr_cache();
	write_vbar_el3((uint64_t) runtime_exceptions);
	isb();
	next_image_type = NON_SECURE;

	/*
	 * All the cold boot actions on the primary cpu are done. We now need to
	 * decide which is the next image (BL32 or BL33) and how to execute it.
	 * If the SPD runtime service is present, it would want to pass control
	 * to BL32 first in S-EL1. In that case, SPD would have registered a
	 * function to intialize bl32 where it takes responsibility of entering
	 * S-EL1 and returning control back to bl31_main. Once this is done we
	 * can prepare entry into BL33 as normal.
	 */

	/*
	 * If SPD had registerd an init hook, invoke it.
	 */
    if(teearg->tee_support)
    {
        printf("[BL31] Jump to secure OS for initialization!\n\r");
        if (bl32_init)
        {
            (*bl32_init)(teearg->tee_entry, teearg->tee_boot_arg_addr);
        }
        else
        {
            printf("[ERROR] Secure OS is not initialized!\n\r");
            //assert(0);            
        }
	}
    else
    {
        printf("[BL31] Jump to FIQD for initialization!\n\r");
        if (bl32_init)
        {
            (*bl32_init)(0, 0);
        }
    }

	/*
	 * We are ready to enter the next EL. Prepare entry into the image
	 * corresponding to the desired security state after the next ERET.
	 */
	bl31_prepare_next_image_entry();
    
    printf("[BL31] Final dump!\n\r");

	clear_uart_flag();
    
    printf("[BL31] SHOULD not dump in UART but in log buffer!\n\r");
}

/*******************************************************************************
 * Accessor functions to help runtime services decide which image should be
 * executed after BL31. This is BL33 or the non-secure bootloader image by
 * default but the Secure payload dispatcher could override this by requesting
 * an entry into BL32 (Secure payload) first. If it does so then it should use
 * the same API to program an entry into BL33 once BL32 initialisation is
 * complete.
 ******************************************************************************/
void bl31_set_next_image_type(uint32_t security_state)
{
	assert(security_state == NON_SECURE || security_state == SECURE);
	next_image_type = security_state;
}

uint32_t bl31_get_next_image_type(void)
{
	return next_image_type;
}

extern entry_point_info_t *bl31_plat_get_next_kernel_ep_info(uint32_t type);

void bl31_prepare_k64_entry(void)
{
	entry_point_info_t *next_image_info;
	uint32_t scr, image_type;
	cpu_context_t *ctx;
	gp_regs_t *gp_regs;

	/* Determine which image to execute next */
	image_type = NON_SECURE; //bl31_get_next_image_type();

	/*
	 * Setup minimal architectural state of the next highest EL to
	 * allow execution in it immediately upon entering it.
	 */
	bl31_next_el_arch_setup(image_type);

	/* Program EL3 registers to enable entry into the next EL */
	next_image_info = bl31_plat_get_next_kernel_ep_info(image_type);


	assert(next_image_info);
	assert(image_type == GET_SECURITY_STATE(next_image_info->h.attr));


    /* check is set 64bit kernel*/
    printf("next_image_info->spsr = 0x%llx\n", next_image_info->spsr);

	scr = read_scr();
	scr &= ~SCR_NS_BIT;
	if (image_type == NON_SECURE)
		scr |= SCR_NS_BIT;

	scr &= ~SCR_RW_BIT;
	if ((next_image_info->spsr & (1 << MODE_RW_SHIFT)) ==
				(MODE_RW_64 << MODE_RW_SHIFT))
    {
		scr |= SCR_RW_BIT;

        printf("spsr is 64 bit\n");
    }

	scr |= SCR_HCE_BIT;

	/*
	 * Tell the context mgmt. library to ensure that SP_EL3 points to
	 * the right context to exit from EL3 correctly.
	 */
	cm_set_el3_eret_context(image_type,
			next_image_info->pc,
			next_image_info->spsr,
			scr);

	/*
	 * Save the args generated in BL2 for the image in the right context
	 * used on its entry
	 */
	ctx = cm_get_context(read_mpidr(), image_type);
	gp_regs = get_gpregs_ctx(ctx);
	memcpy(gp_regs, (void *)&next_image_info->args, sizeof(aapcs64_params_t));

    printf("Finally set the next context\n");

	/* Finally set the next context */
	cm_set_next_eret_context(image_type);
}



/*******************************************************************************
 * This function programs EL3 registers and performs other setup to enable entry
 * into the next image after BL31 at the next ERET.
 ******************************************************************************/
void bl31_prepare_next_image_entry()
{
	entry_point_info_t *next_image_info;
	uint32_t scr, image_type;
	cpu_context_t *ctx;
	gp_regs_t *gp_regs;

	/* Determine which image to execute next */
	image_type = bl31_get_next_image_type();

	/*
	 * Setup minimal architectural state of the next highest EL to
	 * allow execution in it immediately upon entering it.
	 */
	bl31_next_el_arch_setup(image_type);

	/* Program EL3 registers to enable entry into the next EL */
	next_image_info = bl31_plat_get_next_image_ep_info(image_type);
	assert(next_image_info);
	assert(image_type == GET_SECURITY_STATE(next_image_info->h.attr));

	scr = read_scr();
	scr &= ~SCR_NS_BIT;
	if (image_type == NON_SECURE)
		scr |= SCR_NS_BIT;

	scr &= ~SCR_RW_BIT;
	if ((next_image_info->spsr & (1 << MODE_RW_SHIFT)) ==
				(MODE_RW_64 << MODE_RW_SHIFT))
	{
		scr |= SCR_RW_BIT;
		scr |= SCR_HCE_BIT;
	}
	else
	{
		scr &= ~(SCR_HCE_BIT);
	}

	/*
	 * FIXME: Need a configurable flag when we have hypervisor installed
	 * This is for PSCI CPU_UP api to work correctly
	 * PSCI uses scr.hce to determine the target CPU of CPU_UP
	 * returns to NS world with HYP mode(HCE is set) or SVC mode(HCE is not set)
	 * (refer to psci_set_ns_entry_info() in psci_common.c)
	 * since we don't have hypervisor installed for now, we need to
	 * enter linux with SVC mode.
	 */
	// FIXME: For 64bit kernel, we need return to normal world with EL2
	// Temporary comment out this to enable 64bit kernel smp.
	// Need a method to configure this for 32bit/64bit kernel
	//scr &= ~(SCR_HCE_BIT);

	/*
	 * Tell the context mgmt. library to ensure that SP_EL3 points to
	 * the right context to exit from EL3 correctly.
	 */
	cm_set_el3_eret_context(image_type,
			next_image_info->pc,
			next_image_info->spsr,
			scr);

	/*
	 * Save the args generated in BL2 for the image in the right context
	 * used on its entry
	 */
	ctx = cm_get_context(read_mpidr(), image_type);
	gp_regs = get_gpregs_ctx(ctx);
	memcpy(gp_regs, (void *)&next_image_info->args, sizeof(aapcs64_params_t));

	/* Finally set the next context */
	cm_set_next_eret_context(image_type);
}

/*******************************************************************************
 * This function initializes the pointer to BL32 init function. This is expected
 * to be called by the SPD after it finishes all its initialization
 ******************************************************************************/
void bl31_register_bl32_init(int32_t (*func)(unsigned int, unsigned int))
{
	bl32_init = func;
}
