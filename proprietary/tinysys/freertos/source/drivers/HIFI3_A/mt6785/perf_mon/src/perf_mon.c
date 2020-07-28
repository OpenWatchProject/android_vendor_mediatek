/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2018
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtensa/xt_perfmon.h>
#include <xtensa_rtos.h>
#include <xtensa_api.h>
#include "interrupt.h"
#include "mt_printf.h"
#include "perf_mon.h"

#define NUM_OF_PERF_COUNTER (8)
#define NUM_OF_PRESETS      (32)

#define PMADDR_PMG        (0x101000)
#define PMADDR_INTPC      (0x101010)
#define PMADDR_PM(n)      (0x101080 + ((n) << 2))
#define PMADDR_PMCTRL(n)  (0x101100 + ((n) << 2))
#define PMADDR_PMSTAT(n)  (0x101180 + ((n) << 2))

#define PERF_MON_INTERRUPT_INDEX    28
#define PERF_MON_INTERRUPT_MASK     (0x1 << PERF_MON_INTERRUPT_INDEX)

#define PMG_PMEN        0x00000001  /* Overall enable for all performance counting */
#define PMSTAT_OVFL     0x00000001  /* Counter Overflow. Sticky bit set when a counter rolls over from 0xffffffff to 0x0. */
#define PMSTAT_INTASRT  0x00000010  /* This counter's overflow caused PerfMonInt to be asserted. */

#define PMCTRL_CONFIG_MASK      ((PMCTRL_SEL_MASK << PMCTRL_SEL_SHIFT) | (PMCTRL_MSK_MASK << PMCTRL_MSK_SHIFT) | (PMCTRL_TRL_MASK << PMCTRL_TRL_SHIFT) | (PMCTRL_KNC_MASK << PMCTRL_KNC_SHIFT))
#define GET_PMCTRL_SEL(config)  (((config) >> PMCTRL_SEL_SHIFT) & PMCTRL_SEL_MASK)
#define GET_PMCTRL_MSK(config)  (((config) >> PMCTRL_MSK_SHIFT) & PMCTRL_MSK_MASK)
#define GET_PMCTRL_TRL(config)  (((config) >> PMCTRL_TRL_SHIFT) & PMCTRL_TRL_MASK)
#define GET_PMCTRL_KNC(config)  (((config) >> PMCTRL_KNC_SHIFT) & PMCTRL_KNC_MASK)

#define ENTER_SESSION(status)   do {                \
    if (is_in_isr()) {                              \
        status = portSET_INTERRUPT_MASK_FROM_ISR(); \
    } else {                                        \
        taskENTER_CRITICAL();                       \
    }                                               \
} while(0)

#define LEAVE_SESSION(status)   do {                \
    if (is_in_isr()) {                              \
        portCLEAR_INTERRUPT_MASK_FROM_ISR(status);  \
    } else {                                        \
        taskEXIT_CRITICAL();                        \
    }                                               \
} while(0)

#define PERF_MON_VERBOSE

#ifdef PERF_MON_VERBOSE
#define PRINT_PERF_MON_E(x...)  PRINTF_E(x)
#else   /* PERF_MON_VERBOSE */
#define PRINT_PERF_MON_E(x...)
#endif  /* PERF_MON_VERBOSE */

static const uint32_t perf_mon_default_config_table[NUM_OF_PERF_COUNTER] = {
    SET_PMCTRL(PMSEL_I_MEM     , PMMSK_I_MEM_CACHE_MISSES  , 0xF, 0),
    SET_PMCTRL(PMSEL_I_MEM     , PMMSK_I_MEM_CACHE_HITS    , 0xF, 0),
    SET_PMCTRL(PMSEL_D_LOAD_U1 , PMMSK_D_LOAD_CACHE_MISSES , 0xF, 0),
    SET_PMCTRL(PMSEL_D_LOAD_U1 , PMMSK_D_LOAD_CACHE_HITS   , 0xF, 0),
    SET_PMCTRL(PMSEL_D_STORE_U1, PMMSK_D_STORE_CACHE_MISSES, 0xF, 0),
    SET_PMCTRL(PMSEL_D_STORE_U1, PMMSK_D_STORE_CACHE_HITS  , 0xF, 0),
    SET_PMCTRL(PMSEL_PREFETCH  , PMMSK_PREFETCH_I_MISS     , 0xF, 0),
    SET_PMCTRL(PMSEL_PREFETCH  , PMMSK_PREFETCH_D_MISS     , 0xF, 0)
};

static const uint32_t perf_mon_preset_config_table[NUM_OF_PRESETS] = {
    SET_PMCTRL(PMSEL_I_MEM     , PMMSK_I_MEM_CACHE_MISSES  , 0xF, 0),   /* bit 0 */
    SET_PMCTRL(PMSEL_I_MEM     , PMMSK_I_MEM_CACHE_HITS    , 0xF, 0),   /* bit 1 */
    SET_PMCTRL(PMSEL_D_LOAD_U1 , PMMSK_D_LOAD_CACHE_MISSES , 0xF, 0),   /* bit 2 */
    SET_PMCTRL(PMSEL_D_LOAD_U1 , PMMSK_D_LOAD_CACHE_HITS   , 0xF, 0),   /* bit 3 */
    SET_PMCTRL(PMSEL_D_STORE_U1, PMMSK_D_STORE_CACHE_MISSES, 0xF, 0),   /* bit 4 */
    SET_PMCTRL(PMSEL_D_STORE_U1, PMMSK_D_STORE_CACHE_HITS  , 0xF, 0),   /* bit 5 */
    SET_PMCTRL(PMSEL_PREFETCH  , PMMSK_PREFETCH_I_MISS     , 0xF, 0),   /* bit 6 */
    SET_PMCTRL(PMSEL_PREFETCH  , PMMSK_PREFETCH_D_MISS     , 0xF, 0),   /* bit 7 */
    0,                                                                  /* bit 8 */
    0,                                                                  /* bit 9 */
    0,                                                                  /* bit 10 */
    0,                                                                  /* bit 11 */
    0,                                                                  /* bit 12 */
    0,                                                                  /* bit 13 */
    0,                                                                  /* bit 14 */
    0,                                                                  /* bit 15 */
    0,                                                                  /* bit 16 */
    0,                                                                  /* bit 17 */
    0,                                                                  /* bit 18 */
    0,                                                                  /* bit 19 */
    0,                                                                  /* bit 20 */
    0,                                                                  /* bit 21 */
    0,                                                                  /* bit 22 */
    0,                                                                  /* bit 23 */
    0,                                                                  /* bit 24 */
    0,                                                                  /* bit 25 */
    0,                                                                  /* bit 26 */
    0,                                                                  /* bit 27 */
    0,                                                                  /* bit 28 */
    0,                                                                  /* bit 29 */
    0,                                                                  /* bit 30 */
    0                                                                   /* bit 31 */
};

static uint32_t perf_mon_config_array[NUM_OF_PERF_COUNTER];
static volatile uint32_t perf_mon_counter_hi32[NUM_OF_PERF_COUNTER];
static uint32_t perf_mon_counter_in_use = 0;    // for each bit, 1: occupied, 0: empty
static uint32_t perf_mon_start_flag = 0;
static uint32_t perf_mon_default_flag = 0;

static inline uint32_t RER (uint32_t addr)
{
    uint32_t value;
    __asm__ __volatile__ ("rer %0, %1" : "=a" (value) : "a" (addr) : "memory");
    return value;
}

static inline void WER (uint32_t addr, uint32_t value)
{
    __asm__ __volatile__ ("wer %0, %1" : : "a" (value), "a" (addr) : "memory");
}

static uint64_t perf_mon_read_counter_64bit(int32_t idx)
{
    uint32_t hi1 = perf_mon_counter_hi32[idx];
    uint32_t lo1 = RER(PMADDR_PM(idx));
    uint32_t hi2 = perf_mon_counter_hi32[idx];
    uint32_t lo2 = RER(PMADDR_PM(idx));
    uint64_t cnt_val;
    if (hi1 == hi2) {
        cnt_val = ((uint64_t)hi1 << 32) | lo1;
    } else {
        cnt_val = ((uint64_t)hi2 << 32) | lo2;
    }
    return cnt_val;
}

static void perf_mon_interrupt_handler (void *arg)
{
    uint32_t pmg = RER(PMADDR_PMG);
    int32_t idx;
    WER(PMADDR_PMG, pmg & ~PMG_PMEN);
    for (idx = 0; idx < NUM_OF_PERF_COUNTER; idx++) {
        uint32_t stat = RER(PMADDR_PMSTAT(idx));
        if (stat & PMSTAT_INTASRT) {
            WER(PMADDR_PMSTAT(idx), PMSTAT_INTASRT);
        }
        if (stat & PMSTAT_OVFL) {
            perf_mon_counter_hi32[idx]++;
            WER(PMADDR_PMSTAT(idx), PMSTAT_OVFL);
        }
    }
    WER(PMADDR_PMG, pmg);
}

static void perf_mon_enable (void)
{
    xt_ints_on(PERF_MON_INTERRUPT_MASK);
    WER(PMADDR_PMG, RER(PMADDR_PMG) | PMG_PMEN);
}

static void perf_mon_disable (void)
{
    WER(PMADDR_PMG, RER(PMADDR_PMG) & ~PMG_PMEN);
    xt_ints_off(PERF_MON_INTERRUPT_MASK);
}

static void perf_mon_clear_counter_values (void)
{
    int32_t idx;
    for (idx = 0; idx < NUM_OF_PERF_COUNTER; idx++) {
        WER(PMADDR_PMSTAT(idx), PMSTAT_OVFL | PMSTAT_INTASRT);
        WER(PMADDR_PM(idx), 0);
        perf_mon_counter_hi32[idx] = 0;
    }
}

static void perf_mon_clear_all (void)
{
    int32_t idx;
    for (idx = 0; idx < NUM_OF_PERF_COUNTER; idx++) {
        WER(PMADDR_PMCTRL(idx), 0);
        perf_mon_counter_in_use &= ~(0x1 << idx);
    }
    perf_mon_clear_counter_values();
}

static uint64_t perf_mon_get_counter_64bit_value (int32_t idx)
{
    uint64_t value;
    if (idx >= NUM_OF_PERF_COUNTER || idx < 0 || (perf_mon_counter_in_use & (0x1 << idx)) == 0) {
        value = 0;
    } else {
        value = perf_mon_read_counter_64bit(idx);
    }
    return value;
}

static perf_mon_status_t perf_mon_set_counter_64bit (uint32_t config)
{
    perf_mon_status_t result;
    uint32_t idx;
    for (idx = 0; idx < NUM_OF_PERF_COUNTER; idx++) {   /* Get available counter id */
        if ((perf_mon_counter_in_use & (0x1 << idx)) == 0) {
            break;
        }
    }
    if (idx >= NUM_OF_PERF_COUNTER ||                               /* No available counter (all in use) */
        (GET_PMCTRL_SEL(config) == PMSEL_OVERFLOW && idx == 0)) {   /* for select field "1" (overflow of index n - 1), the index is valid only for 1 ~ 7, which counts overflow of index 0 ~ 6 respectively. */
        PRINT_PERF_MON_E("[%s] Error, idx %d, config 0x%08X\n", __func__, idx, config);
        result = PERF_MON_STATUS_ERROR;
    } else {    /* Available counter id (0 ~ 7) */
        perf_mon_counter_hi32[idx] = 0;
        perf_mon_counter_in_use |= 0x1 << idx;
        WER(PMADDR_PMCTRL(idx), (config & PMCTRL_CONFIG_MASK) | PMCTRL_INT);
        WER(PMADDR_PMSTAT(idx), PMSTAT_OVFL | PMSTAT_INTASRT);
        WER(PMADDR_PM(idx), 0);
        result = PERF_MON_STATUS_OK;
    }
    return result;
}

static perf_mon_status_t perf_mon_init_counters (void)
{
    int32_t flag = 0;
    int32_t idx;
    for (idx = 0; idx < NUM_OF_PERF_COUNTER; idx++) {
        uint32_t config = perf_mon_config_array[idx];
        perf_mon_status_t result = perf_mon_set_counter_64bit(config);
        if (result != PERF_MON_STATUS_OK) {
            flag = 1;
        }
    }
    return flag ? PERF_MON_STATUS_ERROR : PERF_MON_STATUS_OK;
}

perf_mon_status_t perf_mon_init (void)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_start_flag != 0) {
        PRINT_PERF_MON_E("[%s] Stop before init\n", __func__);
        result = PERF_MON_STATUS_CALL_STOP;
    } else {
        perf_mon_clear_all();
        xt_set_interrupt_handler(PERF_MON_INTERRUPT_INDEX, perf_mon_interrupt_handler, NULL);
        memcpy(perf_mon_config_array, perf_mon_default_config_table, NUM_OF_PERF_COUNTER * sizeof(uint32_t));
        perf_mon_default_flag = 1;
        result = perf_mon_init_counters();
    }
    LEAVE_SESSION(status);
    return result;
}

perf_mon_status_t perf_mon_start (void)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_start_flag != 0) { /* Re-entry */
        PRINT_PERF_MON_E("[%s] Re-entry\n", __func__);
        result = PERF_MON_STATUS_REENTRY;
    } else {
        perf_mon_start_flag = 1;
        perf_mon_enable();
        result = PERF_MON_STATUS_OK;
    }
    LEAVE_SESSION(status);
    return result;
}

perf_mon_status_t perf_mon_stop (void)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_start_flag == 0) { /* Re-entry */
        PRINT_PERF_MON_E("[%s] Re-entry\n", __func__);
        result = PERF_MON_STATUS_REENTRY;
    } else {
        perf_mon_disable();
        perf_mon_start_flag = 0;
        result = PERF_MON_STATUS_OK;
    }
    LEAVE_SESSION(status);
    return result;
}

perf_mon_status_t perf_mon_get_value (perf_mon_menu_t order, uint64_t *p_value)
{
    perf_mon_status_t result = PERF_MON_STATUS_OK;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_default_flag == 0) {
        PRINT_PERF_MON_E("[%s] Not using default config\n", __func__);
        result = PERF_MON_STATUS_INVALID_CONFIGURATION;
    } else if (p_value == NULL) {
        PRINT_PERF_MON_E("[%s] NULL pointer\n", __func__);
        result = PERF_MON_STATUS_NULL;
    } else {
        uint64_t value;
        uint64_t i_miss_cnt          = perf_mon_get_counter_64bit_value(0);
        uint64_t i_hit_cnt           = perf_mon_get_counter_64bit_value(1);
        uint64_t d_load_miss_cnt     = perf_mon_get_counter_64bit_value(2);
        uint64_t d_load_hit_cnt      = perf_mon_get_counter_64bit_value(3);
        uint64_t d_store_miss_cnt    = perf_mon_get_counter_64bit_value(4);
        uint64_t d_store_hit_cnt     = perf_mon_get_counter_64bit_value(5);
        uint64_t i_prefetch_miss_cnt = perf_mon_get_counter_64bit_value(6);
        uint64_t d_prefetch_miss_cnt = perf_mon_get_counter_64bit_value(7);
        switch (order) {
            case PERF_MON_ICACHE_MISS_COUNT:
                value = i_miss_cnt;
                break;
            case PERF_MON_ICACHE_HIT_COUNT:
                value = i_hit_cnt;
                break;
            case PERF_MON_ICACHE_ACCESS_COUNT:
                value = i_miss_cnt + i_hit_cnt;
                break;
            case PERF_MON_DCACHE_MISS_COUNT:
                value = d_load_miss_cnt + d_store_miss_cnt;
                break;
            case PERF_MON_DCACHE_HIT_COUNT:
                value = d_load_hit_cnt + d_store_hit_cnt;
                break;
            case PERF_MON_DCACHE_ACCESS_COUNT:
                value = d_load_miss_cnt + d_load_hit_cnt + d_store_miss_cnt + d_store_hit_cnt;
                break;
            case PERF_MON_DCACHE_LOAD_MISS_COUNT:
                value = d_load_miss_cnt;
                break;
            case PERF_MON_DCACHE_LOAD_HIT_COUNT:
                value = d_load_hit_cnt;
                break;
            case PERF_MON_DCACHE_LOAD_ACCESS_COUNT:
                value = d_load_miss_cnt + d_load_hit_cnt;
                break;
            case PERF_MON_DCACHE_STORE_MISS_COUNT:
                value = d_store_miss_cnt;
                break;
            case PERF_MON_DCACHE_STORE_HIT_COUNT:
                value = d_store_hit_cnt;
                break;
            case PERF_MON_DCACHE_STORE_ACCESS_COUNT:
                value = d_store_miss_cnt + d_store_hit_cnt;
                break;
            case PERF_MON_IPREFETCH_MISS_COUNT:
                value = i_prefetch_miss_cnt;
                break;
            case PERF_MON_IPREFETCH_HIT_COUNT:
                value = i_miss_cnt - i_prefetch_miss_cnt;
                break;
            case PERF_MON_IPREFETCH_ACCESS_COUNT:
                value = i_miss_cnt;
                break;
            case PERF_MON_DPREFETCH_MISS_COUNT:
                value = d_prefetch_miss_cnt;
                break;
            case PERF_MON_DPREFETCH_HIT_COUNT:
                value = d_load_miss_cnt + d_store_miss_cnt - d_prefetch_miss_cnt;
                break;
            case PERF_MON_DPREFETCH_ACCESS_COUNT:
                value = d_load_miss_cnt + d_store_miss_cnt;
                break;
            default:
                value = 0;
                PRINT_PERF_MON_E("[%s] Invalid order = %d\n", __func__, order);
                result = PERF_MON_STATUS_INVALID_ORDER;
                break;
        }
        *p_value = value;
    }
    LEAVE_SESSION(status);
    return result;
}

perf_mon_status_t perf_mon_reset_counters (void)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_start_flag != 0) {
        PRINT_PERF_MON_E("[%s] Stop before reset\n", __func__);
        result = PERF_MON_STATUS_CALL_STOP;
    } else {
        perf_mon_clear_counter_values();
        result = PERF_MON_STATUS_OK;
    }
    LEAVE_SESSION(status);
    return result;
}

uint32_t perf_mon_get_number_of_counters (void)
{
    return NUM_OF_PERF_COUNTER;
}

perf_mon_status_t perf_mon_set_configuration (uint32_t *p_config, uint32_t num)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_start_flag != 0) {
        PRINT_PERF_MON_E("[%s] Stop before set\n", __func__);
        result = PERF_MON_STATUS_CALL_STOP;
    } else if (p_config == NULL) {
        PRINT_PERF_MON_E("[%s] NULL pointer\n", __func__);
        result = PERF_MON_STATUS_NULL;
    } else if (num > NUM_OF_PERF_COUNTER || num <= 0) {
        PRINT_PERF_MON_E("[%s] Invalid number = %d\n", __func__, num);
        result = PERF_MON_STATUS_INVALID_PARAMETER;
    } else {
        perf_mon_clear_all();
        memset(perf_mon_config_array, 0, NUM_OF_PERF_COUNTER * sizeof(uint32_t));
        memcpy(perf_mon_config_array, p_config, num * sizeof(uint32_t));
        perf_mon_default_flag = 0;
        result = perf_mon_init_counters();
    }
    LEAVE_SESSION(status);
    return result;
}

perf_mon_status_t perf_mon_select_preset_bits (uint32_t preset_bits)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (perf_mon_start_flag != 0) {
        PRINT_PERF_MON_E("[%s] Stop before select\n", __func__);
        result = PERF_MON_STATUS_CALL_STOP;
    } else {
        perf_mon_clear_all();
        memset(perf_mon_config_array, 0, NUM_OF_PERF_COUNTER * sizeof(uint32_t));
        {
            uint32_t bit_idx;
            uint32_t config_idx = 0;
            for (bit_idx = 0; bit_idx < NUM_OF_PRESETS; bit_idx++) {
                uint32_t bit_mask = 0x1 << bit_idx;
                if ((preset_bits & bit_mask) != 0) {
                    perf_mon_config_array[config_idx] = perf_mon_preset_config_table[bit_idx];
                    config_idx++;
                    if (config_idx >= NUM_OF_PERF_COUNTER) {
                        break;
                    }
                }
            }
        }
        perf_mon_default_flag = 0;
        result = perf_mon_init_counters();
    }
    LEAVE_SESSION(status);
    return result;
}

perf_mon_status_t perf_mon_get_counters (uint64_t *p_value, uint32_t num)
{
    perf_mon_status_t result;
    UBaseType_t status = 0;
    ENTER_SESSION(status);
    if (p_value == NULL) {
        PRINT_PERF_MON_E("[%s] NULL pointer\n", __func__);
        result = PERF_MON_STATUS_NULL;
    } else if (num > NUM_OF_PERF_COUNTER || num <= 0) {
        PRINT_PERF_MON_E("[%s] Invalid number = %d\n", __func__, num);
        result = PERF_MON_STATUS_INVALID_PARAMETER;
    } else {
        uint32_t idx;
        for (idx = 0; idx < num; idx++) {
            if ((perf_mon_counter_in_use & (0x1 << idx)) != 0) {
                *p_value++ = perf_mon_get_counter_64bit_value(idx);
            } else {
                *p_value++ = 0;
            }
        }
        result = PERF_MON_STATUS_OK;
    }
    LEAVE_SESSION(status);
    return result;
}
