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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef CAMERA_FLASH_ALGO_TUNING_H
#define CAMERA_FLASH_ALGO_TUNING_H

#include <stddef.h>
#include "MediaTypes.h"

/*!
    \file camera_flash_algo_tuning.h
    \brief Flash AWB & dual Flash tuning parameter header file
    \version 0.1
    \date 2019-12-18
*/

#define NVRAM_CUSTOM_FLASH_REVISION   7511001

/*******************************************************************************
* FLASH NVRAM
********************************************************************************/
#define FLASH_LV_INDEX_UNIT (10) // 1.0 LV
#define FLASH_LV_INDEX_MIN  (0)  // LV 0
#define FLASH_LV_INDEX_MAX  (18) // LV 18
#define FLASH_LV_INDEX_NUM ((FLASH_LV_INDEX_MAX - FLASH_LV_INDEX_MIN) + 1)

#define FLASH_AWB_SCALE_UNIT (512) // 1.0 = 512

// AWB gain
#ifndef AWBGAINT
#define AWBGAINT
typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
} AWB_GAIN_T;
#endif

// Rotation matrix parameter
typedef struct
{
    MINT32 i4RotationAngle; // Rotation angle
    MINT32 i4Cos;           // cos
    MINT32 i4Sin;           // sin
} FLASH_AWB_ROTATION_MATRIX_T;

// Flash AWB tuning parameter
typedef struct
{
//=== Foreground and Background definition ===
    MUINT32 ForeGroundPercentage;  //>50   default: 9
    MUINT32 BackGroundPercentage;  //<50   default: 95

//=== Table to decide foreground weight (m_FG_Weight) ===
//Th1 < Th2 < Th3 < Th4
//FgPercentage_Thx_Val < 2000
    MUINT32 FgPercentage_Th1;  //default: 2
    MUINT32 FgPercentage_Th2;  //default: 5
    MUINT32 FgPercentage_Th3; //default: 10
    MUINT32 FgPercentage_Th4; //default: 15
    MUINT32 FgPercentage_Th1_Val; //default: 200
    MUINT32 FgPercentage_Th2_Val; //default: 250
    MUINT32 FgPercentage_Th3_Val; //default: 300
    MUINT32 FgPercentage_Th4_Val; //default: 350

//=== Location weighting map ===//
//Th1 < Th2 < Th3 < Th4
//location_map_val1 <= location_map_val2 <= location_map_val3 <= location_map_val4 < 500
    MUINT32 location_map_th1; //default: 10
    MUINT32 location_map_th2; //default: 20
    MUINT32 location_map_th3; //default: 40
    MUINT32 location_map_th4; //default: 50
    MUINT32 location_map_val1; //default: 100
    MUINT32 location_map_val2; //default: 110
    MUINT32 location_map_val3; //default: 130
    MUINT32 location_map_val4; //default: 150

//=== Decide foreground Weighting ===//
// FgBgTbl_Y0 <= 2000
    MUINT32 SelfTuningFbBgWeightTbl;  //default: 0
    MUINT32 FgBgTbl_Y0;
    MUINT32 FgBgTbl_Y1;
    MUINT32 FgBgTbl_Y2;
    MUINT32 FgBgTbl_Y3;
    MUINT32 FgBgTbl_Y4;
    MUINT32 FgBgTbl_Y5;


//=== Decide luminance weight === //
//YPrimeWeightTh[i] <= 256
//YPrimeWeight[i] <= 10
    MUINT32 YPrimeWeightTh[5];     // default: {5,9,11,13,15}
    MUINT32 YPrimeWeight[4];     // default: {0, 0.1, 0.3, 0.5, 0.7}

    AWB_GAIN_T FlashPreferenceGain[FLASH_LV_INDEX_NUM];

}FLASH_AWB_ALGO_TUNING_PARAM_T;

#define FLASH_CUSTOM_MAX_DUTY_NUM_HT (40) // Note, related to NVRAM spec
#define FLASH_CUSTOM_MAX_DUTY_NUM_LT (40) // Note, related to NVRAM spec
#define FLASH_CUSTOM_MAX_DUTY_NUM (FLASH_CUSTOM_MAX_DUTY_NUM_HT * FLASH_CUSTOM_MAX_DUTY_NUM_LT)

#define FLASH_DUTY_NUM FLASH_CUSTOM_MAX_DUTY_NUM

// Flash AWB NVRAM structure
typedef struct
{
    FLASH_AWB_ALGO_TUNING_PARAM_T rTuningParam; // Flash AWB tuning parameter
} FLASH_AWB_ALGO_NVRAM_T;

//==============================
// flash nvram
//==============================
typedef enum
{
  FLASH_ALGO_CHOOSE_WARM,
  FLASH_ALGO_CHOOSE_COLD,
}EFLASH_ALGO_CHOOSE_TYPE;

// AWB gain
typedef struct
{
    MINT16 i2R; // R gain
    MINT16 i2B; // B gain
} FLASH_AWB_ALGO_GAIN_N_T;

#endif // _CAMERA_FLASH_ALGO_TUNING_H_
