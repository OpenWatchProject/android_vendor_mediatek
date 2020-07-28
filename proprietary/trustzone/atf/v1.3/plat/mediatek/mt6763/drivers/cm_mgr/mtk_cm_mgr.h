#ifndef __MTK_CM_MGR_H__
#define __MTK_CM_MGR_H__

#define CLUSTOR_COUNT 2
#define CPU_COUNT 8
struct pmu_s {
	uint32_t clustor[CLUSTOR_COUNT];
	uint32_t pmu_val[CPU_COUNT][3];
	uint32_t pmu_val_diff[CPU_COUNT][3];
	uint64_t pmu_val_diff_sum[CPU_COUNT];
	uint64_t time_ns[CPU_COUNT];
	uint32_t time_ns_diff[CPU_COUNT];
	uint32_t bw[CPU_COUNT];
	uint32_t bw_max;
	uint32_t ratio[CPU_COUNT];
	uint32_t ratio_max[CLUSTOR_COUNT];
	uint32_t cpu;
	uint32_t cpu_count[CLUSTOR_COUNT];
	uint32_t cpu_ack;
	uint32_t cpu_count_ack[CLUSTOR_COUNT];
	uint32_t result;
};

extern struct pmu_s pmu_all;
extern struct pmu_s *ppmu_all;

extern unsigned int cpus_dont_flush_cache;
extern uint64_t sched_clock(void);
extern void debug_pmu(int cpu);
extern void debug_pmu_all(void);
extern uint32_t test_pmu(uint64_t x1, uint64_t x2, uint64_t x3);
extern uint32_t check_pmu(uint64_t x1, uint64_t x2, uint64_t x3);

#endif /* __MTK_CM_MGR_H__ */

