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
        Capture_NR_Th_t* pTh
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
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_ContinuousShot:
                case eShotMode_ContinuousShotCc:
                    pTh->hwth = DISABLE_CAPTURE_NR;
                    pTh->swth = DISABLE_CAPTURE_NR;
                    break;
                case eShotMode_HdrShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_ZsdShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_FaceBeautyShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_VideoSnapShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                default:
                    pTh->hwth = DISABLE_CAPTURE_NR;
                    pTh->swth = DISABLE_CAPTURE_NR;
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
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                case eShotMode_FaceBeautyShot:
                    pTh->hwth = 400;
                    pTh->swth = 800;
                    break;
                default:
                    pTh->hwth = DISABLE_CAPTURE_NR;
                    pTh->swth = DISABLE_CAPTURE_NR;
                    break;
                // note: special case
                //  eShotMode_SmileShot, eShotMode_AsdShot
                //      --> NormalShot or ZsdShot
            }
        }
    }
    else
    {
        pTh->hwth = DISABLE_CAPTURE_NR;
        pTh->swth = DISABLE_CAPTURE_NR;
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


EIdxSwNR
map_ISO_value_to_index(MUINT32 const u4Iso, MBOOL const isMfll, MUINT32 method)
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if( method == 0 ) //nearest
    {
        if ( u4Iso < 150 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_100 : eIDX_SWNR_SINGLE_ISO_100;
        }
        else if ( u4Iso < 300 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_200 : eIDX_SWNR_SINGLE_ISO_200;
        }
        else if ( u4Iso < 600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_400 : eIDX_SWNR_SINGLE_ISO_400;
        }
        else if ( u4Iso < 1000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_800 : eIDX_SWNR_SINGLE_ISO_800;
        }
        else if ( u4Iso < 1400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1200 : eIDX_SWNR_SINGLE_ISO_1200;
        }
        else if ( u4Iso < 1800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1600 : eIDX_SWNR_SINGLE_ISO_1600;
        }
        else if ( u4Iso < 2200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2000 : eIDX_SWNR_SINGLE_ISO_2000;
        }
        else if ( u4Iso < 2600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2400 : eIDX_SWNR_SINGLE_ISO_2400;
        }
        else if ( u4Iso < 3000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2800 : eIDX_SWNR_SINGLE_ISO_2800;
        }
        return  isMfll ? eIDX_SWNR_MFLL_ISO_3200 : eIDX_SWNR_SINGLE_ISO_3200;
    }
    else if( method == 1 ) //lower bound
    {
        if ( u4Iso < 200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_100 : eIDX_SWNR_SINGLE_ISO_100;
        }
        else if ( u4Iso < 400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_200 : eIDX_SWNR_SINGLE_ISO_200;
        }
        else if ( u4Iso < 800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_400 : eIDX_SWNR_SINGLE_ISO_400;
        }
        else if ( u4Iso < 1200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_800 : eIDX_SWNR_SINGLE_ISO_800;
        }
        else if ( u4Iso < 1600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1200 : eIDX_SWNR_SINGLE_ISO_1200;
        }
        else if ( u4Iso < 2000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1600 : eIDX_SWNR_SINGLE_ISO_1600;
        }
        else if ( u4Iso < 2400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2000 : eIDX_SWNR_SINGLE_ISO_2000;
        }
        else if ( u4Iso < 2800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2400 : eIDX_SWNR_SINGLE_ISO_2400;
        }
        else if ( u4Iso < 3200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2800 : eIDX_SWNR_SINGLE_ISO_2800;
        }
        return isMfll ? eIDX_SWNR_MFLL_ISO_3200 : eIDX_SWNR_SINGLE_ISO_3200;
    }
    else if( method == 2 ) //upper bound
    {
        if ( u4Iso <= 100 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_100 : eIDX_SWNR_SINGLE_ISO_100;
        }
        else if ( u4Iso <= 200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_200 : eIDX_SWNR_SINGLE_ISO_200;
        }
        else if ( u4Iso <= 400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_400 : eIDX_SWNR_SINGLE_ISO_400;
        }
        else if ( u4Iso <= 800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_800 : eIDX_SWNR_SINGLE_ISO_800;
        }
        else if ( u4Iso <= 1200 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1200 : eIDX_SWNR_SINGLE_ISO_1200;
        }
        else if ( u4Iso <= 1600 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_1600 : eIDX_SWNR_SINGLE_ISO_1600;
        }
        else if ( u4Iso <= 2000 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2000 : eIDX_SWNR_SINGLE_ISO_2000;
        }
        else if ( u4Iso <= 2400 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2400 : eIDX_SWNR_SINGLE_ISO_2400;
        }
        else if ( u4Iso <= 2800 )
        {
            return  isMfll ? eIDX_SWNR_MFLL_ISO_2800 : eIDX_SWNR_SINGLE_ISO_2800;
        }
        return isMfll ? eIDX_SWNR_MFLL_ISO_3200 : eIDX_SWNR_SINGLE_ISO_3200;
    }
    // error
    return eNUM_OF_SWNR_IDX;
}


MUINT32
map_index_to_ISO_value(EIdxSwNR const u4Idx)
{
    switch(u4Idx)
    {
        case eIDX_SWNR_SINGLE_ISO_100:
            return 100;
        case eIDX_SWNR_SINGLE_ISO_200:
            return 200;
        case eIDX_SWNR_SINGLE_ISO_400:
            return 400;
        case eIDX_SWNR_SINGLE_ISO_800:
            return 800;
        case eIDX_SWNR_SINGLE_ISO_1200:
            return 1200;
        case eIDX_SWNR_SINGLE_ISO_1600:
            return 1600;
        case eIDX_SWNR_SINGLE_ISO_2000:
            return 2000;
        case eIDX_SWNR_SINGLE_ISO_2400:
            return 2400;
        case eIDX_SWNR_SINGLE_ISO_2800:
            return 2800;
        case eIDX_SWNR_SINGLE_ISO_3200:
            return 3200;
        case eIDX_SWNR_SINGLE_ISO_ANR_ENC_OFF:
        case eIDX_SWNR_SINGLE_ISO_ANR_ENC_ON:
            //TODO: interpolation
            return -1;
        case eIDX_SWNR_MFLL_ISO_100:
            return 100;
        case eIDX_SWNR_MFLL_ISO_200:
            return 200;
        case eIDX_SWNR_MFLL_ISO_400:
            return 400;
        case eIDX_SWNR_MFLL_ISO_800:
            return 800;
        case eIDX_SWNR_MFLL_ISO_1200:
            return 1200;
        case eIDX_SWNR_MFLL_ISO_1600:
            return 1600;
        case eIDX_SWNR_MFLL_ISO_2000:
            return 2000;
        case eIDX_SWNR_MFLL_ISO_2400:
            return 2400;
        case eIDX_SWNR_MFLL_ISO_2800:
            return 2800;
        case eIDX_SWNR_MFLL_ISO_3200:
            return 3200;
        case eIDX_SWNR_MFLL_ISO_ANR_ENC_OFF:
        case eIDX_SWNR_MFLL_ISO_ANR_ENC_ON:
            //TODO: interpolation
            return -1;
        default:
            break;
    }
    //MY_LOGE("cannot map idx %d to iso", u4Idx);
    return -1;
}

MBOOL
is_to_invoke_swnr_interpolation(MBOOL const /*isMfll*/, MUINT32 const /*u4Iso*/)
{
    return MTRUE;
}

MINT32 get_swnr_type()
{
    return eSWNRType_Default;
    //return eSWNRType_IIR;
}
