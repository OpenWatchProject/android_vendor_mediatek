#ifndef __DP_WRAPPER_TILE_H__
#define __DP_WRAPPER_TILE_H__

#include "tile_param.h"

ISP_MESSAGE_ENUM wrapper_tile_cal_main_dp(int dp_mode,
                                          TILE_PARAM_STRUCT *p_tile_param)
{
    return tile_cal_main_dp(dp_mode,
                            p_tile_param->ptr_tile_reg_map,
                            p_tile_param->ptr_tile_func_param,
                            p_tile_param->ptr_isp_tile_hex_dump_param,
                            p_tile_param->ptr_isp_tile_descriptor);
}

ISP_MESSAGE_ENUM wrapper_tile_proc_main_single(TILE_PARAM_STRUCT *p_tile_param,
                                               int tile_no,
                                               bool *stop_flag,
                                               FILE *fpt_log)
{
    return tile_proc_main_single(p_tile_param->ptr_tile_reg_map,
                                 p_tile_param->ptr_tile_func_param,
                                 p_tile_param->ptr_isp_tile_hex_dump_param,
                                 p_tile_param->ptr_isp_tile_descriptor,
                                 tile_no,
                                 stop_flag,
                                 fpt_log);
}

#if CONFIG_FOR_VERIFY_FPGA
ISP_MESSAGE_ENUM wrapper_tile_fprint_reg_map(TILE_REG_MAP_STRUCT *ptr_tile_reg_map)
{
    return tile_fprint_reg_map(ptr_tile_reg_map,
                               "tile_reg_map.txt",
                               "tile_reg_map_d.txt",
                               "tile_reg_map_wpe.txt",
                               "tile_reg_map_wpe_d.txt",
                               "tile_reg_map_eaf.txt");
}

ISP_MESSAGE_ENUM wrapper_tile_main_read_isp_reg_file(TILE_REG_MAP_STRUCT *ptr_tile_reg_map)
{
    return tile_main_read_isp_reg_file(ptr_tile_reg_map,
                                       "tile_reg_map.txt",
                                       "tile_reg_map_d.txt",
                                       "tile_reg_map_wpe.txt",
                                       "tile_reg_map_wpe_d.txt",
                                       "tile_reg_map_eaf.txt");
}
#endif
#endif  // __DP_WRAPPER_TILE_H__