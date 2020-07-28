/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "trace.h"
#include "dma.h"
#include "trace_log.h"

/* #define MYDBG */ /* output debug message */
/* #define MYDBG_DMA *//* output DMA debug message */

static unsigned int trace_dram_addr, trace_dram_size, trace_dram_water_mark;
/* used for ring buffer mgm of DRAM buffer */
static unsigned int trace_dram_left;
trace_dram_idx_t trace_dram_ridx;
trace_dram_idx_t trace_dram_widx;

unsigned int trace_log_sram[TRACE_LOG_SIZE];

/* used for ring buffer mgm of log buffer */
unsigned short trace_sram_size, trace_sram_water_mark, trace_sram_left;
trace_sram_idx_t trace_sram;

static int dram2SD_read_idx, dram2SD_dump_size;

// SemaphoreHandle_t xSemaDram;
SemaphoreHandle_t xSemaSramDMA, xSemaDramDMA;
SemaphoreHandle_t xSemaDump;

unsigned short trace_log_isr_discard_num;
trace_log_update_mgm_t trace_log_update;

#define WRAP_NONE 0
#define WRAP_SRC  1
#define WRAP_DEST 2

void reset_log_index()
{
    trace_sram.ridx = 0;
    trace_sram.widx = 0;
    trace_sram_size = TRACE_LOG_SIZE;
    trace_sram_left = trace_sram_size;
    trace_sram_water_mark = trace_sram_size >> 1;

    trace_dram_ridx.idx = 0;
    trace_dram_widx.idx = 0;
    trace_dram_addr = 0;
    dram2SD_read_idx = 0;
    dram2SD_dump_size = 0;

    trace_log_update.dumping_to_file = 0;
    trace_log_isr_discard_num = 0;
    trace_log_task_discard_num = 0;

    memset(trace_log_sram, 0, TRACE_LOG_SIZE * 4);
}

void vTRACE_log_init()
{
    reset_log_index();
}

void setup_dram(unsigned int addr, unsigned int size)
{
    trace_dram_addr = addr;
    trace_dram_size = size >> 2; /* unit is 32-bits */
    trace_dram_water_mark = trace_dram_size >> 1; /* half the size */
    trace_dram_left = trace_dram_size;
}
