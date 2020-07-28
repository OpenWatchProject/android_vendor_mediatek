/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __PLAT_TEEI_H__
#define __PLAT_TEEI_H__

#include <platform.h>

/*secure world FIQ */
extern unsigned int SEC_APP_INTR;
extern unsigned int SEC_DRV_INTR;
extern unsigned int SEC_RDRV_INTR;
extern unsigned int SEC_TDUMP_INTR;

extern void migrate_gic_context(uint32_t secure_state);
extern void disable_group(unsigned int grp);
extern void enable_group(unsigned int grp);
extern void trigger_soft_intr(unsigned int id);
extern void prepare_gic_for_nsec_boot(void);
extern void prepare_gic_for_sec_boot(void);
extern void teei_gic_setup(void);
extern void teei_ack_gic(void);
extern void teei_triggerSgiDump(void);
extern void plat_teei_dump(void);

extern uint32_t plat_get_rnd(uint32_t *val);
extern uint32_t plat_ic_acknowledge_interrupt(void);
extern void plat_ic_end_of_interrupt(uint32_t id);

extern uint32_t teei_rng(void);
extern unsigned int teei_get_fp_id(void);
extern int set_drm_int_secure(unsigned int id);
extern int set_drm_int_nonsecure(unsigned int id);
#endif
