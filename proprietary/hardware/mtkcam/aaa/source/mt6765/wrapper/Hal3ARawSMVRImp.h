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
#ifndef _HAL_3A_RAW_SMVRIMP_H_
#define _HAL_3A_RAW_SMVRIMP_H_

#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <string.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Log.h>
#include <IThreadRaw.h>
#include <IThreadRawSMVR.h>
#include <mtkcam/drv/IHalSensor.h>

#include "Hal3ARawSMVR.h"
#include <ae_mgr/ae_mgr.h>
#include <awb_mgr/awb_mgr_if.h>
#include <sensor_mgr/aaa_sensor_mgr.h>
#include <ISync3A.h>
#include <isp_tuning/isp_tuning_mgr.h>
#include <isp_mgr/isp_mgr.h>
#include <state_mgr/aaa_state_mgr.h>
#include <aaa_common_custom.h>

#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <dbg_aaa_param.h>
#include <aaa_hal_sttCtrl.h>
#include <debug/DebugUtil.h>
#if CAM3_AF_FEATURE_EN
#include <state_mgr_af/af_state_mgr.h>
#include <dbg_af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr/af_mgr_if.h>
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

#include "private/PDTblGen.h"

#include <android/sensor.h>                     // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener

//Thread Use
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

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

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuningv3;

class Hal3ARawSMVRImp : public Hal3ARawImp, public Hal3ARawSMVR
{
public:
    static I3AWrapper*  getInstance(MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount);
    virtual MVOID       destroyInstance();
    virtual MBOOL       start();
    virtual MBOOL       stop();
    virtual MVOID       pause(){}
    virtual MVOID       resume(MINT32){}
    //virtual MVOID       setSensorMode(MINT32 i4SensorMode);
    virtual MBOOL       generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2);
    virtual MBOOL       validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario);
    //virtual MBOOL       setAfParams(AF_Param_T const &rNewParam);
    //virtual MBOOL       autoFocus();
    //virtual MBOOL       cancelAutoFocus();
    //virtual MVOID       setFDEnable(MBOOL fgEnable);
    //virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    //virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT);
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/);
    //virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    //virtual MBOOL       isNeedTurnOnPreFlash() const;
    //virtual MBOOL       chkMainFlashOnCond() const;
    virtual MBOOL       chkPreFlashOnCond() const;
    //virtual MBOOL       isStrobeBVTrigger() const;
    //virtual MBOOL       isFlashOnCapture() const;
    virtual MBOOL       chkCapFlash() const {return m_bIsHighQualityCaptureOn;}
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, Result_T& rResult, MINT32 i4SubsampleIndex = 0) const;
    //virtual MINT32      getCurrentHwId() const;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    virtual MUINT32     queryMagicNumber() const {return m_u4MagicNumber;}
    //virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    //virtual MINT32      attachCb(I3ACallBack* cb);
    //virtual MINT32      detachCb(I3ACallBack* cb);
    //virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    //virtual MBOOL       notifyPwrOn();
    //virtual MBOOL       notifyPwrOff();
    //virtual MBOOL       notifyP1PwrOn(); //Open CCU power.
    //virtual MBOOL       notifyP1PwrOff(); //Close CCU power.
    virtual MVOID       querySensorStaticInfo();
    //virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum, Result_T& Result);
    virtual MBOOL       dumpP1Params(MUINT32 /*u4MagicNum*/){return MTRUE;}
    //virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    //virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       notifyResult4TG(MINT32 /*i4PreFrmId*/, Result_T* /*rPreResult*/) {return MTRUE;}
    virtual MBOOL       notify4CCU(MUINT32 /*u4PreFrmId*/, Result_T* /*rPreResult*/){return MTRUE;}
    //virtual MVOID       notifyPreStop();

protected:  //    Ctor/Dtor.
                        Hal3ARawSMVRImp(MINT32 const i4SensorDevId);
    virtual             ~Hal3ARawSMVRImp(){}

                        Hal3ARawSMVRImp(const Hal3ARawSMVRImp&);
                        Hal3ARawSMVRImp& operator=(const Hal3ARawSMVRImp&);

    MBOOL               init(MINT32 i4SensorOpenIndex, MINT32 i4SubsampleCount);
    MBOOL               uninit();
    //MRESULT             updateTGInfo();
    MINT32              config(const ConfigInfo_T& rConfigInfo);
    //MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    //MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;

    //MINT32              m_3ALogEnable;
    //MINT32              m_DebugLogWEn;
    //volatile int        m_Users;
    mutable Mutex       m_Lock;
    Mutex               mP2Mtx;
    Mutex               m3AOperMtx1;
    Mutex               m3AOperMtx2;
    //MINT32              m_i4SensorIdx;
    //MINT32              m_i4SensorDev;
    //MUINT32             m_u4SensorMode;
    //MUINT               m_u4TgInfo;
    //MBOOL               m_bEnable3ASetParams;
    //MBOOL               m_bFaceDetectEnable;
    MINT32              m_i4InCaptureProcess;
    //MINT32              m_i4TgWidth;
    //MINT32              m_i4TgHeight;
    //MBOOL               mbIsHDRShot;
    //MINT32              m_i4HbinWidth;
    //MINT32              m_i4HbinHeight;
    //MBOOL               m_fgEnableShadingMeta;
    MINT32              m_i4SubsampleCount;
    Result_T            m_rResultLastSof;
    //MBOOL               m_bIsRecordingFlash;
    //MINT32              m_i4HWSuppportFlash;
    //MINT32              m_i4SensorPreviewDelay;

    MINT32              m_i4AETargetMode = AE_MODE_NORMAL;
    //MBOOL               m_bPreStop;

    //NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    //IspTuningMgr*       m_pTuning;
    //IThreadRaw*         m_pThreadRaw;
    //StateMgr*           m_pStateMgr;
    //AfStateMgr*         m_pAfStateMgr;
    //NSCcuIf::ICcuMgrExt* m_pICcuMgr;
    //Param_T             m_rParam;
    //AF_Param_T          m_rAfParam;
    //ScenarioParam       m_sParam;
    //I3ACallBack*        m_pCbSet;
    MUINT32             m_u4MagicNumber;
    MBOOL               m_bIsHighQualityCaptureOn;
    SMVR_GAIN_DELAY_T   m_rGainDelay;
};

#endif

