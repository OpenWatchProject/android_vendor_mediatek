/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/smp.h> /* on_each_cpu */
#include <linux/cpumask.h> /* for_each_possible_cpu(cpu) */

#include "interface.h"
#include "cpu_pmu_v2.h"
#include "v8_pmu_hw_v2.h"
#include "met_kernel_symbol.h"


/*******************************************************************************
*				Type Define
*******************************************************************************/
/*
 * Per-CPU PMCR: config reg
 */
#define ARMV8_PMCR_E		(1 << 0)	/* Enable all counters */
#define ARMV8_PMCR_P		(1 << 1)	/* Reset all counters */
#define ARMV8_PMCR_C		(1 << 2)	/* Cycle counter reset */
#define ARMV8_PMCR_D		(1 << 3)	/* CCNT counts every 64th cpu cycle */
#define ARMV8_PMCR_X		(1 << 4)	/* Export to ETM */
#define ARMV8_PMCR_DP		(1 << 5)	/* Disable CCNT if non-invasive debug */
#define	ARMV8_PMCR_N_SHIFT	11		/* Number of counters supported */
#define	ARMV8_PMCR_N_MASK	0x1f
#define	ARMV8_PMCR_MASK		0x3f		/* Mask for writable bits */

/*
 * PMOVSR: counters overflow flag status reg
 */
#define	ARMV8_OVSR_MASK		0xffffffff	/* Mask for writable bits */
#define	ARMV8_OVERFLOWED_MASK	ARMV8_OVSR_MASK


/*******************************************************************************
*				Fuction Pototypes
*******************************************************************************/
static int armv8_pmu_hw_get_event_desc(int event, char *event_desc);
static int armv8_pmu_hw_check_event(struct met_pmu_v2 *pmu, int idx, int event);
static void armv8_pmu_hw_start(struct met_pmu_v2 *pmu, int count);
static void armv8_pmu_hw_stop(int count);
static unsigned int armv8_pmu_hw_polling(struct met_pmu_v2 *pmu, int count, unsigned int *pmu_value);


/*******************************************************************************
*				Globe Variables
*******************************************************************************/
struct pmu_desc_v2 a53_pmu_desc_v2[] = {
	{0x00, "SW_INCR"},
	{0x01, "L1I_CACHE_REFILL"},
	{0x02, "L1I_TLB_REFILL"},
	{0x03, "L1D_CACHE_REFILL"},
	{0x04, "L1D_CACHE"},
	{0x05, "L1D_TLB_REFILL"},
	{0x06, "LD_RETIRED"},
	{0x07, "ST_RETIRED"},
	{0x08, "INST_RETIRED"},
	{0x09, "EXC_TAKEN"},
	{0x0A, "EXC_RETURN"},
	{0x0B, "CID_WRITE_RETIRED"},
	{0x0C, "PC_WRITE_RETIRED"},
	{0x0D, "BR_IMMED_RETIRED"},
	{0x0E, "BR_RETURN_RETIRED"},
	{0x0F, "UNALIGNED_LDST_RETIRED"},

	{0x10, "BR_MIS_PRED"},
	{0x11, "CPU_CYCLES"},
	{0x12, "BR_PRED"},
	{0x13, "MEM_ACCESS"},
	{0x14, "L1I_CACHE"},
	{0x15, "L1D_CACHE_WB"},
	{0x16, "L2D_CACHE"},
	{0x17, "L2D_CACHE_REFILL"},
	{0x18, "L2D_CACHE_WB"},
	{0x19, "BUS_ACCESS"},
	{0x1A, "MEMORY_ERROR"},
	{0x1D, "BUS_CYCLES"},
	{0x1E, "CHAIN"},

	{0x60, "BUS_READ_ACCESS"},
	{0x61, "BUS_WRITE_ACCESS"},

	{0x7A, "BR_INDIRECT_SPEC"},

	{0x86, "IRQ_EXC_TAKEN"},
	{0x87, "FIQ_EXC_TAKEN"},

	{0xC0, "EXT_MEM_REQ"},
	{0xC1, "NO_CACHE_EXT_MEM_REQ"},
	{0xC2, "PREFETCH_LINEFILL"},
	{0xC4, "ENT_READ_ALLOC_MODE"},
	{0xC5, "READ_ALLOC_MODE"},
	{0xC6, "PRE_DECODE_ERROR"},
	{0xC7, "WRITE_STALL"},
	{0xC8, "SCU_SNOOP_DATA_FROM_ANOTHER_CPU"},
	{0xC9, "CONDITIONAL_BRANCH_EXE"},
	{0xCA, "INDIRECT_BRANCH_MISPREDICT"},
	{0xCB, "INDIRECT_BRANCH_MISPREDICT_ADDR"},/*"INDIRECT_BRANCH_MISPREDICT_ADDR_MISSCOMPARE" */
	{0xCC, "COND_BRANCH_MISPREDICT"},

	{0xD0, "L1_INST_CACHE_MEM_ERR"},

	{0xE1, "ICACHE_MISS_STALL"},
	{0xE2, "DPU_IQ_EMPTY"},
	{0xE4, "NOT_FPU_NEON_INTERLOCK"},
	{0xE5, "LOAD_STORE_INTERLOCK"},
	{0xE6, "FPU_NEON_INTERLOCK"},
	{0xE7, "LOAD_MISS_STALL"},
	{0xE8, "STORE_STALL"},
	{0xFF, "CPU_CYCLES"}
};
#define A53_PMU_DESC_COUNT (sizeof(a53_pmu_desc_v2) / sizeof(struct pmu_desc_v2))

/* Cortex-A73 */
struct pmu_desc_v2 a73_pmu_desc_v2[] = {
	{0x00, "SW_INCR"},
	{0x01, "L1I_CACHE_REFILL"},
	{0x02, "L1I_TLB_REFILL"},
	{0x03, "L1D_CACHE_REFILL"},
	{0x04, "L1D_CACHE"},
	{0x05, "L1D_TLB_REFILL"},
	{0x08, "INST_RETIRED"},
	{0x09, "EXC_TAKEN"},
	{0x0A, "EXC_RETURN"},
	{0x0B, "CID_WRITE_RETIRED"},
	{0x0C, "PC_WRITE_RETIRED"},
	{0x0D, "BR_IMMED_RETIRED"},
	{0x0E, "BR_RETURN_RETIRED"},

	{0x10, "BR_MIS_PRED"},
	{0x11, "CPU_CYCLES"},
	{0x12, "BR_PRED"},
	{0x13, "MEM_ACCESS"},
	{0x14, "L1I_CACHE"},
	{0x15, "L1D_CACHE_WB"},
	{0x16, "L2D_CACHE"},
	{0x17, "L2D_CACHE_REFILL"},
	{0x18, "L2D_CACHE_WB"},
	{0x19, "BUS_ACCESS"},
	{0x1B, "INT_SPEC"},
	{0x1C, "TTBR_WRITE_RETIRED"},
	{0x1D, "BUS_CYCLES"},
	{0x1E, "CHAIN"},

	{0x40, "L1D_CACHE_RD"},
	{0x41, "L1D_CACHE_WR"},

	{0x50, "L2D_CACHE_RD"},
	{0x51, "L2D_CACHE_WR"},
	{0x56, "L2D_CACHE_WB_VICTIM"},
	{0x57, "L2D_CACHE_WB_CLEAN"},
	{0x58, "L2D_CACHE_INVAL"},

	{0x62, "BUS_ACCESS_SHARED"},
	{0x63, "BUS_ACCESS_NOT_SHARED"},
	{0x64, "BUS_ACCESS_NORMAL"},
	{0x65, "BUS_ACCESS_SO_DIV"},
	{0x66, "MEM_ACCESS_RD"},
	{0x67, "MEM_ACCESS_WR"},
	{0x6A, "UNALIGNED_LDST_SPEC"},
	{0x6C, "LDREX_SPEC"},
	{0x6E, "STREC_FAIL_SPEC"},

	{0x70, "LD_SPEC"},
	{0x71, "ST_SPEC"},
	{0x72, "LDST_SPEC"},
	{0x73, "DP_SPEC"},
	{0x74, "ASE_SPEC"},
	{0x75, "VFP_SPEC"},
	{0x77, "CRYPTO_SPEC"},
	{0x7A, "BR_INDIRECT_SPEC"},
	{0x7C, "ISB_SPEC"},
	{0x7D, "DSB_SPEC"},
	{0x7E, "DMB_SPEC"},

	{0x8A, "EXC_HVC"},

	{0xC0, "LF_STALL"},
	{0xC1, "PTW_STALL"},
	{0xC2, "I_TAG_RAM_RD"},
	{0xC3, "I_DATA_RAM_RD"},
	{0xC4, "I_BTAC_RAM_RD"},

	{0xD3, "D_LSU_SLOT_FULL"},
	{0xD8, "LS_IQ_FULL"},
	{0xD9, "DP_IQ_FULL"},
	{0xDA, "DE_IQ_FULL"},
	{0xDC, "EXC_TRAP_HYP"},
	{0xDE, "ETM_EXT_OUT0"},
	{0xDF, "ETM_EXT_OUT1"},

	{0xE0, "MMU_PTW"},
	{0xE1, "MMU_PTW_ST1"},
	{0xE2, "MMU_PTW_ST2"},
	{0xE3, "MMU_PTW_LSU"},
	{0xE4, "MMU_PTW_ISIDE"},
	{0xE5, "MMU_PTW_PLD"},
	{0xE6, "MMU_PTW_CP15"},
	{0xE7, "PLD_UTLB_REFILL"},
	{0xE8, "CP15_UTLB_REFILL"},
	{0xE9, "UTLB_FLUSH"},
	{0xEA, "TLB_ACESS"},
	{0xEB, "TLB_MISS"},
	{0xEC, "DCACHE_SELF_HIT_VIPT"},
        {0xEE, "CYCLES_L2_IDLE"},
        {0xEF, "CPU_DECODE_UNIT_STALLED"},
	{0xFF, "CPU_CYCLES"}
};
#define A73_PMU_DESC_COUNT (sizeof(a73_pmu_desc_v2) / sizeof(struct pmu_desc_v2))


static struct chip_pmu_v2 *gChip[MXNR_CPU_V2];
static struct chip_pmu_v2 chips[] = {
	{CORTEX_A53, a53_pmu_desc_v2, A53_PMU_DESC_COUNT, 0, "Cortex-A53"},
	{CORTEX_A35, a53_pmu_desc_v2, A53_PMU_DESC_COUNT, 0, "Cortex-A35"},
	{CORTEX_A57, a53_pmu_desc_v2, A53_PMU_DESC_COUNT, 0, "Cortex-A57"},
	{CORTEX_A72, a53_pmu_desc_v2, A53_PMU_DESC_COUNT, 0, "Cortex-A72"},
	{CORTEX_A73, a73_pmu_desc_v2, A73_PMU_DESC_COUNT, 0, "Cortex-A73"},
};
static struct chip_pmu_v2 chip_unknown = { CHIP_UNKNOWN, NULL, 0, 0, "Unknown CPU" };
#define CHIP_PMU_COUNT (sizeof(chips) / sizeof(struct chip_pmu_v2))

struct cpu_pmu_hw_v2 armv8_pmu_v2 = {
	.name = "armv8_pmu",
	.get_event_desc = armv8_pmu_hw_get_event_desc,
	.check_event = armv8_pmu_hw_check_event,
	.start = armv8_pmu_hw_start,
	.stop = armv8_pmu_hw_stop,
	.polling = armv8_pmu_hw_polling,
};


/*******************************************************************************
*				Iplement Start
*******************************************************************************/
static struct chip_pmu_v2 *get_chip_pmu_by_cpu_id(int cpu)
{
	return gChip[cpu];
}

static void set_chip_pmu_by_cpu_id(int cpu, struct chip_pmu_v2 *chip)
{
	gChip[cpu] = chip;
}

static inline void armv8_pmu_counter_select(unsigned int idx)
{
	asm volatile ("msr pmselr_el0, %0"::"r" (idx));
	isb();
}

static inline void armv8_pmu_type_select(unsigned int idx, unsigned int type)
{
	armv8_pmu_counter_select(idx);
	asm volatile ("msr pmxevtyper_el0, %0"::"r" (type));
}

static inline unsigned int armv8_pmu_read_count(unsigned int idx)
{
	unsigned int value;

	if (idx == 31) {
		asm volatile ("mrs %0, pmccntr_el0":"=r" (value));
	} else {
		armv8_pmu_counter_select(idx);
		asm volatile ("mrs %0, pmxevcntr_el0":"=r" (value));
	}
	return value;
}

static inline void armv8_pmu_write_count(int idx, u32 value)
{
	if (idx == 31) {
		asm volatile ("msr pmccntr_el0, %0"::"r" (value));
	} else {
		armv8_pmu_counter_select(idx);
		asm volatile ("msr pmxevcntr_el0, %0"::"r" (value));
	}
}

static inline void armv8_pmu_enable_count(unsigned int idx)
{
	asm volatile ("msr pmcntenset_el0, %0"::"r" (1 << idx));
}

static inline void armv8_pmu_disable_count(unsigned int idx)
{
	asm volatile ("msr pmcntenclr_el0, %0"::"r" (1 << idx));
}

static inline void armv8_pmu_enable_intr(unsigned int idx)
{
	asm volatile ("msr pmintenset_el1, %0"::"r" (1 << idx));
}

static inline void armv8_pmu_disable_intr(unsigned int idx)
{
	asm volatile ("msr pmintenclr_el1, %0"::"r" (1 << idx));
	isb();
	asm volatile ("msr pmovsclr_el0, %0"::"r" (1 << idx));
	isb();
}

static inline unsigned int armv8_pmu_overflow(void)
{
	unsigned int val;

	asm volatile ("mrs %0, pmovsclr_el0":"=r" (val));	/* read */
	val &= ARMV8_OVSR_MASK;
	asm volatile ("mrs %0, pmovsclr_el0"::"r" (val));
	return val;
}

static inline unsigned int armv8_pmu_control_read(void)
{
	unsigned int val;

	asm volatile ("mrs %0, pmcr_el0":"=r" (val));
	return val;
}

static inline void armv8_pmu_control_write(u32 val)
{
	val &= ARMV8_PMCR_MASK;
	isb();
	asm volatile ("msr pmcr_el0, %0"::"r" (val));
}

static int armv8_pmu_hw_get_counters(void)
{
	int count = armv8_pmu_control_read();
	/* N, bits[15:11] */
	count = ((count >> ARMV8_PMCR_N_SHIFT) & ARMV8_PMCR_N_MASK);
	return count;
}

static void armv8_pmu_hw_reset_all(int generic_counters)
{
	int i;

	armv8_pmu_control_write(ARMV8_PMCR_C | ARMV8_PMCR_P);
	/* generic counter */
	for (i = 0; i < generic_counters; i++) {
		armv8_pmu_disable_intr(i);
		armv8_pmu_disable_count(i);
	}
	/* cycle counter */
	armv8_pmu_disable_intr(31);
	armv8_pmu_disable_count(31);
	armv8_pmu_overflow();	/* clear overflow */
}

static int armv8_pmu_hw_get_event_desc(int event, char *event_desc)
{
	int i;
	int this_cpu;
	struct chip_pmu_v2 *chip_pmu;

	this_cpu = smp_processor_id();
	chip_pmu = get_chip_pmu_by_cpu_id(this_cpu);
	if (event_desc == NULL)
		return -1;

	for (i = 0; i < chip_pmu->pmu_count; i++) {
		if (chip_pmu->desc[i].event == event) {
			strncpy(event_desc, chip_pmu->desc[i].name, MXSIZE_PMU_DESC - 1);
			break;
		}
	}
	if (i == chip_pmu->pmu_count)
		return -1;

	return 0;
}

static int armv8_pmu_hw_check_event(struct met_pmu_v2 *pmu, int idx, int event)
{
	int this_cpu;
	struct chip_pmu_v2 *chip_pmu;
	int i;

	this_cpu = smp_processor_id();
	chip_pmu = get_chip_pmu_by_cpu_id(this_cpu);
	for (i = 0; i < chip_pmu->pmu_count; i++) {
		if (chip_pmu->desc[i].event == event)
			break;
	}

	if (i == chip_pmu->pmu_count) {                
                PR_BOOTMSG("%s:%d => i=%d, pmu_count=%d\n", __FUNCTION__, __LINE__, i, chip_pmu->pmu_count);
                return -1;
        }

	return 0;
}

static void armv8_pmu_hw_start(struct met_pmu_v2 *pmu, int count)
{
	int i;
	int generic = count - 1;

	armv8_pmu_hw_reset_all(generic);
	for (i = 0; i < generic; i++) {
		if (pmu[i].mode == MODE_POLLING) {
			armv8_pmu_type_select(i, pmu[i].event);
			armv8_pmu_enable_count(i);
		}
	}
	if (pmu[count - 1].mode == MODE_POLLING)
		armv8_pmu_enable_count(31);

	armv8_pmu_control_write(ARMV8_PMCR_E);
}

static void armv8_pmu_hw_stop(int count)
{
	int generic = count - 1;

	armv8_pmu_hw_reset_all(generic);
}

static unsigned int armv8_pmu_hw_polling(struct met_pmu_v2 *pmu, int count, unsigned int *pmu_value)
{
	int i, cnt = 0;
	int generic = count - 1;

	for (i = 0; i < generic; i++) {
		if (pmu[i].mode == MODE_POLLING) {
			pmu_value[cnt] = armv8_pmu_read_count(i);
			cnt++;
		}
	}
	if (pmu[count - 1].mode == MODE_POLLING) {
		pmu_value[cnt] = armv8_pmu_read_count(31);
		cnt++;
	}
	armv8_pmu_control_write(ARMV8_PMCR_C | ARMV8_PMCR_P | ARMV8_PMCR_E);

	return cnt;
}

static void armv8_get_ic(void *info)
{
	unsigned int value;
	unsigned int *type = (unsigned int *)info;

	/* Read Main ID Register */
	asm("mrs %0, midr_el1":"=r"(value));
	*type = (value & 0xffff) >> 4;	/* primary part number */
}

struct cpu_pmu_hw_v2 *cpu_pmu_hw_init_v2(void)
{
	enum ARM_TYPE_v2 type;
	struct chip_pmu_v2 *chip;
	int this_cpu = smp_processor_id();
	int cpu;
	int i;

	for_each_possible_cpu(cpu) {
		if (cpu == this_cpu)
			armv8_get_ic(&type);
		else
			met_smp_call_function_single_symbol(cpu, armv8_get_ic, &type, 1);

		PR_BOOTMSG("CPU TYPE - v8: %x\n", (unsigned int)type);
		for (i = 0; i < CHIP_PMU_COUNT; i++) {
			if (chips[i].type == type) {
				chip = &(chips[i]);
				chip->hw_count = armv8_pmu_hw_get_counters() + 1;
				set_chip_pmu_by_cpu_id(cpu, chip);
				armv8_pmu_v2.chip_pmu[cpu] = chip;
				if (chip->hw_count >= armv8_pmu_v2.max_hw_count)
					armv8_pmu_v2.max_hw_count = chip->hw_count;
				break;
			}
		}
		if (i == CHIP_PMU_COUNT) {
			set_chip_pmu_by_cpu_id(cpu, &chip_unknown);
			return NULL;
		}
	}

	return &armv8_pmu_v2;
}

