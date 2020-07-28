#ifndef __MT_SPM_VCOREFS__H__
#define __MT_SPM_VCOREFS__H__

void spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_request_dvfs_opp(uint64_t id, uint64_t reg);

enum vcorefs_smc_cmd {
	VCOREFS_SMC_CMD_0,
	VCOREFS_SMC_CMD_1,
	VCOREFS_SMC_CMD_2,
	VCOREFS_SMC_CMD_3,
	NUM_VCOREFS_SMC_CMD,
};

enum dvfsrc_channel {
	DVFSRC_CHANNEL_1 = 1,
	DVFSRC_CHANNEL_2,
	DVFSRC_CHANNEL_3,
	DVFSRC_CHANNEL_4,
	NUM_DVFSRC_CHANNEL,
};

#define DVFSRC_MD_TURBO    (DVFSRC_BASE + 0x00DC)
#define DVFSRC_SW_REQ3     (DVFSRC_BASE + 0x000C)
#define DDR_SW_AP_SHIFT    12
#define DDR_SW_AP_MASK     0x7


#endif /* __MT_SPM_VCOREFS__H__ */
