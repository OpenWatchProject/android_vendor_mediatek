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
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <kd_camera_feature.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <ae_algo_if.h>

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
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).cameraPreviewInit(i4SensorIdx, rParam);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::Start(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).Start();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).Start();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).Start();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).Start();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).Start();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::uninit(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).uninit();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).uninit();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).uninit();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::Stop(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).Stop();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).Stop();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).Stop();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).Stop();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).Stop();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setAeMeterAreaEn(MINT32 i4SensorDev, int en)
{
    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setAeMeterAreaEn(en);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setAeMeterAreaEn(en);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setAeMeterAreaEn(en);
    if (i4SensorDev & ESensorDev_SubSecond)
        AeMgr::getInstance(ESensorDev_SubSecond).setAeMeterAreaEn(en);
    if (i4SensorDev & ESensorDev_MainThird)
        AeMgr::getInstance(ESensorDev_MainThird).setAeMeterAreaEn(en);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEMeteringArea(sNewAEMeteringArea);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFDenable(MINT32 i4SensorDev, MBOOL bFDenable)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setFDenable(bFDenable);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEFDMeteringAreaInfo(MINT32 i4SensorDev, android::Vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).getAEFDMeteringAreaInfo(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).getAEFDMeteringAreaInfo(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getAEFDMeteringAreaInfo(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getAEFDMeteringAreaInfo(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).getAEFDMeteringAreaInfo(vecOut, i4tgwidth, i4tgheight);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setOTInfo(MINT32 i4SensorDev, MVOID* a_sOT)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setOTInfo(a_sOT);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setOTInfo(a_sOT);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setOTInfo(a_sOT);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setOTInfo(a_sOT);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setOTInfo(a_sOT);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEEVCompIndex(i4NewEVIndex, fStep);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getEVCompensateIndex(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getEVCompensateIndex();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getEVCompensateIndex();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getEVCompensateIndex();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getEVCompensateIndex();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getEVCompensateIndex();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringMode(MINT32 i4SensorDev, MUINT32 u4NewAEMeteringMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEMeteringMode(u4NewAEMeteringMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEMeterMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeterMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeterMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeterMode();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMeterMode();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEMeterMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEISOSpeed(MINT32 i4SensorDev, MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEISOSpeed(u4NewAEISOSpeed);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEISOSpeedMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEISOSpeedMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEISOSpeedMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEISOSpeedMode();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEISOSpeedMode();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEISOSpeedMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMinMaxFrameRate(MINT32 i4SensorDev, MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEFLKMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEFlickerMode(u4NewAEFLKMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEAutoFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEAutoFLKMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEAutoFlickerMode(u4NewAEAutoFLKMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setFlickerFrameRateActive(a_bFlickerFPSAvtive);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAECamMode(MINT32 i4SensorDev, MUINT32 u4NewAECamMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAECamMode(u4NewAECamMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAESMBuffermode(MINT32 i4SensorDev, MBOOL bSMBuffmode, MINT32 i4subframeCnt)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAESMBuffermode(bSMBuffmode, i4subframeCnt);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAESMBuffermode(bSMBuffmode, i4subframeCnt);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAESMBuffermode(bSMBuffmode, i4subframeCnt);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAESMBuffermode(bSMBuffmode, i4subframeCnt);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAESMBuffermode(bSMBuffmode, i4subframeCnt);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEShotMode(MINT32 i4SensorDev, MUINT32 u4NewAEShotMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEShotMode(u4NewAEShotMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setCamScenarioMode(MINT32 i4SensorDev, MUINT32 u4CamScenarioMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setCamScenarioMode(u4CamScenarioMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEHDRMode(MINT32 i4SensorDev, MUINT32 u4AEHDRmode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEHDRMode(u4AEHDRmode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEHDRMode(u4AEHDRmode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEHDROnOff();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAELimiterMode(MINT32 i4SensorDev, MBOOL bAELimter)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAELimiterMode(bAELimter);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAELimiterMode(bAELimter);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAELimiterMode(bAELimter);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAELimiterMode(bAELimter);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAELimiterMode(bAELimter);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSceneMode(MINT32 i4SensorDev, MUINT32 u4NewScene)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setSceneMode(u4NewScene);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEScene();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMode(MINT32 i4SensorDev, MUINT32 u4NewAEmode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAEMode(u4NewAEmode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEMode();

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEState();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MUINT32 width, MUINT32 height)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setSensorMode(i4NewSensorMode, width,  height);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setSensorMode(i4NewSensorMode, width,  height);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setSensorMode(i4NewSensorMode, width, height);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setSensorMode(i4NewSensorMode, width, height);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setSensorMode(i4NewSensorMode, width, height);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEScenarioMode(MINT32 i4SensorDev, EIspProfile_T eIspProfile)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateAEScenarioMode(eIspProfile);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateAEScenarioMode(eIspProfile);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateAEScenarioMode(eIspProfile);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).updateAEScenarioMode(eIspProfile);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).updateAEScenarioMode(eIspProfile);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getSensorMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getSensorMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getSensorMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getSensorMode();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getSensorMode();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getSensorMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAPAELock(MINT32 i4SensorDev, MBOOL bAPAELock)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAPAELock(bAPAELock);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAFAELock(MINT32 i4SensorDev, MBOOL bAFAELock)
{
	MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAFAELock(bAFAELock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAFAELock(bAFAELock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAFAELock(bAFAELock);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAFAELock(bAFAELock);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAFAELock(bAFAELock);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 IAeMgr::getAEMaxMeterAreaNum(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMaxMeterAreaNum();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMaxMeterAreaNum();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMaxMeterAreaNum();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMaxMeterAreaNum();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEMaxMeterAreaNum();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).enableAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).enableAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).enableAE();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).enableAE();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).enableAE();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAeMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setTGInfo(i4TGInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).setTGInfo(i4TGInfo);

    CAM_LOGE("Err IAeMgr::setTGInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::disableAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).disableAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).disableAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).disableAE();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).disableAE();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).disableAE();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doPvAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*MRESULT IAeMgr::doPvAE_TG_INT(MINT32 i4SensorDev, MVOID *pAEStatBuf)
{
    return MFALSE;
}*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAEAAO(MINT32 i4SensorDev, MVOID *pAEStatBuf, MUINT32 u4AAOUpdate)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doAFAEAAO(pAEStatBuf, u4AAOUpdate);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doAFAEAAO(pAEStatBuf, u4AAOUpdate);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doAFAEAAO(pAEStatBuf, u4AAOUpdate);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doAFAEAAO(pAEStatBuf, u4AAOUpdate);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doAFAEAAO(pAEStatBuf, u4AAOUpdate);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doAFAE(i8TimeStamp, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPreCapAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doPreCapAE(i8TimeStamp, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doCapAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doCapAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doCapAE();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doCapAE();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doCapAE();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doBackAEInfo(MINT32 i4SensorDev, MBOOL bTorchMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doBackAEInfo(bTorchMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doBackAEInfo(bTorchMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doBackAEInfo(bTorchMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doBackAEInfo(bTorchMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doBackAEInfo(bTorchMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setRestore(MINT32 i4SensorDev, int frm)
{
    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setRestore(frm);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setRestore(frm);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setRestore(frm);
    if (i4SensorDev & ESensorDev_SubSecond)
        AeMgr::getInstance(ESensorDev_SubSecond).setRestore(frm);
    if (i4SensorDev & ESensorDev_MainThird)
        AeMgr::getInstance(ESensorDev_MainThird).setRestore(frm);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doRestoreAEInfo(MINT32 i4SensorDev, MBOOL bRestorePrvOnly, MBOOL bTorchMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doRestoreAEInfo(bRestorePrvOnly, bTorchMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doRestoreAEInfo(bRestorePrvOnly, bTorchMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doRestoreAEInfo(bRestorePrvOnly, bTorchMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doRestoreAEInfo(bRestorePrvOnly, bTorchMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doRestoreAEInfo(bRestorePrvOnly, bTorchMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapFlare(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doCapFlare(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doCapFlare(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doCapFlare(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).doCapFlare(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).doCapFlare(pAEStatBuf, bIsStrobe);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setExp(MINT32 i4SensorDev, int exp)
{
    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setExp(exp);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setExp(exp);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setExp(exp);
    if (i4SensorDev & ESensorDev_SubSecond)
        AeMgr::getInstance(ESensorDev_SubSecond).setExp(exp);
    if (i4SensorDev & ESensorDev_MainThird)
        AeMgr::getInstance(ESensorDev_MainThird).setExp(exp);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setAfe(MINT32 i4SensorDev, int afe)
{
    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setAfe(afe);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setAfe(afe);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setAfe(afe);
    if (i4SensorDev & ESensorDev_SubSecond)
        AeMgr::getInstance(ESensorDev_SubSecond).setAfe(afe);
    if (i4SensorDev & ESensorDev_MainThird)
        AeMgr::getInstance(ESensorDev_MainThird).setAfe(afe);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setIsp(MINT32 i4SensorDev, int isp)
{
    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setIsp(isp);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setIsp(isp);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setIsp(isp);
    if (i4SensorDev & ESensorDev_SubSecond)
        AeMgr::getInstance(ESensorDev_SubSecond).setIsp(isp);
    if (i4SensorDev & ESensorDev_MainThird)
        AeMgr::getInstance(ESensorDev_MainThird).setIsp(isp);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getLVvalue(isStrobeOn);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAOECompLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAOECompLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAOECompLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAOECompLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAOECompLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAOECompLVvalue(isStrobeOn);

    return MFALSE;
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getCaptureLVvalue();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getDebugInfo(MINT32 i4SensorDev, AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAENvramData(rAENVRAM);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getNvramData(i4SensorDev);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getCurrentPlineTable(MINT32 i4SensorDev, strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAECapPlineTable(MINT32 i4SensorDev, MINT32 *i4CapIndex, strAETable &a_CapAEPlineTable)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAECapPlineTable(i4CapIndex, a_CapAEPlineTable);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAECapPlineTable(i4CapIndex, a_CapAEPlineTable);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAECapPlineTable(i4CapIndex, a_CapAEPlineTable);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAECapPlineTable(i4CapIndex, a_CapAEPlineTable);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAECapPlineTable(i4CapIndex, a_CapAEPlineTable);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getSensorDeviceInfo(MINT32 i4SensorDev, AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getSensorDeviceInfo(a_rDeviceInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getSensorDeviceInfo(a_rDeviceInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getSensorDeviceInfo(a_rDeviceInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getSensorDeviceInfo(a_rDeviceInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getSensorDeviceInfo(a_rDeviceInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsDoAEInPreAF(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsDoAEInPreAF();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsDoAEInPreAF();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsDoAEInPreAF();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsDoAEInPreAF();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsDoAEInPreAF();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAEStable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsAEStable();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsAEStable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsAEStable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsAEStable();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsAEStable();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAELock(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsAELock();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsAELock();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsAELock();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsAELock();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsAELock();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAPAELock(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsAPAELock();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsAPAELock();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsAPAELock();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsAPAELock();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsAPAELock();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getBVvalue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getBVvalue();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getBVvalue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getBVvalue();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getBVvalue();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getBVvalue();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getDeltaBV(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getDeltaBV();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getDeltaBV();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getDeltaBV();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getDeltaBV();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getDeltaBV();

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsStrobeBVTrigger();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setStrobeMode(MINT32 i4SensorDev, MBOOL bIsStrobeOn)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setStrobeMode(bIsStrobeOn);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setStrobeMode(bIsStrobeOn);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setStrobeMode(bIsStrobeOn);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setStrobeMode(bIsStrobeOn);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setStrobeMode(bIsStrobeOn);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getPreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getPreviewParams(a_rPreviewInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getPreviewParams(a_rPreviewInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getPreviewParams(a_rPreviewInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getPreviewParams(a_rPreviewInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getPreviewParams(a_rPreviewInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getExposureInfo(MINT32 i4SensorDev, ExpSettingParam_T &strHDRInputSetting)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getExposureInfo(strHDRInputSetting);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getExposureInfo(strHDRInputSetting);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getExposureInfo(strHDRInputSetting);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getExposureInfo(strHDRInputSetting);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getExposureInfo(strHDRInputSetting);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getCaptureParams(a_rCaptureInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorListenerParams(MINT32 i4SensorDev, MINT32 *i4SensorInfo)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).updateSensorListenerParams(i4SensorInfo);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MRESULT IAeMgr::updateISPNvramOBCTable(MINT32 i4SensorDev, MVOID* pISPNvramOBCTable, MINT32 i4Tableidx)
// {
//     return MFALSE;
// }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEUnderExpdeltaBVIdx(MINT32 i4SensorDev, MINT32 i4AEdeltaBV)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateAEUnderExpdeltaBVIdx(i4AEdeltaBV);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateAEUnderExpdeltaBVIdx(i4AEdeltaBV);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateAEUnderExpdeltaBVIdx(i4AEdeltaBV);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).updateAEUnderExpdeltaBVIdx(i4AEdeltaBV);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).updateAEUnderExpdeltaBVIdx(i4AEdeltaBV);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updatePreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo, MBOOL bRemosaicEn)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateCaptureParams(a_rCaptureInfo, bRemosaicEn);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateCaptureParams(a_rCaptureInfo, bRemosaicEn);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateCaptureParams(a_rCaptureInfo, bRemosaicEn);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).updateCaptureParams(a_rCaptureInfo, bRemosaicEn);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).updateCaptureParams(a_rCaptureInfo, bRemosaicEn);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::prepareCapParams(MINT32 i4SensorDev, MBOOL bRemosaicEn, MBOOL bIsEVChange)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).prepareCapParams(bRemosaicEn, bIsEVChange);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).prepareCapParams(bRemosaicEn, bIsEVChange);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).prepareCapParams(bRemosaicEn, bIsEVChange);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).prepareCapParams(bRemosaicEn, bIsEVChange);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).prepareCapParams(bRemosaicEn, bIsEVChange);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringYvalue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEMeteringYvalue(rWinSize, uYvalue);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringBlockAreaValue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);

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
MRESULT IAeMgr::getRTParams(MINT32 i4SensorDev, FrameOutputParam_T &a_strFrameInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getRTParams(a_strFrameInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getRTParams(a_strFrameInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getRTParams(a_strFrameInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getRTParams(a_strFrameInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getRTParams(a_strFrameInfo);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAECondition(i4AECondition);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getLCEPlineInfo(a_rLCEInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAERotateDegree(MINT32 i4SensorDev, MINT32 i4RotateDegree)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setAERotateDegree(i4RotateDegree);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::UpdateSensorISPParams(MINT32 i4SensorDev, AE_STATE_T eNewAEState)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).UpdateSensorISPParams(eNewAEState);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).UpdateSensorISPParams(eNewAEState);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).UpdateSensorISPParams(eNewAEState);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).UpdateSensorISPParams(eNewAEState);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).UpdateSensorISPParams(eNewAEState);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEFaceDiffIndex();

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getBrightnessValue(bFrameUpdate, i4Yvalue);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEBlockYvalues(pYvalues, size);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::SetAETargetMode(MINT32 i4SensorDev, eAETargetMODE eAETargetMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).SetAETargetMode(eAETargetMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).SetAETargetMode(eAETargetMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).SetAETargetMode(eAETargetMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).SetAETargetMode(eAETargetMode);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).SetAETargetMode(eAETargetMode);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setEMVHDRratio(MINT32 i4SensorDev, MUINT32 u4ratio)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setEMVHDRratio(u4ratio);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setEMVHDRratio(u4ratio);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setEMVHDRratio(u4ratio);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setEMVHDRratio(u4ratio);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).setEMVHDRratio(u4ratio);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).setVideoDynamicFrameRate(bVdoDynamicFps);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEdelayInfo(MINT32 i4SensorDev, MINT32* i4ExpDelay, MINT32* i4AEGainDelay, MINT32* i4IspGainDelay)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEdelayInfo(i4ExpDelay, i4AEGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEdelayInfo(i4ExpDelay, i4AEGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEdelayInfo(i4ExpDelay, i4AEGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAEdelayInfo(i4ExpDelay, i4AEGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEdelayInfo(i4ExpDelay, i4AEGainDelay, i4IspGainDelay);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MRESULT IAeMgr::getTgIntAEInfo(MINT32 i4SensorDev, MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio)
// {
//     return MFALSE;
// }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getPerframeAEFlag(MINT32 i4SensorDev, MBOOL &bPerframeAE)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getPerframeAEFlag(bPerframeAE);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getPerframeAEFlag(bPerframeAE);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getPerframeAEFlag(bPerframeAE);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getPerframeAEFlag(bPerframeAE);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getPerframeAEFlag(bPerframeAE);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAESensorActiveCycle(i4ActiveCycle);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsNeedUpdateSensor();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorbyI2CBufferMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateSensorbyI2CBufferMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateSensorbyI2CBufferMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateSensorbyI2CBufferMode();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).updateSensorbyI2CBufferMode();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).updateSensorbyI2CBufferMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorbyI2C(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateSensorbyI2C();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateSensorbyI2C();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateSensorbyI2C();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).updateSensorbyI2C();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).updateSensorbyI2C();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEState2Converge(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setAEState2Converge();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setAEState2Converge();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setAEState2Converge();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setAEState2Converge();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).setAEState2Converge();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::UpdateSensorParams(MINT32 i4SensorDev, AE_SENSOR_PARAM_T strSensorParams)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).UpdateSensorParams(strSensorParams);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).UpdateSensorParams(strSensorParams);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).UpdateSensorParams(strSensorParams);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).UpdateSensorParams(strSensorParams);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).UpdateSensorParams(strSensorParams);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).bBlackLevelLock(bLockBlackLevel);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getSensorParams(a_rSensorInfo);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEConfigParams(bHDRen, bOverCnten, bTSFen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 IAeMgr::getSensorRollingShutter(MINT32 i4SensorDev) const
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getSensorRollingShutter();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getSensorRollingShutter();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getSensorRollingShutter();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getSensorRollingShutter();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getSensorRollingShutter();

    return 0L;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEBV(MINT32 i4SensorDev,MVOID *pAEStatBuf)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateAEBV(pAEStatBuf);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateAEBV(pAEStatBuf);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateAEBV(pAEStatBuf);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).updateAEBV(pAEStatBuf);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).updateAEBV(pAEStatBuf);

   return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSensorDirectly(MINT32 i4SensorDev, CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAAOLineByteSize(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAAOLineByteSize();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAAOLineByteSize();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAAOLineByteSize();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getAAOLineByteSize();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAAOLineByteSize();

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEEnable(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEEnable();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEDisable(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEDisable();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetAEScene(MINT32 i4SensorDev, MINT32 a_AEScene)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetSceneMode(a_AEScene);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return  AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetMeteringMode(MINT32 i4SensorDev, MINT32 a_AEMeteringMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetMeteringMode(a_AEMeteringMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEApplyExpParam(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetFlickerMode(MINT32 i4SensorDev, MINT32 a_AEFlickerMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetFlickerMode(a_AEFlickerMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return  AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return  AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 u4CamScenarioMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEApplyNVRAMParam(a_pAENVRAM, u4CamScenarioMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return  AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESaveNVRAMParam(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESaveNVRAMParam();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAELockExpSetting(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAELockExpSetting();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEUnLockExpSetting(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEUnLockExpSetting();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetIspOB(MINT32 i4SensorDev, MUINT32 *a_pIspOB, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetIspOB(MINT32 i4SensorDev, MUINT32 a_IspOB)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetIspOB(a_IspOB);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetIspOB(a_IspOB);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetIspOB(a_IspOB);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetIspOB(a_IspOB);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetIspOB(a_IspOB);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetIspRAWGain(MINT32 i4SensorDev, MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetIspRAWGain(MINT32 i4SensorDev, MUINT32 a_IspRAWGain)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetIspRAWGain(a_IspRAWGain);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetIspRAWGain(a_IspRAWGain);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetIspRAWGain(a_IspRAWGain);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetIspRAWGain(a_IspRAWGain);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetIspRAWGain(a_IspRAWGain);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorExpTime(MINT32 i4SensorDev, MUINT32 a_ExpTime)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSensorExpTime(a_ExpTime);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSensorExpTime(a_ExpTime);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSensorExpTime(a_ExpTime);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetSensorExpTime(a_ExpTime);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetSensorExpTime(a_ExpTime);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorExpLine(MINT32 i4SensorDev, MUINT32 a_ExpLine)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSensorExpLine(a_ExpLine);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSensorExpLine(a_ExpLine);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSensorExpLine(a_ExpLine);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetSensorExpLine(a_ExpLine);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetSensorExpLine(a_ExpLine);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorGain(MINT32 i4SensorDev, MUINT32 a_SensorGain)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSensorGain(a_SensorGain);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSensorGain(a_SensorGain);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSensorGain(a_SensorGain);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetSensorGain(a_SensorGain);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetSensorGain(a_SensorGain);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetCaptureMode(MINT32 i4SensorDev, MUINT32 a_CaptureMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetCaptureMode(a_CaptureMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetCaptureMode(a_CaptureMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetCaptureMode(a_CaptureMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESetCaptureMode(a_CaptureMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESetCaptureMode(a_CaptureMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOSetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOSetCaptureParams(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOGetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOGetCaptureParams(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetFlareOffset(MINT32 i4SensorDev, MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPSetAETargetValue(MINT32 i4SensorDev, MUINT32 u4AETargetValue)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPSetAETargetValue(u4AETargetValue);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::modifyAEPlineTableLimitation(MINT32 i4SensorDev, MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAESGG1Gain(pSGG1Gain);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEOneShotControl(MINT32 i4SensorDev, MBOOL bAEControl)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableAEOneShotControl(bAEControl);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableAEOneShotControl(bAEControl);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableAEOneShotControl(bAEControl);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableAEOneShotControl(bAEControl);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableAEOneShotControl(bAEControl);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEPlineTable(eTableID, a_AEPlineTable);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESavePlineNVRAM(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).CCTOPAESavePlineNVRAM();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::get3ACaptureDelayFrame(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).get3ACaptureDelayFrame();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).get3ACaptureDelayFrame();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).get3ACaptureDelayFrame();
    if (i4SensorDev & ESensorDev_SubSecond)
        return  AeMgr::getInstance(ESensorDev_SubSecond).get3ACaptureDelayFrame();
    if (i4SensorDev & ESensorDev_MainThird)
        return  AeMgr::getInstance(ESensorDev_MainThird).get3ACaptureDelayFrame();

    return MFALSE;
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getAEAutoFlickerState();

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsMultiCapture(bMultiCap);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::IsAEContinueShot(MINT32 i4SensorDev, MBOOL bCShot)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsAEContinueShot(bCShot);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsAEContinueShot(bCShot);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsAEContinueShot(bCShot);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).IsAEContinueShot(bCShot);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).IsAEContinueShot(bCShot);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableEISRecording(MINT32 i4SensorDev, MBOOL bEISRecording)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableEISRecording(bEISRecording);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableEISRecording(bEISRecording);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableEISRecording(bEISRecording);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableEISRecording(bEISRecording);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableEISRecording(bEISRecording);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAISManualPline(MINT32 i4SensorDev, MBOOL bAISPline)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableAISManualPline(bAISPline);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableAISManualPline(bAISPline);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableAISManualPline(bAISPline);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableAISManualPline(bAISPline);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableAISManualPline(bAISPline);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableMFHRManualPline(MINT32 i4SensorDev, MBOOL bMFHRPline)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableMFHRManualPline(bMFHRPline);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableMFHRManualPline(bMFHRPline);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableMFHRManualPline(bMFHRPline);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableMFHRManualPline(bMFHRPline);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableMFHRManualPline(bMFHRPline);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableBMDNManualPline(MINT32 i4SensorDev, MBOOL bBMDNPline)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableBMDNManualPline(bBMDNPline);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableBMDNManualPline(bBMDNPline);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableBMDNManualPline(bBMDNPline);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableBMDNManualPline(bBMDNPline);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableBMDNManualPline(bBMDNPline);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateStereoDenoiseRatio(MINT32 i4SensorDev, MINT32* i4StereoDenoiserto)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateStereoDenoiseRatio(i4StereoDenoiserto);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateStereoDenoiseRatio(i4StereoDenoiserto);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateStereoDenoiseRatio(i4StereoDenoiserto);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).updateStereoDenoiseRatio(i4StereoDenoiserto);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).updateStereoDenoiseRatio(i4StereoDenoiserto);

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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableStereoDenoiseRatio(u4enableStereoDenoise);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableFlareInManualControl(MINT32 i4SensorDev, MBOOL bIsFlareInManual)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableFlareInManualControl(bIsFlareInManual);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableFlareInManualControl(bIsFlareInManual);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableFlareInManualControl(bIsFlareInManual);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableFlareInManualControl(bIsFlareInManual);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableFlareInManualControl(bIsFlareInManual);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEStereoManualPline(MINT32 i4SensorDev, MBOOL bstereomode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableAEStereoManualPline(bstereomode);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableAEStereoManualPline(bstereomode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableAEStereoManualPline(bstereomode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableAEStereoManualPline(bstereomode);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableAEStereoManualPline(bstereomode);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setzCHDRShot(MINT32 i4SensorDev, MBOOL bHDRShot)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setzCHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setzCHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setzCHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).setzCHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).setzCHDRShot(bHDRShot);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableHDRShot(MINT32 i4SensorDev, MBOOL bHDRShot)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).enableHDRShot(bHDRShot);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).enableHDRShot(bHDRShot);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::updateCaptureShutterValue(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateCaptureShutterValue();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateCaptureShutterValue();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateCaptureShutterValue();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).updateCaptureShutterValue();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).updateCaptureShutterValue();

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::prepareCapParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).prepareCapParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).prepareCapParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).prepareCapParam();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).prepareCapParam();
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).prepareCapParam();

    return S_AE_OK;
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
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).isLVChangeTooMuch();

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::switchCapureDiffEVState(MINT32 i4SensorDev, MINT8 iDiffEV, strAEOutput &aeoutput)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).switchCapureDiffEVState(iDiffEV, aeoutput);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).switchCapureDiffEVState(iDiffEV, aeoutput);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).switchCapureDiffEVState(iDiffEV, aeoutput);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).switchCapureDiffEVState(iDiffEV, aeoutput);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).switchCapureDiffEVState(iDiffEV, aeoutput);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::SaveAEMgrInfo(MINT32 i4SensorDev, const char * fname)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).SaveAEMgrInfo(fname);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).SaveAEMgrInfo(fname);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).SaveAEMgrInfo(fname);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).SaveAEMgrInfo(fname);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).SaveAEMgrInfo(fname);

    CAM_LOGE("Err IAEMgr::SaveAEMgrInfo()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AeMgr::getInstance(ESensorDev_SubSecond).getNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AeMgr::getInstance(ESensorDev_MainThird).getNVRAMParam(a_pAENVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEInitExpoSetting(MINT32 i4SensorDev, AEInitExpoSetting_T &a_rAEInitExpoSetting)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).getAEInitExpoSetting(a_rAEInitExpoSetting);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).getAEInitExpoSetting(a_rAEInitExpoSetting);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getAEInitExpoSetting(a_rAEInitExpoSetting);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getAEInitExpoSetting(a_rAEInitExpoSetting);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).getAEInitExpoSetting(a_rAEInitExpoSetting);

  return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAE2AFInfo(MINT32 i4SensorDev, AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).getAE2AFInfo(rAeWinSize, rAEInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).getAE2AFInfo(rAeWinSize, rAEInfo);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setDigZoomRatio(MINT32 i4SensorDev, MINT32 i4ZoomRatio)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).setDigZoomRatio(i4ZoomRatio);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).setDigZoomRatio(i4ZoomRatio);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::switchExpSettingByShutterISOpriority(MINT32 i4SensorDev, AE_EXP_SETTING_T &a_strExpInput, AE_EXP_SETTING_T &a_strExpOutput)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).switchExpSettingByShutterISOpriority(a_strExpInput, a_strExpOutput);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).switchExpSettingByShutterISOpriority(a_strExpInput, a_strExpOutput);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).switchExpSettingByShutterISOpriority(a_strExpInput, a_strExpOutput);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).switchExpSettingByShutterISOpriority(a_strExpInput, a_strExpOutput);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).switchExpSettingByShutterISOpriority(a_strExpInput, a_strExpOutput);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::UpdateAF2AEInfo(MINT32 i4SensorDev, AF2AEInfo_T &rAFInfo)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).UpdateAF2AEInfo(rAFInfo);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).UpdateAF2AEInfo(rAFInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).UpdateAF2AEInfo(rAFInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AeMgr::getInstance(ESensorDev_SubSecond).UpdateAF2AEInfo(rAFInfo);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AeMgr::getInstance(ESensorDev_MainThird).UpdateAF2AEInfo(rAFInfo);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}
