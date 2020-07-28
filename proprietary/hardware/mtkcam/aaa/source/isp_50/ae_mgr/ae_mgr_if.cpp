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

void IAeMgr::setAeMeterAreaEn(MINT32 i4SensorDev, int en)
{
    return AeMgr::getInstance(i4SensorDev).setAeMeterAreaEn(en);
}

MRESULT IAeMgr::setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea)
{
    return AeMgr::getInstance(i4SensorDev).setAEMeteringArea(sNewAEMeteringArea);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFDenable(MINT32 i4SensorDev, MBOOL bFDenable)
{
    return AeMgr::getInstance(i4SensorDev).setFDenable(bFDenable);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEFDMeteringAreaInfo(MINT32 i4SensorDev, android::Vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    return AeMgr::getInstance(i4SensorDev).getAEFDMeteringAreaInfo(vecOut,i4tgwidth,i4tgheight);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT IAeMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    return AeMgr::getInstance(i4SensorDev).setFDInfo(a_sFaces,i4tgwidth,i4tgheight);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setOTInfo(MINT32 i4SensorDev, MVOID* a_sOT)
{
    return AeMgr::getInstance(i4SensorDev).setOTInfo(a_sOT);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep, MBOOL is_preset)
{
    return AeMgr::getInstance(i4SensorDev).setAEEVCompIndex(i4NewEVIndex, fStep, is_preset);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 IAeMgr::getEVCompensateIndex(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getEVCompensateIndex();
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
MINT32 IAeMgr::getAEMeterMode(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEMeterMode();
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
MINT32 IAeMgr::getAEISOSpeedMode(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEISOSpeedMode();
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
MRESULT IAeMgr::setAEAutoFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEAutoFLKMode)
{
    return AeMgr::getInstance(i4SensorDev).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
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
MRESULT IAeMgr::setAESMBuffermode(MINT32 i4SensorDev, MBOOL bSMBuffmode, MINT32 i4subframeCnt)
{
    return AeMgr::getInstance(i4SensorDev).setAESMBuffermode(bSMBuffmode,i4subframeCnt);
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
MRESULT IAeMgr::setCamScenarioMode(MINT32 i4SensorDev, MUINT32 u4CamScenarioMode)
{
    return AeMgr::getInstance(i4SensorDev).setCamScenarioMode(u4CamScenarioMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setNVRAMIndex(MINT32 i4SensorDev, MUINT32 a_eNVRAMIndex)
{
    return AeMgr::getInstance(i4SensorDev).setNVRAMIndex(a_eNVRAMIndex);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAAOMode(MINT32 i4SensorDev, MUINT32 u4AEAAOmode)
{
    return AeMgr::getInstance(i4SensorDev).setAAOMode(u4AEAAOmode);
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
MRESULT IAeMgr::setAELimiterMode(MINT32 i4SensorDev, MBOOL bAELimter)
{
    return AeMgr::getInstance(i4SensorDev).setAELimiterMode(bAELimter);
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
MRESULT IAeMgr::setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MUINT32 width, MUINT32 height)
{
    return AeMgr::getInstance(i4SensorDev).setSensorMode(i4NewSensorMode,width,height);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEScenarioMode(MINT32 i4SensorDev, EIspProfile_T eIspProfile)
{
    return AeMgr::getInstance(i4SensorDev).updateAEScenarioMode(eIspProfile);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getSensorMode(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getSensorMode();
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
MRESULT IAeMgr::setAFAELock(MINT32 i4SensorDev, MBOOL bAFAELock)
{
	return AeMgr::getInstance(i4SensorDev).setAFAELock(bAFAELock);
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
MUINT32 IAeMgr::getAEMaxMeterAreaNum(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAEMaxMeterAreaNum();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAE(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).enableAE();
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
MRESULT IAeMgr::disableAE(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).disableAE();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule,MUINT8 u1AeMode)
{
    return AeMgr::getInstance(i4SensorDev).doPvAE(i8TimeStamp,pAEStatBuf,i4ActiveAEItem,u4AAOUpdate,bAAASchedule,u1AeMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAE_TG_INT(MINT32 i4SensorDev, MVOID *pAEStatBuf)
{
    return AeMgr::getInstance(i4SensorDev).doPvAE_TG_INT(pAEStatBuf);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAEAAO(MINT32 i4SensorDev, MVOID *pAEStatBuf, MUINT32 u4AAOUpdate)
{
    return AeMgr::getInstance(i4SensorDev).doAFAEAAO(pAEStatBuf,u4AAOUpdate);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule,MBOOL bIsFlashCapture)
{
    return AeMgr::getInstance(i4SensorDev).doAFAE(i8TimeStamp,pAEStatBuf,i4ActiveAEItem,u4AAOUpdate,bAAASchedule,bIsFlashCapture);
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
MRESULT IAeMgr::doBackAEInfo(MINT32 i4SensorDev,MBOOL bTorchMode)
{
    return AeMgr::getInstance(i4SensorDev).doBackAEInfo(bTorchMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setRestore(MINT32 i4SensorDev, int frm)
{
    AeMgr::getInstance(i4SensorDev).setRestore(frm);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doRestoreAEInfo(MINT32 i4SensorDev, MBOOL bRestorePrvOnly,MBOOL bTorchMode)
{
    return AeMgr::getInstance(i4SensorDev).doRestoreAEInfo(bRestorePrvOnly,bTorchMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAAOProcInfo(MINT32 i4SensorDev, MVOID *pPSOStatBuf, AAO_PROC_INFO_T const *sNewAAOProcInfo)
{
    return AeMgr::getInstance(i4SensorDev).setAAOProcInfo(pPSOStatBuf,sNewAAOProcInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setMVHDR3ExpoProcInfo(MINT32 i4SensorDev, MVOID *pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100)
{
    return AeMgr::getInstance(i4SensorDev).setMVHDR3ExpoProcInfo(pMVHDR3ExpoStatBuf,u4MVHDRRatio_x100);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapFlare(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    return AeMgr::getInstance(i4SensorDev).doCapFlare(pAEStatBuf,bIsStrobe);
}

void IAeMgr::setExp(MINT32 i4SensorDev, int exp)
{
    AeMgr::getInstance(i4SensorDev).setExp(exp);
}

void IAeMgr::setAfe(MINT32 i4SensorDev, int afe)
{
    AeMgr::getInstance(i4SensorDev).setAfe(afe);
}

void IAeMgr::setIsp(MINT32 i4SensorDev, int isp)
{
    AeMgr::getInstance(i4SensorDev).setIsp(isp);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::getCCUresultCBActive(MINT32 i4SensorDev, MVOID* pfCB)
{
    return AeMgr::getInstance(i4SensorDev).getCCUresultCBActive(pfCB);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn)
{
    return AeMgr::getInstance(i4SensorDev).getLVvalue(isStrobeOn);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAOECompLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn)
{
    return AeMgr::getInstance(i4SensorDev).getAOECompLVvalue(isStrobeOn);
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
MRESULT IAeMgr::getDebugInfo(MINT32 i4SensorDev, AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo)
{
    return AeMgr::getInstance(i4SensorDev).getDebugInfo(rAEDebugInfo,rAEPlineDebugInfo);
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
MRESULT IAeMgr::getCurrentPlineTable(MINT32 i4SensorDev, strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo)
{
    return AeMgr::getInstance(i4SensorDev).getCurrentPlineTable(a_PrvAEPlineTable,a_CapAEPlineTable,a_StrobeAEPlineTable,a_StrobeAEPlineInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getCurrentPlineTableF(MINT32 i4SensorDev, strFinerEvPline &a_PrvAEPlineTable, strFinerEvPline &a_CapAEPlineTable)
{
    return AeMgr::getInstance(i4SensorDev).getCurrentPlineTableF(a_PrvAEPlineTable,a_CapAEPlineTable);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAECapPlineTable(MINT32 i4SensorDev, MINT32 *i4CapIndex, strAETable &a_CapAEPlineTable)
{
    return AeMgr::getInstance(i4SensorDev).getAECapPlineTable(i4CapIndex,a_CapAEPlineTable);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getSensorDeviceInfo(MINT32 i4SensorDev, AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    return AeMgr::getInstance(i4SensorDev).getSensorDeviceInfo(a_rDeviceInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsDoAEInPreAF(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsDoAEInPreAF();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAEStable(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsAEStable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAELock(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsAELock();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAPAELock(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsAPAELock();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getBVvalue(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getBVvalue();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getDeltaBV(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getDeltaBV();
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
MRESULT IAeMgr::setStrobeMode(MINT32 i4SensorDev, MBOOL bIsStrobeOn)
{
    return AeMgr::getInstance(i4SensorDev).setStrobeMode(bIsStrobeOn);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getPreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo, MBOOL bReCalcul)
{
    return AeMgr::getInstance(i4SensorDev).getPreviewParams(a_rPreviewInfo, bReCalcul);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::IsGetManualExpState(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).IsGetManualExpState();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getExposureInfo(MINT32 i4SensorDev, ExpSettingParam_T &strHDRInputSetting)
{
    return AeMgr::getInstance(i4SensorDev).getExposureInfo(strHDRInputSetting);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo)
{
    return AeMgr::getInstance(i4SensorDev).getCaptureParams(a_rCaptureInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorListenerParams(MINT32 i4SensorDev, MINT32 *i4SensorInfo)
{
    return AeMgr::getInstance(i4SensorDev).updateSensorListenerParams(i4SensorInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateISPNvramOBCTable(MINT32 i4SensorDev, MVOID* pISPNvramOBCTable, MINT32 i4Tableidx)
{
    return AeMgr::getInstance(i4SensorDev).updateISPNvramOBCTable(pISPNvramOBCTable,i4Tableidx);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateISPNvramOBCIndex(MINT32 i4SensorDev, MVOID* rMappingInfo, MINT32 i4Tableidx)
{
    return AeMgr::getInstance(i4SensorDev).updateISPNvramOBCIndex(rMappingInfo,i4Tableidx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEUnderExpdeltaBVIdx(MINT32 i4SensorDev, MINT32 i4AEdeltaBV)
{
    return AeMgr::getInstance(i4SensorDev).updateAEUnderExpdeltaBVIdx(i4AEdeltaBV);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updatePreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext, MINT32 i4AEidxNextF)
{
    return AeMgr::getInstance(i4SensorDev).updatePreviewParams(a_rPreviewInfo,i4AEidxNext,i4AEidxNextF);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo, MBOOL bRemosaicEn)
{
    return AeMgr::getInstance(i4SensorDev).updateCaptureParams(a_rCaptureInfo,bRemosaicEn);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::prepareCapParams(MINT32 i4SensorDev,MBOOL bRemosaicEn,MBOOL bIsEVChange)
{
    return AeMgr::getInstance(i4SensorDev).prepareCapParams(bRemosaicEn,bIsEVChange);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringYvalue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    return AeMgr::getInstance(i4SensorDev).getAEMeteringYvalue(rWinSize,uYvalue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringBlockAreaValue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
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
MRESULT IAeMgr::getRTParams(MINT32 i4SensorDev, FrameOutputParam_T &a_strFrameInfo)
{
    return AeMgr::getInstance(i4SensorDev).getRTParams(a_strFrameInfo);
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
MRESULT IAeMgr::UpdateSensorISPParams(MINT32 i4SensorDev, AE_STATE_T eNewAEState)
{
    return AeMgr::getInstance(i4SensorDev).UpdateSensorISPParams(eNewAEState);
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
MRESULT IAeMgr::SetAETargetMode(MINT32 i4SensorDev, eAETargetMODE eAETargetMode)
{
    return AeMgr::getInstance(i4SensorDev).SetAETargetMode(eAETargetMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setEMVHDRratio(MINT32 i4SensorDev, MUINT32 u4ratio)
{
    return AeMgr::getInstance(i4SensorDev).setEMVHDRratio(u4ratio);
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
MRESULT IAeMgr::getAEdelayInfo(MINT32 i4SensorDev, MINT32* i4ExpDelay, MINT32* i4AEGainDelay, MINT32* i4IspGainDelay)
{
    return AeMgr::getInstance(i4SensorDev).getAEdelayInfo(i4ExpDelay,i4AEGainDelay,i4IspGainDelay);
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
MRESULT IAeMgr::getPerframeAEFlag(MINT32 i4SensorDev, MBOOL &bPerframeAE)
{
    return AeMgr::getInstance(i4SensorDev).getPerframeAEFlag(bPerframeAE);
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
MRESULT IAeMgr::updateSensorbyI2CBufferMode(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).updateSensorbyI2CBufferMode();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorbyI2C(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).updateSensorbyI2C();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEState2Converge(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).setAEState2Converge();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::UpdateSensorParams(MINT32 i4SensorDev, AE_SENSOR_PARAM_T strSensorParams)
{
    return AeMgr::getInstance(i4SensorDev).UpdateSensorParams(strSensorParams);
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
MRESULT IAeMgr::getSensorParamsCCU(MINT32 i4SensorDev, AE_SENSOR_PARAM_T &a_rSensorInfo)
{
    return AeMgr::getInstance(i4SensorDev).getSensorParamsCCU(a_rSensorInfo);
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
MINT64 IAeMgr::getSensorRollingShutter(MINT32 i4SensorDev) const
{
    return AeMgr::getInstance(i4SensorDev).getSensorRollingShutter();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEBV(MINT32 i4SensorDev,MVOID *pAEStatBuf)
{
   return AeMgr::getInstance(i4SensorDev).updateAEBV(pAEStatBuf);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSensorDirectly(MINT32 i4SensorDev, CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting)
{
    return AeMgr::getInstance(i4SensorDev).setSensorDirectly(a_rCaptureInfo,bDirectlySetting);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAAOLineByteSize(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getAAOLineByteSize();
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
MINT32 IAeMgr::getCaptureMaxFPS(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).getCaptureMaxFPS();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::UpdateCustParams(MINT32 i4SensorDev, AE_Cust_Param_T a_rCustomParam)
{
    return AeMgr::getInstance(i4SensorDev).UpdateCustParams(&a_rCustomParam);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::EnableSuperNightShot(MINT32 i4SensorDev, MINT32 i4SuperNightShot)
{
    return AeMgr::getInstance(i4SensorDev).EnableSuperNightShot(i4SuperNightShot);
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
MINT32 IAeMgr::CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 u4CamScenarioMode)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEApplyNVRAMParam(a_pAENVRAM,u4CamScenarioMode);
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
MINT32 IAeMgr::CCTOPAEGetIspOB(MINT32 i4SensorDev, MUINT32 *a_pIspOB, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetIspOB(a_pIspOB,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetIspOB(MINT32 i4SensorDev, MUINT32 a_IspOB)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetIspOB(a_IspOB);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetIspRAWGain(MINT32 i4SensorDev, MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetIspRAWGain(a_pIspRawGain,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetIspRAWGain(MINT32 i4SensorDev, MUINT32 a_IspRAWGain)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetIspRAWGain(a_IspRAWGain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorExpTime(MINT32 i4SensorDev, MUINT32 a_ExpTime)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetSensorExpTime(a_ExpTime);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorExpLine(MINT32 i4SensorDev, MUINT32 a_ExpLine)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetSensorExpLine(a_ExpLine);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorGain(MINT32 i4SensorDev, MUINT32 a_SensorGain)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetSensorGain(a_SensorGain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetCaptureMode(MINT32 i4SensorDev, MUINT32 a_CaptureMode)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAESetCaptureMode(a_CaptureMode);
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
MINT32 IAeMgr::CCTOPAEGetFlareOffset(MINT32 i4SensorDev, MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).CCTOPAEGetFlareOffset(a_FlareThres,a_pAEFlareOffset,a_pOutLen);
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
MINT32 IAeMgr::modifyAEPlineTableLimitation(MINT32 i4SensorDev, MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    return AeMgr::getInstance(i4SensorDev).modifyAEPlineTableLimitation(bEnable,bEquivalent,u4IncreaseISO_x100,u4IncreaseShutter_x100);
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
MRESULT IAeMgr::enableAEOneShotControl(MINT32 i4SensorDev, MBOOL bAEControl)
{
    return AeMgr::getInstance(i4SensorDev).enableAEOneShotControl(bAEControl);
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
MINT32 IAeMgr::get3ACaptureDelayFrame(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).get3ACaptureDelayFrame();
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
MINT32 IAeMgr::IsAEContinueShot(MINT32 i4SensorDev, MBOOL bCShot)
{
    return AeMgr::getInstance(i4SensorDev).IsAEContinueShot(bCShot);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableEISRecording(MINT32 i4SensorDev, MBOOL bEISRecording)
{
    return AeMgr::getInstance(i4SensorDev).enableEISRecording(bEISRecording);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAISManualPline(MINT32 i4SensorDev, MUINT32 u4AISPline)
{
    return AeMgr::getInstance(i4SensorDev).enableAISManualPline(u4AISPline);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableMFHRManualPline(MINT32 i4SensorDev, MBOOL bMFHRPline)
{
    return AeMgr::getInstance(i4SensorDev).enableMFHRManualPline(bMFHRPline);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableBMDNManualPline(MINT32 i4SensorDev, MBOOL bBMDNPline)
{
    return AeMgr::getInstance(i4SensorDev).enableBMDNManualPline(bBMDNPline);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateStereoDenoiseRatio(MINT32 i4SensorDev, MINT32* i4StereoDenoiserto)
{
    return AeMgr::getInstance(i4SensorDev).updateStereoDenoiseRatio(i4StereoDenoiserto);
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
MRESULT IAeMgr::enableFlareInManualControl(MINT32 i4SensorDev, MBOOL bIsFlareInManual)
{
    return AeMgr::getInstance(i4SensorDev).enableFlareInManualControl(bIsFlareInManual);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEStereoManualPline(MINT32 i4SensorDev, MBOOL bstereomode)
{
    return AeMgr::getInstance(i4SensorDev).enableAEStereoManualPline(bstereomode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setzCHDRShot(MINT32 i4SensorDev, MBOOL bHDRShot)
{
    return AeMgr::getInstance(i4SensorDev).setzCHDRShot(bHDRShot);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableHDRShot(MINT32 i4SensorDev, MBOOL bHDRShot)
{
    return AeMgr::getInstance(i4SensorDev).enableHDRShot(bHDRShot);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::updateCaptureShutterValue(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).updateCaptureShutterValue();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::prepareCapParam(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).prepareCapParam();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::isLVChangeTooMuch(MINT32 i4SensorDev)
{
    return AeMgr::getInstance(i4SensorDev).isLVChangeTooMuch();
}
MINT32 IAeMgr::switchCapureDiffEVState(MINT32 i4SensorDev, MINT8 iDiffEV, strAEOutput &aeoutput)
{
    return AeMgr::getInstance(i4SensorDev).switchCapureDiffEVState(iDiffEV,aeoutput);
}

MBOOL IAeMgr::SaveAEMgrInfo(MINT32 i4SensorDev, const char * fname)
{
    return AeMgr::getInstance(i4SensorDev).SaveAEMgrInfo(fname);

    CAM_LOGE("Err IAEMgr::SaveAEMgrInfo()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    return AeMgr::getInstance(i4SensorDev).getNVRAMParam(a_pAENVRAM,a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEInitExpoSetting(MINT32 i4SensorDev, AEInitExpoSetting_T &a_rAEInitExpoSetting)
{
    return AeMgr::getInstance(i4SensorDev).getAEInitExpoSetting(a_rAEInitExpoSetting);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAE2AFInfo(MINT32 i4SensorDev, AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo)
{
    return AeMgr::getInstance(i4SensorDev).getAE2AFInfo(rAeWinSize,rAEInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::reconfig(MINT32 i4SensorDev, MVOID *pDBinInfo, MVOID *pOutRegCfg)
{
    return AeMgr::getInstance(i4SensorDev).reconfig(pDBinInfo,pOutRegCfg);
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
MRESULT IAeMgr::setFrameRateLock(MINT32 i4SensorDev,MBOOL enable)
{
   return AeMgr::getInstance(i4SensorDev).setFrameRateLock(enable);
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
MRESULT IAeMgr::switchExpSettingByShutterISOpriority(MINT32 i4SensorDev, AE_EXP_SETTING_T &a_strExpInput, AE_EXP_SETTING_T &a_strExpOutput)
{
    return AeMgr::getInstance(i4SensorDev).switchExpSettingByShutterISOpriority(a_strExpInput,a_strExpOutput);
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
MRESULT IAeMgr::PresetControlCCU(MINT32 i4SensorDev,MBOOL SkipCCUAE)
{
   return AeMgr::getInstance(i4SensorDev).PresetControlCCU(SkipCCUAE);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::CCUManualControl(MINT32 i4SensorDev,MUINT32 u4ExpTime,MUINT32 u4AfeGain,MUINT32 u4IspGain,MBOOL EnableManual)
{
   return AeMgr::getInstance(i4SensorDev).CCUManualControl(u4ExpTime,u4AfeGain,u4IspGain,EnableManual);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsNeedPresetControlCCU(MINT32 i4SensorDev)
{
   return AeMgr::getInstance(i4SensorDev).IsNeedPresetControlCCU();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsCCUAEInit(MINT32 i4SensorDev,MBOOL bInit)
{
   return AeMgr::getInstance(i4SensorDev).IsCCUAEInit(bInit);
}

