/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_CIRQ__
#define __MTK_CIRQ__

#include <stdint.h>

enum {
	IRQ_MASK_HEADER = 0xF1F1F1F1,
	IRQ_MASK_FOOTER = 0xF2F2F2F2
};

struct mtk_irq_mask {
	uint32_t header;	/* for error checking */
	uint32_t mask0;
	uint32_t mask1;
	uint32_t mask2;
	uint32_t mask3;
	uint32_t mask4;
	uint32_t mask5;
	uint32_t mask6;
	uint32_t mask7;
	uint32_t mask8;
	uint32_t mask9;
	uint32_t mask10;
	uint32_t mask11;
	uint32_t mask12;
	uint32_t footer;	/* for error checking */
};

/*
 * Define hardware register
 */
#define  CIRQ_STA_BASE         (SYS_CIRQ_BASE + 0x000)
#define  CIRQ_ACK_BASE         (SYS_CIRQ_BASE + 0x040)
#define  CIRQ_MASK_BASE        (SYS_CIRQ_BASE + 0x080)
#define  CIRQ_MASK_SET_BASE    (SYS_CIRQ_BASE + 0x0C0)
#define  CIRQ_MASK_CLR_BASE    (SYS_CIRQ_BASE + 0x100)
#define  CIRQ_SENS_BASE        (SYS_CIRQ_BASE + 0x140)
#define  CIRQ_SENS_SET_BASE    (SYS_CIRQ_BASE + 0x180)
#define  CIRQ_SENS_CLR_BASE    (SYS_CIRQ_BASE + 0x1C0)
#define  CIRQ_POL_BASE         (SYS_CIRQ_BASE + 0x200)
#define  CIRQ_POL_SET_BASE     (SYS_CIRQ_BASE + 0x240)
#define  CIRQ_POL_CLR_BASE     (SYS_CIRQ_BASE + 0x280)
#define  CIRQ_CON              (SYS_CIRQ_BASE + 0x300)

/*
 * Register placement
 */
#define  CIRQ_CON_EN_BITS           (0)
#define  CIRQ_CON_EDGE_ONLY_BITS    (1)
#define  CIRQ_CON_FLUSH_BITS        (2)
#define  CIRQ_CON_EVENT_BITS        (31)
#define  CIRQ_CON_BITS_MASK         (0x7)

/*
 * Register setting
 */
#define  CIRQ_CON_EN            (0x1)
#define  CIRQ_CON_EDGE_ONLY     (0x1)
#define  CIRQ_CON_FLUSH         (0x1)

/*
 * Define constant
 */
#define  CIRQ_CTRL_REG_NUM      ((CIRQ_IRQ_NUM + 31) / 32)

#define  MT_CIRQ_POL_NEG        (0)
#define  MT_CIRQ_POL_POS        (1)

#define IRQ_TO_CIRQ_NUM(irq)  ((irq) - (32 + CIRQ_SPI_START))
#define CIRQ_TO_IRQ_NUM(cirq) ((cirq) + (32 + CIRQ_SPI_START))

/*
 * Define function prototypes.
 */
void mt_cirq_enable(void);
void mt_cirq_disable(void);
void mt_cirq_clone_gic(void);
void mt_cirq_flush(void);
int mt_cirq_test(void);
void mt_cirq_dump_reg(void);
int mt_irq_mask_restore(struct mtk_irq_mask *mask);
int mt_irq_mask_all(struct mtk_irq_mask *mask);
void mt_cirq_clone_gic(void);
void mt_cirq_enable(void);
void mt_cirq_flush(void);
void mt_cirq_disable(void);
void mt_irq_unmask_for_sleep_ex(uint32_t irq);
void mt_irq_set_pending(uint32_t irq);
void set_wakeup_sources(uint32_t *list, uint32_t num_of_events);

struct cirq_reg {
	uint32_t reg_num;
	uint32_t used;
	uint32_t mask;
	uint32_t pol;
	uint32_t sen;
	uint32_t pending;
	uint32_t the_link;
};

struct cirq_events {
	uint32_t num_reg;
	uint32_t spi_start;
	uint32_t num_of_events;
	uint32_t *wakeup_events;
	struct cirq_reg table[CIRQ_REG_NUM];
	uint32_t dist_base;
	uint32_t cirq_base;
	uint32_t used_reg_head;
};

#endif /* end of __MTK_CIRQ__ */

