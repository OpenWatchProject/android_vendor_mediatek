#ifndef _MTSPMC_PRIVATE_H_
#define _MTSPMC_PRIVATE_H_

uint64_t read_cpuectlr(void);
void write_cpuectlr(uint64_t);

/*
 * per_cpu/cluster helper
 */
struct per_cpu_reg {
	int cluster_addr;
	int cpu_stride;
};

#define per_cpu(cluster, cpu, reg) (reg[cluster].cluster_addr + (cpu << reg[cluster].cpu_stride))
#define per_cluster(cluster, reg) (reg[cluster].cluster_addr)

/* SPMC related registers */
#define SPM_POWERON_CONFIG_EN              (SPM_BASE + 0x000)
/* bit-fields of SPM_POWERON_CONFIG_EN */
#define BCLK_CG_EN                         (1 << 0)
#define MD_BCLK_CG_EN                      (1 << 1)
#define PROJECT_CODE                       (0xb16 << 16)

#define SPM_PWR_STATUS                     (SPM_BASE + 0x180)
#define SPM_PWR_STATUS_2ND                 (SPM_BASE + 0x184)
/* bit-fields of SPM_PWR_STATUS */
#define MD                      (1 << 0)
#define CONN                    (1 << 1)
#define DDRPHY                  (1 << 2)
#define DISP                    (1 << 3)
#define MFG                     (1 << 4)
#define ISP                     (1 << 5)
#define INFRA                   (1 << 6)
#define VDEC                    (1 << 7)
#define MP0_CPUTOP              (1 << 8)
#define MP0_CPU0                (1 << 9)
#define MP0_CPU1                (1 << 10)
#define MP0_CPU2                (1 << 11)
#define MP0_CPU3                (1 << 12)
#define MCUSYS                  (1 << 14)
#define MP1_CPUTOP              (1 << 15)
#define MP1_CPU0                (1 << 16)
#define MP1_CPU1                (1 << 17)
#define MP1_CPU2                (1 << 18)
#define MP1_CPU3                (1 << 19)
#define VEN                     (1 << 21)
#define MFG_ASYNC               (1 << 23)
#define AUDIO                   (1 << 24)
#define C2K                     (1 << 28)
#define MD_INFRA                (1 << 29)

#define SPM_MP0_CPUTOP_PWR_CON             (SPM_BASE + 0x204)
#define SPM_MP0_CPU0_PWR_CON               (SPM_BASE + 0x208)
#define SPM_MP0_CPU1_PWR_CON               (SPM_BASE + 0x20C)
#define SPM_MP0_CPU2_PWR_CON               (SPM_BASE + 0x210)
#define SPM_MP0_CPU3_PWR_CON               (SPM_BASE + 0x214)
#define SPM_MP1_CPUTOP_PWR_CON             (SPM_BASE + 0x218)
#define SPM_MP1_CPU0_PWR_CON               (SPM_BASE + 0x21C)
#define SPM_MP1_CPU1_PWR_CON               (SPM_BASE + 0x220)
#define SPM_MP1_CPU2_PWR_CON               (SPM_BASE + 0x224)
#define SPM_MP1_CPU3_PWR_CON               (SPM_BASE + 0x228)
#define SPM_MP0_CPUTOP_L2_PDN              (SPM_BASE + 0x240)
#define SPM_MP0_CPUTOP_L2_SLEEP_B          (SPM_BASE + 0x244)
#define SPM_MP0_CPU0_L1_PDN                (SPM_BASE + 0x248)
#define SPM_MP0_CPU1_L1_PDN                (SPM_BASE + 0x24C)
#define SPM_MP0_CPU2_L1_PDN                (SPM_BASE + 0x250)
#define SPM_MP0_CPU3_L1_PDN                (SPM_BASE + 0x254)
#define SPM_MP1_CPUTOP_L2_PDN              (SPM_BASE + 0x258)
#define SPM_MP1_CPUTOP_L2_SLEEP_B          (SPM_BASE + 0x25C)
#define SPM_MP1_CPU0_L1_PDN                (SPM_BASE + 0x260)
#define SPM_MP1_CPU1_L1_PDN                (SPM_BASE + 0x264)
#define SPM_MP1_CPU2_L1_PDN                (SPM_BASE + 0x268)
#define SPM_MP1_CPU3_L1_PDN                (SPM_BASE + 0x26C)

#define SPM_CPU_EXT_BUCK_ISO               (SPM_BASE + 0x290)
#define SPM_DUMMY1_PWR_CON                 (SPM_BASE + 0x2b0)
/* bit-fields of SPM_CPU_EXT_BUCK_ISO */
#define MP0_EXT_BUCK_ISO                   (1 << 0)
#define MP1_EXT_BUCK_ISO                   (1 << 1)
#define MP_EXT_BUCK_ISO                    (1 << 2)
/* per_cluster registers for MP0/1 BUCK ISO */
static const struct per_cpu_reg SPM_CLUSTER_BUCK[] = {
	[0] = { .cluster_addr = SPM_CPU_EXT_BUCK_ISO },
	[1] = { .cluster_addr = SPM_DUMMY1_PWR_CON },
};


#define SPM_BYPASS_SPMC                    (SPM_BASE + 0x2b4)
#define SPM_SPMC_DORMANT_ENABLE            (SPM_BASE + 0x2b8)

/* bit-fields of SPM_*_PWR_CON */
#define PWR_RST_B                  (1 << 0)
#define PWR_ISO                    (1 << 1)
#define PWR_ON                     (1 << 2)
#define PWR_ON_2ND                 (1 << 3)
#define PWR_CLK_DIS                (1 << 4)
#define SRAM_CKISO                 (1 << 5)
#define SRAM_ISOINT_B              (1 << 6)
#define SRAM_PD_SLPB_CLAMP         (1 << 7)
#define SRAM_PDN                   (1 << 8)
#define SRAM_SLEEP_B               (1 << 12)
#define SRAM_PDN_ACK               (1 << 24)
#define SRAM_SLEEP_B_ACK           (1 << 28)
#define PWR_ACK                    (1 << 30)
#define PWR_2ND_ACK                (1 << 31)

/* per_cpu registers for SPM_MP?_CPU?_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2 },
	[1] = { .cluster_addr = SPM_MP1_CPU0_PWR_CON, .cpu_stride = 2 },
};

/* per_cluster registers for SPM_MP?_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPUTOP_PWR_CON },
	[1] = { .cluster_addr = SPM_MP1_CPUTOP_PWR_CON },
};

/* APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN_1          (INFRACFG_AO_BASE + 0x250)
#define INFRA_TOPAXI_PROTECTEN_STA1_1     (INFRACFG_AO_BASE + 0x258)
#define INFRA_TOPAXI_PROTECTEN_1_SET      (INFRACFG_AO_BASE + 0x2A8)
#define INFRA_TOPAXI_PROTECTEN_1_CLR      (INFRACFG_AO_BASE + 0x2AC)

/* bit-fields of INFRA_TOPAXI_PROTECTEN_1_SET/CLR */
#define MP0_CPUTOP_PROT_BIT_MASK      ((1 << 10)|(1 << 12)|(1 << 13)|(1 << 26))
#define MP1_CPUTOP_PROT_BIT_MASK      ((1 << 11)|(1 << 14)|(1 << 15)|(1 << 27))

/*
 * MCU configuration registers
 */
#define MCUCFG_MP0_AXI_CONFIG                   MP0_AXI_CONFIG
#define MCUCFG_MP1_AXI_CONFIG                   MP1_AXI_CONFIG
/* bit-fields of MCUCFG_MP?_AXI_CONFIG */
#define ACINACTM              (1 << 4)
#define AINACTS               (1 << 5)
#define ACP_RMW_EN            (1 << 8)

/* per_cpu registers for MCUCFG_MP?_AXI_CONFIG */
static const struct per_cpu_reg MCUCFG_SCUCTRL[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_AXI_CONFIG },
	[1] = { .cluster_addr = MCUCFG_MP1_AXI_CONFIG },
};

#define MCUCFG_MP0_MISC_CONFIG2         MP0_MISC_CONFIG2
#define MCUCFG_MP0_MISC_CONFIG3         MP0_MISC_CONFIG3
#define MCUCFG_MP1_MISC_CONFIG2         MP1_MISC_CONFIG2
#define MCUCFG_MP1_MISC_CONFIG3         MP1_MISC_CONFIG3

#define MCUCFG_CPUSYS0_SPARKVRETCNTRL         (MCUCFG_BASE + 0x1c00)
#define MCUCFG_CPUSYS1_SPARKVRETCNTRL         (MCUCFG_BASE + 0x3c00)
/* bit-fields of MCUCFG_CPUSYS[01]_SPARKVRETCNTRL */
#define CPU0_SPARK_VRET_CTRL                  (0x3f << 0)
#define CPU1_SPARK_VRET_CTRL                  (0x3f << 8)
#define CPU2_SPARK_VRET_CTRL                  (0x3f << 16)
#define CPU3_SPARK_VRET_CTRL                  (0x3f << 24)

/* SPARK control registers*/
#define MCUCFG_MP0_SPMC                       (MCUCFG_BASE + 0x0788)
#define MCUCFG_MP1_SPMC                       (MCUCFG_BASE + 0x078c)
#define MCUCFG_CPUSYS0_CPU0_SPMC_CTL          (MCUCFG_BASE + 0x1c30)
#define MCUCFG_CPUSYS0_CPU1_SPMC_CTL          (MCUCFG_BASE + 0x1c34)
#define MCUCFG_CPUSYS0_CPU2_SPMC_CTL          (MCUCFG_BASE + 0x1c38)
#define MCUCFG_CPUSYS0_CPU3_SPMC_CTL          (MCUCFG_BASE + 0x1c3c)
#define MCUCFG_CPUSYS1_CPU0_SPMC_CTL          (MCUCFG_BASE + 0x3c30)
#define MCUCFG_CPUSYS1_CPU1_SPMC_CTL          (MCUCFG_BASE + 0x3c34)
#define MCUCFG_CPUSYS1_CPU2_SPMC_CTL          (MCUCFG_BASE + 0x3c38)
#define MCUCFG_CPUSYS1_CPU3_SPMC_CTL          (MCUCFG_BASE + 0x3c3c)
/* bit-fields of MCUCFG_CPUSYS0_CPU?_SPMC_CTL */
#define SW_SPARK_EN                           (1 << 0)
#define SW_NO_WAIT_Q                          (1 << 1)
#define SW_COQ_DIS                            (1 << 25)

/* per_cpu registers for MCUCFG_MP?_MISC_CONFIG2 */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_MISC_CONFIG2, .cpu_stride = 3 },
	[1] = { .cluster_addr = MCUCFG_MP1_MISC_CONFIG2, .cpu_stride = 3 },
};

/* per_cpu registers for MCUCFG_MP?_MISC_CONFIG3 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_MISC_CONFIG3 },
	[1] = { .cluster_addr = MCUCFG_MP1_MISC_CONFIG3 },
};

/* per_cpu registers for SPARK */
static const struct per_cpu_reg MCUCFG_SPARK[] = {
	[0] = { .cluster_addr = MCUCFG_CPUSYS0_CPU0_SPMC_CTL, .cpu_stride = 2 },
	[1] = { .cluster_addr = MCUCFG_CPUSYS1_CPU0_SPMC_CTL, .cpu_stride = 2 },
};

/* per_cpu registers for SPARK2LDO */
static const struct per_cpu_reg MCUCFG_SPARK2LDO[] = {
	[0] = { .cluster_addr = MCUCFG_CPUSYS0_SPARKVRETCNTRL },
	[1] = { .cluster_addr = MCUCFG_CPUSYS1_SPARKVRETCNTRL },
};

/* per_cluster registers for MCUCFG_MP?_SPMC */
static const struct per_cpu_reg MCUCFG_CLUSTER_SPMC[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_SPMC },
	[1] = { .cluster_addr = MCUCFG_MP1_SPMC },
};

#endif
