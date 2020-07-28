/*
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
#include <string.h>
#include <driver_api.h>
#include <mt_reg_base.h>
#include <interrupt.h>
#include <mt_gpt.h>
#include <FreeRTOS.h>
#include <main.h>
#include <task.h>
#include <platform_mtk.h>

#define CLOCK_TIMER         TMR0
#define CLOCK_TIMER_IRQ     TIMER0_IRQn

//TICK_TIMER used for system tick & softtimer
#define TICK_TIMER          TMR1
#define TICK_TIMER_IRQ      TIMER1_IRQn

//DELAY_TIMER used for mdelay()/udelay()
#define DELAY_TIMER          TMR2
#define DELAY_TIMER_RSTVAL   0xffffffff

#define UNUSE1_TIMER         TMR3
#define UNUSE1_TIMER_RSTVAL  0xffffffff

#define MAX_RG_BIT         0xFFFFFFFF

#define TIMESYNC_OFFSET_BASE (0)

#define TIMESYNC_TICK_H      (TIMESYNC_OFFSET_BASE + 0)
#define TIMESYNC_TICK_L      (TIMESYNC_OFFSET_BASE + 1)
#define TIMESYNC_TS_H        (TIMESYNC_OFFSET_BASE + 2)
#define TIMESYNC_TS_L        (TIMESYNC_OFFSET_BASE + 3)
#define TIMESYNC_FREEZE      (TIMESYNC_OFFSET_BASE + 4)

#define TIMESYNC_BASE_UNIT_SIZE         (4)
#define TIMESYNC_MAX_VER                (0x7)
#define TIMESYNC_HEADER_FREEZE_OFS      (31)
#define TIMESYNC_HEADER_FREEZE          (1U << TIMESYNC_HEADER_FREEZE_OFS)
#define TIMESYNC_HEADER_VER_OFS         (28)
#define TIMESYNC_HEADER_VER_MASK        (TIMESYNC_MAX_VER << TIMESYNC_HEADER_VER_OFS)
#define TIMESYNC_HEADER_MASK            (TIMESYNC_HEADER_FREEZE | TIMESYNC_HEADER_VER_MASK)

#define TIMERSYNC_BASE_ADDR             ADSP_TIMESYC_BASE
#define OSTIMER_32BITS_TO_64BITS(val_h, val_l) \
    (((unsigned long long)val_h << 32) & 0xFFFFFFFF00000000ULL) | \
    ((unsigned long long)val_l & 0x00000000FFFFFFFFULL)

#define OSTIMER_64BITS_TO_32BITS_H(val) \
    (unsigned int)((val >> 32) & 0x00000000FFFFFFFFULL)

#define OSTIMER_64BITS_TO_32BITS_L(val) \
    (unsigned int)(val & 0x00000000FFFFFFFFULL)

#define ts_timesync_get_ver_in_reg_h(val) \
    ((((val) & TIMESYNC_HEADER_VER_MASK)) >> TIMESYNC_HEADER_VER_OFS)

#define ts_timesync_get_freeze_in_reg_h(val) \
    (((val) & TIMESYNC_HEADER_FREEZE) ? 1 : 0)

struct clock_data {
    unsigned long long epoch_ns;
    unsigned long long epoch_ns_raw;
    unsigned long long epoch_cyc;
    unsigned char      epoch_freeze;
    unsigned char      epoch_ver;
    unsigned char      insuspend;
    unsigned int       base_addr;
    unsigned int       mult;
    unsigned int       shift;
};

struct clock_data cd INTERNAL_INITIAL_DATA = {
    .epoch_ns = 0,
    .epoch_cyc = 0,
    .epoch_freeze = 0,

    /*
     * ap initial version is 0, thus set initial version in
     * co-processor as 0xFF to ensure synchronization will be
     * started.
     */
    .epoch_ver = 0xFF,

    .insuspend = 0,
    .base_addr = 0,

#ifdef CFG_FPGA
    /* 6000000's mult and shift*/
    .mult = 27962027,
    .shift = 22,
#else
    /*
     * AP sys-timer uses 13MHz clock.
     * Use 13000000's mult and shift.
     */
    .mult = 161319385,
    .shift = 21,
#endif
} ;

struct timer_device {
    unsigned int id;
    unsigned int base_addr;
    unsigned int irq_id;
};

static struct timer_device hifi3_timer[NR_TMRS] INTERNAL_NON_INITIAL_DATA;

static int timer_irq[NR_TMRS] INTERNAL_INITIAL_DATA = {TIMER0_IRQn, TIMER1_IRQn, TIMER2_IRQn, TIMER3_IRQn};
static platform_timer_callback tick_timer_callback INTERNAL_NON_INITIAL_DATA;
static void *tick_timer_callback_arg INTERNAL_NON_INITIAL_DATA;

static unsigned long long ostimer_init_cycle INTERNAL_NON_INITIAL_DATA;

/* prototype */
static struct timer_device *id_to_dev(unsigned int id) INTERNAL_FUNCTION;
static void __timer_enable_irq(struct timer_device *dev) INTERNAL_FUNCTION;
static void __timer_disable_irq(struct timer_device *dev) INTERNAL_FUNCTION;
static void __timer_ack_irq(struct timer_device *dev) INTERNAL_FUNCTION;
static void __timer_enable(struct timer_device *dev) INTERNAL_FUNCTION;
static void __timer_disable(struct timer_device *dev) INTERNAL_FUNCTION;
static void __timer_set_clk(struct timer_device *dev, unsigned int clksrc) INTERNAL_FUNCTION;
static void __timer_set_rstval(struct timer_device *dev, unsigned int val) INTERNAL_FUNCTION;
static void __timer_get_curval(struct timer_device *dev, unsigned long *ptr) INTERNAL_FUNCTION;
static void __timer_reset(struct timer_device *dev) INTERNAL_FUNCTION;

/* get ostimer counter */
static void get_ostimer_counter_high_low(unsigned int *high, unsigned int *low) INTERNAL_FUNCTION;

static void tick_timer_irq(void *arg) INTERNAL_FUNCTION;
static void tick_timer_irq_handle(void) INTERNAL_FUNCTION;
static unsigned long long get_boot_time_ns(void) INTERNAL_FUNCTION;

//===========================================================================
// delay function
//===========================================================================

static unsigned int delay_get_current_tick(void) INTERNAL_FUNCTION;
static int check_timeout_tick(unsigned int start_tick,
                              unsigned int timeout_tick) INTERNAL_FUNCTION;
static unsigned int time2tick_us(unsigned int time_us) INTERNAL_FUNCTION;
static unsigned int time2tick_ms(unsigned int time_ms) INTERNAL_FUNCTION;

//===========================================================================
// busy wait
//===========================================================================
static void busy_wait_us(unsigned int timeout_us) INTERNAL_FUNCTION;
static void busy_wait_ms(unsigned int timeout_ms) INTERNAL_FUNCTION;
#define TIMER_TICK_RATE     26000000

static unsigned int ts_timesync_base_init(void) INTERNAL_FUNCTION;
static unsigned int ts_timesync_base_read(unsigned int id) INTERNAL_FUNCTION;
static unsigned int ts_timesync_base_write(unsigned int id, unsigned int val) INTERNAL_FUNCTION;

//===========================================================================

static struct timer_device *id_to_dev(unsigned int id)
{
    return id < NR_TMRS ? hifi3_timer + id : 0;
}

static void __timer_enable_irq(struct timer_device *dev)
{
    DRV_SetReg32(dev->base_addr + TIMER_IRQ_CTRL_REG, 0x1);
}

static void __timer_disable_irq(struct timer_device *dev)
{
    DRV_ClrReg32(dev->base_addr + TIMER_IRQ_CTRL_REG, 0x1);
}

static void __timer_ack_irq(struct timer_device *dev)
{
    DRV_SetReg32(dev->base_addr + TIMER_IRQ_CTRL_REG, TIMER_IRQ_CLEAR);
}

static void __timer_enable(struct timer_device *dev)
{
    DRV_SetReg32(dev->base_addr + TIMER_EN, 0x1);
}

static void __timer_disable(struct timer_device *dev)
{
    DRV_ClrReg32(dev->base_addr + TIMER_EN, 0x1);
}

static void __timer_set_clk(struct timer_device *dev, unsigned int clksrc)
{
    DRV_ClrReg32(dev->base_addr + TIMER_CLK_SRC,
                 TIMER_CLK_SRC_MASK << TIMER_CLK_SRC_SHIFT);
    DRV_SetReg32(dev->base_addr + TIMER_CLK_SRC, clksrc << TIMER_CLK_SRC_SHIFT);
}

static void __timer_set_rstval(struct timer_device *dev, unsigned int val)
{
    DRV_WriteReg32(dev->base_addr + TIMER_RST_VAL, val);
}

static void __timer_get_curval(struct timer_device *dev, unsigned long *ptr)
{
    *ptr = DRV_Reg32(dev->base_addr + TIMER_CUR_VAL_REG);
}

static void __timer_reset(struct timer_device *dev)
{
    __timer_disable(dev);
    __timer_disable_irq(dev);
    __timer_ack_irq(dev);
    __timer_set_rstval(dev, 0);
    __timer_set_clk(dev, TIMER_CLK_SRC_CLK_26M);
}


/* get ostimer counter */
unsigned long long timer_get_global_timer_tick(void)
{
    unsigned long long val = 0;
    unsigned long high = 0, low = 0;
    unsigned long long new_high = 0, new_low = 0;
    UBaseType_t uxSavedInterruptStatus;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    low = DRV_Reg32(OSTIMER_CUR_L);
    high = DRV_Reg32(OSTIMER_CUR_H);

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    new_high = high;
    new_low = low;
    val = ((new_high << 32) & 0xFFFFFFFF00000000ULL) |
          (new_low & 0x00000000FFFFFFFFULL);

    return val;
}

/* get ostimer counter */
static void get_ostimer_counter_high_low(unsigned int *high, unsigned int *low)
{
    UBaseType_t uxSavedInterruptStatus;
    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    *low = DRV_Reg32(OSTIMER_CUR_L);
    *high = DRV_Reg32(OSTIMER_CUR_H);

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

static unsigned long long ostimer_init_cycle;
static unsigned long long last_timestamp = 0;

/* get ostimer timestamp */
static unsigned long long get_boot_time_ns(void)
{
    /* multiplier and shifter for 13MHz global timer */
#define OSTIMER_TIMER_MULT  (161319385)
#define OSTIMER_TIMER_SHIFT (21)
    unsigned int high = 0, low = 0;
    unsigned long long new_high = 0, new_low = 0;
    unsigned long long cycle = 0;
    unsigned long long timestamp = 0;

    get_ostimer_counter_high_low(&high, &low);
    new_high = high;
    new_low = low;
    cycle = ((new_high << 32) & 0xFFFFFFFF00000000ULL) | (new_low &
                                                          0x00000000FFFFFFFFULL);
    cycle = cycle - ostimer_init_cycle;
    new_high = (cycle >> 32) & 0x00000000FFFFFFFFULL;
    new_low = cycle & 0x00000000FFFFFFFFULL;
    timestamp = (((unsigned long long)new_high * OSTIMER_TIMER_MULT) << 11) +
                (((unsigned long long)new_low * OSTIMER_TIMER_MULT) >> OSTIMER_TIMER_SHIFT);
    if (timestamp < last_timestamp)
        timestamp = last_timestamp + 1;
    last_timestamp = timestamp;
    return timestamp;
}

unsigned long long read_xgpt_stamp_ns(void)
{
    return get_boot_time_ns();
}

void mt_platform_timer_init(void)
{
    int i;
    struct timer_device *dev;

    for (i = 0; i < NR_TMRS; i++) {
        hifi3_timer[i].id = i;
        hifi3_timer[i].base_addr = ADSP_TIMER_BASE + 0x10 * i;
        hifi3_timer[i].irq_id = timer_irq[i];
    }

    // reset timer
    for (i = 0; i < NR_TMRS; i++) {
        __timer_reset(&hifi3_timer[i]);
    }

    /* enable delay GPT */
    dev = id_to_dev(DELAY_TIMER);
    __timer_set_rstval(dev, DELAY_TIMER_RSTVAL);
    __timer_enable(dev);

    mdelay(1);
    /* enable ostimer */
    DRV_WriteReg32(OSTIMER_CON, 0x1);
    last_timestamp = 0;
    ostimer_init_cycle = timer_get_global_timer_tick();
    PRINTF_D("[ostimer] ostimer_init_cycle = %lld cycles\n", ostimer_init_cycle);
}
static void tick_timer_irq(void *arg)
{
    /*diff with before*/
    if (tick_timer_callback != NULL) {
        return tick_timer_callback(tick_timer_callback_arg);
    }
}

static void tick_timer_irq_handle(void)
{
    struct timer_device *dev = id_to_dev(TICK_TIMER);

    __timer_disable(dev);
    __timer_disable_irq(dev);
    __timer_ack_irq(dev);
    tick_timer_irq(0);

    return ;
}

int platform_set_periodic_timer(platform_timer_callback callback, void *arg,
                                mt_time_t interval)
{
    struct timer_device *dev;
    unsigned long long tmp_64 = 0;
    unsigned long long interval_tmp = (unsigned long long)interval;

    tick_timer_callback = callback;
    tick_timer_callback_arg  = arg;

    tmp_64 = (unsigned long long)TIMER_TICK_RATE * interval_tmp;
    //    __asm volatile("":::"cc", "memory");  //liang check
    tmp_64 = (tmp_64 / 1000ULL - 32);
    dev = id_to_dev(TICK_TIMER);
    __timer_disable(dev);

    if (interval >= 1) {
        __timer_set_rstval(dev, (unsigned int)tmp_64);    //0.3ms(sw)+0.7ms(hw wake)
    }
    else {
        __timer_set_rstval(dev, 1);
    }
    __timer_enable_irq(dev);

    request_irq(dev->irq_id, tick_timer_irq_handle, "tick_timer");

    __timer_enable(dev);

    return 0;
}

//===========================================================================
// delay function
//===========================================================================

static unsigned int delay_get_current_tick(void)
{
    unsigned long current_count;
    struct timer_device *dev = id_to_dev(DELAY_TIMER);

    __timer_get_curval(dev, &current_count);
    return current_count;
}

static int check_timeout_tick(unsigned int start_tick,
                              unsigned int timeout_tick)
{
    //register unsigned int cur_tick;
    //register unsigned int elapse_tick;
    unsigned int cur_tick;
    unsigned int elapse_tick;
    // get current tick
    cur_tick = delay_get_current_tick();

    // check elapse time, down counter
    if (start_tick >= cur_tick) {
        elapse_tick = start_tick - cur_tick;
    }
    else {
        elapse_tick = (DELAY_TIMER_RSTVAL - cur_tick) + start_tick;
    }
    // check if timeout
    if (timeout_tick <= elapse_tick) {
        // timeout
        return 1;
    }

    return 0;
}

static unsigned int time2tick_us(unsigned int time_us)
{
    return TIME_TO_TICK_US(time_us);
}

static unsigned int time2tick_ms(unsigned int time_ms)
{
    return TIME_TO_TICK_MS(time_ms);
}

//===========================================================================
// busy wait
//===========================================================================
static void busy_wait_us(unsigned int timeout_us)
{
    unsigned int start_tick, timeout_tick;

    // get timeout tick
    timeout_tick = time2tick_us(timeout_us);
    start_tick = delay_get_current_tick();

    // wait for timeout
    while (!check_timeout_tick(start_tick, timeout_tick));
}

static void busy_wait_ms(unsigned int timeout_ms)
{
    unsigned int start_tick, timeout_tick;

    // get timeout tick
    timeout_tick = time2tick_ms(timeout_ms);
    start_tick = delay_get_current_tick();

    // wait for timeout
    while (!check_timeout_tick(start_tick, timeout_tick));
}

/* delay msec mseconds */
void mdelay(unsigned long msec)
{
    busy_wait_ms(msec);
}
/* delay usec useconds */
void udelay(unsigned long usec)
{
    busy_wait_us(usec);
}

static inline unsigned long long ts_cyc_to_ns(
    unsigned long long cyc, unsigned int mult, unsigned int shift)
{
    return (cyc * mult) >> shift;
}

static unsigned int ts_timesync_base_read(unsigned int id)
{
    unsigned int val;
    unsigned int byte_ofs;

    if (ts_timesync_base_init()) {
        return 0;
    }

    byte_ofs = TIMESYNC_BASE_UNIT_SIZE * id;

    memcpy(&val, (void *)(cd.base_addr + byte_ofs), TIMESYNC_BASE_UNIT_SIZE);

    return val;
}

static unsigned int ts_timesync_base_write(unsigned int id, unsigned int val)
{
    unsigned int byte_ofs;

    if (ts_timesync_base_init()) {
        return 0;
    }

    byte_ofs = TIMESYNC_BASE_UNIT_SIZE * id;

    memcpy((void *)(cd.base_addr + byte_ofs), &val, TIMESYNC_BASE_UNIT_SIZE);

    return TIMESYNC_BASE_UNIT_SIZE;
}

static unsigned int ts_timesync_base_init(void)
{
    if (!cd.base_addr) {
        cd.base_addr = (unsigned int)TIMERSYNC_BASE_ADDR;

        /* base will be 0 if something wrong */
        if (!cd.base_addr) {
            return 1;
        }
        else {
            /*
             * reset base to avoid infinite retry in ts_timesync_update_base
             *
             * note here we assume this api will be executed before ap's sys_timer init
             */
            ts_timesync_base_write(TIMESYNC_TICK_H, 0);
            ts_timesync_base_write(TIMESYNC_TICK_L, 0);
            ts_timesync_base_write(TIMESYNC_TS_H, 0);
            ts_timesync_base_write(TIMESYNC_TS_L, 0);
        }
    }

    return 0;
}

static void ts_timesync_update_base(void)
{
    unsigned long tick_h, tick_l, ts_h, ts_l;
    unsigned char fz;
    UBaseType_t uxSavedInterruptStatus;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    tick_h = ts_timesync_base_read(TIMESYNC_TICK_H);
    tick_l = ts_timesync_base_read(TIMESYNC_TICK_L);

    cd.epoch_cyc = OSTIMER_32BITS_TO_64BITS(tick_h, tick_l);

    ts_l = ts_timesync_base_read(TIMESYNC_TS_L);
    ts_h = ts_timesync_base_read(TIMESYNC_TS_H);

    cd.epoch_ns_raw = OSTIMER_32BITS_TO_64BITS(ts_h, ts_l);
    cd.epoch_ns = OSTIMER_32BITS_TO_64BITS(ts_h, ts_l);

    fz = ts_timesync_base_read(TIMESYNC_FREEZE);

    cd.epoch_ver = 0;
    cd.epoch_freeze = fz;

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

static int ts_timesync_if_need_base_update(void)
{
    unsigned long val, fz;

    val = ts_timesync_base_read(TIMESYNC_TS_H);
    fz = ts_timesync_base_read(TIMESYNC_FREEZE);

    if ((val != (unsigned long)((cd.epoch_ns_raw >> 32) & 0xffffffff))
        || (fz != cd.epoch_freeze)) {
        return 1;
    }

    return 0;
}

int timer_sched_clock(struct timer_sys_time_t *ap_ts)
{
    unsigned long long clk;

    if (ts_timesync_if_need_base_update()) {
        ts_timesync_update_base();
    }

    clk = timer_get_global_timer_tick();

    if (cd.epoch_freeze) {
        ap_ts->ts = cd.epoch_ns;
        ap_ts->clk = clk;
        ap_ts->off_ns = ts_cyc_to_ns(clk - cd.epoch_cyc, cd.mult, cd.shift);
        ap_ts->insuspend = 1;
    }
    else {
        ap_ts->ts = cd.epoch_ns + ts_cyc_to_ns(clk - cd.epoch_cyc, cd.mult, cd.shift);
        ap_ts->clk = clk;
        ap_ts->off_ns = 0;
        ap_ts->insuspend = 0;
    }

    return ap_ts->insuspend;
}

unsigned char timer_get_ap_suspend(void)
{
    return cd.epoch_freeze;
}

unsigned long long timer_get_ap_timestamp(void)
{
    struct timer_sys_time_t systime;
    unsigned char suspend_ret = 0;
    memset(&systime, 0, sizeof(struct timer_sys_time_t));
    suspend_ret = timer_sched_clock(&systime);
    if (0 == suspend_ret) { /* APMCU in running mode */
        return systime.ts;
    }
    else {   /* APMCU in suspend mode */
        return systime.ts + systime.off_ns;
    }
}


