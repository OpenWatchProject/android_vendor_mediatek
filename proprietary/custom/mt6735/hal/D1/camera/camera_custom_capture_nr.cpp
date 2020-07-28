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

#include "camera_custom_capture_nr.h"
#include <mtkcam/Modes.h>
#include <mtkcam/hal/IHalSensor.h>
#include <camera_custom_nvram.h>
#include "isp_tuning/isp_tuning_custom_swnr.h"

using namespace NSCam;

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        MBOOL const isMfll,
        int* hwth,
        int* swth
        )
{
    if( sensorDev == SENSOR_DEV_MAIN   ||
        sensorDev == SENSOR_DEV_SUB    ||
        sensorDev == SENSOR_DEV_MAIN_2
            )
    {
        if( !isMfll )
        {
            switch(shotmode)
            {
                case eShotMode_NormalShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                case eShotMode_ContinuousShot:
                case eShotMode_ContinuousShotCc:
                    *hwth = DISABLE_CAPTURE_NR;
                    *swth = DISABLE_CAPTURE_NR;
                    break;
                case eShotMode_HdrShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                case eShotMode_ZsdShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                case eShotMode_FaceBeautyShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                case eShotMode_VideoSnapShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                default:
                    *hwth = DISABLE_CAPTURE_NR;
                    *swth = DISABLE_CAPTURE_NR;
                    break;
                // note: special case
                //  eShotMode_SmileShot, eShotMode_AsdShot
                //      --> NormalShot or ZsdShot
            }
        }
        else
        {
            switch(shotmode)
            {
                case eShotMode_NormalShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                case eShotMode_FaceBeautyShot:
                    *hwth = 400;
                    *swth = 800;
                    break;
                default:
                    *hwth = DISABLE_CAPTURE_NR;
                    *swth = DISABLE_CAPTURE_NR;
                    break;
                // note: special case
                //  eShotMode_SmileShot, eShotMode_AsdShot
                //      --> NormalShot or ZsdShot
            }
        }
    }
    else
    {
        *hwth = DISABLE_CAPTURE_NR;
        *swth = DISABLE_CAPTURE_NR;
    }

    return MTRUE;
}


// return value: performance 2 > 1 > 0, -1: default
MINT32 get_performance_level(
        MUINT32 const /*sensorDev*/,
        MUINT32 const shotmode,
        MBOOL const /*isMfll*/,
        MBOOL const isMultiOpen
        )
{
    // if is PIP...
    if( isMultiOpen )
        return -1;

    switch(shotmode)
    {
        case eShotMode_NormalShot:
            return -1;
            break;
        case eShotMode_ContinuousShot:
        case eShotMode_ContinuousShotCc:
            return -1;
            break;
        case eShotMode_HdrShot:
            return -1;
            break;
        case eShotMode_ZsdShot:
            return -1;
            break;
        case eShotMode_FaceBeautyShot:
        case eShotMode_FaceBeautyShotCc:
            return -1;
            break;
        case eShotMode_VideoSnapShot:
            return -1;
            break;
        default:
            return -1;
            break;
            // note: special case
            //  eShotMode_SmileShot, eShotMode_AsdShot
            //      --> NormalShot or ZsdShot
    }
    return -1;
}

static const MUINT32 tab_ISO_value_to_index[] =
{
    100,    200,    400,    800,    1200,   1600,   2000,   2400,   2800,   3200
};

MUINT32
map_ISO_value_to_index(MUINT32 u4Iso)
{
    MUINT32 i;
    for (i=0; i<sizeof(tab_ISO_value_to_index)/sizeof(tab_ISO_value_to_index[0]); i++) {
        if (u4Iso < tab_ISO_value_to_index[i]) {
            return i>0 ? (i-1) : i; //lower bound
        }
    }
    return sizeof(tab_ISO_value_to_index)/sizeof(tab_ISO_value_to_index[0])-1;
}


MUINT32
map_index_to_ISO_value(MUINT32 u4Idx)
{
    if (u4Idx >= sizeof(tab_ISO_value_to_index)/sizeof(tab_ISO_value_to_index[0]))
        return (MUINT32)-1;
    return tab_ISO_value_to_index[u4Idx];
}

MBOOL
is_to_invoke_swnr_interpolation(MBOOL /*isMfll*/, MUINT32 /*u4Iso*/)
{
    return MTRUE;
}

MINT32 get_swnr_type(MUINT32 sensorDev)
{
    switch ( sensorDev )
    {
        case SENSOR_DEV_MAIN:
        case SENSOR_DEV_SUB:
        case SENSOR_DEV_MAIN_2:
        default:
            return eSWNRType_SW;
            //return eSWNRType_SW2;
    }
}
