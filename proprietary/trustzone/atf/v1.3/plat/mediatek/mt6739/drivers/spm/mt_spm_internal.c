#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mtk_plat_common.h>

/**************************************
 * Config and Parameter
 **************************************/

/**************************************
 * Define and Declare
 **************************************/

/**************************************
 * Function and API
 **************************************/

int spm_for_gps_flag;

static int md_srcclkena = -1;
int __spm_get_md_srcclkena_setting(void)
{
	int val;

	if (md_srcclkena < 0) {
		val = gteearg.devinfo[2];
		if (((((val >> 28) & 0x3) == 0x1) || (((val >> 28) & 0x3) == 0x2))
				& (((val >> 25) & 0x1) == 0x0))
			md_srcclkena = 1;
		else
			md_srcclkena = 0;
	}

	return md_srcclkena;
}

unsigned int __spm_output_wake_reason(const struct wake_status *wakesta,
		const struct pcm_desc *pcmdesc)
{
	int i;
	unsigned int wr = WR_UNKNOWN;

	if (wakesta->assert_pc != 0) {
		/* add size check for vcoredvfs */
		ERROR("PCM ASSERT AT %u (%s%s), r13 = 0x%x, debug_flag = 0x%x 0x%x\n",
				wakesta->assert_pc, (wakesta->assert_pc > pcmdesc->size) ? "NOT " : "",
				pcmdesc->version, wakesta->r13, wakesta->debug_flag, wakesta->debug_flag1);
		return WR_PCM_ASSERT;
	}

	if (wakesta->r12 & WAKE_SRC_R12_PCM_TIMER) {
		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER)
			wr = WR_PCM_TIMER;
		if (wakesta->wake_misc & WAKE_MISC_TWAM)
			wr = WR_WAKE_SRC;
		if (wakesta->wake_misc & WAKE_MISC_CPU_WAKE)
			wr = WR_WAKE_SRC;
	}
	for (i = 1; i < 32; i++) {
		if (wakesta->r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

	return wr;
}

void __spm_set_cpu_status(int cpu)
{
	if (cpu >= 0 && cpu < 4) {
		mmio_write_32(ROOT_CPUTOP_ADDR, 0x10006204);
		mmio_write_32(ROOT_CORE_ADDR, 0x10006208 + (cpu * 0x4));
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

static void spm_code_swapping(void)
{
	uint32_t con1;
	int retry = 0, timeout = 5000;

	con1 = mmio_read_32(SPM_WAKEUP_EVENT_MASK);

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, (con1 & ~(0x1)));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);

	while ((mmio_read_32(SPM_IRQ_STA) & PCM_IRQ_ROOT_MASK_LSB) == 0) {
		if (retry > timeout) {
			ERROR("[%s] r15: 0x%x, r6: 0x%x, r1: 0x%x, pcmsta: 0x%x, irqsta: 0x%x [%d]\n",
				__func__,
				mmio_read_32(PCM_REG15_DATA), mmio_read_32(PCM_REG6_DATA), mmio_read_32(PCM_REG1_DATA),
				mmio_read_32(PCM_FSM_STA), mmio_read_32(SPM_IRQ_STA), timeout);
		}
		udelay(1);
		retry++;
	}

	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, con1);
}

void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	uint32_t con1;
	uint32_t spm_pwr_on_val0_mask = 0;
	uint32_t spm_pwr_on_val0_read = 0;
	uint32_t spm_pwr_on_val0_write = 0;

	/* SPM code swapping */
	if (mmio_read_32(PCM_REG1_DATA) == 0x1)
	if ((mmio_read_32(PCM_REG1_DATA) == 0x1) && !(mmio_read_32(PCM_REG15_DATA) == 0x0))
		spm_code_swapping();

	/* backup mem control from r0 to POWER_ON_VAL0 */
	if (!(mmio_read_32(PCM_REG1_DATA) == 0x1))
		spm_pwr_on_val0_mask = 0x30F80000;
	else
		spm_pwr_on_val0_mask = 0x00F80000;

	spm_pwr_on_val0_read = mmio_read_32(SPM_POWER_ON_VAL0);
	spm_pwr_on_val0_read &= spm_pwr_on_val0_mask;

	spm_pwr_on_val0_write = mmio_read_32(PCM_REG0_DATA);
	spm_pwr_on_val0_write &= ~spm_pwr_on_val0_mask;
	spm_pwr_on_val0_write |= spm_pwr_on_val0_read;

	if (mmio_read_32(SPM_POWER_ON_VAL0) != spm_pwr_on_val0_write) {
		VERBOSE("VAL0 from 0x%x to 0x%x\n", mmio_read_32(SPM_POWER_ON_VAL0), spm_pwr_on_val0_write);
		mmio_write_32(SPM_POWER_ON_VAL0, spm_pwr_on_val0_write);
	}

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~PCM_TIMER_EN_LSB));

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	if ((mmio_read_32(PCM_FSM_STA) & 0x7fffff) != PCM_FSM_STA_DEF)
		ERROR("reset pcm(PCM_FSM_STA=0x%x)\n", mmio_read_32(PCM_FSM_STA));

	/* init PCM_CON0 (disable event vector) */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | EN_IM_SLEEP_DVS_LSB);

	/* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
	con1 = mmio_read_32(PCM_CON1) & (PCM_WDT_WAKE_MODE_LSB);
	mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | EVENT_LOCK_EN_LSB |
			SPM_SRAM_ISOINT_B_LSB |
			(pcmdesc->replace ? 0 : IM_NONRP_EN_LSB) |
			MIF_APBEN_LSB | SCP_APB_INTERNAL_EN_LSB);
}

void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{
	uint32_t ptr, len, con0;

	/* tell IM where is PCM code (use slave mode if code existed) */
	ptr = pcmdesc->base_dma;
	len = pcmdesc->size - 1;
	if (mmio_read_32(PCM_IM_PTR) != ptr || mmio_read_32(PCM_IM_LEN) != len || pcmdesc->sess > 2) {
		mmio_write_32(PCM_IM_PTR, ptr);
		mmio_write_32(PCM_IM_LEN, len);
	} else {
		mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | IM_SLAVE_LSB);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(PCM_CON0) & ~(IM_KICK_L_LSB | PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | IM_KICK_L_LSB);
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

void __spm_init_event_vector(const struct pcm_desc *pcmdesc)
{
	/* init event vector register */
	mmio_write_32(PCM_EVENT_VECTOR0, pcmdesc->vec0);
	mmio_write_32(PCM_EVENT_VECTOR1, pcmdesc->vec1);
	mmio_write_32(PCM_EVENT_VECTOR2, pcmdesc->vec2);
	mmio_write_32(PCM_EVENT_VECTOR3, pcmdesc->vec3);
	mmio_write_32(PCM_EVENT_VECTOR4, pcmdesc->vec4);
	mmio_write_32(PCM_EVENT_VECTOR5, pcmdesc->vec5);
	mmio_write_32(PCM_EVENT_VECTOR6, pcmdesc->vec6);
	mmio_write_32(PCM_EVENT_VECTOR7, pcmdesc->vec7);
	mmio_write_32(PCM_EVENT_VECTOR8, pcmdesc->vec8);
	mmio_write_32(PCM_EVENT_VECTOR9, pcmdesc->vec9);
	mmio_write_32(PCM_EVENT_VECTOR10, pcmdesc->vec10);
	mmio_write_32(PCM_EVENT_VECTOR11, pcmdesc->vec11);
	mmio_write_32(PCM_EVENT_VECTOR12, pcmdesc->vec12);
	mmio_write_32(PCM_EVENT_VECTOR13, pcmdesc->vec13);
	mmio_write_32(PCM_EVENT_VECTOR14, pcmdesc->vec14);
	mmio_write_32(PCM_EVENT_VECTOR15, pcmdesc->vec15);
}

void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage)
{
	uint8_t spm_apsrc_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->spm_apsrc_req;
	uint8_t spm_ddren_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->spm_ddren_req;
	uint8_t spm_vrf18_req = (resource_usage & SPM_RESOURCE_MAINPLL) ? 1 : pwrctrl->spm_vrf18_req;
	uint8_t spm_f26m_req  = (resource_usage & SPM_RESOURCE_CK_26M)  ? 1 : pwrctrl->spm_f26m_req;

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((spm_apsrc_req & 0x1) << 0) |
		((spm_f26m_req & 0x1) << 1) |
		((pwrctrl->spm_lte_req & 0x1) << 2) |
		((pwrctrl->spm_infra_req & 0x1) << 3) |
		((spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->spm_dvfs_req & 0x1) << 5) |
		((pwrctrl->spm_dvfs_force_down & 0x1) << 6) |
		((spm_ddren_req & 0x1) << 7) |
		((pwrctrl->spm_rsv_src_req & 0x7) << 8) |
		((pwrctrl->spm_ddren_2_req & 0x1) << 11) |
		((pwrctrl->cpu_md_dvfs_sop_force_on & 0x1) << 16));
}

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	mmio_write_32(SPM_AP_STANDBY_CON,
		((pwrctrl->wfi_op & 0x1) << 0) |
		((pwrctrl->mp0_cputop_idle_mask & 0x1) << 1) |
		((pwrctrl->mcusys_idle_mask & 0x1) << 4) |
		((pwrctrl->mcu_ddren_req_dbc_en & 0x1) << 5) |
		((pwrctrl->mcu_apsrc_sel & 0x1) << 6) |
		((pwrctrl->mm_mask_b & 0x3) << 16) |
		((pwrctrl->md_ddr_en_0_dbc_en & 0x1) << 18) |
		((pwrctrl->md_ddr_en_1_dbc_en & 0x1) << 19) |
		((pwrctrl->md_mask_b & 0x3) << 20) |
		((pwrctrl->lte_mask_b & 0x1) << 23) |
		((pwrctrl->srcclkeni_mask_b & 0x1) << 24) |
		((pwrctrl->md_apsrc_1_sel & 0x1) << 25) |
		((pwrctrl->md_apsrc_0_sel & 0x1) << 26) |
		((pwrctrl->conn_ddr_en_dbc_en & 0x1) << 27) |
		((pwrctrl->conn_mask_b & 0x1) << 28) |
		((pwrctrl->conn_apsrc_sel & 0x1) << 29) |
		((pwrctrl->conn_srcclkena_sel_mask & 0x3) << 30));

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((pwrctrl->spm_apsrc_req & 0x1) << 0) |
		((pwrctrl->spm_f26m_req & 0x1) << 1) |
		((pwrctrl->spm_lte_req & 0x1) << 2) |
		((pwrctrl->spm_infra_req & 0x1) << 3) |
		((pwrctrl->spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->spm_dvfs_req & 0x1) << 5) |
		((pwrctrl->spm_dvfs_force_down & 0x1) << 6) |
		((pwrctrl->spm_ddren_req & 0x1) << 7) |
		((pwrctrl->spm_rsv_src_req & 0x7) << 8) |
		((pwrctrl->spm_ddren_2_req & 0x1) << 11) |
		((pwrctrl->cpu_md_dvfs_sop_force_on & 0x1) << 16));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		((pwrctrl->csyspwreq_mask & 0x1) << 0) |
		((pwrctrl->ccif0_md_event_mask_b & 0x1) << 1) |
		((pwrctrl->ccif0_ap_event_mask_b & 0x1) << 2) |
		((pwrctrl->ccif1_md_event_mask_b & 0x1) << 3) |
		((pwrctrl->ccif1_ap_event_mask_b & 0x1) << 4) |
		((pwrctrl->ccifmd_md1_event_mask_b & 0x1) << 5) |
		((pwrctrl->ccifmd_md2_event_mask_b & 0x1) << 6) |
		((pwrctrl->dsi0_vsync_mask_b & 0x1) << 7) |
		((pwrctrl->dsi1_vsync_mask_b & 0x1) << 8) |
		((pwrctrl->dpi_vsync_mask_b & 0x1) << 9) |
		((pwrctrl->isp0_vsync_mask_b & 0x1) << 10) |
		((pwrctrl->isp1_vsync_mask_b & 0x1) << 11) |
		((pwrctrl->md_srcclkena_0_infra_mask_b & 0x1) << 12) |
		((pwrctrl->md_srcclkena_1_infra_mask_b & 0x1) << 13) |
		((pwrctrl->conn_srcclkena_infra_mask_b & 0x1) << 14) |
		((pwrctrl->sspm_srcclkena_infra_mask_b & 0x1) << 15) |
		((pwrctrl->srcclkeni_infra_mask_b & 0x1) << 16) |
		((pwrctrl->md_apsrc_req_0_infra_mask_b & 0x1) << 17) |
		((pwrctrl->md_apsrc_req_1_infra_mask_b & 0x1) << 18) |
		((pwrctrl->conn_apsrcreq_infra_mask_b & 0x1) << 19) |
		((pwrctrl->mcu_apsrcreq_infra_mask_b & 0x1) << 20) |
		((pwrctrl->md_ddr_en_0_mask_b & 0x1) << 21) |
		((pwrctrl->md_ddr_en_1_mask_b & 0x1) << 22) |
		((pwrctrl->md_vrf18_req_0_mask_b & 0x1) << 23) |
		((pwrctrl->md_vrf18_req_1_mask_b & 0x1) << 24) |
		((pwrctrl->md1_dvfs_req_mask & 0x3) << 25) |
		((pwrctrl->cpu_dvfs_req_mask & 0x1) << 27) |
		((pwrctrl->emi_bw_dvfs_req_mask & 0x1) << 28) |
		((pwrctrl->md_srcclkena_0_dvfs_req_mask_b & 0x1) << 29) |
		((pwrctrl->md_srcclkena_1_dvfs_req_mask_b & 0x1) << 30) |
		((pwrctrl->conn_srcclkena_dvfs_req_mask_b & 0x1) << 31));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->dvfs_halt_mask_b & 0x1f) << 0) |
		((pwrctrl->vdec_req_mask_b & 0x1) << 6) |
		((pwrctrl->gce_req_mask_b & 0x1) << 7) |
		((pwrctrl->cpu_md_dvfs_req_merge_mask_b & 0x1) << 8) |
		((pwrctrl->md_ddr_en_dvfs_halt_mask_b & 0x3) << 9) |
		((pwrctrl->dsi0_vsync_dvfs_halt_mask_b & 0x1) << 11) |
		((pwrctrl->dsi1_vsync_dvfs_halt_mask_b & 0x1) << 12) |
		((pwrctrl->dpi_vsync_dvfs_halt_mask_b & 0x1) << 13) |
		((pwrctrl->isp0_vsync_dvfs_halt_mask_b & 0x1) << 14) |
		((pwrctrl->isp1_vsync_dvfs_halt_mask_b & 0x1) << 15) |
		((pwrctrl->conn_ddr_en_mask_b & 0x1) << 16) |
		((pwrctrl->disp_req_mask_b & 0x1) << 17) |
		((pwrctrl->disp1_req_mask_b & 0x1) << 18) |
		((pwrctrl->mfg_req_mask_b & 0x1) << 19) |
		((pwrctrl->mcu_ddren_req_mask_b & 0x1) << 20) |
		((pwrctrl->mcu_apsrc_req_mask_b & 0x1) << 21) |
		((pwrctrl->ps_c2k_rccif_wake_mask_b & 0x1) << 22) |
		((pwrctrl->l1_c2k_rccif_wake_mask_b & 0x1) << 23) |
		((pwrctrl->sdio_on_dvfs_req_mask_b & 0x1) << 24) |
		((pwrctrl->emi_boost_dvfs_req_mask_b & 0x1) << 25) |
		((pwrctrl->cpu_md_emi_dvfs_req_prot_dis & 0x1) << 26) |
		((pwrctrl->dramc_spcmd_apsrc_req_mask_b & 0x1) << 27) |
		((pwrctrl->emi_boost_dvfs_req_2_mask_b & 0x1) << 28) |
		((pwrctrl->emi_bw_dvfs_req_2_mask & 0x1) << 29) |
		((pwrctrl->gce_vrf18_req_mask_b & 0x1) << 30));

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->spm_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		((pwrctrl->spm_wakeup_event_ext_mask & 0xffffffff) << 0));

	/* SPM_SRC3_MASK */
	mmio_write_32(SPM_SRC3_MASK,
		((pwrctrl->md_ddr_en_2_0_mask_b & 0x1) << 0) |
		((pwrctrl->md_ddr_en_2_1_mask_b & 0x1) << 1) |
		((pwrctrl->conn_ddr_en_2_mask_b & 0x1) << 2) |
		((pwrctrl->dramc_spcmd_apsrc_req_2_mask_b & 0x1) << 3) |
		((pwrctrl->spare1_ddren_2_mask_b & 0x1) << 4) |
		((pwrctrl->spare2_ddren_2_mask_b & 0x1) << 5) |
		((pwrctrl->ddren_emi_self_refresh_ch0_mask_b & 0x1) << 6) |
		((pwrctrl->ddren_emi_self_refresh_ch1_mask_b & 0x1) << 7) |
		((pwrctrl->ddren_mm_state_mask_b & 0x1) << 8) |
		((pwrctrl->ddren_sspm_apsrc_req_mask_b & 0x1) << 9) |
		((pwrctrl->ddren_dqssoc_req_mask_b & 0x1) << 10) |
		((pwrctrl->ddren2_emi_self_refresh_ch0_mask_b & 0x1) << 16) |
		((pwrctrl->ddren2_emi_self_refresh_ch1_mask_b & 0x1) << 17) |
		((pwrctrl->ddren2_mm_state_mask_b & 0x1) << 18) |
		((pwrctrl->ddren2_sspm_apsrc_req_mask_b & 0x1) << 19) |
		((pwrctrl->ddren2_dqssoc_req_mask_b & 0x1) << 20));

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
	/* Auto-gen End */

	/* FIXME: */
#if 0
	/* for gps only case */
	if (spm_for_gps_flag) {
		uint32_t value;

		INFO("for gps only case\n");
		value = mmio_read_32(SPM_CLK_CON);
		value &= (~(0x1 << 6));
		value &= (~(0x1 << 13));
		value |= (0x1 << 1);
		value &= (~(0x1 << 0));
		mmio_write_32(SPM_CLK_CON, value);

		value = mmio_read_32(SPM_SRC3_MASK);
		value &= (~(0x1 << 25));
		mmio_write_32(SPM_SRC3_MASK, value);

		value = mmio_read_32(SPM_SRC_MASK);
		value &= (~(0x1 << 12));
		mmio_write_32(SPM_SRC_MASK, value);
	}
#endif
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	uint32_t val, mask, isr;

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | PCM_TIMER_EN_LSB);

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->csyspwreq_mask)
		mask &= ~WAKE_SRC_R12_CSYSPWREQ_B;
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	isr = mmio_read_32(SPM_IRQ_MASK) & SPM_TWAM_IRQ_MASK_LSB;
	mmio_write_32(SPM_IRQ_MASK, isr | ISRM_RET_IRQ_AUX);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	uint32_t con0;

	/* init register to match PCM expectation */
	mmio_write_32(SPM_MAS_PAUSE_MASK_B, 0xffffffff);
	mmio_write_32(SPM_MAS_PAUSE2_MASK_B, 0xffffffff);
	mmio_write_32(PCM_REG_DATA_INI, 0);

	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0)
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	if (pwrctrl->pcm_flags_cust_set != 0)
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	if (pwrctrl->pcm_flags1_cust_clr != 0)
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	if (pwrctrl->pcm_flags1_cust_set != 0)
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;
	mmio_write_32(SPM_SW_FLAG, pwrctrl->pcm_flags);

	/* enable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

	/* check IM ready */
	while ((mmio_read_32(PCM_FSM_STA) & (0x7 << 7)) != (0x4 << 7))
		;

	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(PCM_CON0) & ~(IM_KICK_L_LSB | PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}


void __spm_get_wakeup_status(struct wake_status *wakesta)
{
	/* get PC value if PCM assert (pause abort) */
	wakesta->assert_pc = mmio_read_32(PCM_REG_DATA_INI);

	/* get wakeup event */
	wakesta->r12 = mmio_read_32(SPM_SW_RSV_0);        /* backup of PCM_REG12_DATA */
	wakesta->r12_ext = mmio_read_32(PCM_REG12_EXT_DATA);
	wakesta->raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->wake_misc = mmio_read_32(SPM_WAKEUP_MISC);   /* backup of SPM_WAKEUP_MISC */

	/* get sleep time */
	wakesta->timer_out = mmio_read_32(PCM_TIMER_OUT);   /* backup of PCM_TIMER_OUT */

	/* get other SYS and co-clock status */
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);
	wakesta->req_sta = mmio_read_32(SRC_REQ_STA);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = mmio_read_32(SPM_SW_DEBUG);
	wakesta->debug_flag1 = mmio_read_32(WDT_LATCH_SPARE0_FIX);

	/* get special pattern (0xf0000 or 0x10000) if sleep abort */
	wakesta->event_reg = mmio_read_32(PCM_EVENT_REG_STA);   /* PCM_EVENT_REG_STA */

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);
}

void __spm_clean_after_wakeup(void)
{
	uint32_t sta;
	/*
	 * Copy SPM_WAKEUP_STA to SPM_SW_RSV_0
	 * before clear SPM_WAKEUP_STA
	 *
	 * CPU dormant driver @kernel will copy  edge-trig IRQ pending
	 * (recorded @SPM_SW_RSV_0) to GIC
	 */
	sta = mmio_read_32(SPM_WAKEUP_STA) | (mmio_read_32(SPM_SW_RSV_0) & WAKE_SRC_R12_PCM_TIMER);
	mmio_write_32(SPM_SW_RSV_0, sta);


	/* [Vcorefs] can not switch back to POWER_ON_VAL0 here, */
	/* the FW stays in VCORE DVFS which use r0 to Ctrl MEM */
	/* disable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, 0); */

	/* clean CPU wakeup event */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/* [Vcorefs] not disable pcm timer here, due to the */
	/* following vcore dvfs will use it for latency check */
	/* clean PCM timer event */
	/* mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~PCM_TIMER_EN_LSB)); */

	/* clean wakeup event raw status (for edge trigger event) */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~0);

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

		con1 = mmio_read_32(PCM_CON1) & ~(PCM_WDT_WAKE_MODE_LSB);
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | con1);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL, mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | PCM_WDT_EN_LSB);
	} else {
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) &
		~PCM_WDT_EN_LSB));
	}

}
