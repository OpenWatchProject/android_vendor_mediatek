/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/kallsyms.h>
#include "met_drv.h"
#include "met_api_tbl.h"
#include "interface.h"
#include "chip_plf_init.h"

#undef	DEBUG

/*
 *   VCORE DVFS
 */
int  (*vcorefs_get_opp_info_num_symbol)(void);
char ** (*vcorefs_get_opp_info_name_symbol)(void);
unsigned int * (*vcorefs_get_opp_info_symbol)(void);
int  (*vcorefs_get_src_req_num_symbol)(void);
char ** (*vcorefs_get_src_req_name_symbol)(void);
unsigned int * (*vcorefs_get_src_req_symbol)(void);
u32 (*spm_vcorefs_get_MD_status_symbol)(void);

void (*spm_vcorefs_register_handler_symbol)(vcorefs_handler_t handler);
void (*vcorefs_register_req_notify_symbol)(vcorefs_req_handler_t handler);

char *(*governor_get_kicker_name_symbol)(int id);
int (*vcorefs_enable_debug_isr_symbol)(bool);

int (*vcorefs_get_hw_opp_symbol)(void);
int (*vcorefs_get_curr_vcore_symbol)(void);
int (*vcorefs_get_curr_ddr_symbol)(void);
int (*vcorefs_get_num_opp_symbol)(void);
int *kicker_table_symbol;

static int met_chip_symbol_get(void)
{
#define _MET_SYMBOL_GET(_func_name_) \
	do { \
		_func_name_##_symbol = (void *)symbol_get(_func_name_); \
		if (_func_name_##_symbol == NULL) { \
			pr_debug("MET ext. symbol : %s is not found!\n", #_func_name_); \
			PR_BOOTMSG_ONCE("MET ext. symbol : %s is not found!\n", #_func_name_); \
		} \
	} while (0)

	/* VCORE DVFS */
	_MET_SYMBOL_GET(vcorefs_get_opp_info_num);
	_MET_SYMBOL_GET(vcorefs_get_opp_info_name);
	_MET_SYMBOL_GET(vcorefs_get_opp_info);
	_MET_SYMBOL_GET(vcorefs_get_src_req_num);
	_MET_SYMBOL_GET(vcorefs_get_src_req_name);
	_MET_SYMBOL_GET(vcorefs_get_src_req);

	_MET_SYMBOL_GET(spm_vcorefs_register_handler);
	_MET_SYMBOL_GET(vcorefs_register_req_notify);

	_MET_SYMBOL_GET(governor_get_kicker_name);
	_MET_SYMBOL_GET(vcorefs_enable_debug_isr);

	_MET_SYMBOL_GET(vcorefs_get_hw_opp);
	_MET_SYMBOL_GET(vcorefs_get_curr_vcore);
	_MET_SYMBOL_GET(vcorefs_get_curr_ddr);
	_MET_SYMBOL_GET(spm_vcorefs_get_MD_status);
	_MET_SYMBOL_GET(vcorefs_get_num_opp);
	_MET_SYMBOL_GET(kicker_table);

	return 0;
}

static int met_chip_symbol_put(void)
{
#define _MET_SYMBOL_PUT(_func_name_) { \
		if (_func_name_##_symbol) { \
			symbol_put(_func_name_); \
			_func_name_##_symbol = NULL; \
		} \
	}

	/* VCORE DVFS */
	_MET_SYMBOL_PUT(vcorefs_get_opp_info_num);
	_MET_SYMBOL_PUT(vcorefs_get_opp_info_name);
	_MET_SYMBOL_PUT(vcorefs_get_opp_info);
	_MET_SYMBOL_PUT(vcorefs_get_src_req_num);
	_MET_SYMBOL_PUT(vcorefs_get_src_req_name);
	_MET_SYMBOL_PUT(vcorefs_get_src_req);

	_MET_SYMBOL_PUT(spm_vcorefs_register_handler);
	_MET_SYMBOL_PUT(vcorefs_register_req_notify);

	_MET_SYMBOL_PUT(governor_get_kicker_name);
	_MET_SYMBOL_PUT(vcorefs_enable_debug_isr);

	_MET_SYMBOL_PUT(vcorefs_get_hw_opp);
	_MET_SYMBOL_PUT(vcorefs_get_curr_vcore);
	_MET_SYMBOL_PUT(vcorefs_get_curr_ddr);
	_MET_SYMBOL_PUT(spm_vcorefs_get_MD_status);
	_MET_SYMBOL_PUT(vcorefs_get_num_opp);
	_MET_SYMBOL_PUT(kicker_table);

	return 0;
}

int chip_plf_init(void)
{
	/*initial met chip external symbol*/
	met_chip_symbol_get();

	met_register(&met_vcoredvfs);

	return 0;
}

void chip_plf_exit(void)
{
	/*release met chip external symbol*/
	met_chip_symbol_put();

	met_deregister(&met_vcoredvfs);
}
