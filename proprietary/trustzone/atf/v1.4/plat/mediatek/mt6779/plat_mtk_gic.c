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
#include <fiq_smp_call.h>
#include <debug.h>
#include <gicv3.h>
#include <gic_v2.h>
#include <plat_mtk_gic.h>
#include <mtk_aee_debug.h>
#include <platform.h>
#include <platform_def.h>
#include <mtk_gic_v3_main.h>
#include <pccif.h>

/* override setup_int_schedule_mode for platform */
void setup_int_schedule_mode(enum irq_schedule_mode mode, unsigned int active_cpu)
{
	assert(mode <= HW_MODE);
	assert(active_cpu <= 0xFF);

	if (mode == HW_MODE) {
		mmio_write_32(GIC_INT_MASK,
				(mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_SEL_MASK)) |
				(0x1 << GIC500_ACTIVE_SEL_SHIFT));
		mmio_write_32(GIC_INT_MASK,
				(mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_CPU_MASK)) |
				(active_cpu << GIC500_ACTIVE_CPU_SHIFT));
	} else if (mode == SW_MODE) {
		mmio_write_32(GIC_INT_MASK,
				(mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_SEL_MASK)));
		mmio_write_32(GIC_INT_MASK,
				(mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_CPU_MASK)) |
				(active_cpu << GIC500_ACTIVE_CPU_SHIFT));
	}
}

/* override int_schedule_mode_save for platform */
void int_schedule_mode_save(void)
{
	/* TODO */
#if 0
	gic_data[0].saved_active_sel = mmio_read_32(GIC_INT_MASK);
#endif
}

/* override int_schedule_mode_restore for platform */
void int_schedule_mode_restore(void)
{
	/* TODO */
#if 0
	mmio_write_32(GIC_INT_MASK, gic_data[0].saved_active_sel);
#endif
}

void mt_irq_set_mask_all(unsigned int enable)
{
	mmio_write_32(INT_MSK_ALL, enable);

	dsb();
}

#ifndef MTK_INDIRECT_ACCESS_SUPPORT
void mt_irq_set_unmask(unsigned int irq)
{
	unsigned int offset, reg_index, value;

	/* spi only */
	if (irq >= GIC_PRIVATE_SIGNALS) {
		offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
		reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;

		value = mmio_read_32(INT_MSK_CTL0 + (reg_index * 4));
		/* INFO("%s(%u): red_index: 0x%x, val=0x%x\n", __func__, irq, reg_index * 4, value); */

		value &= ~(1 << offset);

		mmio_write_32((INT_MSK_CTL0 + (reg_index * 4)), value);

		/* INFO("%s(%u): new val=0x%x", __func__, irq, value); */
	}
}

void mt_irq_set_mask(unsigned int irq)
{
	unsigned int offset, reg_index, value;

	/* spi only */
	if (irq >= GIC_PRIVATE_SIGNALS) {
		offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
		reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;

		value = mmio_read_32(INT_MSK_CTL0 + (reg_index * 4));
		/* INFO("%s(%u): red_index: 0x%x, val=0x%x\n", __func__, irq, reg_index * 4, value); */

		value |= (1 << offset);

		mmio_write_32((INT_MSK_CTL0 + (reg_index * 4)), value);

		/* INFO("%s(%u): new val=0x%x", __func__, irq, value); */
	}
}

/* override mt_irq_set_polarity for platform */
void mt_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
	unsigned int offset, reg_index, value;
	unsigned int result;

	offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
	reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;

	if (polarity == 0) {
		/* active low */
		value = mmio_read_32(INT_POL_CTL0 + (reg_index * 4));
		value |= (1 << offset);
		mmio_write_32((INT_POL_CTL0 + (reg_index * 4)), value);
	} else {
		/* active high */
		value = mmio_read_32(INT_POL_CTL0 + (reg_index * 4));
		value &= ~(0x1 << offset);
		mmio_write_32(INT_POL_CTL0 + (reg_index * 4), value);
	}

	/* unmask irq */
	result = gicd_read_iidr(MT_GIC_BASE);
	if ((result >> GICD_V3_IIDR_PROD_ID_SHIFT) == GICD_V3_IIDR_GIC600)
		mt_irq_set_unmask(irq);

}


#else
void direct_access_ctrl(unsigned int enable)
{
	mmio_write_32(INT_CFG_DIRECT_ACCESS_EN, enable);

	dsb();
}

void mt_irq_set_unmask(unsigned int irq)
{
	unsigned int value;

	/* spi only */
	if (irq >= GIC_PRIVATE_SIGNALS) {
		/* set unmask */
		value = 0;
		/* select spi id*/
		value |= ((irq - GIC_PRIVATE_SIGNALS) << 16);
		/* select mask control*/
		value |= (1 << 30);

		mmio_write_32(INT_CFG_INDIRECT_ACCESS, value);
		/* INFO("%s(%u): new val=0x%x", __func__, irq, value); */
	}
}

void mt_irq_set_mask(unsigned int irq)
{
	unsigned int value;

	/* spi only */
	if (irq >= GIC_PRIVATE_SIGNALS) {
		/* set mask */
		value = 1;
		/* select spi id*/
		value |= ((irq - GIC_PRIVATE_SIGNALS) << 16);
		/* select mask control*/
		value |= (1 << 30);

		mmio_write_32(INT_CFG_INDIRECT_ACCESS, value);
		/* INFO("%s(%u): new val=0x%x", __func__, irq, value); */
	}
}

/* override mt_irq_set_polarity for platform */
void mt_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
	unsigned int value;
	unsigned int result;

	if (polarity == 0)
		value = 1;	/* active low */
	else
		value = 0;	/* active high */

	/* select spi id */
	value |= ((irq - GIC_PRIVATE_SIGNALS) << 16);
	/* select mask control*/
	value |= (1 << 29);

	mmio_write_32(INT_CFG_INDIRECT_ACCESS, value);

	/* unmask irq */
	result = gicd_read_iidr(MT_GIC_BASE);
	if ((result >> GICD_V3_IIDR_PROD_ID_SHIFT) == GICD_V3_IIDR_GIC600)
		mt_irq_set_unmask(irq);

}

#endif

static void clear_sec_pol_ctl_en(void)
{
	unsigned int i;

	/* total 19 polarity ctrl registers */
	for (i = 0; i <= NR_INT_POL_CTL-1; i++)
		mmio_write_32((SEC_POL_CTL_EN0 + (i * 4)), 0);

	dsb();
}

unsigned int mt_irq_mask_init_done;
unsigned int mt_irq_prefer_init_done;

void gic_setup(void)
{
	unsigned int gicd_base = 0;
	unsigned int result;
	int i;

	/* Init cpu logic map to avoid unnecessary SGI interrupt
	 * to cpu0 if WDT happens before cpu1~cpu7 on
	 */
	for (i = 0 ; i < sizeof(cpu_logical_map)/sizeof(uint64_t); i++)
		cpu_logical_map[i] = CPU_LOGIC_MAP_RESET_VAL;

	gicd_base = MT_GIC_BASE; /* get_plat_config()->gicd_base; */
	gic_distif_init(gicd_base);

	result = gicd_read_iidr(gicd_base);
	if ((result >> GICD_V3_IIDR_PROD_ID_SHIFT) == GICD_V3_IIDR_GIC600) {
		if (mt_irq_mask_init_done != 1) {
			/* mask all spi in 1st ATF */
			mt_irq_set_mask_all(0x1);
			mt_irq_mask_init_done = 1;
		}
	}

	gic600_rdistif_init();
	gic_cpuif_init();

	/* INFO("[ATF GIC] before clear_sec_pol_ctl_en\n"); */
	clear_sec_pol_ctl_en();

#ifdef MTK_INDIRECT_ACCESS_SUPPORT
	/* disable direct access */
	direct_access_ctrl(0x0);
#endif

#if CFG_MICROTRUST_TEE_SUPPORT
	teei_gic_setup();
#endif

	/* Register WDT handler */
	request_fiq(WDT_IRQ_BIT_ID, aee_wdt_dump_all_core, INT_EDGE_FALLING, INTR_GROUP0, NULL);
	request_fiq(PCCIF1_IRQ0_BIT_ID, ccif_irq0_handler, INT_LEVEL_LOW, INTR_GROUP0, NULL);
	request_fiq(PCCIF1_IRQ1_BIT_ID, ccif_irq1_handler, INT_LEVEL_LOW, INTR_GROUP0, NULL);

#ifndef MTK_FPGA_EARLY_PORTING
	if (mt_irq_prefer_init_done == 0) {
		setup_int_schedule_mode(SW_MODE, 0xFF);
		mt_irq_prefer_init_done = 1;
	}
#endif

}
