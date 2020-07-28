#include "trace.h"
#include "trace_rtos_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void freertos_start(void);
void freertos_stop(void);
static void freertos_process_argument(unsigned int argu, unsigned int m2, unsigned int m3);
/* for FreeRTOS context switch trace output */

struct task_create_event_t task_create_event[FREERTOS_MAX_TASKS];
struct timer_create_event_t timer_create_event[FREERTOS_MAX_TIMER];
struct queue_create_event_t queue_create_event[FREERTOS_MAX_QUEUES];
void *queue_handle[FREERTOS_MAX_QUEUES];
int trace_rtos_task_no = 0;
int trace_rtos_queue_no = 0;
int trace_rtos_swtimer_no = 0;
int trace_current_task_no = 0;
unsigned long long trace_rtos_ts = 0;
unsigned long long trace_rtos_ts_diff = 0;
int trace_rtos_ts_nesting = 0;

union trace_hook_mode trace_rtos;
struct trace_rtos_hook_t trace_rtos_hook_error;
#if 0
#define RTOS_TASK       0x01
#define RTOS_QUEUE      0x04
#define RTOS_SWTIMER    0x10
#define RTOS_IRQ        0x40
#define RTOS_SYS        0x100 /* suspend, resume */
#define RTOS_TICK       0x400
#endif
//static unsigned short features = RTOS_SWTIMER;
static unsigned short features;// = RTOS_TASK|RTOS_IRQ;
unsigned short rtos_flags;
union trace_hook_mode flag;

#define REPORT_TASK_STACK_USAGE
#ifdef REPORT_TASK_STACK_USAGE
//static void MyTaskList(void *pvParameters);
TaskStatus_t pcWriteBuffer[20];

/* usStackHighWaterMark: The minimum amount of stack space that has remained
   for the task since the task was created. */
/* The closer this value is to zero the closer the task has come to overflowing its stack. */
void trace_get_stack_usage()
{
    volatile UBaseType_t uxArraySize, x, taskno;

    taskno = uxTaskGetNumberOfTasks();
    uxArraySize = uxTaskGetSystemState(pcWriteBuffer, taskno, NULL);

    /* PRINTF_D("trace_get_stack_usage: task number=%d\n", taskno); */
    for (x = 0; x < uxArraySize; x++) {
#if 0
        PRINTF_D("task[%d]:%s, HighWater_mark=%d\n", pcWriteBuffer[x].xTaskNumber, pcWriteBuffer[x].pcTaskName,
                 pcWriteBuffer[x].usStackHighWaterMark);
#endif
        task_create_event[pcWriteBuffer[x].xTaskNumber - 1].watermark = pcWriteBuffer[x].usStackHighWaterMark;
    }

    /* PRINTF_D("\n"); */
}

#endif

/* the features of RTOS is defined as below setting */
#if 0
#define RTOS_TASK       0x01
#define RTOS_QUEUE      0x04
#define RTOS_SWTIMER    0x10
#define RTOS_IRQ        0x40
#define RTOS_SYS        0x100 /* suspend, resume */
#define RTOS_TICK       0x400
#endif

#if 0
void print_freertos_task_create_event()
{
    int i;

    PRINTF_D("print_freertos_task_create_event: tasks_no=%d\n", trace_rtos_task_no);

    for (i = 0; i < trace_rtos_task_no; i++) {
        PRINTF_D("taskno:%d, priority:%d, name:%s\n", task_create_event[i].taskno, task_create_event[i].priority,
                 task_create_event[i].name);
    }
}
#endif

#if 0
void print_freertos_timer_create_event()
{
    int i;

    PRINTF_D("print_freertos_timer_create_event: trace_TimerID=%d\n", trace_rtos_swtimer_no);

    for (i = 0; i < trace_rtos_swtimer_no; i++) {
        PRINTF_D("evtype:%d, timerno:%d, name:%s\n", timer_create_event[i].evtype, timer_create_event[i].timerno,
                 timer_create_event[i].name);
    }
}
#endif

struct tracedevice trace_freertos = {
    .next = 0,
    .id = MID_RTOS,
    .mode = 0,
    .start = freertos_start,
    .stop = freertos_stop,
    .process_argument = freertos_process_argument,
};

static void freertos_process_argument(unsigned int argu, unsigned int m2, unsigned int m3)
{
    features |= (unsigned short)m2;
}
void freertos_set_features(uint32_t input)
{
    features = input;
}

void freertos_start(void)
{
    int i;
    struct task_switch_event_t ev;

    if (features != 0) {
        taskENTER_CRITICAL();
        // trace_freertos.mode = 1;
        rtos_flags = 0;

        /* generate the first context switch record */
        ev.taskno = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
        ev.evtype = EV_TASK_SWTCH_1;
        trace_printb_isr(MID_RTOS | MID_RTOS_TASK_SWITCH, (unsigned int *)&ev, 1);

        /* get current Q dlen */
        for (i = 0; i < trace_rtos_queue_no; i++) {
            queue_create_event[i].dlen = uxQueueMessagesWaiting(queue_handle[i]);
        }

        trace_rtos.all = features;
        trace_started = 1;
        taskEXIT_CRITICAL();
    } else
        trace_freertos.mode = 0;
}

void freertos_stop(void)
{

    taskENTER_CRITICAL();
    flag.all = trace_rtos.all;
    trace_rtos.all = 0; /* clear all hook enable */
    features = 0;
    trace_started = 0;
    taskEXIT_CRITICAL();
    /* trace_freertos.mode = 0; */
}

void trace_dump_freertos_info(void)
{
    unsigned int task_info[1];
    uint8_t size;

#ifdef REPORT_TASK_STACK_USAGE
    trace_get_stack_usage();
#endif
    /* always output task information */
    size = sizeof(struct task_create_event_t) / sizeof(unsigned int);
    task_info[0] = (trace_rtos_task_no << 16) | (configMAX_TASK_NAME_LEN << 8) | size;
#if 0
    taskENTER_CRITICAL();
    trace_printb_isr(MID_RTOS | MID_RTOS_TASKINFO, task_info, 1);
    trace_printb_isr(MID_RTOS | MID_RTOS_TASK_CREATE, (unsigned int *)&task_create_event, trace_rtos_task_no * size);
    taskEXIT_CRITICAL();
#else
    trace_printb(MID_RTOS | MID_RTOS_TASKINFO, task_info, 1, 0);
    trace_printb(MID_RTOS | MID_RTOS_TASK_CREATE, (unsigned int *)&task_create_event, trace_rtos_task_no * size, 0);
#endif

    /* output timer information if it has */
    if (flag.hook.swtimer != 0) {
        size = sizeof(struct timer_create_event_t) / sizeof(unsigned int);
        task_info[0] = (trace_rtos_swtimer_no << 16) | (configMAX_TASK_NAME_LEN << 8) | size;
#if 0
        taskENTER_CRITICAL();
        trace_printb_isr(MID_RTOS | MID_RTOS_SWTIMER_INFO, task_info, 1);
        trace_printb_isr(MID_RTOS | MID_RTOS_SWTIMER_CREATE, (unsigned int *)&timer_create_event, trace_rtos_swtimer_no * size);
        taskEXIT_CRITICAL();
#else
        trace_printb(MID_RTOS | MID_RTOS_SWTIMER_INFO, task_info, 1, 0);
        trace_printb(MID_RTOS | MID_RTOS_SWTIMER_CREATE, (unsigned int *)&timer_create_event, trace_rtos_swtimer_no * size, 0);
#endif
    }
    /* output queue information if it has */
    if (flag.hook.queue != 0) {
        size = sizeof(struct queue_create_event_t) / sizeof(unsigned int);
        task_info[0] = (trace_rtos_queue_no << 16) | (configMAX_TASK_NAME_LEN << 8) | size;
#if 0
        taskENTER_CRITICAL();
        trace_printb_isr(MID_RTOS | MID_RTOS_QUEUE_INFO, task_info, 1);
        trace_printb_isr(MID_RTOS | MID_RTOS_QUEUE_CREATE, (unsigned int *)&queue_create_event, trace_rtos_queue_no * size);
        taskEXIT_CRITICAL();
#else
        trace_printb(MID_RTOS | MID_RTOS_QUEUE_INFO, task_info, 1, 0);
        trace_printb(MID_RTOS | MID_RTOS_QUEUE_CREATE, (unsigned int *)&queue_create_event, trace_rtos_queue_no * size, 0);
#endif
    }
}

void traceINT_ON_LOG(unsigned int evtype, unsigned int mask)
{
    trace_rtos_ts_diff = trace_get_timestamp(CTX_ISR) - trace_rtos_ts;
    if (trace_rtos_ts_diff > configTRACE_INT_MASK_TIME_BOUND_NS) {
        //PRINTF_E_ISR("XXX traceINT_ON_LOG: type=%x, [%llu:%llu]\n", evtype, trace_rtos_ts/1000000000, trace_rtos_ts & 0x0ffffffffLL);
        struct int_event_t ev;
        ev.evtype = evtype;
        ev.mask = mask;
        ev.duration_high = (unsigned int)(trace_rtos_ts_diff >> 32);
        ev.duration_low = (unsigned int)(trace_rtos_ts_diff & 0x0ffffffffLL);
        trace_printb_isr(MID_RTOS | MID_RTOS_INT, (unsigned int *)&ev, 4);
    }
    // trace_rtos_ts = 0;
}
