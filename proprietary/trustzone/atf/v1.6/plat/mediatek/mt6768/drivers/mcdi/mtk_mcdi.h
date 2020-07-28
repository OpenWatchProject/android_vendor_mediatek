#ifndef __MTK_MCDI_H__
#define __MTK_MCDI_H__

#include <mmio.h>
#include <mcucfg.h>
#include <platform_def.h>
#include <mt_spm_reg.h>

#define MCDI_CPC_MODE

#define MCDI_SRAM_REG(r) (MTK_MCDI_SRAM_BASE + (r))

enum {
	MCDI_SMC_EVENT_ASYNC_WAKEUP_EN = 0,
	MCDI_SMC_EVENT_DUMP_TRACE_DATA,
	MCDI_SMC_EVENT_LAST_CORE_REQ,
	MCDI_SMC_EVENT_LAST_CORE_CLR,
	MCDI_SMC_EVENT_GIC_DPG_SET,
	MCDI_SMC_EVENT_CPC_CONFIG,

	NF_MCDI_SMC_EVENT
};

enum {
	SEM_MCDI = 0x0,
	SEM_MAX = 0x10,
};

enum {
	MCDI_CPC_CFG_PROF,
	MCDI_CPC_CFG_AUTO_OFF,
	MCDI_CPC_CFG_AUTO_OFF_THRES,
	MCDI_CPC_CFG_CNT_CLR,

	NF_MCDI_CPC_CFG,
};

#define NF_CPU      8
#define TIMEOUT_US  5

#define SEM_ID_HOST_CPU(id)	(2 * (id) + 1)
#define SEM_ID_MCDI		SEM_ID_HOST_CPU(SEM_MCDI)

#define CPC_TRACE_SIZE		(0x20)
#define CPC_TRACE_ID_NUM	(10)
#define MCDI_SRAM_CPC_ST(id)	(MCDI_SRAM_REG(0x28) + (id) * CPC_TRACE_SIZE)

#define CORE_PWR_STA_MASK	(0x79E00)

#define SET_LAST_CORE_MASK_CNT		MCDI_SRAM_REG(0x7F0)
#define CLR_LAST_CORE_MASK_CNT		MCDI_SRAM_REG(0x7F4)
#define CPC_CLUSTER_CNT_BACKUP		MCDI_SRAM_REG(0x7F8)

#define NF_MCDI_MBOX                            20
#define MCDI_MBOX_CLUSTER_0_CAN_POWER_OFF       0
#define MCDI_MBOX_CLUSTER_1_CAN_POWER_OFF       1
#define MCDI_MBOX_BUCK_POWER_OFF_MASK           2
#define MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE     3
#define MCDI_MBOX_CLUSTER_1_ATF_ACTION_DONE     4
#define MCDI_MBOX_BOOTADDR                      5
#define MCDI_MBOX_PAUSE_ACTION                  6
#define MCDI_MBOX_AVAIL_CPU_MASK                7
#define MCDI_MBOX_CPU_CLUSTER_PWR_STAT          8
#define MCDI_MBOX_ACTION_STAT                   9
#define MCDI_MBOX_CLUSTER_0_CNT                 10
#define MCDI_MBOX_CLUSTER_1_CNT                 11
#define MCDI_MBOX_CPU_ISOLATION_MASK            12
#define MCDI_MBOX_PAUSE_ACK                     13
#define MCDI_MBOX_PENDING_ON_EVENT              14
#define MCDI_MBOX_PROF_CMD                      15
#define MCDI_MBOX_PROF_CLUSTER                  16
#define MCDI_MBOX_AP_READY                      17

void cpc_disable_all_wakeup_event(void);
void cpc_set_mcusys_off_en(void);
void cpc_clr_mcusys_off_en(void);
void mcdi_handler(unsigned long long x1, unsigned long long x2, unsigned long long x3);
unsigned long long get_isolation_status(void);
void cpc_core_on_hint(int cpu);

/* mcdi interface before/after cpu/cluster/mcusys on/off */
void plat_mcdi_before_cpu_pwrdwn(int cpu, int cluster, unsigned int afflvl,
				unsigned int cpuoff_afflvl);
void plat_mcdi_after_cpu_pwron(int cpu, int cluster);
void plat_mcdi_before_cluster_pwrdwn(void);
void plat_mcdi_after_cluster_pwron(void);
#endif /* __MTK_MCDI_H__ */

