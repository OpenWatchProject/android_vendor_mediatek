#include <mmio.h>
#include <string.h>
#include <delay_timer.h>
#include <mtspmc.h>
#include <debug.h>
#include <mtk_mcdi.h>

#define CPUSYS_PROT_MODE 0 /* 0: HW mode, 1: SW mode */

static void mcdi_last_core_prot_check(int off)
{
#if CPUSYS_PROT_MODE
	static unsigned int clr_cnt;
	static unsigned int set_cnt;
	unsigned int mask, sta, retry;

	mask = mmio_read_32(CPC_MCUSYS_PWR_ON_MASK) & 0xFF;

	if (!off && mask) {

		mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, (0x3 << 8));
		mmio_write_32(CLR_LAST_CORE_MASK_CNT, ++clr_cnt);

	} else if (off) {

		if (mask) {

			mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, (1 << 9) | 0xFF);
			sta = mmio_read_32(CPC_MCUSYS_MP_LAST_CORE_RESP);

			if (((sta >> 16) & 0x3) == 1)
				return;
		}

		retry = 0;

		while (retry++ < 50) {

			mmio_write_32(CPC_MCUSYS_LAST_CORE_REQ, 1);

			udelay(1);

			sta = mmio_read_32(CPC_MCUSYS_MP_LAST_CORE_RESP);

			if (((sta >> 16) & 0x3) == 1) {
				mmio_write_32(SET_LAST_CORE_MASK_CNT, ++set_cnt);
				break;
			}
		}
	}
#endif
}

static void mcdi_ildo_lock(void)
{
#if 0 /* TODO: wait SSPM HW sem. ready */
	unsigned int val, cnt;

	val = (mmio_read_32(SSPM_HW_SEM) >> SEM_ID_MCDI) & 0x1;
	cnt = TIMEOUT_US;

	if (val == 0) {
		while (cnt > 0) {
			mmio_write_32(SSPM_HW_SEM, (1 << SEM_ID_MCDI));
			val = (mmio_read_32(SSPM_HW_SEM) >> SEM_ID_MCDI) & 0x1;

			if (val == 1)
				return;
			udelay(1);
			/* cnt--; */
		}
	}
	ERROR("[MCDI] can NOT get SSPM HW SEMAPHORE!\n");
#endif
}

static void mcdi_ildo_unlock(void)
{
#if 0 /* TODO: wait SSPM HW sem. ready */
	unsigned int val, cnt;

	val = (mmio_read_32(SSPM_HW_SEM) >> SEM_ID_MCDI) & 0x1;
	cnt = TIMEOUT_US;

	if (val == 1) {
		while (cnt > 0) {
			mmio_write_32(SSPM_HW_SEM, (1 << SEM_ID_MCDI));
			val = (mmio_read_32(SSPM_HW_SEM) >> SEM_ID_MCDI) & 0x1;

			if (val == 0)
				return;
			udelay(1);
			/* cnt--; */
		}
	} else {
		INFO("[MCDI] has NOT acquired SPM HW SEMAPHORE\n");
	}
	ERROR("[MCDI] release SPM HW SEMAPHORE fail!\n");
#endif
}

void cpc_disable_all_wakeup_event(void)
{
#if 0
	int i;

	for (i = 0; i < 8; i++)
		mcucfg_disable_gic_wakeup(0, i);
#endif
}

static void cpc_backup_counter(void)
{
	int backup_cnt;
	int curr_cnt;

	/* Single Cluster */
	backup_cnt = mmio_read_32(CPC_CLUSTER_CNT_BACKUP);
	curr_cnt = mmio_read_32(CPC_MCUSYS_CLUSTER_COUNTER);

	/* Get off count if dormant count is 0 */
	if ((curr_cnt & 0x7FFF) == 0)
		curr_cnt = (curr_cnt >> 16) & 0x7FFF;
	else
		curr_cnt = curr_cnt & 0x7FFF;

	mmio_write_32(CPC_CLUSTER_CNT_BACKUP, backup_cnt + curr_cnt);
}

void cpc_set_mcusys_off_en(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_CTRL, 1);
	cpc_backup_counter();
}

void cpc_clr_mcusys_off_en(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_CTRL, 0);
}

void cpc_core_on_hint(int cpu)
{
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_SET, 1 << cpu);
}

static void mcdi_dump_cpc_timestamp(void)
{
	unsigned int id;

	for (id = 0; id < CPC_TRACE_ID_NUM; id++) {
		mmio_write_32(CPC_MCUSYS_TRACE_SEL, id);

		memcpy((void *)(uintptr_t)MCDI_SRAM_CPC_ST(id),
				(const void *)(uintptr_t)CPC_MCUSYS_TRACE_DATA,
				CPC_TRACE_SIZE);
	}
}

static void mcdi_cpc_config(unsigned int cfg, unsigned int data)
{
	unsigned int val;
	unsigned int reg = 0;

	switch (cfg) {
	case MCDI_CPC_CFG_PROF:
		reg = CPC_MCUSYS_CPC_DBG_SETTING;
		val = mmio_read_32(reg);
		val = data ? val | 0x1 : val & ~0x1;
		break;
	case MCDI_CPC_CFG_AUTO_OFF:
		reg = CPC_MCUSYS_CPC_FLOW_CTRL_CFG;
		val = mmio_read_32(reg);
		val = data ? val | 0x20 : val & ~0x20;
		break;
	case MCDI_CPC_CFG_AUTO_OFF_THRES:
		reg = CPC_MCUSYS_CPC_OFF_THRES;
		val = data;
		break;
	case MCDI_CPC_CFG_CNT_CLR:
		reg = CPC_MCUSYS_CLUSTER_COUNTER_CLR;
		val = 0x3;
		break;
	default:
		break;
	}

	if (reg)
		mmio_write_32(reg, val);
}

static unsigned long long isolated_status __section("mtk_noncache_dram");

unsigned long long get_isolation_status(void)
{
	return isolated_status;
}

void mcdi_handler(unsigned long long x1, unsigned long long x2, unsigned long long x3)
{
	unsigned long long cmd = x1;

	switch (cmd) {
	case MCDI_SMC_EVENT_DUMP_TRACE_DATA:
		mcdi_dump_cpc_timestamp();
		break;
	case MCDI_SMC_EVENT_LAST_CORE_REQ:
		mmio_write_32(CPC_MCUSYS_LAST_CORE_REQ, (unsigned int)x2);
		break;
	case MCDI_SMC_EVENT_LAST_CORE_CLR:
		mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, (unsigned int)x2);
		break;
	case MCDI_SMC_EVENT_GIC_DPG_SET:
		isolated_status = x2;
		break;
	case MCDI_SMC_EVENT_CPC_CONFIG:
		mcdi_cpc_config((unsigned int)x2, (unsigned int)x3);
		break;
	default:
		break;
	}
}

/* mcdi interface before/after cpu/cluster/mcusys on/off */
void plat_mcdi_before_cpu_pwrdwn(int cpu, int cluster, unsigned int afflvl,
				unsigned int cpuoff_afflvl)
{
	mcdi_last_core_prot_check((afflvl == cpuoff_afflvl) ? 0 : 1);
}
void plat_mcdi_after_cpu_pwron(int cpu, int cluster) {}

void plat_mcdi_before_cluster_pwrdwn(void)
{
	mcdi_ildo_lock();
}

void plat_mcdi_after_cluster_pwron(void)
{
	mcdi_ildo_unlock();
}
