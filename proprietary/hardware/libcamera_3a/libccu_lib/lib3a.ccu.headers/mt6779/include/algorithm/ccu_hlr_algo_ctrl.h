#ifndef __CCU_HLR_CTRL_H__
#define __CCU_HLR_CTRL_H__
#include "ccu_hlr_data.h"
#include "ccu_hlr_utility.h"
#include "ccu_hlr_nvram.h"

#define HLR_ALGO_IF_REVISION 8292001

// For CCU drv contrl.
enum hlr_ctrl_enum {
	HLR_SET_VSYNC_INFO,
	HLR_SET_RUN_TIME_INFO,
	HLR_SET_LAST_RESULT,
	HLR_GET_INIT_DATA_POINTER_MAIN,
	HLR_GET_INIT_DATA_POINTER_MAIN2,
	HLR_GET_EXIF_PTR,
	HLR_MAX
};

// For multicam.
enum hlr_cam_id_enum {
	HLR_CAM_ID_MAIN,
	HLR_CAM_ID_MAIN2,
	HLR_CAM_ID_SUB,
	HLR_CAM_ID
};

// CCU drv input to Algo.
struct hlr_main_in {
	int32_t  cam_id;	
	int32_t  master_cam_id;

	//Input Info
	int32_t lv_input;	//aeout	N-1
	int32_t cct_input;	//MGR
	uint32_t ltm_clip_th;	//N
	uint32_t hdr_ratio;	//Fix
	uint32_t dgn_gr; 	//aeout	N-1
	uint32_t dgn_gb;	//aeout	N-1
	uint32_t dgn_r;	//aeout	N-1
	uint32_t dgn_b;	//aeout	N-1
	enum hlr_mode_enum hlr_mode;	//LTM ON/OFF
};

// Algo output to CCU drv.
struct hlr_main_out {
	struct hlr_default hlr_out_hw_setting;
};

struct hlr_ctrl_in {
	struct hlr_main_in *p_hlr_main_in;
};

struct hlr_ctrl_out {
	struct hlr_main_out *p_hlr_main_out;
};

struct hlr_init {
	struct ccu_hlr_nvram *p_hlr_nvram;
};

struct hlr_init_out {
	struct hlr_default hlr_out_hw_setting;
};
// For CCU drv, filling initialized data.
struct hlr_ctrl_data_pointer {
	struct hlr_init * p_init_ptr;
};

// AP to CCU. Pass every VSYNC
struct hlr_ctrl_ccu_vsync_info {
	// Face, Touch
};

// AP to CCU. Pass if changed
struct hlr_ctrl_run_time_info {
	//struct CCU_HLR_NVRAM_T* pltm_nvram;
	const struct ccu_hlr_nvram  *p_hlr_nvram;
};

struct hlr_ctrl_get_exif_ptr {
	struct hlr_init * p_init_ptr;
	struct ccu_hlr_algo_data* p_hlr_algo_data_ptr;
};


struct hlr_core_ctrl_data_pointer {
	struct hlr_init *p_init_ptr;
};

struct hlr_core_ctrl_get_algo_ptr {
	struct hlr_init *p_init_ptr;
	struct hlr_ctrl_ccu_vsync_info *p_vsync_info_ptr;
	struct ccu_hlr_algo_data *p_hlr_algo_data_ptr;
};

#define PTR_CAST(ptr_dst, ptr_src) (*ptr_dst = ptr_src)

void hlr_core_init(enum hlr_cam_id_enum hlr_cam_id, struct hlr_init *in, struct hlr_init_out *out);
void hlr_core_main(struct hlr_main_in *in, struct hlr_main_out *out);
void hlr_core_ctrl(enum hlr_ctrl_enum id, void *ctrl_in, void *ctrl_out, enum hlr_cam_id_enum cam_id);
void hlr_data_select(enum hlr_cam_id_enum cam_id);
void hlr_dual_sync(struct hlr_main_out *master_cam_out,struct hlr_main_out *slave_cam_out);

#endif
