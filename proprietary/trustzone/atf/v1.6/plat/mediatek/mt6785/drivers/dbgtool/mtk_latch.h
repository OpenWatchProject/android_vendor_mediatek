#ifndef __LATCH_H__
#define __LATCH_H__

#define BUS_PERI_R0	(PERICFG_BASE + 0x500)
#define BUS_PERI_R1	(PERICFG_BASE + 0x504)
#define BUS_PERI_MON0	(PERICFG_BASE + 0x508)
#define BUS_MAX_NR_MON	10

unsigned long lastbus_perimon_init(unsigned int val_timeout, unsigned int val_enable);
unsigned long lastbus_perimon_check_hang(void);
unsigned long lastbus_perimon_get(unsigned int index);

#endif
