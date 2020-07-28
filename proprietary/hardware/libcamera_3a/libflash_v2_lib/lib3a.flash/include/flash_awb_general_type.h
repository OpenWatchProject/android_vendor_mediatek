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

#ifndef _FLASH_AWB_GENERAL_TYPE_H_
#define _FLASH_AWB_GENERAL_TYPE_H_

#include "flash_param.h"
#include "flash_awb_param.h"
#include "FlashAlg.h"

/*!
    \file flash_awb_general_type.h
    \brief Flash AWB algorithm general type header file
    \version 0.1
    \date 2019-12-18
*/

/*!
    \enum FlashAWBGeneralInputArgument
    \brief Flash AWB algorithm general input argument list
*/
typedef enum
{
    FLASH_AWB_INPUT_P_FLASH_INFO,        /*!< input Flash information pointer */
    FLASH_AWB_INPUT_FLASH_AWB_INIT_T,    /*!< input Flash AWB init parameter */
    FLASH_AWB_INPUT_EXP,                 /*!< input exposure time */
    FLASH_AWB_INPUT_ISO                  /*!< input ISO */
}FlashAWBGeneralInputArgument;

/*!
    \enum FlashAWBGeneralOutputArgument
    \brief Flash AWB algorithm general output argument list
*/
typedef enum
{
    FLASH_AWB_OUTPUT_P_RESULT_WEIGHT    /*!< output result weight pointer */
}FlashAWBGeneralOutputArgument;

/*!
    \struct FlashAWBUnit
    \brief Flash AWB algorithm unit
*/
typedef struct
{
    int type;      /*!< data type */
    void* data;    /*!< data pointer */
    int size;      /*!< data size */
}FlashAWBUnit;

/*!
    \struct FlashAWBGroup
    \brief Flash AWB algorithm group
*/
typedef struct
{
    int number;             /*!< unit number */
    FlashAWBUnit* units;    /*!< unit pointer */
}FlashAWBGroup;

/* Flash AWB API input */
/*!
    \struct FlashAWBInput_Int
    \brief Flash AWB algorithm input int
*/
typedef struct{
    int input;    /*!< input int */
}FlashAWBInput_Int;

/*!
    \struct FlashAWBInput_FlashAlgInputInfoStruct
    \brief Flash AWB algorithm input Flash algorithm input information pointer
*/
typedef struct
{
    FlashAlgInputInfoStruct* input;    /*!< input Flash algorithm input information pointer */
}FlashAWBInput_FlashAlgInputInfoStruct;

/*!
    \struct FlashAWBInput_FLASH_AWB_INIT_T
    \brief Flash AWB algorithm input Flash AWB init parameter pointer
*/
typedef struct
{
    FLASH_AWB_INIT_T* input;    /*!< input Flash AWB init parameter pointer */
}FlashAWBInput_FLASH_AWB_INIT_T;

/* Flash AWB API output */
/*!
    \struct FlashAWBOutput_RESULT_WEIGHT
    \brief Flash AWB algorithm output result weight pointer
*/
typedef struct
{
    MUINT32* output;    /*!< output result weight pointer */
}FlashAWBOutput_RESULT_WEIGHT;

#endif
