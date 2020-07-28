#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "FreeRTOS.h"
#include <task.h>

#include <platform_mtk.h>
#include <driver_api.h>
#include <mt_reg_base.h>
void mask_irq(IRQn_Type IRQn);
void unmask_irq(IRQn_Type IRQn);
void irq_status_dump(void);
void wakeup_source_count(uint32_t wakeup_src);
void irq_init(void);
void free_irq(uint32_t irq);
uint32_t irq_total_times(void);

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

void request_irq(uint32_t irq, irq_handler_t handler, const char *name);
uint32_t is_in_isr(void);

#define DEFAULT_IRQ_PRIORITY (0x8)

void set_max_cs_limit(uint64_t limit_time);
void disable_cs_limit(void);
uint64_t get_max_cs_duration_time(void);
uint64_t get_max_cs_limit(void);
void set_irq_limit(uint32_t irq_limit_time);

#endif
