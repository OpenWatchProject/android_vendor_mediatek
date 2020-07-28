#ifndef __CCU_HLR_NVRAM_H__
#define __CCU_HLR_NVRAM_H__
#include "ccu_hlr_utility.h"


enum hlr_mode_enum {
	CCU_HLR_MODE_OFF = 0,
	CCU_HLR_MODE_ON
};
struct hlr_default {
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
};

struct ccu_hlr_nvram {
	uint32_t lv[4];
	uint32_t cct[3];
	enum hlr_mode_enum hlr_mode;
	struct hlr_default hlr_default;
	uint32_t hlr_lut[3][4][3];
	uint32_t hlr_reserv_lut[4][3];
	uint32_t reserv0;
	uint32_t reserv1;
	uint32_t reserv2;
	uint32_t reserv3;
	uint32_t reserv4;
	uint32_t reserv5;
	uint32_t reserv6;
	uint32_t reserv7;
};


#endif
