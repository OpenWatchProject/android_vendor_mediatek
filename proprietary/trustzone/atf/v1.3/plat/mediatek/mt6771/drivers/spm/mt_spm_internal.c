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

static void spm_dump_pmic_wrap_reg(void)
{
	unsigned int pmic_wrap_reg[] = {
		0x3CC,
		0xC04,
		0xC14,
		0xC24,
		0xC34,
		0x054,
		0x064,
		0x068,
		0x06C,
		0x050,
		0x1A8,
		0x1AC,
		0x1B0,
		0x1B4,
		0x1EC,
		0x1F0,
		0x280,
		0x284,
		0x288,
		0x28C,
		0x364,
		0x368,
		0x36C,
		0x370,
		0x374,
		0x378,
		0x37C,
		0x380,
		0x384,
		0x388,
		0x38C,
		0x390,
		0x394,
		0x398,
		0x39C,
		0x3A0,
		0x3A4,
		0x3A8,
		0x3AC,
		0x3B0};
	unsigned int val = 0;
	int i = 0;

	for (i = 0; i < (sizeof(pmic_wrap_reg)/sizeof(unsigned int)); i++) {
		val = mmio_read_32(0x1000D000 + pmic_wrap_reg[i]);
		ERROR("#@# %s(%d) pmic_wrap_reg(0x%x) = 0x%x\n", __func__, __LINE__,
				pmic_wrap_reg[i], val);
	}
}

wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta,
		const struct pcm_desc *pcmdesc, const char *scenario)
{
	int i;
	wake_reason_t wr = WR_UNKNOWN;

	if (wakesta->assert_pc != 0) {
		/* add size check for vcoredvfs */
		ERROR("PCM ASSERT AT %u (%s%s) (%s), r13 = 0x%x, debug_flag = 0x%x 0x%x, ULPOSC_CON = 0x%x\n",
				wakesta->assert_pc, (wakesta->assert_pc > pcmdesc->size) ? "NOT " : "",
				pcmdesc->version, scenario, wakesta->r13,
				wakesta->debug_flag, wakesta->debug_flag1, mmio_read_32(ULPOSC_CON));
		spm_dump_pmic_wrap_reg();
		return WR_PCM_ASSERT;
	}

	if (wakesta->r12 & WAKE_SRC_R12_PCM_TIMER) {
		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER) {
			wr = WR_PCM_TIMER;
		}
		if (wakesta->wake_misc & WAKE_MISC_TWAM) {
			wr = WR_WAKE_SRC;
		}
		if (wakesta->wake_misc & WAKE_MISC_CPU_WAKE) {
			wr = WR_WAKE_SRC;
		}
	}
	for (i = 1; i < 32; i++) {
		if (wakesta->r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

#if 0
	INFO("timer_out = %u, r13 = 0x%x, debug_flag = 0x%x 0x%x\n",
			wakesta->timer_out, wakesta->r13, wakesta->debug_flag, wakesta->debug_flag1);

	INFO("r12 = 0x%x, r12_ext = 0x%x, raw_sta = 0x%x, idle_sta = 0x%x, req_sta =  0x%x, event_reg = 0x%x, isr = 0x%x\n",
			wakesta->r12, wakesta->r12_ext, wakesta->raw_sta, wakesta->idle_sta,
			wakesta->req_sta, wakesta->event_reg, wakesta->isr);

	INFO("raw_ext_sta = 0x%x, wake_misc = 0x%x\n", wakesta->raw_ext_sta,
			wakesta->wake_misc);
#endif
	return wr;
}

int __spm_get_spmfw_idx(void) {
	return __spmfw_idx;
}

void __spm_set_cpu_status(int cpu)
{
	if (cpu >= 0 && cpu < 4) {
		mmio_write_32(ROOT_CPUTOP_ADDR, 0x10006204);
		mmio_write_32(ROOT_CORE_ADDR, 0x10006208 + (cpu * 0x4));
	} else if (cpu >= 4 && cpu < 8) {
		mmio_write_32(ROOT_CPUTOP_ADDR, 0x10006218);
		mmio_write_32(ROOT_CORE_ADDR, 0x1000621c + ((cpu - 4) * 0x4));
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

static void spm_code_swapping(void)
{
	__uint32_t con1;
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
	__uint32_t con1;

	bool first_load_fw = true;

	if ((mmio_read_32(PCM_REG1_DATA) == 0x1) && !(mmio_read_32(PCM_REG15_DATA) == 0x0))
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
	__uint32_t ptr, len, con0;

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
	__uint8_t spm_apsrc_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->spm_apsrc_req;
	__uint8_t spm_ddren_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->spm_ddren_req;
	__uint8_t spm_vrf18_req = (resource_usage & SPM_RESOURCE_MAINPLL) ? 1 : pwrctrl->spm_vrf18_req;
	__uint8_t spm_f26m_req  = (resource_usage & SPM_RESOURCE_CK_26M)  ? 1 : pwrctrl->spm_f26m_req;

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((spm_apsrc_req & 0x1) << 0) |
		((spm_f26m_req & 0x1) << 1) |
		((pwrctrl->spm_infra_req & 0x1) << 3) |
		((spm_vrf18_req & 0x1) << 4) |
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
		((pwrctrl->mp1_cputop_idle_mask & 0x1) << 2) |
		((pwrctrl->mcusys_idle_mask & 0x1) << 4) |
		((pwrctrl->mm_mask_b & 0x3) << 16) |
		((pwrctrl->md_ddr_en_0_dbc_en & 0x1) << 18) |
		((pwrctrl->md_ddr_en_1_dbc_en & 0x1) << 19) |
		((pwrctrl->md_mask_b & 0x3) << 20) |
		((pwrctrl->sspm_mask_b & 0x1) << 22) |
		((pwrctrl->scp_mask_b & 0x1) << 23) |
		((pwrctrl->srcclkeni_mask_b & 0x1) << 24) |
		((pwrctrl->md_apsrc_1_sel & 0x1) << 25) |
		((pwrctrl->md_apsrc_0_sel & 0x1) << 26) |
		((pwrctrl->conn_ddr_en_dbc_en & 0x1) << 27) |
		((pwrctrl->conn_mask_b & 0x1) << 28) |
		((pwrctrl->conn_apsrc_sel & 0x1) << 29));

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((pwrctrl->spm_apsrc_req & 0x1) << 0) |
		((pwrctrl->spm_f26m_req & 0x1) << 1) |
		((pwrctrl->spm_infra_req & 0x1) << 3) |
		((pwrctrl->spm_vrf18_req & 0x1) << 4) |
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
		((pwrctrl->ccif2_md_event_mask_b & 0x1) << 5) |
		((pwrctrl->ccif2_ap_event_mask_b & 0x1) << 6) |
		((pwrctrl->ccif3_md_event_mask_b & 0x1) << 7) |
		((pwrctrl->ccif3_ap_event_mask_b & 0x1) << 8) |
		((pwrctrl->md_srcclkena_0_infra_mask_b & 0x1) << 9) |
		((pwrctrl->md_srcclkena_1_infra_mask_b & 0x1) << 10) |
		((pwrctrl->conn_srcclkena_infra_mask_b & 0x1) << 11) |
		((pwrctrl->ufs_infra_req_mask_b & 0x1) << 12) |
		((pwrctrl->srcclkeni_infra_mask_b & 0x1) << 13) |
		((pwrctrl->md_apsrc_req_0_infra_mask_b & 0x1) << 14) |
		((pwrctrl->md_apsrc_req_1_infra_mask_b & 0x1) << 15) |
		((pwrctrl->conn_apsrcreq_infra_mask_b & 0x1) << 16) |
		((pwrctrl->ufs_srcclkena_mask_b & 0x1) << 17) |
		((pwrctrl->md_vrf18_req_0_mask_b & 0x1) << 18) |
		((pwrctrl->md_vrf18_req_1_mask_b & 0x1) << 19) |
		((pwrctrl->ufs_vrf18_req_mask_b & 0x1) << 20) |
		((pwrctrl->gce_vrf18_req_mask_b & 0x1) << 21) |
		((pwrctrl->conn_infra_req_mask_b & 0x1) << 22) |
		((pwrctrl->gce_apsrc_req_mask_b & 0x1) << 23) |
		((pwrctrl->disp0_apsrc_req_mask_b & 0x1) << 24) |
		((pwrctrl->disp1_apsrc_req_mask_b & 0x1) << 25) |
		((pwrctrl->mfg_req_mask_b & 0x1) << 26) |
		((pwrctrl->vdec_req_mask_b & 0x1) << 27));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->md_ddr_en_0_mask_b & 0x1) << 0) |
		((pwrctrl->md_ddr_en_1_mask_b & 0x1) << 1) |
		((pwrctrl->conn_ddr_en_mask_b & 0x1) << 2) |
		((pwrctrl->ddren_sspm_apsrc_req_mask_b & 0x1) << 3) |
		((pwrctrl->ddren_scp_apsrc_req_mask_b & 0x1) << 4) |
		((pwrctrl->disp0_ddren_mask_b & 0x1) << 5) |
		((pwrctrl->disp1_ddren_mask_b & 0x1) << 6) |
		((pwrctrl->gce_ddren_mask_b & 0x1) << 7) |
		((pwrctrl->ddren_emi_self_refresh_ch0_mask_b & 0x1) << 8) |
		((pwrctrl->ddren_emi_self_refresh_ch1_mask_b & 0x1) << 9));

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
		((pwrctrl->ddren2_sspm_apsrc_req_mask_b & 0x1) << 3) |
		((pwrctrl->ddren2_scp_apsrc_req_mask_b & 0x1) << 4) |
		((pwrctrl->disp0_ddren2_mask_b & 0x1) << 5) |
		((pwrctrl->disp1_ddren2_mask_b & 0x1) << 6) |
		((pwrctrl->gce_ddren2_mask_b & 0x1) << 7) |
		((pwrctrl->ddren2_emi_self_refresh_ch0_mask_b & 0x1) << 8) |
		((pwrctrl->ddren2_emi_self_refresh_ch1_mask_b & 0x1) << 9));

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

	/* MP1_CPU0_WFI_EN */
	mmio_write_32(MP1_CPU0_WFI_EN,
		((pwrctrl->mp1_cpu0_wfi_en & 0x1) << 0));

	/* MP1_CPU1_WFI_EN */
	mmio_write_32(MP1_CPU1_WFI_EN,
		((pwrctrl->mp1_cpu1_wfi_en & 0x1) << 0));

	/* MP1_CPU2_WFI_EN */
	mmio_write_32(MP1_CPU2_WFI_EN,
		((pwrctrl->mp1_cpu2_wfi_en & 0x1) << 0));

	/* MP1_CPU3_WFI_EN */
	mmio_write_32(MP1_CPU3_WFI_EN,
		((pwrctrl->mp1_cpu3_wfi_en & 0x1) << 0));
	/* Auto-gen End */

	/* FIXME: */
#if 0
	/* for gps only case */
	if (spm_for_gps_flag) {
		__uint32_t value;

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
	__uint32_t val, mask, isr;

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

	mmio_write_32(SPM_SW_FLAG, pwrctrl->pcm_flags);

	__spm_set_fw_resume_option(pwrctrl);
	mmio_write_32(SPM_SW_RSV_2, pwrctrl->pcm_flags1);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	__uint32_t con0;

	/* init register to match PCM expectation */
	mmio_write_32(SPM_MAS_PAUSE_MASK_B, 0xffffffff);
	mmio_write_32(SPM_MAS_PAUSE2_MASK_B, 0xffffffff);
	mmio_write_32(PCM_REG_DATA_INI, 0);

	__spm_set_pcm_flags(pwrctrl);

	/* enable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

	/* check IM ready */
	while ((mmio_read_32(PCM_FSM_STA) & (0x7 << 7)) != (0x4 << 7));

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
	wakesta->wake_misc = mmio_read_32(SPM_BSI_D0_SR);   /* backup of SPM_WAKEUP_MISC */

	/* get sleep time */
	wakesta->timer_out = mmio_read_32(SPM_BSI_D1_SR);   /* backup of PCM_TIMER_OUT */

	/* get other SYS and co-clock status */
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);
	wakesta->req_sta = mmio_read_32(SRC_REQ_STA);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = mmio_read_32(SPM_SW_DEBUG);
	wakesta->debug_flag1 = mmio_read_32(WDT_LATCH_SPARE0_FIX);

	/* get special pattern (0xf0000 or 0x10000) if sleep abort */
	wakesta->event_reg = mmio_read_32(SPM_BSI_D2_SR);   /* PCM_EVENT_REG_STA */

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);
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
	   the FW stays in VCORE DVFS which use r0 to Ctrl MEM */
	/* disable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, 0); */

	/* clean CPU wakeup event */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/* [Vcorefs] not disable pcm timer here, due to the
	   following vcore dvfs will use it for latency check */
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
		__uint32_t con1;

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

void __spm_send_cpu_wakeup_event(void)
{
	/* clear PCM_REG_DATA_INI which is pcm assert address */
	mmio_write_32(PCM_REG_DATA_INI, 0);

	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	/* SPM will clear SPM_CPU_WAKEUP_EVENT */
}
