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
#define LOG_TAG "Hal3ARaw"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include "Hal3ARawImp.h"
#include <af_define.h>
#include <af_mgr/af_mgr_if.h>
#include <stdio.h>
#include <string.h>
#include <array>
#include <private/aaa_utils.h>

struct TUNING_INIT_CTRL_T
{
    MBOOL bHadModuleId;
    MBOOL bIsFirstTime;
    MUINT32 u4ModuleId;
    TUNING_INIT_CTRL_T()
        : bHadModuleId(MFALSE)
        , bIsFirstTime(MTRUE)
        , u4ModuleId(0)
    {}
};

static std::array<TUNING_INIT_CTRL_T, SENSOR_IDX_MAX> gTuningInitCtrl;

#define MY_INST NS3Av3::INST_T<Hal3ARawImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

#define MY_LOGD(fmt, arg...) \
    do { \
        CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK_IP_BASE, fmt, ##arg); \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if ( (cond) ){ CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK_IP_BASE, __VA_ARGS__); } \
    }while(0)

#define MY_LOGW(fmt, arg...) \
    do { \
        CAM_ULOGW(Utils::ULog::MOD_3A_FRAMEWORK_IP_BASE, fmt, ##arg); \
    }while(0)

#define MY_LOGE(fmt, arg...) \
    do { \
        CAM_ULOGE(Utils::ULog::MOD_3A_FRAMEWORK_IP_BASE, fmt, ##arg); \
    }while(0)

I3AWrapper*
Hal3ARaw::
getInstance(MINT32 const i4SensorOpenIndex)
{
    return Hal3ARawImp::getInstance(i4SensorOpenIndex);
}

I3AWrapper*
Hal3ARawImp::
getInstance(MINT32 const i4SensorOpenIndex)
{
    if(i4SensorOpenIndex >= SENSOR_IDX_MAX || i4SensorOpenIndex < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorOpenIndex);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3ARawImp>(i4SensorOpenIndex);
    } );
    (rSingleton.instance)->init(i4SensorOpenIndex);

    return rSingleton.instance.get();
}

MVOID
Hal3ARawImp::
destroyInstance()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    uninit();
}

Hal3ARawImp::
Hal3ARawImp(MINT32 const i4SensorIdx)
    : m_3ALogEnable(0)
    , m_i4IdxCacheLogEnable(0)
    , m_i4CopyLscP1En(0)
    , m_Users(0)
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorDev(0)
    , m_i4sensorId(0)
    , m_u4SensorMode(0)
    , m_u4TgInfo(0)
    , m_bEnable3ASetParams(MTRUE)
    , m_bFaceDetectEnable(MFALSE)
    , m_i4TgWidth   (1000)
    , m_i4TgHeight  (1000)
    , mbIsHDRShot(MFALSE)
    , m_i4HbinWidth (1000)
    , m_i4HbinHeight(1000)
    , m_i4BinWidth (1000)
    , m_i4BinHeight(1000)
    , m_fgEnableShadingMeta(MTRUE)
    , m_bIsRecordingFlash(MFALSE)
    , m_i4EVCap(0)
    , m_i4SensorPreviewDelay(0)
    , m_i4AeShutDelayFrame(0)
    , m_i4AeISPGainDelayFrame(0)
    , m_bPreStop(MFALSE)
    , m_i4OverrideMinFrameRate(0)
    , m_i4OverrideMaxFrameRate(0)
    , m_u4AutoAECount(5)
    , m_pCamIO(NULL)
    , m_pThreadRaw(NULL)
    , m_pTaskMgr(NULL)
    , m_pICcuMgr(NULL)
    , m_pCbSet(NULL)
    , m_i4SttMagicNumber(0)
    , m_bIsHighQualityCaptureOn(MFALSE)
    , m_bIsCapEnd(MFALSE)
    , m_bIsFlashOpened(MFALSE)
    , m_bIsSkipSync3A(MFALSE)
    , m_i4StereoWarning(0)
    , m_i4ShortExpCount(0)
    , m_bDbgInfoEnable(0)
    , m_pCcuCtrl3ACtrl(NULL)
    //CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    , m_pCqTuningMgr(NULL)
#endif
    , m_u4LastRequestNumber(0)
    , m_bFlashOpenedByTask(MFALSE)
    , m_i4FlashType(-1)
    , m_pResultPoolObj(NULL)
    , m_3ACctEnable(0)
    , m_DebugLogWEn(0)
    , m_bAAOMode(MFALSE)
    , m_bFrontalBin(MFALSE)
    , m_u1LastCaptureIntent(0)
    , m_ThreadAE(0)
    , m_ThreadAF(0)
    , m_pCallbackHub(NULL)
    , m_pIdxMgr(IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev)))
    , m_faceNum(0)
    , m_bIsFirstSetParams(MTRUE)
    , m_i4CamModeEnable(0)
    , m_p3ASttCtrl(NULL)
    , m_bMappingQueryFlag_3A(MTRUE)
    , m_i4OperMode(NSIspTuning::EOperMode_Normal)
    , m_i4EffectiveFrame(0)
    , m_i4SubsampleCount(1)
    , m_u4PrePresetKey(0)
    , m_pScenarios(NULL)
    , m_scenarioNs(0)
    , m_scenarioIdx(0)
    , mPostCommandMtx()
    , m_i4BypassStt(0)
    , m_i4QuerryCBNum(0)
    , m_i4CamFeatureMode(-1)
    , m_strAppName("MTKCam")
{
    m_i4SensorDev = NS3Av3::mapSensorIdxToDev(i4SensorIdx);
    m_pIdxMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
    m_vShortExpFrame = {0, 1, 0, -1};   // The fourth entry is to prevent segmentation fault
    CAM_LOGD("[%s] sensorIdx(%d), sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);
}

MBOOL
Hal3ARawImp::
init(MINT32 i4SensorOpenIdx)
{
    MBOOL dbgInfoEnable;
    int err=0;
#if (IS_BUILD_USER)
    dbgInfoEnable = 0;
#else
    dbgInfoEnable = 1;
#endif

    GET_PROP("vendor.debug.camera.dbginfo", dbgInfoEnable, m_bDbgInfoEnable);

    GET_PROP("vendor.debug.camera.log", 0, m_3ALogEnable);
    GET_PROP("vendor.debug.idxcache.log", 0, m_i4IdxCacheLogEnable);
    GET_PROP("vendor.debug.camera.copy.p1.lsc", 0, m_i4CopyLscP1En);
    if ( m_3ALogEnable == 0 ) {
        GET_PROP("vendor.debug.camera.log.hal3a", 0, m_3ALogEnable);
    }
    m_DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);
    m_i4SensorIdx = i4SensorOpenIdx;

    CAM_LOGD("[%s] m_Users: %d, SensorDev %d, index %d \n", __FUNCTION__, std::atomic_load((&m_Users)), m_i4SensorDev, m_i4SensorIdx);

    AAA_TRACE_D("read NvBuf");
    //read module id and update data only one time
    if(!gTuningInitCtrl[m_i4SensorIdx].bHadModuleId)
    {
        CAM_LOGD("[%s] Read buf again for new module id \n", __FUNCTION__);
        void *p, *q, *r, *s, *t, *u, *v;
        NvBufUtil::getInstance().getSensorIdAndModuleId(m_i4SensorDev, m_i4SensorIdx, m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);

        openCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);

        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4SensorDev, p, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_3A getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_STROBE, m_i4SensorDev, q, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_STROBE getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, m_i4SensorDev, r, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_FLASH_CALIBRATION getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, m_i4SensorDev, s, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_ISP getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_i4SensorDev, t, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_SHADING getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_IDX_TBL, m_i4SensorDev, u, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_IDX_TBL getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4SensorDev, v, MFALSE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_LENS getBufAndRead ERROR \n", __FUNCTION__);
    } else {
        if(!gTuningInitCtrl[m_i4SensorIdx].bIsFirstTime && m_Users!=0)
            lockCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);
    }
    AAA_TRACE_END_D;

    // check user count
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created, SensorDev %d, SensorIdx %d\n", __FUNCTION__, std::atomic_load((&m_Users)), m_i4SensorDev, m_i4SensorIdx);
        MINT32 i4BeforeUserCount = std::atomic_fetch_add((&m_Users), 1);

        CAM_LOGD("[%s] BeforeUserCount %d, %d has created\n", __FUNCTION__, i4BeforeUserCount, std::atomic_load((&m_Users)));
        return S_3A_OK;
    }

    m_fgEnableShadingMeta = MTRUE;

    CAM_LOGD("[%s] 2D", __FUNCTION__);
    m_pTaskMgr = ITaskMgr::create(m_i4SensorDev);

    // CCT init
    /*GET_PROP("vendor.3a.cct.enable", 0, m_3ACctEnable);
    if ( m_3ACctEnable == 1 ) {
        MBOOL ret1, ret2;
        // 1. delete previous CCT Server
        CAM_LOGD("CCT before init");
        ret1 = CctSvrEnt_Ctrl(CCT_SVR_CTL_STOP_SERVER_THREAD);
        ret2 = CctSvrEnt_DeInit();
        //CAM_LOGD("CCT before init: ret = %d, %d", ret1, ret2);

        // 2. get sensor index for CCT Server
        MINT32 cct_sensor_index = 1;
        GET_PROP("vendor.3a.cct.sensor.index", 1, cct_sensor_index);

        // 3. init CCT Server
        CAM_LOGD("CCT init: sensor = %d", cct_sensor_index);
        ret1 = CctSvrEnt_Init(cct_sensor_index);
        ret2 = CctSvrEnt_Ctrl(CCT_SVR_CTL_START_SERVER_THREAD);
        //CAM_LOGD("CCT init: ret = %d, %d", ret1, ret2);
    }*/

    // init Thread and state mgr
    m_pThreadRaw = IThreadRaw::createInstance(this, m_i4SensorDev, m_i4SensorIdx);

    // AE init
    AAA_TRACE_D("AE init");
    IAeMgr::getInstance().cameraPreviewInit(m_i4SensorDev, m_i4SensorIdx, m_rParam);
    AAA_TRACE_END_D;

    // AWB init
    AAA_TRACE_D("AWB init");
    IAwbMgr::getInstance().init(m_i4SensorDev, m_i4SensorIdx);
    AAA_TRACE_END_D;

    // AF init
#if CAM3_AF_FEATURE_EN
    AAA_TRACE_D("AF init");
    IAfMgr::getInstance(m_i4SensorDev).init( m_i4SensorIdx);
    AAA_TRACE_END_D;
#endif

#if CAM3_FLASH_FEATURE_EN
    AAA_TRACE_D("FLASH init");
    //FLASH HAL init
    FlashHal::getInstance(m_i4SensorDev)->init();
    //FLASH init
    FlashMgr::getInstance(m_i4SensorDev)->init();
    AAA_TRACE_END_D;
#endif

    //FLICKER init
#if CAM3_FLICKER_FEATURE_EN
    AAA_TRACE_D("FLICKER init");
    IFlickerHal::getInstance(m_i4SensorDev)->init(m_i4SensorIdx);
    AAA_TRACE_END_D;
#endif

    ILscMgr::createInstance((MUINT32)m_i4SensorDev, m_i4SensorIdx)->init();

    //ILscMgr::getInstance((MUINT32)m_i4SensorDev)->setFactoryMode(MTRUE);

    // ResultPool - Get result pool object
    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(m_pResultPoolObj == NULL)
        CAM_LOGE("ResultPool getInstance fail");

    m_pCallbackHub = new Hal3ACbHub(m_i4SensorDev);
    if (!m_pCallbackHub)
        CAM_LOGE("[%s] CallbackHub Object created failed for SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    AAA_TRACE_D("querySensorStaticInfo");
    querySensorStaticInfo();
    AAA_TRACE_END_D;

    CAM_LOGD("[%s] done SensorDev %d, SensorIdx %d\n", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);
    MINT32 i4BeforeUserCount = std::atomic_fetch_add((&m_Users), 1);

    CAM_LOGD("[%s] BeforeUserCount %d, %d has created\n", __FUNCTION__, i4BeforeUserCount, std::atomic_load((&m_Users)));

    // Get ScenarioInfo
    char *pScenario;
    m_pIdxMgr->getScenarioInfo(static_cast<ESensorDev_T>(m_i4SensorDev), &pScenario, m_scenarioNs);
    m_pScenarios = (char (*)[][64])pScenario;
    if (!m_pScenarios) {
        CAM_LOGE("[%s] m_pScenarios(NULL)\n", __FUNCTION__);
    }

    if(!gTuningInitCtrl[m_i4SensorIdx].bHadModuleId)
    {
        closeCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);
        gTuningInitCtrl[m_i4SensorIdx].bHadModuleId=MTRUE;
    }

    return S_3A_OK;
}

MBOOL
Hal3ARawImp::
uninit()
{
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    std::lock_guard<std::mutex> lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }
    CAM_LOGD("[%s] m_Users: %d SensorDev %d, SensorIdx %d\n", __FUNCTION__, std::atomic_load((&m_Users)), m_i4SensorDev, m_i4SensorIdx);

    // More than one user, so decrease one User.
    MINT32 i4BeforeUserCount = std::atomic_fetch_sub((&m_Users), 1);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        m_pThreadRaw->destroyInstance();

        MRESULT err = S_3A_OK;

        ILscMgr::getInstance((ESensorDev_T)m_i4SensorDev)->uninit();

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
        err = IAfMgr::getInstance(m_i4SensorDev).uninit();
        if (FAILED(err)) {
            CAM_LOGE("IAfMgr::getInstance().uninit() fail\n");
            return err;
        }
#endif

#if CAM3_FLASH_FEATURE_EN
        //FLASH uninit
        FlashMgr::getInstance(m_i4SensorDev)->uninit();
        //FLASH HAL uninit
        FlashHal::getInstance(m_i4SensorDev)->uninit();
#endif

#if CAM3_FLICKER_FEATURE_EN
        // FLICKER uninit
        err = IFlickerHal::getInstance(m_i4SensorDev)->uninit();
        if (FAILED(err)) {
            CAM_LOGE("IFlickerHal::getInstance().Uninit fail\n");
            return err;
        }
#endif
        if (m_pCbSet)
        {
            m_pCbSet = NULL;
            CAM_LOGE("User did not detach callbacks!");
        }

        // TaskMgr uninit
        m_pTaskMgr->destroy();
        m_pTaskMgr = NULL;

        delete m_pCallbackHub;
        m_pCallbackHub = NULL;


        m_i4OverrideMinFrameRate = 0;
        m_i4OverrideMaxFrameRate = 0;

        // GyroSensor uninit
        //enableGyroSensor(MFALSE);

        CAM_LOGD("[%s] done SensorDev %d, SensorIdx %d\n", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);

    }
    else    // There are still some users.
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] BeforeUserCount Still %d users \n", __FUNCTION__, i4BeforeUserCount, std::atomic_load((&m_Users)));
    }

    unlockCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);

    return S_3A_OK;
}

MINT32
Hal3ARawImp::
config(const ConfigInfo_T& rConfigInfo)
{
    CAM_LOGD("[%s]+ sensorDev(%d), SensorIdx(%d), Mode(%d), i4SubsampleCount(%d), i4RequestCount(%d), CamFeatureMode(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, m_u4SensorMode, rConfigInfo.i4SubsampleCount, rConfigInfo.i4RequestCount, rConfigInfo.i4CamFeatureMode);
    MRESULT err = S_3A_OK;

#if CAM3_STEREO_FEATURE_EN
#if 0
    m_i4CamFeatureMode = rConfigInfo.i4CamFeatureMode;
    if(m_i4CamFeatureMode ==0)
        m_i4QuerryCBNum = 5;
#endif
    ::memset(&m_rLastStereoParam, 0, sizeof(m_rLastStereoParam));
#endif

    m_bIsFirstSetParams = MTRUE;
    m_bIsHighQualityCaptureOn = MFALSE;
    GET_PROP("vendor.debug.camera.cammode", 0, m_i4CamModeEnable);

    AAA_TRACE_D("NormalIOPipe sendcommand");
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

    if (m_pCallbackHub) {
        CAM_LOGD("Register callback to Pass1 driver(%p)", m_pCallbackHub);
        // Set Dynamic Binning Callback to driver
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_SET_3A_HAL_CBFP, (MINTPTR)(m_pCallbackHub->getCallbackAddr(0)), 0, 0);
        //p1 dump call back
        //m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_SET_REGDUMP_CBFP, (MINTPTR)(     m_pCallbackHub->getCallbackAddr(1)), 0, 0);
    }
    AAA_TRACE_END_D;

    if(rConfigInfo.i4BitMode != EBitMode_12Bit && rConfigInfo.i4BitMode != EBitMode_14Bit)
    {
        CAM_LOGE("Not support BitMode(%d)", rConfigInfo.i4BitMode);
        return err;
    }

    m_bAAOMode = (rConfigInfo.i4BitMode == EBitMode_14Bit);
    m_i4SubsampleCount = rConfigInfo.i4SubsampleCount;

    MUINT32 u4AAWidth, u4AAHight;
    MUINT32 u4AFWidth, u4AFHeight;

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

    AAA_TRACE_D("sendcommand");
    // query input size info for AAO and FLKO
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_HBIN_INFO,
                        (MINTPTR)&u4AAWidth, (MINTPTR)&u4AAHight, 0);
    m_i4HbinWidth = u4AAWidth;
    m_i4HbinHeight= u4AAHight;

    // query input size info for AFO
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO,
                        (MINTPTR)&u4AFWidth, (MINTPTR)&u4AFHeight, 0);

    m_i4BinWidth = u4AFWidth;
    m_i4BinHeight= u4AFHeight;
    AAA_TRACE_END_D;

    // update HBIN and BIN info to AE/AWB/AF/FLICKER

    AAA_TRACE_D("AWB setSensorMode");
    // set sensor mode to 3A modules
    //IAeMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AAWidth, u4AAHight);     move to AE start thread     //~Nelson
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight, u4AAWidth, u4AAHight);
    AAA_TRACE_END_D;

#if CAM3_FLICKER_FEATURE_EN
    AAA_TRACE_D("Flicker setSensorMode");
    IFlickerHal::getInstance(m_i4SensorDev)->setSensorMode(m_u4SensorMode, u4AAWidth, u4AAHight);
    AAA_TRACE_END_D;
#endif

    AAA_TRACE_D("CCU");
    if(m_pCcuCtrl3ACtrl)
    {
        //collect frame subsample info
        struct ccu_frame_subsmpl_info subsmpl_info;

        subsmpl_info.work_type = CAM_WORK_TYPE_DL; //direct link mode
        subsmpl_info.subsmpl_ratio = rConfigInfo.i4SubsampleCount; //subsample 4:1 (120->30)

        //send fram subsample info to CCU via ccuControl command: MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO
        m_pCcuCtrl3ACtrl->ccuControl(MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO, &subsmpl_info, NULL);
    }
    AAA_TRACE_END_D;

    // Create thread for starting AE & AWB & AF
    m_sParam.SensorMode = m_u4SensorMode;
    ::pthread_create(&m_ThreadAE, NULL, Hal3ARawImp::ThreadAEStart, this);
    ::pthread_create(&m_ThreadAF, NULL, Hal3ARawImp::ThreadAFStart, this);

#if CAM3_AF_FEATURE_EN
    AAA_TRACE_D("AF config");
    MUINT32 u4AFNVRAMIndex = 0xFFFFFFFF;
    CAM_IDX_QRY_COMB rMapping_InfoAF;
    m_pIdxMgr->getMappingInfo(static_cast<ESensorDev_T>(m_i4SensorDev), rMapping_InfoAF, m_rAfParam.i4MagicNum);
    u4AFNVRAMIndex = m_pIdxMgr->query(static_cast<MUINT32>(m_i4SensorDev), NSIspTuning::EModule_AF, rMapping_InfoAF, __FUNCTION__);
    IAfMgr::getInstance(m_i4SensorDev).setNVRAMIndex( u4AFNVRAMIndex);
    err = IAfMgr::getInstance(m_i4SensorDev).config();
    IAfMgr::getInstance(m_i4SensorDev).getStaticInfo(m_rAFStaticInfo, LOG_TAG);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().config() fail\n");
        return err;
    }
#endif

    AAA_TRACE_D("AE");
    //get AAO size information for LSC
    shadingConfig_T lscConfig;
    IAeMgr::getInstance().getAAOSize(m_i4SensorDev, lscConfig.AAOBlockW, lscConfig.AAOBlockH);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(lscConfig.AAOstrideSize)), NULL, NULL, NULL);
    MBOOL bAESMBuffermode = MFALSE;
    if(rConfigInfo.i4SubsampleCount > 1)
        bAESMBuffermode = MTRUE;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSMBuffermode, bAESMBuffermode, rConfigInfo.i4SubsampleCount, NULL, NULL);
    AAA_TRACE_END_D;

    AAA_TRACE_D("LSC");
    //set Hbin for TSFS
    lscConfig.u4HBinWidth = u4AAWidth;
    lscConfig.u4HBinHeight= u4AAHight;
    lscConfig.i4BinWidth  = u4AFWidth;
    lscConfig.i4BinHight  = u4AFHeight;

    ILscMgr *pLscMgr=ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));
    pLscMgr->config(lscConfig);
    AAA_TRACE_END_D;

    AAA_TRACE_D("updateTGInfo");
    // FIXME (remove): update TG Info to 3A modules
    updateTGInfo();
    AAA_TRACE_END_D;

    //Frontal Binning
    //MBOOL fgFrontalBin;
    m_bFrontalBin = (m_i4TgWidth == (MINT32)u4AFWidth && m_i4TgHeight == (MINT32)u4AFHeight) ? MFALSE : MTRUE;

#if CAM3_FLASH_FEATURE_EN
    // Flash start
    AAA_TRACE_D("FLASH Start");
    CAM_IDX_QRY_COMB rMapping_Info;
    FlashMgr::getInstance(m_i4SensorDev)->setBitMode(rConfigInfo.i4BitMode);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AE, m_3A_Index.u4FlashAENVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_Calibration, m_3A_Index.u4FlashCaliNVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AE, m_3A_Index.u4FlashAENVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_CALIBRATION, m_3A_Index.u4FlashCaliNVRAMIndex);
    IAwbMgr::getInstance().setStrobeNvramIdx(m_i4SensorDev, m_3A_Index.u4FlashAWBNVRAMIndex);
    IAwbMgr::getInstance().setFlashCaliNvramIdx(m_i4SensorDev, m_3A_Index.u4FlashCaliNVRAMIndex);

    err = FlashMgr::getInstance(m_i4SensorDev)->start();
    if (err) {
        CAM_LOGE("FlashMgr::getInstance().start() fail\n");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    FlashHal::getInstance(m_i4SensorDev)->setInCharge(MTRUE);
    FlashMgr::getInstance(m_i4SensorDev)->setInCharge(MTRUE);
    AAA_TRACE_END_D;
#endif

#if CAM3_FLICKER_FEATURE_EN
    // Flicker start
    AAA_TRACE_D("FLICKER Start");
    err = IFlickerHal::getInstance(m_i4SensorDev)->config();
    if (FAILED(err)) {
        CAM_LOGE("IFlickerHal::getInstance().config() fail\n");
        AAA_TRACE_END_D;
        return err;
    }
    AAA_TRACE_END_D;
#endif

    /*******************************************************
     ****** Update Module config result to ResultPool ******
     *******************************************************/
    // ResultPool - update LSC config result to ResutlPool
    AAA_TRACE_D("LSCConfig2Pool");
    LSCConfigResult_T rLSCConfigResult;
    pLscMgr->getLSCResultPool(&rLSCConfigResult);
    m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_LSC_CONFIGRESULTTOISP, &rLSCConfigResult); // frame id 1 for config
    AAA_TRACE_END_D;

#if CAM3_AF_FEATURE_EN
    // ResultPool - update AF config result to ResutlPool
    AAA_TRACE_D("AFConfig2Pool");
    AFResultConfig_T AFResultConfig;
    IAfMgr::getInstance(m_i4SensorDev).getHWCfgReg(&AFResultConfig);
    m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_AF_CONFIGRESULTTOISP, &AFResultConfig); // frame id 1 for config
    AAA_TRACE_END_D;
#endif

#if CAM3_FLICKER_FEATURE_EN
    // ResultPool - update FLK config result to ResutlPool
    AAA_TRACE_D("FLKConfig2Pool");
    FLKResultConfig_T FLKResultConfig;
    IFlickerHal::getInstance(m_i4SensorDev)->getHWCfgReg(&FLKResultConfig);
    m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_FLK_CONFIGRESULTTOISP, &FLKResultConfig); // frame id 1 for config
    AAA_TRACE_END_D;
#endif

    AAA_TRACE_D("SpecialLongExp");
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }

    //long exposure for N+1 or N+2 frame
    const char* const callerName = "Hal3AQueryAEEffectiveFrame";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
    MINT32 i4Ret = pHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_AE_EFFECTIVE_FRAME_FOR_LE, (MUINTPTR)&m_i4EffectiveFrame, 0 , 0 );
    pHalSensor->destroyInstance(callerName);

    cust_initSpecialLongExpOnOff(m_i4EffectiveFrame);
    AAA_TRACE_END_D;

    AAA_TRACE_D("join ThreadAE");
    MVOID* ThreadAEret;
    ::pthread_join(m_ThreadAE, &ThreadAEret);
    if (ThreadAEret)    CAM_LOGD("Thread AE create failed");
    AAA_TRACE_END_D;

    AAA_TRACE_D("Thread AF join");
    MVOID* ThreadAFret;
    ::pthread_join(m_ThreadAF, &ThreadAFret);
    if (ThreadAFret)    CAM_LOGD("Thread AF create failed");
    AAA_TRACE_END_D;

    // ResultPool - update PDO config result to ResutlPool, need to wait ThreadAFStart done
    AAA_TRACE_D("PDOConfig2Pool");
    isp_pdo_cfg_t PDOResultConfig;
    IPDMgr::getInstance().getPDOHWCfg(m_i4SensorDev, &PDOResultConfig);
    m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_PDO_CONFIGRESULTTOISP, &PDOResultConfig); // frame id 1 for config
    AAA_TRACE_END_D;

    CAM_LOGD("[%s]- sensorDev(%d), SensorIdx(%d), setAAOMode(%d), AAWH(%dx%d), AFWH(%dx%d), enableAFHw(%d), module update config(#%d), EffectiveFrame(%d) done", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, rConfigInfo.i4BitMode, u4AAWidth, u4AAHight, u4AFWidth, u4AFHeight, AFResultConfig.afIspRegInfo.enableAFHw, ConfigMagic, m_i4EffectiveFrame);
    return S_3A_OK;
}

MBOOL
Hal3ARawImp::
start()
{
    CAM_LOGD("[%s] + sensorDev(%d), SensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);
    MRESULT isAFLampOn = MFALSE;
    m_i4SttMagicNumber = 0;
    m_bPreStop = MFALSE;
    gTuningInitCtrl[m_i4SensorIdx].bIsFirstTime = MFALSE;

    /*******************************************************
       *******     For Customization Short Exposuretime Frame     **********
        *******************************************************/
    m_i4ShortExpCount = 0;
    m_vShortExpFrame = getShortExpFrame();

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::getInstance()->DevCount(MTRUE, m_i4SensorDev);
    ISync3AMgr::getInstance()->sendSync3ACtrl(ISync3AMgr::ESync3ACtrl_SetIsStart, (MINTPTR)(MTRUE), NULL);
#endif

#if CAM3_AF_FEATURE_EN
    if(m_rAFStaticInfo.isAfThreadEnable > 0)
    {
        // enable AF thread
        m_pThreadRaw->enableAFThread(m_pTaskMgr);
    }
    else
    {
        CAM_LOGD("m_rAFStaticInfo.isAfThreadEnable=0");
    }
#endif

    m_pThreadRaw->createEventThread();

#if CAM3_FLASH_FEATURE_EN
    isAFLampOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
#endif

#if CAM3_FLICKER_FEATURE_EN
    IFlickerHal::getInstance(m_i4SensorDev)->start(FLK_ATTACH_PRIO_MEDIUM);
#endif

    // setStrobeMode
    if((FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() != LIB3A_FLASH_MODE_FORCE_TORCH)){
        MBOOL bStrobeMode = isAFLampOn ? MTRUE : MFALSE;
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);
        CAM_LOGD("[%s] Non Torch ae setStrobeMode", __FUNCTION__);
    }
    IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);

#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    // CmdQ get instance & init
    if (m_pCqTuningMgr == NULL)
    {
        m_pCqTuningMgr = (CqTuningMgr*)CqTuningMgr::getInstance(m_i4SensorIdx);

        if(m_pCqTuningMgr->init(LOG_TAG) == MFALSE)
            CAM_LOGD("[%s] CqTuningMgr init error", __FUNCTION__);
        else
            CAM_LOGD("[%s] CqTuningMgr init", __FUNCTION__);
    }
#endif
#if 1
    MBOOL cctEnable = MFALSE;
    GET_PROP("vendor.3a.cct.enable", 0, cctEnable);
    if (cctEnable) {
        MINT32 cct_sensor_dev = 0;
        GET_PROP("vendor.3a.cct.sensor.index", 1, cct_sensor_dev);
        CAM_LOGD("CCT init: sensor = (%d/%d) ", cct_sensor_dev, m_i4SensorDev);
        if(cct_sensor_dev == m_i4SensorDev)
        {
            m_3ACctEnable = MTRUE;
            CctSvrEnt_Init(m_i4SensorDev);
            CctSvrEnt_Ctrl(CCT_SVR_CTL_START_SERVER_THREAD);
        }
    }
#endif
    CAM_LOGD("[%s] - sensorDev(%d), SensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
stop()
{
    CAM_LOGD("[%s] + sensorDev(%d), SensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);
    MRESULT err = S_3A_OK;
    m_i4SttMagicNumber = 0;
    m_bIsFlashOpened = MFALSE;

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::getInstance()->DevCount(MFALSE, m_i4SensorDev);
    ISync3AMgr::getInstance()->sendSync3ACtrl(ISync3AMgr::ESync3ACtrl_SetIsStart, (MINTPTR)(MFALSE), NULL);

    MBOOL fgIsActive = ISync3AMgr::getInstance()->isActive();
    MBOOL fgIsInit = ISync3AMgr::getInstance()->isInit();
    if(fgIsActive || fgIsInit)
    {
        ISync3AMgr::getInstance()->uninit();
        m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
    }
    m_bIsSkipSync3A = MFALSE;
#endif

    // Clear Task3a
    m_pTaskMgr->clearTaskQueue(Task_Update_3A);

    // AE stop
    AAA_TRACE_D("AE Stop");
/*#if CAM3_STEREO_FEATURE_EN
    if (ISync3AMgr::getInstance()->isActive())
    {
        MBOOL bEnableAEStereoManualPline = MFALSE;
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);
    }
#endif*/
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, 5000, 30000);
    CAM_LOGD("[%s] AE Min Max Frame Rate has been reset", __FUNCTION__);

    err = IAeMgr::getInstance().Stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
    CAM_LOGE("IAeMgr::getInstance().Stop() fail\n");
        return err;
    }
    m_pThreadRaw->destroyEventThread();

    // AWB stop
    AAA_TRACE_D("AWB Stop");
    err = IAwbMgr::getInstance().stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().stop() fail\n");
        return E_3A_ERR;
    }


#if CAM3_AF_FEATURE_EN
    // disable AF thread
    AAA_TRACE_D("AF THREAD disable");
    m_pThreadRaw->disableAFThread();
    AAA_TRACE_END_D;

    // AF stop
    // If call AF config or start, need to call stop
    AAA_TRACE_D("AF Stop");
    err = IAfMgr::getInstance(m_i4SensorDev).stop();
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Stop() fail\n");
        return err;
    }
    // PD stop
    // If call PD start, need to call stop
    AAA_TRACE_D("PD Stop");
    err = IPDMgr::getInstance().stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("IPDMgr::getInstance().stop fail\n");
        return err;
    }
#endif

#if CAM3_FLASH_FEATURE_EN
    // Flash stop
    AAA_TRACE_D("Flash Stop");
    err = FlashMgr::getInstance(m_i4SensorDev)->stop();
    if (err) {
        CAM_LOGE("FlashMgr::getInstance().stop() fail\n");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    // Flash Hal close flashlight
    FlashHal::getInstance(m_i4SensorDev)->setFlashOff();
    AAA_TRACE_END_D;
#endif

#if CAM3_FLICKER_FEATURE_EN
    // Flicker close
    AAA_TRACE_D("Flicker stop");
    IFlickerHal::getInstance(m_i4SensorDev)->stop();
    AAA_TRACE_END_D;
#endif

    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL)
    {
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }

#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    // CmdQ uninit
    if (m_pCqTuningMgr != NULL)
    {
        m_pCqTuningMgr->uninit(LOG_TAG);
        m_pCqTuningMgr= NULL;
    }
#endif
#if 1
    if ( m_3ACctEnable == 1 ) {
        CctSvrEnt_Ctrl(CCT_SVR_CTL_STOP_SERVER_THREAD);
        CctSvrEnt_DeInit();
    }
#endif
    ::IPDTblGen::getInstance()->stop(m_i4SensorDev);

    CAM_LOGD("[%s] - sensorDev(%d), SensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);
    return MTRUE;
}

MVOID
Hal3ARawImp::
pause()
{
    if(m_pICcuMgr)
    {
        CAM_LOGD("[%s] CCU_SYNC_DBG ccuSuspend, dev=%d",__FUNCTION__, m_i4SensorDev);
        MINT32 ret = m_pICcuMgr->ccuSuspend((ESensorDev_T)m_i4SensorDev, m_i4SensorIdx);
        if(ret != 0)
            CAM_LOGW("[%s] ccuSuspend fail",__FUNCTION__);
    }
    m_pThreadRaw->pauseAFThread();
}

MVOID
Hal3ARawImp::
resume(MINT32 MagicNum)
{
    DebugUtil::getInstance(m_i4SensorDev)->resetDetect(DBG_AE);
    DebugUtil::getInstance(m_i4SensorDev)->resetDetect(DBG_AF);
    if(m_pICcuMgr)
    {
        CAM_LOGD("[%s] CCU_SYNC_DBG ccuResume, dev=%d",__FUNCTION__, m_i4SensorDev);
        MINT32 ret = m_pICcuMgr->ccuResume((ESensorDev_T)m_i4SensorDev, m_i4SensorIdx);
        if(ret != 0)
            CAM_LOGW("[%s] ccuResume fail",__FUNCTION__);
    }
    m_pThreadRaw->resumeAFThread();
}

static MY_FORCEINLINE bool operator!=(const CameraMeteringArea_T& _new, const CameraMeteringArea_T& _old)
{
    MBOOL bIsNoWeight = MTRUE;
    if(_new.u4Count == 0)
        return MFALSE;

    if(_new.u4Count != _old.u4Count)
    {
        return MTRUE;
    }

    for(unsigned int i=0; i < _new.u4Count; ++i)
    {
        if(_new.rAreas[i].i4Weight != 0)
            bIsNoWeight = MFALSE;
    }

    if(bIsNoWeight) return MFALSE;

    for(unsigned int i=0; i < _new.u4Count; ++i)
    {
        if (_new.rAreas[i].i4Weight != _old.rAreas[i].i4Weight){
            return MTRUE;
        }
        if (_new.rAreas[i].i4Left != _old.rAreas[i].i4Left){
            return MTRUE;
        }
        if (_new.rAreas[i].i4Top != _old.rAreas[i].i4Top){
            return MTRUE;
        }
        if (_new.rAreas[i].i4Right != _old.rAreas[i].i4Right){
            return MTRUE;
        }
        if (_new.rAreas[i].i4Bottom != _old.rAreas[i].i4Bottom){
            return MTRUE;
        }
    }

    return MFALSE;
}

MBOOL
Hal3ARawImp::
setParams(Param_T const &rNewParam, MBOOL bUpdateScenario)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    AE_PARAM_SET_INFO rAEParam;
    std::lock_guard<std::mutex> autoLock(m3AOperMtx2);
    if (FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCalibration()) {
        rAEParam.bIsAELock = MTRUE;
        IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, MTRUE);
        IAeMgr::getInstance().setAEParams(m_i4SensorDev, rAEParam);
        return MTRUE;
    }
    if (!m_bEnable3ASetParams){
        m_rParam = rNewParam;
        return MTRUE;
    }
    // ====================================== Macro define =============================================
#define F(_f, _arg) { \
    if((rNewParam._arg != m_rParam._arg) || m_bIsFirstSetParams) { \
        _f(m_i4SensorDev, rNewParam._arg); }}

#define F2(_f, _arg, _arg2) { \
    if(((rNewParam._arg != m_rParam._arg) || (rNewParam._arg2 != m_rParam._arg2)) || m_bIsFirstSetParams) { \
        _f(m_i4SensorDev, rNewParam._arg, rNewParam._arg2); }}

#define FZoom(_f) { \
        if(rNewParam.rScaleCropRect.i4Xwidth != 0 && rNewParam.rScaleCropRect.i4Yheight != 0) { \
            if(((rNewParam.rScaleCropRect.i4Xoffset != m_rParam.rScaleCropRect.i4Xoffset) \
                || (rNewParam.rScaleCropRect.i4Yoffset != m_rParam.rScaleCropRect.i4Yoffset) \
                || (rNewParam.rScaleCropRect.i4Xwidth != m_rParam.rScaleCropRect.i4Xwidth) \
                || (rNewParam.rScaleCropRect.i4Yheight != m_rParam.rScaleCropRect.i4Yheight)) || m_bIsFirstSetParams) { \
                _f(m_i4SensorDev, rNewParam.rScaleCropRect.i4Xoffset,rNewParam.rScaleCropRect.i4Yoffset,rNewParam.rScaleCropRect.i4Xwidth,rNewParam.rScaleCropRect.i4Yheight); }}}


#if CAM3_LSC_FEATURE_EN
    // ====================================== Shading =============================================
    if (m_fgEnableShadingMeta){
        if((rNewParam.u1ShadingMode != m_rParam.u1ShadingMode) || m_bIsFirstSetParams)
        {
            NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(rNewParam.u1ShadingMode ? MTRUE : MFALSE);
        }
    }
#endif

    // ====================================== AE ==============================================
    rAEParam.i4CustomPlineMode = rNewParam.i4CustomPlineMode;
    rAEParam.i4ManualPlineIdx = rNewParam.i4ManualPlineIdx;
    rAEParam.bIsAELock = rNewParam.bIsAELock;
    rAEParam.bBlackLvlLock = rNewParam.u1BlackLvlLock;
    rAEParam.i4MinFps = rNewParam.i4MinFps;
    rAEParam.i4MaxFps = rNewParam.i4MaxFps;
    rAEParam.u4AeMeterMode = rNewParam.u4AeMeterMode;
    rAEParam.i4RotateDegree = rNewParam.i4RotateDegree;
    rAEParam.i4IsoSpeedMode = rNewParam.i4IsoSpeedMode;
    rAEParam.i4ExpIndex = rNewParam.i4ExpIndex;
    rAEParam.fExpCompStep = rNewParam.fExpCompStep;
    rAEParam.u4AeMode = rNewParam.u4AeMode;
    rAEParam.i4DenoiseMode = rNewParam.i4DenoiseMode;
    rAEParam.u4AntiBandingMode = rNewParam.u4AntiBandingMode;
    rAEParam.u4CamMode = rNewParam.u4CamMode;
    rAEParam.u4ShotMode = rNewParam.u4ShotMode;
    rAEParam.u4SceneMode = rNewParam.u4SceneMode;
    rAEParam.u1HdrMode = rNewParam.u1HdrMode;
    rAEParam.i4ZoomRatio = rNewParam.i4ZoomRatio;
    rAEParam.rMeteringAreas = rNewParam.rMeteringAreas;
    rAEParam.u4ZoomXOffset = rNewParam.rScaleCropRect.i4Xoffset;
    rAEParam.u4ZoomYOffset = rNewParam.rScaleCropRect.i4Yoffset;
    rAEParam.u4ZoomWidth = rNewParam.rScaleCropRect.i4Xwidth;
    rAEParam.u4ZoomHeight = rNewParam.rScaleCropRect.i4Yheight;
    rAEParam.u1IsDummyFrame  = rNewParam.u1Dummy|rNewParam.bDummyAfterCapture;
    IAeMgr::getInstance().setAEParams(m_i4SensorDev, rAEParam);

    if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF && m_i4SubsampleCount == 1)
    {
        AE_SENSOR_PARAM_T strSensorParams;
        if(rNewParam.i8ExposureTimeDummy == 0 && rNewParam.i4SensitivityDummy == 0 && rNewParam.i8FrameDurationDummy == 0)
        {
            strSensorParams.u4Sensitivity   = rNewParam.i4Sensitivity;
            strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTime;
            strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
        }
        else // For dummy frame setting
        {
            strSensorParams.u4Sensitivity   = rNewParam.i4SensitivityDummy;
            strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTimeDummy;
            strSensorParams.u8FrameDuration = rNewParam.i8FrameDurationDummy;
        }
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSensorParams, reinterpret_cast<MINTPTR>(&(strSensorParams)), NULL, NULL, NULL);
        NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->lockRatio(MTRUE);
        m_u4AutoAECount=0;
    } else {
        m_u4AutoAECount++;
        if(m_u4AutoAECount>=5)
        {
            NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->lockRatio(MFALSE);
        }
    }

    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetDynamicHFPS, rNewParam.i4DynamicSubsampleCount * 30, NULL, NULL, NULL);

    if( bUpdateScenario == MTRUE && ((rNewParam.bIsFDReady != m_rParam.bIsFDReady) || m_bIsFirstSetParams) )
    {
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetFDSkipCalAE, (rNewParam.bIsFDReady ==0)? MTRUE:MFALSE, NULL, NULL, NULL);
    }

    //please send the flag of super night mode by this API
    MINT32 enNight = property_get_int32("vendor.lsc.force.superNight", 0);
    NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableSuperNightMode(enNight);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] setAEMode(%d)", __FUNCTION__, rNewParam.u4AeMode);
    //test raw exposure will set set dummy after long exp one frame.
    // So 3A HAL will receive 2 long exp, test time out
    if(rNewParam.i8ExposureTimeDummy == 0)
        cust_setSpecialLongExpOnOff(rNewParam.i8ExposureTime);
    else
        cust_setSpecialLongExpOnOff(rNewParam.i8ExposureTimeDummy);
    // ====================================== AWB ==============================================
    if(m_i4OverrideMinFrameRate == 0 && m_i4OverrideMaxFrameRate == 0)
    {
        IAwbMgr::getInstance().setMaxFPS(m_i4SensorDev, rNewParam.i4MaxFps);
    }
    F(IAwbMgr::getInstance().setAWBLock, bIsAWBLock);
    F(IAwbMgr::getInstance().setAWBMode, u4AwbMode);
    // TODO
#if 0
    IAwbMgr::getInstance().setMWBColorTemperature(m_i4SensorDev, rNewParam.i4MWBColorTemperature);
#endif
    FZoom(IAwbMgr::getInstance().setZoomWinInfo);
    F(IAwbMgr::getInstance().setColorCorrectionMode, u1ColorCorrectMode);

    if (rNewParam.u4AwbMode == MTK_CONTROL_AWB_MODE_OFF &&
        rNewParam.u1ColorCorrectMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        MBOOL bNoEqual = MFALSE;
        for(int i=0; i<4; ++i)
        {
            if(rNewParam.fColorCorrectGain[i] != m_rParam.fColorCorrectGain[i]){
                bNoEqual = MTRUE;
                break;
            }
        }
        if(bNoEqual)
        {
            IAwbMgr::getInstance().setColorCorrectionGain(m_i4SensorDev, rNewParam.fColorCorrectGain[0], rNewParam.fColorCorrectGain[1], rNewParam.fColorCorrectGain[3]);
        }
    }
    F(IAwbMgr::getInstance().setMWBColorTemperature, i4AwbValue);

#if CAM3_FLASH_FEATURE_EN
    // ====================================== Flash ==============================================
    if(((rNewParam.u4AeMode != m_rParam.u4AeMode) || (rNewParam.u4StrobeMode != m_rParam.u4StrobeMode)) || m_bIsFirstSetParams)
        FlashMgr::getInstance(m_i4SensorDev)->setAeFlashMode(rNewParam.u4AeMode, rNewParam.u4StrobeMode);

    if((rNewParam.u4CamMode != m_rParam.u4CamMode) || m_bIsFirstSetParams)
        FlashMgr::getInstance(m_i4SensorDev)->setCamMode(rNewParam.u4CamMode);

    if(((rNewParam.i4ExpIndex != m_rParam.i4ExpIndex) || (rNewParam.fExpCompStep != m_rParam.fExpCompStep)) || m_bIsFirstSetParams)
        FlashMgr::getInstance(m_i4SensorDev)->setEvComp(rNewParam.i4ExpIndex, rNewParam.fExpCompStep);

    if((rNewParam.i4ZoomRatio != m_rParam.i4ZoomRatio) || m_bIsFirstSetParams)
        FlashMgr::getInstance(m_i4SensorDev)->setDigZoom(rNewParam.i4ZoomRatio);

    if((rNewParam.i8ExposureTime != m_rParam.i8ExposureTime) || m_bIsFirstSetParams)
        FlashMgr::getInstance(m_i4SensorDev)->setManualExposureTime(rNewParam.i8ExposureTime);

    if((rNewParam.i4Sensitivity != m_rParam.i4Sensitivity) || m_bIsFirstSetParams)
        FlashMgr::getInstance(m_i4SensorDev)->setManualSensitivity(rNewParam.i4Sensitivity);

    if((rNewParam.i4FlashCalEn != m_rParam.i4FlashCalEn) || m_bIsFirstSetParams)
    {
        if(rNewParam.i4FlashCalEn)
            FlashMgr::getInstance(m_i4SensorDev)->cctSetSpModeQuickCalibration2();    // flash calibration enable
        else
            FlashMgr::getInstance(m_i4SensorDev)->cctSetSpModeNormal();    // flash calibration disable
    }

    if((rNewParam.u1SubFlashCustomization != m_rParam.u1SubFlashCustomization) || m_bIsFirstSetParams)
        FlashHal::getInstance(m_i4SensorDev)->setSubFlashCustomization(rNewParam.u1SubFlashCustomization);
#endif

    if(rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD || rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT) {
        if((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_PREVIEW || m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG) && isStrobeBVTrigger() && m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH)
            m_bIsRecordingFlash = MTRUE;
    } else {
        m_bIsRecordingFlash = MFALSE;
    }
#if CAM3_FLICKER_FEATURE_EN
    // ====================================== Flicker ==============================================
    if((rNewParam.u4AntiBandingMode != m_rParam.u4AntiBandingMode) || m_bIsFirstSetParams)
        IFlickerHal::getInstance(m_i4SensorDev)->setFlickerMode(rNewParam.u4AntiBandingMode);
#endif

    // ====================================== FlowCtrl ==============================================
    //This not use, because ae commannd test_ev_compensation_basic/test_ev_compensation_advanced fail
    //m_u1LastCaptureIntent = m_rParam.u1CaptureIntent;
    m_rParam = rNewParam;
    m_sPreParam = m_sParam;
    m_bIsFirstSetParams = MFALSE;

    CAM_LOGD_IF(m_3ALogEnable, "[%s] m_rParam.u1ShadingMapMode(%d)", __FUNCTION__, m_rParam.u1ShadingMapMode);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setAfParams(AF_Param_T const &rNewParam)
{
        //if (!m_bEnable3ASetParams) return MTRUE;
#if CAM3_AF_FEATURE_EN
        // ====================================== AF ==============================================
        IThreadRaw::AFParam_T rAFPAram;
        rAFPAram.i4MagicNum = rNewParam.i4MagicNum;
        rAFPAram.u4AfMode = rNewParam.u4AfMode;
        rAFPAram.rFocusAreas = rNewParam.rFocusAreas;

        if( m_i4OperMode == NSIspTuning::EOperMode_Normal)
            rAFPAram.fFocusDistance = rNewParam.fFocusDistance;
        else
            rAFPAram.fFocusDistance = -1;

        rAFPAram.u1AfTrig  = rNewParam.u1AfTrig;
        rAFPAram.u1PrecapTrig  = rNewParam.u1PrecapTrig;
        rAFPAram.u1FlashTrigAF = chkLampFlashOnCond();
        rAFPAram.u1AfPause = rNewParam.u1AfPause;
        rAFPAram.u1MZOn = rNewParam.u1MZOn;
        rAFPAram.bFaceDetectEnable = m_bFaceDetectEnable;
        rAFPAram.rScaleCropArea = rNewParam.rScaleCropArea;
        rAFPAram.bEnable3ASetParams = m_bEnable3ASetParams;
        rAFPAram.u4AFNVRAMIndex = m_rParam.u4AFNVRAMIndex;
        rAFPAram.u1ZSDCaptureIntent = rNewParam.u1ZSDCaptureIntent;
        rAFPAram.u1CaptureIntent = rNewParam.u1CaptureIntent;
        //
        IPDMgr::getInstance().setRequest(m_i4SensorDev, rNewParam.i4MagicNum);
        m_pThreadRaw->sendRequest(ECmd_AFUpdate,(MINTPTR)&rAFPAram);
        m_rAfParam = rNewParam;
        m_rThreadRawAFPAram = rAFPAram;
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setStereoParams(Stereo_Param_T const & rNewParam)
{
    MINT32 LogEnable = ( m_3ALogEnable || (rNewParam.i4Sync3ASwitchOn == NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON) );
    CAM_LOGD_IF( LogEnable, "Dev(%d) i4FrameNum(%d) MasterIdx = %d, SlaveIdx = %d, m_bIsSkipSync3A(%d) rNewParam(%d,%d,%d)(%d,%d) switchOn(%d)", m_i4SensorDev, rNewParam.i4FrameNum, rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx, m_bIsSkipSync3A,
                rNewParam.i4Sync2AMode,
                rNewParam.i4SyncAFMode,
                rNewParam.i4HwSyncMode,
                rNewParam.bIsByFrame,
                rNewParam.bIsDummyFrame,
                rNewParam.i4Sync3ASwitchOn);
#if CAM3_STEREO_FEATURE_EN
    // dymanic enale/disable Sync3A
    if(!m_bIsSkipSync3A)
    {
        MINT32 i4IsDoSync3A = getIsDoSync3A(rNewParam.i4Sync3ASwitchOn);
        MINT32 i4IsStart = 0;
        ISync3AMgr::getInstance()->sendSync3ACtrl(ISync3AMgr::ESync3ACtrl_GetIsStart,  reinterpret_cast<MINTPTR>(&i4IsStart), NULL);
        MINT32 i4IsUninitSync = ISync3AMgr::getInstance()->isUninitSync(m_i4SensorDev, rNewParam);

        MBOOL fgIsInit = ISync3AMgr::getInstance()->isInit();
        if( rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF ||
            rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE ||
            rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM )
        {
            if(!fgIsInit && (rNewParam.i4MasterIdx != rNewParam.i4SlaveIdx) )
                ISync3AMgr::getInstance()->init(0, rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx);

            if(i4IsStart==MTRUE)
                m_pTaskMgr->sendEvent(ECmd_Sync3AStart);
            else
                m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);

        } else if(rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE)
        {
            if(fgIsInit && i4IsUninitSync == 1)
                ISync3AMgr::getInstance()->uninit();
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
        } else if(rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE)
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);

        //if(i4IsDoSync3A == MTRUE)
            ISync3AMgr::getInstance()->setStereoParams(rNewParam);
    }

    if (rNewParam.i4MasterIdx != m_rLastStereoParam.i4MasterIdx)
    {
        m_rLastStereoParam.bIsChange4Master = MTRUE;
    }
    else
    {
        m_rLastStereoParam.bIsChange4Master = MFALSE;
    }

    if (rNewParam.i4SlaveIdx != m_rLastStereoParam.i4SlaveIdx)
    {
        m_rLastStereoParam.bIsChange4Slave = MTRUE;
    }
    else
    {
        m_rLastStereoParam.bIsChange4Slave = MFALSE;
    }

    MBOOL isChanged = MFALSE;
    if(rNewParam.i4MasterIdx != 0 || rNewParam.i4SlaveIdx != 0)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.i4MasterIdx = rNewParam.i4MasterIdx;
        m_rLastStereoParam.i4SlaveIdx = rNewParam.i4SlaveIdx;
    }
    if(rNewParam.i4Sync2AMode != NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.i4Sync2AMode = rNewParam.i4Sync2AMode;
    }
    if(rNewParam.i4SyncAFMode != NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_IDLE)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.i4SyncAFMode = rNewParam.i4SyncAFMode;
    }
    if(rNewParam.i4HwSyncMode!= NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_IDLE)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.i4HwSyncMode = rNewParam.i4HwSyncMode;
    }
    if(rNewParam.bIsByFrame != m_rLastStereoParam.bIsByFrame)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.bIsByFrame = rNewParam.bIsByFrame;
    }
    if(rNewParam.bIsDummyFrame != m_rLastStereoParam.bIsDummyFrame)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.bIsDummyFrame = rNewParam.bIsDummyFrame;
    }
    if(rNewParam.i4Sync3ASwitchOn != m_rLastStereoParam.i4Sync3ASwitchOn)
    {
        isChanged = MTRUE;
        m_rLastStereoParam.i4Sync3ASwitchOn = rNewParam.i4Sync3ASwitchOn;
    }

    if(isChanged)
    {
        CAM_LOGD("[%s] Dev(%d) m_rLastStereoParam (%d,%d)(%d,%d,%d)(%d,%d)switchOn(%d)IsChange(%d,%d)", __FUNCTION__,
        m_i4SensorDev,
        m_rLastStereoParam.i4MasterIdx, m_rLastStereoParam.i4SlaveIdx,
        m_rLastStereoParam.i4Sync2AMode,
        m_rLastStereoParam.i4SyncAFMode,
        m_rLastStereoParam.i4HwSyncMode,
        m_rLastStereoParam.bIsByFrame,
        m_rLastStereoParam.bIsDummyFrame,
        m_rLastStereoParam.i4Sync3ASwitchOn,
        m_rLastStereoParam.bIsChange4Master,
        m_rLastStereoParam.bIsChange4Slave);
    }
#endif
    return MTRUE;
}

MINT32
Hal3ARawImp::
getIsDoSync3A(MINT32 i4Sync3ASwitchOn)
{
    MINT32 i4Ret = MFALSE;
#if CAM3_STEREO_FEATURE_EN
    MINT32 i4SwitchCamCount = ISync3AMgr::getInstance()->getSwitchCamCount();
    if(i4SwitchCamCount == 2 || i4Sync3ASwitchOn == NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON)
        i4Ret = MTRUE;

    CAM_LOGD_IF( m_3ALogEnable, "[%s] Dev(%d) i4SwitchCamCount = %d, i4Sync3ASwitchOn = %d", __FUNCTION__, m_i4SensorDev, i4SwitchCamCount, i4Sync3ASwitchOn);
#endif
    return i4Ret;
}

MBOOL
Hal3ARawImp::
autoFocus()
{
#if CAM3_AF_FEATURE_EN
    CAM_LOGD("[%s()] +", __FUNCTION__);
    EventOpt rOpt;
    rOpt.fgPreflashCond = chkPreFlashOnCond();
    rOpt.fgLampflashCond = chkLampFlashOnCond();
    rOpt.bStrobeBVTrigger = isStrobeBVTrigger();
    m_pTaskMgr->sendEvent(ECmd_TouchAEStart, rOpt);
    CAM_LOGD("[%s()] -", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
cancelAutoFocus()
{
#if CAM3_AF_FEATURE_EN
    CAM_LOGD("[%s()] +", __FUNCTION__);
    EventOpt rOpt;
    rOpt.fgPreflashCond = chkPreFlashOnCond();
    rOpt.fgLampflashCond = chkLampFlashOnCond();
    rOpt.bStrobeBVTrigger = isStrobeBVTrigger();
    m_pTaskMgr->sendEvent(ECmd_TouchAEEnd, rOpt);
    CAM_LOGD("[%s()] -", __FUNCTION__);
#endif
    return MTRUE;
}

MVOID
Hal3ARawImp::
setFDEnable(MBOOL fgEnable)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] fgEnable(%d)", __FUNCTION__, fgEnable);
    m_bFaceDetectEnable = fgEnable;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetFDenable, fgEnable, NULL, NULL, NULL);
    IAwbMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    if (!m_bFaceDetectEnable)
        m_faceNum = 0;
}

MBOOL
Hal3ARawImp::
setFDInfo(MVOID* prFaces, MVOID* prAFFaces)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] m_bFaceDetectEnable(%d)", __FUNCTION__, m_bFaceDetectEnable);
    if (m_bFaceDetectEnable)
    {
        MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)prAFFaces;
        m_faceNum = pFaces->number_of_faces;
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance(m_i4SensorDev).setOTFDInfo( prAFFaces, 0);
#endif
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetFDInfo, reinterpret_cast<MINTPTR>(prFaces), m_i4TgWidth, m_i4TgHeight, NULL);
        IAwbMgr::getInstance().setFDInfo(m_i4SensorDev, prFaces, m_i4TgWidth, m_i4TgHeight);
        FlashMgr::getInstance(m_i4SensorDev)->setFDInfo(prFaces, m_i4TgWidth, m_i4TgHeight);
    }
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setOTInfo(MVOID* prOT, MVOID* prAFOT)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance(m_i4SensorDev).setOTFDInfo( prAFOT, 1);
#endif
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetOTInfo, reinterpret_cast<MINTPTR>(prOT), NULL, NULL, NULL);
    return MTRUE;
}


MUINT32
Hal3ARawImp::
getNvramIndex(const CAM_IDX_QRY_COMB& info, EModule_T module, MUINT32& index)
{
    if (m_bMappingQueryFlag_3A)
    {
        index = (MUINT32)m_pIdxMgr->query(static_cast<MUINT32>(m_i4SensorDev), module, info, __FUNCTION__);
    }

    if (m_scenarioIdx >= m_scenarioNs)
    {
        CAM_LOGE("[Hal3ARawImp::getNvramIndex] Scenario Index out of bounds, Scenario idx: %d, Total: %d\n", m_scenarioIdx, m_scenarioNs);
        return 0;
    }

    if (m_pScenarios)
    {
        CAM_LOGD_IF(m_i4IdxCacheLogEnable,
            "[IdxCache::From:%s] [Dev:%d-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d)",
            m_bMappingQueryFlag_3A ? "IdxMgr":"IdxCache", m_i4SensorDev, strEModule[module], module, index, (*m_pScenarios)[m_scenarioIdx], strEIspProfile[info.eIspProfile], strESensorMode[info.eSensorMode], info.eFrontBin, info.eSize, info.eFlash, strEApp[info.eApp], info.eFaceDetection, info.eZoom_Idx, info.eLV_Idx, info.eCT_Idx);
    }

    return index;
}

MBOOL
Hal3ARawImp::
setFlashLightOnOff(MBOOL bOnOff, MBOOL bMainPre, MINT32 i4P1DoneSttNum /* = -1 */)
{
#if CAM3_FLASH_FEATURE_EN
    CAM_LOGD_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    if (m_bIsFlashOpened == bOnOff)
    {
        CAM_LOGD("[%s] flashlight unchanged, bOnOff(%d)", __FUNCTION__, bOnOff);
    }
    else if (!bOnOff)
    {
        //modified to update strobe state after capture for ae manager
        MBOOL bStrobeMode = MFALSE;
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);

        IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_OFF);

        if (bMainPre){
            FlashHal::getInstance(m_i4SensorDev)->setOnOff(MFALSE, FLASH_HAL_SCENARIO_MAIN_FLASH);
            FlashMgr::getInstance(m_i4SensorDev)->setSubFlashState(e_noflash);
        }
        else
            FlashHal::getInstance(m_i4SensorDev)->setOnOff(MFALSE, FLASH_HAL_SCENARIO_VIDEO_TORCH);
    }
    else //flash on
    {
        if (bMainPre){
            FlashHal::getInstance(m_i4SensorDev)->setOnOff(MTRUE, FLASH_HAL_SCENARIO_MAIN_FLASH);
            //m_pThreadRaw->postToEventThread(E_3AEvent_MainFlash_On, FLASH_HAL_SCENARIO_MAIN_FLASH);
            IAwbMgr::getInstance().SetMainFlashInfo(m_i4SensorDev, MTRUE);
            }

        else{
            FlashHal::getInstance(m_i4SensorDev)->setOnOff(MTRUE, FLASH_HAL_SCENARIO_VIDEO_TORCH);
            IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_ON);
            }
        CAM_LOGD("[%s] OFF FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
        m_bFlashOpenedByTask = MFALSE;
        m_i4FlashType = -1;
    }
    if(m_bIsFlashOpened != bOnOff)
    {
        CAM_LOGD("[%s] flashlight is changed (%d->%d), and assign bOnOff to m_bIsFlashOpened", __FUNCTION__, m_bIsFlashOpened, bOnOff);
        m_bIsFlashOpened = bOnOff;
        handleBadPicture(m_rParam.i4MagicNum, i4P1DoneSttNum);
    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setPreFlashOnOff(MBOOL bOnOff)
{
#if CAM3_FLASH_FEATURE_EN
    if(m_bIsFlashOpened != bOnOff)
    {
        handleBadPicture(m_rParam.i4MagicNum);
        CAM_LOGD("[%s] flashlight is changed (%d->%d)", __FUNCTION__, m_bIsFlashOpened, bOnOff);
    }
    m_bIsFlashOpened = bOnOff;
    MY_LOG_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    FlashHal::getInstance(m_i4SensorDev)->setPfOnOff(bOnOff);
    MY_LOG_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
isNeedTurnOnPreFlash() const
{
	return MFALSE;
}

MBOOL
Hal3ARawImp::
chkMainFlashOnCond() const
{
    // Main Flash
    FlashPolicy_T rFlashPolicy;
    rFlashPolicy.bIsFlashOnCapture = isFlashOnCapture();
    rFlashPolicy.bIsFlashCharging = (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING);
    FlashHal::getInstance(m_i4SensorDev)->hasHw(rFlashPolicy.i4HWSuppportFlash);
    rFlashPolicy.i4HWSuppportFlash |= FlashHal::getInstance(m_i4SensorDev)->getSubFlashCustomization();

    CAM_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), i4HWSuppportFlash(%d)", __FUNCTION__,
        rFlashPolicy.bIsFlashOnCapture,
        rFlashPolicy.bIsFlashCharging,
        rFlashPolicy.i4HWSuppportFlash);
    return HAL3A_POLICY_MAIN_FLASH_ONOFF(m_i4SensorDev, m_rParam, rFlashPolicy);
}

MBOOL
Hal3ARawImp::
chkPreFlashOnCond() const
{
    // Torch
    FlashPolicy_T rFlashPolicy;
    rFlashPolicy.bIsFlashOnCapture = isFlashOnCapture();
    rFlashPolicy.bIsFlashCharging = (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING);
    FlashHal::getInstance(m_i4SensorDev)->hasHw(rFlashPolicy.i4HWSuppportFlash);
    rFlashPolicy.i4HWSuppportFlash |= FlashHal::getInstance(m_i4SensorDev)->getSubFlashCustomization();

    CAM_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), i4HWSuppportFlash(%d)", __FUNCTION__,
        rFlashPolicy.bIsFlashOnCapture,
        rFlashPolicy.bIsFlashCharging,
        rFlashPolicy.i4HWSuppportFlash);
    return HAL3A_POLICY_TORCH_ONOFF(m_i4SensorDev, m_rParam, rFlashPolicy);
}

MBOOL
Hal3ARawImp::
chkLampFlashOnCond() const
{
    AE_PERFRAME_INFO_T AEPerframeInfo;
    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
    // for touch, precapture
    FlashPolicy_T rFlashPolicy;
    rFlashPolicy.bIsFlashOnCapture = isFlashOnCapture();
    rFlashPolicy.bIsFlashCharging = (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING);
    rFlashPolicy.bIsStrobeBVTrigger = AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger;
    FlashHal::getInstance(m_i4SensorDev)->hasHw(rFlashPolicy.i4HWSuppportFlash);
    rFlashPolicy.i4HWSuppportFlash |= FlashHal::getInstance(m_i4SensorDev)->getSubFlashCustomization();

    CAM_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), i4HWSuppportFlash(%d), AEmode(%d)", __FUNCTION__,
        rFlashPolicy.bIsFlashOnCapture,
        rFlashPolicy.bIsFlashCharging,
        rFlashPolicy.i4HWSuppportFlash,
        m_rParam.u4AeMode);
    return HAL3A_POLICY_LAMP_FLASH_ONOFF(m_i4SensorDev, m_rParam, rFlashPolicy);
}

MBOOL
Hal3ARawImp::
isStrobeBVTrigger() const
{
    AE_PERFRAME_INFO_T AEPerframeInfo;
    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
    return AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger;
}

MBOOL
Hal3ARawImp::
isFlashOnCapture() const
{
    return FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture();
}

MBOOL
Hal3ARawImp::
dumpP1Params(MINT32 i4MagicNum)
{
    return MTRUE;
}

MINT32
Hal3ARawImp::
getCurrResult(MUINT32 i4FrmId, MINT32 /*i4SubsampleIndex*/) const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]+ i4MagicNum(%d) m_bIsHighQualityCaptureOn:%d", __FUNCTION__, i4FrmId, m_bIsHighQualityCaptureOn);

    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState = MTK_CONTROL_AWB_STATE_INACTIVE;

    AAA_TRACE_HAL(getAllResult);

    // ResultPool - To update Vector info
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(i4FrmId);
    if(pAllResult == NULL)
    {
        CAM_LOGE("[%s] pAllResult is NULL", __FUNCTION__);
        AAA_TRACE_END_HAL;
        return MFALSE;
    }
    AAA_TRACE_END_HAL;

    // ResultPool - To update resultPool
    HALResultToMeta_T     rHALResult;
    AEResultToMeta_T      rAEResult;
    AWBResultToMeta_T     rAWBResult;
    ISPResultToMeta_T     rISPResult;
    LSCResultToMeta_T     rLSCResult;
    FLASHResultToMeta_T   rFLASHResult;
    FLKResultToMeta_T     rFLKResult;
    LSCConfigResult_T     rLSCConfigResult;

    //===== Get HAL result =====
    rHALResult.i4FrmId = i4FrmId;
    rHALResult.fgKeep = m_bIsCapEnd;
    rHALResult.fgFdEnable = m_bFaceDetectEnable;
    rHALResult.fgBadPicture = MFALSE;

    rHALResult.u1SceneMode = m_rParam.u4SceneMode;
    rHALResult.i4ZoomRatio = m_rParam.i4ZoomRatio;

    //===== Get AE result =====
    AEResultInfo_T AEResultInfo;
    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEResultInfo.AEPerframeInfo);

    //===== Get AE FD region =====
    // ResultPool - put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
    {
        std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
        AAA_TRACE_MGR(getAEFD);
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetFDMeteringAreaInfo, reinterpret_cast<MINTPTR>(&(pAllResult->vecAEROI)), m_i4TgWidth, m_i4TgHeight, NULL);
        AAA_TRACE_END_MGR;
    }
    AAA_TRACE_MGR(getAEState);
    rAEResult.u1AeMode = m_rParam.u4AeMode;
    MUINT8 u1AeState = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.i4AEState;
    MBOOL isExistPrecap = (m_pTaskMgr!= NULL) ? m_pTaskMgr ->isExistTask(TASK_ENUM_3A_PRECAPTURE) : 0;
    if (isExistPrecap == MTRUE)
        u1AeState = MTK_CONTROL_AE_STATE_PRECAPTURE;

    if (AEResultInfo.AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rAEResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rAEResult.u1AeState = u1AeState;

    rAEResult.fgAeBvTrigger = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger;
    // AutoHdrResult
    rAEResult.i4AutoHdrResult = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.i4HdrOnOff;

    rAEResult.i4SensorGain = AEResultInfo.AEPerframeInfo.rAEISPInfo.u4P1SensorGain;
    rAEResult.i4IspGain = AEResultInfo.AEPerframeInfo.rAEISPInfo.u4P1DGNGain;
    rAEResult.i4LuxIndex = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.u4LuxValue_x10000;
    rAEResult.u8P1Exposuretime_ns = AEResultInfo.AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns;
    AAA_TRACE_END_MGR;

    AAA_TRACE_MGR(getSensorParams);
    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rAEResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rAEResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rAEResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
    if (m_rParam.u1RollingShutterSkew)
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetSensorRollingShutter, reinterpret_cast<MINTPTR>(&(rAEResult.i8SensorRollingShutterSkew)), NULL, NULL, NULL);

    // update AE config result to ResutlPool
    AEResultConfig_T AEResultConfig;
    IAeMgr::getInstance().configReg(m_i4SensorDev, &AEResultConfig);
    AAA_TRACE_END_MGR;

    //===== Get AWB result =====

    //===== Get AWB FD region =====
    // ResultPool - put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
    {
        std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
        AAA_TRACE_MGR(getAwbFD);
        IAwbMgr::getInstance().getFocusArea(m_i4SensorDev, pAllResult->vecAWBROI, m_i4TgWidth, m_i4TgHeight);
        AAA_TRACE_END_MGR;
    }

    AAA_TRACE_MGR(getAWBState);
    IAwbMgr::getInstance().getAWBState(m_i4SensorDev, eAwbState);
    rAWBResult.u1AwbState= eAwbState;
    AWB_GAIN_T rAwbGain  = {};
    IAwbMgr::getInstance().getAWBGain(m_i4SensorDev, rAwbGain, rAWBResult.i4AwbGainScaleUint);
    rAWBResult.i4AwbGain[0] = rAwbGain.i4R;
    rAWBResult.i4AwbGain[1] = rAwbGain.i4G;
    rAWBResult.i4AwbGain[2] = rAwbGain.i4B;

    // update AWB config result to ResutlPool
    AWBResultConfig_T AWBResultConfig;
    IAwbMgr::getInstance().configReg(m_i4SensorDev, &AWBResultConfig);
    AAA_TRACE_END_MGR;

    AAA_TRACE_MGR(getColorCorrectGain);
    IAwbMgr::getInstance().getColorCorrectionGain(m_i4SensorDev, rAWBResult.fColorCorrectGain[0],rAWBResult.fColorCorrectGain[1],rAWBResult.fColorCorrectGain[3]);
    rAWBResult.fColorCorrectGain[2] = rAWBResult.fColorCorrectGain[1];
    AAA_TRACE_END_MGR;

    AWBResultInfo_T AWBResultInfo;
    IAwbMgr::getInstance().getAWBInfo(m_i4SensorDev, AWBResultInfo.AWBInfo4ISP);

    MUINT32 u4AwbRangeMin = 0; MUINT32 u4AwbRangeMax = 0;
    IAwbMgr::getInstance().getSupportMWBColorTemperature(m_i4SensorDev, u4AwbRangeMax, u4AwbRangeMin);
    rAWBResult.i4AwbAvailableRange[0] = static_cast<MINT32>(u4AwbRangeMin);
    rAWBResult.i4AwbAvailableRange[1] = static_cast<MINT32>(u4AwbRangeMax);

    // TODO
#if 0
    AAA_TRACE_MGR(MWBColorTemperature);
    IAwbMgr::getInstance().getSupportMWBColorTemperature(m_i4SensorDev, rAWBResult.i4MWBColorTemperatureMax, rAWBResult.i4MWBColorTemperatureMin);
    AAA_TRACE_END_MGR;
#endif

    //===== Get FLASH result =====
    AAA_TRACE_MGR(getFlashState);
#if CAM3_FLASH_FEATURE_EN
    // check the flash state is updated or not
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4FrmId,E_FLASH_RESULTTOMETA, __FUNCTION__);
    MBOOL bIsUpdated = MFALSE;
    if(pFLASHResult != NULL) {
        if( pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_READY ){
            bIsUpdated = MTRUE;
            CAM_LOGD("[%s] FlashState(%d) is updated", __FUNCTION__, pFLASHResult->u1FlashState);
        }
    }

    rFLASHResult.u1FlashState =
        (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_UNAVAILABLE) ?
        MTK_FLASH_STATE_UNAVAILABLE :
        (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING) ?
        MTK_FLASH_STATE_CHARGING :
        bIsUpdated ? pFLASHResult->u1FlashState :
        (FlashHal::getInstance(m_i4SensorDev)->isAFLampOn() ? MTK_FLASH_STATE_FIRED : MTK_FLASH_STATE_READY);

    if( CUST_ENABLE_FLASH_DURING_TOUCH() )
        rFLASHResult.bCustEnableFlash = CUST_ENABLE_FLASH_DURING_TOUCH();

    MINT32 flashCalResult = FlashMgr::getInstance(m_i4SensorDev)->cctGetQuickCalibrationResult();
    if(rFLASHResult.i4FlashCalResult != flashCalResult)
    {
        CAM_LOGD("[%s] rFLASHResult.i4FlashCalResult(%d=>%d)", __FUNCTION__, rFLASHResult.i4FlashCalResult, flashCalResult);
        rFLASHResult.i4FlashCalResult = flashCalResult;
    }
    rFLASHResult.u1SubFlashState = FlashMgr::getInstance(m_i4SensorDev)->getSubFlashState();

#endif
    AAA_TRACE_END_MGR;

    //===== Get Flicker result =====
    AAA_TRACE_MGR(getFlkState);
#if CAM3_FLICKER_FEATURE_EN
    MINT32 i4FlkStatus = IAeMgr::getInstance().getAEAutoFlickerState(m_i4SensorDev);
    MUINT8 u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_NONE;
    if (i4FlkStatus == 0) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_50HZ;
    if (i4FlkStatus == 1) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_60HZ;
    rFLKResult.u1SceneFlk = (MUINT8)u1ScnFlk;
    // update FLK config result to ResutlPool
    FLKResultConfig_T FLKResultConfig;
    IFlickerHal::getInstance(m_i4SensorDev)->getHWCfgReg(&FLKResultConfig);
#endif
    AAA_TRACE_END_MGR;

    //===== Get LSC result =====
    AAA_TRACE_LSC(getLscResult);
#if CAM3_LSC_FEATURE_EN
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));

    //Get the computing result of LSC to result pool
    pLsc->getLSCResultPool(&rLSCConfigResult);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] #lsc(%d) m_rParam.i4RawType(%d) m_bIsCapEnd(%d)", __FUNCTION__, pLsc->getOnOff(), m_rParam.i4RawType, m_bIsCapEnd);
    MBOOL fgRequireShadingIntent = 1; /*
        (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG);*/

    rLSCResult.i4CurrTblIndex = -1;
    if (pLsc->getOnOff())
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] #(%d) LSC table to metadata", __FUNCTION__, i4FrmId);
        //pLsc->getCurrTbl(pAllResult->vecLscData);
        rLSCResult.i4CurrTblIndex = pLsc->getCurrTblIndex();
        rLSCResult.i4CurrTsfIndex = pLsc->getCurTsfoIndex();
        CAM_LOGD_IF(m_3ALogEnable, "[%s] CurrTblIndex:%d", __FUNCTION__, rLSCResult.i4CurrTblIndex);
    }
    rLSCResult.fgShadingMapOn = (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON);
    #if 0
    // Shading map
    if (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON)
    {
        rResult.vecShadingMap.resize(m_rParam.u1ShadingMapXGrid*m_rParam.u1ShadingMapYGrid*4);
        MFLOAT* pfShadingMap = rResult.vecShadingMap.editArray();
        NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->getGainTable(3, m_rParam.u1ShadingMapXGrid, m_rParam.u1ShadingMapYGrid, pfShadingMap);
    }
    #endif
#endif
    AAA_TRACE_END_LSC;


    //===== Get Stereo result =====
    AAA_TRACE_MGR(StereoFeatureEnable);
    rHALResult.i4StereoWarning = 0;
#if CAM3_STEREO_FEATURE_EN
    MUINT32 u4AEStableMagic = CUST_GET_SYNC3A_AESTABLE_MAGIC();
    if (ISync3AMgr::getInstance()->isActive() && i4FrmId > u4AEStableMagic)
    {
        MINT32 i4Master = ISync3AMgr::getInstance()->getMasterDev();
        MINT32 i4Slave = ISync3AMgr::getInstance()->getSlaveDev();
        if(i4Master == m_i4SensorDev)
        {
            AE_PERFRAME_INFO_T AEPerframeInfoMaster;
            AE_PERFRAME_INFO_T AEPerframeInfoSlave;
            IAeMgr::getInstance().getAEInfo(i4Master, AEPerframeInfoMaster);
            IAeMgr::getInstance().getAEInfo(i4Slave, AEPerframeInfoSlave);
            // large lv diff
            MINT32 i4LvMaster = AEPerframeInfoMaster.rAEISPInfo.i4LightValue_x10;
            MINT32 i4LvSlave = AEPerframeInfoSlave.rAEISPInfo.i4LightValue_x10;
            rHALResult.i4StereoWarning |= CUST_LENS_COVER_COUNT(i4LvMaster, i4LvSlave);
            // low light
            rHALResult.i4StereoWarning |= ((max(i4LvMaster, i4LvSlave) < 10) ? 0x2 : 0);
            // close shot
            MINT32 i4FocusDistance      = m_rAFStaticInfo.lastFocusPos;  // Current DAC
            MINT32 i4MaxFocusDistance   = m_rAFStaticInfo.maxAfTablePos; // AFTable maxDAC
            MINT32 i4MinFocusDistance   = m_rAFStaticInfo.minAfTablePos; // AFTable minDAC
            if(i4FocusDistance > (i4MaxFocusDistance - (i4MaxFocusDistance - i4MinFocusDistance) / 5))
                rHALResult.i4StereoWarning |= 0x4;

            CAM_LOGD_IF(rHALResult.i4StereoWarning,
                "[%s] warning(%d) lv(%d,%d), fd(%d,%d,%d)", __FUNCTION__, rHALResult.i4StereoWarning, i4LvMaster, i4LvSlave, i4FocusDistance, i4MaxFocusDistance, i4MinFocusDistance);
        }
    }
#endif
    AAA_TRACE_END_MGR;


    // 3A fill Header Offset. So vecDbgIspInfo need to stay
    AAA_TRACE_MGR(get2AExif);
    //===== Get Exif result =====
    if (m_rParam.u1IsGetExif || m_bIsCapEnd)
    {
        // protect vector before use vector
        std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);

        if(pAllResult->vecExifInfo.size()==0)
            pAllResult->vecExifInfo.resize(1);
        get3AEXIFInfo(pAllResult->vecExifInfo.back());


        if (m_bDbgInfoEnable) {
            // debug exif
            if(pAllResult->vecDbg3AInfo.size()==0)
                pAllResult->vecDbg3AInfo.resize(sizeof(AAA_DEBUG_INFO1_T));
            if(pAllResult->vecDbgIspInfo.size()==0)
                pAllResult->vecDbgIspInfo.resize(sizeof(AAA_DEBUG_INFO2_T));
            if(pAllResult->vecDbgShadTbl.size()==0)
                pAllResult->vecDbgShadTbl.resize(sizeof(DEBUG_SHAD_ARRAY_INFO_T));

            AAA_DEBUG_INFO1_T& rDbg3AInfo = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResult->vecDbg3AInfo.editArray());
            AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
            DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl = *reinterpret_cast<DEBUG_SHAD_ARRAY_INFO_T*>(pAllResult->vecDbgShadTbl.editArray());

            if(pAllResult->vecDbgShadTbl.size() != 0 && pAllResult->vecDbgIspInfo.size() != 0 && pAllResult->vecDbg3AInfo.size() != 0)
            {
                CAM_LOGD_IF(m_3ALogEnable, "[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)pAllResult->vecDbgShadTbl.size(), (MINT32)pAllResult->vecDbgIspInfo.size(), (MINT32)pAllResult->vecDbg3AInfo.size(),
                                            &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);
                getP1DbgInfo(rDbg3AInfo, rDbgShadTbl, rDbgIspInfo);
            }
            else
                CAM_LOGE("[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)pAllResult->vecDbgShadTbl.size(), (MINT32)pAllResult->vecDbgIspInfo.size(), (MINT32)pAllResult->vecDbg3AInfo.size(),
                                            &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);

#if CAM3_STEREO_FEATURE_EN
            // debug exif for N3D
            if (ISync3AMgr::getInstance()->isActive())
            {
                pAllResult->vecDbgN3DInfo.resize(sizeof(N3D_DEBUG_INFO_T));
                N3D_DEBUG_INFO_T& rDbgN3DInfo = *reinterpret_cast<N3D_DEBUG_INFO_T*>(pAllResult->vecDbgN3DInfo.editArray());
                if(pAllResult->vecDbgN3DInfo.size() != 0)
                    ISync3AMgr::getInstance()->getSync3A()->setDebugInfo((void*)(&rDbgN3DInfo));
            }
#endif
        }
    }
    AAA_TRACE_END_MGR;

    //===== update modules result to ResultPool =====
    AAA_TRACE_MGR(updateModuleResult42A);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_HAL_RESULTTOMETA, &rHALResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_AE_RESULTTOMETA, &rAEResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_AWB_RESULTTOMETA, &rAWBResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_LSC_RESULTTOMETA, &rLSCResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_FLASH_RESULTTOMETA, &rFLASHResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_FLK_RESULTTOMETA, &rFLKResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_LSC_CONFIGRESULTTOISP, &rLSCConfigResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_AE_CONFIGRESULTTOISP, &AEResultConfig);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_FLK_CONFIGRESULTTOISP, &FLKResultConfig);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_AWB_CONFIGRESULTTOISP, &AWBResultConfig);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_AE_RESULTINFO, &AEResultInfo);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_AWB_RESULTINFO4ISP, &AWBResultInfo);
    AAA_TRACE_END_MGR;

    //===== get AF state, and update to ResultPool =====
    // MW Handshake : Early Call Back. ZSD-flash capture,MW need Af state to judge precapture done
    AAA_TRACE_MGR(getEarlyCBState);
    AFResult_T rAFResultFromMgr;
    IAfMgr::getInstance(m_i4SensorDev).getResult(rAFResultFromMgr);
    EARLY_CALL_BACK rEarlyCB;
    rEarlyCB.u1AfState = rAFResultFromMgr.afState;
    rEarlyCB.u1AeState = rAEResult.u1AeState;
    rEarlyCB.u1AeMode = rAEResult.u1AeMode;
    rEarlyCB.u1FlashState = rFLASHResult.u1FlashState;
    rEarlyCB.fgAeBvTrigger = rAEResult.fgAeBvTrigger;

    // CCT 3A Need to overwrite AF state when supportAF
    MINT32 IsSupportAF = m_rAFStaticInfo.isAfSupport;
    MINT32 i4AFEnable = 0;
    MUINT32 i4OutLens = 0;
    IAfMgr::getInstance(m_i4SensorDev).CCTOPAFGetEnableInfo( (MVOID*)&i4AFEnable, &i4OutLens);
    if( IsSupportAF && (!i4AFEnable) && rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED &&  rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] overwrite AF state(%d)", __FUNCTION__, rEarlyCB.u1AfState);
        rEarlyCB.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
    }

    // update stereo master slave index
#if CAM3_STEREO_FEATURE_EN
    if(m_rLastStereoParam.i4MasterIdx != 0 || m_rLastStereoParam.i4SlaveIdx != 0)
    {
        rEarlyCB.i4MasterIdx = m_rLastStereoParam.i4MasterIdx;
        rEarlyCB.i4SlaveIdx = m_rLastStereoParam.i4SlaveIdx;
        CAM_LOGD_IF(m_3ALogEnable, "[%s] rEarlyCB (MasterIdx, SlaveIdx)(%d,%d)", __FUNCTION__, rEarlyCB.i4MasterIdx, rEarlyCB.i4SlaveIdx);
    }
#endif

    m_pResultPoolObj->updateEarlyCB(i4FrmId, rEarlyCB);
    AAA_TRACE_END_MGR;

    //===== AF disable, then 3A thread to get AF result ====
    //if(!IsSupportAF)
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] disable AF (%d), 3A thread to get AF result MagicNum(%d)", __FUNCTION__, IsSupportAF, m_rThreadRawAFPAram.i4MagicNum);
        IThreadRaw::AFParam_T AFParam;
        AFParam = m_rThreadRawAFPAram;
        m_pThreadRaw->getCurrResult4AF(AFParam.i4MagicNum, AFParam);
    }

    //===== Callback autoHdr information =====
    // Auto HDR Detection, default is -1
    // -1: auto hdr off, 0: not detected, 1: detected, -2: use algo
    MINT32 debugData = property_get_int32("vendor.debug.aaa_state.hdrMode", -2);
    MINT32 autoHdrRes = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.i4HdrOnOff;
    CAM_LOGD_IF(m_3ALogEnable, "[%s] autoHdrRes(%d), debugData(%d)",__FUNCTION__, autoHdrRes, debugData);
    if(CC_UNLIKELY((debugData > -2) && (debugData < 2)))
    {
        CAM_LOGD("[%s] use debugData(%d), ori autoHdrRes(%d)",__FUNCTION__, debugData, autoHdrRes);
        autoHdrRes = debugData ;
    }
    if (m_pCbSet)
    {
        m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_HDRD_RESULT, reinterpret_cast<MINTPTR> (&autoHdrRes), 0, 0);
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s]- i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return MTRUE;
}

MINT32
Hal3ARawImp::
getCurrentHwId() const
{
    MINT32 idx = 0;
    if(m_pCamIO != NULL)
    {
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX,
                            (MINTPTR)&idx, 0, 0);
        CAM_LOGD_IF(m_3ALogEnable, "[%s] idx(%d)", __FUNCTION__, idx);
    }
    else
        CAM_LOGE("[%s] m_pCamIO(%p), p1node call CamIO configPipe fail, so p1node don't config 3A HAL", __FUNCTION__, m_pCamIO);
    return idx;
}

MVOID
Hal3ARawImp::
setSensorMode(MINT32 i4SensorMode)
{
    CAM_LOGD("[%s] mode(%d)", __FUNCTION__, i4SensorMode);
    m_u4SensorMode = i4SensorMode;
}

MBOOL
Hal3ARawImp::
postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam)
{
    std::lock_guard<std::mutex> autoLock(mPostCommandMtx);
    CAM_LOGD_IF(m_3ALogEnable, "[%s]+ cmd(%d)", __FUNCTION__, r3ACmd);
    if (pParam != NULL)
    {
        CAM_LOGD_IF(m_i4ShortExpCount != 0, "Check Validate %d, Short Exp Count %d, Frame %d", pParam->iValidateOpt, m_i4ShortExpCount, m_vShortExpFrame[m_i4ShortExpCount]);
        if (pParam->iValidateOpt == ParamIspProfile_T::EParamValidate_None && m_vShortExpFrame[m_i4ShortExpCount++])
        {
            AAASensorMgr::getInstance().setSensorExpTime(m_i4SensorDev, 100000);
            CAM_LOGD("[%s] set next frame rate 10fps", __FUNCTION__);
            return MTRUE;
        }
    }
    else
    {
        CAM_LOGE("[%s] pParam (ParamIspProfile_T) should not be NULL", __FUNCTION__);
        return MFALSE;
    }

    if(m_pThreadRaw && m_pCcuCtrl3ACtrl)
        m_pThreadRaw->postToEventThread(E_3AEvent_CCU_CB);

#if 0
#if CAM3_STEREO_FEATURE_EN
    //MTK_MULTI_CAM_FEATURE_MODE_ZOOM = 0, MTK_MULTI_CAM_FEATURE_MODE_VSDOF = 1, MTK_MULTI_CAM_FEATURE_MODE_DENOISE = 2
    if(m_i4CamFeatureMode == 0 && m_i4QuerryCBNum != 0)
    {
        MINT32 i4TgWidth = 0, i4TgHeight = 0;
        MUINT32 u4AAWidth = 0, u4AAHight = 0;
        MUINT32 u4AFWidth = 0, u4AFHeight = 0;

        m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&i4TgWidth, (MINTPTR)&i4TgHeight, 0);
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_HBIN_INFO, (MINTPTR)&u4AAWidth, (MINTPTR)&u4AAHight, 0);
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO, (MINTPTR)&u4AFWidth, (MINTPTR)&u4AFHeight, 0);

        CAM_LOGD("[%s] TgWidth(%d/%d) TgHeight(%d/%d) BinWidth(%d/%d) BinHeight(%d/%d) HbinWidth(%d/%d) HbinHeight(%d/%d)", __FUNCTION__,
                    m_i4TgWidth, i4TgWidth, m_i4TgHeight, i4TgHeight, m_i4BinWidth, u4AFWidth, m_i4BinHeight, u4AFHeight, m_i4HbinWidth, u4AAWidth, m_i4HbinHeight, u4AAHight);

        if(i4TgWidth == m_i4TgWidth && i4TgHeight == m_i4TgHeight && u4AFWidth == m_i4BinWidth && u4AFHeight == m_i4BinHeight && u4AAWidth == m_i4HbinWidth && u4AAHight == m_i4HbinHeight)
            m_i4QuerryCBNum = 0;
        else
        {
            updateTGInfo();
            IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight, u4AAWidth, u4AAHight);
#if CAM3_FLICKER_FEATURE_EN
            IFlickerHal::getInstance(m_i4SensorDev)->setSensorMode(m_u4SensorMode, u4AAWidth, u4AAHight);
#endif
            //get AAO size information for LSC
            shadingConfig_T lscConfig;
            IAeMgr::getInstance().getAAOSize(m_i4SensorDev, lscConfig.AAOBlockW, lscConfig.AAOBlockH);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(lscConfig.AAOstrideSize)), NULL, NULL, NULL);
            MBOOL bAESMBuffermode = MFALSE;
            //set Hbin for TSFS
            lscConfig.u4HBinWidth = u4AAWidth;
            lscConfig.u4HBinHeight= u4AAHight;
            ILscMgr *pLscMgr=ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
            pLscMgr->config(lscConfig);
            pLscMgr->setSensorMode(static_cast<ESensorMode_T>(m_u4SensorMode), u4AFWidth, u4AFHeight, MFALSE);
            m_i4BinWidth = u4AFWidth;
            m_i4BinHeight = u4AFHeight;
            m_i4HbinWidth = u4AAWidth;
            m_i4HbinHeight = u4AAHight;
        }
        m_i4QuerryCBNum--;
    }
#endif
#endif
    /*****************************
     *     Handle 3A command
     *****************************/
    AAA_TRACE_HAL(Handle3Acommand);
    if(r3ACmd != ECmd_Update)
    {
        // Use r3ACmd to create the tasks of precapture, touch, capture,etc.
        EventOpt rOpt;
        rOpt.fgPreflashCond = chkPreFlashOnCond();
        rOpt.fgLampflashCond = chkLampFlashOnCond();
        rOpt.bStrobeBVTrigger = isStrobeBVTrigger();
        m_pTaskMgr->sendEvent(r3ACmd, rOpt);
    }
    AAA_TRACE_END_HAL;

    /*****************************
     *     Capture Flow
     *****************************/
    AAA_TRACE_HAL(CaptureFlow);
    MBOOL bDummyAfterCapture = m_rParam.bDummyAfterCapture;
    CAM_LOGD_IF(m_3ALogEnable,"[%s] bDummyAfterCapture(%d) ", __FUNCTION__, bDummyAfterCapture);

    if ((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) &&
        (r3ACmd == ECmd_Update) && (m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF) && (!bDummyAfterCapture))
    {
        MBOOL bSkipZsdCap = MFALSE;

        // high quality cap
        MINT32 iForceHqc = ::property_get_int32("vendor.debug.camera.force_hqc", 1);
        if ( chkMainFlashOnCond() ||
            ((iForceHqc & 0x1) && (m_rParam.u1IsStartCapture && m_rParam.i4RawType == NSIspTuning::ERawType_Pure)) ||
            (iForceHqc & 0x2))
        {
            m_bIsHighQualityCaptureOn = MTRUE;
            CAM_LOGD("[HQC] Start : Request(#%d)", pParam->i4MagicNum);
        } else
        // ZSD cap
        {
            AE_PERFRAME_INFO_T AEPerframeInfo;
            IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
            MINT32 i4OperMode = m_i4OperMode;
            MBOOL bAELock = MFALSE;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAELock, reinterpret_cast<MINTPTR>(&bAELock), NULL, NULL, NULL);
            MINT32 i4EVCap = 0;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetEVCompensateIndex, reinterpret_cast<MINTPTR>(&i4EVCap), NULL, NULL, NULL);
            MBOOL bIsEVchaged = m_i4EVCap != i4EVCap;
            MBOOL bIsStartCapture = m_rParam.u1IsStartCapture;

            // skip capture when the 2nd, 3rd, etc. (C shot)
            if(m_rParam.i4Cshot)
                bSkipZsdCap = MTRUE;

            if (bIsEVchaged){
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetEVCompensateIndex, reinterpret_cast<MINTPTR>(&m_i4EVCap), NULL, NULL, NULL);
                //bAELock = MFALSE;
            }
            CAM_LOGD("[%s] i4OperMode(%d) mbIsHDRShot(%d) bAELock(%d) EV(%d) EVchanged (%d) CaptureIntent(%d)", __FUNCTION__, i4OperMode, mbIsHDRShot, bAELock, m_i4EVCap, bIsEVchaged, m_u1LastCaptureIntent);
            /*
            some senerio should not updateCaptureParams
            1. capture with flash
            2. HDR shot, camera3
            3. i4OperMode != EOperMode_Meta
            4. AELock is set
            5. 2nd frame,3rd frame,etc. (C shot)
            */
#if CAM3_FLASH_FEATURE_EN
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetEVCompensateIndex, reinterpret_cast<MINTPTR>(&i4EVCap), NULL, NULL, NULL);
            if(!mbIsHDRShot && (i4OperMode != EOperMode_Meta) &&
                (!bAELock) && !bSkipZsdCap)
#endif
            {
                AE_MODE_CFG_T previewInfo;
                AE_MODE_CFG_T rCaptureInfo;
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetCapParams, reinterpret_cast<MINTPTR>(&rCaptureInfo), NULL, NULL, NULL);

                previewInfo.u4ExposureMode = rCaptureInfo.u4ExposureMode;
                previewInfo.u2FrameRate = rCaptureInfo.u2FrameRate;
                previewInfo.u4RealISO = rCaptureInfo.u4RealISO;
                previewInfo.u4Eposuretime = rCaptureInfo.u4Eposuretime;
                previewInfo.u4AfeGain = rCaptureInfo.u4AfeGain;
                previewInfo.u4IspGain = rCaptureInfo.u4IspGain;
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetCaptureParams, reinterpret_cast<MINTPTR>(&(previewInfo)), NULL, NULL, NULL);
            }
        }

        // reset flag of HDR shot
        if (mbIsHDRShot)
            mbIsHDRShot = MFALSE;

        // notify AE for single frame HDR
        if (m_rParam.u1IsSingleFrameHDR)
        {
            MBOOL bZCHDRShot = MTRUE;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetzCHDRShot, bZCHDRShot, NULL, NULL, NULL);
        }
        if(!bSkipZsdCap)
            m_pTaskMgr->sendEvent(ECmd_CaptureStart);
    }

    // get current task to remapping ae state
    TaskInfo rInfo = m_pTaskMgr->getCurrTaskInfo();
    MINT32 i4AeState = taskEnumToAeState(rInfo.i4Enum);
    if(i4AeState > 0)
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetAEMgrPreState, (MUINT32)i4AeState, NULL, NULL, NULL);
    AAA_TRACE_END_HAL;

    /*****************************
     *     3A Execution
     *****************************/
    if(r3ACmd == ECmd_Update)
    {
        // Guarantee Sync3A enable or disable
#if CAM3_STEREO_FEATURE_EN
        if(m_rLastStereoParam.bIsDummyFrame)
        {
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
        }
#endif

        /*****************************
         *     Prepare Statistic
         *****************************/
        AAA_TRACE_HAL(PrepareStatistic);
        MBOOL isExistPrecap = m_pTaskMgr->isExistTask(TASK_ENUM_3A_PRECAPTURE);
        MBOOL isExistCap = m_pTaskMgr->isExistTask(TASK_ENUM_3A_CAPTURE);

        TaskData rData;
        rData.i4RequestMagic = pParam->i4MagicNum;
        rData.bFlashOnOff = m_bIsFlashOpened;
        rData.bIsHqCap = m_bIsHighQualityCaptureOn;
        rData.bIsAfTriggerInPrecap = ((m_rAfParam.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START) && (m_rAfParam.u1PrecapTrig || isExistPrecap));
        rData.pThreadRaw = (MVOID*)m_pThreadRaw;
        rData.i4FlashTypeByTask = m_i4FlashType;
        rData.bFlashOpenByTask = m_bFlashOpenedByTask;
        rData.bByPassStt = (m_rParam.u1IsStartCapture && m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF) || (m_rParam.bByPassStt);
        rData.bMainFlashOn = chkMainFlashOnCond();
        rData.i4FrameNum = m_rParam.i4FrameNum;
        CAM_LOGD_IF(rData.bByPassStt, "[%s] bByPassStt(%d) IsStartCapture (%d) u4AeMode(%d) mbByPassStt(%d)", __FUNCTION__, rData.bByPassStt, m_rParam.u1IsStartCapture, m_rParam.u4AeMode, m_rParam.bByPassStt);
        m_i4SttMagicNumber = m_pTaskMgr->prepareStt(rData,Task_Update_3A);

    //EApp_T eApp;
    //EFaceDetection_T eFaceDetection;
    CAM_IDX_QRY_COMB rQueryCommandQ;
    rQueryCommandQ.eIspProfile = m_rParam.eIspProfile;
    rQueryCommandQ.eSensorMode = static_cast<NSIspTuning::ESensorMode_T>(m_u4SensorMode);
    if(m_bFrontalBin)
        rQueryCommandQ.eFrontBin = NSIspTuning::EFrontBin_Yes;
    else
        rQueryCommandQ.eFrontBin = NSIspTuning::EFrontBin_No;

    if(m_bFaceDetectEnable)
    {
        if (m_faceNum)
           rQueryCommandQ.eFaceDetection = NSIspTuning::EFaceDetection_Yes;
        else
           rQueryCommandQ.eFaceDetection = NSIspTuning::EFaceDetection_No;
    }
    IspTuningCustom* pIspTuningCustom = IspTuningCustom::createInstance((MUINT32)m_i4SensorDev, m_i4SensorIdx);

    MBOOL ret = NSCam::ATMsEventMgr::getInstance()->getFactor("APP_NAME", m_strAppName);
    rQueryCommandQ.eApp = (EApp_T)pIspTuningCustom->map_AppName_to_MappingInfo(m_strAppName.c_str());
    rQueryCommandQ.eZoom_Idx = (EZoom_T)pIspTuningCustom->map_zoom_value_to_index(m_rParam.i4ZoomRatio);
    rQueryCommandQ.eSize = (ESize_T)pIspTuningCustom->map_Target_Size_to_MappingInfo(m_rParam.targetSize.w, m_rParam.targetSize.h, "hal3araw_postcommand");

    // check the flash state, and update to index map mgr
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(m_i4SttMagicNumber,E_FLASH_RESULTTOMETA,__FUNCTION__);
    if(pFLASHResult != NULL) {
        if( pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED){
            if((FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() == LIB3A_FLASH_MODE_FORCE_TORCH))
                rQueryCommandQ.eFlash = EFlash_Torch;
            else if(m_i4FlashType == FLASH_HAL_SCENARIO_PRE_FLASH)
                rQueryCommandQ.eFlash = EFlash_PreFlash;
            else
                rQueryCommandQ.eFlash = EFlash_MainFlash;
        } else
            rQueryCommandQ.eFlash = EFlash_No;
    }

    MINT32 i4IsFlashOnCapture = isFlashOnCapture();
    if(i4IsFlashOnCapture && m_bIsHighQualityCaptureOn)
        rQueryCommandQ.eFlash = EFlash_MainFlash;

    AAA_TRACE_HAL(IdxMgrQuery);

    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_IspProfile, (MVOID*)&(rQueryCommandQ.eIspProfile));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_SensorMode, (MVOID*)&(rQueryCommandQ.eSensorMode));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_FrontBin, (MVOID*)&(rQueryCommandQ.eFrontBin));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_Flash, (MVOID*)&(rQueryCommandQ.eFlash));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_DriverIC, (MVOID*)&(rQueryCommandQ.eDriverIC));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_FaceDetection, (MVOID*)&(rQueryCommandQ.eFaceDetection));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_App, (MVOID*)&(rQueryCommandQ.eApp));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_Zoom, (MVOID*)&(rQueryCommandQ.eZoom_Idx));
    IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev))->setMappingInfoByDim(static_cast<MUINT32>(m_i4SensorDev), m_rParam.i4MagicNum, EDim_Size, (MVOID*)&(rQueryCommandQ.eSize));

    CAM_IDX_QRY_COMB rMapping_Info;
    m_pIdxMgr->getMappingInfo(static_cast<MUINT32>(m_i4SensorDev), rMapping_Info, m_rParam.i4MagicNum);

    MINT32 cacheEnable = property_get_int32("vendor.debug.cache.mode", 1);

    if (memcmp(&(rMapping_Info), &m_Mapping_Info_3A, sizeof(CAM_IDX_QRY_COMB)) || !cacheEnable){
        memcpy(&m_Mapping_Info_3A, &(rMapping_Info), sizeof(CAM_IDX_QRY_COMB));
        m_bMappingQueryFlag_3A = MTRUE;
    } else {
        CAM_LOGD_IF(m_3ALogEnable,"[%s] use last mapping info(%d)", __FUNCTION__, pParam->i4MagicNum);
        m_bMappingQueryFlag_3A = MFALSE;
    }

    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AE, m_3A_Index.u4AENVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AWB, m_3A_Index.u4AWBNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AF, m_3A_Index.u4AFNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AE, m_3A_Index.u4FlashAENVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_Calibration, m_3A_Index.u4FlashCaliNVRAMIndex);

    memset(&rMapping_Info.eISO_Idx, 0, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_LTM, m_3A_Index.u4LTMNVRAMIndex);

    AAA_TRACE_END_HAL;

    MINT32 isMvHDREnable = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isMvHDREnable();
    CAM_LOGD_IF(m_3ALogEnable, "[%s] MvHDREnable(%d), StrobeMode(%d) i4IsFlashOnCapture(%d) bIsHighQualityCaptureOn(%d) rQueryCommandQ eFlash(%d) i4Enum(%d)->i4AeState(%d)", __FUNCTION__, isMvHDREnable, m_rParam.u4StrobeMode, i4IsFlashOnCapture, m_bIsHighQualityCaptureOn, rQueryCommandQ.eFlash, rInfo.i4Enum, i4AeState);
    if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL) {
        // TODO : AE Mgr need to modify SCENARIO
        MINT32 i4Scenario = 0;
        if(m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH)
        {
            i4Scenario = CAM_SCENARIO_VIDEO;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetCamScenarioMode, i4Scenario, NULL, NULL, NULL);
        }
        else
        {
            i4Scenario = CAM_SCENARIO_PREVIEW;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetCamScenarioMode, i4Scenario, NULL, NULL, NULL);
        }
    }

    IAwbMgr::getInstance().setNVRAMIndex(m_i4SensorDev, m_3A_Index.u4AWBNVRAMIndex);
    IAfMgr::getInstance(m_i4SensorDev).setNVRAMIndex( m_3A_Index.u4AFNVRAMIndex);
    IPDMgr::getInstance().setNVRAMIndex( m_i4SensorDev, m_3A_Index.u4AFNVRAMIndex);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetNVRAMIndex, m_3A_Index.u4AENVRAMIndex, NULL, NULL, NULL);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AE, m_3A_Index.u4FlashAENVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_CALIBRATION, m_3A_Index.u4FlashCaliNVRAMIndex);
    IAwbMgr::getInstance().setStrobeNvramIdx(m_i4SensorDev, m_3A_Index.u4FlashAWBNVRAMIndex);
    IAwbMgr::getInstance().setFlashCaliNvramIdx(m_i4SensorDev, m_3A_Index.u4FlashCaliNVRAMIndex);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetISPNvramLTMIndex, m_3A_Index.u4LTMNVRAMIndex, NULL, NULL, NULL);
    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setAppInfo(rQueryCommandQ.eApp);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] NVRAM Index AE(%d) AWB(%d) AF(%d) F_AE(%d) F_AWB(%d) F_Cali(%d) OBC(%d)",
                    __FUNCTION__, m_3A_Index.u4AENVRAMIndex, m_3A_Index.u4AWBNVRAMIndex, m_3A_Index.u4AFNVRAMIndex, m_3A_Index.u4FlashAENVRAMIndex, m_3A_Index.u4FlashAWBNVRAMIndex, m_3A_Index.u4FlashCaliNVRAMIndex,m_3A_Index.u4LTMNVRAMIndex);

        CAM_LOGD_IF(m_3ALogEnable,"[%s] TaskData : Req(#%d)/Stt(#%d)/Frm(#%d)/Flash(%d)/HqCap(%d)/AfTrigger(%d)/FlashTypeByTask(%d)/FlashOpenByTask(%d)",
            __FUNCTION__, rData.i4RequestMagic,
            m_i4SttMagicNumber,
            rData.i4FrameNum,
            rData.bFlashOnOff,
            rData.bIsHqCap,
            rData.bIsAfTriggerInPrecap,
            rData.i4FlashTypeByTask,
            rData.bFlashOpenByTask);
        AAA_TRACE_END_HAL;

        /*****************************
         *     Execute task
         *****************************/
        AAA_TRACE_HAL(Execute2ATask);
        TASK_RESULT eResult = m_pTaskMgr->execute(Task_Update_3A);
        AAA_TRACE_END_HAL;

        /*****************************
         *     Open/Close Flashlight
         *****************************/
        AAA_TRACE_HAL(OpenCloseFlashlight);
        MBOOL isOpenFlash = MFALSE;
        MBOOL isCloseFlash = MFALSE;
        MBOOL isClosePreFlash = MFALSE;
        MBOOL isFlashchanged = MFALSE;
        MBOOL isMainFlashRestore = MFALSE;
        MBOOL isRestoreLock = MFALSE;
        // Open Flash
        if(eResult == TASK_RESULT_AFLAMP_REQUIRE)
            isOpenFlash = checkAndOpenFlash(FLASH_HAL_SCENARIO_AF_LAMP);
        else if(eResult == TASK_RESULT_PREFLASH_REQUIRE)
            isOpenFlash = checkAndOpenFlash(FLASH_HAL_SCENARIO_PRE_FLASH);
        else if(eResult == TASK_RESULT_MAINFLASH_RESTORE_REQUIRE)
            isMainFlashRestore = MTRUE;
        else if(eResult == TASK_RESULT_MAINFLASH_CAPTURE_END)
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_NotifyMainFlashON, MFALSE, NULL, NULL, NULL);
        else if(eResult == TASK_RESULT_CLOSE_PREFLASH_REQUIRE)
            isClosePreFlash = MTRUE;
        else if(eResult == TASK_RESULT_FLASH_RESTOREANDAPAELOCK)
            isRestoreLock = MTRUE;
        else if(eResult == TASK_RESULT_CLOSE_PREFLASH_REQUIRE_AND_FLASH_RESTOREANDAPAELOCK)
        {
            isClosePreFlash = MTRUE;
            isRestoreLock   = MTRUE;
        }
        if(isOpenFlash)
        {
            doBackup2A();
            MBOOL bStrobeMode = MTRUE;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);
            IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_ON);
            handleBadPicture(pParam->i4MagicNum);
        }

        // Close Flash
        if((m_pTaskMgr->queryTaskQueueSize(Task_Update_3A) == 1 && // only exist preview task
            m_pTaskMgr->queryTaskQueueSize(Task_Update_AF) == 1) || isClosePreFlash
          )
            isCloseFlash = checkAndCloseFlash();

        if(isCloseFlash || isMainFlashRestore)
        {
            doRestore2A();
            MBOOL bStrobeMode = MFALSE;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);
            IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_OFF);
            handleBadPicture(pParam->i4MagicNum);
        }

        if(isRestoreLock)
        {
            IAeMgr::getInstance().setAPAELock(m_i4SensorDev, MTRUE);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetState2Converge, NULL, NULL, NULL, NULL);
        }
        isFlashchanged = isOpenFlash || isCloseFlash;

#if CAM3_STEREO_FEATURE_EN
        //For DualCam control flash on off
        MBOOL bIsTurnOff =  HAL3A_POLICY_FLASH_ONOFF_DUALCAM(m_i4SensorDev, m_rParam, m_rLastStereoParam);
        if (bIsTurnOff)
        {
            FlashHal::getInstance(m_i4SensorDev)->setOnOff(MFALSE, FLASH_HAL_SCENARIO_MAIN_FLASH);
            m_bFlashOpenedByTask = MFALSE;
            m_i4FlashType = -1;
            CAM_LOGD("[%s] OFF FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
        }
#endif
        AAA_TRACE_END_HAL;

        /*****************************
         *     Capture End flow
         *****************************/
        AAA_TRACE_HAL(CaptureEndFlow);
        // check capture task is removed or not,
        // if the result is true, this frame is capture end.
        m_bIsCapEnd = isExistCap && !m_pTaskMgr->isExistTask(TASK_ENUM_3A_CAPTURE);

        if(m_bIsCapEnd == MTRUE){
          IAwbMgr::getInstance().SetMainFlashInfo(m_i4SensorDev, MFALSE);
          CAM_LOGD("[%s] set AWB main flash info False", __FUNCTION__);
        }

        // reset HQC flag
        if(m_bIsCapEnd && m_bIsHighQualityCaptureOn)
        {
            m_bIsHighQualityCaptureOn = MFALSE;
            CAM_LOGD("[HQC] End : Stt(#%d)", m_i4SttMagicNumber);
        }
        AAA_TRACE_END_HAL;


#if CAM3_FLASH_FEATURE_EN
        FlashMgr::getInstance(m_i4SensorDev)->updateFlashState();
#endif

        /*****************************
         *     Release Statistic
         *****************************/
        if (!m_rParam.bByPassStt)
        {
            m_pTaskMgr->releaseStt(Task_Update_3A);
        }

        m_u4LastRequestNumber = pParam->i4MagicNum;
    }

    /*****************************
     *     Stereo flow
     *****************************/
#if 0
#if CAM3_STEREO_FEATURE_EN
    if (r3ACmd == ECmd_PrecaptureStart)
    {
        // suspend Sync3A when precapture with flash
        if( (m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||
            ((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isStrobeBVTrigger()))
        {
            CAM_LOGD("[%s] skip Sync3A for precapture with flash +", __FUNCTION__);
            Stereo_Param_T rStereoParam;
            Stereo_Param_T rLastParam = m_rLastStereoParam;
            rStereoParam.i4Sync2AMode= NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
            rStereoParam.i4SyncAFMode= NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
            rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
            setStereoParams(rStereoParam);
            m_bIsSkipSync3A = MTRUE;
            m_rLastStereoParam = rLastParam;
            CAM_LOGD("[%s] m_rLastStereoParam(%d, %d, %d)", __FUNCTION__,
                m_rLastStereoParam.i4Sync2AMode,
                m_rLastStereoParam.i4SyncAFMode,
                m_rLastStereoParam.i4HwSyncMode);
        }
    }
    if(m_bIsCapEnd && m_bIsSkipSync3A)
    {
        // restore Sync3A
        CAM_LOGD("[%s] m_rLastStereoParam(%d, %d, %d)", __FUNCTION__,
                    m_rLastStereoParam.i4Sync2AMode,
                    m_rLastStereoParam.i4SyncAFMode,
                    m_rLastStereoParam.i4HwSyncMode);
        m_bIsSkipSync3A = MFALSE;
        setStereoParams(m_rLastStereoParam);
        CAM_LOGD("[%s] skip Sync3A for precapture with flash -", __FUNCTION__);
    }
#endif
#endif

    CAM_LOGD_IF(m_3ALogEnable,"Task done.");

    // query magic number after deque sw buffer.
    CAM_LOGD_IF(m_3ALogEnable,"[%s] m_i4SttMagicNumber = %d", __FUNCTION__, m_i4SttMagicNumber);

    do {
        android::sp<ISttBufQ> pSttBufQ;
        android::sp<ISttBufQ::DATA> pData;

        pSttBufQ = NSCam::ISttBufQ::getInstance(m_i4SensorDev);
        if (pSttBufQ == NULL) break;

        pData = pSttBufQ->deque_last();
        if (!pData.get()) {
            CAM_LOGE("SttBufQ empty!!");
            break;
        }
        if(pData->MagicNumberRequest != 0) {
            CAM_LOGW("SttBufQ request=%d -> request=%d", pData->MagicNumberRequest, m_u4LastRequestNumber);
        }
        pData->MagicNumberRequest = m_u4LastRequestNumber;

        pSttBufQ->enque(pData);
    } while(0);

    CAM_LOGD_IF(m_3ALogEnable, "[%s]-", __FUNCTION__);
    return MTRUE;
}

MRESULT
Hal3ARawImp::
updateTGInfo()
{
    //Before wait for VSirq of IspDrv, we need to query IHalsensor for the current TG info
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return E_3A_ERR;
    }
    const char* const callerName = "Hal3AQueryTG";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
    //Note that Middleware has configured sensor before
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = pHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);
    pHalSensor->destroyInstance(callerName);

    CAM_LOGD_IF(m_3ALogEnable, "m_i4SensorDev = %d, senInfo.TgInfo = %d\n", m_i4SensorDev, senInfo.TgInfo);

    m_u4TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2

    IAwbMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);

    m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&m_i4TgWidth, (MINTPTR)&m_i4TgHeight, 0);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] TG size(%d,%d)", __FUNCTION__, m_i4TgWidth, m_i4TgHeight);
    IFlickerHal::getInstance(m_i4SensorDev)->setTGInfo(m_u4TgInfo, m_i4TgWidth, m_i4TgHeight);

    return S_3A_OK;
}

MBOOL
Hal3ARawImp::
get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const
{
    AE_DEVICES_INFO_T rDeviceInfo;
    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rExifInfo.u4CapExposureTime = rAESensorInfo.u8ExposureTime / 1000;  // naro sec change to micro sec
#if CAM3_FLASH_FEATURE_EN
    if (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_FIRED ||
        FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_PARTIAL)
        rExifInfo.u4FlashLightTimeus = 30000;
    else
#endif
        rExifInfo.u4FlashLightTimeus = 0;

    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetSensorDeviceInfo, reinterpret_cast<MINTPTR>(&rDeviceInfo), NULL, NULL, NULL);
    rExifInfo.u4FNumber     = rDeviceInfo.u4LensFno; // Format: F2.8 = 28
    rExifInfo.u4FocalLength = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
    //rExifInfo.u4SceneMode   = m_rParam.u4SceneMode; // Scene mode   (SCENE_MODE_XXX)
    MINT32 i4AEMeterMode = 0;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAEMeterMode, reinterpret_cast<MINTPTR>(&i4AEMeterMode), NULL, NULL, NULL);
    switch (i4AEMeterMode)
    {
    case LIB3A_AE_METERING_MODE_AVERAGE:
        rExifInfo.u4AEMeterMode = eMeteringMode_Average;
        break;
    case LIB3A_AE_METERING_MODE_CENTER_WEIGHT:
        rExifInfo.u4AEMeterMode = eMeteringMode_Center;
        break;
    case LIB3A_AE_METERING_MODE_SOPT:
        rExifInfo.u4AEMeterMode = eMeteringMode_Spot;
        break;
    default:
        rExifInfo.u4AEMeterMode = eMeteringMode_Other;
        break;
    }
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetEVCompensateIndex, reinterpret_cast<MINTPTR>(&rExifInfo.i4AEExpBias), NULL, NULL, NULL); // Exposure index  (AE_EV_COMP_XX)
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetISOSpeedMode, reinterpret_cast<MINTPTR>(&rExifInfo.u4AEISOSpeed), NULL, NULL, NULL);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] AEISOSpeedMode:%d \n", __FUNCTION__, rExifInfo.u4AEISOSpeed);

    rExifInfo.u4AWBMode     = (m_rParam.u4AwbMode == MTK_CONTROL_AWB_MODE_AUTO) ? 0 : 1;
    switch (m_rParam.u4AwbMode)
    {
    case MTK_CONTROL_AWB_MODE_AUTO:
    case MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT:
    case MTK_CONTROL_AWB_MODE_TWILIGHT:
    case MTK_CONTROL_AWB_MODE_INCANDESCENT:
        rExifInfo.u4LightSource = eLightSourceId_Other;
        break;
    case MTK_CONTROL_AWB_MODE_DAYLIGHT:
        rExifInfo.u4LightSource = eLightSourceId_Daylight;
        break;
    case MTK_CONTROL_AWB_MODE_FLUORESCENT:
        rExifInfo.u4LightSource = eLightSourceId_Fluorescent;
        break;
#if 0
    case MTK_CONTROL_AWB_MODE_TUNGSTEN:
        rExifInfo.u4LightSource = eLightSourceId_Tungsten;
        break;
#endif
    case MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
        rExifInfo.u4LightSource = eLightSourceId_Cloudy;
        break;
    case MTK_CONTROL_AWB_MODE_SHADE:
        rExifInfo.u4LightSource = eLightSourceId_Shade;
        break;
    default:
        rExifInfo.u4LightSource = eLightSourceId_Other;
        break;
    }

    switch (m_rParam.u4SceneMode)
    {
    case MTK_CONTROL_SCENE_MODE_PORTRAIT:
        rExifInfo.u4ExpProgram = eExpProgramId_Portrait;
        break;
    case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
        rExifInfo.u4ExpProgram = eExpProgramId_Landscape;
        break;
    default:
        rExifInfo.u4ExpProgram = eExpProgramId_NotDefined;
        break;
    }

    switch (m_rParam.u4SceneMode)
    {
    case MTK_CONTROL_SCENE_MODE_DISABLED:
    case MTK_CONTROL_SCENE_MODE_NORMAL:
    case MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
    case MTK_CONTROL_SCENE_MODE_THEATRE:
    case MTK_CONTROL_SCENE_MODE_BEACH:
    case MTK_CONTROL_SCENE_MODE_SNOW:
    case MTK_CONTROL_SCENE_MODE_SUNSET:
    case MTK_CONTROL_SCENE_MODE_STEADYPHOTO:
    case MTK_CONTROL_SCENE_MODE_FIREWORKS:
    case MTK_CONTROL_SCENE_MODE_SPORTS:
    case MTK_CONTROL_SCENE_MODE_PARTY:
    case MTK_CONTROL_SCENE_MODE_CANDLELIGHT:
        rExifInfo.u4SceneCapType = eCapTypeId_Standard;
        break;
    case MTK_CONTROL_SCENE_MODE_PORTRAIT:
        rExifInfo.u4SceneCapType = eCapTypeId_Portrait;
        break;
    case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
        rExifInfo.u4SceneCapType = eCapTypeId_Landscape;
        break;
    case MTK_CONTROL_SCENE_MODE_NIGHT:
        rExifInfo.u4SceneCapType = eCapTypeId_Night;
        break;
    default:
        rExifInfo.u4SceneCapType = eCapTypeId_Standard;
        break;
    }

    return MTRUE;
}

MBOOL
Hal3ARawImp::
getASDInfo(ASDInfo_T &a_rASDInfo) const
{
    // AWB
    AWB_ASD_INFO_T rAWBASDInfo;
    IAwbMgr::getInstance().getASDInfo(m_i4SensorDev, rAWBASDInfo);
    a_rASDInfo.i4AWBRgain_X128 = rAWBASDInfo.i4AWBRgain_X128;
    a_rASDInfo.i4AWBBgain_X128 = rAWBASDInfo.i4AWBBgain_X128;
    a_rASDInfo.i4AWBRgain_D65_X128 = rAWBASDInfo.i4AWBRgain_D65_X128;
    a_rASDInfo.i4AWBBgain_D65_X128 = rAWBASDInfo.i4AWBBgain_D65_X128;
    a_rASDInfo.i4AWBRgain_CWF_X128 = rAWBASDInfo.i4AWBRgain_CWF_X128;
    a_rASDInfo.i4AWBBgain_CWF_X128 = rAWBASDInfo.i4AWBBgain_CWF_X128;
    a_rASDInfo.bAWBStable = rAWBASDInfo.bAWBStable;

#if CAM3_AF_FEATURE_EN
    // TODO : get these info from getResult or getStaticInfo
    #if 0
    a_rASDInfo.i4AFPos           = IAfMgr::getInstance(m_i4SensorDev).getAFPos();           // getResult:     lensPosition
    a_rASDInfo.pAFTable          = IAfMgr::getInstance(m_i4SensorDev).getAFTable();         // getStaticInfo: afTable
    a_rASDInfo.i4AFTableOffset   = IAfMgr::getInstance(m_i4SensorDev).getAFTableOffset();   // getStaticInfo: minAfTablePos
    a_rASDInfo.i4AFTableMacroIdx = IAfMgr::getInstance(m_i4SensorDev).getAFTableMacroIdx(); // getStaticInfo: maxAfTablePos
    a_rASDInfo.i4AFTableIdxNum   = IAfMgr::getInstance(m_i4SensorDev).getAFTableIdxNum();   // getStaticInfo: afTableStepCount
    a_rASDInfo.bAFStable         = IAfMgr::getInstance(m_i4SensorDev).getAFStable();        // getResult:     isFocusFinish
    #endif
#endif

    AE_PERFRAME_INFO_T AEPerframeInfo;
    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
    a_rASDInfo.i4AELv_x10 = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
    a_rASDInfo.bAEBacklit = ((AE_CONDITION_BACKLIGHT & AEPerframeInfo.rAEUpdateInfo.u4AECondition) > 0) ? MTRUE : MFALSE;
    a_rASDInfo.bAEStable = AEPerframeInfo.rAEISPInfo.bAEStable;
    a_rASDInfo.i2AEFaceDiffIndex = AEPerframeInfo.rAEUpdateInfo.i2AEFaceDiffIndex;
    //a_rASDInfo.i4AELv_x10 = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
    //a_rASDInfo.bAEBacklit = IAeMgr::getInstance().getAECondition(m_i4SensorDev, AE_CONDITION_BACKLIGHT);
    //a_rASDInfo.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4SensorDev);
    //a_rASDInfo.i2AEFaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4SensorDev);

    return MTRUE;
}

MBOOL _append_string(MUINT8 *&dst, const char src [], int &dst_remands)
{
    int src_len = strlen((const char*)src);
    MUINT8 *dst_ptr = dst;

    if(src_len > (dst_remands-2))
    {
        dst_remands = 0;
        return MFALSE;
    }

    *dst_ptr = 0;
    dst_ptr = (unsigned char*)strncat((char*)dst_ptr, (const char*)src, dst_remands);
    dst_ptr += src_len;
    *dst_ptr++ = 0;
    *dst_ptr = 0;

    dst_remands -= (src_len + 1);
    dst = dst_ptr;

    return MTRUE;
}
void _append_string_tag(MUINT8 *&dst, const char tag_name [], const char tag_value [], int &dst_remands, int &str_count)
{
    int tag_len = 3 + strlen((const char*)tag_name) + strlen((const char*)tag_value);
    if( tag_len <= dst_remands &&
    _append_string(dst, tag_name, dst_remands) &&
    _append_string(dst, tag_value, dst_remands) )
    {
        str_count++;
    }
    else
    {
        //string buffer is full
        CAM_LOGE("[%s] EXIF string buffer is full", __FUNCTION__);
        dst_remands = 0;
    }
}

MBOOL
Hal3ARawImp::
getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const
{
    // AE Flash Flicker Shading debug info
    rDbg3AInfo1.hdr.u4KeyID = AAA_DEBUG_KEYID;
    rDbg3AInfo1.hdr.u4ModuleCount = MODULE_NUM(7,5);

    rDbg3AInfo1.hdr.u4AEDebugInfoOffset        = sizeof(rDbg3AInfo1.hdr);
    rDbg3AInfo1.hdr.u4AFDebugInfoOffset        = rDbg3AInfo1.hdr.u4AEDebugInfoOffset + sizeof(AE_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4FlashDebugInfoOffset     = rDbg3AInfo1.hdr.u4AFDebugInfoOffset + sizeof(AF_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4FlickerDebugInfoOffset   = rDbg3AInfo1.hdr.u4FlashDebugInfoOffset + sizeof(FLASH_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4ShadingDebugInfoOffset   = rDbg3AInfo1.hdr.u4FlickerDebugInfoOffset + sizeof(FLICKER_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4AEDebugDataOffset        = rDbg3AInfo1.hdr.u4ShadingDebugInfoOffset + sizeof(SHADING_DEBUG_INFO_T);
    rDbg3AInfo1.hdr.u4StringDebugInfoOffset    = rDbg3AInfo1.hdr.u4AEDebugDataOffset + sizeof(AE_DEBUG_DATA_T);

    // module check sum & ver
#define SET_COMM_INFO1(_module, _chksum, _ver) \
    { \
        rDbg3AInfo1.hdr.rCommDebugInfo.r##_module.chkSum = _chksum; \
        rDbg3AInfo1.hdr.rCommDebugInfo.r##_module.ver = _ver; \
    }

    rDbg3AInfo1.hdr.rCommDebugInfo.u4Size = sizeof(rDbg3AInfo1.hdr.rCommDebugInfo);
    SET_COMM_INFO1(AE,       CHKSUM_DBG_AE_PARAM,       AE_DEBUG_TAG_VERSION_DP)
    SET_COMM_INFO1(AF,       CHKSUM_DBG_AF_PARAM,       AF_DEBUG_TAG_VERSION_DP)
    SET_COMM_INFO1(FLASH,    CHKSUM_DBG_FLASH_PARAM,    FLASH_DEBUG_TAG_VERSION_DP)
    SET_COMM_INFO1(FLICKER,  CHKSUM_DBG_FLICKER_PARAM,  FLICKER_DEBUG_TAG_VERSION_DP)
    SET_COMM_INFO1(SHADING,  CHKSUM_DBG_SHADING_PARAM,  SHAD_DEBUG_TAG_VERSION_DP)
    //

    // AE
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetDebugInfo, reinterpret_cast<MINTPTR>(&(rDbg3AInfo1.rAEDebugInfo)), reinterpret_cast<MINTPTR>(&rDbg3AInfo1.rAEDebugData), NULL, NULL);

#if CAM3_FLASH_FEATURE_EN
    // Flash
    FlashMgr::getInstance(m_i4SensorDev)->getDebugInfo(&rDbg3AInfo1.rFlashDebugInfo);
#endif

    // Flicker
    //Flicker::getInstance()->getDebugInfo(&rDbg3AInfo1.rFlickerDebugInfo);

#if CAM3_LSC_FEATURE_EN
    // LSC
    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
#endif

    // STRING INFO
    MUINT8 *str_ptr = rDbg3AInfo1.rStringDebugInfo.u1StrData;
    //Fix coverity:overrun-buffer
    int str_remands = (STRING_DEBUG_INFO_SIZE-1);
    int str_cnt = 0;
#if 1 //for test
    _append_string_tag(str_ptr, "Example NAME:0123_,*!@#$%^&*()~+-", "Example CONTENT:0123_,*!@#$%^&*()~+-", str_remands, str_cnt);
    _append_string_tag(str_ptr, "String-NAME-2", "String-CONTENT-2", str_remands, str_cnt);
#endif
    rDbg3AInfo1.rStringDebugInfo.u4StrCount = str_cnt;
    rDbg3AInfo1.rStringDebugInfo.u4Size = sizeof(STRING_DEBUG_INFO_T);


    // ISP Offset 3A HAL Fill
    // AWB AF ISP debug info
    rDbg3AInfo2.hdr.u4KeyID = ISP_DEBUG_KEYID;
    rDbg3AInfo2.hdr.u4ModuleCount = MODULE_NUM(6,2);
    rDbg3AInfo2.hdr.rCommDebugInfo.rISP.P1Offset = P1_START_ADDR_COUNT*4;
    rDbg3AInfo2.hdr.rCommDebugInfo.rISP.P2Offset = P2_START_ADDR_COUNT*4;

    rDbg3AInfo2.hdr.u4AWBDebugInfoOffset      = sizeof(rDbg3AInfo2.hdr);
    rDbg3AInfo2.hdr.u4ISPDebugInfoOffset      = rDbg3AInfo2.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T) + sizeof(NSIspExifDebug::IspExifDebugInfo_T::Header);
    rDbg3AInfo2.hdr.u4ISPP1RegDataOffset      = rDbg3AInfo2.hdr.u4ISPDebugInfoOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo);
    rDbg3AInfo2.hdr.u4ISPP2RegDataOffset      = rDbg3AInfo2.hdr.u4ISPP1RegDataOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::P1RegInfo);
    rDbg3AInfo2.hdr.u4MFBRegInfoOffset        = rDbg3AInfo2.hdr.u4ISPP2RegDataOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::P2RegInfo);
    rDbg3AInfo2.hdr.u4AWBDebugDataOffset      = rDbg3AInfo2.hdr.u4ISPDebugInfoOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T) - sizeof(NSIspExifDebug::IspExifDebugInfo_T::Header);

    // module check sum & ver
#define SET_COMM_INFO2(_module, _chksum, _ver) \
    { \
        rDbg3AInfo2.hdr.rCommDebugInfo.r##_module.chkSum = _chksum; \
        rDbg3AInfo2.hdr.rCommDebugInfo.r##_module.ver = _ver; \
    }

    rDbg3AInfo2.hdr.rCommDebugInfo.u4Size = sizeof(rDbg3AInfo2.hdr.rCommDebugInfo);
    SET_COMM_INFO2(AWB,      CHKSUM_DBG_AWB_PARAM,      AWB_DEBUG_TAG_VERSION_DP)
    SET_COMM_INFO2(ISP,      CHKSUM_DBG_ISP_PARAM,      ISP_DEBUG_TAG_VERSION_DP)

    // AWB
    IAwbMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo2.rAWBDebugInfo, rDbg3AInfo2.rAWBDebugData);

    return MTRUE;
}

MBOOL
Hal3ARawImp::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = 0;
    switch (e3ACtrl)
    {
        // --------------------------------- AE ---------------------------------
        case E3ACtrl_SetHalHdr:
            if (iArg1)
            {
                AE_MODE_CFG_T rCaptureInfo = {};
                CaptureParam_T &a_rCaptureInfo = *reinterpret_cast<CaptureParam_T*>(iArg2);
                rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
                rCaptureInfo.u4Eposuretime  = a_rCaptureInfo.u4Eposuretime;
                rCaptureInfo.u4AfeGain      = a_rCaptureInfo.u4AfeGain;
                rCaptureInfo.u4IspGain      = a_rCaptureInfo.u4IspGain;
                rCaptureInfo.u4RealISO      = a_rCaptureInfo.u4RealISO;
                rCaptureInfo.i2FlareGain    = a_rCaptureInfo.u4FlareGain;
                rCaptureInfo.i2FlareOffset  = a_rCaptureInfo.u4FlareOffset;
                MBOOL bEnableHDRShot = MTRUE;
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetCaptureParams, reinterpret_cast<MINTPTR>(&(rCaptureInfo)), NULL, NULL, NULL);
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableHDRShot, bEnableHDRShot, NULL, NULL, NULL);
                mbIsHDRShot = MTRUE;
            }
            else
            {
                MBOOL bEnableHDRShot = MFALSE;
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableHDRShot, bEnableHDRShot, NULL, NULL, NULL);
            }
            break;
        case E3ACtrl_GetCurrentEV:
            IAeMgr::getInstance().CCTOPAEGetCurrentEV(m_i4SensorDev, reinterpret_cast<MINT32 *>(iArg1), reinterpret_cast<MUINT32 *>(iArg2));
            break;
        case E3ACtrl_GetBVOffset:{
            AE_NVRAM_T aeNvram;
            ::memset(&aeNvram, 0, sizeof(aeNvram));
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetNVRAParam, reinterpret_cast<MINTPTR>(&aeNvram), iArg2, NULL, NULL);
            *reinterpret_cast<MINT32*>(iArg1) = aeNvram.rCCTConfig.i4BVOffset;
            }break;
        case E3ACtrl_GetNVRAMParam:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetNVRAParam, iArg1, iArg2, NULL, NULL);
            break;
        case E3ACtrl_SetAEIsoSpeedMode:
            IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAETargetMode:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetTargetMode, iArg1, NULL, NULL, NULL);
            IAwbMgr::getInstance().SetAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            IPDMgr::getInstance().setAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            m_i4AETargetMode = static_cast<eAETargetMODE>(iArg1);
            break;
        case E3ACtrl_SetAELimiterMode:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetLimiterMode, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetAECamMode:
            IAeMgr::getInstance().setAECamMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAEEISRecording:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableEISRecording, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetAEPlineLimitation:
            AE_Pline_Limitation_T rLimitParams;
            rLimitParams = *reinterpret_cast<AE_Pline_Limitation_T*>(iArg1);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetPlineTableLimitation, rLimitParams.bEnable, rLimitParams.bEquivalent, rLimitParams.u4IncreaseISO_x100, rLimitParams.u4IncreaseShutter_x100);
            break;
        case E3ACtrl_EnableDisableAE:
            if (iArg1) i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableAE, NULL, NULL, NULL, NULL);
            else       i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_DisableAE, NULL, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetAEVHDRratio:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetEMVHDRratio, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_EnableAIS:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableAISManualPline, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_EnableBMDN:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableBMDNManualPline, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_EnableMFHR:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableMFHRManualPline, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetMinMaxFps:
            if((MINT32)iArg1 != m_i4OverrideMinFrameRate || (MINT32)iArg2 != m_i4OverrideMaxFrameRate)
            {
                m_i4OverrideMinFrameRate = (MUINT32)iArg1;
                m_i4OverrideMaxFrameRate = (MUINT32)iArg2;
                CAM_LOGD("[%s] E3ACtrl_SetMinMaxFps (%d,%d)", __FUNCTION__, m_i4OverrideMinFrameRate, m_i4OverrideMaxFrameRate);
            }
            IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, m_i4OverrideMinFrameRate, m_i4OverrideMaxFrameRate);
            break;
        case E3ACtrl_SetCaptureMaxFPS:
            CAM_LOGD("[%s] E3ACtrl_SetCaptureMaxFPS (not support now)", __FUNCTION__);
            break;
        case E3ACtrl_EnableFlareInManualCtrl:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableFlareInManualControl, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_ResetMvhdrRatio:
            IAeMgr::getInstance().resetMvhdrRatio(m_i4SensorDev, (MBOOL)iArg1);
            break;
        case E3ACtrl_GetExposureInfo:
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetExposureInfo, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_GetAECapPLineTable:
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetCapPlineTable, iArg1, iArg2, NULL, NULL);
            break;
        case E3ACtrl_GetInitExposureTime:
            {
                AE_MODE_CFG_T rPreviewInfo;
                AE_PERFRAME_INFO_T AEPerframeInfo;
                i4Ret = IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
                *(reinterpret_cast<MUINT32*>(iArg1)) = (MUINT32)(AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns/1000);
            }
            break;
        case E3ACtrl_GetExposureParam:
            CaptureParam_T* a_rCaptureInfo;
            a_rCaptureInfo = reinterpret_cast<CaptureParam_T*>(iArg1);

            AE_MODE_CFG_T rCaptureInfo;
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetCapParams, reinterpret_cast<MINTPTR>(&rCaptureInfo), NULL, NULL, NULL);
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
            {
                AE_PERFRAME_INFO_T AEPerframeInfo;
                i4Ret = IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
                *(reinterpret_cast<MUINT32*>(iArg1)) = AEPerframeInfo.rAEISPInfo.bAEStable;
            }
            break;
        case E3ACtrl_GetRTParamsInfo:
//            i4Ret = IAeMgr::getInstance().getRTParams(m_i4SensorDev, *reinterpret_cast<FrameOutputParam_T*>(iArg1));
            FrameOutputParam_T *a_RTParams;
            a_RTParams = reinterpret_cast<FrameOutputParam_T*>(iArg1);

            AE_PERFRAME_INFO_T AEPerframeInfo;
            IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
            a_RTParams->u4AEIndex = AEPerframeInfo.rAEUpdateInfo.i4AEidxCur;
            a_RTParams->u4AEIndexF = AEPerframeInfo.rAEUpdateInfo.u4AEidxCurrentF;
            a_RTParams->u4FinerEVIdxBase = AEPerframeInfo.rAEISPInfo.u4AEFinerEVIdxBase;
            a_RTParams->u4FRameRate_x10 = AEPerframeInfo.rAEUpdateInfo.u2FrameRate_x10;
            a_RTParams->u4PreviewShutterSpeed_us = AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns / 1000;
            a_RTParams->u4PreviewSensorGain_x1024 = AEPerframeInfo.rAEISPInfo.u4P1SensorGain;
            a_RTParams->u4PreviewISPGain_x1024 = AEPerframeInfo.rAEISPInfo.u4P1DGNGain;
            a_RTParams->u4RealISOValue = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
            a_RTParams->u4CapShutterSpeed_us = AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns / 1000;
            a_RTParams->u4CapSensorGain_x1024 = AEPerframeInfo.rAEISPInfo.u4P1SensorGain;
            a_RTParams->u4CapISPGain_x1024 = AEPerframeInfo.rAEISPInfo.u4P1DGNGain;
            a_RTParams->i4BrightValue_x10 = AEPerframeInfo.rAEUpdateInfo.i4BVvalue_x10;
            a_RTParams->i4ExposureValue_x10 = AEPerframeInfo.rAEUpdateInfo.i4BVvalue_x10;     // EV value
            a_RTParams->i4LightValue_x10 = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
            a_RTParams->i2FlareOffset = AEPerframeInfo.rAEISPInfo.i2FlareOffset;
            a_RTParams->i2FlareGain = AEPerframeInfo.rAEISPInfo.i2FlareGain;
            a_RTParams->u4AvgY = AEPerframeInfo.rAEUpdateInfo.u4AvgY;
            a_RTParams->i4LuxValue_x10000 = AEPerframeInfo.rAEUpdateInfo.u4LuxValue_x10000;
            break;
        case E3ACtrl_GetEvCapture:
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetCapDiffEVState, iArg1, iArg2, NULL, NULL);
            break;
        case E3ACtrl_GetEvSetting:{
            strAEOutput aeOutput;
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetCapDiffEVState, iArg1, reinterpret_cast<MINTPTR>(&aeOutput), NULL, NULL);
            *reinterpret_cast<strEvSetting*>(iArg2) = aeOutput.EvSetting;
            }break;
        case E3ACtrl_GetCaptureDelayFrame:
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetCapDelayFrame, iArg1, iArg2, NULL, NULL);
            break;
        case E3ACtrl_GetSensorSyncInfo:
            MINT32 i4SutterDelay;
            MINT32 i4SensorGainDelay;
            MINT32 i4IspGainDelay;
            AAASensorMgr::getInstance().getSensorSyncinfo(m_i4SensorDev,&i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay, reinterpret_cast<MINT32*>(iArg1));
            break;
        case E3ACtrl_GetSensorPreviewDelay:
            *(reinterpret_cast<MINT32*>(iArg1)) = 2;
            //*(reinterpret_cast<MINT32*>(iArg1)) = m_i4SensorPreviewDelay;
            break;
        case E3ACtrl_GetSensorDelayInfo:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4AeShutDelayFrame;
            *(reinterpret_cast<MINT32*>(iArg2)) = m_i4AeISPGainDelayFrame;
            break;
        case E3ACtrl_GetIsoSpeed:
            {
                //AE_SENSOR_PARAM_T rAESensorInfo;
                MINT32 i4AEISOSpeedMode;
                AE_PERFRAME_INFO_T AEPerframeInfo;
                IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetISOSpeedMode, reinterpret_cast<MINTPTR>(&i4AEISOSpeedMode), NULL, NULL, NULL);
                *(reinterpret_cast<MINT32*>(iArg1)) = i4AEISOSpeedMode;
            }
            break;
        case E3ACtrl_GetAEInitExpoSetting:
            {
                //Set SubsampleCount to AE from config.
                ConfigInfo_T rConfigInfo;
                rConfigInfo = *reinterpret_cast<ConfigInfo_T*>(iArg2);
                MBOOL bAESMBuffermode = MFALSE;
                if(rConfigInfo.i4SubsampleCount > 1)
                    bAESMBuffermode = MTRUE;
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSMBuffermode, bAESMBuffermode, rConfigInfo.i4SubsampleCount, NULL, NULL);

                AEInitExpoSetting_T* a_rAEInitExpoSetting;
                i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAEInitExpSetting, iArg1, NULL, NULL, NULL);
                a_rAEInitExpoSetting = reinterpret_cast<AEInitExpoSetting_T*>(iArg1);

                CAM_LOGD("[%s()] E3ACtrl_GetInitExpoSetting: u4SensorMode(%d) u4AETargetMode(%d) u4Eposuretime(%d) u4AfeGain(%d) u4Eposuretime_se(%d) u4AfeGain_se(%d) u4Eposuretime_me(%d) u4AfeGain_me(%d) u4Eposuretime_vse(%d) u4AfeGain_vse(%d) SubsampleCount(%d)",
                          __FUNCTION__, a_rAEInitExpoSetting->u4SensorMode, a_rAEInitExpoSetting->u4AETargetMode,
                          a_rAEInitExpoSetting->u4Eposuretime, a_rAEInitExpoSetting->u4AfeGain,
                          a_rAEInitExpoSetting->u4Eposuretime_se, a_rAEInitExpoSetting->u4AfeGain_se,
                          a_rAEInitExpoSetting->u4Eposuretime_me, a_rAEInitExpoSetting->u4AfeGain_me,
                          a_rAEInitExpoSetting->u4Eposuretime_vse, a_rAEInitExpoSetting->u4AfeGain_vse,
                          rConfigInfo.i4SubsampleCount);
            }
            break;
        case E3ACtrl_GetPrioritySetting:
            AE_EXP_SETTING_T *rAEInputExpoSetting;// = *reinterpret_cast<AE_EXP_SETTING_T*>(iArg1);
            rAEInputExpoSetting = reinterpret_cast<AE_EXP_SETTING_T*>(iArg1);
            AE_EXP_SETTING_T* a_rAEOutputExpoSetting;
            a_rAEOutputExpoSetting = reinterpret_cast<AE_EXP_SETTING_T*>(iArg2);
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetExpSettingByShutterISOPriority, iArg1, iArg2, NULL, NULL);
            CAM_LOGD("[%s()] E3ACtrl_GetPrioritySetting: Input Shutter(%d) ISO(%d) Output Shutter(%d) ISO(%d)",
                      __FUNCTION__, rAEInputExpoSetting->u4ExposureTime, rAEInputExpoSetting->u4Sensitivity,
                      a_rAEOutputExpoSetting->u4ExposureTime, a_rAEOutputExpoSetting->u4Sensitivity);
            break;
        case E3ACtrl_GetISOThresStatus:
            IAeMgr::getInstance().getISOThresStatus(m_i4SensorDev, reinterpret_cast<MINT32 *>(iArg1), reinterpret_cast<MINT32 *>(iArg2));
            break;
        case E3ACtrl_ResetGetISOThresStatus:
            IAeMgr::getInstance().resetGetISOThresStatus(m_i4SensorDev, reinterpret_cast<MINT32 *>(iArg1), reinterpret_cast<MINT32 *>(iArg2));
            break;
#if 0 //CAM3_STEREO_FEATURE_EN
        case E3ACtrl_GetAEStereoDenoiseInfo:
            AE_MODE_CFG_T rDenoiseInfoInfo;
            NSIspTuning::ISP_INFO_T* pIspInfo;
            i4Ret = IAeMgr::getInstance().getStereoDenoiseInfo(m_i4SensorDev, rDenoiseInfoInfo);
            pIspInfo = reinterpret_cast<NSIspTuning::ISP_INFO_T*>(iArg1);
            if(pIspInfo->rCamInfo.rAEInfo.u4RealISOValue != rDenoiseInfoInfo.u4RealISO)
            {
                CAM_LOGD("[%s()] ISO (%d -> %d)", __FUNCTION__, pIspInfo->rCamInfo.rAEInfo.u4RealISOValue, rDenoiseInfoInfo.u4RealISO);
                pIspInfo->rCamInfo.rAEInfo.u4RealISOValue = rDenoiseInfoInfo.u4RealISO;
                MINT32 i4AEISOSpeedMode = 0;
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetISOSpeedMode, reinterpret_cast<MINTPTR>(&i4AEISOSpeedMode), NULL, NULL, NULL);
                *(reinterpret_cast<MUINT32*>(iArg2))  = (i4AEISOSpeedMode ==LIB3A_AE_ISO_SPEED_AUTO) ? rDenoiseInfoInfo.u4RealISO : i4AEISOSpeedMode;
            }
            if(pIspInfo->rCamInfo.rAEInfo.u4IspGain != rDenoiseInfoInfo.u4IspGain)
            {
                CAM_LOGD("[%s()] IspGain (%d -> %d)", __FUNCTION__, pIspInfo->rCamInfo.rAEInfo.u4IspGain, rDenoiseInfoInfo.u4IspGain);
                pIspInfo->rCamInfo.rAEInfo.u4IspGain = rDenoiseInfoInfo.u4IspGain;
            }
            break;
#endif
        case E3ACtrl_EnableAEStereoManualPline:
            CAM_LOGD("[%s] DualCamera control (%d)", __FUNCTION__, (MBOOL)iArg1);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableAEStereoManualPline, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetIspProfile:
            m_rParam.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(iArg1);
            break;
        // --------------------------------- AWB ---------------------------------
        case E3ACtrl_SetAwbBypCalibration:
            i4Ret = IAwbMgr::getInstance().CCTOPAWBBypassCalibration(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
            break;
#if CAM3_AF_FEATURE_EN
        // --------------------------------- AF ---------------------------------
        case E3ACtrl_SetAFMode:
            IAfMgr::getInstance(m_i4SensorDev).setAFMode( iArg1, AF_CMD_CALLER);
            break;
        case E3ACtrl_SetAFMFPos:
//            IAfMgr::getInstance(m_i4SensorDev).setMFPos( iArg1, AF_CMD_CALLER);
            break;
        case E3ACtrl_SetAFFullScanStep:
            IAfMgr::getInstance(m_i4SensorDev).setFullScanstep( iArg1);
            break;
        case E3ACtrl_SetAFCamMode:

            break;
        case E3ACtrl_GetAFDAFTable:
        {
            AFStaticInfo_T staticInfo;
            IAfMgr::getInstance(m_i4SensorDev).getDAFTbl(staticInfo);
            MVOID **ptbl = (MVOID**)iArg1;
            *ptbl = (MVOID*)(staticInfo.dafTbl);
        }
            break;
        case E3ACtrl_SetEnableOIS:
            break;
        case E3ACtrl_SetEnablePBin:
            IPDMgr::getInstance().setPBNen(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetPureRawData:
            // arg1 : meta data
            // arg2 : image buffer
            //reference : pdenode
            IPDMgr::getInstance().doSWPDE(m_i4SensorDev, (MVOID*)iArg1, (MVOID*)iArg2);
            break;
        case E3ACtrl_GetOISPos:
            mcuOISPosInfo* rGetOisPosInfo;
            rGetOisPosInfo = reinterpret_cast<mcuOISPosInfo*>(iArg1);
            MCUDrv::getInstance(m_i4SensorDev)->getOISPosInfo(rGetOisPosInfo);
            break;
    case E3ACtrl_Get_AF_FSC_INIT_INFO:
        if(iArg1)
        {
            FSC_DAC_INIT_INFO_T* ptrFscInitInfo = reinterpret_cast<FSC_DAC_INIT_INFO_T*>(iArg1);
            AFStaticInfo_T staticInfo;
            IAfMgr::getInstance(m_i4SensorDev).getStaticInfo(staticInfo, LOG_TAG);
            memcpy(ptrFscInitInfo, &(staticInfo.fscInitInfo), sizeof(FSC_DAC_INIT_INFO_T));
            CAM_LOGD_IF(m_3ALogEnable, "[%s] Get_AF_FSC_INIT_INFO %d, %d, %d, %d, %d, %d, %d, %d",
                        __FUNCTION__,
                        ptrFscInitInfo->macro_To_Inf_Ratio,
                        ptrFscInitInfo->dac_Inf,
                        ptrFscInitInfo->dac_Macro,
                        ptrFscInitInfo->damping_Time,
                        ptrFscInitInfo->readout_Time_us,
                        ptrFscInitInfo->init_DAC,
                        ptrFscInitInfo->af_Table_Start,
                        ptrFscInitInfo->af_Table_End);
        }
        break;
    case E3ACtrl_NOTIFY_AF_FSC_INFO:
        if (m_pCbSet)
        {
            m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FSC_INFO, iArg1, 0, 0);
        }
        else
            CAM_LOGE("[%s] m_pCbSet NULL", __FUNCTION__);
        break;
#endif

#if CAM3_LSC_FEATURE_EN
        // --------------------------------- Shading ---------------------------------
        case E3ACtrl_SetShadingSdblkCfg:
            if (NSIspTuning::EOperMode_Meta != m_i4OperMode)
                i4Ret = ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->CCTOPSetSdblkFileCfg((iArg1 ? MTRUE : MFALSE), reinterpret_cast<const char*>(iArg2));
            break;
        case E3ACtrl_SetShadingEngMode:
            m_fgEnableShadingMeta = MFALSE;
            if (NSIspTuning::EOperMode_Meta != m_i4OperMode)
            {
                switch (iArg1)
                {
                case 0: // Disable
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(2);
                    break;
                case 1: // Auto
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableDynamicShading(MTRUE, (MUINT32)m_i4OperMode);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    break;
                case 2: // Low
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(0);
                    break;
                case 3: // Mid
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(1);
                    break;
                case 4: // High
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(2);
                    break;
                case 5: // TSF
                default:
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->enableDynamicShading(MTRUE, (MUINT32)m_i4OperMode);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(2);
                    break;
                }
            }
            break;
        case E3ACtrl_SetShadingByp123:
            if (NSIspTuning::EOperMode_Meta != m_i4OperMode)
                ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->CCTOPSetBypass123(iArg1 ? MTRUE : MFALSE);
            break;
#endif

#if CAM3_FLASH_FEATURE_EN
        // --------------------------------- Flash ---------------------------------
        case E3ACtrl_GetQuickCalibration:
            i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctGetQuickCalibrationResult();
            *(reinterpret_cast<MINT32*>(iArg1)) = i4Ret;
            CAM_LOGD_IF(m_3ALogEnable, "getFlashQuickCalibrationResult ret=%d", i4Ret);
            break;
        case E3ACtrl_EnableFlashQuickCalibration:
            if(iArg1==1)
                i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctSetSpModeQuickCalibration2();
            else
                i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctSetSpModeNormal();
            break;
        case E3ACtrl_SetIsFlashOnCapture:
            FlashMgr::getInstance(m_i4SensorDev)->setIsFlashOnCapture(iArg1);
            CAM_LOGD_IF(m_3ALogEnable, "setIsFlashOnCapture=%ld", (long)iArg1);
            break;
        case E3ACtrl_GetIsFlashOnCapture:
        {
            if (m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
                m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH ||
                (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_SINGLE))) {
                *(reinterpret_cast<MINT32*>(iArg1)) = FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture();
            } else {
                *(reinterpret_cast<MINT32*>(iArg1)) = 0;
            }
        }
            break;
        case E3ACtrl_ChkMFNRFlash:
            // Any platform under P40 would take default branch which returns MFALSE
            return MTRUE;
#endif

        // --------------------------------- flow control ---------------------------------
        case E3ACtrl_Enable3ASetParams:
            m_bEnable3ASetParams = iArg1;
            CAM_LOGD("[%s] E3ACtrl_Enable3ASetParams %d\n",__FUNCTION__, m_bEnable3ASetParams);
            break;
        case E3ACtrl_SetOperMode:
            CAM_LOGD_IF(m_3ALogEnable, "[%s] prev_mode(%d), new_mode(%ld)", __FUNCTION__, m_i4OperMode, (long)iArg1);
            m_i4OperMode = iArg1;
            break;
       case E3ACtrl_GetOperMode:
            *(reinterpret_cast<MUINT32*>(iArg1)) = m_i4OperMode;
            break;
#if CAM3_STEREO_FEATURE_EN
        case E3ACtrl_SetStereoParams:
            setStereoParams(*reinterpret_cast<Stereo_Param_T*>(iArg1));
            break;
#endif
        case E3ACtrl_SetIsZsdCapture:
            if(iArg1)
                DebugUtil::getInstance(m_i4SensorDev)->pauseDetect();
            else
                DebugUtil::getInstance(m_i4SensorDev)->resumeDetect();
            break;
        case E3ACtrl_SetParamAEMode:
            CAM_LOGD("[%s] E3ACtrl_SetParamAEMode %d -> %d\n",__FUNCTION__, m_rParam.u4AeMode, *reinterpret_cast<MUINT32*>(iArg1));
            m_rParam.u4AeMode = *reinterpret_cast<MUINT32*>(iArg1);
            break;
        // --------------------------------- get info ---------------------------------
        case E3ACtrl_GetAsdInfo:
            i4Ret = getASDInfo(*reinterpret_cast<ASDInfo_T*>(iArg1));
            break;
        case E3ACtrl_GetExifInfo:
            i4Ret = get3AEXIFInfo(*reinterpret_cast<EXIF_3A_INFO_T*>(iArg1));
            break;
        case E3ACtrl_GetSupportedInfo:
            AE_DEVICES_INFO_T rDeviceInfo;
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetSensorDeviceInfo, reinterpret_cast<MINTPTR>(&rDeviceInfo), NULL, NULL, NULL);
            FeatureParam_T* rFeatureParam;
            rFeatureParam = reinterpret_cast<FeatureParam_T*>(iArg1);
            rFeatureParam->bAutoWhiteBalanceLockSupported = IAwbMgr::getInstance().isAWBLockSupported();
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetIsAELockSupported, reinterpret_cast<MINTPTR>(&(rFeatureParam->bExposureLockSupported)), NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAEMaxMeterAreaNum, reinterpret_cast<MINTPTR>(&(rFeatureParam->u4MaxMeterAreaNum)), NULL, NULL, NULL);
            rFeatureParam->u4FocusLength_100x = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
            rFeatureParam->bEnableDynamicFrameRate = CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE();

            m_rAFStaticInfo.isAfSupport = IAfMgr::getInstance(m_i4SensorDev).getAfSupport();
            rFeatureParam->u4MaxFocusAreaNum = m_rAFStaticInfo.isAfSupport;
            rFeatureParam->i4MaxLensPos      = m_rAFStaticInfo.maxAfTablePos;
            rFeatureParam->i4MinLensPos      = m_rAFStaticInfo.minAfTablePos;
            rFeatureParam->i4AFBestPos       = m_rAFStaticInfo.lastFocusPos;

            if(NSCcuIf::ICcuMgrExt::ccuIsSupportSecurity())
                rFeatureParam->bCcuIsSupportSecurity = 1;
            else
            {
                CAM_LOGW("[%s] Fail get ccu interface", __FUNCTION__);
                rFeatureParam->bCcuIsSupportSecurity = 0;
            }
            break;
#if CAM3_STEREO_FEATURE_EN
        case E3ACtrl_GetDualZoomInfo:
            {
                DualZoomInfo_T* rDualZoomInfo;
                AE_MODE_CFG_T rAeInfo;
                AE_PERFRAME_INFO_T AEPerframeInfo;
                //prepare output structure instance
                struct ccu_tg_switch_stat tgsw_stat;

                //call the command "MSG_TO_CCU_QUERY_TG_SWITCH_STAT" with ccuControl
                //tgsw_stat will be filled after command is done
                CAM_LOGD("[%s] m_pCcuCtrl3ACtrl(%p)", __FUNCTION__, m_pCcuCtrl3ACtrl);
                if(m_pCcuCtrl3ACtrl)
                    m_pCcuCtrl3ACtrl->ccuControl(MSG_TO_CCU_QUERY_TG_SWITCH_STAT, NULL, &tgsw_stat);

                i4Ret = IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);

                rDualZoomInfo = reinterpret_cast<DualZoomInfo_T*>(iArg1);
                rDualZoomInfo->i4AELv_x10 = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
                rDualZoomInfo->i4AEIso = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
                rDualZoomInfo->bIsAEBvTrigger= AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger;
                rDualZoomInfo->bIsAEAPLock= AEPerframeInfo.rAEUpdateInfo.bAPAELock;
#if 1   // TODO: replacing with getResult
        AFResult_T rAFResultFromMgr;
        IAfMgr::getInstance(m_i4SensorDev).getResult(rAFResultFromMgr);
        rDualZoomInfo->i4AFDAC = rAFResultFromMgr.lensPosition;// lensPosition
        rDualZoomInfo->bAFDone = rAFResultFromMgr.isFocusFinish;// isFocusFinish
#endif
                rDualZoomInfo->bSyncAFDone = ISync3AMgr::getInstance()->getSync3A()->isAFSyncFinish();
                rDualZoomInfo->bSync2ADone = ISync3AMgr::getInstance()->getSync3A()->is2ASyncFinish();
                CAM_LOGW("[%s] FrameSyncDone(%d -> %d)", __FUNCTION__, rDualZoomInfo->bIsFrameSyncDone, tgsw_stat.is_ready);
                rDualZoomInfo->bIsFrameSyncDone = tgsw_stat.is_ready;
#if CAM3_AF_FEATURE_EN
                mcuMotorOISInfo rOisInfo;
                rDualZoomInfo->bOISIsValid = MCUDrv::getInstance(m_i4SensorDev)->getMCUOISInfo(&rOisInfo);
                rDualZoomInfo->fOISHallPosXum  = (MFLOAT)rOisInfo.i4OISHallPosXum / (MFLOAT)rOisInfo.i4OISHallFactorX;
                rDualZoomInfo->fOISHallPosYum  = (MFLOAT)rOisInfo.i4OISHallPosYum / (MFLOAT)rOisInfo.i4OISHallFactorY;
#endif
            }
            break;
        case E3ACtrl_GetCCUFrameSyncInfo:
            if(m_pCcuCtrl3ACtrl)
               *(reinterpret_cast<MBOOL*>(iArg1)) = m_pCcuCtrl3ACtrl->isSupportFrameSync();
            break;
        case E3ACtrl_Sync3A_IsActive:
            *(reinterpret_cast<MBOOL*>(iArg1)) = ISync3AMgr::getInstance()->isActive();
            break;
        case E3ACtrl_Sync3A_Sync2ASetting:
            {
                MINT32 i4MasterIdx = (MINT32)iArg1;
                MINT32 i4SlaveIdx = (MINT32)iArg2;
                CAM_LOGD("[%s] Dev(%d) MasterIdx(%d) SlaveIdx(%d)", __FUNCTION__, m_i4SensorDev, i4MasterIdx, i4SlaveIdx);
                NS3Av3::ISync3AMgr::getInstance()->getSync3A()->sync2ASetting(i4MasterIdx, i4SlaveIdx);
            }
            break;
        case E3ACtrl_SetSync3ADevDoSwitch:
            {
                MINT32 i4Sync3ADevDoSwitch = static_cast<MINT32>(iArg1);
                MINT32 i4SwitchCamCount = ISync3AMgr::getInstance()->getSwitchCamCount();

                if (i4Sync3ADevDoSwitch == MTRUE && i4SwitchCamCount == 2)
                    ISync3AMgr::getInstance()->setSwitchCamCount(i4Sync3ADevDoSwitch);
                CAM_LOGD("[%s] Dev(%d) i4Sync3ADevDoSwitch(%d) i4SwitchCamCount(%d)", __FUNCTION__, m_i4SensorDev, i4Sync3ADevDoSwitch, i4SwitchCamCount);
            }
            break;
        case E3ACtrl_SetCCUTGDynamicSwitch:
            if(m_pCcuCtrl3ACtrl)
               m_pCcuCtrl3ACtrl->ccuControl(MSG_TO_CCU_TG_DYNAMIC_SWITCH, NULL, NULL);
            break;
#endif
        case E3ACtrl_GetCurrResult:
            {
                MUINT32 u4Magic = (MUINT32)iArg1;
                getCurrResult(u4Magic);
            }
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

VOID
Hal3ARawImp::
queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight)
{
    i4TgWidth = m_i4TgWidth;
    i4TgHeight = m_i4TgHeight;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3ARawImp::
attachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    m_pTaskMgr->attachCb(cb);
    return 0;
}

MINT32
Hal3ARawImp::
detachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pTaskMgr->detachCb(cb);
    m_pCbSet = NULL;
    return 0;

}

VOID
Hal3ARawImp::
queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight)
{

    i4HbinWidth = m_i4HbinWidth;
    i4HbinHeight = m_i4HbinHeight;

}

MVOID
Hal3ARawImp::
querySensorStaticInfo()
{
    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer

    m_i4SensorPreviewDelay = sensorStaticInfo.previewDelayFrame;
    m_i4AeShutDelayFrame = sensorStaticInfo.aeShutDelayFrame;
    m_i4AeISPGainDelayFrame = sensorStaticInfo.aeISPGainDelayFrame;

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), rawFmtType(%d), SensorPreviewDelay(%d), AeShutDelayFrame(%d), AeISPGainDelayFrame(%d)\n"
            , __FUNCTION__
            , m_i4SensorDev, m_i4SensorIdx, u4RawFmtType, m_i4SensorPreviewDelay, m_i4AeShutDelayFrame, m_i4AeISPGainDelayFrame);

    // 3A/ISP mgr can query sensor static information here
    IAwbMgr::getInstance().setIsMono(m_i4SensorDev, (u4RawFmtType == SENSOR_RAW_MONO ? MTRUE : MFALSE), u4RawFmtType);

/*#if CAM3_STEREO_FEATURE_EN
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
#endif*/
}

MBOOL
Hal3ARawImp::
notifyPwrOn()
{
    IAfMgr::getInstance(m_i4SensorDev).camPwrOn();
    IPDMgr::getInstance().CamPwrOnState(m_i4SensorDev);
    return MTRUE;
}


MBOOL
Hal3ARawImp::
notifyPwrOff()
{
    IAfMgr::getInstance(m_i4SensorDev).camPwrOff();
    IPDMgr::getInstance().CamPwrOffState(m_i4SensorDev);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyP1PwrOn()
{
    AAA_TRACE_D("notifyP1PwrOn");
    CAM_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
    m_pICcuMgr = NSCcuIf::ICcuMgrExt::createInstance("3AHal");
    if(m_pICcuMgr)
    {
        if(m_pICcuMgr->ccuInit() != 0)
        {
            m_pICcuMgr = NULL;
            CAM_LOGD("[%s] m_pICcuMgr->ccuInit() fail return false %p", __FUNCTION__, m_pICcuMgr);
            AAA_TRACE_END_D;
            return MFALSE;
        }
        if(m_pICcuMgr->ccuBoot() != 0)
        {
            m_pICcuMgr =NULL;
            CAM_LOGD("[%s] m_pICcuMgr->ccuBoot() fail return false %p", __FUNCTION__, m_pICcuMgr);
            AAA_TRACE_END_D;
            return MFALSE;
        }
        IAeMgr::getInstance().IsCCUAEInit(m_i4SensorDev,MTRUE);
        m_pICcuMgr->ccuSetSensorIdx((NSIspTuning::ESensorDev_T)m_i4SensorDev, m_i4SensorIdx);
        CAM_LOGD("[%s] m_pICcuMgr->ccuBoot() and m_pICcuMgr->ccuInit()success", __FUNCTION__);
    }
    if(m_pCcuCtrl3ACtrl == NULL)
         m_pCcuCtrl3ACtrl = ICcuCtrl3ACtrl::createInstance();
    if(m_pCcuCtrl3ACtrl)
    {
        if(m_pCcuCtrl3ACtrl->init(m_i4SensorIdx, (NSIspTuning::ESensorDev_T)m_i4SensorDev) != 0){
            CAM_LOGD("[%s] m_pCcuCtrl3ACtrl->init() fail return false", __FUNCTION__);
            AAA_TRACE_END_D;
            return MFALSE;
        }
    }
    AAA_TRACE_END_D;
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyP1PwrOff()
{
    AAA_TRACE_D("notifyP1PwrOff");
    if(m_pCcuCtrl3ACtrl)
    {
        m_pCcuCtrl3ACtrl->destroyInstance();
        m_pCcuCtrl3ACtrl = NULL;
    }
    if(m_pICcuMgr == NULL)
        m_pICcuMgr = NSCcuIf::ICcuMgrExt::createInstance("3AHal");

    if(m_pICcuMgr)
    {
        CAM_LOGD("[%s] notifyP1PwrOff", __FUNCTION__);
        m_pICcuMgr->ccuInvalidateSensorIdx((NSIspTuning::ESensorDev_T)m_i4SensorDev, m_i4SensorIdx);

        if(m_pICcuMgr->ccuShutdown() != 0)
        {
            CAM_LOGD("[%s] m_pICcuMgr->ccuShutdown() fail return false", __FUNCTION__);
            AAA_TRACE_END_D;
            return MFALSE;
        }
        if(m_pICcuMgr->ccuUninit() != 0)
        {
            CAM_LOGD("[%s] m_pICcuMgr->ccuUninit() fail return false", __FUNCTION__);
            AAA_TRACE_END_D;
            return MFALSE;
        }
        CAM_LOGD("[%s] m_pICcuMgr->ccuUninit() and m_pICcuMgr->ccuShutdown() success return true", __FUNCTION__);

        m_pICcuMgr->destroyInstance();
        m_pICcuMgr = NULL;
    }
    AAA_TRACE_END_D;
    return MTRUE;
}

MVOID
Hal3ARawImp::
setIspSensorInfo2AF(MINT32 MagicNum)
{
#if 0
    ISP_SENSOR_INFO_T rIspSensorInfo;
    const ISPResultToMeta_T *pISPResult = NULL;
    pISPResult = (const ISPResultToMeta_T*)m_pResultPoolObj->getResult(MagicNum, E_ISP_RESULTTOMETA);

    if(pISPResult == NULL)
        CAM_LOGW("[%s] pISPResult NULL", __FUNCTION__);
    else
    {
        rIspSensorInfo.i4FrameId            = MagicNum;
        rIspSensorInfo.bHLREnable           = MFALSE;
        rIspSensorInfo.bAEStable            = pISPResult->rCamInfo.rAEInfo.bAEStable;
        rIspSensorInfo.bAELock              = pISPResult->rCamInfo.rAEInfo.bAELock;
        rIspSensorInfo.bAEScenarioChange    = pISPResult->rCamInfo.rAEInfo.bAEScenarioChange;
        rIspSensorInfo.i4deltaIndex         = pISPResult->rCamInfo.rAEInfo.i4deltaIndex;
        rIspSensorInfo.u4AfeGain            = pISPResult->rCamInfo.rAEInfo.u4AfeGain;
        rIspSensorInfo.u4IspGain            = pISPResult->rCamInfo.rAEInfo.u4IspGain;
        rIspSensorInfo.u4Eposuretime        = pISPResult->rCamInfo.rAEInfo.u4Eposuretime;
        rIspSensorInfo.u4RealISOValue       = pISPResult->rCamInfo.rAEInfo.u4RealISOValue;
        rIspSensorInfo.u4AEFinerEVIdxBase   = pISPResult->rCamInfo.rAEInfo.u4AEFinerEVIdxBase;
        rIspSensorInfo.u4AEidxCurrentF      = pISPResult->rCamInfo.rAEInfo.u4AEidxCurrentF;
        rIspSensorInfo.i4LightValue_x10     = pISPResult->rCamInfo.rAEInfo.i4LightValue_x10;
        rIspSensorInfo.u4MgrCWValue         = pISPResult->rCamInfo.rAEInfo.u4MgrCWValue;

        IAfMgr::getInstance(m_i4SensorDev).setIspSensorInfo2AF( rIspSensorInfo);
    }
    CAM_LOGD_IF(m_3ALogEnable,"[%s] FrameId:%d AFEGain:%d ISPGain:%d", __FUNCTION__,rIspSensorInfo.i4FrameId, rIspSensorInfo.u4AfeGain, rIspSensorInfo.u4IspGain);
#endif
}

MVOID
Hal3ARawImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_bPreStop(%d)", __FUNCTION__, m_bPreStop);
    m_bPreStop = MTRUE;
    m_pThreadRaw->notifyPreStop();
}

MBOOL
Hal3ARawImp::
preset(Param_T const &rNewParam)
{
    if (IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetNeedPresetControlCCU, NULL, NULL, NULL, NULL))
    {
        // get current task to remapping ae state
        TaskInfo rInfo = m_pTaskMgr->getCurrTaskInfo();
        MINT32 i4AeState = taskEnumToAeState(rInfo.i4Enum);

        if(rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
        {
            CAM_LOGD("[%s] over-write i4AeState(%d) when CaptureIntent is capture", __FUNCTION__, i4AeState);
            i4AeState = AE_MGR_STATE_DOPOSTCAPAE;
        }

        CAM_LOGD("[%s] input : i4MagicNum for CCU(%d), i4Enum(%d)->i4AeState(%d), bEnable3ASetParams(%d), m_i4SubsampleCount(%d), u4AeMode(%d) CaptureIntent(%d)", __FUNCTION__, rNewParam.i4PresetKey, rInfo.i4Enum, i4AeState, m_bEnable3ASetParams, m_i4SubsampleCount, rNewParam.u4AeMode, rNewParam.u1CaptureIntent);
        if(i4AeState > 0)
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetAEMgrPreState, (MUINT32)i4AeState, NULL, NULL, NULL);

        IAeMgr::getInstance().setRequestNum(m_i4SensorDev, rNewParam.i4PresetKey);
        if (m_bEnable3ASetParams)
        {
            IAeMgr::getInstance().setDummyFrameInfo(m_i4SensorDev, (rNewParam.u1Dummy|rNewParam.bDummyAfterCapture), MTRUE);
            IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps);
            IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev, rNewParam.u4AeMeterMode);
            IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.i4IsoSpeedMode);
            IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas);
            CAM_LOGD_IF(m_rParam.bIsAELock != rNewParam.bIsAELock, "[%s] setAPAELock(%d/%d) ", __FUNCTION__, rNewParam.bIsAELock, m_rParam.bIsAELock);
            if(m_rParam.bIsAELock != rNewParam.bIsAELock)
                IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock);
            IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock);
            IAeMgr::getInstance().setAEEVCompIndex(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep, MTRUE);
            //IAeMgr::getInstance().setAEMode(m_i4SensorDev, rNewParam.u4AeMode);
            if(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON && rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSkipCalAE, (MBOOL)MTRUE, NULL, NULL, NULL);
            else if(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF && rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSkipCalAE, (MBOOL)MFALSE, NULL, NULL, NULL);

            if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF && m_i4SubsampleCount > 1)
            {
                if(((rNewParam.i4Sensitivity != m_rParam.i4Sensitivity)
                    || (rNewParam.i8ExposureTime != m_rParam.i8ExposureTime)
                    || (rNewParam.i8FrameDuration != m_rParam.i8FrameDuration)) || m_bIsFirstSetParams)
                {
                    AE_SENSOR_PARAM_T strSensorParams;
                    strSensorParams.u4Sensitivity   = rNewParam.i4Sensitivity;
                    strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTime;
                    strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
                    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSensorParams, reinterpret_cast<MINTPTR>(&(strSensorParams)), NULL, NULL, NULL);
                }
            }
            IAeMgr::getInstance().setAEFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);
            if( rNewParam.rScaleCropRect.i4Xwidth != 0 && rNewParam.rScaleCropRect.i4Yheight != 0 )
                IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewParam.rScaleCropRect.i4Xoffset, rNewParam.rScaleCropRect.i4Yoffset, rNewParam.rScaleCropRect.i4Xwidth, rNewParam.rScaleCropRect.i4Yheight);
            IAeMgr::getInstance().setAEHDRMode(m_i4SensorDev, rNewParam.u1HdrMode);
            IAeMgr::getInstance().setDigZoomRatio(m_i4SensorDev, rNewParam.i4ZoomRatio);
        }
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetPresetControlCCU, NULL, NULL, NULL, NULL);
    }

    return MTRUE;
}

#if 0
MBOOL
Hal3ARawImp::
prepareCCUCB(MUINT32 u4PreFrmId, ISP_NVRAM_OBC_T const &OBCResult)
{
    if(m_bCCUEn)
    {
        //1. get AE info and set AE info to ISP(AE_mgr)
        AE_PERFRAME_INFO_T AEPerframeInfo;
        IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);

        //2. update TSF ratio
        LSC_INPUT_INFO_T rLscInputInfo;
        const FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(u4PreFrmId,E_FLASH_RESULTTOMETA, __FUNCTION__);
        NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));

        rLscInputInfo.u4ShadingCCT = pLsc->getCTIdx();
        rLscInputInfo.i4AWBCCT = IAwbMgr::getInstance().getAWBCCT(m_i4SensorDev);
        if(pFLASHResult)
            rLscInputInfo.isFlash  = ((pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED) || (pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL));
        else
            rLscInputInfo.isFlash = 0;
        rLscInputInfo.u4RealISOValue = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
        ::memcpy(rLscInputInfo.pu4AEBlock, AEPerframeInfo.rAEUpdateInfo.pu4AEBlock, sizeof(MUINT32)*AE_BLOCK_NO*AE_BLOCK_NO);
        rLscInputInfo.bEnableRAFastConverge = AEPerframeInfo.rAEUpdateInfo.bEnableRAFastConverge;
        rLscInputInfo.u4MgrCWValue = AEPerframeInfo.rAEUpdateInfo.u4CWValue;
        rLscInputInfo.TgCtrlRight = AEPerframeInfo.rAEUpdateInfo.TgCtrlRight;
        rLscInputInfo.i4deltaIndex = AEPerframeInfo.rAEISPInfo.i4deltaIndex;
        rLscInputInfo.u4AEFinerEVIdxBase = AEPerframeInfo.rAEISPInfo.u4AEFinerEVIdxBase;
        rLscInputInfo.bAEStable = AEPerframeInfo.rAEISPInfo.bAEStable;
        rLscInputInfo.u4AvgWValue = AEPerframeInfo.rAEUpdateInfo.u4AvgY;
        rLscInputInfo.bAELock = AEPerframeInfo.rAEUpdateInfo.bAELock;
        rLscInputInfo.bAELimiter = AEPerframeInfo.rAEUpdateInfo.bAELimiter;

        pLsc->evaluate_cus_Ratio(rLscInputInfo, 0);

        ThreadSetCCUPara::AE_SENSOR_INFO_T rAESensorInfo;
        rAESensorInfo.u8FrameDuration = AEPerframeInfo.rAEUpdateInfo.u8FrameDuration_ns;
        rAESensorInfo.u8ExposureTime = AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns;
        rAESensorInfo.u4Sensitivity = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;

        m_CCUCmd = ThreadSetCCUPara::Cmd_T(u4PreFrmId, m_rParam.u1IsGetExif, m_bIsCapEnd, rAESensorInfo, OBCResult, AEPerframeInfo);

        CAM_LOGD_IF(m_3ALogEnable, "[%s] , i8SensorExposureTime(%lld), i8SensorFrameDuration(%lld), i4SensorSensitivity(%d)", __FUNCTION__,
        rAESensorInfo.u8ExposureTime, rAESensorInfo.u8FrameDuration, rAESensorInfo.u4Sensitivity/*, OBCResult.gain0.val, OBCResult.offst0.val*/);

        m_pThreadRaw->postToEventThread(E_3AEvent_CCU_CB);
    }

    return MTRUE;
}
#endif

MBOOL
Hal3ARawImp::
notify4CCU()
{
    //===================== AE part =====================//
    /************************************
     * 1. get result of CCU
     ************************************/
    AE_INFO_T rAEInfo;
    MUINT64 u8RealEposuretime;
    MUINT32 u4PresetKey = m_pCcuCtrl3ACtrl->getAeInfo(m_i4SensorIdx, static_cast<ESensorDev_T>(m_i4SensorDev), &rAEInfo, &u8RealEposuretime);

    /************************************
     * 2. get object of result pool
     ************************************/
    AllResult_T *pAllResult = NULL;
    MINT32 i4ReqMagic = -1;
    if(u4PresetKey > 0 && m_u4PrePresetKey != u4PresetKey && m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF)
    {
        pAllResult = m_pResultPoolObj->getAllResultByPresetKey(u4PresetKey);
        if(pAllResult != NULL)
        {
            i4ReqMagic = pAllResult->rResultCfg.i4ReqMagic;
            CAM_LOGD("[%s] output : i4MagicNum for CCU(%d/%d), exp(%llu)/isp(%d)/afe(%d)/iso(%d)", __FUNCTION__, u4PresetKey, i4ReqMagic,
                    rAEInfo.u4Eposuretime,
                    rAEInfo.u4IspGain,
                    rAEInfo.u4AfeGain,
                    rAEInfo.u4RealISOValue); // Debug log

            /************************************
             * 3. update object to result pool
             ************************************/
            // Update AE result
            AEResultToMeta_T rAEResult;
            rAEResult = *((AEResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read()));
            rAEResult.i4SensorSensitivity = rAEInfo.u4RealISOValue;//ISO value updated
            rAEResult.i8SensorExposureTime = rAEInfo.u4Eposuretime;
            rAEResult.u8P1Exposuretime_ns = u8RealEposuretime;
            //rAEResult.i8SensorFrameDuration = rAEInfo.u8FrameDuration;
            m_pResultPoolObj->updateResult(LOG_TAG, i4ReqMagic, E_AE_RESULTTOMETA, &rAEResult);

            // Update CCU result
            CCUResultInfo_T CCUResult;
            CCUResult.u4P1DGNGain = rAEInfo.u4IspGain;
            m_pResultPoolObj->updateResult(LOG_TAG, i4ReqMagic, E_CCU_RESULTINFO4OVERWRITE, &CCUResult);

            // Update EXIF
            {
                // protect vector before use vector
                std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
                if( pAllResult->vecExifInfo.size() > 0)
                {
                    // standard exif
                    //get3AEXIFInfo(*reinterpret_cast<EXIF_3A_INFO_T*>(&(pAllResult->vecExifInfo.back())));
                    EXIF_3A_INFO_T& rExifInfo = *reinterpret_cast<EXIF_3A_INFO_T*>(&(pAllResult->vecExifInfo.back()));

                    CAM_LOGD_IF(m_3ALogEnable, "[%s] (EXP, ISO)(%d, %d) -> (%d, %d)", __FUNCTION__, rExifInfo.u4CapExposureTime, rExifInfo.u4AEISOSpeed, (rAEInfo.u4Eposuretime / 1000), rAEInfo.u4RealISOValue);

                    rExifInfo.u4CapExposureTime = rAEInfo.u4Eposuretime / 1000;  // naro sec change to micro sec
                    rExifInfo.u4AEISOSpeed = rAEInfo.u4RealISOValue;//ISO value updated
                }
            }
            {
                // protect vector before use vector
                std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
                if( pAllResult->vecDbg3AInfo.size() > 0 && pAllResult->vecDbgIspInfo.size() > 0)
                {
                    // debug exif
                    AAA_DEBUG_INFO1_T& rDbg3AInfo1 = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResult->vecDbg3AInfo.editArray());
                    AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                    NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;
                    // AE
                    CAM_LOGD_IF(m_3ALogEnable, "[%s] (EXP,Afe,Ispgain,ISO)(%d, %d, %d, %d) -> (%d, %d, %d, %d)", __FUNCTION__,
                                rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue,
                                rAEInfo.u4Eposuretime, rAEInfo.u4AfeGain, rAEInfo.u4IspGain, rAEInfo.u4RealISOValue);
                    MUINT32 ReqMagic = m_pCcuCtrl3ACtrl->getAeDebugInfo(m_i4SensorIdx, static_cast<ESensorDev_T>(m_i4SensorDev), &rDbg3AInfo1.rAEDebugInfo, &rDbg3AInfo1.rAEDebugData);
                    if(ReqMagic != u4PresetKey)
                        CAM_LOGW("[%s] (Req,EXP,Afe,Ispgain,ISO)(#%d, %d, %d, %d, %d) (Key,EXP,Afe,Ispgain,ISO)(#%d, %d, %d, %d, %d)", __FUNCTION__, ReqMagic,
                                rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue,
                                u4PresetKey, rAEInfo.u4Eposuretime, rAEInfo.u4AfeGain, rAEInfo.u4IspGain, rAEInfo.u4RealISOValue);
                }
            }
        }
    }
    else
    {
        // ResultPool - Get History
        MINT32 rHistoryReqMagic[HistorySize] = {-1,-1,-1};
        m_pResultPoolObj->getHistory(rHistoryReqMagic);
        if(rHistoryReqMagic[1] > 0)
            i4ReqMagic = rHistoryReqMagic[1];

        pAllResult = m_pResultPoolObj->getAllResult(i4ReqMagic);
        CAM_LOGW("[%s] CCU not return PresetKey(%d) to convert metadata, so 3A get Req(#%d) to convert metadata", __FUNCTION__, u4PresetKey, i4ReqMagic);
    }

    AAA_TRACE_D("convertToMetadataFlow");
    // ResultPool - Get the specified result
    AllMetaResult_T* pMetaResult = m_pResultPoolObj->getMetadata(i4ReqMagic);
    MINT32 i4ConvertType = E_2A_CONVERT;
    if(!m_rAFStaticInfo.isAfThreadEnable)
        i4ConvertType != E_AF_CONVERT;
    if(pMetaResult && pAllResult)
        m_pResultPoolObj->convertToMetadataFlow(i4ReqMagic, i4ConvertType, pAllResult, pMetaResult);
    else
        CAM_LOGW("[%s] Req(#%d), pMetaResult(%p) or pAllResult(%p) is null", __FUNCTION__, i4ReqMagic, pMetaResult, pAllResult);
    AAA_TRACE_END_D;

    //===================== LTM part =====================//
    /************************************
     * 1. get result of CCU
     ************************************/
    ccu_ltm_info_isp rLTMInfo;
    MUINT32 u4LTMReqNumber = m_pCcuCtrl3ACtrl->getLtmInfo(m_i4SensorIdx, static_cast<ESensorDev_T>(m_i4SensorDev), &rLTMInfo);

    /************************************
     * 2. get object of result pool
     ************************************/
    if(u4LTMReqNumber > 0)
    {
        pAllResult = m_pResultPoolObj->getAllResult(u4LTMReqNumber);
        if(pAllResult != NULL)
        {
            /************************************
             * 3. update object to result pool
             ************************************/
            // Update LTM result
            ISPResultToMeta_T rISPResult;
            rISPResult = *((ISPResultToMeta_T*)(pAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read()));

            rISPResult.rCamInfo.rCCU_Result.LTM.LTM_EN = rLTMInfo.ltm_en;

            rISPResult.rCamInfo.rCCU_Result.LTM.CLIP_TH = rLTMInfo.ltm_ct;

            for (int numy = 0; numy < CCU_LTM_BLK_Y_NUM; numy++)
                for (int numx = 0; numx < CCU_LTM_BLK_X_NUM; numx++){

                    int Bin_Pos= (numy * 12 + numx);

                    for (int k = 0; k < (LTM_BIN - 1); k++){
                        if((k%2) == 0){
                            rISPResult.rCamInfo.rCCU_Result.LTM.LTM_Curve.lut[Bin_Pos * 16 + (k/2)].bits.LTMTC_TONECURVE_LUT_L = rLTMInfo.ltm_curve[numy][numx][k];
                        }
                        else{
                            rISPResult.rCamInfo.rCCU_Result.LTM.LTM_Curve.lut[Bin_Pos * 16 + (k/2)].bits.LTMTC_TONECURVE_LUT_H = rLTMInfo.ltm_curve[numy][numx][k];
                        }
                    }
                    //special case (colltct k=32(in each bin) on the final part of 1-dim array)
                    if((Bin_Pos % 2) == 0)
                        rISPResult.rCamInfo.rCCU_Result.LTM.LTM_Curve.lut[(CCU_LTM_BLK_Y_NUM * CCU_LTM_BLK_X_NUM ) * 16 + (Bin_Pos / 2)].bits.LTMTC_TONECURVE_LUT_L = rLTMInfo.ltm_curve[numy][numx][(LTM_BIN - 1)];
                    else
                        rISPResult.rCamInfo.rCCU_Result.LTM.LTM_Curve.lut[(CCU_LTM_BLK_Y_NUM * CCU_LTM_BLK_X_NUM ) * 16 + (Bin_Pos / 2)].bits.LTMTC_TONECURVE_LUT_H = rLTMInfo.ltm_curve[numy][numx][(LTM_BIN - 1)];
                }

            rISPResult.rCamInfo.rCCU_Result.LTM.LTM_Valid = CCU_VALID_PRESENT;

            //LTM twin A
            ::memcpy(&(rISPResult.rCamInfo.rCCU_Result.LTM.P1_LTM_Reg[0]), &(rLTMInfo.ltm_hw_reg[0][0]), sizeof(MUINT32)*LTM_HW_REG_MAX);
            //LTM twin B
            ::memcpy(&(rISPResult.rCamInfo.rCCU_Result.LTM.P1_LTM_Reg[1]), &(rLTMInfo.ltm_hw_reg[1][0]), sizeof(MUINT32)*LTM_HW_REG_MAX);
            //LTMS
            ::memcpy(&rISPResult.rCamInfo.rCCU_Result.LTM.P1_LTMS_Reg, &(rLTMInfo.ltms_hw_reg[0]), sizeof(MUINT32)*LTMS_HW_REG_MAX);
            //LTMSO
            ::memcpy(&(rISPResult.rCamInfo.rCCU_Result.LTM.LTMSO[0]), rLTMInfo.ltmso_buffer_addr_va, rLTMInfo.ltmso_size);  //ltmso_size should be 2160 * 4

#if 0
          ALOGE("Choooooooo, Result FrmId: %d, pointer: %p, Normal Addr: %p ",
                      u4LTMReqNumber,
                      rLTMInfo.ltmso_buffer_addr_va,
                      &(rISPResult.rCamInfo.rCCU_Result.LTM.LTMSO[0]));

          for(int i=0; i<2400; i++ )
                ALOGE("Choooooo, i= %d, (%d)",  i, rISPResult.rCamInfo.rCCU_Result.LTM.LTMSO[i]);
#endif


            // Coding
            m_pResultPoolObj->updateResult(LOG_TAG, u4LTMReqNumber, E_ISP_RESULTTOMETA, &rISPResult);

            // Update EXIF
            {
                // protect vector before use vector
                std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
                if( pAllResult->vecExifInfo.size() > 0 && pAllResult->vecDbg3AInfo.size() > 0 && pAllResult->vecDbgIspInfo.size() > 0)
                {
                    // debug exif
                    AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                    NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo* pDbgIspInfo = &(rDbg3AInfo2.rISPDebugInfo.debugInfo);

                    MUINT32 u4LTMExifNumber = m_pCcuCtrl3ACtrl->getLtmDebugInfo(m_i4SensorIdx, static_cast<ESensorDev_T>(m_i4SensorDev), pDbgIspInfo);

                    if(u4LTMReqNumber != u4LTMExifNumber)
                        CAM_LOGE("[%s] u4LTMReqNumber(%d) != u4LTMExifNumber(%d)",
                        __FUNCTION__,
                        u4LTMReqNumber,
                        u4LTMExifNumber);

                }
            }
        }
        else{
            CAM_LOGD("[%s] Can't find the LTM result, Req = (%d)", __FUNCTION__, u4LTMReqNumber);
        }
    }

    //===================== HLR part =====================//
    /************************************
     * 1. get result of CCU
     ************************************/
    ccu_hlr_info_isp rHLRInfo;
    MUINT32 u4HLRReqNumber = m_pCcuCtrl3ACtrl->getHlrInfo(m_i4SensorIdx, static_cast<ESensorDev_T>(m_i4SensorDev), &rHLRInfo);

    /************************************
     * 2. get object of result pool
     ************************************/
    if(u4HLRReqNumber > 0)
    {
        pAllResult = m_pResultPoolObj->getAllResult(u4HLRReqNumber);
        if(pAllResult != NULL)
        {
            /************************************
             * 3. update object to result pool
             ************************************/
            // Update HLR result
            ISPResultToMeta_T rISPResult;
            rISPResult = *((ISPResultToMeta_T*)(pAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read()));

            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_EN = rHLRInfo.hlr_en;

            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y0.bits.HLR_SAT_0        = rHLRInfo.hlr_out.hlr_sat_0;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y0.bits.HLR_SAT_1        = rHLRInfo.hlr_out.hlr_sat_1;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y1.bits.HLR_SAT_2        = rHLRInfo.hlr_out.hlr_sat_2;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y1.bits.HLR_SAT_3        = rHLRInfo.hlr_out.hlr_sat_3;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y2.bits.HLR_SAT_4        = rHLRInfo.hlr_out.hlr_sat_4;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y2.bits.HLR_SAT_5        = rHLRInfo.hlr_out.hlr_sat_5;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y3.bits.HLR_SAT_6        = rHLRInfo.hlr_out.hlr_sat_6;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_y3.bits.HLR_SAT_7        = rHLRInfo.hlr_out.hlr_sat_7;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x0.bits.HLR_NODE_0       = rHLRInfo.hlr_out.hlr_node_0;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x0.bits.HLR_NODE_1       = rHLRInfo.hlr_out.hlr_node_1;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x1.bits.HLR_NODE_2       = rHLRInfo.hlr_out.hlr_node_2;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x1.bits.HLR_NODE_3       = rHLRInfo.hlr_out.hlr_node_3;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x2.bits.HLR_NODE_4       = rHLRInfo.hlr_out.hlr_node_4;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x2.bits.HLR_NODE_5       = rHLRInfo.hlr_out.hlr_node_5;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x3.bits.HLR_NODE_6       = rHLRInfo.hlr_out.hlr_node_6;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_x3.bits.HLR_NODE_7       = rHLRInfo.hlr_out.hlr_node_7;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s0.bits.HLR_SLP_0        = rHLRInfo.hlr_out.hlr_slp_0;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s0.bits.HLR_SLP_1        = rHLRInfo.hlr_out.hlr_slp_1;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s0.bits.HLR_SLP_2        = rHLRInfo.hlr_out.hlr_slp_2;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s1.bits.HLR_SLP_3        = rHLRInfo.hlr_out.hlr_slp_3;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s1.bits.HLR_SLP_4        = rHLRInfo.hlr_out.hlr_slp_4;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s1.bits.HLR_SLP_5        = rHLRInfo.hlr_out.hlr_slp_5;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s2.bits.HLR_SLP_6        = rHLRInfo.hlr_out.hlr_slp_6;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s2.bits.HLR_RED_PRT_STR  = rHLRInfo.hlr_out.hlr_red_prt_str;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s2.bits.HLR_BLUE_PRT_STR = rHLRInfo.hlr_out.hlr_blue_prt_str;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.est_s2.bits.HLR_MAX_RAT      = rHLRInfo.hlr_out.hlr_max_rat;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.lmg.bits.HLR_BLD_FG          = rHLRInfo.hlr_out.hlr_bld_fg;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.lmg.bits.HLR_BLD_HIGH        = rHLRInfo.hlr_out.hlr_bld_high;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.lmg.bits.HLR_BLD_SLP         = rHLRInfo.hlr_out.hlr_bld_slp;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.prt.bits.HLR_PRT_TH          = rHLRInfo.hlr_out.hlr_prt_th;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.prt.bits.HLR_PRT_SLP         = rHLRInfo.hlr_out.hlr_prt_slp;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.prt.bits.HLR_PRT_EN          = rHLRInfo.hlr_out.hlr_prt_en;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.clp.bits.HLR_CLP_VAL         = rHLRInfo.hlr_out.hlr_clip_val;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.efct.bits.HLR_OFF_CLP_VAL    = rHLRInfo.hlr_out.hlr_off_clip_val;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.efct.bits.HLR_EFCT_ON        = rHLRInfo.hlr_out.hlr_efct_on;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.ctl.bits.HLR_TILE_EDGE       = rHLRInfo.hlr_out.hlr_tile_edge;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.ctl2.bits.HLR_TDR_WD         = rHLRInfo.hlr_out.hlr_tdr_wd;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.ctl2.bits.HLR_TDR_HT         = rHLRInfo.hlr_out.hlr_tdr_ht;
            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Data.lkmsb.bits.CAMCTL_HLR_R1_LKMSB=rHLRInfo.hlr_out.hlr_r1_lkmsb;

            rISPResult.rCamInfo.rCCU_Result.HLR.HLR_Valid = CCU_VALID_PRESENT;

            // Coding
            m_pResultPoolObj->updateResult(LOG_TAG, u4HLRReqNumber, E_ISP_RESULTTOMETA, &rISPResult);

            // Update EXIF
            {
                // protect vector before use vector
                std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
                if( pAllResult->vecExifInfo.size() > 0 && pAllResult->vecDbg3AInfo.size() > 0 && pAllResult->vecDbgIspInfo.size() > 0)
                {
                    // debug exif
                    AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                    NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo* pDbgIspInfo = &(rDbg3AInfo2.rISPDebugInfo.debugInfo);

                    MUINT32 u4HLRExifNumber = m_pCcuCtrl3ACtrl->getHlrDebugInfo(m_i4SensorIdx, static_cast<ESensorDev_T>(m_i4SensorDev), pDbgIspInfo);

                    if(u4HLRReqNumber != u4HLRExifNumber)
                        CAM_LOGE("[%s] u4HLRReqNumber(%d) != u4HLRExifNumber(%d)",
                        __FUNCTION__,
                        u4HLRReqNumber,
                        u4HLRExifNumber);
                }
            }
        }
        else{
            CAM_LOGD("[%s] Can't find the HLR result, Req = (%d)", __FUNCTION__, u4HLRReqNumber);
        }
    }

    CAM_LOGD_IF( (u4HLRReqNumber <=0 || u4LTMReqNumber <=0),"[%s] HLRReqNumber is InvalidValue(%d), LTMReqNumber is InvalidValue(%d)", __FUNCTION__, u4HLRReqNumber, u4LTMReqNumber);

    m_u4PrePresetKey = u4PresetKey;
    return MTRUE;
}

MINT32
Hal3ARawImp::
taskEnumToAeState(MINT32 i4TaskEnum)
{
    MINT32 i4AeState = -1;
    switch(i4TaskEnum)
    {
        case TASK_ENUM_3A_PREVIEW:
        case TASK_ENUM_3A_FLASH_FRONT:
            i4AeState = AE_MGR_STATE_DOPVAE;
            break;
        case TASK_ENUM_3A_TOUCH:
            i4AeState = AE_MGR_STATE_DOAFAE;
            break;
        case TASK_ENUM_3A_PRECAPTURE:
            i4AeState = AE_MGR_STATE_DOPRECAPAE;
            break;
        case TASK_ENUM_3A_CAPTURE:
            i4AeState = AE_MGR_STATE_DOPOSTCAPAE;
            break;
        default:
            break;
    }
    return i4AeState;
}


MBOOL
Hal3ARawImp::
setCCUParams()
{
#if 0 // TODO by Leo
    CAM_LOGD_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    MINT32 i4MagicNumReq = m_CCUCmd.i4MagicNumReq;
    // 1. get ratio
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
    MUINT32 u4Rto = (MINT32)pLsc->getRatio();
    // 1.1 updateTSF
    CAM_LOGD_IF(m_3ALogEnable, "[%s] PreMagicReq(%d), u4Rto(%d)", __FUNCTION__, i4MagicNumReq, u4Rto);
    pLsc->updateRatio(i4MagicNumReq, u4Rto);//request magicnum, ratio
    // 2. call normalpipe API to set obc gain & ratio
    // CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    CAM_LOGD_IF(m_3ALogEnable, "[%s] Set ratio to HW by CMDQ", __FUNCTION__);
    m_pCqTuningMgr = (CqTuningMgr*)CqTuningMgr::getInstance(m_i4SensorIndex);
    std::list<ECQ_TUNING_USER> module_en;
    CQ_TUNING_CMD_ST cmd;
    //Ratio
    CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_LSC_RATIO_0, ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto));
    CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_LSC_RATIO_1, ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto));
    module_en.push_back(EAA_TUNING_LSC);
    cmd.module = &module_en;
    cmd.serial = (i4MagicNumReq);
    m_pCqTuningMgr->update(cmd);
#endif
    CAM_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
#endif
    return MTRUE;
}


MVOID*
Hal3ARawImp::
ThreadAEStart(MVOID* arg)
{
    ::prctl(PR_SET_NAME, "ThreadAEStart", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = NICE_CAMERA_AE_Start;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);
    //::setpriority(PRIO_PROCESS, 0, priority);

    Hal3ARawImp *_this = static_cast<Hal3ARawImp*>(arg);

    // AWB start
    AAA_TRACE_D("AWB Start");
    IAwbMgr::getInstance().setAAOMode(_this->m_i4SensorDev, _this->m_bAAOMode);
    MRESULT err = IAwbMgr::getInstance().start(_this->m_i4SensorDev);
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().start() fail\n");
    }
    AAA_TRACE_END_D;

    CAM_IDX_QRY_COMB rMapping_Info;
    rMapping_Info.eIspProfile = _this->m_rParam.eIspProfile;
    _this->getNvramIndex(rMapping_Info, NSIspTuning::EModule_AWB, _this->m_3A_Index.u4AWBNVRAMIndex);
    IAwbMgr::getInstance().setNVRAMIndex(_this->m_i4SensorDev, _this->m_3A_Index.u4AWBNVRAMIndex);

    AAA_TRACE_D("AWBInfo2Pool");
    AWBResultInfo_T AWBResultInfo;
    IAwbMgr::getInstance().getAWBInfo(_this->m_i4SensorDev, AWBResultInfo.AWBInfo4ISP);
    _this->m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_AWB_RESULTINFO4ISP, &AWBResultInfo); // frame id 1 for config
    AAA_TRACE_END_D;

    // ResultPool - update AWB config result to ResutlPool
    AAA_TRACE_D("AWBConfig2Pool");
    AWBResultConfig_T AWBResultConfig;
    IAwbMgr::getInstance().configReg(_this->m_i4SensorDev, &AWBResultConfig);
    _this->m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_AWB_CONFIGRESULTTOISP, &AWBResultConfig); // frame id 1 for config
    AAA_TRACE_END_D;

    AAA_TRACE_D("AE Start");
    CAM_LOGD( "update AE config result to ResutlPool, sensorMode(%d) %d, %d",_this->m_u4SensorMode, _this->m_i4HbinWidth, _this->m_i4HbinHeight);
    IAeMgr::getInstance().sendAECtrl(_this->m_i4SensorDev, EAECtrl_SetSensorMode, _this->m_u4SensorMode, _this->m_i4HbinWidth, _this->m_i4HbinHeight, NULL);
    IAeMgr::getInstance().sendAECtrl(_this->m_i4SensorDev, EAECtrl_SetAAOMode, _this->m_bAAOMode, NULL, NULL, NULL);
    err = IAeMgr::getInstance().Start(_this->m_i4SensorDev);
    AAA_TRACE_END_D;

    // update AE config result to ResutlPool
    AAA_TRACE_D("AEConfig2Pool");
    AEResultConfig_T AEResultConfig;
    IAeMgr::getInstance().configReg(_this->m_i4SensorDev, &AEResultConfig);
    _this->m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_AE_CONFIGRESULTTOISP, &AEResultConfig); // frame id 1 for config
    AAA_TRACE_END_D;

    AAA_TRACE_D("AEInfo2Pool");
    AEResultInfo_T AEResultInfo;
    IAeMgr::getInstance().getAEInfo(_this->m_i4SensorDev, AEResultInfo.AEPerframeInfo);
    _this->m_pResultPoolObj->updateResult(LOG_TAG, ConfigMagic, E_AE_RESULTINFO, &AEResultInfo);
    AAA_TRACE_END_D;

    //IAeMgr::getInstance().sendAECtrl(_this->m_i4SensorDev, EAECtrl_SetCamScenarioMode, rCmd.u4Scenario4AEAWB, NULL, NULL, NULL);
    if (FAILED(err)) {
        CAM_LOGE("IAeMgr::getInstance().Start() fail\n");
    }

    ILscMgr::getInstance(static_cast<ESensorDev_T>(_this->m_i4SensorDev))->setCapAAOMagicNum(-1);

    ::pthread_exit((MVOID*)0);
}

MVOID*
Hal3ARawImp::
ThreadAFStart(MVOID* arg)
{
    // (1) set name
    ::prctl(PR_SET_NAME,"ThreadAFStart", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AF;
        int policy = 0, priority = 0;
        NSCam::Utils::setThreadPriority(expect_policy, expect_priority);
        NSCam::Utils::getThreadPriority(policy, priority);
        //
        CAM_LOGD(
            "[changeThreadAFStartSetting] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

    Hal3ARawImp *_this   = static_cast<Hal3ARawImp*>(arg);
    MINT32 i4SensorDev   = _this->m_i4SensorDev;
    MINT32 i4SensorIdx   = _this->m_i4SensorIdx;
    MUINT32 u4SensorMode = _this->m_u4SensorMode;

    AAA_TRACE_D("PD config");
    CAM_LOGD("[%s] +", __FUNCTION__);
    MUINT32 u4AFNVRAMIndex = 0xFFFFFFFF;
    CAM_IDX_QRY_COMB rMapping_InfoAF;
    _this->m_pIdxMgr->getMappingInfo(static_cast<MUINT32>(i4SensorDev), rMapping_InfoAF, _this->m_rAfParam.i4MagicNum);
    u4AFNVRAMIndex = _this->m_pIdxMgr->query(static_cast<MUINT32>(i4SensorDev), NSIspTuning::EModule_AF, rMapping_InfoAF, __FUNCTION__);
    IPDMgr::getInstance().config(i4SensorDev, i4SensorIdx, u4SensorMode, u4AFNVRAMIndex);
    CAM_LOGD("[%s] -", __FUNCTION__);
    AAA_TRACE_END_D;

    ::pthread_exit((MVOID*)0);
}

/*********************************************************
 *
 *   Operate Flashlight
 *
 *********************************************************/
MBOOL
Hal3ARawImp::checkAndOpenFlash(MINT32 i4Type)
{
    if(!m_bFlashOpenedByTask)
    {
        m_i4FlashType = i4Type;
        m_bFlashOpenedByTask = MTRUE;
        m_pThreadRaw->postToEventThread(E_3AEvent_Flash_On,m_i4FlashType);
        CAM_LOGD("[%s] FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
Hal3ARawImp::checkAndCloseFlash()
{
    if(m_bFlashOpenedByTask)
    {
        m_pThreadRaw->postToEventThread(E_3AEvent_Flash_Off,m_i4FlashType);
        m_bFlashOpenedByTask = MFALSE;
        CAM_LOGD("[%s] FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
        m_i4FlashType = -1;
        return MTRUE;
    }
    return MFALSE;
}

/*********************************************************
 *
 *   Flashlight flow
 *
 *********************************************************/

MVOID
Hal3ARawImp::doBackup2A()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetDoBackAEInfo, NULL, NULL, NULL, NULL);
    IAwbMgr::getInstance().backup(m_i4SensorDev);
}

MVOID
Hal3ARawImp::doRestore2A()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    // restore AE
    MBOOL bRestore = MFALSE;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetDoRestoreAEInfo, NULL, NULL, NULL, NULL);
    // restore AWB
    IAwbMgr::getInstance().restore(m_i4SensorDev);
}

MVOID
Hal3ARawImp::handleBadPicture(MINT32 i4ReqMagic, MINT32 i4P1DoneSttNum /* = -1 */)
{
    CAM_LOGD("[%s]+ ReqMagic(#%d)", __FUNCTION__, i4ReqMagic);
    // 1. get History
    MINT32 rHistoryReqMagic[HistorySize];
    ::memset(&rHistoryReqMagic, 0, sizeof(MINT32)*HistorySize);
    MINT32 i4Ret = m_pResultPoolObj->getHistory(rHistoryReqMagic);

    if(i4Ret)
    {
        // 2. get the magic of partial frame
        MINT32 i4MagicOfPartialFrame = rHistoryReqMagic[HistorySize-1];
        if(rHistoryReqMagic[HistorySize-1] == i4ReqMagic)
            i4MagicOfPartialFrame = rHistoryReqMagic[HistorySize-2];
        CAM_LOGD("[%s] ReqMagic(#%d), i4MagicOfPartialFrame(%d), History(#%d,#%d,#%d)", __FUNCTION__,
            i4ReqMagic,
            i4MagicOfPartialFrame,
            rHistoryReqMagic[0],
            rHistoryReqMagic[1],
            rHistoryReqMagic[2]);

        if(i4P1DoneSttNum >= 0)
        {
            i4MagicOfPartialFrame = i4P1DoneSttNum + 1;  // Flash off will let the next stt frame be partial
            i4ReqMagic = i4P1DoneSttNum + 2;             // The next two frame should be ready
            MY_LOGD("[%s] New i4MagicOfPartialFrame(%d), New ReqMagic(%d), use i4P1DoneSttNum(%d) to refine", __FUNCTION__, i4MagicOfPartialFrame, i4ReqMagic, i4P1DoneSttNum);
        }

        // 3. handle the partial flashlight on/off frame
        HALResultToMeta_T* pHalResult = (HALResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicOfPartialFrame,E_HAL_RESULTTOMETA, __FUNCTION__);
        FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicOfPartialFrame,E_FLASH_RESULTTOMETA, __FUNCTION__);
        AllMetaResult_T *pMetaResult = m_pResultPoolObj->getMetadata(i4MagicOfPartialFrame);
        if(pHalResult != NULL) {
            pHalResult->fgBadPicture = MTRUE;
            m_pResultPoolObj->updateResult(LOG_TAG,i4MagicOfPartialFrame,E_HAL_RESULTTOMETA,pHalResult);
        }
        if(pFLASHResult != NULL) {
            pFLASHResult->u1FlashState = MTK_FLASH_STATE_PARTIAL;
            m_pResultPoolObj->updateResult(LOG_TAG,i4MagicOfPartialFrame,E_FLASH_RESULTTOMETA,pFLASHResult);

            if(pMetaResult != NULL)
            {
                std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
                UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_STATE, pFLASHResult->u1FlashState);
            }
        }

        // 4. handle the full flashlight on/off frame
        FLASHResultToMeta_T rFLASHResult;
        pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4ReqMagic,E_FLASH_RESULTTOMETA, __FUNCTION__);
        if(pFLASHResult == NULL)
            pFLASHResult = &rFLASHResult;
        if(pFLASHResult != NULL){
            if(m_bFlashOpenedByTask || m_bIsFlashOpened)
                pFLASHResult->u1FlashState = MTK_FLASH_STATE_FIRED;
            else
                pFLASHResult->u1FlashState = MTK_FLASH_STATE_READY;
            m_pResultPoolObj->updateResult(LOG_TAG,i4ReqMagic,E_FLASH_RESULTTOMETA,pFLASHResult);
        }
    }

    CAM_LOGD("[%s]- i4Ret(%d) ReqMagic(#%d) History(#%d,#%d,#%d)", __FUNCTION__, i4Ret, i4ReqMagic, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);
}

