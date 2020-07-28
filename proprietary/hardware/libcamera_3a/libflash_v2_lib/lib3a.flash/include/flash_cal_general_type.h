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

#ifndef FLASH_CAL_GENERAL_TYPE_H
#define FLASH_CAL_GENERAL_TYPE_H

#include "flash_param.h"
#include "flash_awb_param.h"
#include "FlashAlg.h"

/*!
    \file flash_cal_general_type.h
    \brief Flash calibration algorithm general type header file
    \version 0.1
    \date 2019-12-18
*/

/*!
    \enum FlashCalGeneralInputArgument
    \brief Flash calibration algorithm general input argument list
*/
typedef enum
{
    FLASH_CAL_INPUT_P_YTAB,               /*!< input Y table pointer */
    FLASH_CAL_INPUT_P_DUTY_I,             /*!< input current table pointer */
    FLASH_CAL_INPUT_P_DUTY_LTI,           /*!< input low color temperature current table pointer */
    FLASH_CAL_INPUT_P_CAL_DATA,           /*!< input calibration data pointer */
    FLASH_CAL_INPUT_P_GOLDEN_WB_GAIN,     /*!< input golden Flash AWB gain table pointer */
    FLASH_CAL_INPUT_CAL_NUM,              /*!< input calibration number */
    FLASH_CAL_INPUT_TOTAL_STEP,           /*!< input total step */
    FLASH_CAL_INPUT_TOTAL_STEP_LT,        /*!< input low color temperature total step */
    FLASH_CAL_INPUT_DUAL_FLASH_ENABLE,    /*!< input dual Flash enable */
} FlashCalGeneralInputArgument;

/*!
    \enum FlashCalGeneralOutputArgument
    \brief Flash calibration algorithm general output argument list
*/
typedef enum
{
    FLASH_CAL_OUTPUT_P_CHOOSE,      /*!< output choose pointer */
    FLASH_CAL_OUTPUT_P_OUT_WB,      /*!< output Flash AWB gain table pointer */
    FLASH_CAL_OUTPUT_P_OUT_YTAB,    /*!< output Y table pointer */
} FlashCalGeneralOutputArgument;

/*!
    \struct FlashCalUnit
    \brief Flash calibration algorithm unit
*/
typedef struct
{
    int type;      /*!< data type */
    void* data;    /*!< data pointer */
    int size;      /*!< data size */
} FlashCalUnit;

/*!
    \struct FlashCalGroup
    \brief Flash calibration algorithm group
*/
typedef struct
{
    int number;             /*!< unit number */
    FlashCalUnit* units;    /*!< unit pointer */
} FlashCalGroup;

/* Flash API Input */
/*!
    \struct FlashCalInput_ShortPtr
    \brief Flash calibration algorithm input short pointer
*/
typedef struct
{
    short* input;    /*!< input short pointer */
} FlashCalInput_ShortPtr;

/*!
    \struct FlashCalInput_Int
    \brief Flash calibration algorithm input int
*/
typedef struct
{
    int input;    /*!< input int */
} FlashCalInput_Int;

/*!
    \struct FlashCalInput_CalData
    \brief Flash calibration algorithm input calibration data pointer
*/
typedef struct
{
    CalData* input;    /*!< input calibration data pointer */
} FlashCalInput_CalData;

/*!
    \struct FlashCalInput_FLASH_AWB_ALGO_GAIN_N_T
    \brief Flash calibration algorithm input Flash AWB gain table pointer
*/
typedef struct
{
    FLASH_AWB_ALGO_GAIN_N_T* input;    /*!< input Flash AWB gain table pointer */
} FlashCalInput_FLASH_AWB_ALGO_GAIN_N_T;

/* Flash API Output */
/*!
    \struct FlashCalOutput_ShortPtr
    \brief Flash calibration algorithm output short pointer
*/
typedef struct
{
    short* output;    /*!< output short pointer */
} FlashCalOutput_ShortPtr;

/*!
    \struct FlashCalOutput_ChooseResult
    \brief Flash calibration algorithm output choose result pointer
*/
typedef struct
{
    ChooseResult* output;    /*!< output choose result pointer */
} FlashCalOutput_ChooseResult;

/*!
    \struct FlashCalOutput_AWB_GAIN_T
    \brief Flash calibration algorithm output Flash AWB gain table pointer
*/
typedef struct
{
    AWB_GAIN_T* output;    /*!< output Flash AWB gain table pointer */
} FlashCalOutput_AWB_GAIN_T;

#endif
