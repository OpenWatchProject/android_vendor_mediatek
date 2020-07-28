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
    return AeMgr::getInstance(i4SensorDev).cameraPreviewInit(i4SensorIdx,rParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::Start(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).Start();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::uninit(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).uninit();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::Stop(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).Stop();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea)
{
    return AeMgr::getInstance(i4SensorDev).setAEMeteringArea(sNewAEMeteringArea);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep, MBOOL is_preset)
{
    return AeMgr::getInstance(i4SensorDev).setAEEVCompIndex(i4NewEVIndex, fStep, is_preset);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringMode(MINT32 i4SensorDev, MUINT32 u4NewAEMeteringMode)
{
    return AeMgr::getInstance(i4SensorDev).setAEMeteringMode(u4NewAEMeteringMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEISOSpeed(MINT32 i4SensorDev, MUINT32 u4NewAEISOSpeed)
{
    return AeMgr::getInstance(i4SensorDev).setAEISOSpeed(u4NewAEISOSpeed);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMinMaxFrameRate(MINT32 i4SensorDev, MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    return AeMgr::getInstance(i4SensorDev).setAEMinMaxFrameRate(i4NewAEMinFps,i4NewAEMaxFps);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEFLKMode)
{
    return AeMgr::getInstance(i4SensorDev).setAEFlickerMode(u4NewAEFLKMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive)
{
    return AeMgr::getInstance(i4SensorDev).setFlickerFrameRateActive(a_bFlickerFPSAvtive);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAECamMode(MINT32 i4SensorDev, MUINT32 u4NewAECamMode)
{

    return AeMgr::getInstance(i4SensorDev).setAECamMode(u4NewAECamMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEManualPline(MINT32 i4SensorDev, EAEManualPline_T eAEManualPline, MBOOL bEnable)
{
    return AeMgr::getInstance(i4SensorDev).enableAEManualPline(eAEManualPline,bEnable);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEManualCapPline(MINT32 i4SensorDev, EAEManualPline_T eAEManualCapPline, MBOOL bEnable)
{
   return AeMgr::getInstance(i4SensorDev).enableAEManualCapPline(eAEManualCapPline,bEnable);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEShotMode(MINT32 i4SensorDev, MUINT32 u4NewAEShotMode)
{
    return AeMgr::getInstance(i4SensorDev).setAEShotMode(u4NewAEShotMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEHDRMode(MINT32 i4SensorDev, MUINT32 u4AEHDRmode)
{
    return AeMgr::getInstance(i4SensorDev).setAEHDRMode(u4AEHDRmode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEHDROnOff(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEHDROnOff();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSceneMode(MINT32 i4SensorDev, MUINT32 u4NewScene)
{
    return AeMgr::getInstance(i4SensorDev).setSceneMode(u4NewScene);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEScene(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEScene();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMode(MINT32 i4SensorDev, MUINT32 u4NewAEmode)
{
    return AeMgr::getInstance(i4SensorDev).setAEMode(u4NewAEmode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEMode(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEMode();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEState(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEState();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAPAELock(MINT32 i4SensorDev, MBOOL bAPAELock)
{
    return AeMgr::getInstance(i4SensorDev).setAPAELock(bAPAELock);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    return AeMgr::getInstance(i4SensorDev).setZoomWinInfo(u4XOffset,u4YOffset,u4Width,u4Height);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAeMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    return AeMgr::getInstance(i4SensorDev).setTGInfo(i4TGInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    return AeMgr::getInstance(i4SensorDev).doPvAE(i8TimeStamp,pAEStatBuf,i4ActiveAEItem,u4AAOUpdate,bAAASchedule);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    return AeMgr::getInstance(i4SensorDev).doAFAE(i8TimeStamp,pAEStatBuf,i4ActiveAEItem,u4AAOUpdate,bAAASchedule);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPreCapAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    return AeMgr::getInstance(i4SensorDev).doPreCapAE(i8TimeStamp,bIsStrobeFired,pAEStatBuf,i4ActiveAEItem,u4AAOUpdate,bAAASchedule);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapAE(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).doCapAE();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPostCapAE(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    return AeMgr::getInstance(i4SensorDev).doPostCapAE(pAEStatBuf,bIsStrobe);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getCaptureLVvalue(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getCaptureLVvalue();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAENvramData(MINT32 i4SensorDev, AE_NVRAM_T &rAENVRAM)
{
    return AeMgr::getInstance(i4SensorDev).getAENvramData(rAENVRAM);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getNvramData(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getNvramData(i4SensorDev);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsStrobeBVTrigger(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsStrobeBVTrigger();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringYvalue(MINT32 i4SensorDev, CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    return AeMgr::getInstance(i4SensorDev).getAEMeteringYvalue(rWinSize,uYvalue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringBlockAreaValue(MINT32 i4SensorDev, CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    return AeMgr::getInstance(i4SensorDev).getAEMeteringBlockAreaValue(rWinSize,uYvalue,u2YCnt);
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
    return AeMgr::getInstance(i4SensorDev).getAECondition(i4AECondition);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getLCEPlineInfo(MINT32 i4SensorDev, LCEInfo_T &a_rLCEInfo)
{
    return AeMgr::getInstance(i4SensorDev).getLCEPlineInfo(a_rLCEInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAERotateDegree(MINT32 i4SensorDev, MINT32 i4RotateDegree)
{
    return AeMgr::getInstance(i4SensorDev).setAERotateDegree(i4RotateDegree);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 IAeMgr::getAEFaceDiffIndex(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEFaceDiffIndex();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorDelayInfo(MINT32 i4SensorDev, MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay)
{
    return AeMgr::getInstance(i4SensorDev).updateSensorDelayInfo(i4SutterDelay,i4SensorGainDelay,i4IspGainDelay);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getBrightnessValue(MINT32 i4SensorDev, MBOOL * bFrameUpdate, MINT32* i4Yvalue)
{
    return AeMgr::getInstance(i4SensorDev).getBrightnessValue(bFrameUpdate,i4Yvalue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEBlockYvalues(MINT32 i4SensorDev, MUINT8 *pYvalues, MUINT8 size)
{
    return AeMgr::getInstance(i4SensorDev).getAEBlockYvalues(pYvalues,size);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setVideoDynamicFrameRate(MINT32 i4SensorDev, MBOOL bVdoDynamicFps)
{
    return AeMgr::getInstance(i4SensorDev).setVideoDynamicFrameRate(bVdoDynamicFps);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getTgIntAEInfo(MINT32 i4SensorDev, MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio)
{
    return AeMgr::getInstance(i4SensorDev).getTgIntAEInfo(bTgIntAEEn,fTgIntAERatio);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAESensorActiveCycle(MINT32 i4SensorDev, MINT32* i4ActiveCycle)
{
    return AeMgr::getInstance(i4SensorDev).getAESensorActiveCycle(i4ActiveCycle);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::IsNeedUpdateSensor(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsNeedUpdateSensor();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::bBlackLevelLock(MINT32 i4SensorDev, MBOOL bLockBlackLevel)
{
    return AeMgr::getInstance(i4SensorDev).bBlackLevelLock(bLockBlackLevel);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getSensorParams(MINT32 i4SensorDev, AE_SENSOR_PARAM_T &a_rSensorInfo)
{
    return AeMgr::getInstance(i4SensorDev).getSensorParams(a_rSensorInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEConfigParams(MINT32 i4SensorDev, MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen)
{
   return AeMgr::getInstance(i4SensorDev).getAEConfigParams(bHDRen,bOverCnten,bTSFen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAAOSize(MINT32 i4SensorDev, MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH)
{
    return AeMgr::getInstance(i4SensorDev).getAAOSize(u4BlockNumW,u4BlockNumH);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEEnable(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEEnable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEDisable(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEDisable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::CCTOPAEGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAE, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetEnableInfo(a_pEnableAE,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetAEScene(MINT32 i4SensorDev, MINT32 a_AEScene)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetSceneMode(a_AEScene);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetAEScene(MINT32 i4SensorDev, MINT32 *a_pAEScene, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetAEScene(a_pAEScene,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetMeteringMode(MINT32 i4SensorDev, MINT32 a_AEMeteringMode)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetMeteringMode(a_AEMeteringMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEApplyExpParam(a_pAEExpParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetFlickerMode(MINT32 i4SensorDev, MINT32 a_AEFlickerMode)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetFlickerMode(a_AEFlickerMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetExpParam(a_pAEExpParamIn,a_pAEExpParamOut,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetFlickerMode(MINT32 i4SensorDev, MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetFlickerMode(a_pAEFlickerMode,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetMeteringMode(MINT32 i4SensorDev, MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetMeteringMode(a_pAEMEteringMode,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 u4AENVRAMIdx)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEApplyNVRAMParam(a_pAENVRAM,u4AENVRAMIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetNVRAMParam(a_pAENVRAM,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESaveNVRAMParam(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESaveNVRAMParam();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetCurrentEV(MINT32 i4SensorDev, MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetCurrentEV(a_pAECurrentEV,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAELockExpSetting(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAELockExpSetting();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEUnLockExpSetting(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEUnLockExpSetting();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOSetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    return AeMgr::getInstance(i4SensorDev).CCTOSetCaptureParams(a_pAEExpParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOGetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    return AeMgr::getInstance(i4SensorDev).CCTOGetCaptureParams(a_pAEExpParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPSetAETargetValue(MINT32 i4SensorDev, MUINT32 u4AETargetValue)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPSetAETargetValue(u4AETargetValue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAESGG1Gain(MINT32 i4SensorDev, MUINT32 *pSGG1Gain)
{
    return AeMgr::getInstance(i4SensorDev).getAESGG1Gain(pSGG1Gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEPlineTable(MINT32 i4SensorDev, eAETableID eTableID, strAETable &a_AEPlineTable)
{
    return AeMgr::getInstance(i4SensorDev).getAEPlineTable(eTableID,a_AEPlineTable);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESavePlineNVRAM(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESavePlineNVRAM();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEAutoFlickerState(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEAutoFlickerState();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::IsMultiCapture(MINT32 i4SensorDev, MBOOL bMultiCap)
{
    return AeMgr::getInstance(i4SensorDev).IsMultiCapture(bMultiCap);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableStereoDenoiseRatio(MINT32 i4SensorDev, MUINT32 u4enableStereoDenoise)
{
    return AeMgr::getInstance(i4SensorDev).enableStereoDenoiseRatio(u4enableStereoDenoise);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::isLVChangeTooMuch(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).isLVChangeTooMuch();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAE2AFInfo(MINT32 i4SensorDev, CCU_AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo)
{
    return AeMgr::getInstance(i4SensorDev).getAE2AFInfo(rAeWinSize,rAEInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setDigZoomRatio(MINT32 i4SensorDev, MINT32 i4ZoomRatio)
{
    return AeMgr::getInstance(i4SensorDev).setDigZoomRatio(i4ZoomRatio);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::resetMvhdrRatio(MINT32 i4SensorDev, MBOOL bReset)
{
    return AeMgr::getInstance(i4SensorDev).resetMvhdrRatio(bReset);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setCCUOnOff(MINT32 i4SensorDev,MBOOL enable)
{
    return AeMgr::getInstance(i4SensorDev).setCCUOnOff(enable);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getISOThresStatus(MINT32 i4SensorDev, MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    return AeMgr::getInstance(i4SensorDev).getISOThresStatus(a_ISOIdx1Status,a_ISOIdx2Status);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::resetGetISOThresStatus(MINT32 i4SensorDev, MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    return AeMgr::getInstance(i4SensorDev).resetGetISOThresStatus(a_ISOIdx1Status,a_ISOIdx2Status);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsCCUAEInit(MINT32 i4SensorDev,MBOOL bInit)
{
    return AeMgr::getInstance(i4SensorDev).IsCCUAEInit(bInit);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setRequestNum(MINT32 i4SensorDev, MUINT32 u4ReqNum)
{
    return AeMgr::getInstance(i4SensorDev).setRequestNum(u4ReqNum);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEInfo(MINT32 i4SensorDev, AE_PERFRAME_INFO_T& rAEPerframeInfo)
{
    return AeMgr::getInstance(i4SensorDev).getAEInfo(rAEPerframeInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::configReg(MINT32 i4SensorDev, AEResultConfig_T *pResultConfig)
{
    return AeMgr::getInstance(i4SensorDev).configReg(pResultConfig);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::sendAECtrl(MINT32 i4SensorDev, EAECtrl_T eAECtrl, MINTPTR iArg1, MINTPTR iArg2, MINTPTR iArg3, MINTPTR iArg4)
{
    return AeMgr::getInstance(i4SensorDev).sendAECtrl(eAECtrl,iArg1,iArg2,iArg3,iArg4);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEParams(MINT32 i4SensorDev, AE_PARAM_SET_INFO const &rNewParam)
{
    return AeMgr::getInstance(i4SensorDev).setAEParams(rNewParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setDummyFrameInfo(MINT32 i4SensorDev, MUINT8 u1IsDummyFramemmy, MBOOL is_preset)
{
    return AeMgr::getInstance(i4SensorDev).setDummyFrameInfo(u1IsDummyFramemmy, is_preset);
}
