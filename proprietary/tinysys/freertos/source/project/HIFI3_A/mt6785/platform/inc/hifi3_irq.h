#ifndef HIFI3_IRQ_H
#define HIFI3_IRQ_H

typedef enum {
    SPM_IRQn         = 0,
    DBG_IRQn         = 1,
    OS_TIMER_IRQn    = 2,
    TIMER0_IRQn      = 3,
    TIMER1_IRQn      = 4,
    IPC0_IRQn        = 5,
    AP_AWAKE_IRQn    = 6,
    SW2_INT_IRQn     = 7,
    CONNSYS1_IRQn    = 8,  // BTCVSD
    CONNSYS2_IRQn    = 9,  // A2DP
    CONNSYS3_IRQn    = 10, // ISO
    USB0_IRQn        = 11,
    USB1_IRQn        = 12,
    CCIF3_IRQn       = 13,
    DMA_IRQn         = 14,
    AUDIO_IRQn       = 15,
    HIFI3_WDT_IRQn   = 16,
    RESERVED_IRQn    = 17,
    TIMER2_IRQn      = 18,
    TIMER3_IRQn      = 19,
    SW3_INT_IRQn     = 20,
    LATENCY_MON_IRQn = 21,
    BUS_TRACKER_IRQn = 22,
    WAKEUP_SRC_IRQn  = 23,
    INFRA_HANG_IRQn  = 24,
    WDT_IRQn         = 25,
    IRQ_MAX_CHANNEL  = 26
} IRQn_Type;

#endif /*HIFI3_IRQ_H  */
