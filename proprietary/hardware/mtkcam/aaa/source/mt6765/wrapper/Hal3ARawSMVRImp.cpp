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
#define LOG_TAG "Hal3ARawSMVR"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <IThreadRawSMVR.h>

#include "Hal3ARawImp.h"
#include "Hal3ARawSMVRImp.h"

// g/gyro sensor listener handler and data
static MINT32  gAcceInfo[3];
static MUINT64 gAcceTS;
static MBOOL   gAcceValid = MFALSE;
static MUINT64 gPreAcceTS;
static MINT32  gGyroInfo[3];
static MUINT64 gGyroTS;
static MBOOL   gGyroValid = MFALSE;
static MUINT64 gPreGyroTS;
static SensorListener* gpSensorListener=NULL;
static Mutex gCommonLock;

I3AWrapper*
Hal3ARawSMVR::
getInstance(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    return Hal3ARawSMVRImp::getInstance(i4SensorOpenIndex, i4SubsampleCount);
}

I3AWrapper*
Hal3ARawSMVRImp::
getInstance(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MUINT32 i4SensorDevId = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
        {
            static Hal3ARawSMVRImp _singleton(SENSOR_DEV_MAIN);
            _singleton.init(i4SensorOpenIndex, i4SubsampleCount);
            return &_singleton;
        }
        case SENSOR_DEV_SUB:
        {
            static Hal3ARawSMVRImp _singleton(SENSOR_DEV_SUB);
            _singleton.init(i4SensorOpenIndex, i4SubsampleCount);
            return &_singleton;
        }
        case SENSOR_DEV_MAIN_2:
        {
            static Hal3ARawSMVRImp _singleton(SENSOR_DEV_MAIN_2);
            _singleton.init(i4SensorOpenIndex, i4SubsampleCount);
            return &_singleton;
        }
        case SENSOR_DEV_SUB_2:
        {
            static Hal3ARawSMVRImp _singleton(SENSOR_DEV_SUB_2);
            _singleton.init(i4SensorOpenIndex, i4SubsampleCount);
            return &_singleton;
        }
        default:
            CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }
}

MVOID
Hal3ARawSMVRImp::
destroyInstance()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    uninit();
}

Hal3ARawSMVRImp::
Hal3ARawSMVRImp(MINT32 const i4SensorDevId)
    : Hal3ARawImp(i4SensorDevId)
    , m_i4InCaptureProcess(0)
    , m_i4SubsampleCount(1)
    , m_u4MagicNumber(0)
    , m_bIsHighQualityCaptureOn(MFALSE)
    , m_rGainDelay()
{
    CAM_LOGD("[%s] sensorDev(%d)", __FUNCTION__, i4SensorDevId);
}

MBOOL
Hal3ARawSMVRImp::
init(MINT32 i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    GET_PROP("vendor.debug.camera.log", 0, m_3ALogEnable);
    if ( m_3ALogEnable == 0 ) {
        GET_PROP("vendor.debug.camera.log.hal3a", 0, m_3ALogEnable);
    }
    m_DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // check user count
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_i4SensorIdx = i4SensorOpenIndex;
    m_i4SubsampleCount = i4SubsampleCount;
    m_rResultLastSof.i4FrmId = -1; //result havn't update yet
    m_fgEnableShadingMeta = MTRUE;

    // init Thread and state mgr
    m_pThreadRaw = IThreadRawSMVR::createInstance((Hal3ARaw*)this, m_i4SensorDev, m_i4SensorIdx);
    m_pStateMgr = new StateMgr(m_i4SensorDev);
    m_pAfStateMgr = new AfStateMgr(m_i4SensorDev);

    m_pStateMgr->setThreadRaw(m_pThreadRaw);

    // AE init
    CAM_TRACE_BEGIN("AE init");
    IAeMgr::getInstance().cameraPreviewInit(m_i4SensorDev, m_i4SensorIdx, m_rParam);
    CAM_TRACE_END();

    // AWB init
    CAM_TRACE_BEGIN("AWB init");
    IAwbMgr::getInstance().init(m_i4SensorDev, m_i4SensorIdx);
    CAM_TRACE_END();

    // AF init
#if CAM3_AF_FEATURE_EN
    CAM_TRACE_BEGIN("AF init");
    IAfMgr::getInstance().init(m_i4SensorDev, m_i4SensorIdx);
    CAM_TRACE_END();
#endif

    //FLASH init
#if CAM3_FLASH_FEATURE_EN
    CAM_TRACE_BEGIN("FLASH init");
    FlashMgr::getInstance().init(m_i4SensorDev, m_i4SensorIdx);
    FlashMgr::getInstance().hasFlashHw(m_i4SensorDev, m_i4HWSuppportFlash);
    CAM_LOGD("[%s] m_i4SensorDev(%d) m_i4HWSuppportFlash(%d)", __FUNCTION__, m_i4SensorDev, m_i4HWSuppportFlash);
    CAM_TRACE_END();
#endif

    //FLICKER init
#if CAM3_FLICKER_FEATURE_EN
    CAM_TRACE_BEGIN("FLICKER init");
    FlickerHalBase::getInstance().Init(m_i4SensorDev, m_i4SensorIdx);
    CAM_TRACE_END();
#endif

    // TuningMgr init
    if (m_pTuning == NULL)
    {
        CAM_TRACE_BEGIN("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx, i4SubsampleCount))
        {
            CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_3A_HAL("Fail to init IspTuningMgr");
            CAM_TRACE_END();
            return MFALSE;
        }
        CAM_TRACE_END();
    }

    CAM_TRACE_BEGIN("querySensorStaticInfo");
    //querySensorStaticInfo();
    CAM_TRACE_END();

    // state mgr transit to Init state.
    bRet = postCommand(ECmd_Init);
    if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");

    CAM_LOGD("[%s] done\n", __FUNCTION__);
    android_atomic_inc(&m_Users);
    return S_3A_OK;
}

MBOOL
Hal3ARawSMVRImp::
uninit()
{
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }
    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        bRet = postCommand(ECmd_Uninit);
        if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");
        m_i4SubsampleCount = 1;

        m_pThreadRaw->destroyInstance();

        MRESULT err = S_3A_OK;

        // AE uninit
        err = IAeMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)){
            CAM_LOGE("IAeMgr::getInstance().uninit() fail\n");
            return err;
        }

        // AWB uninit
        err = IAwbMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)){
            CAM_LOGE("IAwbMgr::getInstance().uninit() fail\n");
            return E_3A_ERR;
        }

#if CAM3_AF_FEATURE_EN
        // AF uninit
        err = IAfMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)) {
            CAM_LOGE("IAfMgr::getInstance().uninit() fail\n");
            return err;
        }
#endif

#if CAM3_FLASH_FEATURE_EN
        // FLASH uninit
        err = FlashMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)) {
            CAM_LOGE("FlashMgr::getInstance().uninit() fail\n");
            return err;
        }
#endif

#if CAM3_FLICKER_FEATURE_EN
        // FLICKER uninit
        err = FlickerHalBase::getInstance().Uninit(m_i4SensorDev);
        if (FAILED(err)) {
            CAM_LOGE("FlickerHalBase::getInstance().Uninit fail\n");
            return err;
        }
#endif

        // TuningMgr uninit
        if (m_pTuning)
        {
            m_pTuning->uninit(m_i4SensorDev);
            m_pTuning = NULL;
        }

        if (m_pCbSet)
        {
            m_pCbSet = NULL;
            CAM_LOGE("User did not detach callbacks!");
        }

        delete m_pStateMgr;
        m_pStateMgr = NULL;
        delete m_pAfStateMgr;
        m_pAfStateMgr = NULL;

        CAM_LOGD("[%s] done\n", __FUNCTION__);

    }
    else    // There are still some users.
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] Still %d users \n", __FUNCTION__, m_Users);
    }

    return S_3A_OK;
}

MINT32
Hal3ARawSMVRImp::
config(const ConfigInfo_T& rConfigInfo)
{
    CAM_LOGD("[%s]+ sensorDev(%d), Mode(%d)", __FUNCTION__, m_i4SensorDev, m_u4SensorMode);

    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
            return MFALSE;
        }
    }

    MUINT32 u4AAWidth, u4AAHight;
    MUINT32 u4AFWidth, u4AFHeight;
    MRESULT err = S_3A_OK;

    CAM_TRACE_BEGIN("3A SetSensorMode");
    // query input size info for AAO and FLKO
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_HBIN_INFO,
                        (MINTPTR)&u4AAWidth, (MINTPTR)&u4AAHight, 0);
    m_i4HbinWidth = u4AAWidth;
    m_i4HbinHeight= u4AAHight;

    // query input size info for AFO
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO,
                        (MINTPTR)&u4AFWidth, (MINTPTR)&u4AFHeight, 0);
    CAM_LOGD("[%s] AAWH(%dx%d), AFWH(%dx%d)", __FUNCTION__, u4AAWidth, u4AAHight, u4AFWidth, u4AFHeight);

    // update HBIN and BIN info to AE/AWB/AF/FLICKER

    // set sensor mode to 3A modules
    IAeMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AAWidth, u4AAHight);
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight, u4AAWidth, u4AAHight);
    NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setSensorMode(
        static_cast<ESensorMode_T>(m_u4SensorMode), u4AFWidth, u4AFHeight, MFALSE);

#if CAM3_AF_FEATURE_EN
    IAfMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight);
#endif

#if CAM3_FLICKER_FEATURE_EN
    FlickerHalBase::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AAWidth, u4AAHight);
#endif


    // FIXME (remove): update TG Info to 3A modules
    Hal3ARawImp::updateTGInfo();

    //Frontal Binning
    MBOOL fgFrontalBin;
    fgFrontalBin = (m_i4TgWidth == (MINT32)u4AFWidth && m_i4TgHeight == (MINT32)u4AFHeight) ? MFALSE : MTRUE;

    m_pTuning->setSensorMode(m_i4SensorDev, m_u4SensorMode, fgFrontalBin, u4AFWidth, u4AFHeight);
    m_pTuning->setIspProfile(m_i4SensorDev, NSIspTuning::EIspProfile_Preview);
    CAM_TRACE_END();

    // AE start
    CAM_TRACE_BEGIN("AE Start");
    MBOOL bAAOMode;
    if(rConfigInfo.i4BitMode == EBitMode_12Bit)
        bAAOMode = 0;
    else if(rConfigInfo.i4BitMode == EBitMode_14Bit)
        bAAOMode = 1;
    else
    {
        CAM_LOGE("Not support BitMode(%d)", rConfigInfo.i4BitMode);
        return err;
    }
    IAeMgr::getInstance().setAAOMode(m_i4SensorDev, bAAOMode);
    IAeMgr::getInstance().setAESMBuffermode(m_i4SensorDev, MTRUE, m_i4SubsampleCount);
    err = IAeMgr::getInstance().Start(m_i4SensorDev);
    CAM_TRACE_END();
    IAeMgr::getInstance().setCamScenarioMode(m_i4SensorDev, CAM_SCENARIO_PREVIEW);
    if (FAILED(err)) {
    CAM_LOGE("IAeMgr::getInstance().Start() fail\n");
        return err;
    }

    // AWB start
    CAM_TRACE_BEGIN("AWB Start");
    err = IAwbMgr::getInstance().start(m_i4SensorDev);
    CAM_TRACE_END();
    IAwbMgr::getInstance().setCamScenarioMode(m_i4SensorDev, CAM_SCENARIO_PREVIEW);
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().start() fail\n");
        return E_3A_ERR;
    }

#if CAM3_FLASH_FEATURE_EN
    // Flash cameraPreviewStart + start
    CAM_TRACE_BEGIN("FLASH Start");
    err = FlashMgr::getInstance().cameraPreviewStart(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().cameraPreviewStart() fail\n");
        CAM_TRACE_END();
        return E_3A_ERR;
    }
    err = FlashMgr::getInstance().start(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().start() fail\n");
        CAM_TRACE_END();
        return E_3A_ERR;
    }
    CAM_TRACE_END();
#endif

#if CAM3_AF_FEATURE_EN
    // AF start
    IAfMgr::getInstance().setCamScenarioMode(m_i4SensorDev, CAM_SCENARIO_PREVIEW);
    CAM_TRACE_BEGIN("AF Start");
    err = IAfMgr::getInstance().Start(m_i4SensorDev);
    CAM_TRACE_END();
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Start() fail\n");
        return err;
    }
#endif
    MBOOL enable_flk = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isFlkEnable();
    CAM_LOGD("[%s] enable_flk=%d", __FUNCTION__, (int)enable_flk);
#if CAM3_FLICKER_FEATURE_EN
    // Set Uni status for flicker enable
    FlickerHalBase::getInstance().setUniInfo(m_i4SensorDev,enable_flk);
    // Flicker start
    // AAASensorMgr::getInstance().setFlickerFrameRateActive(m_i4SensorDev, 1);
    CAM_TRACE_BEGIN("FLICKER Start");
    err = FlickerHalBase::getInstance().Start(m_i4SensorDev);
    if (FAILED(err)) {
        CAM_LOGE("FlickerHalBase::getInstance().Start() fail\n");
        return err;
    }
    CAM_TRACE_END();
#endif

    CAM_LOGD("[%s] m_i4SubsampleCount(%d)", __FUNCTION__, m_i4SubsampleCount);

    // apply 3A module's config
    MINT32 i4Num;
    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (i4Num = 1; i4Num <= m_i4SubsampleCount; i4Num++)
    {
        rRequestSet.vNumberSet.push_back(i4Num);
    }
    IspTuningMgr::getInstance().setFlkEnable(m_i4SensorDev, enable_flk);
    m_pTuning->notifyRPGEnable(m_i4SensorDev, MTRUE);   // apply awb gain for init stat
    IspTuningMgr::getInstance().validate(m_i4SensorDev, rRequestSet, MTRUE);

    CAM_LOGD("[%s]-", __FUNCTION__);

    return S_3A_OK;
}

MBOOL
Hal3ARawSMVRImp::
start()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    MRESULT isAFLampOn = MFALSE;
    m_i4InCaptureProcess = 0;
    m_u4MagicNumber = 0;
    m_bPreStop = MFALSE;

#if CAM3_AF_FEATURE_EN
    if(IAfMgr::getInstance().isAFSupport(m_i4SensorDev) > 0)
    {
    // enable AF thread
    m_pThreadRaw->enableAFThread(m_pAfStateMgr);
    }
#endif

    m_pThreadRaw->createAEThread();

#if CAM3_FLASH_FEATURE_EN
    isAFLampOn = FlashMgr::getInstance().isAFLampOn(m_i4SensorDev);
#endif

    // setStrobeMode
    IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? MTRUE : MFALSE);
    IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);

    IAeMgr::getInstance().getAEdelayInfo(m_i4SensorDev, (MINT32*)&m_rGainDelay.u4SutterGain_Delay, (MINT32*)&m_rGainDelay.u4SensorGain_Delay, (MINT32*)&m_rGainDelay.u4ISPGain_Delay);
    CAM_LOGD("[%s] delay frame, SutterGain_Delay:(%d) SensorGain_Delay(%d) ISPGain_Delay(%d)",
        __FUNCTION__, m_rGainDelay.u4SutterGain_Delay, m_rGainDelay.u4SensorGain_Delay, m_rGainDelay.u4ISPGain_Delay);

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
stop()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    MRESULT err = S_3A_OK;
    m_u4MagicNumber = 0;
    m_i4SubsampleCount = 1;
    // AE stop
    CAM_TRACE_BEGIN("AE Stop");
    IAeMgr::getInstance().setAESMBuffermode(m_i4SensorDev, MFALSE, m_i4SubsampleCount);
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, 30000, 30000);    // Prevent screen dimming issue after stop recording SloMo Video while AE is locked : ALPS03453598
    err = IAeMgr::getInstance().Stop(m_i4SensorDev);
    CAM_TRACE_END();
    if (FAILED(err)) {
    CAM_LOGE("IAeMgr::getInstance().Stop() fail\n");
        return err;
    }
    m_pThreadRaw->destroyAEThread();

    // AWB stop
    CAM_TRACE_BEGIN("AWB Stop");
    err = IAwbMgr::getInstance().stop(m_i4SensorDev);
    CAM_TRACE_END();
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().stop() fail\n");
        return E_3A_ERR;
    }

#if CAM3_FLASH_FEATURE_EN
    // Flash cameraPreviewEnd + stop
    CAM_TRACE_BEGIN("Flash Stop");
    if(FlashMgr::getInstance().isAFLampOn(m_i4SensorDev))
    {
        CAM_LOGD("[%s] Flash mode is OFF, but Flash is still ON", __FUNCTION__);
        if (!m_pStateMgr->getIsFlashOpened())
            IAeMgr::getInstance().doRestoreAEInfo(m_i4SensorDev, MTRUE);

        FlashMgr::getInstance().setAFLampOnOff(m_i4SensorDev, MFALSE);
    }

    err = FlashMgr::getInstance().cameraPreviewEnd(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().cameraPreviewEnd() fail\n");
        CAM_TRACE_END();
        return E_3A_ERR;
    }
    err = FlashMgr::getInstance().stop(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().stop() fail\n");
        CAM_TRACE_END();
        return E_3A_ERR;
    }
    CAM_TRACE_END();
#endif

#if CAM3_AF_FEATURE_EN
    // AF stop
    CAM_TRACE_BEGIN("AF Stop");
    err = IAfMgr::getInstance().Stop(m_i4SensorDev);
    CAM_TRACE_END();
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Stop() fail\n");
        return err;
    }
    // disable AF thread
    CAM_TRACE_BEGIN("AF THREAD disable");
    m_pThreadRaw->disableAFThread();
    CAM_TRACE_END();
#endif

#if CAM3_FLICKER_FEATURE_EN
    // Flicker close
    CAM_TRACE_BEGIN("Flicker Stop");
    FlickerHalBase::getInstance().Stop(m_i4SensorDev);
    CAM_TRACE_END();
#endif

    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL)
    {
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }

    ::IPDTblGen::getInstance()->stop(m_i4SensorDev);

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pTuningBuf, ResultP2_T* pResultP2)
{
    Mutex::Autolock lock(mP2Mtx);

    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    CAM_LOGD_IF(m_3ALogEnable, "[%s] + flow(%d), buf(%p)", __FUNCTION__, flowType, pRegBuf);
    IspTuningMgr::getInstance().validatePerFrameP2(m_i4SensorDev, flowType, rIspInfo, pTuningBuf);
#if CAM3_LSC_FEATURE_EN
    ILscBuf* pLscBuf = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getP2Buf();
    if (pLscBuf)
        ((TuningParam*)pTuningBuf)->pLsc2Buf = pLscBuf->getBuf();
    else
        ((TuningParam*)pTuningBuf)->pLsc2Buf = NULL;
#endif

    if (!((dip_x_reg_t*)pRegBuf)->DIP_X_CTL_RGB_EN.Bits.LCE_EN){
        ((TuningParam*)pTuningBuf)->pLcsBuf = NULL;
    }
    ((TuningParam*)pTuningBuf)->pBpc2Buf = IspTuningMgr::getInstance().getDMGItable(m_i4SensorDev);

    // debug info
    if (pResultP2)
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] get debug info p2 #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
        if (0 == pResultP2->vecDbgIspP2.size())
        {
            CAM_LOGD_IF(m_3ALogEnable, "[%s] Need to allocate P2 result", __FUNCTION__);
            pResultP2->vecDbgIspP2.resize(sizeof(AAA_DEBUG_INFO2_T));
        }
        AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pResultP2->vecDbgIspP2.editArray());
        NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo = rDbg3AInfo2.rISPDebugInfo;
        IspTuningMgr::getInstance().getDebugInfo(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, MFALSE);
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] sensor(%d) #(%d) profile(%d) rpg(%d)", __FUNCTION__, m_i4SensorDev, rParamIspProfile.i4MagicNum, rParamIspProfile.eIspProfile, rParamIspProfile.iEnableRPG);
    m_pTuning->setIspProfile(m_i4SensorDev, rParamIspProfile.eIspProfile);
    m_pTuning->notifyRPGEnable(m_i4SensorDev, rParamIspProfile.iEnableRPG);
    //isp tuning query here
    m_pTuning->validatePerFrame(m_i4SensorDev, rParamIspProfile.rRequestSet, fgPerframe, (MINT32)m_rGainDelay.u4ISPGain_Delay);

    ISP_LCS_IN_INFO_T lcs_info;
    m_pTuning->getLCSparam(m_i4SensorDev, lcs_info);
    lcs_info.i4FrmId = rParamIspProfile.i4MagicNum;
    m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_LCS_ISP_PARAMS, rParamIspProfile.i4MagicNum, (MINTPTR)&lcs_info, 0);

    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
setParams(Param_T const &rNewParam, MBOOL bUpdateScenario)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(m3AOperMtx2);

    if (!m_bEnable3ASetParams) return MTRUE;

#if CAM3_LSC_FEATURE_EN
    // ====================================== Shading =============================================
    if (m_fgEnableShadingMeta)
        NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(rNewParam.u1ShadingMode ? MTRUE : MFALSE);
#endif

    // ====================================== ISP tuning =============================================
    IspTuningMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode);
    IspTuningMgr::getInstance().setEffect(m_i4SensorDev, rNewParam.u4EffectMode);
/*    IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewParam.i4BrightnessMode);
    IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewParam.i4HueMode);
    IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewParam.i4SaturationMode);
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewParam.i4EdgeMode);
    IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewParam.i4ContrastMode);*/
    IspTuningMgr::getInstance().setEdgeMode(m_i4SensorDev, rNewParam.u1EdgeMode ? MTK_EDGE_MODE_FAST : MTK_EDGE_MODE_OFF);
    IspTuningMgr::getInstance().setNoiseReductionMode(m_i4SensorDev, rNewParam.u1NRMode);
    IspTuningMgr::getInstance().setZoomRatio(m_i4SensorDev, rNewParam.i4ZoomRatio);

    /*
    IspTuningMgr::getInstance().setToneMapMode(m_i4SensorDev, rNewParam.u1TonemapMode);
    if (rNewParam.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        android::Vector<MFLOAT> vecIn, vecOut;
        MINT32 i4Cnt = rNewParam.vecTonemapCurveBlue.size() / 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        MFLOAT* pArrayIn = vecIn.editArray();
        MFLOAT* pArrayOut = vecOut.editArray();
        const MFLOAT* pCurve = rNewParam.vecTonemapCurveBlue.array();
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            CAM_LOGD_IF(m_3ALogEnable, "[Blue]#%d(%f,%f)", rNewParam.u4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Blue(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);

        i4Cnt = rNewParam.vecTonemapCurveGreen.size() / 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = vecIn.editArray();
        pArrayOut = vecOut.editArray();
        pCurve = rNewParam.vecTonemapCurveGreen.array();
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            CAM_LOGD_IF(m_3ALogEnable, "[Green]#%d(%f,%f)", rNewParam.u4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Green(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);

        i4Cnt = rNewParam.vecTonemapCurveRed.size() / 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = vecIn.editArray();
        pArrayOut = vecOut.editArray();
        pCurve = rNewParam.vecTonemapCurveRed.array();
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            CAM_LOGD_IF(m_3ALogEnable, "[Red]#%d(%f,%f)", rNewParam.u4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Red(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);
    }
    */
    // ====================================== AE ==============================================
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps);
    IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev, rNewParam.u4AeMeterMode);
    IAeMgr::getInstance().setAERotateDegree(m_i4SensorDev, rNewParam.i4RotateDegree);
    IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.i4IsoSpeedMode);
    IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas);
    if (bUpdateScenario)
        IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock);
    IAeMgr::getInstance().setAEEVCompIndex(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep);
    IAeMgr::getInstance().setAEMode(m_i4SensorDev, rNewParam.u4AeMode);
    IAeMgr::getInstance().setAEFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);
    IAeMgr::getInstance().setAECamMode(m_i4SensorDev, rNewParam.u4CamMode);
    IAeMgr::getInstance().setAEShotMode(m_i4SensorDev, rNewParam.u4ShotMode);
    IAeMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode);
    IAeMgr::getInstance().bBlackLevelLock(m_i4SensorDev, rNewParam.u1BlackLvlLock);
    if( rNewParam.rScaleCropRect.i4Xoffset != 0 && rNewParam.rScaleCropRect.i4Yoffset != 0 &&
        rNewParam.rScaleCropRect.i4Xwidth  != 0 && rNewParam.rScaleCropRect.i4Yheight != 0 )
        IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewParam.rScaleCropRect.i4Xoffset,rNewParam.rScaleCropRect.i4Yoffset,rNewParam.rScaleCropRect.i4Xwidth,rNewParam.rScaleCropRect.i4Yheight);
    IAeMgr::getInstance().setAEHDRMode(m_i4SensorDev, rNewParam.u1HdrMode);

#if CAM3_STEREO_FEATURE_EN
        m_sParam = ScenarioParam( rNewParam.u1CaptureIntent
                                , rNewParam.u1HdrMode
                                , (ISync3AMgr::getInstance()->getStereoParams()).i4Sync2AMode
                                , m_i4AETargetMode
                                , m_u4SensorMode);
#else
        m_sParam = ScenarioParam( rNewParam.u1CaptureIntent
                                , rNewParam.u1HdrMode
                                , 0
                                , m_i4AETargetMode
                                , m_u4SensorMode);
#endif

    if (bUpdateScenario){
            IAwbMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AWB(m_sParam));
            IAfMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AF(m_sParam));
            IAeMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AE(m_sParam));
    }

    if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
    {
        AE_SENSOR_PARAM_T strSensorParams;
        strSensorParams.u4Sensitivity   = rNewParam.i4Sensitivity;
        strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTime;
        strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
        IAeMgr::getInstance().UpdateSensorParams(m_i4SensorDev, strSensorParams);
    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] setAEMode(%d)", __FUNCTION__, rNewParam.u4AeMode);
    m_pStateMgr->updateMaxFrameRate(rNewParam.i4MaxFps);

    // ====================================== AWB ==============================================
    IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock);
    IAwbMgr::getInstance().setAWBMode(m_i4SensorDev, rNewParam.u4AwbMode);
    if( rNewParam.rScaleCropRect.i4Xoffset != 0 && rNewParam.rScaleCropRect.i4Yoffset != 0 &&
        rNewParam.rScaleCropRect.i4Xwidth  != 0 && rNewParam.rScaleCropRect.i4Yheight != 0 )
        IAwbMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewParam.rScaleCropRect.i4Xoffset,rNewParam.rScaleCropRect.i4Yoffset,rNewParam.rScaleCropRect.i4Xwidth,rNewParam.rScaleCropRect.i4Yheight);

    IAwbMgr::getInstance().setColorCorrectionMode(m_i4SensorDev, rNewParam.u1ColorCorrectMode);
    IspTuningMgr::getInstance().setColorCorrectionMode(m_i4SensorDev, rNewParam.u1ColorCorrectMode);
    if (rNewParam.u4AwbMode == MTK_CONTROL_AWB_MODE_OFF &&
        rNewParam.u1ColorCorrectMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        IAwbMgr::getInstance().setColorCorrectionGain(m_i4SensorDev, rNewParam.fColorCorrectGain[0], rNewParam.fColorCorrectGain[1], rNewParam.fColorCorrectGain[3]);
        IspTuningMgr::getInstance().setColorCorrectionTransform(m_i4SensorDev,
            rNewParam.fColorCorrectMat[0], rNewParam.fColorCorrectMat[1], rNewParam.fColorCorrectMat[2],
            rNewParam.fColorCorrectMat[3], rNewParam.fColorCorrectMat[4], rNewParam.fColorCorrectMat[5],
            rNewParam.fColorCorrectMat[6], rNewParam.fColorCorrectMat[7], rNewParam.fColorCorrectMat[8]);
    }

#if CAM3_FLASH_FEATURE_EN
    // ====================================== Flash ==============================================
    FlashMgr::getInstance().setAeFlashMode(m_i4SensorDev, rNewParam.u4AeMode, rNewParam.u4StrobeMode);
    int bMulti;
    if(rNewParam.u4CapType == ECapType_MultiCapture)
        bMulti=1;
    else
        bMulti=0;
    FlashMgr::getInstance().setCamMode(m_i4SensorDev, rNewParam.u4CamMode);
    FlashMgr::getInstance().setEvComp(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep);
#endif

    if(rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD || rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT) {
        if((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_PREVIEW || m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG) && isStrobeBVTrigger() && m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH)
            m_bIsRecordingFlash = MTRUE;
    } else {
        m_bIsRecordingFlash = MFALSE;
    }
#if CAM3_FLICKER_FEATURE_EN
    // ====================================== Flicker ==============================================
    FlickerHalBase::getInstance().setFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);
#endif

    // ====================================== FlowCtrl ==============================================
    m_rParam = rNewParam;

    CAM_LOGD_IF(m_3ALogEnable, "[%s] m_rParam.u1ShadingMapMode(%d)", __FUNCTION__, m_rParam.u1ShadingMapMode);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
setFlashLightOnOff(MBOOL bOnOff, MBOOL bMainPre)
{
#if CAM3_FLASH_FEATURE_EN
    m_pStateMgr->setIsFlashOpened(bOnOff);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    if (!bOnOff)
    {
        if (m_i4InCaptureProcess)
        {
            //modified to update strobe state after capture for ae/flash manager
            FlashMgr::getInstance().cameraPreviewStart(m_i4SensorDev);
            IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, MFALSE);
            m_i4InCaptureProcess = 0;
        }
        FlashMgr::getInstance().setCaptureFlashOnOff(m_i4SensorDev, 0);
    }
    else //flash on
    {
        if (bMainPre) FlashMgr::getInstance().setCaptureFlashOnOff(m_i4SensorDev, 1);
        else FlashMgr::getInstance().setTorchOnOff(m_i4SensorDev, 1);
    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
chkPreFlashOnCond() const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] IsRecordingFlash(%d)", __FUNCTION__, m_bIsRecordingFlash);
    MBOOL bChkFlash;
    bChkFlash = (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH))
                ||
                m_bIsRecordingFlash;

    return bChkFlash && (m_i4HWSuppportFlash != 0) && (FlashMgr::getInstance().getFlashState(m_i4SensorDev) != MTK_FLASH_STATE_CHARGING);//shouldn't return MTRUE if Hw does not support flash
}

MINT32
Hal3ARawSMVRImp::
getCurrResult(MUINT32 i4FrmId, Result_T& rResult, MINT32 i4SubsampleIndex) const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] + i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState;
    rResult.i4FrmId = i4FrmId;
    MINT32 i4NeedUpdateResult = 0;

    // AE
    MUINT8 u1AeState = IAeMgr::getInstance().getAEState(m_i4SensorDev);
    if (IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev) && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rResult.u1AeState = u1AeState;

    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    if (rResult.i8SensorExposureTime != rAESensorInfo.u8ExposureTime && (i4SubsampleIndex >= (MINT32)m_rGainDelay.u4SutterGain_Delay))
    {
        rResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
        i4NeedUpdateResult++;
    }
    if (rResult.i4SensorSensitivity != rAESensorInfo.u4Sensitivity && (i4SubsampleIndex >= (MINT32)m_rGainDelay.u4SensorGain_Delay))
    {
        rResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
        i4NeedUpdateResult++;
    }

    rResult.fgAeBvTrigger = IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);

    if (m_rParam.u1RollingShutterSkew)
        rResult.i8SensorRollingShutterSkew = IAeMgr::getInstance().getSensorRollingShutter(m_i4SensorDev);
    rResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;

#if CAM3_AF_FEATURE_EN
    // AF
    //if (m_pStateMgr->getStateStatus().eCurrState == eState_AF)  // CTS verified later
        //rResult.u1AfState = MTK_CONTROL_AF_STATE_ACTIVE_SCAN;
    if (rResult.u1AfState != IAfMgr::getInstance().getAFState(m_i4SensorDev))
    {
        CAM_LOGD_IF(m_3ALogEnable,"[%s] Afstate change from (%d) -> (%d)", __FUNCTION__, rResult.u1AfState, IAfMgr::getInstance().getAFState(m_i4SensorDev));
        rResult.u1AfState = IAfMgr::getInstance().getAFState(m_i4SensorDev);
        i4NeedUpdateResult++;
    }
    IAfMgr::getInstance().getFocusAreaResult(m_i4SensorDev, rResult.vecFocusAreaPos, rResult.vecFocusAreaRes, rResult.i4FocusAreaSz[0], rResult.i4FocusAreaSz[1]);

    rResult.u1LensState = IAfMgr::getInstance().getLensState(m_i4SensorDev);
    rResult.fLensFocusDistance = IAfMgr::getInstance().getFocusDistance(m_i4SensorDev);
    IAfMgr::getInstance().getFocusRange(m_i4SensorDev, &rResult.fLensFocusRange[0], &rResult.fLensFocusRange[1]);
#endif

    // Cam Info
    if (!IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rResult.rCamInfo))
    {
        CAM_LOGE("Fail to get CamInfo");
    }

    rResult.rCamInfo.bBypassLCE = MTRUE;

    if (i4NeedUpdateResult > 0)
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] need update result(%d)", __FUNCTION__, i4FrmId);
        return E_Repeat_OFF;
    }
    if (i4SubsampleIndex > 0) //if index = 0, update result
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] don't need update result(%d)", __FUNCTION__, i4FrmId);
        return E_Repeat_ON;
    } else {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] need update result(%d)", __FUNCTION__, i4FrmId);
    }

    // clear memory
    rResult.vecExifInfo.clear();
    rResult.vecTonemapCurveRed.clear();
    rResult.vecTonemapCurveGreen.clear();
    rResult.vecTonemapCurveBlue.clear();
    rResult.vecColorCorrectMat.clear();
    rResult.vecDbg3AInfo.clear();
    rResult.vecDbgShadTbl.clear();

    rResult.u1SceneMode = m_rParam.u4SceneMode;


    // AWB
    IAwbMgr::getInstance().getAWBState(m_i4SensorDev, eAwbState);
    rResult.u1AwbState= eAwbState;
    AWB_GAIN_T rAwbGain;
    IAwbMgr::getInstance().getAWBGain(m_i4SensorDev, rAwbGain, rResult.i4AwbGainScaleUint);
    rResult.i4AwbGain[0] = rAwbGain.i4R;
    rResult.i4AwbGain[1] = rAwbGain.i4G;
    rResult.i4AwbGain[2] = rAwbGain.i4B;
    IAwbMgr::getInstance().getColorCorrectionGain(m_i4SensorDev, rResult.fColorCorrectGain[0],rResult.fColorCorrectGain[1],rResult.fColorCorrectGain[3]);
    rResult.fColorCorrectGain[2] = rResult.fColorCorrectGain[1];
    if (m_rParam.u1ColorCorrectMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        rResult.vecColorCorrectMat.resize(9);
        MFLOAT* pfColorCorrectMat = rResult.vecColorCorrectMat.editArray();
    IspTuningMgr::getInstance().getColorCorrectionTransform(m_i4SensorDev,
            pfColorCorrectMat[0], pfColorCorrectMat[1], pfColorCorrectMat[2],
            pfColorCorrectMat[3], pfColorCorrectMat[4], pfColorCorrectMat[5],
            pfColorCorrectMat[6], pfColorCorrectMat[7], pfColorCorrectMat[8]
    );
    }

#if CAM3_FLASH_FEATURE_EN
    // Flash
    //rResult.u1FlashState = FlashMgr::getInstance()->getFlashState();
#if 1 //mark out temporarily
    rResult.u1FlashState =
        (FlashMgr::getInstance().getFlashState(m_i4SensorDev) == MTK_FLASH_STATE_UNAVAILABLE) ?
        MTK_FLASH_STATE_UNAVAILABLE :
        (FlashMgr::getInstance().getFlashState(m_i4SensorDev) == MTK_FLASH_STATE_CHARGING) ?
        MTK_FLASH_STATE_CHARGING :
        (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev) ? MTK_FLASH_STATE_FIRED : MTK_FLASH_STATE_READY);
#endif
#endif

#if CAM3_FLICKER_FEATURE_EN
    // Flicker
    MINT32 i4FlkStatus = IAeMgr::getInstance().getAEAutoFlickerState(m_i4SensorDev);
    //FlickerHalBase::getInstance().getFlickerResult(i4FlkStatus);
    MUINT8 u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_NONE;
    if (i4FlkStatus == 0) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_50HZ;
    if (i4FlkStatus == 1) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_60HZ;
    rResult.u1SceneFlk = (MUINT8)u1ScnFlk;
#endif

#if CAM3_LSC_FEATURE_EN
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
    MBOOL fgRequireShadingIntent = 1; /*
        (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG);*/
    rResult.vecLscData.clear();
    if ((pLsc->getOnOff() && fgRequireShadingIntent && m_rParam.i4RawType == NSIspTuning::ERawType_Pure /*IMGO pure raw*/) ||
        (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON))
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] #(%d) LSC table to metadata", __FUNCTION__, i4FrmId);
        pLsc->getCurrTbl(rResult.vecLscData);
    }
    rResult.fgShadingMapOn = (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON);
#if 0
    // Shading map
    if (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON)
    {
        rResult.vecShadingMap.resize(m_rParam.u1ShadingMapXGrid*m_rParam.u1ShadingMapYGrid*4);
        MFLOAT* pfShadingMap = rResult.vecShadingMap.editArray();
        NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getGainTable(3, m_rParam.u1ShadingMapXGrid, m_rParam.u1ShadingMapYGrid, pfShadingMap);
    }
#endif
#endif

    // Tonemap
    if (m_rParam.u1TonemapMode != MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        MFLOAT *pIn, *pOut;
        MINT32 i4NumPt;
        IspTuningMgr::getInstance().getTonemapCurve_Blue(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            rResult.vecTonemapCurveBlue.push_back(*pIn++);
            rResult.vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            rResult.vecTonemapCurveGreen.push_back(*pIn++);
            rResult.vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            rResult.vecTonemapCurveRed.push_back(*pIn++);
            rResult.vecTonemapCurveRed.push_back(*pOut++);
        }
    }

    // Exif
    if (m_rParam.u1IsGetExif)
    {
        rResult.vecExifInfo.resize(1);
        Hal3ARawImp::get3AEXIFInfo(rResult.vecExifInfo.editTop());
        // debug exif
        rResult.vecDbg3AInfo.resize(sizeof(AAA_DEBUG_INFO1_T));
        rResult.vecDbgIspInfo.resize(sizeof(AAA_DEBUG_INFO2_T));
        rResult.vecDbgShadTbl.resize(sizeof(DEBUG_SHAD_ARRAY_INFO_T));
        AAA_DEBUG_INFO1_T& rDbg3AInfo = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(rResult.vecDbg3AInfo.editArray());
        AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rResult.vecDbgIspInfo.editArray());
        DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl = *reinterpret_cast<DEBUG_SHAD_ARRAY_INFO_T*>(rResult.vecDbgShadTbl.editArray());
        getP1DbgInfo(rDbg3AInfo, rDbgShadTbl, rDbgIspInfo);
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s] - i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return E_Repeat_OFF;
}

MBOOL
Hal3ARawSMVRImp::
postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]+ cmd(%d)", __FUNCTION__, r3ACmd);
    // update Gyro sensor info
    if(gAcceValid && gGyroValid)
    {
        GyroSensor_Param_T rGyroSensorParam;
        memcpy(rGyroSensorParam.i4AcceInfo, gAcceInfo, sizeof(MINT32) * 3);
        memcpy(rGyroSensorParam.i4GyroInfo, gGyroInfo, sizeof(MINT32) * 3);
        rGyroSensorParam.u8AcceTS = gAcceTS;
        rGyroSensorParam.u8GyroTS = gGyroTS;

        IAeMgr::getInstance().updateSensorListenerParams(m_i4SensorDev, (MINT32*)&rGyroSensorParam);
        IAfMgr::getInstance().updateSensorListenerParams(m_i4SensorDev, (MINT32*)&rGyroSensorParam);
    }
#if CAM3_AF_FEATURE_EN
    // for AF state
    switch(r3ACmd)
    {
        case ECmd_Init:
        case ECmd_Uninit:
        case ECmd_CameraPreviewStart:
        case ECmd_CameraPreviewEnd:
            m_pAfStateMgr->sendCmd(r3ACmd);
        default:
            break;
    }
#endif
    CAM_LOGD_IF(m_3ALogEnable, "AF command(%d) done.", r3ACmd);

    // for 3A state

    // back next state when previewEnd or uninit command on AF state.
    if(r3ACmd == ECmd_CameraPreviewEnd || r3ACmd == ECmd_Uninit)
    {
        if(m_pStateMgr->getStateStatus().eCurrState == eState_AF)
        {
            m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
            if(m_pStateMgr->getStateStatus().eNextState!=eState_Invalid)
            {
                m_pStateMgr->transitState(eState_AF, m_pStateMgr->getStateStatus().eNextState);
                m_pStateMgr->setNextState(eState_Invalid);
            }
            else
                m_pStateMgr->transitState(eState_AF, m_pStateMgr->getStateStatus().ePrevState);
        }
    }
    if (r3ACmd == ECmd_Update)
    {
        if(m_pStateMgr != NULL)
        {
             m_pStateMgr->updateMagicNumberRequest(pParam->i4MagicNum);
             CAM_LOGD_IF(m_3ALogEnable,"[HQC] updateMagicNumberRequest(%d)", pParam->i4MagicNum);
             m_bIsHighQualityCaptureOn = MFALSE;
        }
        if (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD)
            m_pStateMgr->mAFStateCntSet.bIsRecording = MTRUE; //notify af_state_mgr recoding is ongoing, when state transit to AF state, touch AF won't flash
    }
    // capture flow.
    if ((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) && (r3ACmd == ECmd_Update) && (m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF))
    {
        if (m_pStateMgr->getStateStatus().eCurrState == eState_Precapture)
        {
            CAM_LOGD("Unexpected Operation since precapture is not finished.");
            m_pStateMgr->sendCmd(ECmd_PrecaptureEnd);
        }

#if CAM3_FLASH_FEATURE_EN
        if(!pParam->bMainFlash && !mbIsHDRShot)
#endif
        {
            AE_MODE_CFG_T previewInfo;
            IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);
            IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, previewInfo);
        }
        if (mbIsHDRShot)
        {
            mbIsHDRShot = MFALSE;
        }

        {
            m_i4InCaptureProcess = 1;
            m_pStateMgr->sendCmd(ECmd_CaptureStart);
        }

        if(pParam->bMainFlash)
        {
            MINT32 i4MagicNumbeRequest;
            i4MagicNumbeRequest = m_pStateMgr->queryMagicNumberRequest();
            m_pStateMgr->updateCapQueue(i4MagicNumbeRequest);
            m_bIsHighQualityCaptureOn = MTRUE;
            CAM_LOGD_IF(m_3ALogEnable,"[HQC] i4MagicNumbeRequest = %d", i4MagicNumbeRequest);
        }
    }
    else m_pStateMgr->sendCmd(r3ACmd);
    CAM_LOGD_IF(m_3ALogEnable,"3A command done.");

    // query magic number after deque sw buffer.
    m_u4MagicNumber = m_pStateMgr->queryMagicNumber();
    CAM_LOGD_IF(m_3ALogEnable,"[%s] m_u4MagicNumber = %d",__FUNCTION__, m_u4MagicNumber);

    CAM_TRACE_FMT_BEGIN("P1_VLD");
    if (ECmd_Update == r3ACmd)
    {
        if (m_pStateMgr->getFrameCount() >= 0)
        {
#if CAM3_FLASH_FEATURE_EN
            FlashMgr::getInstance().doPreviewOneFrame(m_i4SensorDev);
#endif
            validateP1(*pParam, MTRUE);

        }
    }
    CAM_TRACE_FMT_END();
    CAM_LOGD_IF(m_3ALogEnable, "[%s]-", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const
{
    // AE AF Flash Flicker Shading debug info
    rDbg3AInfo1.hdr.u4KeyID = AAA_DEBUG_KEYID;
    rDbg3AInfo1.hdr.u4ModuleCount = MODULE_NUM(5,5);

    rDbg3AInfo1.hdr.u4AEDebugInfoOffset        = sizeof(rDbg3AInfo1.hdr);
    rDbg3AInfo1.hdr.u4AFDebugInfoOffset        = rDbg3AInfo1.hdr.u4AEDebugInfoOffset + sizeof(AE_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4FlashDebugInfoOffset     = rDbg3AInfo1.hdr.u4AFDebugInfoOffset + sizeof(AF_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4FlickerDebugInfoOffset   = rDbg3AInfo1.hdr.u4FlashDebugInfoOffset + sizeof(FLASH_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4ShadingDebugInfoOffset   = rDbg3AInfo1.hdr.u4FlickerDebugInfoOffset + sizeof(FLICKER_DEBUG_INFO_T);

    // AE
    AE_PLINE_DEBUG_INFO_T temp_AEPlineDebugInfo;
    IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo1.rAEDebugInfo, temp_AEPlineDebugInfo);

#if CAM3_AF_FEATURE_EN
    // AF
    IAfMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo1.rAFDebugInfo);
#endif

#if CAM3_FLASH_FEATURE_EN
    // Flash
    FlashMgr::getInstance().getDebugInfo(m_i4SensorDev, &rDbg3AInfo1.rFlashDebugInfo);
#endif

    // Flicker
    //Flicker::getInstance()->getDebugInfo(&rDbg3AInfo1.rFlickerDebugInfo);

#if CAM3_LSC_FEATURE_EN
    // LSC
    ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
#endif

    // AWB ISP debug info
    rDbg3AInfo2.hdr.u4KeyID = ISP_DEBUG_KEYID;
    rDbg3AInfo2.hdr.u4ModuleCount = MODULE_NUM(6,2);

    rDbg3AInfo2.hdr.u4AWBDebugInfoOffset      = sizeof(rDbg3AInfo2.hdr);
    rDbg3AInfo2.hdr.u4ISPDebugInfoOffset      = rDbg3AInfo2.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T) + sizeof(NSIspExifDebug::IspExifDebugInfo_T::Header);
    rDbg3AInfo2.hdr.u4ISPP1RegDataOffset      = rDbg3AInfo2.hdr.u4ISPDebugInfoOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo);
    rDbg3AInfo2.hdr.u4ISPP2RegDataOffset      = rDbg3AInfo2.hdr.u4ISPP1RegDataOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::P1RegInfo);
    rDbg3AInfo2.hdr.u4MFBRegInfoOffset        = rDbg3AInfo2.hdr.u4ISPP2RegDataOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::P2RegInfo);
    rDbg3AInfo2.hdr.u4AWBDebugDataOffset      = rDbg3AInfo2.hdr.u4ISPDebugInfoOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T) - sizeof(NSIspExifDebug::IspExifDebugInfo_T::Header);

    // AWB
    IAwbMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo2.rAWBDebugInfo, rDbg3AInfo2.rAWBDebugData);

    // P1 ISP
    IspTuningMgr::getInstance().getDebugInfoP1(m_i4SensorDev, rDbg3AInfo2.rISPDebugInfo, MFALSE);

    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = 0;
    switch (e3ACtrl)
    {
        // --------------------------------- AE ---------------------------------
        case E3ACtrl_SetHalHdr:
            if (iArg1)
            {
                AE_MODE_CFG_T rCaptureInfo;
                CaptureParam_T &a_rCaptureInfo = *reinterpret_cast<CaptureParam_T*>(iArg2);
                rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
                rCaptureInfo.u4Eposuretime  = a_rCaptureInfo.u4Eposuretime;
                rCaptureInfo.u4AfeGain      = a_rCaptureInfo.u4AfeGain;
                rCaptureInfo.u4IspGain      = a_rCaptureInfo.u4IspGain;
                rCaptureInfo.u4RealISO      = a_rCaptureInfo.u4RealISO;
                rCaptureInfo.i2FlareGain    = a_rCaptureInfo.u4FlareGain;
                rCaptureInfo.i2FlareOffset  = a_rCaptureInfo.u4FlareOffset;
                IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, rCaptureInfo);
                IAeMgr::getInstance().enableHDRShot(m_i4SensorDev, MTRUE);
                mbIsHDRShot = MTRUE;
            }
            else
            {
                IAeMgr::getInstance().enableHDRShot(m_i4SensorDev, MFALSE);
            }
            break;
        case E3ACtrl_GetCurrentEV:
            IAeMgr::getInstance().CCTOPAEGetCurrentEV(m_i4SensorDev, reinterpret_cast<MINT32 *>(iArg1), reinterpret_cast<MUINT32 *>(iArg2));
            break;
        case E3ACtrl_GetBVOffset:{
            MUINT32 outLen = 0;
            AE_NVRAM_T aeNvram;
            ::memset(&aeNvram, 0, sizeof(aeNvram));
            IAeMgr::getInstance().getNVRAMParam(m_i4SensorDev, reinterpret_cast<MVOID *>(&aeNvram), &outLen);
            *reinterpret_cast<MINT32*>(iArg1) = aeNvram.rCCTConfig.i4BVOffset;
            }break;
        case E3ACtrl_GetNVRAMParam:
            IAeMgr::getInstance().getNVRAMParam(m_i4SensorDev, reinterpret_cast<MVOID *>(iArg1), reinterpret_cast<MUINT32 *>(iArg2));
            break;
        case E3ACtrl_SetAEIsoSpeedMode:
            IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAETargetMode:
            IAeMgr::getInstance().SetAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            IAfMgr::getInstance().SetAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            m_i4AETargetMode = static_cast<eAETargetMODE>(iArg1);
            break;
        case E3ACtrl_SetAELimiterMode:
            IAeMgr::getInstance().setAELimiterMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAECamMode:
            IAeMgr::getInstance().setAECamMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_EnableDisableAE:
            if (iArg1) i4Ret = IAeMgr::getInstance().enableAE(m_i4SensorDev);
            else       i4Ret = IAeMgr::getInstance().disableAE(m_i4SensorDev);
            break;
        case E3ACtrl_SetMinMaxFps:
            IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, (MUINT32)iArg1, (MUINT32)iArg2);
            break;
        case E3ACtrl_GetExposureInfo:
            i4Ret = IAeMgr::getInstance().getExposureInfo(m_i4SensorDev, *reinterpret_cast<ExpSettingParam_T*>(iArg1));
            break;
        case E3ACtrl_GetAECapPLineTable:
            IAeMgr::getInstance().getAECapPlineTable(m_i4SensorDev, reinterpret_cast<MINT32*>(iArg1), *reinterpret_cast<strAETable*>(iArg2));
            break;
        case E3ACtrl_GetInitExposureTime:
            AE_MODE_CFG_T rPreviewInfo;
            i4Ret = IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, rPreviewInfo);
            *(reinterpret_cast<MUINT32*>(iArg1)) = rPreviewInfo.u4Eposuretime;
            break;
        case E3ACtrl_GetExposureParam:
            CaptureParam_T* a_rCaptureInfo;
            a_rCaptureInfo = reinterpret_cast<CaptureParam_T*>(iArg1);

            AE_MODE_CFG_T rCaptureInfo;
            i4Ret = IAeMgr::getInstance().getCaptureParams(m_i4SensorDev, rCaptureInfo);

            a_rCaptureInfo->u4ExposureMode = rCaptureInfo.u4ExposureMode;
            a_rCaptureInfo->u4Eposuretime = rCaptureInfo.u4Eposuretime;
            a_rCaptureInfo->u4AfeGain = rCaptureInfo.u4AfeGain;
            a_rCaptureInfo->u4IspGain = rCaptureInfo.u4IspGain;
            a_rCaptureInfo->u4RealISO = rCaptureInfo.u4RealISO;
            a_rCaptureInfo->u4FlareGain = (MUINT32) rCaptureInfo.i2FlareGain;
            a_rCaptureInfo->u4FlareOffset = (MUINT32) rCaptureInfo.i2FlareOffset;
            a_rCaptureInfo->i4LightValue_x10 = IAeMgr::getInstance().getCaptureLVvalue(m_i4SensorDev);
            break;
        case E3ACtrl_GetIsAEStable:
            *(reinterpret_cast<MUINT32*>(iArg1)) = IAeMgr::getInstance().IsAEStable(m_i4SensorDev);
            break;
        case E3ACtrl_GetSensorSyncInfo:
            MINT32 i4SutterDelay;
            MINT32 i4SensorGainDelay;
            MINT32 i4IspGainDelay;
            AAASensorMgr::getInstance().getSensorSyncinfo(m_i4SensorDev,&i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay, reinterpret_cast<MINT32*>(iArg1));
            break;
        case E3ACtrl_GetSensorPreviewDelay:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4SensorPreviewDelay;
            break;
        case E3ACtrl_GetAEInitExpoSetting:
            AEInitExpoSetting_T* a_rAEInitExpoSetting;
            a_rAEInitExpoSetting = reinterpret_cast<AEInitExpoSetting_T*>(iArg1);
            i4Ret = IAeMgr::getInstance().getAEInitExpoSetting(m_i4SensorDev, *reinterpret_cast<AEInitExpoSetting_T*>(iArg1));
            CAM_LOGD("[%s()] E3ACtrl_GetInitExpoSetting: u4SensorMode(%d) u4AETargetMode(%d) u4Eposuretime(%d) u4AfeGain(%d) u4Eposuretime_se(%d) u4AfeGain_se(%d)",
                      __FUNCTION__, a_rAEInitExpoSetting->u4SensorMode, a_rAEInitExpoSetting->u4AETargetMode,
                      a_rAEInitExpoSetting->u4Eposuretime, a_rAEInitExpoSetting->u4AfeGain,
                      a_rAEInitExpoSetting->u4Eposuretime_se, a_rAEInitExpoSetting->u4AfeGain_se);
            break;
        // ----------------------------------ISP----------------------------------
        case E3ACtrl_GetIspGamma:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA, iArg1, iArg2);
            break;
        case E3ACtrl_ValidatePass1:
            {
                MINT32 i4Magic = (MINT32)iArg1;
                RequestSet_T rRequestSet;
                rRequestSet.vNumberSet.clear();
                rRequestSet.vNumberSet.push_back(i4Magic);
                m_pTuning->validate(m_i4SensorDev, rRequestSet, MTRUE);
            }
            break;
        case E3ACtrl_SetLcsoParam:
            {
                ISP_LCS_OUT_INFO_T tempLCS_Info = (*(ISP_LCS_OUT_INFO_T*)iArg1);

                IHal3AResultBuf::getInstance(m_i4SensorDev)->lockBuf();
                Result_T *rCurrResult = NULL;
                rCurrResult = IHal3AResultBuf::getInstance(m_i4SensorDev)->getResult(tempLCS_Info.i4FrmId);
                if(rCurrResult == NULL)
                {
                    MY_LOGE("[%s]  E3ACtrl_SetLcsoParam getResult fail, P1 Driver FrmId: %d", __FUNCTION__, tempLCS_Info.i4FrmId);
                }
                else
                {
                    rCurrResult->rCamInfo.rLCS_Info.rOutSetting = tempLCS_Info;
                    rCurrResult->rCamInfo.bBypassLCE = MFALSE;
                }
                IHal3AResultBuf::getInstance(m_i4SensorDev)->unlockBuf();
            }
            break;

        // --------------------------------- AWB ---------------------------------
        case E3ACtrl_SetAwbBypCalibration:
            i4Ret = IAwbMgr::getInstance().CCTOPAWBBypassCalibration(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
            break;

#if CAM3_AF_FEATURE_EN
        // --------------------------------- AF ---------------------------------
        case E3ACtrl_SetAFMode:
            IAfMgr::getInstance().setAFMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAFMFPos:
            IAfMgr::getInstance().setMFPos(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAFFullScanStep:
            IAfMgr::getInstance().setFullScanstep(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAFCamMode:

            break;
        case E3ACtrl_GetAFDAFTable:
            IAfMgr::getInstance().getDAFtbl(m_i4SensorDev, (MVOID**)iArg1);
            break;
        case E3ACtrl_SetEnableOIS:
            IAfMgr::getInstance().setOpticalStabilizationMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetEnablePBin:
            IAfMgr::getInstance().enablePBIN(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetPureRawData:
            // arg1 : meta data
            // arg2 : image buffer
            //reference : pdenode
            IAfMgr::getInstance().doSWPDE(m_i4SensorDev, (MVOID*)iArg1, (MVOID*)iArg2);
            break;
#endif

#if CAM3_LSC_FEATURE_EN
        // --------------------------------- Shading ---------------------------------
        case E3ACtrl_SetShadingSdblkCfg:
            i4Ret = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->CCTOPSetSdblkFileCfg((iArg1 ? MTRUE : MFALSE), reinterpret_cast<const char*>(iArg2));
            break;
        case E3ACtrl_SetShadingEngMode:
            m_fgEnableShadingMeta = MFALSE;
            switch (iArg1)
            {
            case 0: // Disable
                IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(2);
                break;
            case 1: // Auto
                IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                break;
            case 2: // Low
                IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(0);
                break;
            case 3: // Mid
                IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(1);
                break;
            case 4: // High
                IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(2);
                break;
            case 5: // TSF
            default:
                IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(2);
                break;
            }
            break;
        case E3ACtrl_SetShadingByp123:
            ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->CCTOPSetBypass123(iArg1 ? MTRUE : MFALSE);
            break;
#endif

#if CAM3_FLASH_FEATURE_EN
        // --------------------------------- Flash ---------------------------------
        case E3ACtrl_GetQuickCalibration:
            i4Ret = FlashMgr::getInstance().cctGetQuickCalibrationResult(m_i4SensorDev);
            CAM_LOGD_IF(m_3ALogEnable, "getFlashQuickCalibrationResult ret=%d", i4Ret);
            break;
        case E3ACtrl_EnableFlashQuickCalibration:
            if(iArg1==1)
                i4Ret = FlashMgr::getInstance().cctSetSpModeQuickCalibration2(m_i4SensorDev);
            else
                i4Ret = FlashMgr::getInstance().cctSetSpModeNormal(m_i4SensorDev);
            break;
        case E3ACtrl_SetIsFlashOnCapture:
            FlashMgr::getInstance().setIsFlashOnCapture(m_i4SensorDev, iArg1);
            CAM_LOGD_IF(m_3ALogEnable, "setIsFlashOnCapture=%d", (MINT32)iArg1);
            break;
#endif

        // --------------------------------- flow control ---------------------------------
        case E3ACtrl_Enable3ASetParams:
            m_bEnable3ASetParams = iArg1;
            break;
        case E3ACtrl_SetOperMode:
            i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_GetOperMode:
            i4Ret = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            break;
        // --------------------------------- get info ---------------------------------
        case E3ACtrl_GetAsdInfo:
            i4Ret = Hal3ARawImp::getASDInfo(*reinterpret_cast<ASDInfo_T*>(iArg1));
            break;
        case E3ACtrl_GetExifInfo:
            i4Ret = Hal3ARawImp::get3AEXIFInfo(*reinterpret_cast<EXIF_3A_INFO_T*>(iArg1));
            break;
        case E3ACtrl_GetSupportedInfo:
            AE_DEVICES_INFO_T rDeviceInfo;
            i4Ret = IAeMgr::getInstance().getNvramData(m_i4SensorDev);
            IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev, rDeviceInfo);

            FeatureParam_T* rFeatureParam;
            rFeatureParam = reinterpret_cast<FeatureParam_T*>(iArg1);
            rFeatureParam->bAutoWhiteBalanceLockSupported = IAwbMgr::getInstance().isAWBLockSupported();
            rFeatureParam->bExposureLockSupported = IAeMgr::getInstance().isAELockSupported();
            rFeatureParam->u4MaxMeterAreaNum = IAeMgr::getInstance().getAEMaxMeterAreaNum(m_i4SensorDev);
            rFeatureParam->u4FocusLength_100x = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
            rFeatureParam->u4MaxFocusAreaNum = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);
            rFeatureParam->bEnableDynamicFrameRate = CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE();
            rFeatureParam->i4MaxLensPos = IAfMgr::getInstance().getMaxLensPos(m_i4SensorDev);
            rFeatureParam->i4MinLensPos = IAfMgr::getInstance().getMinLensPos(m_i4SensorDev);
            rFeatureParam->i4AFBestPos = IAfMgr::getInstance().getAFBestPos(m_i4SensorDev);
            rFeatureParam->u4PureRawInterval = IAfMgr::getInstance().PDPureRawInterval(m_i4SensorDev);
            break;
        default:
            CAM_LOGD_IF(m_3ALogEnable, "[%s] Unsupport Command(%d)", __FUNCTION__, e3ACtrl);
            return MFALSE;
    }
    if(i4Ret == S_3A_OK)
        return MTRUE;
    else
        return MFALSE;
}

MVOID
Hal3ARawSMVRImp::
querySensorStaticInfo()
{
    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        MY_ERR("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
    MUINT32 u4PDAF_Support = sensorStaticInfo.PDAF_Support; //0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(Full), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode

    m_i4SensorPreviewDelay = sensorStaticInfo.previewDelayFrame;

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), rawFmtType(%d)\n", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, u4RawFmtType);

    // 3A/ISP mgr can query sensor static information here

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE, u4RawFmtType, u4PDAF_Support);

    IAwbMgr::getInstance().setIsMono(m_i4SensorDev, (u4RawFmtType == SENSOR_RAW_MONO ? MTRUE : MFALSE), u4RawFmtType);

#if CAM3_STEREO_FEATURE_EN
    // For AE/AWB/AF Sync
    if(u4RawFmtType == SENSOR_RAW_MONO && ISync3AMgr::getInstance()->isActive())
    {
        IAwbMgr::getInstance().disableAWB(m_i4SensorDev);
        ISync3AMgr::getInstance()->getSync3A()->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
        ISync3AMgr::getInstance()->getSync3A(ISync3AMgr::E_SYNC3AMGR_CAPTURE)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);

        IAfMgr::getInstance().setAFMode(m_i4SensorDev, MTK_CONTROL_AF_MODE_OFF);
        ISync3AMgr::getInstance()->getSync3A()->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
        ISync3AMgr::getInstance()->getSync3A(ISync3AMgr::E_SYNC3AMGR_CAPTURE)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
    }
#endif
}

