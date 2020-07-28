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
#ifndef _HAL_3A_RAWIMP_H_
#define _HAL_3A_RAWIMP_H_

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
#include <isp_tuning/isp_tuning_mgr.h>
#include <isp_mgr/isp_mgr.h>
#include <state_mgr/aaa_state_mgr.h>
#include <IThread.h>
#include <atomic>

#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
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
#include <flash_tuning_custom.h>
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

#include "private/PDTblGen.h"

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

#define MY_FORCEINLINE inline __attribute__((always_inline))

// define g/gyro info
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuningv3;
using namespace NSIspExifDebug;

struct FlashCond_T
{
    MUINT32  u4AeMode;
    MUINT32  u4StrobeMode;
    MBOOL    bIsFlashOnCapture;
    MUINT8   u1CaptureIntent;
    MBOOL    bChkMainFlash;
    MBOOL    bChkPreFlash;
    MINT32   i4HWSuppportFlash;
    int      i4FlashState;
    MBOOL    bPreFlashOnOff;
    MBOOL    bFlashOnOff;
    MBOOL    bFlashMainPre;
    FlashCond_T()
        : u4AeMode(0)
        , u4StrobeMode(0)
        , bIsFlashOnCapture(MFALSE)
        , u1CaptureIntent(0)
        , bChkMainFlash(MFALSE)
        , bChkPreFlash(MFALSE)
        , i4HWSuppportFlash(0)
        , i4FlashState(0)
        , bPreFlashOnOff(MFALSE)
        , bFlashOnOff(MFALSE)
        , bFlashMainPre(MFALSE)
    {}
};

class Hal3ARawImp : virtual public Hal3ARaw
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
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond();
    virtual MBOOL       chkPreFlashOnCond();
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
                        Hal3ARawImp(MINT32 const i4SensorDevId);
    virtual             ~Hal3ARawImp(){}

                        Hal3ARawImp(const Hal3ARawImp&);
                        Hal3ARawImp& operator=(const Hal3ARawImp&);

    MBOOL               init(MINT32 i4SensorOpenIndex);
    MBOOL               uninit();
    MRESULT             updateTGInfo();
    MINT32              config(const ConfigInfo_T& rConfigInfo);
    MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;
    MBOOL               setP1DbgInfo4TG(NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo, MUINT32 u4Rto, MINT32 OBCGain) const;
    MBOOL               setTG_INTParams();

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
    MINT32              m_i4sensorId;
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
    MBOOL               m_bIsAfTrigInLowLight;
    MINT32              m_i4KeepPrecapParamsCount;

    MINT32              m_i4OverrideMinFrameRate;
    MINT32              m_i4OverrideMaxFrameRate;

    MINT32              m_i4AETargetMode = AE_MODE_NORMAL;
    MBOOL               m_bCCUEn;
    MBOOL               m_bTgIntAEEn;
    MFLOAT              m_fTgIntAERatio;
    std::vector<AF2AEInfo_T>       AFinfovec;
    std::atomic<bool>   m_bBPCIThreadAlive;

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IspTuningMgr*       m_pTuning;
    IThreadRaw*         m_pThreadRaw;
    StateMgr*           m_pStateMgr;
    AfStateMgr*         m_pAfStateMgr;
    NSCcuIf::ICcuMgrExt* m_pICcuMgr;
    Param_T             m_rParam;
    AF_Param_T          m_rAfParam;
    ScenarioParam       m_sParam;
    ScenarioParam       m_sPreParam;
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
    MBOOL               m_bIsFirstSetParams;
    MINT32              m_i4CamModeEnable;
    MBOOL               m_bDoPreCapInFlashAF;
    MBOOL               m_bDbgInfoEnable;
    FlashCond_T         m_FlashCond;
};

#endif

