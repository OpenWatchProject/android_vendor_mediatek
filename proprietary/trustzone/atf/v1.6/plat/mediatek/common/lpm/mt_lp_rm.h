#ifndef __MT_LP_RM__
#define __MT_LP_RM__

#define MT_RM_STATUS_OK		(0)
#define MT_RM_STATUS_BAD	(-1)
#define MT_RM_STATUS_STOP	(-2)

enum PLAT_MT_LPM_RC_UPDATE_TYPE {
	PLAT_RC_UPDATE_CONDITION,
	PLAT_RC_UPDATE_STATUS,
	PLAT_RC_UPDATE_REMAIN_IRQS,
	PLAT_RC_IS_FMAUDIO,
	PLAT_RC_IS_ADSP,
	PLAT_RC_IS_VALID,
	PLAT_RC_FW_READY,
	PLAT_RC_IS_COND_CHECK,
	PLAT_RC_IS_COND_BLOCK,
	PLAT_RC_IS_BLOCK_LATCH,
	PLAT_RC_COND_BLOCK_DETAIL,
	PLAT_RC_ENTER_CNT,
};

enum PLAT_MT_LPM_HW_CTRL_TYPE {
	PLAT_AP_MDSRC_REQ,
	PLAT_AP_MDSRC_ACK,
};

struct mt_resource_constraint {
	/* Resource's constraint level to determine regulator's level
	 * The higher level will be the maximum of power's conservation
	 */
	int level;

	/* Init resource constraint when attachment */
	int (*init)(void);

	/* Determine the resource constraint */
	int (*is_valid)(int cpu, int stateid);

	/* Update constraint status */
	int (*update)(int stateid, int type, const void *p);

	/* Into the resource constraint */
	int (*run)(int cpu, int stateid);

	/* Reset the resource constraint */
	int (*reset)(int cpu, int stateid);

	/* Get the status of resource constraint (is_valid) */
	int (*get_status)(const void *p, int type, void *priv);

	unsigned int (*allow)(int stateid);
};

struct mt_resource_manager {
	/* update the platform resource information */
	int (*update)(struct mt_resource_constraint **con,
					int num, int stateid, void *priv);

	/* control for platform hardware access */
	int (*hwctrl)(int type, int set, void *priv);

	struct mt_resource_constraint **consts;
};

#define mt_lp_rm_find_constraint(priv)\
	mt_lp_rm_find_and_run_constraint(0, 0, 0, 0, priv)

int mt_lp_resource_manager_register(struct mt_resource_manager *rm);

int mt_lp_rm_find_and_run_constraint(int IsRun, int idx,
					int cpuid, int stateid,
					void *priv);

int mt_lp_rm_do_hwctrl(int type, int set, void *priv);

int mt_lp_rm_do_constraint(int constraint_id, int cpuid, int stateid);

int mt_lp_rm_reset_constraint(int constraint_id, int cpuid, int stateid);

int mt_lp_rm_get_status(void const *p, int type, void *priv);

unsigned int mt_lp_rm_constraint_allow(int constraint_id, int stateid);

int mt_lp_rm_do_update(int stateid, int type, void const *p);

int mt_lp_rm_do_constraint_update(int constraint_id, int stateid,
					int type, void const *p);

int mt_lp_rm_init(void);

int mt_lp_rm_do_minimum_constraint(int cpu, int stateid);
#endif
