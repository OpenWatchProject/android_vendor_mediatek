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

/* include <asm/page.h> */
#include <linux/slab.h>
#include <linux/version.h>

#include "interface.h"
#include "trace.h"
#include "cpu_pmu.h"
#include "met_drv.h"

#define MET_USER_EVENT_SUPPORT

#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/perf_event.h>
#include "met_kernel_symbol.h"
#include "interface.h"

static int counter_cnt;

struct metdevice met_cpupmu;
struct cpu_pmu_hw *cpu_pmu;
static int nr_counters;

static struct kobject *kobj_cpu;
static struct met_pmu *pmu;
static int nr_arg;

#define CNTMAX NR_CPUS
static DEFINE_PER_CPU(unsigned long long[CNTMAX], perfCurr);
static DEFINE_PER_CPU(unsigned long long[CNTMAX], perfPrev);
static DEFINE_PER_CPU(int[CNTMAX], perfCntFirst);
static DEFINE_PER_CPU(struct perf_event * [CNTMAX], pevent);
static DEFINE_PER_CPU(struct perf_event_attr [CNTMAX], pevent_attr);
static DEFINE_PER_CPU(int, perfSet);
static DEFINE_PER_CPU(struct task_struct *, perf_task_setup);
static DEFINE_PER_CPU(unsigned int, perf_task_init_done);
static DEFINE_PER_CPU(unsigned int, perf_cpuid);

static inline int reset_driver_stat(int counters)
{
	int i;

	nr_arg = 0;
	counter_cnt = 0;
	met_cpupmu.mode = 0;
	for (i = 0; i < counters; i++) {
		pmu[i].mode = MODE_DISABLED;
		pmu[i].event = 0;
		pmu[i].freq = 0;
	}

	return 0;
}

static inline struct met_pmu *lookup_pmu(struct kobject *kobj)
{
	int i;

	for (i = 0; i < nr_counters; i++) {
		if (pmu[i].kobj_cpu_pmu == kobj)
			return &pmu[i];
	}
	return NULL;
}

static ssize_t count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", nr_counters - 1);
}

static ssize_t count_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	return -EINVAL;
}

static ssize_t event_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu *p = lookup_pmu(kobj);

	if (p != NULL)
		return snprintf(buf, PAGE_SIZE, "0x%hx\n", p->event);

	return -EINVAL;
}

static ssize_t event_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	struct met_pmu *p = lookup_pmu(kobj);
	unsigned short event;

	if (p != NULL) {
		if (sscanf(buf, "0x%hx", &event) != 1)
			return -EINVAL;

		if (p == &(pmu[nr_counters - 1])) {	/* cycle counter */
			if (event != 0xff)
				return -EINVAL;
		} else {
			if (cpu_pmu->check_event(pmu, nr_arg, event) < 0)
				return -EINVAL;
		}

		p->event = event;
		return n;
	}
	return -EINVAL;
}

static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu *p = lookup_pmu(kobj);

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

static ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	unsigned int mode;
	struct met_pmu *p = lookup_pmu(kobj);

	if (p != NULL) {
		if (kstrtouint(buf, 0, &mode) != 0)
			return -EINVAL;

		if (mode <= 2) {
			p->mode = (unsigned char)mode;
			if (mode > 0)
				met_cpupmu.mode = 1;
			return n;
		}
	}
	return -EINVAL;
}

static ssize_t freq_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu *p = lookup_pmu(kobj);

	if (p != NULL)
		return snprintf(buf, PAGE_SIZE, "%ld\n", p->freq);

	return -EINVAL;
}

static ssize_t freq_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	struct met_pmu *p = lookup_pmu(kobj);

	if (p != NULL) {
		if (kstrtoul(buf, 0, &(p->freq)) != 0)
			return -EINVAL;

		return n;
	}
	return -EINVAL;
}

static struct kobj_attribute count_attr = __ATTR(count, 0664, count_show, count_store);
static struct kobj_attribute event_attr = __ATTR(event, 0664, event_show, event_store);
static struct kobj_attribute mode_attr = __ATTR(mode, 0664, mode_show, mode_store);
static struct kobj_attribute freq_attr = __ATTR(freq, 0664, freq_show, freq_store);

static int cpupmu_create_subfs(struct kobject *parent)
{
	int ret = 0;
	int i;
	char buf[16];

	cpu_pmu = cpu_pmu_hw_init();
	if (cpu_pmu == NULL) {
		PR_BOOTMSG("Failed to init CPU PMU HW!!\n");
		return -ENODEV;
	}
	nr_counters = cpu_pmu->nr_cnt;

	pmu = kmalloc_array(nr_counters, sizeof(struct met_pmu), GFP_KERNEL);
	if (pmu == NULL)
		return -ENOMEM;

	memset(pmu, 0, sizeof(struct met_pmu) * nr_counters);
	cpu_pmu->pmu = pmu;
	kobj_cpu = parent;

	ret = sysfs_create_file(kobj_cpu, &count_attr.attr);
	if (ret != 0) {
		PR_BOOTMSG("Failed to create count in sysfs\n");
		goto out;
	}

	for (i = 0; i < nr_counters; i++) {
		snprintf(buf, sizeof(buf), "%d", i);
		pmu[i].kobj_cpu_pmu = kobject_create_and_add(buf, kobj_cpu);

		ret = sysfs_create_file(pmu[i].kobj_cpu_pmu, &event_attr.attr);
		if (ret != 0) {
			PR_BOOTMSG("Failed to create event in sysfs\n");
			goto out;
		}

		ret = sysfs_create_file(pmu[i].kobj_cpu_pmu, &mode_attr.attr);
		if (ret != 0) {
			PR_BOOTMSG("Failed to create mode in sysfs\n");
			goto out;
		}

		ret = sysfs_create_file(pmu[i].kobj_cpu_pmu, &freq_attr.attr);
		if (ret != 0) {
			PR_BOOTMSG("Failed to create freq in sysfs\n");
			goto out;
		}
	}

 out:
	if (ret != 0) {
		if (pmu != NULL) {
			kfree(pmu);
			pmu = NULL;
		}
	}
	return ret;
}

static void cpupmu_delete_subfs(void)
{
	int i;

	if (kobj_cpu != NULL) {
		for (i = 0; i < nr_counters; i++) {
			sysfs_remove_file(pmu[i].kobj_cpu_pmu, &event_attr.attr);
			sysfs_remove_file(pmu[i].kobj_cpu_pmu, &mode_attr.attr);
			sysfs_remove_file(pmu[i].kobj_cpu_pmu, &freq_attr.attr);
			kobject_del(pmu[i].kobj_cpu_pmu);
			kobject_put(pmu[i].kobj_cpu_pmu);
			pmu[i].kobj_cpu_pmu = NULL;
		}
		sysfs_remove_file(kobj_cpu, &count_attr.attr);
		kobj_cpu = NULL;
	}

	if (pmu != NULL) {
		kfree(pmu);
		pmu = NULL;
	}

	cpu_pmu  = NULL;
}

noinline void mp_cpu(unsigned char cnt, unsigned int *value)
{
	MET_GENERAL_PRINT(MET_TRACE, cnt, value);
}

static void dummy_handler(struct perf_event *event, struct perf_sample_data *data,
			  struct pt_regs *regs)
{
/* Required as perf_event_create_kernel_counter() requires an overflow handler, even though all we do is poll */
}

void perf_cpupmu_polling(unsigned long long stamp, int cpu)
{
	int i, count;
	long long int delta;
	struct perf_event *ev;
	unsigned int pmu_value[MXNR_CPU];

	if (per_cpu(perfSet, cpu) == 0)
		return;

	memset(pmu_value, 0, sizeof(pmu_value));
	count = 0;
	for (i = 0; i < nr_counters; i++) {
		if (pmu[i].mode == 0)
			continue;

		ev = per_cpu(pevent, cpu)[i];
		if ((ev != NULL) && (ev->state == PERF_EVENT_STATE_ACTIVE)) {
			per_cpu(perfCurr, cpu)[i] = met_perf_event_read_local_symbol(ev);
			delta = (long long int)(per_cpu(perfCurr, cpu)[i] - per_cpu(perfPrev, cpu)[i]);
			if (delta < 0)
				delta *= (-1);
			per_cpu(perfPrev, cpu)[i] = per_cpu(perfCurr, cpu)[i];
			if (per_cpu(perfCntFirst, cpu)[i] == 1) {
				/* we shall omit delta counter when we get first counter */
				per_cpu(perfCntFirst, cpu)[i] = 0;
				continue;
			}
			pmu_value[i] = (unsigned int) delta;
			count++;
		}
	}

	if (count == counter_cnt)
		mp_cpu(count, pmu_value);
}

static int perf_thread_set_perf_events(unsigned int cpu)
{
	int i, size;
	struct perf_event *ev;

	size = sizeof(struct perf_event_attr);
	if (per_cpu(perfSet, cpu) == 0) {
		for (i = 0; i < nr_counters; i++) {
			per_cpu(pevent, cpu)[i] = NULL;
			if (!pmu[i].mode) {
				/* Skip disabled counters */
				continue;
			}
			per_cpu(perfPrev, cpu)[i] = 0;
			per_cpu(perfCurr, cpu)[i] = 0;
			memset(&per_cpu(pevent_attr, cpu)[i], 0, size);
			per_cpu(pevent_attr, cpu)[i].config = pmu[i].event;
			per_cpu(pevent_attr, cpu)[i].type = PERF_TYPE_RAW;
			per_cpu(pevent_attr, cpu)[i].size = size;
			per_cpu(pevent_attr, cpu)[i].sample_period = 0;
			per_cpu(pevent_attr, cpu)[i].pinned = 1;
			if (pmu[i].event == 0xff) {
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
			if (ev != NULL)
				perf_event_enable(ev);
			per_cpu(perfCntFirst, cpu)[i] = 1;
		} /* for all PMU counter */
		per_cpu(perfSet, cpu) = 1;
	} /* for perfSet */
	return 0;
}

static int perf_thread_setup(void *data)
{
	unsigned int cpu;

	cpu = *((unsigned int *)data);
	if (per_cpu(perf_task_init_done, cpu) == 0) {
		per_cpu(perf_task_init_done, cpu) = 1;
		perf_thread_set_perf_events(cpu);
	}

	return 0;
}

void met_perf_cpupmu_online(unsigned int cpu)
{
	if (met_cpupmu.mode == 0)
		return;

	per_cpu(perf_cpuid, cpu) = cpu;
	if (per_cpu(perf_task_setup, cpu) == NULL) {
		per_cpu(perf_task_setup, cpu) = met_kthread_create_on_cpu_symbol(perf_thread_setup,
			(void *)&per_cpu(perf_cpuid, cpu), cpu, "met_perf");
		kthread_unpark(per_cpu(perf_task_setup, cpu));
	}
}

void met_perf_cpupmu_down(void *data)
{
	unsigned int cpu;
	unsigned int i;
	struct perf_event *ev;

	cpu = *((unsigned int *)data);
	if (met_cpupmu.mode == 0)
		return;
	if (per_cpu(perfSet, cpu) == 0)
		return;
	per_cpu(perfSet, cpu) = 0;
	for (i = 0; i < nr_counters; i++) {
		if (!pmu[i].mode)
			continue;
		ev = per_cpu(pevent, cpu)[i];
		if ((ev != NULL) && (ev->state == PERF_EVENT_STATE_ACTIVE)) {
			perf_event_disable(ev);
			perf_event_release_kernel(ev);
		}
	}
	per_cpu(perf_task_init_done, cpu) = 0;
	per_cpu(perf_task_setup, cpu) = NULL;
}

void met_perf_cpupmu_stop(void)
{
	unsigned int cpu;

	for_each_online_cpu(cpu) {
		per_cpu(perf_cpuid, cpu) = cpu;
		met_perf_cpupmu_down((void *)&per_cpu(perf_cpuid, cpu));
	}
}

void met_perf_cpupmu_start(void)
{
	unsigned int cpu;

	for_each_online_cpu(cpu) {
		met_perf_cpupmu_online(cpu);
	}
}

void cpupmu_polling(unsigned long long stamp, int cpu)
{
	int count;
	unsigned int pmu_value[MXNR_CPU];

	if (met_cpu_pmu_method == 0) {
		count = cpu_pmu->polling(pmu, nr_counters, pmu_value);
		mp_cpu(count, pmu_value);
	} else {
		perf_cpupmu_polling(stamp, cpu);
	}
}

static void cpupmu_start(void)
{
	if (met_cpu_pmu_method == 0) {
		nr_arg = 0;
		cpu_pmu->start(pmu, nr_counters);
	}
}

static void cpupmu_stop(void)
{
	if (met_cpu_pmu_method == 0)
		cpu_pmu->stop(nr_counters);
}

static const char cache_line_header[] =
	"met-info [000] 0.0: met_cpu_cache_line_size: %d\n";
static const char header_n[] =
	"# mp_cpu: pmu_value1, ...\n"
	"met-info [000] 0.0: met_cpu_header: 0x%x:%s";
static const char header[] =
	"# mp_cpu: pmu_value1, ...\n"
	"met-info [000] 0.0: met_cpu_header: 0x%x";

static const char help[] =
	"  --pmu-cpu-evt=EVENT                   select CPU-PMU events. in %s,\n"
	"                                        you can enable at most \"%d general purpose events\"\n"
	"                                        plus \"one special 0xff (CPU_CYCLE) event\"\n";

static int cpupmu_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help, cpu_pmu->cpu_name, nr_counters - 1);
}

static int cpupmu_print_header(char *buf, int len)
{
	int i, ret, first;
	char name[32];

	first = 1;
	ret = 0;

	/*append CPU PMU access method*/
	if (met_cpu_pmu_method == 0)
		ret += snprintf(buf + ret, PAGE_SIZE,
			"met-info [000] 0.0: CPU_PMU_method: PMU registers\n");
	else
		ret += snprintf(buf + ret, PAGE_SIZE,
			"met-info [000] 0.0: CPU_PMU_method: perf APIs\n");

	/*append cache line size*/
	ret += snprintf(buf + ret, PAGE_SIZE - ret, cache_line_header, cache_line_size());

	for (i = 0; i < nr_counters; i++) {
		if (pmu[i].mode == 0)
			continue;
		if (cpu_pmu->get_event_desc && 0 == cpu_pmu->get_event_desc(i, pmu[i].event, name)) {
			if (first) {
				ret += snprintf(buf + ret, PAGE_SIZE - ret, header_n, pmu[i].event, name);
				first = 0;
			} else {
				ret += snprintf(buf + ret, PAGE_SIZE - ret, ",0x%x:%s", pmu[i].event, name);
			}
		} else {
			if (first) {
				ret += snprintf(buf + ret, PAGE_SIZE - ret, header, pmu[i].event);
				first = 0;
			} else {
				ret += snprintf(buf + ret, PAGE_SIZE - ret, ",0x%x", pmu[i].event);
			}
		}
		pmu[i].mode = 0;

	}

	ret += snprintf(buf + ret, PAGE_SIZE - ret, "\n");
	met_cpupmu.mode = 0;
	reset_driver_stat(nr_counters);
	nr_arg = 0;
	return ret;
}

/*
 * "met-cmd --start --pmu_cpu_evt=0x3"
 */
static int cpupmu_process_argument(const char *arg, int len)
{
	unsigned int value;

	if (met_cpu_pmu_method == 0)
		nr_counters = cpu_pmu->nr_cnt;
	else
		nr_counters = perf_num_counters();

	if (nr_counters == 0)
		goto arg_out;

	if (met_parse_num(arg, &value, len) < 0)
		goto arg_out;

	if (cpu_pmu->check_event(pmu, nr_arg, value) < 0)
		goto arg_out;

	if (value == 0xff) {
		if (met_cpu_pmu_method == 0) {
			pmu[nr_counters - 1].mode = MODE_POLLING;
			pmu[nr_counters - 1].event = 0xff;
			pmu[nr_counters - 1].freq = 0;
		} else {
			if (nr_arg > (nr_counters - 1))
				goto arg_out;

			pmu[nr_arg].mode = MODE_POLLING;
			pmu[nr_arg].event = value;
			pmu[nr_arg].freq = 0;
			nr_arg++;
		}
	} else {

		if (nr_arg >= (nr_counters - 1))
			goto arg_out;

		pmu[nr_arg].mode = MODE_POLLING;
		pmu[nr_arg].event = value;
		pmu[nr_arg].freq = 0;
		nr_arg++;
	}
	counter_cnt++;

	met_cpupmu.mode = 1;
	return 0;

arg_out:
	reset_driver_stat(nr_counters);
	return -EINVAL;
}

struct metdevice met_cpupmu = {
	.name = "cpu",
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
