#include <arch_helpers.h>
#include <arch.h>
#include <bakery_lock.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_dpidle.h>
#include <mt_spm_sodi.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <platform_def.h>
#include <plat_private.h>
#include <scu.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mtk_plat_common.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware, i.e.,
 * - spm_hotplug.c for cpu power control in cpu hotplug flow.
 * - spm_mcdi.c for cpu power control in cpu idle power saving state.
 * - spm_suspend.c for system power control in system suspend scenario.
 *
 * This file provide utility functions common to hotplug, mcdi(idle), suspend
 * power scenarios. A bakery lock (software lock) is incoporated to protect
 * certain critical sections to avoid kicking different SPM firmware
 * concurrently.
 */

DEFINE_BAKERY_LOCK(spm_lock);

void spm_lock_init(void)
{
	bakery_lock_init(&spm_lock);
}

void spm_lock_get(void)
{
	bakery_lock_get(&spm_lock);
}

void spm_lock_release(void)
{
	bakery_lock_release(&spm_lock);
}

#define SPM_FW_MAGIC    0x53504D32

static uint64_t spm_base_addr;
static uint64_t spm_base_size;
static int dyna_load_pcm_done;
static char *dyna_load_pcm_path[] = {
	[DYNA_LOAD_PCM_SUSPEND] = "pcm_suspend_dvfs.bin",
	[DYNA_LOAD_PCM_DPIDLE] = "pcm_deepidle_dvfs.bin",
	[DYNA_LOAD_PCM_SODI] = "pcm_sodi_dvfs.bin",
	[DYNA_LOAD_PCM_MAX] = "pcm_path_max",
};

struct dyna_load_pcm_t dyna_load_pcm[DYNA_LOAD_PCM_MAX];

#pragma pack(push)
#pragma pack(2)
struct spm_fw_header {
	unsigned int magic;
	unsigned int size;
	char name[58];
};
#pragma pack(pop)

char *spm_load_firmware(char *name)
{
	struct spm_fw_header *header;
	char *addr;

	addr = 0;
	header = (struct spm_fw_header *) spm_base_addr;

	while (header->magic == SPM_FW_MAGIC) {
		if (!strcmp(header->name, name))
			break;

		header = (struct spm_fw_header *) (((char *) header) + sizeof(*header) + header->size);
	}

	if (header->magic == SPM_FW_MAGIC) {
		addr = (((char *) header) + sizeof(*header));
		return addr;
	}

	return NULL;
}

int spm_load_pcm_firmware(void)
{
	int err = 0;
	int i;
	int offset = 0;
	int spm_fw_count = 0;

	if (dyna_load_pcm_done)
		return err;

	for (i = DYNA_LOAD_PCM_SUSPEND; i < DYNA_LOAD_PCM_MAX; i++) {
		uint16_t firmware_size = 0;
		int copy_size = 0;
		struct pcm_desc *pdesc = &(dyna_load_pcm[i].desc);
		char *ptr;

		ptr = spm_load_firmware(dyna_load_pcm_path[i]);
		if (ptr == NULL) {
			ERROR("Failed to load %s, err = %d.\n", dyna_load_pcm_path[i], err);
			err = -1;
			continue;
		}

		/* Do whatever it takes to load firmware into device. */
		/* start of binary size */
		offset = 0;
		copy_size = 2;
		memcpy(&firmware_size, ptr + offset, copy_size);

		/* start of binary */
		offset += copy_size;
		copy_size = firmware_size * 4;
		dyna_load_pcm[i].buf = ptr + offset;

		/* start of pcm_desc without pointer */
		offset += copy_size;
		copy_size = sizeof(struct pcm_desc) - offsetof(struct pcm_desc, size);
		memcpy((void *)&(dyna_load_pcm[i].desc.size), ptr + offset, copy_size);

		/* start of pcm_desc version */
		offset += copy_size;
		snprintf(dyna_load_pcm[i].version, PCM_FIRMWARE_VERSION_SIZE - 1,
				"%s", ptr + offset);
		pdesc->version = dyna_load_pcm[i].version;
		pdesc->base = (uint32_t *) dyna_load_pcm[i].buf;
		pdesc->base_dma = (uint64_t)dyna_load_pcm[i].buf;

		INFO("#@# %s(%d) use spmfw partition for %s - %s\n", __func__, __LINE__,
				dyna_load_pcm_path[i], pdesc->version);

		dyna_load_pcm[i].ready = 1;
		spm_fw_count++;
	}

	if (spm_fw_count == DYNA_LOAD_PCM_MAX)
		dyna_load_pcm_done = 1;

	return err;
}

uint64_t spm_load_firmware_status(void)
{
	return dyna_load_pcm_done;
}

void spm_irq0_handler(uint64_t x1)
{
	mmio_write_32(SPM_IRQ_MASK, mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, x1);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT0);
}

void spm_ap_mdsrc_req(uint64_t x1)
{
	if (x1)
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) | AP_MDSMSRC_REQ_LSB);
	else
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) & ~AP_MDSMSRC_REQ_LSB);
}

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
void save_pwr_ctrl(struct pwr_ctrl *pwrctrl, unsigned int index, unsigned int val)
{
	switch (index) {
	case PWR_PCM_FLAGS:
		pwrctrl->pcm_flags = val;
		break;
	case PWR_PCM_FLAGS_CUST:
		pwrctrl->pcm_flags_cust = val;
		break;
	case PWR_PCM_FLAGS_CUST_SET:
		pwrctrl->pcm_flags_cust_set = val;
		break;
	case PWR_PCM_FLAGS_CUST_CLR:
		pwrctrl->pcm_flags_cust_clr = val;
		break;
	case PWR_PCM_FLAGS1:
		pwrctrl->pcm_flags1 = val;
		break;
	case PWR_PCM_FLAGS1_CUST:
		pwrctrl->pcm_flags1_cust = val;
		break;
	case PWR_PCM_FLAGS1_CUST_SET:
		pwrctrl->pcm_flags1_cust_set = val;
		break;
	case PWR_PCM_FLAGS1_CUST_CLR:
		pwrctrl->pcm_flags1_cust_clr = val;
		break;
	case PWR_TIMER_VAL:
		pwrctrl->timer_val = val;
		break;
	case PWR_TIMER_VAL_CUST:
		pwrctrl->timer_val_cust = val;
		break;
	case PWR_TIMER_VAL_RAMP_EN:
		pwrctrl->timer_val_ramp_en = val;
		break;
	case PWR_TIMER_VAL_RAMP_EN_SEC:
		pwrctrl->timer_val_ramp_en_sec = val;
		break;
	case PWR_WAKE_SRC:
		pwrctrl->wake_src = val;
		break;
	case PWR_WAKE_SRC_CUST:
		pwrctrl->wake_src_cust = val;
		break;
	case PWR_WAKELOCK_TIMER_VAL:
		pwrctrl->wakelock_timer_val = val;
		break;
	case PWR_WDT_DISABLE:
		pwrctrl->wdt_disable = val;
		break;
	case PWR_WFI_OP:
		pwrctrl->wfi_op = val;
		break;
	case PWR_MP0_CPUTOP_IDLE_MASK:
		pwrctrl->mp0_cputop_idle_mask = val;
		break;
	case PWR_MCUSYS_IDLE_MASK:
		pwrctrl->mcusys_idle_mask = val;
		break;
	case PWR_MCU_DDREN_REQ_DBC_EN:
		pwrctrl->mcu_ddren_req_dbc_en = val;
		break;
	case PWR_MCU_APSRC_SEL:
		pwrctrl->mcu_apsrc_sel = val;
		break;
	case PWR_MM_MASK_B:
		pwrctrl->mm_mask_b = val;
		break;
	case PWR_MD_DDR_EN_0_DBC_EN:
		pwrctrl->md_ddr_en_0_dbc_en = val;
		break;
	case PWR_MD_DDR_EN_1_DBC_EN:
		pwrctrl->md_ddr_en_1_dbc_en = val;
		break;
	case PWR_MD_MASK_B:
		pwrctrl->md_mask_b = val;
		break;
	case PWR_LTE_MASK_B:
		pwrctrl->lte_mask_b = val;
		break;
	case PWR_SRCCLKENI_MASK_B:
		pwrctrl->srcclkeni_mask_b = val;
		break;
	case PWR_MD_APSRC_1_SEL:
		pwrctrl->md_apsrc_1_sel = val;
		break;
	case PWR_MD_APSRC_0_SEL:
		pwrctrl->md_apsrc_0_sel = val;
		break;
	case PWR_CONN_DDR_EN_DBC_EN:
		pwrctrl->conn_ddr_en_dbc_en = val;
		break;
	case PWR_CONN_MASK_B:
		pwrctrl->conn_mask_b = val;
		break;
	case PWR_CONN_APSRC_SEL:
		pwrctrl->conn_apsrc_sel = val;
		break;
	case PWR_CONN_SRCCLKENA_SEL_MASK:
		pwrctrl->conn_srcclkena_sel_mask = val;
		break;
	case PWR_SPM_APSRC_REQ:
		pwrctrl->spm_apsrc_req = val;
		break;
	case PWR_SPM_F26M_REQ:
		pwrctrl->spm_f26m_req = val;
		break;
	case PWR_SPM_LTE_REQ:
		pwrctrl->spm_lte_req = val;
		break;
	case PWR_SPM_INFRA_REQ:
		pwrctrl->spm_infra_req = val;
		break;
	case PWR_SPM_VRF18_REQ:
		pwrctrl->spm_vrf18_req = val;
		break;
	case PWR_SPM_DVFS_REQ:
		pwrctrl->spm_dvfs_req = val;
		break;
	case PWR_SPM_DVFS_FORCE_DOWN:
		pwrctrl->spm_dvfs_force_down = val;
		break;
	case PWR_SPM_DDREN_REQ:
		pwrctrl->spm_ddren_req = val;
		break;
	case PWR_SPM_RSV_SRC_REQ:
		pwrctrl->spm_rsv_src_req = val;
		break;
	case PWR_SPM_DDREN_2_REQ:
		pwrctrl->spm_ddren_2_req = val;
		break;
	case PWR_CPU_MD_DVFS_SOP_FORCE_ON:
		pwrctrl->cpu_md_dvfs_sop_force_on = val;
		break;
	case PWR_CSYSPWREQ_MASK:
		pwrctrl->csyspwreq_mask = val;
		break;
	case PWR_CCIF0_MD_EVENT_MASK_B:
		pwrctrl->ccif0_md_event_mask_b = val;
		break;
	case PWR_CCIF0_AP_EVENT_MASK_B:
		pwrctrl->ccif0_ap_event_mask_b = val;
		break;
	case PWR_CCIF1_MD_EVENT_MASK_B:
		pwrctrl->ccif1_md_event_mask_b = val;
		break;
	case PWR_CCIF1_AP_EVENT_MASK_B:
		pwrctrl->ccif1_ap_event_mask_b = val;
		break;
	case PWR_CCIFMD_MD1_EVENT_MASK_B:
		pwrctrl->ccifmd_md1_event_mask_b = val;
		break;
	case PWR_CCIFMD_MD2_EVENT_MASK_B:
		pwrctrl->ccifmd_md2_event_mask_b = val;
		break;
	case PWR_DSI0_VSYNC_MASK_B:
		pwrctrl->dsi0_vsync_mask_b = val;
		break;
	case PWR_DSI1_VSYNC_MASK_B:
		pwrctrl->dsi1_vsync_mask_b = val;
		break;
	case PWR_DPI_VSYNC_MASK_B:
		pwrctrl->dpi_vsync_mask_b = val;
		break;
	case PWR_ISP0_VSYNC_MASK_B:
		pwrctrl->isp0_vsync_mask_b = val;
		break;
	case PWR_ISP1_VSYNC_MASK_B:
		pwrctrl->isp1_vsync_mask_b = val;
		break;
	case PWR_MD_SRCCLKENA_0_INFRA_MASK_B:
		pwrctrl->md_srcclkena_0_infra_mask_b = val;
		break;
	case PWR_MD_SRCCLKENA_1_INFRA_MASK_B:
		pwrctrl->md_srcclkena_1_infra_mask_b = val;
		break;
	case PWR_CONN_SRCCLKENA_INFRA_MASK_B:
		pwrctrl->conn_srcclkena_infra_mask_b = val;
		break;
	case PWR_SSPM_SRCCLKENA_INFRA_MASK_B:
		pwrctrl->sspm_srcclkena_infra_mask_b = val;
		break;
	case PWR_SRCCLKENI_INFRA_MASK_B:
		pwrctrl->srcclkeni_infra_mask_b = val;
		break;
	case PWR_MD_APSRC_REQ_0_INFRA_MASK_B:
		pwrctrl->md_apsrc_req_0_infra_mask_b = val;
		break;
	case PWR_MD_APSRC_REQ_1_INFRA_MASK_B:
		pwrctrl->md_apsrc_req_1_infra_mask_b = val;
		break;
	case PWR_CONN_APSRCREQ_INFRA_MASK_B:
		pwrctrl->conn_apsrcreq_infra_mask_b = val;
		break;
	case PWR_MCU_APSRCREQ_INFRA_MASK_B:
		pwrctrl->mcu_apsrcreq_infra_mask_b = val;
		break;
	case PWR_MD_DDR_EN_0_MASK_B:
		pwrctrl->md_ddr_en_0_mask_b = val;
		break;
	case PWR_MD_DDR_EN_1_MASK_B:
		pwrctrl->md_ddr_en_1_mask_b = val;
		break;
	case PWR_MD_VRF18_REQ_0_MASK_B:
		pwrctrl->md_vrf18_req_0_mask_b = val;
		break;
	case PWR_MD_VRF18_REQ_1_MASK_B:
		pwrctrl->md_vrf18_req_1_mask_b = val;
		break;
	case PWR_MD1_DVFS_REQ_MASK:
		pwrctrl->md1_dvfs_req_mask = val;
		break;
	case PWR_CPU_DVFS_REQ_MASK:
		pwrctrl->cpu_dvfs_req_mask = val;
		break;
	case PWR_EMI_BW_DVFS_REQ_MASK:
		pwrctrl->emi_bw_dvfs_req_mask = val;
		break;
	case PWR_MD_SRCCLKENA_0_DVFS_REQ_MASK_B:
		pwrctrl->md_srcclkena_0_dvfs_req_mask_b = val;
		break;
	case PWR_MD_SRCCLKENA_1_DVFS_REQ_MASK_B:
		pwrctrl->md_srcclkena_1_dvfs_req_mask_b = val;
		break;
	case PWR_CONN_SRCCLKENA_DVFS_REQ_MASK_B:
		pwrctrl->conn_srcclkena_dvfs_req_mask_b = val;
		break;
	case PWR_DVFS_HALT_MASK_B:
		pwrctrl->dvfs_halt_mask_b = val;
		break;
	case PWR_VDEC_REQ_MASK_B:
		pwrctrl->vdec_req_mask_b = val;
		break;
	case PWR_GCE_REQ_MASK_B:
		pwrctrl->gce_req_mask_b = val;
		break;
	case PWR_CPU_MD_DVFS_REQ_MERGE_MASK_B:
		pwrctrl->cpu_md_dvfs_req_merge_mask_b = val;
		break;
	case PWR_MD_DDR_EN_DVFS_HALT_MASK_B:
		pwrctrl->md_ddr_en_dvfs_halt_mask_b = val;
		break;
	case PWR_DSI0_VSYNC_DVFS_HALT_MASK_B:
		pwrctrl->dsi0_vsync_dvfs_halt_mask_b = val;
		break;
	case PWR_DSI1_VSYNC_DVFS_HALT_MASK_B:
		pwrctrl->dsi1_vsync_dvfs_halt_mask_b = val;
		break;
	case PWR_DPI_VSYNC_DVFS_HALT_MASK_B:
		pwrctrl->dpi_vsync_dvfs_halt_mask_b = val;
		break;
	case PWR_ISP0_VSYNC_DVFS_HALT_MASK_B:
		pwrctrl->isp0_vsync_dvfs_halt_mask_b = val;
		break;
	case PWR_ISP1_VSYNC_DVFS_HALT_MASK_B:
		pwrctrl->isp1_vsync_dvfs_halt_mask_b = val;
		break;
	case PWR_CONN_DDR_EN_MASK_B:
		pwrctrl->conn_ddr_en_mask_b = val;
		break;
	case PWR_DISP_REQ_MASK_B:
		pwrctrl->disp_req_mask_b = val;
		break;
	case PWR_DISP1_REQ_MASK_B:
		pwrctrl->disp1_req_mask_b = val;
		break;
	case PWR_MFG_REQ_MASK_B:
		pwrctrl->mfg_req_mask_b = val;
		break;
	case PWR_MCU_DDREN_REQ_MASK_B:
		pwrctrl->mcu_ddren_req_mask_b = val;
		break;
	case PWR_MCU_APSRC_REQ_MASK_B:
		pwrctrl->mcu_apsrc_req_mask_b = val;
		break;
	case PWR_PS_C2K_RCCIF_WAKE_MASK_B:
		pwrctrl->ps_c2k_rccif_wake_mask_b = val;
		break;
	case PWR_L1_C2K_RCCIF_WAKE_MASK_B:
		pwrctrl->l1_c2k_rccif_wake_mask_b = val;
		break;
	case PWR_SDIO_ON_DVFS_REQ_MASK_B:
		pwrctrl->sdio_on_dvfs_req_mask_b = val;
		break;
	case PWR_EMI_BOOST_DVFS_REQ_MASK_B:
		pwrctrl->emi_boost_dvfs_req_mask_b = val;
		break;
	case PWR_CPU_MD_EMI_DVFS_REQ_PROT_DIS:
		pwrctrl->cpu_md_emi_dvfs_req_prot_dis = val;
		break;
	case PWR_DRAMC_SPCMD_APSRC_REQ_MASK_B:
		pwrctrl->dramc_spcmd_apsrc_req_mask_b = val;
		break;
	case PWR_EMI_BOOST_DVFS_REQ_2_MASK_B:
		pwrctrl->emi_boost_dvfs_req_2_mask_b = val;
		break;
	case PWR_EMI_BW_DVFS_REQ_2_MASK:
		pwrctrl->emi_bw_dvfs_req_2_mask = val;
		break;
	case PWR_GCE_VRF18_REQ_MASK_B:
		pwrctrl->gce_vrf18_req_mask_b = val;
		break;
	case PWR_SPM_WAKEUP_EVENT_MASK:
		pwrctrl->spm_wakeup_event_mask = val;
		break;
	case PWR_SPM_WAKEUP_EVENT_EXT_MASK:
		pwrctrl->spm_wakeup_event_ext_mask = val;
		break;
	case PWR_MD_DDR_EN_2_0_MASK_B:
		pwrctrl->md_ddr_en_2_0_mask_b = val;
		break;
	case PWR_MD_DDR_EN_2_1_MASK_B:
		pwrctrl->md_ddr_en_2_1_mask_b = val;
		break;
	case PWR_CONN_DDR_EN_2_MASK_B:
		pwrctrl->conn_ddr_en_2_mask_b = val;
		break;
	case PWR_DRAMC_SPCMD_APSRC_REQ_2_MASK_B:
		pwrctrl->dramc_spcmd_apsrc_req_2_mask_b = val;
		break;
	case PWR_SPARE1_DDREN_2_MASK_B:
		pwrctrl->spare1_ddren_2_mask_b = val;
		break;
	case PWR_SPARE2_DDREN_2_MASK_B:
		pwrctrl->spare2_ddren_2_mask_b = val;
		break;
	case PWR_DDREN_EMI_SELF_REFRESH_CH0_MASK_B:
		pwrctrl->ddren_emi_self_refresh_ch0_mask_b = val;
		break;
	case PWR_DDREN_EMI_SELF_REFRESH_CH1_MASK_B:
		pwrctrl->ddren_emi_self_refresh_ch1_mask_b = val;
		break;
	case PWR_DDREN_MM_STATE_MASK_B:
		pwrctrl->ddren_mm_state_mask_b = val;
		break;
	case PWR_DDREN_SSPM_APSRC_REQ_MASK_B:
		pwrctrl->ddren_sspm_apsrc_req_mask_b = val;
		break;
	case PWR_DDREN_DQSSOC_REQ_MASK_B:
		pwrctrl->ddren_dqssoc_req_mask_b = val;
		break;
	case PWR_DDREN2_EMI_SELF_REFRESH_CH0_MASK_B:
		pwrctrl->ddren2_emi_self_refresh_ch0_mask_b = val;
		break;
	case PWR_DDREN2_EMI_SELF_REFRESH_CH1_MASK_B:
		pwrctrl->ddren2_emi_self_refresh_ch1_mask_b = val;
		break;
	case PWR_DDREN2_MM_STATE_MASK_B:
		pwrctrl->ddren2_mm_state_mask_b = val;
		break;
	case PWR_DDREN2_SSPM_APSRC_REQ_MASK_B:
		pwrctrl->ddren2_sspm_apsrc_req_mask_b = val;
		break;
	case PWR_DDREN2_DQSSOC_REQ_MASK_B:
		pwrctrl->ddren2_dqssoc_req_mask_b = val;
		break;
	case PWR_MP0_CPU0_WFI_EN:
		pwrctrl->mp0_cpu0_wfi_en = val;
		break;
	case PWR_MP0_CPU1_WFI_EN:
		pwrctrl->mp0_cpu1_wfi_en = val;
		break;
	case PWR_MP0_CPU2_WFI_EN:
		pwrctrl->mp0_cpu2_wfi_en = val;
		break;
	case PWR_MP0_CPU3_WFI_EN:
		pwrctrl->mp0_cpu3_wfi_en = val;
		break;
	default:
		ERROR("%s: %d is undefined!!!\n", __func__, index);
	}
}

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
unsigned int load_pwr_ctrl(struct pwr_ctrl *pwrctrl, unsigned int index)
{
	unsigned int val = 0;

	switch (index) {
	case PWR_PCM_FLAGS:
		val = pwrctrl->pcm_flags;
		break;
	case PWR_PCM_FLAGS_CUST:
		val = pwrctrl->pcm_flags_cust;
		break;
	case PWR_PCM_FLAGS_CUST_SET:
		val = pwrctrl->pcm_flags_cust_set;
		break;
	case PWR_PCM_FLAGS_CUST_CLR:
		val = pwrctrl->pcm_flags_cust_clr;
		break;
	case PWR_PCM_FLAGS1:
		val = pwrctrl->pcm_flags1;
		break;
	case PWR_PCM_FLAGS1_CUST:
		val = pwrctrl->pcm_flags1_cust;
		break;
	case PWR_PCM_FLAGS1_CUST_SET:
		val = pwrctrl->pcm_flags1_cust_set;
		break;
	case PWR_PCM_FLAGS1_CUST_CLR:
		val = pwrctrl->pcm_flags1_cust_clr;
		break;
	case PWR_TIMER_VAL:
		val = pwrctrl->timer_val;
		break;
	case PWR_TIMER_VAL_CUST:
		val = pwrctrl->timer_val_cust;
		break;
	case PWR_TIMER_VAL_RAMP_EN:
		val = pwrctrl->timer_val_ramp_en;
		break;
	case PWR_TIMER_VAL_RAMP_EN_SEC:
		val = pwrctrl->timer_val_ramp_en_sec;
		break;
	case PWR_WAKE_SRC:
		val = pwrctrl->wake_src;
		break;
	case PWR_WAKE_SRC_CUST:
		val = pwrctrl->wake_src_cust;
		break;
	case PWR_WAKELOCK_TIMER_VAL:
		val = pwrctrl->wakelock_timer_val;
		break;
	case PWR_WDT_DISABLE:
		val = pwrctrl->wdt_disable;
		break;
	case PWR_WFI_OP:
		val = pwrctrl->wfi_op;
		break;
	case PWR_MP0_CPUTOP_IDLE_MASK:
		val = pwrctrl->mp0_cputop_idle_mask;
		break;
	case PWR_MCUSYS_IDLE_MASK:
		val = pwrctrl->mcusys_idle_mask;
		break;
	case PWR_MCU_DDREN_REQ_DBC_EN:
		val = pwrctrl->mcu_ddren_req_dbc_en;
		break;
	case PWR_MCU_APSRC_SEL:
		val = pwrctrl->mcu_apsrc_sel;
		break;
	case PWR_MM_MASK_B:
		val = pwrctrl->mm_mask_b;
		break;
	case PWR_MD_DDR_EN_0_DBC_EN:
		val = pwrctrl->md_ddr_en_0_dbc_en;
		break;
	case PWR_MD_DDR_EN_1_DBC_EN:
		val = pwrctrl->md_ddr_en_1_dbc_en;
		break;
	case PWR_MD_MASK_B:
		val = pwrctrl->md_mask_b;
		break;
	case PWR_LTE_MASK_B:
		val = pwrctrl->lte_mask_b;
		break;
	case PWR_SRCCLKENI_MASK_B:
		val = pwrctrl->srcclkeni_mask_b;
		break;
	case PWR_MD_APSRC_1_SEL:
		val = pwrctrl->md_apsrc_1_sel;
		break;
	case PWR_MD_APSRC_0_SEL:
		val = pwrctrl->md_apsrc_0_sel;
		break;
	case PWR_CONN_DDR_EN_DBC_EN:
		val = pwrctrl->conn_ddr_en_dbc_en;
		break;
	case PWR_CONN_MASK_B:
		val = pwrctrl->conn_mask_b;
		break;
	case PWR_CONN_APSRC_SEL:
		val = pwrctrl->conn_apsrc_sel;
		break;
	case PWR_CONN_SRCCLKENA_SEL_MASK:
		val = pwrctrl->conn_srcclkena_sel_mask;
		break;
	case PWR_SPM_APSRC_REQ:
		val = pwrctrl->spm_apsrc_req;
		break;
	case PWR_SPM_F26M_REQ:
		val = pwrctrl->spm_f26m_req;
		break;
	case PWR_SPM_LTE_REQ:
		val = pwrctrl->spm_lte_req;
		break;
	case PWR_SPM_INFRA_REQ:
		val = pwrctrl->spm_infra_req;
		break;
	case PWR_SPM_VRF18_REQ:
		val = pwrctrl->spm_vrf18_req;
		break;
	case PWR_SPM_DVFS_REQ:
		val = pwrctrl->spm_dvfs_req;
		break;
	case PWR_SPM_DVFS_FORCE_DOWN:
		val = pwrctrl->spm_dvfs_force_down;
		break;
	case PWR_SPM_DDREN_REQ:
		val = pwrctrl->spm_ddren_req;
		break;
	case PWR_SPM_RSV_SRC_REQ:
		val = pwrctrl->spm_rsv_src_req;
		break;
	case PWR_SPM_DDREN_2_REQ:
		val = pwrctrl->spm_ddren_2_req;
		break;
	case PWR_CPU_MD_DVFS_SOP_FORCE_ON:
		val = pwrctrl->cpu_md_dvfs_sop_force_on;
		break;
	case PWR_CSYSPWREQ_MASK:
		val = pwrctrl->csyspwreq_mask;
		break;
	case PWR_CCIF0_MD_EVENT_MASK_B:
		val = pwrctrl->ccif0_md_event_mask_b;
		break;
	case PWR_CCIF0_AP_EVENT_MASK_B:
		val = pwrctrl->ccif0_ap_event_mask_b;
		break;
	case PWR_CCIF1_MD_EVENT_MASK_B:
		val = pwrctrl->ccif1_md_event_mask_b;
		break;
	case PWR_CCIF1_AP_EVENT_MASK_B:
		val = pwrctrl->ccif1_ap_event_mask_b;
		break;
	case PWR_CCIFMD_MD1_EVENT_MASK_B:
		val = pwrctrl->ccifmd_md1_event_mask_b;
		break;
	case PWR_CCIFMD_MD2_EVENT_MASK_B:
		val = pwrctrl->ccifmd_md2_event_mask_b;
		break;
	case PWR_DSI0_VSYNC_MASK_B:
		val = pwrctrl->dsi0_vsync_mask_b;
		break;
	case PWR_DSI1_VSYNC_MASK_B:
		val = pwrctrl->dsi1_vsync_mask_b;
		break;
	case PWR_DPI_VSYNC_MASK_B:
		val = pwrctrl->dpi_vsync_mask_b;
		break;
	case PWR_ISP0_VSYNC_MASK_B:
		val = pwrctrl->isp0_vsync_mask_b;
		break;
	case PWR_ISP1_VSYNC_MASK_B:
		val = pwrctrl->isp1_vsync_mask_b;
		break;
	case PWR_MD_SRCCLKENA_0_INFRA_MASK_B:
		val = pwrctrl->md_srcclkena_0_infra_mask_b;
		break;
	case PWR_MD_SRCCLKENA_1_INFRA_MASK_B:
		val = pwrctrl->md_srcclkena_1_infra_mask_b;
		break;
	case PWR_CONN_SRCCLKENA_INFRA_MASK_B:
		val = pwrctrl->conn_srcclkena_infra_mask_b;
		break;
	case PWR_SSPM_SRCCLKENA_INFRA_MASK_B:
		val = pwrctrl->sspm_srcclkena_infra_mask_b;
		break;
	case PWR_SRCCLKENI_INFRA_MASK_B:
		val = pwrctrl->srcclkeni_infra_mask_b;
		break;
	case PWR_MD_APSRC_REQ_0_INFRA_MASK_B:
		val = pwrctrl->md_apsrc_req_0_infra_mask_b;
		break;
	case PWR_MD_APSRC_REQ_1_INFRA_MASK_B:
		val = pwrctrl->md_apsrc_req_1_infra_mask_b;
		break;
	case PWR_CONN_APSRCREQ_INFRA_MASK_B:
		val = pwrctrl->conn_apsrcreq_infra_mask_b;
		break;
	case PWR_MCU_APSRCREQ_INFRA_MASK_B:
		val = pwrctrl->mcu_apsrcreq_infra_mask_b;
		break;
	case PWR_MD_DDR_EN_0_MASK_B:
		val = pwrctrl->md_ddr_en_0_mask_b;
		break;
	case PWR_MD_DDR_EN_1_MASK_B:
		val = pwrctrl->md_ddr_en_1_mask_b;
		break;
	case PWR_MD_VRF18_REQ_0_MASK_B:
		val = pwrctrl->md_vrf18_req_0_mask_b;
		break;
	case PWR_MD_VRF18_REQ_1_MASK_B:
		val = pwrctrl->md_vrf18_req_1_mask_b;
		break;
	case PWR_MD1_DVFS_REQ_MASK:
		val = pwrctrl->md1_dvfs_req_mask;
		break;
	case PWR_CPU_DVFS_REQ_MASK:
		val = pwrctrl->cpu_dvfs_req_mask;
		break;
	case PWR_EMI_BW_DVFS_REQ_MASK:
		val = pwrctrl->emi_bw_dvfs_req_mask;
		break;
	case PWR_MD_SRCCLKENA_0_DVFS_REQ_MASK_B:
		val = pwrctrl->md_srcclkena_0_dvfs_req_mask_b;
		break;
	case PWR_MD_SRCCLKENA_1_DVFS_REQ_MASK_B:
		val = pwrctrl->md_srcclkena_1_dvfs_req_mask_b;
		break;
	case PWR_CONN_SRCCLKENA_DVFS_REQ_MASK_B:
		val = pwrctrl->conn_srcclkena_dvfs_req_mask_b;
		break;
	case PWR_DVFS_HALT_MASK_B:
		val = pwrctrl->dvfs_halt_mask_b;
		break;
	case PWR_VDEC_REQ_MASK_B:
		val = pwrctrl->vdec_req_mask_b;
		break;
	case PWR_GCE_REQ_MASK_B:
		val = pwrctrl->gce_req_mask_b;
		break;
	case PWR_CPU_MD_DVFS_REQ_MERGE_MASK_B:
		val = pwrctrl->cpu_md_dvfs_req_merge_mask_b;
		break;
	case PWR_MD_DDR_EN_DVFS_HALT_MASK_B:
		val = pwrctrl->md_ddr_en_dvfs_halt_mask_b;
		break;
	case PWR_DSI0_VSYNC_DVFS_HALT_MASK_B:
		val = pwrctrl->dsi0_vsync_dvfs_halt_mask_b;
		break;
	case PWR_DSI1_VSYNC_DVFS_HALT_MASK_B:
		val = pwrctrl->dsi1_vsync_dvfs_halt_mask_b;
		break;
	case PWR_DPI_VSYNC_DVFS_HALT_MASK_B:
		val = pwrctrl->dpi_vsync_dvfs_halt_mask_b;
		break;
	case PWR_ISP0_VSYNC_DVFS_HALT_MASK_B:
		val = pwrctrl->isp0_vsync_dvfs_halt_mask_b;
		break;
	case PWR_ISP1_VSYNC_DVFS_HALT_MASK_B:
		val = pwrctrl->isp1_vsync_dvfs_halt_mask_b;
		break;
	case PWR_CONN_DDR_EN_MASK_B:
		val = pwrctrl->conn_ddr_en_mask_b;
		break;
	case PWR_DISP_REQ_MASK_B:
		val = pwrctrl->disp_req_mask_b;
		break;
	case PWR_DISP1_REQ_MASK_B:
		val = pwrctrl->disp1_req_mask_b;
		break;
	case PWR_MFG_REQ_MASK_B:
		val = pwrctrl->mfg_req_mask_b;
		break;
	case PWR_MCU_DDREN_REQ_MASK_B:
		val = pwrctrl->mcu_ddren_req_mask_b;
		break;
	case PWR_MCU_APSRC_REQ_MASK_B:
		val = pwrctrl->mcu_apsrc_req_mask_b;
		break;
	case PWR_PS_C2K_RCCIF_WAKE_MASK_B:
		val = pwrctrl->ps_c2k_rccif_wake_mask_b;
		break;
	case PWR_L1_C2K_RCCIF_WAKE_MASK_B:
		val = pwrctrl->l1_c2k_rccif_wake_mask_b;
		break;
	case PWR_SDIO_ON_DVFS_REQ_MASK_B:
		val = pwrctrl->sdio_on_dvfs_req_mask_b;
		break;
	case PWR_EMI_BOOST_DVFS_REQ_MASK_B:
		val = pwrctrl->emi_boost_dvfs_req_mask_b;
		break;
	case PWR_CPU_MD_EMI_DVFS_REQ_PROT_DIS:
		val = pwrctrl->cpu_md_emi_dvfs_req_prot_dis;
		break;
	case PWR_DRAMC_SPCMD_APSRC_REQ_MASK_B:
		val = pwrctrl->dramc_spcmd_apsrc_req_mask_b;
		break;
	case PWR_EMI_BOOST_DVFS_REQ_2_MASK_B:
		val = pwrctrl->emi_boost_dvfs_req_2_mask_b;
		break;
	case PWR_EMI_BW_DVFS_REQ_2_MASK:
		val = pwrctrl->emi_bw_dvfs_req_2_mask;
		break;
	case PWR_GCE_VRF18_REQ_MASK_B:
		val = pwrctrl->gce_vrf18_req_mask_b;
		break;
	case PWR_SPM_WAKEUP_EVENT_MASK:
		val = pwrctrl->spm_wakeup_event_mask;
		break;
	case PWR_SPM_WAKEUP_EVENT_EXT_MASK:
		val = pwrctrl->spm_wakeup_event_ext_mask;
		break;
	case PWR_MD_DDR_EN_2_0_MASK_B:
		val = pwrctrl->md_ddr_en_2_0_mask_b;
		break;
	case PWR_MD_DDR_EN_2_1_MASK_B:
		val = pwrctrl->md_ddr_en_2_1_mask_b;
		break;
	case PWR_CONN_DDR_EN_2_MASK_B:
		val = pwrctrl->conn_ddr_en_2_mask_b;
		break;
	case PWR_DRAMC_SPCMD_APSRC_REQ_2_MASK_B:
		val = pwrctrl->dramc_spcmd_apsrc_req_2_mask_b;
		break;
	case PWR_SPARE1_DDREN_2_MASK_B:
		val = pwrctrl->spare1_ddren_2_mask_b;
		break;
	case PWR_SPARE2_DDREN_2_MASK_B:
		val = pwrctrl->spare2_ddren_2_mask_b;
		break;
	case PWR_DDREN_EMI_SELF_REFRESH_CH0_MASK_B:
		val = pwrctrl->ddren_emi_self_refresh_ch0_mask_b;
		break;
	case PWR_DDREN_EMI_SELF_REFRESH_CH1_MASK_B:
		val = pwrctrl->ddren_emi_self_refresh_ch1_mask_b;
		break;
	case PWR_DDREN_MM_STATE_MASK_B:
		val = pwrctrl->ddren_mm_state_mask_b;
		break;
	case PWR_DDREN_SSPM_APSRC_REQ_MASK_B:
		val = pwrctrl->ddren_sspm_apsrc_req_mask_b;
		break;
	case PWR_DDREN_DQSSOC_REQ_MASK_B:
		val = pwrctrl->ddren_dqssoc_req_mask_b;
		break;
	case PWR_DDREN2_EMI_SELF_REFRESH_CH0_MASK_B:
		val = pwrctrl->ddren2_emi_self_refresh_ch0_mask_b;
		break;
	case PWR_DDREN2_EMI_SELF_REFRESH_CH1_MASK_B:
		val = pwrctrl->ddren2_emi_self_refresh_ch1_mask_b;
		break;
	case PWR_DDREN2_MM_STATE_MASK_B:
		val = pwrctrl->ddren2_mm_state_mask_b;
		break;
	case PWR_DDREN2_SSPM_APSRC_REQ_MASK_B:
		val = pwrctrl->ddren2_sspm_apsrc_req_mask_b;
		break;
	case PWR_DDREN2_DQSSOC_REQ_MASK_B:
		val = pwrctrl->ddren2_dqssoc_req_mask_b;
		break;
	case PWR_MP0_CPU0_WFI_EN:
		val = pwrctrl->mp0_cpu0_wfi_en;
		break;
	case PWR_MP0_CPU1_WFI_EN:
		val = pwrctrl->mp0_cpu1_wfi_en;
		break;
	case PWR_MP0_CPU2_WFI_EN:
		val = pwrctrl->mp0_cpu2_wfi_en;
		break;
	case PWR_MP0_CPU3_WFI_EN:
		val = pwrctrl->mp0_cpu3_wfi_en;
		break;
	default:
		ERROR("%s: %d is undefined!!!\n", __func__, index);
	}

	return val;
}

void spm_pwr_ctrl_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	switch (x1) {
	case SPM_PWR_CTRL_SUSPEND:
		save_pwr_ctrl(__spm_suspend.pwrctrl, x2, x3);
		break;
	case SPM_PWR_CTRL_DPIDLE:
		save_pwr_ctrl(__spm_dpidle.pwrctrl, x2, x3);
		break;
	case SPM_PWR_CTRL_SODI:
		save_pwr_ctrl(__spm_sodi.pwrctrl, x2, x3);
		break;
	case SPM_PWR_CTRL_SODI3:
		save_pwr_ctrl(__spm_sodi.pwrctrl, x2, x3);
		break;
	case SPM_PWR_CTRL_VCOREFS:
		/* save_pwr_ctrl(__spm_vcorefs.pwrctrl, x2, x3); */
		break;
	}
}

uint64_t spm_get_pwr_ctrl_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t ret = 0;

	switch (x1) {
	case SPM_PWR_CTRL_SUSPEND:
		ret = load_pwr_ctrl(__spm_suspend.pwrctrl, x2);
		break;
	case SPM_PWR_CTRL_DPIDLE:
		ret = load_pwr_ctrl(__spm_dpidle.pwrctrl, x2);
		break;
	case SPM_PWR_CTRL_SODI:
		ret = load_pwr_ctrl(__spm_sodi.pwrctrl, x2);
		break;
	case SPM_PWR_CTRL_SODI3:
		ret = load_pwr_ctrl(__spm_sodi.pwrctrl, x2);
		break;
	case SPM_PWR_CTRL_VCOREFS:
		/* ret = load_pwr_ctrl(__spm_vcorefs.pwrctrl, x2); */
		break;
	}

	return ret;
}

void spm_dcs_s1_setting(uint64_t x1, uint64_t x2)
{
	/* FIXME: */
#if 0
	uint32_t enable = x1;
	uint32_t flags = x2;

	mmio_write_32(DRAMC_DPY_CLK_SW_CON5, mmio_read_32(DRAMC_DPY_CLK_SW_CON5) | flags);
	while ((mmio_read_32(DRAMC_DPY_CLK_SW_CON5) & (0xf << 4)) != (flags << 4))
		;

	if (!!enable)
		mmio_write_32(SPM_SPARE_CON_SET, flags << 24);
	else
		mmio_write_32(SPM_SPARE_CON_CLR, flags << 24);

	mmio_write_32(DRAMC_DPY_CLK_SW_CON5, mmio_read_32(DRAMC_DPY_CLK_SW_CON5) & ~flags);
	while ((mmio_read_32(DRAMC_DPY_CLK_SW_CON5) & (0xf << 4)) != 0)
		;
#endif
}

void spm_dummy_read(uint64_t x1, uint64_t x2)
{
	uint32_t rank0_addr = x1;
	uint32_t rank1_addr = x2;

	mmio_write_32(SPM_PASR_DPD_1, rank0_addr & 0xffffffff);
	mmio_write_32(SPM_PASR_DPD_2, rank1_addr & 0xffffffff);

	/* FIXME: */
#if 0
	if ((x1 >> 32) & 0x1)
		mmio_write_32(SPM_SW_RSV_5, mmio_read_32(SPM_SW_RSV_5) | (1 << 3));
	else
		mmio_write_32(SPM_SW_RSV_5, mmio_read_32(SPM_SW_RSV_5) & ~(1 << 3));
	if ((x2 >> 32) & 0x1)
		mmio_write_32(SPM_SW_RSV_5, mmio_read_32(SPM_SW_RSV_5) | (1 << 4));
	else
		mmio_write_32(SPM_SW_RSV_5, mmio_read_32(SPM_SW_RSV_5) & ~(1 << 4));
#endif
}

void spm_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t cmd = x1;

	switch (cmd) {
	case SPM_ARGS_SPMFW_IDX:
		break;
	case SPM_ARGS_SPMFW_INIT:
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
		spm_config_spmfw_base(x2, x3);
		spm_load_pcm_firmware();
		console_uninit();
		break;
	case SPM_ARGS_SUSPEND:
		spm_suspend();
		break;
	case SPM_ARGS_SUSPEND_FINISH:
		spm_suspend_finish();
		break;
	case SPM_ARGS_SODI:
		spm_sodi();
		break;
	case SPM_ARGS_SODI_FINISH:
		spm_sodi_finish();
		break;
	case SPM_ARGS_DPIDLE:
		spm_dpidle();
		break;
	case SPM_ARGS_DPIDLE_FINISH:
		spm_dpidle_finish();
		break;
	case SPM_ARGS_PCM_WDT:
		spm_pcm_wdt(x2, x3);
		break;
	default:
		break;
	}
}

void spm_register_init(void)
{
	/* enable register control */
	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB | MD_BCLK_CG_EN_LSB);

	/* init power control register */
	/* dram will set this register */
	/* mmio_write_32(SPM_POWER_ON_VAL0, 0); */
	mmio_write_32(SPM_POWER_ON_VAL1, POWER_ON_VAL1_DEF | (0x1 << 31));
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	if ((mmio_read_32(PCM_FSM_STA) & 0x7fffff) != PCM_FSM_STA_DEF)
		ERROR("PCM reset failed\n");

	/* init PCM control register */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | EN_IM_SLEEP_DVS_LSB);
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | EVENT_LOCK_EN_LSB |
			SPM_SRAM_ISOINT_B_LSB | MIF_APBEN_LSB | SCP_APB_INTERNAL_EN_LSB);
	mmio_write_32(PCM_IM_PTR, 0);
	mmio_write_32(PCM_IM_LEN, 0);

	mmio_write_32(SPM_CLK_CON, mmio_read_32(SPM_CLK_CON) | (0x3 << 2) |
			SPM_LOCK_INFRA_DCM_LSB | (1 << 6) |
			CXO32K_REMOVE_EN_MD1_LSB |
			CLKSQ1_SEL_CTRL_LSB | SRCLKEN0_EN_LSB |
			(0x10 << 23));

	/* clean wakeup event raw status */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, SPM_WAKEUP_EVENT_MASK_DEF);

	/* clean ISR status */
	mmio_write_32(SPM_IRQ_MASK, ISRM_ALL);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);

	/* init r7 with POWER_ON_VAL1 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(PCM_PWR_IO_EN, 0);

	mmio_write_32(DDR_EN_DBC_LEN, 0x82 | (0x82 << 10) | (0x82 << 20));
	mmio_write_32(SPM_PC_TRACE_CON, 0x8 | SPM_PC_TRACE_HW_EN_LSB);
}

void spm_config_spmfw_base(uint64_t addr, uint64_t size)
{
	spm_base_addr = addr;
	spm_base_size = size;

	INFO("#@# %s(%d) spm_base_addr 0x%lx, spm_base_size 0x%lx\n", __func__, __LINE__,
			spm_base_addr, spm_base_size);

	mmap_add_region((addr & ~(PAGE_SIZE_MASK)),
			(addr & ~(PAGE_SIZE_MASK)),
			ALIGN(size, (PAGE_SIZE_MASK)),
			MT_MEMORY | MT_RW | MT_SECURE);

	/* set up translation tables */
	init_xlat_tables();
}

void spm_boot_init(void)
{
	spm_lock_init();
#if !defined(MTK_FPGA_EARLY_PORTING)
	spm_register_init();
#endif /* MTK_FPGA_EARLY_PORTING */
}

void spm_legacy_sleep_wfi(uint64_t x1, uint64_t x2, uint64_t x3)
{
	unsigned long mpidr;

	/* Enable the gic cpu interface */
	gic_cpuif_init();

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/*
	 * Enter standby state. dsb is good practice before using wfi
	 * to enter low power states.
	 */
	isb();
	dsb();

	/* Disable coherency if this cluster is to be turned off */
	disable_scu(mpidr);

	wfi();

	/* Enable coherency if this cluster was off */
	enable_scu(mpidr);
}
