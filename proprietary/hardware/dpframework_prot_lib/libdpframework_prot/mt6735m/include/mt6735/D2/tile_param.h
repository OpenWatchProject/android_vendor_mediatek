#ifndef __TILE_PARAM_H__
#define __TILE_PARAM_H__

#include "tpipe_config.h"

/* only refer by tile core, tile driver, & ut entry file only */
typedef struct TILE_PARAM_STRUCT
{
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map;
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param;
    TILE_REG_DUMP_STRUCT  *ptr_tile_reg_dump_param;
    ISP_TILE_HEX_DUMP_STRUCT *ptr_isp_tile_hex_dump_param;
	ISP_TPIPE_DESCRIPTOR_STRUCT *ptr_isp_tile_descriptor;
    MDP_TILE_HEX_DUMP_STRUCT *ptr_mdp_tile_hex_dump_param;
}TILE_PARAM_STRUCT;

/* prototype main flow */
extern ISP_MESSAGE_ENUM tile_driver_init(TILE_PARAM_STRUCT *p_tile_param);

extern ISP_MESSAGE_ENUM tile_cal_main(TILE_PARAM_STRUCT* p_tile_param);

extern ISP_MESSAGE_ENUM tile_proc_main_single(int               tile_no,
                                              FILE              *fpt_log,
                                              bool              *stop_flag,
                                              TILE_PARAM_STRUCT *p_tile_param);

extern ISP_MESSAGE_ENUM tile_fprint_platform_config(const ISP_TPIPE_CONFIG_STRUCT *ptr_tile_config,
                                                    const char                    *filename,
                                                    TILE_REG_MAP_STRUCT           *ptr_tile_reg_map);

extern ISP_MESSAGE_ENUM tile_cal_main_dp(int dp_mode, TILE_PARAM_STRUCT *p_tile_param);

extern ISP_MESSAGE_ENUM tile_copy_config_by_platform(TILE_PARAM_STRUCT* p_tile_param,
                            const ISP_TPIPE_CONFIG_STRUCT *ptr_isp_config,
							const MDP_TILE_CONFIG_STRUCT *ptr_mdp_config);



#endif
