#ifndef __CCCI_H__
#define __CCCI_H__

enum MD_REG_ID {
	MD_REG_AP_MDSRC_REQ = 0,
	MD_REG_PC_MONITOR,
	MD_REG_PLL_REG,
	MD_REG_BUS,
	MD_REG_MDMCU_BUSMON,
	MD_REG_MDINFRA_BUSMON,
	MD_REG_ECT,
	MD_REG_TOPSM_REG,
	MD_REG_MD_RGU_REG,
	MD_REG_OST_STATUS,
	MD_REG_CSC_REG,
	MD_REG_ELM_REG,
};

unsigned int ccci_md_dbgsys_config(uint64_t reg_id, uint64_t value);

#endif
