/***********************************************************************
*   This software/firmware and related documentation ("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc. (C) [2008]. All rights reserved.
*
*************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   mtk_gps_sys_fp.h
 *
 * Description:
 * ------------
 *   Marco porting layer APT to Function pointer using by MTK navigation library
 *
 ****************************************************************************/

#ifndef MTK_GPS_SYS_FP_MACRO_H
#define MTK_GPS_SYS_FP_MACRO_H


#ifdef __cplusplus
  extern "C" {
#endif


#define  mtk_gps_sys_gps_mnl_callback                   gfpmtk_gps_sys_gps_mnl_callback
#define  mtk_gps_sys_nmea_output_to_app                 gfpmtk_gps_sys_nmea_output_to_app
#define  mtk_gps_sys_nmea_output_to_mnld                gfpmtk_gps_sys_nmea_output_to_mnld
#define  mtk_gps_sys_frame_sync_enable_sleep_mode       gfpmtk_gps_sys_frame_sync_enable_sleep_mode
#define  mtk_gps_sys_frame_sync_meas_req_by_network     gfpmtk_gps_sys_frame_sync_meas_req_by_network
#define  mtk_gps_sys_frame_sync_meas_req                gfpmtk_gps_sys_frame_sync_meas_req
#define  mtk_gps_sys_agps_disaptcher_callback           gfpmtk_gps_sys_agps_disaptcher_callback
#define  mtk_gps_sys_pmtk_cmd_cb                        gfpmtk_gps_sys_pmtk_cmd_cb
#define  SUPL_encrypt                                   gfpSUPL_encrypt
#define  SUPL_decrypt                                   gfpSUPL_decrypt
#define  mtk_gps_sys_alps_gps_dbg2file_mnld             gfmtk_gps_sys_alps_gps_dbg2file_mnld

#define mtk_gps_ofl_sys_rst_stpgps_req                  gfmtk_gps_ofl_sys_rst_stpgps_req
#define mtk_gps_ofl_sys_submit_flp_data                 gfmtk_gps_ofl_sys_submit_flp_data
#define mtk_gps_ofl_sys_mnl_offload_cb                  gfmtk_gps_ofl_sys_mnl_offload_cb
#define mtk_gps_sys_msg_output_to_app                   gfpmtk_gps_sys_msg_output_to_app
#define _mtk_gps_sys_do_hw_suspend_resume               gfmtk_gps_sys_do_hw_suspend_resume

#ifdef __cplusplus
   }
#endif

#endif /* MTK_GPS_SYS_FP_MACRO_H */
