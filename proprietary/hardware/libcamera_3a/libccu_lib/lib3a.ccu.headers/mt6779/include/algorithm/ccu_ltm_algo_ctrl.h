#ifndef __CCU_LTM_CTRL_H__
#define __CCU_LTM_CTRL_H__
#include "ccu_ltm_data.h"
#include "ccu_ltm_utility.h"
#ifndef WIN32
#include "ccu_ae_algo_data.h"
#include "ccu_ltm_extif.h"
#endif
#define LTM_ALGO_IF_REVISION 8292001


 // For CCU drv contrl.
enum ltm_ctrl_enum{
	LTM_SET_VSYNC_INFO,
	LTM_SET_RUN_TIME_INFO,
	LTM_SET_LAST_RESULT,
	LTM_GET_INIT_DATA_POINTER_MAIN,
	LTM_GET_INIT_DATA_POINTER_MAIN2,
	LTM_GET_EXIF_PTR,
	LTM_MAX
};

// For multicam.
enum ltm_cam_id_enum{
	LTM_CAM_ID_MAIN,
	LTM_CAM_ID_MAIN2,
	LTM_CAM_ID_SUB,
	LTM_CAM_ID
};

// CCU drv input to Algo.
struct ltm_main_in{
	int32_t  cam_id;
	int32_t  master_cam_id;
	// LTMSO, AEINFO

	// AE histogram
	uint8_t *p_ae_full_y_hist;
	uint8_t *p_ae_full_y_se_hist;

	// LTMSO
	uint32_t blk_x_num;
	uint32_t blk_y_num;
	uint32_t blk_b_num;
	uint32_t *p_ltmso_addr;	
	
	// CT buf
	uint32_t* ct_buf;
	uint32_t  target_ct_index;

	// AEINFO
#ifdef WIN32
	ccu_ltm_ae_info_t* p_ltm_ae_info;
#else
	CCU_AeAlgo *p_ltm_ae_info;
#endif
};

// Algo output to CCU drv.
struct ltm_main_out{
	uint32_t ltm_global_turning_point;
	uint32_t ltm_global_clipping_thd;
	uint32_t ltm_global_middle_point;

	uint32_t ltm_local_turning_point[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_clipping_thd[LTM_BLK_NUM_MAX][LTM_BLK_NUM_MAX];
	uint32_t ltm_local_tone_curve_y[CCU_LTM_BLK_Y_NUM][CCU_LTM_BLK_X_NUM][LTM_CURVE_POINT_NUM];
	uint32_t ltm_local_tone_curve_x[LTM_CURVE_POINT_NUM];
};

struct ltm_init{
	struct ccu_ltm_nvram* p_ltm_nvram;
	
	uint32_t blk_x_num;
	uint32_t blk_y_num;
	uint32_t blk_b_num;

	uint32_t aao_hist_size_se;
};

// For CCU drv, filling initialized data.
struct ltm_ctrl_data_pointer{
	struct ltm_init * p_init_ptr;
};

// AP to CCU. Pass every VSYNC
struct ltm_ctrl_ccu_vsync_info{
	// Face, Touch
};

// AP to CCU. Pass if changed
struct ltm_ctrl_run_time_info{
	//struct CCU_LTM_NVRAM_T* pltm_nvram;
	const struct ccu_ltm_nvram *p_ltm_nvram;
};

struct ltm_ctrl_get_exif_ptr{
	struct ltm_init * p_init_ptr;
	struct ccu_ltm_algo_data* p_ltm_algo_data_ptr;
};


struct ltm_core_ctrl_data_pointer {
	struct ltm_init *p_init_ptr;
};

struct ltm_core_ctrl_get_algo_ptr {
	struct ltm_init *p_init_ptr;
	struct ltm_ctrl_ccu_vsync_info *p_vsync_info_ptr;
	struct ccu_ltm_algo_data *p_ltm_algo_data_ptr;
};

#define PTR_CAST(ptr_dst, ptr_src) (*ptr_dst = ptr_src)

void ltm_core_init(enum ltm_cam_id_enum cam_id, struct ltm_main_in *in, struct ltm_main_out *out, uint32_t *lock_cnt);
void ltm_core_main(struct ltm_main_in *in, struct ltm_main_out *out, uint32_t part);
void ltm_core_ctrl(enum ltm_ctrl_enum id, void *ctrl_in, void *ctrl_out, enum ltm_cam_id_enum cam_id);
uint32_t ltms_core_main(enum ltm_cam_id_enum cam_id, uint8_t * hist);
void ltm_curve_sync(enum ltm_cam_id_enum cam_id, struct ltm_main_out *master_cam_out, struct ltm_main_out *slave_cam_out);

#endif