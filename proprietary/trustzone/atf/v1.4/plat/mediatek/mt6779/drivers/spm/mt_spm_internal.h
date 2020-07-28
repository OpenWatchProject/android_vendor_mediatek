#ifndef __MT_SPM_INTERNAL__
#define __MT_SPM_INTERNAL__

/**************************************
 * Config and Parameter
 **************************************/
#define POWER_ON_VAL0_DEF	0x0000F100
/* SPM_POWER_ON_VAL1 */
#define POWER_ON_VAL1_DEF	0x80015810
/* SPM_WAKEUP_EVENT_MASK */
#define SPM_WAKEUP_EVENT_MASK_DEF	0xFFFFFFFF

/* PCM_WDT_VAL */
#define PCM_WDT_TIMEOUT		(30 * 32768)	/* 30s */
/* PCM_TIMER_VAL */
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

/**************************************
 * Define and Declare
 **************************************/
/* PCM_PWR_IO_EN */
#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

/* SPM_SWINT */
#define PCM_SW_INT0		(1U << 0)
#define PCM_SW_INT1		(1U << 1)
#define PCM_SW_INT2		(1U << 2)
#define PCM_SW_INT3		(1U << 3)
#define PCM_SW_INT4		(1U << 4)
#define PCM_SW_INT5		(1U << 5)
#define PCM_SW_INT6		(1U << 6)
#define PCM_SW_INT7		(1U << 7)
#define PCM_SW_INT8		(1U << 8)
#define PCM_SW_INT9		(1U << 9)
#define PCM_SW_INT_ALL		(PCM_SW_INT9 | PCM_SW_INT8 | PCM_SW_INT7 | \
				 PCM_SW_INT6 | PCM_SW_INT5 | PCM_SW_INT4 | \
				 PCM_SW_INT3 | PCM_SW_INT2 | PCM_SW_INT1 | \
				 PCM_SW_INT0)

/* SPM_AP_STANDBY_CON */
#define WFI_OP_AND		1
#define WFI_OP_OR		0

/* SPM_IRQ_MASK */
#define ISRM_TWAM		(1U << 2)
#define ISRM_PCM_RETURN		(1U << 3)
#define ISRM_RET_IRQ0		(1U << 8)
#define ISRM_RET_IRQ1		(1U << 9)
#define ISRM_RET_IRQ2		(1U << 10)
#define ISRM_RET_IRQ3		(1U << 11)
#define ISRM_RET_IRQ4		(1U << 12)
#define ISRM_RET_IRQ5		(1U << 13)
#define ISRM_RET_IRQ6		(1U << 14)
#define ISRM_RET_IRQ7		(1U << 15)
#define ISRM_RET_IRQ8		(1U << 16)
#define ISRM_RET_IRQ9		(1U << 17)
#define ISRM_RET_IRQ_AUX	((ISRM_RET_IRQ9) | (ISRM_RET_IRQ8) | \
				 (ISRM_RET_IRQ7) | (ISRM_RET_IRQ6) | \
				 (ISRM_RET_IRQ5) | (ISRM_RET_IRQ4) | \
				 (ISRM_RET_IRQ3) | (ISRM_RET_IRQ2) | \
				 (ISRM_RET_IRQ1))
#define ISRM_ALL_EXC_TWAM	(ISRM_RET_IRQ_AUX /*| ISRM_RET_IRQ0 | ISRM_PCM_RETURN*/)
#define ISRM_ALL		(ISRM_ALL_EXC_TWAM | ISRM_TWAM)

/* SPM_IRQ_STA */
#define ISRS_TWAM		(1U << 2)
#define ISRS_PCM_RETURN		(1U << 3)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_GIC_WAKEUP             0x3FF  /* bit0 ~ bit9 */
#define WAKE_MISC_DVFSRC_IRQ	         DVFSRC_IRQ_LSB
#define WAKE_MISC_REG_CPU_WAKEUP         SPM_WAKEUP_MISC_REG_CPU_WAKEUP_LSB
#define WAKE_MISC_PCM_TIMER_EVENT        PCM_TIMER_EVENT_LSB
#define WAKE_MISC_PMIC_OUT_B		     ((1U << 19) | (1U << 20))
#define WAKE_MISC_TWAM_IRQ_B             TWAM_IRQ_B_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET0        PMSR_IRQ_B_SET0_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET1        PMSR_IRQ_B_SET1_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET2        PMSR_IRQ_B_SET2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_0   SPM_ACK_CHK_WAKEUP_0_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_1	 SPM_ACK_CHK_WAKEUP_1_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_2	 SPM_ACK_CHK_WAKEUP_2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_3	 SPM_ACK_CHK_WAKEUP_3_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL SPM_ACK_CHK_WAKEUP_ALL_LSB
#define WAKE_MISC_PMIC_IRQ_ACK           PMIC_IRQ_ACK_LSB
#define WAKE_MISC_PMIC_SCP_IRQ           PMIC_SCP_IRQ_LSB

/* MD32PCM ADDR for SPM code fetch */
#define MD32PCM_BASE				(SPM_BASE + 0x0A00)
#define MD32PCM_CFGREG_SW_RSTN		(MD32PCM_BASE + 0x0000)
#define MD32PCM_DMA0_SRC			(MD32PCM_BASE + 0x0200)
#define MD32PCM_DMA0_DST			(MD32PCM_BASE + 0x0204)
#define MD32PCM_DMA0_WPPT			(MD32PCM_BASE + 0x0208)
#define MD32PCM_DMA0_WPTO			(MD32PCM_BASE + 0x020C)
#define MD32PCM_DMA0_COUNT			(MD32PCM_BASE + 0x0210)
#define MD32PCM_DMA0_CON			(MD32PCM_BASE + 0x0214)
#define MD32PCM_DMA0_START			(MD32PCM_BASE + 0x0218)
#define MD32PCM_DMA0_RLCT			(MD32PCM_BASE + 0x0224)
#define MD32PCM_INTC_IRQ_RAW_STA	(MD32PCM_BASE + 0x033C)

/* ABORT MASK for DEBUG FOORTPRINT */
#define DEBUG_ABORT_MASK (DEBUG_IDX_DRAM_SREF_ABORT_IN_APSRC \
	| DEBUG_IDX_DRAM_SREF_ABORT_IN_DDREN)

#define DEBUG_ABORT_MASK_1 (DEBUG_IDX_VTCXO_SLEEP_ABORT_0 \
	| DEBUG_IDX_VTCXO_SLEEP_ABORT_1 \
	| DEBUG_IDX_PMIC_IRQ_ACK_LOW_ABORT \
	| DEBUG_IDX_PMIC_IRQ_ACK_HIGH_ABORT \
	| DEBUG_IDX_PWRAP_SLEEP_ACK_LOW_ABORT \
	| DEBUG_IDX_PWRAP_SLEEP_ACK_HIGH_ABORT \
	| DEBUG_IDX_EMI_SLP_IDLE_ABORT \
	| DEBUG_IDX_SCP_SLP_ACK_LOW_ABORT \
	| DEBUG_IDX_SCP_SLP_ACK_HIGH_ABORT \
	| DEBUG_IDX_SPM_DVFS_CMD_RDY_ABORT \
	| DEBUG_IDX_MCUSYS_PWR_ACK_LOW_ABORT \
	| DEBUG_IDX_CORE_PWR_ACK_HIGH_ABORT)

struct pcm_desc {
	const char *version;	/* PCM code version */
	__uint32_t *base;		/* binary array base */
	__uint32_t base_dma;	/* dma addr of base */
	__uint32_t pmem_words;
	__uint32_t total_words;
	__uint32_t pmem_start;
	__uint32_t dmem_start;
};

struct pwr_ctrl {

	/* for SPM */
	__uint32_t pcm_flags;
	__uint32_t pcm_flags_cust;
	__uint32_t pcm_flags_cust_set;
	__uint32_t pcm_flags_cust_clr;
	__uint32_t pcm_flags1;
	__uint32_t pcm_flags1_cust;
	__uint32_t pcm_flags1_cust_set;
	__uint32_t pcm_flags1_cust_clr;
	__uint32_t timer_val;
	__uint32_t timer_val_cust;
	__uint32_t timer_val_ramp_en;
	__uint32_t timer_val_ramp_en_sec;
	__uint32_t wake_src;
	__uint32_t wake_src_cust;
	__uint32_t wakelock_timer_val;
	__uint8_t wdt_disable;
	/* Auto-gen Start */

	/* SPM_CLK_CON */
	__uint8_t reg_srcclken0_ctl;
	__uint8_t reg_srcclken1_ctl;
	__uint8_t reg_spm_lock_infra_dcm;
	__uint8_t reg_srcclken_mask;
	__uint8_t reg_md1_c32rm_en;
	__uint8_t reg_md2_c32rm_en;
	__uint8_t reg_clksq0_sel_ctrl;
	__uint8_t reg_clksq1_sel_ctrl;
	__uint8_t reg_srcclken0_en;
	__uint8_t reg_srcclken1_en;
	__uint32_t reg_sysclk0_src_mask_b;
	__uint32_t reg_sysclk1_src_mask_b;

	/* SPM_AP_STANDBY_CON */
	__uint8_t reg_wfi_op;
	__uint8_t reg_wfi_type;
	__uint8_t reg_mp0_cputop_idle_mask;
	__uint8_t reg_mp1_cputop_idle_mask;
	__uint8_t reg_mcusys_idle_mask;
	__uint8_t reg_md_apsrc_1_sel;
	__uint8_t reg_md_apsrc_0_sel;
	__uint8_t reg_conn_apsrc_sel;

	/* SPM_SRC_REQ */
	__uint8_t reg_spm_apsrc_req;
	__uint8_t reg_spm_f26m_req;
	__uint8_t reg_spm_infra_req;
	__uint8_t reg_spm_vrf18_req;
	__uint8_t reg_spm_ddr_en_req;
	__uint8_t reg_spm_ddr_en2_req;
	__uint8_t reg_spm_dvfs_req;
	__uint8_t reg_spm_sw_mailbox_req;
	__uint8_t reg_spm_sspm_mailbox_req;
	__uint8_t reg_spm_adsp_mailbox_req;
	__uint8_t reg_spm_scp_mailbox_req;
	__uint8_t reg_spm_mcusys_pwr_event_req;
	__uint8_t cpu_md_dvfs_sop_force_on;

	/* SPM_SRC_MASK */
	__uint8_t reg_md_srcclkena_0_mask_b;
	__uint8_t reg_md_srcclkena2infra_req_0_mask_b;
	__uint8_t reg_md_apsrc2infra_req_0_mask_b;
	__uint8_t reg_md_apsrc_req_0_mask_b;
	__uint8_t reg_md_vrf18_req_0_mask_b;
	__uint8_t reg_md_ddr_en_0_mask_b;
	__uint8_t reg_md_ddr_en2_0_mask_b;
	__uint8_t reg_md_srcclkena_1_mask_b;
	__uint8_t reg_md_srcclkena2infra_req_1_mask_b;
	__uint8_t reg_md_apsrc2infra_req_1_mask_b;
	__uint8_t reg_md_apsrc_req_1_mask_b;
	__uint8_t reg_md_vrf18_req_1_mask_b;
	__uint8_t reg_md_ddr_en_1_mask_b;
	__uint8_t reg_md_ddr_en2_1_mask_b;
	__uint8_t reg_conn_srcclkena_mask_b;
	__uint8_t reg_conn_srcclkenb_mask_b;
	__uint8_t reg_conn_infra_req_mask_b;
	__uint8_t reg_conn_apsrc_req_mask_b;
	__uint8_t reg_conn_vrf18_req_mask_b;
	__uint8_t reg_conn_ddr_en_mask_b;
	__uint8_t reg_conn_ddr_en2_mask_b;
	__uint8_t reg_srcclkeni0_srcclkena_mask_b;
	__uint8_t reg_srcclkeni0_infra_req_mask_b;
	__uint8_t reg_srcclkeni1_srcclkena_mask_b;
	__uint8_t reg_srcclkeni1_infra_req_mask_b;
	__uint8_t reg_srcclkeni2_srcclkena_mask_b;
	__uint8_t reg_srcclkeni2_infra_req_mask_b;
	__uint8_t reg_infrasys_apsrc_req_mask_b;
	__uint8_t reg_infrasys_ddr_en_mask_b;
	__uint8_t reg_infrasys_ddr_en2_mask_b;
	__uint8_t reg_md32_srcclkena_mask_b;
	__uint8_t reg_conn_vfe28_req_mask_b;

	/* SPM_SRC2_MASK */
	__uint8_t reg_md32_infra_req_mask_b;
	__uint8_t reg_md32_apsrc_req_mask_b;
	__uint8_t reg_md32_vrf18_req_mask_b;
	__uint8_t reg_md32_ddr_en_mask_b;
	__uint8_t reg_md32_ddr_en2_mask_b;
	__uint8_t reg_scp_srcclkena_mask_b;
	__uint8_t reg_scp_infra_req_mask_b;
	__uint8_t reg_scp_apsrc_req_mask_b;
	__uint8_t reg_scp_vrf18_req_mask_b;
	__uint8_t reg_scp_ddr_en_mask_b;
	__uint8_t reg_scp_ddr_en2_mask_b;
	__uint8_t reg_ufs_srcclkena_mask_b;
	__uint8_t reg_ufs_infra_req_mask_b;
	__uint8_t reg_ufs_apsrc_req_mask_b;
	__uint8_t reg_ufs_vrf18_req_mask_b;
	__uint8_t reg_ufs_ddr_en_mask_b;
	__uint8_t reg_ufs_ddr_en2_mask_b;
	__uint8_t reg_disp0_apsrc_req_mask_b;
	__uint8_t reg_disp0_ddr_en_mask_b;
	__uint8_t reg_disp0_ddr_en2_mask_b;
	__uint8_t reg_disp1_apsrc_req_mask_b;
	__uint8_t reg_disp1_ddr_en_mask_b;
	__uint8_t reg_disp1_ddr_en2_mask_b;
	__uint8_t reg_gce_infra_req_mask_b;
	__uint8_t reg_gce_apsrc_req_mask_b;
	__uint8_t reg_gce_vrf18_req_mask_b;
	__uint8_t reg_gce_ddr_en_mask_b;
	__uint8_t reg_gce_ddr_en2_mask_b;
	__uint8_t reg_emi_ch0_ddr_en_mask_b;
	__uint8_t reg_emi_ch1_ddr_en_mask_b;
	__uint8_t reg_emi_ch0_ddr_en2_mask_b;
	__uint8_t reg_emi_ch1_ddr_en2_mask_b;

	/* SPM_SRC3_MASK */
	__uint8_t reg_dvfsrc_event_trigger_mask_b;
	__uint8_t reg_sw2spm_int0_mask_b;
	__uint8_t reg_sw2spm_int1_mask_b;
	__uint8_t reg_sw2spm_int2_mask_b;
	__uint8_t reg_sw2spm_int3_mask_b;
	__uint8_t reg_sc_adsp2spm_wakeup_mask_b;
	__uint8_t reg_sc_sspm2spm_wakeup_mask_b;
	__uint8_t reg_sc_scp2spm_wakeup_mask_b;
	__uint8_t reg_csyspwrreq_mask;
	__uint8_t reg_spm_srcclkena_reserved_mask_b;
	__uint8_t reg_spm_infra_req_reserved_mask_b;
	__uint8_t reg_spm_apsrc_req_reserved_mask_b;
	__uint8_t reg_spm_vrf18_req_reserved_mask_b;
	__uint8_t reg_spm_ddr_en_reserved_mask_b;
	__uint8_t reg_spm_ddr_en2_reserved_mask_b;
	__uint8_t reg_audio_dsp_srcclkena_mask_b;
	__uint8_t reg_audio_dsp_infra_req_mask_b;
	__uint8_t reg_audio_dsp_apsrc_req_mask_b;
	__uint8_t reg_audio_dsp_vrf18_req_mask_b;
	__uint8_t reg_audio_dsp_ddr_en_mask_b;
	__uint8_t reg_audio_dsp_ddr_en2_mask_b;
	__uint8_t reg_mcusys_pwr_event_mask_b;
	__uint8_t reg_msdc0_srcclkena_mask_b;
	__uint8_t reg_msdc0_infra_req_mask_b;
	__uint8_t reg_msdc0_apsrc_req_mask_b;
	__uint8_t reg_msdc0_vrf18_req_mask_b;
	__uint8_t reg_msdc0_ddr_en_mask_b;
	__uint8_t reg_msdc0_ddr_en2_mask_b;
	__uint8_t reg_conn_srcclkenb2pwrap_mask_b;

	/* SPM_SRC4_MASK */
	__uint32_t ccif_event_mask_b;
	__uint8_t reg_apu_core0_srcclkena_mask_b;
	__uint8_t reg_apu_core0_infra_req_mask_b;
	__uint8_t reg_apu_core0_apsrc_req_mask_b;
	__uint8_t reg_apu_core0_vrf18_req_mask_b;
	__uint8_t reg_apu_core0_ddr_en_mask_b;
	__uint8_t reg_apu_core1_srcclkena_mask_b;
	__uint8_t reg_apu_core1_infra_req_mask_b;
	__uint8_t reg_apu_core1_apsrc_req_mask_b;
	__uint8_t reg_apu_core1_vrf18_req_mask_b;
	__uint8_t reg_apu_core1_ddr_en_mask_b;
	__uint8_t reg_apu_core2_srcclkena_mask_b;
	__uint8_t reg_apu_core2_infra_req_mask_b;
	__uint8_t reg_apu_core2_apsrc_req_mask_b;
	__uint8_t reg_apu_core2_vrf18_req_mask_b;
	__uint8_t reg_apu_core2_ddr_en_mask_b;
	__uint8_t reg_apu_core2_ddr_en2_mask_b;

	/* SPM_SRC5_MASK */
	__uint32_t reg_mcusys_merge_apsrc_req_mask_b;
	__uint32_t reg_mcusys_merge_ddr_en_mask_b;
	__uint32_t reg_mcusys_merge_ddr_en2_mask_b;
	__uint8_t reg_apu_core0_ddr_en2_mask_b;
	__uint8_t reg_apu_core1_ddr_en2_mask_b;
	__uint8_t reg_cg_check_ddr_en_mask_b;
	__uint8_t reg_cg_check_ddr_en2_mask_b;

	/* SPM_WAKEUP_EVENT_MASK */
	__uint32_t reg_wakeup_event_mask;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	__uint32_t reg_ext_wakeup_event_mask;

	/* SPM_SRC6_MASK */
	__uint8_t reg_msdc1_srcclkena_mask_b;
	__uint8_t reg_msdc1_infra_req_mask_b;
	__uint8_t reg_msdc1_apsrc_req_mask_b;
	__uint8_t reg_msdc1_vrf18_req_mask_b;
	__uint8_t reg_msdc1_ddr_en_mask_b;
	__uint8_t reg_msdc1_ddr_en2_mask_b;
	__uint8_t reg_msdc1_srcclkena_ack_mask;
	__uint8_t reg_msdc1_infra_ack_mask;
	__uint8_t reg_msdc1_apsrc_ack_mask;
	__uint8_t reg_msdc1_vrf18_ack_mask;
	__uint8_t reg_msdc1_ddr_en_ack_mask;
	__uint8_t reg_msdc1_ddr_en2_ack_mask;

	/* MP0_CPU0_WFI_EN */
	__uint8_t mp0_cpu0_wfi_en;

	/* MP0_CPU1_WFI_EN */
	__uint8_t mp0_cpu1_wfi_en;

	/* MP0_CPU2_WFI_EN */
	__uint8_t mp0_cpu2_wfi_en;

	/* MP0_CPU3_WFI_EN */
	__uint8_t mp0_cpu3_wfi_en;

	/* MP0_CPU4_WFI_EN */
	__uint8_t mp0_cpu4_wfi_en;

	/* MP0_CPU5_WFI_EN */
	__uint8_t mp0_cpu5_wfi_en;

	/* MP0_CPU6_WFI_EN */
	__uint8_t mp0_cpu6_wfi_en;

	/* MP0_CPU7_WFI_EN */
	__uint8_t mp0_cpu7_wfi_en;

	/* Auto-gen End */
};


/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
enum pwr_ctrl_enum {
	PW_PCM_FLAGS,
	PW_PCM_FLAGS_CUST,
	PW_PCM_FLAGS_CUST_SET,
	PW_PCM_FLAGS_CUST_CLR,
	PW_PCM_FLAGS1,
	PW_PCM_FLAGS1_CUST,
	PW_PCM_FLAGS1_CUST_SET,
	PW_PCM_FLAGS1_CUST_CLR,
	PW_TIMER_VAL,
	PW_TIMER_VAL_CUST,
	PW_TIMER_VAL_RAMP_EN,
	PW_TIMER_VAL_RAMP_EN_SEC,
	PW_WAKE_SRC,
	PW_WAKE_SRC_CUST,
	PW_WAKELOCK_TIMER_VAL,
	PW_WDT_DISABLE,
	PW_REG_SRCCLKEN0_CTL,
	PW_REG_SRCCLKEN1_CTL,
	PW_REG_SPM_LOCK_INFRA_DCM,
	PW_REG_SRCCLKEN_MASK,
	PW_REG_MD1_C32RM_EN,
	PW_REG_MD2_C32RM_EN,
	PW_REG_CLKSQ0_SEL_CTRL,
	PW_REG_CLKSQ1_SEL_CTRL,
	PW_REG_SRCCLKEN0_EN,
	PW_REG_SRCCLKEN1_EN,
	PW_REG_SYSCLK0_SRC_MASK_B,
	PW_REG_SYSCLK1_SRC_MASK_B,
	PW_REG_WFI_OP,
	PW_REG_WFI_TYPE,
	PW_REG_MP0_CPUTOP_IDLE_MASK,
	PW_REG_MP1_CPUTOP_IDLE_MASK,
	PW_REG_MCUSYS_IDLE_MASK,
	PW_REG_MD_APSRC_1_SEL,
	PW_REG_MD_APSRC_0_SEL,
	PW_REG_CONN_APSRC_SEL,
	PW_REG_SPM_APSRC_REQ,
	PW_REG_SPM_F26M_REQ,
	PW_REG_SPM_INFRA_REQ,
	PW_REG_SPM_VRF18_REQ,
	PW_REG_SPM_DDR_EN_REQ,
	PW_REG_SPM_DDR_EN2_REQ,
	PW_REG_SPM_DVFS_REQ,
	PW_REG_SPM_SW_MAILBOX_REQ,
	PW_REG_SPM_SSPM_MAILBOX_REQ,
	PW_REG_SPM_ADSP_MAILBOX_REQ,
	PW_REG_SPM_SCP_MAILBOX_REQ,
	PW_REG_SPM_MCUSYS_PWR_EVENT_REQ,
	PW_CPU_MD_DVFS_SOP_FORCE_ON,
	PW_REG_MD_SRCCLKENA_0_MASK_B,
	PW_REG_MD_SRCCLKENA2INFRA_REQ_0_MASK_B,
	PW_REG_MD_APSRC2INFRA_REQ_0_MASK_B,
	PW_REG_MD_APSRC_REQ_0_MASK_B,
	PW_REG_MD_VRF18_REQ_0_MASK_B,
	PW_REG_MD_DDR_EN_0_MASK_B,
	PW_REG_MD_DDR_EN2_0_MASK_B,
	PW_REG_MD_SRCCLKENA_1_MASK_B,
	PW_REG_MD_SRCCLKENA2INFRA_REQ_1_MASK_B,
	PW_REG_MD_APSRC2INFRA_REQ_1_MASK_B,
	PW_REG_MD_APSRC_REQ_1_MASK_B,
	PW_REG_MD_VRF18_REQ_1_MASK_B,
	PW_REG_MD_DDR_EN_1_MASK_B,
	PW_REG_MD_DDR_EN2_1_MASK_B,
	PW_REG_CONN_SRCCLKENA_MASK_B,
	PW_REG_CONN_SRCCLKENB_MASK_B,
	PW_REG_CONN_INFRA_REQ_MASK_B,
	PW_REG_CONN_APSRC_REQ_MASK_B,
	PW_REG_CONN_VRF18_REQ_MASK_B,
	PW_REG_CONN_DDR_EN_MASK_B,
	PW_REG_CONN_DDR_EN2_MASK_B,
	PW_REG_SRCCLKENI0_SRCCLKENA_MASK_B,
	PW_REG_SRCCLKENI0_INFRA_REQ_MASK_B,
	PW_REG_SRCCLKENI1_SRCCLKENA_MASK_B,
	PW_REG_SRCCLKENI1_INFRA_REQ_MASK_B,
	PW_REG_SRCCLKENI2_SRCCLKENA_MASK_B,
	PW_REG_SRCCLKENI2_INFRA_REQ_MASK_B,
	PW_REG_INFRASYS_APSRC_REQ_MASK_B,
	PW_REG_INFRASYS_DDR_EN_MASK_B,
	PW_REG_INFRASYS_DDR_EN2_MASK_B,
	PW_REG_MD32_SRCCLKENA_MASK_B,
	PW_REG_CONN_VFE28_REQ_MASK_B,
	PW_REG_MD32_INFRA_REQ_MASK_B,
	PW_REG_MD32_APSRC_REQ_MASK_B,
	PW_REG_MD32_VRF18_REQ_MASK_B,
	PW_REG_MD32_DDR_EN_MASK_B,
	PW_REG_MD32_DDR_EN2_MASK_B,
	PW_REG_SCP_SRCCLKENA_MASK_B,
	PW_REG_SCP_INFRA_REQ_MASK_B,
	PW_REG_SCP_APSRC_REQ_MASK_B,
	PW_REG_SCP_VRF18_REQ_MASK_B,
	PW_REG_SCP_DDR_EN_MASK_B,
	PW_REG_SCP_DDR_EN2_MASK_B,
	PW_REG_UFS_SRCCLKENA_MASK_B,
	PW_REG_UFS_INFRA_REQ_MASK_B,
	PW_REG_UFS_APSRC_REQ_MASK_B,
	PW_REG_UFS_VRF18_REQ_MASK_B,
	PW_REG_UFS_DDR_EN_MASK_B,
	PW_REG_UFS_DDR_EN2_MASK_B,
	PW_REG_DISP0_APSRC_REQ_MASK_B,
	PW_REG_DISP0_DDR_EN_MASK_B,
	PW_REG_DISP0_DDR_EN2_MASK_B,
	PW_REG_DISP1_APSRC_REQ_MASK_B,
	PW_REG_DISP1_DDR_EN_MASK_B,
	PW_REG_DISP1_DDR_EN2_MASK_B,
	PW_REG_GCE_INFRA_REQ_MASK_B,
	PW_REG_GCE_APSRC_REQ_MASK_B,
	PW_REG_GCE_VRF18_REQ_MASK_B,
	PW_REG_GCE_DDR_EN_MASK_B,
	PW_REG_GCE_DDR_EN2_MASK_B,
	PW_REG_EMI_CH0_DDR_EN_MASK_B,
	PW_REG_EMI_CH1_DDR_EN_MASK_B,
	PW_REG_EMI_CH0_DDR_EN2_MASK_B,
	PW_REG_EMI_CH1_DDR_EN2_MASK_B,
	PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B,
	PW_REG_SW2SPM_INT0_MASK_B,
	PW_REG_SW2SPM_INT1_MASK_B,
	PW_REG_SW2SPM_INT2_MASK_B,
	PW_REG_SW2SPM_INT3_MASK_B,
	PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B,
	PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B,
	PW_REG_SC_SCP2SPM_WAKEUP_MASK_B,
	PW_REG_CSYSPWRREQ_MASK,
	PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B,
	PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B,
	PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B,
	PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B,
	PW_REG_SPM_DDR_EN_RESERVED_MASK_B,
	PW_REG_SPM_DDR_EN2_RESERVED_MASK_B,
	PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B,
	PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B,
	PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B,
	PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B,
	PW_REG_AUDIO_DSP_DDR_EN_MASK_B,
	PW_REG_AUDIO_DSP_DDR_EN2_MASK_B,
	PW_REG_MCUSYS_PWR_EVENT_MASK_B,
	PW_REG_MSDC0_SRCCLKENA_MASK_B,
	PW_REG_MSDC0_INFRA_REQ_MASK_B,
	PW_REG_MSDC0_APSRC_REQ_MASK_B,
	PW_REG_MSDC0_VRF18_REQ_MASK_B,
	PW_REG_MSDC0_DDR_EN_MASK_B,
	PW_REG_MSDC0_DDR_EN2_MASK_B,
	PW_REG_CONN_SRCCLKENB2PWRAP_MASK_B,
	PW_CCIF_EVENT_MASK_B,
	PW_REG_APU_CORE0_SRCCLKENA_MASK_B,
	PW_REG_APU_CORE0_INFRA_REQ_MASK_B,
	PW_REG_APU_CORE0_APSRC_REQ_MASK_B,
	PW_REG_APU_CORE0_VRF18_REQ_MASK_B,
	PW_REG_APU_CORE0_DDR_EN_MASK_B,
	PW_REG_APU_CORE1_SRCCLKENA_MASK_B,
	PW_REG_APU_CORE1_INFRA_REQ_MASK_B,
	PW_REG_APU_CORE1_APSRC_REQ_MASK_B,
	PW_REG_APU_CORE1_VRF18_REQ_MASK_B,
	PW_REG_APU_CORE1_DDR_EN_MASK_B,
	PW_REG_APU_CORE2_SRCCLKENA_MASK_B,
	PW_REG_APU_CORE2_INFRA_REQ_MASK_B,
	PW_REG_APU_CORE2_APSRC_REQ_MASK_B,
	PW_REG_APU_CORE2_VRF18_REQ_MASK_B,
	PW_REG_APU_CORE2_DDR_EN_MASK_B,
	PW_REG_APU_CORE2_DDR_EN2_MASK_B,
	PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B,
	PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B,
	PW_REG_MCUSYS_MERGE_DDR_EN2_MASK_B,
	PW_REG_APU_CORE0_DDR_EN2_MASK_B,
	PW_REG_APU_CORE1_DDR_EN2_MASK_B,
	PW_REG_CG_CHECK_DDR_EN_MASK_B,
	PW_REG_CG_CHECK_DDR_EN2_MASK_B,
	PW_REG_WAKEUP_EVENT_MASK,
	PW_REG_EXT_WAKEUP_EVENT_MASK,
	PW_REG_MSDC1_SRCCLKENA_MASK_B,
	PW_REG_MSDC1_INFRA_REQ_MASK_B,
	PW_REG_MSDC1_APSRC_REQ_MASK_B,
	PW_REG_MSDC1_VRF18_REQ_MASK_B,
	PW_REG_MSDC1_DDR_EN_MASK_B,
	PW_REG_MSDC1_DDR_EN2_MASK_B,
	PW_REG_MSDC1_SRCCLKENA_ACK_MASK,
	PW_REG_MSDC1_INFRA_ACK_MASK,
	PW_REG_MSDC1_APSRC_ACK_MASK,
	PW_REG_MSDC1_VRF18_ACK_MASK,
	PW_REG_MSDC1_DDR_EN_ACK_MASK,
	PW_REG_MSDC1_DDR_EN2_ACK_MASK,
	PW_MP0_CPU0_WFI_EN,
	PW_MP0_CPU1_WFI_EN,
	PW_MP0_CPU2_WFI_EN,
	PW_MP0_CPU3_WFI_EN,
	PW_MP0_CPU4_WFI_EN,
	PW_MP0_CPU5_WFI_EN,
	PW_MP0_CPU6_WFI_EN,
	PW_MP0_CPU7_WFI_EN,
	PW_MAX_COUNT,
};

enum {
	SPM_SUSPEND,
	SPM_RESUME,
	SPM_DPIDLE_ENTER,
	SPM_DPIDLE_LEAVE,
	SPM_ENTER_SODI,
	SPM_LEAVE_SODI,
	SPM_ENTER_SODI3,
	SPM_LEAVE_SODI3,
	SPM_SUSPEND_PREPARE,
	SPM_POST_SUSPEND,
	SPM_DPIDLE_PREPARE,
	SPM_POST_DPIDLE,
	SPM_SODI_PREPARE,
	SPM_POST_SODI,
	SPM_SODI3_PREPARE,
	SPM_POST_SODI3,
	SPM_VCORE_PWARP_CMD,
	SPM_PWR_CTRL_SUSPEND,
	SPM_PWR_CTRL_DPIDLE,
	SPM_PWR_CTRL_SODI,
	SPM_PWR_CTRL_SODI3,
	SPM_PWR_CTRL_VCOREFS,
};

#define DYNA_LOAD_PCM_PATH_SIZE 128
#define PCM_FIRMWARE_VERSION_SIZE 128

enum dyna_load_pcm_index {
	DYNA_LOAD_PCM_SUSPEND = 0,
	DYNA_LOAD_PCM_MAX,
};

struct dyna_load_pcm_t {
	char path[DYNA_LOAD_PCM_PATH_SIZE];
	char version[PCM_FIRMWARE_VERSION_SIZE];
	char *buf;
	struct pcm_desc desc;
	int ready;
};

extern struct dyna_load_pcm_t dyna_load_pcm[DYNA_LOAD_PCM_MAX];

struct wake_status {
	__uint32_t r12;				/* SPM_SW_RSV_0 */
	__uint32_t r12_ext;			/* SPM_WAKEUP_EXT_STA */
	__uint32_t raw_sta;			/* SPM_WAKEUP_STA */
	__uint32_t raw_ext_sta;			/* SPM_WAKEUP_EXT_STA */
	__uint32_t md32pcm_wakeup_sta;		/* MD32PCM_WAKEUP_STA */
	__uint32_t md32pcm_event_sta;		/* MD32PCM_EVENT_STA */
	__uint32_t wake_misc;			/* SPM_SW_RSV_5 */
	__uint32_t timer_out;			/* SPM_SW_RSV_6*/
	__uint32_t r13;				/* PCM_REG13_DATA */
	__uint32_t idle_sta;			/* SUBSYS_IDLE_STA */
	__uint32_t req_sta0;			/* SRC_REQ_STA_0 */
	__uint32_t req_sta1;			/* SRC_REQ_STA_1 */
	__uint32_t req_sta2;			/* SRC_REQ_STA_2 */
	__uint32_t req_sta3;			/* SRC_REQ_STA_3 */
	__uint32_t req_sta4;			/* SRC_REQ_STA_4 */
	__uint32_t debug_flag;			/* PCM_WDT_LATCH_SPARE_0 */
	__uint32_t debug_flag1;			/* PCM_WDT_LATCH_SPARE_1 */
	__uint32_t b_sw_flag0;			/* SPM_SW_RSV_7 */
	__uint32_t b_sw_flag1;			/* SPM_SW_RSV_8 */
	__uint32_t isr;				/* SPM_IRQ_STA */
	__uint32_t sw_flag0;			/* SPM_SW_FLAG_0 */
	__uint32_t sw_flag1;			/* SPM_SW_FLAG_1 */
	__uint32_t log_index;
	__uint32_t is_abort;
};

struct spm_lp_scen {
	struct pcm_desc *pcmdesc;
	struct pwr_ctrl *pwrctrl;
};

extern struct spm_lp_scen __spm_suspend;
extern struct spm_lp_scen __spm_vcorefs;
extern struct spm_lp_scen __spm_dpidle;
extern struct spm_lp_scen __spm_sodi;

extern int __spm_get_spmfw_idx(void);
extern void __spm_set_cpu_status(int cpu);
extern void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc);
extern void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc);

extern void __spm_init_pcm_register(void);	/* init r0 and r7 */
extern void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage);
extern void __spm_set_power_control(const struct pwr_ctrl *pwrctrl);
extern void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
extern void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl);
extern void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl);
extern void __spm_send_cpu_wakeup_event(void);

extern void __spm_get_wakeup_status(struct wake_status *wakesta);
extern void __spm_clean_after_wakeup(void);
extern wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta,
		const struct pcm_desc *pcmdesc);

extern void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl);

extern void __spm_set_pcm_wdt(int en);
extern __uint32_t _spm_get_wake_period(int pwake_time, wake_reason_t last_wr);
extern void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl);
extern void __spm_ext_int_wakeup_req_clr(void);

static inline void set_pwrctrl_pcm_flags(struct pwr_ctrl *pwrctrl, __uint32_t flags)
{
	if (pwrctrl->pcm_flags_cust == 0)
		pwrctrl->pcm_flags = flags;
	else
		pwrctrl->pcm_flags = pwrctrl->pcm_flags_cust;
}

static inline void set_pwrctrl_pcm_flags1(struct pwr_ctrl *pwrctrl, __uint32_t flags)
{
	if (pwrctrl->pcm_flags1_cust == 0)
		pwrctrl->pcm_flags1 = flags;
	else
		pwrctrl->pcm_flags1 = pwrctrl->pcm_flags1_cust;
}

#endif /* __MT_SPM_INTERNAL__ */
