/* Copyright Statement:
 *
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

#include <driver_api.h>
#include <mt_reg_base.h>
#include <interrupt.h>
#include <mt_gpt.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <FreeRTOSConfig.h>
#include <main.h>
#include <task.h>
#include <platform_mtk.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <mt_gpt.h>

static struct timer_device scp_timer[NR_TMRS];

static platform_timer_callback tick_timer_callback;
static void *tick_timer_callback_arg;

#ifdef CFG_CHRE_SUPPORT
static platform_timer_callback chre_timer_callback;
static void *chre_timer_callback_arg;
#endif

// 200MHz setting
#define MCLK_RATE           200*1024*1024     //200M
#define MCLK_1TICK_NS       5       //  1/(200M) = 5nsec

#define SEC_TO_NSEC         (1000 * 1000 * 1000)

#define TIMER_TICK_RATE     32768

/* multiplier and shifter for 13MHz global timer */
#define OSTIMER_TIMER_MULT          (161319385)
#define OSTIMER_TIMER_SHIFT         (21)
#define SYSTICK_CLOCK_HZ            32768

#ifdef OSTIMER_LATCH_TIME_SUPPORT
void init_latch_time(void);
#else
void init_latch_time(void)
{
}
#endif

struct timer_device *id_to_dev(unsigned int id)
{
    return id < NR_TMRS ? scp_timer + id : 0;
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
    DRV_ClrReg32(dev->base_addr + TIMER_CLK_SRC, TIMER_CLK_SRC_MASK << TIMER_CLK_SRC_SHIFT);
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
    __timer_set_clk(dev, TIMER_CLK_SRC_CLK_32K);
}

/* get ostimer counter */
unsigned long long timer_get_global_timer_tick(void)
{
    unsigned long long val = 0;
    unsigned long high = 0, low = 0;
    unsigned long long new_high = 0, new_low = 0;

    if (!is_in_isr())
        taskENTER_CRITICAL();

    low = DRV_Reg32(OSTIMER_CUR_L);
    high = DRV_Reg32(OSTIMER_CUR_H);

    if (!is_in_isr())
        taskEXIT_CRITICAL();

    new_high = high;
    new_low = low;
    val = ((new_high << 32) & 0xFFFFFFFF00000000ULL) | (new_low & 0x00000000FFFFFFFFULL);

    return val;
}

/* get ostimer counter */
static void get_ostimer_counter_high_low(unsigned int *high, unsigned int *low)
{
    if (!is_in_isr())
        taskENTER_CRITICAL();

    *low = DRV_Reg32(OSTIMER_CUR_L);
    *high = DRV_Reg32(OSTIMER_CUR_H);

    if (!is_in_isr())
        taskEXIT_CRITICAL();
}

static unsigned long long ostimer_init_cycle;
static unsigned long long last_timestamp = 0;

/* get ostimer timestamp */
unsigned long long get_boot_time_ns(void)
{
    unsigned int high = 0, low = 0;
    unsigned long long new_high = 0, new_low = 0;
    unsigned long long cycle = 0;
    unsigned long long timestamp = 0;

    get_ostimer_counter_high_low(&high, &low);
    new_high = high;
    new_low = low;
    cycle = ((new_high << 32) & 0xFFFFFFFF00000000ULL) | (new_low & 0x00000000FFFFFFFFULL);
    cycle = cycle - ostimer_init_cycle;
    new_high = (cycle >> 32) & 0x00000000FFFFFFFFULL;
    new_low = cycle & 0x00000000FFFFFFFFULL;
    timestamp = (((unsigned long long)new_high * OSTIMER_TIMER_MULT) << 11) +
                (((unsigned long long)new_low * OSTIMER_TIMER_MULT) >> OSTIMER_TIMER_SHIFT);
    if (!is_in_isr())
        taskENTER_CRITICAL();

    if (timestamp < last_timestamp)
        timestamp = last_timestamp + 1;
    last_timestamp = timestamp;

    if (!is_in_isr())
        taskEXIT_CRITICAL();

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

    /* enable clock */
    DRV_SetReg32(TIMER_IN_CLK, (TMR_MCLK_CG | TMR_BCLK_CG));

    for (i = 0; i < NR_TMRS; i++) {
        scp_timer[i].id = i;
        scp_timer[i].base_addr = SCP_TIMER_BASE + 0x10 * i;
        scp_timer[i].irq_id = TIMER0_IRQn + i;
    }

    // reset timer
    for (i = 0; i < NR_TMRS; i++) {
        __timer_reset(&scp_timer[i]);
    }
#ifdef CFG_VCORE_DVFS_SUPPORT
    scp_wakeup_src_setup(TIMER0_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(TIMER1_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(TIMER2_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(TIMER3_WAKE_CLK_CTRL, 1);
    scp_wakeup_src_setup(TIMER4_WAKE_CLK_CTRL, 1);
#endif

    /* enable delay GPT */
    dev = id_to_dev(DELAY_TIMER);
    __timer_set_rstval(dev, DELAY_TIMER_RSTVAL);
    __timer_enable(dev);

    mdelay(1);

    /* enable ostimer */
    DRV_WriteReg32(OSTIMER_CON, 0x1);
    last_timestamp = 0;
    ostimer_init_cycle = timer_get_global_timer_tick();
    printf("[ostimer] ostimer_init_cycle = %lld cycles\n", ostimer_init_cycle);

    init_latch_time();
}

static void tick_timer_irq(void *arg)
{
    /*diff with before*/
    if (tick_timer_callback != NULL)
        return tick_timer_callback(tick_timer_callback_arg);
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

int platform_set_periodic_timer(platform_timer_callback callback, void *arg, mt_time_t interval)
{
    struct timer_device *dev;
    unsigned long long tmp_64 = 0;

    tick_timer_callback = callback;
    tick_timer_callback_arg  = arg;

    tmp_64 = (unsigned long long)TIMER_TICK_RATE * interval;
    __asm volatile("":::"cc", "memory");
    tmp_64 = (tmp_64 - TIMER_TICK_RATE * portMCU_WAKEUP_COST) / configTICK_RATE_HZ;

    dev = id_to_dev(TICK_TIMER);
    __timer_disable(dev);

    if (interval >= 1)
        __timer_set_rstval(dev, (unsigned int)tmp_64); //0.3ms(sw)+0.7ms(hw wake)
    else
        __timer_set_rstval(dev, 1);

    __timer_enable_irq(dev);
    request_irq(dev->irq_id, tick_timer_irq_handle, "tick_timer");
    __timer_enable(dev);

    return 0;
}

#ifdef CFG_CHRE_SUPPORT
static void chre_timer_stop(void)
{
    struct timer_device *dev = id_to_dev(CHRE_TIMER);

    __timer_disable(dev);
}

static void chre_timer_irq(void *arg)
{
    if (chre_timer_callback != NULL)
        return chre_timer_callback(chre_timer_callback_arg);
}

static void chre_timer_irq_handle(void)
{
    struct timer_device *dev = id_to_dev(CHRE_TIMER);
    __timer_disable(dev);
    __timer_disable_irq(dev);
    __timer_ack_irq(dev);
    chre_timer_irq(0);

    return ;
}

static int platform_set_periodic_timer_chre(platform_timer_callback callback, void *arg, mt_time_t interval_ns)
{
    struct timer_device *dev;
    unsigned long long interval_tick = 0;

    chre_timer_callback = callback;
    chre_timer_callback_arg  = arg;

    /* calculate how many ticks shall we wait */
    interval_tick = (interval_ns / 1000) * (unsigned long long)TIMER_TICK_RATE / 1000000ULL;

    __asm volatile("":::"cc", "memory");

    dev = id_to_dev(CHRE_TIMER);

    /* setup 1-tick timer if required tick is < 1 */
    if (interval_tick >= 1)
        __timer_set_rstval(dev, (unsigned int)interval_tick);
    else
        __timer_set_rstval(dev, 1);

    __timer_enable_irq(dev);

    request_irq(dev->irq_id, chre_timer_irq_handle, "chre_timer");

    __timer_enable(dev);

    return 0;
}
#endif

static unsigned int delay_get_current_tick(void)
{
    unsigned long current_count;
    struct timer_device *dev = id_to_dev(DELAY_TIMER);

    __timer_get_curval(dev, &current_count);
    return current_count;
}

static int check_timeout_tick(unsigned int start_tick, unsigned int timeout_tick)
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
    } else {
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
    unsigned long usec_t;
    if (usec < US_LIMIT) {
        //PRINTF_D("usec < 31us, error parameter\n");
        busy_wait_us(1);
    } else {
        usec_t = usec / 31 + 1;
        busy_wait_us(usec_t);
    }
}

#ifdef CFG_CHRE_SUPPORT
//===========================================================================
// chre feature
//===========================================================================
extern uint64_t cpuIntsOff(void);
extern void cpuIntsRestore(uint64_t state);
extern TaskHandle_t CHRE_TaskHandle;

void chre_timer_wakeup()
{
    extern int timIntHandler(void);
    timIntHandler();
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE)
        portYIELD_WITHIN_API();
}
unsigned int platSleepClockRequest(uint64_t wakeupTime, uint32_t maxJitterPpm,
                                   uint32_t maxDriftPpm, uint32_t maxErrTotalPpm)
{
    // uint64_t intState, curTime;
    uint64_t curTime;
    unsigned long long diff_time;
    chre_timer_stop();
    if (wakeupTime == 0) {
        return 1;
    }
    curTime = (uint64_t)read_xgpt_stamp_ns();

    if (wakeupTime && curTime >= wakeupTime)
        return 0;

    /* diff_time unit: ns */
    diff_time = wakeupTime - curTime;

    // intState = cpuIntsOff();
    // TODO: set an actual alarm here so that if we keep running and do not sleep till this is due,
    // we still fire an interrupt for it!
    platform_set_periodic_timer_chre(chre_timer_wakeup, NULL, diff_time);
    // cpuIntsRestore(intState);
    return 1;
}
#endif

#ifdef OSTIMER_LATCH_TIME_SUPPORT
#define MAX_LATCH_TIMER     3
#define CRTL_BIT_SHIFT      8
#define IRQ_BIT_SHIFT       0
#define ENABLE_BIT_SHIFT    5
#define CYC_BASE_SHIFT      8
struct latch_time_struct {
    unsigned int ctrl_base;
    unsigned int enable_offset;
    unsigned int irq_offset;
    unsigned int cyc_low_base;
    unsigned int cyc_high_base;
};
static struct latch_time_struct latch_time[MAX_LATCH_TIMER];
static unsigned int latch_time_used[MAX_LATCH_TIMER];
void init_latch_time(void)
{
    int i = 0;
    for (i = 0; i < MAX_LATCH_TIMER; ++i) {
        latch_time[i].ctrl_base = OS_TIMER_LATCH_CTRL;
        latch_time[i].enable_offset = i * CRTL_BIT_SHIFT + ENABLE_BIT_SHIFT;
        latch_time[i].irq_offset = i * CRTL_BIT_SHIFT + IRQ_BIT_SHIFT;
        latch_time[i].cyc_low_base = OS_TIMER_LATCH_VALUE_0 + i * CYC_BASE_SHIFT;
        latch_time[i].cyc_high_base = latch_time[i].cyc_low_base + 4;

        latch_time_used[i] = 0;
    }
    DRV_WriteReg32(OS_TIMER_LATCH_CTRL, 0);
}
void enable_latch_time(int id, int irq)
{
    unsigned int base = 0, control = 0;

    if (id < 0 || id >= MAX_LATCH_TIMER)
        return;
    if (irq > 0x1f)
        return;
    base = latch_time[id].ctrl_base;
    control = ((1 << latch_time[id].enable_offset) |
               (irq << latch_time[id].irq_offset));
    if (!is_in_isr())
        taskENTER_CRITICAL();
    DRV_SetReg32(base, control);
    if (!is_in_isr())
        taskEXIT_CRITICAL();
}
void disable_latch_time(int id)
{
    unsigned int base = 0, control = 0;

    if (id < 0 || id >= MAX_LATCH_TIMER)
        return;
    base = latch_time[id].ctrl_base;
    control = ((0 << latch_time[id].enable_offset) |
               (0 << latch_time[id].irq_offset));
    if (!is_in_isr())
        taskENTER_CRITICAL();
    DRV_SetReg32(base, control);
    if (!is_in_isr())
        taskEXIT_CRITICAL();
}
int alloc_latch_time(void)
{
    int i = 0, id = -1;
    if (!is_in_isr())
        taskENTER_CRITICAL();
    for (i = 0; i < MAX_LATCH_TIMER; ++i) {
        if (latch_time_used[i] == 0) {
            latch_time_used[i] = 1;
            id = i;
            break;
        }
    }
    if (!is_in_isr())
        taskEXIT_CRITICAL();
    return id;
}
void free_latch_time(int id)
{
    if (id < 0 || id >= MAX_LATCH_TIMER)
        return;
    if (!is_in_isr())
        taskENTER_CRITICAL();
    latch_time_used[id] = 0;
    if (!is_in_isr())
        taskEXIT_CRITICAL();
}
static unsigned long long get_latch_time_counter(struct latch_time_struct *base)
{
    unsigned long long val;
    unsigned long high_1, high_2, low_1, low_2;

    if (!is_in_isr())
        taskENTER_CRITICAL();

    low_1 = DRV_Reg32(base->cyc_low_base);
    high_1 = DRV_Reg32(base->cyc_high_base);
    low_2 = DRV_Reg32(base->cyc_low_base);
    high_2 = DRV_Reg32(base->cyc_high_base);

    if (low_2 < low_1) {
        high_1 = high_2;
        low_1 = low_2;
    }

    val = (((unsigned long long)high_1 << 32) & 0xFFFFFFFF00000000) |
          ((unsigned long long)low_1 & 0x00000000FFFFFFFF);

    if (!is_in_isr())
        taskEXIT_CRITICAL();

    return val;
}
uint64_t get_latch_time_timestamp(int id)
{
    unsigned long long cycle = 0;
    unsigned long long high = 0, low = 0;
    uint64_t timestamp = 0;

    if (id < 0 || id >= MAX_LATCH_TIMER)
        return 0;
    cycle = get_latch_time_counter(&latch_time[id]);

    cycle = cycle - ostimer_init_cycle;
    high = (cycle >> 32) & 0x00000000FFFFFFFFULL;
    low = cycle & 0x00000000FFFFFFFFULL;
    timestamp = (((unsigned long long)high * OSTIMER_TIMER_MULT) << 11) +
                (((unsigned long long)low * OSTIMER_TIMER_MULT) >> OSTIMER_TIMER_SHIFT);
    return timestamp;
}
#endif
