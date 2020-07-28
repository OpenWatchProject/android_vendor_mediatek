#ifndef __MT_SPM_DPIDLE__H__
#define __MT_SPM_DPIDLE__H__

void spm_dpidle_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_sleep_dpidle_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_dpidle(void);
void spm_dpidle_finish(void);

enum dpidle_smc_cmd {
	DPIDLE_SMC_CMD_0,
	DPIDLE_SMC_CMD_1,
	DPIDLE_SMC_CMD_2,
	NUM_DPIDLE_SMC_CMD,
};

#endif /* __MT_SPM_DPIDLE__H__ */
