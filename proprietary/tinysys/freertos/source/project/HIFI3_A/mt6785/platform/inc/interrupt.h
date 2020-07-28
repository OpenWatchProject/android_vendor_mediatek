/*
 * interrupt.h
 *
 *  Created on: 2017/9/21
 *      Author: MTK09865
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "FreeRTOS.h"
#include <task.h>
#include <xtensa_api.h>
#include <platform_mtk.h>
#include <driver_api.h>
#include <mt_reg_base.h>
#include <main.h>

typedef void(*irq_handler_t)(void);

struct irq_desc_t {
    irq_handler_t handler;
    const char  *name;
    uint32_t priority;
    uint32_t irq_count;
    uint32_t wakeup_count;
    uint32_t init_count;
    uint64_t last_enter;
    uint64_t last_exit;
    uint64_t max_duration;
};

extern uint32_t wakeup_src_mask;

void mask_irq(IRQn_Type IRQn) INTERNAL_FUNCTION;
void unmask_irq(IRQn_Type IRQn) INTERNAL_FUNCTION;
void turn_off_non_wakeup_src_irq(void) INTERNAL_FUNCTION;
void turn_on_non_wakeup_src_irq(void) INTERNAL_FUNCTION;

void irq_status_dump(void) INTERNAL_FUNCTION;
void wakeup_source_count(uint32_t wakeup_src) INTERNAL_FUNCTION;
void wakeup_src_handler(void) INTERNAL_FUNCTION;
void hw_isr_dispatch(void *arg) INTERNAL_FUNCTION;
void irq_init(void) INTERNAL_FUNCTION;
void irq_sync(void) INTERNAL_FUNCTION;

uint32_t is_in_isr(void) __attribute__((always_inline)) INTERNAL_FUNCTION;

void request_irq(uint32_t irq, irq_handler_t handler, const char *name) INTERNAL_FUNCTION;
void set_irq_limit(uint32_t irq_limit_time) INTERNAL_FUNCTION;
uint64_t get_max_cs_duration_time(void) INTERNAL_FUNCTION;
uint64_t get_max_cs_limit(void) INTERNAL_FUNCTION;


#endif /*INTERRUPT_H */
