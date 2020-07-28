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
#define LOG_TAG "aaa_sensor_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <atomic>
#include <string.h>
//#include <cutils/pmem.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_param.h>
//#include <isp_drv.h>
#include <isp_tuning.h>
//#include "buf_mgr.h"

#include <aaa_sensor_mgr.h>
//Will be modified
#include <mtkcam/drv/IHwSyncDrv.h>

using namespace NS3Av3;
using namespace android;
using namespace NSCam;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr&
AAASensorMgr::
getInstance()
{
    static  AAASensorMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr::
AAASensorMgr()
    : m_pHalSensorObj(MNULL)
    , m_Users(ATOMIC_VAR_INIT(0))
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_bFlickerState_Main(MFALSE)
    , m_bFlickerState_Sub(MFALSE)
    , m_bFlickerState_MainTwo(MFALSE)
    , m_bFlickerState_SubTwo(MFALSE)
    , m_u4SensorFrameRateMain(0xFFFF)
    , m_u4SensorFrameRateSub(0xFFFF)
    , m_u4SensorFrameRateMain2(0xFFFF)
    , m_u4SensorFrameRateSub2(0xFFFF)
    , m_pHwSyncDrvObj(MNULL)
    , m_i4FrameSyncMainFps(0)
    , m_i4FrameSyncSubFps(0)
    , m_i4FrameSyncMain2Fps(0)
    , m_i4FrameSyncSub2Fps(0)
    , m_bEnableBuffMode_Main(MFALSE)
    , m_bEnableBuffMode_Sub(MFALSE)
    , m_bEnableBuffMode_MainTwo(MFALSE)
    , m_bEnableBuffMode_SubTwo(MFALSE)
{
    memset(&m_pSensorModeDelay, 0, 11*sizeof(MINT32));

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr::
~AAASensorMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
init(MINT32 const i4SensorIdx)
{
    MRESULT ret = S_AAA_SENSOR_MGR_OK;

    CAM_LOGD("[%s()] - E. m_Users: %d i4SensorIdx:%d\n", __FUNCTION__, std::atomic_load((&m_Users)), i4SensorIdx);

    std::lock_guard<std::mutex> lock((m_Lock));

    if (m_Users > 0) {
        CAM_LOGD("%d has created \n", std::atomic_load((&m_Users)));
        MINT32 i4BeforeUserCount = std::atomic_fetch_add((&m_Users), 1);        
        CAM_LOGD("[%s] BeforeUserCount %d, %d has created\n", __FUNCTION__, i4BeforeUserCount, std::atomic_load((&m_Users)));
        return ret;
    }

    // Sensor hal init
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    m_pHalSensorObj = pIHalSensorList->createSensor("aaa_sensor_mgr", i4SensorIdx);

    if(m_pHalSensorObj == NULL) {
        CAM_LOGE("[AAA Sensor Mgr] Can not create SensorHal obj\n");
    }

    m_pHwSyncDrvObj = MAKE_HWSyncDrv();
    if(m_pHwSyncDrvObj==NULL)
    {
        CAM_LOGE("[AAA Sensor Mgr] Can not create hwsync driver obj\n");
    }
    else
    {
        m_pHwSyncDrvObj->init(HW_SYNC_USER_AE, 0, 3);
    }

    MINT32 i4BeforeUserCount = std::atomic_fetch_add((&m_Users), 1);
    CAM_LOGD("[%s()] FrameRate: Main/Sub/Main2 = %d/%d/%d BeforeUserCount:%d\n", __FUNCTION__, m_u4SensorFrameRateMain, m_u4SensorFrameRateSub, m_u4SensorFrameRateMain2, i4BeforeUserCount);

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.debug.aaa_sensor_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    m_bFlickerState_Main = 0;
    m_bFlickerState_Sub = 0;
    m_bFlickerState_MainTwo = 0;
    m_bFlickerState_SubTwo = 0;
    m_i4FrameSyncMainFps      = 0;
    m_i4FrameSyncSubFps       = 0;
    m_i4FrameSyncMain2Fps     = 0;
    m_i4FrameSyncSub2Fps     = 0;
    m_bEnableBuffMode_Main    = 0;
    m_bEnableBuffMode_Sub     = 0;
    m_bEnableBuffMode_MainTwo = 0;
    m_bEnableBuffMode_SubTwo = 0;

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
uninit()
{
    MRESULT ret = S_AAA_SENSOR_MGR_OK;

    CAM_LOGD("[%s()] - E. m_Users: %d \n", __FUNCTION__, std::atomic_load((&m_Users)));

    std::lock_guard<std::mutex> lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0) {
        return ret;
    }

    // More than one user, so decrease one User.
    MINT32 i4BeforeUserCount = std::atomic_fetch_sub((&m_Users), 1);

    if (m_Users == 0) { // There is no more User after decrease one User
        if(m_pHwSyncDrvObj)
        {
            m_pHwSyncDrvObj->uninit(HW_SYNC_USER_AE, 0);
            m_pHwSyncDrvObj->destroyInstance();
            m_pHwSyncDrvObj=NULL;
        }
        if(m_pHalSensorObj) {
            m_pHalSensorObj->destroyInstance("aaa_sensor_mgr");
            m_pHalSensorObj = NULL;
        }
    } else {  // There are still some users.
        CAM_LOGD("[%s] BeforeUserCount Still %d users \n", __FUNCTION__, i4BeforeUserCount, std::atomic_load((&m_Users)));
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorVCinfo(MINT32 i4SensorDev, SENSOR_VC_INFO_T *a_rSensorVCInfo, MINT32 i4SensorMode)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MINT32 i4SensorSenraio = i4SensorMode;
    SensorVCInfo rSensorVCInfo;


    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

//    CAM_LOGD_IF(m_bDebugEnable,"[getSensorVCinfo]  \n");
    CAM_LOGD("[getSensorVCinfo]  \n");

    // Get default frame rate
    if(i4SensorDev == ESensorDev_Main) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else {
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    a_rSensorVCInfo->u2VCModeSelect = rSensorVCInfo.ModeSelect;
    a_rSensorVCInfo->u2VCShutterRatio = rSensorVCInfo.EXPO_Ratio;
    a_rSensorVCInfo->u2VCOBvalue = rSensorVCInfo.ODValue;

    switch(rSensorVCInfo.RG_STATSMODE)
    {
        case 1:   // 8x8
            a_rSensorVCInfo->u2VCStatWidth = 8;
            a_rSensorVCInfo->u2VCStatHeight = 8;
            break;
        case 2:   // 4x4
            a_rSensorVCInfo->u2VCStatWidth = 4;
            a_rSensorVCInfo->u2VCStatHeight = 4;
            break;
        case 3:   // 1x1
            a_rSensorVCInfo->u2VCStatWidth = 1;
            a_rSensorVCInfo->u2VCStatHeight = 1;
            break;
        default:
        case 0:   // 16x16
            a_rSensorVCInfo->u2VCStatWidth = 16;
            a_rSensorVCInfo->u2VCStatHeight = 16;
            break;
    }

    CAM_LOGD("[getSensorVCinfo] Mode:%d ShutterRatio:%d OBvalue:%d Stat width:%d height:%d %d Error:%d \n", a_rSensorVCInfo->u2VCModeSelect, a_rSensorVCInfo->u2VCShutterRatio,
    a_rSensorVCInfo->u2VCOBvalue, a_rSensorVCInfo->u2VCStatWidth, a_rSensorVCInfo->u2VCStatHeight, rSensorVCInfo.RG_STATSMODE, err);

    return S_AAA_SENSOR_MGR_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorSyncinfo(MINT32 i4SensorDev, MINT32 *i4SutterDelay, MINT32 *i4SensorGainDelay, MINT32 *i4IspGainDelay, MINT32 *i4SensorModeDelay)
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();

    CAM_LOGD("[setSensorDev] Sensor m_eSensorDevId:%d\n", i4SensorDev);

    if(i4SensorDev == ESensorDev_Main) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_Sub) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
    } else {
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    // Get sensor delay frame for sync
    *i4SutterDelay = rSensorStaticInfo.aeShutDelayFrame;
    *i4SensorGainDelay = rSensorStaticInfo.aeSensorGainDelayFrame;
    *i4IspGainDelay = rSensorStaticInfo.aeISPGainDelayFrame;
    *i4SensorModeDelay = rSensorStaticInfo.captureDelayFrame;
   m_pSensorModeDelay[0] = rSensorStaticInfo.previewDelayFrame;
   m_pSensorModeDelay[1] = rSensorStaticInfo.captureDelayFrame;
   m_pSensorModeDelay[2] = rSensorStaticInfo.videoDelayFrame;
   m_pSensorModeDelay[3] = rSensorStaticInfo.video1DelayFrame;
   m_pSensorModeDelay[4] = rSensorStaticInfo.video2DelayFrame;
   m_pSensorModeDelay[5] = rSensorStaticInfo.Custom1DelayFrame;
   m_pSensorModeDelay[6] = rSensorStaticInfo.Custom2DelayFrame;
   m_pSensorModeDelay[7] = rSensorStaticInfo.Custom3DelayFrame;
   m_pSensorModeDelay[8] = rSensorStaticInfo.Custom4DelayFrame;
   m_pSensorModeDelay[9] = rSensorStaticInfo.Custom5DelayFrame;
   m_pSensorModeDelay[10] = rSensorStaticInfo.previewDelayFrame;
    CAM_LOGD("[setSensorDev] Sensor delay frame Shutter:%d Gain:%d Isp:%d Capture delay:%d\n", *i4SutterDelay, *i4SensorGainDelay, *i4IspGainDelay, *i4SensorModeDelay);
    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorWidthHeight(MINT32 i4SensorDev, SENSOR_RES_INFO_T* a_rSensorResolution)
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    if(i4SensorDev == ESensorDev_Main) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_Sub) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
    } else {
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    a_rSensorResolution->u2SensorPreviewWidth = rSensorStaticInfo.previewWidth;
    a_rSensorResolution->u2SensorPreviewHeight = rSensorStaticInfo.previewHeight;
    a_rSensorResolution->u2SensorVideoWidth = rSensorStaticInfo.videoWidth;
    a_rSensorResolution->u2SensorVideoHeight = rSensorStaticInfo.videoHeight;
    a_rSensorResolution->u2SensorFullWidth = rSensorStaticInfo.captureWidth;
    a_rSensorResolution->u2SensorFullHeight = rSensorStaticInfo.captureHeight;
    a_rSensorResolution->u2SensorVideo1Width = rSensorStaticInfo.video1Width;
    a_rSensorResolution->u2SensorVideo1Height = rSensorStaticInfo.video1Height;
    a_rSensorResolution->u2SensorVideo2Width = rSensorStaticInfo.video2Width;
    a_rSensorResolution->u2SensorVideo2Height = rSensorStaticInfo.video2Height;

    CAM_LOGD("[getSensorWidthHeight] Sensor id:%d Prv:%d %d Video:%d %d Cap:%d %d Video1:%d %d Video2:%d %d\n", i4SensorDev, a_rSensorResolution->u2SensorPreviewWidth,
                  a_rSensorResolution->u2SensorPreviewHeight, a_rSensorResolution->u2SensorVideoWidth, a_rSensorResolution->u2SensorVideoHeight,
                  a_rSensorResolution->u2SensorFullWidth, a_rSensorResolution->u2SensorFullHeight, a_rSensorResolution->u2SensorVideo1Width, a_rSensorResolution->u2SensorVideo1Height,
                  a_rSensorResolution->u2SensorVideo2Width, a_rSensorResolution->u2SensorVideo2Height);
    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorMaxFrameRate(MINT32 i4SensorDev, MUINT32 rSensorMaxFrmRate[NSIspTuning::ESensorMode_NUM])
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    if(i4SensorDev == ESensorDev_Main) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_Sub) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Preview] = rSensorStaticInfo.previewFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Capture] = rSensorStaticInfo.captureFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Video] = rSensorStaticInfo.videoFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_SlimVideo1] = rSensorStaticInfo.video1FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_SlimVideo2] = rSensorStaticInfo.video2FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom1] = rSensorStaticInfo.custom1FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom2] = rSensorStaticInfo.custom2FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom3] = rSensorStaticInfo.custom3FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom4] = rSensorStaticInfo.custom4FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom5] = rSensorStaticInfo.custom5FrameRate;

    for (MUINT32 i = 0; i < NSIspTuning::ESensorMode_NUM; i++)
    {
        CAM_LOGD("[%s] SensorDev(%d) Mode(%d) MaxFPS(%d)", __FUNCTION__,
            i4SensorDev, i, rSensorMaxFrmRate[i]);
    }
    return S_AAA_SENSOR_MGR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorExpTime(MINT32 i4SensorDev, MUINT32 a_u4ExpTime)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;
    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setSensorExpTime] i4SensorDev:%d a_u4ExpTime = %d \n", i4SensorDev, a_u4ExpTime);

    if (a_u4ExpTime == 0) {
        CAM_LOGE("setSensorExpTime() error: exposure time = 0\n");
        return MHAL_INVALID_PARA;
    }


    if(i4SensorDev == ESensorDev_Main) {
        if (m_i4FrameSyncMainFps == 0){
            /*if(m_bEnableBuffMode_Main)
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE, (MUINTPTR)&a_u4ExpTime, 0, 0);
            else*/
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
        } else {
            // set exposure time and fps
            ret = setStereoFrameSyncExpAndFps(i4SensorDev, a_u4ExpTime ,m_i4FrameSyncMainFps);
        }
    } else if(i4SensorDev == ESensorDev_Sub) {
        if (m_i4FrameSyncSubFps == 0){
            /*if(m_bEnableBuffMode_Sub)
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE, (MUINTPTR)&a_u4ExpTime, 0, 0);
            else*/
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
        }else{
            // set exposure time and fps
            ret = setStereoFrameSyncExpAndFps(i4SensorDev, a_u4ExpTime ,m_i4FrameSyncSubFps);
        }
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        if (m_i4FrameSyncMain2Fps == 0){
            /*if (m_bEnableBuffMode_MainTwo)
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE, (MUINTPTR)&a_u4ExpTime, 0, 0);
            else*/
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
        }else{
            // set exposure time and fps
            ret = setStereoFrameSyncExpAndFps(i4SensorDev, a_u4ExpTime ,m_i4FrameSyncMain2Fps);
        }
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        if (m_i4FrameSyncSub2Fps == 0){
            /*if (m_bEnableBuffMode_MainTwo)
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE, (MUINTPTR)&a_u4ExpTime, 0, 0);
            else*/
                ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
        }else{
            // set exposure time and fps
            ret = setStereoFrameSyncExpAndFps(i4SensorDev, a_u4ExpTime ,m_i4FrameSyncSub2Fps);
        }
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }



    if(ret) {
        CAM_LOGE("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, a_u4ExpTime);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorGain(MINT32 i4SensorDev, MUINT32 a_u4SensorGain)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setSensorGain] i4SensorDev:%d a_u4SensorGain = %d \n", i4SensorDev, a_u4SensorGain);

    if (a_u4SensorGain < 1024) {
        CAM_LOGE("setSensorGain() error: sensor gain:%d \n", a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set sensor gain
    if(i4SensorDev == ESensorDev_Main) {
        /*if (m_bEnableBuffMode_Main)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE, (MUINTPTR)&a_u4SensorGain, 0, 0);
        else*/
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        /*if (m_bEnableBuffMode_Sub)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE, (MUINTPTR)&a_u4SensorGain, 0, 0);
        else*/
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        /*if (m_bEnableBuffMode_MainTwo)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE, (MUINTPTR)&a_u4SensorGain, 0, 0);
        else*/
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        /*if (m_bEnableBuffMode_MainTwo)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE, (MUINTPTR)&a_u4SensorGain, 0, 0);
        else*/
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_SENSOR_GAIN, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, a_u4SensorGain);
    }

    return (ret);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorDualGain(MINT32 i4SensorDev, MUINT32 a_u4LESensorGain, MUINT32 a_u4SESensorGain)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setSensorDualGain] i4SensorDev:%d a_u4LESensorGain = %d a_u4SESensorGain = %d\n", i4SensorDev, a_u4LESensorGain, a_u4SESensorGain);

    if (a_u4LESensorGain < 1024 || a_u4SESensorGain < 1024) {
        CAM_LOGE("setSensorDualGain() error: a_u4LESensorGain:%d  a_u4SESensorGain = %d\n", a_u4LESensorGain, a_u4SESensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set sensor gain
    if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4SESensorGain, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4SESensorGain, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4SESensorGain, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4SESensorGain, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_SENSOR_DUAL_GAIN, Sensor dev:%d a_u4LESensorGain:%d a_u4SESensorGain:%d\n", i4SensorDev, a_u4LESensorGain, a_u4SESensorGain);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorHDRTriGain(MINT32 i4SensorDev, MUINT32 a_u4LESensorGain, MUINT32 a_u4MESensorGain, MUINT32 a_u4SESensorGain)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setSensorHDRTriGain] i4SensorDev:%d a_u4LESensorGain = %d a_u4MESensorGain = %d a_u4SESensorGain = %d\n", i4SensorDev, a_u4LESensorGain, a_u4MESensorGain, a_u4SESensorGain);

    if (a_u4LESensorGain < 1024 || a_u4MESensorGain < 1024 || a_u4SESensorGain < 1024) {
        CAM_LOGE("setSensorHDRTriGain() error: a_u4LESensorGain:%d  a_u4MESensorGain:%d a_u4SESensorGain = %d\n", a_u4LESensorGain, a_u4MESensorGain, a_u4SESensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set sensor gain
    if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_TRI_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4MESensorGain, (MUINTPTR)&a_u4SESensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_TRI_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4MESensorGain, (MUINTPTR)&a_u4SESensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_TRI_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4MESensorGain, (MUINTPTR)&a_u4SESensorGain);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_HDR_TRI_GAIN, (MUINTPTR)&a_u4LESensorGain, (MUINTPTR)&a_u4MESensorGain, (MUINTPTR)&a_u4SESensorGain);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_SENSOR_HDR_TRI_GAIN, Sensor dev:%d a_u4LESensorGain:%d a_u4MESensorGain:%d a_u4SESensorGain:%d\n", i4SensorDev, a_u4LESensorGain, a_u4MESensorGain, a_u4SESensorGain);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorATR(MINT32 i4SensorDev, MUINT32 a_u4LimitGain, MUINT32 a_u4LtcRate)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setSensorATR] i4SensorDev:%d a_u4LimitGain = %d a_u4LtcRate = %d\n", i4SensorDev, a_u4LimitGain, a_u4LtcRate);

    // Set sensor gain
    if(i4SensorDev == ESensorDev_Main) {
            //ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_ATR, (MUINTPTR)&a_u4LimitGain, (MUINTPTR)&a_u4LtcRate, 0); // temp for test
    } else if(i4SensorDev == ESensorDev_Sub) {
            //ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_ATR, (MUINTPTR)&a_u4LimitGain, (MUINTPTR)&a_u4LtcRate, 0); // temp for test
    } else if(i4SensorDev == ESensorDev_MainSecond) {
            //ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_ATR, (MUINTPTR)&a_u4LimitGain, (MUINTPTR)&a_u4LtcRate, 0); // temp for test
    } else if(i4SensorDev == ESensorDev_SubSecond) {
            //ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_HDR_ATR, (MUINTPTR)&a_u4LimitGain, (MUINTPTR)&a_u4LtcRate, 0); // temp for test
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_SENSOR_HDR_ATR, Sensor dev:%d a_u4LimitGain:%d a_u4LtcRate:%d\n", i4SensorDev, a_u4LimitGain, a_u4LtcRate);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorFrameRate(MINT32 i4SensorDev, MUINT32 a_u4SensorFrameRate)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    MUINT32 u4SensorFrameRate;
    MUINT32 u4SensorFrameRateCur;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }
    // Current Frame Rate
    if(i4SensorDev == ESensorDev_Main) {
        u4SensorFrameRateCur = m_u4SensorFrameRateMain;
    } else if(i4SensorDev == ESensorDev_Sub) {
        u4SensorFrameRateCur = m_u4SensorFrameRateSub;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        u4SensorFrameRateCur = m_u4SensorFrameRateMain2;
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        u4SensorFrameRateCur = m_u4SensorFrameRateSub2;
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }
    CAM_LOGD_IF(m_bDebugEnable,"[setSensorFrameRate] i4SensorDev:%d a_u4SensorFrameRate = %d \n", i4SensorDev, a_u4SensorFrameRate);

    // Set sensor frame rate
    u4SensorFrameRate = a_u4SensorFrameRate/10;    // 10 base frame rate from AE

    // Set frame rate
    if(u4SensorFrameRate == u4SensorFrameRateCur){
        CAM_LOGD_IF(m_bDebugEnable,"[setSensorFrameRate] Same frame rate , SensorFrameRateCur = %d \n", u4SensorFrameRateCur);
    } else {
        if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
            m_u4SensorFrameRateMain = u4SensorFrameRate;
        } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
            m_u4SensorFrameRateSub = u4SensorFrameRate;
        } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
            m_u4SensorFrameRateMain2 = u4SensorFrameRate;
        } else if(i4SensorDev == ESensorDev_SubSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
            m_u4SensorFrameRateSub2 = u4SensorFrameRate;
        } else{
            CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
            return E_AAA_SENSOR_NULL;
        }
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_VIDEO_FRAME_RATE, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, u4SensorFrameRate);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorExpLine(MINT32 i4SensorDev, MUINT32 a_u4ExpLine)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setSensorExpTime] i4SensorDev:%d a_u4ExpLine = %d \n", i4SensorDev, a_u4ExpLine);

    if (a_u4ExpLine == 0) {
        CAM_LOGE("setSensorExpTime() error: exposure line = 0\n");
        return MHAL_INVALID_PARA;
    }

    // Set exposure line
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_SENSOR_EXP_LINE, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, a_u4ExpLine);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    CAM_LOGD("setFlickerFrameRateActive en=%d",a_bFlickerFPSAvtive);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4FlickerInfo;

    if  (!m_pHalSensorObj) {
       // CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setFlickerFrameRateActive] a_bFlickerFPSAvtive = %d \n", a_bFlickerFPSAvtive);

    u4FlickerInfo = (MUINT32)a_bFlickerFPSAvtive;

    // Set frame rate
    CAM_LOGD_IF(m_bDebugEnable,"sensorDev(%d) a_bFlickerFPSAvtive(%d) m_bFlickerState(%d, %d, %d)\n", i4SensorDev, a_bFlickerFPSAvtive, m_bFlickerState_Main, m_bFlickerState_Sub, m_bFlickerState_MainTwo);

    if(i4SensorDev == ESensorDev_Main) {
        if(a_bFlickerFPSAvtive != m_bFlickerState_Main)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        m_bFlickerState_Main = a_bFlickerFPSAvtive;
    } else if(i4SensorDev == ESensorDev_Sub) {
        if(a_bFlickerFPSAvtive != m_bFlickerState_Sub)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        m_bFlickerState_Sub = a_bFlickerFPSAvtive;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        if(a_bFlickerFPSAvtive != m_bFlickerState_MainTwo)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        m_bFlickerState_MainTwo = a_bFlickerFPSAvtive;
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        if(a_bFlickerFPSAvtive != m_bFlickerState_SubTwo)
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        m_bFlickerState_SubTwo = a_bFlickerFPSAvtive;
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_FLICKER_FRAME_RATE, Sensor dev:%d flicker status:%d\n", i4SensorDev, a_bFlickerFPSAvtive);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setPreviewParams(MINT32 i4SensorDev, MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    CAM_LOGD("[setPreviewParams] Id:%d Shutter:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);

    if(i4SensorDev == ESensorDev_Main) {
        //ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN, a_u4ExpTime);
        //ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
        m_u4SensorFrameRateMain = 0xFFFF;
    } else if(i4SensorDev == ESensorDev_Sub) {
        //ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_SUB, a_u4ExpTime);
        //ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
        m_u4SensorFrameRateSub = 0xFFFF;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        //ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpTime);
        //ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
        m_u4SensorFrameRateMain2 = 0xFFFF;
    } else if(i4SensorDev == ESensorDev_SubSecond) {
       // ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_SUB_2, a_u4ExpTime);
       // ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB_2, a_u4SensorGain);
        m_u4SensorFrameRateSub2 = 0xFFFF;
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret_shutter | ret_gain) {
        CAM_LOGE("Err setPreviewParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setCaptureParams(MINT32 i4SensorDev, MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;
    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    CAM_LOGD("[setCaptureParams] Id:%d Shutter:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);
    if(i4SensorDev == ESensorDev_Main) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_SUB, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_SUB_2, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB_2, a_u4SensorGain);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret_shutter | ret_gain) {
        CAM_LOGE("Err setCaptureParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setPreviewLineBaseParams(MINT32 i4SensorDev, MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    CAM_LOGD("[setPreviewLineBaseParams] Id:%d Exp. Line:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);

    if(i4SensorDev == ESensorDev_Main) {
       // ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN, a_u4ExpLine);
       // ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
        m_u4SensorFrameRateMain = 0xFFFF;
    } else if(i4SensorDev == ESensorDev_Sub) {
       // ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_SUB, a_u4ExpLine);
       // ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
        m_u4SensorFrameRateSub = 0xFFFF;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
       // ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpLine);
       // ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
        m_u4SensorFrameRateMain2 = 0xFFFF;
    } else if(i4SensorDev == ESensorDev_SubSecond) {
       // ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_SUB_2, a_u4ExpLine);
       // ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB_2, a_u4SensorGain);
        m_u4SensorFrameRateSub2 = 0xFFFF;
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }


    if(ret_shutter | ret_gain) {
        CAM_LOGE("Err setPreviewLineBaseParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setCaptureLineBaseParams(MINT32 i4SensorDev, MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    CAM_LOGD("[setCaptureLinaeBaseParams] Id:%d Exp. Line:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);

    if(i4SensorDev == ESensorDev_Main) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_SUB, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_SUB_2, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB_2, a_u4SensorGain);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }



    if(ret_shutter | ret_gain) {
        CAM_LOGE("Err setCaptureLinaeBaseParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AAASensorMgr::setPreviewMaxFrameRate(MINT32 i4SensorDev, MUINT32 frameRate, MUINT32 u4SensorMode)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4SensorFrameRateCur;
    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }
    // Current Frame Rate
    if(i4SensorDev == ESensorDev_Main) {
        u4SensorFrameRateCur = m_u4SensorFrameRateMain;
    } else if(i4SensorDev == ESensorDev_Sub) {
        u4SensorFrameRateCur = m_u4SensorFrameRateSub;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        u4SensorFrameRateCur = m_u4SensorFrameRateMain2;
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        u4SensorFrameRateCur = m_u4SensorFrameRateSub2;
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[setPreviewMaxFrameRate] frameRate = %d \n", frameRate);

    if (frameRate == 0) {
        CAM_LOGE("setPreviewMaxFrameRate() error: frameRate = 0\n");
        return MHAL_INVALID_PARA;
    }

    if(frameRate == u4SensorFrameRateCur) {
        CAM_LOGD_IF(m_bDebugEnable,"[setPreviewMaxFrameRate] Same frame rate SensorFrameRateCur = %d \n", u4SensorFrameRateCur);
    } else {
    // Set max frame rate
        CAM_LOGD("[setPreviewMaxFrameRate] frameRate = %d \n", frameRate);
        if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
            m_u4SensorFrameRateMain = frameRate;
        } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
            m_u4SensorFrameRateSub = frameRate;
        } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
            m_u4SensorFrameRateMain2 = frameRate;
        } else if(i4SensorDev == ESensorDev_SubSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
            m_u4SensorFrameRateSub2 = frameRate;
        } else{
            CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
            return E_AAA_SENSOR_NULL;
        }
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, Sensor dev:%d frameRate:%d\n", i4SensorDev, frameRate);
    }

    return (ret);
}

MUINT32 AAASensorMgr::getPreviewDefaultFrameRate(MINT32 i4SensorDev, MUINT32 u4SensorMode)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MUINT32 frmRate;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable,"[getPreviewMaxFrameRate]  \n");

    // Get default frame rate
    if(i4SensorDev == ESensorDev_Main) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    return frmRate;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
set2ShutterParams(MINT32 i4SensorDev, MUINT32 a_u4LEExpTime, MUINT32 a_u4SEExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[setiVHDRParams] Id:%d Shutter:%d %d Sensor Gain:%d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime, a_u4SensorGain);

    if ((a_u4LEExpTime == 0) || (a_u4SEExpTime == 0) || (a_u4SensorGain == 0)){
        CAM_LOGE("setiVHDRParams() error Id:%d Shutter:%d %d Sensor Gain:%d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d Shutter:%d %d Sensor Gain:%d\n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime, a_u4SensorGain);
    }

    return (ret);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
set2ShutterControl(MINT32 i4SensorDev, MUINT32 a_u4LEExpTime, MUINT32 a_u4SEExpTime,MINT32 a_i4LVvalue)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[setVHDRParams] Id:%d Shutter:%d %d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime);

    if ((a_u4LEExpTime == 0) || (a_u4SEExpTime == 0)){
        CAM_LOGE("setiVHDRParams() error Id:%d Shutter:%d %d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime);
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_i4LVvalue);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_i4LVvalue);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_i4LVvalue);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_i4LVvalue);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d Shutter:%d %d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
set3ShutterControl(MINT32 i4SensorDev, MUINT32 a_u4LEExpTime, MUINT32 a_u4MEExpTime, MUINT32 a_u4SEExpTime)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[set3ShutterControl] Id:%d Shutter:%d %d %d\n", i4SensorDev, a_u4LEExpTime, a_u4MEExpTime, a_u4SEExpTime);

    if ((a_u4LEExpTime == 0) || (a_u4MEExpTime == 0) || (a_u4SEExpTime == 0)){
        CAM_LOGE("setiVHDRParams() error Id:%d Shutter:%d %d %d\n", i4SensorDev, a_u4LEExpTime, a_u4MEExpTime, a_u4SEExpTime);
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_TRI_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4MEExpTime, (MUINTPTR)&a_u4SEExpTime);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_TRI_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4MEExpTime, (MUINTPTR)&a_u4SEExpTime);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_TRI_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4MEExpTime, (MUINTPTR)&a_u4SEExpTime);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_HDR_TRI_SHUTTER, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4MEExpTime, (MUINTPTR)&a_u4SEExpTime);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err SENSOR_CMD_SET_SENSOR_HDR_TRI_SHUTTER, Sensor dev:%d Shutter:%d %d %d \n", i4SensorDev, a_u4LEExpTime, a_u4MEExpTime, a_u4SEExpTime);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorAWBGain(MINT32 i4SensorDev, strSensorAWBGain *strSensorAWBInfo)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD("[setSensorAWBGain] Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
    strSensorAWBInfo->u4R, strSensorAWBInfo->u4B, strSensorAWBInfo->u4GB);

    CAM_LOGD_IF(m_bDebugEnable, "[setSensorAWBGain] Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
    strSensorAWBInfo->u4R, strSensorAWBInfo->u4B, strSensorAWBInfo->u4GB);

    if ((strSensorAWBInfo->u4GB == 0) || (strSensorAWBInfo->u4R == 0) || (strSensorAWBInfo->u4B == 0) || (strSensorAWBInfo->u4GB == 0)){
        CAM_LOGE("setSensorAWBGain() error Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B,
        strSensorAWBInfo->u4GB);
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d GR:%d R:%d B:%d GB:%d \n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B,
        strSensorAWBInfo->u4GB);
    }

    CAM_LOGD("[setSensorAWBGain1] Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
    strSensorAWBInfo->u4R, strSensorAWBInfo->u4B,
    strSensorAWBInfo->u4GB);

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorShadingTbl(MINT32 i4SensorDev, std::vector<MUINT16*> &vecGain, MUINT32 tblSize)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    if (vecGain.size()<4) {
        CAM_LOGE("wrong size of vector pointer");
        return E_AAA_SENSOR_NULL;
    }

    MUINT16* bPtr =(MUINT16*)vecGain.at(2);
    MUINT16* gbPtr=(MUINT16*)vecGain.at(1);
    MUINT16* grPtr=(MUINT16*)vecGain.at(3);
    MUINT16* rPtr =(MUINT16*)vecGain.at(0);

    MUINT32 index0=0;
    MUINT32 index1=1;
    MUINT32 index2=2;
    MUINT32 index3=3;

    CAM_LOGD("Send %d bytes shading table to sensor", tblSize);


    // #1: color channel index: (0:B, 1:Gb, 2:Gr, 3:R)
    // #2: shading tbl buffer
    // #3: shading tbl size
    // Set shading table
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index0, (MUINTPTR) &bPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index1, (MUINTPTR) &gbPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index2, (MUINTPTR) &grPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index3, (MUINTPTR) &rPtr, (MUINTPTR)&tblSize);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index0, (MUINTPTR) &bPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index1, (MUINTPTR) &gbPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index2, (MUINTPTR) &grPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index3, (MUINTPTR) &rPtr, (MUINTPTR)&tblSize);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index0, (MUINTPTR) &bPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index1, (MUINTPTR) &gbPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index2, (MUINTPTR) &grPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index3, (MUINTPTR) &rPtr, (MUINTPTR)&tblSize);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index0, (MUINTPTR) &bPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index1, (MUINTPTR) &gbPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index2, (MUINTPTR) &grPtr, (MUINTPTR)&tblSize);
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_LSC_TBL, (MUINTPTR) &index3, (MUINTPTR) &rPtr, (MUINTPTR)&tblSize);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err command in %s\n", __func__);
    }



    return (ret);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorModeDelay(MINT32 i4SensorMode,MINT32 *i4SensorModeDelay)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    *i4SensorModeDelay = m_pSensorModeDelay[i4SensorMode];

    CAM_LOGD("[getSensorModeDelay] SensorMode:%d SensorModeDelay:%d \n", i4SensorMode, *i4SensorModeDelay);

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getRollingShutter(MINT32 i4SensorDev, MUINT32& tline, MUINT32& vsize) const
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    // Set exposure time
    if (i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] sensor(%d), tline(%d), vsize(%d)", __FUNCTION__, i4SensorDev, tline, vsize);

    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::setSensorOBLock(MINT32 i4SensorDev, MBOOL bLockSensorOB)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[setSensorOBLock] Id:%d bLockSensorOB:%d\n", i4SensorDev, bLockSensorOB);

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        CAM_LOGE("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d bLockSensorOB:%d\n", i4SensorDev, bLockSensorOB);
    }

    return (ret);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setStereoFrameSyncExpAndFps(MINT32 i4SensorDev, MUINT32 i4ExpTime ,MUINT32 i4Fps)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, (MUINTPTR)&i4ExpTime, (MUINTPTR)&i4Fps, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, (MUINTPTR)&i4ExpTime, (MUINTPTR)&i4Fps, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, (MUINTPTR)&i4ExpTime, (MUINTPTR)&i4Fps, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, (MUINTPTR)&i4ExpTime, (MUINTPTR)&i4Fps, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }
    CAM_LOGD("[setStereoFrameSyncExpAndFps] SensorDev:%d Exposure Time:%d Fps:%d \n", i4SensorDev, i4ExpTime,i4Fps);

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
updateStereoFrameSyncFps(MINT32 i4SensorDev, MINT32 i4SensorDev2, MUINT32 i4ExpTimeMain,MUINT32 i4ExpTimeMain2)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;
    MUINT32 expTime[2], frmTime[2];
    MUINT32 *ptr_expTime = expTime;
    MUINT32 *ptr_frmTime = frmTime;
    MUINT32 senDev;
    // Initial
    expTime[0] = i4ExpTimeMain;
    expTime[1] = i4ExpTimeMain2;
    frmTime[0] = 0;
    frmTime[1] = 0;
    // Inform exposure time to Hardware sync

    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHwSyncDrvObj->sendCommand(HW_SYNC_CMD_GET_FRAME_TIME, NSCam::SENSOR_DEV_MAIN, (MUINTPTR)ptr_expTime, (MUINTPTR)ptr_frmTime);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHwSyncDrvObj->sendCommand(HW_SYNC_CMD_GET_FRAME_TIME, NSCam::SENSOR_DEV_SUB, (MUINTPTR)ptr_expTime, (MUINTPTR)ptr_frmTime);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHwSyncDrvObj->sendCommand(HW_SYNC_CMD_GET_FRAME_TIME, NSCam::SENSOR_DEV_MAIN_2, (MUINTPTR)ptr_expTime, (MUINTPTR)ptr_frmTime);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHwSyncDrvObj->sendCommand(HW_SYNC_CMD_GET_FRAME_TIME, NSCam::SENSOR_DEV_SUB_2, (MUINTPTR)ptr_expTime, (MUINTPTR)ptr_frmTime);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }
    // Update Main/Main2 fps in class param
    if(ret < 0){
        CAM_LOGE("[updateStereoFrameSyncFps]sendCommand Fail");
    } else {
        if(i4SensorDev == ESensorDev_Main) {
            m_i4FrameSyncMainFps = frmTime[0];
            if(i4SensorDev2 == ESensorDev_MainSecond)
                m_i4FrameSyncMain2Fps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_Sub)
                m_i4FrameSyncSubFps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_SubSecond)
                m_i4FrameSyncSub2Fps = frmTime[1];
            else
                CAM_LOGE("[%s()] Sensor2 type error:%d", __FUNCTION__, i4SensorDev2);
        } else if(i4SensorDev == ESensorDev_Sub) {
            m_i4FrameSyncSubFps  = frmTime[0];
            if(i4SensorDev2 == ESensorDev_Main)
                m_i4FrameSyncMainFps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_MainSecond)
                m_i4FrameSyncMain2Fps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_SubSecond)
                m_i4FrameSyncSub2Fps = frmTime[1];
            else
                CAM_LOGE("[%s()] Sensor2 type error:%d", __FUNCTION__, i4SensorDev2);
        } else if(i4SensorDev == ESensorDev_MainSecond) {
            m_i4FrameSyncMain2Fps  = frmTime[0];
            if(i4SensorDev2 == ESensorDev_Main)
                m_i4FrameSyncMainFps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_Sub)
                m_i4FrameSyncSubFps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_SubSecond)
                m_i4FrameSyncSub2Fps = frmTime[1];
            else
                CAM_LOGE("[%s()] Sensor2 type error:%d", __FUNCTION__, i4SensorDev2);
        } else if(i4SensorDev == ESensorDev_SubSecond) {
            m_i4FrameSyncSub2Fps  = frmTime[0];
            if(i4SensorDev2 == ESensorDev_Main)
                m_i4FrameSyncMainFps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_Sub)
                m_i4FrameSyncSubFps = frmTime[1];
            else if(i4SensorDev2 == ESensorDev_MainSecond)
                m_i4FrameSyncMain2Fps = frmTime[1];
            else
                CAM_LOGE("[%s()] Sensor2 type error:%d", __FUNCTION__, i4SensorDev2);
        } else{
            CAM_LOGE("[%s()] Sensor1 type error:%d", __FUNCTION__, i4SensorDev);
            return E_AAA_SENSOR_NULL;
        }

    }

    CAM_LOGD("[updateStereoFrameSyncFps] i4SensorDev/Dev2:%d/%d exp:%d/%d (master/slave) , Fps:%d/%d/%d/%d (main/sub/main2/sub2)\n", i4SensorDev,i4SensorDev2, i4ExpTimeMain,i4ExpTimeMain2,m_i4FrameSyncMainFps,m_i4FrameSyncSubFps,m_i4FrameSyncMain2Fps,m_i4FrameSyncSub2Fps);

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
enableSensorI2CBuffMode(MINT32 i4SensorDev, MBOOL bEnableBuffMode)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    // Set Exposure time and FPS (waiting for lungbro)
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_I2C_BUF_MODE_EN, (MUINTPTR)&bEnableBuffMode, 0, 0);
        CAM_LOGD("[enableSensorI2CBuffMode] SensorDev:%d EnableBuffMode:%d -> %d\n", i4SensorDev, m_bEnableBuffMode_Main, bEnableBuffMode);
        m_bEnableBuffMode_Main = bEnableBuffMode;
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_I2C_BUF_MODE_EN, (MUINTPTR)&bEnableBuffMode, 0, 0);
        CAM_LOGD("[enableSensorI2CBuffMode] SensorDev:%d EnableBuffMode:%d -> %d\n", i4SensorDev, m_bEnableBuffMode_Sub, bEnableBuffMode);
        m_bEnableBuffMode_Sub = bEnableBuffMode;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_I2C_BUF_MODE_EN, (MUINTPTR)&bEnableBuffMode, 0, 0);
        CAM_LOGD("[enableSensorI2CBuffMode] SensorDev:%d EnableBuffMode:%d -> %d\n", i4SensorDev, m_bEnableBuffMode_MainTwo, bEnableBuffMode);
        m_bEnableBuffMode_MainTwo= bEnableBuffMode;
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_I2C_BUF_MODE_EN, (MUINTPTR)&bEnableBuffMode, 0, 0);
        CAM_LOGD("[enableSensorI2CBuffMode] SensorDev:%d EnableBuffMode:%d -> %d\n", i4SensorDev, m_bEnableBuffMode_SubTwo, bEnableBuffMode);
        m_bEnableBuffMode_SubTwo= bEnableBuffMode;
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    return (ret);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
disableFrameSync()
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;
    m_i4FrameSyncMainFps      = 0;
    m_i4FrameSyncSubFps       = 0;
    m_i4FrameSyncMain2Fps     = 0;
    m_i4FrameSyncSub2Fps     = 0;
    return (ret);
}
MRESULT
AAASensorMgr::
enableSensorTestPattern(MINT32 i4SensorDev)
{
 INT32 ret = S_AAA_SENSOR_MGR_OK;
 MINT32 u32Enable = 1;

    // Set Exposure time and FPS (waiting for lungbro)
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_TEST_PATTERN_OUTPUT, (MUINTPTR)&u32Enable, 0, 0);
        CAM_LOGD("[enableSensorTestPattern] SensorDev:%d \n", i4SensorDev);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_TEST_PATTERN_OUTPUT, (MUINTPTR)&u32Enable, 0, 0);
        CAM_LOGD("[enableSensorTestPattern] SensorDev:%d \n", i4SensorDev);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_TEST_PATTERN_OUTPUT, (MUINTPTR)&u32Enable, 0, 0);
        CAM_LOGD("[enableSensorTestPattern] SensorDev:%d \n", i4SensorDev);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_SET_TEST_PATTERN_OUTPUT, (MUINTPTR)&u32Enable, 0, 0);
        CAM_LOGD("[enableSensorTestPattern] SensorDev:%d \n", i4SensorDev);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    return (ret);
}

MUINT32 AAASensorMgr::getSensorBinningType(MINT32 i4SensorDev, NSCam::IHalSensor::ConfigParam sensorCfg)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4BinSumRatio = 1;

    if  (!m_pHalSensorObj) {
        CAM_LOGE("No Sensor object error");
        return u4BinSumRatio;
    }

    // Get default frame rate
    if(i4SensorDev == ESensorDev_Main) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_BINNING_TYPE, (MUINTPTR)&u4BinSumRatio, (MUINTPTR)&sensorCfg, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_BINNING_TYPE, (MUINTPTR)&u4BinSumRatio, (MUINTPTR)&sensorCfg, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_BINNING_TYPE, (MUINTPTR)&u4BinSumRatio, (MUINTPTR)&sensorCfg, 0);
    } else if(i4SensorDev == ESensorDev_SubSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB_2, SENSOR_CMD_GET_BINNING_TYPE, (MUINTPTR)&u4BinSumRatio, (MUINTPTR)&sensorCfg, 0);
    } else{
        CAM_LOGE("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }
    CAM_LOGD("[%d] i4SensorDev: %d, u4BinSumRatio: %d\n", __FUNCTION__, i4SensorDev, u4BinSumRatio);

    return u4BinSumRatio;
}

