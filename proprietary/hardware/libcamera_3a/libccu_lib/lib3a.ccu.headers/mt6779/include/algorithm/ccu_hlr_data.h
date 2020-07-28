#ifndef __CCU_HLR_DATA_H__
#define __CCU_HLR_DATA_H__
#include "ccu_hlr_param.h"

struct ccu_hlr_algo_data {
    uint32_t frame_idx;
	uint32_t hlr_version; 
	//input info
	int32_t  cam_id;	
	int32_t  master_cam_id;
	int32_t lv_input;	//aeout	N-1
	int32_t cct_input;	//MGR
	uint32_t ltm_clip_th;	//N
	uint32_t hdr_ratio;	//Fix
	uint32_t dgn_gr; 	//aeout	N-1
	uint32_t dgn_gb;	//aeout	N-1
	uint32_t dgn_r;	//aeout	N-1
	uint32_t dgn_b;	//aeout	N-1
	uint32_t hlr_mode;	//LTM ON/OFF
	
	//HW Setting
	uint32_t hlr_sat_0;
	uint32_t hlr_sat_1;
	uint32_t hlr_sat_2;
	uint32_t hlr_sat_3;
	uint32_t hlr_sat_4;
	uint32_t hlr_sat_5;
	uint32_t hlr_sat_6;
	uint32_t hlr_sat_7;
	uint32_t hlr_node_0;
	uint32_t hlr_node_1;
	uint32_t hlr_node_2;
	uint32_t hlr_node_3;
	uint32_t hlr_node_4;
	uint32_t hlr_node_5;
	uint32_t hlr_node_6;
	uint32_t hlr_node_7;
	uint32_t hlr_bld_high_swo;  //sw       
	uint32_t hlr_bld_low_swo;   //sw          
	uint32_t hlr_prt_high_swo;  //sw          
	uint32_t hlr_prt_low_swo;   //sw          
	uint32_t hlr_prt_en_swo;    //sw          
	uint32_t hlr_slp_0;
	uint32_t hlr_slp_1;
	uint32_t hlr_slp_2;
	uint32_t hlr_slp_3;
	uint32_t hlr_slp_4;
	uint32_t hlr_slp_5;
	uint32_t hlr_max_rat;
	uint32_t hlr_blue_prt_str;
	uint32_t hlr_red_prt_str;
	uint32_t hlr_slp_6;
	uint32_t hlr_bld_fg;
	uint32_t hlr_bld_high;
	uint32_t hlr_bld_slp;
	uint32_t hlr_prt_th;
	uint32_t hlr_prt_slp;
	uint32_t hlr_prt_en;
	uint32_t hlr_clip_val;
	uint32_t hlr_off_clip_val;
	uint32_t hlr_efct_on;
	uint32_t hlr_tile_edge;
	uint32_t hlr_tdr_wd;
	uint32_t hlr_tdr_ht;
	uint32_t hlr_r1_en; //resrve top ctrl 
	uint32_t hlr_d1_en; //resrve top ctrl
	uint32_t hlr_r1_lkmsb; //resrve top ctrl
	uint32_t hlr_d1_lkmsb; //resrve top ctrl
	uint32_t hlr_r1_lkmode; //resrve top ctrl 
	uint32_t hlr_d1_lkmode; //resrve top ctrl
	uint32_t hlr_resrv_a; //resrve top ctrl
	
	/* HLR NVRAM */
	struct ccu_hlr_nvram *p_hlr_nvram;
};

#endif
