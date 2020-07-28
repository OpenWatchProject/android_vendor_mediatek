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
#define VCORE_BASE_UV 51875 /* PMIC MT6357 */
#define VOLT_TO_PMIC_VAL(volt)  (((volt) - VCORE_BASE_UV + 625 - 1) / 625) /* ((((volt) - 700 * 100 + 625 - 1) / 625) */
#define PMIC_VAL_TO_VOLT(pmic)  (((pmic) * 625) + VCORE_BASE_UV) /* (((pmic) * 625) / 100 + 700) */

#define DEFAULT_VOLT_VSRAM      (100000)
#define DEFAULT_VOLT_VCORE      (100000)
#define NR_PMIC_WRAP_CMD (NR_IDX_ALL)
#define MAX_RETRY_COUNT (100)
#define SPM_DATA_SHIFT (16)

#define BUCK_VCORE_ELR0 0x152A
#define TOP_SPI_CON0 0x0448
#define BUCK_VPROC_CON0 0x1488
#define LDO_VSRAM_PROC_CON0 0x19B2
#define LDO_VSRAM_OTHERS_CON0 0x19D0

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

#if 1
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0]    = {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(80000)),},
		._[CMD_1]    = {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(75000)),},
		._[CMD_2]    = {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(70000)),},
		._[CMD_3]    = {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(70000)),},
		._[CMD_4]    = {BUCK_VCORE_ELR0, _BITS_(6:0, VOLT_TO_PMIC_VAL(65000)),},
		._[CMD_6]    = {TOP_SPI_CON0, _BITS_(0:0, 1),},
		._[CMD_7]    = {TOP_SPI_CON0, _BITS_(0:0, 0),},
		._[CMD_8]    = {BUCK_VPROC_CON0, _BITS_(1:0, 3),},
		._[CMD_9]    = {BUCK_VPROC_CON0, _BITS_(1:0, 1),},
		._[CMD_10]   = {LDO_VSRAM_PROC_CON0, _BITS_(1:0, 3),},
		._[CMD_11]   = {LDO_VSRAM_PROC_CON0, _BITS_(1:0, 1),},
		._[CMD_12]   = {BUCK_VPROC_CON0, _BITS_(1:0, 0),},
		._[CMD_13]   = {BUCK_VPROC_CON0, _BITS_(1:0, 1),},
		._[CMD_14]   = {LDO_VSRAM_PROC_CON0, _BITS_(1:0, 0),},
		._[CMD_15]   = {LDO_VSRAM_PROC_CON0, _BITS_(1:0, 1),},
		.nr_idx = NR_IDX_ALL,
	},
#endif
};

void _mt_spm_pmic_table_init(void)
{
#if 1
	struct pmic_wrap_cmd pwrap_cmd_default[NR_PMIC_WRAP_CMD] = {
		{(uint32_t)SPM_DVFS_CMD0, (uint32_t)SPM_DVFS_CMD0,},
		{(uint32_t)SPM_DVFS_CMD1, (uint32_t)SPM_DVFS_CMD1,},
		{(uint32_t)SPM_DVFS_CMD2, (uint32_t)SPM_DVFS_CMD2,},
		{(uint32_t)SPM_DVFS_CMD3, (uint32_t)SPM_DVFS_CMD3,},
		{(uint32_t)SPM_DVFS_CMD4, (uint32_t)SPM_DVFS_CMD4,},
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
	};

	memcpy(pw.addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));
#endif
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

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB | MD_BCLK_CG_EN_LSB);
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

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB | MD_BCLK_CG_EN_LSB);
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

void mt_spm_pmic_wrap_set_mrv_cmd(enum pmic_wrap_phase_id phase)
{
	if (phase >= NR_PMIC_WRAP_PHASE)
		return;

	pw.set[phase]._[CMD_10].cmd_addr = LDO_VSRAM_OTHERS_CON0;
	pw.set[phase]._[CMD_11].cmd_addr = LDO_VSRAM_OTHERS_CON0;
	pw.set[phase]._[CMD_14].cmd_addr = LDO_VSRAM_OTHERS_CON0;
	pw.set[phase]._[CMD_15].cmd_addr = LDO_VSRAM_OTHERS_CON0;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB | MD_BCLK_CG_EN_LSB);
	if (pw.phase == phase) {
		mmio_write_32(pw.addr[CMD_10].cmd_addr,
				(pw.set[phase]._[CMD_10].cmd_addr << SPM_DATA_SHIFT) |
					pw.set[phase]._[CMD_10].cmd_wdata);

		mmio_write_32(pw.addr[CMD_11].cmd_addr,
				(pw.set[phase]._[CMD_11].cmd_addr << SPM_DATA_SHIFT) |
					pw.set[phase]._[CMD_11].cmd_wdata);

		mmio_write_32(pw.addr[CMD_14].cmd_addr,
				(pw.set[phase]._[CMD_14].cmd_addr << SPM_DATA_SHIFT) |
					pw.set[phase]._[CMD_14].cmd_wdata);

		mmio_write_32(pw.addr[CMD_15].cmd_addr,
				(pw.set[phase]._[CMD_15].cmd_addr << SPM_DATA_SHIFT) |
					pw.set[phase]._[CMD_15].cmd_wdata);
	}
}

