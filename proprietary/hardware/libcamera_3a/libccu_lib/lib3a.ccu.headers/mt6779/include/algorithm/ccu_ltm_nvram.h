#ifndef __CCU_LTM_NVRAM_H__
#define __CCU_LTM_NVRAM_H__
#include "ccu_ltm_utility.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

enum ccu_ltm_mode_enum{
	CCU_LTM_MODE_LINEAR = 0,
	CCU_LTM_MODE_GLOBAL,
	CCU_LTM_MODE_LOCAL
};

struct ccu_ltm_mid_perc_lut{
	uint32_t size;
	uint32_t x[10];
};

struct ccu_ltm_local_global_blending {
	uint32_t x[2];
	uint32_t ratio[2];
	uint32_t tp_ratio[14];
};

struct ccu_ltm_smooth_speed_type {
	uint32_t ae_stable_speed;
	uint32_t same_direction_speed;
	uint32_t reverse_direction_speed;
	uint32_t other_speed;
};

struct ccu_ltm_nvram{
	enum ccu_ltm_mode_enum local_tone_mode;

	// Global Tone Metering
	uint32_t clipping_thd_perc;
	uint32_t high_bound_perc;
	struct ccu_ltm_mid_perc_lut mid_perc_lut;

	// fix Global Tone
	uint32_t fix_clipping_thd;
	uint32_t fix_turning_point;
	uint32_t fix_gtc_blending_ratio;

	// Local Tone Metering
	uint32_t strength_lct[14];
	uint32_t strength_ltp_mean_idx[2];
	uint32_t strength_ltp[2][14];

	// Face Tone Metering
	bool ltm_face_protection_enable;
	bool multi_face_protection_enable;
	uint32_t non_robust_face_turning_point_reduction_ratio;
	uint32_t face_ratio_low_bound;
	uint32_t face_ratio_high_bound;
	uint32_t face_protection_strength[14];

	// Temporal Smooth
	struct ccu_ltm_smooth_speed_type smooth_speed;
	struct ccu_ltm_smooth_speed_type face_smooth_speed;
	uint32_t delta_cwr_threshold;
	uint32_t max_index_count_threshold;

	// spatial smooth
	bool spatial_smooth_enable;
	uint32_t spatial_smooth_strength_table_x[2];
	uint32_t spatial_smooth_strength_table_y[2][14];
	uint32_t spatial_smooth_diff_table_x[4];
	uint32_t spatial_smooth_var_table_x[4];
	uint32_t spatial_smooth_var_table_y[4];

	// Face spatial smooth
	bool ltm_face_spatial_smooth_enable;
	uint32_t ltm_b2f_turning_point_diff_spatial_smooth_weighting_table[4][14];
	uint32_t ltm_b2f_distance_reduction_ratio[2];
	uint32_t ltm_f2b_turning_point_diff_spatial_smooth_weighting_table[4][14];
	uint32_t ltm_f2b_distance_reduction_ratio[2];

	// Face temporal smooth
	bool ltm_face_state_link_enable;
	// Local & Global Tone Blending
	struct ccu_ltm_local_global_blending local_global_blending;
	// LTM param
	uint32_t ltm_param_reserve[6][14];
};

#endif