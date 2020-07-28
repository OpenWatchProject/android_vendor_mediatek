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
#ifndef _ISP_TUNING_H_
#define _ISP_TUNING_H_

#include "isp_tuning_sensor.h"
#include "tuning_mapping/cam_idx_struct_ext.h"

namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
typedef enum MERROR_ENUM
{
    MERR_OK         = 0,
    MERR_UNKNOWN    = 0x80000000, // Unknown error
    MERR_UNSUPPORT,
    MERR_BAD_PARAM,
    MERR_BAD_CTRL_CODE,
    MERR_BAD_FORMAT,
    MERR_BAD_ISP_DRV,
    MERR_BAD_NVRAM_DRV,
    MERR_BAD_SENSOR_DRV,
    MERR_BAD_SYSRAM_DRV,
    MERR_SET_ISP_REG,
    MERR_NO_MEM,
    MERR_NO_SYSRAM_MEM,
    MERR_NO_RESOURCE,
    MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND,
    MERR_CUSTOM_NOT_READY,
    MERR_PREPARE_HW,
    MERR_APPLY_TO_HW,
    MERR_CUSTOM_ISO_ENV_ERR,
    MERR_CUSTOM_CT_ENV_ERR
} MERROR_ENUM_T;

/*******************************************************************************
* Operation Mode
*******************************************************************************/
typedef enum
{
    EOperMode_Normal    = 0,
    EOperMode_PureRaw,
    EOperMode_Meta,
    EOperMode_EM,
    EOperMpde_Factory
} EOperMode_T;


typedef enum
{
    ESensorTG_None = 0,
    ESensorTG_1,
    ESensorTG_2,
    ESensorTG_3
}   ESensorTG_T;

typedef enum
{
    ERawType_Proc = 0,
    ERawType_Pure = 1
}   ERawType_T;

typedef enum
{
    ERaw2Yuv      = 0,
    EYuv2Yuv      = 1,
    ERaw2Yuv_MONO = 5  //without CamInfo
}   EP2IN_FMT_T;

typedef enum
{
    ENormalUpdate      = 0,
    EPartKeep          = 1,
    EAllKeep           = 2,
    ELPCNR_8Bit_Pass1  = 3,
    ELPCNR_8Bit_Pass2  = 4,
    EContinuousShots   = 5,
    ELPCNR_10Bit_Pass1 = 6,
    ELPCNR_10Bit_Pass2 = 7,
    EIdenditySetting   = 8,
    EDSDN_setting      = 9,
}   EP2UPDATE_MODE;


};  //  NSIspTuning

#endif //  _ISP_TUNING_H_

