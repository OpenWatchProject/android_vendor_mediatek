#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_pmic_wrap.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**************************************
 * Config and Parameter
 **************************************/

/**************************************
 * Define and Declare
 **************************************/
#define	SPM_INIT_DONE_US	20 /* Simulation result */

/**************************************
 * Function and API
 **************************************/

wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta,
		const struct pcm_desc *pcmdesc)
{
	int i;
	wake_reason_t wr = WR_UNKNOWN;

	if (wakesta->is_abort) {
		ERROR("SPM ABORT (%s), r13 = 0x%x, debug_flag = 0x%x 0x%x sw_flag = 0x%x 0x%x b_sw_flag = 0x%x 0x%x\n",
		pcmdesc->version, wakesta->r13, wakesta->debug_flag, wakesta->debug_flag1,
		wakesta->sw_flag0, wakesta->sw_flag1, wakesta->b_sw_flag0, wakesta->b_sw_flag1);
		mt_spm_dump_pmic_warp_reg();
	}

	if (wakesta->r12 & R12_PCM_TIMER_EVENT) {

		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER_EVENT)
			wr = WR_PCM_TIMER;
	}

	if (wakesta->r12 & R12_SPM_TWAM_IRQ_B) {

		if (wakesta->wake_misc & WAKE_MISC_DVFSRC_IRQ)
			wr = WR_DVFSRC;

		if (wakesta->wake_misc & WAKE_MISC_TWAM_IRQ_B)
			wr = WR_TWAM;

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET0)
			wr = WR_PMSR;

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET1)
			wr = WR_PMSR;

		if (wakesta->wake_misc & WAKE_MISC_PMSR_IRQ_B_SET2)
			wr = WR_PMSR;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_0)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_1)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_2)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_3)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL)
			wr = WR_SPM_ACK_CHK;
	}

	for (i = 2; i < 32; i++) {
		if (wakesta->r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

	return wr;
}

int __spm_get_spmfw_idx(void)
{
	return __spmfw_idx;
}

void __spm_set_cpu_status(int cpu)
{
	if (cpu >= 0 && cpu < 8) {
		mmio_write_32(ROOT_CPUTOP_ADDR, (1U << cpu));
		mmio_write_32(ROOT_CORE_ADDR, MP0_CPU0_PWR_CON + (cpu * 0x4) + 0x20000000);
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

static void spm_code_swapping(void)
{
	uint32_t con1;
	/* int retry = 0, timeout = 5000; */

	con1 = mmio_read_32(SPM_WAKEUP_EVENT_MASK);

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, (con1 & ~(0x1)));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, con1);
}

void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	uint32_t con1;
	unsigned char first_load_fw = true;

	/* check the SPM FW is run or not */
	if (mmio_read_32(MD32PCM_CFGREG_SW_RSTN) & 0x1)
		first_load_fw = false;

	if (!first_load_fw) {
		/* SPM code swapping */
		spm_code_swapping();

		/* Backup PCM r0 -> SPM_POWER_ON_VAL0 before `reset PCM` */
		mmio_write_32(SPM_POWER_ON_VAL0, mmio_read_32(PCM_REG0_DATA));
	}

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~RG_PCM_TIMER_EN_LSB));

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);

	/* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
	con1 = mmio_read_32(PCM_CON1) & (RG_PCM_WDT_WAKE_LSB);
	mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | REG_EVENT_LOCK_EN_LSB |
			REG_SPM_SRAM_ISOINT_B_LSB | RG_AHBMIF_APBEN_LSB | REG_MD32_APB_INTERNAL_EN_LSB);
}

void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{

	uint32_t con0;
	uint32_t pmem_words;
	uint32_t total_words;
	uint32_t pmem_start;
	uint32_t dmem_start;
	uint32_t ptr;

	/* tell IM where is PCM code (use slave mode if code existed) */
	ptr = pcmdesc->base_dma + 0x40000000;
	pmem_words = pcmdesc->pmem_words;
	total_words = pcmdesc->total_words;
	pmem_start = pcmdesc->pmem_start;
	dmem_start = pcmdesc->dmem_start;

	if (mmio_read_32(MD32PCM_DMA0_SRC) != ptr
			|| mmio_read_32(MD32PCM_DMA0_DST) != pmem_start
			|| mmio_read_32(MD32PCM_DMA0_WPPT) != pmem_words
			|| mmio_read_32(MD32PCM_DMA0_WPTO) != dmem_start
			|| mmio_read_32(MD32PCM_DMA0_COUNT) !=  total_words
			|| mmio_read_32(MD32PCM_DMA0_CON) !=  0x0003820E) {
		mmio_write_32(MD32PCM_DMA0_SRC, ptr);
		mmio_write_32(MD32PCM_DMA0_DST, pmem_start);
		mmio_write_32(MD32PCM_DMA0_WPPT, pmem_words);
		mmio_write_32(MD32PCM_DMA0_WPTO, dmem_start);
		mmio_write_32(MD32PCM_DMA0_COUNT, total_words);
		mmio_write_32(MD32PCM_DMA0_CON, 0x0003820E);
		mmio_write_32(MD32PCM_DMA0_START, 0x00008000);
	} else {
		mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_IM_SLAVE_LSB);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(PCM_CON0);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}

void __spm_init_pcm_register(void)
{
	/* init r0 with POWER_ON_VAL0 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL0));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* init r7 with POWER_ON_VAL1 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(PCM_PWR_IO_EN, 0);
}

void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage)
{

	uint8_t reg_spm_apsrc_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->reg_spm_apsrc_req;
	uint8_t reg_spm_ddr_en_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->reg_spm_ddr_en_req;
	uint8_t reg_spm_vrf18_req = (resource_usage & SPM_RESOURCE_MAINPLL) ? 1 : pwrctrl->reg_spm_vrf18_req;
	uint8_t reg_spm_f26m_req  = (resource_usage & SPM_RESOURCE_CK_26M)  ? 1 : pwrctrl->reg_spm_f26m_req;

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((reg_spm_apsrc_req & 0x1) << 0) |
		((reg_spm_f26m_req & 0x1) << 1) |
		((pwrctrl->reg_spm_infra_req & 0x1) << 3) |
		((reg_spm_vrf18_req & 0x1) << 4) |
		((reg_spm_ddr_en_req & 0x1) << 7) |
		((pwrctrl->reg_spm_ddr_en2_req & 0x1) << 8) |
		((pwrctrl->reg_spm_dvfs_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 12) |
		((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 13) |
		((pwrctrl->reg_spm_mcusys_pwr_event_req & 0x1) << 14) |
		((pwrctrl->cpu_md_dvfs_sop_force_on & 0x1) << 15));
}

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
#if MCUSYS_MTCMOS_ON
	mmio_write_32(SPM_AP_STANDBY_CON,
		((pwrctrl->reg_wfi_op & 0x1) << 0) |
		((pwrctrl->reg_wfi_type & 0x1) << 1) |
		((pwrctrl->reg_mp0_cputop_idle_mask & 0x1) << 2) |
		((pwrctrl->reg_mp1_cputop_idle_mask & 0x1) << 3) |
		((pwrctrl->reg_mcusys_idle_mask & 0x1) << 4) |
		((pwrctrl->reg_md_apsrc_1_sel & 0x1) << 25) |
		((pwrctrl->reg_md_apsrc_0_sel & 0x1) << 26) |
		((pwrctrl->reg_conn_apsrc_sel & 0x1) << 29));
#else
	mmio_write_32(SPM_AP_STANDBY_CON,
		((pwrctrl->reg_wfi_type & 0x1) << 1) |
		((pwrctrl->reg_md_apsrc_1_sel & 0x1) << 25) |
		((pwrctrl->reg_md_apsrc_0_sel & 0x1) << 26) |
		((pwrctrl->reg_conn_apsrc_sel & 0x1) << 29));
#endif
	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((pwrctrl->reg_spm_apsrc_req & 0x1) << 0) |
		((pwrctrl->reg_spm_f26m_req & 0x1) << 1) |
		((pwrctrl->reg_spm_infra_req & 0x1) << 3) |
		((pwrctrl->reg_spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->reg_spm_ddr_en_req & 0x1) << 7) |
		((pwrctrl->reg_spm_ddr_en2_req & 0x1) << 8) |
		((pwrctrl->reg_spm_dvfs_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 12) |
		((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 13) |
		((pwrctrl->reg_spm_mcusys_pwr_event_req & 0x1) << 14) |
		((pwrctrl->cpu_md_dvfs_sop_force_on & 0x1) << 15));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		((pwrctrl->reg_md_srcclkena_0_mask_b & 0x1) << 0) |
		((pwrctrl->reg_md_srcclkena2infra_req_0_mask_b & 0x1) << 1) |
		((pwrctrl->reg_md_apsrc2infra_req_0_mask_b & 0x1) << 2) |
		((pwrctrl->reg_md_apsrc_req_0_mask_b & 0x1) << 3) |
		((pwrctrl->reg_md_vrf18_req_0_mask_b & 0x1) << 4) |
		((pwrctrl->reg_md_ddr_en_0_mask_b & 0x1) << 5) |
		((pwrctrl->reg_md_ddr_en2_0_mask_b & 0x1) << 6) |
		((pwrctrl->reg_md_srcclkena_1_mask_b & 0x1) << 7) |
		((pwrctrl->reg_md_srcclkena2infra_req_1_mask_b & 0x1) << 8) |
		((pwrctrl->reg_md_apsrc2infra_req_1_mask_b & 0x1) << 9) |
		((pwrctrl->reg_md_apsrc_req_1_mask_b & 0x1) << 10) |
		((pwrctrl->reg_md_vrf18_req_1_mask_b & 0x1) << 11) |
		((pwrctrl->reg_md_ddr_en_1_mask_b & 0x1) << 12) |
		((pwrctrl->reg_md_ddr_en2_1_mask_b & 0x1) << 13) |
		((pwrctrl->reg_conn_srcclkena_mask_b & 0x1) << 14) |
		((pwrctrl->reg_conn_srcclkenb_mask_b & 0x1) << 15) |
		((pwrctrl->reg_conn_infra_req_mask_b & 0x1) << 16) |
		((pwrctrl->reg_conn_apsrc_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_conn_vrf18_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_conn_ddr_en_mask_b & 0x1) << 19) |
		((pwrctrl->reg_conn_ddr_en2_mask_b & 0x1) << 20) |
		((pwrctrl->reg_srcclkeni0_srcclkena_mask_b & 0x1) << 21) |
		((pwrctrl->reg_srcclkeni0_infra_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_srcclkeni1_srcclkena_mask_b & 0x1) << 23) |
		((pwrctrl->reg_srcclkeni1_infra_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_srcclkeni2_srcclkena_mask_b & 0x1) << 25) |
		((pwrctrl->reg_srcclkeni2_infra_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_infrasys_apsrc_req_mask_b & 0x1) << 27) |
		((pwrctrl->reg_infrasys_ddr_en_mask_b & 0x1) << 28) |
		((pwrctrl->reg_infrasys_ddr_en2_mask_b & 0x1) << 29) |
		((pwrctrl->reg_md32_srcclkena_mask_b & 0x1) << 30) |
		((pwrctrl->reg_conn_vfe28_req_mask_b & 0x1) << 31));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->reg_md32_infra_req_mask_b & 0x1) << 0) |
		((pwrctrl->reg_md32_apsrc_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_md32_vrf18_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_md32_ddr_en_mask_b & 0x1) << 3) |
		((pwrctrl->reg_md32_ddr_en2_mask_b & 0x1) << 4) |
		((pwrctrl->reg_scp_srcclkena_mask_b & 0x1) << 5) |
		((pwrctrl->reg_scp_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_scp_apsrc_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_scp_vrf18_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_scp_ddr_en_mask_b & 0x1) << 9) |
		((pwrctrl->reg_scp_ddr_en2_mask_b & 0x1) << 10) |
		((pwrctrl->reg_ufs_srcclkena_mask_b & 0x1) << 11) |
		((pwrctrl->reg_ufs_infra_req_mask_b & 0x1) << 12) |
		((pwrctrl->reg_ufs_apsrc_req_mask_b & 0x1) << 13) |
		((pwrctrl->reg_ufs_vrf18_req_mask_b & 0x1) << 14) |
		((pwrctrl->reg_ufs_ddr_en_mask_b & 0x1) << 15) |
		((pwrctrl->reg_ufs_ddr_en2_mask_b & 0x1) << 16) |
		((pwrctrl->reg_disp0_apsrc_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_disp0_ddr_en_mask_b & 0x1) << 18) |
		((pwrctrl->reg_disp0_ddr_en2_mask_b & 0x1) << 19) |
		((pwrctrl->reg_disp1_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_disp1_ddr_en_mask_b & 0x1) << 21) |
		((pwrctrl->reg_disp1_ddr_en2_mask_b & 0x1) << 22) |
		((pwrctrl->reg_gce_infra_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_gce_apsrc_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_gce_vrf18_req_mask_b & 0x1) << 25) |
		((pwrctrl->reg_gce_ddr_en_mask_b & 0x1) << 26) |
		((pwrctrl->reg_gce_ddr_en2_mask_b & 0x1) << 27) |
		((pwrctrl->reg_emi_ch0_ddr_en_mask_b & 0x1) << 28) |
		((pwrctrl->reg_emi_ch1_ddr_en_mask_b & 0x1) << 29) |
		((pwrctrl->reg_emi_ch0_ddr_en2_mask_b & 0x1) << 30) |
		((pwrctrl->reg_emi_ch1_ddr_en2_mask_b & 0x1) << 31));

	/* SPM_SRC3_MASK */
	mmio_write_32(SPM_SRC3_MASK,
		((pwrctrl->reg_dvfsrc_event_trigger_mask_b & 0x1) << 0) |
		((pwrctrl->reg_sw2spm_int0_mask_b & 0x1) << 1) |
		((pwrctrl->reg_sw2spm_int1_mask_b & 0x1) << 2) |
		((pwrctrl->reg_sw2spm_int2_mask_b & 0x1) << 3) |
		((pwrctrl->reg_sw2spm_int3_mask_b & 0x1) << 4) |
		((pwrctrl->reg_sc_adsp2spm_wakeup_mask_b & 0x1) << 5) |
		((pwrctrl->reg_sc_sspm2spm_wakeup_mask_b & 0xf) << 6) |
		((pwrctrl->reg_sc_scp2spm_wakeup_mask_b & 0x1) << 10) |
		((pwrctrl->reg_csyspwrreq_mask & 0x1) << 11) |
		((pwrctrl->reg_spm_srcclkena_reserved_mask_b & 0x1) << 12) |
		((pwrctrl->reg_spm_infra_req_reserved_mask_b & 0x1) << 13) |
		((pwrctrl->reg_spm_apsrc_req_reserved_mask_b & 0x1) << 14) |
		((pwrctrl->reg_spm_vrf18_req_reserved_mask_b & 0x1) << 15) |
		((pwrctrl->reg_spm_ddr_en_reserved_mask_b & 0x1) << 16) |
		((pwrctrl->reg_spm_ddr_en2_reserved_mask_b & 0x1) << 17) |
		((pwrctrl->reg_audio_dsp_srcclkena_mask_b & 0x1) << 18) |
		((pwrctrl->reg_audio_dsp_infra_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_audio_dsp_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_audio_dsp_vrf18_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_audio_dsp_ddr_en_mask_b & 0x1) << 22) |
		((pwrctrl->reg_audio_dsp_ddr_en2_mask_b & 0x1) << 23) |
		((pwrctrl->reg_mcusys_pwr_event_mask_b & 0x1) << 24) |
		((pwrctrl->reg_msdc0_srcclkena_mask_b & 0x1) << 25) |
		((pwrctrl->reg_msdc0_infra_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_msdc0_apsrc_req_mask_b & 0x1) << 27) |
		((pwrctrl->reg_msdc0_vrf18_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_msdc0_ddr_en_mask_b & 0x1) << 29) |
		((pwrctrl->reg_msdc0_ddr_en2_mask_b & 0x1) << 30) |
		((pwrctrl->reg_conn_srcclkenb2pwrap_mask_b & 0x1) << 31));

	/* SPM_SRC4_MASK */
	mmio_write_32(SPM_SRC4_MASK,
		((pwrctrl->ccif_event_mask_b & 0xffff) << 0) |
		((pwrctrl->reg_apu_core0_srcclkena_mask_b & 0x1) << 16) |
		((pwrctrl->reg_apu_core0_infra_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_apu_core0_apsrc_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_apu_core0_vrf18_req_mask_b & 0x1) << 19) |
		((pwrctrl->reg_apu_core0_ddr_en_mask_b & 0x1) << 20) |
		((pwrctrl->reg_apu_core1_srcclkena_mask_b & 0x1) << 21) |
		((pwrctrl->reg_apu_core1_infra_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_apu_core1_apsrc_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_apu_core1_vrf18_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_apu_core1_ddr_en_mask_b & 0x1) << 25) |
		((pwrctrl->reg_apu_core2_srcclkena_mask_b & 0x1) << 26) |
		((pwrctrl->reg_apu_core2_infra_req_mask_b & 0x1) << 27) |
		((pwrctrl->reg_apu_core2_apsrc_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_apu_core2_vrf18_req_mask_b & 0x1) << 29) |
		((pwrctrl->reg_apu_core2_ddr_en_mask_b & 0x1) << 30) |
		((pwrctrl->reg_apu_core2_ddr_en2_mask_b & 0x1) << 31));

	/* SPM_SRC5_MASK */
	mmio_write_32(SPM_SRC5_MASK,
		((pwrctrl->reg_mcusys_merge_apsrc_req_mask_b & 0x1ff) << 0) |
		((pwrctrl->reg_mcusys_merge_ddr_en_mask_b & 0x1ff) << 9) |
		((pwrctrl->reg_mcusys_merge_ddr_en2_mask_b & 0x1ff) << 18) |
		((pwrctrl->reg_apu_core0_ddr_en2_mask_b & 0x1) << 27) |
		((pwrctrl->reg_apu_core1_ddr_en2_mask_b & 0x1) << 28) |
		((pwrctrl->reg_cg_check_ddr_en_mask_b & 0x1) << 29) |
		((pwrctrl->reg_cg_check_ddr_en2_mask_b & 0x1) << 30));

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		((pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_SRC6_MASK */
	mmio_write_32(SPM_SRC6_MASK,
		((pwrctrl->reg_msdc1_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_msdc1_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_msdc1_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_msdc1_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_msdc1_ddr_en_mask_b & 0x1) << 4) |
		((pwrctrl->reg_msdc1_ddr_en2_mask_b & 0x1) << 5) |
		((pwrctrl->reg_msdc1_srcclkena_ack_mask & 0x1) << 16) |
		((pwrctrl->reg_msdc1_infra_ack_mask & 0x1) << 17) |
		((pwrctrl->reg_msdc1_apsrc_ack_mask & 0x1) << 18) |
		((pwrctrl->reg_msdc1_vrf18_ack_mask & 0x1) << 19) |
		((pwrctrl->reg_msdc1_ddr_en_ack_mask & 0x1) << 20) |
		((pwrctrl->reg_msdc1_ddr_en2_ack_mask & 0x1) << 21));

	/* MP0_CPU0_WFI_EN */
	mmio_write_32(MP0_CPU0_WFI_EN,
		((pwrctrl->mp0_cpu0_wfi_en & 0x1) << 0));

	/* MP0_CPU1_WFI_EN */
	mmio_write_32(MP0_CPU1_WFI_EN,
		((pwrctrl->mp0_cpu1_wfi_en & 0x1) << 0));

	/* MP0_CPU2_WFI_EN */
	mmio_write_32(MP0_CPU2_WFI_EN,
		((pwrctrl->mp0_cpu2_wfi_en & 0x1) << 0));

	/* MP0_CPU3_WFI_EN */
	mmio_write_32(MP0_CPU3_WFI_EN,
		((pwrctrl->mp0_cpu3_wfi_en & 0x1) << 0));

	/* MP0_CPU4_WFI_EN */
	mmio_write_32(MP0_CPU4_WFI_EN,
		((pwrctrl->mp0_cpu4_wfi_en & 0x1) << 0));

	/* MP0_CPU5_WFI_EN */
	mmio_write_32(MP0_CPU5_WFI_EN,
		((pwrctrl->mp0_cpu5_wfi_en & 0x1) << 0));

	/* MP0_CPU6_WFI_EN */
	mmio_write_32(MP0_CPU6_WFI_EN,
		((pwrctrl->mp0_cpu6_wfi_en & 0x1) << 0));

	/* MP0_CPU7_WFI_EN */
	mmio_write_32(MP0_CPU7_WFI_EN,
		((pwrctrl->mp0_cpu7_wfi_en & 0x1) << 0));

	/* Auto-gen End */
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	uint32_t val, mask, isr;

	/* toggle event counter clear */
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_SPM_EVENT_COUNTER_CLR_LSB);

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_PCM_TIMER_EN_LSB);

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->reg_csyspwrreq_mask)
		mask &= ~R12_CSYSPWREQ_B;
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	isr = mmio_read_32(SPM_IRQ_MASK);
	mmio_write_32(SPM_IRQ_MASK, isr | ISRM_RET_IRQ_AUX);

	/* toggle event counter clear */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~RG_SPM_EVENT_COUNTER_CLR_LSB));
}

void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl)
{
#if SPM_FW_NO_RESUME
    /* do Nothing */
#else
	pwrctrl->pcm_flags1 |= SPM_FLAG1_DISABLE_NO_RESUME;
#endif
}

void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0)
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	if (pwrctrl->pcm_flags_cust_set != 0)
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	if (pwrctrl->pcm_flags1_cust_clr != 0)
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	if (pwrctrl->pcm_flags1_cust_set != 0)
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;

	mmio_write_32(SPM_SW_FLAG_0, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_FLAG_1, pwrctrl->pcm_flags1);

	mmio_write_32(SPM_SW_RSV_7, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_RSV_8, pwrctrl->pcm_flags1);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	uint32_t con0;

	/* Waiting for loading SPMFW done*/
	while (mmio_read_32(MD32PCM_DMA0_RLCT) != 0x0)
		;

	/* init register to match PCM expectation */
	mmio_write_32(SPM_MAS_PAUSE_MASK_B, 0xffffffff);
	mmio_write_32(SPM_MAS_PAUSE2_MASK_B, 0xffffffff);
	mmio_write_32(PCM_REG_DATA_INI, 0);

	__spm_set_pcm_flags(pwrctrl);

	/* enable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7); */
	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(PCM_CON0);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	/* reset md32pcm */
	con0 = mmio_read_32(MD32PCM_CFGREG_SW_RSTN);
	mmio_write_32(MD32PCM_CFGREG_SW_RSTN, con0 | 0x1);

	/* Waiting for SPM init done and entering WFI*/
	udelay(SPM_INIT_DONE_US);
}

void __spm_get_wakeup_status(struct wake_status *wakesta)
{
	/* get wakeup event */
	wakesta->r12 = mmio_read_32(SPM_SW_RSV_0);        /* backup of PCM_REG12_DATA */
	wakesta->r12_ext = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->md32pcm_wakeup_sta = mmio_read_32(MD32PCM_WAKEUP_STA);
	wakesta->md32pcm_event_sta = mmio_read_32(MD32PCM_EVENT_STA);
	wakesta->wake_misc = mmio_read_32(SPM_SW_RSV_5);   /* backup of SPM_WAKEUP_MISC */

	/* get sleep time */
	wakesta->timer_out = mmio_read_32(SPM_SW_RSV_6);   /* backup of PCM_TIMER_OUT */

	/* get other SYS and co-clock status */
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);
	wakesta->req_sta0 = mmio_read_32(SRC_REQ_STA_0);
	wakesta->req_sta1 = mmio_read_32(SRC_REQ_STA_1);
	wakesta->req_sta2 = mmio_read_32(SRC_REQ_STA_2);
	wakesta->req_sta3 = mmio_read_32(SRC_REQ_STA_3);
	wakesta->req_sta4 = mmio_read_32(SRC_REQ_STA_4);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	/* get backup SW flag status */
	wakesta->b_sw_flag0 = mmio_read_32(SPM_SW_RSV_7);   /* SPM_SW_RSV_7 */
	wakesta->b_sw_flag1 = mmio_read_32(SPM_SW_RSV_8);   /* SPM_SW_RSV_8 */

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);

	/* get SW flag status */
	wakesta->sw_flag0 = mmio_read_32(SPM_SW_FLAG_0);
	wakesta->sw_flag1 = mmio_read_32(SPM_SW_FLAG_1);

	/* check abort */
	wakesta->is_abort = wakesta->debug_flag & DEBUG_ABORT_MASK;
	wakesta->is_abort |= wakesta->debug_flag1 & DEBUG_ABORT_MASK_1;
}

void __spm_clean_after_wakeup(void)
{
	/*
	 * Copy SPM_WAKEUP_STA to SPM_SW_RSV_0
	 * before clear SPM_WAKEUP_STA
	 *
	 * CPU dormant driver @kernel will copy  edge-trig IRQ pending
	 * (recorded @SPM_SW_RSV_0) to GIC
	 */
	mmio_write_32(SPM_SW_RSV_0, mmio_read_32(SPM_WAKEUP_STA) | mmio_read_32(SPM_SW_RSV_0));

	/* [Vcorefs] can not switch back to POWER_ON_VAL0 here,
	 *  the FW stays in VCORE DVFS which use r0 to Ctrl MEM
	 */
	/* disable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, 0); */

	/* clean CPU wakeup event */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/* [Vcorefs] not disable pcm timer here, due to the
	 * following vcore dvfs will use it for latency check
	 */
	/* clean PCM timer event */
	/* mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~PCM_TIMER_EN_LSB)); */

	/* clean wakeup event raw status (for edge trigger event) */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, 0xefffffff); /* bit[28] for cpu wake up event */

	/* clean ISR status (except TWAM) */
	mmio_write_32(SPM_IRQ_MASK, mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	/* enable PCM WDT (normal mode) to start count if needed */
	if (en) {
		uint32_t con1;

		con1 = mmio_read_32(PCM_CON1) & ~(RG_PCM_WDT_WAKE_LSB);
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | con1);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL, mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | RG_PCM_WDT_EN_LSB);
	} else {
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) &
		~RG_PCM_WDT_EN_LSB));
	}
}

void __spm_send_cpu_wakeup_event(void)
{
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	/* SPM will clear SPM_CPU_WAKEUP_EVENT */
}

void __spm_ext_int_wakeup_req_clr(void)
{
	mmio_write_32(EXT_INT_WAKEUP_REQ_CLR, mmio_read_32(ROOT_CPUTOP_ADDR));
}
