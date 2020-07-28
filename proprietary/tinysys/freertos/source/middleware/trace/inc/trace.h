#ifndef __TRACE_H
#define __TRACE_H

/* IPI command definition: mbox 0 */
/* main func ID: bit[31-24]; sub func ID: bit[23-18]; argu 0: bit[17-0] */
#define TRACE_MAIN_ID_MASK        0xff000000 /* bit 31 - 24 */
#define TRACE_SUB_ID_MASK         0x00fc0000 /* bit 23 - 18 */
#define TRACE_ARGU0_MASK          0x0003ffff /* bit 17 - 0 */
#define FUNC_BIT_SHIFT          18
#define MID_BIT_SHIFT           9
#define TRACE_MAIN_ID             0x06000000
/* handle argument and attribute */
#define PROCESS_ARGU            0x01
#define PROCESS_ATTR            0x02
#define MODULE_ID_MASK          0x3fe00 /* bit 9 - 17 */
#define ARGUMENT_MASK           0x01ff  /* bit 0 - 9 */

/* the following command is used for AP to MD32 */
#define TRACE_OP            1 << FUNC_BIT_SHIFT
/* argu 0: start: 0x01; stop: 0x02; extract: 0x03 */
#define TRACE_OP_START        0x00000001
#define TRACE_OP_STOP         0x00000002
#define TRACE_OP_EXTRACT      0x00000003
#define TRACE_SR            2 << FUNC_BIT_SHIFT /* sample rate */
#define TRACE_MODULE        3 << FUNC_BIT_SHIFT /* module enable/disable */
#define TRACE_ARGU          4 << FUNC_BIT_SHIFT /* argument passing */
#define TRACE_ATTR          5 << FUNC_BIT_SHIFT /* attribute passing */
/* system memory information for trace log data */
#define TRACE_BUFFER_INFO   6 << FUNC_BIT_SHIFT
#define TRACE_TIMESTAMP     7 << FUNC_BIT_SHIFT /* timestamp info */
#define TRACE_GPT           8 << FUNC_BIT_SHIFT /* GPT counter reading */
#define TRACE_REQ_AP2MD     9 << FUNC_BIT_SHIFT /* user defined command */
#define TRACE_RESP_AP2MD    10 << FUNC_BIT_SHIFT /* may no need */
/* mode: bit 15 - 0: */
/*  Bit 0: MD32 SRAM mode; Bit 1: System DRAM mode */
/*  value: 0: output to next level of storage; 1: loop in its own storage */
#define TRACE_DATA_MODE     11 << FUNC_BIT_SHIFT /* log output mode */
#define DATA_MODE_LOOP 0
#define DATA_MODE_DUMP 1
/* start/stop read data into MD32 SRAM buffer; both DMA and */
#define TRACE_DATA_OP       12 << FUNC_BIT_SHIFT /* data read operation */
/* the following command is used for MD32 to AP */
#define TRACE_DUMP_BUFFER   13 << FUNC_BIT_SHIFT
#define TRACE_REQ_MD2AP     14 << FUNC_BIT_SHIFT /* user defined command */
#define TRACE_CLOSE_FILE    15 << FUNC_BIT_SHIFT /* Inform to close the SD file */
#define TRACE_RESP_MD2AP    16 << FUNC_BIT_SHIFT /* may no need */
#define TRACE_RUN_MODE      17 << FUNC_BIT_SHIFT

/* Note: the module ID and its bit pattern should be fixed as below */
/* DMA based module first */
enum {
    MID_RTOS = 9,
    MID_TRACE_TAG,
    MID_TS,
    MID_TS_ISR,
    MID_TS_LAST,
    MID_TS_ISR_LAST,
    MID_SRAM_INFO,
    MID_TRACE_STOP,
};

#define ID_RTOS        1 << MID_RTOS

/* note: bit 0 - 11: module ID */
/* bit 12 - 19: data length (exclude DATA_START_MARK and timestamp) */
/* #define DATA_START_MARK 0xFFF00000 */
#define DATA_START_MARK 0xFFE00000 /* support TS_bit */
#define TS_ISR_BIT 0x00100000
#define TS_TASK_BIT 0x00000000
#define EOR_TASK_MARK 0xdeadbeef
#define EOR_ISR_MARK 0xbeefdead

/* for delta timestamp: DATA_START_MARK:1, timestamp:1, ts module:3 */
/* #define TRACE_RECORD_ADDED_LENGTH 5 */
#define TRACE_RECORD_ADDED_LENGTH 5+3 /* for duplicate TS record */


struct tracedevice {
    struct tracedevice *next;
    void (*start)(void);
    void (*stop)(void);
    /*int (*reset)(void);*/
    void (*timed_polling)(void);
    void (*process_argument)(unsigned int argu, unsigned int m2, unsigned int m3);
    void (*process_attribute)(unsigned int attr, unsigned int m2, unsigned int m3);
    /* 1 tick is 100us now, need to increase bits for "second" interval */
    unsigned short polling_interval;
    unsigned short polling_count_reload;
    unsigned short polling_count;
    char id;
    char mode;
};

extern void trace_register(struct tracedevice *trace);

extern void trace_process_options(int cmd, unsigned int m1, unsigned int m2, unsigned int m3);
extern void trace_sample_rate(unsigned int rate);
extern int trace_set_sample_rate(int mid, int rate);
extern void trace_module_setting(unsigned int m);
extern void trace_start();
extern void trace_stop();
extern void trace_extract(void);

/* the CTX_xxx is used for timestamp function which has two versions */
#define CTX_ISR  0
#define CTX_TASK 1
#define CTX_STOP 2
#define TRACE_PRINTB_WAIT 0 /* wait until the buffer is available */
/* if the buffer isn't available, discard the data */
#define TRACE_PRINTB_DISCARD 1
#define TRACE_PRINTB_OVERWRITE 2
/* return -1 when the record is discarded */
/* note: the unit of len is 32 bits */
extern int trace_printb(int mid, unsigned int *buf, int len, int mode);
extern int trace_printb_isr(int mid, unsigned int *buf, int len_unit_uint32);
#define trace_printb_wait(mid, buf, len_unit_uint32) trace_printb(mid, buf, len_unit_uint32, TRACE_PRINTB_WAIT)
#define trace_printb_nowait(mid, buf, len_unit_uint32) trace_printb(mid, buf, len_unit_uint32, TRACE_PRINTB_DISCARD)
extern unsigned long long trace_get_timestamp(int ctx);
extern void vMetPollingTask(void *pvParameters);

extern volatile int sample_rate;
extern volatile char trace_started;
extern volatile char trace_log_mode;
#define TRACE_LOG_FILE 0
#define TRACE_LOG_DRAM 2
#define TRACE_LOG_SRAM 1
extern volatile char trace_run_mode;
#define SSPM_RUN_NORMAL 0
#define SSPM_RUN_CONTINUOUS 1


/* set it to use polling function, otherwise use polling task */
#define USE_POLLING_FUNCTION


/* Compensate the timestamp when APMCU in suspend/resume state */
extern void trace_apmcu_suspend_handler(unsigned long long ap_ts, unsigned long long ap_clk);
extern void trace_apmcu_resume_handler(unsigned long long ap_ts, unsigned long long ap_clk);

#endif /* __ONDIETRACE_H */
