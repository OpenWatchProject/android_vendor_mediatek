#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <mt_spm_pmic_wrap.h>
#include <plat_pm.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*
 * BIT Operation
 */
#define _BIT_(_bit_)                    (unsigned)(1 << (_bit_))
#define _BITS_(_bits_, _val_)         \
	((((unsigned) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1)) & ((_val_)<<((0) ? _bits_)))
#define _BITMASK_(_bits_)               (((unsigned) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1))
#define _GET_BITS_VAL_(_bits_, _val_)   (((_val_) & (_BITMASK_(_bits_))) >> ((0) ? _bits_))

/*
 * PMIC_WRAP
 */
#define VCORE_BASE_UV 40000 /* PMIC MT6359 */
#define VOLT_TO_PMIC_VAL(volt)  (((volt) - VCORE_BASE_UV + 625 - 1) / 625)
#define PMIC_VAL_TO_VOLT(pmic)  (((pmic) * 625) + VCORE_BASE_UV)

#define VSX_BASE_UV 80000 /* PMIC MT6359 */
#define VOLT_TO_PMIC_VSX_VAL(volt)  (((volt) - VCORE_BASE_UV + 1250 - 1) / 1250)
#define PMIC_VSX_VAL_TO_VOLT(pmic)  (((pmic) * 1250) + VCORE_BASE_UV)

#define DEFAULT_VOLT_VSRAM      (100000)
#define DEFAULT_VOLT_VCORE      (100000)
#define NR_PMIC_WRAP_CMD (NR_IDX_ALL)
#define MAX_RETRY_COUNT (100)
#define SPM_DATA_SHIFT (16)

#define BUCK_VCORE_ELR0			0x1534
#define TOP_SPI_CON0			0x0456
#define BUCK_VPROC1_CON0		0x1708
#define BUCK_VPROC2_CON0		0x1788
#define LDO_VSRAM_PROC1_CON0	0x1E88
#define LDO_VSRAM_PROC2_CON0	0x1EA6
#define BUCK_VS1_ELR0			0x1834
#define BUCK_VS2_ELR0			0x18B4
#define BUCK_VMODEM_ELR0		0x15A6


struct pmic_wrap_cmd {
	unsigned long cmd_addr;
	unsigned long cmd_wdata;
};

struct pmic_wrap_setting {
	enum pmic_wrap_phase_id phase;
	struct pmic_wrap_cmd addr[NR_PMIC_WRAP_CMD];
	struct {
		struct {
			unsigned long cmd_addr;
			unsigned long cmd_wdata;
		} _[NR_PMIC_WRAP_CMD];
		const int nr_idx;
	} set[NR_PMIC_WRAP_PHASE];
};

static struct pmic_wrap_setting pw = {
	.phase = NR_PMIC_WRAP_PHASE,    /* invalid setting for init */
	.addr = {{0, 0} },
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0]	= {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(62500)),}, /* 0.625 V */
		._[CMD_1]	= {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(70000)),}, /* 0.700 V */
		._[CMD_2]	= {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(75000)),}, /* 0.750 V */
		._[CMD_3]	= {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(80000)),}, /* 0.800 V */
		._[CMD_4]	= {TOP_SPI_CON0, _BITS_(0:0, 1),},
		._[CMD_5]	= {TOP_SPI_CON0, _BITS_(0:0, 0),},
		._[CMD_6]	= {BUCK_VPROC1_CON0, _BITS_(1:0, 1),},	/* Normal */
		._[CMD_7]	= {BUCK_VPROC1_CON0, _BITS_(1:0, 0),},	/* Off */
		._[CMD_8]	= {BUCK_VPROC1_CON0, _BITS_(1:0, 3),},	/* Low power */
		._[CMD_9]	= {BUCK_VPROC2_CON0, _BITS_(1:0, 1),},	/* Normal */
		._[CMD_10]	= {BUCK_VPROC2_CON0, _BITS_(1:0, 0),},	/* Off */
		._[CMD_11]	= {BUCK_VPROC2_CON0, _BITS_(1:0, 3),},	/* Low power */
		._[CMD_12]	= {LDO_VSRAM_PROC1_CON0, _BITS_(15:0, 0x1C25),},	/* Normal */
		._[CMD_13]	= {LDO_VSRAM_PROC1_CON0, _BITS_(15:0, 0x1C24),},	/* Off */
		._[CMD_14]	= {LDO_VSRAM_PROC1_CON0, _BITS_(15:0, 0x1C27),},	/* Low power */
		._[CMD_15]	= {LDO_VSRAM_PROC2_CON0, _BITS_(15:0, 0x1C25),},	/* Normal */
		._[CMD_16]	= {LDO_VSRAM_PROC2_CON0, _BITS_(15:0, 0x1C24),},	/* Off */
		._[CMD_17]	= {LDO_VSRAM_PROC2_CON0, _BITS_(15:0, 0x1C27),},	/* Low power */
		._[CMD_18]	= {BUCK_VS1_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VSX_VAL(190000)),}, /* 1.9 V */
		._[CMD_19]	= {BUCK_VS1_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VSX_VAL(200000)),}, /* 2.0 V */
		._[CMD_20]	= {BUCK_VS2_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VSX_VAL(135000)),}, /* 1.35 V */
		._[CMD_21]	= {BUCK_VS2_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VSX_VAL(138000)),}, /* 1.38 V */
		._[CMD_22]	= {BUCK_VS2_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VSX_VAL(140000)),}, /* 1.4 V */
		.nr_idx = NR_IDX_ALL,
	},
};

void _mt_spm_pmic_table_init(void)
{
	struct pmic_wrap_cmd pwrap_cmd_default[NR_PMIC_WRAP_CMD] = {
		{(uint32_t)SPM_DVFS_CMD0, (uint32_t)SPM_DVFS_CMD0,},
		{(uint32_t)SPM_DVFS_CMD1, (uint32_t)SPM_DVFS_CMD1,},
		{(uint32_t)SPM_DVFS_CMD2, (uint32_t)SPM_DVFS_CMD2,},
		{(uint32_t)SPM_DVFS_CMD3, (uint32_t)SPM_DVFS_CMD3,},
		{(uint32_t)SPM_DVFS_CMD4, (uint32_t)SPM_DVFS_CMD4,},
		{(uint32_t)SPM_DVFS_CMD5, (uint32_t)SPM_DVFS_CMD5,},
		{(uint32_t)SPM_DVFS_CMD6, (uint32_t)SPM_DVFS_CMD6,},
		{(uint32_t)SPM_DVFS_CMD7, (uint32_t)SPM_DVFS_CMD7,},
		{(uint32_t)SPM_DVFS_CMD8, (uint32_t)SPM_DVFS_CMD8,},
		{(uint32_t)SPM_DVFS_CMD9, (uint32_t)SPM_DVFS_CMD9,},
		{(uint32_t)SPM_DVFS_CMD10, (uint32_t)SPM_DVFS_CMD10,},
		{(uint32_t)SPM_DVFS_CMD11, (uint32_t)SPM_DVFS_CMD11,},
		{(uint32_t)SPM_DVFS_CMD12, (uint32_t)SPM_DVFS_CMD12,},
		{(uint32_t)SPM_DVFS_CMD13, (uint32_t)SPM_DVFS_CMD13,},
		{(uint32_t)SPM_DVFS_CMD14, (uint32_t)SPM_DVFS_CMD14,},
		{(uint32_t)SPM_DVFS_CMD15, (uint32_t)SPM_DVFS_CMD15,},
		{(uint32_t)SPM_DVFS_CMD16, (uint32_t)SPM_DVFS_CMD16,},
		{(uint32_t)SPM_DVFS_CMD17, (uint32_t)SPM_DVFS_CMD17,},
		{(uint32_t)SPM_DVFS_CMD18, (uint32_t)SPM_DVFS_CMD18,},
		{(uint32_t)SPM_DVFS_CMD19, (uint32_t)SPM_DVFS_CMD19,},
		{(uint32_t)SPM_DVFS_CMD20, (uint32_t)SPM_DVFS_CMD20,},
		{(uint32_t)SPM_DVFS_CMD21, (uint32_t)SPM_DVFS_CMD21,},
		{(uint32_t)SPM_DVFS_CMD22, (uint32_t)SPM_DVFS_CMD22,},
	};

	memcpy(pw.addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));
}

void mt_spm_pmic_wrap_set_phase(enum pmic_wrap_phase_id phase)
{
	int idx;

	if (phase >= NR_PMIC_WRAP_PHASE)
		return;

	if (pw.phase == phase)
		return;

	if (pw.addr[0].cmd_addr == 0)
		_mt_spm_pmic_table_init();

	pw.phase = phase;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);
	for (idx = 0; idx < pw.set[phase].nr_idx; idx++)
		mmio_write_32(pw.addr[idx].cmd_addr,
				(pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT) | (pw.set[phase]._[idx].cmd_wdata));
}

void mt_spm_pmic_wrap_set_cmd(enum pmic_wrap_phase_id phase, uint32_t idx, uint32_t cmd_wdata)
{                /* just set wdata value */
	if (phase >= NR_PMIC_WRAP_PHASE)
		return;

	if (idx >= pw.set[phase].nr_idx)
		return;

	pw.set[phase]._[idx].cmd_wdata = cmd_wdata;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);
	if (pw.phase == phase)
		mmio_write_32(pw.addr[idx].cmd_addr,
				(pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT) | cmd_wdata);

}

uint64_t mt_spm_pmic_wrap_get_cmd(enum pmic_wrap_phase_id phase, uint32_t idx)
{                /* just get wdata value */
	if (phase >= NR_PMIC_WRAP_PHASE)
		return 0;

	if (idx >= pw.set[phase].nr_idx)
		return 0;

	return pw.set[phase]._[idx].cmd_wdata;
}

void mt_spm_dump_pmic_warp_reg(void)
{
	uint32_t temp;
	uint32_t i;

	for (i = 0; i <= 0x3E8; i += 0x4) {
		temp = mmio_read_32(PMIC_WRAP_BASE + i);
		ERROR("#@# pmic_wrap_reg(0x%x) = 0x%x\n", i, temp);
	}

	for (i = 0xC00; i <= 0xC28; i += 0x4) {
		temp = mmio_read_32(PMIC_WRAP_BASE + i);
		ERROR("#@# pmic_wrap_reg(0x%x) = 0x%x\n", i, temp);
	}

	for (i = 0xF00; i <= 0xF54; i += 0x4) {
		temp = mmio_read_32(PMIC_WRAP_BASE + i);
		ERROR("#@# pmic_wrap_reg(0x%x) = 0x%x\n", i, temp);
	}
}
