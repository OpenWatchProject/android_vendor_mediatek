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

#include <asm/page.h>
#include "interface.h"
#include "met_drv.h"

#ifdef	CONFIG_GPU_TRACEPOINTS
#include <trace/events/gpu.h>

#define show_secs_from_ns(ns) \
	({ \
		u64 t = ns + (NSEC_PER_USEC / 2); \
		do_div(t, NSEC_PER_SEC); \
		t; \
	})

#define show_usecs_from_ns(ns) \
	({ \
		u64 t = ns + (NSEC_PER_USEC / 2) ; \
		u32 rem; \
		do_div(t, NSEC_PER_USEC); \
		rem = do_div(t, USEC_PER_SEC); \
	})

static int event_gpu_registered;
static int event_gpu_enabled;

noinline void gpu_sched_switch(const char *gpu_name, u64 timestamp,
				      u32 next_ctx_id, s32 next_prio, u32 next_job_id)
{
	MET_TRACE("gpu_name=%s ts=%llu.%06lu next_ctx_id=%lu next_prio=%ld next_job_id=%lu\n",
		   gpu_name,
		   (unsigned long long)show_secs_from_ns(timestamp),
		   (unsigned long)show_usecs_from_ns(timestamp),
		   (unsigned long)next_ctx_id, (long)next_prio, (unsigned long)next_job_id);
}

MET_DEFINE_PROBE(gpu_sched_switch, TP_PROTO(const char *gpu_name, u64 timestamp,
		u32 next_ctx_id, s32 next_prio, u32 next_job_id))
{
	gpu_sched_switch(gpu_name, timestamp, next_ctx_id, next_prio, next_job_id);
}

noinline void gpu_job_enqueue(u32 ctx_id, u32 job_id, const char *type)
{
	MET_TRACE("ctx_id=%lu job_id=%lu type=%s",
		   (unsigned long)ctx_id, (unsigned long)job_id, type);
}

MET_DEFINE_PROBE(gpu_job_enqueue, TP_PROTO(u32 ctx_id, u32 job_id, const char *type))
{
	gpu_job_enqueue(ctx_id, job_id, type);
}
#endif

static int reset_driver_stat(void)
{
#ifdef	CONFIG_GPU_TRACEPOINTS
	event_gpu_enabled = 0;
#endif
	met_trace_event.mode = 0;
	return 0;
}



static void met_event_start(void)
{
#ifdef	CONFIG_GPU_TRACEPOINTS
	/* register trace event for gpu */
	do {
		if (!event_gpu_enabled)
			break;
		if (MET_REGISTER_TRACE(gpu_sched_switch)) {
			pr_debug("can not register callback of gpu_sched_switch\n");
			break;
		}
		if (MET_REGISTER_TRACE(gpu_job_enqueue)) {
			pr_debug("can not register callback of gpu_job_enqueue\n");
			MET_UNREGISTER_TRACE(gpu_sched_switch);
			break;
		}
		event_gpu_registered = 1;
	} while (0);
#endif
}

static void met_event_stop(void)
{
#ifdef	CONFIG_GPU_TRACEPOINTS
	/* unregister trace event for gpu */
	if (event_gpu_registered) {
		MET_UNREGISTER_TRACE(gpu_job_enqueue);
		MET_UNREGISTER_TRACE(gpu_sched_switch);
		event_gpu_registered = 0;
	}
#endif
}

static int met_event_process_argument(const char *arg, int len)
{
	int	ret = -1;

#ifdef	CONFIG_GPU_TRACEPOINTS
	if (strcasecmp(arg, "gpu") == 0) {
		event_gpu_enabled = 1;
		met_trace_event.mode = 1;
		ret = 0;
	}
#endif

	return ret;
}

static const char help[] = "\b"
#ifdef	CONFIG_GPU_TRACEPOINTS
	"  --event=gpu                           output gpu trace events\n"
#endif
	;

static int met_event_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static const char header[] =
	"met-info [000] 0.0: met_ftrace_event:"
#ifdef	CONFIG_GPU_TRACEPOINTS
	" gpu:gpu_sched_switch gpu:gpu_job_enqueue"
#endif
	"\n";

static int met_event_print_header(char *buf, int len)
{
	int	ret;

	ret = snprintf(buf, PAGE_SIZE, header);
	return ret;
}

struct metdevice met_trace_event = {
	.name			= "event",
	.type			= MET_TYPE_PMU,
	.start			= met_event_start,
	.stop			= met_event_stop,
	.reset			= reset_driver_stat,
	.process_argument	= met_event_process_argument,
	.print_help		= met_event_print_help,
	.print_header		= met_event_print_header,
};
