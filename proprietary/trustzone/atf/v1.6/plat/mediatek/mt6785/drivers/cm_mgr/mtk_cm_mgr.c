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

void init_cpu_stall_counter(int cluster)
{
	unsigned int val;
	int i;

	if (cluster == 0) {
		for (i = 0; i < 8; i++) {
			if (i >= 6) {
				val = CM_SET_NAME_VAL(
						mmio_read_32(STALL_INFO_CONF(i)),
						CPU_STALL_CACHE_OPT, 8);
				val = CM_SET_NAME_VAL(
						val,
						CPU_STALL_IDLE_CNT, 8);
				mmio_write_32(STALL_INFO_CONF(i), val);
			}

			val = RG_FMETER_EN;
			val |= RG_MP0_AVG_STALL_PERIOD_1MS;
			val |= RG_CPU_AVG_STALL_RATIO_EN |
				RG_CPU_AVG_STALL_RATIO_RESTART |
				RG_CPU_STALL_COUNTER_EN;
			mmio_write_32(CPU_AVG_STALL_RATIO_CTRL(i), val);

			val = RG_CPU0_NON_WFX_COUNTER_EN;
			mmio_write_32(MP0_CPU_NONWFX_CTRL(i), val);
		}
	}
}

void init_cpu_stall_counter_all(void)
{
	init_cpu_stall_counter(0);
}
