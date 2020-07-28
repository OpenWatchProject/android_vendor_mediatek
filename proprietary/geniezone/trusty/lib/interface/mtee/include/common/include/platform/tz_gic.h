/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stddef.h>

#ifndef _TZ_GIC_H_
#define _TZ_GIC_H_

/* GIC Public function prototype */
void tz_gic_init(unsigned int dist_base, unsigned int cpu_base,
                 const unsigned int *allowed_fiq_list, int fiq_number);
void tz_gic_cpu_init(void);
void gic_raise_softirq(unsigned int cpumask, unsigned int irq);
void gic_ack_softirq(unsigned int irq);
void gic_save_context(void);
void gic_restore_context(void);

/**
 * TZ FIQ set.
 */
TZ_RESULT gic_fiq_set(unsigned int irq, unsigned long irq_flags);

/**
 * TZ GIC FIQ enable.
 */
TZ_RESULT gic_fiq_enable(int irq, int enable);

/**
 * TZ GIC FIQ query
 */
TZ_RESULT gic_fiq_query(int irq, unsigned int *pstate);

/**
 * Get GIC INTACT register
 */
TZ_RESULT gic_fiq_get_intack(unsigned int *iar);

/**
 * AcK GIC EOI
 */
TZ_RESULT gic_fiq_eoi(unsigned int iar);

/**
 * Trigger FIQ SGI.
 *
 * @param mask The CPU to send this irq.
 * @param irq The irq number to send. Must be FIQ.
 * @return return value.
 */
TZ_RESULT gic_trigger_softfiq(unsigned int mask, int irq);

/**
 * Mask all irq
 */
TZ_RESULT gic_irq_mask_all(unsigned int *pmask, unsigned int size);

/**
 * Restore all irq mask
 */
TZ_RESULT gic_irq_mask_restore(unsigned int *pmask, unsigned int size);


#endif
