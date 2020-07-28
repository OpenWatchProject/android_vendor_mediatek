#ifndef _MTSPMC_PRIVATE_H_
#define _MTSPMC_PRIVATE_H_

unsigned long read_cpuectlr(void);
void write_cpuectlr(unsigned long cpuectlr);

unsigned long read_cpupwrctlr_el1(void);
void write_cpupwrctlr_el1(unsigned long cpuectlr);

/*
 * per_cpu/cluster helper
 */
struct per_cpu_reg {
	int cluster_addr;
	int cpu_stride;
};

#define per_cpu(cluster, cpu, reg) (reg[cluster].cluster_addr + \
				   (cpu << reg[cluster].cpu_stride))

#define per_cluster(cluster, reg)  (reg[cluster].cluster_addr)

/* === SPMC related registers */
#define SPM_POWERON_CONFIG_EN              (SPM_BASE + 0x000)
/* bit-fields of SPM_POWERON_CONFIG_EN */
#define PROJECT_CODE                       (0xb16 << 16)
#define BCLK_CG_EN                         (1 << 0)

#define SPM_PWR_STATUS			(SPM_BASE + 0x160)
#define SPM_PWR_STATUS_2ND		(SPM_BASE + 0x164)
#define SPM_CPU_PWR_STATUS		(SPM_BASE + 0x168)

/* bit-fields of SPM_PWR_STATUS */
#define MD                                 (1 << 0)
#define CONN                               (1 << 1)
#define DDRPHY                             (1 << 2)
#define DISP                               (1 << 3)
#define MFG                                (1 << 4)
#define ISP                                (1 << 5)
#define INFRA                              (1 << 6)
#define VDEC                               (1 << 7)
#define MP0_CPUTOP                         (1 << 8)
#define MP0_CPU0                           (1 << 9)
#define MP0_CPU1                           (1 << 10)
#define MP0_CPU2                           (1 << 11)
#define MP0_CPU3                           (1 << 12)
#define MCUSYS                             (1 << 14)
#define MP0_CPU4                           (1 << 15)
#define MP0_CPU5                           (1 << 16)
#define MP0_CPU6                           (1 << 17)
#define MP0_CPU7                           (1 << 18)
#define VEN                                (1 << 21)

/* === SPMC related registers */
#define SPM_MCUSYS_PWR_CON                    (SPM_BASE + 0x200)
#define SPM_MP0_CPUTOP_PWR_CON                (SPM_BASE + 0x204)
#define SPM_MP0_CPU0_PWR_CON                  (SPM_BASE + 0x208)
#define SPM_MP0_CPU1_PWR_CON                  (SPM_BASE + 0x20c)
#define SPM_MP0_CPU2_PWR_CON                  (SPM_BASE + 0x210)
#define SPM_MP0_CPU3_PWR_CON                  (SPM_BASE + 0x214)
#define SPM_MP0_CPU4_PWR_CON                  (SPM_BASE + 0x218)
#define SPM_MP0_CPU5_PWR_CON                  (SPM_BASE + 0x21c)
#define SPM_MP0_CPU6_PWR_CON                  (SPM_BASE + 0x220)
#define SPM_MP0_CPU7_PWR_CON                  (SPM_BASE + 0x224)

/* bit fields of SPM_*_PWR_CON */
#define PWR_ON_ACK                            (1 << 31)
#define VPROC_EXT_OFF                         (1 << 7)
#define DORMANT_EN                            (1 << 6)
#define RESETPWRON_CONFIG                     (1 << 5)
#define PWR_CLK_DIS                           (1 << 4)
#define PWR_ON                                (1 << 2)
#define PWR_RST_B                             (1 << 0)

/**** per_cpu registers for SPM_MP0_CPU?_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2 },
};

/**** per_cluster registers for SPM_MP0_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPUTOP_PWR_CON, },
};


/* === MCUCFG related registers */
/* aa64naa32 */
#define MCUCFG_MP0_CLUSTER_CFG5               (MCUCFG_BASE + 0xc8e4)
/* reset vectors */
#define MCUCFG_MP0_CLUSTER_CFG8               (MCUCFG_BASE + 0xc900)
#define MCUCFG_MP0_CLUSTER_CFG10              (MCUCFG_BASE + 0xc908)
#define MCUCFG_MP0_CLUSTER_CFG12              (MCUCFG_BASE + 0xc910)
#define MCUCFG_MP0_CLUSTER_CFG14              (MCUCFG_BASE + 0xc918)
#define MCUCFG_MP0_CLUSTER_CFG16              (MCUCFG_BASE + 0xc920)
#define MCUCFG_MP0_CLUSTER_CFG18              (MCUCFG_BASE + 0xc928)
#define MCUCFG_MP0_CLUSTER_CFG20              (MCUCFG_BASE + 0xc930)
#define MCUCFG_MP0_CLUSTER_CFG22              (MCUCFG_BASE + 0xc938)

/**** per_cpu registers for MCUCFG_MP0_CLUSTER_CFG? */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_CLUSTER_CFG8, .cpu_stride = 3 },
};

/**** per_cpu registers for MCUCFG_MP0_CLUSTER_CFG5 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_CLUSTER_CFG5 },
};

/* === CPC control */
#define MCUCFG_CPC_FLOW_CTRL_CFG              (MCUCFG_BASE + 0xa814)
#define MCUCFG_CPC_SPMC_PWR_STATUS            (MCUCFG_BASE + 0xa840)

/* bit fields of CPC_FLOW_CTRL_CFG */
#define CPC_CTRL_ENABLE                       (1 << 16)
#define SSPM_ALL_PWR_CTRL_EN                  (1 << 13) /* for cpu-hotplug */
#define GIC_WAKEUP_IGNORE(cpu)                (1 << (21 + cpu))

/* bit fields of CPC_SPMC_PWR_STATUS */
#define CORE_SPMC_PWR_ON_ACK                  (0xfff << 0)

/* === APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN                (INFRACFG_AO_BASE + 0x0220)
#define INFRA_TOPAXI_PROTECTEN_STA0           (INFRACFG_AO_BASE + 0x0224)
#define INFRA_TOPAXI_PROTECTEN_STA1           (INFRACFG_AO_BASE + 0x0228)
#define INFRA_TOPAXI_PROTECTEN_SET            (INFRACFG_AO_BASE + 0x02a0)
#define INFRA_TOPAXI_PROTECTEN_CLR            (INFRACFG_AO_BASE + 0x02a4)
#define INFRA_TOPAXI_PROTECTEN_1              (INFRACFG_AO_BASE + 0x0250)
#define INFRA_TOPAXI_PROTECTEN_STA0_1         (INFRACFG_AO_BASE + 0x0254)
#define INFRA_TOPAXI_PROTECTEN_STA1_1         (INFRACFG_AO_BASE + 0x0258)
#define INFRA_TOPAXI_PROTECTEN_1_SET          (INFRACFG_AO_BASE + 0x02a8)
#define INFRA_TOPAXI_PROTECTEN_1_CLR          (INFRACFG_AO_BASE + 0x02ac)

/* bit fields of INFRA_TOPAXI_PROTECTEN */
#define MP0_SPMC_PROT_STEP1_0_MASK       ((1 << 12))
#define MP0_SPMC_PROT_STEP1_1_MASK       ((1 << 26) | (1 << 12))


/* === SPARK */
#define VOLTAGE_04                      0x40
#define VOLTAGE_05                      0x60

#define PTP3_CPU0_SPMC_SW_CFG           (MCUCFG_BASE + 0x200)
#define CPU0_ILDO_CONTROL5              (MCUCFG_BASE + 0x334)
#define CPU0_ILDO_CONTROL8              (MCUCFG_BASE + 0x340)

/* bit fields of CPU0_ILDO_CONTROL5 */
#define ILDO_RET_VOSEL                  (0xff << 0)

/* bit fields of PTP3_CPU_SPMC_SW_CFG */
#define SW_SPARK_EN                     (1 << 0)

/* bit fields of CPU0_ILDO_CONTROL8 */
#define ILDO_BYPASS_B                   (1 << 0)

static const struct per_cpu_reg MCUCFG_SPARK[] = {
	[0] = { .cluster_addr = PTP3_CPU0_SPMC_SW_CFG, .cpu_stride = 11 },
};

static const struct per_cpu_reg ILDO_CONTROL5[] = {
	[0] = { .cluster_addr = CPU0_ILDO_CONTROL5, .cpu_stride = 11 },
};

static const struct per_cpu_reg ILDO_CONTROL8[] = {
	[0] = { .cluster_addr = CPU0_ILDO_CONTROL8, .cpu_stride = 11 },
};

#endif
