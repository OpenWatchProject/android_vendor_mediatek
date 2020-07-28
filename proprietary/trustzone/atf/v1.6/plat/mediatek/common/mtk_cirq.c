/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <gicv3.h>
#include <arm/gic_common.h>
#include <arch_helpers.h>
#include <platform_def.h>
#include <mmio.h>
#include <stdint.h>
#include "mtk_cirq.h"

static struct cirq_events cirq_all_events = {
	.spi_start = CIRQ_SPI_START,
};
static uint32_t already_cloned;
/*
 * mt_irq_mask_restore: restore all interrupts
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 * (This is ONLY used for the idle current measurement by the factory mode.)
 */
int mt_irq_mask_restore(struct mtk_irq_mask *mask)
{
	uint32_t dist_base;

	dist_base = BASE_GICD_BASE;

	if (!mask)
		return -1;
	if (mask->header != IRQ_MASK_HEADER)
		return -1;
	if (mask->footer != IRQ_MASK_FOOTER)
		return -1;

	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x4), mask->mask1);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x8), mask->mask2);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0xc), mask->mask3);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x10), mask->mask4);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x14), mask->mask5);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x18), mask->mask6);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x1c), mask->mask7);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x20), mask->mask8);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x24), mask->mask9);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x28), mask->mask10);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x2c), mask->mask11);
	mmio_write_32_relax((dist_base + GICD_ISENABLER + 0x30), mask->mask12);
	/* make sure dist changes happen */
	dsb();

	return 0;
}

/*
 * mt_irq_mask_all: disable all interrupts
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 * (This is ONLY used for the idle current measurement by the factory mode.)
 */
int mt_irq_mask_all(struct mtk_irq_mask *mask)
{
	uint32_t dist_base;

	dist_base = BASE_GICD_BASE;

	if (mask) {
		/* for SPI */
		mask->mask1 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x4));
		mask->mask2 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x8));
		mask->mask3 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0xc));
		mask->mask4 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x10));
		mask->mask5 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x14));
		mask->mask6 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x18));
		mask->mask7 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x1c));
		mask->mask8 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x20));
		mask->mask9 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x24));
		mask->mask10 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x28));
		mask->mask11 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x2c));
		mask->mask12 = mmio_read_32_relax((dist_base + GICD_ISENABLER + 0x30));

		/* for SPI */
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x4), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x8), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0xC), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x10), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x14), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x18), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x1C), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x20), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x24), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x28), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x2c), 0xFFFFFFFF);
		mmio_write_32_relax((dist_base + GICD_ICENABLER + 0x30), 0xFFFFFFFF);
		/* make sure distributor changes happen */
		dsb();

		mask->header = IRQ_MASK_HEADER;
		mask->footer = IRQ_MASK_FOOTER;

		return 0;
	} else {
		return -1;
	}
}

static uint32_t mt_irq_get_pol(uint32_t irq)
{
#ifdef CIRQ_WITH_POLARITY
	uint32_t reg;
	uint32_t base = INT_POL_CTL0;

	if (irq < 32)
		return 0;

	reg = ((irq - 32) / 32);

	return mmio_read_32_relax(base + reg * 4);
#else
	return 0;
#endif
}

static void collect_all_wakeup_events(void)
{
	uint32_t i;
	uint32_t gic_irq;
	uint32_t cirq;
	uint32_t cirq_reg;
	uint32_t cirq_offset;
	uint32_t mask;
	uint32_t pol_mask;
	uint32_t irq_offset;
	uint32_t irq_mask;

	if ((!cirq_all_events.wakeup_events) ||
			cirq_all_events.num_of_events == 0)
		return;

	for (i = 0; i < cirq_all_events.num_of_events; i++) {
		if (cirq_all_events.wakeup_events[i] > 0) {
			gic_irq = cirq_all_events.wakeup_events[i];
			cirq = gic_irq - cirq_all_events.spi_start - 32;
			cirq_reg = cirq / 32;
			cirq_offset = cirq % 32;
			mask = 0x1 << cirq_offset;
			irq_offset = gic_irq % 32;
			irq_mask = 0x1 << irq_offset;
			/*
			 * CIRQ default masks all
			 */
			cirq_all_events.table[cirq_reg].mask |= mask;
			/*
			 * CIRQ default pol is low
			 */
			pol_mask = mt_irq_get_pol(
					cirq_all_events.wakeup_events[i])
					& irq_mask;
			/*
			 * 0 means rising
			 */
			if (pol_mask == 0)
				cirq_all_events.table[cirq_reg].pol |= mask;
			/*
			 * CIRQ only monitor edge trigger
			 */
			cirq_all_events.table[cirq_reg].sen |= mask;

			cirq_all_events.table[cirq_reg].used = 1;
		}
	}
}

/*
 * mt_cirq_set_pol: Set the polarity for the specified SYS_CIRQ number.
 * @cirq_num: the SYS_CIRQ number to set
 * @pol: polarity to set
 * @return:
 *    0: set pol success
 *   -1: cirq num is out of range
 */
#ifdef CIRQ_WITH_POLARITY
static int mt_cirq_set_pol(uint32_t cirq_num, uint32_t pol)
{
	uint32_t base;
	uint32_t bit = 1 << (cirq_num % 32);

	if (cirq_num >= CIRQ_IRQ_NUM)
		return -1;

	if (pol == MT_CIRQ_POL_NEG)
		base = (cirq_num / 32) * 4 + CIRQ_POL_CLR_BASE;
	else if (pol == MT_CIRQ_POL_POS)
		base = (cirq_num / 32) * 4 + CIRQ_POL_SET_BASE;
	else
		return -1;

	mmio_write_32(base, bit);
	return 0;
}
#endif

/*
 * mt_cirq_mask: Mask the specified SYS_CIRQ.
 * @cirq_num: the SYS_CIRQ number to mask
 * @return:
 *    0: mask success
 *   -1: cirq num is out of range
 */
static int mt_cirq_mask(uint32_t cirq_num)
{
	uint32_t bit = 1 << (cirq_num % 32);

	if (cirq_num >= CIRQ_IRQ_NUM)
		return -1;

	mmio_write_32((cirq_num / 32) * 4 + CIRQ_MASK_SET_BASE, bit);

	return 0;
}

/*
 * mt_cirq_unmask: Unmask the specified SYS_CIRQ.
 * @cirq_num: the SYS_CIRQ number to unmask
 * @return:
 *    0: umask success
 *   -1: cirq num is out of range
 */
static int mt_cirq_unmask(uint32_t cirq_num)
{
	uint32_t bit = 1 << (cirq_num % 32);

	if (cirq_num >= CIRQ_IRQ_NUM)
		return -1;

	mmio_write_32((cirq_num / 32) * 4 + CIRQ_MASK_CLR_BASE, bit);

	return 0;
}

uint32_t mt_irq_get_en(uint32_t irq)
{
	uint32_t addr;
	uint32_t bit = (1 << (irq % 32));

	addr = BASE_GICD_BASE + GICD_ISENABLER + (irq / 32) * 4;

	return (mmio_read_32(addr) & bit) ? 1 : 0;
}

static void __cirq_fast_clone(void)
{
	struct cirq_reg *reg;
	int i;

	for (i = 0; i < CIRQ_REG_NUM ; ++i) {
		uint32_t cirq_bit;

		reg = &cirq_all_events.table[i];

		if (!reg->used)
			continue;

		mmio_write_32(CIRQ_SENS_CLR_BASE + (reg->reg_num * 4),
				    reg->sen);

		for (cirq_bit = 0; cirq_bit < 32 ; ++cirq_bit) {
			uint32_t val, cirq_id;
			uint32_t gic_id;
#ifdef CIRQ_WITH_POLARITY
			uint32_t gic_bit, pol;
#endif
			uint32_t en;

			val = ((1 << cirq_bit) & reg->mask);

			if (val == 0)
				continue;

			cirq_id = (reg->reg_num << 5) + cirq_bit;
			gic_id = CIRQ_TO_IRQ_NUM(cirq_id);
#ifdef CIRQ_WITH_POLARITY
			gic_bit = (0x1 << ((gic_id - 32) % 32));
			pol = mt_irq_get_pol(gic_id) & gic_bit;
			if (pol)
				mt_cirq_set_pol(cirq_id, MT_CIRQ_POL_NEG);
			else
				mt_cirq_set_pol(cirq_id, MT_CIRQ_POL_POS);
#endif
			en = mt_irq_get_en(gic_id);
			if (en)
				mt_cirq_unmask(cirq_id);
			else
				mt_cirq_mask(cirq_id);
		}
	}
}

static void cirq_fast_clone(void)
{
	if (!already_cloned) {
		collect_all_wakeup_events();
		already_cloned = 1;
	}
	__cirq_fast_clone();
}

void set_wakeup_sources(uint32_t *list, uint32_t num_of_events)
{
	cirq_all_events.num_of_events = num_of_events;
	cirq_all_events.wakeup_events = list;
}
/*
 * mt_cirq_clone_gic: Copy the setting from GIC to SYS_CIRQ
 */
void mt_cirq_clone_gic(void)
{
	cirq_fast_clone();
}

uint32_t mt_irq_get_pending_vec(uint32_t start_irq)
{
	uint32_t base = 0;
	uint32_t pending_vec = 0;
	uint32_t reg = start_irq / 32;
	uint32_t LSB_num, MSB_num;
	uint32_t LSB_vec, MSB_vec;

	base = BASE_GICD_BASE;

	/* if start_irq is not aligned 32, do some assembling */
	MSB_num = start_irq % 32;
	if (MSB_num != 0) {
		LSB_num = 32 - MSB_num;
		LSB_vec = mmio_read_32(base + GICD_ISPENDR + reg * 4) >> MSB_num;
		MSB_vec = mmio_read_32(base + GICD_ISPENDR + (reg + 1) * 4) << LSB_num;
		pending_vec = MSB_vec | LSB_vec;
	} else {
		pending_vec = mmio_read_32(base + GICD_ISPENDR + reg * 4);
	}

	return pending_vec;
}

static int mt_cirq_get_mask_vec(int i)
{
	return mmio_read_32((i * 4) + CIRQ_MASK_BASE);
}

/*
 * mt_cirq_ack_all: Ack all the interrupt on SYS_CIRQ
 */
void mt_cirq_ack_all(void)
{
	uint32_t ack_vec, pend_vec, mask_vec;
	int i;

	for (i = 0; i < CIRQ_CTRL_REG_NUM; i++) {
		/*
		 * if a irq is pending & not masked, don't ack it
		 * , since cirq start irq might not be 32 aligned with gic,
		 * need an exotic API to get proper vector of pending irq
		 */
		pend_vec = mt_irq_get_pending_vec(CIRQ_SPI_START+(i+1)*32);
		mask_vec = mt_cirq_get_mask_vec(i);
		/* those should be acked are: "not (pending & not masked)",
		 */
		ack_vec = (~pend_vec) | mask_vec;
		mmio_write_32(CIRQ_ACK_BASE + (i * 4), ack_vec);
	}

	/*
	 * make sure all cirq setting take effect
	 * before doing other things
	 */
	dsb();
}
/*
 * mt_cirq_enable: Enable SYS_CIRQ
 */
void mt_cirq_enable(void)
{
	uint32_t st;

	mt_cirq_ack_all();

	st = mmio_read_32(CIRQ_CON);
	st |=
	    (CIRQ_CON_EN << CIRQ_CON_EN_BITS) | (CIRQ_CON_EDGE_ONLY <<
						 CIRQ_CON_EDGE_ONLY_BITS);
	mmio_write_32(CIRQ_CON, (st & CIRQ_CON_BITS_MASK));
}

/*
 * mt_cirq_disable: Disable SYS_CIRQ
 */
void mt_cirq_disable(void)
{
	uint32_t st;

	st = mmio_read_32(CIRQ_CON);
	st &= ~(CIRQ_CON_EN << CIRQ_CON_EN_BITS);
	mmio_write_32(CIRQ_CON, (st & CIRQ_CON_BITS_MASK));
}

void mt_irq_unmask_for_sleep_ex(uint32_t irq)
{
	uint32_t dist_base;
	uint32_t mask;

	dist_base = BASE_GICD_BASE;
	mask = 1 << (irq % 32);

	mmio_write_32(dist_base + GICD_ISENABLER + irq / 32 * 4, mask);
}

void mt_irq_set_pending(uint32_t irq)
{
	uint32_t addr;
	uint32_t bit = 1 << (irq % 32);

	addr = BASE_GICD_BASE + GICD_ISPENDR + (irq / 32) * 4;

	mmio_write_32(addr, bit);
}

void mt_cirq_mask_all(void)
{
	uint32_t i;

	for (i = 0; i < CIRQ_CTRL_REG_NUM; i++)
		mmio_write_32_relax(CIRQ_MASK_SET_BASE + (i * 4), 0xFFFFFFFF);

	dsb();
}

static void cirq_fast_sw_flush(void)
{
	struct cirq_reg *reg;
	int i;

	for (i = 0; i < CIRQ_REG_NUM ; ++i) {
		uint32_t cirq_bit;

		reg = &cirq_all_events.table[i];

		if (!reg->used)
			continue;

		reg->pending = mmio_read_32(CIRQ_STA_BASE + (reg->reg_num << 2));
		reg->pending &= reg->mask;

		for (cirq_bit = 0; cirq_bit < 32 ; ++cirq_bit) {
			uint32_t val, cirq_id;

			val = (1 << cirq_bit) & reg->pending;
			if (val == 0)
				continue;

			cirq_id = (reg->reg_num << 5) + cirq_bit;
			mt_irq_set_pending(CIRQ_TO_IRQ_NUM(cirq_id));
		}
	}
}

/*
 * mt_cirq_disable: Flush interrupt from SYS_CIRQ to GIC
 */
void mt_cirq_flush(void)
{
	cirq_fast_sw_flush();
	mt_cirq_mask_all();
	mt_cirq_ack_all();
}
