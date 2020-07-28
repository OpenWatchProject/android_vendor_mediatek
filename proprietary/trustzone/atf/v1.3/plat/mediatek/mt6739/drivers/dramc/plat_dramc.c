#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <plat_dramc.h>
#include <interrupt_mgmt.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <delay_timer.h>

static unsigned int chc_sref_dummy_read = 0;
static unsigned int chd_sref_dummy_read = 0;

static void apmcu_dram_chcd_on(void);
static void apmcu_dram_chcd_off(void);
static int spm_mtcmos_ctrl_dpy_ch2_apmcu(int state);
static int spm_mtcmos_ctrl_dpy_ch3_apmcu(int state);

uint32_t sip_dram_smc_chcd(unsigned int OnOff)
{
	if (OnOff) {
		apmcu_dram_chcd_on();
	} else {
		apmcu_dram_chcd_off();
	}

	return MTK_SIP_E_SUCCESS;
}

static void apmcu_dram_chcd_on(void)
{
	unsigned int temp;

	/* DCS DRAM CHC/D turn on flow start */

	/* check DVFS status, and wait until done */
	do {
		temp = (mmio_read_32(APMCU_PCM_REG0_DATA) >> 25) & 0x1;
	} while (temp == 0x1);

	spm_mtcmos_ctrl_dpy_ch2_apmcu(1);
	spm_mtcmos_ctrl_dpy_ch3_apmcu(1);

	/* test add for supporting DVFS in DCS */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON4) | (0x3 << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON4, temp); // enable CHC/D bclk en
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON4) | (0x3 << 10);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON4, temp); // enable CHC/D CHB restore en

	udelay(1);

	/* set CG to enable freerun 26M for CHC/D */
	temp = mmio_read_32(APMCU_CLK_SCP_CFG_2) | (0x3 << 19);
	mmio_write_32(APMCU_CLK_SCP_CFG_2, temp);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) & ~(0xc << 4);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2) | (0xc << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp);

	udelay(1);

	/* disable CHC/D CHB restore en */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON4) & ~(0x3 << 10);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON4, temp);

	udelay(1);

	/* disable CHC/D bclk en */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON4) & ~(0x3 << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON4, temp);

	/* switch PLL control mode from low power mode to normal mode */
	temp = mmio_read_32(APMCU_DDRPHY2AO_BASE + (0x9d << 2)) & ~(0x3 << 7);
	mmio_write_32(APMCU_DDRPHY2AO_BASE + (0x9d << 2), temp);
	temp = mmio_read_32(APMCU_DDRPHY3AO_BASE + (0x9d << 2)) & ~(0x3 << 7);
	mmio_write_32(APMCU_DDRPHY3AO_BASE + (0x9d << 2), temp);

	udelay(20);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2) & ~(0xc << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON)  | (0xc << 4);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);

	/* CHB from S1 to Sidle */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) | (0xc << 28);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) | (0xc << 24);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2) | (0xc << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp);

	/* test add for supporting DVFS in DCS */
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) & ~(0x3 << 4);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp | (0x1 << 4));
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp | 0x1);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) & ~(0x3 << 4);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp | (0x1 << 4));
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp | 0x1);
	udelay(1);
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp);

	/* test add for supporting DVFS in DCS */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) | (0xc << 16);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);

	udelay(1);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) | (0xc << 20);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);

	udelay(1);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) & ~(0xc << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) & ~(0xc << 4);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2) & ~(0xc << 8);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp);

	udelay(1);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) & ~(0xc << 12);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON) & ~(0xc << 0);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON4) | (0x3 << 12);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON4, temp); // enable precal restore en
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON4) & ~(0x3 << 12);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON4, temp); // disable precal restore en

	/* trigger SW shuffle */
	temp = mmio_read_32(DRAMC_AO_CHC_SHUCTRL);
	mmio_write_32(DRAMC_AO_CHC_SHUCTRL, temp | (0x1 << 31));
	temp = mmio_read_32(DRAMC_AO_CHD_SHUCTRL);
	mmio_write_32(DRAMC_AO_CHD_SHUCTRL, temp | (0x1 << 31));

	/* polling CH2 SHU END */
	do {
		temp = mmio_read_32(DRAMC_AO_CHC_SHUSTATUS) & 0x1;
	} while (temp != 0x1);

	/* polling CH3 SHU END */
	do {
		temp = mmio_read_32(DRAMC_AO_CHD_SHUSTATUS) & 0x1;
	} while (temp != 0x1);


	/* SW SHU END */
	temp = mmio_read_32(DRAMC_AO_CHC_SHUCTRL) & ~(0x1 << 31);
	mmio_write_32(DRAMC_AO_CHC_SHUCTRL, temp);
	temp = mmio_read_32(DRAMC_AO_CHD_SHUCTRL) & ~(0x1 << 31);
	mmio_write_32(DRAMC_AO_CHD_SHUCTRL, temp);

	/* control DRAMC CHB backup restore function */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL4);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL4, temp |
		(0xc << 0) | (0xc << 4) | (0xc << 8) | (0xc << 12));

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL, temp |
		(0xc << 0) | (0xc << 4) | (0xc << 8) | (0xc << 12) |
		(0xc << 16) | (0xc << 20) | (0xc << 24) | (0xc << 28));

	/* switch SPM to SW mode */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2, temp |
		(0xc << 0) | (0xc << 4) | (0xc << 8) | (0x6 << 28));

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2) &
		~((0xc << 12) | (0xc << 16) | (0xc << 20) | (0xc << 24));
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2, temp);

	/* enable dummy read, but disable dummy if exit SREF */
	temp = mmio_read_32(DRAMC_AO_CHC_DUMMY_RD);
	mmio_write_32(DRAMC_AO_CHC_DUMMY_RD, temp | (chc_sref_dummy_read << 21));
	temp = mmio_read_32(DRAMC_AO_CHD_DUMMY_RD);
	mmio_write_32(DRAMC_AO_CHD_DUMMY_RD, temp | (chd_sref_dummy_read << 21));

	//! 20161027 mazar add
	/* enable impedance tracking*/
	temp = mmio_read_32(DRAMC_AO_CHC_BASE + 0x22C) | (0x3 << 30);
	mmio_write_32(DRAMC_AO_CHC_BASE + 0x22C, temp  | (0x1 << 20) );
	temp = mmio_read_32(DRAMC_AO_CHD_BASE + 0x22C) | (0x3 << 30);
	mmio_write_32(DRAMC_AO_CHD_BASE + 0x22C, temp  | (0x1 << 20) );

	/* release bus protect */
	mmio_write_32(APMCU_INFRA_TOPAXI_PROTECTEN_1_CLR, APMCU_DPY_CH2_PROT_BIT_MASK);
	mmio_write_32(APMCU_INFRA_TOPAXI_PROTECTEN_1_CLR, APMCU_DPY_CH3_PROT_BIT_MASK);

	/* finish to turn on DPY CHC/D */

	/* switch EMI clock source to CH C */
	temp = mmio_read_32(0x102101b0);
	mmio_write_32(0x102101b0, temp | 0x100);
}

static void apmcu_dram_chcd_off(void)
{
	unsigned int temp;

	/* switch EMI clock source to CH A */
	temp = mmio_read_32(0x102101b0);
	mmio_write_32(0x102101b0, temp & ~0x100);

	/* DCS DRAM CHC/D turn off flow start */

	/* In DRAM CHB DCS conflict with DVFS, wait DVFS end */
	do {
		temp = (mmio_read_32(APMCU_PCM_REG0_DATA) >> 25) & 0x1;
	} while (temp == 0x1);

	/* disable CHC/D issue dummy RD when exit SREF due to DVFS support in DCS period */
	temp = mmio_read_32(DRAMC_AO_CHC_DUMMY_RD);
	chc_sref_dummy_read = (temp >> 21) & 0x1;
	mmio_write_32(DRAMC_AO_CHC_DUMMY_RD, temp & ~(0x1 << 21));
	temp = mmio_read_32(DRAMC_AO_CHD_DUMMY_RD);
	chd_sref_dummy_read = (temp >> 21) & 0x1;
	mmio_write_32(DRAMC_AO_CHD_DUMMY_RD, temp & ~(0x1 << 21));

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp |
		(0xc << 16) | (0xc << 20) | (0xc << 24) | (0xc << 28));

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL4);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL4, temp &
		~((0xc << 0) | (0xc << 4) | (0xc << 8) | (0xc << 12)));

	/* switch PLL control path to SPM conf */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL3) & ~(0xc << 0);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL3, temp);

	/* change CHC/D selection from SW mode */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL, temp &
		~((0xc << 0) | (0xc << 4) | (0xc << 8) | (0xc << 12) |
		(0xc << 16) | (0xc << 20) | (0xc << 24) | (0xc << 28)));

	/* switch SPM to SW mode */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2, temp &
		~((0xc << 0) | (0xc << 4) | (0xc << 8) | (0x3 << 12) |
		(0x3 << 16) | (0x3 << 20) | (0x3 << 24) | (0x6 << 28)));

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON_SEL2, temp |
		(0xc << 12) | (0xc << 16) | (0xc << 20) | (0xc << 24));

	//! 20161027 mazar add
	/* disable impedance tracking*/
	temp = mmio_read_32(DRAMC_AO_CHC_BASE + 0x22C) & ~(0x3 << 30);
	mmio_write_32(DRAMC_AO_CHC_BASE + 0x22C, temp  & ~(0x1 << 20) );
	temp = mmio_read_32(DRAMC_AO_CHD_BASE + 0x22C) & ~(0x3 << 30);
	mmio_write_32(DRAMC_AO_CHD_BASE + 0x22C, temp  & ~(0x1 << 20) );

	/* Sidle to S1, CHC/D disable DQSOSCENDIS */
	temp = mmio_read_32(DRAMC_AO_CHC_DQSOSCR);
	mmio_write_32(DRAMC_AO_CHC_DQSOSCR, temp | (0x1 << 28));
	temp = mmio_read_32(DRAMC_AO_CHD_DQSOSCR);
	mmio_write_32(DRAMC_AO_CHD_DQSOSCR, temp | (0x1 << 28));

	udelay(12);

	mmio_write_32(APMCU_INFRA_TOPAXI_PROTECTEN_1_SET, APMCU_DPY_CH2_PROT_BIT_MASK);
	mmio_write_32(APMCU_INFRA_TOPAXI_PROTECTEN_1_SET, APMCU_DPY_CH3_PROT_BIT_MASK);

	udelay(2);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp | (0xc << 0));
	do {
		temp = (mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2) >> 16) & 0xc;
	}while (temp != 0xc);
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp | (0xc << 8));
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp | (0xc << 12));

	/* add for supporting DVFS in DCS */
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) & ~(0x3 << 4);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) | 0x1;
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) & ~(0x3 << 4);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) | 0x1;
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp);
	udelay(1);
	temp = mmio_read_32(DDRPHY_CHC_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHC_MISC_CG_CTRL0, temp);
	temp = mmio_read_32(DDRPHY_CHD_MISC_CG_CTRL0) & ~(0x1);
	mmio_write_32(DDRPHY_CHD_MISC_CG_CTRL0, temp);

	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp | (0xc << 8));

	/* test add for DVFS support in DCS */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp | (0xc << 4));
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp & ~(0xc << 20));
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp & ~(0xc << 16));
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp & ~(0xc << 24));
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON, temp & ~(0xc << 28));

	/* S1 to S0 */
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp & ~(0xc << 0));

	/* switch PLL control mode from normal mode to low power mode */
	temp = mmio_read_32(APMCU_DDRPHY2AO_BASE + (0x9d << 2));
	mmio_write_32(APMCU_DDRPHY2AO_BASE + (0x9d << 2), temp | (0x3 << 7));
	temp = mmio_read_32(APMCU_DDRPHY3AO_BASE + (0x9d << 2));
	mmio_write_32(APMCU_DDRPHY3AO_BASE + (0x9d << 2), temp | (0x3 << 7));
	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2);
	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp & ~(0xc << 8));

	/* test add for supporting DVFS in DCS */
	//!20161027mazar	temp = mmio_read_32(APMCU_DRAMC_DPY_CLK_SW_CON2);
	//!20161027mazar	mmio_write_32(APMCU_DRAMC_DPY_CLK_SW_CON2, temp & ~(0x3 << 19));

	temp = mmio_read_32(APMCU_CLK_SCP_CFG_2);
	mmio_write_32(APMCU_CLK_SCP_CFG_2, temp & ~(0x3 << 19));


	/* start to turn off CHC/D */
	spm_mtcmos_ctrl_dpy_ch2_apmcu(0);
	spm_mtcmos_ctrl_dpy_ch3_apmcu(0);
}

static int spm_mtcmos_ctrl_dpy_ch2_apmcu(int state)
{
	unsigned int temp;
	int err = 0;

	if(state == 0) {
		/* set SRAM PDN */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp | APMCU_DPY_CH2_SRAM_PDN);

		/* set APMCU_PWR_ISO = 1 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp | APMCU_PWR_ISO);
		/* set APMCU_PWR_CLK_DIS = 1 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp | APMCU_PWR_CLK_DIS);
		/* set APMCU_PWR_RST_B = 0 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~APMCU_PWR_RST_B);
	} else {
		/* power on, start to turn on DPY CH2 */
		/* set APMCU_PWR_CLK_DIS = 0 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~APMCU_PWR_CLK_DIS);
		/* set APMCU_PWR_ISO = 0 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~APMCU_PWR_ISO);
		/* set APMCU_PWR_RST_B = 1 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp | APMCU_PWR_RST_B);
		/* set SRAM_PDN = 0 */
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~(0x1 << 8));
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~(0x1 << 9));
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~(0x1 << 10));
		temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON);
		mmio_write_32(APMCU_DPY_CH2_PWR_CON, temp & ~(0x1 << 11));

		/* wait until APMCU_DPY_CH2_SRAM_PDN_ACK_BIT2 = 0 */
		do {
			temp = mmio_read_32(APMCU_DPY_CH2_PWR_CON) & APMCU_DPY_CH2_SRAM_PDN_ACK_BIT2;
		} while (temp);
	}

	return err;
}

static int spm_mtcmos_ctrl_dpy_ch3_apmcu(int state)
{
	unsigned int temp;
	int err = 0;

	if(state == 0) {
		/* set SRAM PDN */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp | APMCU_DPY_CH3_SRAM_PDN);

		/* set APMCU_PWR_ISO = 1 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp | APMCU_PWR_ISO);
		/* set APMCU_PWR_CLK_DIS = 1 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp | APMCU_PWR_CLK_DIS);
		/* set APMCU_PWR_RST_B = 0 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~APMCU_PWR_RST_B);
	} else {
		/* power on, start to turn on DPY CH3 */
		/* set APMCU_PWR_CLK_DIS = 0 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~APMCU_PWR_CLK_DIS);
		/* set APMCU_PWR_ISO = 0 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~APMCU_PWR_ISO);
		/* set APMCU_PWR_RST_B = 1 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp | APMCU_PWR_RST_B);
		/* set SRAM_PDN = 0 */
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~(0x1 << 8));
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~(0x1 << 9));
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~(0x1 << 10));
		temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON);
		mmio_write_32(APMCU_DPY_CH3_PWR_CON, temp & ~(0x1 << 11));

		/* wait until APMCU_DPY_CH3_SRAM_PDN_ACK_BIT3 = 0 */
		do {
			temp = mmio_read_32(APMCU_DPY_CH3_PWR_CON) & APMCU_DPY_CH3_SRAM_PDN_ACK_BIT3;
		} while (temp);
	}

	return err;
}
