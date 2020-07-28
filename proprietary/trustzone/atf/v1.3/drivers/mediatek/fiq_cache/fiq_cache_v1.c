#include <arch.h>
#include <arch_helpers.h>
#include <fiq_smp_call.h>
#include <platform.h>
#include <platform_def.h>
#include <spinlock.h>
#include <stdio.h>

extern void dcsw_op_level1(unsigned int type);
extern void dcsw_op_level2(unsigned int type);

static spinlock_t fiq_cache_lock;
/* by default, all cores need not to do cache flush */
static unsigned int cpus_dont_flush_cache = 0xffffffff;

typedef struct {
	int counter;
} atomic_t;

#define ACCESS_ONCE(x) (*(volatile unsigned int *)&(x))

static inline void atomic_add(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	__asm__ volatile("// atomic_add" "\n"
"1:     ldxr    %w0, %2\n"
"       add     %w0, %w0, %w3\n"
"       stxr    %w1, %w0, %2\n"
"       cbnz    %w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i));
}

#define DMB()	__asm__ volatile("dmb sy" : : : "memory")

/* only updated by the current core, could be not atomic */
static volatile int  L2_flush_check[PLATFORM_CORE_COUNT];
/* only updated by the current core, could be not atomic */

static uint64_t fiq_step_addr;

void init_fiq_cache_step(uint64_t addr)
{
	fiq_step_addr = addr;
}

static void set_fiq_cache_debug_step(unsigned int step)
{
	if (!fiq_step_addr)
		return;

	*(volatile uint32_t *)fiq_step_addr = step;
	DMB();
}

static void flush_L1_cache(void *info)
{
	dcsw_op_level1(DCCISW);
}

static void flush_L2_cache(void *info)
{
	dcsw_op_level2(DCCISW);
}

static void clear_L2_flush_check(void)
{
	int i;

	for (i = 0; i < PLATFORM_CORE_COUNT; ++i) {
		L2_flush_check[i] = 0;
	}

}

int fiq_cache_trylock(void)
{
	return spin_trylock(&fiq_cache_lock);
}

void fiq_cache_unlock(void)
{
	spin_unlock(&fiq_cache_lock);
}

static int cids_init_done = 0;
static unsigned int cids[PLATFORM_CORE_COUNT];
static int cpus_in_cluster[16]; /* should not be more than 16 clusters */

static int this_cpu_need_flush_L2(void)
{
	unsigned int this_cluster = (read_mpidr() >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	int first_cpu = 0, last_cpu = 0;
	int i;

	/* find the semi-open ended range for current cluster */
	for (i = 0; i < this_cluster; ++i) {
		first_cpu += cpus_in_cluster[i];
	}

	last_cpu = first_cpu + cpus_in_cluster[i];

	/*
	 * if there is any cpu in the same cluster with the current cpu,
	 * current cpu does not need to do L2 flush
	 */
	for (i = first_cpu; i < last_cpu ; ++i) {
		if (L2_flush_check[i])
			return 0;
	}

	return 1;
}

static unsigned int get_cid(int cpu)
{
	int cluster_count;
	int i, j;
	int first_cpu = 0, last_cpu = 0;

	if (!cids_init_done) {
		cluster_count = plat_get_aff_count(1, 0);
		for (i = 0; i < cluster_count; ++i) {
			cpus_in_cluster[i] = plat_get_aff_count(0,
						(i << MPIDR_AFF1_SHIFT));
			first_cpu = last_cpu;
			last_cpu += cpus_in_cluster[i];

			for (j = first_cpu;j < last_cpu; ++j)
				cids[j] = (i << MPIDR_AFF1_SHIFT);
		}
	}

	return cids[cpu];
}

void cache_flush_all_by_fiq(void)
{
#define PLATFORM_CPUMASK	((1 << PLATFORM_CORE_COUNT)-1)
	unsigned int cpu = platform_get_core_pos(read_mpidr());
	unsigned int cpus_should_flush_L1_cache;
	unsigned int cpus_should_flush_L2_cache;
	int i = 0;
	unsigned int cid, last_cid;

	set_fiq_cache_debug_step(0x1);

	while (!fiq_cache_trylock());

	set_fiq_cache_debug_step(0x2);

	clear_L2_flush_check();

	/* make sure the flags are all updated before anyone reads */
	DMB();

	set_fiq_cache_debug_step(0x3);

	/* get the cpu mask for those cpu need to flush cache but current one */
	cpus_should_flush_L1_cache = ((~cpus_dont_flush_cache) & PLATFORM_CPUMASK)
					& (~(1 << cpu));

	set_fiq_cache_debug_step(0x4);

	/* ask other cores to flush L1, no wait!! */
	fiq_smp_call_function(cpus_should_flush_L1_cache, flush_L1_cache, 0, 1);

	set_fiq_cache_debug_step(0x5);

	/* flush L1 by this core */
	dcsw_op_level1(DCCISW);

	set_fiq_cache_debug_step(0x6);

	/* make sure the above loop exit and then proceed */
	DMB();

	/* use 99 as the impossible cluster ID */
	last_cid = 99;
	cpus_should_flush_L2_cache = 0;
	for (i = 0; i < PLATFORM_CORE_COUNT; ++i) {
		if (!((1 << i) & cpus_should_flush_L1_cache))
			continue;

		cid = get_cid(i);
		if (cid != last_cid) {
			last_cid = cid;
			L2_flush_check[i] = 1;
			cpus_should_flush_L2_cache |= (1<<i);
		}
	}

	fiq_smp_call_function(cpus_should_flush_L2_cache, flush_L2_cache, 0, 1);
	set_fiq_cache_debug_step(0x7);

	/*
	 * since other cores will flush L2 by checking all_L1_flush_done flag
	 * make sure L2_flush_check flag has effect before setting all_L1_flush_done
	 */
	DMB();

	set_fiq_cache_debug_step(0x8);

	if (this_cpu_need_flush_L2()) {
		dcsw_op_level2(DCCISW);
	}

	set_fiq_cache_debug_step(0x9);

	/* make sure the above loop exit and then proceed */
	DMB();

	set_fiq_cache_debug_step(0xa);

	fiq_cache_unlock();

	set_fiq_cache_debug_step(0x0);
}

void disable_cpu_status_for_cache(void)
{
	int cpu = platform_get_core_pos(read_mpidr());

	cpus_dont_flush_cache |= (1 << cpu);

	/* make sure the above change can be seen */
	DMB();
}

void enable_cpu_status_for_cache(unsigned long cpu)
{
	cpus_dont_flush_cache &= ~(1 << cpu);

	/* make sure the above change can be seen */
	DMB();
}
