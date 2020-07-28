#ifndef HIFI3_IRQ_H
#define HIFI3_IRQ_H

typedef enum {
    UART_IRQn      = 0,
    DBG_IRQn       = 1,
    OS_TIMER_IRQn  = 2,
    TIMER0_IRQn    = 3,
    TIMER1_IRQn    = 4,
    GENERAL_IRQn   = 5,
    SPM_IRQn       = 6,
    IPC0_IRQn      = 8, //SW_int 0
    AP_AWAKE_IRQn  = 9, //SW_int 1
    AUDIO_IRQn     = 10,
    USB0_IRQn      = 11,
    USB1_IRQn      = 12,
    CCIF3_IRQn     = 13,
    DMA_IRQn       = 14,
    TIMER2_IRQn    = 15,
    TIMER3_IRQn    = 16,
    LATENCY_MON_IRQn = 17,
    SW2_INT_IRQn     = 20,
    SW3_INT_IRQn     = 21,
    BUS_TRACKER_IRQn = 22,
    INFRA_HANG_IRQn  = 24,
    WDT_IRQn         = 25,
    IRQ_MAX_CHANNEL  = 26
} IRQn_Type;
/*
 *   SYS_TIMER0_IRQn  = 26,
     SYS_TIMER1_IRQn  = 27,
     PMU_IRQn         = 28,
     WRITE_ERR_IRQn   = 29,
     SYS_SW_INT_IRQn  = 30,
     NMI_IRQn         = 31
*/
#endif /*HIFI3_IRQ_H  */
