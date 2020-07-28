/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
 #define PMTK_MAX_PKT_LENGTH     256

#ifdef MTK_GPS_DUAL_FREQ_SUPPORT  /*Defined in Android.mk*/
#define DSP_DEV                     "/dev/gpsdl0" /* device for L1 */
#define DSP_DEV2                    "/dev/gpsdl1" /* device for L5 */
#else
#define DSP_DEV                     "/dev/stpgps" /* stp-gps char dev*/
#define DSP_DEV2                    "none"        /* DSP2 invalid*/
#endif

/*****************************************************************************
 * FUNCTION
 *  power_on_3332
 * DESCRIPTION
 *  power on MT3332 chip for chip detector.
 * PARAMETERS
 *  
 * RETURNS
 *  
 *****************************************************************************/
void power_on_3332();

/*****************************************************************************
 * FUNCTION
 *  power_off_3332
 * DESCRIPTION
 *  power off MT3332 chip when chip detect done.
 * PARAMETERS
 *  
 * RETURNS
 *  
 *****************************************************************************/
void power_off_3332();


/*****************************************************************************
 * FUNCTION
 *  read_NVRAM
 * DESCRIPTION
 *  Read NVRAM to check if the uart is for MT3332
 * PARAMETERS
 *  
 * RETURNS
 *  1: NOT mt3332, -1: read error, 0:read ok
 *****************************************************************************/
int read_NVRAM();

/*****************************************************************************
 * FUNCTION
 *  init_3332_interface
 * DESCRIPTION
 *  Init UART parameter.
 * PARAMETERS
 *  fd [IN]: UART fd
 * RETURNS
 *  success(0); failure (-1)
 *****************************************************************************/
int init_3332_interface(const int fd);


/*****************************************************************************
 * FUNCTION
 *  hw_test_3332
 * DESCRIPTION
 *  Send command to MT3332 Chip and wait response to check if it is MT3332 CHIP
 * PARAMETERS
 *  fd [IN]: UART fd
 * RETURNS
 * 0: means MT3332, 1 means not MT3332
 *****************************************************************************/
int hw_test_3332(const int fd);

/*****************************************************************************
 * FUNCTION
 *  hand_shake
 * DESCRIPTION
 *  Hand shake with MT3332 chip
 * PARAMETERS
 *  
 * RETURNS
 * 0: means MT3332, 1 means not MT3332
 *****************************************************************************/
int hand_shake();

/*****************************************************************************
 * FUNCTION
 *  confirm_if_3332
 * DESCRIPTION
 *  confirm if it is MT3332
 * PARAMETERS
 *  
 * RETURNS
 * 0: means MT3332, 1 means not MT3332
 *****************************************************************************/
int confirm_if_3332();

/*****************************************************************************
 * FUNCTION
 *  chip_detector
 * DESCRIPTION
 *  To get GPS chip ID
 * PARAMETERS
 *  
 * RETURNS
 * 
 *****************************************************************************/
void chip_detector();
// confirm MT3332 chip - end

/*****************************************************************************
 * FUNCTION
 *  buff_get_int
 * DESCRIPTION
 *  To get a INT data from buff.
 * PARAMETERS
 *  buff [IN]: Store INT data
 *  offset [IN]: offset
 * RETURNS
 * 
 *****************************************************************************/
int buff_get_int(char* buff, int* offset);

/*****************************************************************************
 * FUNCTION
 *  buff_put_int
 * DESCRIPTION
 *  To put a INT data to a buff.
 * PARAMETERS
 *  input [IN]: Input data
 *  buff [IN]: Dest buffer
 *  offset [IN]: offset
 * RETURNS
 * 
 *****************************************************************************/
void buff_put_int(int input, char* buff, int* offset);

/*****************************************************************************
 * FUNCTION
 *  buff_put_string
 * DESCRIPTION
 *  To put CHAR data to a buff.
 * PARAMETERS
 *  str [IN]: Input data
 *  buff [IN]: Dest buffer
 *  offset [IN]: offset
 * RETURNS
 * 
 *****************************************************************************/
void buff_put_string(char* str, char* buff, int* offset);

/*****************************************************************************
 * FUNCTION
 *  buff_put_struct
 * DESCRIPTION
 *  To put struct data to a buff.
 * PARAMETERS
 *  input [IN]: Input data
 *  size  [IN]: Input struct size
 *  buff [IN]: Dest buffer
 *  offset [IN]: offset
 * RETURNS
 * 
 *****************************************************************************/
void buff_put_struct(void* input, int size, char* buff, int* offset);

/*****************************************************************************
 * FUNCTION
 *  buff_get_struct
 * DESCRIPTION
 *  To get struct data from a buff.
 * PARAMETERS
 *  input [OUT]: Dest buffer
 *  size  [IN]: The length to get
 *  buff [IN]: Original buffer
 *  offset [IN]: offset
 * RETURNS
 * 
 *****************************************************************************/
void buff_get_struct(char* output, int size, char* buff, int* offset);

/*****************************************************************************
 * FUNCTION
 *  buff_get_binary
 * DESCRIPTION
 *  To get binary data from a buff.
 * PARAMETERS
 *  output [OUT]: Dest buffer
 *  buff  [IN]: Original buffer
 *  offset [IN]: offset
 * RETURNS
 * 
 *****************************************************************************/
int buff_get_binary(void* output, char* buff, int* offset);

/*****************************************************************************
 * FUNCTION
 *  add_chksum
 * DESCRIPTION
 *  To add checksum in a buff
 *  buff  [IN]: buffer
 * RETURNS
 *
 *****************************************************************************/
int add_chksum(char *pBuf);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_log_is_hide
 * DESCRIPTION
 *  To check hide the gps log or not
 *  buff  [IN]: void
 * RETURNS
 *  0: Print GPS log normally; 1: not print location sensitivity log
 *                                (include gps debug log);
 *****************************************************************************/
int mtk_gps_log_is_hide(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_log_get_hide_opt
 * DESCRIPTION
 *  To check gps log hide option
 *  buff  [IN]: void
 * RETURNS
 *  0: Print GPS log normally; 1: not print location sensitivity log;
 *                             2: encrypt and print location sensitivity log;
 *****************************************************************************/
int mtk_gps_log_get_hide_opt(void);


