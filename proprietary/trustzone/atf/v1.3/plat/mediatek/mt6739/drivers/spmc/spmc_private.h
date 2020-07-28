#ifndef _SPMC_PRIVATE_H_
#define _SPMC_PRIVATE_H_

/*
 * per_cpu helper
 */
struct per_cpu_reg {
	int cluster_addr;
	int cpu_stride;
};

#define per_cpu(cluster, cpu, reg) (reg[cluster].cluster_addr + (cpu << reg[cluster].cpu_stride))
#define per_cluster(cluster, reg) (reg[cluster].cluster_addr)

//--
/* APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN_1       (INFRACFG_AO_BASE + 0x250)
#define INFRA_TOPAXI_PROTECTEN_STA1_1  (INFRACFG_AO_BASE + 0x258)
#define INFRA_TOPAXI_PROTECTEN_1_SET   (INFRACFG_AO_BASE + 0x2A8)
#define INFRA_TOPAXI_PROTECTEN_1_CLR   (INFRACFG_AO_BASE + 0x2AC)

/*
 * MCU configuration registers
 */
#include <mcucfg.h>

#define MCUCFG_MP0_AXI_CONFIG                   MP0_AXI_CONFIG
/* bit-fields of MCUCFG_MP?_AXI_CONFIG */
#define MCUCFG_AXI_CONFIG_BROADCASTINNER        (1 << 0)
#define MCUCFG_AXI_CONFIG_BROADCASTOUTER        (1 << 1)
#define MCUCFG_AXI_CONFIG_BROADCASTCACHEMAINT   (1 << 2)
#define MCUCFG_AXI_CONFIG_SYSBARDISABLE         (1 << 3)
#define MCUCFG_AXI_CONFIG_ACINACTM              (1 << 4)
#define MCUCFG_AXI_CONFIG_AINACTS               (1 << 5)

/* per_cpu registers for MCUCFG_MP?_AXI_CONFIG */
static const struct per_cpu_reg MCUCFG_SCUCTRL[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_AXI_CONFIG },
};

#define MCUCFG_MP0_MISC_CONFIG2         MP0_MISC_CONFIG2
#define MCUCFG_MP0_MISC_CONFIG3         MP0_MISC_CONFIG3

/* per_cpu registers for MCUCFG_MP0_MISC_CONFIG? */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_MISC_CONFIG2, .cpu_stride = 3 },
};

/* per_cpu registers for MCUCFG_MP?_MISC_CONFIG3 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_MISC_CONFIG3 },
};


/*
 * SPM CPU power domain control registers
 */
#include <spm.h>

/* per_cpu registers for SPM_MP0_CPU?_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWRCTRL[] = {
	[0] = { .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2 },
};

/* the bit-fields of SPM_MP0_CPU?_PWR_CON */
#define SPM_CPU_PWR_RST_B            (0x1 << 0)
#define SPM_CPU_PWR_ISO              (0x1 << 1)
#define SPM_CPU_PWR_ON               (0x1 << 2)
#define SPM_CPU_PWR_ON_2ND           (0x1 << 3)
#define SPM_CPU_PWR_CLK_DIS          (0x1 << 4)
#define SPM_CPU_SRAM_CKISO           (0x1 << 5)
#define SPM_CPU_SRAM_ISOINT_B        (0x1 << 6)
#define SPM_CPU_SRAM_PD_SLPB_CLAMP   (0x1 << 7)
#define SPM_CPU_SRAM_PDN             (0x1 << 8)
#define SPM_CPU_SRAM_SLEEP_B         (0x1 << 12)
#define SPM_CPU_SRAM_PDN_ACK         (0x1 << 24)
#define SPM_CPU_SRAM_SLEEP_B_ACK     (0x1 << 28)

/* per_cluster registers for SPM_MP?_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWRCTRL[] = {
	[0] = { .cluster_addr = SPM_MP0_CPUTOP_PWR_CON },
};

/*
 * SPM status registers
 */

/* the bit-fields of SPM_PWR_STATUS[_2ND] */
#define SPM_PWR_STATUS_MD             (0x1 << 0)
#define SPM_PWR_STATUS_CONN           (0x1 << 1)
#define SPM_PWR_STATUS_DDRPHY         (0x1 << 2)
#define SPM_PWR_STATUS_DISP           (0x1 << 3)
#define SPM_PWR_STATUS_MFG            (0x1 << 4)
#define SPM_PWR_STATUS_ISP            (0x1 << 5)
#define SPM_PWR_STATUS_INFRA          (0x1 << 6)
#define SPM_PWR_STATUS_VCODEC         (0x1 << 7)

#define SPM_PWR_STATUS_MP0_CPUTOP     (0x1 << 8)
#define SPM_PWR_STATUS_MP0_CPU0       (0x1 << 9)
#define SPM_PWR_STATUS_MP0_CPU1       (0x1 << 10)
#define SPM_PWR_STATUS_MP0_CPU2       (0x1 << 11)
#define SPM_PWR_STATUS_MP0_CPU3       (0x1 << 12)

#define SPM_PWR_STATUS_MCUSYS         (0x1 << 14)

#define SPM_PWR_STATUS_MP1_CPUTOP     (0x1 << 15)
#define SPM_PWR_STATUS_MP1_CPU0       (0x1 << 16)
#define SPM_PWR_STATUS_MP1_CPU1       (0x1 << 17)
#define SPM_PWR_STATUS_MP1_CPU2       (0x1 << 18)
#define SPM_PWR_STATUS_MP1_CPU3       (0x1 << 19)

#define SPM_PWR_STATUS_VEN            (0x1 << 21)
#define SPM_PWR_STATUS_MFG_ASYNC      (0x1 << 23)
#define SPM_PWR_STATUS_AUDIO          (0x1 << 24)
#define SPM_PWR_STATUS_C2K            (0x1 << 28)
#define SPM_PWR_STATUS_MD_INFRA       (0x1 << 29)
#define SPM_PWR_STATUS_MFG_CORE0      (0x1 << 31)


/*
 * SPM configuration registers
 */
#define SPM_POWERON_CONFIG_EN        SPM_POWERON_CONFIG_SET

/*
 * INFRACFG_AO registers bit-fields
 */

/* bit-fields of INFRA_TOPAXI_PROTECTEN */
#define MP0_CPUTOP_PROT_BIT_2ND_MASK      ((0x1 << 12)|(0x1 << 13))
#define MP0_CPUTOP_PROT_BIT_ACK_2ND_MASK  ((0x1 << 12)|(0x1 << 13))

#endif
