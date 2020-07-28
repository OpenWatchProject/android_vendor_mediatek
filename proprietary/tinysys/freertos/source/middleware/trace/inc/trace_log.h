#ifndef _TRACE_LOG_H_
#define _TRACE_LOG_H_

typedef struct _trace_log_update_mgm {
    unsigned int dump_from_isr : 3,
             dump_from_task    : 3,
             dumping_to_file   : 3,
             unused            : 23;
} trace_log_update_mgm_t;

extern trace_log_update_mgm_t trace_log_update;

/* here we assume the DRAM size is 4M bytes, so the index need 20 bits */
typedef struct _trace_dram_idx {
    unsigned int idx  : 20,
             unused: 12;
} trace_dram_idx_t;

#define LOG_SIZE_BIT 12 /*2^n*/
#define TRACE_LOG_SIZE (1<<LOG_SIZE_BIT)
/* here we assume the SRAM size is 4K bytes */
/* so the index need 10 bits */
//#define TRACE_LOG_SIZE      (512*2*4) /* 512*2*4(byte) = 4K bytes */
typedef struct _trace_sram_idx {
unsigned int ridx  :
    LOG_SIZE_BIT,
widx  :
    LOG_SIZE_BIT,
    unused: 32 - LOG_SIZE_BIT * 2;
} trace_sram_idx_t;

extern unsigned short trace_log_task_discard_num, trace_log_isr_discard_num;
extern unsigned int trace_log_sram[];
/* used for ring buffer mgm of log buffer */
extern trace_sram_idx_t trace_sram;
extern unsigned short trace_sram_size, trace_sram_water_mark, trace_sram_left;

extern SemaphoreHandle_t xSemaSram;
extern SemaphoreHandle_t xSemaSramDMA;
extern SemaphoreHandle_t xSemaDump;
extern void vFlushSram2Dram(void);
extern void vFlushDram2File(void);
extern void vCloseFile(void);
extern void setup_dram(unsigned int addr, unsigned int size);

extern void vTRACE_log_init(void);
extern void DRAM2Flashdone();
extern void reset_log_index(void);
extern void trace_log_sram_allocation(unsigned int m);

#endif /* _TRACE_LOG_H_ */
