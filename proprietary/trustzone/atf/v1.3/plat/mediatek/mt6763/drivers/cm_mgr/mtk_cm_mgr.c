#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <platform.h>
#include <mtk_cm_mgr.h>
#include <mt_spm_reg.h>
#include <fiq_smp_call.h>
#include <mediatek/fiq_cache.h>


/* 0xe1, -xe7, 0xe8, diff */

/* #define _ONE_CPU_ */
#define USE_PMU
#define USE_TIME_NS
/* #define USE_DEBUG_LOG */

#if 1
struct pmu_s pmu_all;
struct pmu_s *ppmu_all = &pmu_all;

#if 0 /* def USE_PMU */
static uint32_t pmu_array[8] = {0x0D430000,
	0x0D530000,
	0x0D630000,
	0x0D730000,
	0x0D830000,
	0x0D930000,
	0x0Da30000,
	0x0Db30000};
#endif

#ifdef USE_DEBUG_LOG
void debug_pmu(int cpu)
{
	int i;

	ERROR("%s: cpu number %d ################\n", __func__, cpu);
	ERROR("%s: clustor[%d] 0x%x\n", __func__, cpu / 4, ppmu_all->clustor[cpu / 4]);
	for (i = 0; i < 3; i++) {
		ERROR("%s: pmu_val[%d][%d] 0x%x\n", __func__, cpu, i, ppmu_all->pmu_val[cpu][i]);
		ERROR("%s: pmu_val_diff[%d][%d] 0x%x\n", __func__, cpu, i, ppmu_all->pmu_val_diff[cpu][i]);
	}
	ERROR("%s: pmu_val_diff_sum[%d] 0x%lx\n", __func__, cpu, ppmu_all->pmu_val_diff_sum[cpu]);
	ERROR("%s: time_ns[%d] 0x%lx\n", __func__, cpu, ppmu_all->time_ns[cpu]);
	ERROR("%s: time_ns_diff[%d] 0x%x\n", __func__, cpu, ppmu_all->time_ns_diff[cpu]);
	ERROR("%s: bw[%d] 0x%x\n", __func__, cpu, ppmu_all->bw[cpu]);
	ERROR("%s: bw_max 0x%x\n", __func__, ppmu_all->bw_max);
	ERROR("%s: ratio[%d] 0x%x\n", __func__, cpu, ppmu_all->ratio[cpu]);
	ERROR("%s: ratio_max[%d] 0x%x\n", __func__, cpu / 4, ppmu_all->ratio_max[cpu / 4]);
	ERROR("%s: cpu 0x%x\n", __func__, ppmu_all->cpu);
	ERROR("%s: cpu_count[%d] 0x%x\n", __func__, cpu / 4, ppmu_all->cpu_count[cpu / 4]);
	ERROR("%s: cpu_ack 0x%x\n", __func__, ppmu_all->cpu_ack);
	ERROR("%s: cpu_count_ack[%d] 0x%x\n", __func__, cpu / 4, ppmu_all->cpu_count_ack[cpu / 4]);
	ERROR("%s: result 0x%x\n", __func__, ppmu_all->result);
}

void debug_pmu_all(void)
{
	int i;

	for (i = 0; i < 8; i++)
		debug_pmu(i);
}
#endif

#define diff_value_overflow(diff, a, b) do {\
	if ((a) >= (b)) \
	diff = (a) - (b);\
	else \
	diff = 0xffffffff - (b) + (a); \
} while (0) \

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define CPU_ARRAY_SIZE 4
static unsigned int cpu_power_gain_up_low[][CPU_ARRAY_SIZE] = {
	/* 20170925 */
	{2, 3, 1, 2},
	{3, 6, 2, 5},
	{5, 10, 4, 7},
	{84, 106, 5, 10},
	{81, 104, 79, 100},
	{77, 101, 75, 96},
	{74, 99, 71, 93},
	{71, 96, 68, 90},
	{67, 93, 64, 86},
	{64, 91, 60, 83},
	{61, 88, 57, 79},
	{58, 85, 53, 76},
	{54, 83, 49, 72},
	{51, 80, 45, 69},
	{48, 78, 42, 65},
	{44, 75, 38, 62},
	{41, 72, 34, 59},
	{38, 70, 31, 55},
	{34, 67, 27, 52},
	{31, 64, 23, 48},
};

static unsigned int cpu_power_gain_down_low[][CPU_ARRAY_SIZE] = {
	/* 20170925 */
	{2, 5, 2, 3},
	{5, 10, 3, 6},
	{96, 122, 5, 10},
	{93, 121, 90, 114},
	{91, 119, 87, 111},
	{88, 118, 83, 108},
	{85, 116, 79, 105},
	{82, 115, 76, 102},
	{79, 113, 72, 99},
	{76, 112, 68, 96},
	{73, 110, 64, 92},
	{70, 109, 61, 89},
	{67, 107, 57, 86},
	{64, 106, 53, 83},
	{61, 104, 50, 80},
	{58, 103, 46, 77},
	{56, 101, 42, 74},
	{53, 100, 39, 71},
	{50, 98, 35, 68},
	{47, 97, 31, 64},
};

static unsigned int cpu_power_gain_up_high[][CPU_ARRAY_SIZE] = {
	/* 20170925 */
	{2, 3, 1, 2},
	{3, 6, 2, 5},
	{5, 10, 4, 7},
	{6, 13, 5, 10},
	{8, 16, 6, 12},
	{9, 19, 7, 14},
	{11, 23, 8, 17},
	{110, 143, 9, 19},
	{103, 136, 11, 22},
	{96, 130, 12, 24},
	{90, 123, 86, 114},
	{83, 117, 79, 107},
	{77, 110, 72, 100},
	{70, 104, 65, 92},
	{64, 97, 58, 85},
	{57, 91, 51, 78},
	{51, 84, 44, 70},
	{44, 77, 37, 63},
	{38, 71, 30, 56},
	{31, 64, 23, 48},
};

static unsigned int cpu_power_gain_down_high[][CPU_ARRAY_SIZE] = {
	/* 20170925 */
	{2, 5, 2, 3},
	{5, 10, 3, 6},
	{7, 14, 5, 10},
	{9, 19, 6, 13},
	{12, 24, 8, 16},
	{139, 179, 9, 19},
	{132, 173, 11, 23},
	{125, 167, 119, 154},
	{119, 161, 112, 147},
	{112, 156, 105, 139},
	{106, 150, 97, 132},
	{99, 144, 90, 124},
	{93, 138, 83, 117},
	{86, 132, 75, 109},
	{80, 126, 68, 102},
	{73, 120, 61, 94},
	{66, 114, 53, 87},
	{60, 108, 46, 79},
	{53, 102, 39, 72},
	{47, 97, 31, 64},
};

static unsigned int *cpu_power_gain_up = &cpu_power_gain_up_high[0][0];
static unsigned int *cpu_power_gain_down = &cpu_power_gain_down_high[0][0];
#define cpu_power_gain(p, i, j) (*(p + (i) * CPU_ARRAY_SIZE + (j)))
#define LOWER_OPP 9
#define IS_UP 1
#define IS_DOWN 0

static unsigned int cpu_power_gain_opp(int is_up, int opp, int ratio_idx, int idx)
{
	if (opp < LOWER_OPP) {
		cpu_power_gain_up = &cpu_power_gain_up_low[0][0];
		cpu_power_gain_down = &cpu_power_gain_down_low[0][0];
	} else {
		cpu_power_gain_up = &cpu_power_gain_up_high[0][0];
		cpu_power_gain_down = &cpu_power_gain_down_high[0][0];
	}

	if (is_up)
		return cpu_power_gain(cpu_power_gain_up, ratio_idx, idx);
	else
		return cpu_power_gain(cpu_power_gain_down, ratio_idx, idx);
}

uint32_t check_pmu(uint64_t x1, uint64_t x2, uint64_t x3)
{
	int i;
	int cpu_power[2] = {0, 0};
	int idx;

	switch (x1) {
	case 0:
		ppmu_all->clustor[0] = x2;
		ppmu_all->clustor[1] = x3;
		ppmu_all->bw_max = 0;
		ppmu_all->result = 0;
		ppmu_all->cpu = 0;
		for (i = 0; i < CLUSTOR_COUNT; i++) {
			ppmu_all->ratio_max[i] = 0;
			ppmu_all->cpu_count[i] = 0;
			ppmu_all->cpu_count_ack[i] = 0;
		}

		return test_pmu(0, 0, 0);
	case 1:
		for (i = 0; i < CPU_COUNT; i++) {
			idx = (ppmu_all->ratio[i] == 100) ? 19 : (ppmu_all->ratio[i] / 5);
			if (i < 4)
				cpu_power[0] += cpu_power_gain_opp(IS_UP, x3 & 0xffff, idx, x2);
			else
				cpu_power[1] += cpu_power_gain_opp(IS_UP, (x3 >> 16) & 0xffff, idx, idx + 1);
		}
		return (cpu_power[0] & 0xffff) | (cpu_power[1] & 0xffff) << 16;
	case 2:
		for (i = 0; i < CPU_COUNT; i++) {
			idx = (ppmu_all->ratio[i] == 100) ? 19 : (ppmu_all->ratio[i] / 5);
			if (i < 4)
				cpu_power[0] += cpu_power_gain_opp(IS_DOWN, x3 & 0xffff, idx, x2);
			else
				cpu_power[1] += cpu_power_gain_opp(IS_DOWN, (x3 >> 16) & 0xffff, idx, x2 + 1);
		}
		return (cpu_power[0] & 0xffff) | (cpu_power[1] & 0xffff) << 16;
	default:
		break;
	}

	return 0;
}

#ifdef USE_PMU
static void read_pmu(void *info)
{
	uint32_t *data = (uint32_t *)info;

	__asm__ volatile("mrs %0, pmevcntr1_el0" : "=r" (data[0]));
	__asm__ volatile("mrs %0, pmevcntr2_el0" : "=r" (data[1]));
	__asm__ volatile("mrs %0, pmevcntr3_el0" : "=r" (data[2]));
}
#endif

uint32_t test_pmu(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint32_t i, j;
	uint32_t clustor;
	uint32_t pmu_val_new[3];
#ifdef USE_TIME_NS
	uint64_t time_ns_new;
#endif
	uint64_t mpidr = read_mpidr();
	uint32_t cpu_now = platform_get_core_pos(mpidr);

#ifdef _ONE_CPU_
	i = cpu_now;
	for (i = cpu_now; i < (cpu_now + 1); i++)
#else
	for (i = 0; i < 8; i++)
#endif
	{
		/* ERROR("%s: cpu number %d\n", __func__, i); */

		if (!(mmio_read_32(PWR_STATUS) & (1 << (9 + i + 3 * (i / 4))))) {
#ifdef USE_DEBUG_LOG
			ERROR("continue! CPU_PWR_STATUS=0x%x\n", mmio_read_32(PWR_STATUS));
#endif
			continue;
		}

		clustor = i / 4;
		ppmu_all->cpu_ack |= (1 << i);
		ppmu_all->cpu_count_ack[clustor]++;

#ifdef USE_PMU
		if (i == cpu_now) {
			read_pmu(pmu_val_new);
		} else {
			while (!fiq_cache_trylock())
				;
			fiq_smp_call_cpu(i, read_pmu, pmu_val_new, 300);
			fiq_cache_unlock();
		}

#if 0
		ERROR("CPU%d PMU = 0x%x, 0x%x, 0x%x\n",
			i, pmu_val_new[0], pmu_val_new[1], pmu_val_new[2]);
#endif

		if (pmu_val_new[0] == 0 || pmu_val_new[1] == 0 || pmu_val_new[2] == 0) {
#ifdef USE_DEBUG_LOG
			ERROR("%s: WARN!!! pmu is 0x%x 0x%x 0x%x\n", __func__,
					pmu_val_new[0], pmu_val_new[1], pmu_val_new[2]);
			debug_pmu(i);
#endif
			continue;
		}
#endif

#ifdef USE_TIME_NS
		time_ns_new = sched_clock();
		ppmu_all->time_ns_diff[i] = time_ns_new - ppmu_all->time_ns[i];
		ppmu_all->time_ns[i] = time_ns_new;
#endif

		ppmu_all->pmu_val_diff_sum[i] = 0;
		for (j = 0; j < 3; j++) {
			diff_value_overflow(ppmu_all->pmu_val_diff[i][j], pmu_val_new[j], ppmu_all->pmu_val[i][j]);
			ppmu_all->pmu_val[i][j] = pmu_val_new[j];
			ppmu_all->pmu_val_diff_sum[i] += ppmu_all->pmu_val_diff[i][j];
		}

#if 0
		ERROR("CPU%d pmu diff are 0x%x 0x%x 0x%x\n", i,
			ppmu_all->pmu_val_diff[i][0], ppmu_all->pmu_val_diff[i][1], ppmu_all->pmu_val_diff[i][2]);
#endif

		if (ppmu_all->pmu_val_diff_sum[i] == 0) {
#ifdef USE_DEBUG_LOG
			ERROR("%s: WARN!!! cpu:%d diff_sum == 0\n", __func__, i);
			debug_pmu(i);
#endif
			continue;
		}

		ppmu_all->ratio[i] = ppmu_all->pmu_val_diff_sum[i] * 100000
			/ ppmu_all->time_ns_diff[i] / ppmu_all->clustor[clustor];
		if (ppmu_all->ratio[i] > 100) {
#ifdef USE_DEBUG_LOG
			ERROR("%s: WARN!!! cpu:%d ratio > 100\n", __func__, i);
			debug_pmu(i);
#endif
			/* FIXME: */
			ppmu_all->ratio[i] = 100;
			/* continue; */
		}

		ppmu_all->cpu |= (1 << i);
		ppmu_all->cpu_count[clustor]++;
		ppmu_all->ratio_max[clustor] = max(ppmu_all->ratio[i], ppmu_all->ratio_max[clustor]);
#ifdef USE_DEBUG_LOG
		/* debug_pmu(i); */
		/* ERROR("%s: cpu:%d ppmu_all->pmu_val[i] 0x%lx 0x%lx 0x%lx 0x%lx ppmu_all->ratio[i] %d\n", */
		/* __func__, i, ppmu_all->pmu_val[i][0], ppmu_all->pmu_val[i][1], ppmu_all->pmu_val[i][2], */
		/* ppmu_all->pmu_val_diff_sum[i], ppmu_all->ratio[i]); */
#endif
	}

#if 0
	/* average bw */
	for (i = 0; i < 8; i++)
		ppmu_all->bw_max += mmio_read_32(0x11cf00 + 4 * i);
	ppmu_all->bw_max /= 8;
#else
	/* last bw */
	ppmu_all->bw_max = (mmio_read_32(0x11cf30) * 1000) >> 17;
#endif
	ppmu_all->bw_max /= 512;
	/* ppmu_all->result = (ppmu_all->ratio_max[0] & 0xff) | ((ppmu_all->ratio_max[1] & 0xff) << 8) | */
		/* ((ppmu_all->cpu_count_ack[0] & 0xf) << 16) | ((ppmu_all->cpu_count_ack[1] & 0xf) << 20) | */
		/* ((ppmu_all->cpu_count[0] & 0xf) << 24) | ((ppmu_all->cpu_count[1] & 0xf) << 28); */
	/* ppmu_all->result = (ppmu_all->ratio_max[0] & 0xff) | ((ppmu_all->ratio_max[1] & 0xff) << 8) | */
		/* ((ppmu_all->cpu_count_ack[0] & 0xf) << 16) | ((ppmu_all->cpu_count_ack[1] & 0xf) << 20) | */
		/* ((ppmu_all->bw_max & 0xf) << 24); */
	ppmu_all->result = (ppmu_all->ratio_max[0] & 0xff) | ((ppmu_all->ratio_max[1] & 0xff) << 8) |
		((ppmu_all->bw_max & 0xff) << 16) |
		((ppmu_all->cpu_count[0] & 0xf) << 24) | ((ppmu_all->cpu_count[1] & 0xf) << 28);
#ifdef USE_DEBUG_LOG
	debug_pmu_all();
	/* ERROR("%s: ppmu_all->result 0x%x, ppmu_all->ratio_max[0] 0x%x, ppmu_all->ratio_max[1] 0x%x\n", */
		/* __func__, ppmu_all->result, ppmu_all->ratio_max[0], ppmu_all->ratio_max[1]); */
#endif

	return ppmu_all->result;
}
#endif

