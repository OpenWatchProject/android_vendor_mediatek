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
 *   agps_agent.h
 *
 * Description:
 * ------------
 *   Prototype of MTK AGPS Agent related functions
 *
 ****************************************************************************/

#ifndef MTK_AGPS_AGENT_H
#define MTK_AGPS_AGENT_H


#if defined(WIN32) || defined(WINCE)
#include "windows.h"
#else
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#endif
#include "mtk_gps_type.h"
#include "MTK_Type.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* Copy required parameters from MNL internal header files.
   Remember to sync with MNL when any of these are changed. */

#define PMTK_MAX_PKT_LENGTH     256
#define MGlONID  (24)

// message between mnl and agps agent
#define MTK_GPS_EVENT_BASE          (0)
#define MTK_GPS_MSG_BASE            (MTK_GPS_EVENT_BASE+500)
#define MTK_GPS_MNL_MSG             (MTK_GPS_MSG_BASE+1) //msg to mnl
#define MTK_GPS_MNL_EPO_MSG         (MTK_GPS_MSG_BASE+2) //msg to agent epo module
#define MTK_GPS_MNL_BEE_MSG         (MTK_GPS_MSG_BASE+3) //msg to agent bee module
#define MTK_GPS_MNL_SUPL_MSG        (MTK_GPS_MSG_BASE+4) //msg to agent supl module
//#define MTK_GPS_MNL_EPO_MSG         (MTK_GPS_MSG_BASE+5) //msg to agent epo module
#define MTK_GPS_MNL_BEE_IND_MSG         (MTK_GPS_MSG_BASE+6) //msg to agent bee module
//#define MTK_GPS_MNL_SUPL_MSG        (MTK_GPS_MSG_BASE+7) //msg to agent supl module

#define AGPS_AGENT_DEBUG

//agps state machine
#define AGENT_ST_IDLE     (0)
#define AGENT_ST_WORKING  (1)

#define AGENT_WORK_MODE_IDLE          (0)
#define AGENT_WORK_MODE_EPO           (1)
#define AGENT_WORK_MODE_BEE           (2)
#define AGENT_WORK_MODE_SUPL_SI       (3)
#define AGENT_WORK_MODE_SUPL_NI       (4)
#define AGENT_WORK_MODE_WIFI_AIDING   (5)
#define AGENT_WORK_MODE_CELLID_AIDING (6)

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_agent_init
 * DESCRIPTION
 *  Initialize AGPS Agent module
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_init(UINT32 u4Param);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_proc
 * DESCRIPTION
 *  process the message recv. from agps agent thread
 * PARAMETERS
 *  prmsg       [IN]   the message recv. from agps agent thread
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_agps_agent_proc (MTK_GPS_AGPS_AGENT_MSG *prmsg);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_epo_init
 * DESCRIPTION
 *  Initialize EPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_epo_init(UINT8 *epo_file_name, UINT8 *epo_update_file_name);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_epo_uninit
 * DESCRIPTION
 *  Un-initialize EPO module
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_epo_uninit();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_init
 * DESCRIPTION
 *  Initialize QEPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_init(UINT8 *qepo_file_name, UINT8 *qepo_update_file_name);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_uninit
 * DESCRIPTION
 *  Un-initialize QEPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_uninit();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_bd_init
 * DESCRIPTION
 *  Initialize QEPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_bd_init(UINT8 *qepo_file_name, UINT8 *qepo_update_file_name);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_bd_uninit
 * DESCRIPTION
 *  Un-initialize QEPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_bd_uninit();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_ga_init
 * DESCRIPTION
 *  Initialize QEPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_ga_init(UINT8 *qepo_file_name, UINT8 *qepo_update_file_name);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_ga_uninit
 * DESCRIPTION
 *  Un-initialize QEPO module
 * PARAMETERS
 *  path             [IN]  single-byte null-terminated string of BEET000A folder path
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_ga_uninit();


/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_epo_read_gps_time
 * DESCRIPTION
 *    Get start time/expire time of the EPO file [gps time]
 * PARAMETERS
 *  pFile: EPO file fd
 *  [out]u4GpsSecs: GPS seconds
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_epo_read_gps_time(UINT32 *u4GpsSecs_start, UINT32 *u4GpsSecs_expire);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_epo_read_utc_time
 * DESCRIPTION
 *    Get start time/expire time of the EPO file [utc time]
 * PARAMETERS
 *  pFile: EPO file fd
 *  [out]u4GpsSecs: GPS seconds
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_epo_read_utc_time(time_t *uSecond_start, time_t *uSecond_expire);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_epo_extract_data
 * DESCRIPTION
 *    Extract ading data from EPO file
 * PARAMETERS
 *  [IN]u4GpsSec: segment time
 *  [out]epo_data: ading data
 * RETURNS
 * success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_epo_extract_data(UINT32 u4GpsSecs, MTK_GPS_PARAM_EPO_DATA_CFG *epo_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_epo_file_update
 * DESCRIPTION
 *    Update the EPO file after the new Epo file download
 * PARAMETERS
 *      None
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_epo_file_update();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_file_update
 * DESCRIPTION
 *    Update the qEPO file after the new qEpo file download
 * PARAMETERS
 *      None
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_file_update();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_bd_file_update
 * DESCRIPTION
 *    Update the qEPO file after the new qEpo file download
 * PARAMETERS
 *      None
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_bd_file_update();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_ga_file_update
 * DESCRIPTION
 *    Update the qEPO file after the new qEpo file download
 * PARAMETERS
 *      None
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_ga_file_update();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_get_rqst_time
 * DESCRIPTION
 *    Get the GPS time and system time of the latest QEPO request
 * PARAMETERS
 *      wn  [OUT]:The week number of GPS time
 *      tow [OUT]:The Time Of Week of GPS time
 *      sys_time    [OUT]:The system time of the latest QEPO request, the unit is second
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_agps_agent_qepo_get_rqst_time(UINT16 *wn, UINT32 *tow, UINT32 *sys_time);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_qepo_file_verify

 *****************************************************************************/

INT32
mtk_agps_agent_qepo_file_verify(MTK_FILE hFile, UINT8 *qepo_type, INT64 *file_size);


#ifdef __cplusplus
   }
#endif

#endif /* MTK_GPS_H */

