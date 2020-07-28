
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

#ifndef FLASH_AE_INTERFACE_CONTROL_H
#define FLASH_AE_INTERFACE_CONTROL_H

#include "exports_aaa_flash.h"

/*!
    \file flash_ae_interface_control.h
    \brief Flash AE algorithm interface control header file
    \version 0.1
    \date 2019-12-18
*/

/*!
    \enum FlashAEInterfaceCommand
    \brief Flash AE algorithm interface command list
*/
typedef enum{
    MTK_FLASH_AE_INIT,                       /*!< Flash AE algorithm init */
    MTK_FLASH_AE_UNINIT,                     /*!< Flash AE algorithm uninit */
    MTK_FLASH_AE_SET_FLASH_TYPE,             /*!< set flash type e.g. dual or single */
    MTK_FLASH_AE_RESET,                      /*!< reset */
    MTK_FLASH_AE_ESTIMATE,                   /*!< Flash AE algorithm estimation */
    MTK_FLASH_AE_ADDSTADATA10,               /*!< add and porcess AAO statistic data */
    MTK_FLASH_AE_CAL_FIRST_EQU_AE_PARA,      /*!< convert and output an corresponding parameter with flash pline */
    MTK_FLASH_AE_SET_FLASH_PROFILE,          /*!< set flash profile e.g. duty, eng table ... */
    MTK_FLASH_AE_SET_CAPTURE_PLINE,          /*!< set main flash capture pline */
    MTK_FLASH_AE_SET_PREFLASH_PLINE,         /*!< set pre-flash pline */
    MTK_FLASH_AE_SET_EV_COMP_EX,             /*!< set ev compensation, ev level, and max target value  */
    MTK_FLASH_AE_SET_DEFAULT_PREFERENCES,    /*!< set default Flash AE algorithm tuning data  */
    MTK_FLASH_AE_SET_TUNING_PREFERENCES,     /*!< set Flash AE algorithm tuning data  */
    MTK_FLASH_AE_SET_IS_SAVE_BIN_FILE,       /*!< decide whether Flash AE algorithm should save simulation data  */
    MTK_FLASH_AE_SET_DEBUG_DIR,              /*!< set debug path and project name */
    MTK_FLASH_AE_GET_DBG_DATA,               /*!< get debug exif data */
    MTK_FLASH_AE_CHECK_INPUT_PARA_ERROR      /*!< check input data for algorithm */
} FlashAEInterfaceCommand;

/* flash ae interface control*/
/*!
    \fn int flashAEAlgoCtrl(FlashAEInterfaceCommand cmd, void* pFlashGroupIn, void* pFlashGroupOut, int sensorDev)
    \brief Flash AE algorithm general interface

    \param cmd input general interface command
    \param pFlashGroupIn input general interface parameter pointer
    \param pFlashGroupOut output general interface parameter pointer
    \param sensorDev input sensor ID
*/
AAA_FLASH_EXPORT int flashAEAlgoCtrl(FlashAEInterfaceCommand cmd, void* pFlashGroupIn, void* pFlashGroupOut, int sensorDev);

#endif
