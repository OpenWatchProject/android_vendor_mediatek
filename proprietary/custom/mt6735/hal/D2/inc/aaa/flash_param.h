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


#ifndef __FLASH_PARAM_H__
#define __FLASH_PARAM_H__

#include <flash_feature.h>



enum
{
    ENUM_FLASH_TIME_NO_TIME_OUT = 1000000,


};


typedef struct
{
    MINT32 flashMode; //LIB3A_FLASH_MODE_AUTO, LIB3A_FLASH_MODE_ON, LIB3A_FLASH_MODE_OFF,
    MBOOL  isFlash; //0: no flash, 1: image with flash

} FLASH_INFO_T;


typedef struct
{
    MUINT32 toleranceEV_pos;  //0~100, 100 base, ex: 10 = 10/100 = 0.1EV
    MUINT32 toleranceEV_neg;  //0~100, 100 base,  ex: 10 = 10/100 = -0.1EV
    MUINT32 XYWeighting;  //128 base

    MBOOL  useAwbPreferenceGain;

    MINT32 envOffsetIndex[4];
    MINT32 envXrOffsetValue[4];
    MINT32 envYrOffsetValue[4];
}DualFlashAlgPreferences;



typedef struct
{
	int yTarget;  // 188 (10bit)
    int fgWIncreaseLevelbySize; // 10
	int fgWIncreaseLevelbyRef;  // 0
	int ambientRefAccuracyRatio;  // 5  5/256=2%
	int flashRefAccuracyRatio;  // 1   1/256=0.4%
	int backlightAccuracyRatio; // 18 18/256=7%
	int backlightUnderY;  //  40 (10-bit)
    int backlightWeakRefRatio;  // 32  32/256=12.5%
	int safetyExp; // 33322
	int maxUsableISO;  // 680
	int yTargetWeight;  // 0 base:256
	int lowReflectanceThreshold;  // 13  13/256=5%
	int flashReflectanceWeight;  // 0 base:256
	int bgSuppressMaxDecreaseEV;  // 2EV
	int bgSuppressMaxOverExpRatio; // 6 „³ 6/256=2%
	int fgEnhanceMaxIncreaseEV; // 5EV
	int fgEnhanceMaxOverExpRatio; // 6 „³ 10/256=2%
	int isFollowCapPline;  // 0 for auto mode, 1 for others
	int histStretchMaxFgYTarget; // 266 (10bit)
	int histStretchBrightestYTarget; // 328 (10bit)
	int fgSizeShiftRatio; // 0 0/256=0%
	int backlitPreflashTriggerLV; // 90 (unit:0.1EV)
	int backlitMinYTarget; //100 (10bit)

       DualFlashAlgPreferences dualFlashPref; //dual flash tuning parameters
} FLASH_TUNING_PARA;





typedef struct
{
    int tabNum;
    int tabMode;
    int tabId[10]; //index or current
    float coolingTM[10]; //time multiply factor
    int timOutMs[10];

}FLASH_COOL_TIMEOUT_PARA;


typedef bool (*DUTY_MASK_FUNC)(int d, int dLt);
typedef struct
{
    int dutyNum;
    FLASH_TUNING_PARA tuningPara;
    FLASH_COOL_TIMEOUT_PARA coolTimeOutPara;
	int dutyNumLT;
	FLASH_COOL_TIMEOUT_PARA coolTimeOutParaLT;
	DUTY_MASK_FUNC dutyAvailableMaskFunc;
    int maxCapExpTimeUs; //us

	int quickCalibrationNum;


} FLASH_PROJECT_PARA;





#endif  //#ifndef __FLASH_PARAM_H__

