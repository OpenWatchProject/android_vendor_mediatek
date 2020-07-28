/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <mcucfg.h>
#include <mmio.h>
#include <platform_def.h>

#define SYSTIMER_BASE		0x10017000
#define CNTCR_REG			(SYSTIMER_BASE + 0x0)
#define CNTSR_REG			(SYSTIMER_BASE + 0x4)
#define COMP_15_EN     (1 << 10)
#define COMP_20_EN     (1 << 11)
#define COMP_25_EN     (1 << 12)



#if 0
#define SPM_REGISTER_LOCK	(0x10A06000)
#define CONTROL_MUX			(0x10A0602c)
#define SELECT_CLOCK		(0x10A06008)
#endif

static void enable_systimer_compensation(void)
{
	unsigned int reg = 0;

	reg = mmio_read_32(CNTCR_REG);
	reg |= (COMP_20_EN | COMP_25_EN);
	reg &= ~(COMP_15_EN);
	mmio_write_32(CNTCR_REG, reg);

	INFO("[systimer] CNTCR_REG(0x%x)\n", mmio_read_32(CNTCR_REG));
#if 0
	/* unlock spm register */
	mmio_write_32(SPM_REGISTER_LOCK, 0x0b160001);
	/* set control mux */
	reg = mmio_read_32(CONTROL_MUX);
	reg &= ~(1 << 7);
	mmio_write_32(CONTROL_MUX, reg);
	/* select clock */
	reg = mmio_read_32(SELECT_CLOCK);
	reg |= 1 << 17;
	mmio_write_32(SELECT_CLOCK, reg);

	reg = mmio_read_32(CNTSR_REG);
	reg &= (7 << 8);
	reg >>= 8;
	INFO("[systimer] new clock=%d\n", reg);
#endif
}

void setup_systimer(void)
{
	/* systimer is default on, so we only enable systimer compensation */
	enable_systimer_compensation();
}
