#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdint.h>
#include "plat_private.h"
#include <platform_def.h>
#include <stdio.h>
#include <mmio.h>
#include <eint.h>
#define mt_reg_sync_writel(bit, base) mmio_write_32(base, bit)
#define IOMEM(reg) (reg)


static void mt_eint_disable_deint_selection(unsigned int deint_mapped);
static void mt_eint_enable_deint_selection(unsigned int deint_mapped);
unsigned int deint_possible_irq[] = {222, 223, 224, 225};
unsigned int MAX_DEINT_CNT = 4;
unsigned int EINT_MAX_CHANNEL = 152;

void mt_eint_mask(unsigned int eint_num)
{
	unsigned long base;
	unsigned int bit = 1 << (eint_num % 32);

	if (eint_num < EINT_MAX_CHANNEL)
		base = (eint_num / 32) * 4 + EINT_MASK_SET_BASE;
	else
		return;

	mt_reg_sync_writel(bit, base);
}

void mt_eint_unmask(unsigned int eint_num)
{
	unsigned long base;
	unsigned int bit = 1 << (eint_num % 32);

	if (eint_num < EINT_MAX_CHANNEL)
		base = (eint_num / 32) * 4 + EINT_MASK_CLR_BASE;
	else
		return;

	mt_reg_sync_writel(bit, base);
}

void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol)
{
	unsigned long base;
	unsigned int bit = 1 << (eint_num % 32);

	if (pol == MT_EINT_POL_NEG) {
		if (eint_num < EINT_MAX_CHANNEL)
			base = (eint_num / 32) * 4 + EINT_POL_CLR_BASE;
		else
			return;
	} else {
		if (eint_num < EINT_MAX_CHANNEL)
			base = (eint_num / 32) * 4 + EINT_POL_SET_BASE;
		else
			return;
	}

	mt_reg_sync_writel(bit, base);

	if (eint_num < EINT_MAX_CHANNEL)
		base = (eint_num / 32) * 4 + EINT_INTACK_BASE;
	else
		return;

	mt_reg_sync_writel(bit, base);
}

static void mt_eint_clr_deint_selection(unsigned int deint_mapped)
{
	if (deint_mapped < 4)
		mt_reg_sync_writel(0xff << (deint_mapped * 8),
				   IOMEM(DEINT_SEL_CLR_BASE));
	else if ((deint_mapped >= 4) && (deint_mapped < 8))
		mt_reg_sync_writel(0xff << ((deint_mapped - 4) * 8),
				   IOMEM(DEINT_SEL_CLR_BASE + 4));
}

int mt_eint_clr_deint(unsigned int irq_num)
{
	unsigned int deint_mapped = 0;
	unsigned int i;
	unsigned int ret;

	for (i = 0; i < MAX_DEINT_CNT; ++i) {
		if (deint_possible_irq[i] == irq_num)
			break;
	}

	if (i == MAX_DEINT_CNT)
		return -1;

	deint_mapped = irq_num - deint_possible_irq[0];
	mt_eint_disable_deint_selection(deint_mapped);
	mt_eint_set_deint_sec_en(deint_mapped, 0);
	mt_eint_clr_deint_selection(deint_mapped);
	ret = mt_eint_get_deint_sec_en(deint_mapped);

	return ret;
}

static void mt_eint_set_deint_selection(unsigned int eint_num,
				unsigned int deint_mapped)
{
	/* set our new deint_sel setting */
	if (deint_mapped < 4)
		mt_reg_sync_writel((eint_num << (deint_mapped * 8)),
			IOMEM(DEINT_SEL_SET_BASE));
	else if ((deint_mapped >= 4) && (deint_mapped < 8))
		mt_reg_sync_writel((eint_num << ((deint_mapped-4) * 8)),
			IOMEM(DEINT_SEL_SET_BASE + 4));
}

static void mt_eint_enable_deint_selection(unsigned int deint_mapped)
{
	mt_reg_sync_writel(mmio_read_32(IOMEM(DEINT_CON_BASE)) |
			   (1 << deint_mapped),
			   IOMEM(DEINT_CON_BASE));
}

static void mt_eint_disable_deint_selection(unsigned int deint_mapped)
{
	mt_reg_sync_writel(mmio_read_32(IOMEM(DEINT_CON_BASE)) &
			   ~(1 << deint_mapped),
			   IOMEM(DEINT_CON_BASE));
}

unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens)
{
	unsigned long base;
	unsigned int bit = 1 << (eint_num % 32);

	if (sens == MT_EDGE_SENSITIVE) {
		if (eint_num < EINT_MAX_CHANNEL)
			base = (eint_num / 32) * 4 + EINT_SENS_CLR_BASE;
		else
			return 0;
	} else if (sens == MT_LEVEL_SENSITIVE)
		if (eint_num < EINT_MAX_CHANNEL)
			base = (eint_num / 32) * 4 + EINT_SENS_SET_BASE;
		else
			return 0;
	else
		return 0;

	mt_reg_sync_writel(bit, base);

	return 0;
}

unsigned int
mt_eint_set_deint_sec_en(unsigned int deint_mapped, unsigned int value)
{
	unsigned int base;
	unsigned int bit;

	base = SECURE_DIR_EINT_EN;

	if (value == 1) {
		bit = 0x1 << (deint_mapped % 16);
		bit = mmio_read_32(SECURE_DIR_EINT_EN) | bit;
	} else {
		bit = 0x1 << (deint_mapped % 16);
		bit = mmio_read_32(SECURE_DIR_EINT_EN) & ~(bit);
	}

	mt_reg_sync_writel(bit, base);

	return 0;
}



unsigned int
mt_eint_get_deint_sec_en(unsigned int deint_mapped)
{
	unsigned int base;
	unsigned int st;
	unsigned int bit = 1 << (deint_mapped % 16);

	base = SECURE_DIR_EINT_EN;
	st = mmio_read_32(base);

	return ((st & bit)?1:0);
}

int mt_eint_set_secure_deint(unsigned int eint_num, unsigned int irq_num)
{
	unsigned int deint_mapped = 0;
	int i = 0;
	int ret;

	if (eint_num >= EINT_MAX_CHANNEL)
		return -1;

	for (i = 0; i < MAX_DEINT_CNT; ++i)
		if (deint_possible_irq[i] == irq_num)
			break;

	if (i == MAX_DEINT_CNT)
		return -1;

	deint_mapped = irq_num - deint_possible_irq[0];

	mt_eint_set_deint_sec_en(deint_mapped, 1);

	/* mask from eintc, only triggered by GIC */
	mt_eint_mask(eint_num);

	/* set eint part as high-level to bypass signal to GIC */
	mt_eint_set_polarity(eint_num, MT_POLARITY_HIGH);
	mt_eint_set_sens(eint_num, MT_LEVEL_SENSITIVE);

	mt_eint_clr_deint_selection(deint_mapped);
	mt_eint_set_deint_selection(eint_num, deint_mapped);
	mt_eint_enable_deint_selection(deint_mapped);
	ret = mt_eint_get_deint_sec_en(deint_mapped);

	return ret;
}
