/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __CHIP_PLF_INIT_H__
#define __CHIP_PLF_INIT_H__

/*
 *   MET Chip External Symbol
 */

/*
 *   VCORE DVFS
 */
#include <mtk_spm.h>
#include <mtk_vcorefs_manager.h>
#include <mtk_vcorefs_governor.h>
#include <mtk_spm_vcore_dvfs.h>

extern void (*spm_vcorefs_register_handler_symbol)(vcorefs_handler_t handler);
extern void (*vcorefs_register_req_notify_symbol)(vcorefs_req_handler_t handler);

extern char *(*governor_get_kicker_name_symbol)(int id);
extern int (*vcorefs_get_opp_info_num_symbol)(void);
extern char **(*vcorefs_get_opp_info_name_symbol)(void);
extern unsigned int *(*vcorefs_get_opp_info_symbol)(void);
extern int (*vcorefs_get_src_req_num_symbol)(void);
extern char **(*vcorefs_get_src_req_name_symbol)(void);
extern unsigned int *(*vcorefs_get_src_req_symbol)(void);
extern int (*vcorefs_enable_debug_isr_symbol)(bool);
extern int (*vcorefs_get_num_opp_symbol)(void);
extern int *kicker_table_symbol;

extern char *governor_get_kicker_name(int id);
extern int vcorefs_get_opp_info_num(void);
extern char **vcorefs_get_opp_info_name(void);
extern unsigned int *vcorefs_get_opp_info(void);
extern int vcorefs_get_src_req_num(void);
extern char **vcorefs_get_src_req_name(void);
extern unsigned int *vcorefs_get_src_req(void);
extern int vcorefs_enable_debug_isr(bool);
extern int vcorefs_get_num_opp(void);

extern int (*vcorefs_get_hw_opp_symbol)(void);
extern int (*vcorefs_get_curr_vcore_symbol)(void);
extern int (*vcorefs_get_curr_ddr_symbol)(void);
extern u32 (*spm_vcorefs_get_MD_status_symbol)(void);

extern struct metdevice met_vcoredvfs;

#endif /*__CHIP_PLF_INIT_H__*/
