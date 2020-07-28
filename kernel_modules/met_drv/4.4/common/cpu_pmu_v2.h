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

#ifndef _PMU_V2_H_
#define _PMU_V2_H_

#include <linux/device.h>
#include <linux/threads.h> /* NR_CPUS */

#define MODE_DISABLED	0
#define MODE_INTERRUPT	1
#define MODE_POLLING	2
#define MXSIZE_PMU_DESC 32
#define MXNR_CPU_V2     NR_CPUS


/*******************************************************************************
*                                Type Define
*******************************************************************************/
struct met_pmu_v2 {
	unsigned char mode;
	unsigned short event;
	struct kobject *kobj_cpu_pmu;
        const char *cpu_name;
};

enum ARM_TYPE_v2 {
	CORTEX_A53 = 0xD03,
	CORTEX_A35 = 0xD04,
	CORTEX_A57 = 0xD07,
	CORTEX_A72 = 0xD08,
	CORTEX_A73 = 0xD09,
	CHIP_UNKNOWN = 0xFFF
};

struct pmu_desc_v2 {
	unsigned int event;
	char name[MXSIZE_PMU_DESC];
};

struct chip_pmu_v2 {
	enum ARM_TYPE_v2 type;
	struct pmu_desc_v2 *desc;
        unsigned int pmu_count;
	unsigned int hw_count;
	const char *cpu_name;
};

struct cpu_pmu_hw_v2 {
	const char *name;
        int max_hw_count;
	int (*get_event_desc)(int event, char *event_desc);
	int (*check_event)(struct met_pmu_v2 *pmu, int idx, int event);
	void (*start)(struct met_pmu_v2 *pmu, int count);
	void (*stop)(int count);
	unsigned int (*polling)(struct met_pmu_v2 *pmu, int count, unsigned int *pmu_value);
	struct met_pmu_v2 *met_pmu[MXNR_CPU_V2];
        struct chip_pmu_v2 *chip_pmu[MXNR_CPU_V2];
};


extern struct cpu_pmu_hw_v2 *cpu_pmu_hw_v2;


/*******************************************************************************
*                                Fuction Pototypes
*******************************************************************************/
extern noinline void mp_cpu_v2(unsigned char cnt, unsigned int *value);
extern void met_perf_cpupmu_online_v2(unsigned int cpu);
extern void met_perf_cpupmu_down_v2(void *cpu);
extern void met_perf_cpupmu_start_v2(void);
extern void met_perf_cpupmu_stop_v2(void);
extern void cpupmu_polling_v2(unsigned long long stamp, int cpu);

#endif /* _PMU_V2_H_ */
