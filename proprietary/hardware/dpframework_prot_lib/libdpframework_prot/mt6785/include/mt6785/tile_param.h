#ifndef __TILE_PARAM_H__
#define __TILE_PARAM_H__

#include "tpipe_config.h"

/* only refer by tile core, tile driver, & ut entry file only */
typedef struct TILE_PARAM_STRUCT
{
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map;
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param;
    ISP_TILE_HEX_DUMP_STRUCT *ptr_isp_tile_hex_dump_param;
    ISP_TPIPE_DESCRIPTOR_STRUCT *ptr_isp_tile_descriptor;
	DIRECT_LINK_DUMP_STRUCT *ptr_direct_link_dump_param;
}TILE_PARAM_STRUCT;

/* prototype main flow */
/* current c model ut wrapper */
extern ISP_TILE_MESSAGE_ENUM tile_cal_main(TILE_PARAM_STRUCT* p_tile_param);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count,
					unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_internal(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_wpe(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_eaf(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_bld(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_internal_wpe(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_internal_eaf(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
extern ISP_TILE_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg_internal_bld(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
					unsigned int *hex_ptr, unsigned int max_count, unsigned int config_no_per_tile);
/* prototype dp interface */
extern ISP_TILE_MESSAGE_ENUM tile_cal_main_dp(int dp_mode, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
			FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param, ISP_TILE_HEX_DUMP_STRUCT *ptr_isp_tile_hex_dump_param,
			ISP_TPIPE_DESCRIPTOR_STRUCT *ptr_isp_tile_descriptor);
extern ISP_TILE_MESSAGE_ENUM tile_proc_main_single(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
					FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param, ISP_TILE_HEX_DUMP_STRUCT *ptr_isp_tile_hex_dump_param,
					ISP_TPIPE_DESCRIPTOR_STRUCT *ptr_isp_tile_descriptor, int tile_no, bool *stop_flag, FILE *fpt_log);
extern ISP_TILE_MESSAGE_ENUM tile_main_read_isp_reg_file(TILE_REG_MAP_STRUCT *ptr_tile_reg_map, const char* filename,
							const char* filename_d, const char* filename_wpe, const char* filename_wpe_d, const char* filename_eaf);
extern ISP_TILE_MESSAGE_ENUM tile_fprint_reg_map(TILE_REG_MAP_STRUCT *ptr_tile_reg_map, const char *filename,
							const char *filename_d, const char *filename_wpe, const char *filename_wpe_d, const char *filename_eaf);
extern ISP_TILE_MESSAGE_ENUM tile_copy_config_by_platform(TILE_PARAM_STRUCT* p_tile_param,
                            const ISP_TPIPE_CONFIG_STRUCT *ptr_isp_config);
#ifndef TILE_PLATFORM_DRIVER
/* module sim check */
extern ISP_TILE_MESSAGE_ENUM tile_module_sim_main(TILE_PARAM_STRUCT* p_tile_param);
#endif
#endif
