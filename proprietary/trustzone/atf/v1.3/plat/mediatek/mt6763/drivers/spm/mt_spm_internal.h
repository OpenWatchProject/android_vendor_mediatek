#ifndef __MT_SPM_INTERNAL__
#define __MT_SPM_INTERNAL__

/**************************************
 * Config and Parameter
 **************************************/
// SPM_POWER_ON_VAL1
#define POWER_ON_VAL1_DEF	0x00015820
// PCM_FSM_STA
#define PCM_FSM_STA_DEF		0x00048490
// SPM_WAKEUP_EVENT_MASK
#define SPM_WAKEUP_EVENT_MASK_DEF	0xF0F92218

// PCM_WDT_VAL
#define PCM_WDT_TIMEOUT		(30 * 32768)	/* 30s */
// PCM_TIMER_VAL
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

/**************************************
 * Define and Declare
 **************************************/
// PCM_PWR_IO_EN
#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

// SPM_SWINT
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

// SPM_AP_STANDBY_CON
#define WFI_OP_AND		1
#define WFI_OP_OR		0

// SPM_IRQ_MASK
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
#define ISRM_RET_IRQ_AUX	(ISRM_RET_IRQ9 | ISRM_RET_IRQ8 | \
				 ISRM_RET_IRQ7 | ISRM_RET_IRQ6 | \
				 ISRM_RET_IRQ5 | ISRM_RET_IRQ4 | \
				 ISRM_RET_IRQ3 | ISRM_RET_IRQ2 | \
				 ISRM_RET_IRQ1)
#define ISRM_ALL_EXC_TWAM	(ISRM_RET_IRQ_AUX /*| ISRM_RET_IRQ0 | ISRM_PCM_RETURN*/)
#define ISRM_ALL		(ISRM_ALL_EXC_TWAM | ISRM_TWAM)

// SPM_IRQ_STA
#define ISRS_TWAM		(1U << 2)
#define ISRS_PCM_RETURN		(1U << 3)
#define ISRS_SW_INT0		(1U << 4)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

// SPM_WAKEUP_MISC
#define WAKE_MISC_TWAM		(1U << 18)
#define WAKE_MISC_PCM_TIMER	(1U << 19)
#define WAKE_MISC_CPU_WAKE	(1U << 20)

struct pcm_desc {
	const char *version;	/* PCM code version */
	__uint32_t *base;	/* binary array base */
	__uint32_t base_dma;	/* dma addr of base */
	const __uint16_t size;		/* binary array size */
	const __uint8_t sess;		/* session number */
	const __uint8_t replace;	/* replace mode */
	const __uint16_t addr_2nd;	/* 2nd binary array size */
	const __uint16_t reserved;	/* for 32bit alignment */

	__uint32_t vec0;		/* event vector 0 config */
	__uint32_t vec1;		/* event vector 1 config */
	__uint32_t vec2;		/* event vector 2 config */
	__uint32_t vec3;		/* event vector 3 config */
	__uint32_t vec4;		/* event vector 4 config */
	__uint32_t vec5;		/* event vector 5 config */
	__uint32_t vec6;		/* event vector 6 config */
	__uint32_t vec7;		/* event vector 7 config */
	__uint32_t vec8;		/* event vector 8 config */
	__uint32_t vec9;		/* event vector 9 config */
	__uint32_t vec10;		/* event vector 10 config */
	__uint32_t vec11;		/* event vector 11 config */
	__uint32_t vec12;		/* event vector 12 config */
	__uint32_t vec13;		/* event vector 13 config */
	__uint32_t vec14;		/* event vector 14 config */
	__uint32_t vec15;		/* event vector 15 config */
};

struct pwr_ctrl {
	/* for SPM */
	__uint32_t pcm_flags;
	__uint32_t pcm_flags_cust;	/* can override pcm_flags */
	__uint32_t pcm_flags_cust_set;	/* set bit of pcm_flags, after pcm_flags_cust */
	__uint32_t pcm_flags_cust_clr;	/* clr bit of pcm_flags, after pcm_flags_cust */
	__uint32_t pcm_flags1;
	__uint32_t pcm_flags1_cust;	/* can override pcm_flags1 */
	__uint32_t pcm_flags1_cust_set;	/* set bit of pcm_flags1, after pcm_flags1_cust */
	__uint32_t pcm_flags1_cust_clr;	/* clr bit of pcm_flags1, after pcm_flags1_cust */
	__uint32_t timer_val;		/* @ 1T 32K */
	__uint32_t timer_val_cust;	/* @ 1T 32K, can override timer_val */
	__uint32_t timer_val_ramp_en;		/* stress for dpidle */
	__uint32_t timer_val_ramp_en_sec;	/* stress for suspend */
	__uint32_t wake_src;
	__uint32_t wake_src_cust;	/* can override wake_src */
	__uint32_t wakelock_timer_val;
	__uint8_t wdt_disable;		/* disable wdt in suspend */

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	__uint8_t wfi_op;
	__uint8_t mp0_cputop_idle_mask;
	__uint8_t mp1_cputop_idle_mask;
	__uint8_t mcusys_idle_mask;
	__uint8_t mm_mask_b;
	__uint8_t md_ddr_en_0_dbc_en;
	__uint8_t md_ddr_en_1_dbc_en;
	__uint8_t md_mask_b;
	__uint8_t sspm_mask_b;
	__uint8_t lte_mask_b;
	__uint8_t srcclkeni_mask_b;
	__uint8_t md_apsrc_1_sel;
	__uint8_t md_apsrc_0_sel;
	__uint8_t conn_ddr_en_dbc_en;
	__uint8_t conn_mask_b;
	__uint8_t conn_apsrc_sel;

	/* SPM_SRC_REQ */
	__uint8_t spm_apsrc_req;
	__uint8_t spm_f26m_req;
	__uint8_t spm_lte_req;
	__uint8_t spm_infra_req;
	__uint8_t spm_vrf18_req;
	__uint8_t spm_dvfs_req;
	__uint8_t spm_dvfs_force_down;
	__uint8_t spm_ddren_req;
	__uint8_t spm_rsv_src_req;
	__uint8_t spm_ddren_2_req;
	__uint8_t cpu_md_dvfs_sop_force_on;

	/* SPM_SRC_MASK */
	__uint8_t csyspwreq_mask;
	__uint8_t ccif0_md_event_mask_b;
	__uint8_t ccif0_ap_event_mask_b;
	__uint8_t ccif1_md_event_mask_b;
	__uint8_t ccif1_ap_event_mask_b;
	__uint8_t ccifmd_md1_event_mask_b;
	__uint8_t ccifmd_md2_event_mask_b;
	__uint8_t dsi0_vsync_mask_b;
	__uint8_t dsi1_vsync_mask_b;
	__uint8_t dpi_vsync_mask_b;
	__uint8_t isp0_vsync_mask_b;
	__uint8_t isp1_vsync_mask_b;
	__uint8_t md_srcclkena_0_infra_mask_b;
	__uint8_t md_srcclkena_1_infra_mask_b;
	__uint8_t conn_srcclkena_infra_mask_b;
	__uint8_t sspm_srcclkena_infra_mask_b;
	__uint8_t srcclkeni_infra_mask_b;
	__uint8_t md_apsrc_req_0_infra_mask_b;
	__uint8_t md_apsrc_req_1_infra_mask_b;
	__uint8_t conn_apsrcreq_infra_mask_b;
	__uint8_t sspm_apsrcreq_infra_mask_b;
	__uint8_t md_ddr_en_0_mask_b;
	__uint8_t md_ddr_en_1_mask_b;
	__uint8_t md_vrf18_req_0_mask_b;
	__uint8_t md_vrf18_req_1_mask_b;
	__uint8_t md1_dvfs_req_mask;
	__uint8_t cpu_dvfs_req_mask;
	__uint8_t emi_bw_dvfs_req_mask;
	__uint8_t md_srcclkena_0_dvfs_req_mask_b;
	__uint8_t md_srcclkena_1_dvfs_req_mask_b;
	__uint8_t conn_srcclkena_dvfs_req_mask_b;

	/* SPM_SRC2_MASK */
	__uint8_t dvfs_halt_mask_b;
	__uint8_t vdec_req_mask_b;
	__uint8_t gce_req_mask_b;
	__uint8_t cpu_md_dvfs_req_merge_mask_b;
	__uint8_t md_ddr_en_dvfs_halt_mask_b;
	__uint8_t dsi0_vsync_dvfs_halt_mask_b;
	__uint8_t dsi1_vsync_dvfs_halt_mask_b;
	__uint8_t dpi_vsync_dvfs_halt_mask_b;
	__uint8_t isp0_vsync_dvfs_halt_mask_b;
	__uint8_t isp1_vsync_dvfs_halt_mask_b;
	__uint8_t conn_ddr_en_mask_b;
	__uint8_t disp_req_mask_b;
	__uint8_t disp1_req_mask_b;
	__uint8_t mfg_req_mask_b;
	__uint8_t ufs_srcclkena_mask_b;
	__uint8_t ufs_vrf18_req_mask_b;
	__uint8_t ps_c2k_rccif_wake_mask_b;
	__uint8_t l1_c2k_rccif_wake_mask_b;
	__uint8_t sdio_on_dvfs_req_mask_b;
	__uint8_t emi_boost_dvfs_req_mask_b;
	__uint8_t cpu_md_emi_dvfs_req_prot_dis;
	__uint8_t dramc_spcmd_apsrc_req_mask_b;
	__uint8_t emi_boost_dvfs_req_2_mask_b;
	__uint8_t emi_bw_dvfs_req_2_mask;
	__uint8_t gce_vrf18_req_mask_b;

	/* SPM_WAKEUP_EVENT_MASK */
	__uint32_t spm_wakeup_event_mask;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	__uint32_t spm_wakeup_event_ext_mask;

	/* SPM_SRC3_MASK */
	__uint8_t md_ddr_en_2_0_mask_b;
	__uint8_t md_ddr_en_2_1_mask_b;
	__uint8_t conn_ddr_en_2_mask_b;
	__uint8_t dramc_spcmd_apsrc_req_2_mask_b;
	__uint8_t spare1_ddren_2_mask_b;
	__uint8_t spare2_ddren_2_mask_b;
	__uint8_t ddren_emi_self_refresh_ch0_mask_b;
	__uint8_t ddren_emi_self_refresh_ch1_mask_b;
	__uint8_t ddren_mm_state_mask_b;
	__uint8_t ddren_sspm_apsrc_req_mask_b;
	__uint8_t ddren_dqssoc_req_mask_b;
	__uint8_t ddren2_emi_self_refresh_ch0_mask_b;
	__uint8_t ddren2_emi_self_refresh_ch1_mask_b;
	__uint8_t ddren2_mm_state_mask_b;
	__uint8_t ddren2_sspm_apsrc_req_mask_b;
	__uint8_t ddren2_dqssoc_req_mask_b;

	/* MP0_CPU0_WFI_EN */
	__uint8_t mp0_cpu0_wfi_en;

	/* MP0_CPU1_WFI_EN */
	__uint8_t mp0_cpu1_wfi_en;

	/* MP0_CPU2_WFI_EN */
	__uint8_t mp0_cpu2_wfi_en;

	/* MP0_CPU3_WFI_EN */
	__uint8_t mp0_cpu3_wfi_en;

	/* MP1_CPU0_WFI_EN */
	__uint8_t mp1_cpu0_wfi_en;

	/* MP1_CPU1_WFI_EN */
	__uint8_t mp1_cpu1_wfi_en;

	/* MP1_CPU2_WFI_EN */
	__uint8_t mp1_cpu2_wfi_en;

	/* MP1_CPU3_WFI_EN */
	__uint8_t mp1_cpu3_wfi_en;

	/* Auto-gen End */
};

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
enum pwr_ctrl_enum {
	PWR_PCM_FLAGS,
	PWR_PCM_FLAGS_CUST,
	PWR_PCM_FLAGS_CUST_SET,
	PWR_PCM_FLAGS_CUST_CLR,
	PWR_PCM_FLAGS1,
	PWR_PCM_FLAGS1_CUST,
	PWR_PCM_FLAGS1_CUST_SET,
	PWR_PCM_FLAGS1_CUST_CLR,
	PWR_TIMER_VAL,
	PWR_TIMER_VAL_CUST,
	PWR_TIMER_VAL_RAMP_EN,
	PWR_TIMER_VAL_RAMP_EN_SEC,
	PWR_WAKE_SRC,
	PWR_WAKE_SRC_CUST,
	PWR_WAKELOCK_TIMER_VAL,
	PWR_WDT_DISABLE,
	PWR_WFI_OP,
	PWR_MP0_CPUTOP_IDLE_MASK,
	PWR_MP1_CPUTOP_IDLE_MASK,
	PWR_MCUSYS_IDLE_MASK,
	PWR_MM_MASK_B,
	PWR_MD_DDR_EN_0_DBC_EN,
	PWR_MD_DDR_EN_1_DBC_EN,
	PWR_MD_MASK_B,
	PWR_SSPM_MASK_B,
	PWR_LTE_MASK_B,
	PWR_SRCCLKENI_MASK_B,
	PWR_MD_APSRC_1_SEL,
	PWR_MD_APSRC_0_SEL,
	PWR_CONN_DDR_EN_DBC_EN,
	PWR_CONN_MASK_B,
	PWR_CONN_APSRC_SEL,
	PWR_SPM_APSRC_REQ,
	PWR_SPM_F26M_REQ,
	PWR_SPM_LTE_REQ,
	PWR_SPM_INFRA_REQ,
	PWR_SPM_VRF18_REQ,
	PWR_SPM_DVFS_REQ,
	PWR_SPM_DVFS_FORCE_DOWN,
	PWR_SPM_DDREN_REQ,
	PWR_SPM_RSV_SRC_REQ,
	PWR_SPM_DDREN_2_REQ,
	PWR_CPU_MD_DVFS_SOP_FORCE_ON,
	PWR_CSYSPWREQ_MASK,
	PWR_CCIF0_MD_EVENT_MASK_B,
	PWR_CCIF0_AP_EVENT_MASK_B,
	PWR_CCIF1_MD_EVENT_MASK_B,
	PWR_CCIF1_AP_EVENT_MASK_B,
	PWR_CCIFMD_MD1_EVENT_MASK_B,
	PWR_CCIFMD_MD2_EVENT_MASK_B,
	PWR_DSI0_VSYNC_MASK_B,
	PWR_DSI1_VSYNC_MASK_B,
	PWR_DPI_VSYNC_MASK_B,
	PWR_ISP0_VSYNC_MASK_B,
	PWR_ISP1_VSYNC_MASK_B,
	PWR_MD_SRCCLKENA_0_INFRA_MASK_B,
	PWR_MD_SRCCLKENA_1_INFRA_MASK_B,
	PWR_CONN_SRCCLKENA_INFRA_MASK_B,
	PWR_SSPM_SRCCLKENA_INFRA_MASK_B,
	PWR_SRCCLKENI_INFRA_MASK_B,
	PWR_MD_APSRC_REQ_0_INFRA_MASK_B,
	PWR_MD_APSRC_REQ_1_INFRA_MASK_B,
	PWR_CONN_APSRCREQ_INFRA_MASK_B,
	PWR_SSPM_APSRCREQ_INFRA_MASK_B,
	PWR_MD_DDR_EN_0_MASK_B,
	PWR_MD_DDR_EN_1_MASK_B,
	PWR_MD_VRF18_REQ_0_MASK_B,
	PWR_MD_VRF18_REQ_1_MASK_B,
	PWR_MD1_DVFS_REQ_MASK,
	PWR_CPU_DVFS_REQ_MASK,
	PWR_EMI_BW_DVFS_REQ_MASK,
	PWR_MD_SRCCLKENA_0_DVFS_REQ_MASK_B,
	PWR_MD_SRCCLKENA_1_DVFS_REQ_MASK_B,
	PWR_CONN_SRCCLKENA_DVFS_REQ_MASK_B,
	PWR_DVFS_HALT_MASK_B,
	PWR_VDEC_REQ_MASK_B,
	PWR_GCE_REQ_MASK_B,
	PWR_CPU_MD_DVFS_REQ_MERGE_MASK_B,
	PWR_MD_DDR_EN_DVFS_HALT_MASK_B,
	PWR_DSI0_VSYNC_DVFS_HALT_MASK_B,
	PWR_DSI1_VSYNC_DVFS_HALT_MASK_B,
	PWR_DPI_VSYNC_DVFS_HALT_MASK_B,
	PWR_ISP0_VSYNC_DVFS_HALT_MASK_B,
	PWR_ISP1_VSYNC_DVFS_HALT_MASK_B,
	PWR_CONN_DDR_EN_MASK_B,
	PWR_DISP_REQ_MASK_B,
	PWR_DISP1_REQ_MASK_B,
	PWR_MFG_REQ_MASK_B,
	PWR_UFS_SRCCLKENA_MASK_B,
	PWR_UFS_VRF18_REQ_MASK_B,
	PWR_PS_C2K_RCCIF_WAKE_MASK_B,
	PWR_L1_C2K_RCCIF_WAKE_MASK_B,
	PWR_SDIO_ON_DVFS_REQ_MASK_B,
	PWR_EMI_BOOST_DVFS_REQ_MASK_B,
	PWR_CPU_MD_EMI_DVFS_REQ_PROT_DIS,
	PWR_DRAMC_SPCMD_APSRC_REQ_MASK_B,
	PWR_EMI_BOOST_DVFS_REQ_2_MASK_B,
	PWR_EMI_BW_DVFS_REQ_2_MASK,
	PWR_GCE_VRF18_REQ_MASK_B,
	PWR_SPM_WAKEUP_EVENT_MASK,
	PWR_SPM_WAKEUP_EVENT_EXT_MASK,
	PWR_MD_DDR_EN_2_0_MASK_B,
	PWR_MD_DDR_EN_2_1_MASK_B,
	PWR_CONN_DDR_EN_2_MASK_B,
	PWR_DRAMC_SPCMD_APSRC_REQ_2_MASK_B,
	PWR_SPARE1_DDREN_2_MASK_B,
	PWR_SPARE2_DDREN_2_MASK_B,
	PWR_DDREN_EMI_SELF_REFRESH_CH0_MASK_B,
	PWR_DDREN_EMI_SELF_REFRESH_CH1_MASK_B,
	PWR_DDREN_MM_STATE_MASK_B,
	PWR_DDREN_SSPM_APSRC_REQ_MASK_B,
	PWR_DDREN_DQSSOC_REQ_MASK_B,
	PWR_DDREN2_EMI_SELF_REFRESH_CH0_MASK_B,
	PWR_DDREN2_EMI_SELF_REFRESH_CH1_MASK_B,
	PWR_DDREN2_MM_STATE_MASK_B,
	PWR_DDREN2_SSPM_APSRC_REQ_MASK_B,
	PWR_DDREN2_DQSSOC_REQ_MASK_B,
	PWR_MP0_CPU0_WFI_EN,
	PWR_MP0_CPU1_WFI_EN,
	PWR_MP0_CPU2_WFI_EN,
	PWR_MP0_CPU3_WFI_EN,
	PWR_MP1_CPU0_WFI_EN,
	PWR_MP1_CPU1_WFI_EN,
	PWR_MP1_CPU2_WFI_EN,
	PWR_MP1_CPU3_WFI_EN,
	PWR_MAX_COUNT,
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
	DYNA_LOAD_PCM_SUSPEND_LP4_CH2 = 0,
	DYNA_LOAD_PCM_SUSPEND_LP4_CH1,
	DYNA_LOAD_PCM_SUSPEND_LP3_CH1,
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
	__uint32_t assert_pc;          /* PCM_REG_DATA_INI */
	__uint32_t r12;                /* PCM_REG12_DATA */
	__uint32_t r12_ext;            /* PCM_REG12_EXT_DATA */
	__uint32_t raw_sta;            /* SPM_WAKEUP_STA */
	__uint32_t raw_ext_sta;        /* SPM_WAKEUP_EXT_STA */
	__uint32_t wake_misc;          /* SPM_WAKEUP_MISC */
	__uint32_t timer_out;          /* PCM_TIMER_OUT */
	__uint32_t r13;                /* PCM_REG13_DATA */
	__uint32_t idle_sta;           /* SUBSYS_IDLE_STA */
	__uint32_t req_sta;            /* SRC_REQ_STA */
	__uint32_t debug_flag;         /* SPM_SW_DEBUG */
	__uint32_t debug_flag1;        /* WDT_LATCH_SPARE0_FIX */
	__uint32_t event_reg;          /* PCM_EVENT_REG_STA */
	__uint32_t isr;                /* SPM_IRQ_STA */
	__uint32_t log_index;
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
extern void __spm_init_event_vector(const struct pcm_desc *pcmdesc);
extern void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage);
extern void __spm_set_power_control(const struct pwr_ctrl *pwrctrl);
extern void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
extern void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl);

extern void __spm_get_wakeup_status(struct wake_status *wakesta);
extern void __spm_clean_after_wakeup(void);
extern wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta,
		const struct pcm_desc *pcmdesc);

extern void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl);

extern void __spm_set_pcm_wdt(int en);
extern __uint32_t _spm_get_wake_period(int pwake_time, wake_reason_t last_wr);


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
