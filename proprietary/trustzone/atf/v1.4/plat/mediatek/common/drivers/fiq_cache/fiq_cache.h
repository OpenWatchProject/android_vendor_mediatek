#ifndef __FIQ_CACHE_H__
#define __FIQ_CACHE_H__

#include <sys/stdint.h>

extern void dcsw_op_level1(unsigned int type);
extern void dcsw_op_level2(unsigned int type);
extern void dcsw_op_level3(unsigned int type);

void init_fiq_cache_step(uint64_t addr);
void cache_flush_all_by_fiq(void);
void enable_cpu_status_for_cache(unsigned long cpu);
void disable_cpu_status_for_cache(void);
int fiq_cache_trylock(void);
void fiq_cache_unlock(void);

#endif
