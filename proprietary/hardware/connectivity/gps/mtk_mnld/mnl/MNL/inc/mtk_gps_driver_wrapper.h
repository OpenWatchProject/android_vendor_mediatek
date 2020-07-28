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
 *   mtk_gps_driver_wrapper.h
 *
 * Description:
 * ------------
 *   Prototype of  driver layer API
 *
 ****************************************************************************/

#ifndef MTK_GPS_DRIVER_WRAPPER_H
#define MTK_GPS_DRIVER_WRAPPER_H

#include "mtk_gps_type.h"

#ifdef __cplusplus
  extern "C" {
#endif

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_threads_create
 * DESCRIPTION
 *  Create MNL thread in the porting layer
 * PARAMETERS
 * void
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32
mtk_gps_threads_create(MTK_GPS_THREAD_ID_ENUM thread_id);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_threads_release
 * DESCRIPTION
 *  Release MNL thread (array) in the porting layer
 * PARAMETERS
 * void
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32
mtk_gps_threads_release(void);

//----------------------------------------------------------------------------------

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_mnl_run
 * DESCRIPTION
 *  RUN MTK Nav Library
 * PARAMETERS
 *  default_cfg     [IN]  factory default configuration
 *  driver_dfg       [IN]  UART/COM PORT setting
 * RETURNS
 *  MTK_GPS_BOOT_STATUS
 *****************************************************************************/
MTK_GPS_BOOT_STATUS
mtk_gps_mnl_run(const MTK_GPS_INIT_CFG* default_cfg, const MTK_GPS_DRIVER_CFG* driver_dfg);
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_mnl_stop
 * DESCRIPTION
 *  STOP MTK Nav Library
 * PARAMETERS
 * void
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_mnl_stop(void);


INT32
mtk_gps_sys_event_wait_timeout(MTK_GPS_EVENT_ENUM event_idx,int time_sec);


#ifdef __cplusplus
   }
#endif

#endif /* MTK_GPS_DRIVER_WRAPPER_H */
