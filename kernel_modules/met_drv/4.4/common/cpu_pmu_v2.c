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

#include <linux/slab.h>
#include <linux/version.h>

#include "interface.h"
#include "trace.h"
#include "cpu_pmu_v2.h"
#include "v8_pmu_hw_v2.h"
#include "met_drv.h"


#define MET_USER_EVENT_SUPPORT

#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/workqueue.h>
#include <linux/perf_event.h>
#include "met_kernel_symbol.h"


/*******************************************************************************
*				Type Define
*******************************************************************************/
#define CNTMAX NR_CPUS


/*******************************************************************************
*				Fuction Pototypes
*******************************************************************************/
static inline struct met_pmu_v2 *get_met_pmu_by_cpu_id(const unsigned int cpu);
static inline void set_met_pmu_by_cpu_id(const unsigned int cpu, struct met_pmu_v2 *met_pmu);

static int reset_driver_stat(void);
static struct met_pmu_v2 *lookup_pmu(struct kobject *kobj);

static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

static int cpupmu_create_subfs(struct kobject *parent);
static void cpupmu_delete_subfs(void);
static void _cpupmu_start(void *info);
static void cpupmu_start(void);
static void _cpupmu_stop(void *info);
static void cpupmu_stop(void);
static void cpupmu_polling(unsigned long long stamp, int cpu);
extern void cpupmu_polling_v2(unsigned long long stamp, int cpu);
static int cpupmu_print_help(char *buf, int len);
static int cpupmu_print_header(char *buf, int len);
static int cpupmu_process_argument(const char *arg, int len);


/*******************************************************************************
*				Globe Variables
*******************************************************************************/
static int module_status;

struct cpu_pmu_hw_v2 *met_pmu_hw_v2;

static unsigned int gPMU_CNT[2*MXNR_CPU_V2];
static unsigned int gMAX_PMU_HW_CNT;

static struct kobject *gKOBJ_CPU;
static struct met_pmu_v2 *gMET_PMU[2*MXNR_CPU_V2];

static struct kobj_attribute mode_attr = __ATTR(mode, 0444, mode_show, NULL);

static const char cache_line_header[] =
	"met-info [000] 0.0: met_cpu_cache_line_size: %d\n";
static const char header[] =
	"met-info [000] 0.0: met_cpu_header_v2: ";
static const char help[] =
	"  --cpu-pmu=CORE_ID:EVENT	     select CPU-PMU events. in %s,\n"
	"				      you can enable at most \"%d general purpose events\"\n"
	"				      plus \"one special 0xff (CPU_CYCLE) event\"\n";

static DEFINE_PER_CPU(int[CNTMAX], perfCurr);
static DEFINE_PER_CPU(int[CNTMAX], perfPrev);
static DEFINE_PER_CPU(struct perf_event * [CNTMAX], pevent);
static DEFINE_PER_CPU(struct perf_event_attr [CNTMAX], pevent_attr);
static DEFINE_PER_CPU(int, perfSet);
static DEFINE_PER_CPU(unsigned int, perf_task_init_done);
static DEFINE_PER_CPU(unsigned int, perf_cpuid);

static DEFINE_PER_CPU(struct delayed_work, cpu_pmu_dwork);
static DEFINE_PER_CPU(struct delayed_work *, perf_delayed_work_setup);

struct metdevice met_cpupmu_v2 = {
	.name = "cpu-pmu",
	.type = MET_TYPE_PMU,
	.cpu_related = 1,
	.create_subfs = cpupmu_create_subfs,
	.delete_subfs = cpupmu_delete_subfs,
	.start = cpupmu_start,
	.stop = cpupmu_stop,
	.polling_interval = 1,
	.timed_polling = cpupmu_polling,
	.print_help = cpupmu_print_help,
	.print_header = cpupmu_print_header,
	.process_argument = cpupmu_process_argument
};


/*******************************************************************************
*				Iplement Start
*******************************************************************************/
static inline struct met_pmu_v2 *get_met_pmu_by_cpu_id(const unsigned int cpu)
{
	if (cpu < MXNR_CPU_V2)
		return gMET_PMU[cpu];
	else
		return NULL;
}


static inline void set_met_pmu_by_cpu_id(const unsigned int cpu, struct met_pmu_v2 *met_pmu)
{
	if (cpu < MXNR_CPU_V2)
		gMET_PMU[cpu] = met_pmu;
}


static int reset_driver_stat()
{
	int i;
	int cpu;
	struct met_pmu_v2 *met_pmu;

	met_cpupmu_v2.mode = 0;
	for_each_possible_cpu(cpu) {
		met_pmu = get_met_pmu_by_cpu_id(cpu);
		for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
			met_pmu[i].mode = MODE_DISABLED;
			met_pmu[i].event = 0;
		}
		gPMU_CNT[cpu] = 0;
	}
	module_status = 0;
	return 0;
}


static struct met_pmu_v2 *lookup_pmu(struct kobject *kobj)
{
	int i;
	int cpu;
	struct met_pmu_v2 *met_pmu;

	for_each_possible_cpu(cpu) {
		met_pmu = get_met_pmu_by_cpu_id(cpu);
		for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
			if (met_pmu[i].kobj_cpu_pmu == kobj)
				return &met_pmu[i];
		}
	}
	return NULL;
}


static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu_v2 *p = lookup_pmu(kobj);

	if (p != NULL) {
		switch (p->mode) {
		case 0:
			return snprintf(buf, PAGE_SIZE, "%hhd (disabled)\n", p->mode);
		case 1:
			return snprintf(buf, PAGE_SIZE, "%hhd (interrupt)\n", p->mode);
		case 2:
			return snprintf(buf, PAGE_SIZE, "%hhd (polling)\n", p->mode);
		}
	}
	return -EINVAL;
}


static int cpupmu_create_subfs(struct kobject *parent)
{
	int ret = 0;
	unsigned int i;
	unsigned int cpu;
	char buf[16];
	struct met_pmu_v2 *met_pmu;

	met_pmu_hw_v2 = cpu_pmu_hw_init_v2();
	if (met_pmu_hw_v2 == NULL) {
		PR_BOOTMSG("Failed to init CPU PMU HW!!\n");
		return -ENODEV;
	}
	gMAX_PMU_HW_CNT = met_pmu_hw_v2->max_hw_count;

	gKOBJ_CPU = parent;
	for_each_possible_cpu(cpu) {
		met_pmu = kmalloc_array(gMAX_PMU_HW_CNT, sizeof(struct met_pmu_v2), GFP_KERNEL);
		if (met_pmu != NULL) {
			memset(met_pmu, 0x0, gMAX_PMU_HW_CNT * sizeof(struct met_pmu_v2));
			met_pmu_hw_v2->met_pmu[cpu] = met_pmu;
			set_met_pmu_by_cpu_id(cpu, met_pmu);
		} else
			ret = -ENOMEM;

		for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
			snprintf(buf, sizeof(buf), "CPU-%d-%d", cpu, i);
			met_pmu[i].kobj_cpu_pmu = kobject_create_and_add(buf, gKOBJ_CPU);
			if (met_pmu[i].kobj_cpu_pmu) {
				ret = sysfs_create_file(met_pmu[i].kobj_cpu_pmu, &mode_attr.attr);
				if (ret != 0) {
					PR_BOOTMSG("Failed to create mode in sysfs\n");
					goto out;
				}
			}
		}
	}
 out:
	if (ret != 0) {
		for_each_possible_cpu(cpu) {
			met_pmu = get_met_pmu_by_cpu_id(cpu);
			if (met_pmu != NULL) {
				kfree(met_pmu);
				set_met_pmu_by_cpu_id(cpu, NULL);
			}
		}
	}
	return ret;
}


static void cpupmu_delete_subfs(void)
{
	unsigned int i;
	unsigned int cpu;
	struct met_pmu_v2 *met_pmu;

	for_each_possible_cpu(cpu) {
		met_pmu = get_met_pmu_by_cpu_id(cpu);
		if (met_pmu != NULL) {
			for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
				sysfs_remove_file(met_pmu[i].kobj_cpu_pmu, &mode_attr.attr);
				kobject_del(met_pmu[i].kobj_cpu_pmu);
				kobject_put(met_pmu[i].kobj_cpu_pmu);
				met_pmu[i].kobj_cpu_pmu = NULL;
			}
			kfree(met_pmu);
		}
		set_met_pmu_by_cpu_id(cpu, NULL);
	}

	if (gKOBJ_CPU != NULL) {
		gKOBJ_CPU = NULL;
	}

	met_pmu_hw_v2  = NULL;
}


noinline void mp_cpu_v2(unsigned char cnt, unsigned int *value)
{
        if (cnt < MXNR_CPU_V2)
	        MET_GENERAL_PRINT(MET_TRACE, cnt, value);
}


static void dummy_handler(struct perf_event *event, struct perf_sample_data *data,
			  struct pt_regs *regs)
{
/* Required as perf_event_create_kernel_counter() requires an overflow handler, even though all we do is poll */
}


void perf_cpupmu_polling_v2(unsigned long long stamp, int cpu)
{
	int i, count, delta;
	struct perf_event *ev;
	unsigned int pmu_value[MXNR_CPU_V2];
	struct met_pmu_v2 *met_pmu;

	if (per_cpu(perfSet, cpu) == 0)
		return;

	memset(pmu_value, 0, sizeof(pmu_value));
	count = 0;
	met_pmu = get_met_pmu_by_cpu_id(cpu);
	for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
		if (met_pmu[i].mode == 0)
			continue;

		ev = per_cpu(pevent, cpu)[i];
		if ((ev != NULL) && (ev->state == PERF_EVENT_STATE_ACTIVE)) {
			if (per_cpu(perfPrev, cpu)[i] == 0) {
				per_cpu(perfPrev, cpu)[i] = met_perf_event_read_local(ev);
				continue;
			}
			per_cpu(perfCurr, cpu)[i] = met_perf_event_read_local(ev);
			delta = per_cpu(perfCurr, cpu)[i] - per_cpu(perfPrev, cpu)[i];
			per_cpu(perfPrev, cpu)[i] = per_cpu(perfCurr, cpu)[i];
			if (delta < 0)
				delta *= -1;
			pmu_value[i] = delta;
			count++;
		}
	}

	if (count == gPMU_CNT[cpu])
		mp_cpu_v2(count, pmu_value);
}


static int perf_thread_set_perf_events_v2(unsigned int cpu)
{
	int i, size;
	struct perf_event *ev;
	struct met_pmu_v2 *met_pmu;

	size = sizeof(struct perf_event_attr);
	if (per_cpu(perfSet, cpu) == 0) {
		met_pmu = get_met_pmu_by_cpu_id(cpu);
		for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
			per_cpu(pevent, cpu)[i] = NULL;
			if (!met_pmu[i].mode) {/* Skip disabled counters */
				continue;
			}
			per_cpu(perfPrev, cpu)[i] = 0;
			per_cpu(perfCurr, cpu)[i] = 0;
			memset(&per_cpu(pevent_attr, cpu)[i], 0, size);
			per_cpu(pevent_attr, cpu)[i].config = met_pmu[i].event;
			per_cpu(pevent_attr, cpu)[i].type = PERF_TYPE_RAW;
			per_cpu(pevent_attr, cpu)[i].size = size;
			per_cpu(pevent_attr, cpu)[i].sample_period = 0;
			per_cpu(pevent_attr, cpu)[i].pinned = 1;
			if (met_pmu[i].event == 0xff) {
				per_cpu(pevent_attr, cpu)[i].type = PERF_TYPE_HARDWARE;
				per_cpu(pevent_attr, cpu)[i].config = PERF_COUNT_HW_CPU_CYCLES;
			}

			per_cpu(pevent, cpu)[i] =
			    perf_event_create_kernel_counter(&per_cpu(pevent_attr, cpu)[i], cpu, NULL,
				     dummy_handler, NULL);
			if (IS_ERR(per_cpu(pevent, cpu)[i])) {
				per_cpu(pevent, cpu)[i] = NULL;
				PR_BOOTMSG("CPU=%d, %s:%d\n", cpu, __FUNCTION__, __LINE__);
				continue;
			}

			if (per_cpu(pevent, cpu)[i]->state != PERF_EVENT_STATE_ACTIVE) {
				perf_event_release_kernel(per_cpu(pevent, cpu)[i]);
				per_cpu(pevent, cpu)[i] = NULL;
				PR_BOOTMSG("CPU=%d, %s:%d\n", cpu, __FUNCTION__, __LINE__);
				continue;
			}

			ev = per_cpu(pevent, cpu)[i];
			if (ev != NULL) {
				perf_event_enable(ev);
			}
		} /* for all PMU counter */
		per_cpu(perfSet, cpu) = 1;
	} /* for perfSet */
	return 0;
}


static void perf_thread_setup_v2(struct work_struct *work)
{
	unsigned int cpu;
	struct delayed_work *dwork = to_delayed_work(work);

	cpu = dwork->cpu;
	if (per_cpu(perf_task_init_done, cpu) == 0) {
		per_cpu(perf_task_init_done, cpu) = 1;
		perf_thread_set_perf_events_v2(cpu);
	}

	return ;
}


void met_perf_cpupmu_online_v2(unsigned int cpu)
{
	if (met_cpupmu_v2.mode == 0) {
		PR_BOOTMSG("CPU=%d, %s:%d\n", cpu, __FUNCTION__, __LINE__);
		return;
	}

	per_cpu(perf_cpuid, cpu) = cpu;
	if (per_cpu(perf_delayed_work_setup, cpu) == NULL) {
		struct delayed_work *dwork;

		dwork = &per_cpu(cpu_pmu_dwork, cpu);
		dwork->cpu = cpu;
		INIT_DELAYED_WORK(dwork, perf_thread_setup_v2);
		schedule_delayed_work(dwork, 0);
		per_cpu(perf_delayed_work_setup, cpu) = dwork;
	}
}


void met_perf_cpupmu_down_v2(void *data)
{
	unsigned int cpu;
	unsigned int i;
	struct perf_event *ev;
	struct met_pmu_v2 *met_pmu;

	cpu = *((unsigned int *)data);
	if (met_cpupmu_v2.mode == 0)
		return;
	if (per_cpu(perfSet, cpu) == 0)
		return;

	met_pmu = get_met_pmu_by_cpu_id(cpu);
	per_cpu(perfSet, cpu) = 0;
	for (i = 0; i < gMAX_PMU_HW_CNT; i++) {
		if (!met_pmu[i].mode)
			continue;
		ev = per_cpu(pevent, cpu)[i];
		if ((ev != NULL) && (ev->state == PERF_EVENT_STATE_ACTIVE)) {
			perf_event_disable(ev);
			perf_event_release_kernel(ev);
		}
	}
	per_cpu(perf_task_init_done, cpu) = 0;
	per_cpu(perf_delayed_work_setup, cpu) = NULL;
}


void met_perf_cpupmu_start_v2(void)
{
	unsigned int cpu;

	for_each_online_cpu(cpu) {
		met_perf_cpupmu_online_v2(cpu);
	}
}


void met_perf_cpupmu_stop_v2(void)
{
	unsigned int cpu;

	for_each_online_cpu(cpu) {
		per_cpu(perf_cpuid, cpu) = cpu;
		met_perf_cpupmu_down_v2((void *)&per_cpu(perf_cpuid, cpu));
	}
}


static void cpupmu_polling(unsigned long long stamp, int cpu)
{
	int count;
	struct met_pmu_v2 *met_pmu;
	unsigned int pmu_value[MXNR_CPU_V2];

	met_pmu = get_met_pmu_by_cpu_id(cpu);
	if (met_cpu_pmu_method == 0) {
		count = met_pmu_hw_v2->polling(met_pmu, gMAX_PMU_HW_CNT, pmu_value);
		mp_cpu_v2(count, pmu_value);
	} else
		perf_cpupmu_polling_v2(stamp, cpu);
}


void cpupmu_polling_v2(unsigned long long stamp, int cpu)
{
	cpupmu_polling(stamp, cpu);
}


static void _cpupmu_start(void *info)
{
	unsigned int *cpu = (unsigned int *)info;
	struct met_pmu_v2 *met_pmu;

	met_pmu = get_met_pmu_by_cpu_id(*cpu);
	met_pmu_hw_v2->start(met_pmu, gMAX_PMU_HW_CNT);
}

static void cpupmu_start(void)
{
	if (module_status == 1) {
		PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
		return;
	}

	if (met_cpu_pmu_method == 0) {
		int this_cpu = smp_processor_id();
		int cpu;

		for_each_possible_cpu(cpu) {
			if (cpu == this_cpu)
				_cpupmu_start(&cpu);
			else
				met_smp_call_function_single_symbol(cpu, _cpupmu_start, &cpu, 1);
		}
	}
	module_status = 1;
}

static void _cpupmu_stop(void *info)
{
	(void)info;

	met_pmu_hw_v2->stop(gMAX_PMU_HW_CNT);
}

static void cpupmu_stop(void)
{
	if (module_status == 0) {
		PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
		return;
	}

	if (met_cpu_pmu_method == 0) {
		int this_cpu = smp_processor_id();
		int cpu;

		for_each_possible_cpu(cpu) {
			if (cpu == this_cpu)
				_cpupmu_stop(&cpu);
			else
				met_smp_call_function_single_symbol(cpu, _cpupmu_stop, &cpu, 1);
		}
	}
	module_status = 0;
}

static int cpupmu_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help, met_pmu_hw_v2->name, gMAX_PMU_HW_CNT - 1);
}

static int cpupmu_print_header(char *buf, int len)
{
	int i;
	int ret = 0;
	int pmu_cnt = 0;
	char name[32];
	unsigned int cpu;
	struct met_pmu_v2 *met_pmu;

	/*append CPU PMU access method*/
	if (met_cpu_pmu_method == 0)
		ret += snprintf(buf + ret, PAGE_SIZE,
			"met-info [000] 0.0: CPU_PMU_method: PMU registers\n");
	else
		ret += snprintf(buf + ret, PAGE_SIZE,
			"met-info [000] 0.0: CPU_PMU_method: perf APIs\n");

	/*append cache line size*/
	ret += snprintf(buf + ret, PAGE_SIZE - ret, cache_line_header, cache_line_size());
	ret += snprintf(buf + ret, PAGE_SIZE - ret, header);
	for_each_online_cpu(cpu) {
		int cnt = 0;

		pmu_cnt = gPMU_CNT[cpu];
		met_pmu = get_met_pmu_by_cpu_id(cpu);
		for (i = 0; i < pmu_cnt; i++) {
			if (met_pmu[i].mode == 0)
				continue;

			if (met_pmu_hw_v2->get_event_desc && 0 == met_pmu_hw_v2->get_event_desc(met_pmu[i].event, name)) {
				if (cnt == 0) {
					ret += snprintf(buf + ret, PAGE_SIZE - ret, "CPU-%d=0x%x:%s", cpu, met_pmu[i].event, name);
					cnt++;
				} else
					ret += snprintf(buf + ret, PAGE_SIZE - ret, ",0x%x:%s", met_pmu[i].event, name);
			}
			met_pmu[i].mode = 0;
		}
		if (cnt > 0 && cpu < MXNR_CPU_V2 - 1)
			ret += snprintf(buf + ret, PAGE_SIZE - ret, ";");
	}
	ret += snprintf(buf + ret, PAGE_SIZE - ret, "\n");
	met_cpupmu_v2.mode = 0;
	reset_driver_stat();
	return ret;
}

/*
 * "met-cmd --start --pmu_core_evt=0:0x3,0x16,0x17"
 */
static int cpupmu_process_argument(const char *arg, int len)
{
	int ret;
	unsigned int cpu;
	unsigned int value;
	unsigned int idx = 0;
	char *str = NULL;
	char *token = NULL;
	struct met_pmu_v2 *met_pmu = NULL;

	if (met_cpu_pmu_method == 0)
		gMAX_PMU_HW_CNT = met_pmu_hw_v2->max_hw_count;
	else
		gMAX_PMU_HW_CNT = perf_num_counters();

	if (gMAX_PMU_HW_CNT == 0) {
		PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
		goto arg_out;
	}

	str = kstrdup(arg, GFP_KERNEL);
	token = strsep(&str, ":");
	ret = met_parse_num(token, &cpu, strlen(token));
	if (ret != 0) {
		PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
		goto arg_out;
	}

	met_pmu = get_met_pmu_by_cpu_id(cpu);
	while (token && met_pmu && idx < gMAX_PMU_HW_CNT) {
		token = strsep(&str, ",\r\n");
		if (token) {
			ret = met_parse_num(token, &value, strlen(token));
			if (ret != 0) {
				PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
				goto arg_out;
			}
			if (value != 0xff) {
				if (idx >= (gMAX_PMU_HW_CNT - 1)) {
					PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
					goto arg_out;
				}
				met_pmu[idx].mode = MODE_POLLING;
				met_pmu[idx].event = value;
				idx++;
				gPMU_CNT[cpu]++;
			} else {
				if (met_cpu_pmu_method == 0) {
					met_pmu[gMAX_PMU_HW_CNT - 1].mode = MODE_POLLING;
					met_pmu[gMAX_PMU_HW_CNT - 1].event = 0xff;
					gPMU_CNT[cpu]++;
				} else {
					if (idx > (gMAX_PMU_HW_CNT - 1)) {
						PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
						goto arg_out;
					}
					met_pmu[idx].mode = MODE_POLLING;
					met_pmu[idx].event = 0xff;
					idx++;
					gPMU_CNT[cpu]++;
				}
			}
			if (met_pmu_hw_v2->check_event(met_pmu, gPMU_CNT[cpu], value) < 0) {
				PR_BOOTMSG("%s:%d\n", __FUNCTION__, __LINE__);
				goto arg_out;
			}
		}
	}
	met_cpupmu_v2.mode = 1;
	module_status = 0;
	return 0;

arg_out:
	if (str)
		kfree(str);
	reset_driver_stat();
	return -EINVAL;
}
