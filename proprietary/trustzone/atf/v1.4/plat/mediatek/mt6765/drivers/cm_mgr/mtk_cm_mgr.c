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

	if (cluster == 0) {
		val = 0x11000;
		mmio_write_32(MP0_CPU_STALL_INFO, val);

		val = RG_FMETER_EN;
		val |= RG_MP0_AVG_STALL_PERIOD_1MS;
		val |= RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP0_CPU0_AVG_STALL_RATIO_CTRL, val);

		val = RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP0_CPU1_AVG_STALL_RATIO_CTRL, val);

		val = RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP0_CPU2_AVG_STALL_RATIO_CTRL, val);

		val = RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP0_CPU3_AVG_STALL_RATIO_CTRL, val);
	} else {
		val = 0x11000;
		mmio_write_32(MP1_CPU_STALL_INFO, val);

		val = RG_FMETER_EN;
		val |= RG_MP0_AVG_STALL_PERIOD_1MS;
		val |= RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP1_CPU0_AVG_STALL_RATIO_CTRL, val);

		val = RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP1_CPU1_AVG_STALL_RATIO_CTRL, val);

		val = RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP1_CPU2_AVG_STALL_RATIO_CTRL, val);

		val = RG_CPU0_AVG_STALL_RATIO_EN |
			RG_CPU0_STALL_COUNTER_EN |
			RG_CPU0_NON_WFX_COUNTER_EN;
		mmio_write_32(MP1_CPU3_AVG_STALL_RATIO_CTRL, val);
	}
}

void init_cpu_stall_counter_all(void)
{
	init_cpu_stall_counter(0);
	init_cpu_stall_counter(1);
}


