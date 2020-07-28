#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#ifdef OSHOOK_TEST
#undef trace_printb
#undef trace_printb_isr
#undef trace_get_timestamp
#endif

#include "trace.h"
#include "trace_log.h"
#ifdef TRACE_TAG
#include "trace_tag.h"
#endif
#ifdef CFG_FREERTOS_TRACE_SUPPORT
#include "mt_gpt.h"
#else
#include <ostimer.h>
#endif

static struct tracedevice *trace_dev_root = NULL;
unsigned short trace_log_task_discard_num;
static char polling_enable;
unsigned long long trace_last_ts;

#define trace_printb_DELAY ((portTickType) 10000 / portTICK_RATE_MS)

extern SemaphoreHandle_t xSemaTRACEPolling;
extern TimerHandle_t xTRACEPollTimer;
extern void trace_dump_freertos_info(void);
extern void end_dump_to_dram(unsigned int *src, int size);

/* Compensate the timestamp when APMCU in suspend/resume state */
unsigned long long apmcu_suspend_offset = 0ULL;
static unsigned long long apmcu_suspend_ts = 0ULL;
static unsigned long long apmcu_suspend_clk = 0ULL;

void trace_apmcu_suspend_handler(unsigned long long ap_ts, unsigned long long ap_clk)
{
    apmcu_suspend_ts = ap_ts;
    apmcu_suspend_clk = ap_clk;
}

void trace_apmcu_resume_handler(unsigned long long ap_ts, unsigned long long ap_clk)
{
    apmcu_suspend_offset += ((ap_clk - apmcu_suspend_clk) * 1 / 13000000) - (ap_ts - apmcu_suspend_ts);
}

static void trace_apmcu_clr_suspend_offset(void)
{
    apmcu_suspend_offset = 0ULL;
    apmcu_suspend_ts = 0ULL;
    apmcu_suspend_clk = 0ULL;
}

#if 0 /* it seems no needed */
static unsigned long long trace_apmcu_get_suspend_offset(void)
{
    return apmcu_suspend_offset;
}
#endif
#if 1
struct sys_time_t {
    unsigned long long ts;
    unsigned long long off_ns;
    unsigned long long clk;
    int insuspend;
};
struct clock_data {
    unsigned long long epoch_ns;
    unsigned long long epoch_cyc;
    unsigned int mult;
    unsigned int shift;
    unsigned int slave_mult;
    unsigned int slave_shift;
    int insuspend;
};

struct clock_data cd = {
    .epoch_ns = 0,
    .epoch_cyc = 0,
    /*
     * AP System Timer uses 13MHz while AP is awake.
     * Use 13000000's mult and shift.
     */
    .mult = 161319385,
    .shift = 21,
    /*
     * AP System Timer has auto compensation mechanism in 32KHz zone
     * during system suspend, thus use 13000000's mult and shift as well.
     */
    .slave_mult = 161319385,
    .slave_shift = 21,
    .insuspend = 0,
};

static unsigned long long cyc_to_ns(unsigned long long cyc, unsigned int mult, unsigned int shift)
{
    return (cyc * mult) >> shift;
}
static unsigned long long apxgpt_get_cnt(void)
{
    unsigned long long cnt[2];
    unsigned long long val;


    cnt[0] = *(unsigned int*)(0x409C308C);
    cnt[1] = *(unsigned int*)(0x409C3090);

    return val = ((cnt[1] << 32) & 0xFFFFFFFF00000000) | (0x00000000FFFFFFFF & cnt[0]);
}
static unsigned long long read_sched_clock(void)
{
    return apxgpt_get_cnt();
}

static unsigned long long __sched_clock(void)
{

    unsigned long long epoch_ns;
    unsigned long long epoch_cyc;
    unsigned long long cyc;

    epoch_cyc = cd.epoch_cyc;
    epoch_ns = cd.epoch_ns;

    cyc = read_sched_clock();
    cyc = cyc - epoch_cyc;

    return epoch_ns + cyc_to_ns(cyc, cd.mult, cd.shift);
}
unsigned long long wall_sched_clock(void)
{
    return __sched_clock();
}
int ts_apmcu_time(struct sys_time_t* ap_ts)
{

    ap_ts->ts = __sched_clock();
    ap_ts->clk = 0;
    ap_ts->off_ns = 0;
    ap_ts->insuspend = 0;
    return 0;
}

#endif
unsigned long long trace_get_timestamp_tmp(void)
{
    struct sys_time_t apmcu_ts;
    int ret;

    ret = ts_apmcu_time(&apmcu_ts);

    if (0 == ret) { /* APMCU in running mode */
        return apmcu_ts.ts + apmcu_suspend_offset;
        // return apmcu_ts.ts;
    } else { /* APMCU in suspend mode */
        return apmcu_ts.ts + apmcu_ts.off_ns + apmcu_suspend_offset;
        // return (apmcu_ts.ts + apmcu_ts.off_ns);
    }
}
unsigned long long trace_get_timestamp(int ctx)
{
#ifdef CFG_FREERTOS_TRACE_SUPPORT
    return trace_get_timestamp_tmp();
#else
    struct sys_time_t apmcu_ts;
    int ret;

    if (ctx == CTX_ISR)
        ret = ts_apmcu_time_isr(&apmcu_ts);
    else /* CTX_TASK */
        ret = ts_apmcu_time(&apmcu_ts);

    if (0 == ret) { /* APMCU in running mode */
        return apmcu_ts.ts + apmcu_suspend_offset;
        // return apmcu_ts.ts;
    } else { /* APMCU in suspend mode */
        return apmcu_ts.ts + apmcu_ts.off_ns + apmcu_suspend_offset;
        // return (apmcu_ts.ts + apmcu_ts.off_ns);
    }
#endif
}

void trace_register(struct tracedevice *trace)
{
    struct tracedevice *c;


    if (trace_dev_root == NULL) { /* no registered yet */
        trace_dev_root = trace;
    } else { /* add to the next field of trace_dev_root */
        c = trace_dev_root->next;
        trace_dev_root->next = trace;
        trace->next = c;
    }
}

static void trace_add_ts()
{
    trace_log_sram[trace_sram.widx++] = DATA_START_MARK | MID_TS_ISR;
    trace_log_sram[trace_sram.widx++] = (unsigned int)(trace_last_ts >> 32);
    trace_log_sram[trace_sram.widx++] = (unsigned int)(trace_last_ts & 0x0ffffffffLL);
}

int trace_set_sample_rate(int mid, int rate)
{
    struct tracedevice *c;

    for (c = trace_dev_root; c != NULL; c = c->next) {
        if ((c->id) == mid) {
            c->polling_count_reload = rate;
            return 0;
        }
    }

    return -1;
}

void trace_module_setting(unsigned int m)
{
    struct tracedevice *c;
    c = trace_dev_root;

    /* PRINTF_D("trace_module_settig: 0x%x\n", m); */

    while (c != NULL) {
        if (((1 << c->id) & m) == (1 << c->id)) {
            c->mode = 1;
        } else {
            c->mode = 0;
        }

        c = c->next;
    }
}

/* m1: module ID (9 bits: 17--9), argument ID (9 bits: 8--0) */
void trace_process_options(int cmd, unsigned int m1, unsigned int m2, unsigned int m3)
{
    int id;
    struct tracedevice *c;

    id = (m1 & MODULE_ID_MASK) >> MID_BIT_SHIFT;

    for (c = trace_dev_root; c != NULL; c = c->next) {
        if ((c->id) == id) {
            if (cmd == PROCESS_ARGU) {
                if (c->process_argument != NULL) {
                    c->process_argument(m1, m2, m3);
                }
            } else {
                if (c->process_attribute != NULL) {
                    c->process_attribute(m1, m2, m3);
                }
            }

            break;
        }
    }
}

void trace_start(void)
{
    struct tracedevice *c;

    /* reset suspend offset time */
    trace_apmcu_clr_suspend_offset();
    /* output the timestamp, so that the delta value can be calcaulated */
    trace_last_ts = trace_get_timestamp(CTX_TASK);
    trace_add_ts();

    PRINTF_D("trace_starting: log_mode=%d, run_mode=%d\n", trace_log_mode, trace_run_mode);

    for (c = trace_dev_root; c != NULL; c = c->next) {
        if ((c->mode) && (c->start)) {
            if (c->timed_polling != NULL)
                polling_enable = 1;

            c->start();
        }
    }

    if (polling_enable == 1)
        xTimerStart(xTRACEPollTimer, portMAX_DELAY);

}

void trace_stop(void)
{
    struct tracedevice *c;

    if (polling_enable == 1)
        xTimerStop(xTRACEPollTimer, portMAX_DELAY);

    for (c = trace_dev_root; c != NULL; c = c->next) {
        if ((c->mode) && (c->stop)) {
            c->stop();
            c->mode = 0;
        }
    }

    polling_enable = 0;
    /* record the stop timestamp */
    trace_last_ts = trace_get_timestamp(CTX_TASK);
    PRINTF_D("trace_stopped\n");
    /* reset suspend offset time */
    trace_apmcu_clr_suspend_offset();
}

/* flash SRAM to System memory, then to SD card */
void trace_extract(void)
{
    unsigned int buf[6];
#if 0
    int i;
#endif
    (void) buf;
    /* DMA: dump all the sram data to dram */
    if (trace_log_mode == TRACE_LOG_SRAM) { /* DRAM loop mode */
        /* add task and ISR SRAM info */
        trace_log_sram[0] = DATA_START_MARK | MID_SRAM_INFO;
        trace_log_sram[1] = TRACE_LOG_SIZE - 24 / 4;
        trace_log_sram[2] = 0;
        trace_log_sram[3] = DATA_START_MARK | MID_TRACE_STOP;
        trace_log_sram[4] = (unsigned int)(trace_last_ts >> 32);
        trace_log_sram[5] = (unsigned int)(trace_last_ts & 0x0ffffffffLL);
        trace_log_sram[trace_sram.widx++] = EOR_TASK_MARK;
#ifdef TRACE_TAG
        trace_tag_output_info();
#endif
        trace_dump_freertos_info();
    } else {

    }
#if 0
    for (i = 0; i < 0x1000; i++)
        PRINTF_D("0x%x\n", trace_log_sram[i]);
#endif

    PRINTF_D("trace_extracted\n");
}

static void trace_printb_cp_comm(int mid, unsigned int *buf, int len)
{
    int i;
    unsigned long long ts;
    unsigned long long ts_diff;
    unsigned int delta;

    ts = trace_get_timestamp(CTX_ISR);
    ts_diff = ts - trace_last_ts;

    if (ts_diff > 0x0ffffffff) {
        /* for duplicate TS record */
        if (trace_log_mode != TRACE_LOG_FILE) {
            trace_add_ts();
        }
        trace_last_ts = ts;
        trace_add_ts();
        delta = 0;
    } else {
        delta = (unsigned int)(ts_diff & 0x0ffffffffLL);
    }

    trace_log_sram[trace_sram.widx++] = DATA_START_MARK | ((len << 12) & 0x000ff000) | mid | TS_ISR_BIT;
    trace_log_sram[trace_sram.widx++] = delta;
    for (i = 0; i < len; i++) {
        memcpy(&trace_log_sram[trace_sram.widx++], &buf[i], 4);
    }
    if (trace_sram.widx >= trace_sram.ridx) {
        trace_sram_left = trace_sram_size - (trace_sram.widx - trace_sram.ridx);
    } else {
        trace_sram_left = trace_sram.ridx - trace_sram.widx;
    }
}

int trace_printb(int mid, unsigned int *buf, int len, int mode)
{
    unsigned long long ts;

    (void) ts;
    if (len > (TRACE_LOG_SIZE - TRACE_RECORD_ADDED_LENGTH))
        return -1;

    if ((trace_log_mode != TRACE_LOG_SRAM) && (trace_sram_left < (TRACE_RECORD_ADDED_LENGTH + len))) {
        if (mode == TRACE_PRINTB_DISCARD) {
            trace_log_task_discard_num++;
            return -2;
        }

        /* wait until DMA: SRAM to DRAM is done */
        /* PRINTF_D("Polling Wait until DMA is done SR:%d, SW:%d. SL:%d \n", trace_sram.ridx, trace_sram.widx, trace_sram_left); */

        if (xSemaphoreTake(xSemaSramDMA, trace_printb_DELAY) != pdTRUE) {
            return -3;
        }
    }

    taskENTER_CRITICAL();
    trace_printb_isr(mid, buf, len);
    taskEXIT_CRITICAL();
    return 0;
}

int trace_printb_isr(int mid, unsigned int *buf, int len)
{
    int ret;
    unsigned long long ts;
    unsigned long ulSavedInterruptMask;

    (void) ulSavedInterruptMask;
    if (len > (TRACE_LOG_SIZE - TRACE_RECORD_ADDED_LENGTH))
        return -1;

#ifdef TRACE_TRACE_ENABLE /* disable profiling if it has */

    if (0 != trace_rtos.all) {
        rtos_flags = trace_rtos.all;
        ulSavedInterruptMask = portSET_INTERRUPT_MASK_FROM_ISR();
        trace_rtos.all = 0;
    }

#endif

    if (trace_log_mode != TRACE_LOG_SRAM) {
        if (trace_sram_left < (TRACE_RECORD_ADDED_LENGTH + len)) {
            trace_log_isr_discard_num++;
            ret = -2;
            goto trace_printb_isr_exit;
        }

        trace_printb_cp_comm(mid, buf, len);

        if (trace_sram_left < trace_sram_water_mark) {
            /* kick the dump task */
            xSemaphoreGiveFromISR(xSemaDump, NULL);
        }
    } /* log_mode */
    else { /* SRAM looping mode */
        trace_printb_cp_comm(mid, buf, len);
        if (trace_sram.widx > trace_sram_water_mark) {
            ts = trace_get_timestamp(CTX_ISR);
            trace_add_ts();
            trace_last_ts = ts;
            trace_add_ts();
        }
    }

    ret = 0;
trace_printb_isr_exit:
#ifdef TRACE_TRACE_ENABLE /* enable profiling if it has */

    if (0 != rtos_flags) {
        trace_rtos.all = rtos_flags;
        rtos_flags = 0;
        portCLEAR_INTERRUPT_MASK_FROM_ISR_AND_LEAVE(ulSavedInterruptMask);
    }

#endif
    return ret;
}

