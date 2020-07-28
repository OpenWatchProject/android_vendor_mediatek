/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __PLAT_TEEI_INNER_H__
#define __PLAT_TEEI_INNER_H__

extern uint32_t TEEI_STATE;
extern uint32_t uart_apc_num;
extern void mt_irq_set_sens(unsigned int base, unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
#ifndef LEGACY_GIC_SUPPORT
extern void gicc_write_grpen1_el1(uint64_t val);
#endif
extern void gicd_v3_do_wait_for_rwp(unsigned int gicd_base);
extern void mt_log_secure_os_print(int c);
extern uint64_t gic_mpidr_to_affinity(uint64_t mpidr);
extern int gic_populate_rdist(unsigned int *rdist_base);

#endif
