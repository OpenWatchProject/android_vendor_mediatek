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

#ifndef _FLASH_ERROR_CODE_H_
#define _FLASH_ERROR_CODE_H_

///////////////////////////////////////////////////////////////////////////////
//!  Error code formmat is:
//!
//!  Bit 31~24 is global, each module must follow it, bit 23~0 is defined by module
//!  | 31(1 bit) |30-24(7 bits) |         23-0   (24 bits)      |
//!  | Indicator | Module ID    |   Module-defined error Code   |
//!
//!  Example 1:
//!  | 31(1 bit) |30-24(7 bits) |   23-16(8 bits)   | 15-0(16 bits) |
//!  | Indicator | Module ID    | group or sub-mod  |    Err Code   |
//!
//!  Example 2:
//!  | 31(1 bit) |30-24(7 bits) | 23-12(12 bits)| 11-8(8 bits) | 7-0(16 bits)  |
//!  | Indicator | Module ID    |   line number |    group     |    Err Code   |
//!
//!  Indicator  : 0 - success, 1 - error
//!  Module ID  : module ID, defined below
//!  Extended   : module dependent, but providee macro to add partial line info
//!  Err code   : defined in each module's public include file,
//!               IF module ID is MODULE_COMMON, the errocode is
//!               defined here
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//! Error code type definition
///////////////////////////////////////////////////////////////////////////
typedef MINT32 MRESULT;

///////////////////////////////////////////////////////////////////////////
//! Helper macros to define error code
///////////////////////////////////////////////////////////////////////////
#define ERROR_CODE_DEFINE(modid, errid)           \
  ((MINT32)                              \
    ((MUINT32)(0x80000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(errid & 0xffff))          \
  )

#define OK_CODE_DEFINE(modid, okid)             \
  ((MINT32)                              \
    ((MUINT32)(0x00000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(okid & 0xffff))           \
  )

///////////////////////////////////////////////////////////////////////////
//! Helper macros to check error code
///////////////////////////////////////////////////////////////////////////
#define SUCCEEDED(Status)   ((MRESULT)(Status) >= 0)
#define FAILED(Status)      ((MRESULT)(Status) < 0)

#define MODULE_MTK_FLASH (0) // Temp value

#define MTKFLASH_OKCODE(errid)         OK_CODE_DEFINE(MODULE_MTK_FLASH, errid)
#define MTKFLASH_ERRCODE(errid)        ERROR_CODE_DEFINE(MODULE_MTK_FLASH, errid)


// FLASH error code
#define S_FLASH_OK                  			MTKFLASH_OKCODE(0x0000)

#define E_FLASH_DUAL_CALI_PARAM_ERR				MTKFLASH_ERRCODE(0x0001)
#define E_FLASH_DUAL_CALI_IN_Y_RANGE_ERR			MTKFLASH_ERRCODE(0x0002)
#define E_FLASH_DUAL_CALI_IN_Y_VALUE_ERR			MTKFLASH_ERRCODE(0x0003)
#define E_FLASH_DUAL_CALI_IN_WB_RANGE_ERR		MTKFLASH_ERRCODE(0x0004)
#define E_FLASH_DUAL_CALI_IN_WB_VALUE_ERR		MTKFLASH_ERRCODE(0x0005)
#define E_FLASH_DUAL_CALI_GOLD_Y_RANGE_ERR		MTKFLASH_ERRCODE(0x0006)
#define E_FLASH_DUAL_CALI_GOLD_Y_VALUE_ERR		MTKFLASH_ERRCODE(0x0007)
#define E_FLASH_DUAL_CALI_GOLD_WB_RANGE_ERR		MTKFLASH_ERRCODE(0x0008)
#define E_FLASH_DUAL_CALI_GOLD_WB_VALUE_ERR		MTKFLASH_ERRCODE(0x0009)
#define E_FLASH_DUAL_CALI_OUT_Y_RANGE_ERR		MTKFLASH_ERRCODE(0x0010)
#define E_FLASH_DUAL_CALI_OUT_Y_VALUE_ERR		MTKFLASH_ERRCODE(0x0011)
#define E_FLASH_DUAL_CALI_OUT_WB_RANGE_ERR		MTKFLASH_ERRCODE(0x0012)
#define E_FLASH_DUAL_CALI_OUT_WB_VALUE_ERR		MTKFLASH_ERRCODE(0x0013)

//============================ mgr error
#define E_FLASH_CALI_ENVIRONMENT_NOT_DARK_ERR		MTKFLASH_ERRCODE(0x0080)
#define E_FLASH_CALI_OVEREXPOSURE_ERR		MTKFLASH_ERRCODE(0x0081)

#define E_FLASH_ERR								MTKFLASH_ERRCODE(0x0100)

#endif //_FLASH_ERROR_CODE_H_

