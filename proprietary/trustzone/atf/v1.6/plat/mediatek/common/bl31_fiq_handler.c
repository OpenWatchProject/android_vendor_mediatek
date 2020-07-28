/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31_fiq_handler.h>
#include <debug.h>
#include <fiq_smp_call.h>
#include <gic_common.h>
#include <gicv3.h>
#include <interrupt_mgmt.h>
#include <log.h>
#include <platform_def.h>
#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <bl31_fiq_handler.h>

struct fiq_desc fiq_handlers_array[MAX_FIQ_LINE_NUM];
extern void dump_gic(void);

#pragma weak dump_gic
void dump_gic(void)
{
	return;
}

/* For legacy SoC compatible that not refine GiC driver*/
#pragma weak mt_bl31_spi_enable
extern void mt_bl31_spi_enable(uint32_t fiq_num, uint32_t int_group, uint32_t trigger_type);

static void handle_IPI(uint32_t fiqnr)
{
	switch (fiqnr) {
		case FIQ_SMP_CALL_SGI:
			fiq_icc_isr();
			break;
		default:
			ERROR("IPI number[%u] not define!\n", fiqnr);
			dump_gic();
			assert(0);
	}
}
uint32_t request_fiq(uint32_t fiq_num, fiq_handler_t handler,
			uint32_t trigger_type, uint32_t int_group, void *cookie)
{
	uint32_t i;

	assert((int_group == INTR_GROUP1S) || (int_group == INTR_GROUP0));
	assert(handler != NULL);
	for (i = 0 ; i < MAX_FIQ_LINE_NUM ; i++) {
		if ((fiq_handlers_array[i].fiq_num_handler != NULL) && (fiq_handlers_array[i].fiqnr != fiq_num))
			continue;
		fiq_handlers_array[i].fiqnr = fiq_num;
		fiq_handlers_array[i].fiq_num_handler = handler;
		fiq_handlers_array[i].int_group = int_group;
		fiq_handlers_array[i].cookie = cookie;
		if (mt_bl31_spi_enable)
			mt_bl31_spi_enable(fiq_num, int_group, trigger_type);
		else
			NOTICE("=====mt_bl31_spi_enable is not implemented!=====\n");
		break;
	}
	assert(i < MAX_FIQ_LINE_NUM);
	/* INFO("[%s]fiq_num:%d, handler:0x%lx, trigger_type:%d, int_group:%d\n", __func__, fiq_num, (uintptr_t)handler, trigger_type, int_group); */
	return 0;
}

uint64_t bl31_el3_fiq_handler(uint32_t id,
			   uint32_t flags,
			   void *handle,
			   void *cookie)
{
	uint32_t fiqnr;
	uint64_t ret = INTR_TYPE_INVAL;

	/* For now, MTK EL3 FIQ handler only service group0 interrupt from non-secure state */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	fiqnr = plat_ic_acknowledge_interrupt();

#if defined(MTK_PLAT_PORTING_LAYER)
		/* footprint of irq */
		if (fiqnr == WDT_IRQ_BIT_ID || fiqnr == FIQ_SMP_CALL_SGI) {
			atf_enter_footprint(fiqnr);
		}
#endif

	switch (fiqnr) {
		case PENDING_G1S_INTID:
			ret = INTR_TYPE_S_EL1;
			break;
		case PENDING_G1NS_INTID:
			ret = INTR_TYPE_NS;
			break;
		case GIC_SPURIOUS_INTERRUPT:
			NOTICE("SPURIOUS FIQ[%d] number not support!\n", fiqnr);
			break;
		default:
			ret = bl31_fiq_dispatcher(fiqnr);
			assert(ret != INTR_TYPE_INVAL);
			plat_ic_end_of_interrupt(fiqnr);
			break;
	}

#if defined(MTK_PLAT_PORTING_LAYER)
	atf_exit_footprint();
#endif
	return ret;
}

uint32_t bl31_fiq_dispatcher(uint32_t fiqnr)
{
	int	i;
	uint32_t ret = INTR_TYPE_INVAL;

	if (fiqnr < 16) {
		handle_IPI(fiqnr);
		ret = INTR_TYPE_EL3;
	} else {
		for (i = 0 ; i < MAX_FIQ_LINE_NUM ; i++) {
			if (fiq_handlers_array[i].fiqnr == fiqnr &&
				fiq_handlers_array[i].fiq_num_handler) {
				fiq_handlers_array[i].fiq_num_handler(fiq_handlers_array[i].cookie);
				ret = INTR_TYPE_EL3;
				break;
			}
		}
	}
	return ret;
}
