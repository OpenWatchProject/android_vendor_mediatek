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


#ifndef FLASH_AE_GENERAL_TYPE_H
#define FLASH_AE_GENERAL_TYPE_H

#include "flash_param.h"
#include "flash_awb_param.h"
#include "FlashAlg.h"

/*!
    \file flash_ae_general_type.h
    \brief Flash AE algorithm general type header file
    \version 0.1
    \date 2019-12-18
*/

/*!
    \enum FlashAEGeneralInputArgument
    \brief Flash AE algorithm general input argument list
*/
typedef enum{
    FLASH_AE_INPUT_P_STA_DATA,          /*!< input Flash AAO statistic data pointer */
    FLASH_AE_INPUT_P_EXP,               /*!< input AE converge parameter information pointer */
    FLASH_AE_INPUT_P_LAST_FRAME,        /*!< input whether it is last frame(for leagcy flow) */
    FLASH_AE_INPUT_P_FACE_INFO,         /*!< input Face infromation data pointer */
    FLASH_AE_INPUT_P_STROBE_PROFILE,    /*!< input strobe information pointer e.g. duty num, eng table... */
    FLASH_AE_INPUT_P_PLINE,             /*!< input converted pline table pointer */
    FLASH_AE_INPUT_P_TUNING,            /*!< input tuning data pointer */
    FLASH_AE_INPUT_P_DIR_NAME,          /*!< input flash dump data path pointer */
    FLASH_AE_INPUT_P_PRJ_NAME,          /*!< input flash project name pointer */
    FLASH_AE_INPUT_P_NUM,               /*!< input error number pointer */
    FLASH_AE_INPUT_P_ERR_BUF,           /*!< input error buffer pointer */
    FLASH_AE_INPUT_FLASH_TYPE,          /*!< input flash type, dual flash or single flash */
    FLASH_AE_INPUT_LV,                  /*!< input current Lightness value*/
    FLASH_AE_INPUT_ISO_AT_GAIN1X,       /*!< input sensitivity ratio*/
    FLASH_AE_INPUT_EV_COMP,             /*!< input EV compensation*/
    FLASH_AE_INPUT_TAR,                 /*!< input max target value*/
    FLASH_AE_INPUT_EV_LEVEL,            /*!< input EV level*/
    FLASH_AE_INPUT_IS_SAVE_BIN          /*!< input whether flash ae algorithm should save simulation data*/
} FlashAEGeneralInputArgument;

/*!
    \enum FlashAEGeneralOutputArgument
    \brief Flash AE algorithm general output argument list
*/
typedef enum{
    FLASH_AE_OUTPUT_P_IS_NEED_NEXT,     /*!< output whether Flash AE algorithm should do next estimation round(for legacy flow)*/
    FLASH_AE_OUTPUT_P_EXP_NEXT,         /*!< output pointer next estimation paramter for pre-flash stage(for legacy flow)*/
    FLASH_AE_OUTPUT_P_EXP,              /*!< output Flash AE algorithm calculation result pointer*/
    FLASH_AE_OUTPUT_P_IS_LOW_REF,       /*!< output pointer whether current scenario is low reflectance or not*/
    FLASH_AE_OUTPUT_P_EQU_EXP,          /*!< output converted exp, iso parameter pointer*/
    FLASH_AE_OUTPUT_P_DEBUG_INFO        /*!< output debug exif pointer*/
} FlashAEGeneralOutputArgument;

/*!
    \struct FlashAEUnit
    \brief Flash AE algorithm unit
*/
typedef struct{
    int   type;    /*!< data type */
    void* data;    /*!< data pointer */
    int   size;    /*!< data size */
} FlashAEUnit;

/*!
    \struct FlashAEGroup
    \brief Flash AE algorithm group
*/
typedef struct{
    int        number;    /*!< unit number */
    FlashAEUnit* units;   /*!< unit pointer */
} FlashAEGroup;

/* Flash AE API Input */
/*!
    \struct FlashAEInput_ShortPtr
    \brief Flash AE algorithm short pointer
*/
typedef struct{
    short* input;    /*!< input short pointer */
} FlashAEInput_ShortPtr;

/*!
    \struct FlashAEInput_Int
    \brief Flash AE algorithm input int
*/
typedef struct{
    int input;    /*!< input int */
} FlashAEInput_Int;

/*!
    \struct FlashAEInput_IntPtr
    \brief Flash AE algorithm input int pointer
*/
typedef struct{
    int* input;    /*!< input int pointer */
} FlashAEInput_IntPtr;

/*!
    \struct FlashAEInput_Float
    \brief Flash AE algorithm input float
*/
typedef struct{
    float input;    /*!< input float */
} FlashAEInput_Float;

/*!
    \struct FlashAEInput_ConstCharPtr
    \brief Flash AE algorithm input char pointer
*/
typedef struct{
    const char* input;    /*!< input char pointer */
} FlashAEInput_ConstCharPtr;

/*!
    \struct FlashAEInput_FlashAlgExpPara
    \brief Flash AE algorithm input Flash AE algorithm infmoration pointer
*/
typedef struct{
    FlashAlgExpPara* input;    /*!< input Flash AE algorithm infmoration pointer */
} FlashAEInput_FlashAlgExpPara;

/*!
    \struct FlashAEInput_FlashAlgStaData
    \brief Flash AE algorithm input AAO statistic information pointer
*/
typedef struct{
    FlashAlgStaData* input;    /*!< input AAO statistic information pointer */
} FlashAEInput_FlashAlgStaData;

/*!
    \struct FlashAEInput_FlashAlgFacePos
    \brief Flash AE algorithm input face information pointer
*/
typedef struct{
    FlashAlgFacePos* input;    /*!< input face information pointer */
} FlashAEInput_FlashAlgFacePos;

/*!
    \struct FlashAEInput_PLine
    \brief Flash AE algorithm input flash pline pointer
*/
typedef struct{
    PLine* input;    /*!< input flash pline pointer */
} FlashAEInput_PLine;

/*!
    \struct FlashAEInput_FlashAlgStrobeProfile
    \brief Flash AE algorithm input flash profile(e.g. duty, eng table...) pointer
*/
typedef struct{
    FlashAlgStrobeProfile* input;    /*!< input flash profile(e.g. duty, eng table...) pointer */
} FlashAEInput_FlashAlgStrobeProfile;

/*!
    \struct FlashAEInput_FLASH_TUNING_PARA
    \brief Flash AE algorithm input Flash AE algorithm tuning data pointer
*/
typedef struct{
    FLASH_TUNING_PARA* input;    /*!< input Flash AE algorithm tuning data pointer */
} FlashAEInput_FLASH_TUNING_PARA;

/* Flash API Output */
/*!
    \struct FlashAEOutput_ShortPtr
    \brief Flash AE algorithm output short pointer
*/
typedef struct{
    short* output;    /*!< output short pointer */
} FlashAEOutput_ShortPtr;

/*!
    \struct FlashAEOutput_Int
    \brief Flash AE algorithm output int
*/
typedef struct{
    int output;    /*!< output int */
} FlashAEOutput_Int;

/*!
    \struct FlashAEOutput_IntPtr
    \brief Flash AE algorithm output int pointer
*/
typedef struct{
    int* output;   /*!< output int pointer */
} FlashAEOutput_IntPtr;

/*!
    \struct FlashAEOutput_FlashAlgExpPara
    \brief Flash AE algorithm output Flash AE algorithm infmoration pointer
*/
typedef struct{
    FlashAlgExpPara* output;    /*!< output Flash AE algorithm infmoration pointer */
} FlashAEOutput_FlashAlgExpPara;

/*!
    \struct FlashAEOutput_FlashAlgStrobeProfile
    \brief Flash AE algorithm output flash profile(e.g. duty, eng table...) pointer
*/
typedef struct{
    FlashAlgStrobeProfile* output;    /*!< output flash profile(e.g. duty, eng table...) pointer */
} FlashAEOutput_FlashAlgStrobeProfile;

/*!
    \struct FlashAEOutput_FLASH_ALGO_DEBUG_INFO_T
    \brief Flash AE algorithm output Flash AE algorithm debug exif pointer
*/
typedef struct{
    FLASH_ALGO_DEBUG_INFO_T* output;    /*!< output Flash AE algorithm debug exif pointer */
} FlashAEOutput_FLASH_ALGO_DEBUG_INFO_T;

#endif //_FLASH_AE_GENERAL_TYPE_H_
