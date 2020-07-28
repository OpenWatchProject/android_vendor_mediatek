/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   CFG_file_info.c
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   Configuration File List
 *
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 ****************************************************************************/
/*
#ifndef __CFG_FILE_INFO_H__
#define __CFG_FILE_INFO_H__

#ifdef __cplusplus
extern "C"
{
#endif
*/
#define __ALLOCATE_CFG_AUDIO_DEFAULT_H

#include "CFG_file_public.h"
#include "libnvram.h"
#include "libnvram_log.h"
#include "inc/CFG_file_lid.h"
#include "inc/CFG_module_file.h"
#include "inc/CFG_module_default.h"
#include "inc/CFG_file_info.h"
#include "CFG_file_info_custom.h"
#include <string.h>
#include <stdio.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_FILENAMELEN 128;
#define RESERVE_PATH	"Reserved"
#define RESERVE_VER	"000"

int BT_ConvertFunc(int, int, char*, char*);
int SBP_ConvertFunc(int, int, char*, char*);

const TCFG_FILE g_akCFG_File[] = {
	//nvram version information
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/FILE_VER",		VER(AP_CFG_FILE_VER_INFO_LID), 		4,
		CFG_FILE_VER_FILE_REC_TOTAL,			DEFAULT_ZERO,						0,  DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr",		VER(AP_CFG_RDEB_FILE_BT_ADDR_LID),	CFG_FILE_BT_ADDR_REC_SIZE,
		CFG_FILE_BT_ADDR_REC_TOTAL,			DEFAULT_ZERO,				0,  DataConvert , BT_ConvertFunc
	},

#if 0
	{
		"/mnt/vendor/nvdata/APCFG/APRDEB/WIFI",	    	VER(AP_CFG_RDEB_FILE_WIFI_LID),		    CFG_FILE_WIFI_REC_SIZE,
		CFG_FILE_WIFI_REC_TOTAL,		    	SIGNLE_DEFUALT_REC,				    (char *)&stWifiCfgDefault, DataReset , NULL
	},
#endif
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/AUXADC",			VER(AP_CFG_RDCL_FILE_AUXADC_LID),	CFG_FILE_AUXADC_REC_SIZE,
		CFG_FILE_AUXADC_REC_TOTAL,				SIGNLE_DEFUALT_REC,					(char *)&stADCDefualt, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_Para",	VER(AP_CFG_RDCL_CAMERA_PARA_LID),   CFG_FILE_CAMERA_PARA_REC_SIZE,
		CFG_FILE_CAMERA_PARA_REC_TOTAL,			DEFAULT_ZERO,						0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_3A",	   	VER(AP_CFG_RDCL_CAMERA_3A_LID),     CFG_FILE_CAMERA_3A_REC_SIZE,
		CFG_FILE_CAMERA_3A_REC_TOTAL,			    DEFAULT_ZERO,						0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_DEFECT",	VER(AP_CFG_RDCL_CAMERA_DEFECT_LID), CFG_FILE_CAMERA_DEFECT_REC_SIZE,
		CFG_FILE_CAMERA_DEFECT_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SENSOR",	VER(AP_CFG_RDCL_CAMERA_SENSOR_LID), CFG_FILE_CAMERA_SENSOR_REC_SIZE,
		CFG_FILE_CAMERA_SENSOR_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_LENS",	VER(AP_CFG_RDCL_CAMERA_LENS_LID),   CFG_FILE_CAMERA_LENS_REC_SIZE,
		CFG_FILE_CAMERA_LENS_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/UART",			VER(AP_CFG_RDCL_UART_LID), 			CFG_FILE_UART_CONFIG_SIZE,
		CFG_FILE_UART_CONFIG_TOTAL,					SIGNLE_DEFUALT_REC,				(char *)&stUARTConfigDefault, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/FACTORY",		VER(AP_CFG_RDCL_FACTORY_LID), 		CFG_FILE_FACTORY_REC_SIZE,
		CFG_FILE_FACTORY_REC_TOTAL,				DEFAULT_ZERO,			    	    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/BWCS",			VER(AP_CFG_RDCL_BWCS_LID), 	        CFG_FILE_BWCS_CONFIG_SIZE,
		CFG_FILE_BWCS_CONFIG_TOTAL,				SIGNLE_DEFUALT_REC,					(char *)&stBWCSConfigDefault, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/HWMON_ACC",		VER(AP_CFG_RDCL_HWMON_ACC_LID), 	CFG_FILE_HWMON_ACC_REC_SIZE,
		CFG_FILE_HWMON_ACC_REC_TOTAL,		    DEFAULT_ZERO,					    0, DataReset , NULL
	},
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/HWMON_GYRO",	VER(AP_CFG_RDCL_HWMON_GYRO_LID), 	CFG_FILE_HWMON_GYRO_REC_SIZE,
		CFG_FILE_HWMON_GYRO_REC_TOTAL,		    DEFAULT_ZERO,					    0, DataReset, NULL
	},
#if 0
	{
		"/mnt/vendor/nvdata/APCFG/APRDEB/WIFI_CUSTOM",	VER(AP_CFG_RDEB_WIFI_CUSTOM_LID),	CFG_FILE_WIFI_CUSTOM_REC_SIZE,
		CFG_FILE_WIFI_CUSTOM_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stWifiCustomDefault, DataReset , NULL
	},
#endif
	{
		"/mnt/vendor/nvdata/APCFG/APRDEB/OMADM_USB",		VER(AP_CFG_RDEB_OMADM_USB_LID),		CFG_FILE_OMADMUSB_REC_SIZE,
		CFG_FILE_OMADMUSB_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stOMADMUSBDefualt, DataReset , NULL
	},
	// { "/nvram/APCFG/APRDCL/ADDED",	VER(AP_CFG_RDCL_ADDED_LID),	CFG_FILE_added_CONFIG_SIZE,
	//	CFG_FILE_added_CONFIG_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stAddedDefault},
	{
		"/mnt/vendor/nvdata/media/Voice_Recognize_Param",   VER(AP_CFG_RDCL_FILE_VOICE_RECOGNIZE_PARAM_LID), CFG_FILE_VOICE_RECOGNIZE_PAR_SIZE,
		CFG_FILE_VOICE_RECOGNIZE_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Voice_Recognize_Par_default, DataReset , NULL
	},
//Reserved ten item
	{
		"/mnt/vendor/nvdata/media/Audio_AudEnh_Control_Opt",   VER(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID), CFG_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_SIZE,
		CFG_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&AUDENH_Control_Option_Par_default, DataReset , NULL
	},
	{
		"/mnt/vendor/nvdata/media/Audio_VOIP_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID), CFG_FILE_AUDIO_VOIP_PAR_SIZE,
		CFG_FILE_AUDIO_VOIP_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Audio_VOIP_Par_default, DataReset , NULL
	},
	/*yucong add for PS calibration*/
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/HWMON_PS",	VER(AP_CFG_RDCL_HWMON_PS_LID), 	CFG_FILE_HWMON_PS_REC_SIZE,
		CFG_FILE_HWMON_PS_REC_TOTAL,		    DEFAULT_ZERO,					    0, DataReset, NULL
	},
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/MD_Type",   VER(AP_CFG_FILE_MDTYPE_LID), CFG_FILE_MDTYPE_CONFIG_SIZE,
		CFG_FILE_MDTYPE_CONFIG_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&stMDTypeDefault, DataReset , NULL
	},
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/EXT_MD_Type",   VER(AP_CFG_FILE_EXT_MDTYPE_LID), CFG_FILE_EXT_MDTYPE_CONFIG_SIZE,
		CFG_FILE_EXT_MDTYPE_CONFIG_TOTAL, SIGNLE_DEFUALT_REC,    (char *)&stEXTMDTypeDefault, DataReset , NULL
	},
	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/SDIO",   VER(AP_CFG_RDCL_FILE_SDIO_LID), CFG_FILE_SDIO_CONFIG_SIZE,
		CFG_FILE_SDIO_CONFIG_TOTAL, SIGNLE_DEFUALT_REC  , (char *)&stSDIOConfigDefault, DataReset , NULL
	},
	{
		"/mnt/vendor/nvdata/media/CAMERA_VERSION",	VER(AP_CFG_RDCL_CAMERA_VERSION_LID),   CFG_FILE_CAMERA_VERSION_REC_SIZE,
		CFG_FILE_CAMERA_VERSION_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_FEATURE",	VER(AP_CFG_RDCL_CAMERA_FEATURE_LID),   CFG_FILE_CAMERA_FEATURE_REC_SIZE,
		CFG_FILE_CAMERA_FEATURE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_GEOMETRY",	VER(AP_CFG_RDCL_CAMERA_GEOMETRY_LID),   CFG_FILE_CAMERA_GEOMETRY_REC_SIZE,
		CFG_FILE_CAMERA_GEOMETRY_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/APCFG/APRDCL/MD_SBP",   VER(AP_CFG_RDCL_FILE_MD_SBP_LID), CFG_FILE_MD_SBP_CONFIG_SIZE,
		CFG_FILE_MD_SBP_CONFIG_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&stMDSBPConfigDefault, DataReset , NULL
	},

//Reserved

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING2",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING3",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING4",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING5",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING6",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING7",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING8",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING9",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING10",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING11",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_SHADING12",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID), CFG_FILE_CAMERA_SHADING_REC_SIZE,
		CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
	},


	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE2",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE3",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE4",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE5",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE6",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE7",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE8",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE9",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE10",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE11",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_PLINE12",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
		CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_AF",	VER(AP_CFG_RDCL_CAMERA_AF_LID),   CFG_FILE_CAMERA_AF_REC_SIZE,
		CFG_FILE_CAMERA_AF_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},

	{
		"/mnt/vendor/nvdata/media/CAMERA_FLASH_CALIBRATION",	VER(AP_CFG_RDCL_CAMERA_FLASH_CALIBRATION_LID),   CFG_FILE_CAMERA_FLASH_CALIBRATION_REC_SIZE,
		CFG_FILE_CAMERA_FLASH_CALIBRATION_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
	},



};
int iCustomBeginLID = AP_CFG_CUSTOM_BEGIN_LID;
extern int iCustomBeginLID;
int iFileVerInfoLID = AP_CFG_FILE_VER_INFO_LID;
extern int iFileVerInfoLID;
int iFileBTAddrLID = AP_CFG_RDEB_FILE_BT_ADDR_LID;
extern int iFileBTAddrLID;
int iFileAuxADCLID = AP_CFG_RDCL_FILE_AUXADC_LID;
extern int iFileAuxADCLID;
int iFileOMADMUSBLID = AP_CFG_RDEB_OMADM_USB_LID;
extern int iFileOMADMUSBLID;


typedef struct {
	char	cFileName[128];
	unsigned int iLID;
} FileName;
typedef struct {
	char cCheckFileName[128];
	int  iCheckFileIndex;

} CheckFileName;

typedef struct {
	char cRecordCallerName[128];
	int  iRecordCallerIndex;
} RecordCallerName;


typedef struct {
	int lid;
	unsigned char meta_mode_write;
	unsigned char security_enable;
} TCFG_FILE_PROTECT;



FileName aBackupToBinRegion[] = {
	{"FILE_VER", AP_CFG_FILE_VER_INFO_LID},
	{"BT_Addr", AP_CFG_RDEB_FILE_BT_ADDR_LID},
	{"WIFI", AP_CFG_RDEB_FILE_WIFI_LID},
	{"AUXADC", AP_CFG_RDCL_FILE_AUXADC_LID},
	{"FACTORY", AP_CFG_RDCL_FACTORY_LID},
	{"HWMON_ACC", AP_CFG_RDCL_HWMON_ACC_LID},
	{"HWMON_GYRO", AP_CFG_RDCL_HWMON_GYRO_LID},
	{"HWMON_PS", AP_CFG_RDCL_HWMON_PS_LID}, //yucong add for ALSPS calibration
	{"WIFI_CUSTOM", AP_CFG_RDEB_WIFI_CUSTOM_LID},
	{"GPS", AP_CFG_CUSTOM_FILE_GPS_LID},
#ifndef MTK_PRODUCT_INFO_SUPPORT
	{"PRODUCT_INFO", AP_CFG_REEB_PRODUCT_INFO_LID},
#endif
#ifdef	MTK_SDIORETRY_SUPPORT
	{"SDIO_RETRY", AP_CFG_RDEB_SDIO_RETRY_LID},
#endif
#if	defined(MTK_MT8193_HDMI_SUPPORT)||defined(MTK_INTERNAL_HDCP_SUPPORT)
	{"Hdcp_Key_Table", AP_CFG_RDCL_FILE_HDCP_KEY_LID},
#endif
	{"SDIO", AP_CFG_RDCL_FILE_SDIO_LID},
	{"MD_SBP", AP_CFG_RDCL_FILE_MD_SBP_LID},
#ifdef MTK_SPEAKER_MONITOR_SUPPORT
	{"Spk_Monitor", AP_CFG_RDCL_FILE_AUDIO_SPEAKER_MONITOR_LID},
#endif
	{"CAMERA_VERSION", AP_CFG_RDCL_CAMERA_VERSION_LID},
	{"CAMERA_AF", AP_CFG_RDCL_CAMERA_AF_LID},
	{"CAMERA_FLASH_CALIBRATION", AP_CFG_RDCL_CAMERA_FLASH_CALIBRATION_LID},
#ifdef CFG_FILE_FG_CONFIG_TOTAL
	{"FG", AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID},
#endif
#ifdef MTK_FACTORY_GAMMA_SUPPORT
	{"PQ_CUSTOM", AP_CFG_RDCL_FILE_PQ_LID},
#endif
#ifdef MTK_SCP_SMARTPA_SUPPORT
	{"smartpa_calib", AP_CFG_CUSTOM_FILE_SMARTPA_CALIB_LID},
#endif
};
FileName aPerformance[] = {
	{"CAMERA_Para", AP_CFG_RDCL_CAMERA_PARA_LID},
	{"CAMERA_3A", AP_CFG_RDCL_CAMERA_3A_LID},
	{"CAMERA_SHADING", AP_CFG_RDCL_CAMERA_SHADING_LID},
	{"CAMERA_DEFECT", AP_CFG_RDCL_CAMERA_DEFECT_LID},
	{"CAMERA_SENSOR", AP_CFG_RDCL_CAMERA_SENSOR_LID},
	{"CAMERA_LENS", AP_CFG_RDCL_CAMERA_LENS_LID},
	{"CAMERA_SHADING2", AP_CFG_RDCL_CAMERA_SHADING2_LID},
	{"CAMERA_SHADING3", AP_CFG_RDCL_CAMERA_SHADING3_LID},
	{"CAMERA_SHADING4", AP_CFG_RDCL_CAMERA_SHADING4_LID},
	{"CAMERA_SHADING5", AP_CFG_RDCL_CAMERA_SHADING5_LID},
	{"CAMERA_SHADING6", AP_CFG_RDCL_CAMERA_SHADING6_LID},
	{"CAMERA_SHADING7", AP_CFG_RDCL_CAMERA_SHADING7_LID},
	{"CAMERA_SHADING8", AP_CFG_RDCL_CAMERA_SHADING8_LID},
	{"CAMERA_SHADING9", AP_CFG_RDCL_CAMERA_SHADING9_LID},
	{"CAMERA_SHADING10", AP_CFG_RDCL_CAMERA_SHADING10_LID},
	{"CAMERA_SHADING11", AP_CFG_RDCL_CAMERA_SHADING11_LID},
	{"CAMERA_SHADING12", AP_CFG_RDCL_CAMERA_SHADING12_LID},
	{"CAMERA_PLINE", AP_CFG_RDCL_CAMERA_PLINE_LID},
	{"CAMERA_PLINE2", AP_CFG_RDCL_CAMERA_PLINE2_LID},
	{"CAMERA_PLINE3", AP_CFG_RDCL_CAMERA_PLINE3_LID},
	{"CAMERA_PLINE4", AP_CFG_RDCL_CAMERA_PLINE4_LID},
	{"CAMERA_PLINE5", AP_CFG_RDCL_CAMERA_PLINE5_LID},
	{"CAMERA_PLINE6", AP_CFG_RDCL_CAMERA_PLINE6_LID},
	{"CAMERA_PLINE7", AP_CFG_RDCL_CAMERA_PLINE7_LID},
	{"CAMERA_PLINE8", AP_CFG_RDCL_CAMERA_PLINE8_LID},
	{"CAMERA_PLINE9", AP_CFG_RDCL_CAMERA_PLINE9_LID},
	{"CAMERA_PLINE10", AP_CFG_RDCL_CAMERA_PLINE10_LID},
	{"CAMERA_PLINE11", AP_CFG_RDCL_CAMERA_PLINE11_LID},
	{"CAMERA_PLINE12", AP_CFG_RDCL_CAMERA_PLINE12_LID},
	{"CAMERA_GEOMETRY", AP_CFG_RDCL_CAMERA_GEOMETRY_LID},
	{"CAMERA_AF", AP_CFG_RDCL_CAMERA_AF_LID},
	{"CAMERA_FLASH_CALIBRATION", AP_CFG_RDCL_CAMERA_FLASH_CALIBRATION_LID},


};
const TCFG_FILE_PROTECT g_akCFG_File_Protect[] = {
	// {AP_CFG_RDEB_FILE_BT_ADDR_LID,1,1},
};
CheckFileName aCheckExistList[] = {
///	{"/mnt/vendor/nvdata/APCFG/APRDCL/FILE_VER",		0},


};

RecordCallerName aRecordCallerList[] = {
	{"Barcode",		1},
	{"IMEI",		2},
};



extern const TCFG_FILE g_akCFG_File[];
extern FileName aBackupToBinRegion[];
extern FileName aPerformance[];
extern CheckFileName aCheckExistList[];
extern const TCFG_FILE_PROTECT g_akCFG_File_Protect[];



const unsigned int g_i4CFG_File_Count = sizeof(g_akCFG_File) / sizeof(
        TCFG_FILE);
const unsigned int g_Backup_File_Count = sizeof(aBackupToBinRegion) / (sizeof(
            FileName));
const unsigned int g_Performance_File_Count = sizeof(aPerformance) / (sizeof(
            FileName));
const unsigned int g_Check_File_Count = sizeof(aCheckExistList) / (sizeof(
        CheckFileName));
const unsigned int g_i4CFG_File_Protect_Count = sizeof(g_akCFG_File_Protect) /
        (sizeof(TCFG_FILE_PROTECT));
const unsigned int g_RecordCaller_Count = sizeof(aRecordCallerList) / (sizeof(
            RecordCallerName));

extern const unsigned int g_i4CFG_File_Count;
extern const unsigned int g_Backup_File_Count;
extern const unsigned int g_Performance_File_Count;
extern const unsigned int g_Check_File_Count ;
extern const unsigned int g_i4CFG_File_Protect_Count;
extern const unsigned int g_RecordCaller_Count;
//misc log feature
int nvram_misc_log_feature = 0;
int nvram_misc_log_block_offset = 2;


//check exist feature
int nvram_check_exist_feature = 0;
int nvram_check_exist_block_offset = 1;


//read back check feature
int nvram_read_back_feature = 0;

pfConvertFunc aNvRamConvertFuncTable[] = {
	NULL,//AP_CFG_FILE_VER_INFO_LID
	NULL,//AP_CFG_RDEB_FILE_BT_ADDR_LID
	NULL,//AP_CFG_RDCL_FILE_AUXADC_LID
	NULL,//AP_CFG_RDCL_CAMERA_PARA_LID
	NULL,//AP_CFG_RDCL_CAMERA_3A_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING_LID
	NULL,//AP_CFG_RDCL_CAMERA_DEFECT_LID
	NULL,//AP_CFG_RDCL_CAMERA_SENSOR_LID
	NULL,//AP_CFG_RDCL_CAMERA_LENS_LID
	NULL,//AP_CFG_RDCL_UART_LID
	NULL,//AP_CFG_RDCL_FACTORY_LID
	NULL,//AP_CFG_RDCL_BWCS_LID
	NULL,//AP_CFG_RDCL_HWMON_ACC_LID
	NULL,//AP_CFG_RDCL_HWMON_GYRO_LID
	NULL,//AP_CFG_RDCL_HWMON_PS_LID
	NULL,//AP_CFG_RDCL_FILE_AUDIO_LID
	NULL,//AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID
	NULL,//AP_CFG_RDCL_FILE_AUDIO_EFFECT_LID
	NULL,//AP_CFG_CUSTOM_FILE_GPS_LID
	NULL,//AP_CFG_RDEB_FILE_WIFI_LID
	NULL,//AP_CFG_RDEB_WIFI_CUSTOM_LID
	NULL,//AP_CFG_RDEB_OMADM_USB_LID
	NULL,//AP_CFG_REEB_PRODUCT_INFO_LID
#ifdef	MTK_SDIORETRY_SUPPORT
	NULL,//AP_CFG_REEB_SDIO_RETRY_LID
#endif
	NULL,//AP_CFG_RDCL_FILE_MD_SBP_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING2_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING3_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING4_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING5_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING6_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING7_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING8_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING9_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING10_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING11_LID
	NULL,//AP_CFG_RDCL_CAMERA_SHADING12_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE2_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE3_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE4_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE5_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE6_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE7_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE8_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE9_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE10_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE11_LID
	NULL,//AP_CFG_RDCL_CAMERA_PLINE12_LID

};

extern pfConvertFunc aNvRamConvertFuncTable[];

//Add for new nvram partition feature
pfCallbackForDaemon callback_for_nvram_daemon = NULL;
extern pfCallbackForDaemon callback_for_nvram_daemon;

#ifdef MTK_PRODUCT_INFO_SUPPORT
extern bool nvram_new_partition_support() {
	return true;
}
const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[] = {
#ifdef MTK_MLC_NAND_SUPPORT
	{ AP_CFG_REEB_PRODUCT_INFO_LID, 0, 2 * 1024 * 1024 },
#else
	{ AP_CFG_REEB_PRODUCT_INFO_LID, 0, 1024 * 1024 },
#endif
};
const unsigned int g_new_nvram_lid_count = sizeof(g_new_nvram_lid) / sizeof(
            TABLE_FOR_SPECIAL_LID);
const char *nvram_new_partition_name = "/dev/pro_info";
extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
#else
extern bool nvram_new_partition_support() {
	return false;
}
const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[] = {0 , 0, 0};
const unsigned int g_new_nvram_lid_count = 0;
const char *nvram_new_partition_name = NULL;
extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
#endif
//end new nvram partition feature


int nvram_bt_default_value(unsigned char *ucNvRamData) {
	unsigned int chipId = 0;
	int  chipId_ready_retry = 0;
	char chipId_val[PROPERTY_VALUE_MAX];
	int  rec_size = CFG_FILE_BT_ADDR_REC_SIZE;

	do {
		if (property_get("persist.vendor.connsys.chipid", chipId_val, NULL) &&
		        0 != strcmp(chipId_val, "-1")) {
			chipId = (unsigned int)strtoul(chipId_val, NULL, 16);
			break;
		} else {
			chipId_ready_retry ++;
			usleep(500000);
		}
	} while (chipId_ready_retry < 10);

	NVRAM_LOG("Get combo chip id retry %d\n", chipId_ready_retry);
	if (chipId_ready_retry >= 10) {
		NVRAM_LOG("Get combo chip id fails!\n");
		return false;
	} else {
		NVRAM_LOG("Combo chip id %04x\n", chipId);

		switch (chipId) {
		case 0x6628:
			/* NVRAM is MT6628 default */
			memcpy(ucNvRamData, &stBtDefault_6628, rec_size);
			break;
		case 0x6630:
			/* NVRAM is MT6630 default */
			memcpy(ucNvRamData, &stBtDefault_6630, rec_size);
			break;
		case 0x6632:
			/* NVRAM is MT6632 default */
			memcpy(ucNvRamData, &stBtDefault_6632, rec_size);
			break;
		case 0x8163:
		case 0x8127:
		case 0x8167:
		case 0x6582:
		case 0x6592:
		case 0x6752:
		case 0x0321:
		case 0x0335:
		case 0x0337:
		case 0x6580:
		case 0x6570:
		case 0x6735:
		case 0x6755:
		case 0x6797:
		case 0x6757:
		case 0x6759:
		case 0x6763:
		case 0x6758:
		case 0x6739:
		case 0x6771:
		case 0x6775:
			/* NVRAM is A-D die chip default */
			memcpy(ucNvRamData, &stBtDefault_consys, rec_size);
			break;
		case 0x6765:
		case 0x3967:
		case 0x6761:
		case 0x6768:
		case 0x6785:
			/* NVRAM is CONNAC1.0 SOC1_0 chip default */
			memcpy(ucNvRamData, &stBtDefault_connac_soc_1_0, rec_size);
			break;
		case 0x6779:
			/* NVRAM is CONNAC1.0 SOC2_0 chip default */
			#ifdef MTK_CONSYS_ADIE_6631
				memcpy(ucNvRamData, &stBtDefault_connac_soc_2_0_6631, rec_size);
			#else
				memcpy(ucNvRamData, &stBtDefault_connac_soc_2_0, rec_size);
			#endif
			break;
		case 0x6885:
			memcpy(ucNvRamData, &stBtDefault_connac_soc_3_0, rec_size);
			break;
		default:
			NVRAM_LOG("Unknown combo chip id\n");
			return false;
		}

		return true;
	}
}

int BT_ConvertFunc(int CurrentVerID, int NewVerID, char *pSrcMem,
                   char *pDstMem) {
	int rec_size = CFG_FILE_BT_ADDR_REC_SIZE;
	int rec_num = CFG_FILE_BT_ADDR_REC_TOTAL;

	if (NULL == pSrcMem || NULL == pDstMem) {
		return false;
	} else {
		NVRAM_LOG("BT_ConvertFunc: CurrentVerID = %d, NewVerID = %d\n", CurrentVerID, NewVerID);

		if (0 == CurrentVerID && 1 == NewVerID) {
			memcpy(pDstMem, pSrcMem, 30); /* Keep the customization data */
			memset(pDstMem + 30, 0, rec_size * rec_num - 30);
			return true;
		} else if (1 == CurrentVerID && 2 == NewVerID) {
			memcpy(pDstMem, pSrcMem, 64); /* Keep the customization data */
			memset(pDstMem + 64, 0, rec_size * rec_num - 64);
			return true;
		}
		return false;
	}
}

#ifdef MTK_EMMC_SUPPORT
extern bool nvram_emmc_support() {
	return true;
}
#else
extern bool nvram_emmc_support() {
	return false;
}
#endif

#ifdef MTK_UFS_SUPPORT
extern bool nvram_ufs_support() {
	return true;
}
#else
extern bool nvram_ufs_support() {
	return false;
}
#endif

extern bool nvram_md1_support() {
	char prop_value[PROPERTY_VALUE_MAX];
	unsigned int md1_support = 0;
	bool is_support_md1 = false;
	if (property_get("ro.vendor.mtk_md1_support", prop_value, NULL)) {
		md1_support = (unsigned int)strtoul(prop_value, NULL, 0);
		if (md1_support)
			is_support_md1 = true;
	} else {
		NVRAM_LOG("ro.vendor.mtk_md1_support property get fail : %s\n", prop_value);
	}
	return is_support_md1;
}

extern bool nvram_md5_support() {
	char prop_value[PROPERTY_VALUE_MAX];
	unsigned int md5_support = 0;
	bool is_support_md5 = false;
	if (property_get("ro.boot.opt_md5_support", prop_value, NULL)) {
		md5_support = (unsigned int)strtoul(prop_value, NULL, 0);
		if (md5_support)
			is_support_md5 = true;
	} else {
		//NVRAM_LOG("ro.boot.opt_md5_support property get fail : %s\n", prop_value);
	}
	return is_support_md5;
}


extern bool nvram_evdo_support() {
	static int run_count = 0;
	char c2k_state_value[PROPERTY_VALUE_MAX];
	unsigned int c2k_support = 0;
	bool is_support_c2k = false;
	if (property_get("ro.vendor.mtk_md3_support", c2k_state_value, NULL)) {
		c2k_support = (unsigned int)strtoul(c2k_state_value, NULL, 0);
		if (c2k_support)
			is_support_c2k = true;

		if ((run_count++)%5 == 0) {
			NVRAM_LOG("mtk_md3_support %d\n", c2k_support);
		}
	} else {
		NVRAM_LOG("ro.vendor.mtk_md3_support property get fail : %s\n", c2k_state_value);
	}
	return is_support_c2k;
}
extern bool nvram_ecci_c2k_support() {
	static int run_count = 0;
	char prop_value[PROPERTY_VALUE_MAX];
	unsigned int ecci_c2k_support = 0;
	bool is_support_ecci_c2k = false;
	if (property_get("ro.vendor.mtk_eccci_c2k", prop_value, NULL)) {
		ecci_c2k_support = (unsigned int)strtoul(prop_value, NULL, 0);
		if (ecci_c2k_support)
			is_support_ecci_c2k = true;
	} else {
		NVRAM_LOG("ro.vendor.mtk_eccci_c2k property get fail : %s\n", prop_value);
	}
	if (is_support_ecci_c2k == false) {
		property_get("ro.hardware", prop_value, "");
		if (!strstr(prop_value, "mt2601") && !strstr(prop_value, "mt6572") &&
		        !strstr(prop_value, "mt6580") && !strstr(prop_value, "mt6582") &&
		        !strstr(prop_value, "mt6592") && !strstr(prop_value, "mt6735") &&
		        !strstr(prop_value, "mt6752") && !strstr(prop_value, "mt6755") &&
		        !strstr(prop_value, "mt6795") && !strstr(prop_value, "mt6797") &&
		        !strstr(prop_value, "mt7623") && !strstr(prop_value, "mt8127") &&
		        !strstr(prop_value, "mt8163") && !strstr(prop_value, "mt8173")) {
			is_support_ecci_c2k = true;
		}
	}
	if ((run_count++)%5 == 0) {
		NVRAM_LOG("is_support_ecci_c2k %d\n", is_support_ecci_c2k);
	}
	return is_support_ecci_c2k;
}



#ifdef NVRAM_MULTI_STORAGE_SUPPORT
extern int nvram_multi_storage_support(NVRAM_PLATFORM_T* pPlatform ) {
	pPlatform->log_block = g_CpPlatform_Block_Custom.log_block;
	pPlatform->resv_block = g_CpPlatform_Block_Custom.resv_block;
	pPlatform->DM_block = g_CpPlatform_Block_Custom.DM_block;
	pPlatform->layout_version = g_CpPlatform_Block_Custom.layout_version;
	pPlatform->header_offset = g_CpPlatform_Block_Custom.header_offset;
	return true;
}
#else
extern int nvram_multi_storage_support(NVRAM_PLATFORM_T* pPlatform ) {
	pPlatform->log_block = 0;
	pPlatform->resv_block = 0;
	pPlatform->DM_block = 0;
	pPlatform->layout_version = 0;
	pPlatform->header_offset = 0;
	return false;
}
#endif


int SBP_ConvertFunc(int CurrentVerID, int NewVerID, char *pSrcMem,
                    char *pDstMem) {
	int rec_size = CFG_FILE_MD_SBP_CONFIG_SIZE;
	int rec_num = CFG_FILE_MD_SBP_CONFIG_TOTAL;
	int src_size = 2 * sizeof(unsigned int);
	unsigned int *pStart = (unsigned int *)pSrcMem;
	int i, len = 0;

	if (NULL == pSrcMem || NULL == pDstMem) {
		return false;
	} else {
		if (0 == CurrentVerID && 1 == NewVerID) {
			NVRAM_LOG("SBP_ConvertFunc curid:%d, newid:%d, src[0]:%d, src[1]:%d\n"
			          , CurrentVerID, NewVerID, *pStart, *(pStart + 1));
			memcpy(pDstMem, pSrcMem, src_size); // Keep the customization data
			// default value: {0, 1}
			pStart = (unsigned int *)(pDstMem + src_size);
			len = (rec_size * rec_num - src_size) / sizeof(unsigned int);
			for (i = 0; i < len; i += 2) {
				*pStart = 0;
				*(pStart + 1) = 1;
				NVRAM_LOG("SBP_ConvertFunc data[%d]:%d, data[%d]:%d\n", i + 2, *pStart, i + 3,
				          *(pStart + 1));
				pStart += 2;
			}
			return true;
		}
		return false;
	}
}

int WIFI_ConvertFunc(int CurrentVerID, int NewVerID, char *pSrcMem, char *pDstMem) {
	unsigned int *pStart = (unsigned int *)pSrcMem;

	if (!pSrcMem && !pDstMem) {
		NVRAM_LOG("WIFI_ConvertFunc curid:%d, newid:%d, src[%p] or dst[%p]\n",
			CurrentVerID, NewVerID, pSrcMem, pDstMem);
		return 0;
	}

	NVRAM_LOG("WIFI_ConvertFunc curid:%d, newid:%d, src[0]:%d, src[1]:%d\n",
		CurrentVerID, NewVerID, *pStart, *(pStart + 1));

	/* Keep the customization data.
	 * Copy version 0 nvram data to the same location of the version 1.
	 * Modify Wi-Fi nvram version from 0x0106 to 0x0107,
	 * Then Set new 0x200 bytes value to 0*/
	if (CurrentVerID == 0 && NewVerID == 1) {
		memcpy(pDstMem, pSrcMem, 0x200);
		memset(pDstMem, 0x07, sizeof(char));
		memset(pDstMem + 0x200, 0, 0x200);
		return 1;
	}
	return 0;
}


/*
#ifdef __cplusplus
}
#endif

#endif
*/
