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

#include <mtkcam/def/BuiltinTypes.h>
#include "camera_custom_capture_nr.h"
#include <mtkcam/def/Modes.h>
#include <mtkcam/drv/IHalSensor.h>
#include <camera_custom_nvram.h>
#include "isp_tuning/isp_tuning_custom_swnr.h"
#include <log/log.h>

using namespace NSCam;

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        MBOOL const isMfll,
        int* hwth,
        int* swth
        )
{
#if 1
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
                    *swth = 400;
                    break;
                case eShotMode_ContinuousShot:
                case eShotMode_ContinuousShotCc:
                    *hwth = DISABLE_CAPTURE_NR;
                    *swth = DISABLE_CAPTURE_NR;
                    break;
                case eShotMode_HdrShot:
                    *hwth = 400;
                    *swth = 400;
                    break;
                case eShotMode_ZsdShot:
                    *hwth = 400;
                    *swth = 400;
                    break;
                case eShotMode_FaceBeautyShot:
                    *hwth = 400;
                    *swth = 400;
                    break;
                case eShotMode_VideoSnapShot:
                    *hwth = 400;
                    *swth = 400;
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
                    *swth = 400;
                    break;
                case eShotMode_FaceBeautyShot:
                    *hwth = 400;
                    *swth = 400;
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
#endif
    {
        *hwth = DISABLE_CAPTURE_NR;
        *swth = DISABLE_CAPTURE_NR;
    }

    return MTRUE;
}


// return value: performance 2 > 1 > 0, -1: default
MINT32 get_performance_level(
        MUINT32 const /*sensorDev*/,
        MUINT32 const /*shotmode*/,
        MBOOL const /*isMfll*/,
        MBOOL const /*isMultiOpen*/
        )
{
    return eSWNRPerf_Default;
}


MBOOL
is_to_invoke_swnr_interpolation(MUINT32 scenario, MUINT32 /*u4Iso*/)
{
    if (scenario == *(MUINT32*)"MFNR") {
        return MTRUE;
    } else if (scenario == *(MUINT32*)"DUAL") {
        return MTRUE;
    } else if (scenario == *(MUINT32*)"NORM") {
        return MTRUE;
    } else {
        ALOGW("Undefined scenario!!");
        return MTRUE;
    }
}


MINT32 get_swnr_type(MUINT32 const sensorDev)
{
    switch ( sensorDev )
    {
        case SENSOR_DEV_MAIN:
        case SENSOR_DEV_SUB:
        case SENSOR_DEV_MAIN_2:
        default:
            return eSWNRType_SW2_VPU;
            //return eSWNRType_SW;
    }
    return eSWNRType_NUM;
}
