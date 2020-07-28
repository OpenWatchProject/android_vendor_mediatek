#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdint.h>
#include "plat_private.h"
#include <platform_def.h>
#include <stdio.h>  //for ERROR
#include <mmio.h>
#include <eint.h>
#define mt_reg_sync_writel(bit,base) mmio_write_32(base,bit)
#define IOMEM(reg) (reg)


static void mt_eint_disable_deint_selection(unsigned int deint_mapped);
static void mt_eint_enable_deint_selection(unsigned int deint_mapped);
unsigned int mt_eint_get_deint_sec_en (unsigned int deint_mapped);
unsigned int mt_eint_set_deint_sec_en (unsigned int deint_mapped, unsigned int value);


unsigned int deint_possible_irq[] = {191, 192, 193, 194};
unsigned int MAX_DEINT_CNT = 4;
unsigned int EINT_MAX_CHANNEL = 192;
void mt_eint_mask(unsigned int eint_num)
{
        unsigned long base;
        unsigned int bit = 1 << (eint_num % 32);

        if (eint_num < EINT_MAX_CHANNEL) {
                base = (eint_num / 32) * 4 + EINT_MASK_SET_BASE;
        } else {
                VERBOSE
                    ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                     __func__, eint_num);
                return;
        }
        mt_reg_sync_writel(bit, base);

        VERBOSE("[EINT] mask addr:%lx = %x\n", base, bit);
}

void mt_eint_unmask(unsigned int eint_num)
{
        unsigned long base;
        unsigned int bit = 1 << (eint_num % 32);

        if (eint_num < EINT_MAX_CHANNEL) {
                base = (eint_num / 32) * 4 + EINT_MASK_CLR_BASE;
        } else {
                VERBOSE
                    ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                     __func__, eint_num);
                return;
        }
        mt_reg_sync_writel(bit, base);

        VERBOSE("[EINT] unmask addr:%lx = %x\n", base, bit);
}

void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol)
{
        unsigned long base;
        unsigned int bit = 1 << (eint_num % 32);

        if (pol == MT_EINT_POL_NEG) {
                if (eint_num < EINT_MAX_CHANNEL) {
                        base = (eint_num / 32) * 4 + EINT_POL_CLR_BASE;
                } else {
                        VERBOSE
                            ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                             __func__, eint_num);
                        return;
                }
        } else {
                if (eint_num < EINT_MAX_CHANNEL) {
                        base = (eint_num / 32) * 4 + EINT_POL_SET_BASE;
                } else {
                        VERBOSE
                            ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                             __func__, eint_num);
                        return;
                }
        }
        mt_reg_sync_writel(bit, base);
        VERBOSE("[EINT] %s :%lx, bit: %x\n", __func__, base, bit);

        /* accodring to DE's opinion, the longest latency need is about 250 ns */
        extern void udelay(unsigned long us);
        udelay(1);

        if (eint_num < EINT_MAX_CHANNEL) {
                base = (eint_num / 32) * 4 + EINT_INTACK_BASE;
        } else {
                VERBOSE
                    ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                     __func__, eint_num);
                return;
        }
        mt_reg_sync_writel(bit, base);
}

static void mt_eint_clr_deint_selection(unsigned int deint_mapped)
{
        if (deint_mapped < 4)
                mt_reg_sync_writel(0xff << (deint_mapped * 8),
                        IOMEM(DEINT_SEL_CLR_BASE));
        else if ((deint_mapped >= 4) && (deint_mapped < 8))
                mt_reg_sync_writel(0xff << ((deint_mapped-4) * 8),
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

        if (i == MAX_DEINT_CNT) {
                VERBOSE("%s: no matched possible deint irq for %u\n", __func__, irq_num);
                return -1;
        }

        deint_mapped = irq_num - deint_possible_irq[0];
	mt_eint_disable_deint_selection(deint_mapped);
	mt_eint_set_deint_sec_en(deint_mapped , 0);
        mt_eint_clr_deint_selection(deint_mapped);
	ret = mt_eint_get_deint_sec_en(deint_mapped);
        return ret;
}
static void mt_eint_set_deint_selection(unsigned int eint_num, unsigned int deint_mapped)
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
        mt_reg_sync_writel(mmio_read_32(IOMEM(DEINT_CON_BASE)) | (1 << deint_mapped), IOMEM(DEINT_CON_BASE));
}

static void mt_eint_disable_deint_selection(unsigned int deint_mapped)
{
        mt_reg_sync_writel(mmio_read_32(IOMEM(DEINT_CON_BASE)) & ~(1 << deint_mapped), IOMEM(DEINT_CON_BASE));
}


unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens)
{
        unsigned long base;
        unsigned int bit = 1 << (eint_num % 32);

        if (sens == MT_EDGE_SENSITIVE) {
                if (eint_num < EINT_MAX_CHANNEL) {
                        base = (eint_num / 32) * 4 + EINT_SENS_CLR_BASE;
                } else {
                        VERBOSE
                            ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                             __func__, eint_num);
                        return 0;
                }
        } else if (sens == MT_LEVEL_SENSITIVE) {
                if (eint_num < EINT_MAX_CHANNEL) {
                        base = (eint_num / 32) * 4 + EINT_SENS_SET_BASE;
                } else {
                        VERBOSE
                            ("Error in %s [EINT] num:%d is larger than EINT_MAX_CHANNEL\n",
                             __func__, eint_num);
                        return 0;
                }
        } else {
                VERBOSE("%s invalid sensitivity value\n", __func__);
                return 0;
        }
        mt_reg_sync_writel(bit, base);
        VERBOSE("[EINT] %s :%lx, bit: %x\n", __func__, base, bit);
        return 0;
}


unsigned int
mt_eint_set_deint_sec_en (unsigned int deint_mapped, unsigned int value)
{
        unsigned int base;
        unsigned int bit;
        base = SECURE_DIR_EINT_EN;

	if(value == 1){
		bit = 0x1 << (deint_mapped % 16);
		bit = mmio_read_32(SECURE_DIR_EINT_EN) | bit;
	}
	else{
		bit = 0x1 << (deint_mapped % 16);
		bit = mmio_read_32(SECURE_DIR_EINT_EN) & ~(bit);
	}

        mt_reg_sync_writel (bit, base);
        VERBOSE ("[EINT] %s :%x, bit: %x,\n", __func__, base, bit);
        return 0;
}



unsigned int
mt_eint_get_deint_sec_en (unsigned int deint_mapped)
{
        unsigned int base;
        unsigned int st;
        unsigned int bit = 1 << (deint_mapped % 16);
        base = SECURE_DIR_EINT_EN;
        st = mmio_read_32 (base);
        VERBOSE ("[EINT] %s :%x,value: 0x%x,bit: %x\n", __func__, base,st ,bit);
        return ((st & bit)?1:0);
}

int mt_eint_set_secure_deint(unsigned int eint_num, unsigned int irq_num)
{
        unsigned int deint_mapped = 0;
        int i = 0;
	int ret;
        if (eint_num >= EINT_MAX_CHANNEL) {
                VERBOSE("%s: eint_num(%u) is not in (0, %d)\n", __func__,
                       eint_num, EINT_MAX_CHANNEL);
                return -1;
        }

        for (i = 0; i < MAX_DEINT_CNT; ++i) {
                if (deint_possible_irq[i] == irq_num){
                        break;
		}
        }
        if (i == MAX_DEINT_CNT) {
                VERBOSE("%s: no matched possible deint irq for %u\n", __func__, irq_num);
                return -1;
        }

        deint_mapped = irq_num - deint_possible_irq[0];

	mt_eint_set_deint_sec_en(deint_mapped , 1);

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
