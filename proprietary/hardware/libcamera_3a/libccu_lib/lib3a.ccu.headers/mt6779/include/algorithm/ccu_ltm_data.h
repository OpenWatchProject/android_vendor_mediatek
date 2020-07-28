#ifndef __CCU_LTM_DATA_H__
#define __CCU_LTM_DATA_H__
#include "ccu_ltm_param.h"


struct ccu_ltm_algo_data{

	uint32_t frame_idx;

    uint32_t chip_version;
    uint32_t main_version;
    uint32_t sub_version;
    uint32_t system_version;

	/* AE histogram */
	uint32_t ae_full_y_hist[AE_HISTOGRAM_BIN_SW];
	uint32_t ae_full_y_se_hist[AE_HISTOGRAM_BIN_SW];

    /* LTMS */
    bool ltms_ct_lock;
    uint32_t ltms_pre_ct;
    uint32_t ltms_in_lock_cnt;
    uint32_t ltms_out_lock_cnt;

	/* LTMSO */
	uint32_t blk_x_num;
	uint32_t blk_y_num;
	uint32_t blk_b_num;
	uint32_t detail_hist[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX][LTM_CURVE_POINT_NUM];
	uint32_t apl[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t var[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t flatness[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t blk_max[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t blk_max_cnt[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	
	/* accumulate histogram */
	uint32_t cdf_full_y_se_hist[AE_HISTOGRAM_BIN_SW];
	uint32_t cdf_full_y_le_hist[AE_HISTOGRAM_BIN_SW];
	uint32_t cdf_local_hist[LTM_CURVE_POINT_NUM];
	uint32_t se_hist_total_count;
	uint32_t local_hist_total_count;
	
	/* ae info */
	uint32_t real_light_value_x10;
	int32_t detla_index;
	uint32_t current_cwr;
    uint32_t current_cwv;
	uint32_t previous_cwr;
    bool ae_stable;
	uint32_t max_index_stable_count;
	bool touch_ae_enable;
	
	/* global tone variable */
	uint32_t ltm_global_turning_point;
	uint32_t ltm_global_clipping_thd;
	uint32_t ltm_global_middle_point;
    bool     ltm_global_tone_lock;
    uint32_t ltm_global_in_lock_cnt;
    uint32_t ltm_global_out_lock_cnt;
    uint32_t ltm_gtp_target_candidate;
    uint32_t ltm_gct_target_candidate;
    uint32_t ltm_gmp_target_candidate;

	/* local tone variable */
	uint32_t ltm_ltmso_ct;
	uint32_t ltm_local_clipping_thd[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_turning_point[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];

	uint32_t ltm_local_clipping_thd_meter[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_turning_point_meter[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];

	uint32_t ltm_local_clipping_thd_smooth[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_turning_point_smooth[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	
	uint32_t ltm_local_tone_curve_x[LTM_CURVE_POINT_NUM];
	uint32_t ltm_local_tone_curve_y[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX][LTM_CURVE_POINT_NUM];

	uint32_t strength_lct;
	uint32_t strength_ltp;
	uint32_t strength_ltp_mean_idx_l;
	uint32_t strength_ltp_mean_idx_h;

	uint32_t strength_x[2];
	uint32_t strength_y[2];
	uint32_t diff_weighting_x[4];
	uint32_t diff_weighting_y[4];
	uint32_t var_weighting_x[4];
	uint32_t var_weighting_y[4];

    bool     ltm_stable[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
    uint32_t ltm_stable_cnt;
    bool     ltm_local_tone_lock[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
    uint32_t ltm_local_in_lock_cnt[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
    uint32_t ltm_local_out_lock_cnt[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
    uint32_t ltm_ltp_target_candidate[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
    uint32_t ltm_lct_target_candidate[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
    
    uint32_t ltm_max_lct;
    
    uint32_t pline_index_f;
    uint32_t pre_pline_index_f;
    uint32_t pline_index_f_max;
    uint32_t pline_index_f_min;
    uint32_t pline_boundary_cnt;

	/* blending */
	uint32_t tp_blending_ratio;
	uint32_t face_ct_ratio;

	/* Face LTM variables */
	uint32_t face_num;
	uint32_t protect_face_num;
	bool face_check_flag;
	struct ccu_ltm_ae_meter_area_t fd_area[CCU_LTM_MAX_FACE_AREAS];
	int32_t face_alpha_map[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	int32_t face_bright_tone_protect_value[CCU_LTM_MAX_FACE_AREAS];
	uint32_t face_protection_strength;
	uint32_t max_face_turning_point;

	bool face_ae_stable;
	bool enable_face_ae;
	uint8_t  face_state_from_ae;
	uint32_t face_robust_count;
	uint32_t face_robust_trust_count;
	uint32_t face_lost_lock_max_count;
	uint32_t face_lost_temporal_smooth_frame_num;
	uint32_t non_robust_face_ae_converge_timeout;
	bool   non_robust_face_timeout_lock_ae;

	uint32_t non_robust_face_count;
	uint32_t last_robust_max_face_turning_point;
	LTM_FACE_SMOOTH_TYPE_T ltm_face_smooth_type;

	uint32_t background_to_face_tp_diff_x[2];
	uint32_t background_to_face_weighting_y[2];
	uint32_t face_to_background_tp_diff_x[2];
	uint32_t face_to_background_weighting_y[2];

	bool ltm_smooth_lock_result;

	/* smooth */
	uint32_t ltm_local_turning_point_result[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_clipping_thd_result[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];

	uint32_t ltm_local_turning_point_target[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_clipping_thd_target[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];


	/* LTM NVRAM */
	const struct ccu_ltm_nvram *p_ltm_nvram;

};

#endif