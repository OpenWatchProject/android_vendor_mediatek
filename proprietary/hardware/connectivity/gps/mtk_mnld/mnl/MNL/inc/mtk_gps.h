/***********************************************************************
*   This software/firmware and related documentation("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc.(C) [2008]. All rights reserved.
*
*************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   mtk_gps.h
 *
 * Description:
 * ------------
 *   Prototype of MTK navigation library
 *
 ****************************************************************************/

#ifndef MTK_GPS_H
#define MTK_GPS_H

#include "mtk_gps_type.h"
#include "mtk_gps_agps.h"

#include "mtk_gps_driver_wrapper.h"
#ifdef USING_NAMING_MARC
#include "mtk_gps_macro.h"
#endif


#ifdef __cplusplus
  extern "C" {
#endif
/* ================= Application layer functions ================= */

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_run
 * DESCRIPTION
 *  The main routine for the MTK Nav Library task
 * PARAMETERS
 *  application_cb      [IN]
 *  default_cfg(mtk_init_cfg)       [IN]
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_run(MTK_GPS_CALLBACK application_cb, const MTK_GPS_INIT_CFG* default_cfg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_hotstill_run
 * DESCRIPTION
 *  The main routine for the HotStill task
 * PARAMETERS
  *  driver_cfg     [IN]  factory default configuration
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_hotstill_run(MTK_GPS_DRIVER_CFG* driver_cfg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_agent_run
 * DESCRIPTION
 *  The main routine for the Agent task
 * PARAMETERS
  *  driver_cfg     [IN]  factory default configuration
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_agent_run(MTK_GPS_DRIVER_CFG* driver_cfg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_update_gps_data
 * DESCRIPTION
 *  Force to write NV-RAM data to storage file
 * PARAMETERS
 *
 * RETURNS
 *  None
 *****************************************************************************/
INT32
mtk_gps_update_gps_data(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_data_input
 * DESCRIPTION
 *
 * PARAMETERS
 *  buffer      [IN] the content of the gps measuremnt
 *  length      [IN] the length of the gps measurement
 *  p_accepted_length [OUT]  indicate how many data was actually accepted into library
 *                          if this value is not equal to length, then it means library internal
 *                          fifo is full, please let library task can get cpu usage to digest input data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_data_input(const char* buffer, UINT32 length, UINT32* p_accepted_length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_rtcm_input
 * DESCRIPTION
 *  accept RTCM differential correction data
 * PARAMETERS
 *  buffer      [IN]   the content of RTCM data
 *  length      [IN]   the length of RTCM data(no more than 1KB)
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_rtcm_input(const char* buffer, UINT32 length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_nmea_input
 * DESCRIPTION
 *  accept NMEA(PMTK) sentence raw data
 * PARAMETERS
 *  buffer      [IN]   the content of NMEA(PMTK) data
 *  length      [IN]   the length of NMEA(PMTK) data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_nmea_input(const char* buffer, UINT32 length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_agps_input
 * DESCRIPTION
 *  accept NMEA(PMTK) sentence raw data(only for agent using)
 * PARAMETERS
 *  buffer      [IN]   the content of NMEA(PMTK) data
 *  length      [IN]   the length of NMEA(PMTK) data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_agps_input(const char* buffer, UINT32 length);


/* ====================== Utility functions ====================== */
/*  These functions must be used in application_cb() callback
    function specified in mtk_gps_run()                            */

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_position
 * DESCRIPTION
 *  obtain detailed fix information
 * PARAMETERS
 *  pvt_data    [OUT]  pointer to detailed fix information
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_position(MTK_GPS_POSITION* pvt_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GPS/QZSS
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_sv_info(MTK_GPS_SV_INFO* sv_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_gleo_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GALIILEO
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_gleo_sv_info(MTK_GLEO_SV_INFO* sv_gleo_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_glon_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GALIILEO
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_glon_sv_info(MTK_GLON_SV_INFO* sv_glon_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_bedo_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GALIILEO
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_bedo_sv_info(MTK_BEDO_SV_INFO* sv_bedo_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_param
 * DESCRIPTION
 *  Get the current setting of the GPS receiver
 * PARAMETERS
 *  key         [IN]   the configuration you want to know
 *  value       [OUT]  the current setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 * EXAMPLE
 *  //  get the current DGPS mode
 *  mtk_param_dgps_config param_dgps_config;
 *  mtk_gps_get_param(MTK_PARAM_DGPS_CONFIG, &param_dgps_config);
 *  printf("DGPS mode=%d",(int)param_dgps_config.dgps_mode);
 *****************************************************************************/

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sv_list
 * DESCRIPTION
 *  Return SV list(Elev >= 5)
 * PARAMETERS
 *   *UINT32 SV list bit map
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_sv_list(UINT32 *svListBitMap);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_param
 * DESCRIPTION
 *  Get the current setting of the GPS receiver
 * PARAMETERS
 *  key         [IN]   the configuration you want to know
 *  value       [OUT]  the current setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 * EXAMPLE
 *  //  get the current DGPS mode
 *  mtk_param_dgps_config param_dgps_config;
 *  mtk_gps_get_param(MTK_PARAM_DGPS_CONFIG, &param_dgps_config);
 *  printf("DGPS mode=%d",(int)param_dgps_config.dgps_mode);
 *****************************************************************************/
INT32
mtk_gps_get_param(MTK_GPS_PARAM key, void* value);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_param
 * DESCRIPTION
 *  Change the behavior of the GPS receiver
 * PARAMETERS
 *  key         [IN]   the configuration needs to be changed
 *  value       [IN]   the new setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_param(MTK_GPS_PARAM key, const void* value);
INT32
mtk_gps_get_mnl_info(MTK_GPS_MNL_INFO* lib_info);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_mnl_create_dbg_port
 * DESCRIPTION
 *  Create debug socket in running time
 *  If debug socket is disabled at start time, will create it
 *  If debug socket is enabled at start time, will not create it repeatedly
 * PARAMETERS
 *  port_num         [IN]   the port number needs to be created
 * RETURNS
 *  void
 *****************************************************************************/
void mtk_gps_mnl_create_dbg_port(UINT32 port_num);

//****************************************************************************
// mtk_gps_set_nmea_format : Set NMEA format
//
// Parameters:
//  Mode = 0 : NMEA show as normal
//  Mode = 1 : NMEA show
//                       $GPGGA,,,,,,0,,,,,,,,*66
//                       $GPVTG,,T,,M,,N,,K,N*2C
//                       $GPRMC,,V,,,,,,,,,,N*53
//  Mode > 1 : Reserved
//
//
// Description :
//    show different GGA/VTF/RMC format when GNSS in no-fix status
//
UINT8 mtk_gps_set_nmea_format(UINT8 *NMEA_format);

// *****************************************************************************
//  mtk_gps_set_navigation_speed_threshold :
//  The function will keep fix the postion output to the same point if the current
//  estimated 3D speed is less than SpeedThd or the distance of true position and the fix points
//  are large than 20 meter.
//  Parameters :
//               SpeedThd must be >= 0
//               The unit of SpeedThd is [meter/second]
//               if SpeedThd = 0,  The navigation speed threshold function will be disabled.
//  For exapmle,
//  The fix points will keep to the same points until estimated speed is large than 0.4m/s
//
//  float SpeedThd = 0.4;
//  MTK_Set_Navigation_Speed_Threshold(SpeedThd);
//

INT32
mtk_gps_set_navigation_speed_threshold(float SpeedThd);

// *****************************************************************************
//  mtk_gps_get_navigation_speed_threshold :
//  Query the current Static Navigation Speed Threshold
//  Parameters :  The unit of SpeedThd  is [meter/second]
//  if SpeedThd = 0, The navigation speed threshold is not functional.

INT32
mtk_gps_get_navigation_speed_threshold(float *SpeedThd);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_pmtk_data
 * DESCRIPTION
 *  send PMTK command to GPS receiver
 * PARAMETERS
 *  prPdt       [IN]   pointer to the data structure of the PMTK command
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_pmtk_data(const MTK_GPS_PMTK_DATA_T *prPdt);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_pmtk_response
 * DESCRIPTION
 *  obtain detailed information of PMTK response
 * PARAMETERS
 *  rs_data     [OUT]  pointer to PMTK response data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_pmtk_response(MTK_GPS_PMTK_RESPONSE_T *prRsp);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_position
 * DESCRIPTION
 *  Set the receiver's initial position
 *  Notes: To make the initial position take effect, please invoke restart
 *        (hot start/warm start) after this function
 * PARAMETERS
 *  LLH         [IN]  LLH[0]: receiver latitude in degrees(positive for North)
 *                    LLH[1]: receiver longitude in degrees(positive for East)
 *                    LLH[2]: receiver WGS84 height in meters
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_position(const double LLH[3]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_time
 * DESCRIPTION
 *  Set the current GPS time
 *  Note:       The time will not be set if the receiver has better knowledge
 *              of the time than the new value.
 * PARAMETERS
 *  weekno      [IN]   GPS week number(>1024)
 *  TOW         [IN]   time of week(in second; 0.0~684800.0)
 *  timeRMS     [IN]   estimated RMS error of the TOW value(sec^2)
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_time(UINT16 weekno, double tow, float timeRMS);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_ephemeris
 * DESCRIPTION
 *  Upload ephemeris
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN(1~32)
 *  data        [IN]   binary ephemeris words from words 3-10 of subframe 1-3
 *                     all parity bits(bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3, subframe 1
 *                     data[1]: bit 21-14 of word 3, subframe 1
 *                     data[2]: bit 29-22 of word 3, subframe 1
 *                     data[3]: bit 13-6 of word 4, subframe 1
 *                     ......
 *                     data[71]: bit 29-22 of word 10, subframe 3
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_ephemeris(UINT8 svid, const char data[72]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_tcxo_mode
 * DESCRIPTION
 *  Set MNL TCXO mode
 * PARAMETERS
 *

 *  MTK_TCXO_NORMAL,  // normal mode
 *  MTK_TCXO_PHONE    // phone mode

 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_set_tcxo_mode(MTK_GPS_TCXO_MODE tcxo_mode);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_AIC_mode
 * DESCRIPTION
 *  Set AIC mode
 * PARAMETERS
 *
 * disalbe = 0
 * enable = 1

 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_set_AIC_mode(MTK_GPS_AIC_MODE AIC_Enable);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_Custom_AIC_mode
 * DESCRIPTION
 *  Set Custom AIC
 * PARAMETERS
 *  EnabledOrNot : 0 = disable
 *                 1 = enable
 *  Frequency    : GPS     : 1575420000 +/- 8000000 (Unit: Hz)
 *                 Glonass : 1602000000 +/- 8000000 (Unit: Hz) @ GGGB mode
 *                           1601796000 +/- 8000000 (Unit: Hz) @ GGG  mode
 *                 Beidou  : 1561098000 +/- 8000000 (Unit: Hz) @ GGGB mode
 *                           1561092000 +/- 8000000 (Unit: Hz) @ GB   mode
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_Custom_AIC_mode (unsigned char EnabledOrNot, unsigned int Frequency);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_Custom_AIC_mode
 * DESCRIPTION
 *  Get AIC mode
 * PARAMETERS
 *  Frequency    : GPS     : 1575420000 +/- 8000000 (Unit: Hz)
 *                 Glonass : 1602000000 +/- 8000000 (Unit: Hz) @ GGGB mode
 *                           1601796000 +/- 8000000 (Unit: Hz) @ GGG  mode
 *                 Beidou  : 1561098000 +/- 8000000 (Unit: Hz) @ GGGB mode
 *                           1561092000 +/- 8000000 (Unit: Hz) @ GB   mode
 *
 *****************************************************************************/
void
mtk_gps_get_Custom_AIC_mode (unsigned int Frequency[12]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_almanac
 * DESCRIPTION
 *  Upload almanac
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN(1~32)
 *  weekno      [IN]   the week number of the almanac data record
 *  data        [IN]   binary almanac words from words 3-10 of either subframe 4
 *                     pages 2-10 or subframe 5 pages 1-24
 *                     all parity bits(bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3
 *                     data[1]: bit 21-14 of word 3
 *                     data[2]: bit 29-22 of word 3
 *                     data[3]: bit 13-6 of word 4
 *                     ......
 *                     data[23]: bit 29-22 of word 10
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_almanac(UINT8 svid, UINT16 weekno, const char data[24]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_ephemeris
 * DESCRIPTION
 *  Download ephemeris
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN(1~32)
 *  data        [OUT]  binary ephemeris words from words 3-10 of subframe 1-3
 *                     all parity bits(bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3, subframe 1
 *                     data[1]: bit 21-14 of word 3, subframe 1
 *                     data[2]: bit 29-22 of word 3, subframe 1
 *                     data[3]: bit 13-6 of word 4, subframe 1
 *                     ......
 *                     data[71]: bit 29-22 of word 10, subframe 3
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_ephemeris(UINT8 svid, char data[72]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_almanac
 * DESCRIPTION
 *  Download almanac
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN(1~32)
 *  p_weekno    [OUT]  pointer to the week number of the almanac data record
 *  data        [OUT]  binary almanac words from words 3-10 of either subframe 4
 *                     pages 2-10 or subframe 5 pages 1-24
 *                     all parity bits(bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3
 *                     data[1]: bit 21-14 of word 3
 *                     data[2]: bit 29-22 of word 3
 *                     data[3]: bit 13-6 of word 4
 *                     ......
 *                     data[23]: bit 29-22 of word 10
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_almanac(UINT8 svid, UINT16* p_weekno, char data[24]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_assist_bitmap
 * DESCRIPTION
 *  Set assist(EPH/ALM) bitmap
 * PARAMETERS
 *  UINT8 AssistBitMap
 *     If you want EPH assist data, please set AssistBitMap to 0x08
 *     If you want ALM assist data, please set AssistBitMap to 0x01
 *     If you want EPH and ALM assist data, please set AssistBitMap to 0x09
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_assist_bitmap(UINT16 AssistBitMap);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_assist_bitmap
 * DESCRIPTION
 *  Get Re-aiding assist(EPH/ALM) bitmap
 * PARAMETERS
 *  uint8* pAssistBitMap
 *     Get current Re-aiding assist bitmap
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_assist_bitmap(UINT16 *pAssistBitMap);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_clear_ephemeris
 * DESCRIPTION
 *  clear the ephemeris of the specified PRN
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN(1~32)
 *****************************************************************************/
void
mtk_gps_clear_ephemeris(UINT8 svid);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_clear_almanac
 * DESCRIPTION
 *  clear the almanac of the specified PRN
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN(1~32)
 *****************************************************************************/
void
mtk_gps_clear_almanac(UINT8 svid);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sbas_msg_amount
 * DESCRIPTION
 *  Get the number of SBAS message blocks received in this epoch.
 *  Later on, please use mtk_gps_get_sbas_msg() to get the message
 *  content one by one.
 * PARAMETERS
 *  p_msg_amount  [OUT]   The number of SBAS message blocks received in this epoch
 * RETURNS
 *
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_sbas_msg_amount(UINT32* p_msg_amount);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sbas_msg
 * DESCRIPTION
 *  After calling mtk_gps_get_sbas_msg_amount(), we know the
 *  number of SBAS messages received in this epoch.
 *  mtk_gps_get_sbas_msg() gives a way to access each message
 *  data
 * PARAMETERS
 *  index        [IN]   which message you want to read
 *  pSVID        [OUT]  the PRN of the SBAS satellite
 *  pMsgType     [OUT]  the SBAS message type
 *  pParityError [OUT]  nonzero(parity error); zero(parity check pass)
 *  data         [OUT]  The 212-bit message data excluding the preamble,
 *                      message type field, and the parity check.
 *                      Regarding to endian, the data[0] is the beginning the
 *                      message, such as IODP field. The data[26] is the end of
 *                      message, and the bit 3..0 are padding zero.
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 * EXAMPLE
 *   // dump the SBAS message to UART output
 *   int  i, count;
 *   unsigned char PRN, MsgType, ParityError;
 *   char data[27];
 *
 *   mtk_gps_get_sbas_msg_amount(&count);
 *   for(i = 0; i < count; i++)
 *   {
 *      mtk_gps_get_sbas_msg(i, &PRN, &MsgType, &ParityError, data);
 *   }
 *****************************************************************************/
INT32
mtk_gps_get_sbas_msg(INT32 index, unsigned char* pSVID,
     unsigned char* pMsgType, unsigned char* pParityError, char data[27]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_cn0_test
 * DESCRIPTION
 *  Get Channel CNR and in CN0 test mode
 * PARAMETERS
 *  ChnSNR       [OUT]  Channel SNR
 *  PRN[0]: GPS PRN 1
 *  PRN[1]: GPS PRN 2
 *  PRN[2]: GLO PRN 8
 *  PRN[3]: BDS PRN 6
 *  PRN[4]: BDS PRN 7
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *  if you do not enter test mode first or Channel tracking not ready,
 *  return MTK_GPS_ERROR
 *****************************************************************************/

INT32
mtk_gps_get_cn0_test(INT32 SVID[5],INT32 SV_type[5],INT32 ChnSNR[5]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_chn_status
 * DESCRIPTION
 *  Get Channel SNR and Clock Drift Status in Channel Test Mode
 * PARAMETERS
 *  ChnSNR       [OUT]  Channel SNR
 *  ClkDrift     [OUT]  Clock Drift
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *  if you do not enter test mode first or Channel tracking not ready,
 *  return MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_gps_get_chn_test(float ChnSNR[16], float *ClkDrift);



/****************************************************************************
* FUNCTION
*mtk_gps_D2_Set_Enable
*DESCRIPTION
* Bediou D2 data Enable/Disable functions
*PARAMETERS
* UsedD2Corr =1, Enable D2 correction data.
* UsedD2Corr =0, Disable D2 correction data.
*RETURNS
* None
 *****************************************************************************/

void mtk_gps_D2_Set_Enable(unsigned char bUsedD2Corr);



/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_glonass_chn_status
 * DESCRIPTION
 *  Get Channel SNR and Clock Drift Status in Channel Test Mode
 * PARAMETERS
 *  ChnSNR       [OUT]  Channel SNR
 *  ClkDrift     [OUT]  Clock Drift
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *  if you do not enter test mode first or Channel tracking not ready,
 *  return MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_gps_get_glonass_chn_test(float ChnSNR[3]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_jammer_test
 * DESCRIPTION
 *  Obtain the CW jammer estimation result
 * PARAMETERS
 *  Freq         [OUT]  jammer frequency offset in KHz
 *  JNR          [OUT]  JNR of the associated jammer
 *  jammer_peaks [OUT]  The number(0~195) of jammer peaks if ready
 *                      0 means no jammer detected
 *                      Negative if not ready
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_jammer_test(INT32 *jammer_peaks, short Freq[195], UINT16 JNR[195]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_jammer_test
 * DESCRIPTION
 *  Enter or leave Jammer Test Mode
 * PARAMETERS
 *  action [IN]   1 = enter Jammer test(old); 2 = enter Jammer test(new); 0 = leave Jammer test
 *  SVid [IN] please assign any value between 1~32
 *  range [IN] no use in new jammer scan, any value is ok.
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
*****************************************************************************/
INT32
mtk_gps_set_jammer_test(INT32 action, UINT16 mode, UINT16 arg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_gnss_jammer_test
 * DESCRIPTION
 *   Enter or leave Jammer Test Mode
 * PARAMETERS
 *  action       [IN]   1 = enter Jammer test; 0 = leave Jammer test, not ready
 *  mode         [IN]   0 = GPS; 1 = GLONASS ;2 = Beidou
 *  arg          [IN]   unused
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_gnss_jammer_test(INT32 action, UINT16 mode, UINT16 arg);
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_phase_test
 * DESCRIPTION
 *  Obtain the last phase error calibration result
 * PARAMETERS
 *  result       [OUT]  0~64(success)
 *                      Negative(failure or not ready)
 *                      The return value is 64*{|I|/sqrt(I*I + Q*Q)}
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_phase_test(INT32 *result);


// *************************************************************************************
//  mtk_gps_get_sat_accurate_snr  :  Get the accurate SNR of all satellites
//
//     Note  :  SNR is an array with 32 float.
//  Example:
//     float SNR[32];
//     mtk_gps_get_sat_accurate_snr(SNR);
//     // Get SNR of SV 17
//     MTK_UART_OutputData("SV17: SNR = %lf", SNR[16]);
//
//  =====> SV17: SNR = 38.1

void mtk_gps_get_sat_accurate_snr(float SNR[32]);


// *************************************************************************************
//  mtk_gps_get_glon_sat_accurate_snr  :  Get the accurate SNR of all GLONASS satellites
//
//     Note  :  SNR is an array with [24] float.
//  Example:
//     float SNR[24];
//     mtk_gps_get_glon_sat_accurate_snr(SNR);
//     // Get SNR of SV 1
//     MTK_UART_OutputData("GLON,SV1: SNR = %lf", SNR[0]);
//
//  =====>GLON,SV1: SNR = 38.1

void mtk_gps_get_glon_sat_accurate_snr(float ASNR[24]);


// *************************************************************************************
//  mtk_gps_get_bd_sat_accurate_snr  :  Get the accurate SNR of all Beidou satellites
//
//     Note  :  SNR is an array with 30 float.
//  Example:
//     float SNR[30];
//     mtk_gps_get_bd_sat_accurate_snr(SNR);
//     // Get SNR of SV 17
//     MTK_UART_OutputData("BD,SV17: SNR = %lf", SNR[16]);
//
//  =====>BD,SV17: SNR = 38.1

void mtk_gps_get_bd_sat_accurate_snr(float SNR[30]);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_per_test
 * DESCRIPTION
 *   Enable PER test
 * PARAMETERS
 *  Threshold       [IN]   power of 2(1,2,4,8,...)
 *  SVid               [IN]   SVid for PER test that can be tracked in sky
 *  TargetCount   [IN]   Test time in 20ms unit
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_per_test(INT16 Threshold, UINT16 SVid, UINT16 TargetCount);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_chip_capability
 * DESCRIPTION
 *  send chip capability to mnld (ex: 6797: GPS + Beidou + Glonass)
 * PARAMETERS
 *  pradt       [out]
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
void
mtk_gps_get_chip_capability (UINT8* SV_TYPE);

/*****************************************************************************
 * FUNCTION
 *  mtk_gnss_get_gps_time_infor
 * DESCRIPTION
 *  send GPS TOW,WK and leap second when send PMTK761/763
 * PARAMETERS
 *  pradt       [IN]   pointer to the data structure of the A-GPS command or data
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gnss_get_gps_time_infor (unsigned int *p_dfTOW, short *p_i2WeekNo, double *p_UTC_Cor);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_agps_data
 * DESCRIPTION
 *  send A-GPS assistance data or command to GPS receiver
 * PARAMETERS
 *  pradt       [IN]   pointer to the data structure of the A-GPS command or data
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_agps_data(const MTK_GPS_AGPS_CMD_DATA_T *prAdt);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_agps_response
 * DESCRIPTION
 *  obtain detailed information of A-GPS reponse
 * PARAMETERS
 *  prRsp     [OUT]  pointer to A-GPS response data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_agps_response(MTK_GPS_AGPS_RESPONSE_T *prRsp);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_agps_lppe_3Dstatus_response
 * DESCRIPTION
 *  obtain detailed information of A-GPS reponse
 * PARAMETERS
 *  prRsp     [OUT]  pointer to A-GPS response data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_agps_lppe_3Dstatus_response(gnss_ha_3Dposition_struct *hight_3Dposition,gnss_ha_3Dvelocity_struct *hight_3Dvelocity);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_agps_lppe_meas_response
 * DESCRIPTION
 *  obtain detailed information of A-GPS reponse
 * PARAMETERS
 *  prRsp     [OUT]  pointer to A-GPS response data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_agps_lppe_meas_response(gnss_ha_Measure_struct *ha_Measure);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_agps_req_mod
 * DESCRIPTION
 *  obtain req module of A-GPS response
 * PARAMETERS
 *  ReqMod     [OUT]  pointer to request module
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_agps_req_mod(MTK_GPS_MODULE *pReqMod);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_time_change_notify
 * DESCRIPTION
 *  Notify MNL to handle RTC time change
 * PARAMETERS
 *  INT32RtcDiff      [IN]  System RTC time changed: old rtc time - new rtc time
 * RETURNS
 *
 *****************************************************************************/
void
mtk_gps_time_change_notify(INT32 INT32RtcDiff);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_rtc_offset
 * DESCRIPTION
 *  Notify MNL to handle RTC time change
 * PARAMETERS
 *  r8RtcOffset      [OUT]  System RTC time changed
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_rtc_offset(double *r8RtcOffset);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_time_update_notify
 * DESCRIPTION
 *  Notify MNL to handle RTC time change
 * PARAMETERS
 *  fgSyncGpsTime      [IN]  System time sync to GPS time
 *  i4RtcDiff      [IN]  Difference of system RTC time changed
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_time_update_notify(UINT8 fgSyncGpsTime, INT32 i4RtcDiff);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_debug_config
 * DESCRIPTION
 *  config the debug log type and level
 * PARAMETERS
 *  DbgType         [IN]  Debug message category
 *  DbgLevel        [IN]  Debug message output level
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_sys_debug_config(MTK_GPS_DBG_TYPE DbgType, MTK_GPS_DBG_LEVEL DbgLevel);


/* =================== Porting layer functions =================== */
/*            The function body needs to be implemented            */
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_gps_mnl_callback
 * DESCRIPTION
 *  MNL CallBack function
 * PARAMETERS
 *  mtk_gps_notification_type
 * RETURNS
 *   success(MTK_GPS_SUCCESS)
 *****************************************************************************/

INT32
mtk_gps_sys_gps_mnl_callback(MTK_GPS_NOTIFICATION_TYPE msg);

/*****************************************************************************
 * FUNCTION
*  mtk_gps_sys_time_tick_get
* DESCRIPTION
*  get the current system tick of target platform(msec)
* PARAMETERS
*  none
* RETURNS
*  system time tick
*****************************************************************************/
UINT32
mtk_gps_sys_time_tick_get(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_time_tick_get_max
 * DESCRIPTION
 *  get the maximum system tick of target platform(msec)
 * PARAMETERS
 *  none
 * RETURNS
 *  system time tick
 *****************************************************************************/
UINT32
mtk_gps_sys_time_tick_get_max(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_time_read
 * DESCRIPTION
 *  Read system time
 * PARAMETERS
 *  utctime     [IN/OUT] get the host system time
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *  failed(MTK_GPS_ERROR)
 *  system time changed since last call(MTK_GPS_ERROR_TIME_CHANGED)
 *****************************************************************************/
INT32
mtk_gps_sys_time_read(MTK_GPS_TIME* utctime);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_task_sleep
 * DESCRIPTION
 *  Task sleep function
 * PARAMETERS
 *  milliseconds [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_task_sleep(UINT32 milliseconds);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_open
 * DESCRIPTION
 *  Open a non-volatile file
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_storage_open(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_close
 * DESCRIPTION
 *  Close a non-volatile file
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_storage_close(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_delete
 * DESCRIPTION
 *  Delete a non-volatile file
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_storage_delete(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_read
 * DESCRIPTION
 *  Read a non-volatite file
 *  - blocking read until reaching 'length' or EOF
 * PARAMETERS
 *  buffer      [OUT]
 *  offset      [IN]
 *  length      [IN]
 *  p_nRead     [OUT]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_storage_read(void* buffer, UINT32 offset, UINT32 length,
                      UINT32* p_nRead);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_write
 * DESCRIPTION
 *  Write a non-volatite file
 * PARAMETERS
 *  buffer      [IN]
 *  offset      [IN]
 *  length      [IN]
 *  p_nWritten  [OUT]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_storage_write(const void* buffer, UINT32 offset, UINT32 length,
                       UINT32* p_nWritten);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_mem_alloc
 * DESCRIPTION
 *  Allocate a block of memory
 * PARAMETERS
 *  size        [IN]   the length of the whole memory to be allocated
 * RETURNS
 *  On success, return the pointer to the allocated memory
 *  NULL(0) if failed
 *****************************************************************************/
void*
mtk_gps_sys_mem_alloc(UINT32 size);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_mem_free
 * DESCRIPTION
 *  Release unused memory
 * PARAMETERS
 *  pmem         [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_mem_free(void* pmem);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_delete
* DESCRIPTION
*  event delete for Android
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_delete(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_create
* DESCRIPTION
*  event create for Android
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_create(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_set
* DESCRIPTION
*  event set for Android
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_set(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_wait
* DESCRIPTION
*  event wait for android
*
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_wait(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_init
 * DESCRIPTION
 *  Initiialize UART
 * PARAMETERS
 *  portname      [IN]
 *  baudrate      [IN]
 *  txbufsize      [IN]
 *  rxbufsize      [IN]
 * RETURNS
 *  Result of Handler
 *****************************************************************************/
INT32
mtk_gps_sys_uart_init(char* portname, UINT32 baudrate, UINT32 txbufsize,
                      UINT32 rxbufsize);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_read
 * DESCRIPTION
 *  Initiialize UART
 * PARAMETERS
 *  UARTHandle      [IN]
 *  buffer      [IN]
 *  bufsize      [IN]
 *  length      [IN]
 * RETURNS
 *  Result of Handler
 *****************************************************************************/
INT32
mtk_gps_sys_uart_read(INT32 UARTHandle, char* buffer, UINT32 bufsize,
                   INT32* length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_write
 * DESCRIPTION
 *  Initiialize UART
 * PARAMETERS
 *  UARTHandle      [IN]
 *  buffer      [IN]
 *  bufsize      [IN]
 *  length      [IN]
 * RETURNS
 *  Result of Handler
 *****************************************************************************/
INT32 mtk_gps_sys_uart_write(INT32 UARTHandle, const char* buffer, UINT32 bufsize,
       INT32* wrotenlength);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_uninit
 * DESCRIPTION
 *  Initiialize UART
 * PARAMETERS
 *  UARTHandle      [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_uart_uninit(INT32 UARTHandle);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_if_set_spd
 * DESCRIPTION
 *  Set baud rate at host side from GPS lib
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  baudrate         [IN] UART baudrate
 *  hw_fc            [IN] UART hardware flow control
 *                        0: without hardware flow contorl(defualt)
 *                        1: with hardware flow contorl
 * RETURNS
 *  success(0)
 *****************************************************************************/
INT32
mtk_gps_sys_if_set_spd(UINT32 baudrate, UINT8 hw_fc);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_data_output
 * DESCRIPTION
 *  Transmit data to the GPS chip
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  msg         [IN]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_data_output(char* buffer, UINT32 length);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_nmea_output
* DESCRIPTION
*  Transmit NMEA data out to task
*  (The function body needs to be implemented)
* PARAMETERS
*  buffer         [IN] data pointer
*  length         [IN] size of data
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_nmea_output(char* buffer, UINT32 length);


/*****************************************************************************
* FUNCTION
*  mtk_sys_nmea_output_to_app
* DESCRIPTION
*  Transmit NMEA data out to APP layer
*  (The function body needs to be implemented)
* PARAMETERS
*  buffer         [IN] data pointer
*  length         [IN] size of data
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_nmea_output_to_app(const char * buffer, UINT32 length);

INT32
mtk_gps_sys_nmea_output_to_app_print(const char *fmt, ...);

#if 1
/*****************************************************************************
* FUNCTION
*  mtk_sys_nmea_output_to_mnld
* DESCRIPTION
*  Transmit NMEA data out to MNLD
*  (The function body needs to be implemented)
* PARAMETERS
*  buffer         [IN] data pointer
*  length         [IN] size of data
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_nmea_output_to_mnld(const char * buffer, UINT32 length);
#endif

/*****************************************************************************
* FUNCTION
*  mnl_sys_alps_gps_dbg2file_mnld
* DESCRIPTION
*  Transfer GPS debug log to MNLD to write to file
*  (The function body needs to be implemented)
* PARAMETERS
*  buffer         [IN] data pointer
*  length         [IN] size of data
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mnl_sys_alps_gps_dbg2file_mnld(const char * buffer, UINT32 length);

//  Otehr platform

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_start_result_handler
 * DESCRIPTION
 *  Handler routine for the result of restart command
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  result         [IN]  the result of restart
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_start_result_handler(MTK_GPS_START_RESULT result);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_spi_poll
 * DESCRIPTION
 *  Polling data input routine for SPI during dsp boot up stage.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_spi_poll(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_set_spi_mode
 * DESCRIPTION
 *  Set SPI interrupt/polling and support burst or not.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  enable_int         [IN]  1 for enter interrupt mode , 0 for entering polling mode
 *  enable_burst       [IN]  1 for enable burst transfer, 0 for disable burst transfer
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_set_spi_mode(UINT8 enable_int, UINT8 enable_burst);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_dsp_boot_begin_handler
 * DESCRIPTION
 *  Handler routine for porting layer implementation right before GPS DSP boot up
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  none
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_dsp_boot_begin_handler(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_dsp_boot_end_handler
 * DESCRIPTION
 *  Handler routine for porting layer implementation right after GPS DSP boot up
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  none
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_dsp_boot_end_handler(void);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas
 * DESCRIPTION
 * PARAMETERS
 *  pFrameTime [OUT] frame time of the issued frame pulse(seconds)
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/

INT32 mtk_gps_sys_frame_sync_meas(double *pdfFrameTime);

INT32 mtk_gps_sys_frame_sync_enable_sleep_mode(unsigned char mode);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas_resp
 * DESCRIPTION
 *  accept a frame sync measurement response
 * PARAMETERS
 *  eResult     [IN] success to issue a frame sync meas request
 *  dfFrameTime [IN] frame time of the issued frame pulse(seconds)
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_frame_sync_meas_resp(MTK_GPS_FS_RESULT eResult, double dfFrameTime);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas_req_by_network
 * DESCRIPTION
 *  issue a frame sync measurement request
 * PARAMETERS
 *  none
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *  fail(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_frame_sync_meas_req_by_network(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas_req
 * DESCRIPTION
 *  issue a frame sync measurement request
 * PARAMETERS
 *  mode       [out] frame sync request indication for aiding or maintain
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *  fail(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_frame_sync_meas_req(MTK_GPS_FS_WORK_MODE mode);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_initialize_mutex
 * DESCRIPTION
 *  Inialize mutex array
 * PARAMETERS
 *  void
  * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_initialize_mutex(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_create_mutex
 * DESCRIPTION
 *  Create a mutex object
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
  * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_create_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_take_mutex
 * DESCRIPTION
 *  Request ownership of a mutex and if it's not available now, then block the thread execution
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
 * RETURNS
 *  void
 *****************************************************************************/

void
mtk_gps_sys_take_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_give_mutex
 * DESCRIPTION
 *  Release a mutex ownership
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
 * RETURNS
 *  void
 *****************************************************************************/

void
mtk_gps_sys_give_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_destroy_mutex
 * DESCRIPTION
 *  Destroy a mutex object
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_destroy_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_pmtk_cmd_cb
 * DESCRIPTION
 *  Notify porting layer that MNL has received one PMTK command.
 * PARAMETERS
 *  UINT16CmdNum        [IN]  The received PMTK command number.
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_pmtk_cmd_cb(UINT16 UINT16CmdNum);

/*****************************************************************************
 * FUNCTION
 *  mtk_sys_spi_poll
 * DESCRIPTION
 *  Polling data input routine for SPI during dsp boot up stage.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 mtk_gps_sys_spi_poll(void);

 /*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_poll
 * DESCRIPTION
 *  GPS RX polling function
 * PARAMETERS
 *  void
 * RETURNS
 *  success(0)
 *****************************************************************************/
INT32 mtk_gps_sys_uart_poll(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_rtc_info
 * DESCRIPTION
 *
 * PARAMETERS
 *  dfrtcD        [OUT]  RTC clock drift(unit : ppm).
 *  dfage         [OUT]  RTC drift age : current gps time - gps time of latest rtc drift calculated.(unit : sec)
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_rtc_info(double *dfrtcD, double *dfage);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_delete_nv_data
 * DESCRIPTION
 *
 * PARAMETERS
 *  u4Bitmap    [INPUT]
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32 mtk_gps_delete_nv_data(UINT32 assist_data_bit_map);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_tsx_xvt
 * DESCRIPTION
 *
 * PARAMETERS
 *  u4Bitmap    [INPUT]
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_tsx_xvt(UINT8 enable);

/*****************************************************************************
 * mtk_gps_set_wifi_location_aiding :
 *   set Wifi location information to MNL
 * Parameters :
 *  latitude :Wifi latitude      unit: [degree]
 *  longitude:Wifi longitude     unit: [degree]
 *  posvar   :position accuracy  unit: [m]
 * Return Value: 0: fail; 1: pass
 *****************************************************************************/
INT32 mtk_gps_set_wifi_location_aiding(MTK_GPS_REF_LOCATION *RefLocation,
      double latitude, double longitude, double accuracy);

/*****************************************************************************
 * mtk_gps_inject_ntp_time :
 *   inject ntp time into MNL
 * Parameters :
 *  time : ntp time      unit: [degree]
 *  timeRef: the system time tick when sync NTP time to network    unit: mini-second
 *  Uncertainty: ntp time uncertainty error: [mini]
 * success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32 mtk_gps_inject_ntp_time(MTK_GPS_NTP_T* ntp);

/*****************************************************************************
 * mtk_gps_inject_nlp_location :
 *   inject nlp location into MNL
 * Parameters :
 *  latitude : nlp latitude
 *  longitude:  nlp longitude
 *  accuracy: nlp location uncertainty : [m]
 * success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32 mtk_gps_inject_nlp_location(MTK_GPS_NLP_T* nlp);


/*****************************************************************************
 * mtk_gps_measurement :
 *   get gps measurement
 * Parameters :
 * flag:                  A set of flags indicating the validity of the fields in this data structure
 * PRN:                 Pseudo-random number in the range of [1, 32]
 * TimeOffsetInNs: Time offset at which the measurement was taken in nanoseconds.
                            The reference receiver's time is specified by GpsData::clock::time_ns and should be
                            interpreted in the same way as indicated by GpsClock::type.
                            The sign of time_offset_ns is given by the following equation:
                            measurement time = GpsClock::time_ns + time_offset_ns
                            It provides an individual time-stamp for the measurement, and allows sub-nanosecond accuracy.
 * state:                Per satellite sync state. It represents the current sync state for the associated satellite.
                             Based on the sync state, the 'received GPS tow' field should be interpreted accordingly.
 * ReGpsTowInNs:  Received GPS Time-of-Week at the measurement time, in nanoseconds.
                            The value is relative to the beginning of the current GPS week.
                            Given the sync state of GPS receiver, per each satellite, valid range for this field can be:
                                 Searching           : [ 0       ]   : GPS_MEASUREMENT_STATE_UNKNOWN
                                 Ranging code lock   : [ 0   1ms ]   : GPS_MEASUREMENT_STATE_CODE_LOCK is set
                                 Bit sync            : [ 0  20ms ]   : GPS_MEASUREMENT_STATE_BIT_SYNC is set
                                 Subframe sync       : [ 0   6ms ]   : GPS_MEASUREMENT_STATE_SUBFRAME_SYNC is set
                                 TOW decoded         : [ 0 1week ]   : GPS_MEASUREMENT_STATE_TOW_DECODED is set
 * ReGpsTowUnInNs: 1-Sigma uncertainty of the Received GPS Time-of-Week in nanoseconds
 * Cn0InDbHz:                Carrier-to-noise density in dB-Hz, in the range [0, 63].
                                     It contains the measured C/N0 value for the signal at the antenna input.
 * PRRateInMeterPreSec: Pseudorange rate at the timestamp in m/s.
                                     The value also includes the effects of the receiver clock frequency and satellite clock
                                     frequency errors.
                                     The value includes the 'pseudorange rate uncertainty' in it.
                                     A positive value indicates that the pseudorange is getting larger.
 * PRRateUnInMeterPreSec: 1-Sigma uncertainty of the pseudurange rate in m/s.
                                         The uncertainty is represented as an absolute(single sided) value.
 * AcDRState10:         Accumulated delta range's state. It indicates whether ADR is reset or there is a cycle slip
                               (indicating loss of lock).
 * AcDRInMeters:       Accumulated delta range since the last channel reset in meters.
 * AcDRUnInMeters:   1-Sigma uncertainty of the accumulated delta range in meters.
 * PRInMeters:           Best derived Pseudorange by the chip-set, in meters.
                                The value contains the 'pseudorange uncertainty' in it.
 * PRUnInMeters:       1-Sigma uncertainty of the pseudorange in meters.
                               The value contains the 'pseudorange' and 'clock' uncertainty in it.
                               The uncertainty is represented as an absolute(single sided) value.
 * CPInChips:           A fraction of the current C/A code cycle, in the range [0.0, 1023.0]
                              This value contains the time(in Chip units) since the last C/A code cycle(GPS Msec epoch).
                              The reference frequency is given by the field 'carrier_frequency_hz'.
                              The value contains the 'code-phase uncertainty' in it.
 * CPUnInChips:       1-Sigma uncertainty of the code-phase, in a fraction of chips.
                              The uncertainty is represented as an absolute(single sided) value.
 * CFInhZ:               Carrier frequency at which codes and messages are modulated, it can be L1 or L2.
                              If the field is not set, the carrier frequency is assumed to be L1.
 * CarrierCycle:       The number of full carrier cycles between the satellite and the receiver.
                              The reference frequency is given by the field 'carrier_frequency_hz'.
 * CarrierPhase:      The RF phase detected by the receiver, in the range [0.0, 1.0].
                              This is usually the fractional part of the complete carrier phase measurement.
                              The reference frequency is given by the field 'carrier_frequency_hz'.
                              The value contains the 'carrier-phase uncertainty' in it.
 * CarrierPhaseUn:   1-Sigma uncertainty of the carrier-phase.
 * LossOfLock:               An enumeration that indicates the 'loss of lock' state of the event.
 * BitNumber:                The number of GPS bits transmitted since Sat-Sun midnight(GPS week).
 * TimeFromLastBitInMs: The elapsed time since the last received bit in milliseconds, in the range [0, 20]
 * DopperShiftInHz:        Doppler shift in Hz.
                                    A positive value indicates that the SV is moving toward the receiver.
                                    The reference frequency is given by the field 'carrier_frequency_hz'.
                                    The value contains the 'doppler shift uncertainty' in it.
 * DopperShiftUnInHz:    1-Sigma uncertainty of the doppler shift in Hz.
 * MultipathIndicater:     An enumeration that indicates the 'multipath' state of the event.
 * SnrInDb:                  Signal-to-noise ratio in dB.
 * ElInDeg:                   Elevation in degrees, the valid range is [-90, 90].
                                   The value contains the 'elevation uncertainty' in it.
 * ElUnInDeg:               1-Sigma uncertainty of the elevation in degrees, the valid range is [0, 90].
                                   The uncertainty is represented as the absolute(single sided) value.
 * AzInDeg:                  Azimuth in degrees, in the range [0, 360).
                                  The value contains the 'azimuth uncertainty' in it.
 * AzUnInDeg:             1-Sigma uncertainty of the azimuth in degrees, the valid range is [0, 180].
                                 The uncertainty is represented as an absolute(single sided) value.
 * UsedInFix:              Whether the GPS represented by the measurement was used for computing the most recent fix.
 *****************************************************************************/
void
mtk_gps_get_measurement(MTK_GPS_MEASUREMENT GpsMeasurement[32], INT8 Ch_Proc_Ord_PRN[32]);

/*****************************************************************************
 * mtk_gps_clock :
 *   get clock parameter
 * Parameters :
 * flag:            A set of flags indicating the validity of the fields in this data structure.
 * leapsecond: Leap second data.
                      The sign of the value is defined by the following equation:
                      utc_time_ns = time_ns +(full_bias_ns + bias_ns) - leap_second * 1,000,000,000
 * type:           Indicates the type of time reported by the 'time_ns' field.
 * TimeInNs:    The GPS receiver internal clock value. This can be either the local hardware clock value
                     (GPS_CLOCK_TYPE_LOCAL_HW_TIME), or the current GPS time derived inside GPS receiver
                     (GPS_CLOCK_TYPE_GPS_TIME). The field 'type' defines the time reported.

                      For local hardware clock, this value is expected to be monotonically increasing during
                      the reporting session. The real GPS time can be derived by compensating the 'full bias'
                     (when it is available) from this value.

                      For GPS time, this value is expected to be the best estimation of current GPS time that GPS
                      receiver can achieve. Set the 'time uncertainty' appropriately when GPS time is specified.

                      Sub-nanosecond accuracy can be provided by means of the 'bias' field.
                      The value contains the 'time uncertainty' in it.
 * TimeUncertaintyInNs: 1-Sigma uncertainty associated with the clock's time in nanoseconds.
                                    The uncertainty is represented as an absolute(single sided) value.
                                    This value should be set if GPS_CLOCK_TYPE_GPS_TIME is set.
 * FullBiasInNs:              The difference between hardware clock('time' field) inside GPS receiver and the true GPS
                                    time since 0000Z, January 6, 1980, in nanoseconds.
                                    This value is used if and only if GPS_CLOCK_TYPE_LOCAL_HW_TIME is set, and GPS receiver
                                    has solved the clock for GPS time.
                                    The caller is responsible for using the 'bias uncertainty' field for quality check.

                                    The sign of the value is defined by the following equation:
                                    true time(GPS time) = time_ns +(full_bias_ns + bias_ns)

                                    This value contains the 'bias uncertainty' in it.
 * BiasInNs:                  Sub-nanosecond bias.
                                    The value contains the 'bias uncertainty' in it.
 * BiasUncertaintyInNs:   1-Sigma uncertainty associated with the clock's bias in nanoseconds.
                                    The uncertainty is represented as an absolute(single sided) value.
 * DriftInNsPerSec:        The clock's drift in nanoseconds(per second).
                                    A positive value means that the frequency is higher than the nominal frequency.

                                    The value contains the 'drift uncertainty' in it.
 * DriftUncertaintyInNsPerSec:  1-Sigma uncertainty associated with the clock's drift in nanoseconds(per second).
                                              The uncertainty is represented as an absolute(single sided) value.
 *
 * Return Value: 0: fail; 1: pass
 *****************************************************************************/
INT32
mtk_gps_get_clock(MTK_GPS_CLOCK *GpsClock);

/*****************************************************************************
 * mtk_gps_navigation_event :
 *   get navigation event
 * Parameters :
 * type:                The type of message contained in the structure.
 * prn:                  Pseudo-random number in the range of [1, 32]
 * meaasgeID:      Message identifier.
                           It provides an index so the complete Navigation Message can be assembled. i.e. fo L1 C/A
                           subframe 4 and 5, this value corresponds to the 'frame id' of the navigation message.
                           Subframe 1, 2, 3 does not contain a 'frame id' and this value can be set to -1.
 * submeaasgeID: Sub-message identifier.
                            If required by the message 'type', this value contains a sub-index within the current
                            message(or frame) that is being transmitted.
                            i.e. for L1 C/A the submessage id corresponds to the sub-frame id of the navigation message.
 * data[]:              The data of the reported GPS message.
                            The bytes(or words) specified using big endian format(MSB first).

                            For L1 C/A, each subframe contains 10 30-bit GPS words. Each GPS word(30 bits) should be
                            fitted into the last 30 bits in a 4-byte word(skip B31 and B32), with MSB first.
 *
 * Return Value: 0: fail; 1: pass
 *****************************************************************************/
INT32
mtk_gps_get_navigation_event(MTK_GPS_NAVIGATION_EVENT *NavEvent, UINT8 SVID);


void
mtk_gnss_get_measurement(Gnssmeasurement GpQzMeasurement[40], Gnssmeasurement GloMeasurement[24],
Gnssmeasurement BDMeasurement[37], Gnssmeasurement GalMeasurement[36],Gnssmeasurement SBASMeasurement[42],
INT8 GpQz_Ch_Proc_Ord_PRN[40],INT8 Glo_Ch_Proc_Ord_PRN[24],INT8 BD_Ch_Proc_Ord_PRN[37],
INT8 Gal_Ch_Proc_Ord_PRN[36],INT8 SBAS_Ch_Proc_Ord_PRN[42]);

INT32
mtk_gnss_get_clock(Gnssclock *GnssClock);


INT32
mtk_gnss_get_navigation_event (GnssNavigationmessage *NavEvent, UINT8 SVID, UINT8 SV_type);

// *****************************************************************************
//  mtk_gps_set_mpe_event :(MNL to MPE)
//  set mpe event
//  Parameters :
// *****************************************************************************
UINT16
mtk_gps_set_mpe_info(UINT8 *msg);

// *****************************************************************************
//  mtk_gps_mnl_mpe_callback_reg :(MNLD to MNL)
//  MNLD register callback function in libmnl
//  Parameters :
//   name    [IN]  callback function address in MNLD
//  Return
//    success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
// *****************************************************************************
int
mtk_gps_mnl_mpe_callback_reg(MPECallBack *name);

// *****************************************************************************
//  mtk_gps_mnl_mpe_run_callback :(MNL to MNLD)
//  Run MNLD function registered in libmnl
//  Parameters :
//  Return
//    success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
// *****************************************************************************
int
mtk_gps_mnl_mpe_run_callback(void);

/*****************************************************************************
 * mtk_gps_get_sensor_info :
 *   get sensor LLH,head,head acc,vout information(MPE to MNL)
 * Parameters :
 *****************************************************************************/
void
mtk_gps_get_sensor_info(INT8 *msg, int len);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_debug_type
 * DESCRIPTION
 *  Set the GPS debug type in running time
 * PARAMETERS
 *  debug_type         [IN]   the debug type needs to be changed
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_debug_type(const UINT8 debug_type);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_debug_type
 * DESCRIPTION
 *  Get the GPS debug type in running time
 * PARAMETERS
 *  debug_type         [out]   current debug type
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_debug_type(UINT8* debug_type);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_debug_file
 * DESCRIPTION
 *  Set the GPS debug file name(include the path name) in running time
 * PARAMETERS
 *  file_name         [IN]   the debug file name needs to be changed
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_debug_file(char* file_name);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_rtc_info
 * DESCRIPTION
 *
 * PARAMETERS
 *  PRN           [OUT]  PRN Number  (1~32 are valid).
 *  ParityError   [OUT]  Parity Error flag (ex :3FF = 10 words are wrong)
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_word_parity(UINT32 *TTick,UINT8 PRN[26], UINT16 ParityError[26]);

/*****************************************************************************
 * FUNCTION
 *  mtk_get_jamming_indicator
 * DESCRIPTION
 *
 * PARAMETERS
 *  JammingIndicator           [OUT]  Jamming Indicator
 *                                    1 : No jammer, healthy status
 *                                    2 : Warning status
 *                                    3 : Critical status
 *****************************************************************************/
void mtk_get_jamming_indicator(unsigned char *JammingIndicator);

/*****************************************************************************
 * FUNCTION
 *  mtk_set_gnss_low_power_mode
 * DESCRIPTION
 *  Set GNSS Low Power mode
 * PARAMETERS
 *  GNSSLowPowerMode           [IN]  Enable GNSS Low Power or not
 *                                    0 : Disable GNSS Low Power
 *                                    1 : Enable GNSS Low Power
 *****************************************************************************/
void mtk_set_gnss_low_power_mode(unsigned char GNSSLowPowerMode);

/*****************************************************************************
 * FUNCTION
 *  mtk_get_gnss_low_power_mode
 * DESCRIPTION
 *  Get GNSS Low Power mode
 * PARAMETERS
 *  GNSSLowPowerMode           [OUT]  GNSS Low Power enabled or not
 *                                    0 : GNSS Low Power disabled
 *                                    1 : GNSS Low Power enabled
 *****************************************************************************/
void mtk_get_gnss_low_power_mode(unsigned char *GNSSLowPowerMode);

/*****************************************************************************
 * FUNCTION
 *  mtk_set_ext_lna_control_test
 * DESCRIPTION
 *  Test the external LNA. The external LNA would be turn off when calling this function.
 *****************************************************************************/
void mtk_set_ext_lna_control_test(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_position_accuracy
 * DESCRIPTION
 *  get position
 * PARAMETERS
 *  double *Lat,double *Lon,int *accuracy)
 *     Get return result
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure(MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_position_accuracy(double *Lat, double *Lon, int *accuracy);

void
mtk_gps_mnl_get_sensor_info(UINT8 *msg, int len);

void
mtk_gps_mnl_set_sensor_info(UINT8 *msg, int len);

int
mtk_gps_mnl_trigger_mpe(void);

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
INT32 mtk_agps_agent_qepo_file_update();

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_agent_mtknav_file_update
 * DESCRIPTION
 *    Update the MTKNAV file after the new MTKNAV file download
 * PARAMETERS
 *      None
 * RETURNS
 *  success: MTK_GPS_SUCCESS  fail: MTK_GPS_ERROR
 *****************************************************************************/
INT32 mtk_agps_agent_mtknav_file_update();

#if 1  // mnl offload interface start
/*****************************************************************************
 * \bref    for mnl offload
 *          enable  offload:
 *              1. mtk_gps_ofl_set_cfg(MNL_OFL_CFG_ENALBE_OFFLOAD)
 *              2. mnl_sys_alps_gps_run
 *          disable offload, only one step:
 *              1. mnl_sys_alps_gps_run
 */
// ============================================================================
//  Offload: MNLD => libmnl.so
// ============================================================================
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_set_option
 * DESCRIPTION
 *  Configure mnl offload parameters.
 *  Now whether to enable mnl offload is controlled by this function
 * PARAMETERS
 *  user_bitmask     [IN]  bitmask like MNL_OFL_CFG_XXX
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
#define MNL_OFL_OPTION_ENALBE_OFFLOAD   0x1

INT32 mtk_gps_ofl_set_option(UINT32 cfg_bitmask);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_set_user
 * DESCRIPTION
 *  Set the owners who use MNL, then how to init CONNSYS-MNL can be initilised.
 * PARAMETERS
 *  user_bitmask     [IN]  bitmask which stands for different users like GPS_USR_XXX.
 *                      The bitmask must contaion all the users of GPS so mnld msut
 *                      must keep a copy of this bitmask.
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
#define GPS_USER_UNKNOWN        0x0
#define GPS_USER_APP            0x1
#define GPS_USER_AGPS           0x2
#define GPS_USER_META           0x4   //  meta or factory mode
#define GPS_USER_RESTART        0x8
#define GPS_USER_FLP            0x10
#define GPS_USER_OFL_TEST       0x20  //  for offload test
#define GPS_USER_AT_CMD         0x40  //  for offload test
#define GPS_USER_APM            0x80
#define GPS_USER_GEOFENCE       0x100
#define GPS_USER_RAW_MEAS       0x200  //RAW measurement

INT32 mtk_gps_ofl_set_user(UINT32 user_bitmask);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_rst_stpgps_begin_ntf
 * DESCRIPTION
 *  Notify libmnl.so that MNLD is to reopen /dev/stpgps.
 * PARAMETERS
 *  void
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 mtk_gps_ofl_rst_stpgps_begin_ntf();

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_rst_stpgps_begin_ntf
 * DESCRIPTION
 *  Notify libmnl.so that MNLD has reopened /dev/stpgps and tell libmnl.so the new fd.
 * PARAMETERS
 *  dsp_fd      [IN] the new fd of /dev/stpgps after reopened.
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 mtk_gps_ofl_rst_stpgps_end_ntf(int dsp_fd);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_send_flp_data
 * DESCRIPTION
 *  Reroute data from HOST FLP to CONNSYS FLP.
 * PARAMETERS
 *  buf     [IN] buffer to send
 *  len     [IN] length of the data
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 mtk_gps_ofl_send_flp_data(UINT8 *buf, UINT32 len);

#if 0
/* These 2 API can be replaced by:
mtk_gps_ofl_set_user(gps_user |  GPS_USER_FLP);
mtk_gps_ofl_set_user(gps_user & ~GPS_USER_FLP);
*/
INT32 mtk_gps_ofl_flp_init();
INT32 mtk_gps_ofl_flp_deinit();
#endif

//  ============================================================================
//  Offload: libmnl.so => MNLD
//  ============================================================================
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_sys_rst_stpgps_req
 * DESCRIPTION
 *  Request MNLD to reopen /dev/stpgps.
 * PARAMETERS
 *  void
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/

INT32 sys_gps_mnl_data2mnld_callback(const char *data, unsigned int length);
/*****************************************************************************
 * FUNCTION
 *  sys_gps_mnl_data2mnld_callback
 * DESCRIPTION
 *  Submit data from mnl to mnld.
 * PARAMETERS
 *  buf     [IN] buffer to send
 *  len     [IN] length of the data
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/

INT32 mtk_gps_ofl_sys_rst_stpgps_req(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_sys_submit_flp_data
 * DESCRIPTION
 *  Submit data from CONNSYS FLP to HOST FLP.
 * PARAMETERS
 *  buf     [IN] buffer to send
 *  len     [IN] length of the data
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 mtk_gps_ofl_sys_submit_flp_data(UINT8 *buf, UINT32 len);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_ofl_sys_mnl_offload_callback
 * DESCRIPTION
 *  Callback for libmnl.so to notify MNLD for something.
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 mtk_gps_ofl_sys_mnl_offload_callback(
    MTK_GPS_OFL_CB_TYPE type, UINT16 length, UINT8 *data);
#endif  //  mnl offload interface end


#if 1  // for FLP AP mode function only
/*****************************************************************************
 * FUNCTION
 *   mtk_gps_flp_get_location
 *
 * DESCRIPTION
 *   Porting layer or MNL daemon can call this API to fill location data to
 *     "buf", then it can be reported to FLP daemon.
 *
 *   The "buf" will be filled with "MtkGpsLocation". Due to MNLD use
 *     "buf" rather then "MtkGpsLocation" as parameter, any change on
 *     "MtkGpsLocation" structure only need to be synchronized between
 *     FLP daemon and libmnl, then maintian effort can be eased.
 *
 *   Please note that:
 *   1. It's better if "buf" is 4-byte alligned. On some platform, non-alligned
 *     address for structure filling may cause issue.
 *   2. The "buf_len" should be greater than or equal to
 *     sizeof("MtkGpsLocation"), otherwise function will return
 *     "MTK_GPS_ERROR" and no data will be filled into "buf".
 *   3. If function return MTK_GPS_SUCCESS, "buf" will be filled and the filled
 *     length can be get from "*p_get_len".
 *   4. If function return MTK_GPS_ERROR, "*p_get_len" indicates the wanted "buf_len"
 *
 * PARAMETERS
 *  buf         [OUT] buffer to be filled
 *  buf_len     [IN]  length of the buffer
 *  p_get_len   [OUT] the actual length of data be filled to buffer.
 *                    it should be
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 mtk_gps_flp_get_location(void *buf, UINT32 buf_len, UINT32 *p_get_len);
#endif // for FLP AP mode function only


//*************************************************************************************
// mtk_gps_get_gnss_operation_mode  :  Set GNSS operation mode
//
//    Note  :  fgGLONStatus = [0] Disable GLONASS [1] Enable GLONASS
//             fgGALIStatus = [0] Disable GALIELO [1] Enable GALIELO
//             fgBEDOStatus = [0] Disable BEIDOU  [1] Enable BEIDOU
// Example:
// unsigned char fgGLONStatus = 0;
// unsigned char fgGALIStatus = 0;
// unsigned char fgBEDOStatus = 0;
// mtk_gps_get_gnss_operation_mode(&fgGLONStatus,&fgGALIStatus,&fgBEDOStatus);

void mtk_gps_get_gnss_operation_mode(unsigned char *fgGLONStatus, unsigned char *fgGALIStatus, unsigned char *fgBEDOStatus, unsigned char *fgGPSStatus);

//*************************************************************************************
// mtk_gps_set_gnss_operation_mode  :  Set GNSS operation mode
//
//    Note  :  fgGLONStatus = [0] Disable GLONASS [1] Enable GLONASS
//             fgGALIStatus = [0] Disable GALIELO [1] Enable GALIELO
//             fgBEDOStatus = [0] Disable BEIDOU  [1] Enable BEIDOU
// Notice :
//    GLONASS + GALILEO and Beidou are mutal exclusion.
//    If you enable GLONASS, GALILEO and Beidou at the same time, Beidou will be disabled automatically.
//
// Notice : The GALILEO mode switch is not support in this release.
//
// Example:
// mtk_gps_set_gnss_operation_mode(1,0,0) : Enable GLONASS
// mtk_gps_set_gnss_operation_mode(0,0,0) : Disable GLOANSS, GALIELO , BEIDOU
// mtk_gps_set_gnss_operation_mode(0,0,1) : Enable BEIDOU
// mtk_gps_set_gnss_operation_mode(1,1,1) : Enable GLONASS. Beidou will be disabled automatically.

void mtk_gps_set_gnss_operation_mode(unsigned char fgGLONStatus, unsigned char fgGALIStatus, unsigned char fgBEDOStatus);

/*****************************************************************************
 * FUNCTION
 *   mtk_gps_log_line_enc_inplace
 *
 * DESCRIPTION
 *   Byte to byte encrypt the log buffer. Stop and return after
 *   non-printable char (ASCII0~31,128~255) is found or the length is reached.
 *
 * PARAMETERS
 *  buffer      [IN/OUT]    log buffer whith can be read or write
 *  length      [IN]        length of the data in buffer
 * RETURNS
 *  The length actual replaced with encrypted char
 *****************************************************************************/
UINT32 mtk_gps_log_line_enc_inplace(char *buffer, UINT32 length);

 /*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_do_hw_suspend_resume
 * DESCRIPTION
 *  callback to do GPS HW suspend and resume to reset GPS HW
 * PARAMETERS
 *  bitmask     [IN] bitmask to identify why libmnl call this api
 *                   see: MNL_HW_SUSPEND_MASK_*
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
#define MNL_HW_SUSPEND_MASK_ENABLED_FLAG  0x0001
#define MNL_HW_SUSPEND_MASK_RESUME_FLAG   0x0002
#define MNL_HW_SUSPEND_MASK_RESTART_FLAG  0x0004
INT32 mtk_gps_sys_do_hw_suspend_resume(UINT32 bitmask);


#ifdef __cplusplus
   }
#endif

#endif /* MTK_GPS_H */
