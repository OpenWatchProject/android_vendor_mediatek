/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <mt_lp_rm.h>
#include <mtk_plat_common.h>

struct _mt_resource_manager_ {
	unsigned int constraint_num;
	struct mt_resource_constraint *low;
	struct mt_resource_manager *plat_rm;
};

static struct _mt_resource_manager_ plat_mt_rm;

int mt_lp_resource_manager_register(struct mt_resource_manager *rm)
{
	int i = 0;
	int level = -1;
	struct mt_resource_constraint *const *con;

	/* Resource manager allow setup when atf in initial stage.
	 * There's a single core in this stage and avoid to race condition
	 */
	if (!rm || !rm->consts
		|| plat_mt_rm.plat_rm
		|| (!mtk_lk_stage))
		return -1;

	for (i = 0, con = rm->consts; *con ; i++, con++) {
		if ((*con)->level > level) {
			level = (*con)->level;
			plat_mt_rm.low = *con;
		}

		if ((*con)->init)
			(*con)->init();
	}

	plat_mt_rm.plat_rm = rm;
	plat_mt_rm.constraint_num = i;

	return 0;
}

int mt_lp_rm_do_hwctrl(int type, int set, void *priv)
{
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;
	int res = 0;

	if (!rm || !rm->hwctrl)
		return -1;

	res = rm->hwctrl(type, set, priv);

	return res;
}

int mt_lp_rm_do_constraint(int constraint_id, int cpuid, int stateid)
{
	struct mt_resource_constraint const *con;
	int res = 0;

	if (!plat_mt_rm.plat_rm ||
		(constraint_id < 0) ||
		(constraint_id >= plat_mt_rm.constraint_num))
		return -1;

	con = plat_mt_rm.plat_rm->consts[constraint_id];

	if (con && con->run)
		res = con->run(cpuid, stateid);
	else
		res = -1;

	return res;
}

int mt_lp_rm_reset_constraint(int constraint_id, int cpuid, int stateid)
{
	struct mt_resource_constraint const *con;
	int res = 0;

	if (!plat_mt_rm.plat_rm ||
		(constraint_id < 0) ||
		(constraint_id >= plat_mt_rm.constraint_num))
		return -1;

	con = plat_mt_rm.plat_rm->consts[constraint_id];

	if (con && con->reset)
		res = con->reset(cpuid, stateid);
	else
		res = -1;

	return res;
}

int mt_lp_rm_get_status(void const *p, int type, void *priv)
{
	int i = 0;
	int res = 0;
	struct mt_resource_constraint *const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if (!rm)
		return -1;

	for (i = 0, con = rm->consts;
		*con && (*con)->get_status; i++, con++) {
		res = (*con)->get_status(p, type, priv);
		if (res == MT_RM_STATUS_OK)
			break;
	}
	return res;
}

int mt_lp_rm_find_and_run_constraint(int IsRun, int idx,
					     int cpuid, int stateid,
					     void *priv)
{
	int i, res = -1;
	struct mt_resource_constraint *const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if (!rm || (idx < 0))
		return -1;

	if (rm->update) {
		i = rm->update(rm->consts, plat_mt_rm.constraint_num,
					stateid, priv);
		if (i)
			return i;
	}

	for (i = idx, con = (rm->consts + idx); *con ; i++, con++) {
		if ((*con)->is_valid(cpuid, stateid)) {
			res = i;

			if (IsRun && (*con)->run)
				(*con)->run(cpuid, stateid);

			break;
		}
	}
	return res;
}

unsigned int mt_lp_rm_constraint_allow(int constraint_id, int stateid)
{
	struct mt_resource_constraint const *con;
	unsigned int res = 0;

	if (!plat_mt_rm.plat_rm ||
		(constraint_id < 0) ||
		(constraint_id >= plat_mt_rm.constraint_num))
		return res;

	con = plat_mt_rm.plat_rm->consts[constraint_id];

	if (con && con->allow)
		res = con->allow(stateid);
	return res;
}

int mt_lp_rm_do_constraint_update(int constraint_id, int stateid,
					  int type, void const *p)
{
	int res = -1;
	struct mt_resource_constraint const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if ((constraint_id < 0) ||
		(constraint_id >= plat_mt_rm.constraint_num))
		return -1;

	con = rm->consts[constraint_id];

	if (con && con->update)
		res = con->update(stateid, type, p);
	return res;
}

int mt_lp_rm_do_update(int stateid, int type, void const *p)
{
	int i = 0;
	int res = 0;
	struct mt_resource_constraint *const *con;
	struct mt_resource_manager *rm = plat_mt_rm.plat_rm;

	if (!rm)
		return -1;

	for (i = 0, con = rm->consts;
		*con && (*con)->update; i++, con++) {
		res = (*con)->update(stateid, type, p);
		if (res != MT_RM_STATUS_OK)
			break;
	}
	return res;
}

int mt_lp_rm_do_minimum_constraint(int cpu, int stateid)
{
	if (!plat_mt_rm.low)
		return -1;
	plat_mt_rm.low->run(cpu, stateid);
	return 0;
}

