#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include "trace.h"
#include "trace_tag.h"
#include "trace_log.h"

#define TRACE_MAX_TAGS 10

/* if trace_taged, will output the tag name information */
static char trace_taged;
static char trace_tag_count;
char trace_tag_initialized = 0;

struct trace_tag_t trace_tag[TRACE_MAX_TAGS];

void trace_tag_initialize()
{
    int i;

    if (trace_tag_initialized == 0) {
        for (i = 0; i < TRACE_MAX_TAGS; i++) {
            trace_tag[i].used = 0;
        }
        trace_taged = 0;
        trace_tag_count = 0;
        trace_tag_initialized = 1;
    }
}

int trace_tag_create(char *name)
{
    int i;

    if (name == NULL)
        return -1;
    if (trace_tag_initialized == 0)
        trace_tag_initialize();
    for (i = 0; i < TRACE_MAX_TAGS; i++) {
        if (trace_tag[i].used == 0) {
            trace_tag_count++;
            trace_tag[i].used = 1;
            memcpy(trace_tag[i].name, name, 7);
            return i;
        }
    }
    return -2;
}

/* Note: support ISR tagging: when ISR buffer is available */
void trace_tag_call(int id, unsigned int data, int ctx, int mode)
{
    struct trace_tag_data_t buf;

    configASSERT(trace_tag_initialized == 1);
    if (trace_started == 1) {
        trace_taged = 1;
        if ((id >= 0) && (id < TRACE_MAX_TAGS)) {
            buf.id = id;
            buf.taskno = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
            buf.data = data & 0x0000ffff;
            if (ctx == CTX_ISR)
                trace_printb_isr(MID_TRACE_TAG | mode, (unsigned int *)&buf, 1);
            else {
                taskENTER_CRITICAL();
                trace_printb_isr(MID_TRACE_TAG | mode, (unsigned int *)&buf, 1);
                taskEXIT_CRITICAL();
            }
        }
    }
}

void trace_tag_output_info()
{
    int size;
    unsigned int info[1];

    if (trace_taged == 1) {
        size = sizeof(struct trace_tag_t) / sizeof(unsigned int);
        // info[0] = (trace_tag_count << 8) | size;
        info[0] = trace_tag_count;
        trace_printb(MID_TRACE_TAG | MID_TAG_INFO, info, 1, 0);
        trace_printb(MID_TRACE_TAG | MID_TAG_CREATE, (unsigned int *)&trace_tag, trace_tag_count * size, 0);
    }
}

