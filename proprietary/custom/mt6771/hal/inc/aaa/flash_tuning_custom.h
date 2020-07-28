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


#ifndef __FLASH_TUNING_CUSTOM_H__
#define __FLASH_TUNING_CUSTOM_H__

#include "flash_param.h"
#include "strobe_param.h"
#include "camera_custom_nvram.h"

/* flash_tuning_custom_cct_[main|main2|sub|sub2][_part2].cpp */
int cust_fillDefaultStrobeNVRam(int sensorDev, void* data);
int cust_fillDefaultStrobeNVRam_main (void* data);
int cust_fillDefaultStrobeNVRam_main2 (void* data);
int cust_fillDefaultStrobeNVRam_main3 (void* data);
int cust_fillDefaultStrobeNVRam_main_part2 (void* data);
int cust_fillDefaultStrobeNVRam_main2_part2 (void* data);
int cust_fillDefaultStrobeNVRam_main3_part2 (void* data);
int cust_fillDefaultStrobeNVRam_sub (void* data);
int cust_fillDefaultStrobeNVRam_sub2 (void* data);
int cust_fillDefaultStrobeNVRam_sub_part2 (void* data);
int cust_fillDefaultStrobeNVRam_sub2_part2 (void* data);

int cust_fillDefaultFlashCalibrationNVRam(int sensorDev, void* data);
int cust_fillDefaultFlashCalibrationNVRam_main(void* data);
int cust_fillDefaultFlashCalibrationNVRam_main2(void* data);
int cust_fillDefaultFlashCalibrationNVRam_main3(void* data);
int cust_fillDefaultFlashCalibrationNVRam_main_part2(void* data);
int cust_fillDefaultFlashCalibrationNVRam_main2_part2(void* data);
int cust_fillDefaultFlashCalibrationNVRam_main3_part2(void* data);
int cust_fillDefaultFlashCalibrationNVRam_sub(void* data);
int cust_fillDefaultFlashCalibrationNVRam_sub2(void* data);
int cust_fillDefaultFlashCalibrationNVRam_sub_part2(void* data);
int cust_fillDefaultFlashCalibrationNVRam_sub2_part2(void* data);


/* flash_tuning_custom_[main|main2|sub|sub2][_part2].cpp */
void cust_getFlashQuick2CalibrationExp(int sensorDev, int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_main(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_main_part2(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_sub(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_sub_part2(int* exp, int* afe, int* isp);

void cust_getFlashHalTorchDuty(int sensorDev, int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_main(int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_main_part2(int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_sub(int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_sub_part2(int* duty, int* dutyLt);

typedef int (*FlashIMapFP)(int, int );
FlashIMapFP cust_getFlashIMapFunc(int sensorDev);
FlashIMapFP cust_getFlashIMapFunc_main();
FlashIMapFP cust_getFlashIMapFunc_main_part2();
FlashIMapFP cust_getFlashIMapFunc_sub();
FlashIMapFP cust_getFlashIMapFunc_sub_part2();

void cust_getFlashITab1(int sensorDev, short* ITab1);
void cust_getFlashITab1_main(short* ITab1);
void cust_getFlashITab1_main_part2(short* ITab1);
void cust_getFlashITab1_sub(short* ITab1);
void cust_getFlashITab1_sub_part2(short* ITab1);

void cust_getFlashITab2(int sensorDev, short* ITab2);
void cust_getFlashITab2_main(short* ITab2);
void cust_getFlashITab2_main_part2(short* ITab2);
void cust_getFlashITab2_sub(short* ITab2);
void cust_getFlashITab2_sub_part2(short* ITab2);

void cust_getFlashMaxIDuty(int sensorDev, int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_main(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_main_part2(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_sub(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_sub_part2(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);

FLASH_PROJECT_PARA& cust_getFlashProjectPara_V2(int sensorDev, int AEScene, NVRAM_CAMERA_STROBE_STRUCT* nvrame);                   // isForceFlash: 0: auto (always)
FLASH_PROJECT_PARA& cust_getFlashProjectPara_V3(int sensorDev, int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame); // isForceFlash: 0: auto, 1: forceOn
FLASH_PROJECT_PARA& cust_getFlashProjectPara_main(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_main_part2(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub_part2(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);


/* flash_custom.cpp */
int cust_isDualFlashSupport(int sensorDev);
void cust_setFlashPartId(int dev, int id);
STROBE_DEVICE_ENUM cust_getStrobeDevice(int sensorDev);
int cust_isIRProjectorSupport(int sensorDev);
int cust_getFlashFrameRateDelay(int sensorDev);
int cust_getHwTriggerSensor();

/* flash_custom_1.cpp */
int cust_getFlashModeStyle(int sensorType, int flashMode);
int cust_getVideoFlashModeStyle(int sensorType, int flashMode);
void cust_getEvCompPara(int& maxEvTar10Bit, int& indNum, float*& evIndTab, float*& evTab, float*& evLevel);
int cust_isNeedAFLamp(int flashMode, int afLampMode, int isBvHigherTriger);


/* flash_custom_2.cpp */
int cust_isSubFlashSupport();


/* flash_custom_3.cpp */
enum {
    e_PrecapAf_None,
    e_PrecapAf_BeforePreflash,
    e_PrecapAf_AfterPreflash,
};
int cust_getPrecapAfMode();
int cust_isNeedDoPrecapAF_v2(int isLastFocusModeTAF, int isFocused, int flashMode, int afLampMode, int isBvLowerTriger);


#endif //#ifndef __FLASH_TUNING_CUSTOM_H__

