#ifndef __MT_SPM_VCOREFS__H__
#define __MT_SPM_VCOREFS__H__

void spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_request_dvfs_opp(__uint64_t id, __uint64_t reg);

enum vcorefs_smc_cmd {
	VCOREFS_SMC_CMD_0,
	VCOREFS_SMC_CMD_1,
	VCOREFS_SMC_CMD_2,
	NUM_VCOREFS_SMC_CMD,
};

#endif /* __MT_SPM_VCOREFS__H__ */
