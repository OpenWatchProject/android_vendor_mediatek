
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

#ifndef FLASH_CAL_INTERFACE_CONTROL_H
#define FLASH_CAL_INTERFACE_CONTROL_H

#include "exports_aaa_flash.h"

/*!
    \file flash_cal_interface_control.h
    \brief Flash calibration algorithm interface control header file
    \version 0.1
    \date 2019-12-18
*/

/*!
    \enum FlashCalInterfaceCommand
    \brief Flash calibration algorithm interface command list
*/
typedef enum{
    MTK_FLASH_CAL_DECIDE_CAL_FLASH_COMB,    /*!< decide calibration Flash combination */
    MTK_FLASH_CAL_INTERPOLATE_CAL_DATA,     /*!< interpolate calibration data */
} FlashCalInterfaceCommand;

#ifdef __cplusplus
extern "C" {
#endif //#ifdef __cplusplus

AAA_FLASH_EXPORT const char* aaa_flash_GetModuleVersionString();

/* flash calibration interface control*/
/*!
    \fn int flashCalAlgoCtrl(int sensorDev, FlashCalInterfaceCommand cmd, void* pFlashCalGroupIn, void* pFlashCalGroupOut)
    \brief Flash calibration algorithm general interface

    \param sensorDev input sensor ID
    \param cmd input general interface command
    \param pFlashCalGroupIn input general interface parameter pointer
    \param pFlashCalGroupOut output general interface parameter pointer
*/
AAA_FLASH_EXPORT int flashCalAlgoCtrl(int sensorDev, FlashCalInterfaceCommand cmd, void* pFlashCalGroupIn, void* pFlashCalGroupOut);

#ifdef __cplusplus
} // extern "C" {
#endif //#ifdef __cplusplus

#endif
