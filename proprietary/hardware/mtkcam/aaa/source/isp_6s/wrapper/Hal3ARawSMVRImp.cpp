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
#include "Hal3ARawSMVRImp.h"
#include <property_utils.h>

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

//static Mutex gCommonLock;

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
        case SENSOR_DEV_MAIN_3:
        {
            static Hal3ARawSMVRImp _singleton(SENSOR_DEV_MAIN_3);
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
    , m_Lock()
    , mP2Mtx()
    , m3AOperMtx1()
    , m3AOperMtx2()
    , m_i4SubsampleCount(1)
    , m_u4LastRequestNumber(-1)
    , m_i4SttMagicNumber(-1)
    , m_bIsHighQualityCaptureOn(0)
    , m_rGainDelay()
    , m_rNew3AInfo()
{
    CAM_LOGD("[%s] sensorDev(%d)", __FUNCTION__, i4SensorDevId);
}

MBOOL
Hal3ARawSMVRImp::
init(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    getPropInt("vendor.debug.camera.log", &m_3ALogEnable, 0);
    if ( m_3ALogEnable == 0 ) {
        getPropInt("vendor.debug.camera.log.hal3a", &m_3ALogEnable, 0);
    }
    m_DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, std::atomic_load((&m_Users)));

    // check user count
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, std::atomic_load((&m_Users)));
        MINT32 i4BeforeUserCount __unused = std::atomic_fetch_add((&m_Users), 1);
        return S_3A_OK;
    }

    m_i4SensorIdx = i4SensorOpenIndex;
    m_i4SubsampleCount = i4SubsampleCount;
    m_fgEnableShadingMeta = MTRUE;

    // init Thread and state mgr
    //Test SMVR run IThreadRaw
    //m_pThreadRaw = IThreadRawSMVR::createInstance(reinterpret_cast<I3AWrapper*>(this), m_i4SensorDev, m_i4SensorIdx, m_i4SubsampleCount);
    m_pThreadRaw = IThreadRaw::createInstance(this, m_i4SensorDev, m_i4SensorIdx, m_i4SubsampleCount);
    m_pTaskMgr = ITaskMgr::create(m_i4SensorDev);


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
    IAfMgr::getInstance(m_i4SensorDev).notify(E_AFNOTIFY_SW);
#endif

    //FLASH init
#if CAM3_FLASH_FEATURE_EN
    AAA_TRACE_D("FLASH init");
    //FLASH HAL init
    FlashHal::getInstance(m_i4SensorDev)->init();

    //FLASH init
    FlashMgr::getInstance(m_i4SensorDev)->init();
    CAM_LOGD("[%s] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    AAA_TRACE_END_D;
#endif

    //FLICKER init
#if CAM3_FLICKER_FEATURE_EN
    AAA_TRACE_D("FLICKER init");
    IFlickerHal::getInstance(m_i4SensorDev)->init(m_i4SensorIdx);
    AAA_TRACE_END_D;
#endif

    ILscMgr::createInstance((ESensorDev_T)m_i4SensorDev, m_i4SensorIdx)->init();

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

    CAM_LOGD("[%s] done\n", __FUNCTION__);
    MINT32 i4BeforeUserCount __unused = std::atomic_fetch_add((&m_Users), 1);
    return S_3A_OK;
}

MBOOL
Hal3ARawSMVRImp::
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
    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, std::atomic_load((&m_Users)));

    // More than one user, so decrease one User.
    MINT32 i4BeforeUserCount __unused = std::atomic_fetch_sub((&m_Users), 1);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        //bRet = postCommand(ECmd_Uninit);
        //if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");
        m_i4SubsampleCount = 1;

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
        // FLASH uninit
        err = FlashMgr::getInstance(m_i4SensorDev)->uninit();
        if (FAILED(err)) {
            CAM_LOGE("FlashMgr::getInstance().uninit() fail\n");
            return err;
        }
        // FLASH HAL uninit
        err = FlashHal::getInstance(m_i4SensorDev)->uninit();
        if (FAILED(err)) {
            CAM_LOGE("FlashHal::getInstance()->uninit() fail\n");
            return err;
        }
#endif

#if CAM3_FLICKER_FEATURE_EN
        // FLICKER uninit
        err = IFlickerHal::getInstance(m_i4SensorDev)->uninit();
        if (FAILED(err)) {
            CAM_LOGE("IFlickerHal::getInstance().uninit fail\n");
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

        CAM_LOGD("[%s] done\n", __FUNCTION__);

    }
    else    // There are still some users.
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] Still %d users \n", __FUNCTION__, std::atomic_load((&m_Users)));
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
    AllResult_T *pAllResult = m_pResultPoolObj->getResultByReqNum(ConfigMagic, __FUNCTION__);

    AAA_TRACE_D("3A SetSensorMode");
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
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSensorMode, m_u4SensorMode, u4AAWidth, u4AAHight, NULL);
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight, u4AAWidth, u4AAHight);

    //get AAO size information for LSC
    shadingConfig_T lscConfig;
    IAeMgr::getInstance().getAAOSize(m_i4SensorDev, lscConfig.AAOBlockW, lscConfig.AAOBlockH);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(lscConfig.AAOstrideSize)), NULL, NULL, NULL);

    //set Hbin for TSFS
    lscConfig.u4HBinWidth = u4AAWidth;
    lscConfig.u4HBinHeight= u4AAHight;

    ILscMgr *pLscMgr=ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
    pLscMgr->config(lscConfig);
    pLscMgr->setSensorMode(static_cast<ESensorMode_T>(m_u4SensorMode), u4AFWidth, u4AFHeight, MFALSE);

    //update LSC config result to ResutlPool
    LSCConfigResult_T rLSCConfigResult;
    pLscMgr->getLSCResultPool(&rLSCConfigResult);
    pAllResult->ModuleResult[E_LSC_CONFIGRESULTTOISP]->write(&rLSCConfigResult); // frame id 1 for config


#if CAM3_FLICKER_FEATURE_EN
    IFlickerHal::getInstance(m_i4SensorDev)->setSensorMode(m_u4SensorMode, u4AAWidth, u4AAHight);
#endif

    if(m_pCcuCtrl3ACtrl == NULL)
         m_pCcuCtrl3ACtrl = ICcuCtrl3ACtrl::createInstance();
    if(m_pCcuCtrl3ACtrl)
    {
        //collect frame subsample info
        struct ccu_frame_subsmpl_info subsmpl_info;

        subsmpl_info.work_type = CAM_WORK_TYPE_DL; //direct link mode
        subsmpl_info.subsmpl_ratio = rConfigInfo.i4SubsampleCount; //subsample 4:1 (120->30)

        //send fram subsample info to CCU via ccuControl command: MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO
        m_pCcuCtrl3ACtrl->ccuControl(MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO, &subsmpl_info, NULL);
    }

    // FIXME (remove): update TG Info to 3A modules
    Hal3ARawImp::updateTGInfo();

    AAA_TRACE_END_D;

    m_bFrontalBin = (m_i4TgWidth == (MINT32)u4AFWidth && m_i4TgHeight == (MINT32)u4AFHeight) ? MFALSE : MTRUE;

    // AE start
    AAA_TRACE_D("AE Start");
    MBOOL bAAOMode;
    if(rConfigInfo.i4BitMode == EBitMode_12Bit)
        bAAOMode = 0;
    else if(rConfigInfo.i4BitMode == EBitMode_14Bit)
        bAAOMode = 1;
    else
    {
        CAM_LOGE("Not support BitMode(%d)", rConfigInfo.i4BitMode);
        AAA_TRACE_END_D;
        return err;
    }
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetAAOMode, bAAOMode, NULL, NULL, NULL);
    MBOOL bAESMBuffermode = MTRUE;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSMBuffermode, bAESMBuffermode, m_i4SubsampleCount, NULL, NULL);

    CAM_IDX_QRY_COMB rMapping_Info;
    m_pIdxMgr->getMappingInfo(static_cast<ESensorDev_T>(m_i4SensorDev), rMapping_Info, m_rParam.i4MagicNum);
    MUINT32 u4LTMNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_LTM, rMapping_Info, __FUNCTION__);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetISPNvramLTMIndex, u4LTMNVRAMIndex, NULL, NULL,NULL);

    err = IAeMgr::getInstance().Start(m_i4SensorDev);
    AAA_TRACE_END_D;
    MUINT32 u4AENVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_AE, rMapping_Info, __FUNCTION__);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetNVRAMIndex, u4AENVRAMIndex, NULL, NULL, NULL);

    if (FAILED(err)) {
    CAM_LOGE("IAeMgr::getInstance().Start() fail\n");
        return err;
    }

    AAA_TRACE_D("AEInfo2Pool");
    AEResultInfo_T AEResultInfo;
    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEResultInfo.AEPerframeInfo);
    pAllResult->ModuleResult[E_AE_RESULTINFO]->write(&AEResultInfo); // frame id 1 for config
    AAA_TRACE_END_D;

    // update AE config result to ResutlPool
    CAM_LOGD("[%s] update AE config result to ResutlPool", __FUNCTION__);
    AEResultConfig_T AEResultConfig;
    IAeMgr::getInstance().configReg(m_i4SensorDev, &AEResultConfig);
    pAllResult->ModuleResult[E_AE_CONFIGRESULTTOISP]->write(&AEResultConfig); // frame id 1 for config

    // AWB start
    AAA_TRACE_D("AWB Start");
    err = IAwbMgr::getInstance().start(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().start() fail\n");
        return E_3A_ERR;
    }
    AAA_TRACE_END_D;
    rMapping_Info.eIspProfile = m_rParam.eIspProfile;
    Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_AWB, m_3A_Index.u4AWBNVRAMIndex);
    IAwbMgr::getInstance().setNVRAMIndex(m_i4SensorDev, m_3A_Index.u4AWBNVRAMIndex);

    AAA_TRACE_D("AWBInfo2Pool");
    AWBResultInfo_T AWBResultInfo;
    IAwbMgr::getInstance().getAWBInfo(m_i4SensorDev, AWBResultInfo.AWBInfo4ISP);
    pAllResult->ModuleResult[E_AWB_RESULTINFO4ISP]->write(&AWBResultInfo); // frame id 1 for config
    AAA_TRACE_END_D;

    // update AWB config result to ResutlPool
    CAM_LOGD("[%s] update AWB config result to ResutlPool", __FUNCTION__);
    AWBResultConfig_T AWBResultConfig;
    IAwbMgr::getInstance().configReg(m_i4SensorDev, &AWBResultConfig);
    pAllResult->ModuleResult[E_AWB_CONFIGRESULTTOISP]->write(&AWBResultConfig); // frame id 1 for config

#if CAM3_FLASH_FEATURE_EN
    // Flash start
    AAA_TRACE_D("FLASH Start");
    FlashMgr::getInstance(m_i4SensorDev)->setBitMode(rConfigInfo.i4BitMode);

    Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AE, m_3A_Index.u4FlashAENVRAMIndex);
    Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_Calibration, m_3A_Index.u4FlashCaliNVRAMIndex);
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

#if CAM3_AF_FEATURE_EN
    MUINT32 u4AFNVRAMIndex = 0xFFFFFFFF;
    CAM_IDX_QRY_COMB rMapping_InfoAF;
    m_pIdxMgr->getMappingInfo(static_cast<ESensorDev_T>(m_i4SensorDev), rMapping_InfoAF, m_rAfParam.i4MagicNum);
    u4AFNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_AF, rMapping_InfoAF, __FUNCTION__);
    IAfMgr::getInstance(m_i4SensorDev).setNVRAMIndex( u4AFNVRAMIndex);


    AAA_TRACE_D("PD Start");
    err = IPDMgr::getInstance().config(m_i4SensorDev, m_i4SensorIdx, m_u4SensorMode, u4AFNVRAMIndex);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("IPDMgr::getInstance().config fail\n");
        return err;
    }
    AAA_TRACE_D("AF Start");
    err = IAfMgr::getInstance(m_i4SensorDev).config();
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Start() fail\n");
        return err;
    }

    // update AF config result to ResutlPool
    CAM_LOGD("[%s] update AF config result to ResutlPool", __FUNCTION__);
    AFResultConfig_T AFResultConfig;
    IAfMgr::getInstance(m_i4SensorDev).getHWCfgReg(&AFResultConfig);

    CAM_LOGD("HW-%s : enableAFHw(%d)", __FUNCTION__, AFResultConfig.afIspRegInfo.enableAFHw);
    pAllResult->ModuleResult[E_AF_CONFIGRESULTTOISP]->write(&AFResultConfig); // frame id 1 for config

    // update PDO config result to ResutlPool
    CAM_LOGD("[%s] update PDO config result to ResutlPool", __FUNCTION__);
    isp_pdo_cfg_t PDOResultConfig;
    IPDMgr::getInstance().getPDOHWCfg(m_i4SensorDev, &PDOResultConfig);
    pAllResult->ModuleResult[E_PDO_CONFIGRESULTTOISP]->write(&PDOResultConfig); // frame id 1 for config

#endif
    MBOOL enable_flk = IFlickerHal::getInstance(m_i4SensorDev)->getInfo();
    CAM_LOGD("[%s] enable_flk=%d", __FUNCTION__, (int)enable_flk);
#if CAM3_FLICKER_FEATURE_EN
    // Flicker start
    AAA_TRACE_D("FLICKER Start");
    err = IFlickerHal::getInstance(m_i4SensorDev)->config();
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("IFlickerHal::getInstance().config() fail\n");
        return err;
    }

    // update FLK config result to ResutlPool
    CAM_LOGD("[%s] update FLK config result to ResutlPool", __FUNCTION__);
    FLKResultConfig_T FLKResultConfig;
    IFlickerHal::getInstance(m_i4SensorDev)->getHWCfgReg(&FLKResultConfig);
    pAllResult->ModuleResult[E_FLK_CONFIGRESULTTOISP]->write(&FLKResultConfig); // frame id 1 for config
#endif
    m_pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

    CAM_LOGD("[%s] m_i4SubsampleCount(%d)", __FUNCTION__, m_i4SubsampleCount);

    CAM_LOGD("[%s]-", __FUNCTION__);

    return S_3A_OK;
}

MBOOL
Hal3ARawSMVRImp::
start()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    MRESULT isAFLampOn = MFALSE;
    m_i4SttMagicNumber = 0;
    m_bPreStop = MFALSE;

#if CAM3_AF_FEATURE_EN
    // enable AF thread
    m_pThreadRaw->enableAFThread(m_pTaskMgr);
#endif

    m_pThreadRaw->createEventThread();

#if CAM3_FLASH_FEATURE_EN
    isAFLampOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
#endif

#if CAM3_FLICKER_FEATURE_EN
    IFlickerHal::getInstance(m_i4SensorDev)->start(FLK_ATTACH_PRIO_MEDIUM);
#endif

    // setStrobeMode
    MBOOL bStrobeMode = isAFLampOn ? MTRUE : MFALSE;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);
    IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetSensorDelayInfo, reinterpret_cast<MINTPTR>(&m_rGainDelay.u4SutterGain_Delay), reinterpret_cast<MINTPTR>(&m_rGainDelay.u4SensorGain_Delay), reinterpret_cast<MINTPTR>(&m_rGainDelay.u4ISPGain_Delay), NULL);
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
    m_i4SttMagicNumber = 0;
    m_i4SubsampleCount = 1;
    m_bIsFlashOpened = MFALSE;

    // Clear Task3a
    m_pTaskMgr->clearTaskQueue(Task_Update_3A);

    // AE stop
    AAA_TRACE_D("AE Stop");
    MBOOL bAESMBuffermode = MFALSE;
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSMBuffermode, bAESMBuffermode, m_i4SubsampleCount, NULL, NULL);
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

#if CAM3_AF_FEATURE_EN
    // disable AF thread
    AAA_TRACE_D("AF THREAD disable");
    m_pThreadRaw->disableAFThread();
    AAA_TRACE_END_D;

    // AF stop
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

    ::IPDTblGen::getInstance()->stop(m_i4SensorDev);

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pTuningBuf, ResultP2_T* pResultP2)
{
// ISP6.0 remove content
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe)
{
// ISP6.0 remove content
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
setParams(Param_T const &rNewParam, MBOOL bUpdateScenario)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    std::lock_guard<std::mutex> autoLock(m3AOperMtx2);

    if (!m_bEnable3ASetParams) return MTRUE;

#if CAM3_LSC_FEATURE_EN
    // ====================================== Shading =============================================
    if (m_fgEnableShadingMeta)
        NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(rNewParam.u1ShadingMode ? MTRUE : MFALSE);
#endif

    // ====================================== AE ==============================================
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps);
    IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev, rNewParam.u4AeMeterMode);
    IAeMgr::getInstance().setAERotateDegree(m_i4SensorDev, rNewParam.i4RotateDegree);
    IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.i4IsoSpeedMode);
    IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas);
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

    if((rNewParam.bIsFDReady != m_rParam.bIsFDReady) || m_bIsFirstSetParams)
    {
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetFDSkipCalAE, (rNewParam.bIsFDReady ==0)? MTRUE:MFALSE, NULL, NULL, NULL);
    }

    if (bUpdateScenario){
        CAM_IDX_QRY_COMB rMapping_Info;
        m_pIdxMgr->getMappingInfo(static_cast<ESensorDev_T>(m_i4SensorDev), rMapping_Info, rNewParam.i4MagicNum);
        MUINT32 u4AENVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_AE, rMapping_Info, __FUNCTION__);
        MUINT32 u4AWBNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_AWB, rMapping_Info, __FUNCTION__);
        MUINT32 u4AFNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_AF, rMapping_Info, __FUNCTION__);
        MUINT32 u4FlashAENVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_Flash_AE, rMapping_Info, __FUNCTION__);
        MUINT32 u4FlashAWBNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_Flash_AWB, rMapping_Info, __FUNCTION__);
        MUINT32 u4FlashCaliNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_Flash_Calibration, rMapping_Info, __FUNCTION__);

        memset(&rMapping_Info.eISO_Idx, 0, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
        MUINT32 u4LTMNVRAMIndex = m_pIdxMgr->query(static_cast<ESensorDev_T>(m_i4SensorDev), NSIspTuning::EModule_LTM, rMapping_Info, __FUNCTION__);

        IAwbMgr::getInstance().setNVRAMIndex(m_i4SensorDev, u4AWBNVRAMIndex);
        IAfMgr::getInstance(m_i4SensorDev).setNVRAMIndex( u4AFNVRAMIndex);
        IPDMgr::getInstance().setNVRAMIndex( m_i4SensorDev, u4AFNVRAMIndex);
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetNVRAMIndex, u4AENVRAMIndex, NULL, NULL, NULL);
        FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AE, u4FlashAENVRAMIndex);
        FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AWB, u4FlashAWBNVRAMIndex);
        IAwbMgr::getInstance().setStrobeNvramIdx(m_i4SensorDev, u4FlashAWBNVRAMIndex);
        FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_CALIBRATION, u4FlashCaliNVRAMIndex);
        IAwbMgr::getInstance().setFlashCaliNvramIdx(m_i4SensorDev, u4FlashCaliNVRAMIndex);
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetISPNvramLTMIndex,u4LTMNVRAMIndex, NULL, NULL, NULL);
        CAM_LOGD_IF(m_3ALogEnable, "[%s] NVRAM Index AE(%d) AWB(%d) AF(%d) F_AE(%d) F_AWB(%d) F_Cali(%d) OBC(%d)",
                        __FUNCTION__, u4AENVRAMIndex, u4AWBNVRAMIndex, u4AFNVRAMIndex, u4FlashAENVRAMIndex, u4FlashAWBNVRAMIndex, u4FlashCaliNVRAMIndex, u4LTMNVRAMIndex);
    }

    if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
    {
        AE_SENSOR_PARAM_T strSensorParams;
        strSensorParams.u4Sensitivity   = rNewParam.i4Sensitivity;
        strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTime;
        strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSensorParams, reinterpret_cast<MINTPTR>(&(strSensorParams)), NULL, NULL, NULL);
    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] setAEMode(%d)", __FUNCTION__, rNewParam.u4AeMode);
    if (IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetNeedPresetControlCCU, NULL, NULL, NULL, NULL)){
      IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetPresetControlCCU, NULL, NULL, NULL, NULL);
    }
    // ====================================== AWB ==============================================
    IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock);
    IAwbMgr::getInstance().setAWBMode(m_i4SensorDev, rNewParam.u4AwbMode);
    if( rNewParam.rScaleCropRect.i4Xoffset != 0 && rNewParam.rScaleCropRect.i4Yoffset != 0 &&
        rNewParam.rScaleCropRect.i4Xwidth  != 0 && rNewParam.rScaleCropRect.i4Yheight != 0 )
        IAwbMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewParam.rScaleCropRect.i4Xoffset,rNewParam.rScaleCropRect.i4Yoffset,rNewParam.rScaleCropRect.i4Xwidth,rNewParam.rScaleCropRect.i4Yheight);

    IAwbMgr::getInstance().setColorCorrectionMode(m_i4SensorDev, rNewParam.u1ColorCorrectMode);

    if (rNewParam.u4AwbMode == MTK_CONTROL_AWB_MODE_OFF &&
        rNewParam.u1ColorCorrectMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        IAwbMgr::getInstance().setColorCorrectionGain(m_i4SensorDev, rNewParam.fColorCorrectGain[0], rNewParam.fColorCorrectGain[1], rNewParam.fColorCorrectGain[3]);
    }

#if CAM3_FLASH_FEATURE_EN
    // ====================================== Flash ==============================================
    FlashMgr::getInstance(m_i4SensorDev)->setAeFlashMode(rNewParam.u4AeMode, rNewParam.u4StrobeMode);
    int bMulti;
    if(rNewParam.u4CapType == ECapType_MultiCapture)
        bMulti=1;
    else
        bMulti=0;
    FlashMgr::getInstance(m_i4SensorDev)->setCamMode(rNewParam.u4CamMode);
    FlashMgr::getInstance(m_i4SensorDev)->setEvComp(rNewParam.i4ExpIndex, rNewParam.fExpCompStep);
#endif

    if(rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD || rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT) {
        if((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_PREVIEW || m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG) && isStrobeBVTrigger() && m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH)
            m_bIsRecordingFlash = MTRUE;
    } else {
        m_bIsRecordingFlash = MFALSE;
    }
#if CAM3_FLICKER_FEATURE_EN
    // ====================================== Flicker ==============================================
    IFlickerHal::getInstance(m_i4SensorDev)->setFlickerMode(rNewParam.u4AntiBandingMode);
#endif

    // ====================================== FlowCtrl ==============================================
    m_rParam = rNewParam;

    CAM_LOGD_IF(m_3ALogEnable, "[%s] m_rParam.u1ShadingMapMode(%d)", __FUNCTION__, m_rParam.u1ShadingMapMode);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
setFlashLightOnOff(MBOOL bOnOff, MBOOL bMainPre, MINT32 i4P1DoneSttNum /* = -1 */ __unused)
{
#if CAM3_FLASH_FEATURE_EN
    m_bIsFlashOpened = bOnOff;
    CAM_LOGD_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    if (!bOnOff)
    {
        //modified to update strobe state after capture for ae manager
        MBOOL bStrobeMode = MFALSE;
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);
        FlashHal::getInstance(m_i4SensorDev)->setCaptureFlashOnOff(0);
    }
    else //flash on
    {
        if (bMainPre) FlashHal::getInstance(m_i4SensorDev)->setCaptureFlashOnOff(1);
        else FlashHal::getInstance(m_i4SensorDev)->setVideoTorchOnOff(1);
    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
chkPreFlashOnCond() const
{
    // Torch
    FlashPolicy_T rFlashPolicy;
    rFlashPolicy.bIsFlashOnCapture = isFlashOnCapture();
    rFlashPolicy.bIsFlashCharging = (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING);
    rFlashPolicy.bIsRecordingFlash = m_bIsRecordingFlash;
    FlashHal::getInstance(m_i4SensorDev)->hasHw(rFlashPolicy.i4HWSuppportFlash);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), bIsRecordingFlash(%d), i4HWSuppportFlash(%d)", __FUNCTION__,
        rFlashPolicy.bIsFlashOnCapture,
        rFlashPolicy.bIsFlashCharging,
        rFlashPolicy.bIsRecordingFlash,
        rFlashPolicy.i4HWSuppportFlash);
    return HAL3A_POLICY_TORCH_ONOFF(m_i4SensorDev, m_rParam, rFlashPolicy);
}

MINT32
Hal3ARawSMVRImp::
getCurrResult(MUINT32 i4FrmId, MINT32 i4SubsampleIndex) const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] + i4MagicNum(%d) i4SubsampleIndex:%d", __FUNCTION__, i4FrmId, i4SubsampleIndex);

    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState = MTK_CONTROL_AWB_STATE_INACTIVE;

    // ResultPool - To update Vector info
    AllResult_T *pAllResult = m_pResultPoolObj->getInstance(m_i4SensorDev)->getResultByReqNum(i4FrmId, __FUNCTION__);
    if(pAllResult == NULL)
    {
        CAM_LOGE("[%s] pAllResult is NULL", __FUNCTION__);
        return MFALSE;
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s]  pAllResult(%p)", __FUNCTION__, pAllResult);

    // ResultPool - update previous setting to resultPool
    CAM_LOGD_IF(m_3ALogEnable, "[%s] update old setting: AeState/Exp/ISO %d/%lld/%d", __FUNCTION__, m_rNew3AInfo.u1AeState, m_rNew3AInfo.i8SensorExposureTime, m_rNew3AInfo.i4SensorSensitivity);
    pAllResult->rOld3AInfo.u1AeState = m_rNew3AInfo.u1AeState;
    pAllResult->rOld3AInfo.i8SensorExposureTime = m_rNew3AInfo.i8SensorExposureTime;
    pAllResult->rOld3AInfo.i4SensorSensitivity = m_rNew3AInfo.i4SensorSensitivity;
    ::memcpy(&pAllResult->rOld3AInfo.rGainDelay, &m_rGainDelay, sizeof(m_rGainDelay));

    // ResultPool - To update resultPool
    HALResultToMeta_T     rHALResult;
    AEResultToMeta_T      rAEResult;
    AWBResultToMeta_T     rAWBResult;
    LSCResultToMeta_T     rLSCResult;
    FLASHResultToMeta_T   rFLASHResult;
    FLKResultToMeta_T     rFLKResult;
    LSCConfigResult_T     rLSCConfigResult;

    rHALResult.i4FrmId = i4FrmId;
    rHALResult.fgBadPicture = MFALSE;

    //===== Get AE/AWB FD region =====
    AEResultInfo_T AEResultInfo;
    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEResultInfo.AEPerframeInfo);
    MUINT8 u1AeState = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.i4AEState;
    if (AEResultInfo.AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rAEResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rAEResult.u1AeState = u1AeState;

    rAEResult.fgAeBvTrigger = AEResultInfo.AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger;

    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rAEResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rAEResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rAEResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
    if (m_rParam.u1RollingShutterSkew)
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetSensorRollingShutter, reinterpret_cast<MINTPTR>(&(rAEResult.i8SensorRollingShutterSkew)), NULL, NULL, NULL);

    // update AE config result to ResutlPool
    CAM_LOGD("[%s] update AE config result to ResutlPool", __FUNCTION__);
    AEResultConfig_T AEResultConfig;
    IAeMgr::getInstance().configReg(m_i4SensorDev, &AEResultConfig);

    CAM_LOGD("[%s] u4P1DGNGain(%d)", __FUNCTION__, AEResultInfo.AEPerframeInfo.rAEISPInfo.u4P1DGNGain);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] update new setting: AeState/Exp/ISO %d/%lld/%d", __FUNCTION__, rAEResult.u1AeState, rAEResult.i8SensorExposureTime, rAEResult.i4SensorSensitivity/*, rAFResult.u1AfState*/);

    *(const_cast<MUINT8*>(&m_rNew3AInfo.u1AeState)) = rAEResult.u1AeState;
    *(const_cast<MINT64*>(&m_rNew3AInfo.i8SensorExposureTime)) = rAEResult.i8SensorExposureTime;
    *(const_cast<MINT32*>(&m_rNew3AInfo.i4SensorSensitivity)) = rAEResult.i4SensorSensitivity;

    // clear memory
    pAllResult->vecExifInfo.clear();
    pAllResult->vecTonemapCurveRed.clear();
    pAllResult->vecTonemapCurveGreen.clear();
    pAllResult->vecTonemapCurveBlue.clear();
    pAllResult->vecColorCorrectMat.clear();
    pAllResult->vecDbg3AInfo.clear();
    pAllResult->vecDbgShadTbl.clear();

    //===== Get HAL result =====
    rHALResult.u1SceneMode = m_rParam.u4SceneMode;


    //===== Get AWB result =====
    IAwbMgr::getInstance().getAWBState(m_i4SensorDev, eAwbState);
    rAWBResult.u1AwbState= eAwbState;
    AWB_GAIN_T rAwbGain = {};
    IAwbMgr::getInstance().getAWBGain(m_i4SensorDev, rAwbGain, rAWBResult.i4AwbGainScaleUint);
    rAWBResult.i4AwbGain[0] = rAwbGain.i4R;
    rAWBResult.i4AwbGain[1] = rAwbGain.i4G;
    rAWBResult.i4AwbGain[2] = rAwbGain.i4B;

    // update AWB config result to ResutlPool
    CAM_LOGD("[%s] update AWB config result to ResutlPool", __FUNCTION__);
    AWBResultConfig_T AWBResultConfig;
    IAwbMgr::getInstance().configReg(m_i4SensorDev, &AWBResultConfig);

    IAwbMgr::getInstance().getColorCorrectionGain(m_i4SensorDev, rAWBResult.fColorCorrectGain[0],rAWBResult.fColorCorrectGain[1],rAWBResult.fColorCorrectGain[3]);
    rAWBResult.fColorCorrectGain[2] = rAWBResult.fColorCorrectGain[1];

    AWBResultInfo_T AWBResultInfo;
    IAwbMgr::getInstance().getAWBInfo(m_i4SensorDev, AWBResultInfo.AWBInfo4ISP);

    //===== Get FLASH result =====
#if CAM3_FLASH_FEATURE_EN
    //rResult.u1FlashState = FlashMgr::getInstance(m_i4SensorDev)->getFlashState();
#if 1 //mark out temporarily
    rFLASHResult.u1FlashState =
        (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_UNAVAILABLE) ?
        MTK_FLASH_STATE_UNAVAILABLE :
        (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING) ?
        MTK_FLASH_STATE_CHARGING :
        (FlashHal::getInstance(m_i4SensorDev)->isAFLampOn() ? MTK_FLASH_STATE_FIRED : MTK_FLASH_STATE_READY);
#endif
#endif

    //===== Get Flicker result =====
#if CAM3_FLICKER_FEATURE_EN
    MINT32 i4FlkStatus = IAeMgr::getInstance().getAEAutoFlickerState(m_i4SensorDev);
    MUINT8 u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_NONE;
    if (i4FlkStatus == 0) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_50HZ;
    if (i4FlkStatus == 1) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_60HZ;
    rFLKResult.u1SceneFlk = (MUINT8)u1ScnFlk;
    // update FLK config result to ResutlPool
    CAM_LOGD("[%s] update FLK config result to ResutlPool", __FUNCTION__);
    FLKResultConfig_T FLKResultConfig;
    IFlickerHal::getInstance(m_i4SensorDev)->getHWCfgReg(&FLKResultConfig);
#endif

    //===== Get LSC result =====
#if CAM3_LSC_FEATURE_EN
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));

    //Get the computing result of LSC to result pool
    pLsc->getLSCResultPool(&rLSCConfigResult);

    MBOOL fgRequireShadingIntent = 1; /*
        (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG);*/
    pAllResult->vecLscData.clear();
    rLSCResult.i4CurrTblIndex = -1;
    if ((pLsc->getOnOff() && fgRequireShadingIntent && m_rParam.i4RawType == NSIspTuning::ERawType_Pure /*IMGO pure raw*/) ||
        (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON))
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] #(%d) LSC table to metadata", __FUNCTION__, i4FrmId);
        //pLsc->getCurrTbl(pAllResult->vecLscData);
        rLSCResult.i4CurrTblIndex = pLsc->getCurrTblIndex();
        CAM_LOGD_IF(m_3ALogEnable, "[%s] CurrTblIndex:%d", __FUNCTION__, rLSCResult.i4CurrTblIndex);
    }
    rLSCResult.fgShadingMapOn = (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON);
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

    //===== Get Exif result =====
    if (m_rParam.u1IsGetExif)
    {
        if(pAllResult->vecExifInfo.size()==0)
            pAllResult->vecExifInfo.resize(1);
        Hal3ARawImp::get3AEXIFInfo(pAllResult->vecExifInfo.back());

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
    }

    //===== update modules result to ResultPool =====
    pAllResult->ModuleResult[E_HAL_RESULTTOMETA]->write(&rHALResult);
    pAllResult->ModuleResult[E_AE_RESULTTOMETA]->write(&rAEResult);
    pAllResult->ModuleResult[E_AWB_RESULTTOMETA]->write(&rAWBResult);
    pAllResult->ModuleResult[E_LSC_RESULTTOMETA]->write(&rLSCResult);
    pAllResult->ModuleResult[E_FLASH_RESULTTOMETA]->write(&rFLASHResult);
    pAllResult->ModuleResult[E_FLK_RESULTTOMETA]->write(&rFLKResult);
    pAllResult->ModuleResult[E_LSC_CONFIGRESULTTOISP]->write(&rLSCConfigResult);
    pAllResult->ModuleResult[E_AE_CONFIGRESULTTOISP]->write(&AEResultConfig);
    pAllResult->ModuleResult[E_FLK_CONFIGRESULTTOISP]->write(&FLKResultConfig);
    pAllResult->ModuleResult[E_AWB_CONFIGRESULTTOISP]->write(&AWBResultConfig);
    pAllResult->ModuleResult[E_AE_RESULTINFO]->write(&AEResultInfo);
    pAllResult->ModuleResult[E_AWB_RESULTINFO4ISP]->write(&AWBResultInfo);

  //===== get AF state, and update to ResultPool =====
    // MW Handshake : Early Call Back. ZSD-flash capture,MW need Af state to judge precapture done
    AFResult_T afResult;
    IAfMgr::getInstance(m_i4SensorDev).getResult(afResult);
    pAllResult->rEarlyCB.u1AfState = afResult.afState;
    pAllResult->rEarlyCB.u1AeState = rAEResult.u1AeState;
    pAllResult->rEarlyCB.u1AeMode = rAEResult.u1AeMode;
    pAllResult->rEarlyCB.u1FlashState = rFLASHResult.u1FlashState;
    pAllResult->rEarlyCB.fgAeBvTrigger = rAEResult.fgAeBvTrigger;

    // CCT 3A Need to overwrite AF state when supportAF
    MINT32 IsSupportAF = IAfMgr::getInstance(m_i4SensorDev).getAfSupport();//isAFSupport();
    MINT32 i4AFEnable = 0;
    MUINT32 i4OutLens = 0;
    IAfMgr::getInstance(m_i4SensorDev).CCTOPAFGetEnableInfo((MVOID*)&i4AFEnable, &i4OutLens);
    if( IsSupportAF && (!i4AFEnable)
        && pAllResult->rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED
        && pAllResult->rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
    {
        CAM_LOGD_IF(1, "[%s] overwrite AF state(%d)", __FUNCTION__, pAllResult->rEarlyCB.u1AfState);
        pAllResult->rEarlyCB.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
    }
    m_pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] - i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]+ cmd(%d)", __FUNCTION__, r3ACmd);

    if (pParam == NULL) {
        CAM_LOGE("[%s:%d] pParam can not be NULL", __FUNCTION__, __LINE__);
        return MFALSE;
    }

    // update Gyro sensor info
    if(gAcceValid && gGyroValid)
    {
        GyroSensor_Param_T rGyroSensorParam;
        memcpy(rGyroSensorParam.i4AcceInfo, gAcceInfo, sizeof(MINT32) * 3);
        memcpy(rGyroSensorParam.i4GyroInfo, gGyroInfo, sizeof(MINT32) * 3);
        rGyroSensorParam.u8AcceTS = gAcceTS;
        rGyroSensorParam.u8GyroTS = gGyroTS;

        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSensorListenerParams, reinterpret_cast<MINTPTR>(&(rGyroSensorParam)), NULL, NULL, NULL);
    }

    /*****************************
     *     Handle 3A command
     *****************************/
    if(r3ACmd != ECmd_Update)
    {
        // Use r3ACmd to create the tasks of precapture, touch, capture,etc.
        m_pTaskMgr->sendEvent(r3ACmd);
    }

    /*****************************
     *     Capture Flow
     *****************************/
    if ((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) &&
        (r3ACmd == ECmd_Update) && (m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF))
    {
        // high quality cap
        MINT32 iForceHqc = 1;
        getPropInt("vendor.debug.camera.force_hqc", &iForceHqc, 1);
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
            MBOOL bAELock = AEPerframeInfo.rAEUpdateInfo.bAPAELock;
            MINT32 i4EVCap = 0;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetEVCompensateIndex, reinterpret_cast<MINTPTR>(&i4EVCap), NULL, NULL, NULL);
            MBOOL bIsEVchaged = m_i4EVCap != i4EVCap;
            MBOOL bIsStartCapture = m_rParam.u1IsStartCapture;

            if (bIsEVchaged){
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetEVCompensateIndex, reinterpret_cast<MINTPTR>(&m_i4EVCap), NULL, NULL, NULL);
                bAELock = MFALSE;
            }
            CAM_LOGD("[%s] i4OperMode(%d) mbIsHDRShot(%d) bAELock(%d) EV(%d) EVchanged (%d)", __FUNCTION__, i4OperMode, mbIsHDRShot, bAELock, m_i4EVCap, bIsEVchaged);
            /*
            some senerio should not updateCaptureParams
            1. capture with flash
            2. HDR shot, camera3
            3. i4OperMode != EOperMode_Meta
            4. AELock is set
            */
#if CAM3_FLASH_FEATURE_EN
            if(!mbIsHDRShot && (i4OperMode != EOperMode_Meta) && !bAELock)
#endif
            {
                AE_MODE_CFG_T previewInfo;
                previewInfo.u4ExposureMode = AEPerframeInfo.rAEUpdateInfo.u4ExposureMode;
                previewInfo.u2FrameRate = AEPerframeInfo.rAEUpdateInfo.u2FrameRate_x10;
                previewInfo.u4CWValue = AEPerframeInfo.rAEUpdateInfo.u4CWValue;
                previewInfo.u4AvgY = AEPerframeInfo.rAEUpdateInfo.u4AvgY;
                previewInfo.u4RealISO = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
                previewInfo.i2FlareOffset = AEPerframeInfo.rAEISPInfo.i2FlareOffset;
                previewInfo.i2FlareGain = AEPerframeInfo.rAEISPInfo.i2FlareGain;
                previewInfo.u4AEFinerEVIdxBase = AEPerframeInfo.rAEISPInfo.u4AEFinerEVIdxBase;;
                previewInfo.u4Eposuretime = (MUINT32)(AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns/1000);
                previewInfo.u4AfeGain = AEPerframeInfo.rAEISPInfo.u4P1SensorGain;
                previewInfo.u4IspGain = AEPerframeInfo.rAEISPInfo.u4P1DGNGain;
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

        m_pTaskMgr->sendEvent(ECmd_CaptureStart);
    }

    /*****************************
     *     3A Execution
     *****************************/
    else if(r3ACmd == ECmd_Update)
    {
        /*****************************
         *     Prepare data
         *****************************/
        MBOOL isExistPrecap = m_pTaskMgr->isExistTask(TASK_ENUM_3A_PRECAPTURE);
        MBOOL isExistCap = m_pTaskMgr->isExistTask(TASK_ENUM_3A_CAPTURE);

        TaskData rData;
        rData.i4RequestMagic = pParam->i4MagicNum;
        rData.bFlashOnOff = m_bIsFlashOpened;
        rData.bIsHqCap = m_bIsHighQualityCaptureOn;
        rData.bIsAfTriggerInPrecap = ((m_rAfParam.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START) && (m_rAfParam.u1PrecapTrig || isExistPrecap));
        rData.pThreadRaw = (MVOID*)m_pThreadRaw;
        m_i4SttMagicNumber = m_pTaskMgr->prepareStt(rData,Task_Update_3A);

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


        IspTuningCustom* pIspTuningCustom = IspTuningCustom::createInstance((ESensorDev_T)m_i4SensorDev, m_i4SensorIdx);
        rQueryCommandQ.eApp = (EApp_T)pIspTuningCustom->map_AppName_to_MappingInfo();
        rQueryCommandQ.eZoom_Idx = (EZoom_T)pIspTuningCustom->map_zoom_value_to_index(m_rParam.i4ZoomRatio);
        rQueryCommandQ.eSize = (ESize_T)pIspTuningCustom->map_Target_Size_to_MappingInfo(m_rParam.targetSize.w, m_rParam.targetSize.h, "hal3araw_postcommand");


        // check the flash state, and update to index map mgr
        AllResult_T *pAllResult = m_pResultPoolObj->getResultByReqNum(m_i4SttMagicNumber, __FUNCTION__);
        FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)(pAllResult->ModuleResult[E_FLK_RESULTTOMETA]->read());
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
        m_pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

        MINT32 i4IsFlashOnCapture = isFlashOnCapture();
        if(i4IsFlashOnCapture && m_bIsHighQualityCaptureOn)
            rQueryCommandQ.eFlash = EFlash_MainFlash;
        CAM_LOGD("[%s] i4IsFlashOnCapture(%d) bIsHighQualityCaptureOn(%d) rQueryCommandQ eFlash(%d)", __FUNCTION__, i4IsFlashOnCapture, m_bIsHighQualityCaptureOn, rQueryCommandQ.eFlash);

        AAA_TRACE_HAL(IdxMgrQuery);

        m_pIdxMgr->setMappingInfo(static_cast<NSIspTuning::ESensorDev_T>(m_i4SensorDev), rQueryCommandQ, m_rParam.i4MagicNum);
        CAM_IDX_QRY_COMB rMapping_Info;
        m_pIdxMgr->getMappingInfo(static_cast<ESensorDev_T>(m_i4SensorDev), rMapping_Info, m_rParam.i4MagicNum);

        MINT32 cacheEnable = 1;
        getPropInt("vendor.debug.cache.mode", &cacheEnable, 1);

        m_rIdxCache.setEnable(cacheEnable);
        m_rIdxCache.updateKey(rMapping_Info, IDXCACHE_STAGE_P1);
        m_rIdxCache.setLogEnable();

        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_AE, m_3A_Index.u4AENVRAMIndex);
        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_AWB, m_3A_Index.u4AWBNVRAMIndex);
        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_AF, m_3A_Index.u4AFNVRAMIndex);
        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AE, m_3A_Index.u4FlashAENVRAMIndex);
        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_Calibration, m_3A_Index.u4FlashCaliNVRAMIndex);

        memset(&rMapping_Info.eISO_Idx, 0, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
        Hal3ARawImp::getNvramIndex(rMapping_Info, NSIspTuning::EModule_LTM, m_3A_Index.u4LTMNVRAMIndex);

        AAA_TRACE_END_HAL;

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

        CAM_LOGD_IF(m_3ALogEnable,"[%s] TaskData : Req(#%d)/Stt(#%d)/Flash(%d)/HqCap(%d)/AfTrigger(%d)",
            __FUNCTION__, rData.i4RequestMagic,
            m_i4SttMagicNumber,
            rData.bFlashOnOff,
            rData.bIsHqCap,
            rData.bIsAfTriggerInPrecap);

        /*****************************
         *     Execute task
         *****************************/
         m_pTaskMgr->execute(Task_Update_3A);

        // check capture task is removed or not,
        // if the result is true, this frame is capture end.
        m_bIsCapEnd = isExistCap && !m_pTaskMgr->isExistTask(TASK_ENUM_3A_CAPTURE);

        // reset HQC flag
        if(m_bIsCapEnd && m_bIsHighQualityCaptureOn)
        {
            m_bIsHighQualityCaptureOn = MFALSE;
            CAM_LOGD("[HQC] End : Stt(#%d)", m_i4SttMagicNumber);
        }

        /*****************************
         *     ISP Validate
         *****************************/
        AAA_TRACE_D("P1_VLD");
        AAA_TRACE_ISP(P1_VLD);
#if CAM3_FLASH_FEATURE_EN
        FlashMgr::getInstance(m_i4SensorDev)->updateFlashState();
#endif
        AAA_TRACE_END_ISP;
        AAA_TRACE_END_D;

        /*****************************
         *     Release Statistic
         *****************************/
        AAA_TRACE_HAL(ReleaseStatistic);
        m_pTaskMgr->releaseStt(Task_Update_3A);
        AAA_TRACE_END_HAL;

        if(pParam)
            m_u4LastRequestNumber = pParam->i4MagicNum;
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s]-", __FUNCTION__);
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
            MUINT32 outLen = 0;
            AE_NVRAM_T aeNvram;
            ::memset(&aeNvram, 0, sizeof(aeNvram));
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetNVRAParam, reinterpret_cast<MINTPTR>(&aeNvram), reinterpret_cast<MINTPTR>(&outLen), NULL, NULL);
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
            IPDMgr::getInstance().setAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            m_i4AETargetMode = static_cast<eAETargetMODE>(iArg1);
            break;
        case E3ACtrl_SetAELimiterMode:
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetLimiterMode, iArg1, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetAECamMode:
            IAeMgr::getInstance().setAECamMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_EnableDisableAE:
            if (iArg1) i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_EnableAE, NULL, NULL, NULL, NULL);
            else       i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_DisableAE, NULL, NULL, NULL, NULL);
            break;
        case E3ACtrl_SetMinMaxFps:
            IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, (MUINT32)iArg1, (MUINT32)iArg2);
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
                *(reinterpret_cast<MUINT32*>(iArg1)) = (MUINT32)(AEPerframeInfo.rAEUpdateInfo.u8FrameDuration_ns/1000);
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
            i4Ret = IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_GetAEInitExpSetting, iArg1, iArg2, NULL, NULL);
            a_rAEInitExpoSetting = reinterpret_cast<AEInitExpoSetting_T*>(iArg1);
            CAM_LOGD("[%s()] E3ACtrl_GetInitExpoSetting: u4SensorMode(%d) u4AETargetMode(%d) u4Eposuretime(%d) u4AfeGain(%d) u4Eposuretime_se(%d) u4AfeGain_se(%d)",
                      __FUNCTION__, a_rAEInitExpoSetting->u4SensorMode, a_rAEInitExpoSetting->u4AETargetMode,
                      a_rAEInitExpoSetting->u4Eposuretime, a_rAEInitExpoSetting->u4AfeGain,
                      a_rAEInitExpoSetting->u4Eposuretime_se, a_rAEInitExpoSetting->u4AfeGain_se);
            break;
        case E3ACtrl_SetAwbBypCalibration:
            i4Ret = IAwbMgr::getInstance().CCTOPAWBBypassCalibration(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
            break;

#if CAM3_AF_FEATURE_EN
        // --------------------------------- AF ---------------------------------
//===========================ACDK USE E3ACtrl_SetAFMode=======================================//
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
//            IAfMgr::getInstance(m_i4SensorDev).getDAFtbl( (MVOID**)iArg1);
            break;
//===========================ACDK USE E3ACtrl_SetAFMode=======================================//
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
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(2);
                break;
            case 1: // Auto
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->enableDynamicShading(MTRUE, (MUINT32)m_i4OperMode);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                break;
            case 2: // Low
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(0);
                break;
            case 3: // Mid
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(1);
                break;
            case 4: // High
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->enableDynamicShading(MFALSE, (MUINT32)m_i4OperMode);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(MTRUE);
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setCTIdx(2);
                break;
            case 5: // TSF
            default:
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->enableDynamicShading(MTRUE, (MUINT32)m_i4OperMode);
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
            i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctGetQuickCalibrationResult();
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
            CAM_LOGD_IF(m_3ALogEnable, "setIsFlashOnCapture=%d", (MINT32)iArg1);
            break;
#endif

        // --------------------------------- flow control ---------------------------------
        case E3ACtrl_Enable3ASetParams:
            m_bEnable3ASetParams = iArg1;
            break;
        case E3ACtrl_SetOperMode:
            CAM_LOGD_IF(m_3ALogEnable, "[%s] prev_mode(%d), new_mode(%ld)", __FUNCTION__, m_i4OperMode, (long)iArg1);
            m_i4OperMode = iArg1;
            break;
        case E3ACtrl_GetOperMode:
            *(reinterpret_cast<MUINT32*>(iArg1)) = m_i4OperMode;
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
            rFeatureParam->u4MaxFocusAreaNum = m_rAFStaticInfo.isAfSupport;
            rFeatureParam->bEnableDynamicFrameRate = CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE();
            rFeatureParam->i4MaxLensPos = m_rAFStaticInfo.maxAfTablePos;
            rFeatureParam->i4MinLensPos = m_rAFStaticInfo.minAfTablePos;
            rFeatureParam->i4AFBestPos = m_rAFStaticInfo.lastFocusPos;
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

    m_i4SensorPreviewDelay = sensorStaticInfo.previewDelayFrame;

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), rawFmtType(%d)\n", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, u4RawFmtType);

    // 3A/ISP mgr can query sensor static information here
    IAwbMgr::getInstance().setIsMono(m_i4SensorDev, (u4RawFmtType == SENSOR_RAW_MONO ? MTRUE : MFALSE), u4RawFmtType);

#if CAM3_STEREO_FEATURE_EN
    // For AE/AWB/AF Sync
    if(u4RawFmtType == SENSOR_RAW_MONO && ISync3AMgr::getInstance()->isActive())
    {
        IAwbMgr::getInstance().disableAWB(m_i4SensorDev);
        ISync3AMgr::getInstance()->getSync3A()->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
        ISync3AMgr::getInstance()->getSync3A(ISync3AMgr::E_SYNC3AMGR_CAPTURE)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);

        IAfMgr::getInstance(m_i4SensorDev).setAFMode( MTK_CONTROL_AF_MODE_OFF, AF_CMD_CALLER);
        ISync3AMgr::getInstance()->getSync3A()->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
        ISync3AMgr::getInstance()->getSync3A(ISync3AMgr::E_SYNC3AMGR_CAPTURE)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
    }
#endif
}

MBOOL
Hal3ARawSMVRImp::
notifyP1PwrOn()
{
    CAM_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
    m_pICcuMgr = NSCcuIf::ICcuMgrExt::createInstance("3AHal");
    if(m_pICcuMgr)
    {
        if(m_pICcuMgr->ccuInit() != 0)
        {
            CAM_LOGD("[%s] m_pICcuMgr->ccuInit() fail return false", __FUNCTION__);
            return MFALSE;
        }
        CAM_LOGD("[%s] m_pICcuMgr->ccuInit() success", __FUNCTION__);
        if(m_pICcuMgr->ccuBoot() != 0)
        {
            CAM_LOGD("[%s] m_pICcuMgr->ccuBoot() fail return false", __FUNCTION__);
            return MFALSE;
        }
        CAM_LOGD("[%s] m_pICcuMgr->ccuBoot() success return true", __FUNCTION__);
    }
    if(m_pCcuCtrl3ACtrl == NULL)
         m_pCcuCtrl3ACtrl = ICcuCtrl3ACtrl::createInstance();
    if(m_pCcuCtrl3ACtrl)
    {
        if(m_pCcuCtrl3ACtrl->init(m_i4SensorIdx, (NSIspTuning::ESensorDev_T)m_i4SensorDev) != 0){
            CAM_LOGD("[%s] m_pCcuCtrl3ACtrl->init() fail return false", __FUNCTION__);
            return MFALSE;
        }
    }

    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
notifyP1PwrOff()
{
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
        if(m_pICcuMgr->ccuShutdown() != 0)
        {
            CAM_LOGD("[%s] m_pICcuMgr->ccuShutdown() fail return false", __FUNCTION__);
            return MFALSE;
        }
        CAM_LOGD("[%s] m_pICcuMgr->ccuShutdown() success", __FUNCTION__);
        if(m_pICcuMgr->ccuUninit() != 0)
        {
            CAM_LOGD("[%s] m_pICcuMgr->ccuUninit() fail return false", __FUNCTION__);
            return MFALSE;
        }
        CAM_LOGD("[%s] m_pICcuMgr->ccuUninit() success return true", __FUNCTION__);

        m_pICcuMgr->destroyInstance();
        m_pICcuMgr = NULL;
    }
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
preset(Param_T const &rNewParam)
{
    // SMVR no full CCR by far
    return MTRUE;
}

