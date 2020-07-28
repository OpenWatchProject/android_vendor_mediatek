/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "trace.h"
#include "trace_log.h"
#ifdef TRACE_TAG
#include "trace_tag.h"
#endif
#include "scp_ipi.h"


volatile char trace_started = 0;
/* log mode: 1: loop in SRAM; 2: loop in DRAM; 0: write to file system */
volatile char trace_log_mode = TRACE_LOG_SRAM; /* default is 0 */
volatile char trace_run_mode = 0; /* default is 0 */


typedef struct {
    uint32_t MARKER1;
    uint32_t MARKER2;
    uint32_t trace_data_start;
    uint32_t trace_data_end;
    uint32_t cpu_id;
    uint32_t configuration;
} trace_info_t;

static trace_info_t trace_info;
extern void freertos_start(void);
extern void freertos_stop(void);
extern void freertos_set_features(uint32_t);
#define RTOS_TASK       0x01
#define RTOS_QUEUE      0x04
#define RTOS_SWTIMER    0x10
#define RTOS_IRQ        0x40
#define RTOS_SYS        0x100 /* suspend, resume */
#define RTOS_TICK       0x400

struct timesync_ctrl_s {
    unsigned int base;
    unsigned int size;
    unsigned int ts_h;
    unsigned int ts_l;
    unsigned int clk_h;
    unsigned int clk_l;
};
static struct timesync_ctrl_s ts_ctl;


struct clock_data {
    unsigned long long epoch_ns;
    unsigned long long epoch_cyc;
    unsigned int mult;
    unsigned int shift;
    unsigned int slave_mult;
    unsigned int slave_shift;
    int insuspend;
};

extern struct clock_data cd;

static void send_trace_info(void)
{
    PRINTF_D("CPU:%d start to trace.\n\r", trace_info.cpu_id);
    scp_ipi_send(IPI_TRACE_SCP, &trace_info, sizeof(trace_info), 0, IPI_SCP2AP);
}
void timer_handler(int id, void* data, unsigned int len)
{
    struct timesync_ctrl_s *ts_ctl_ptr;

    ts_ctl_ptr = (struct timesync_ctrl_s *)data;
    memcpy(&ts_ctl, ts_ctl_ptr, sizeof(struct timesync_ctrl_s));
    PRINTF_D("in %s =>%u, %u\n", __func__, ts_ctl.ts_h, ts_ctl.ts_l);
    cd.epoch_ns = (((unsigned long long)ts_ctl_ptr->ts_h << 32) & 0xFFFFFFFF00000000) |
                  (ts_ctl_ptr->ts_l & 0x00000000FFFFFFFF);
    cd.epoch_cyc = (((unsigned long long)ts_ctl_ptr->clk_h << 32) & 0xFFFFFFFF00000000) |
                   (ts_ctl_ptr->clk_l & 0x00000000FFFFFFFF);
}
void trace_handler(int id, void* data, unsigned int len)
{
    uint32_t *trace_ipi_buf;

    trace_ipi_buf = (uint32_t *)data;
    PRINTF_D("in %s, trace handler:%x\n", __func__, trace_ipi_buf[0]);
    switch (trace_ipi_buf[0]&TRACE_SUB_ID_MASK) {
        case TRACE_OP:
            if ((trace_ipi_buf[0] & 0x0000000f) == TRACE_OP_START) {
                if (0 == trace_started) {
                    xTimerPendFunctionCallFromISR((PendedFunction_t) reset_log_index,
                                                  NULL,
                                                  0,
                                                  NULL);
                    freertos_set_features(RTOS_TASK | RTOS_IRQ);
                    xTimerPendFunctionCallFromISR((PendedFunction_t) freertos_start,
                                                  NULL,
                                                  0,
                                                  NULL);
                    xTimerPendFunctionCallFromISR((PendedFunction_t) send_trace_info,
                                                  NULL,
                                                  0,
                                                  NULL);
                }
            } else if ((trace_ipi_buf[0] & 0x0000000f) == TRACE_OP_STOP) {
                if (1 == trace_started) {
                    xTimerPendFunctionCallFromISR((PendedFunction_t) freertos_stop,
                                                  NULL,
                                                  0,
                                                  NULL);
                }
            } else if ((trace_ipi_buf[0] & 0x0000000f) == TRACE_OP_EXTRACT) {
                xTimerPendFunctionCallFromISR((PendedFunction_t) trace_extract,
                                              NULL,
                                              0,
                                              NULL);


            }
            break;
        default:
            PRINTF_D("in %s, no command:%x\n", __func__, trace_ipi_buf[0]);
            break;
    }
}

int trace_main(void)
{
#ifdef TRACE_TAG
    trace_tag_initialize();
#endif
    vTRACE_log_init();

    trace_info.MARKER1 = 0x5A5ADEAD;
    trace_info.MARKER2 = 0x5A5ABEEF;
    trace_info.trace_data_start = (uint32_t)&trace_log_sram[0];
    trace_info.trace_data_end = (uint32_t)&trace_log_sram[0] + TRACE_LOG_SIZE * 4;
    trace_info.cpu_id = *(unsigned int*)0x409C01FC;
    PRINTF_D("trace_data_start:0x%x, trace_data_end:0x%x \n\r",
             trace_info.trace_data_start,
             trace_info.trace_data_end);

    scp_ipi_registration(IPI_TRACE_SCP, (ipi_handler_t)trace_handler,
                         "IPI_TRACE_SCP");
    scp_ipi_registration(IPI_SCP_TIMER, (ipi_handler_t)timer_handler,
                         "IPI_SCP_TIMER");
    return (0);
}
