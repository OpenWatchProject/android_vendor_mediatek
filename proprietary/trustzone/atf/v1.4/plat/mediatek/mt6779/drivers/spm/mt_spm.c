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
#include <mt_spm_pmic_wrap.h>
#include <plat_pm.h>
#include <platform_def.h>
#include <plat_private.h>
#include <scu.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <mtk_plat_common.h>
#include <mtk_gic_v3_main.h>
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
	[DYNA_LOAD_PCM_SUSPEND] = "spm_firmware.bin",
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

enum idle_cond_check_type {
	HWCG_CHECK_SCENARIO_ORIENTED = 0,
	HWCG_CHECK_RESOURCE_ORIENTED,
};

enum idle_cond_check_item {
	IDLE_COND_MTCMOS1 = 0,
	IDLE_COND_MTCMOS2,
	IDLE_COND_INFRA0,
	IDLE_COND_INFRA1,
	IDLE_COND_INFRA2,
	IDLE_COND_INFRA3,
	IDLE_COND_MMSYS0,
	IDLE_COND_MMSYS1,
	NR_CG_GRPS,
};

/* scenario-oriented */
enum mtk_idle_type_id {
	IDLE_TYPE_DP = 0,
	IDLE_TYPE_SO3,
	IDLE_TYPE_SO,
	NR_IDLE_TYPES,
};

static uint32_t idle_cond_mask_scenario[NR_IDLE_TYPES][NR_CG_GRPS] = {
	[IDLE_TYPE_DP] = {
		0xBE7000B8,     /* MTCMOS1 */
		0xBE7000B8,     /* MTCMOS2 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C5,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0xFFFFFFFF,     /* MMSYS0  */
		0x0001FFFF,     /* MMSYS1  */
	},
	[IDLE_TYPE_SO3] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS2 */
		0x0A040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000D1,     /* INFRA2  */
		0x08000000,     /* INFRA3  */
		0xFFFFFFFF,     /* MMSYS0  */
		0x0001FFFF,     /* MMSYS1  */
	},
	[IDLE_TYPE_SO] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS2 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C1,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0x000DFC00,     /* MMSYS0  */
		0x00003F7C,     /* MMSYS1  */
	},
};

enum mtk_resource_level_id {
	SPM_RES_LEVEL_DRAM,
	SPM_RES_LEVEL_SYSPLL,
	SPM_RES_LEVEL_BUS_26M,
	SPM_RES_LEVEL_PMIC_LP,
	NR_SPM_RES_LEVEL_TYPES,
};

static uint32_t idle_cond_mask_resource[NR_SPM_RES_LEVEL_TYPES][NR_CG_GRPS] = {
	[SPM_RES_LEVEL_DRAM] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS2 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C1,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0x03F63000,     /* MMSYS0  */
		0x00000000,     /* MMSYS1  */
	},
	[SPM_RES_LEVEL_SYSPLL] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS1 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C1,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0x03F63000,     /* MMSYS0  */
		0x00000000,     /* MMSYS1  */
	},
	[SPM_RES_LEVEL_BUS_26M] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS1 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000D1,     /* INFRA2  */
		0x08000000,     /* INFRA3  */
		0x00000000,     /* MMSYS0  */
		0x00000000,     /* MMSYS1  */
	},
	[SPM_RES_LEVEL_PMIC_LP] = {
		0, 0, 0, 0, 0, 0, 0, 0},
};

#define PLL_BIT_UNIVPLL (1 << 0)
#define PLL_BIT_MFGPLL  (1 << 1)
#define PLL_BIT_MSDCPLL (1 << 2)
#define PLL_BIT_TVDPLL  (1 << 3)
#define PLL_BIT_MMPLL   (1 << 4)

static unsigned int idle_pll_cond_mask = {
	PLL_BIT_UNIVPLL | PLL_BIT_MFGPLL |
	PLL_BIT_MSDCPLL | PLL_BIT_TVDPLL |
	PLL_BIT_MMPLL,
};

void spm_set_hardware_cg_check(void)
{
	if (hwcg_check_type == HWCG_CHECK_RESOURCE_ORIENTED) {
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA3]);

	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_0,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_1,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_MMSYS1]);

	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA3]);
	mmio_write_32(OTHER2SPM_CG_CHECK_MASK,
	~(0x0000001F & idle_pll_cond_mask));

	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_0,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_1,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MMSYS1]);
	} else if (hwcg_check_type == HWCG_CHECK_SCENARIO_ORIENTED) {
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_4_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_0_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_1_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_2_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_3_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_0,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_1,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_MMSYS1]);

	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_4_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_0_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_1_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_2_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_3_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA3]);
	mmio_write_32(OTHER2SPM_CG_CHECK_MASK,
	~(0x0000001F & idle_pll_cond_mask));

	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_4_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_0_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_1_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_2_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_3_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_0,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_1,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_MMSYS1]);
	}
}

void spm_set_sysclk_settle(void)
{
	__uint32_t settle;

	/* SYSCLK settle = MD SYSCLK settle but set it again for MD PDN */
	mmio_write_32(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE);
	settle = mmio_read_32(SPM_CLK_SETTLE);

	INFO("md_settle = %u, settle = %u\n", SPM_SYSCLK_SETTLE, settle);
}

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
		__uint16_t firmware_size = 0;
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
		copy_size = sizeof(struct pcm_desc) - offsetof(struct pcm_desc, pmem_words);
		memcpy((void *)&(dyna_load_pcm[i].desc.pmem_words), ptr + offset, copy_size);

		/* start of pcm_desc version */
		offset += copy_size;
		snprintf(dyna_load_pcm[i].version, PCM_FIRMWARE_VERSION_SIZE - 1,
				"%s", ptr + offset);
		pdesc->version = dyna_load_pcm[i].version;
		pdesc->base = (__uint32_t *) dyna_load_pcm[i].buf;
		pdesc->base_dma = (__uint64_t)dyna_load_pcm[i].buf;

		INFO("#@# %s(%d) use spmfw partition for %s - %s\n", __func__, __LINE__,
				dyna_load_pcm_path[i], pdesc->version);

		dyna_load_pcm[i].ready = 1;
		spm_fw_count++;
	}

	if (spm_fw_count == DYNA_LOAD_PCM_MAX)
#if defined(SPM_PC_CHECKABLE)
		dyna_load_pcm_done = SPM_FIRMWARE_STATUS_LOADED;
#else
		dyna_load_pcm_done = SPM_FIRMWARE_STATUS_LOADED_KICKED;
#endif

	return err;
}

__uint64_t spm_load_firmware_status(void)
{
#if defined(SPM_PC_CHECKABLE)
	__uint64_t bRet = SPM_FIRMWARE_STATUS_NOT_LOADED;

	if (dyna_load_pcm_done == SPM_FIRMWARE_STATUS_LOADED) {
		bRet = SPM_FIRMWARE_STATUS_LOADED;
		if (mmio_read_32(SPM_PC_STA) != 0)
			bRet = SPM_FIRMWARE_STATUS_LOADED_KICKED;
	}
	return bRet;
#else
	return dyna_load_pcm_done;
#endif
}

static void spm_kick_to_run(void)
{
	__uint64_t spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO |
				SPM_FLAG_DISABLE_VCORE_DVS |
				SPM_FLAG_DISABLE_VCORE_DFS;

	spm_dvfsfw_init(0, 0);

	/* Reuse spm_go_to_vcorefs for first kicking SPM */
	spm_go_to_vcorefs(spm_flags);
}

void spm_irq0_handler(__uint64_t x1, __uint64_t x2)
{
	if (x2 == 0) {
		mmio_write_32(SPM_IRQ_MASK, mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
		mmio_write_32(SPM_IRQ_STA, x1);
		mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT0);
	}
}

void spm_ap_mdsrc_req(__uint64_t x1)
{
	if (x1)
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) | AP_MDSMSRC_REQ_LSB);
	else
		mmio_write_32(AP_MDSRC_REQ, mmio_read_32(AP_MDSRC_REQ) & ~AP_MDSMSRC_REQ_LSB);
}

unsigned int spm_is_md1_wdt_wakeup(void)
{
	return (!!(mmio_read_32(SPM_WAKEUP_STA) & R12_MD1_WDT_B));
}

unsigned int spm_is_md1_pwr_on(void)
{
	return (!!(mmio_read_32(PWR_STATUS) & 0x1) &&
		(!!(mmio_read_32(MD1_PWR_CON) & MD1_PWR_ON_LSB)));
}

unsigned int spm_is_ap_mdsrc_ack(void)
{
	return (!!(mmio_read_32(AP_MDSRC_REQ) & AP_MDSMSRC_ACK_LSB));
}

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
void save_pwr_ctrl(struct pwr_ctrl *pwrctrl, unsigned int index, unsigned int val)
{
	switch (index) {
	case PW_PCM_FLAGS:
		pwrctrl->pcm_flags = val;
		break;
	case PW_PCM_FLAGS_CUST:
		pwrctrl->pcm_flags_cust = val;
		break;
	case PW_PCM_FLAGS_CUST_SET:
		pwrctrl->pcm_flags_cust_set = val;
		break;
	case PW_PCM_FLAGS_CUST_CLR:
		pwrctrl->pcm_flags_cust_clr = val;
		break;
	case PW_PCM_FLAGS1:
		pwrctrl->pcm_flags1 = val;
		break;
	case PW_PCM_FLAGS1_CUST:
		pwrctrl->pcm_flags1_cust = val;
		break;
	case PW_PCM_FLAGS1_CUST_SET:
		pwrctrl->pcm_flags1_cust_set = val;
		break;
	case PW_PCM_FLAGS1_CUST_CLR:
		pwrctrl->pcm_flags1_cust_clr = val;
		break;
	case PW_TIMER_VAL:
		pwrctrl->timer_val = val;
		break;
	case PW_TIMER_VAL_CUST:
		pwrctrl->timer_val_cust = val;
		break;
	case PW_TIMER_VAL_RAMP_EN:
		pwrctrl->timer_val_ramp_en = val;
		break;
	case PW_TIMER_VAL_RAMP_EN_SEC:
		pwrctrl->timer_val_ramp_en_sec = val;
		break;
	case PW_WAKE_SRC:
		pwrctrl->wake_src = val;
		break;
	case PW_WAKE_SRC_CUST:
		pwrctrl->wake_src_cust = val;
		break;
	case PW_WAKELOCK_TIMER_VAL:
		pwrctrl->wakelock_timer_val = val;
		break;
	case PW_WDT_DISABLE:
		pwrctrl->wdt_disable = val;
		break;
	case PW_REG_SRCCLKEN0_CTL:
		pwrctrl->reg_srcclken0_ctl = val;
		break;
	case PW_REG_SRCCLKEN1_CTL:
		pwrctrl->reg_srcclken1_ctl = val;
		break;
	case PW_REG_SPM_LOCK_INFRA_DCM:
		pwrctrl->reg_spm_lock_infra_dcm = val;
		break;
	case PW_REG_SRCCLKEN_MASK:
		pwrctrl->reg_srcclken_mask = val;
		break;
	case PW_REG_MD1_C32RM_EN:
		pwrctrl->reg_md1_c32rm_en = val;
		break;
	case PW_REG_MD2_C32RM_EN:
		pwrctrl->reg_md2_c32rm_en = val;
		break;
	case PW_REG_CLKSQ0_SEL_CTRL:
		pwrctrl->reg_clksq0_sel_ctrl = val;
		break;
	case PW_REG_CLKSQ1_SEL_CTRL:
		pwrctrl->reg_clksq1_sel_ctrl = val;
		break;
	case PW_REG_SRCCLKEN0_EN:
		pwrctrl->reg_srcclken0_en = val;
		break;
	case PW_REG_SRCCLKEN1_EN:
		pwrctrl->reg_srcclken1_en = val;
		break;
	case PW_REG_SYSCLK0_SRC_MASK_B:
		pwrctrl->reg_sysclk0_src_mask_b = val;
		break;
	case PW_REG_SYSCLK1_SRC_MASK_B:
		pwrctrl->reg_sysclk1_src_mask_b = val;
		break;
	case PW_REG_WFI_OP:
		pwrctrl->reg_wfi_op = val;
		break;
	case PW_REG_WFI_TYPE:
		pwrctrl->reg_wfi_type = val;
		break;
	case PW_REG_MP0_CPUTOP_IDLE_MASK:
		pwrctrl->reg_mp0_cputop_idle_mask = val;
		break;
	case PW_REG_MP1_CPUTOP_IDLE_MASK:
		pwrctrl->reg_mp1_cputop_idle_mask = val;
		break;
	case PW_REG_MCUSYS_IDLE_MASK:
		pwrctrl->reg_mcusys_idle_mask = val;
		break;
	case PW_REG_MD_APSRC_1_SEL:
		pwrctrl->reg_md_apsrc_1_sel = val;
		break;
	case PW_REG_MD_APSRC_0_SEL:
		pwrctrl->reg_md_apsrc_0_sel = val;
		break;
	case PW_REG_CONN_APSRC_SEL:
		pwrctrl->reg_conn_apsrc_sel = val;
		break;
	case PW_REG_SPM_APSRC_REQ:
		pwrctrl->reg_spm_apsrc_req = val;
		break;
	case PW_REG_SPM_F26M_REQ:
		pwrctrl->reg_spm_f26m_req = val;
		break;
	case PW_REG_SPM_INFRA_REQ:
		pwrctrl->reg_spm_infra_req = val;
		break;
	case PW_REG_SPM_VRF18_REQ:
		pwrctrl->reg_spm_vrf18_req = val;
		break;
	case PW_REG_SPM_DDR_EN_REQ:
		pwrctrl->reg_spm_ddr_en_req = val;
		break;
	case PW_REG_SPM_DDR_EN2_REQ:
		pwrctrl->reg_spm_ddr_en2_req = val;
		break;
	case PW_REG_SPM_DVFS_REQ:
		pwrctrl->reg_spm_dvfs_req = val;
		break;
	case PW_REG_SPM_SW_MAILBOX_REQ:
		pwrctrl->reg_spm_sw_mailbox_req = val;
		break;
	case PW_REG_SPM_SSPM_MAILBOX_REQ:
		pwrctrl->reg_spm_sspm_mailbox_req = val;
		break;
	case PW_REG_SPM_ADSP_MAILBOX_REQ:
		pwrctrl->reg_spm_adsp_mailbox_req = val;
		break;
	case PW_REG_SPM_SCP_MAILBOX_REQ:
		pwrctrl->reg_spm_scp_mailbox_req = val;
		break;
	case PW_REG_SPM_MCUSYS_PWR_EVENT_REQ:
		pwrctrl->reg_spm_mcusys_pwr_event_req = val;
		break;
	case PW_CPU_MD_DVFS_SOP_FORCE_ON:
		pwrctrl->cpu_md_dvfs_sop_force_on = val;
		break;
	case PW_REG_MD_SRCCLKENA_0_MASK_B:
		pwrctrl->reg_md_srcclkena_0_mask_b = val;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_0_MASK_B:
		pwrctrl->reg_md_srcclkena2infra_req_0_mask_b = val;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_0_MASK_B:
		pwrctrl->reg_md_apsrc2infra_req_0_mask_b = val;
		break;
	case PW_REG_MD_APSRC_REQ_0_MASK_B:
		pwrctrl->reg_md_apsrc_req_0_mask_b = val;
		break;
	case PW_REG_MD_VRF18_REQ_0_MASK_B:
		pwrctrl->reg_md_vrf18_req_0_mask_b = val;
		break;
	case PW_REG_MD_DDR_EN_0_MASK_B:
		pwrctrl->reg_md_ddr_en_0_mask_b = val;
		break;
	case PW_REG_MD_DDR_EN2_0_MASK_B:
		pwrctrl->reg_md_ddr_en2_0_mask_b = val;
		break;
	case PW_REG_MD_SRCCLKENA_1_MASK_B:
		pwrctrl->reg_md_srcclkena_1_mask_b = val;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_1_MASK_B:
		pwrctrl->reg_md_srcclkena2infra_req_1_mask_b = val;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_1_MASK_B:
		pwrctrl->reg_md_apsrc2infra_req_1_mask_b = val;
		break;
	case PW_REG_MD_APSRC_REQ_1_MASK_B:
		pwrctrl->reg_md_apsrc_req_1_mask_b = val;
		break;
	case PW_REG_MD_VRF18_REQ_1_MASK_B:
		pwrctrl->reg_md_vrf18_req_1_mask_b = val;
		break;
	case PW_REG_MD_DDR_EN_1_MASK_B:
		pwrctrl->reg_md_ddr_en_1_mask_b = val;
		break;
	case PW_REG_MD_DDR_EN2_1_MASK_B:
		pwrctrl->reg_md_ddr_en2_1_mask_b = val;
		break;
	case PW_REG_CONN_SRCCLKENA_MASK_B:
		pwrctrl->reg_conn_srcclkena_mask_b = val;
		break;
	case PW_REG_CONN_SRCCLKENB_MASK_B:
		pwrctrl->reg_conn_srcclkenb_mask_b = val;
		break;
	case PW_REG_CONN_INFRA_REQ_MASK_B:
		pwrctrl->reg_conn_infra_req_mask_b = val;
		break;
	case PW_REG_CONN_APSRC_REQ_MASK_B:
		pwrctrl->reg_conn_apsrc_req_mask_b = val;
		break;
	case PW_REG_CONN_VRF18_REQ_MASK_B:
		pwrctrl->reg_conn_vrf18_req_mask_b = val;
		break;
	case PW_REG_CONN_DDR_EN_MASK_B:
		pwrctrl->reg_conn_ddr_en_mask_b = val;
		break;
	case PW_REG_CONN_DDR_EN2_MASK_B:
		pwrctrl->reg_conn_ddr_en2_mask_b = val;
		break;
	case PW_REG_SRCCLKENI0_SRCCLKENA_MASK_B:
		pwrctrl->reg_srcclkeni0_srcclkena_mask_b = val;
		break;
	case PW_REG_SRCCLKENI0_INFRA_REQ_MASK_B:
		pwrctrl->reg_srcclkeni0_infra_req_mask_b = val;
		break;
	case PW_REG_SRCCLKENI1_SRCCLKENA_MASK_B:
		pwrctrl->reg_srcclkeni1_srcclkena_mask_b = val;
		break;
	case PW_REG_SRCCLKENI1_INFRA_REQ_MASK_B:
		pwrctrl->reg_srcclkeni1_infra_req_mask_b = val;
		break;
	case PW_REG_SRCCLKENI2_SRCCLKENA_MASK_B:
		pwrctrl->reg_srcclkeni2_srcclkena_mask_b = val;
		break;
	case PW_REG_SRCCLKENI2_INFRA_REQ_MASK_B:
		pwrctrl->reg_srcclkeni2_infra_req_mask_b = val;
		break;
	case PW_REG_INFRASYS_APSRC_REQ_MASK_B:
		pwrctrl->reg_infrasys_apsrc_req_mask_b = val;
		break;
	case PW_REG_INFRASYS_DDR_EN_MASK_B:
		pwrctrl->reg_infrasys_ddr_en_mask_b = val;
		break;
	case PW_REG_INFRASYS_DDR_EN2_MASK_B:
		pwrctrl->reg_infrasys_ddr_en2_mask_b = val;
		break;
	case PW_REG_MD32_SRCCLKENA_MASK_B:
		pwrctrl->reg_md32_srcclkena_mask_b = val;
		break;
	case PW_REG_CONN_VFE28_REQ_MASK_B:
		pwrctrl->reg_conn_vfe28_req_mask_b = val;
		break;
	case PW_REG_MD32_INFRA_REQ_MASK_B:
		pwrctrl->reg_md32_infra_req_mask_b = val;
		break;
	case PW_REG_MD32_APSRC_REQ_MASK_B:
		pwrctrl->reg_md32_apsrc_req_mask_b = val;
		break;
	case PW_REG_MD32_VRF18_REQ_MASK_B:
		pwrctrl->reg_md32_vrf18_req_mask_b = val;
		break;
	case PW_REG_MD32_DDR_EN_MASK_B:
		pwrctrl->reg_md32_ddr_en_mask_b = val;
		break;
	case PW_REG_MD32_DDR_EN2_MASK_B:
		pwrctrl->reg_md32_ddr_en2_mask_b = val;
		break;
	case PW_REG_SCP_SRCCLKENA_MASK_B:
		pwrctrl->reg_scp_srcclkena_mask_b = val;
		break;
	case PW_REG_SCP_INFRA_REQ_MASK_B:
		pwrctrl->reg_scp_infra_req_mask_b = val;
		break;
	case PW_REG_SCP_APSRC_REQ_MASK_B:
		pwrctrl->reg_scp_apsrc_req_mask_b = val;
		break;
	case PW_REG_SCP_VRF18_REQ_MASK_B:
		pwrctrl->reg_scp_vrf18_req_mask_b = val;
		break;
	case PW_REG_SCP_DDR_EN_MASK_B:
		pwrctrl->reg_scp_ddr_en_mask_b = val;
		break;
	case PW_REG_SCP_DDR_EN2_MASK_B:
		pwrctrl->reg_scp_ddr_en2_mask_b = val;
		break;
	case PW_REG_UFS_SRCCLKENA_MASK_B:
		pwrctrl->reg_ufs_srcclkena_mask_b = val;
		break;
	case PW_REG_UFS_INFRA_REQ_MASK_B:
		pwrctrl->reg_ufs_infra_req_mask_b = val;
		break;
	case PW_REG_UFS_APSRC_REQ_MASK_B:
		pwrctrl->reg_ufs_apsrc_req_mask_b = val;
		break;
	case PW_REG_UFS_VRF18_REQ_MASK_B:
		pwrctrl->reg_ufs_vrf18_req_mask_b = val;
		break;
	case PW_REG_UFS_DDR_EN_MASK_B:
		pwrctrl->reg_ufs_ddr_en_mask_b = val;
		break;
	case PW_REG_UFS_DDR_EN2_MASK_B:
		pwrctrl->reg_ufs_ddr_en2_mask_b = val;
		break;
	case PW_REG_DISP0_APSRC_REQ_MASK_B:
		pwrctrl->reg_disp0_apsrc_req_mask_b = val;
		break;
	case PW_REG_DISP0_DDR_EN_MASK_B:
		pwrctrl->reg_disp0_ddr_en_mask_b = val;
		break;
	case PW_REG_DISP0_DDR_EN2_MASK_B:
		pwrctrl->reg_disp0_ddr_en2_mask_b = val;
		break;
	case PW_REG_DISP1_APSRC_REQ_MASK_B:
		pwrctrl->reg_disp1_apsrc_req_mask_b = val;
		break;
	case PW_REG_DISP1_DDR_EN_MASK_B:
		pwrctrl->reg_disp1_ddr_en_mask_b = val;
		break;
	case PW_REG_DISP1_DDR_EN2_MASK_B:
		pwrctrl->reg_disp1_ddr_en2_mask_b = val;
		break;
	case PW_REG_GCE_INFRA_REQ_MASK_B:
		pwrctrl->reg_gce_infra_req_mask_b = val;
		break;
	case PW_REG_GCE_APSRC_REQ_MASK_B:
		pwrctrl->reg_gce_apsrc_req_mask_b = val;
		break;
	case PW_REG_GCE_VRF18_REQ_MASK_B:
		pwrctrl->reg_gce_vrf18_req_mask_b = val;
		break;
	case PW_REG_GCE_DDR_EN_MASK_B:
		pwrctrl->reg_gce_ddr_en_mask_b = val;
		break;
	case PW_REG_GCE_DDR_EN2_MASK_B:
		pwrctrl->reg_gce_ddr_en2_mask_b = val;
		break;
	case PW_REG_EMI_CH0_DDR_EN_MASK_B:
		pwrctrl->reg_emi_ch0_ddr_en_mask_b = val;
		break;
	case PW_REG_EMI_CH1_DDR_EN_MASK_B:
		pwrctrl->reg_emi_ch1_ddr_en_mask_b = val;
		break;
	case PW_REG_EMI_CH0_DDR_EN2_MASK_B:
		pwrctrl->reg_emi_ch0_ddr_en2_mask_b = val;
		break;
	case PW_REG_EMI_CH1_DDR_EN2_MASK_B:
		pwrctrl->reg_emi_ch1_ddr_en2_mask_b = val;
		break;
	case PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B:
		pwrctrl->reg_dvfsrc_event_trigger_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT0_MASK_B:
		pwrctrl->reg_sw2spm_int0_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT1_MASK_B:
		pwrctrl->reg_sw2spm_int1_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT2_MASK_B:
		pwrctrl->reg_sw2spm_int2_mask_b = val;
		break;
	case PW_REG_SW2SPM_INT3_MASK_B:
		pwrctrl->reg_sw2spm_int3_mask_b = val;
		break;
	case PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B:
		pwrctrl->reg_sc_adsp2spm_wakeup_mask_b = val;
		break;
	case PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B:
		pwrctrl->reg_sc_sspm2spm_wakeup_mask_b = val;
		break;
	case PW_REG_SC_SCP2SPM_WAKEUP_MASK_B:
		pwrctrl->reg_sc_scp2spm_wakeup_mask_b = val;
		break;
	case PW_REG_CSYSPWRREQ_MASK:
		pwrctrl->reg_csyspwrreq_mask = val;
		break;
	case PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B:
		pwrctrl->reg_spm_srcclkena_reserved_mask_b = val;
		break;
	case PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B:
		pwrctrl->reg_spm_infra_req_reserved_mask_b = val;
		break;
	case PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B:
		pwrctrl->reg_spm_apsrc_req_reserved_mask_b = val;
		break;
	case PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B:
		pwrctrl->reg_spm_vrf18_req_reserved_mask_b = val;
		break;
	case PW_REG_SPM_DDR_EN_RESERVED_MASK_B:
		pwrctrl->reg_spm_ddr_en_reserved_mask_b = val;
		break;
	case PW_REG_SPM_DDR_EN2_RESERVED_MASK_B:
		pwrctrl->reg_spm_ddr_en2_reserved_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B:
		pwrctrl->reg_audio_dsp_srcclkena_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B:
		pwrctrl->reg_audio_dsp_infra_req_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B:
		pwrctrl->reg_audio_dsp_apsrc_req_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B:
		pwrctrl->reg_audio_dsp_vrf18_req_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_DDR_EN_MASK_B:
		pwrctrl->reg_audio_dsp_ddr_en_mask_b = val;
		break;
	case PW_REG_AUDIO_DSP_DDR_EN2_MASK_B:
		pwrctrl->reg_audio_dsp_ddr_en2_mask_b = val;
		break;
	case PW_REG_MCUSYS_PWR_EVENT_MASK_B:
		pwrctrl->reg_mcusys_pwr_event_mask_b = val;
		break;
	case PW_REG_MSDC0_SRCCLKENA_MASK_B:
		pwrctrl->reg_msdc0_srcclkena_mask_b = val;
		break;
	case PW_REG_MSDC0_INFRA_REQ_MASK_B:
		pwrctrl->reg_msdc0_infra_req_mask_b = val;
		break;
	case PW_REG_MSDC0_APSRC_REQ_MASK_B:
		pwrctrl->reg_msdc0_apsrc_req_mask_b = val;
		break;
	case PW_REG_MSDC0_VRF18_REQ_MASK_B:
		pwrctrl->reg_msdc0_vrf18_req_mask_b = val;
		break;
	case PW_REG_MSDC0_DDR_EN_MASK_B:
		pwrctrl->reg_msdc0_ddr_en_mask_b = val;
		break;
	case PW_REG_MSDC0_DDR_EN2_MASK_B:
		pwrctrl->reg_msdc0_ddr_en2_mask_b = val;
		break;
	case PW_REG_CONN_SRCCLKENB2PWRAP_MASK_B:
		pwrctrl->reg_conn_srcclkenb2pwrap_mask_b = val;
		break;
	case PW_CCIF_EVENT_MASK_B:
		pwrctrl->ccif_event_mask_b = val;
		break;
	case PW_REG_APU_CORE0_SRCCLKENA_MASK_B:
		pwrctrl->reg_apu_core0_srcclkena_mask_b = val;
		break;
	case PW_REG_APU_CORE0_INFRA_REQ_MASK_B:
		pwrctrl->reg_apu_core0_infra_req_mask_b = val;
		break;
	case PW_REG_APU_CORE0_APSRC_REQ_MASK_B:
		pwrctrl->reg_apu_core0_apsrc_req_mask_b = val;
		break;
	case PW_REG_APU_CORE0_VRF18_REQ_MASK_B:
		pwrctrl->reg_apu_core0_vrf18_req_mask_b = val;
		break;
	case PW_REG_APU_CORE0_DDR_EN_MASK_B:
		pwrctrl->reg_apu_core0_ddr_en_mask_b = val;
		break;
	case PW_REG_APU_CORE1_SRCCLKENA_MASK_B:
		pwrctrl->reg_apu_core1_srcclkena_mask_b = val;
		break;
	case PW_REG_APU_CORE1_INFRA_REQ_MASK_B:
		pwrctrl->reg_apu_core1_infra_req_mask_b = val;
		break;
	case PW_REG_APU_CORE1_APSRC_REQ_MASK_B:
		pwrctrl->reg_apu_core1_apsrc_req_mask_b = val;
		break;
	case PW_REG_APU_CORE1_VRF18_REQ_MASK_B:
		pwrctrl->reg_apu_core1_vrf18_req_mask_b = val;
		break;
	case PW_REG_APU_CORE1_DDR_EN_MASK_B:
		pwrctrl->reg_apu_core1_ddr_en_mask_b = val;
		break;
	case PW_REG_APU_CORE2_SRCCLKENA_MASK_B:
		pwrctrl->reg_apu_core2_srcclkena_mask_b = val;
		break;
	case PW_REG_APU_CORE2_INFRA_REQ_MASK_B:
		pwrctrl->reg_apu_core2_infra_req_mask_b = val;
		break;
	case PW_REG_APU_CORE2_APSRC_REQ_MASK_B:
		pwrctrl->reg_apu_core2_apsrc_req_mask_b = val;
		break;
	case PW_REG_APU_CORE2_VRF18_REQ_MASK_B:
		pwrctrl->reg_apu_core2_vrf18_req_mask_b = val;
		break;
	case PW_REG_APU_CORE2_DDR_EN_MASK_B:
		pwrctrl->reg_apu_core2_ddr_en_mask_b = val;
		break;
	case PW_REG_APU_CORE2_DDR_EN2_MASK_B:
		pwrctrl->reg_apu_core2_ddr_en2_mask_b = val;
		break;
	case PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B:
		pwrctrl->reg_mcusys_merge_apsrc_req_mask_b = val;
		break;
	case PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B:
		pwrctrl->reg_mcusys_merge_ddr_en_mask_b = val;
		break;
	case PW_REG_MCUSYS_MERGE_DDR_EN2_MASK_B:
		pwrctrl->reg_mcusys_merge_ddr_en2_mask_b = val;
		break;
	case PW_REG_APU_CORE0_DDR_EN2_MASK_B:
		pwrctrl->reg_apu_core0_ddr_en2_mask_b = val;
		break;
	case PW_REG_APU_CORE1_DDR_EN2_MASK_B:
		pwrctrl->reg_apu_core1_ddr_en2_mask_b = val;
		break;
	case PW_REG_CG_CHECK_DDR_EN_MASK_B:
		pwrctrl->reg_cg_check_ddr_en_mask_b = val;
		break;
	case PW_REG_CG_CHECK_DDR_EN2_MASK_B:
		pwrctrl->reg_cg_check_ddr_en2_mask_b = val;
		break;
	case PW_REG_WAKEUP_EVENT_MASK:
		pwrctrl->reg_wakeup_event_mask = val;
		break;
	case PW_REG_EXT_WAKEUP_EVENT_MASK:
		pwrctrl->reg_ext_wakeup_event_mask = val;
		break;
	case PW_REG_MSDC1_SRCCLKENA_MASK_B:
		pwrctrl->reg_msdc1_srcclkena_mask_b = val;
		break;
	case PW_REG_MSDC1_INFRA_REQ_MASK_B:
		pwrctrl->reg_msdc1_infra_req_mask_b = val;
		break;
	case PW_REG_MSDC1_APSRC_REQ_MASK_B:
		pwrctrl->reg_msdc1_apsrc_req_mask_b = val;
		break;
	case PW_REG_MSDC1_VRF18_REQ_MASK_B:
		pwrctrl->reg_msdc1_vrf18_req_mask_b = val;
		break;
	case PW_REG_MSDC1_DDR_EN_MASK_B:
		pwrctrl->reg_msdc1_ddr_en_mask_b = val;
		break;
	case PW_REG_MSDC1_DDR_EN2_MASK_B:
		pwrctrl->reg_msdc1_ddr_en2_mask_b = val;
		break;
	case PW_REG_MSDC1_SRCCLKENA_ACK_MASK:
		pwrctrl->reg_msdc1_srcclkena_ack_mask = val;
		break;
	case PW_REG_MSDC1_INFRA_ACK_MASK:
		pwrctrl->reg_msdc1_infra_ack_mask = val;
		break;
	case PW_REG_MSDC1_APSRC_ACK_MASK:
		pwrctrl->reg_msdc1_apsrc_ack_mask = val;
		break;
	case PW_REG_MSDC1_VRF18_ACK_MASK:
		pwrctrl->reg_msdc1_vrf18_ack_mask = val;
		break;
	case PW_REG_MSDC1_DDR_EN_ACK_MASK:
		pwrctrl->reg_msdc1_ddr_en_ack_mask = val;
		break;
	case PW_REG_MSDC1_DDR_EN2_ACK_MASK:
		pwrctrl->reg_msdc1_ddr_en2_ack_mask = val;
		break;
	case PW_MP0_CPU0_WFI_EN:
		pwrctrl->mp0_cpu0_wfi_en = val;
		break;
	case PW_MP0_CPU1_WFI_EN:
		pwrctrl->mp0_cpu1_wfi_en = val;
		break;
	case PW_MP0_CPU2_WFI_EN:
		pwrctrl->mp0_cpu2_wfi_en = val;
		break;
	case PW_MP0_CPU3_WFI_EN:
		pwrctrl->mp0_cpu3_wfi_en = val;
		break;
	case PW_MP0_CPU4_WFI_EN:
		pwrctrl->mp0_cpu4_wfi_en = val;
		break;
	case PW_MP0_CPU5_WFI_EN:
		pwrctrl->mp0_cpu5_wfi_en = val;
		break;
	case PW_MP0_CPU6_WFI_EN:
		pwrctrl->mp0_cpu6_wfi_en = val;
		break;
	case PW_MP0_CPU7_WFI_EN:
		pwrctrl->mp0_cpu7_wfi_en = val;
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
	case PW_PCM_FLAGS:
		val = pwrctrl->pcm_flags;
		break;
	case PW_PCM_FLAGS_CUST:
		val = pwrctrl->pcm_flags_cust;
		break;
	case PW_PCM_FLAGS_CUST_SET:
		val = pwrctrl->pcm_flags_cust_set;
		break;
	case PW_PCM_FLAGS_CUST_CLR:
		val = pwrctrl->pcm_flags_cust_clr;
		break;
	case PW_PCM_FLAGS1:
		val = pwrctrl->pcm_flags1;
		break;
	case PW_PCM_FLAGS1_CUST:
		val = pwrctrl->pcm_flags1_cust;
		break;
	case PW_PCM_FLAGS1_CUST_SET:
		val = pwrctrl->pcm_flags1_cust_set;
		break;
	case PW_PCM_FLAGS1_CUST_CLR:
		val = pwrctrl->pcm_flags1_cust_clr;
		break;
	case PW_TIMER_VAL:
		val = pwrctrl->timer_val;
		break;
	case PW_TIMER_VAL_CUST:
		val = pwrctrl->timer_val_cust;
		break;
	case PW_TIMER_VAL_RAMP_EN:
		val = pwrctrl->timer_val_ramp_en;
		break;
	case PW_TIMER_VAL_RAMP_EN_SEC:
		val = pwrctrl->timer_val_ramp_en_sec;
		break;
	case PW_WAKE_SRC:
		val = pwrctrl->wake_src;
		break;
	case PW_WAKE_SRC_CUST:
		val = pwrctrl->wake_src_cust;
		break;
	case PW_WAKELOCK_TIMER_VAL:
		val = pwrctrl->wakelock_timer_val;
		break;
	case PW_WDT_DISABLE:
		val = pwrctrl->wdt_disable;
		break;
	case PW_REG_SRCCLKEN0_CTL:
		val = pwrctrl->reg_srcclken0_ctl;
		break;
	case PW_REG_SRCCLKEN1_CTL:
		val = pwrctrl->reg_srcclken1_ctl;
		break;
	case PW_REG_SPM_LOCK_INFRA_DCM:
		val = pwrctrl->reg_spm_lock_infra_dcm;
		break;
	case PW_REG_SRCCLKEN_MASK:
		val = pwrctrl->reg_srcclken_mask;
		break;
	case PW_REG_MD1_C32RM_EN:
		val = pwrctrl->reg_md1_c32rm_en;
		break;
	case PW_REG_MD2_C32RM_EN:
		val = pwrctrl->reg_md2_c32rm_en;
		break;
	case PW_REG_CLKSQ0_SEL_CTRL:
		val = pwrctrl->reg_clksq0_sel_ctrl;
		break;
	case PW_REG_CLKSQ1_SEL_CTRL:
		val = pwrctrl->reg_clksq1_sel_ctrl;
		break;
	case PW_REG_SRCCLKEN0_EN:
		val = pwrctrl->reg_srcclken0_en;
		break;
	case PW_REG_SRCCLKEN1_EN:
		val = pwrctrl->reg_srcclken1_en;
		break;
	case PW_REG_SYSCLK0_SRC_MASK_B:
		val = pwrctrl->reg_sysclk0_src_mask_b;
		break;
	case PW_REG_SYSCLK1_SRC_MASK_B:
		val = pwrctrl->reg_sysclk1_src_mask_b;
		break;
	case PW_REG_WFI_OP:
		val = pwrctrl->reg_wfi_op;
		break;
	case PW_REG_WFI_TYPE:
		val = pwrctrl->reg_wfi_type;
		break;
	case PW_REG_MP0_CPUTOP_IDLE_MASK:
		val = pwrctrl->reg_mp0_cputop_idle_mask;
		break;
	case PW_REG_MP1_CPUTOP_IDLE_MASK:
		val = pwrctrl->reg_mp1_cputop_idle_mask;
		break;
	case PW_REG_MCUSYS_IDLE_MASK:
		val = pwrctrl->reg_mcusys_idle_mask;
		break;
	case PW_REG_MD_APSRC_1_SEL:
		val = pwrctrl->reg_md_apsrc_1_sel;
		break;
	case PW_REG_MD_APSRC_0_SEL:
		val = pwrctrl->reg_md_apsrc_0_sel;
		break;
	case PW_REG_CONN_APSRC_SEL:
		val = pwrctrl->reg_conn_apsrc_sel;
		break;
	case PW_REG_SPM_APSRC_REQ:
		val = pwrctrl->reg_spm_apsrc_req;
		break;
	case PW_REG_SPM_F26M_REQ:
		val = pwrctrl->reg_spm_f26m_req;
		break;
	case PW_REG_SPM_INFRA_REQ:
		val = pwrctrl->reg_spm_infra_req;
		break;
	case PW_REG_SPM_VRF18_REQ:
		val = pwrctrl->reg_spm_vrf18_req;
		break;
	case PW_REG_SPM_DDR_EN_REQ:
		val = pwrctrl->reg_spm_ddr_en_req;
		break;
	case PW_REG_SPM_DDR_EN2_REQ:
		val = pwrctrl->reg_spm_ddr_en2_req;
		break;
	case PW_REG_SPM_DVFS_REQ:
		val = pwrctrl->reg_spm_dvfs_req;
		break;
	case PW_REG_SPM_SW_MAILBOX_REQ:
		val = pwrctrl->reg_spm_sw_mailbox_req;
		break;
	case PW_REG_SPM_SSPM_MAILBOX_REQ:
		val = pwrctrl->reg_spm_sspm_mailbox_req;
		break;
	case PW_REG_SPM_ADSP_MAILBOX_REQ:
		val = pwrctrl->reg_spm_adsp_mailbox_req;
		break;
	case PW_REG_SPM_SCP_MAILBOX_REQ:
		val = pwrctrl->reg_spm_scp_mailbox_req;
		break;
	case PW_REG_SPM_MCUSYS_PWR_EVENT_REQ:
		val = pwrctrl->reg_spm_mcusys_pwr_event_req;
		break;
	case PW_CPU_MD_DVFS_SOP_FORCE_ON:
		val = pwrctrl->cpu_md_dvfs_sop_force_on;
		break;
	case PW_REG_MD_SRCCLKENA_0_MASK_B:
		val = pwrctrl->reg_md_srcclkena_0_mask_b;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_0_MASK_B:
		val = pwrctrl->reg_md_srcclkena2infra_req_0_mask_b;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_0_MASK_B:
		val = pwrctrl->reg_md_apsrc2infra_req_0_mask_b;
		break;
	case PW_REG_MD_APSRC_REQ_0_MASK_B:
		val = pwrctrl->reg_md_apsrc_req_0_mask_b;
		break;
	case PW_REG_MD_VRF18_REQ_0_MASK_B:
		val = pwrctrl->reg_md_vrf18_req_0_mask_b;
		break;
	case PW_REG_MD_DDR_EN_0_MASK_B:
		val = pwrctrl->reg_md_ddr_en_0_mask_b;
		break;
	case PW_REG_MD_DDR_EN2_0_MASK_B:
		val = pwrctrl->reg_md_ddr_en2_0_mask_b;
		break;
	case PW_REG_MD_SRCCLKENA_1_MASK_B:
		val = pwrctrl->reg_md_srcclkena_1_mask_b;
		break;
	case PW_REG_MD_SRCCLKENA2INFRA_REQ_1_MASK_B:
		val = pwrctrl->reg_md_srcclkena2infra_req_1_mask_b;
		break;
	case PW_REG_MD_APSRC2INFRA_REQ_1_MASK_B:
		val = pwrctrl->reg_md_apsrc2infra_req_1_mask_b;
		break;
	case PW_REG_MD_APSRC_REQ_1_MASK_B:
		val = pwrctrl->reg_md_apsrc_req_1_mask_b;
		break;
	case PW_REG_MD_VRF18_REQ_1_MASK_B:
		val = pwrctrl->reg_md_vrf18_req_1_mask_b;
		break;
	case PW_REG_MD_DDR_EN_1_MASK_B:
		val = pwrctrl->reg_md_ddr_en_1_mask_b;
		break;
	case PW_REG_MD_DDR_EN2_1_MASK_B:
		val = pwrctrl->reg_md_ddr_en2_1_mask_b;
		break;
	case PW_REG_CONN_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_conn_srcclkena_mask_b;
		break;
	case PW_REG_CONN_SRCCLKENB_MASK_B:
		val = pwrctrl->reg_conn_srcclkenb_mask_b;
		break;
	case PW_REG_CONN_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_conn_infra_req_mask_b;
		break;
	case PW_REG_CONN_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_conn_apsrc_req_mask_b;
		break;
	case PW_REG_CONN_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_conn_vrf18_req_mask_b;
		break;
	case PW_REG_CONN_DDR_EN_MASK_B:
		val = pwrctrl->reg_conn_ddr_en_mask_b;
		break;
	case PW_REG_CONN_DDR_EN2_MASK_B:
		val = pwrctrl->reg_conn_ddr_en2_mask_b;
		break;
	case PW_REG_SRCCLKENI0_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_srcclkeni0_srcclkena_mask_b;
		break;
	case PW_REG_SRCCLKENI0_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_srcclkeni0_infra_req_mask_b;
		break;
	case PW_REG_SRCCLKENI1_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_srcclkeni1_srcclkena_mask_b;
		break;
	case PW_REG_SRCCLKENI1_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_srcclkeni1_infra_req_mask_b;
		break;
	case PW_REG_SRCCLKENI2_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_srcclkeni2_srcclkena_mask_b;
		break;
	case PW_REG_SRCCLKENI2_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_srcclkeni2_infra_req_mask_b;
		break;
	case PW_REG_INFRASYS_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_infrasys_apsrc_req_mask_b;
		break;
	case PW_REG_INFRASYS_DDR_EN_MASK_B:
		val = pwrctrl->reg_infrasys_ddr_en_mask_b;
		break;
	case PW_REG_INFRASYS_DDR_EN2_MASK_B:
		val = pwrctrl->reg_infrasys_ddr_en2_mask_b;
		break;
	case PW_REG_MD32_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_md32_srcclkena_mask_b;
		break;
	case PW_REG_CONN_VFE28_REQ_MASK_B:
		val = pwrctrl->reg_conn_vfe28_req_mask_b;
		break;
	case PW_REG_MD32_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_md32_infra_req_mask_b;
		break;
	case PW_REG_MD32_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_md32_apsrc_req_mask_b;
		break;
	case PW_REG_MD32_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_md32_vrf18_req_mask_b;
		break;
	case PW_REG_MD32_DDR_EN_MASK_B:
		val = pwrctrl->reg_md32_ddr_en_mask_b;
		break;
	case PW_REG_MD32_DDR_EN2_MASK_B:
		val = pwrctrl->reg_md32_ddr_en2_mask_b;
		break;
	case PW_REG_SCP_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_scp_srcclkena_mask_b;
		break;
	case PW_REG_SCP_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_scp_infra_req_mask_b;
		break;
	case PW_REG_SCP_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_scp_apsrc_req_mask_b;
		break;
	case PW_REG_SCP_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_scp_vrf18_req_mask_b;
		break;
	case PW_REG_SCP_DDR_EN_MASK_B:
		val = pwrctrl->reg_scp_ddr_en_mask_b;
		break;
	case PW_REG_SCP_DDR_EN2_MASK_B:
		val = pwrctrl->reg_scp_ddr_en2_mask_b;
		break;
	case PW_REG_UFS_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_ufs_srcclkena_mask_b;
		break;
	case PW_REG_UFS_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_ufs_infra_req_mask_b;
		break;
	case PW_REG_UFS_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_ufs_apsrc_req_mask_b;
		break;
	case PW_REG_UFS_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_ufs_vrf18_req_mask_b;
		break;
	case PW_REG_UFS_DDR_EN_MASK_B:
		val = pwrctrl->reg_ufs_ddr_en_mask_b;
		break;
	case PW_REG_UFS_DDR_EN2_MASK_B:
		val = pwrctrl->reg_ufs_ddr_en2_mask_b;
		break;
	case PW_REG_DISP0_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_disp0_apsrc_req_mask_b;
		break;
	case PW_REG_DISP0_DDR_EN_MASK_B:
		val = pwrctrl->reg_disp0_ddr_en_mask_b;
		break;
	case PW_REG_DISP0_DDR_EN2_MASK_B:
		val = pwrctrl->reg_disp0_ddr_en2_mask_b;
		break;
	case PW_REG_DISP1_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_disp1_apsrc_req_mask_b;
		break;
	case PW_REG_DISP1_DDR_EN_MASK_B:
		val = pwrctrl->reg_disp1_ddr_en_mask_b;
		break;
	case PW_REG_DISP1_DDR_EN2_MASK_B:
		val = pwrctrl->reg_disp1_ddr_en2_mask_b;
		break;
	case PW_REG_GCE_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_gce_infra_req_mask_b;
		break;
	case PW_REG_GCE_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_gce_apsrc_req_mask_b;
		break;
	case PW_REG_GCE_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_gce_vrf18_req_mask_b;
		break;
	case PW_REG_GCE_DDR_EN_MASK_B:
		val = pwrctrl->reg_gce_ddr_en_mask_b;
		break;
	case PW_REG_GCE_DDR_EN2_MASK_B:
		val = pwrctrl->reg_gce_ddr_en2_mask_b;
		break;
	case PW_REG_EMI_CH0_DDR_EN_MASK_B:
		val = pwrctrl->reg_emi_ch0_ddr_en_mask_b;
		break;
	case PW_REG_EMI_CH1_DDR_EN_MASK_B:
		val = pwrctrl->reg_emi_ch1_ddr_en_mask_b;
		break;
	case PW_REG_EMI_CH0_DDR_EN2_MASK_B:
		val = pwrctrl->reg_emi_ch0_ddr_en2_mask_b;
		break;
	case PW_REG_EMI_CH1_DDR_EN2_MASK_B:
		val = pwrctrl->reg_emi_ch1_ddr_en2_mask_b;
		break;
	case PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B:
		val = pwrctrl->reg_dvfsrc_event_trigger_mask_b;
		break;
	case PW_REG_SW2SPM_INT0_MASK_B:
		val = pwrctrl->reg_sw2spm_int0_mask_b;
		break;
	case PW_REG_SW2SPM_INT1_MASK_B:
		val = pwrctrl->reg_sw2spm_int1_mask_b;
		break;
	case PW_REG_SW2SPM_INT2_MASK_B:
		val = pwrctrl->reg_sw2spm_int2_mask_b;
		break;
	case PW_REG_SW2SPM_INT3_MASK_B:
		val = pwrctrl->reg_sw2spm_int3_mask_b;
		break;
	case PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B:
		val = pwrctrl->reg_sc_adsp2spm_wakeup_mask_b;
		break;
	case PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B:
		val = pwrctrl->reg_sc_sspm2spm_wakeup_mask_b;
		break;
	case PW_REG_SC_SCP2SPM_WAKEUP_MASK_B:
		val = pwrctrl->reg_sc_scp2spm_wakeup_mask_b;
		break;
	case PW_REG_CSYSPWRREQ_MASK:
		val = pwrctrl->reg_csyspwrreq_mask;
		break;
	case PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_srcclkena_reserved_mask_b;
		break;
	case PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_infra_req_reserved_mask_b;
		break;
	case PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_apsrc_req_reserved_mask_b;
		break;
	case PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_vrf18_req_reserved_mask_b;
		break;
	case PW_REG_SPM_DDR_EN_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_ddr_en_reserved_mask_b;
		break;
	case PW_REG_SPM_DDR_EN2_RESERVED_MASK_B:
		val = pwrctrl->reg_spm_ddr_en2_reserved_mask_b;
		break;
	case PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_audio_dsp_srcclkena_mask_b;
		break;
	case PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_audio_dsp_infra_req_mask_b;
		break;
	case PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_audio_dsp_apsrc_req_mask_b;
		break;
	case PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_audio_dsp_vrf18_req_mask_b;
		break;
	case PW_REG_AUDIO_DSP_DDR_EN_MASK_B:
		val = pwrctrl->reg_audio_dsp_ddr_en_mask_b;
		break;
	case PW_REG_AUDIO_DSP_DDR_EN2_MASK_B:
		val = pwrctrl->reg_audio_dsp_ddr_en2_mask_b;
		break;
	case PW_REG_MCUSYS_PWR_EVENT_MASK_B:
		val = pwrctrl->reg_mcusys_pwr_event_mask_b;
		break;
	case PW_REG_MSDC0_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_msdc0_srcclkena_mask_b;
		break;
	case PW_REG_MSDC0_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_msdc0_infra_req_mask_b;
		break;
	case PW_REG_MSDC0_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_msdc0_apsrc_req_mask_b;
		break;
	case PW_REG_MSDC0_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_msdc0_vrf18_req_mask_b;
		break;
	case PW_REG_MSDC0_DDR_EN_MASK_B:
		val = pwrctrl->reg_msdc0_ddr_en_mask_b;
		break;
	case PW_REG_MSDC0_DDR_EN2_MASK_B:
		val = pwrctrl->reg_msdc0_ddr_en2_mask_b;
		break;
	case PW_REG_CONN_SRCCLKENB2PWRAP_MASK_B:
		val = pwrctrl->reg_conn_srcclkenb2pwrap_mask_b;
		break;
	case PW_CCIF_EVENT_MASK_B:
		val = pwrctrl->ccif_event_mask_b;
		break;
	case PW_REG_APU_CORE0_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_apu_core0_srcclkena_mask_b;
		break;
	case PW_REG_APU_CORE0_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_apu_core0_infra_req_mask_b;
		break;
	case PW_REG_APU_CORE0_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_apu_core0_apsrc_req_mask_b;
		break;
	case PW_REG_APU_CORE0_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_apu_core0_vrf18_req_mask_b;
		break;
	case PW_REG_APU_CORE0_DDR_EN_MASK_B:
		val = pwrctrl->reg_apu_core0_ddr_en_mask_b;
		break;
	case PW_REG_APU_CORE1_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_apu_core1_srcclkena_mask_b;
		break;
	case PW_REG_APU_CORE1_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_apu_core1_infra_req_mask_b;
		break;
	case PW_REG_APU_CORE1_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_apu_core1_apsrc_req_mask_b;
		break;
	case PW_REG_APU_CORE1_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_apu_core1_vrf18_req_mask_b;
		break;
	case PW_REG_APU_CORE1_DDR_EN_MASK_B:
		val = pwrctrl->reg_apu_core1_ddr_en_mask_b;
		break;
	case PW_REG_APU_CORE2_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_apu_core2_srcclkena_mask_b;
		break;
	case PW_REG_APU_CORE2_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_apu_core2_infra_req_mask_b;
		break;
	case PW_REG_APU_CORE2_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_apu_core2_apsrc_req_mask_b;
		break;
	case PW_REG_APU_CORE2_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_apu_core2_vrf18_req_mask_b;
		break;
	case PW_REG_APU_CORE2_DDR_EN_MASK_B:
		val = pwrctrl->reg_apu_core2_ddr_en_mask_b;
		break;
	case PW_REG_APU_CORE2_DDR_EN2_MASK_B:
		val = pwrctrl->reg_apu_core2_ddr_en2_mask_b;
		break;
	case PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_mcusys_merge_apsrc_req_mask_b;
		break;
	case PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B:
		val = pwrctrl->reg_mcusys_merge_ddr_en_mask_b;
		break;
	case PW_REG_MCUSYS_MERGE_DDR_EN2_MASK_B:
		val = pwrctrl->reg_mcusys_merge_ddr_en2_mask_b;
		break;
	case PW_REG_APU_CORE0_DDR_EN2_MASK_B:
		val = pwrctrl->reg_apu_core0_ddr_en2_mask_b;
		break;
	case PW_REG_APU_CORE1_DDR_EN2_MASK_B:
		val = pwrctrl->reg_apu_core1_ddr_en2_mask_b;
		break;
	case PW_REG_CG_CHECK_DDR_EN_MASK_B:
		val = pwrctrl->reg_cg_check_ddr_en_mask_b;
		break;
	case PW_REG_CG_CHECK_DDR_EN2_MASK_B:
		val = pwrctrl->reg_cg_check_ddr_en2_mask_b;
		break;
	case PW_REG_WAKEUP_EVENT_MASK:
		val = pwrctrl->reg_wakeup_event_mask;
		break;
	case PW_REG_EXT_WAKEUP_EVENT_MASK:
		val = pwrctrl->reg_ext_wakeup_event_mask;
		break;
	case PW_REG_MSDC1_SRCCLKENA_MASK_B:
		val = pwrctrl->reg_msdc1_srcclkena_mask_b;
		break;
	case PW_REG_MSDC1_INFRA_REQ_MASK_B:
		val = pwrctrl->reg_msdc1_infra_req_mask_b;
		break;
	case PW_REG_MSDC1_APSRC_REQ_MASK_B:
		val = pwrctrl->reg_msdc1_apsrc_req_mask_b;
		break;
	case PW_REG_MSDC1_VRF18_REQ_MASK_B:
		val = pwrctrl->reg_msdc1_vrf18_req_mask_b;
		break;
	case PW_REG_MSDC1_DDR_EN_MASK_B:
		val = pwrctrl->reg_msdc1_ddr_en_mask_b;
		break;
	case PW_REG_MSDC1_DDR_EN2_MASK_B:
		val = pwrctrl->reg_msdc1_ddr_en2_mask_b;
		break;
	case PW_REG_MSDC1_SRCCLKENA_ACK_MASK:
		val = pwrctrl->reg_msdc1_srcclkena_ack_mask;
		break;
	case PW_REG_MSDC1_INFRA_ACK_MASK:
		val = pwrctrl->reg_msdc1_infra_ack_mask;
		break;
	case PW_REG_MSDC1_APSRC_ACK_MASK:
		val = pwrctrl->reg_msdc1_apsrc_ack_mask;
		break;
	case PW_REG_MSDC1_VRF18_ACK_MASK:
		val = pwrctrl->reg_msdc1_vrf18_ack_mask;
		break;
	case PW_REG_MSDC1_DDR_EN_ACK_MASK:
		val = pwrctrl->reg_msdc1_ddr_en_ack_mask;
		break;
	case PW_REG_MSDC1_DDR_EN2_ACK_MASK:
		val = pwrctrl->reg_msdc1_ddr_en2_ack_mask;
		break;
	case PW_MP0_CPU0_WFI_EN:
		val = pwrctrl->mp0_cpu0_wfi_en;
		break;
	case PW_MP0_CPU1_WFI_EN:
		val = pwrctrl->mp0_cpu1_wfi_en;
		break;
	case PW_MP0_CPU2_WFI_EN:
		val = pwrctrl->mp0_cpu2_wfi_en;
		break;
	case PW_MP0_CPU3_WFI_EN:
		val = pwrctrl->mp0_cpu3_wfi_en;
		break;
	case PW_MP0_CPU4_WFI_EN:
		val = pwrctrl->mp0_cpu4_wfi_en;
		break;
	case PW_MP0_CPU5_WFI_EN:
		val = pwrctrl->mp0_cpu5_wfi_en;
		break;
	case PW_MP0_CPU6_WFI_EN:
		val = pwrctrl->mp0_cpu6_wfi_en;
		break;
	case PW_MP0_CPU7_WFI_EN:
		val = pwrctrl->mp0_cpu7_wfi_en;
		break;
	default:
		ERROR("%s: %d is undefined!!!\n", __func__, index);
	}

	return val;
}

void spm_pwr_ctrl_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
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
		save_pwr_ctrl(__spm_vcorefs.pwrctrl, x2, x3);
		break;
	}
}

__uint64_t spm_get_pwr_ctrl_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	__uint64_t ret = 0;

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
		ret = load_pwr_ctrl(__spm_vcorefs.pwrctrl, x2);
		break;
	}

	return ret;
}

void spm_dcs_s1_setting(__uint64_t x1, __uint64_t x2)
{
	/* FIXME: */
#if 0
	__uint32_t enable = x1;
	__uint32_t flags = x2;

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

void spm_dummy_read(__uint64_t x1, __uint64_t x2)
{
	/* FIXME: */
#if 0
	__uint32_t rank0_addr = x1;
	__uint32_t rank1_addr = x2;

	mmio_write_32(SPM_PASR_DPD_2, rank0_addr & 0xffffffff);
	mmio_write_32(SPM_PASR_DPD_3, rank1_addr & 0xffffffff);
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

__uint32_t __spmfw_idx;
__uint32_t is_ext_buck;
__uint32_t hwcg_check_enable;
__uint32_t hwcg_check_type;

void spm_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	__uint64_t cmd = x1;

	switch (cmd) {
	case SPM_ARGS_SPMFW_IDX_KICK:
		__spmfw_idx = x2;
		is_ext_buck = x3;
		spm_kick_to_run();
		break;
	case SPM_ARGS_SPMFW_INIT:
#if !defined(MTK_FPGA_EARLY_PORTING)
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
		spm_config_spmfw_base(x2, x3);
		spm_load_pcm_firmware();
		console_uninit();
#endif /* MTK_FPGA_EARLY_PORTING */
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
	case SPM_ARGS_SUSPEND_CALLBACK:
		spm_suspend_callback(x2, x3);
		break;
	case SPM_ARGS_HARDWARE_CG_CHECK:
		hwcg_check_enable = x2;
		hwcg_check_type = x3;
		if (hwcg_check_enable)
			spm_set_hardware_cg_check();
		break;
	default:
		break;
	}
}

void spm_register_init(void)
{
	/* enable register control */
	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);

	/* init power control register */
	/* dram will set this register */
	/* mmio_write_32(SPM_POWER_ON_VAL0, POWER_ON_VAL0_DEF); */
	mmio_write_32(SPM_POWER_ON_VAL1, POWER_ON_VAL1_DEF);
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | REG_EVENT_LOCK_EN_LSB |
			REG_SPM_SRAM_ISOINT_B_LSB | RG_AHBMIF_APBEN_LSB | REG_MD32_APB_INTERNAL_EN_LSB);
	/* initial SPM CLK control register */
	mmio_write_32(SPM_CLK_CON, mmio_read_32(SPM_CLK_CON) | (0x3 << 2) |
			REG_SPM_LOCK_INFRA_DCM_LSB | (1 << 6) |
			REG_CLKSQ1_SEL_CTRL_LSB | REG_SRCCLKEN0_EN_LSB |
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

	mmio_write_32(DDR_EN_DBC_CON0, 0x154 | (0x154 << 10) | (0x154 << 20));
	mmio_write_32(DDR_EN_DBC_CON1, 0x154 | REG_CONN_DDR_EN_DBC_EN_LSB | REG_MD_DDR_EN_0_DBC_EN_LSB);

	/* Configure ARMPLL Control Mode */
	mmio_write_32(ARMPLL_CLK_SEL, 0x0);

	/* Init for SPM Resource ACK */
	mmio_write_32(SPM_RESOURCE_ACK_CON0, 0xE0820820);
	mmio_write_32(SPM_RESOURCE_ACK_CON1, 0xFFC24820);
	mmio_write_32(SPM_RESOURCE_ACK_CON2, 0x1F1F1F1F);
	mmio_write_32(SPM_RESOURCE_ACK_CON3, 0x0000001F);

	/* SPM_DVFS_LEVEL */
	mmio_write_32(SPM_DVFS_LEVEL, 0x08001000);

	/* MD polling ACK setting */
	mmio_write_32(SPM_SRC_MASK, mmio_read_32(SPM_SRC_MASK) | REG_MD_SRCCLKENA_0_MASK_B_LSB);
}

void spm_config_spmfw_base(uint64_t addr, uint64_t size)
{
	spm_base_addr = addr;
	spm_base_size = size;

	INFO("#@# %s(%d) spm_base_addr 0x%lx, spm_base_size 0x%lx\n", __func__, __LINE__,
			spm_base_addr, spm_base_size);

	mmap_add_dynamic_region((addr & ~(PAGE_SIZE_MASK)),
			(addr & ~(PAGE_SIZE_MASK)),
			ALIGN(size, (PAGE_SIZE_MASK)),
			MT_MEMORY | MT_RW | MT_SECURE);
}

#define GPIO_BANK		(GPIO_BASE + 0x6F0)
#define TRAP_UFS_FIRST		(1U << 11) /* bit 11, 1: UFS, 0: eMMC */

void spm_boot_init(void)
{
	hwcg_check_enable = 0;
	hwcg_check_type = HWCG_CHECK_RESOURCE_ORIENTED;
	spm_lock_init();
#if !defined(MTK_FPGA_EARLY_PORTING)
	spm_register_init();
	spm_set_sysclk_settle();
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
#endif /* MTK_FPGA_EARLY_PORTING */

	/* for ufs, emmc storage type workaround */
	if ((mmio_read_32(GPIO_BANK) & TRAP_UFS_FIRST) != 0) {
		/* eMMC booting, mask ufs*/
		spm_suspend_storage_mask_set();
		spm_sodi_storage_mask_set();
		spm_dpidle_storage_mask_set();
	}
}

void spm_legacy_sleep_wfi(__uint64_t x1, __uint64_t x2, __uint64_t x3)
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

	plat_cci_disable();
	disable_scu(mpidr);

	wfi();

	/* Enable coherency if this cluster was off */
	enable_scu(mpidr);
	plat_cci_enable();
}

void spm_set_bootaddr(unsigned long bootaddr)
{
	/* initialize core4~7 boot entry address */
	mmio_write_32(SW2SPM_MAILBOX_3, bootaddr);
}
