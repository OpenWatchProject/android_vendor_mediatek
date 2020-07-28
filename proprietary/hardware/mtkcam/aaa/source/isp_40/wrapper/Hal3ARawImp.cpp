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
#include <array>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <mtkcam/utils/std/ULog.h>
#include <IThreadRaw.h>
#include <mtkcam/drv/IHalSensor.h>
#include <Hal3ACbHub.h>
#include <private/aaa_utils.h>

#include "Hal3ARaw.h"
#include <ae_mgr.h>
#include <awb_mgr_if.h>
#include <aaa_sensor_mgr.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/sys/IFileCache.h>

#include <isp_mgr.h>
// Task header
#include <task/ITaskMgr.h>
#include <IThread.h>

#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <dbg_aaa_param.h>
#include <ISync3A.h>
#include <aaa_hal_sttCtrl.h>
#include <DebugUtil.h>
#include <aaa_common_custom.h>
#include <Hal3APolicy.h>
#if CAM3_AF_FEATURE_EN
#include <dbg_af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_define.h>
#include <af_mgr_if.h>
#endif

#if CAM3_FLASH_FEATURE_EN
#include <flash_hal.h>
#include <flash_mgr.h>
#endif
#include <flash_feature.h>

#if CAM3_FLICKER_FEATURE_EN
#include <flicker_hal_if.h>
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

#if (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
#include "private/PDTblGen.h"
#endif

#include <android/sensor.h>                     // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener

#include <IResultPool.h>
#include <mtkcam/aaa/IDngInfo.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <nvbuf_util.h>
#include "camera_custom_msdk.h"

// Index manager
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include <EModule_string.h>
#include <EApp_string.h>
#include <EIspProfile_string.h>
#include <ESensorMode_string.h>
#endif

//Thread Use
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#if CAM3_ICCU_EN
#include "iccu_mgr.h"
#endif

using namespace NSCcuIf;
#include "SttBufQ.h"
#include <faces.h>

// DP5.0 check sum
#include "aaa_exif_tag_chksum.h"
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>

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

#define MY_FORCEINLINE inline __attribute__((always_inline))

// define g/gyro info
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

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
static Mutex& gCommonLock = *new Mutex();

using namespace std;
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

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuningv3;
using namespace NSIspExifDebug;
using namespace NSCam::TuningUtils;

class Hal3ARawImp : public Hal3ARaw
{
public:
    static I3AWrapper*  getInstance(MINT32 const i4SensorOpenIndex);
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
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/, MINT32 i4P1DoneSttNum = -1);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond() const;
    virtual MBOOL       chkPreFlashOnCond() const;
    virtual MBOOL       chkLampFlashOnCond() const;
    virtual MBOOL       isStrobeBVTrigger() const;
    virtual MBOOL       isFlashOnCapture() const;
    virtual MBOOL       chkCapFlash() const {return m_bIsHighQualityCaptureOn;}
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, MINT32 i4SubsampleCount = 0) const;
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
    virtual MBOOL       dumpP1Params(MINT32 i4MagicNum);
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type);
    virtual MBOOL       preset(Param_T const &rNewParam);
    virtual MVOID       querySensorStaticInfo();
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum);
    virtual MBOOL       notifyResult4TG(MINT32 i4PreFrmId);
    virtual MBOOL       notify4CCU(MUINT32 u4PreFrmId, ISP_NVRAM_OBC_T const &OBCResult);
    virtual MVOID       notifyPreStop();
    static  MVOID*      ThreadAEStart(MVOID*);
    static  MVOID*      ThreadAFStart(MVOID*);

public:  //    Ctor/Dtor.
                        Hal3ARawImp(MINT32 const i4SensorIdx);
    virtual             ~Hal3ARawImp(){}

protected:
                        Hal3ARawImp(const Hal3ARawImp&);
                        Hal3ARawImp& operator=(const Hal3ARawImp&);

    MBOOL               init(MINT32 const i4SensorOpenIndex);
    MBOOL               uninit();
    MRESULT             updateTGInfo();
    MINT32              config(const ConfigInfo_T& rConfigInfo);
    MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;
    MBOOL               setP1DbgInfo4TG(NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo, MUINT32 u4Rto, MINT32 OBCGain) const;
    MBOOL               setTG_INTParams();
    // Flash flow
    MBOOL               checkAndOpenFlash(MINT32 i4Type);
    MBOOL               checkAndCloseFlash();
    MVOID               doBackup2A();
    MVOID               doRestore2A();
    MVOID               handleBadPicture(MINT32 i4ReqMagic, MINT32 i4P1DoneSttNum = -1);
#if MTK_CAM_NEW_NVRAM_SUPPORT
    MUINT32             getNvramIndex(const CAM_IDX_QRY_COMB& rMappingInfo, EModule_T module, MUINT32& index);
#endif

private:
    MINT32              m_3ALogEnable;
    MINT32              m_i4IdxCacheLogEnable;
    MINT32              m_i4CopyLscP1En;
    MINT32              m_3ACctEnable;
    MINT32              m_DebugLogWEn;
    volatile int        m_Users;
    mutable Mutex       m_Lock;
    Mutex               mP2Mtx;
    Mutex               m3AOperMtx1;
    Mutex               m3AOperMtx2;
    MINT32              m_i4SensorIdx;
    MINT32              m_i4SensorDev;
    MINT32              m_i4sensorId;
    MUINT32             m_u4SensorMode;
    MUINT               m_u4TgInfo;
    MBOOL               m_bAAOMode;
    MBOOL               m_bEnable3ASetParams;
    MBOOL               m_bFaceDetectEnable;
    MINT32              m_i4TgWidth;
    MINT32              m_i4TgHeight;
    MBOOL               mbIsHDRShot;
    MINT32              m_i4HbinWidth;
    MINT32              m_i4HbinHeight;
    MBOOL               m_fgEnableShadingMeta;
    MBOOL               m_bIsRecordingFlash;
    MINT32              m_i4EVCap;
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
    MINT32              m_u4LastRequestNumber;
    MBOOL               m_bFlashOpenedByTask;
    MINT32              m_i4FlashType;
    MBOOL               m_bFrontalBin;
    MUINT8              m_u1LastCaptureIntent;
    pthread_t           m_ThreadAE;
    pthread_t           m_ThreadAF;

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IspTuningMgr*       m_pTuning;
    IThreadRaw*         m_pThreadRaw;
    ITaskMgr*           m_pTaskMgr;
#if CAM3_ICCU_EN
    NSCcuIf::ICcuMgrExt* m_pICcuMgr;
#endif
    Param_T             m_rParam;
    AF_Param_T          m_rAfParam;
    ScenarioParam       m_sParam;
    ScenarioParam       m_sPreParam;
    I3ACallBack*        m_pCbSet;
    MINT32              m_i4SttMagicNumber;
    MBOOL               m_bIsHighQualityCaptureOn;
    MBOOL               m_bIsCapEnd;
    MBOOL               m_bIsFlashOpened;
    MBOOL               m_bIsSkipSync3A;
    MINT32              m_i4StereoWarning;
    vector<MINT32>      m_vShortExpFrame;
    MINT32              m_i4ShortExpCount;
    MINT32              m_i4BatchModeCount;
    MBOOL               m_bIsSMVRTuning;
    ConfigInfo_T        m_rConfigInfo;
    MBOOL               m_bDbgInfoEnable;

#if CAM3_STEREO_FEATURE_EN
    Stereo_Param_T m_rLastStereoParam;
#endif

    //CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    CqTuningMgr*        m_pCqTuningMgr;
#endif
    ThreadSetCCUPara*   m_pThreadSetCCUPara;
    IResultPool*        m_pResultPoolObj;
    Hal3ACbHub*         m_pCallbackHub;
#if MTK_CAM_NEW_NVRAM_SUPPORT
    IdxMgr*             m_pIdxMgr;
    MBOOL               m_bMappingQueryFlag_3A;
    CAM_IDX_QRY_COMB    m_Mapping_Info_3A;
    Query_3A_Index_T    m_3A_Index;
#endif
    MINT32              m_faceNum;
    MBOOL               m_bIsFirstSetParams;
    MINT32              m_i4CamModeEnable;
    Hal3ASttCtrl*       m_p3ASttCtrl;
    MINT32              m_i4Count;
    MUINT32             m_i4EffectiveFrame;
};

#define MY_INST NS3Av3::INST_T<Hal3ARawImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

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
    MY_LOGD("[%s]", __FUNCTION__);
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
    , m_fgEnableShadingMeta(MTRUE)
    , m_bIsRecordingFlash(MFALSE)
    , m_i4EVCap(0)
    , m_i4SensorPreviewDelay(0)
    , m_i4AeShutDelayFrame(0)
    , m_i4AeISPGainDelayFrame(0)
    , m_bPreStop(MFALSE)
    , m_i4OverrideMinFrameRate(0)
    , m_i4OverrideMaxFrameRate(0)
    , m_bCCUEn(0)
    , m_bTgIntAEEn(0)
    , m_fTgIntAERatio(0)
    , m_pCamIO(NULL)
    , m_pTuning(NULL)
    , m_pThreadRaw(NULL)
    , m_pTaskMgr(NULL)
#if CAM3_ICCU_EN
    , m_pICcuMgr(NULL)
#endif
    , m_pCbSet(NULL)
    , m_i4SttMagicNumber(0)
    , m_bIsHighQualityCaptureOn(MFALSE)
    , m_bIsCapEnd(MFALSE)
    , m_bIsFlashOpened(MFALSE)
    , m_bIsSkipSync3A(MFALSE)
    , m_i4StereoWarning(0)
    , m_i4ShortExpCount(0)
    , m_i4BatchModeCount(1)
    , m_bDbgInfoEnable(MFALSE)
    //CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    , m_pCqTuningMgr(NULL)
#endif
    , m_pThreadSetCCUPara(NULL)
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
#if MTK_CAM_NEW_NVRAM_SUPPORT
    , m_pIdxMgr(IdxMgr::createInstance(static_cast<MUINT32>(m_i4SensorDev)))
#endif
    , m_faceNum(0)
    , m_bIsFirstSetParams(MTRUE)
    , m_i4CamModeEnable(0)
    , m_p3ASttCtrl(NULL)
#if MTK_CAM_NEW_NVRAM_SUPPORT
    , m_bMappingQueryFlag_3A(MTRUE)
#endif
    , m_i4Count(4)
    , m_bIsSMVRTuning(MFALSE)
    , m_i4EffectiveFrame(0)
{
    m_i4SensorDev = NS3Av3::mapSensorIdxToDev(i4SensorIdx);
    m_vShortExpFrame = {0, 1, 0, -1};   // The fourth entry is to prevent segmentation fault
    MY_LOGD("[%s] sensorIdx(%d), sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);
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

    MY_LOGD("[%s] m_Users: %d, SensorDev %d, index %d \n", __FUNCTION__, m_Users, m_i4SensorDev, m_i4SensorIdx);

    //read module id and update data only one time
    if(!gTuningInitCtrl[m_i4SensorIdx].bHadModuleId && gTuningInitCtrl[m_i4SensorIdx].bIsFirstTime)
    {
        MY_LOGD("[%s] Read buf again for new module id \n", __FUNCTION__);
        void *p, *q, *r, *s, *t, *u;
        NvBufUtil::getInstance().getSensorIdAndModuleId(m_i4SensorDev, m_i4SensorIdx, m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);

        openCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);

        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4SensorDev, p, MTRUE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_3A getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_STROBE, m_i4SensorDev, q, MTRUE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_STROBE getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, m_i4SensorDev, r, MTRUE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_FLASH_CALIBRATION getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, m_i4SensorDev, s, MTRUE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_ISP getBufAndRead ERROR \n", __FUNCTION__);
        err=NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_i4SensorDev, t, MTRUE);
        if(err != 0)
            MY_LOGE("[%s] CAMERA_NVRAM_DATA_SHADING getBufAndRead ERROR \n", __FUNCTION__);
    } else {
        if(!gTuningInitCtrl[m_i4SensorIdx].bIsFirstTime && m_Users!=0)
            lockCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);
    }

    // check user count
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOGD("[%s] %d has created \n", __FUNCTION__, m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_fgEnableShadingMeta = MTRUE;
    m_bIsSMVRTuning = MFALSE;

    // init Thread and state mgr
    m_pThreadRaw = IThreadRaw::createInstance(this, m_i4SensorDev, m_i4SensorIdx);

    MY_LOGD("[%s] 2D", __FUNCTION__);
    m_pTaskMgr = ITaskMgr::create(m_i4SensorDev);

    // CCT init
    /*GET_PROP("vendor.3a.cct.enable", 0, m_3ACctEnable);
    if ( m_3ACctEnable == 1 ) {
        MBOOL ret1, ret2;
        // 1. delete previous CCT Server
        MY_LOGD("CCT before init");
        ret1 = CctSvrEnt_Ctrl(CCT_SVR_CTL_STOP_SERVER_THREAD);
        ret2 = CctSvrEnt_DeInit();
        //MY_LOGD("CCT before init: ret = %d, %d", ret1, ret2);

        // 2. get sensor index for CCT Server
        MINT32 cct_sensor_index = 1;
        GET_PROP("vendor.3a.cct.sensor.index", 1, cct_sensor_index);

        // 3. init CCT Server
        MY_LOGD("CCT init: sensor = %d", cct_sensor_index);
        ret1 = CctSvrEnt_Init(cct_sensor_index);
        ret2 = CctSvrEnt_Ctrl(CCT_SVR_CTL_START_SERVER_THREAD);
        //MY_LOGD("CCT init: ret = %d, %d", ret1, ret2);
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

    // TuningMgr init
    if (m_pTuning == NULL)
    {
        AAA_TRACE_D("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx))
        {
            MY_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_3A_HAL("Fail to init IspTuningMgr");
            AAA_TRACE_END_D;
            return MFALSE;
        }
        AAA_TRACE_END_D;
    }
    //ILscMgr::getInstance((MUINT32)m_i4SensorDev)->setFactoryMode(MTRUE);

    // ResultPool - Get result pool object
    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(m_pResultPoolObj == NULL)
        MY_LOGE("ResultPool getInstance fail");

    m_pCallbackHub = new Hal3ACbHub(m_i4SensorDev);
    if (!m_pCallbackHub)
        MY_LOGE("[%s] CallbackHub Object created failed for SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    AAA_TRACE_D("querySensorStaticInfo");
    querySensorStaticInfo();
    AAA_TRACE_END_D;

    m_i4Count = ::property_get_int32("vendor.3a.lv.consecutive", 4);
    MY_LOGD_IF(m_3ALogEnable, "[%s] vendor.3a.lv.consecutive : %d", __FUNCTION__, m_i4Count);

    MY_LOGD("[%s] done\n", __FUNCTION__);
    android_atomic_inc(&m_Users);

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

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }
    MY_LOGD("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        m_pThreadRaw->destroyInstance();

        MRESULT err = S_3A_OK;

        // AE uninit
        err = IAeMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)){
            MY_LOGE("IAeMgr::getInstance().uninit() fail\n");
            return err;
        }

        // AWB uninit
        err = IAwbMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)){
            MY_LOGE("IAwbMgr::getInstance().uninit() fail\n");
            return E_3A_ERR;
        }

#if (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
        ::IPDTblGen::getInstance()->stop(m_i4SensorDev);
#endif

#if CAM3_AF_FEATURE_EN
        // AF uninit
        err = IAfMgr::getInstance().uninit(m_i4SensorDev);
        if (FAILED(err)) {
            MY_LOGE("IAfMgr::getInstance().uninit() fail\n");
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
            MY_LOGE("IFlickerHal::getInstance()->Uninit fail\n");
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
            MY_LOGE("User did not detach callbacks!");
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

        MY_LOGD("[%s] done\n", __FUNCTION__);

    }
    else    // There are still some users.
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] Still %d users \n", __FUNCTION__, m_Users);
    }

    unlockCustomTuningLibrary(m_i4sensorId, gTuningInitCtrl[m_i4SensorIdx].u4ModuleId);

    return S_3A_OK;
}

MINT32
Hal3ARawImp::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD("[%s]+ sensorDev(%d), Mode(%d)", __FUNCTION__, m_i4SensorDev, m_u4SensorMode);
    MRESULT err = S_3A_OK;

    MBOOL cctEnable = MFALSE;
    GET_PROP("vendor.3a.cct.enable", 0, cctEnable);
    if (cctEnable)
        IAeMgr::getInstance().setCCUOnOff(m_i4SensorDev,MFALSE);

#if CAM3_STEREO_FEATURE_EN
    ::memset(&m_rLastStereoParam, 0, sizeof(m_rLastStereoParam));
#endif

    // for Secure Cam Configuration
    if (rConfigInfo.bIsSecureCam)
    {
        IAeMgr::getInstance().setCCUOnOff(m_i4SensorDev, MFALSE);
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_SetSecureFlag, 0, 0);
#endif
    } else {
        IAeMgr::getInstance().setCCUOnOff(m_i4SensorDev, MTRUE);
    }

    m_rConfigInfo = rConfigInfo;
    if(rConfigInfo.i4SubsampleCount > 1 && m_pTuning)
    {
        m_pTuning->uninit(m_i4SensorDev);
        m_pTuning->init(m_i4SensorDev, m_i4SensorIdx, rConfigInfo.i4SubsampleCount);
        querySensorStaticInfo();
        m_bIsSMVRTuning = MTRUE;
        MY_LOGD("[%s] re-init isp tuning for subsample count(%d), smvr tuning", __FUNCTION__, rConfigInfo.i4SubsampleCount);
    }
    else if(m_bIsSMVRTuning)
    {
        m_pTuning->uninit(m_i4SensorDev);
        m_pTuning->init(m_i4SensorDev, m_i4SensorIdx);
        querySensorStaticInfo();
        m_bIsSMVRTuning = MFALSE;
        MY_LOGD("[%s] re-init isp tuning for subsample count(%d), normal tuning", __FUNCTION__, rConfigInfo.i4SubsampleCount);
    }

    m_bIsFirstSetParams = MTRUE;
    m_bIsHighQualityCaptureOn = MFALSE;
    GET_PROP("vendor.debug.camera.cammode", 0, m_i4CamModeEnable);

    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            MY_LOGE("Fail to create NormalPipe");
            return MFALSE;
        }
    }

    if (m_pCallbackHub) {
        //p1 dump call back
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_SET_REGDUMP_CBFP, (MINTPTR)(m_pCallbackHub->getCallbackAddr(1)), 0, 0);
    }

    if(rConfigInfo.i4BitMode != EBitMode_12Bit && rConfigInfo.i4BitMode != EBitMode_14Bit)
    {
        MY_LOGE("Not support BitMode(%d)", rConfigInfo.i4BitMode);
        return err;
    }

    m_bAAOMode = (rConfigInfo.i4BitMode == EBitMode_14Bit);

    MUINT32 u4AAWidth, u4AAHight;
    MUINT32 u4AFWidth, u4AFHeight;

    MBOOL bAAOMode;
    if(rConfigInfo.i4BitMode == EBitMode_12Bit)
        bAAOMode = 0;
    else if(rConfigInfo.i4BitMode == EBitMode_14Bit)
        bAAOMode = 1;
    else
    {
        MY_LOGE("Not support BitMode(%d)", rConfigInfo.i4BitMode);
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

    // update HBIN and BIN info to AE/AWB/AF/FLICKER

    // set sensor mode to 3A modules
    //IAeMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AAWidth, u4AAHight);     move to AE start thread     //~Nelson
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight, u4AAWidth, u4AAHight);

#if CAM3_AF_FEATURE_EN
    IAfMgr::getInstance().setSensorMode(m_i4SensorDev, m_u4SensorMode, u4AFWidth, u4AFHeight);
#endif

#if CAM3_FLICKER_FEATURE_EN
    IFlickerHal::getInstance(m_i4SensorDev)->setSensorMode(m_u4SensorMode, u4AAWidth, u4AAHight);
#endif

    // Create thread for starting AE & AF
    m_sParam.SensorMode = m_u4SensorMode;
    ::pthread_create(&m_ThreadAE, NULL, Hal3ARawImp::ThreadAEStart, this);
    ::pthread_create(&m_ThreadAF, NULL, Hal3ARawImp::ThreadAFStart, this);

    NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setSensorMode(
        static_cast<ESensorMode_T>(m_u4SensorMode), u4AFWidth, u4AFHeight, MFALSE);
    // FIXME (remove): update TG Info to 3A modules
    updateTGInfo();

    //Frontal Binning
    //MBOOL fgFrontalBin;
    m_bFrontalBin = (m_i4TgWidth == (MINT32)u4AFWidth && m_i4TgHeight == (MINT32)u4AFHeight) ? MFALSE : MTRUE;
    m_pTuning->setSensorMode(m_i4SensorDev, m_u4SensorMode, m_bFrontalBin, u4AFWidth, u4AFHeight);
    m_pTuning->setIspProfile(m_i4SensorDev, m_rParam.eIspProfile);
    AAA_TRACE_END_D;

    // AWB start
    AAA_TRACE_D("AWB Start");
    IAwbMgr::getInstance().setAAOMode(m_i4SensorDev, m_bAAOMode);
    err = IAwbMgr::getInstance().start(m_i4SensorDev);
    AAA_TRACE_END_D;
#if MTK_CAM_NEW_NVRAM_SUPPORT
    CAM_IDX_QRY_COMB rMapping_Info;
    rMapping_Info.eIspProfile = m_rParam.eIspProfile;
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AWB, m_3A_Index.u4AWBNVRAMIndex);
    IAwbMgr::getInstance().setNVRAMIndex(m_i4SensorDev, m_3A_Index.u4AWBNVRAMIndex);
#else
    IAwbMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AWB(m_sParam));
#endif
    if (!err) {
        MY_LOGE("IAwbMgr::getInstance().start() fail\n");
        return E_3A_ERR;
    }

#if CAM3_FLASH_FEATURE_EN
    // Flash start
    AAA_TRACE_D("FLASH Start");
    FlashMgr::getInstance(m_i4SensorDev)->setBitMode(rConfigInfo.i4BitMode);
#if MTK_CAM_NEW_NVRAM_SUPPORT
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AE, m_3A_Index.u4FlashAENVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_Calibration, m_3A_Index.u4FlashCaliNVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AE, m_3A_Index.u4FlashAENVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_CALIBRATION, m_3A_Index.u4FlashCaliNVRAMIndex);
#else
#endif
    err = FlashMgr::getInstance(m_i4SensorDev)->start();
    if (err) {
        MY_LOGE("FlashMgr::getInstance().start() fail\n");
        AAA_TRACE_END_D;
        return E_3A_ERR;
    }
    FlashHal::getInstance(m_i4SensorDev)->setInCharge(MTRUE);
    FlashMgr::getInstance(m_i4SensorDev)->setInCharge(MTRUE);
    AAA_TRACE_END_D;
#endif

    MBOOL enable_flk = IFlickerHal::getInstance(m_i4SensorDev)->getInfo();
#if CAM3_FLICKER_FEATURE_EN
    // Flicker start
    AAA_TRACE_D("FLICKER Start");
    err = IFlickerHal::getInstance(m_i4SensorDev)->config();
    if (FAILED(err)) {
        MY_LOGE("IFlickerHal::getInstance()->config() fail\n");
        AAA_TRACE_END_D;
        return err;
    }
    AAA_TRACE_END_D;
#endif

    cust_initSpecialLongExpOnOff(m_i4EffectiveFrame);

    MVOID* ThreadAEret;
    ::pthread_join(m_ThreadAE, &ThreadAEret);
    if (ThreadAEret)    MY_LOGD("Thread AE create failed");

    MVOID* ThreadAFret;
    ::pthread_join(m_ThreadAF, &ThreadAFret);
    if (ThreadAFret)    MY_LOGD("Thread AE create failed");

    IspTuningMgr::getInstance().setFlkEnable(m_i4SensorDev, enable_flk);
    m_pTuning->notifyRPGEnable(m_i4SensorDev, MTRUE);   // apply awb gain for init stat

    if(rConfigInfo.i4SubsampleCount > 1 && rConfigInfo.i4RequestCount == 1)
        m_i4BatchModeCount = rConfigInfo.i4SubsampleCount;
    else
        m_i4BatchModeCount = 1;

    MY_LOGD("[%s]- setAAOMode(%d), AAWH(%dx%d), AFWH(%dx%d), enable_flk=%d, BatchCount(%d)", __FUNCTION__, rConfigInfo.i4BitMode, u4AAWidth, u4AAHight, u4AFWidth, u4AFHeight, (int)enable_flk, m_i4BatchModeCount);
    return S_3A_OK;
}

MBOOL
Hal3ARawImp::
start()
{
    MY_LOGD("[%s] +", __FUNCTION__);
    MRESULT isAFLampOn = MFALSE;
    m_i4SttMagicNumber = 0;
    m_bPreStop = MFALSE;
    gTuningInitCtrl[m_i4SensorIdx].bIsFirstTime = MFALSE;

    /****************************************************************************
        ****   Move From Hal3ARawImp::Config() in order to prevent Quick Launch Tuning Fail   *********
        ****************************************************************************/
    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4Num = 1; i4Num <= m_i4BatchModeCount; i4Num++)
    {
        rRequestSet.vNumberSet.push_back(i4Num);
    }
    m_pTuning->validatePerFrame(m_i4SensorDev, rRequestSet, MTRUE);
    ISP_LCS_IN_INFO_T lcs_info;
    m_pTuning->getLCSparam(m_i4SensorDev, lcs_info);
    lcs_info.i4FrmId = 1;
    m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_LCS_ISP_PARAMS, 1, (MINTPTR)&lcs_info, 0);

    /*******************************************************
       *******     For Customization Short Exposuretime Frame     **********
        *******************************************************/
    m_i4ShortExpCount = 0;
    m_vShortExpFrame = getShortExpFrame();

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::getInstance()->DevCount(MTRUE, m_i4SensorDev);
#endif

#if CAM3_AF_FEATURE_EN
    MINT32 IsSupportAF = 0;
#if (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
    IsSupportAF = IAfMgr::getInstance().isAFSupport(m_i4SensorDev);
#else
    IsSupportAF = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);
#endif
    if(IsSupportAF > 0)
    {
        // enable AF thread
        m_pThreadRaw->enableAFThread(m_pTaskMgr);
    }

#ifdef MT6757P_API
    #if defined(MTKCAM_CMDQ_SUPPORT)
    m_pThreadRaw->enableAFVsyncThread();
    #endif
#endif

#endif

    m_pThreadRaw->createEventThread();

#if CAM3_FLASH_FEATURE_EN
    isAFLampOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
#endif

#if CAM3_FLICKER_FEATURE_EN
    IFlickerHal::getInstance(m_i4SensorDev)->start(FLK_ATTACH_PRIO_MEDIUM);
#endif

    // init Set CCU para Thread
    if(m_bCCUEn == MTRUE && m_pThreadSetCCUPara == NULL)
        m_pThreadSetCCUPara = ThreadSetCCUPara::createInstance(this, m_i4SensorDev, m_i4SensorIdx);

    // setStrobeMode
    if((FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() != LIB3A_FLASH_MODE_FORCE_TORCH)){
        IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, isAFLampOn ? MTRUE : MFALSE);
        MY_LOGD("[%s] Non Torch ae setStrobeMode", __FUNCTION__);
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
            MY_LOGD("[%s] CqTuningMgr init error", __FUNCTION__);
        else
            MY_LOGD("[%s] CqTuningMgr init", __FUNCTION__);
    }
#endif
#if 1
    MBOOL cctEnable = MFALSE;
    GET_PROP("vendor.3a.cct.enable", 0, cctEnable);
    if (cctEnable) {
        MINT32 cct_sensor_dev = 0;
        GET_PROP("vendor.3a.cct.sensor.index", 1, cct_sensor_dev);
        MY_LOGD("CCT init: sensor = (%d/%d) ", cct_sensor_dev, m_i4SensorDev);
        if(cct_sensor_dev == m_i4SensorDev)
        {
            m_3ACctEnable = MTRUE;
            CctSvrEnt_Init(m_i4SensorDev);
            CctSvrEnt_Ctrl(CCT_SVR_CTL_START_SERVER_THREAD);
        }
    }
#endif
    MY_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
stop()
{
    MY_LOGD("[%s] +", __FUNCTION__);
    MRESULT err = S_3A_OK;
    m_i4SttMagicNumber = 0;
    m_bIsFlashOpened = MFALSE;
    m_i4OverrideMinFrameRate = 0;
    m_i4OverrideMaxFrameRate = 0;

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::getInstance()->DevCount(MFALSE, m_i4SensorDev);

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
        IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MFALSE);
#endif*/
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, 5000, 30000);
    MY_LOGD("[%s] AE Min Max Frame Rate has been reset", __FUNCTION__);

    err = IAeMgr::getInstance().Stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
    MY_LOGE("IAeMgr::getInstance().Stop() fail\n");
        return err;
    }
    m_pThreadRaw->destroyEventThread();

    // AWB stop
    AAA_TRACE_D("AWB Stop");
    err = IAwbMgr::getInstance().stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (!err) {
        MY_LOGE("IAwbMgr::getInstance().stop() fail\n");
        return E_3A_ERR;
    }


#if CAM3_AF_FEATURE_EN
    // disable AF thread
    AAA_TRACE_D("AF THREAD disable");
    m_pThreadRaw->disableAFThread();
    AAA_TRACE_END_D;

    // AF stop
    AAA_TRACE_D("AF Stop");
    err = IAfMgr::getInstance().Stop(m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        MY_LOGE("AfMgr::getInstance().Stop() fail\n");
        return err;
    }

#ifdef MT6757P_API
    #if defined(MTKCAM_CMDQ_SUPPORT)
    m_pThreadRaw->disableAFVsyncThread();
    #endif
#endif

#endif

#if CAM3_FLASH_FEATURE_EN
    // Flash stop
    AAA_TRACE_D("Flash Stop");
    err = FlashMgr::getInstance(m_i4SensorDev)->stop();
    if (err) {
        MY_LOGE("FlashMgr::getInstance().stop() fail\n");
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

#if (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
    ::IPDTblGen::getInstance()->stop(m_i4SensorDev);
#endif

    MY_LOGD("[%s] -", __FUNCTION__);
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
        MBOOL enable_flk = IFlickerHal::getInstance(m_i4SensorDev)->getInfo();
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
    AAA_TRACE_HAL(generateP2);

    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    MY_LOGD_IF(m_3ALogEnable, "[%s] + flow(%d), buf(%p)", __FUNCTION__, flowType, pRegBuf);
    IspTuningMgr::getInstance().validatePerFrameP2(m_i4SensorDev, flowType, rIspInfo, pTuningBuf);
#if CAM3_LSC_FEATURE_EN
    ILscBuf* pLscBuf = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->getP2Buf();
    if (pLscBuf)
        ((TuningParam*)pTuningBuf)->pLsc2Buf = pLscBuf->getBuf();
    else
        ((TuningParam*)pTuningBuf)->pLsc2Buf = NULL;
#endif

    if (!((dip_x_reg_t*)pRegBuf)->DIP_X_CTL_RGB_EN.Bits.LCE_EN){
        ((TuningParam*)pTuningBuf)->pLcsBuf = NULL;
    }
    ((TuningParam*)pTuningBuf)->pBpc2Buf = IspTuningMgr::getInstance().getDMGItable(m_i4SensorDev);

    if(((TuningParam*)pTuningBuf)->pBpc2Buf == NULL){
        ((dip_x_reg_t*)pRegBuf)->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 0;
        ((dip_x_reg_t*)pRegBuf)->DIP_X_BNR2_PDC_CON.Bits.PDC_EN = 0;
    }

    // debug info
    if (pResultP2)
    {
        if (rIspInfo.rCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_HWNR)
        {
            MY_LOGD_IF(m_3ALogEnable, "[%s] get debug info p2 for Multi_Pass_NR #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            if (0 == pResultP2->vecDbgIspP2_MultiP.size())
            {
                MY_LOGD_IF(m_3ALogEnable, "[%s] Need to allocate P2 result for Multi_Pass_NR", __FUNCTION__);
                pResultP2->vecDbgIspP2_MultiP.resize(sizeof(DEBUG_RESERVEA_INFO_T));
            }
            DEBUG_RESERVEA_INFO_T& rIspExifDebugInfo = *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(pResultP2->vecDbgIspP2_MultiP.editArray());
            IspTuningMgr::getInstance().getDebugInfo_MultiPassNR(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
        else
        {
            MY_LOGD_IF(m_3ALogEnable, "[%s] get debug info p2 #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            if (0 == pResultP2->vecDbgIspP2.size())
            {
                MY_LOGD_IF(m_3ALogEnable, "[%s] Need to allocate P2 result", __FUNCTION__);
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

    MY_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    AAA_TRACE_END_HAL;

    return MTRUE;
}

MBOOL
Hal3ARawImp::
validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe)
{
    MY_LOGD_IF(m_3ALogEnable, "[%s] sensor(%d) #(%d) profile(%d) rpg(%d)", __FUNCTION__, m_i4SensorDev, rParamIspProfile.i4MagicNum, rParamIspProfile.eIspProfile, rParamIspProfile.iEnableRPG);
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
    MY_LOGD_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(m3AOperMtx2);
    if (FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCalibration()) {
        IAeMgr::getInstance().setAPAELock(m_i4SensorDev, MTRUE);
        IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, MTRUE);
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

    if(rNewParam.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE) {
        NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MFALSE);
    }
    else if(m_rParam.u1TonemapMode != rNewParam.u1TonemapMode){
        NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(MTRUE);
    }
#endif

    // ====================================== ISP tuning =============================================
    F(IspTuningMgr::getInstance().setSceneMode, u4SceneMode);
    F(IspTuningMgr::getInstance().setEffect, u4EffectMode);
/*    IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewParam.i4BrightnessMode);
    IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewParam.i4HueMode);
    IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewParam.i4SaturationMode);
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewParam.i4EdgeMode);
    IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewParam.i4ContrastMode);*/
    F(IspTuningMgr::getInstance().setEdgeMode, u1EdgeMode);
    F(IspTuningMgr::getInstance().setNoiseReductionMode, u1NRMode);
    F(IspTuningMgr::getInstance().setZoomRatio, i4ZoomRatio);

    F(IspTuningMgr::getInstance().setIspProfile, eIspProfile);
    IspTuningMgr::getInstance().notifyRPGEnable(m_i4SensorDev, MTRUE);

    // ====================================== AE ==============================================
    if(m_i4OverrideMinFrameRate == 0 && m_i4OverrideMaxFrameRate == 0)
    {
        F2(IAeMgr::getInstance().setAEMinMaxFrameRate, i4MinFps, i4MaxFps);
    }
    IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_CheckFPSCCUResume, rNewParam.i4MaxFps, NULL);

    F(IAeMgr::getInstance().setAEMeteringMode, u4AeMeterMode);
    F(IAeMgr::getInstance().setAERotateDegree, i4RotateDegree);
    if( bUpdateScenario && (( rNewParam.i4IsoSpeedMode!= m_rParam.i4IsoSpeedMode) || m_bIsFirstSetParams) )
    {
        IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.i4IsoSpeedMode);
    }
    if((rNewParam.rMeteringAreas != m_rParam.rMeteringAreas) || m_bIsFirstSetParams)
        IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas);
    // EV Comp must be set aftert AE Lock or CTSTest will fail
    F(IAeMgr::getInstance().setAPAELock, bIsAELock);
    F2(IAeMgr::getInstance().setAEEVCompIndex, i4ExpIndex, fExpCompStep);
    //
    F(IAeMgr::getInstance().setAEMode, u4AeMode);
    F(IAeMgr::getInstance().enableStereoDenoiseRatio, i4DenoiseMode);
    F(IAeMgr::getInstance().setAEFlickerMode, u4AntiBandingMode);
    if((rNewParam.u4CamMode != m_rParam.u4CamMode) || m_bIsFirstSetParams)
    {
        if (NSIspTuning::EOperMode_EM != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev) || m_i4CamModeEnable)
            IAeMgr::getInstance().setAECamMode(m_i4SensorDev, rNewParam.u4CamMode);
    }
    F(IAeMgr::getInstance().setAEShotMode, u4ShotMode);
    if( bUpdateScenario && (( rNewParam.u4SceneMode != m_rParam.u4SceneMode ) || m_bIsFirstSetParams) )
    {
        IAeMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode);
    }
    F(IAeMgr::getInstance().bBlackLevelLock, u1BlackLvlLock);
    FZoom(IAeMgr::getInstance().setZoomWinInfo);
    F(IAeMgr::getInstance().setAEHDRMode, u1HdrMode);
    F(IAeMgr::getInstance().setDigZoomRatio, i4ZoomRatio);
#if MTK_CAM_NEW_NVRAM_SUPPORT == 0
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

    if (bUpdateScenario)
    {
        if(((m_sParam.CaptureIntent != m_sPreParam.CaptureIntent)
            || (m_sParam.HdrMode != m_sPreParam.HdrMode)
            || (m_sParam.Sync2AMode != m_sPreParam.Sync2AMode)
            || (m_sParam.TargetMode != m_sPreParam.TargetMode)
            || (m_sParam.SensorMode != m_sPreParam.SensorMode)) || m_bIsFirstSetParams)
            {
                IAwbMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AWB(m_sParam));
                IAfMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AF(m_sParam));
                IAeMgr::getInstance().setCamScenarioMode(m_i4SensorDev, Scenario4AE(m_sParam));
            }
    }
#endif
    // CCU ISP OBC tuning
    if((rNewParam.eIspProfile != m_rParam.eIspProfile) || m_bIsFirstSetParams)
    {
        MVOID* pObc = NULL;
        MUINT16 u2Obcidx = 0;
        IspTuningMgr::getInstance().getDefaultObc(m_i4SensorDev ,rNewParam.eIspProfile, m_u4SensorMode, pObc, u2Obcidx);
        IAeMgr::getInstance().updateISPNvramOBCTable(m_i4SensorDev, pObc,(MINT32)u2Obcidx);
    }

    if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
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
        IAeMgr::getInstance().UpdateSensorParams(m_i4SensorDev, strSensorParams);
    }
    MY_LOGD_IF(m_3ALogEnable, "[%s] setAEMode(%d)", __FUNCTION__, rNewParam.u4AeMode);
    cust_setSpecialLongExpOnOff(rNewParam.i8ExposureTime);
    // ====================================== AWB ==============================================
    F(IAwbMgr::getInstance().setAWBLock, bIsAWBLock);
    F(IAwbMgr::getInstance().setAWBMode, u4AwbMode);
    // TODO
#if 0
    IAwbMgr::getInstance().setMWBColorTemperature(m_i4SensorDev, rNewParam.i4MWBColorTemperature);
#endif
    FZoom(IAwbMgr::getInstance().setZoomWinInfo);
    F(IAwbMgr::getInstance().setColorCorrectionMode, u1ColorCorrectMode);
    F(IspTuningMgr::getInstance().setColorCorrectionMode, u1ColorCorrectMode);
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
            IspTuningMgr::getInstance().setColorCorrectionTransform(m_i4SensorDev,
                rNewParam.fColorCorrectMat[0], rNewParam.fColorCorrectMat[1], rNewParam.fColorCorrectMat[2],
                rNewParam.fColorCorrectMat[3], rNewParam.fColorCorrectMat[4], rNewParam.fColorCorrectMat[5],
                rNewParam.fColorCorrectMat[6], rNewParam.fColorCorrectMat[7], rNewParam.fColorCorrectMat[8]);
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

    if((rNewParam.u1TorchDuty != m_rParam.u1TorchDuty) || m_bIsFirstSetParams)
        FlashHal::getInstance(m_i4SensorDev)->setTorchDuty(rNewParam.u1TorchDuty);
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
    m_u1LastCaptureIntent = m_rParam.u1CaptureIntent;
    m_rParam = rNewParam;
    m_sPreParam = m_sParam;
    if(bUpdateScenario)
        m_bIsFirstSetParams = MFALSE;

    MY_LOGD_IF(m_3ALogEnable, "[%s] m_rParam.u1ShadingMapMode(%d)", __FUNCTION__, m_rParam.u1ShadingMapMode);

    MY_LOGD_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
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
        if( IspTuningMgr::getInstance().getOperMode(m_i4SensorDev) == NSIspTuning::EOperMode_Normal)
            rAFPAram.fFocusDistance = rNewParam.fFocusDistance;
        else
            rAFPAram.fFocusDistance = -1;
        rAFPAram.u1AfTrig  = rNewParam.u1AfTrig;
        rAFPAram.u1PrecapTrig  = rNewParam.u1PrecapTrig;
        rAFPAram.u1FlashTrigAF = chkLampFlashOnCond() | chkPreFlashOnCond();
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
    MY_LOGD_IF(m_3ALogEnable, "MasterIdx = %d, SlaveIdx = %d", rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx);
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
                ISync3AMgr::getInstance()->init(0, rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx);
            m_pTaskMgr->sendEvent(ECmd_Sync3AStart);
        } else if(rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE)
        {
            if(fgIsInit)
                ISync3AMgr::getInstance()->uninit();
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
        } else if(rNewParam.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE)
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
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
    m_rLastStereoParam.bIsByFrame = rNewParam.bIsByFrame;
    m_rLastStereoParam.bIsDummyFrame = rNewParam.bIsDummyFrame;
    if(isChanged)
    {
        MY_LOGD("[%s] m_rLastStereoParam (%d,%d)(%d,%d,%d)(%d)", __FUNCTION__,
        m_rLastStereoParam.i4MasterIdx, m_rLastStereoParam.i4SlaveIdx,
        m_rLastStereoParam.i4Sync2AMode,
        m_rLastStereoParam.i4SyncAFMode,
        m_rLastStereoParam.i4HwSyncMode,
        m_rLastStereoParam.bIsByFrame);
    }
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
autoFocus()
{
#if CAM3_AF_FEATURE_EN
    MY_LOGD("[%s()] +", __FUNCTION__);
    EventOpt rOpt;
    rOpt.fgPreflashCond = chkPreFlashOnCond();
    rOpt.fgLampflashCond = chkLampFlashOnCond();
    rOpt.bStrobeBVTrigger = isStrobeBVTrigger();
    m_pTaskMgr->sendEvent(ECmd_TouchAEStart, rOpt);
    MY_LOGD("[%s()] -", __FUNCTION__);
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
cancelAutoFocus()
{
#if CAM3_AF_FEATURE_EN
    MY_LOGD("[%s()] +", __FUNCTION__);
    EventOpt rOpt;
    rOpt.fgPreflashCond = chkPreFlashOnCond();
    rOpt.fgLampflashCond = chkLampFlashOnCond();
    rOpt.bStrobeBVTrigger = isStrobeBVTrigger();
    m_pTaskMgr->sendEvent(ECmd_TouchAEEnd, rOpt);
    MY_LOGD("[%s()] -", __FUNCTION__);
#endif
    return MTRUE;
}

MVOID
Hal3ARawImp::
setFDEnable(MBOOL fgEnable)
{
    MY_LOGD_IF(m_3ALogEnable, "[%s] fgEnable(%d)", __FUNCTION__, fgEnable);
    m_bFaceDetectEnable = fgEnable;
    IAeMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    IAwbMgr::getInstance().setFDenable(m_i4SensorDev, fgEnable);
    IspTuningMgr::getInstance().setFDEnable(m_i4SensorDev, fgEnable);
    if (!m_bFaceDetectEnable)
        m_faceNum = 0;
}

MBOOL
Hal3ARawImp::
setFDInfo(MVOID* prFaces, MVOID* prAFFaces)
{
    MY_LOGD_IF(m_3ALogEnable, "[%s] m_bFaceDetectEnable(%d)", __FUNCTION__, m_bFaceDetectEnable);
    if (m_bFaceDetectEnable)
    {
        MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)prAFFaces;
        m_faceNum = pFaces->number_of_faces;
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
    MY_LOGD_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance().setOTInfo(m_i4SensorDev, prAFOT);
#endif
        IAeMgr::getInstance().setOTInfo(m_i4SensorDev, prOT);
    return MTRUE;
}

#if MTK_CAM_NEW_NVRAM_SUPPORT
MUINT32
Hal3ARawImp::
getNvramIndex(const CAM_IDX_QRY_COMB& info, EModule_T module, MUINT32& index)
{
    if (m_bMappingQueryFlag_3A)
    {
        index = (MUINT32)m_pIdxMgr->query(static_cast<MUINT32>(m_i4SensorDev), module, info, __FUNCTION__);
    }

    MY_LOGD_IF(m_i4IdxCacheLogEnable,
        "[IdxCache][From:%s][Dev:%d-Mod:%s(%d)] (Idx %d) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d, IC %d)",
        m_bMappingQueryFlag_3A ? "IdxMgr":"Cache", m_i4SensorDev, strEModule[module], module, index,  strEIspProfile[info.eIspProfile], strESensorMode[info.eSensorMode], info.eFrontBin, info.eP2size, info.eFlash, strEApp[info.eApp], info.eFaceDetection, info.eZoom_Idx, info.eLV_Idx, info.eCT_Idx, info.eISO_Idx, info.eDriverIC);

    return index;
}
#endif

MBOOL
Hal3ARawImp::
setFlashLightOnOff(MBOOL bOnOff, MBOOL bMainPre,  MINT32 i4P1DoneSttNum /* = -1 */)
{
#if CAM3_FLASH_FEATURE_EN
    MY_LOGD_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);
    if (m_bIsFlashOpened == bOnOff)
    {
        MY_LOGD("[%s] flashlight unchanged, bOnOff(%d)", __FUNCTION__, bOnOff);
    }
    else if (!bOnOff)
    {
        //modified to update strobe state after capture for ae manager
        IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, MFALSE);
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
            IAwbMgr::getInstance().SetMainFlashInfo(m_i4SensorDev, MTRUE);
            }

        else{
            FlashHal::getInstance(m_i4SensorDev)->setOnOff(MTRUE, FLASH_HAL_SCENARIO_VIDEO_TORCH);
            IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_ON);
            }
        MY_LOGD("[%s] OFF FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
        m_bFlashOpenedByTask = MFALSE;
        m_i4FlashType = -1;

    }
    if(m_bIsFlashOpened != bOnOff)
    {
        MY_LOGD("[%s] flashlight is changed (%d->%d), and assign bOnOff to m_bIsFlashOpened", __FUNCTION__, m_bIsFlashOpened, bOnOff);
        m_bIsFlashOpened = bOnOff;
        handleBadPicture(m_rParam.i4MagicNum, i4P1DoneSttNum);
    }
    MY_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
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
        MY_LOGD("[%s] flashlight is changed (%d->%d)", __FUNCTION__, m_bIsFlashOpened, bOnOff);
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

    MY_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), i4HWSuppportFlash(%d)", __FUNCTION__,
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

    MY_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), i4HWSuppportFlash(%d)", __FUNCTION__,
        rFlashPolicy.bIsFlashOnCapture,
        rFlashPolicy.bIsFlashCharging,
        rFlashPolicy.i4HWSuppportFlash);
    return HAL3A_POLICY_TORCH_ONOFF(m_i4SensorDev, m_rParam, rFlashPolicy);
}

MBOOL
Hal3ARawImp::
chkLampFlashOnCond() const
{
    // for touch, precapture
    FlashPolicy_T rFlashPolicy;
    rFlashPolicy.bIsFlashOnCapture = isFlashOnCapture();
    rFlashPolicy.bIsFlashCharging = (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING);
    rFlashPolicy.bIsStrobeBVTrigger = IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);
    FlashHal::getInstance(m_i4SensorDev)->hasHw(rFlashPolicy.i4HWSuppportFlash);
    rFlashPolicy.i4HWSuppportFlash |= FlashHal::getInstance(m_i4SensorDev)->getSubFlashCustomization();

    MY_LOGD_IF(m_3ALogEnable, "[%s] bIsFlashOnCapture(%d), bIsFlashCharging(%d), i4HWSuppportFlash(%d)", __FUNCTION__,
        rFlashPolicy.bIsFlashOnCapture,
        rFlashPolicy.bIsFlashCharging,
        rFlashPolicy.i4HWSuppportFlash);
    return HAL3A_POLICY_LAMP_FLASH_ONOFF(m_i4SensorDev, m_rParam, rFlashPolicy);
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
    return FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture();
}

MBOOL
Hal3ARawImp::
dumpP1Params(MINT32 i4MagicNum)
{
    MY_LOGD("[%s] #(%d)", __FUNCTION__, i4MagicNum);
    IspDebug rP1(m_i4SensorIdx, m_i4SensorDev);
    return rP1.dumpRegs(i4MagicNum);
}

MINT32
Hal3ARawImp::
getCurrResult(MUINT32 i4FrmId, MINT32 /*i4SubsampleIndex*/) const
{
    MY_LOGD_IF(m_3ALogEnable, "[%s]+ i4MagicNum(%d) m_bIsHighQualityCaptureOn:%d", __FUNCTION__, i4FrmId, m_bIsHighQualityCaptureOn);

    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState = MTK_CONTROL_AWB_STATE_INACTIVE;

    AAA_TRACE_HAL(getAllResult);

    // ResultPool - To update Vector info
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(i4FrmId);
    if(pAllResult == NULL)
    {
        MY_LOGE("[%s] pAllResult is NULL", __FUNCTION__);
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

    //===== Get HAL result =====
    rHALResult.i4FrmId = i4FrmId;
    rHALResult.fgKeep = m_bIsCapEnd;
    rHALResult.fgBadPicture = MFALSE;

    rHALResult.u1SceneMode = m_rParam.u4SceneMode;

    //===== Get AE result =====

    //===== Get AE FD region =====
    // ResultPool - put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
    {
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        AAA_TRACE_MGR(getAEFD);
        IAeMgr::getInstance().getAEFDMeteringAreaInfo(m_i4SensorDev, pAllResult->vecAEROI, m_i4TgWidth, m_i4TgHeight);
        AAA_TRACE_END_MGR;
    }
    AAA_TRACE_MGR(getAEState);
    rAEResult.u1AeMode = m_rParam.u4AeMode;
    MUINT8 u1AeState = IAeMgr::getInstance().getAEState(m_i4SensorDev);
    MBOOL isExistPrecap = (m_pTaskMgr!= NULL) ? m_pTaskMgr ->isExistTask(TASK_ENUM_3A_PRECAPTURE) : 0;
    if (isExistPrecap == MTRUE)
        u1AeState = MTK_CONTROL_AE_STATE_PRECAPTURE;
    if (IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev) && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rAEResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rAEResult.u1AeState = u1AeState;

    rAEResult.fgAeBvTrigger = IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);
    // AutoHdrResult
    rAEResult.i4AutoHdrResult = (MINT32)IAeMgr::getInstance().getAEHDROnOff(m_i4SensorDev);
    AAA_TRACE_END_MGR;

    AAA_TRACE_MGR(getSensorParams);
    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rAEResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rAEResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rAEResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
    if (m_rParam.u1RollingShutterSkew)
        rAEResult.i8SensorRollingShutterSkew = IAeMgr::getInstance().getSensorRollingShutter(m_i4SensorDev);
    FrameOutputParam_T frameOutInfo;
    IAeMgr::getInstance().getRTParams(m_i4SensorDev, frameOutInfo);
    rAEResult.i4AvgY = static_cast<MINT32>(frameOutInfo.u4AvgY);
    AAA_TRACE_END_MGR;

    //===== Get AWB result =====

    //===== Get AWB FD region =====
    // ResultPool - put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
    {
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
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
    AAA_TRACE_END_MGR;

    AAA_TRACE_MGR(getColorCorrectGain);
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
    AAA_TRACE_END_MGR;

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
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4FrmId,E_FLASH_RESULTTOMETA);
    MBOOL bIsUpdated = MFALSE;
    if(pFLASHResult != NULL) {
        if( pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_READY ){
            bIsUpdated = MTRUE;
            MY_LOGD("[%s] FlashState(%d) is updated", __FUNCTION__, pFLASHResult->u1FlashState);
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

    // flash calibration result
    rFLASHResult.i4FlashCalResult = FlashMgr::getInstance(m_i4SensorDev)->cctGetQuickCalibrationResult();
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
#endif
    AAA_TRACE_END_MGR;

    //===== Get LSC result =====
    AAA_TRACE_LSC(getLscResult);
#if CAM3_LSC_FEATURE_EN
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));
    CAM_LOGD_IF(m_3ALogEnable, "[%s] #lsc(%d) m_rParam.i4RawType(%d) m_bIsCapEnd(%d)", __FUNCTION__, pLsc->getOnOff(), m_rParam.i4RawType, m_bIsCapEnd);
    MBOOL fgRequireShadingIntent = 1; /*
        (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT ||
         m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG);*/

    rLSCResult.i4CurrTblIndex = -1;
    if(pLsc->getOnOff())
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] #(%d) LSC table to metadata", __FUNCTION__, i4FrmId);
        //pLsc->getCurrTbl(pAllResult->vecLscData);
        rLSCResult.i4CurrTblIndex = pLsc->getCurrTblIndex();
        MY_LOGD_IF(m_3ALogEnable, "[%s] CurrTblIndex:%d", __FUNCTION__, rLSCResult.i4CurrTblIndex);
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
    if (ISync3AMgr::getInstance()->isActive() && i4FrmId > u4AEStableMagic && ISync3AMgr::getInstance()->getSync3A()->is2ASyncFinish())
    {
        MINT32 i4Master = ISync3AMgr::getInstance()->getMasterDev();
        MINT32 i4Slave = ISync3AMgr::getInstance()->getSlaveDev();
        if(i4Master == m_i4SensorDev)
        {
            // large lv diff
            MINT32 i4LvMaster = IAeMgr::getInstance().getLVvalue(i4Master, MTRUE);
            MINT32 i4LvSlave = IAeMgr::getInstance().getLVvalue(i4Slave, MTRUE);
            rHALResult.i4StereoWarning |= CUST_LENS_COVER_COUNT(i4LvMaster, i4LvSlave, m_i4Count);
            // low light
            rHALResult.i4StereoWarning |= ((max(i4LvMaster, i4LvSlave) < 10) ? 0x2 : 0);
            // close shot
            MINT32 i4FocusDistance = IAfMgr::getInstance().getAFBestPos(i4Master);
            MINT32 i4MaxFocusDistance = IAfMgr::getInstance().getMaxLensPos(i4Master);
            MINT32 i4MinFocusDistance = IAfMgr::getInstance().getMinLensPos(i4Master);
            if(i4FocusDistance > (i4MaxFocusDistance - (i4MaxFocusDistance - i4MinFocusDistance) / 5))
                rHALResult.i4StereoWarning |= 0x4;

            MY_LOGD_IF(rHALResult.i4StereoWarning,
                "[%s] warning(%d) lv(%d,%d), fd(%d,%d,%d)", __FUNCTION__, rHALResult.i4StereoWarning, i4LvMaster, i4LvSlave, i4FocusDistance, i4MaxFocusDistance, i4MinFocusDistance);
        }
    }
#endif
    AAA_TRACE_END_MGR;

    //===== Get ISP result =====
    // Cam Info
    AAA_TRACE_ISP(getCamInfo);
    MBOOL bRet = IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rISPResult.rCamInfo);
    if (!bRet)
    {
        MY_LOGE("Fail to get CamInfo");
    }else{
        LastInfo_T vLastInfo;
        // Backup caminfo
        ::memcpy(&vLastInfo.mBackupCamInfo, &rISPResult.rCamInfo, sizeof(NSIspTuning::RAWIspCamInfo));
        vLastInfo.mBackupCamInfo_copied = MTRUE;
        m_pResultPoolObj->updateLastInfo(vLastInfo);
        MY_LOGD_IF(m_3ALogEnable, "[%s] Backup caminfo,copied(%d)/mode(%d)/profile(%d)",__FUNCTION__, vLastInfo.mBackupCamInfo_copied,
            vLastInfo.mBackupCamInfo.eSensorMode,
            vLastInfo.mBackupCamInfo.eIspProfile);
    }
#if (!MT6763_API)
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_LCE_GAIN, (MINTPTR)&(rISPResult.i4LceGain), 0);
    MY_LOGD_IF(m_3ALogEnable, "[%s] rISPResult.i4LceGain(%d)",__FUNCTION__, rISPResult.i4LceGain);
#endif

#if MTK_CAM_NEW_NVRAM_SUPPORT
    IdxMgr::createInstance(static_cast<NSIspTuning::MUINT32>(m_i4SensorDev))->setMappingInfo(static_cast<NSIspTuning::MUINT32>(m_i4SensorDev), rISPResult.rCamInfo, i4FrmId);
#endif

    AAA_TRACE_END_ISP;

    AAA_TRACE_MGR(get2AExif);
    //===== Get Exif result =====
    if (m_rParam.u1IsGetExif || m_bIsCapEnd)
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

            if(pAllResult->vecDbgShadTbl.size() != 0 && pAllResult->vecDbgIspInfo.size() != 0 && pAllResult->vecDbg3AInfo.size() != 0)
            {
                MY_LOGD_IF(m_3ALogEnable, "[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)pAllResult->vecDbgShadTbl.size(), (MINT32)pAllResult->vecDbgIspInfo.size(), (MINT32)pAllResult->vecDbg3AInfo.size(),
                                            &rDbgShadTbl, &rDbgIspInfo, &rDbg3AInfo);
                getP1DbgInfo(rDbg3AInfo, rDbgShadTbl, rDbgIspInfo);
            }
            else
                MY_LOGE("[%s] vecDbgShadTbl vecDbgIspInfo vecDbg3AInfo - Size(%d, %d, %d) Addr(%p, %p, %p)", __FUNCTION__, (MINT32)pAllResult->vecDbgShadTbl.size(), (MINT32)pAllResult->vecDbgIspInfo.size(), (MINT32)pAllResult->vecDbg3AInfo.size(),
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
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_ISP_RESULTTOMETA, &rISPResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_LSC_RESULTTOMETA, &rLSCResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_FLASH_RESULTTOMETA, &rFLASHResult);
    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_FLK_RESULTTOMETA, &rFLKResult);
    AAA_TRACE_END_MGR;

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
    MINT32 IsSupportAF = 0;
#if (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
    IsSupportAF = IAfMgr::getInstance().isAFSupport(m_i4SensorDev);
#else
    IsSupportAF = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);
#endif
    MINT32 IsAFSuspend = IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_IsAFSuspend, NULL, NULL);
    MINT32 i4AFEnable = 0;
    MUINT32 i4OutLens = 0;
    IAfMgr::getInstance().CCTOPAFGetEnableInfo(m_i4SensorDev, (MVOID*)&i4AFEnable, &i4OutLens);
    if( IsSupportAF && (!i4AFEnable) && rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED &&  rEarlyCB.u1AfState != MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] overwrite AF state(%d)", __FUNCTION__, rEarlyCB.u1AfState);
        rEarlyCB.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
    }

    // update stereo master slave index
#if CAM3_STEREO_FEATURE_EN
    if(m_rLastStereoParam.i4MasterIdx != 0 || m_rLastStereoParam.i4SlaveIdx != 0)
    {
        rEarlyCB.i4MasterIdx = m_rLastStereoParam.i4MasterIdx;
        rEarlyCB.i4SlaveIdx = m_rLastStereoParam.i4SlaveIdx;
        MY_LOGD_IF(m_3ALogEnable, "[%s] rEarlyCB (MasterIdx, SlaveIdx)(%d,%d)", __FUNCTION__, rEarlyCB.i4MasterIdx, rEarlyCB.i4SlaveIdx);
    }
#endif

    m_pResultPoolObj->updateEarlyCB(i4FrmId, rEarlyCB);
    AAA_TRACE_END_MGR;

    //===== AF disable, then 3A thread to get AF result ====
    if(!IsSupportAF)
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] disable AF (%d), 3A thread to get AF result", __FUNCTION__, IsSupportAF);
        IThreadRaw::AFParam_T AFParam;
        AFParam.i4MagicNum = i4FrmId;
        m_pThreadRaw->getCurrResult4AF(AFParam);
    }

    //===== Call Back modules update done, then convert result to metadata =====
    AAA_TRACE_HAL(RESULTPOOL_DONE);
    if (m_pCbSet && i4FrmId != 0 && m_bIsHighQualityCaptureOn == MFALSE)
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] #(%d)", __FUNCTION__, i4FrmId);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_UPDATE_RESULTPOOL_DONE,
            i4FrmId,     // magic number
            NULL, NULL);
    }
    AAA_TRACE_END_HAL;

    //===== AF disable, then 3A thread to convert AF ====
    if(!IsSupportAF)
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] disable AF(%d), 3A thread to convert AF", __FUNCTION__, IsSupportAF);

        // ResultPool - Add command Info
        ResultPoolCmd4Convert_T CmdAF;
        CmdAF.i4MagicNumReq = i4FrmId;
        CmdAF.eConvertType = E_AF_CONVERT;
        CmdAF.i4SubsampleCount = 0;

        // ResultPool - If modules validate, post thread to convert metadata.
        MINT32 i4Validate = m_pResultPoolObj->isValidate(CmdAF.i4MagicNumReq);
        MY_LOGD_IF(m_3ALogEnable, "[%s] (Req, Validate) = (#%d, %d)", __FUNCTION__, CmdAF.i4MagicNumReq, i4Validate);
        if(i4Validate)
           m_pResultPoolObj->postCmdToThread(&CmdAF);

    }

    //===== Callback autoHdr information =====
    // Auto HDR Detection, default is -1
    // -1: auto hdr off, 0: not detected, 1: detected, -2: use algo
    MINT32 debugData = property_get_int32("vendor.debug.aaa_state.hdrMode", -2);
    MINT32 autoHdrRes = IAeMgr::getInstance().getAEHDROnOff(m_i4SensorDev);
    MY_LOGD_IF(m_3ALogEnable, "[%s] autoHdrRes(%d), debugData(%d)",__FUNCTION__, autoHdrRes, debugData);
    if(CC_UNLIKELY((debugData > -2) && (debugData < 2)))
    {
        MY_LOGD("[%s] use debugData(%d), ori autoHdrRes(%d)",__FUNCTION__, debugData, autoHdrRes);
        autoHdrRes = debugData ;
    }
    if (m_pCbSet)
    {
        m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_HDRD_RESULT, reinterpret_cast<MINTPTR> (&autoHdrRes), 0, 0);
    }

    MY_LOGD_IF(m_3ALogEnable, "[%s]- i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return MTRUE;
}

MINT32
Hal3ARawImp::
getCurrentHwId() const
{
    MINT32 idx = 0;
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX,
                        (MINTPTR)&idx, 0, 0);
    MY_LOGD_IF(m_3ALogEnable, "[%s] idx(%d)", __FUNCTION__, idx);
    return idx;
}

MVOID
Hal3ARawImp::
setSensorMode(MINT32 i4SensorMode)
{
    MY_LOGD("[%s] mode(%d)", __FUNCTION__, i4SensorMode);
    m_u4SensorMode = i4SensorMode;
}

MBOOL
Hal3ARawImp::
postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam)
{
    MY_LOGD_IF(m_3ALogEnable, "[%s]+ cmd(%d)", __FUNCTION__, r3ACmd);
    if (pParam != NULL)
    {
        MY_LOGD_IF(m_i4ShortExpCount != 0, "Check Validate %d, Short Exp Count %d, Frame %d", pParam->iValidateOpt, m_i4ShortExpCount, m_vShortExpFrame[m_i4ShortExpCount]);
        if (pParam->iValidateOpt == ParamIspProfile_T::EParamValidate_None && m_vShortExpFrame[m_i4ShortExpCount++])
        {
            AAASensorMgr::getInstance().setSensorExpTime(m_i4SensorDev, 100000);
            MY_LOGD("[%s] set next frame rate 10fps", __FUNCTION__);
            return MTRUE;
        }
    }
    else
    {
        MY_LOGE("[%s] pParam (ParamIspProfile_T) should not be NULL", __FUNCTION__);
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
    MY_LOGD_IF(m_3ALogEnable,"[%s] bDummyAfterCapture(%d) ", __FUNCTION__, bDummyAfterCapture);

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
            MY_LOGD("[HQC] Start : Request(#%d)", pParam->i4MagicNum);
        } else
        // ZSD cap
        {
            MINT32 i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            MBOOL bAELock = IAeMgr::getInstance().IsAPAELock(m_i4SensorDev);
            MBOOL bIsEVchaged = m_i4EVCap != IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev);

            // skip capture when the 2nd, 3rd, etc. (C shot)
            if(m_u1LastCaptureIntent == m_rParam.u1CaptureIntent)
                bSkipZsdCap = MTRUE;

            if (bIsEVchaged){
                m_i4EVCap = IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev);
                //bAELock = MFALSE;
            }
            MY_LOGD("[%s] i4OperMode(%d) mbIsHDRShot(%d) bAELock(%d) EV(%d) EVchanged (%d) CaptureIntent(%d)", __FUNCTION__, i4OperMode, mbIsHDRShot, bAELock, m_i4EVCap, bIsEVchaged, m_u1LastCaptureIntent);
            /*
            some senerio should not updateCaptureParams
            1. capture with flash
            2. HDR shot, camera3
            3. i4OperMode != EOperMode_Meta
            4. AELock is set
            5. 2nd frame,3rd frame,etc. (C shot)
            */
#if CAM3_FLASH_FEATURE_EN
            if(!mbIsHDRShot && (i4OperMode != EOperMode_Meta) && !bAELock && !bSkipZsdCap)
#endif
            {
                AE_MODE_CFG_T previewInfo;
                IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);
                IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, previewInfo);
            }
        }

        // notify AE for single frame HDR
        if (m_rParam.u1IsSingleFrameHDR)
            IAeMgr::getInstance().setzCHDRShot(m_i4SensorDev, MTRUE);
        if(!bSkipZsdCap || mbIsHDRShot)
            m_pTaskMgr->sendEvent(ECmd_CaptureStart);

        // reset flag of HDR shot
        if (mbIsHDRShot)
            mbIsHDRShot = MFALSE;
    }
    AAA_TRACE_END_HAL;

    /*****************************
     *     3A Execution
     *****************************/
    if(r3ACmd == ECmd_Update)
    {
        // Guarantee Sync3A enable or disable
#if CAM3_STEREO_FEATURE_EN
        MBOOL fgIsActive = ISync3AMgr::getInstance()->isActive();

        // If bIsByFrame == MTRUE, it mean MW update Sync2A mode to main/main2 camera.
        // So 3A framework don't need to enable Sync2A mode to other camera.
        if(!m_rLastStereoParam.bIsByFrame)
        {
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
            //IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MFALSE);
            if(fgIsActive)
            {
                m_pTaskMgr->sendEvent(ECmd_Sync3AStart);
                //IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MTRUE);
            }
            else
            {
                m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
                //IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, MFALSE);
            }
        } else if(m_rLastStereoParam.bIsDummyFrame)
        {
            m_pTaskMgr->sendEvent(ECmd_Sync3AEnd);
        }
#endif
        // update TG INT Params to driver
        if(m_u4LastRequestNumber != 0 && m_bTgIntAEEn)//call before updateMagicNumberRequest
            if(!setTG_INTParams())
                MY_LOGE("[%s] setTG INTParams fail", __FUNCTION__);

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
        rData.bByPassStt = (m_rParam.u1IsStartCapture && m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF);
        rData.bRemosaicEn = m_rParam.u1RemosaicEn;
        rData.bMainFlashOn = chkMainFlashOnCond();
        rData.bIsDummyFrame = m_rParam.bIsDummyFrame;
        rData.i4FrameNum = m_rParam.i4FrameNum;
        MY_LOGD_IF(rData.bByPassStt, "[%s] bByPassStt(%d) IsStartCapture (%d) u4AeMode(%d)", __FUNCTION__, rData.bByPassStt, m_rParam.u1IsStartCapture, m_rParam.u4AeMode);
        m_i4SttMagicNumber = m_pTaskMgr->prepareStt(rData,Task_Update_3A);

#if MTK_CAM_NEW_NVRAM_SUPPORT
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
    rQueryCommandQ.eZoom_Idx = IspTuningMgr::getInstance().getZoomIdx(m_i4SensorDev ,m_rParam.i4ZoomRatio);

    IspTuningCustom* pIspTuningCustom = IspTuningCustom::createInstance((MUINT32)m_i4SensorDev, m_i4SensorIdx);
    rQueryCommandQ.eApp = (EApp_T)pIspTuningCustom->map_AppName_to_MappingInfo();
    rQueryCommandQ.eDriverIC = (EDriverIC_T)pIspTuningCustom->map_FlashId_to_MappingInfo(FlashHal::getInstance(m_i4SensorDev)->getPartId());
    MY_LOGD_IF(m_3ALogEnable,"[%s] DriverIC(%d)", __FUNCTION__, rQueryCommandQ.eDriverIC);

    // check the flash state, and update to index map mgr
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(m_i4SttMagicNumber,E_FLASH_RESULTTOMETA);
    if(pFLASHResult != NULL) {
        if( pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED){
            rQueryCommandQ.eFlash = EFlash_Yes;
        } else
            rQueryCommandQ.eFlash = EFlash_No;
    }

    MINT32 i4IsFlashOnCapture = isFlashOnCapture();
    MY_LOGD("[%s] i4IsFlashOnCapture(%d) bIsHighQualityCaptureOn(%d)", __FUNCTION__, i4IsFlashOnCapture, m_bIsHighQualityCaptureOn);
    if(i4IsFlashOnCapture && m_bIsHighQualityCaptureOn)
        rQueryCommandQ.eFlash = EFlash_Yes;
    MY_LOGD("[%s] rQueryCommandQ eFlash(%d)", __FUNCTION__, rQueryCommandQ.eFlash);

    IspTuningMgr::getInstance().setFlashInfo(m_i4SensorDev ,rQueryCommandQ.eFlash);

    AAA_TRACE_HAL(IdxMgrQuery);

    IdxMgr::createInstance(static_cast<NSIspTuning::MUINT32>(m_i4SensorDev))->setMappingInfo(static_cast<NSIspTuning::MUINT32>(m_i4SensorDev), rQueryCommandQ, m_rParam.i4MagicNum);
    CAM_IDX_QRY_COMB rMapping_Info;
    m_pIdxMgr->getMappingInfo(static_cast<MUINT32>(m_i4SensorDev), rMapping_Info, m_rParam.i4MagicNum);

    if (memcmp(&(rMapping_Info), &m_Mapping_Info_3A, sizeof(CAM_IDX_QRY_COMB))){
        memcpy(&m_Mapping_Info_3A, &(rMapping_Info), sizeof(CAM_IDX_QRY_COMB));
        m_bMappingQueryFlag_3A = MTRUE;
    } else {
        MY_LOGD_IF(m_3ALogEnable,"[%s] use last mapping info(%d)", __FUNCTION__, pParam->i4MagicNum);
        m_bMappingQueryFlag_3A = MFALSE;
    }

    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AE, m_3A_Index.u4AENVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AWB, m_3A_Index.u4AWBNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_AF, m_3A_Index.u4AFNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AE, m_3A_Index.u4FlashAENVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_Flash_Calibration, m_3A_Index.u4FlashCaliNVRAMIndex);

    rMapping_Info.eISO_Idx = NSIspTuning::EISO_IDX_00;
    getNvramIndex(rMapping_Info, NSIspTuning::EModule_OBC, m_3A_Index.u4OBCNVRAMIndex);

    AAA_TRACE_END_HAL;

    IAwbMgr::getInstance().setNVRAMIndex(m_i4SensorDev, m_3A_Index.u4AWBNVRAMIndex);
    IAfMgr::getInstance().setNVRAMIndex(m_i4SensorDev, m_3A_Index.u4AFNVRAMIndex);
    IAeMgr::getInstance().setNVRAMIndex(m_i4SensorDev, m_3A_Index.u4AENVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AE, m_3A_Index.u4FlashAENVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_AWB, m_3A_Index.u4FlashAWBNVRAMIndex);
    FlashMgr::getInstance(m_i4SensorDev)->setNVRAMIndex(FLASH_NVRAM_CALIBRATION, m_3A_Index.u4FlashCaliNVRAMIndex);

    IAeMgr::getInstance().updateISPNvramOBCIndex(m_i4SensorDev,&rMapping_Info,m_3A_Index.u4OBCNVRAMIndex);
    MY_LOGD_IF(m_3ALogEnable, "[%s] NVRAM Index AE(%d) AWB(%d) AF(%d) F_AE(%d) F_AWB(%d) F_Cali(%d) OBC(%d)",
                    __FUNCTION__, m_3A_Index.u4AENVRAMIndex, m_3A_Index.u4AWBNVRAMIndex, m_3A_Index.u4AFNVRAMIndex, m_3A_Index.u4FlashAENVRAMIndex, m_3A_Index.u4FlashAWBNVRAMIndex, m_3A_Index.u4FlashCaliNVRAMIndex,m_3A_Index.u4OBCNVRAMIndex);
#else
    //
    MINT32 isMvHDREnable = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isMvHDREnable();
    if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL) {
        // TODO : AE Mgr need to modify SCENARIO
        if(m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH)
            IAeMgr::getInstance().setCamScenarioMode(m_i4SensorDev, CAM_SCENARIO_VIDEO);
        else
            IAeMgr::getInstance().setCamScenarioMode(m_i4SensorDev, CAM_SCENARIO_PREVIEW);
    }
    // check the flash state, and set info to isp
    FLASH_INFO_T finfo = {};
    finfo.isFlash = EFlash_No;
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(m_i4SttMagicNumber,E_FLASH_RESULTTOMETA);
    if(pFLASHResult != nullptr) {
        if( pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL ||
            pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED){
            if((FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() == LIB3A_FLASH_MODE_FORCE_TORCH))
                finfo.isFlash = EFlash_Torch;
            else if(m_i4FlashType == FLASH_HAL_SCENARIO_PRE_FLASH)
                finfo.isFlash = EFlash_PreFlash;
            else
                finfo.isFlash = EFlash_MainFlash;
        } else
            finfo.isFlash = EFlash_No;
    }
    MINT32 i4IsFlashOnCapture = isFlashOnCapture();
    if(i4IsFlashOnCapture && m_bIsHighQualityCaptureOn)
        finfo.isFlash = EFlash_MainFlash;

    IspTuningMgr::getInstance().setFlashInfo(m_i4SensorDev ,finfo);
    MY_LOGD_IF(m_3ALogEnable, "[%s] MvHDREnable(%d), StrobeMode(%d), i4IsFlashOnCapture(%d), bIsHighQualityCaptureOn(%d), isFlash(%d)",
                    __FUNCTION__, isMvHDREnable, m_rParam.u4StrobeMode, i4IsFlashOnCapture, m_bIsHighQualityCaptureOn, finfo.isFlash);
#endif

        MY_LOGD_IF(m_3ALogEnable,"[%s] TaskData : Req(#%d)/Stt(#%d)/Frm(#%d)/Flash(%d)/HqCap(%d)/AfTrigger(%d)/FlashTypeByTask(%d)/FlashOpenByTask(%d)/RemosaicEn(%d)",
            __FUNCTION__, rData.i4RequestMagic,
            m_i4SttMagicNumber,
            rData.i4FrameNum,
            rData.bFlashOnOff,
            rData.bIsHqCap,
            rData.bIsAfTriggerInPrecap,
            rData.i4FlashTypeByTask,
            rData.bFlashOpenByTask,
            rData.bRemosaicEn);
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
        else if(eResult == TASK_RESULT_CLOSE_PREFLASH_REQUIRE)
            isClosePreFlash = MTRUE;
        else if(eResult == TASK_RESULT_FLASH_RESTOREANDAPAELOCK)
            isRestoreLock = MTRUE;

        if(isOpenFlash)
        {
            doBackup2A();
            IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, MTRUE);
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
            IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, MFALSE);
            IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, AWB_STROBE_MODE_OFF);
            handleBadPicture(pParam->i4MagicNum);
        }

        if(isRestoreLock)
        {
            IAeMgr::getInstance().setAPAELock(m_i4SensorDev, MTRUE);
            IAeMgr::getInstance().setAEState2Converge(m_i4SensorDev);
        }
        isFlashchanged = isOpenFlash || isCloseFlash;
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
          MY_LOGD("[%s] set AWB main flash info False", __FUNCTION__);
        }

        // reset HQC flag
        if(m_bIsCapEnd && m_bIsHighQualityCaptureOn)
        {
            m_bIsHighQualityCaptureOn = MFALSE;
            MY_LOGD("[HQC] End : Stt(#%d)", m_i4SttMagicNumber);
        }
        AAA_TRACE_END_HAL;

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
        if (rParamIsp.rRequestSet.vNumberSet[0] > 0)
            validateP1(rParamIsp, MTRUE);
        AAA_TRACE_END_ISP;
        AAA_TRACE_END_D;

        /*****************************
         *     Release Statistic
         *****************************/
        m_pTaskMgr->releaseStt(Task_Update_3A);

        m_u4LastRequestNumber = pParam->i4MagicNum;
    }

    /*****************************
     *     Stereo flow
     *****************************/
#if CAM3_STEREO_FEATURE_EN
    if (r3ACmd == ECmd_PrecaptureStart)
    {
        // suspend Sync3A when precapture with flash
        if( (m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||
            ((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isStrobeBVTrigger()))
        {
            MY_LOGD("[%s] skip Sync3A for precapture with flash +", __FUNCTION__);
            Stereo_Param_T rStereoParam;
            Stereo_Param_T rLastParam = m_rLastStereoParam;
            rStereoParam.i4Sync2AMode= NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
            rStereoParam.i4SyncAFMode= NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
            rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
            setStereoParams(rStereoParam);
            m_bIsSkipSync3A = MTRUE;
            m_rLastStereoParam = rLastParam;
            MY_LOGD("[%s] m_rLastStereoParam(%d, %d, %d)", __FUNCTION__,
                m_rLastStereoParam.i4Sync2AMode,
                m_rLastStereoParam.i4SyncAFMode,
                m_rLastStereoParam.i4HwSyncMode);
        }
    }
    if(m_bIsCapEnd && m_bIsSkipSync3A)
    {
        // restore Sync3A
        MY_LOGD("[%s] m_rLastStereoParam(%d, %d, %d)", __FUNCTION__,
                    m_rLastStereoParam.i4Sync2AMode,
                    m_rLastStereoParam.i4SyncAFMode,
                    m_rLastStereoParam.i4HwSyncMode);
        m_bIsSkipSync3A = MFALSE;
        setStereoParams(m_rLastStereoParam);
        MY_LOGD("[%s] skip Sync3A for precapture with flash -", __FUNCTION__);
    }
#endif

    MY_LOGD_IF(m_3ALogEnable,"Task done.");

    // query magic number after deque sw buffer.
    MY_LOGD_IF(m_3ALogEnable,"[%s] m_i4SttMagicNumber = %d", __FUNCTION__, m_i4SttMagicNumber);

    do {
        android::sp<ISttBufQ> pSttBufQ;
        android::sp<ISttBufQ::DATA> pData;

        pSttBufQ = NSCam::ISttBufQ::getInstance(m_i4SensorDev);
        if (pSttBufQ == NULL) break;

        pData = pSttBufQ->deque_last();
        if (!pData.get()) {
            MY_LOGE("SttBufQ empty!!");
            break;
        }
        if(pData->MagicNumberRequest != 0) {
            MY_LOGW("SttBufQ request=%d -> request=%d", pData->MagicNumberRequest, m_u4LastRequestNumber);
        }
        pData->MagicNumberRequest = m_u4LastRequestNumber;

        pSttBufQ->enque(pData);
    } while(0);

    MY_LOGD_IF(m_3ALogEnable, "[%s]-", __FUNCTION__);
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
        MY_LOGE("MAKE_HalSensorList() == NULL");
        return E_3A_ERR;
    }
    const char* const callerName = "Hal3AQueryTG";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
    //Note that Middleware has configured sensor before
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = pHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);
    pHalSensor->destroyInstance(callerName);

    MY_LOGD_IF(m_3ALogEnable, "m_i4SensorDev = %d, senInfo.TgInfo = %d\n", m_i4SensorDev, senInfo.TgInfo);

    if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
    {
        MY_LOGE("RAW sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
        return E_3A_ERR;
    }

    m_u4TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2

    IAwbMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);
    IspTuningMgr::getInstance().setTGInfo(m_i4SensorDev, m_u4TgInfo);

    m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&m_i4TgWidth, (MINTPTR)&m_i4TgHeight, 0);
    MY_LOGD_IF(m_3ALogEnable, "[%s] TG size(%d,%d)", __FUNCTION__, m_i4TgWidth, m_i4TgHeight);
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
    MY_LOGD_IF(m_3ALogEnable, "[%s] i4AEISOSpeedMode:%d, rAESensorInfo.u4Sensitivity:%d \n", __FUNCTION__, i4AEISOSpeedMode, rAESensorInfo.u4Sensitivity);

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
getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const
{
    rDbg3AInfo1 = {};
    rDbg3AInfo2 = {};
    rDbgShadTbl = {};
    COMMON_DEBUG_INFO1_T& rInfo1 = rDbg3AInfo1.hdr.rCommDebugInfo;
    COMMON_DEBUG_INFO2_T& rInfo2 = rDbg3AInfo2.hdr.rCommDebugInfo;
    MY_LOGD("[%s] Size(%d), AE(%d/%d), AF(%d/%d), Flash(%d/%d), Flicker(%d/%d), Shading(%d/%d)", __FUNCTION__, rInfo1.u4Size
              , rInfo1.rAE.chkSum, rInfo1.rAE.ver, rInfo1.rAF.chkSum, rInfo1.rAF.ver, rInfo1.rFLASH.chkSum, rInfo1.rFLASH.ver
              , rInfo1.rFLICKER.chkSum, rInfo1.rFLICKER.ver, rInfo1.rSHADING.chkSum, rInfo1.rSHADING.ver);
    MY_LOGD("[%s] Size(%d), AWB(%d/%d), ISP(%d/%d)", __FUNCTION__, rInfo2.u4Size, rInfo2.rAWB.chkSum, rInfo2.rAWB.ver
              , rInfo2.rISP.chkSum, rInfo2.rISP.ver);

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
    FlashMgr::getInstance(m_i4SensorDev)->getDebugInfo(&rDbg3AInfo1.rFlashDebugInfo);
#endif

    // Flicker
    //Flicker::getInstance()->getDebugInfo(&rDbg3AInfo1.rFlickerDebugInfo);

#if CAM3_LSC_FEATURE_EN
    // LSC
    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
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
                MY_LOGD("[%s] E3ACtrl_SetMinMaxFps (%d,%d)", __FUNCTION__, m_i4OverrideMinFrameRate, m_i4OverrideMaxFrameRate);
            }
            IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, m_i4OverrideMinFrameRate, m_i4OverrideMaxFrameRate);
            break;
        case E3ACtrl_SetCaptureMaxFPS:
            MY_LOGD("[%s] E3ACtrl_SetCaptureMaxFPS (not support now)", __FUNCTION__);
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
            *(reinterpret_cast<MINT32*>(iArg1)) = 2;
            //*(reinterpret_cast<MINT32*>(iArg1)) = m_i4SensorPreviewDelay;
            break;
        case E3ACtrl_GetSensorDelayInfo:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4AeShutDelayFrame;
            *(reinterpret_cast<MINT32*>(iArg2)) = m_i4AeISPGainDelayFrame;
            break;
        case E3ACtrl_SetCCUCB:
            m_bCCUEn = IAeMgr::getInstance().getCCUresultCBActive(m_i4SensorDev, (reinterpret_cast<MVOID*>(iArg1)));
            MY_LOGD("[%s()] m_bCCUEn:%d", __FUNCTION__, m_bCCUEn);
            i4Ret = S_3A_OK;
            break;
        case E3ACtrl_EnableTgInt:
            i4Ret = IAeMgr::getInstance().getTgIntAEInfo(m_i4SensorDev, m_bTgIntAEEn, m_fTgIntAERatio);
            *(reinterpret_cast<MBOOL*>(iArg1)) = m_bTgIntAEEn;
            *(reinterpret_cast<MFLOAT*>(iArg2)) = m_fTgIntAERatio;
            MY_LOGD_IF(m_bTgIntAEEn, "[%s()] m_bTgIntAEEn:%d, TG_INT_AE_RATIO:%f", __FUNCTION__, m_bTgIntAEEn, m_fTgIntAERatio);
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
            MY_LOGD("[%s()] E3ACtrl_GetInitExpoSetting: u4SensorMode(%d) u4AETargetMode(%d) u4Eposuretime(%d) u4AfeGain(%d) u4Eposuretime_se(%d) u4AfeGain_se(%d) u4Eposuretime_me(%d) u4AfeGain_me(%d) u4Eposuretime_vse(%d) u4AfeGain_vse(%d)",
                      __FUNCTION__, a_rAEInitExpoSetting->u4SensorMode, a_rAEInitExpoSetting->u4AETargetMode,
                      a_rAEInitExpoSetting->u4Eposuretime, a_rAEInitExpoSetting->u4AfeGain,
                      a_rAEInitExpoSetting->u4Eposuretime_se, a_rAEInitExpoSetting->u4AfeGain_se,
                      a_rAEInitExpoSetting->u4Eposuretime_me, a_rAEInitExpoSetting->u4AfeGain_me,
                      a_rAEInitExpoSetting->u4Eposuretime_vse, a_rAEInitExpoSetting->u4AfeGain_vse);
            break;
        case E3ACtrl_GetPrioritySetting:
            AE_EXP_SETTING_T *rAEInputExpoSetting;// = *reinterpret_cast<AE_EXP_SETTING_T*>(iArg1);
            rAEInputExpoSetting = reinterpret_cast<AE_EXP_SETTING_T*>(iArg1);
            AE_EXP_SETTING_T* a_rAEOutputExpoSetting;
            a_rAEOutputExpoSetting = reinterpret_cast<AE_EXP_SETTING_T*>(iArg2);
            i4Ret = IAeMgr::getInstance().switchExpSettingByShutterISOpriority(m_i4SensorDev, *reinterpret_cast<AE_EXP_SETTING_T*>(iArg1), *reinterpret_cast<AE_EXP_SETTING_T*>(iArg2));
            MY_LOGD("[%s()] E3ACtrl_GetPrioritySetting: Input Shutter(%d) ISO(%d) Output Shutter(%d) ISO(%d)",
                      __FUNCTION__, rAEInputExpoSetting->u4ExposureTime, rAEInputExpoSetting->u4Sensitivity,
                      a_rAEOutputExpoSetting->u4ExposureTime, a_rAEOutputExpoSetting->u4Sensitivity);
            break;
#if 0 //CAM3_STEREO_FEATURE_EN
        case E3ACtrl_GetAEStereoDenoiseInfo:
            AE_MODE_CFG_T rDenoiseInfoInfo;
            NSIspTuning::ISP_INFO_T* pIspInfo;
            i4Ret = IAeMgr::getInstance().getStereoDenoiseInfo(m_i4SensorDev, rDenoiseInfoInfo);
            pIspInfo = reinterpret_cast<NSIspTuning::ISP_INFO_T*>(iArg1);
            if(pIspInfo->rCamInfo.rAEInfo.u4RealISOValue != rDenoiseInfoInfo.u4RealISO)
            {
                MY_LOGD("[%s()] ISO (%d -> %d)", __FUNCTION__, pIspInfo->rCamInfo.rAEInfo.u4RealISOValue, rDenoiseInfoInfo.u4RealISO);
                pIspInfo->rCamInfo.rAEInfo.u4RealISOValue = rDenoiseInfoInfo.u4RealISO;
                MINT32 i4AEISOSpeedMode = IAeMgr::getInstance().getAEISOSpeedMode(m_i4SensorDev);
                *(reinterpret_cast<MUINT32*>(iArg2))  = (i4AEISOSpeedMode ==LIB3A_AE_ISO_SPEED_AUTO) ? rDenoiseInfoInfo.u4RealISO : i4AEISOSpeedMode;
            }
            if(pIspInfo->rCamInfo.rAEInfo.u4IspGain != rDenoiseInfoInfo.u4IspGain)
            {
                MY_LOGD("[%s()] IspGain (%d -> %d)", __FUNCTION__, pIspInfo->rCamInfo.rAEInfo.u4IspGain, rDenoiseInfoInfo.u4IspGain);
                pIspInfo->rCamInfo.rAEInfo.u4IspGain = rDenoiseInfoInfo.u4IspGain;
            }
            break;
#endif
        case E3ACtrl_EnableAEStereoManualPline:
            MY_LOGD("[%s] DualCamera control (%d)", __FUNCTION__, (MBOOL)iArg1);
            IAeMgr::getInstance().enableAEStereoManualPline(m_i4SensorDev, (MBOOL)iArg1);
            break;
        // ----------------------------------ISP----------------------------------
        case E3ACtrl_GetIspGamma:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA, iArg1, iArg2);
            break;
        case E3ACtrl_ValidatePass1:
            {
                MINT32 i4Magic = (MINT32)iArg1;
                NSIspTuning::EIspProfile_T prof = static_cast<NSIspTuning::EIspProfile_T>(iArg2);
                RequestSet_T rRequestSet;
                rRequestSet.vNumberSet.clear();
                rRequestSet.vNumberSet.push_back(i4Magic);
                NS3Av3::ParamIspProfile_T _3AProf(prof, i4Magic, 1, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);
                _3AProf.i4MagicNum = i4Magic;
                IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, prof);
                validateP1(_3AProf, MTRUE);
            }
            break;
        case E3ACtrl_SetIspProfile:
            IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, static_cast<NSIspTuning::EIspProfile_T>(iArg1));
            m_rParam.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(iArg1);
            break;

        case E3ACtrl_GetOBOffset:
        {
            const ISPResultToMeta_T *pISPResult = (const ISPResultToMeta_T*)m_pResultPoolObj->getResult(m_rParam.i4MagicNum, E_ISP_RESULTTOMETA);
            MY_LOGD("[%s], GetOBOffset ISPResult(%p) at MagicNum(%d)", __FUNCTION__, pISPResult, m_rParam.i4MagicNum);
            if (!pISPResult)
            {
                MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
                m_pResultPoolObj->getHistory(rHistoryReqMagic);
                pISPResult = (const ISPResultToMeta_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_ISP_RESULTTOMETA);
                MY_LOGD("[%s], Get ISPResult(%p) at MagicNum(%d) Instead", __FUNCTION__, pISPResult, rHistoryReqMagic[1]);
            }
            MINT32 *OBOffset = reinterpret_cast<MINT32*>(iArg1);

            OBOffset[0] = pISPResult->rCamInfo.rOBC1.offst0.val;
            OBOffset[1] = pISPResult->rCamInfo.rOBC1.offst1.val;
            OBOffset[2] = pISPResult->rCamInfo.rOBC1.offst2.val;
            OBOffset[3] = pISPResult->rCamInfo.rOBC1.offst3.val;
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
            // arg1 : meta data
            // arg2 : image buffer
            //reference : pdenode
#if (!MT6763_API) && (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
            IAfMgr::getInstance().doSWPDE(m_i4SensorDev, (MVOID*)iArg1, (MVOID*)iArg2);
#else
            MY_LOGW("[%s] unsupport cmd (%d)", __FUNCTION__, e3ACtrl);
#endif
            break;
        case E3ACtrl_GetOISPos:
            break;
        case E3ACtrl_Get_AF_FSC_INIT_INFO:
            IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_GetFSCInitInfo, iArg1, 0);
            break;
        case E3ACtrl_NOTIFY_AF_FSC_INFO:
            if (m_pCbSet)
            {
                m_pCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FSC_INFO, iArg1, 0, 0);
            }
            else
                MY_LOGE("[%s] m_pCbSet NULL", __FUNCTION__);
        break;
#endif

#if CAM3_LSC_FEATURE_EN
        // --------------------------------- Shading ---------------------------------
        case E3ACtrl_SetShadingSdblkCfg:
            if (NSIspTuning::EOperMode_Meta != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev))
                i4Ret = ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->CCTOPSetSdblkFileCfg((iArg1 ? MTRUE : MFALSE), reinterpret_cast<const char*>(iArg2));
            break;
        case E3ACtrl_SetShadingEngMode:
            m_fgEnableShadingMeta = MFALSE;
            if (NSIspTuning::EOperMode_Meta != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev))
            {
                switch (iArg1)
                {
                case 0: // Disable
                    IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(2);
                    break;
                case 1: // Auto
                    IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    break;
                case 2: // Low
                    IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(0);
                    break;
                case 3: // Mid
                    IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(1);
                    break;
                case 4: // High
                    IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MFALSE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(2);
                    break;
                case 5: // TSF
                default:
                    IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setTsfOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setOnOff(MTRUE);
                    ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->setCTIdx(2);
                    break;
                }
            }
            break;
        case E3ACtrl_SetShadingByp123:
            if (NSIspTuning::EOperMode_Meta != IspTuningMgr::getInstance().getOperMode(m_i4SensorDev))
                ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->CCTOPSetBypass123(iArg1 ? MTRUE : MFALSE);
            break;
#endif

#if CAM3_FLASH_FEATURE_EN
        // --------------------------------- Flash ---------------------------------
        case E3ACtrl_GetQuickCalibration:
            i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctGetQuickCalibrationResult();
            *(reinterpret_cast<MINT32*>(iArg1)) = i4Ret;
            MY_LOGD_IF(m_3ALogEnable, "getFlashQuickCalibrationResult ret=%d", i4Ret);
            break;
        case E3ACtrl_EnableFlashQuickCalibration:
            if(iArg1==1)
                i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctSetSpModeQuickCalibration2();
            else
                i4Ret = FlashMgr::getInstance(m_i4SensorDev)->cctSetSpModeNormal();
            break;
        case E3ACtrl_SetIsFlashOnCapture:
            FlashMgr::getInstance(m_i4SensorDev)->setIsFlashOnCapture(iArg1);
            MY_LOGD_IF(m_3ALogEnable, "setIsFlashOnCapture=%ld", (long)iArg1);
            break;
        case E3ACtrl_GetIsFlashOnCapture:
            *(reinterpret_cast<MINT32*>(iArg1)) = FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture();
            break;
        case E3ACtrl_ChkMFNRFlash:
            // Any platform under P40 would take default branch which returns MFALSE
            return MFALSE;
        case E3ACtrl_GetIsFlashLowPower:
            *(reinterpret_cast<MBOOL*>(iArg1)) = (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING);
            break;
#endif

        // --------------------------------- flow control ---------------------------------
        case E3ACtrl_Enable3ASetParams:
            m_bEnable3ASetParams = iArg1;
            MY_LOGD("[%s] E3ACtrl_Enable3ASetParams %d\n",__FUNCTION__, m_bEnable3ASetParams);
            break;
        case E3ACtrl_SetOperMode:
            MINT32 i4OperMode;
            i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            if(i4OperMode != EOperMode_Meta)
                i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
            MY_LOGD_IF(m_3ALogEnable, "[%s] prev_mode(%d), new_mode(%ld)", __FUNCTION__, i4OperMode, (long)iArg1);
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

            rFeatureParam->bCcuIsSupportSecurity = 0;
#if CAM3_ICCU_EN
            if(NSCcuIf::ICcuMgrExt::ccuIsSupportSecurity())
                rFeatureParam->bCcuIsSupportSecurity = 1;
            else
            {
                MY_LOGW("[%s] Fail get ccu interface", __FUNCTION__);
            }
#endif
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
            rDualZoomInfo->bOISIsValid = 0;
            rDualZoomInfo->fOISHallPosXum  = 0;
            rDualZoomInfo->fOISHallPosYum  = 0;
#endif
            break;
        case E3ACtrl_Sync3A_IsActive:
            *(reinterpret_cast<MBOOL*>(iArg1)) = ISync3AMgr::getInstance()->isActive();
            break;
        case E3ACtrl_Sync3A_Sync2ASetting:
            NS3Av3::ISync3AMgr::getInstance()->getSync3A()->sync2ASetting((MINT32)iArg1, (MINT32)iArg1);
            break;
#endif
        case E3ACtrl_GetCurrResult:
            {
                MUINT32 u4Magic = (MUINT32)iArg1;
                getCurrResult(u4Magic);
            }
            break;
        default:
            MY_LOGD_IF(m_3ALogEnable, "[%s] Unsupport Command(%d)", __FUNCTION__, e3ACtrl);
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
    MY_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    m_pTaskMgr->attachCb(cb);
    return 0;
}

MINT32
Hal3ARawImp::
detachCb(I3ACallBack* cb)
{
    MY_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
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
        MY_LOGE("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer

    m_i4SensorPreviewDelay = sensorStaticInfo.previewDelayFrame;
    m_i4AeShutDelayFrame = sensorStaticInfo.aeShutDelayFrame;
    m_i4AeISPGainDelayFrame = sensorStaticInfo.aeISPGainDelayFrame;

    //long exposure for N+1 or N+2 frame
    const char* const callerName = "Hal3AQueryAEEffectiveFrame";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
    MINT32 i4Ret  = pHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_AE_EFFECTIVE_FRAME_FOR_LE, (MUINTPTR)&m_i4EffectiveFrame, 0 , 0 );
    pHalSensor->destroyInstance(callerName);

    MY_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), rawFmtType(%d), SensorPreviewDelay(%d), AeShutDelayFrame(%d), AeISPGainDelayFrame(%d) AEEffectiveFrame(%d)\n"
            , __FUNCTION__
            , m_i4SensorDev, m_i4SensorIdx, u4RawFmtType, m_i4SensorPreviewDelay, m_i4AeShutDelayFrame, m_i4AeISPGainDelayFrame
            , m_i4EffectiveFrame);

    // 3A/ISP mgr can query sensor static information here
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE, u4RawFmtType, sensorStaticInfo.PDAF_Support);

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
    return MTRUE;
}


MBOOL
Hal3ARawImp::
notifyPwrOff()
{
    IAfMgr::getInstance().CamPwrOffState(m_i4SensorDev);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyP1PwrOn()
{
    MY_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
#if CAM3_ICCU_EN
    m_pICcuMgr = NSCcuIf::ICcuMgrExt::createInstance("3AHal");
    if(m_pICcuMgr)
    {
        if(m_pICcuMgr->ccuInit() != 0)
        {
            MY_LOGD("[%s] m_pICcuMgr->ccuInit() fail return false", __FUNCTION__);
            return MFALSE;
        }
        if(m_pICcuMgr->ccuBoot() != 0)
        {
            MY_LOGD("[%s] m_pICcuMgr->ccuBoot() fail return false", __FUNCTION__);
            return MFALSE;
        }
        MY_LOGD("[%s] m_pICcuMgr->ccuBoot() and m_pICcuMgr->ccuInit() success return true", __FUNCTION__);
    }
#endif
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyP1PwrOff()
{
#if CAM3_ICCU_EN
    if(m_pICcuMgr == NULL)
        m_pICcuMgr = NSCcuIf::ICcuMgrExt::createInstance("3AHal");

    if(m_pICcuMgr)
    {
        MY_LOGD("[%s] notifyP1PwrOff", __FUNCTION__);
        if(m_pICcuMgr->ccuShutdown() != 0)
        {
            MY_LOGD("[%s] m_pICcuMgr->ccuShutdown() fail return false", __FUNCTION__);
            return MFALSE;
        }
        if(m_pICcuMgr->ccuUninit() != 0)
        {
            MY_LOGD("[%s] m_pICcuMgr->ccuUninit() fail return false", __FUNCTION__);
            return MFALSE;
        }
        MY_LOGD("[%s] m_pICcuMgr->ccuUninit() and m_pICcuMgr->ccuShutdown() success return true", __FUNCTION__);

        m_pICcuMgr->destroyInstance();
        m_pICcuMgr = NULL;
    }
#endif
    return MTRUE;
}

MVOID
Hal3ARawImp::
setIspSensorInfo2AF(MINT32 MagicNum)
{
    ISP_SENSOR_INFO_T rIspSensorInfo;
    const ISPResultToMeta_T *pISPResult = NULL;
    pISPResult = (const ISPResultToMeta_T*)m_pResultPoolObj->getResult(MagicNum, E_ISP_RESULTTOMETA);

    if(pISPResult == NULL)
        MY_LOGW("[%s] pISPResult NULL", __FUNCTION__);
    else
    {
        rIspSensorInfo.i4FrameId = MagicNum;
        rIspSensorInfo.bHLREnable = MFALSE;
        rIspSensorInfo.bAEStable = pISPResult->rCamInfo.rAEInfo.bAEStable;
        rIspSensorInfo.bAELock = pISPResult->rCamInfo.rAEInfo.bAELock;
        rIspSensorInfo.bAEScenarioChange = pISPResult->rCamInfo.rAEInfo.bAEScenarioChange;
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
    MY_LOGD_IF(m_3ALogEnable,"[%s] FrameId:%d AFEGain:%d ISPGain:%d", __FUNCTION__,rIspSensorInfo.i4FrameId, rIspSensorInfo.u4AfeGain, rIspSensorInfo.u4IspGain);
}

MVOID
Hal3ARawImp::
notifyPreStop()
{
    MY_LOGD("[%s] m_bPreStop(%d)", __FUNCTION__, m_bPreStop);
    m_bPreStop = MTRUE;
    m_pThreadRaw->notifyPreStop();
}

MBOOL
Hal3ARawImp::
setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
    AAA_TRACE_HAL(setP2Params);
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
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewP2Param.i4halEdgeMode);
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
            MY_LOGD_IF(m_3ALogEnable & 0x2, "[Red]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
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
            MY_LOGD_IF(m_3ALogEnable & 0x2, "[Green]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
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
            MY_LOGD_IF(m_3ALogEnable & 0x2, "[Blue]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Blue(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);
    }
    AAA_TRACE_END_HAL;

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
            MY_LOGD_IF(m_3ALogEnable & 0x2, "[Blue](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveBlue.push_back(*pIn++);
            pResultP2->vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            MY_LOGD_IF(m_3ALogEnable & 0x2, "[Green](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveGreen.push_back(*pIn++);
            pResultP2->vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            MY_LOGD_IF(m_3ALogEnable & 0x2, "[Red](%f,%f)", *pIn, *pOut);
            pResultP2->vecTonemapCurveRed.push_back(*pIn++);
            pResultP2->vecTonemapCurveRed.push_back(*pOut++);
        }
        MY_LOGD_IF(m_3ALogEnable,"[%s] rsize(%d) gsize(%d) bsize(%d)"
            ,__FUNCTION__, (MINT32)pResultP2->vecTonemapCurveRed.size(), (MINT32)pResultP2->vecTonemapCurveGreen.size(), (MINT32)pResultP2->vecTonemapCurveBlue.size());

    }
    return MTRUE;
}

MBOOL
Hal3ARawImp::
setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type)
{
    AAA_TRACE_HAL(setISPInfo);
    // type == 0 would do the all set
    if (type < 1) {
        rIspInfo.rCamInfo.bBypassLCE                        = rNewP2Param.bBypassLCE;
        rIspInfo.rCamInfo.i4P2InImgFmt                      = rNewP2Param.i4P2InImgFmt;
        rIspInfo.rCamInfo.NR3D_Data.GMVX                    = rNewP2Param.NR3D_Data[0];
        rIspInfo.rCamInfo.NR3D_Data.GMVY                    = rNewP2Param.NR3D_Data[1];
        rIspInfo.rCamInfo.NR3D_Data.confX                   = rNewP2Param.NR3D_Data[2];
        rIspInfo.rCamInfo.NR3D_Data.confY                   = rNewP2Param.NR3D_Data[3];
        rIspInfo.rCamInfo.NR3D_Data.MAX_GMV                 = rNewP2Param.NR3D_Data[4];
        rIspInfo.rCamInfo.NR3D_Data.frameReset              = rNewP2Param.NR3D_Data[5];
        rIspInfo.rCamInfo.NR3D_Data.GMV_Status              = rNewP2Param.NR3D_Data[6];
        rIspInfo.rCamInfo.NR3D_Data.ISO_cutoff              = rNewP2Param.NR3D_Data[7];
        rIspInfo.rCamInfo.NR3D_Data.isGyroValid             = rNewP2Param.NR3D_Data[8];
        rIspInfo.rCamInfo.NR3D_Data.gyroXAccelX1000         = rNewP2Param.NR3D_Data[9];
        rIspInfo.rCamInfo.NR3D_Data.gyroYAccelX1000         = rNewP2Param.NR3D_Data[10];
        rIspInfo.rCamInfo.NR3D_Data.gyroZAccelX1000         = rNewP2Param.NR3D_Data[11];
        rIspInfo.rCamInfo.NR3D_Data.gyroTimeStampHigh       = rNewP2Param.NR3D_Data[12];
        rIspInfo.rCamInfo.NR3D_Data.gyroTimeStampLow        = rNewP2Param.NR3D_Data[13];

        rIspInfo.rCamInfo.eEdgeMode                         = static_cast<mtk_camera_metadata_enum_android_edge_mode_t>(rNewP2Param.u1appEdgeMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Edge      = static_cast<EIndex_Isp_Edge_T>(rNewP2Param.i4halEdgeMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Bright    = static_cast<EIndex_Isp_Brightness_T>(rNewP2Param.i4BrightnessMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Contrast  = static_cast<EIndex_Isp_Contrast_T>(rNewP2Param.i4ContrastMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Hue       = static_cast<EIndex_Isp_Hue_T>(rNewP2Param.i4HueMode);
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Sat       = static_cast<EIndex_Isp_Saturation_T>(rNewP2Param.i4SaturationMode);
        rIspInfo.rCamInfo.eToneMapMode                      = static_cast<mtk_camera_metadata_enum_android_tonemap_mode_t>(rNewP2Param.u1TonemapMode);
    }

    if (type < 2) {
        //     _reprocess  part

        rIspInfo.rCamInfo.fgRPGEnable = !rNewP2Param.u1PGN;

        if (rNewP2Param.u1IspProfile != 255)
        {
            rIspInfo.rCamInfo.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP2Param.u1IspProfile);
        }
        else
        {
            switch (rNewP2Param.u1CapIntent)
            {
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                rIspInfo.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                rIspInfo.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
                if(rIspInfo.rCamInfo.fgRPGEnable){
                    rIspInfo.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Preview;
                }else{
                    rIspInfo.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Capture;
                }
                break;
            }
        }

        rIspInfo.rCamInfo.i4RawType = rNewP2Param.i4RawType;

        if (((!rIspInfo.rCamInfo.fgRPGEnable) && (rNewP2Param.i4RawType == NSIspTuning::ERawType_Pure)) || m_i4CopyLscP1En == 1)
        {
            if (rNewP2Param.rpLscData)
            {
                rIspInfo.rLscData = std::vector<MUINT8>((rNewP2Param.rpLscData)->array(), (rNewP2Param.rpLscData)->array()+(rNewP2Param.rpLscData)->size());
            }
            else
                MY_LOGD("[%s] No shading entry in metadata\n", __FUNCTION__);
        }

        if(rIspInfo.rLscData.size())
        {
            MUINT32 mu4DumpLscP1En=0;
            MUINT32 mu4DumpLscP1CapEn=0;
            MUINT32 mu4DumpLscP2CapEn=0;
            GET_PROP("vendor.debug.camera.dump.p1.lsc", 0, mu4DumpLscP1En);
            GET_PROP("vendor.debug.camera.dump.cap.lsc", 0, mu4DumpLscP1CapEn);
            GET_PROP("vendor.debug.camera.dump.lsc2", 0, mu4DumpLscP2CapEn);

            if(mu4DumpLscP1En || (mu4DumpLscP1CapEn && rIspInfo.isCapture))
            {
                char filename[256] = {'\0'};
                rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.eIspProfile;
                genFileName_LSC(filename, sizeof(filename), &rIspInfo.hint);
                sp<IFileCache> fidLscDump;
                fidLscDump = IFileCache::open(filename);
                if (fidLscDump->write(&rIspInfo.rLscData[0], rIspInfo.rLscData.size()) != rIspInfo.rLscData.size())
                {
                    MY_LOGD("[%s] write error %s", __FUNCTION__, filename);
                }
            }
            if(mu4DumpLscP2CapEn && rIspInfo.isCapture)
            {
                NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));

                ILscTable::Config rCfg;
                ILscTbl::RsvdData rRsvd;
                ::memcpy(&rCfg, rIspInfo.rLscData.data(), sizeof(ILscTable::Config));
                ::memcpy(&rRsvd, rIspInfo.rLscData.data()+sizeof(ILscTable::Config), sizeof(ILscTbl::RsvdData));

                MUINT32 u4W, u4H;
                if (pLsc->getRawSize((ESensorMode_T) rIspInfo.rCamInfo.eSensorMode, u4W, u4H))
                {

                    ILscTbl rTbl(ILscTable::HWTBL, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY);
                    rTbl.setRsvdData(rRsvd);
                    rTbl.setData(rIspInfo.rLscData.data()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData), rTbl.getSize());

                    char strlscdump[512] = {'\0'};
                    rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.eIspProfile;
                    genFileName_LSC2(strlscdump, sizeof(strlscdump), &rIspInfo.hint);

                    android::sp<IFileCache> fidLscDump;
                    fidLscDump = IFileCache::open(strlscdump);
                    if (fidLscDump->write(&rTbl.getConfig(), sizeof(ILscTable::Config)) != sizeof(ILscTable::Config))
                        MY_LOGW("write config error to %s", strlscdump);
                    if (fidLscDump->write(&rTbl.getRsvdData().u4HwRto, sizeof(MUINT32)) != sizeof(MUINT32))
                        MY_LOGW("write rsvdata error to %s", strlscdump);
                    if (fidLscDump->write(rIspInfo.rLscData.data()+sizeof(ILscTable::Config)+sizeof(MUINT32), rTbl.getSize()) != rTbl.getSize())
                        MY_LOGW("write shading table error to %s", strlscdump);

                } else {
                    MY_LOGE("[%s] unsupported sensor mode %d", __FUNCTION__, rIspInfo.rCamInfo.eSensorMode);
                }
            }
        }

        rIspInfo.i4UniqueKey = rNewP2Param.i4UniqueKey;

        if (rNewP2Param.rpP1Crop && rNewP2Param.rpRzSize && rNewP2Param.rpRzInSize &&
            (rIspInfo.rCamInfo.fgRPGEnable))  //||
             //rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise  ||
             //rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise_toGGM))
        {
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstX    = rNewP2Param.rpP1Crop->p.x;
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstY    = rNewP2Param.rpP1Crop->p.y;
            rIspInfo.rCamInfo.rCropRzInfo.i4Width    = rNewP2Param.rpP1Crop->s.w;
            rIspInfo.rCamInfo.rCropRzInfo.i4Height   = rNewP2Param.rpP1Crop->s.h;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzWidth  = rNewP2Param.rpRzSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzHeight = rNewP2Param.rpRzSize->h;
            rIspInfo.rCamInfo.rCropRzInfo.fgOnOff    = MTRUE;

            rIspInfo.rCamInfo.rCropRzInfo.i4FullW    = rNewP2Param.rpRzInSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.i4FullH    = rNewP2Param.rpRzInSize->h;
        }
        else
        {
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstX    = 0;
            rIspInfo.rCamInfo.rCropRzInfo.i4OfstY    = 0;
            rIspInfo.rCamInfo.rCropRzInfo.i4Width    = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rCamInfo.rCropRzInfo.i4Height   = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzWidth  = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rCamInfo.rCropRzInfo.i4RzHeight = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rCamInfo.rCropRzInfo.fgOnOff    = MFALSE;
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
        rIspInfo.rCamInfo.eControlMode = static_cast<mtk_camera_metadata_enum_android_control_mode_t>(rNewP2Param.u1ControlMode);

        if (rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_YUV_Reprocess)
        {
            rIspInfo.rCamInfo.rAEInfo.u4RealISOValue= rNewP2Param.i4ISO;
            rIspInfo.rCamInfo.eIdx_Scene = static_cast<NSIspTuning::EIndex_Scene_T>(0);  //MTK_CONTROL_SCENE_MODE_DISABLED
            rIspInfo.rCamInfo.eSensorMode = NSIspTuning::ESensorMode_Capture;
        }
    }

    AAA_TRACE_END_HAL;

    return MTRUE;
}

MBOOL
Hal3ARawImp::
preset(Param_T const& /*rNewParam*/)
{
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notifyResult4TG(MINT32 i4PreFrmId)
{
    MY_LOGD_IF(m_3ALogEnable, "[%s] Previous FrmId(%d) +", __FUNCTION__, i4PreFrmId);

    // ResultPool - To update Vector info
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(i4PreFrmId);
    if(pAllResult == NULL)
    {
        MY_LOGE("[%s] pAllResult is NULL", __FUNCTION__);
        return MFALSE;
    }
    else
        MY_LOGD_IF(m_3ALogEnable, "[%s]  pAllResult(%p)", __FUNCTION__, pAllResult);

    AEResultToMeta_T  *pAEResult = (AEResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read());
    ISPResultToMeta_T *pISPResult = (ISPResultToMeta_T*)(pAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read());
    LSCResultToMeta_T *pLSCResult = ( (LSCResultToMeta_T*)(pAllResult->ModuleResultAddr[E_LSC_RESULTTOMETA]->read()) );
    HALResultToMeta_T *pHALResult = (HALResultToMeta_T*)(pAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read());

    //===== set AE info to previous result pool =====
    AE_SENSOR_PARAM_T rAESensorInfo;
    AEResultToMeta_T rAEResult;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    // Copy original result
    rAEResult.fgAeBvTrigger = pAEResult->fgAeBvTrigger;
    rAEResult.i4AutoHdrResult = pAEResult->i4AutoHdrResult;
    rAEResult.i8SensorRollingShutterSkew = pAEResult->i8SensorRollingShutterSkew;
    rAEResult.u1AeState = pAEResult->u1AeState;
    // set TG result to local member
    rAEResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;//ISO value updated
    rAEResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rAEResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;

    MY_LOGD_IF(m_3ALogEnable, "[%s] AECal i8SensorExposureTime(%lld), i8SensorFrameDuration(%lld), i4SensorSensitivity(%d)", __FUNCTION__,
            (long long)rAESensorInfo.u8ExposureTime, (long long)rAESensorInfo.u8FrameDuration, rAESensorInfo.u4Sensitivity);
    //update to resultPool
    m_pResultPoolObj->updateResult(LOG_TAG, i4PreFrmId, E_AE_RESULTTOMETA, &rAEResult);

    //===== set new LSC ratio to previous result pool =====
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));
    MUINT32 u4Rto = pLsc->getRatio();
#if 0
    if(pAllResult->vecLscData.size() != 0)
    {
        MUINT8* pDest = pAllResult->vecLscData.editArray();
        *((MUINT32*)(pDest+sizeof(ILscTable::Config))) = u4Rto;
    }
#endif

    //===== set info to Exif =====
    MINT32 OBCGain = ISP_MGR_OBC_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).getOBCGain();

    MY_LOGD_IF(m_3ALogEnable, "[%s] Rto(%d), CurrTblIndex(%d), OBCGain(%d), i8SensorExposureTime(%lld), i8SensorFrameDuration(%lld), i4SensorSensitivity(%d)", __FUNCTION__,
        u4Rto, pLSCResult->i4CurrTblIndex, OBCGain, (long long)pAEResult->i8SensorExposureTime, (long long)pAEResult->i8SensorFrameDuration, pAEResult->i4SensorSensitivity);

    {
        // protect vector before use vector
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);

        MY_LOGD_IF(m_3ALogEnable, "[%s] vecExifInfo.size(%zu), vecDbg3AInfo.size(%zu), vecDbgIspInfo.size(%zu), m_rParam.u1IsGetExif(%d), m_bIsCapEnd(%d)", __FUNCTION__,
            pAllResult->vecExifInfo.size(), pAllResult->vecDbg3AInfo.size(), pAllResult->vecDbgIspInfo.size(), m_rParam.u1IsGetExif,m_bIsCapEnd);
    }

    ISPResultToMeta_T rISPResult;
    if (!IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rISPResult.rCamInfo))
    {
        MY_LOGE("Fail to get CamInfo");
    }
    // update to resultPool
    m_pResultPoolObj->updateResult(LOG_TAG, i4PreFrmId, E_ISP_RESULTTOMETA, &rISPResult);

    MY_LOGD_IF(m_3ALogEnable,"[%s] Rto(%d), u4RealISOValue(%d), u4Eposuretime(%d), u4AfeGain(%d), u4IspGain(%d)", __FUNCTION__,
        u4Rto, pISPResult->rCamInfo.rAEInfo.u4RealISOValue, pISPResult->rCamInfo.rAEInfo.u4Eposuretime, pISPResult->rCamInfo.rAEInfo.u4AfeGain, pISPResult->rCamInfo.rAEInfo.u4IspGain);

    if (m_rParam.u1IsGetExif || m_bIsCapEnd)
    {
        // protect vector before use vector
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        if( pAllResult->vecExifInfo.size() > 0 && pAllResult->vecDbg3AInfo.size() > 0 && pAllResult->vecDbgIspInfo.size() > 0)
        {
            // standard exif
            //get3AEXIFInfo( rPreResult->vecExifInfo.editTop() );

            // debug exif
            AAA_DEBUG_INFO1_T& rDbg3AInfo1 = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResult->vecDbg3AInfo.editArray());
            AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
            NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;

            AE_PLINE_DEBUG_INFO_T tmpBuff;
            // AE
            AE_MODE_CFG_T rPreviewInfo;
            IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, rPreviewInfo);
            MY_LOGD_IF(m_3ALogEnable, "[%s] (EXP,Afe,Ispgain,ISO)(%d, %d, %d, %d) -> (%d, %d, %d, %d)", __FUNCTION__,
                        rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue,
                        rPreviewInfo.u4Eposuretime, rPreviewInfo.u4AfeGain, rPreviewInfo.u4IspGain, rPreviewInfo.u4RealISO);
            rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue = rPreviewInfo.u4Eposuretime;
            rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue = rPreviewInfo.u4AfeGain;
            rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue = rPreviewInfo.u4IspGain;
            rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue = rPreviewInfo.u4RealISO;

            //Avoid memory copy for AE debug info
            //IAeMgr::getInstance().getDebugInfo(m_u4SensorDev, rDbg3AInfo1.rAEDebugInfo, tmpBuff);

#if CAM3_LSC_FEATURE_EN
            // LSC
            //ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
            rDbg3AInfo1.rShadigDebugInfo.Tag[SHAD_TAG_CAM_LSC_RATIO].u4FieldValue = u4Rto;
#endif

            // P1 ISP
            setP1DbgInfo4TG(rDbgIspInfo, u4Rto, OBCGain);
        }

        MY_LOGD_IF(m_3ALogEnable,"[%s] Update rPreResult exif done", __FUNCTION__);
    }

/**************************************************************************************************************
*                                                                                                             *
*                                  Update Result to Meta result Pool - Must using Lock                        *
*                                                                                                             *
**************************************************************************************************************/

    AllMetaResult_T *pMetaResult = m_pResultPoolObj->getMetadata(i4PreFrmId);
    if(pMetaResult == NULL)
        MY_LOGE("[%s] rMetaResult is NULL", __FUNCTION__);
    else
    {
        MY_LOGD_IF(m_3ALogEnable, "[%s] Get Previous Meta FrmId(%d) and lock", __FUNCTION__, pMetaResult->rMetaResult.MagicNum);

        Mutex::Autolock lock(pMetaResult->LockMetaResult);

        MY_LOGD_IF(m_3ALogEnable, "[%s] modify meta result", __FUNCTION__);

        // sensor
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_EXPOSURE_TIME, pAEResult->i8SensorExposureTime);
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_FRAME_DURATION, pAEResult->i8SensorFrameDuration);
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_SENSITIVITY, pAEResult->i4SensorSensitivity);

        IDngInfo* pDngInfo = NULL;
        pDngInfo = MAKE_DngInfo(LOG_TAG, m_i4SensorIdx);
        if(pDngInfo)
        {
            IMetadata rMetaDngDynNoiseProfile = pDngInfo->getDynamicNoiseProfile(pAEResult->i4SensorSensitivity);
            pMetaResult->rMetaResult.appMeta += rMetaDngDynNoiseProfile;
        }

        // shading
        if(pLSCResult->i4CurrTblIndex != -1)
        {
            IMetadata::Memory rLscData;
            QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_LSC_TBL_DATA, rLscData);
            MINT32 i4Size = rLscData.size();
            MY_LOGD("[%s] LscData Size:%d", __FUNCTION__, i4Size);
            if(i4Size)
            {
                *((MUINT32*)(rLscData.array()+sizeof(ILscTable::Config))) = u4Rto;
                UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_LSC_TBL_DATA, rLscData);
                if (pLSCResult->fgShadingMapOn)
                {
                    MUINT8 u1ShadingMode = 0;
                    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SHADING_MODE, u1ShadingMode);
                    if (pDngInfo) {
                        pMetaResult->rMetaResult.appMeta += pDngInfo->getShadingMapFromMem(u1ShadingMode, rLscData);//ToDo other file
                    }
                }
            }
        }

        // tuning
        UPDATE_MEMORY(pMetaResult->rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, pISPResult->rCamInfo);

        // protect vector before use vector
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        // Exif
        if (pAllResult->vecExifInfo.size())
        {
            IMetadata metaExif;

            QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);

            const EXIF_3A_INFO_T& rExifInfo = pAllResult->vecExifInfo[0];

            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_METER_MODE,        rExifInfo.u4AEMeterMode);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_EXP_BIAS,          rExifInfo.i4AEExpBias);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_CAP_EXPOSURE_TIME,    rExifInfo.u4CapExposureTime);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_ISO_SPEED,         rExifInfo.u4AEISOSpeed);

            // debug info
            if (pAllResult->vecDbg3AInfo.size() && pAllResult->vecDbgIspInfo.size())
            {
                MY_LOGD("[%s] DebugInfo #(%d)", __FUNCTION__, pHALResult->i4FrmId);
                IMetadata::Memory dbg3A;
                IMetadata::Memory dbgIspP1;
                dbg3A.appendVector(pAllResult->vecDbg3AInfo);
                dbgIspP1.appendVector(pAllResult->vecDbgIspInfo);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, dbg3A);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIspP1);
            }

            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_ANALOG_GAIN, (MINT32)pISPResult->rCamInfo.rAEInfo.u4AfeGain);
        }
    }
    MY_LOGD_IF(m_3ALogEnable,"[%s] Previous FrmId(%d) rPreResult i4FrmId(%d) -", __FUNCTION__, i4PreFrmId, pHALResult->i4FrmId);
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
    MY_LOGD_IF(m_3ALogEnable, "[%s] + ", __FUNCTION__);

    MINT32 i4Ret = 0;

    if(m_pCamIO != NULL)
    {
        // 1. get OBC gain from isp_mgr_obc
        MINT32 OBCGain = ISP_MGR_OBC_T::getInstance( static_cast<ESensorDev_T>(m_i4SensorDev)).getOBCGain();

        // 2. get ratio
        NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));
        MUINT32 u4Rto = (MINT32)pLsc->getRatio();

        // 2.1 updateTSF
        MINT32 i4MagicNumReq = m_u4LastRequestNumber;
        MY_LOGD_IF(m_3ALogEnable,"[%s] i4MagicNumReq(%d), u4Rto(%d) OBCGain(%d)", __FUNCTION__, i4MagicNumReq, u4Rto, OBCGain);
        pLsc->updateRatio(i4MagicNumReq, u4Rto);//request magicnum, ratio

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
        cmd.serial = (i4MagicNumReq);
        m_pCqTuningMgr->update(cmd);
#endif

    }
    else
        return MFALSE;

    MY_LOGD_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3ARawImp::
notify4CCU(MUINT32 u4PreFrmId, ISP_NVRAM_OBC_T const &OBCResult)
{
    if(m_bCCUEn && m_pThreadSetCCUPara)
    {
        AE_SENSOR_PARAM_T rAESensorInfo;
        IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);

        MY_LOGD_IF(m_3ALogEnable, "[%s] , i8SensorExposureTime(%lld), i8SensorFrameDuration(%lld), i4SensorSensitivity(%d), OBCResult gain0(%d), offst0(%d)", __FUNCTION__,
        (long long)rAESensorInfo.u8ExposureTime, (long long)rAESensorInfo.u8FrameDuration, rAESensorInfo.u4Sensitivity, OBCResult.gain0.val, OBCResult.offst0.val);
        ThreadSetCCUPara::Cmd_T rCmd(u4PreFrmId, m_rParam.u1IsGetExif, m_bIsCapEnd, rAESensorInfo, OBCResult);
        m_pThreadSetCCUPara->postCmd(&rCmd);
    }

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
    AAA_TRACE_D("AE Start");
    IAeMgr::getInstance().setSensorMode(_this->m_i4SensorDev, _this->m_u4SensorMode, _this->m_i4HbinWidth, _this->m_i4HbinHeight);
    IAeMgr::getInstance().setAAOMode(_this->m_i4SensorDev, _this->m_bAAOMode);
    if(_this->m_bIsSMVRTuning)
    {
        IAeMgr::getInstance().setAESMBuffermode(_this->m_i4SensorDev, MTRUE, _this->m_rConfigInfo.i4SubsampleCount);
    }
    else
    {
        IAeMgr::getInstance().setAESMBuffermode(_this->m_i4SensorDev, MFALSE, 1);
    }
    MRESULT err = IAeMgr::getInstance().Start(_this->m_i4SensorDev);
    AAA_TRACE_END_D;
    //IAeMgr::getInstance().setCamScenarioMode(u4SensorDev, rCmd.u4Scenario4AEAWB);
    if (FAILED(err)) {
        MY_LOGE("IAeMgr::getInstance().Start() fail\n");
    }

    ILscMgr::getInstance(static_cast<ESensorDev_T>(_this->m_i4SensorDev))->setCapAAOMagicNum(-1);

    ::pthread_exit((MVOID*)0);
}

MVOID*
Hal3ARawImp::
ThreadAFStart(MVOID* arg)
{
    ::prctl(PR_SET_NAME, "ThreadAFStart", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = NICE_CAMERA_AF_Start;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);
    //::setpriority(PRIO_PROCESS, 0, priority);

    Hal3ARawImp *_this = static_cast<Hal3ARawImp*>(arg);
    //IAfMgr::getInstance().setSensorMode(_this->m_i4SensorDev, _this->m_u4SensorMode, _this->u4AFWidth, _this->u4AFHeight);

#if MTK_CAM_NEW_NVRAM_SUPPORT
    CAM_IDX_QRY_COMB rMapping_Info;
    _this->m_pIdxMgr->getMappingInfo(static_cast<MUINT32>(_this->m_i4SensorDev), rMapping_Info, _this->m_rAfParam.i4MagicNum);
    MUINT32 u4AFNVRAMIndex = _this->m_pIdxMgr->query(static_cast<MUINT32>(_this->m_i4SensorDev), NSIspTuning::EModule_AF, rMapping_Info, __FUNCTION__);
    IAfMgr::getInstance().setNVRAMIndex(_this->m_i4SensorDev, u4AFNVRAMIndex);
#else
    IAfMgr::getInstance().setCamScenarioMode(_this->m_i4SensorDev, Scenario4AF(_this->m_sParam));
#endif
    AAA_TRACE_D("AF Start");
    MRESULT err = IAfMgr::getInstance().Start(_this->m_i4SensorDev);
    AAA_TRACE_END_D;
    if (FAILED(err)) {
        MY_LOGE("AfMgr::getInstance().Start() fail\n");
    }
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
        MY_LOGD("[%s] FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
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
        MY_LOGD("[%s] FlashOpened(%d) m_i4FlashType(%d)",__FUNCTION__, m_bFlashOpenedByTask, m_i4FlashType);
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
    MY_LOGD("[%s]", __FUNCTION__);
    IAeMgr::getInstance().doBackAEInfo(m_i4SensorDev);
    IAwbMgr::getInstance().backup(m_i4SensorDev);
}

MVOID
Hal3ARawImp::doRestore2A()
{
    MY_LOGD("[%s]", __FUNCTION__);
    // restore AE
    IAeMgr::getInstance().doRestoreAEInfo(m_i4SensorDev, MTRUE);
    IAeMgr::getInstance().setRestore(m_i4SensorDev, 0);
    // restore AWB
    IAwbMgr::getInstance().restore(m_i4SensorDev);
}

MVOID
Hal3ARawImp::handleBadPicture(MINT32 i4ReqMagic, MINT32 i4P1DoneSttNum /* = -1 */)
{
    MY_LOGD("[%s]+ ReqMagic(#%d)", __FUNCTION__, i4ReqMagic);
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
        MY_LOGD("[%s] ReqMagic(#%d), i4MagicOfPartialFrame(%d), History(#%d,#%d,#%d)", __FUNCTION__,
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
        HALResultToMeta_T* pHalResult = (HALResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicOfPartialFrame,E_HAL_RESULTTOMETA);
        FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicOfPartialFrame,E_FLASH_RESULTTOMETA);
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
                Mutex::Autolock lock(pMetaResult->LockMetaResult);
                UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_STATE, pFLASHResult->u1FlashState);
            }
        }

        // 4. handle the full flashlight on/off frame
        FLASHResultToMeta_T rFLASHResult;
        pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4ReqMagic,E_FLASH_RESULTTOMETA);
        if(pFLASHResult == NULL)
            pFLASHResult = &rFLASHResult;
        if(pFLASHResult != NULL){
            if(m_bFlashOpenedByTask || m_bIsFlashOpened)
                pFLASHResult->u1FlashState = MTK_FLASH_STATE_FIRED;
            else
                pFLASHResult->u1FlashState = MTK_FLASH_STATE_READY;
            m_pResultPoolObj->updateResult(LOG_TAG,i4ReqMagic,E_FLASH_RESULTTOMETA,pFLASHResult);
        }

        // work around for worse performance
        if(i4ReqMagic-i4P1DoneSttNum >= 3) {
            for(int i=1; i<i4ReqMagic-i4P1DoneSttNum; i++) {
                m_pResultPoolObj->updateResult(LOG_TAG,i4ReqMagic+i,E_FLASH_RESULTTOMETA,pFLASHResult);
                MY_LOGD("[%s] i4ReqMagic(%d), i4P1DoneSttNum(%d), u1FlashState(%d)", __FUNCTION__, i4ReqMagic, i4P1DoneSttNum, pFLASHResult->u1FlashState);
            }
        }
    }

    MY_LOGD("[%s]- i4Ret(%d) ReqMagic(#%d) History(#%d,#%d,#%d)", __FUNCTION__, i4Ret, i4ReqMagic, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);
}


