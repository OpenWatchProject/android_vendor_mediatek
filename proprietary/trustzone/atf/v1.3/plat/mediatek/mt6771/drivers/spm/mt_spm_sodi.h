#ifndef __MT_SPM_SODI_H__
#define __MT_SPM_SODI_H__

void spm_sodi_args(__uint64_t x1, __uint64_t x2, __uint64_t x3);
void spm_sodi(void);
void spm_sodi_finish(void);

enum sodi_smc_cmd {
	SODI_SMC_CMD_0,
	SODI_SMC_CMD_1,
	SODI_SMC_CMD_2,
	NUM_SODI_SMC_CMD,
};

#endif /* __MT_SPM_SODI_H__ */
