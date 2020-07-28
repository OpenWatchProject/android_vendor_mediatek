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

#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <string.h>
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <mtkcam/utils/std/Log.h>
#include <IThreadRaw.h>
#include <mtkcam/drv/IHalSensor.h>

#include "Hal3ARaw.h"
#include <ae_mgr/ae_mgr.h>
#include <awb_mgr/awb_mgr_if.h>
#include <sensor_mgr/aaa_sensor_mgr.h>
#include <sensor_mgr/aaa_sensor_buf_mgr.h>
#include <isp_tuning/isp_tuning_mgr.h>
#include <isp_mgr/isp_mgr.h>
#include <state_mgr/aaa_state_mgr.h>
#include <IThread.h>
#include <atomic>

#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <dbg_aaa_param.h>
#include <sensor_mgr/aaa_sensor_mgr.h>
#include <ISync3A.h>
#include <aaa_hal_sttCtrl.h>
#include <debug/DebugUtil.h>
#include <aaa_common_custom.h>
#include <vector>
#if CAM3_AF_FEATURE_EN
#include <state_mgr_af/af_state_mgr.h>
#include <dbg_af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr/af_mgr_if.h>
#include <mtkcam/aaa/drv/mcu_drv.h>
#endif

#if CAM3_FLASH_FEATURE_EN
#include <flash_mgr/flash_mgr.h>
#endif

#if CAM3_FLICKER_FEATURE_EN
#include <flicker_hal.h>
#endif

#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

#include "cct_server.h"
#include "cctsvr_entry.h"

//CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
#include <list>
#include <drv/cq_tuning_mgr.h>
#endif

#include <android/sensor.h>                     // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener

#include "iccu_mgr.h"
using namespace NSCcuIf;

//Thread Use
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include "SttBufQ.h"

// DP5.0 check sum
#include "aaa_exif_tag_chksum.h"

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
//static MUINT64 gPreAcceTS;  /* fix build warning */
static MINT32  gGyroInfo[3];
static MUINT64 gGyroTS;
static MBOOL   gGyroValid = MFALSE;
//static MUINT64 gPreGyroTS;  /* fix build warning */
//static SensorListener* gpSensorListener=NULL;  /* fix build warning */
static Mutex gCommonLock;

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuningv3;
using namespace NSIspExifDebug;

class Hal3ARawImp : public Hal3ARaw
{
public:
    static I3AWrapper*  createInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID       destroyInstance();
    virtual MBOOL       start();
    virtual MBOOL       stop();
    virtual MVOID       pause();
    virtual MVOID       resume(MINT32 MagicNum = 0);
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);
    virtual MBOOL       generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2);
    virtual MBOOL       validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario);
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam);
    virtual MBOOL       setStereoParams(Stereo_Param_T const &rNewParam);
    virtual MBOOL       autoFocus();
    virtual MBOOL       cancelAutoFocus();
    virtual MVOID       setFDEnable(MBOOL fgEnable);
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT);
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond() const;
    virtual MBOOL       chkPreFlashOnCond() const;
    virtual MBOOL       isStrobeBVTrigger() const;
    virtual MBOOL       isFlashOnCapture() const;
    virtual MBOOL       chkCapFlash() const {return m_bIsHighQualityCaptureOn;}
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, Result_T& rResult, MINT32 i4SubsampleCount = 0) const;
    virtual MINT32      getCurrentHwId() const;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    virtual MUINT32     queryMagicNumber() const {return m_u4MagicNumber;}
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    virtual MBOOL       notifyPwrOn();
    virtual MBOOL       notifyPwrOff();
    virtual MBOOL       notifyP1PwrOn(); //Open CCU power.
    virtual MBOOL       notifyP1PwrOff(); //Close CCU power.
    virtual MBOOL       dumpP1Params(MUINT32 u4MagicNum);
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MVOID       querySensorStaticInfo();
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum, Result_T& Result);
    virtual MBOOL       notifyResult4TG(MINT32 i4PreFrmId, Result_T *rPreResult);
    virtual MBOOL       notify4CCU(MUINT32 u4PreFrmId, Result_T *rPreResult);
    virtual MVOID       notifyPreStop();
    virtual MVOID       updateAFinfo(AF2AEInfo_T& AF2AEinfo);
    virtual AF2AEInfo_T queryAFinfo(const MINT32& sttMagicNum);
    static  MVOID*      ThreadAEStart(MVOID*);
    static  MVOID*      ThreadAFStart(MVOID*);
    static  MVOID*      ThreadIspSetBpciTable(MVOID*);

protected:  //    Ctor/Dtor.
                        Hal3ARawImp(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex);
    virtual             ~Hal3ARawImp(){}

                        Hal3ARawImp(const Hal3ARawImp&);
                        Hal3ARawImp& operator=(const Hal3ARawImp&);

    MBOOL               init();
    MBOOL               uninit();
    MRESULT             updateTGInfo();
    MINT32              config(const ConfigInfo_T& rConfigInfo);
    MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;
    MBOOL               setP1DbgInfo4TG(NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo, MUINT32 u4Rto, MINT32 OBCGain) const;
    MBOOL               setTG_INTParams();

private:
    MINT32              m_3ALogEnable;
    MINT32              m_3ACctEnable;
    MINT32              m_DebugLogWEn;
    volatile int        m_Users;
    mutable Mutex       m_Lock;
    Mutex               mP2Mtx;
    Mutex               m3AOperMtx1;
    Mutex               m3AOperMtx2;
    MINT32              m_i4SensorIdx;
    MINT32              m_i4SensorDev;
    MUINT32             m_u4SensorMode;
    MBOOL               m_bAAOMode;
    MUINT               m_u4TgInfo;
    MBOOL               m_bEnable3ASetParams;
    MBOOL               m_bFaceDetectEnable;
    MINT32              m_i4InCaptureProcess;
    MINT32              m_i4TgWidth;
    MINT32              m_i4TgHeight;
    MBOOL               mbIsHDRShot;
    MINT32              m_i4HbinWidth;
    MINT32              m_i4HbinHeight;
    MBOOL               m_fgEnableShadingMeta;
    MBOOL               m_bIsRecordingFlash;
    MINT32              m_i4HWSuppportFlash;
    MINT32              m_i4EVCap;
    MBOOL               m_bIsAFTrigInPrecapState;
    MINT32              m_i4SensorPreviewDelay;
    MINT32              m_i4AeShutDelayFrame;
    MINT32              m_i4AeISPGainDelayFrame;
    MBOOL               m_bPreStop;

    MINT32              m_i4OverrideMinFrameRate;
    MINT32              m_i4OverrideMaxFrameRate;

    MINT32              m_i4AETargetMode = AE_MODE_NORMAL;
    MBOOL               m_bCCUEn;
    MBOOL               m_bTgIntAEEn;
    MFLOAT              m_fTgIntAERatio;
    std::vector<AF2AEInfo_T>       AFinfovec;
    std::atomic<bool>   m_bBPCIThreadAlive;

private:
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IspTuningMgr*       m_pTuning;
    IThreadRaw*         m_pThreadRaw;
    StateMgr*           m_pStateMgr;
    AfStateMgr*         m_pAfStateMgr;
    NSCcuIf::ICcuMgrExt* m_pICcuMgr;
    Param_T             m_rParam;
    AF_Param_T          m_rAfParam;
    ScenarioParam       m_sParam;
    I3ACallBack*        m_pCbSet;
    MUINT32             m_u4MagicNumber;
    MBOOL               m_bIsHighQualityCaptureOn;
    MBOOL               m_bIsCapEnd;
    MBOOL               m_bIsSkipSync3A;
    MINT32              m_i4StereoWarning;
    pthread_t           m_ThreadAE;
    pthread_t           m_ThreadAF;
    pthread_t           m_ThreadIspBpci;
#if CAM3_STEREO_FEATURE_EN
    Stereo_Param_T m_rLastStereoParam;
#endif

    //CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    CqTuningMgr*        m_pCqTuningMgr;
#endif
    ThreadSetCCUPara*   m_pThreadSetCCUPara;
};

template <MINT32 sensorDevId>
class Hal3ARawImpDev : public Hal3ARawImp
{
public:
    static Hal3ARawImpDev* getInstance(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex)
    {
        static Hal3ARawImpDev<sensorDevId> singleton(i4SensorDevId, i4SensorOpenIndex);
        return &singleton;
    }
    Hal3ARawImpDev(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex)
        : Hal3ARawImp(i4SensorDevId, i4SensorOpenIndex)
    {
    }

private:

};

I3AWrapper*
Hal3ARaw::
createInstance(MINT32 const i4SensorOpenIndex)
{
    return Hal3ARawImp::createInstance(i4SensorOpenIndex);
}

I3AWrapper*
Hal3ARawImp::
createInstance(MINT32 const i4SensorOpenIndex)
{
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    MUINT32 i4SensorDevId = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            Hal3ARawImpDev<SENSOR_DEV_MAIN>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3ARawImpDev<SENSOR_DEV_MAIN>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        case SENSOR_DEV_SUB:
            Hal3ARawImpDev<SENSOR_DEV_SUB>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3ARawImpDev<SENSOR_DEV_SUB>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        case SENSOR_DEV_MAIN_2:
            Hal3ARawImpDev<SENSOR_DEV_MAIN_2>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3ARawImpDev<SENSOR_DEV_MAIN_2>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        case SENSOR_DEV_SUB_2:
            Hal3ARawImpDev<SENSOR_DEV_SUB_2>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3ARawImpDev<SENSOR_DEV_SUB_2>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        default:
            CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }
}

MVOID
Hal3ARawImp::
destroyInstance()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    uninit();
}

Hal3ARawImp::
Hal3ARawImp(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex)
    : m_3ALogEnable(0)
    , m_Users(0)
    , m_i4SensorIdx(i4SensorOpenIndex)
    , m_i4SensorDev(i4SensorDevId)
    , m_u4SensorMode(0)
    , m_u4TgInfo(0)
    , m_bEnable3ASetParams(MTRUE)
    , m_bFaceDetectEnable(MFALSE)
    , m_i4InCaptureProcess(0)
    , m_i4TgWidth   (1000)
    , m_i4TgHeight  (1000)
    , mbIsHDRShot(MFALSE)
    , m_i4HbinWidth (1000)
    , m_i4HbinHeight(1000)
    , m_fgEnableShadingMeta(MTRUE)
    , m_bIsRecordingFlash(MFALSE)
    , m_i4HWSuppportFlash(0)
    , m_i4EVCap(0)
    , m_bIsAFTrigInPrecapState(MFALSE)
    , m_i4SensorPreviewDelay(0)
    , m_i4AeShutDelayFrame(0)
    , m_i4AeISPGainDelayFrame(0)
    , m_bPreStop(MFALSE)
    , m_i4OverrideMinFrameRate(0)
    , m_i4OverrideMaxFrameRate(0)
    , m_bCCUEn(0)
    , m_bTgIntAEEn(0)
    , m_fTgIntAERatio(0)
    , AFinfovec(24, AF2AEInfo_T())
    , m_bBPCIThreadAlive(0)
    , m_pCamIO(NULL)
    , m_pTuning(NULL)
    , m_pThreadRaw(NULL)
    , m_pStateMgr(NULL)
    , m_pAfStateMgr(NULL)
    , m_pICcuMgr(NULL)
    , m_pCbSet(NULL)
    , m_u4MagicNumber(0)
    , m_bIsHighQualityCaptureOn(MFALSE)
    , m_bIsCapEnd(MFALSE)
    , m_bIsSkipSync3A(MFALSE)
    , m_i4StereoWarning(0)
    //CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    , m_pCqTuningMgr(NULL)
#endif
    , m_pThreadSetCCUPara(NULL)
{
    CAM_LOGD("[%s] sensorDev(%d) sensorIdx(%d)", __FUNCTION__, i4SensorDevId, i4SensorOpenIndex);
}

MBOOL
Hal3ARawImp::
init()
{
    GET_PROP("vendor.debug.camera.log", 0, m_3ALogEnable);
    if ( m_3ALogEnable == 0 ) {
        GET_PROP("vendor.debug.camera.log.hal3a", 0, m_3ALogEnable);
    }
    m_DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // check user count
    //MRESULT ret = S_3A_OK;  /* fix build warning */
    MBOOL bRet = MTRUE;
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_fgEnableShadingMeta = MTRUE;

    // init Thread and state mgr
    m_pThreadRaw = IThreadRaw::createInstance(this, m_i4SensorDev, m_i4SensorIdx);

    CAM_LOGD("[%s] 2D", __FUNCTION__);
    m_pStateMgr = new StateMgr(m_i4SensorDev); // this
    m_pStateMgr->setThreadRaw(m_pThreadRaw);
    m_pAfStateMgr = new AfStateMgr(m_i4SensorDev);

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
    IAfMgr::getInstance().init(m_i4SensorDev, m_i4SensorIdx);
    AAA_TRACE_END_D;
#endif

    //FLICKER init
#if CAM3_FLICKER_FEATURE_EN
    AAA_TRACE_D("FLICKER init");
    FlickerHalBase::getInstance().Init(m_i4SensorDev, m_i4SensorIdx);
    AAA_TRACE_END_D;
#endif

    // TuningMgr init
    if (m_pTuning == NULL)
    {
        AAA_TRACE_D("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx))
        {
            CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_3A_HAL("Fail to init IspTuningMgr");
            AAA_TRACE_END_D;
            return MFALSE;
        }
        AAA_TRACE_END_D;
    }

    AAA_TRACE_D("querySensorStaticInfo");
    querySensorStaticInfo();
    ::pthread_create(&m_ThreadIspBpci, NULL, Hal3ARawImp::ThreadIspSetBpciTable, this);
    m_bBPCIThreadAlive = 1;
    AAA_TRACE_END_D;

    // state mgr transit to Init state.
    bRet = postCommand(ECmd_Init);
    if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");

    CAM_LOGD("[%s] done\n", __FUNCTION__);
    android_atomic_inc(&m_Users);
    return S_3A_OK;
}

MBOOL
Hal3ARawImp::
uninit()
{
    //MRESULT ret = S_3A_OK;  /* fix build warning */
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

#if CAM3_FLICKER_FEATURE_EN
        // FLICKER uninit
        err = FlickerHalBase::getInstance().Uninit(m_i4SensorDev);
        if (FAILED(err)) {
            CAM_LOGE("FlickerHalBase::getInstance().Uninit fail\n");
            return err;
        }
#endif
        // check ISP BPCI thread
        if (m_bBPCIThreadAlive){
            MVOID* pRet;
            ::pthread_join(m_ThreadIspBpci, &pRet);
            m_bBPCIThreadAlive = 0;
            if (pRet)    CAM_LOGD("Thread ISP BPCI join failed");
        }

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

        m_i4OverrideMinFrameRate = 0;
        m_i4OverrideMaxFrameRate = 0;

        CAM_LOGD("[%s] done\n", __FUNCTION__);

    }
    else    // There are still some users.
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] Still %d users \n", __FUNCTION__, m_Users);
    }

    return S_3A_OK;
}

MINT32
Hal3ARawImp::
config(const ConfigInfo_T& rConfigInfo)
{
    CAM_LOGD("[%s]+ sensorDev(%d), Mode(%d)", __FUNCTION__, m_i4SensorDev, m_u4SensorMode);
    MRESULT err = S_3A_OK;

    // check ISP BPCI thread
    if (m_bBPCIThreadAlive){
        MVOID* pRet;
        ::pthread_join(m_ThreadIspBpci, &pRet);
        m_bBPCIThreadAlive = 0;
        if (pRet)    CAM_LOGD("Thread ISP BPCI join failed");
    }

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

    if(rConfigInfo.i4BitMode != EBitMode_12Bit && rConfigInfo.i4BitMode != EBitMode_14Bit)
    {
        CAM_LOGE("Not support BitMode(%d)", rConfigInfo.i4BitMode);
        return err;
    }

    m_bAAOMode = rConfigInfo.i4BitMode == EBitMode_14Bit;
    CAM_LOGD("[%s] setAAOMode(%d)", __FUNCTION__, rConfigInfo.i4BitMode);

    MUINT32 u4AAWidth, u4AAHight;
    MUINT32 u4AFWidth, u4AFHeight;

    CAM_LOGD("[%s] setAAOMode(%d)", __FUNCTION__, rConfigInfo.i4BitMode);
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
    AAA_TRACE_D("AWB setSensor");
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight, u4AAWidth, u4AAHight);
    AAA_TRACE_END_D;


    IAEBufMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode);

#if CAM3_AF_FEATURE_EN
    AAA_TRACE_D("AF setSensor");
    IAfMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight);
    AAA_TRACE_END_D;
#endif

#if CAM3_FLICKER_FEATURE_EN
    AAA_TRACE_D("Flicker setSensor");
    FlickerHalBase::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AAWidth, u4AAHight);
    AAA_TRACE_END_D;
#endif

    MVOID* ThreadAEret;
    ::pthread_create(&m_ThreadAE, NULL, Hal3ARawImp::ThreadAEStart, this);

    MVOID* ThreadAFret;
    ::pthread_create(&m_ThreadAF, NULL, Hal3ARawImp::ThreadAFStart, this);

    AAA_TRACE_D("Shading setSensor");
    NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setSensorMode(
        static_cast<ESensorMode_T>(m_u4SensorMode), u4AFWidth, u4AFHeight, MFALSE);
    AAA_TRACE_END_D;
    // FIXME (remove): update TG Info to 3A modules
    updateTGInfo();

    //Frontal Binning
    MBOOL fgFrontalBin = (m_i4TgWidth == (MINT32)u4AFWidth && m_i4TgHeight == (MINT32)u4AFHeight) ? MFALSE : MTRUE;
    AAA_TRACE_D("Tuning setSensor");
    m_pTuning->setSensorMode(m_i4SensorDev, m_u4SensorMode, fgFrontalBin, u4AFWidth, u4AFHeight);
    AAA_TRACE_END_D;
    m_pTuning->setIspProfile(m_i4SensorDev, NSIspTuning::EIspProfile_Preview);

    // AWB start
    AAA_TRACE_D("AWB Start");
    IAwbMgr::getInstance().setAAOMode(m_i4SensorDev, m_bAAOMode);
    AAA_TRACE_D("AWB Scenario Mode");
    IAwbMgr::getInstance().setCamScenarioMode(m_i4SensorDev, CAM_SCENARIO_PREVIEW);
    AAA_TRACE_END_D;
    err = IAwbMgr::getInstance().start(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().start() fail\n");
        return E_3A_ERR;
    }
    AAA_TRACE_END_D;

#if CAM3_FLASH_FEATURE_EN
    // Flash cameraPreviewStart + start
    AAA_TRACE_D("FLASH Start");
    err = FlashMgr::getInstance().cameraPreviewStart(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().cameraPreviewStart() fail\n");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    FlashMgr::getInstance().setAAOMode(m_i4SensorDev, m_bAAOMode);
    err = FlashMgr::getInstance().start(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().start() fail\n");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    AAA_TRACE_END_D;
#endif

    MBOOL enable_flk = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isFlkEnable();
    CAM_LOGD("[%s] enable_flk=%d", __FUNCTION__, (int)enable_flk);
#if CAM3_FLICKER_FEATURE_EN
    // Set Uni status for flicker enable
    FlickerHalBase::getInstance().setUniInfo(m_i4SensorDev,enable_flk);
    // Flicker start
    // AAASensorMgr::getInstance().setFlickerFrameRateActive(m_i4SensorDev, 1);
    AAA_TRACE_D("FLICKER Start");
    err = FlickerHalBase::getInstance().Start(m_i4SensorDev);
    if (FAILED(err)) {
        CAM_LOGE("FlickerHalBase::getInstance().Start() fail\n");
        return err;
    }
    AAA_TRACE_END_D;
#endif

    ::pthread_join(m_ThreadAE, &ThreadAEret);
    if (ThreadAEret)    CAM_LOGD("Thread AE create failed");

    ::pthread_join(m_ThreadAF, &ThreadAFret);
    if (ThreadAFret)    CAM_LOGD("Thread AE create failed");

    // apply 3A module's config
    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    rRequestSet.vNumberSet.push_back(1);
    IspTuningMgr::getInstance().setFlkEnable(m_i4SensorDev, enable_flk);
    m_pTuning->notifyRPGEnable(m_i4SensorDev, MTRUE);   // apply awb gain for init stat
    m_pTuning->validate(m_i4SensorDev, rRequestSet, MTRUE);
    CAM_LOGD("[%s]-", __FUNCTION__);
    AAA_TRACE_END_D;

    return S_3A_OK;
}

MBOOL
Hal3ARawImp::
start()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    MRESULT isAFLampOn = MFALSE;
    m_i4InCaptureProcess = 0;
    m_u4MagicNumber = 0;
    m_bPreStop = MFALSE;

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::getInstance()->DevCount(MTRUE, m_i4SensorDev);
#endif

#if CAM3_AF_FEATURE_EN
    if(IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev) > 0)
    {
        // enable AF thread
        m_pThreadRaw->enableAFThread(m_pAfStateMgr);
    }
#endif

    m_pThreadRaw->createAEThread();

#if CAM3_FLASH_FEATURE_EN
    isAFLampOn = FlashMgr::getInstance().isAFLampOn(m_i4SensorDev);
#endif

    // init Set CCU para Thread
    if(m_bCCUEn == MTRUE && m_pThreadSetCCUPara == NULL)
        m_pThreadSetCCUPara = ThreadSetCCUPara::createInstance(this, m_i4SensorDev, m_i4SensorIdx);

    // setStrobeMode
    if((FlashMgr::getInstance().getFlashMode(m_i4SensorDev) != LIB3A_FLASH_MODE_FORCE_TORCH)){
      IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? MTRUE : MFALSE);
      CAM_LOGD("[%s] Non Torch ae setStrobeMode", __FUNCTION__);
    }
    IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_SET_GMA_SCENARIO, IspTuningMgr::E_GMA_SCENARIO_PREVIEW, 0);
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_SET_LCE_SCENARIO, IspTuningMgr::E_LCE_SCENARIO_PREVIEW, 0);
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_START, 0, 0);

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
        GET_PROP("vendor.3a.cct.sensor.index", 0, cct_sensor_dev);
        CAM_LOGD("CCT init: sensor = (%d/%d) ", cct_sensor_dev, m_i4SensorDev);
        if(cct_sensor_dev == m_i4SensorDev)
        {
            m_3ACctEnable = MTRUE;
            CctSvrEnt_Init(m_i4SensorDev);
            CctSvrEnt_Ctrl(CCT_SVR_CTL_START_SERVER_THREAD);
        }
    }
#endif
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
stop()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    MRESULT err = S_3A_OK;
    m_u4MagicNumber = 0;

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::getInstance()->DevCount(MFALSE, m_i4SensorDev);
    MBOOL fgIsActive = ISync3AMgr::getInstance()->isActive();
    if(fgIsActive)
    {
        ISync3AMgr::getInstance()->uninit();
        m_pStateMgr->switchToNormal();
    }
    m_bIsSkipSync3A = MFALSE;
#endif
    // AE stop
    AAA_TRACE_D("AE Stop");
/*#if CAM3_STEREO_FEATURE_EN
    if (ISync3AMgr::getInstance()->isActive())
        IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MFALSE);
#endif*/
    err = IAeMgr::getInstance().Stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
    CAM_LOGE("IAeMgr::getInstance().Stop() fail\n");
        return err;
    }
    m_pThreadRaw->destroyAEThread();

    // AWB stop
    AAA_TRACE_D("AWB Stop");
    err = IAwbMgr::getInstance().stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (!err) {
        CAM_LOGE("IAwbMgr::getInstance().stop() fail\n");
        return E_3A_ERR;
    }

#if CAM3_FLASH_FEATURE_EN
    // Flash cameraPreviewEnd + stop
    AAA_TRACE_D("Flash Stop");
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
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    err = FlashMgr::getInstance().stop(m_i4SensorDev);
    if (!err) {
        CAM_LOGE("FlashMgr::getInstance().stop() fail\n");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    AAA_TRACE_END_D;
#endif

#if CAM3_AF_FEATURE_EN
    // AF stop
    AAA_TRACE_D("AF Stop");
    err = IAfMgr::getInstance().Stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Stop() fail\n");
        return err;
    }
    // disable AF thread
    AAA_TRACE_D("AF THREAD disable");
    m_pThreadRaw->disableAFThread();
    AAA_TRACE_END_D;
#endif

#if CAM3_FLICKER_FEATURE_EN
    // Flicker close
    AAA_TRACE_D("Flicker Stop");
    FlickerHalBase::getInstance().Stop(m_i4SensorDev);
    AAA_TRACE_END_D;
#endif

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);

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

    if(m_bCCUEn && m_pThreadSetCCUPara)
    {
        m_pThreadSetCCUPara->destroyInstance();
        m_pThreadSetCCUPara = NULL;
    }
#if 1
    if ( m_3ACctEnable == 1 ) {
        CctSvrEnt_Ctrl(CCT_SVR_CTL_STOP_SERVER_THREAD);
        CctSvrEnt_DeInit();
    }
#endif
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MVOID
Hal3ARawImp::
pause()
{
    m_pThreadRaw->pauseAFThread();
}

MVOID
Hal3ARawImp::
resume(MINT32 MagicNum)
{
    DebugUtil::getInstance(m_i4SensorDev)->resetDetect(DBG_AE);
    DebugUtil::getInstance(m_i4SensorDev)->resetDetect(DBG_AF);
    // apply 3A module's config
    if (MagicNum > 0)
    {
        RequestSet_T rRequestSet;
        MBOOL enable_flk = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isFlkEnable();
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(MagicNum);
        IspTuningMgr::getInstance().setFlkEnable(m_i4SensorDev, enable_flk);
        m_pTuning->notifyRPGEnable(m_i4SensorDev, MTRUE);   // apply awb gain for init stat
        m_pTuning->validate(m_i4SensorDev, rRequestSet, MTRUE);
    }

    m_pThreadRaw->resumeAFThread();
}


MBOOL
Hal3ARawImp::
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
        if (rIspInfo.rCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_HWNR)
        {
            CAM_LOGD_IF(m_3ALogEnable, "[%s] get debug info p2 for Multi_Pass_NR #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            if (0 == pResultP2->vecDbgIspP2_MultiP.size())
            {
                CAM_LOGD_IF(m_3ALogEnable, "[%s] Need to allocate P2 result for Multi_Pass_NR", __FUNCTION__);
                pResultP2->vecDbgIspP2_MultiP.resize(sizeof(DEBUG_RESERVEA_INFO_T));
            }
            DEBUG_RESERVEA_INFO_T& rIspExifDebugInfo = *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(pResultP2->vecDbgIspP2_MultiP.editArray());
            IspTuningMgr::getInstance().getDebugInfo_MultiPassNR(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
        else
        {
            CAM_LOGD_IF(m_3ALogEnable, "[%s] get debug info p2 #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            if (0 == pResultP2->vecDbgIspP2.size())
            {
                CAM_LOGD_IF(m_3ALogEnable, "[%s] Need to allocate P2 result", __FUNCTION__);
                pResultP2->vecDbgIspP2.resize(sizeof(AAA_DEBUG_INFO2_T));
            }

            AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pResultP2->vecDbgIspP2.editArray());
            NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo = rDbg3AInfo2.rISPDebugInfo;
            IspTuningMgr::getInstance().getDebugInfo(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, pRegBuf);

            MBOOL bDump = ::property_get_int32("vendor.debug.tuning.dump_capture", 0);
            if (!rIspInfo.rCamInfo.fgRPGEnable && bDump)
            {
                char filename[512];
                sprintf(filename, "/sdcard/debug/p2dbg_dump_capture-%04d.bin", rIspInfo.i4UniqueKey);
                FILE* fp = fopen(filename, "wb");
                if (fp)
                {
                    ::fwrite(rIspExifDebugInfo.P2RegInfo.regDataP2, sizeof(rIspExifDebugInfo.P2RegInfo.regDataP2), 1, fp);
                }
                if (fp)
                    fclose(fp);
            }
        }
    }

    CAM_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] sensor(%d) #(%d) profile(%d) rpg(%d)", __FUNCTION__, m_i4SensorDev, rParamIspProfile.i4MagicNum, rParamIspProfile.eIspProfile, rParamIspProfile.iEnableRPG);
    //m_pTuning->setIspProfile(m_i4SensorDev, rParamIspProfile.eIspProfile);
    //m_pTuning->notifyRPGEnable(m_i4SensorDev, rParamIspProfile.iEnableRPG);
    m_pTuning->validatePerFrame(m_i4SensorDev, rParamIspProfile.rRequestSet, fgPerframe);
    //LCS callback
    ISP_LCS_IN_INFO_T lcs_info;
    m_pTuning->getLCSparam(m_i4SensorDev, lcs_info);
    lcs_info.i4FrmId = rParamIspProfile.i4MagicNum;
    m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_LCS_ISP_PARAMS, rParamIspProfile.i4MagicNum, (MINTPTR)&lcs_info, 0);

    return MTRUE;
}

MBOOL
Hal3ARawImp::
setParams(Param_T const &rNewParam, MBOOL bUpdateScenario)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(m3AOperMtx2);
    if (FlashMgr::getInstance().isFlashOnCalibration(m_i4SensorDev)) {
        IAeMgr::getInstance().setAPAELock(m_i4SensorDev, MTRUE);
        IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, MTRUE);
        return MTRUE;
    }
    if (!m_bEnable3ASetParams){
        m_rParam = rNewParam;
        return MTRUE;
    }

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
    IspTuningMgr::getInstance().setEdgeMode(m_i4SensorDev, rNewParam.u1EdgeMode);
    IspTuningMgr::getInstance().setNoiseReductionMode(m_i4SensorDev, rNewParam.u1NRMode);
    IspTuningMgr::getInstance().setZoomRatio(m_i4SensorDev, rNewParam.i4ZoomRatio);

    // ====================================== AE ==============================================
    if(m_i4OverrideMinFrameRate == 0 && m_i4OverrideMaxFrameRate == 0)
        IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps);
    IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev, rNewParam.u4AeMeterMode);
    IAeMgr::getInstance().setAERotateDegree(m_i4SensorDev, rNewParam.i4RotateDegree);
    IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.i4IsoSpeedMode);
    IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas);
    if (bUpdateScenario)
        IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock);
    IAeMgr::getInstance().setAEEVCompIndex(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep);
    IAeMgr::getInstance().setAEMode(m_i4SensorDev, rNewParam.u4AeMode);
    IAeMgr::getInstance().enableStereoDenoiseRatio(m_i4SensorDev, rNewParam.i4DenoiseMode);
    IAeMgr::getInstance().setAEFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);
    MINT32 i4CamModeEnable = 0;
    GET_PROP("vendor.debug.camera.cammode", 0, i4CamModeEnable);
    if (NSIspTuning::EOperMode_EM != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev) || i4CamModeEnable)
        IAeMgr::getInstance().setAECamMode(m_i4SensorDev, rNewParam.u4CamMode);
    IAeMgr::getInstance().setAEShotMode(m_i4SensorDev, rNewParam.u4ShotMode);
    IAeMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode);
    IAeMgr::getInstance().bBlackLevelLock(m_i4SensorDev, rNewParam.u1BlackLvlLock);
    if( rNewParam.rScaleCropRect.i4Xwidth != 0 && rNewParam.rScaleCropRect.i4Yheight != 0 )
        IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewParam.rScaleCropRect.i4Xoffset,rNewParam.rScaleCropRect.i4Yoffset,rNewParam.rScaleCropRect.i4Xwidth,rNewParam.rScaleCropRect.i4Yheight);
    IAeMgr::getInstance().setAEHDRMode(m_i4SensorDev, rNewParam.u1HdrMode);
    IAeMgr::getInstance().setDigZoomRatio(m_i4SensorDev, rNewParam.i4ZoomRatio);
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
    // CCU ISP OBC tuning
    MVOID* pObc = NULL;
    MUINT16 u2Obcidx = 0;
    IspTuningMgr::getInstance().getDefaultObc(m_i4SensorDev ,rNewParam.eIspProfile, m_u4SensorMode, pObc, u2Obcidx);
    IAeMgr::getInstance().updateISPNvramOBCTable(m_i4SensorDev, pObc,(MINT32)u2Obcidx);

    if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
    {
        AE_SENSOR_PARAM_T strSensorParams;
        strSensorParams.u4Sensitivity   = rNewParam.i4Sensitivity;
        strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTime;
        strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
        IAeMgr::getInstance().UpdateSensorParams(m_i4SensorDev, strSensorParams);
    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] setAEMode(%d)", __FUNCTION__, rNewParam.u4AeMode);
    if ((rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD)||
        (rNewParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT) ){
        IAeMgr::getInstance().updateAEScenarioMode(m_i4SensorDev, EIspProfile_Video);
    }else{
        IAeMgr::getInstance().updateAEScenarioMode(m_i4SensorDev, EIspProfile_Preview);
    }
    m_pStateMgr->updateMaxFrameRate(rNewParam.i4MaxFps);
    // ====================================== AWB ==============================================
    IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock);
    IAwbMgr::getInstance().setAWBMode(m_i4SensorDev, rNewParam.u4AwbMode);
    IAwbMgr::getInstance().setMWBColorTemperature(m_i4SensorDev, rNewParam.i4MWBColorTemperature);
    if( rNewParam.rScaleCropRect.i4Xwidth != 0 && rNewParam.rScaleCropRect.i4Yheight != 0 )
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
    FlashMgr::getInstance().setDigZoom(m_i4SensorDev, rNewParam.i4ZoomRatio);
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
Hal3ARawImp::
setAfParams(AF_Param_T const &rNewParam)
{
        //if (!m_bEnable3ASetParams) return MTRUE;
#if CAM3_AF_FEATURE_EN
        // ====================================== AF ==============================================
        IThreadRaw::AFParam_T rAFPAram;
        rAFPAram.u4MagicNum = rNewParam.i4MagicNum;
        rAFPAram.u4AfMode = rNewParam.u4AfMode;
        rAFPAram.rFocusAreas = rNewParam.rFocusAreas;
        if( IspTuningMgr::getInstance().getOperMode(m_i4SensorDev) == NSIspTuning::EOperMode_Normal)
            rAFPAram.fFocusDistance = rNewParam.fFocusDistance;
        else
            rAFPAram.fFocusDistance = -1;
        rAFPAram.u1AfTrig  = rNewParam.u1AfTrig;
        rAFPAram.u1PrecapTrig  = rNewParam.u1PrecapTrig;
        rAFPAram.u1AfPause = rNewParam.u1AfPause;
        rAFPAram.u1MZOn = rNewParam.u1MZOn;
        rAFPAram.bFaceDetectEnable = m_bFaceDetectEnable;
        rAFPAram.rScaleCropArea = rNewParam.rScaleCropArea;
        rAFPAram.bEnable3ASetParams = m_bEnable3ASetParams;
        m_pThreadRaw->sendRequest(ECmd_AFUpdate,(MINTPTR)&rAFPAram);
        m_rAfParam = rNewParam;
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setStereoParams(Stereo_Param_T const & rNewParam)
{
    CAM_LOGD_IF(m_3ALogEnable, "MasterIdx = %d, SlaveIdx = %d", rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx);
#if CAM3_STEREO_FEATURE_EN
    // dymanic enale/disable Sync3A
    if(!m_bIsSkipSync3A)
    {
    MBOOL fgIsInit = ISync3AMgr::getInstance()->isInit();
    if( rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF ||
        rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE ||
        rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM )
    {
        if(!fgIsInit && (rNewParam.i4MasterIdx != rNewParam.i4SlaveIdx) )
        {
            ISync3AMgr::getInstance()->init(0, rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx);
        }
        m_pStateMgr->switchToN3D();
    } else if(rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE)
    {
        if(fgIsInit)
        {
            ISync3AMgr::getInstance()->uninit();
        }
        m_pStateMgr->switchToNormal();
    }
    ISync3AMgr::getInstance()->setStereoParams(rNewParam);
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
    if(isChanged)
    {
        CAM_LOGD("[%s] m_rLastStereoParam (%d,%d)(%d,%d,%d)", __FUNCTION__,
        m_rLastStereoParam.i4MasterIdx, m_rLastStereoParam.i4SlaveIdx,
        m_rLastStereoParam.i4Sync2AMode,
        m_rLastStereoParam.i4SyncAFMode,
        m_rLastStereoParam.i4HwSyncMode);
    }
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
autoFocus()
{
    if (m_pStateMgr->getStateStatus().eCurrState == eState_Precapture
        || ((m_rParam.u1PrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START) && (m_rAfParam.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)))
    {
        CAM_LOGD("AF trigger in PrecapState");
        m_bIsAFTrigInPrecapState = MTRUE;
        m_pStateMgr->setIsAFTrigInPrecapState(MTRUE);

        m_pStateMgr->mAFStateCntSet.bIsFocused = IAfMgr::getInstance().isFocused(m_i4SensorDev);
        m_pStateMgr->mAFStateCntSet.bIsFocusFinish = IAfMgr::getInstance().isFocusFinish(m_i4SensorDev);
        CAM_LOGD("[%s] isFocused(%d), isFocusFinish(%d)", __FUNCTION__, m_pStateMgr->mAFStateCntSet.bIsFocused, m_pStateMgr->mAFStateCntSet.bIsFocusFinish);
    }
    else
    {
#if CAM3_AF_FEATURE_EN
        CAM_LOGD("[%s()] +", __FUNCTION__);
        //m_pThreadRaw->sendRequest(ECmd_AFStart);
        //if ((m_rAfParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rAfParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO)) {
        m_bIsAFTrigInPrecapState = MFALSE;
        m_pStateMgr->m_bIsAFLastStateFinished = MFALSE;
        m_pStateMgr->sendCmd(ECmd_AFStart);
        //}
        CAM_LOGD("[%s()] -", __FUNCTION__);
    }
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
cancelAutoFocus()
{
#if CAM3_AF_FEATURE_EN
    CAM_LOGD("[%s()] +", __FUNCTION__);
    //m_pThreadRaw->sendRequest(ECmd_AFEnd);
    //if ((m_rAfParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rAfParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        m_pStateMgr->sendCmd(ECmd_AFEnd);
    //}
    m_bIsAFTrigInPrecapState = MFALSE;
    m_pStateMgr->m_bIsAFLastStateFinished = MTRUE;
    m_pStateMgr->setIsAFTrigInPrecapState(MFALSE);
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
    IAeMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    IAwbMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    IspTuningMgr::getInstance().setFDEnable(m_i4SensorDev, fgEnable);
}

MBOOL
Hal3ARawImp::
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
Hal3ARawImp::
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
Hal3ARawImp::
setPreFlashOnOff(MBOOL bOnOff)
{
#if CAM3_FLASH_FEATURE_EN
    MY_LOG_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    FlashMgr::getInstance().setPreFlashOnOff(m_i4SensorDev, bOnOff);
    MY_LOG_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
isNeedTurnOnPreFlash() const
{
    return m_pStateMgr->getPreCapFlashOn();
}

MBOOL
Hal3ARawImp::
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
        IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_OFF);

    }
    else //flash on
    {
        if (bMainPre)
        {
            // check flash is on/off before Main flash on
            if (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev))
            {
                MY_LOG("[%s] get RestoreCount(%d)", __FUNCTION__, m_pStateMgr->getRestoreCount());
                if(m_pStateMgr->getRestoreCount() > 0)
                {
                    // TODO
                    // reset framelength from metadata
                    // frame 2 : set Flag for pass1 done to close flash, and restore frame rate.
                    MINT32 maxFrameRate = m_rParam.i4MaxFps / 100;
                    MY_LOG("[%s] set MaxFrameRate(%d) before Main flash on, MaxFps(%d)", __FUNCTION__, maxFrameRate, m_rParam.i4MaxFps);
                    AAASensorMgr::getInstance().setPreviewMaxFrameRate(
                        m_i4SensorDev,
                        maxFrameRate,
                        IAeMgr::getInstance().getSensorMode(m_i4SensorDev));
                    m_pStateMgr->resetRestoreCount();
                }
                FlashMgr::getInstance().setAFLampOnOff(m_i4SensorDev, MFALSE);
            }
            FlashMgr::getInstance().setCaptureFlashOnOff(m_i4SensorDev, 1);
        }
        else
        {
            // check flash is on/off before Torch on
            if(m_pStateMgr->getRestoreCount() > 0)
            {
                if (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev) && m_rParam.u4StrobeMode != MTK_FLASH_MODE_TORCH)
                    FlashMgr::getInstance().setAFLampOnOff(m_i4SensorDev, MFALSE);
                m_pStateMgr->resetRestoreCount();
            }
            FlashMgr::getInstance().setTorchOnOff(m_i4SensorDev, 1);
        }
        m_pStateMgr->updatePreCapFlashOn(MFALSE);
        IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_ON);

    }
    CAM_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
chkMainFlashOnCond() const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
    MBOOL bChkFlash;
    bChkFlash = (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_SINGLE))
                ||
                (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isFlashOnCapture()))
                && (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE));

    return bChkFlash && (m_i4HWSuppportFlash != 0) && (FlashMgr::getInstance().getFlashState(m_i4SensorDev) != MTK_FLASH_STATE_CHARGING);//shouldn't return MTRUE if Hw does not support flash
}

MBOOL
Hal3ARawImp::
chkPreFlashOnCond() const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] IsRecordingFlash(%d)", __FUNCTION__, m_bIsRecordingFlash);
    MBOOL bChkFlash;
    bChkFlash = (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH));

    return bChkFlash && (m_i4HWSuppportFlash != 0) && (FlashMgr::getInstance().getFlashState(m_i4SensorDev) != MTK_FLASH_STATE_CHARGING);//shouldn't return MTRUE if Hw does not support flash
}

MBOOL
Hal3ARawImp::
isStrobeBVTrigger() const
{
    return IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);
}

MBOOL
Hal3ARawImp::
isFlashOnCapture() const
{
    return FlashMgr::getInstance().isFlashOnCapture(m_i4SensorDev);
}

MBOOL
Hal3ARawImp::
dumpP1Params(MUINT32 u4MagicNum)
{
    CAM_LOGD("[%s] #(%d)", __FUNCTION__, u4MagicNum);
    IspDebug rP1(m_i4SensorIdx, m_i4SensorDev);
    return rP1.dumpRegs(u4MagicNum);
}

MINT32
Hal3ARawImp::
getCurrResult(MUINT32 i4FrmId, Result_T& rResult, MINT32 /*i4SubsampleIndex*/) const
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] + i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState;
    rResult.i4FrmId = i4FrmId;
    rResult.fgKeep = m_bIsCapEnd;

    // clear memory
    rResult.vecExifInfo.clear();
    rResult.vecTonemapCurveRed.clear();
    rResult.vecTonemapCurveGreen.clear();
    rResult.vecTonemapCurveBlue.clear();
    rResult.vecColorCorrectMat.clear();
    rResult.vecDbg3AInfo.clear();
    rResult.vecDbgShadTbl.clear();

    rResult.vecAEROI.clear();
    rResult.vecAFROI.clear();
    rResult.vecAWBROI.clear();

    //put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
    IAeMgr::getInstance().getAEFDMeteringAreaInfo(m_i4SensorDev, rResult.vecAEROI, m_i4TgWidth, m_i4TgHeight);
    IAfMgr::getInstance().getFocusArea(m_i4SensorDev, rResult.vecAFROI);
    IAwbMgr::getInstance().getFocusArea(m_i4SensorDev, rResult.vecAWBROI, m_i4TgWidth, m_i4TgHeight);

    rResult.u1SceneMode = m_rParam.u4SceneMode;

    // AE
    AAA_TRACE_S(getAEState);
    rResult.u1AeMode = m_rParam.u4AeMode;
    MUINT8 u1AeState = IAeMgr::getInstance().getAEState(m_i4SensorDev);
    if (IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev) && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rResult.u1AeState = u1AeState;
    AAA_TRACE_END_S;
    if((m_pStateMgr->getStateStatus().eCurrState == eState_AF && m_pStateMgr->getStateStatus().eNextState == eState_Precapture)
        || m_pStateMgr->getStateStatus().eCurrState == eState_Precapture)
        rResult.u1AeState = MTK_CONTROL_AE_STATE_PRECAPTURE;

    rResult.fgAeBvTrigger = IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);

    // AutoHdrResult
    rResult.i4AutoHdrResult = (MINT32)IAeMgr::getInstance().getAEHDROnOff(m_i4SensorDev);
    //

    AAA_TRACE_S(getSensorParams);
    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
    if (m_rParam.u1RollingShutterSkew)
        rResult.i8SensorRollingShutterSkew = IAeMgr::getInstance().getSensorRollingShutter(m_i4SensorDev);
    AAA_TRACE_END_S;

    // AWB
    AAA_TRACE_S(getAWBState);
    IAwbMgr::getInstance().getAWBState(m_i4SensorDev, eAwbState);
    rResult.u1AwbState= eAwbState;
    AWB_GAIN_T rAwbGain;
    IAwbMgr::getInstance().getAWBGain(m_i4SensorDev, rAwbGain, rResult.i4AwbGainScaleUint);
    rResult.i4AwbGain[0] = rAwbGain.i4R;
    rResult.i4AwbGain[1] = rAwbGain.i4G;
    rResult.i4AwbGain[2] = rAwbGain.i4B;
    AAA_TRACE_END_S;

    AAA_TRACE_S(getColorCorrectGain);
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
    AAA_TRACE_END_S;

    AAA_TRACE_S(getAFState);
#if CAM3_AF_FEATURE_EN
    // AF
    rResult.u1AfState = IAfMgr::getInstance().getAFState(m_i4SensorDev);
    IAfMgr::getInstance().getFocusAreaResult(m_i4SensorDev, rResult.vecFocusAreaPos, rResult.vecFocusAreaRes, rResult.i4FocusAreaSz[0], rResult.i4FocusAreaSz[1]);
    if (m_pStateMgr->getStateStatus().eCurrState == eState_AF || m_pStateMgr->getIsAFTrigInPrecapState())
    {
        if( m_rAfParam.u4AfMode == MTK_CONTROL_AF_MODE_AUTO || m_rAfParam.u4AfMode == MTK_CONTROL_AF_MODE_MACRO)
            rResult.u1AfState = MTK_CONTROL_AF_STATE_ACTIVE_SCAN;
        else if( m_rAfParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
            rResult.u1AfState = MTK_CONTROL_AF_STATE_PASSIVE_SCAN;
        else if( m_rAfParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO )
            rResult.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
    }
    else
    {
        if( (m_rAfParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO)|| (m_rAfParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE))
        {
            if(m_rAfParam.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
            {
                if(rResult.u1AfState == MTK_CONTROL_AF_STATE_PASSIVE_FOCUSED)
                    rResult.u1AfState = MTK_CONTROL_AF_STATE_FOCUSED_LOCKED;
                else
                    rResult.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
            }
        }
    }
    rResult.u1LensState = IAfMgr::getInstance().getLensState(m_i4SensorDev);
    rResult.fLensFocusDistance = IAfMgr::getInstance().getFocusDistance(m_i4SensorDev);
    IAfMgr::getInstance().getFocusRange(m_i4SensorDev, &rResult.fLensFocusRange[0], &rResult.fLensFocusRange[1]);
#endif
    AAA_TRACE_END_S;

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

    AAA_TRACE_S(getFlashState);
#if CAM3_FLICKER_FEATURE_EN
    // Flicker
    MINT32 i4FlkStatus = IAeMgr::getInstance().getAEAutoFlickerState(m_i4SensorDev);
    //FlickerHalBase::getInstance().getFlickerResult(i4FlkStatus);
    MUINT8 u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_NONE;
    if (i4FlkStatus == 0) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_50HZ;
    if (i4FlkStatus == 1) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_60HZ;
    rResult.u1SceneFlk = (MUINT8)u1ScnFlk;
#endif
    AAA_TRACE_END_S;

#if CAM3_LSC_FEATURE_EN
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
    CAM_LOGD_IF(m_3ALogEnable, "[%s] #lsc(%d) m_rParam.i4RawType(%d) m_bIsCapEnd(%d)", __FUNCTION__, pLsc->getOnOff(), m_rParam.i4RawType, m_bIsCapEnd);
    MBOOL fgRequireShadingIntent = 1; /*
        (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG);*/
    rResult.vecLscData.clear();
    if ((pLsc->getOnOff() && fgRequireShadingIntent && (m_rParam.i4RawType == NSIspTuning::ERawType_Pure || m_bIsCapEnd) /*IMGO pure raw*/) ||
        (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON))
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] #(%d) LSC table to metadata", __FUNCTION__, i4FrmId);
        AAA_TRACE_S(getCurrTbl);
        pLsc->getCurrTbl(rResult.vecLscData);
        AAA_TRACE_END_S;
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

    AAA_TRACE_S(StereoFeatureEnable);
    rResult.i4StereoWarning = 0;
#if CAM3_STEREO_FEATURE_EN
    if (ISync3AMgr::getInstance()->isActive())
    {
        MINT32 i4Master = ISync3AMgr::getInstance()->getMasterDev();
        MINT32 i4Slave = ISync3AMgr::getInstance()->getSlaveDev();
        if(i4Master == m_i4SensorDev)
        {
            // large lv diff
            MINT32 i4LvMaster = IAeMgr::getInstance().getLVvalue(i4Master, MTRUE);
            MINT32 i4LvSlave = IAeMgr::getInstance().getLVvalue(i4Slave, MTRUE);
            rResult.i4StereoWarning |= ((abs(i4LvMaster - i4LvSlave) > 50) ? 0x1 : 0);
            // low light
            rResult.i4StereoWarning |= ((max(i4LvMaster, i4LvSlave) < 10) ? 0x2 : 0);
            // close shot
            MINT32 i4FocusDistance = IAfMgr::getInstance().getAFBestPos(i4Master);
            MINT32 i4MaxFocusDistance = IAfMgr::getInstance().getMaxLensPos(i4Master);
            MINT32 i4MinFocusDistance = IAfMgr::getInstance().getMinLensPos(i4Master);
            if(i4FocusDistance > (i4MaxFocusDistance - (i4MaxFocusDistance - i4MinFocusDistance) / 5))
                rResult.i4StereoWarning |= 0x4;

            CAM_LOGD_IF(m_3ALogEnable,
                "[%s] warning(%d) lv(%d,%d), fd(%d,%d,%d)", __FUNCTION__, rResult.i4StereoWarning, i4LvMaster, i4LvSlave, i4FocusDistance, i4MaxFocusDistance, i4MinFocusDistance);
        }
    }
#endif
    AAA_TRACE_END_S;

/*
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
*/
    // Cam Info
    AAA_TRACE_S(getInfo);
    if (!IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rResult.rCamInfo))
    {
        CAM_LOGE("Fail to get CamInfo");
    }

    rResult.rCamInfo.bBypassLCE = MTRUE;

    // Exif
    if (m_rParam.u1IsGetExif || m_bIsCapEnd)
    {
        rResult.vecExifInfo.resize(1);
        get3AEXIFInfo(rResult.vecExifInfo.editTop());
        // debug exif
        rResult.vecDbg3AInfo.resize(sizeof(AAA_DEBUG_INFO1_T));
        rResult.vecDbgIspInfo.resize(sizeof(AAA_DEBUG_INFO2_T));
        rResult.vecDbgShadTbl.resize(sizeof(DEBUG_SHAD_ARRAY_INFO_T));

        AAA_DEBUG_INFO1_T& rDbg3AInfo = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(rResult.vecDbg3AInfo.editArray());
        AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rResult.vecDbgIspInfo.editArray());
        DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl = *reinterpret_cast<DEBUG_SHAD_ARRAY_INFO_T*>(rResult.vecDbgShadTbl.editArray());

        if(rResult.vecDbgShadTbl.size() != 0 && rResult.vecDbgIspInfo.size() != 0 && rResult.vecDbg3AInfo.size() != 0)
        {
            CAM_LOGD_IF(m_3ALogEnable, "[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)rResult.vecDbgShadTbl.size(), (MINT32)rResult.vecDbgIspInfo.size(), (MINT32)rResult.vecDbg3AInfo.size(),
                                        &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);
            getP1DbgInfo(rDbg3AInfo, rDbgShadTbl, rDbgIspInfo);
        }
        else
            CAM_LOGE("[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)rResult.vecDbgShadTbl.size(), (MINT32)rResult.vecDbgIspInfo.size(), (MINT32)rResult.vecDbg3AInfo.size(),
                                        &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);

#if CAM3_STEREO_FEATURE_EN
        // debug exif for N3D
        if (ISync3AMgr::getInstance()->isActive())
        {
            rResult.vecDbgN3DInfo.resize(sizeof(N3D_DEBUG_INFO_T));
            N3D_DEBUG_INFO_T& rDbgN3DInfo = *reinterpret_cast<N3D_DEBUG_INFO_T*>(rResult.vecDbgN3DInfo.editArray());
            if(rResult.vecDbgN3DInfo.size() != 0)
                ISync3AMgr::getInstance()->getSync3A()->setDebugInfo((void*)(&rDbgN3DInfo));
        }
#endif
    }
    AAA_TRACE_END_S;

    CAM_LOGD_IF(m_3ALogEnable, "[%s] - i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return 0;
}

MINT32
Hal3ARawImp::
getCurrentHwId() const
{
    MINT32 idx = 0;
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX,
                        (MINTPTR)&idx, 0, 0);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] idx(%d)", __FUNCTION__, idx);
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
    CAM_LOGD_IF(m_3ALogEnable, "[%s]+ cmd(%d)", __FUNCTION__, r3ACmd);
    if (pParam != NULL)
    {
        if (pParam->iValidateOpt == ParamIspProfile_T::EParamValidate_None)
        {
            AAASensorMgr::getInstance().setSensorExpTime(m_i4SensorDev, 100000);
            CAM_LOGD("[%s] set next frame rate 10fps", __FUNCTION__);
            return MTRUE;
        }
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

    if (ECmd_Update == r3ACmd)
    {
        if (m_pStateMgr->getFrameCount() >= -1)
        {
            ParamIspProfile_T rParamIsp = *pParam;
            m_pTuning->setIspProfile(m_i4SensorDev, rParamIsp.eIspProfile);
            m_pTuning->notifyRPGEnable(m_i4SensorDev, rParamIsp.iEnableRPG);
        }
    }

    // for 3A state
    switch(r3ACmd)
    {
        case ECmd_Uninit:
        case ECmd_CameraPreviewEnd:
            {// back next state when previewEnd or uninit command on AF state.
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
                if(m_bIsAFTrigInPrecapState)
                {
                    CAM_LOGD("[%s]ECmd_CameraPreviewEnd reset AF flow : m_bIsAFTrigInPrecapState",__FUNCTION__);
                    m_bIsAFTrigInPrecapState = MFALSE;
                    m_pStateMgr->setIsAFTrigInPrecapState(MFALSE);
                }
            }
            break;
        case ECmd_Update:
            {
#if CAM3_STEREO_FEATURE_EN
                MBOOL fgIsActive = ISync3AMgr::getInstance()->isActive();
                if(fgIsActive)
                {
                    IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MTRUE);
                    m_pStateMgr->switchToN3D();
                }
                else
                {
                    IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MFALSE);
                    m_pStateMgr->switchToNormal();
                    ISync3AMgr::getInstance()->setAFState(ISync3AMgr::E_SYNC3AMGR_AF_STATE_IDLE);
                }
#endif
                // update TG INT Params to driver
                if(m_pStateMgr->queryMagicNumberRequest() != 0 && m_bTgIntAEEn)//call before updateMagicNumberRequest
                    if(!setTG_INTParams())
                        CAM_LOGE("[%s] setTG INTParams fail", __FUNCTION__);

                //if(m_pStateMgr != NULL) Fix Coverity Issue
                {
                     m_pStateMgr->updateMagicNumberRequest(pParam->i4MagicNum);
                     CAM_LOGD_IF(m_3ALogEnable,"[HQC] updateMagicNumberRequest(%d)", pParam->i4MagicNum);
                     m_bIsHighQualityCaptureOn = MFALSE;
                }

                if (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD)
                    m_pStateMgr->mAFStateCntSet.bIsRecording = MTRUE; //notify af_state_mgr recoding is ongoing, when state transit to AF state, touch AF won't flash

                if(m_bIsAFTrigInPrecapState)
                {
                     if ((m_pStateMgr->getStateStatus().eCurrState != eState_Precapture)
                         && !((m_rParam.u1PrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START) && (m_rAfParam.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)))
                     {
                         CAM_LOGD("[%s] leave PreacapState, AutoFocus", __FUNCTION__);
                         autoFocus();
                     }
                }
            }
            break;
#if CAM3_STEREO_FEATURE_EN
        case ECmd_PrecaptureStart:
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
            break;
#endif
        default:
            break;
    }

    MBOOL fgOldCapQ = m_pStateMgr->isCapQueueEmpty();

    // capture flow.
    if ((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) && (r3ACmd == ECmd_Update) && (m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF))
    {
        MBOOL fgMainFlash = pParam->bMainFlash;
        MBOOL bDummyAfterCapture = m_rParam.bDummyAfterCapture;

        CAM_LOGD_IF(m_3ALogEnable,"[%s] capture flow", __FUNCTION__);
        if (m_pStateMgr->getStateStatus().eCurrState == eState_Precapture)
        {
            CAM_LOGD("Unexpected Operation since precapture is not finished.");
            m_pStateMgr->sendCmd(ECmd_PrecaptureEnd);
        }

        MINT32 i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
        MBOOL bUpdateStatus = m_pStateMgr->queryUpdateStatus();
        MBOOL bAELock = IAeMgr::getInstance().IsAPAELock(m_i4SensorDev);
        MBOOL bIsEVchaged = m_i4EVCap != IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev);
        MBOOL bIsStartCapture = m_rParam.u1IsStartCapture;

        if (bIsEVchaged){
            m_i4EVCap = IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev);
            //bAELock = MFALSE;
        }
        CAM_LOGD_IF(m_3ALogEnable,"[%s] i4OperMode(%d) bUpdateStatus(%d) bDummyAfterCapture(%d) mbIsHDRShot(%d) bAELock(%d) EV(%d) EVchanged (%d)", __FUNCTION__, i4OperMode, bUpdateStatus, bDummyAfterCapture, mbIsHDRShot, bAELock, m_i4EVCap, bIsEVchaged);
        if (bUpdateStatus || bDummyAfterCapture) //should't update captureparams after precapture until capture start
        {
/*
some senerio should not updateCaptureParams
1. capture with flash
2. HDR shot, camera3
3. i4OperMode != EOperMode_Meta
4. AELock is set
*/
#if CAM3_FLASH_FEATURE_EN
            if(!fgMainFlash && !mbIsHDRShot && (i4OperMode != EOperMode_Meta) &&
                (!bAELock && (m_i4EVCap==0)))
#endif
            {
                AE_MODE_CFG_T previewInfo;
                IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);
                IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, previewInfo);
            }
        }
        m_pStateMgr->setUpdateStatus(MTRUE);
        CAM_LOGD_IF(m_3ALogEnable,"[%s] unlock update",__FUNCTION__);

        if (mbIsHDRShot)
        {
            mbIsHDRShot = MFALSE;
        }

        if (m_rParam.u1IsSingleFrameHDR)
            IAeMgr::getInstance().setzCHDRShot(m_i4SensorDev, MTRUE);

        /*if (bIsStartCapture) //optimized capture flow, skip precapture, still need to prepare capture pline
        {
            m_pStateMgr->setStartCapture(MTRUE);
        }*/

        /*{
            m_i4InCaptureProcess = 1;
            m_pStateMgr->sendCmd(ECmd_CaptureStart);
        }*/
        // high quality cap
        MINT32 iForceHqc = ::property_get_int32("vendor.debug.camera.force_hqc", 1);
        if (fgMainFlash ||
            ((iForceHqc & 0x1) && (bIsStartCapture && m_rParam.i4RawType == NSIspTuning::ERawType_Pure)) ||
            (iForceHqc & 0x2))
        {
            MINT32 i4MagicNumbeRequest;
            i4MagicNumbeRequest = m_pStateMgr->queryMagicNumberRequest();
            m_pStateMgr->updateCapQueue(i4MagicNumbeRequest);
            m_bIsHighQualityCaptureOn = MTRUE;
            CAM_LOGD_IF(m_3ALogEnable,"[HQC] i4MagicNumbeRequest = %d", i4MagicNumbeRequest);
        }

        {
            m_i4InCaptureProcess = 1;
            m_pStateMgr->sendCmd(ECmd_CaptureStart);
        }
    }
    else if ((r3ACmd == ECmd_PrecaptureEnd))
    {
        if(m_pStateMgr->getStateStatus().eCurrState == eState_Precapture)
            m_pStateMgr->sendCmd(r3ACmd);
    }
    else
    {
        m_pStateMgr->sendCmd(r3ACmd);
    }

    m_bIsCapEnd = !fgOldCapQ && m_pStateMgr->isCapQueueEmpty();
    m_pStateMgr->setStartCapture(MFALSE);

#if CAM3_STEREO_FEATURE_EN
    if(m_bIsCapEnd && m_bIsSkipSync3A)
    {
        CAM_LOGD("[%s] m_rLastStereoParam(%d, %d, %d)", __FUNCTION__,
                    m_rLastStereoParam.i4Sync2AMode,
                    m_rLastStereoParam.i4SyncAFMode,
                    m_rLastStereoParam.i4HwSyncMode);
        m_bIsSkipSync3A = MFALSE;
        setStereoParams(m_rLastStereoParam);
        CAM_LOGD("[%s] skip Sync3A for precapture with flash -", __FUNCTION__);
    }
#endif

    CAM_LOGD_IF(m_3ALogEnable,"3A command done.");

    // query magic number after deque sw buffer.
    m_u4MagicNumber = m_pStateMgr->queryMagicNumber();
    CAM_LOGD_IF(m_3ALogEnable,"[%s] m_u4MagicNumber = %d",__FUNCTION__, m_u4MagicNumber);

    do {
        android::sp<ISttBufQ> pSttBufQ;
        android::sp<ISttBufQ::DATA> pData;
        //int BufSize;  /* fix build warning */

        pSttBufQ = NSCam::ISttBufQ::getInstance(m_i4SensorDev);
        if (pSttBufQ == NULL) break;

        pData = pSttBufQ->deque_last();
        if (!pData.get()) {
            CAM_LOGE("SttBufQ empty!!");
            break;
        }
        if(pData->MagicNumberRequest != 0) {
            CAM_LOGW("SttBufQ request=%d -> request=%d", pData->MagicNumberRequest, m_pStateMgr->queryMagicNumberRequest());
        }
        pData->MagicNumberRequest = m_pStateMgr->queryMagicNumberRequest();

        pSttBufQ->enque(pData);
    } while(0);

    AAA_TRACE_D("P1_VLD");
    AAA_TRACE_L(P1_VLD);
    if (ECmd_Update == r3ACmd)
    {
        if (m_pStateMgr->getFrameCount() >= 0)
        {
#if CAM3_FLASH_FEATURE_EN
            FlashMgr::getInstance().doPreviewOneFrame(m_i4SensorDev);
#endif
            ParamIspProfile_T rParamIsp = *pParam;
            rParamIsp.rRequestSet.fgKeep = m_bIsCapEnd;
            if (rParamIsp.rRequestSet.vNumberSet[0] > 0)
            validateP1(rParamIsp, MTRUE);

        }
    }
    AAA_TRACE_END_L;
    AAA_TRACE_END_D;
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
    MINT32 i4SensorDevId __unused = pHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);  /* fix build warning */
    pHalSensor->destroyInstance(callerName);

    CAM_LOGD_IF(m_3ALogEnable, "m_i4SensorDev = %d, senInfo.TgInfo = %d\n", m_i4SensorDev, senInfo.TgInfo);

    if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
    {
        CAM_LOGE("RAW sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
        return E_3A_ERR;
    }

    m_u4TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2

    IAwbMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);
    IAEBufMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);
    IspTuningMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);

    m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&m_i4TgWidth, (MINTPTR)&m_i4TgHeight, 0);
    CAM_LOGD_IF(m_3ALogEnable, "[%s] TG size(%d,%d)", __FUNCTION__, m_i4TgWidth, m_i4TgHeight);
    FlickerHalBase::getInstance().setTGSize(m_i4SensorDev, m_i4TgWidth, m_i4TgHeight);

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
    if (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev))
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
    CAM_LOGD_IF(m_3ALogEnable, "[%s] i4AEISOSpeedMode:%d, rAESensorInfo.u4Sensitivity:%d \n", __FUNCTION__, i4AEISOSpeedMode, rAESensorInfo.u4Sensitivity);

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
Hal3ARawImp::
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
    AE_PLINE_DEBUG_INFO_T temp_AEPlineDebugInfo;
    IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo1.rAEDebugInfo, temp_AEPlineDebugInfo);

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

    // AWB AF ISP debug info
    rDbg3AInfo2.hdr.u4KeyID = ISP_DEBUG_KEYID;
    rDbg3AInfo2.hdr.u4ModuleCount = MODULE_NUM(6,2);

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

    // P1 ISP
    IspTuningMgr::getInstance().getDebugInfoP1(m_i4SensorDev, rDbg3AInfo2.rISPDebugInfo, MFALSE);

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
            IAwbMgr::getInstance().SetAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            IAfMgr::getInstance().SetAETargetMode(m_i4SensorDev, (eAETargetMODE)iArg1);
            m_i4AETargetMode = static_cast<eAETargetMODE>(iArg1);
            break;
        case E3ACtrl_SetAELimiterMode:
            IAeMgr::getInstance().setAELimiterMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAECamMode:
            IAeMgr::getInstance().setAECamMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAEEISRecording:
            IAeMgr::getInstance().enableEISRecording(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetAEPlineLimitation:
            AE_Pline_Limitation_T rLimitParams;
            rLimitParams = *reinterpret_cast<AE_Pline_Limitation_T*>(iArg1);
            IAeMgr::getInstance().modifyAEPlineTableLimitation(m_i4SensorDev, rLimitParams.bEnable,
                                                                              rLimitParams.bEquivalent,
                                                                              rLimitParams.u4IncreaseISO_x100,
                                                                              rLimitParams.u4IncreaseShutter_x100);
            break;
        case E3ACtrl_EnableDisableAE:
            if (iArg1) i4Ret = IAeMgr::getInstance().enableAE(m_i4SensorDev);
            else       i4Ret = IAeMgr::getInstance().disableAE(m_i4SensorDev);
            break;
        case E3ACtrl_SetAEVHDRratio:
            IAeMgr::getInstance().setEMVHDRratio(m_i4SensorDev, (MUINT32)iArg1);
            break;
        case E3ACtrl_EnableAIS:
            IAeMgr::getInstance().enableAISManualPline(m_i4SensorDev, (MBOOL)iArg1);
            break;
        case E3ACtrl_EnableBMDN:
            IAeMgr::getInstance().enableBMDNManualPline(m_i4SensorDev, (MBOOL)iArg1);
            break;
        case E3ACtrl_EnableMFHR:
            IAeMgr::getInstance().enableMFHRManualPline(m_i4SensorDev, (MBOOL)iArg1);
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
        case E3ACtrl_EnableFlareInManualCtrl:
            IAeMgr::getInstance().enableFlareInManualControl(m_i4SensorDev, (MBOOL)iArg1);
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
        case E3ACtrl_GetRTParamsInfo:
            i4Ret = IAeMgr::getInstance().getRTParams(m_i4SensorDev, *reinterpret_cast<FrameOutputParam_T*>(iArg1));
            break;
        case E3ACtrl_GetEvCapture:
            i4Ret = IAeMgr::getInstance().switchCapureDiffEVState(m_i4SensorDev, (MINT8) iArg1,  *reinterpret_cast<strAEOutput*>(iArg2));
            break;
        case E3ACtrl_GetEvSetting:{
            strAEOutput aeOutput;
            i4Ret = IAeMgr::getInstance().switchCapureDiffEVState(m_i4SensorDev, (MINT8) iArg1, aeOutput);
            *reinterpret_cast<strEvSetting*>(iArg2) = aeOutput.EvSetting;
            }break;
        case E3ACtrl_GetCaptureDelayFrame:
            *(reinterpret_cast<MUINT32*>(iArg1)) = IAeMgr::getInstance().get3ACaptureDelayFrame(m_i4SensorDev);
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
        case E3ACtrl_GetSensorDelayInfo:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4AeShutDelayFrame;
            *(reinterpret_cast<MINT32*>(iArg2)) = m_i4AeISPGainDelayFrame;
            break;
        case E3ACtrl_SetCCUCB:
            m_bCCUEn = IAeMgr::getInstance().getCCUresultCBActive(m_i4SensorDev, (reinterpret_cast<MVOID*>(iArg1)));
            CAM_LOGD("[%s()] m_bCCUEn:%d", __FUNCTION__, m_bCCUEn);
            i4Ret = S_3A_OK;
            break;
        case E3ACtrl_EnableTgInt:
            i4Ret = IAeMgr::getInstance().getTgIntAEInfo(m_i4SensorDev, m_bTgIntAEEn, m_fTgIntAERatio);
            *(reinterpret_cast<MBOOL*>(iArg1)) = m_bTgIntAEEn;
            *(reinterpret_cast<MFLOAT*>(iArg2)) = m_fTgIntAERatio;
            CAM_LOGD("[%s()] m_bTgIntAEEn:%d, TG_INT_AE_RATIO:%f", __FUNCTION__, m_bTgIntAEEn, m_fTgIntAERatio);
            break;
        case E3ACtrl_GetIsoSpeed:
            AE_SENSOR_PARAM_T rAESensorInfo;
            MINT32 i4AEISOSpeedMode;
            IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
            i4AEISOSpeedMode = IAeMgr::getInstance().getAEISOSpeedMode(m_i4SensorDev);
            *(reinterpret_cast<MINT32*>(iArg1)) = (i4AEISOSpeedMode ==LIB3A_AE_ISO_SPEED_AUTO) ? rAESensorInfo.u4Sensitivity : i4AEISOSpeedMode;
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
                MINT32 i4AEISOSpeedMode = IAeMgr::getInstance().getAEISOSpeedMode(m_i4SensorDev);
                *(reinterpret_cast<MUINT32*>(iArg2))  = (i4AEISOSpeedMode ==LIB3A_AE_ISO_SPEED_AUTO) ? rDenoiseInfoInfo.u4RealISO : i4AEISOSpeedMode;
            }
            if(pIspInfo->rCamInfo.rAEInfo.u4IspGain != rDenoiseInfoInfo.u4IspGain)
            {
                CAM_LOGD("[%s()] IspGain (%d -> %d)", __FUNCTION__, pIspInfo->rCamInfo.rAEInfo.u4IspGain, rDenoiseInfoInfo.u4IspGain);
                pIspInfo->rCamInfo.rAEInfo.u4IspGain = rDenoiseInfoInfo.u4IspGain;
            }
            break;
#endif
        // ----------------------------------ISP----------------------------------
        case E3ACtrl_GetIspGamma:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA, iArg1, iArg2);
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
        case E3ACtrl_ValidatePass1:
            {
                MINT32 i4Magic = (MINT32)iArg1;
                NSIspTuning::EIspProfile_T prof = NSIspTuning::EIspProfile_Preview;
                RequestSet_T rRequestSet;
                rRequestSet.vNumberSet.clear();
                rRequestSet.vNumberSet.push_back(i4Magic);
                NS3Av3::ParamIspProfile_T _3AProf(prof, i4Magic, 1, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);
                _3AProf.i4MagicNum = i4Magic;
                validateP1(_3AProf,MTRUE);
            }
            break;

        // --------------------------------- AWB ---------------------------------
        case E3ACtrl_SetAwbBypCalibration:
            i4Ret = IAwbMgr::getInstance().CCTOPAWBBypassCalibration(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
            break;
        case E3ACtrl_GetRwbInfo:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_RWB_INFO, iArg1, iArg2);
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
            IAfMgr::getInstance().getDAFtbl(m_i4SensorDev, ((MVOID**)iArg1));
            break;
        case E3ACtrl_SetEnableOIS:
            IAfMgr::getInstance().setOpticalStabilizationMode(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetEnablePBin:
            IAfMgr::getInstance().enablePBIN(m_i4SensorDev, iArg1);
            break;
        case E3ACtrl_SetPureRawData:
            IAfMgr::getInstance().passPDBuffer(m_i4SensorDev, (MVOID*)iArg1);
            break;
#endif

#if CAM3_LSC_FEATURE_EN
        // --------------------------------- Shading ---------------------------------
        case E3ACtrl_SetShadingSdblkCfg:
            if (NSIspTuning::EOperMode_Meta != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev))
                i4Ret = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->CCTOPSetSdblkFileCfg((iArg1 ? MTRUE : MFALSE), reinterpret_cast<const char*>(iArg2));
            break;
        case E3ACtrl_SetShadingEngMode:
            m_fgEnableShadingMeta = MFALSE;
            if (NSIspTuning::EOperMode_Meta != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev))
            {
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
            }
            break;
        case E3ACtrl_SetShadingByp123:
            if (NSIspTuning::EOperMode_Meta != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev))
                ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->CCTOPSetBypass123(iArg1 ? MTRUE : MFALSE);
            break;
#endif

#if CAM3_FLASH_FEATURE_EN
        // --------------------------------- Flash ---------------------------------
        case E3ACtrl_GetQuickCalibration:
            i4Ret = FlashMgr::getInstance().cctGetQuickCalibrationResult(m_i4SensorDev);
            *(reinterpret_cast<MINT32*>(iArg1)) = i4Ret;
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
            CAM_LOGD_IF(m_3ALogEnable, "setIsFlashOnCapture=%ld", (long)iArg1);
            break;
        case E3ACtrl_GetIsFlashOnCapture:
            *(reinterpret_cast<MINT32*>(iArg1)) = FlashMgr::getInstance().isFlashOnCapture(m_i4SensorDev);
            break;
#endif

        // --------------------------------- flow control ---------------------------------
        case E3ACtrl_Enable3ASetParams:
            m_bEnable3ASetParams = iArg1;
            break;
        case E3ACtrl_SetOperMode:
            MINT32 i4OperMode;
            i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            if(i4OperMode != EOperMode_Meta)
                i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
            CAM_LOGD_IF(m_3ALogEnable, "[%s] prev_mode(%d), new_mode(%ld)", __FUNCTION__, i4OperMode, (long)iArg1);
            break;
        case E3ACtrl_GetOperMode:
            *(reinterpret_cast<MUINT32*>(iArg1)) = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            break;
        case E3ACtrl_SetStereoParams:
            setStereoParams(*reinterpret_cast<Stereo_Param_T*>(iArg1));
            break;
        case E3ACtrl_SetIsZsdCapture:
            if(iArg1)
                DebugUtil::getInstance(m_i4SensorDev)->pauseDetect();
            else
                DebugUtil::getInstance(m_i4SensorDev)->resumeDetect();
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
#if CAM3_STEREO_FEATURE_EN
        case E3ACtrl_GetDualZoomInfo:
            DualZoomInfo_T* rDualZoomInfo;
            AE_MODE_CFG_T rAeInfo;
            i4Ret = IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, rAeInfo);
            rDualZoomInfo = reinterpret_cast<DualZoomInfo_T*>(iArg1);
            rDualZoomInfo->i4AELv_x10 = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
            rDualZoomInfo->i4AEIso = rAeInfo.u4RealISO;
            rDualZoomInfo->bIsAEBvTrigger= IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);
            rDualZoomInfo->bIsAEAPLock= IAeMgr::getInstance().IsAPAELock(m_i4SensorDev);
            rDualZoomInfo->i4AFDAC = IAfMgr::getInstance().getAFPos(m_i4SensorDev);
            rDualZoomInfo->bAFDone = IAfMgr::getInstance().isFocusFinish(m_i4SensorDev);
            rDualZoomInfo->bSyncAFDone = ISync3AMgr::getInstance()->getSync3A()->isAFSyncFinish();
            rDualZoomInfo->bSync2ADone = ISync3AMgr::getInstance()->getSync3A()->is2ASyncFinish();
#if CAM3_AF_FEATURE_EN
            mcuMotorOISInfo rOisInfo;
            rDualZoomInfo->bOISIsValid = (MCUDrv::createInstance(0)->getMCUOISInfo(&rOisInfo, m_i4SensorDev) == 0) ? 1 : 0;
            rDualZoomInfo->fOISHallPosXum  = (MFLOAT)rOisInfo.i4OISHallPosXum / (MFLOAT)rOisInfo.i4OISHallFactorX;
            rDualZoomInfo->fOISHallPosYum  = (MFLOAT)rOisInfo.i4OISHallPosYum / (MFLOAT)rOisInfo.i4OISHallFactorY;
#endif
            break;
        case E3ACtrl_Sync3A_IsActive:
            *(reinterpret_cast<MBOOL*>(iArg1)) = ISync3AMgr::getInstance()->isActive();
            break;
        case E3ACtrl_Sync3A_Sync2ASetting:
            NS3Av3::ISync3AMgr::getInstance()->getSync3A()->sync2ASetting((MINT32)iArg1, (MINT32)iArg1);
            break;
#endif
        case E3ACtrl_GetAF2AEInfo:
            updateAFinfo(*reinterpret_cast<AF2AEInfo_T*>(iArg1));
            break;
        case E3ACtrl_SetAF2AEInfo:
        {
            MINT32 SttMagicNum = (MINT32)m_pStateMgr->queryMagicNumber();
            AF2AEInfo_T curinfo = queryAFinfo(SttMagicNum);
            CAM_LOGD_IF(m_3ALogEnable, "Current AF2AEInfo_T i4MagicNum = %d, i4IsAFDone = %d, i4AfDac = %d, i4IsSceneStable = %d"
                , curinfo.i4MagicNum, curinfo.i4IsAFDone, curinfo.i4AfDac, curinfo.i4IsSceneStable);

            IAeMgr::getInstance().UpdateAF2AEInfo(m_i4SensorDev,curinfo);
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
    m_pStateMgr->attachCb(cb);
    return 0;
}

MINT32
Hal3ARawImp::
detachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pStateMgr->detachCb(cb);
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
    //MUINT32 u4PDAF_Support = sensorStaticInfo.PDAF_Support;  /* fix build warning */


    m_i4SensorPreviewDelay = sensorStaticInfo.previewDelayFrame;
    m_i4AeShutDelayFrame = sensorStaticInfo.aeShutDelayFrame;
    m_i4AeISPGainDelayFrame = sensorStaticInfo.aeISPGainDelayFrame;

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), rawFmtType(%d), SensorPreviewDelay(%d), AeShutDelayFrame(%d), AeISPGainDelayFrame(%d)\n", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, u4RawFmtType, m_i4SensorPreviewDelay, m_i4AeShutDelayFrame, m_i4AeISPGainDelayFrame);

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
    IAfMgr::getInstance().CamPwrOnState(m_i4SensorDev);

    //FLASH init
#if CAM3_FLASH_FEATURE_EN
    AAA_TRACE_D("FLASH init");
    FlashMgr::getInstance().init(m_i4SensorDev, m_i4SensorIdx);
    FlashMgr::getInstance().hasFlashHw(m_i4SensorDev, m_i4HWSuppportFlash);
    CAM_LOGD("[%s] m_i4SensorDev(%d) m_i4HWSuppportFlash(%d)", __FUNCTION__, m_i4SensorDev, m_i4HWSuppportFlash);
    AAA_TRACE_END_D;
#endif

    return MTRUE;
}


MBOOL
Hal3ARawImp::
notifyPwrOff()
{
    IAfMgr::getInstance().CamPwrOffState(m_i4SensorDev);

#if CAM3_FLASH_FEATURE_EN
    //FLASH uninit
    FlashMgr::getInstance().uninit(m_i4SensorDev);
#endif

    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyP1PwrOn()
{
    CAM_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
    m_pICcuMgr = NSCcuIf::ICcuMgrExt::createInstance("3AHal");
    if(m_pICcuMgr->ccuInit() != 0)
    {
        CAM_LOGD("m_pICcuMgr->ccuInit() fail");
        return MTRUE;
    }
    CAM_LOGD("m_pICcuMgr->ccuInit() success");
    if(m_pICcuMgr->ccuBoot() != 0)
    {
        CAM_LOGD("m_pICcuMgr->ccuBoot() fail");
        return MTRUE;
    }
    CAM_LOGD("m_pICcuMgr->ccuBoot() success");
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyP1PwrOff()
{
    CAM_LOGD("[%s] notifyP1PwrOff", __FUNCTION__);
    if (m_pICcuMgr != NULL)
    {
        if(m_pICcuMgr->ccuShutdown() != 0)
        {
            CAM_LOGD("m_pICcuMgr->ccuShutdown() fail");
            return MTRUE;
        }

        CAM_LOGD("m_pICcuMgr->ccuShutdown() success");

        if(m_pICcuMgr->ccuUninit() != 0)
        {
            CAM_LOGD("m_pICcuMgr->ccuUninit() fail");
            return MTRUE;
        }

        CAM_LOGD("m_pICcuMgr->ccuUninit() success");

        m_pICcuMgr->destroyInstance();
        m_pICcuMgr = NULL;
    }
    return MTRUE;
}

MVOID
Hal3ARawImp::
setIspSensorInfo2AF(MINT32 MagicNum, Result_T& Result)
{
    ISP_SENSOR_INFO_T rIspSensorInfo;
    rIspSensorInfo.i4FrameId = MagicNum;
    rIspSensorInfo.bHLREnable = MFALSE;
    rIspSensorInfo.bAEStable = Result.rCamInfo.rAEInfo.bAEStable;
    rIspSensorInfo.bAELock = Result.rCamInfo.rAEInfo.bAELock;
    rIspSensorInfo.bAEScenarioChange = Result.rCamInfo.rAEInfo.bAEScenarioChange;
    rIspSensorInfo.i4deltaIndex = Result.rCamInfo.rAEInfo.i4deltaIndex;
    rIspSensorInfo.u4AfeGain = Result.rCamInfo.rAEInfo.u4AfeGain;
    rIspSensorInfo.u4IspGain = Result.rCamInfo.rAEInfo.u4IspGain;
    rIspSensorInfo.u4Eposuretime = Result.rCamInfo.rAEInfo.u4Eposuretime;
    rIspSensorInfo.u4RealISOValue = Result.rCamInfo.rAEInfo.u4RealISOValue;
    rIspSensorInfo.u4MaxISO = Result.rCamInfo.rAEInfo.u4MaxISO;
    rIspSensorInfo.u4OrgExposuretime = Result.rCamInfo.rAEInfo.u4OrgExposuretime;
    rIspSensorInfo.u4OrgRealISOValue = Result.rCamInfo.rAEInfo.u4OrgRealISOValue;
    rIspSensorInfo.u4AEStableCnt = Result.rCamInfo.rAEInfo.u4AEStableCnt;
    rIspSensorInfo.u4AEFinerEVIdxBase = Result.rCamInfo.rAEInfo.u4AEFinerEVIdxBase;
    rIspSensorInfo.u4AEidxCurrentF = Result.rCamInfo.rAEInfo.u4AEidxCurrentF;

    IAfMgr::getInstance().setIspSensorInfo2AF(m_i4SensorDev, rIspSensorInfo);
    CAM_LOGD_IF(m_3ALogEnable,"[%s] FrameId:%d AFEGain:%d ISPGain:%d", __FUNCTION__,rIspSensorInfo.i4FrameId, rIspSensorInfo.u4AfeGain, rIspSensorInfo.u4IspGain);
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
setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{

    if( rNewP2Param.rScaleCropRect.i4Xwidth != 0 && rNewP2Param.rScaleCropRect.i4Yheight != 0 )
        IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewP2Param.rScaleCropRect.i4Xoffset,rNewP2Param.rScaleCropRect.i4Yoffset,rNewP2Param.rScaleCropRect.i4Xwidth,rNewP2Param.rScaleCropRect.i4Yheight);
    if( rNewP2Param.rScaleCropRect.i4Xwidth != 0 && rNewP2Param.rScaleCropRect.i4Yheight != 0 )
        IAwbMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewP2Param.rScaleCropRect.i4Xoffset,rNewP2Param.rScaleCropRect.i4Yoffset,rNewP2Param.rScaleCropRect.i4Xwidth,rNewP2Param.rScaleCropRect.i4Yheight);
    // AE for Denoise OB2
    IAeMgr::getInstance().enableStereoDenoiseRatio(m_i4SensorDev, rNewP2Param.i4DenoiseMode);

    //ISP
    IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewP2Param.i4BrightnessMode);
    IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewP2Param.i4HueMode);
    IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewP2Param.i4SaturationMode);
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewP2Param.i4EdgeMode);
    IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewP2Param.i4ContrastMode);

    //IspTuningMgr::getInstance().setToneMapMode(m_i4SensorDev, rNewP2Param.u1TonemapMode);
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
            CAM_LOGD_IF(m_3ALogEnable, "[Red]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
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
            CAM_LOGD_IF(m_3ALogEnable, "[Green]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
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
            CAM_LOGD_IF(m_3ALogEnable, "[Blue]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Blue(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);
    }
    return MTRUE;
}

MBOOL
Hal3ARawImp::
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
            CAM_LOGD_IF(m_3ALogEnable, "[Blue](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveBlue.push_back(*pIn++);
            pResultP2->vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_3ALogEnable, "[Green](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveGreen.push_back(*pIn++);
            pResultP2->vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_3ALogEnable, "[Red](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveRed.push_back(*pIn++);
            pResultP2->vecTonemapCurveRed.push_back(*pOut++);
        }
        CAM_LOGD_IF(m_3ALogEnable,"[%s] rsize(%d) gsize(%d) bsize(%d)"
            ,__FUNCTION__, (MINT32)pResultP2->vecTonemapCurveRed.size(), (MINT32)pResultP2->vecTonemapCurveGreen.size(), (MINT32)pResultP2->vecTonemapCurveBlue.size());

    }
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyResult4TG(MINT32 i4PreFrmId, Result_T *rPreResult)
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] Previous FrmId(%d) rPreResult Addr(%p) +", __FUNCTION__, i4PreFrmId, rPreResult);

    // set AE info to previous result Q
    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rPreResult->i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rPreResult->i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rPreResult->i4SensorSensitivity = rAESensorInfo.u4Sensitivity;//ISO value

    // set new LSC ratio to previous result Q
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
    MUINT32 u4Rto = pLsc->getRatio();
    if(rPreResult->vecLscData.size() != 0)
    {
        MUINT8* pDest = rPreResult->vecLscData.editArray();
        *((MUINT32*)(pDest+sizeof(ILscTable::Config))) = u4Rto;
    }

    // set OBCGain to exif
    MINT32 OBCGain = ISP_MGR_OBC_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).getOBCGain();

    CAM_LOGD_IF(m_3ALogEnable, "[%s] u4Rto(%d), OBCGain(%d), i8SensorExposureTime(%ld), i8SensorFrameDuration(%ld), i4SensorSensitivity(%d)", __FUNCTION__,
        u4Rto, OBCGain, (long)rPreResult->i8SensorExposureTime, (long)rPreResult->i8SensorFrameDuration, rPreResult->i4SensorSensitivity);

    CAM_LOGD_IF(m_3ALogEnable, "[%s] vecExifInfo.size(%d), vecDbg3AInfo.size(%d), vecDbgIspInfo.size(%d), m_rParam.u1IsGetExif(%d), m_bIsCapEnd(%d)", __FUNCTION__,
        (MINT32)rPreResult->vecExifInfo.size(), (MINT32)rPreResult->vecDbg3AInfo.size(), (MINT32)rPreResult->vecDbgIspInfo.size(), m_rParam.u1IsGetExif, m_bIsCapEnd);

    if (!IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rPreResult->rCamInfo))
    {
        CAM_LOGE("Fail to get CamInfo");
    }

    CAM_LOGD_IF(m_3ALogEnable,"[%s] u4Rto(%d), u4RealISOValue(%d), u4Eposuretime(%d), u4AfeGain(%d), u4IspGain(%d)", __FUNCTION__,
            u4Rto,rPreResult->rCamInfo.rAEInfo.u4RealISOValue
            ,rPreResult->rCamInfo.rAEInfo.u4Eposuretime ,rPreResult->rCamInfo.rAEInfo.u4AfeGain, rPreResult->rCamInfo.rAEInfo.u4IspGain);

    if (m_rParam.u1IsGetExif || m_bIsCapEnd)
    {
        if( rPreResult->vecExifInfo.size() > 0 && rPreResult->vecDbg3AInfo.size() > 0 && rPreResult->vecDbgIspInfo.size() > 0)
        {
            // standard exif
            get3AEXIFInfo(rPreResult->vecExifInfo.editTop());

            // debug exif
            AAA_DEBUG_INFO1_T& rDbg3AInfo1 = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(rPreResult->vecDbg3AInfo.editArray());
            AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rPreResult->vecDbgIspInfo.editArray());
            NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;

            AE_PLINE_DEBUG_INFO_T tmpBuff;
            // AE
            IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, rDbg3AInfo1.rAEDebugInfo, tmpBuff);

#if CAM3_LSC_FEATURE_EN
            // LSC
            //Avoid NE for LSC debug info
            //ILscMgr::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
            rDbg3AInfo1.rShadigDebugInfo.Tag[SHAD_TAG_CAM_LSC_RATIO].u4FieldValue = u4Rto;
#endif

            // P1 ISP
            setP1DbgInfo4TG(rDbgIspInfo, u4Rto, OBCGain);
        }

        CAM_LOGD_IF(m_3ALogEnable,"[%s] Update rPreResult exif done", __FUNCTION__);
    }

    CAM_LOGD_IF(m_3ALogEnable,"[%s] Previous FrmId(%d) rPreResult Addr(%p) rPreResult i4FrmId(%d) -", __FUNCTION__, i4PreFrmId, rPreResult, rPreResult->i4FrmId);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setP1DbgInfo4TG(NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo, MUINT32 u4Rto, MINT32 OBCGain) const
{
    IspTuningMgr::getInstance().setDebugInfo4TG(m_i4SensorDev, u4Rto, OBCGain, rDbgIspInfo);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setTG_INTParams()
{
    CAM_LOGD_IF(m_3ALogEnable, "[%s] + ", __FUNCTION__);

    //MINT32 i4Ret = 0;  /* fix build warning */

    if(m_pCamIO != NULL)
    {
        // 1. get OBC gain from isp_mgr_obc
        MINT32 OBCGain = ISP_MGR_OBC_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).getOBCGain();

        // 2. get ratio
        NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
        MUINT32 u4Rto = (MINT32)pLsc->getRatio();

        // 2.1 updateTSF
        MUINT32 u4MagicNumReq = m_pStateMgr->queryMagicNumberRequest();
        CAM_LOGD_IF(m_3ALogEnable,"[%s] u4MagicNumReq(%d), u4Rto(%d) OBCGain(%d)", __FUNCTION__, u4MagicNumReq, u4Rto, OBCGain);
        pLsc->updateRatio(u4MagicNumReq, u4Rto);//request magicnum, ratio

        // 3. call normalpipe API to set obc gain & ratio
        // CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
        std::list<ECQ_TUNING_USER> module_en;
        CQ_TUNING_CMD_ST cmd;

        //OBCGain
        CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_OBC_GAIN0, OBCGain);
        CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_OBC_GAIN1, OBCGain);
        CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_OBC_GAIN2, OBCGain);
        CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_OBC_GAIN3, OBCGain);

        //Ratio
        CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_LSC_RATIO_0, ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto));
        CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_LSC_RATIO_1, ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto));

        module_en.push_back(EAA_TUNING_OB);
        module_en.push_back(EAA_TUNING_LSC);
        cmd.module = &module_en;
        cmd.serial = (u4MagicNumReq);
        m_pCqTuningMgr->update(cmd);
#endif

    }
    else
        return MFALSE;

    CAM_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
    return MTRUE;
}

/*MUINT32
Hal3ARawImp::
Scenario4AEAWB(Param_T const &rParam)
{
#if CAM3_STEREO_FEATURE_EN

    Stereo_Param_T StereoMode = ISync3AMgr::getInstance()->getStereoParams();

    if (StereoMode.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF ||
        StereoMode.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE ||
        StereoMode.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        return CAM_SCENARIO_CUSTOM4;

#endif

    if(rParam.u1HdrMode == 1 && m_i4AETargetMode != AE_MODE_NORMAL)
        return CAM_SCENARIO_CUSTOM1;
    else if(rParam.u1HdrMode == 2 && m_i4AETargetMode != AE_MODE_NORMAL)
        return CAM_SCENARIO_CUSTOM2;
    else{
        switch(m_u4SensorMode){
            case NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                return CAM_SCENARIO_PREVIEW;
            case NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                return CAM_SCENARIO_CAPTURE;
            case NSCam::SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                return CAM_SCENARIO_VIDEO;
            default:
                return CAM_SCENARIO_PREVIEW;
        }
    }
}

MUINT32
Hal3ARawImp::
Scenario4AF(Param_T const &rParam)
{
#if CAM3_STEREO_FEATURE_EN
    if ((ISync3AMgr::getInstance()->getStereoParams()).i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF)
        return CAM_SCENARIO_CUSTOM4;
#endif

    switch(m_u4SensorMode){
        case NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
        {
            if (rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD)
                return CAM_SCENARIO_CUSTOM3;
            else
                return CAM_SCENARIO_PREVIEW;
        }
        case NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            return CAM_SCENARIO_CAPTURE;
        case NSCam::SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            return CAM_SCENARIO_VIDEO;
        default:
            return CAM_SCENARIO_PREVIEW;
    }
}*/

MBOOL
Hal3ARawImp::
notify4CCU(MUINT32 u4PreFrmId, Result_T *rPreResult)
{
    if(m_bCCUEn && m_pThreadSetCCUPara)
    {
        AE_SENSOR_PARAM_T rAESensorInfo;
        IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);

        CAM_LOGD_IF(m_3ALogEnable, "[%s] , i8SensorExposureTime(%ld), i8SensorFrameDuration(%ld), i4SensorSensitivity(%d)", __FUNCTION__,
        (long)rAESensorInfo.u8ExposureTime, (long)rAESensorInfo.u8FrameDuration, rAESensorInfo.u4Sensitivity);

        ThreadSetCCUPara::Cmd_T rCmd(u4PreFrmId, rPreResult, m_rParam.u1IsGetExif, m_bIsCapEnd, rAESensorInfo);
        m_pThreadSetCCUPara->postCmd(&rCmd);
    }

    return MTRUE;
}

MVOID*
Hal3ARawImp::
ThreadAEStart(MVOID* arg)
{
    AAA_TRACE_D("AE Start");
    ::prctl(PR_SET_NAME, "ThreadAEStart", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = NICE_CAMERA_AE_Start;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);
    //::setpriority(PRIO_PROCESS, 0, priority);

    Hal3ARawImp *_this = static_cast<Hal3ARawImp*>(arg);
    IAeMgr::getInstance().setSensorMode(_this->m_i4SensorDev, _this->m_u4SensorMode, _this->m_i4HbinWidth, _this->m_i4HbinHeight);
    IAeMgr::getInstance().setAAOMode(_this->m_i4SensorDev, _this->m_bAAOMode);
    MRESULT err = IAeMgr::getInstance().Start(_this->m_i4SensorDev);
    //IAeMgr::getInstance().setCamScenarioMode(u4SensorDev, rCmd.u4Scenario4AEAWB);
    if (FAILED(err)) {
        CAM_LOGE("IAeMgr::getInstance().Start() fail\n");
    }
    AAA_TRACE_END_D;
    ::pthread_exit((MVOID*)0);
}

MVOID*
Hal3ARawImp::
ThreadAFStart(MVOID* arg)
{
    AAA_TRACE_D("AF Start");
    ::prctl(PR_SET_NAME, "ThreadAFStart", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = NICE_CAMERA_AF_Start;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);
    //::setpriority(PRIO_PROCESS, 0, priority);

    Hal3ARawImp *_this = static_cast<Hal3ARawImp*>(arg);
    //IAfMgr::getInstance().setSensorMode(_this->m_i4SensorDev, _this->m_u4SensorMode, _this->u4AFWidth, _this->u4AFHeight);
    IAfMgr::getInstance().setCamScenarioMode(_this->m_i4SensorDev, Scenario4AF(_this->m_sParam));
    MRESULT err = IAfMgr::getInstance().Start(_this->m_i4SensorDev);
    if (FAILED(err)) {
        CAM_LOGE("AfMgr::getInstance().Start() fail\n");
    }
    AAA_TRACE_END_D;
    ::pthread_exit((MVOID*)0);
}

MVOID*
Hal3ARawImp::
ThreadIspSetBpciTable(MVOID* arg)
{
    AAA_TRACE_D("ThreadIspSetBpciTable");
    ::prctl(PR_SET_NAME, "ThreadIspSetBpciTable", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = NICE_CAMERA_ISP_BPCI;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);

    Hal3ARawImp *_this = static_cast<Hal3ARawImp*>(arg);

    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return NULL;
    }
    pHalSensorList->querySensorStaticInfo(_this->m_i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
    MUINT32 u4PDAF_Support = sensorStaticInfo.PDAF_Support;
    CAM_LOGD("[%s] SensorDev(%d), rawFmtType(%d), PDAF_Support(%d)\n", __FUNCTION__, _this->m_i4SensorDev, u4RawFmtType, u4PDAF_Support);

    // 3A/ISP mgr can query sensor static information here
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(_this->m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE, u4RawFmtType, u4PDAF_Support);

    AAA_TRACE_END_D;
    ::pthread_exit((MVOID*)0);
}

MVOID
Hal3ARawImp::
updateAFinfo(AF2AEInfo_T& AF2AEinfo)
{
    AFinfovec[AF2AEinfo.i4MagicNum%AFinfovec.size()] = AF2AEinfo;
    return ;
}

AF2AEInfo_T
Hal3ARawImp::
queryAFinfo(const MINT32& sttMagicNum)
{
    AF2AEInfo_T query = AFinfovec[sttMagicNum%AFinfovec.size()];

    if (query.i4MagicNum != sttMagicNum)
    {
        CAM_LOGD_IF(m_3ALogEnable, "[%s] Hal3A have no AF info with %d, the number = %d", __FUNCTION__, sttMagicNum, query.i4MagicNum);
        query.i4MagicNum = -1;
    }

    return query;
}

