/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved..
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

#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <string.h>
#include <array>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <mtkcam/utils/std/Log.h>
#include <IThreadRaw.h>
#include <IThreadRawSMVR.h>
#include <mtkcam/drv/IHalSensor.h>

#include "Hal3ARawSMVR.h"
#include <ae_mgr/ae_mgr.h>
#include <awb_mgr/awb_mgr_if.h>
#include <sensor_mgr/aaa_sensor_mgr.h>
#include <ISync3A.h>
#include <isp_tuning_mgr.h>
#include <isp_mgr.h>
#include <aaa_common_custom.h>
#include <private/aaa_utils.h>

// Task header
#include <task/ITaskMgr.h>
#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <dbg_aaa_param.h>
#include <aaa_hal_sttCtrl.h>
#include <debug/DebugUtil.h>
#include <Hal3APolicy.h>
#if CAM3_AF_FEATURE_EN
#include <dbg_af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr/af_mgr_if.h>
#include <pd_mgr/pd_mgr_if.h>
#include <lens/mcu_drv.h>
#include <af_define.h>
#endif

#if CAM3_FLASH_FEATURE_EN
#include <flash_hal.h>
#include <flash_mgr.h>
#endif

#if CAM3_FLICKER_FEATURE_EN
#include <flicker_hal_if.h>
#endif

#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

#include "private/PDTblGen.h"

#include <android/sensor.h>                     // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener

//Thread Use
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>


//CCU
#define AE_MGR_INCLUDING_CCU

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define GET_PROP(prop, init, val)\
{\
    val = property_get_int32(prop, (init));\
}

// define g/gyro info
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

// g/gyro sensor listener handler and data
static MINT32  gAcceInfo[3];
static MUINT64 gAcceTS;
static MBOOL   gAcceValid = MFALSE;
static MINT32  gGyroInfo[3];
static MUINT64 gGyroTS;
static MBOOL   gGyroValid = MFALSE;
static Mutex& gCommonLock = *new Mutex();
using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

class Hal3ARawSMVRImp : public Hal3ARawSMVR
{
public:
    static I3AWrapper*  getInstance(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount);
    virtual MVOID       destroyInstance();
    virtual MBOOL       start();
    virtual MBOOL       stop();
    virtual MVOID       pause(){}
    virtual MVOID       resume(MINT32){}
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);
    virtual MBOOL       generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2);
    virtual MBOOL       validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario);
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam);
    virtual MBOOL       autoFocus();
    virtual MBOOL       cancelAutoFocus();
    virtual MVOID       setFDEnable(MBOOL fgEnable);
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT);
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/, MINT32 i4P1DoneSttNum = -1);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond() const;
    virtual MBOOL       chkPreFlashOnCond() const;
    virtual MBOOL       isStrobeBVTrigger() const;
    virtual MBOOL       isFlashOnCapture() const;
    virtual MBOOL       chkCapFlash() const {return m_bIsHighQualityCaptureOn;}
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, MINT32 i4SubsampleIndex = 0) const;
    virtual MINT32      getCurrentHwId() const;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    virtual MINT32      queryMagicNumber() const {return m_i4SttMagicNumber;}
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    virtual MBOOL       notifyPwrOn();
    virtual MBOOL       notifyPwrOff();
    virtual MBOOL       notifyP1PwrOn(); //Open CCU power.
    virtual MBOOL       notifyP1PwrOff(); //Close CCU power.
    virtual MVOID       querySensorStaticInfo();
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum);
    virtual MBOOL       dumpP1Params(MINT32 /*i4MagicNum*/){return MTRUE;}
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type);
    virtual MBOOL       preset(Param_T const &rNewParam);
    virtual MBOOL       notifyResult4TG(MINT32 /*i4PreFrmId*/) {return MTRUE;}
    virtual MBOOL       notify4CCU(MUINT32 /*u4PreFrmId*/, ISP_NVRAM_OBC_T const &/*OBCResult*/){return MTRUE;}
    virtual MVOID       notifyPreStop();

public:  //    Ctor/Dtor.
                        Hal3ARawSMVRImp(MINT32 const i4SensorIdx);
    virtual             ~Hal3ARawSMVRImp(){}

protected:
                        Hal3ARawSMVRImp(const Hal3ARawSMVRImp&);
                        Hal3ARawSMVRImp& operator=(const Hal3ARawSMVRImp&);

    MBOOL               init(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount);
    MBOOL               uninit();
    MRESULT             updateTGInfo();
    MINT32              config(const ConfigInfo_T& rConfigInfo);
    MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;

private:
    MINT32              m_3ALogEnable;
    MINT32              m_DebugLogWEn;
    volatile int        m_Users;
    mutable Mutex       m_Lock;
    Mutex               mP2Mtx;
    Mutex               m3AOperMtx1;
    Mutex               m3AOperMtx2;
    MINT32              m_i4SensorIdx;
    MINT32              m_i4SensorDev;
    MUINT32             m_u4SensorMode;
    MUINT               m_u4TgInfo;
    MBOOL               m_bEnable3ASetParams;
    MBOOL               m_bFaceDetectEnable;
    MINT32              m_i4TgWidth;
    MINT32              m_i4TgHeight;
    MBOOL               mbIsHDRShot;
    MINT32              m_i4HbinWidth;
    MINT32              m_i4HbinHeight;
    MBOOL               m_fgEnableShadingMeta;
    MINT32              m_i4SubsampleCount;
    MBOOL               m_bIsRecordingFlash;
    MINT32              m_i4HWSuppportFlash;
    MINT32              m_i4EVCap;
    MINT32              m_i4SensorPreviewDelay;
    MINT32              m_i4AETargetMode = AE_MODE_NORMAL;
    MBOOL               m_bPreStop;
    MINT32              m_u4LastRequestNumber;
private:
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IspTuningMgr*       m_pTuning;
    IThreadRaw*         m_pThreadRaw;
    ITaskMgr*           m_pTaskMgr;
    Param_T             m_rParam;
    AF_Param_T          m_rAfParam;
    ScenarioParam       m_sParam;
    I3ACallBack*        m_pCbSet;
    MINT32              m_i4SttMagicNumber;
    MBOOL               m_bIsHighQualityCaptureOn;
    SMVR_GAIN_DELAY_T   m_rGainDelay;
    IResultPool*        mpResultPoolObj;
    SLOW_MOTION_3A_INFO_T m_rNew3AInfo;
    MBOOL               m_bIsCapEnd;
    MBOOL               m_bIsFlashOpened;
    MBOOL               m_bDbgInfoEnable;
};

#define MY_INST NS3Av3::INST_T<Hal3ARawSMVRImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

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
    if(i4SensorOpenIndex >= SENSOR_IDX_MAX || i4SensorOpenIndex < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorOpenIndex);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3ARawSMVRImp>(i4SensorOpenIndex);
    } );
    (rSingleton.instance)->init(i4SensorOpenIndex, i4SubsampleCount);

    return rSingleton.instance.get();
}

MVOID
Hal3ARawSMVRImp::
destroyInstance()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    uninit();
}

Hal3ARawSMVRImp::
Hal3ARawSMVRImp(MINT32 const i4SensorIdx)
    : m_3ALogEnable(0)
    , m_DebugLogWEn(0)
    , m_Users(0)
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorDev(0)
    , m_u4SensorMode(0)
    , m_u4TgInfo(0)
    , m_bEnable3ASetParams(MTRUE)
    , m_bFaceDetectEnable(MFALSE)
    , m_i4TgWidth(1000)
    , m_i4TgHeight(1000)
    , mbIsHDRShot(MFALSE)
    , m_i4HbinWidth (1000)
    , m_i4HbinHeight(1000)
    , m_fgEnableShadingMeta(MTRUE)
    , m_i4SubsampleCount(1)
    , m_bIsRecordingFlash(MFALSE)
    , m_i4HWSuppportFlash(0)
    , m_i4EVCap(0)
    , m_i4SensorPreviewDelay(0)
    , m_bPreStop(MFALSE)
    , m_u4LastRequestNumber(0)
    , m_pCamIO(NULL)
    , m_pTuning(NULL)
    , m_pThreadRaw(NULL)
    , m_pTaskMgr(NULL)
    , m_pCbSet(NULL)
    , m_i4SttMagicNumber(0)
    , m_bIsHighQualityCaptureOn(MFALSE)
    , m_rGainDelay()
    , mpResultPoolObj(NULL)
    , m_bIsCapEnd(MFALSE)
    , m_bIsFlashOpened(MFALSE)
    , m_bDbgInfoEnable(MFALSE)
{
    m_i4SensorDev = NS3Av3::mapSensorIdxToDev(i4SensorIdx);
    CAM_LOGD("[%s] sensorIdx(%d), sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);
}

MBOOL
Hal3ARawSMVRImp::
init(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    MBOOL dbgInfoEnable;
#if (IS_BUILD_USER)
    dbgInfoEnable = 0;
#else
    dbgInfoEnable = 1;
#endif

    GET_PROP("vendor.debug.camera.dbginfo", dbgInfoEnable, m_bDbgInfoEnable);

    GET_PROP("vendor.debug.camera.log", 0, m_3ALogEnable);
    if ( m_3ALogEnable == 0 ) {
        GET_PROP("vendor.debug.camera.log.hal3a", 0, m_3ALogEnable);
    }
    m_DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // check user count
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_i4SensorIdx = i4SensorOpenIndex;
    m_i4SubsampleCount = i4SubsampleCount;
    m_fgEnableShadingMeta = MTRUE;

    // init Thread and state mgr
    m_pThreadRaw = IThreadRawSMVR::createInstance((Hal3ARaw*)this, m_i4SensorDev, m_i4SensorIdx, m_i4SubsampleCount);
    m_pTaskMgr = ITaskMgr::create(m_i4SensorDev);


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
    IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_SetSMVRMode, MTRUE, NULL);
    CAM_TRACE_END();
#endif

    //FLASH init
#if CAM3_FLASH_FEATURE_EN
    CAM_TRACE_BEGIN("FLASH init");
    //FLASH HAL init
    FlashHal::getInstance(m_i4SensorDev)->init();

    //FLASH init
    FlashMgr::getInstance(m_i4SensorDev)->init();
    CAM_LOGD("[%s] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    CAM_TRACE_END();
#endif

    //FLICKER init
#if CAM3_FLICKER_FEATURE_EN
    CAM_TRACE_BEGIN("FLICKER init");
    IFlickerHal::getInstance(m_i4SensorDev)->init(m_i4SensorIdx);
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
    querySensorStaticInfo();
    CAM_TRACE_END();

    // state mgr transit to Init state.
    //bRet = postCommand(ECmd_Init);
    //if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");

    // GyroSensor init
    //enableGyroSensor(MTRUE);

    CAM_LOGD("[%s] done\n", __FUNCTION__);
    android_atomic_inc(&m_Users);
    return S_3A_OK;
}

MBOOL
Hal3ARawSMVRImp::
uninit()
{

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
        //bRet = postCommand(ECmd_Uninit);
        //if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");
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

        // TaskMgr uninit
        m_pTaskMgr->destroy();
        m_pTaskMgr = NULL;

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
    IFlickerHal::getInstance(m_i4SensorDev)->setSensorMode(m_u4SensorMode, u4AAWidth, u4AAHight);
#endif


    // FIXME (remove): update TG Info to 3A modules
    updateTGInfo();

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
        CAM_TRACE_END();
        return err;
    }
//    IAeMgr::getInstance().setAAOMode(m_i4SensorDev, bAAOMode);
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
    // Flash start
    CAM_TRACE_BEGIN("FLASH Start");
    FlashMgr::getInstance(m_i4SensorDev)->setBitMode(rConfigInfo.i4BitMode);
    err = FlashMgr::getInstance(m_i4SensorDev)->start();
    if (err) {
        CAM_LOGE("FlashMgr::getInstance().start() fail\n");
        CAM_TRACE_END();
        return E_3A_ERR;
    }
    FlashHal::getInstance(m_i4SensorDev)->setInCharge(MTRUE);
    FlashMgr::getInstance(m_i4SensorDev)->setInCharge(MTRUE);
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
    MBOOL enable_flk = IFlickerHal::getInstance(m_i4SensorDev)->getInfo();
    CAM_LOGD("[%s] enable_flk=%d", __FUNCTION__, (int)enable_flk);
#if CAM3_FLICKER_FEATURE_EN
    // Flicker start
    CAM_TRACE_BEGIN("FLICKER Start");
    err = IFlickerHal::getInstance(m_i4SensorDev)->config();
    CAM_TRACE_END();
    if (FAILED(err)) {
        CAM_LOGE("IFlickerHal::getInstance().config() fail\n");
        return err;
    }
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
    m_i4SttMagicNumber = 0;
    m_i4SubsampleCount = 1;
    m_bIsFlashOpened = MFALSE;

    // Clear Task3a
    m_pTaskMgr->clearTaskQueue(Task_Update_3A);

    // AE stop
    CAM_TRACE_BEGIN("AE Stop");
    IAeMgr::getInstance().setAESMBuffermode(m_i4SensorDev, MFALSE, m_i4SubsampleCount);
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, 5000, 30000);
    CAM_LOGD("[%s] AE Min Max Frame Rate has been reset", __FUNCTION__);
    err = IAeMgr::getInstance().Stop(m_i4SensorDev);
    CAM_TRACE_END();
    if (FAILED(err)) {
    CAM_LOGE("IAeMgr::getInstance().Stop() fail\n");
        return err;
    }
    m_pThreadRaw->destroyEventThread();

    // AWB stop
    CAM_TRACE_BEGIN("AWB Stop");
    err = IAwbMgr::getInstance().stop(m_i4SensorDev);
    CAM_TRACE_END();
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().stop() fail\n");
        return E_3A_ERR;
    }

#if CAM3_FLASH_FEATURE_EN
    // Flash stop
    CAM_TRACE_BEGIN("Flash Stop");
    err = FlashMgr::getInstance(m_i4SensorDev)->stop();
    if (err) {
        CAM_LOGE("FlashMgr::getInstance().stop() fail\n");
        CAM_TRACE_END();
        return E_3A_ERR;
    }
    // Flash Hal close flashlight
    FlashHal::getInstance(m_i4SensorDev)->setFlashOff();
    CAM_TRACE_END();
#endif

#if CAM3_AF_FEATURE_EN
    // disable AF thread
    CAM_TRACE_BEGIN("AF THREAD disable");
    m_pThreadRaw->disableAFThread();
    CAM_TRACE_END();

    // AF stop
    CAM_TRACE_BEGIN("AF Stop");
    err = IAfMgr::getInstance().Stop(m_i4SensorDev);
    CAM_TRACE_END();
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Stop() fail\n");
        return err;
    }
#endif

#if CAM3_FLICKER_FEATURE_EN
    // Flicker close
    CAM_TRACE_BEGIN("Flicker stop");
    IFlickerHal::getInstance(m_i4SensorDev)->stop();
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

    ((TuningParam*)pTuningBuf)->pLcsBuf = NULL;

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
        //rDbg3AInfo2.hdr.rCommDebugInfo.rISP.P2Offset = rDbg3AInfo2.rISPDebugInfo.P2RegInfo.u4Offset;
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
    m_pTuning->validate(m_i4SensorDev, rParamIspProfile.rRequestSet, fgPerframe, (MINT32)m_rGainDelay.u4ISPGain_Delay);

    /*ISP_LCS_IN_INFO_T lcs_info;
    m_pTuning->getLCSparam(m_i4SensorDev, lcs_info);
    lcs_info.i4FrmId = rParamIspProfile.i4MagicNum;
    m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_LCS_ISP_PARAMS, rParamIspProfile.i4MagicNum, (MINTPTR)&lcs_info, 0);*/

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

#if CAM3_STEREO_FEATURE_EN
        m_sParam = ScenarioParam( rNewParam.eIspProfile
                                , rNewParam.u1CaptureIntent
                                , rNewParam.u1HdrMode
                                , (ISync3AMgr::getInstance()->getStereoParams()).i4Sync2AMode
                                , m_i4AETargetMode
                                , m_u4SensorMode);
#else
        m_sParam = ScenarioParam( rNewParam.eIspProfile
                                , rNewParam.u1CaptureIntent
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
setAfParams(AF_Param_T const &rNewParam)
{
        //if (!m_bEnable3ASetParams) return MTRUE;
#if CAM3_AF_FEATURE_EN
        // ====================================== AF ==============================================
        IThreadRaw::AFParam_T rAFPAram;
        rAFPAram.i4MagicNum = rNewParam.i4MagicNum;
        rAFPAram.u4AfMode = rNewParam.u4AfMode;
        rAFPAram.rFocusAreas = rNewParam.rFocusAreas;
        if( IspTuningMgr::getInstance().getOperMode(m_i4SensorDev) != NSIspTuning::EOperMode_EM)
            rAFPAram.fFocusDistance = rNewParam.fFocusDistance;
        else
            rAFPAram.fFocusDistance = -1;
        rAFPAram.u1AfTrig  = rNewParam.u1AfTrig;
        rAFPAram.u1PrecapTrig  = rNewParam.u1PrecapTrig;
        rAFPAram.u1AfPause = rNewParam.u1AfPause;
        rAFPAram.bFaceDetectEnable = m_bFaceDetectEnable;
        rAFPAram.rScaleCropArea = rNewParam.rScaleCropArea;
        rAFPAram.bEnable3ASetParams = m_bEnable3ASetParams;
        m_pThreadRaw->sendRequest(ECmd_AFUpdate,(MINTPTR)&rAFPAram);
        m_rAfParam = rNewParam;
#endif
    return MTRUE;
}


MBOOL
Hal3ARawSMVRImp::
autoFocus()
{
#if CAM3_AF_FEATURE_EN
    CAM_LOGD("[%s()] +", __FUNCTION__);
    m_pTaskMgr->sendEvent(ECmd_AFStart);
    CAM_LOGD("[%s()] -", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
cancelAutoFocus()
{
#if CAM3_AF_FEATURE_EN
    CAM_LOGD("[%s()] +", __FUNCTION__);
    m_pTaskMgr->sendEvent(ECmd_AFEnd);
    CAM_LOGD("[%s()] -", __FUNCTION__);
#endif
    return MTRUE;
}

MVOID
Hal3ARawSMVRImp::
setFDEnable(MBOOL fgEnable)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] fgEnable(%d)", __FUNCTION__, fgEnable);
    m_bFaceDetectEnable = fgEnable;
    IAeMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    IAwbMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    IspTuningMgr::getInstance().setFDEnable(m_i4SensorDev, fgEnable);
}

MBOOL
Hal3ARawSMVRImp::
setFDInfo(MVOID* prFaces, MVOID* prAFFaces)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] m_bFaceDetectEnable(%d)", __FUNCTION__, m_bFaceDetectEnable);
    if (m_bFaceDetectEnable)
    {
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance().setFDInfo(m_i4SensorDev, prAFFaces);
#endif
        IAeMgr::getInstance().setFDInfo(m_i4SensorDev, prFaces, m_i4TgWidth, m_i4TgHeight);
        IAwbMgr::getInstance().setFDInfo(m_i4SensorDev, prFaces, m_i4TgWidth, m_i4TgHeight);
        IspTuningMgr::getInstance().setFDInfo(m_i4SensorDev, prAFFaces, (m_i4TgWidth * m_i4TgHeight));
    }
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
setOTInfo(MVOID* prOT, MVOID* prAFOT)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance().setOTInfo(m_i4SensorDev, prAFOT);
#endif
        IAeMgr::getInstance().setOTInfo(m_i4SensorDev, prOT);
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
        IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, MFALSE);

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
setPreFlashOnOff(MBOOL bOnOff)
{
#if CAM3_FLASH_FEATURE_EN
    MY_LOG_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    FlashHal::getInstance(m_i4SensorDev)->setPfOnOff(bOnOff);
    MY_LOG_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
isNeedTurnOnPreFlash() const
{
    //return m_pStateMgr->getPreCapFlashOn();
	return MFALSE;
}

MBOOL
Hal3ARawSMVRImp::
chkMainFlashOnCond() const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
    MBOOL bChkFlash;
    bChkFlash = (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_SINGLE))
        ||
                (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isFlashOnCapture()))
                && (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE));

    return bChkFlash && (m_i4HWSuppportFlash != 0) && (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() != MTK_FLASH_STATE_CHARGING);//shouldn't return MTRUE if Hw does not support flash
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

MBOOL
Hal3ARawSMVRImp::
isStrobeBVTrigger() const
{
    return IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);
}

MBOOL
Hal3ARawSMVRImp::
isFlashOnCapture() const
{
    return FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture();
}

MINT32
Hal3ARawSMVRImp::
getCurrResult(MUINT32 i4FrmId, MINT32 i4SubsampleIndex) const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] + i4MagicNum(%d) i4SubsampleIndex:%d", __FUNCTION__, i4FrmId, i4SubsampleIndex);

    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState = MTK_CONTROL_AWB_STATE_INACTIVE;

    // ResultPool - To update Vector info
    AllResult_T *pAllResult = mpResultPoolObj->getInstance(m_i4SensorDev)->getAllResult(i4FrmId);
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
    ISPResultToMeta_T     rISPResult;
    LSCResultToMeta_T     rLSCResult;
    FLASHResultToMeta_T   rFLASHResult;
    FLKResultToMeta_T     rFLKResult;

    rHALResult.i4FrmId = i4FrmId;
    rHALResult.fgBadPicture = MFALSE;

    //===== Get AE/AWB FD region =====
    MUINT8 u1AeState = IAeMgr::getInstance().getAEState(m_i4SensorDev);
    if (IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev) && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rAEResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rAEResult.u1AeState = u1AeState;

    rAEResult.fgAeBvTrigger = IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);

    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rAEResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rAEResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rAEResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
    if (m_rParam.u1RollingShutterSkew)
        rAEResult.i8SensorRollingShutterSkew = IAeMgr::getInstance().getSensorRollingShutter(m_i4SensorDev);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] update new setting: AeState/Exp/ISO %d/%lld/%d", __FUNCTION__, rAEResult.u1AeState, rAEResult.i8SensorExposureTime, rAEResult.i4SensorSensitivity/*, rAFResult.u1AfState*/);

    *(const_cast<MUINT8*>(&m_rNew3AInfo.u1AeState)) = rAEResult.u1AeState;
    *(const_cast<MINT64*>(&m_rNew3AInfo.i8SensorExposureTime)) = rAEResult.i8SensorExposureTime;
    *(const_cast<MINT32*>(&m_rNew3AInfo.i4SensorSensitivity)) = rAEResult.i4SensorSensitivity;

    //===== Get ISP result =====
    if (!IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rISPResult.rCamInfo))
    {
        CAM_LOGE("Fail to get CamInfo");
    }

    {
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        // clear memory
        pAllResult->vecExifInfo.clear();
        pAllResult->vecTonemapCurveRed.clear();
        pAllResult->vecTonemapCurveGreen.clear();
        pAllResult->vecTonemapCurveBlue.clear();
        pAllResult->vecColorCorrectMat.clear();
        pAllResult->vecDbg3AInfo.clear();
        pAllResult->vecDbgShadTbl.clear();
    }

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
    IAwbMgr::getInstance().getColorCorrectionGain(m_i4SensorDev, rAWBResult.fColorCorrectGain[0],rAWBResult.fColorCorrectGain[1],rAWBResult.fColorCorrectGain[3]);
    rAWBResult.fColorCorrectGain[2] = rAWBResult.fColorCorrectGain[1];
    if (m_rParam.u1ColorCorrectMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        pAllResult->vecColorCorrectMat.resize(9);
        MFLOAT* pfColorCorrectMat = pAllResult->vecColorCorrectMat.editArray();
    IspTuningMgr::getInstance().getColorCorrectionTransform(m_i4SensorDev,
            pfColorCorrectMat[0], pfColorCorrectMat[1], pfColorCorrectMat[2],
            pfColorCorrectMat[3], pfColorCorrectMat[4], pfColorCorrectMat[5],
            pfColorCorrectMat[6], pfColorCorrectMat[7], pfColorCorrectMat[8]
    );
    }

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
#endif

    //===== Get LSC result =====
#if CAM3_LSC_FEATURE_EN
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
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

    //===== Get ISP Tonemap result =====
    if (m_rParam.u1TonemapMode != MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        MFLOAT *pIn, *pOut;
        MINT32 i4NumPt;
        IspTuningMgr::getInstance().getTonemapCurve_Blue(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            pAllResult->vecTonemapCurveBlue.push_back(*pIn++);
            pAllResult->vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            pAllResult->vecTonemapCurveGreen.push_back(*pIn++);
            pAllResult->vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            pAllResult->vecTonemapCurveRed.push_back(*pIn++);
            pAllResult->vecTonemapCurveRed.push_back(*pOut++);
        }
    }


    //===== Get Exif result =====
    if (m_rParam.u1IsGetExif)
    {
        // protect vector before use vector
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        if(pAllResult->vecExifInfo.size()==0)
            pAllResult->vecExifInfo.resize(1);
        get3AEXIFInfo(pAllResult->vecExifInfo.editTop());

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

            if(pAllResult->vecDbgShadTbl.size() != 0 && pAllResult->vecDbgIspInfo.size() != 0 && pAllResult->vecDbg3AInfo.size() != 0 && m_bDbgInfoEnable)
            {
                CAM_LOGD_IF(m_3ALogEnable, "[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)pAllResult->vecDbgShadTbl.size(), (MINT32)pAllResult->vecDbgIspInfo.size(), (MINT32)pAllResult->vecDbg3AInfo.size(),
                                            &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);
                getP1DbgInfo(rDbg3AInfo, rDbgShadTbl, rDbgIspInfo);
            }
            else
                CAM_LOGE("[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)pAllResult->vecDbgShadTbl.size(), (MINT32)pAllResult->vecDbgIspInfo.size(), (MINT32)pAllResult->vecDbg3AInfo.size(),
                                            &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);
        }
    }

    //===== update modules result to ResultPool =====
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_HAL_RESULTTOMETA, &rHALResult);
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_AE_RESULTTOMETA, &rAEResult);
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_AWB_RESULTTOMETA, &rAWBResult);
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_ISP_RESULTTOMETA, &rISPResult);
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_LSC_RESULTTOMETA, &rLSCResult);
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_FLASH_RESULTTOMETA, &rFLASHResult);
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateResult(LOG_TAG, i4FrmId, E_FLK_RESULTTOMETA, &rFLKResult);

    //===== get AF state, and update to ResultPool =====
    // MW Handshake : Early Call Back. ZSD-flash capture,MW need Af state to judge precapture done
    AAA_TRACE_MGR(getEarlyCBState);
    EARLY_CALL_BACK rEarlyCB;
    rEarlyCB.u1AfState = IAfMgr::getInstance().getAFState(m_i4SensorDev);
    rEarlyCB.u1AeState = rAEResult.u1AeState;
    rEarlyCB.u1AeMode = rAEResult.u1AeMode;
    rEarlyCB.u1FlashState = rFLASHResult.u1FlashState;
    rEarlyCB.fgAeBvTrigger = rAEResult.fgAeBvTrigger;

    // CCT 3A Need to overwrite AF state when supportAF
    MINT32 IsSupportAF = IAfMgr::getInstance().isAFSupport(m_i4SensorDev);
    MINT32 IsAFSuspend = IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_IsAFSuspend, NULL, NULL);
    MINT32 i4AFEnable = 0;
    MUINT32 i4OutLens = 0;
    IAfMgr::getInstance().CCTOPAFGetEnableInfo(m_i4SensorDev, (MVOID*)&i4AFEnable, &i4OutLens);
    if( IsSupportAF && (!i4AFEnable) && rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED &&  rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
    {
        CAM_LOGD_IF(1, "[%s] overwrite AF state(%d)", __FUNCTION__, rEarlyCB.u1AfState);
        rEarlyCB.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
    }
    mpResultPoolObj->getInstance(m_i4SensorDev)->updateEarlyCB(i4FrmId, rEarlyCB);
    AAA_TRACE_END_MGR;

    //===== Call Back modules update done, then convert result to metadata =====
    if (m_pCbSet && i4FrmId != 0)
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] #(%d)", __FUNCTION__, i4FrmId);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_UPDATE_RESULTPOOL_DONE,
            i4FrmId,     // magic number
            NULL, NULL);
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s] - i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return MTRUE;
}


MINT32
Hal3ARawSMVRImp::
getCurrentHwId() const
{
    MINT32 idx = 0;
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX,
                        (MINTPTR)&idx, 0, 0);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] idx(%d)", __FUNCTION__, idx);
    return idx;
}

MVOID
Hal3ARawSMVRImp::
setSensorMode(MINT32 i4SensorMode)
{
    CAM_LOGD("[%s] mode(%d)", __FUNCTION__, i4SensorMode);
    m_u4SensorMode = i4SensorMode;
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

        IAeMgr::getInstance().updateSensorListenerParams(m_i4SensorDev, (MINT32*)&rGyroSensorParam);
        IAfMgr::getInstance().updateSensorListenerParams(m_i4SensorDev, (MINT32*)&rGyroSensorParam);
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
            MINT32 i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            MBOOL bAELock = IAeMgr::getInstance().IsAPAELock(m_i4SensorDev);
            MBOOL bIsEVchaged = m_i4EVCap != IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev);

            if (bIsEVchaged){
                m_i4EVCap = IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev);
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
                IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);
                IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, previewInfo);
            }
        }

        // reset flag of HDR shot
        if (mbIsHDRShot)
            mbIsHDRShot = MFALSE;

        // notify AE for single frame HDR
        if (m_rParam.u1IsSingleFrameHDR)
            IAeMgr::getInstance().setzCHDRShot(m_i4SensorDev, MTRUE);

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
        ParamIspProfile_T rParamIsp = *pParam;
        rParamIsp.rRequestSet.fgKeep = m_bIsCapEnd;
        validateP1(rParamIsp, MTRUE);
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

MRESULT
Hal3ARawSMVRImp::
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

    if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
    {
        CAM_LOGE("RAW sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
        return E_3A_ERR;
    }

    m_u4TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2

    IAwbMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);
    IspTuningMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);

    m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&m_i4TgWidth, (MINTPTR)&m_i4TgHeight, 0);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] TG size(%d,%d), i4SensorDevId(%d)", __FUNCTION__, m_i4TgWidth, m_i4TgHeight, i4SensorDevId);
    IFlickerHal::getInstance(m_i4SensorDev)->setTGInfo(m_u4TgInfo, m_i4TgWidth, m_i4TgHeight);

    return S_3A_OK;
}

MBOOL
Hal3ARawSMVRImp::
get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const
{
    AE_DEVICES_INFO_T rDeviceInfo;
    AE_SENSOR_PARAM_T rAESensorInfo;

    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rExifInfo.u4CapExposureTime = rAESensorInfo.u8ExposureTime / 1000;  // naro sec change to micro sec
#if CAM3_FLASH_FEATURE_EN
    if (FlashHal::getInstance(m_i4SensorDev)->isAFLampOn())
        rExifInfo.u4FlashLightTimeus = 30000;
    else
#endif
        rExifInfo.u4FlashLightTimeus = 0;

    IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev, rDeviceInfo);
    rExifInfo.u4FNumber     = rDeviceInfo.u4LensFno; // Format: F2.8 = 28
    rExifInfo.u4FocalLength = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
    //rExifInfo.u4SceneMode   = m_rParam.u4SceneMode; // Scene mode   (SCENE_MODE_XXX)
    switch (IAeMgr::getInstance().getAEMeterMode(m_i4SensorDev))
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
    rExifInfo.i4AEExpBias   = IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev); // Exposure index  (AE_EV_COMP_XX)
    MINT32 i4AEISOSpeedMode = IAeMgr::getInstance().getAEISOSpeedMode(m_i4SensorDev);
    rExifInfo.u4AEISOSpeed  = (i4AEISOSpeedMode ==LIB3A_AE_ISO_SPEED_AUTO) ? rAESensorInfo.u4Sensitivity : i4AEISOSpeedMode;

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
Hal3ARawSMVRImp::
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
    a_rASDInfo.i4AFPos = IAfMgr::getInstance().getAFPos(m_i4SensorDev);
    a_rASDInfo.pAFTable = IAfMgr::getInstance().getAFTable(m_i4SensorDev);
    a_rASDInfo.i4AFTableOffset = IAfMgr::getInstance().getAFTableOffset(m_i4SensorDev);
    a_rASDInfo.i4AFTableMacroIdx = IAfMgr::getInstance().getAFTableMacroIdx(m_i4SensorDev);
    a_rASDInfo.i4AFTableIdxNum = IAfMgr::getInstance().getAFTableIdxNum(m_i4SensorDev);
    a_rASDInfo.bAFStable = IAfMgr::getInstance().getAFStable(m_i4SensorDev);
#endif

    a_rASDInfo.i4AELv_x10 = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
    a_rASDInfo.bAEBacklit = IAeMgr::getInstance().getAECondition(m_i4SensorDev, AE_CONDITION_BACKLIGHT);
    a_rASDInfo.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4SensorDev);
    a_rASDInfo.i2AEFaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4SensorDev);

    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& /*rDbgShadTbl*/, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const
{
    // AE Flash Flicker Shading debug info
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

#if CAM3_FLASH_FEATURE_EN
    // Flash
    FlashMgr::getInstance(m_i4SensorDev)->getDebugInfo(&rDbg3AInfo1.rFlashDebugInfo);
#endif

    // Flicker
    //Flicker::getInstance()->getDebugInfo(&rDbg3AInfo1.rFlickerDebugInfo);

#if CAM3_LSC_FEATURE_EN
    // LSC
    ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
#endif

    // AWB AF ISP debug info
    rDbg3AInfo2.hdr.u4KeyID = ISP_DEBUG_KEYID;
    rDbg3AInfo2.hdr.u4ModuleCount = MODULE_NUM(6,2);

    rDbg3AInfo2.hdr.u4AWBDebugInfoOffset      = sizeof(rDbg3AInfo2.hdr);
    rDbg3AInfo2.hdr.u4ISPDebugInfoOffset      = rDbg3AInfo2.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T);
    rDbg3AInfo2.hdr.u4ISPP1RegDataOffset      = rDbg3AInfo2.hdr.u4ISPDebugInfoOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo);
    rDbg3AInfo2.hdr.u4ISPP2RegDataOffset      = rDbg3AInfo2.hdr.u4ISPP1RegDataOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T::P1RegInfo);
    rDbg3AInfo2.hdr.u4MFBRegInfoOffset        =
    rDbg3AInfo2.hdr.u4AWBDebugDataOffset      = rDbg3AInfo2.hdr.u4ISPDebugInfoOffset + sizeof(NSIspExifDebug::IspExifDebugInfo_T);

    // AWB
    IAwbMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo2.rAWBDebugInfo, rDbg3AInfo2.rAWBDebugData);

    // P1 ISP
    IspTuningMgr::getInstance().getDebugInfoP1(m_i4SensorDev, rDbg3AInfo2.rISPDebugInfo, MFALSE);
    //rDbg3AInfo2.hdr.rCommDebugInfo.rISP.P1Offset = rDbg3AInfo2.rISPDebugInfo.P1RegInfo.u4Offset;

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
            IPDMgr::getInstance().doSWPDE(m_i4SensorDev, (MVOID*)iArg1, (MVOID*)iArg2);
            break;
        case E3ACtrl_GetOISPos:
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
            i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_GetOperMode:
            i4Ret = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
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

VOID
Hal3ARawSMVRImp::
queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight)
{
    i4TgWidth = m_i4TgWidth;
    i4TgHeight = m_i4TgHeight;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3ARawSMVRImp::
attachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    m_pTaskMgr->attachCb(cb);
    return 0;
}

MINT32
Hal3ARawSMVRImp::
detachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pTaskMgr->detachCb(cb);
    m_pCbSet = NULL;
    return 0;

}

VOID
Hal3ARawSMVRImp::
queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight)
{

    i4HbinWidth = m_i4HbinWidth;
    i4HbinHeight = m_i4HbinHeight;

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

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE, u4RawFmtType, 0);

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

MBOOL
Hal3ARawSMVRImp::
notifyPwrOn()
{
    IAfMgr::getInstance().CamPwrOnState(m_i4SensorDev);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
notifyPwrOff()
{
    IAfMgr::getInstance().CamPwrOffState(m_i4SensorDev);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
notifyP1PwrOn()
{
    CAM_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
notifyP1PwrOff()
{
    CAM_LOGD("[%s] notifyP1PwrOff", __FUNCTION__);
    return MTRUE;
}


MVOID
Hal3ARawSMVRImp::
setIspSensorInfo2AF(MINT32 MagicNum)
{
    ISP_SENSOR_INFO_T rIspSensorInfo;
    const ISPResultToMeta_T *pISPResult = NULL;
    pISPResult = (const ISPResultToMeta_T*)mpResultPoolObj->getInstance(m_i4SensorDev)->getResult(MagicNum, E_ISP_RESULTTOMETA);

    if(pISPResult == NULL)
        CAM_LOGW("[%s] pISPResult NULL", __FUNCTION__);
    else
    {
        rIspSensorInfo.i4FrameId = MagicNum;
        rIspSensorInfo.bHLREnable = MFALSE;
        rIspSensorInfo.bAEStable = pISPResult->rCamInfo.rAEInfo.bAEStable;
        rIspSensorInfo.bAELock = pISPResult->rCamInfo.rAEInfo.bAELock;
        rIspSensorInfo.i4deltaIndex = pISPResult->rCamInfo.rAEInfo.i4deltaIndex;
        rIspSensorInfo.u4AfeGain = pISPResult->rCamInfo.rAEInfo.u4AfeGain;
        rIspSensorInfo.u4IspGain = pISPResult->rCamInfo.rAEInfo.u4IspGain;
        rIspSensorInfo.u4Eposuretime = pISPResult->rCamInfo.rAEInfo.u4Eposuretime;
        rIspSensorInfo.u4RealISOValue = pISPResult->rCamInfo.rAEInfo.u4RealISOValue;
        rIspSensorInfo.u4MaxISO = pISPResult->rCamInfo.rAEInfo.u4MaxISO;
        rIspSensorInfo.u4OrgExposuretime = pISPResult->rCamInfo.rAEInfo.u4OrgExposuretime;
        rIspSensorInfo.u4OrgRealISOValue = pISPResult->rCamInfo.rAEInfo.u4OrgRealISOValue;
        rIspSensorInfo.u4AEStableCnt = pISPResult->rCamInfo.rAEInfo.u4AEStableCnt;
        rIspSensorInfo.u4AEFinerEVIdxBase = pISPResult->rCamInfo.rAEInfo.u4AEFinerEVIdxBase;
        rIspSensorInfo.u4AEidxCurrentF = pISPResult->rCamInfo.rAEInfo.u4AEidxCurrentF;
        IAfMgr::getInstance().setIspSensorInfo2AF(m_i4SensorDev, rIspSensorInfo);
    }
    CAM_LOGD_IF(m_3ALogEnable,"[%s] FrameId:%d AFEGain:%d ISPGain:%d", __FUNCTION__,rIspSensorInfo.i4FrameId, rIspSensorInfo.u4AfeGain, rIspSensorInfo.u4IspGain);
}

MVOID
Hal3ARawSMVRImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_bPreStop(%d)", __FUNCTION__, m_bPreStop);
    m_bPreStop = MTRUE;
    m_pThreadRaw->notifyPreStop();
}

MBOOL
Hal3ARawSMVRImp::
setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
    //ISP
    IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewP2Param.i4BrightnessMode);
    IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewP2Param.i4HueMode);
    IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewP2Param.i4SaturationMode);
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewP2Param.i4halEdgeMode);
    IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewP2Param.i4ContrastMode);

    IspTuningMgr::getInstance().setToneMapMode(m_i4SensorDev, rNewP2Param.u1TonemapMode);
    if (rNewP2Param.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        android::Vector<MFLOAT> vecIn, vecOut;
        MINT32 i4Cnt = rNewP2Param.u4TonemapCurveRedSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        MFLOAT* pArrayIn = vecIn.editArray();
        MFLOAT* pArrayOut = vecOut.editArray();
        const MFLOAT* pCurve = rNewP2Param.pTonemapCurveRed;
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveRed.push_back(x);
            pResultP2->vecTonemapCurveRed.push_back(y);
            CAM_LOGD_IF(m_3ALogEnable & 0x2, "[Red]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Red(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);

        i4Cnt = rNewP2Param.u4TonemapCurveGreenSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = vecIn.editArray();
        pArrayOut = vecOut.editArray();
        pCurve = rNewP2Param.pTonemapCurveGreen;
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveGreen.push_back(x);
            pResultP2->vecTonemapCurveGreen.push_back(y);
            CAM_LOGD_IF(m_3ALogEnable & 0x2, "[Green]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Green(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);

        i4Cnt = rNewP2Param.u4TonemapCurveBlueSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = vecIn.editArray();
        pArrayOut = vecOut.editArray();
        pCurve = rNewP2Param.pTonemapCurveBlue;
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveBlue.push_back(x);
            pResultP2->vecTonemapCurveBlue.push_back(y);
            CAM_LOGD_IF(m_3ALogEnable & 0x2, "[Blue]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Blue(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);
    }
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
    if (rNewP2Param.u1TonemapMode != MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        // Tonemap
        pResultP2->vecTonemapCurveRed.clear();
        pResultP2->vecTonemapCurveGreen.clear();
        pResultP2->vecTonemapCurveBlue.clear();

        MINT32 i = 0;
        MFLOAT *pIn, *pOut;
        MINT32 i4NumPt;
        IspTuningMgr::getInstance().getTonemapCurve_Blue(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_3ALogEnable & 0x2, "[Blue](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveBlue.push_back(*pIn++);
            pResultP2->vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_3ALogEnable & 0x2, "[Green](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveGreen.push_back(*pIn++);
            pResultP2->vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_3ALogEnable & 0x2, "[Red](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveRed.push_back(*pIn++);
            pResultP2->vecTonemapCurveRed.push_back(*pOut++);
}
        CAM_LOGD_IF(m_3ALogEnable,"[%s] rsize(%d) gsize(%d) bsize(%d)"
            ,__FUNCTION__, (MINT32)pResultP2->vecTonemapCurveRed.size(), (MINT32)pResultP2->vecTonemapCurveGreen.size(), (MINT32)pResultP2->vecTonemapCurveBlue.size());

    }
    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type)
{
    // type == 0 would do the all set
    if (type < 1) {
        rIspInfo.rCamInfo.bBypassLCE                        = rNewP2Param.bBypassLCE;
        rIspInfo.rCamInfo.i4P2InImgFmt                      = rNewP2Param.i4P2InImgFmt;
        rIspInfo.rCamInfo.bBypassNR                         = rNewP2Param.bBypassNR;

        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Bright    = static_cast<EIndex_Isp_Brightness_T>(rNewP2Param.i4BrightnessMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Contrast  = static_cast<EIndex_Isp_Contrast_T>(rNewP2Param.i4ContrastMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Hue       = static_cast<EIndex_Isp_Hue_T>(rNewP2Param.i4HueMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Sat       = static_cast<EIndex_Isp_Saturation_T>(rNewP2Param.i4SaturationMode);
        rIspInfo.rCamInfo.eEdgeMode                         = static_cast<mtk_camera_metadata_enum_android_edge_mode_t>(rNewP2Param.i4halEdgeMode);
        rIspInfo.rCamInfo.eToneMapMode                      = static_cast<mtk_camera_metadata_enum_android_tonemap_mode_t>(rNewP2Param.u1TonemapMode);
    }

    if (type < 2) {
        //     _reprocess  part

        EIspProfile_T       *rIspProfile=&(rIspInfo.rCamInfo.eIspProfile);

        rIspInfo.rCamInfo.fgRPGEnable = !rNewP2Param.u1PGN;

        if (rNewP2Param.u1IspProfile != 255)
        {
            *rIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP2Param.u1IspProfile);
        }
        else
        {
            switch (rNewP2Param.u1CapIntent)
            {
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                *rIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                *rIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
                if(rIspInfo.rCamInfo.fgRPGEnable){
                    *rIspProfile = NSIspTuning::EIspProfile_Preview;
                }else{
                    *rIspProfile = NSIspTuning::EIspProfile_Capture;
                }
                break;
            }
        }

        rIspInfo.rCamInfo.i4RawType = rNewP2Param.i4RawType;

        if ((!rIspInfo.rCamInfo.fgRPGEnable) && (rNewP2Param.i4RawType == NSIspTuning::ERawType_Pure) && rNewP2Param.rpLscData)
        {
            rIspInfo.rLscData = std::vector<MUINT8>((rNewP2Param.rpLscData)->array(), (rNewP2Param.rpLscData)->array()+(rNewP2Param.rpLscData)->size());
        }

        rIspInfo.i4UniqueKey = rNewP2Param.i4UniqueKey;

        if (rNewP2Param.rpP1Crop && rNewP2Param.rpRzSize && rNewP2Param.rpRzInSize &&
            (rIspInfo.rCamInfo.fgRPGEnable))  //||
//                 rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise  ||
//                 rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise_toGGM))
        {
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstX    = rNewP2Param.rpP1Crop->p.x;
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstY    = rNewP2Param.rpP1Crop->p.y;
            rIspInfo.rCamInfo.rCropRzInfo.i4Width    = rNewP2Param.rpP1Crop->s.w;
            rIspInfo.rCamInfo.rCropRzInfo.i4Height    = rNewP2Param.rpP1Crop->s.h;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzWidth     = rNewP2Param.rpRzSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzHeight     = rNewP2Param.rpRzSize->h;
            rIspInfo.rCamInfo.rCropRzInfo.fgOnOff    = MTRUE;


            rIspInfo.rCamInfo.rCropRzInfo.i4RRZinW      = rNewP2Param.rpRzInSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.i4RRZinH      = rNewP2Param.rpRzInSize->h;

            if((rIspInfo.rCamInfo.rCropRzInfo.i4RRZinW != rIspInfo.rCamInfo.rCropRzInfo.i4FullW) ||
               (rIspInfo.rCamInfo.rCropRzInfo.i4RRZinH != rIspInfo.rCamInfo.rCropRzInfo.i4FullH))
            {
                rIspInfo.rCamInfo.rCropRzInfo.fgFBinOnOff   = MTRUE;
            }
            else
            {
                rIspInfo.rCamInfo.rCropRzInfo.fgFBinOnOff   = MFALSE;
            }
        }
        else
        {
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstX    = 0;
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstY    = 0;
            rIspInfo.rCamInfo.rCropRzInfo.i4Width    = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rCamInfo.rCropRzInfo.i4Height    = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzWidth     = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzHeight     = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rCamInfo.rCropRzInfo.fgOnOff    = MFALSE;

            rIspInfo.rCamInfo.rCropRzInfo.i4RRZinW      = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rCamInfo.rCropRzInfo.i4RRZinH      = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rCamInfo.rCropRzInfo.fgFBinOnOff   = MFALSE;
        }

        if(rNewP2Param.i4P2InImgFmt ==1){

            MUINT32 ResizeYUV_W = rNewP2Param.ResizeYUV & 0x0000FFFF;
            MUINT32 ResizeYUV_H = rNewP2Param.ResizeYUV >> 16;

            if( ResizeYUV_W != 0 && ResizeYUV_H !=0){
                rIspInfo.rCamInfo.rCropRzInfo.i4RzWidth     = ResizeYUV_W;
                rIspInfo.rCamInfo.rCropRzInfo.i4RzHeight     = ResizeYUV_H;
                rIspInfo.rCamInfo.rCropRzInfo.fgOnOff    = MTRUE;
            }
        }

#if 1
        // CRZ temporarily disable, so rIspP2CropInfo align RRZ info rCropRzInfo
        rIspInfo.rIspP2CropInfo = rIspInfo.rCamInfo.rCropRzInfo;

#else
        if (rNewP2Param.rpP2OriginSize && rNewP2Param.rpP2Crop && rNewP2Param.rpP2RzSize)
        {
            rIspInfo.rIspP2CropInfo.i4FullW     = rNewP2Param.rpP2OriginSize->w;
            rIspInfo.rIspP2CropInfo.i4FullH     = rNewP2Param.rpP2OriginSize->h;
            rIspInfo.rIspP2CropInfo.i4OfstX     = rNewP2Param.rpP2Crop->p.x;
            rIspInfo.rIspP2CropInfo.i4OfstY     = rNewP2Param.rpP2Crop->p.y;
            rIspInfo.rIspP2CropInfo.i4Width     = rNewP2Param.rpP2Crop->s.w;
            rIspInfo.rIspP2CropInfo.i4Height    = rNewP2Param.rpP2Crop->s.h;
            rIspInfo.rIspP2CropInfo.i4RzWidth   = rNewP2Param.rpP2RzSize->w;
            rIspInfo.rIspP2CropInfo.i4RzHeight  = rNewP2Param.rpP2RzSize->h;
            rIspInfo.rIspP2CropInfo.fgOnOff     = MTRUE;
        }
        else
        {
            rIspInfo.rIspP2CropInfo.i4FullW     = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rIspP2CropInfo.i4FullH     = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rIspP2CropInfo.i4OfstX     = 0;
            rIspInfo.rIspP2CropInfo.i4OfstY     = 0;
            rIspInfo.rIspP2CropInfo.i4Width     = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rIspP2CropInfo.i4Height    = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rIspP2CropInfo.i4RzWidth   = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rIspP2CropInfo.i4RzHeight  = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rIspP2CropInfo.fgOnOff     = MFALSE;
        }
#endif


        rIspInfo.rCamInfo.eEdgeMode = static_cast<mtk_camera_metadata_enum_android_edge_mode_t>(rNewP2Param.u1appEdgeMode);
        rIspInfo.rCamInfo.eNRMode = static_cast<mtk_camera_metadata_enum_android_noise_reduction_mode_t>(rNewP2Param.u1NrMode);


        if (*rIspProfile == NSIspTuning::EIspProfile_YUV_Reprocess)
        {
            rIspInfo.rCamInfo.rAEInfo.u4RealISOValue= rNewP2Param.i4ISO;
            rIspInfo.rCamInfo.eIdx_Scene = static_cast<NSIspTuning::EIndex_Scene_T>(0);  //MTK_CONTROL_SCENE_MODE_DISABLED
            rIspInfo.rCamInfo.eSensorMode = NSIspTuning::ESensorMode_Capture;
        }
    }

    return MTRUE;
}

MBOOL
Hal3ARawSMVRImp::
preset(Param_T const& /*rNewParam*/)
{
    // SMVR no full CCR by far
    return MTRUE;
}

