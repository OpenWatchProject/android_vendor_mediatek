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
#define LOG_TAG "ae_mgr_if"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <ae_tuning_custom.h>
//#include <isp_mgr.h>
#include <isp_tuning.h>
//#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <kd_camera_feature.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <ae_calc_if.h>

using namespace NS3Av3;
using namespace NSIspTuning;

static  IAeMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAeMgr&
IAeMgr::
getInstance()
{
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::cameraPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).cameraPreviewInit(i4SensorIdx, rParam);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::Start(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).Start();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).Start();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).Start();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).Start();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::uninit(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).uninit();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).uninit();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::Stop(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).Stop();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).Stop();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).Stop();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).Stop();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMeteringArea(sNewAEMeteringArea);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEEVCompIndex(i4NewEVIndex, fStep);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringMode(MINT32 i4SensorDev, MUINT32 u4NewAEMeteringMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMeteringMode(u4NewAEMeteringMode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEISOSpeed(MINT32 i4SensorDev, MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEISOSpeed(u4NewAEISOSpeed);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMinMaxFrameRate(MINT32 i4SensorDev, MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEFLKMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEFlickerMode(u4NewAEFLKMode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setFlickerFrameRateActive(a_bFlickerFPSAvtive);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAECamMode(MINT32 i4SensorDev, MUINT32 u4NewAECamMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAECamMode(u4NewAECamMode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEManualPline(MINT32 i4SensorDev, EAEManualPline_T eAEManualPline, MBOOL bEnable)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).enableAEManualPline(eAEManualPline, bEnable);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).enableAEManualPline(eAEManualPline, bEnable);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).enableAEManualPline(eAEManualPline, bEnable);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).enableAEManualPline(eAEManualPline, bEnable);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEManualCapPline(MINT32 i4SensorDev, EAEManualPline_T eAEManualCapPline, MBOOL bEnable)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).enableAEManualCapPline(eAEManualCapPline, bEnable);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).enableAEManualCapPline(eAEManualCapPline, bEnable);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).enableAEManualCapPline(eAEManualCapPline, bEnable);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).enableAEManualCapPline(eAEManualCapPline, bEnable);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEShotMode(MINT32 i4SensorDev, MUINT32 u4NewAEShotMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEShotMode(u4NewAEShotMode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEHDRMode(MINT32 i4SensorDev, MUINT32 u4AEHDRmode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEHDRMode(u4AEHDRmode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEHDROnOff(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEHDROnOff();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEHDROnOff();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEHDROnOff();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEHDROnOff();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSceneMode(MINT32 i4SensorDev, MUINT32 u4NewScene)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setSceneMode(u4NewScene);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEScene(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEScene();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEScene();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEScene();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEScene();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMode(MINT32 i4SensorDev, MUINT32 u4NewAEmode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setAEMode(u4NewAEmode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMode();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEState();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEState();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEState();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAPAELock(MINT32 i4SensorDev, MBOOL bAPAELock)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAPAELock(bAPAELock);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAeMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AeMgr::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AeMgr::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AeMgr::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);
    }
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setTGInfo(i4TGInfo);

    CAM_LOGE("Err IAeMgr::setTGInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPreCapAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doCapAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doCapAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doCapAE();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doCapAE();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPostCapAE(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPostCapAE(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPostCapAE(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPostCapAE(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doPostCapAE(pAEStatBuf, bIsStrobe);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getCaptureLVvalue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getCaptureLVvalue();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getCaptureLVvalue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getCaptureLVvalue();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getCaptureLVvalue();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAENvramData(MINT32 i4SensorDev, AE_NVRAM_T &rAENVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAENvramData(rAENVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAENvramData(rAENVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAENvramData(rAENVRAM);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAENvramData(rAENVRAM);

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getNvramData(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getNvramData(i4SensorDev);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getNvramData(i4SensorDev);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getNvramData(i4SensorDev);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getNvramData(i4SensorDev);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsStrobeBVTrigger(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsStrobeBVTrigger();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsStrobeBVTrigger();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsStrobeBVTrigger();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsStrobeBVTrigger();

    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringYvalue(MINT32 i4SensorDev, CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMeteringYvalue(rWinSize, uYvalue);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringBlockAreaValue(MINT32 i4SensorDev, CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getHDRCapInfo(MINT32 i4SensorDev, Hal3A_HDROutputParam_T & strHDROutputInfo)
{
    (void)i4SensorDev;
    (void)strHDROutputInfo;
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::getAECondition(MINT32 i4SensorDev, MUINT32 i4AECondition)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAECondition(i4AECondition);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAECondition(i4AECondition);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAECondition(i4AECondition);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAECondition(i4AECondition);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getLCEPlineInfo(MINT32 i4SensorDev, LCEInfo_T &a_rLCEInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getLCEPlineInfo(a_rLCEInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getLCEPlineInfo(a_rLCEInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getLCEPlineInfo(a_rLCEInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getLCEPlineInfo(a_rLCEInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAERotateDegree(MINT32 i4SensorDev, MINT32 i4RotateDegree)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAERotateDegree(i4RotateDegree);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 IAeMgr::getAEFaceDiffIndex(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEFaceDiffIndex();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEFaceDiffIndex();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEFaceDiffIndex();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEFaceDiffIndex();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorDelayInfo(MINT32 i4SensorDev, MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getBrightnessValue(MINT32 i4SensorDev, MBOOL * bFrameUpdate, MINT32* i4Yvalue)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getBrightnessValue(bFrameUpdate, i4Yvalue);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getBrightnessValue(bFrameUpdate, i4Yvalue);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getBrightnessValue(bFrameUpdate, i4Yvalue);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getBrightnessValue(bFrameUpdate, i4Yvalue);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEBlockYvalues(MINT32 i4SensorDev, MUINT8 *pYvalues, MUINT8 size)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEBlockYvalues(pYvalues, size);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEBlockYvalues(pYvalues, size);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEBlockYvalues(pYvalues, size);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEBlockYvalues(pYvalues, size);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setVideoDynamicFrameRate(MINT32 i4SensorDev, MBOOL bVdoDynamicFps)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setVideoDynamicFrameRate(bVdoDynamicFps);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setVideoDynamicFrameRate(bVdoDynamicFps);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setVideoDynamicFrameRate(bVdoDynamicFps);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setVideoDynamicFrameRate(bVdoDynamicFps);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getTgIntAEInfo(MINT32 i4SensorDev, MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getTgIntAEInfo(bTgIntAEEn, fTgIntAERatio);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getTgIntAEInfo(bTgIntAEEn, fTgIntAERatio);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getTgIntAEInfo(bTgIntAEEn, fTgIntAERatio);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getTgIntAEInfo(bTgIntAEEn, fTgIntAERatio);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAESensorActiveCycle(MINT32 i4SensorDev, MINT32* i4ActiveCycle)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAESensorActiveCycle(i4ActiveCycle);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAESensorActiveCycle(i4ActiveCycle);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAESensorActiveCycle(i4ActiveCycle);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAESensorActiveCycle(i4ActiveCycle);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::IsNeedUpdateSensor(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsNeedUpdateSensor();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsNeedUpdateSensor();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsNeedUpdateSensor();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsNeedUpdateSensor();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::bBlackLevelLock(MINT32 i4SensorDev, MBOOL bLockBlackLevel)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).bBlackLevelLock(bLockBlackLevel);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).bBlackLevelLock(bLockBlackLevel);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).bBlackLevelLock(bLockBlackLevel);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).bBlackLevelLock(bLockBlackLevel);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getSensorParams(MINT32 i4SensorDev, AE_SENSOR_PARAM_T &a_rSensorInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getSensorParams(a_rSensorInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getSensorParams(a_rSensorInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getSensorParams(a_rSensorInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getSensorParams(a_rSensorInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEConfigParams(MINT32 i4SensorDev, MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEConfigParams(bHDRen, bOverCnten, bTSFen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEConfigParams(bHDRen, bOverCnten, bTSFen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEConfigParams(bHDRen, bOverCnten, bTSFen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEConfigParams(bHDRen, bOverCnten, bTSFen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAAOSize(MINT32 i4SensorDev, MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).getAAOSize(u4BlockNumW, u4BlockNumH);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).getAAOSize(u4BlockNumW, u4BlockNumH);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getAAOSize(u4BlockNumW, u4BlockNumH);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getAAOSize(u4BlockNumW, u4BlockNumH);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEEnable(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEEnable();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEDisable(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEDisable();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::CCTOPAEGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAE, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetAEScene(MINT32 i4SensorDev, MINT32 a_AEScene)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetSceneMode(a_AEScene);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetAEScene(MINT32 i4SensorDev, MINT32 *a_pAEScene, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return  AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetMeteringMode(MINT32 i4SensorDev, MINT32 a_AEMeteringMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetMeteringMode(a_AEMeteringMode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEApplyExpParam(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetFlickerMode(MINT32 i4SensorDev, MINT32 a_AEFlickerMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetFlickerMode(a_AEFlickerMode);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetFlickerMode(MINT32 i4SensorDev, MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return  AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetMeteringMode(MINT32 i4SensorDev, MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return  AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 u4AENVRAMIdx)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4AENVRAMIdx);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4AENVRAMIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4AENVRAMIdx);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4AENVRAMIdx);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return  AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESaveNVRAMParam(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESaveNVRAMParam();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetCurrentEV(MINT32 i4SensorDev, MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAELockExpSetting(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAELockExpSetting();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEUnLockExpSetting(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEUnLockExpSetting();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOSetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOSetCaptureParams(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOGetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOGetCaptureParams(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPSetAETargetValue(MINT32 i4SensorDev, MUINT32 u4AETargetValue)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPSetAETargetValue(u4AETargetValue);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAESGG1Gain(MINT32 i4SensorDev, MUINT32 *pSGG1Gain)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAESGG1Gain(pSGG1Gain);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAESGG1Gain(pSGG1Gain);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAESGG1Gain(pSGG1Gain);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAESGG1Gain(pSGG1Gain);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEPlineTable(MINT32 i4SensorDev, eAETableID eTableID, strAETable &a_AEPlineTable)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEPlineTable(eTableID, a_AEPlineTable);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEPlineTable(eTableID, a_AEPlineTable);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEPlineTable(eTableID, a_AEPlineTable);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEPlineTable(eTableID, a_AEPlineTable);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESavePlineNVRAM(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESavePlineNVRAM();

    return (ret_main | ret_sub | ret_main2 |ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEAutoFlickerState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEAutoFlickerState();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEAutoFlickerState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEAutoFlickerState();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEAutoFlickerState();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::IsMultiCapture(MINT32 i4SensorDev, MBOOL bMultiCap)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsMultiCapture(bMultiCap);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsMultiCapture(bMultiCap);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsMultiCapture(bMultiCap);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsMultiCapture(bMultiCap);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableStereoDenoiseRatio(MINT32 i4SensorDev, MUINT32 u4enableStereoDenoise)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableStereoDenoiseRatio(u4enableStereoDenoise);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableStereoDenoiseRatio(u4enableStereoDenoise);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableStereoDenoiseRatio(u4enableStereoDenoise);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableStereoDenoiseRatio(u4enableStereoDenoise);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::isLVChangeTooMuch(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).isLVChangeTooMuch();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).isLVChangeTooMuch();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).isLVChangeTooMuch();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).isLVChangeTooMuch();

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAE2AFInfo(MINT32 i4SensorDev, CCU_AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getAE2AFInfo(rAeWinSize, rAEInfo);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::reconfig(MINT32 i4SensorDev, MVOID *pDBinInfo, MVOID *pOutRegCfg)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).reconfig(pDBinInfo, pOutRegCfg);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).reconfig(pDBinInfo, pOutRegCfg);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).reconfig(pDBinInfo, pOutRegCfg);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).reconfig(pDBinInfo, pOutRegCfg);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setDigZoomRatio(MINT32 i4SensorDev, MINT32 i4ZoomRatio)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setDigZoomRatio(i4ZoomRatio);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::resetMvhdrRatio(MINT32 i4SensorDev, MBOOL bReset)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).resetMvhdrRatio(bReset);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).resetMvhdrRatio(bReset);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).resetMvhdrRatio(bReset);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).resetMvhdrRatio(bReset);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getISOThresStatus(MINT32 i4SensorDev, MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).getISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).getISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::resetGetISOThresStatus(MINT32 i4SensorDev, MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).resetGetISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).resetGetISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).resetGetISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).resetGetISOThresStatus(a_ISOIdx1Status, a_ISOIdx2Status);

    return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsCCUAEInit(MINT32 i4SensorDev,MBOOL bInit)
{
 MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
   ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

   if (i4SensorDev & ESensorDev_Main)
       ret_main = AeMgr::getInstance(ESensorDev_Main).IsCCUAEInit(bInit);
   if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AeMgr::getInstance(ESensorDev_Sub).IsCCUAEInit(bInit);
   if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).IsCCUAEInit(bInit);
   if (i4SensorDev & ESensorDev_SubSecond)
       ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).IsCCUAEInit(bInit);

   return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setRequestNum(MINT32 i4SensorDev, MUINT32 u4ReqNum)
{
  MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
  ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

   if (i4SensorDev & ESensorDev_Main)
       ret_main = AeMgr::getInstance(ESensorDev_Main).setRequestNum(u4ReqNum);
   if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AeMgr::getInstance(ESensorDev_Sub).setRequestNum(u4ReqNum);
   if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setRequestNum(u4ReqNum);
   if (i4SensorDev & ESensorDev_SubSecond)
       ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setRequestNum(u4ReqNum);

   return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEInfo(MINT32 i4SensorDev, AE_PERFRAME_INFO_T& rAEPerframeInfo)
{
   MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
   ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

   if (i4SensorDev & ESensorDev_Main)
       ret_main = AeMgr::getInstance(ESensorDev_Main).getAEInfo(rAEPerframeInfo);
   if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AeMgr::getInstance(ESensorDev_Sub).getAEInfo(rAEPerframeInfo);
   if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getAEInfo(rAEPerframeInfo);
   if (i4SensorDev & ESensorDev_SubSecond)
       ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getAEInfo(rAEPerframeInfo);

   return (ret_main | ret_sub | ret_main2 | ret_sub2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::configReg(MINT32 i4SensorDev, AEResultConfig_T *pResultConfig)
{
  MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
   ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

   if (i4SensorDev & ESensorDev_Main)
       ret_main = AeMgr::getInstance(ESensorDev_Main).configReg(pResultConfig);
   if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AeMgr::getInstance(ESensorDev_Sub).configReg(pResultConfig);
   if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).configReg(pResultConfig);
   if (i4SensorDev & ESensorDev_SubSecond)
       ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).configReg(pResultConfig);

   return (ret_main | ret_sub | ret_main2 | ret_sub2);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::sendAECtrl(MINT32 i4SensorDev, EAECtrl_T eAECtrl, MINTPTR iArg1, MINTPTR iArg2, MINTPTR iArg3, MINTPTR iArg4)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).sendAECtrl(eAECtrl, iArg1, iArg2, iArg3, iArg4);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).sendAECtrl(eAECtrl, iArg1, iArg2, iArg3, iArg4);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).sendAECtrl(eAECtrl, iArg1, iArg2, iArg3, iArg4);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).sendAECtrl(eAECtrl, iArg1, iArg2, iArg3, iArg4);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEParams(MINT32 i4SensorDev, AE_PARAM_SET_INFO const &rNewParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;

     if (i4SensorDev & ESensorDev_Main)
         ret_main = AeMgr::getInstance(ESensorDev_Main).setAEParams(rNewParam);
     if (i4SensorDev & ESensorDev_Sub)
         ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEParams(rNewParam);
     if (i4SensorDev & ESensorDev_MainSecond)
         ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEParams(rNewParam);
     if (i4SensorDev & ESensorDev_SubSecond)
         ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEParams(rNewParam);

     return (ret_main | ret_sub | ret_main2 | ret_sub2);

}
