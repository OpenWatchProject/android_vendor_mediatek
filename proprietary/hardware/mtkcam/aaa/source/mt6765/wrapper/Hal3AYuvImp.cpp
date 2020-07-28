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
#define LOG_TAG "Hal3AYuv"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/def/BuiltinTypes.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <math.h>
#include <string.h>
#include <mtkcam/utils/std/Trace.h>
#include <pthread.h>
#include <semaphore.h>

#include <aaa_log.h>
#include <aaa_error_code.h>
#include <aaa_common_custom.h>

#include <mtkcam/drv/IHalSensor.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include "Hal3AYuv.h"

#include <kd_imgsensor_define.h>
#include <camera_custom_if_yuv.h>
//#include <camera_custom_flashlight.h>

#include <strobe_drv.h>
#include <mtkcam/aaa/drv/mcu_drv.h>
//#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <debug_exif/aaa/dbg_isp_param.h>
#include <faces.h>


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

#define setYuvFeature(Cmd, Param)\
{\
    int cmd = Cmd;\
    int param = (Param);\
    MY_LOG("[%s] "#Cmd" i4Param(%d)", __FUNCTION__, param);\
    m_pIHalSensor->sendCommand(\
        m_i4SensorDev, SENSOR_CMD_SET_YUV_FEATURE_CMD, (MUINTPTR)&cmd, (MUINTPTR)&param, 0);\
}\

#define MIN_ISO (50)

#define YUV_IMG_WD                  (320)
#define YUV_IMG_HT                  (240)
#define ANDR_IMG_WD                 (2000)
#define ANDR_IMG_HT                 (2000)

typedef enum
{
    E_YUV_STATE_IDLE          = 0,
    E_YUV_STATE_PREVIEW       = 1,
    E_YUV_STATE_CAPTURE       = 2,
    E_YUV_STATE_RECORDING     = 3,
    E_YUV_STATE_PRECAPTURE    = 4
} E_YUV_STATE;

typedef enum
{
    E_YUV_SAF_DONE = 0,
    E_YUV_SAF_FOCUSING = 1,
    E_YUV_SAF_INCAF = 2
} E_YUV_SAF;

typedef enum
{
    E_YUV_WIN_STATE_NO_ACTIVITY  = 0,
    E_YUV_WIN_STATE_RESET_IN_FD  = 1,
    E_YUV_WIN_STATE_RESET_IN_CAF = 2
} E_YUV_WIN_STATE;

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuningv3;

class Hal3AYuvImp : public Hal3AYuv
{
public:
    static I3AWrapper*  getInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID       destroyInstance();
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo);
    virtual MBOOL       start();
    virtual MBOOL       stop();
    virtual MVOID       pause(){};
    virtual MVOID       resume(MINT32){};
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
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum, Result_T& Result);
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond();
    virtual MBOOL       chkPreFlashOnCond();
    virtual MBOOL       isStrobeBVTrigger() const;
    virtual MBOOL       chkCapFlash() const;
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, Result_T& rResult, MINT32 i4SubsampleCount = 0) const;
    virtual MINT32      getCurrentHwId() const;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    virtual MUINT32     queryMagicNumber() const;// {return m_u4MagicNumber;}
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    virtual MBOOL       notifyPwrOn();
    virtual MBOOL       notifyPwrOff();
    virtual MBOOL       notifyP1PwrOn();
    virtual MBOOL       notifyP1PwrOff();
    virtual MBOOL       dumpP1Params(MUINT32 u4MagicNum);//{return MTRUE;}
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);//{return MTRUE;}
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);//{return MTRUE;}
    virtual MBOOL       notifyResult4TG(MINT32 i4PreFrmId, Result_T *rPreResult);//{return MTRUE;}
    virtual MBOOL       notify4CCU(MUINT32 u4PreFrmId, Result_T *rPreResult);//{return MTRUE;}
    virtual MVOID       querySensorStaticInfo();
    virtual MVOID       notifyPreStop();

private:
    void calcASDEv(const SENSOR_AE_AWB_CUR_STRUCT& cur);
    MINT32 ASDLog2Func(MUINT32 numerator, MUINT32 denominator) const;
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;

    /**
     * @brief Reset ready-to-capture flag to 0.
     */
    inline MVOID resetReadyToCapture()
    {
        m_bReadyToCapture = MFALSE;
    }

    /**
     * @brief Update capture parameters for flashlight on.
     */
    MINT32 updateAeFlashCaptureParams();

    /**
     * @brief Set auto exposure lock.
     * @param [in] bLock 1 for lock, 0 for unlock
     */
    MBOOL setAeLock(MBOOL bLock);

    /**
     * @brief Set auto white balance lock.
     * @param [in] bLock 1 for lock, 0 for unlock
     */
    MBOOL setAwbLock(MBOOL bLock);

    /**
     * @brief Set focus area.
     * @param [in] a_i4Cnt number of area; a_psFocusArea array of areas.
     */
    MVOID  setFocusAreas(MINT32 a_i4Cnt, CameraArea_T *a_psFocusArea);

    /**
     * @brief Get focus area.
     * @param [out] a_i4Cnt number of area; a_psFocusArea array of areas.
     */
    MVOID  getFocusAreas(MINT32 &a_i4Cnt, CameraArea_T **a_psFocusArea);

    /**
     * @brief Get AF status.
     * @return
     * - SENSOR_AF_FOCUSED indicates focused and lens stops moving.
     * - SENSOR_AF_FOCUSING indicates lens is moving for focusing.
     * - SENSOR_AF_IDLE indicates lens idle and no action.
     * - SENSOR_AF_ERROR indicates error happened.
     */
    MINT32 isFocused();

    /**
     * @brief Get 3A status.
     * @return
     */
    MINT32 get3AStatusFromSensor();

    /**
     * @brief Query exposure info from sensor.
     */
    MINT32 queryAEFlashlightInfoFromSensor(SENSOR_FLASHLIGHT_AE_INFO_STRUCT& rAeFlashlightInfo);

    /**
     * @brief Log2 operation
     * @param [in] x
     * @return
     * - Log2(x)
     */
    MDOUBLE AEFlashlightLog2(MDOUBLE x);

    /**
     * @brief Set exposure time and sensor gain.
     * @param [in] a_u4ExpTime exposure time in microsecond
     * @param [in] a_u4SensorGain sensor gain in GAIN_BASE
     */
    MINT32 setEShutterParam(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);

    /**
     * @brief Convert flashlight exposure parameters
     */
    void convertFlashExpPara(MINT32 flashEngRatio_x10, MINT32 minAfeGain_x1024,
                                 MINT32 bv0_x1024, MINT32 bv1_x1024,
                                 MINT32  exp1, MINT32  afeGain1_x1024, MINT32  ispGain1_x1024,
                                 MINT32& exp2, MINT32& afeGain2_x1024, MINT32& ispGain2_x1024) const;
    /**
     * @brief Calculate brightness value.
     * @return
     * - BV
     */
    MDOUBLE calcBV(const SENSOR_FLASHLIGHT_AE_INFO_STRUCT& rAeFlashlightInfo);

    /**
     * @brief Utility for clamp x at range from min to max.
     * @param [in] x, min, max
     * @return
     * - clamped value
     */
    MINT32 clamp(MINT32 x, MINT32 min, MINT32 max);

    /**
     * @brief Get AE metering area.
     * @param [out] a_i4Cnt number of area; a_psAEArea array of areas.
     */
    MVOID  getMeteringAreas(MINT32 &a_i4Cnt, CameraArea_T **a_psAEArea);

    /**
     * @brief Set AE metering area.
     * @param [in] a_i4Cnt number of area; a_psAEArea array of areas.
     */
    MVOID  setMeteringAreas(MINT32 a_i4Cnt, CameraArea_T const *a_psAEArea);

    /**
     * @brief Map area to zone.
     * @param [in] p_area pointer to area.
     * @param [in] areaW, areaH dimension of area coordinate.
     * @param [in] zoneW, zoneH dimension of zone coordinate.
     * @param [out] p_zone pointer to zone.
     */
    MVOID  mapAeraToZone(CameraArea_T *p_area, MINT32 areaW,
                         MINT32 areaH, MINT32* p_zone,
                         MINT32 zoneW, MINT32 zoneH);

    /**
     * @brief Reset AF and AE window to center.
     */
    MBOOL  resetAFAEWindow();

    /**
     * @brief Map EV value to enum type AE_EVCOMP_T.
     * @param [in] mval, mstep
     * @return
     * - AE_EVCOMP_T refer to kd_camera_feature_enum.h in detail.
     */
    MINT32 mapAEToEnum(MINT32 mval,MFLOAT mstep);

    /**
     * @brief Map ISO value to enum type AE_ISO_T.
     * @param [in] u4NewAEISOSpeed
     * @return
     * - AE_ISO_T refer to kd_camera_feature_enum.h in detail.
     */
    MINT32 mapISOToEnum(MUINT32 u4NewAEISOSpeed);

    /**
     * @brief Map enum type AE_ISO_T to ISO value.
     * @param [in] u4NewAEIsoEnum.
     * @return
     * - ISO value
     */
    MINT32 mapEnumToISO(MUINT32 u4NewAEIsoEnum) const;

    MINT32 setAFMode(MINT32 AFMode);
    MRESULT onPreCaptureStart();
    MRESULT onPreCaptureEnd();
    MRESULT updatePreCapture();
    MRESULT onCaptureStart();
    //MRESULT onCaptureEnd();


protected:  //    Ctor/Dtor.
                        Hal3AYuvImp(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex);
    virtual             ~Hal3AYuvImp(){}

                        Hal3AYuvImp(const Hal3AYuvImp&);
                        Hal3AYuvImp& operator=(const Hal3AYuvImp&);

    MBOOL               init();
    MBOOL               uninit();
    MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;


private:
    MINT32              m_3ALogEnable;
    volatile int        m_Users;
    mutable Mutex       m_Lock;
    MINT32              m_i4SensorIdx;
    MINT32              m_i4SensorDev;
    MUINT32             m_u4SensorMode;
    MINT32              m_i4SensorPreviewDelay;
    MINT32              m_i4AeShutDelayFrame;
    MINT32              m_i4AeISPGainDelayFrame;
    MINT32              m_i4CaptureDelayFrame;

private:
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IHalSensor*         m_pIHalSensor;
    //IspTuningMgr*       m_pTuning;
    Param_T             m_rParam;
    AF_Param_T          m_rAfParam;
    I3ACallBack*        m_pCbSet;
    MUINT32             m_u4MagicNumber;
    MBOOL               m_bForceUpdatParam;
    MBOOL               m_bIsRecordingFlash;
    MBOOL               m_bIsCapEnd;
    MINT32              m_i4State;
    MINT32              m_i4HWSuppportFlash;
    MINT32              m_i4HbinWidth;
    MINT32              m_i4HbinHeight;
    MINT32              m_i4TgWidth;
    MINT32              m_i4TgHeight;
    Mutex               m_ValidateLock;
    Mutex               m_LockAF;
    MINT32              m_TgInfo;

    //for ASD
    MINT32        m_i4AELv_x10;
    MBOOL         m_bAEStable;
    SENSOR_AE_AWB_REF_STRUCT m_AsdRef;

    //AF related
    MINT32        m_fgIsDummyLens;
    MINT32        m_imageXS;
    MINT32        m_imageYS;
    MINT32        m_max_metering_areas;
    MINT32        m_max_af_areas;
    MINT32        m_i4PreAfStatus;
    MINT32        m_i4PreAfMode;
    MINT32        m_i4AFSwitchCtrl;
    MINT32        m_i4AutoFocus;
    MINT32        m_i4AutoFocusTimeout;
    MINT32        m_i4FDFrmCnt;
    MINT32        m_i4FDApplyCnt;
    MINT32        m_i4WinState;
    MINT32        m_AFzone[6];
    MINT32        m_AEzone[6];
    CameraArea_T  m_sAFAREA[MAX_FOCUS_AREAS];      //
    CameraArea_T  m_sAEAREA[MAX_METERING_AREAS];   //

    MINT32        m_i4AfStatus;
    MINT32        m_i4AeStatus;
    MINT32        m_i4AwbStatus;
    MINT32        m_i4InitReadyCnt;
    MBOOL         m_fgAfTrig;
    MBOOL         m_fgAfTrigMode;
    MBOOL         m_fgAeLockSupp;
    MBOOL         m_fgAwbLockSupp;
    MBOOL         m_bAeLimiter;
    MBOOL         m_fgFDEnable;
    MBOOL         m_bReadyToCapture;

    //Strobe related
    MBOOL         m_bFlashActive;
    MBOOL         m_bExifFlashOn;
    MBOOL         m_isFlashOnCapture;
    MINT32        m_i4LEDPartId;
    StrobeDrv*    m_pStrobeDrvObj;
    MINT32        m_aeFlashlightType;
    MINT32        m_i4PreFlashShutter;
    MINT32        m_i4PreFlashGain;
    MUINT32       m_u4PreFlashFrmCnt;
    MUINT32       m_u4StrobeDuty;
    MDOUBLE       m_fBVThreshold;
    MDOUBLE       m_fNoFlashBV;
    MDOUBLE       m_fPreFlashBV;
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT m_rAeInfo;
    NSCamCustom::YUV_FL_PARAM_T m_rYuvFlParam;
};

template <MINT32 sensorDevId>
class Hal3AYuvImpDev : public Hal3AYuvImp
{
public:
    static Hal3AYuvImpDev* getInstance(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex)
    {
        static Hal3AYuvImpDev<sensorDevId> singleton(i4SensorDevId, i4SensorOpenIndex);
        return &singleton;
    }
    Hal3AYuvImpDev(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex)
        : Hal3AYuvImp(i4SensorDevId, i4SensorOpenIndex)
    {
    }

private:

};

I3AWrapper*
Hal3AYuv::
getInstance(MINT32 const i4SensorOpenIndex)
{
    return Hal3AYuvImp::getInstance(i4SensorOpenIndex);
}

I3AWrapper*
Hal3AYuvImp::
getInstance(MINT32 const i4SensorOpenIndex)
{
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    MUINT32 i4SensorDevId = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            Hal3AYuvImpDev<SENSOR_DEV_MAIN>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3AYuvImpDev<SENSOR_DEV_MAIN>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        case SENSOR_DEV_SUB:
            Hal3AYuvImpDev<SENSOR_DEV_SUB>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3AYuvImpDev<SENSOR_DEV_SUB>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        case SENSOR_DEV_MAIN_2:
            Hal3AYuvImpDev<SENSOR_DEV_MAIN_2>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3AYuvImpDev<SENSOR_DEV_MAIN_2>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        case SENSOR_DEV_SUB_2:
            Hal3AYuvImpDev<SENSOR_DEV_SUB_2>::getInstance(i4SensorDevId, i4SensorOpenIndex)->init();
            return Hal3AYuvImpDev<SENSOR_DEV_SUB_2>::getInstance(i4SensorDevId, i4SensorOpenIndex);
        break;
        default:
            MY_ERR("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }
}

MVOID
Hal3AYuvImp::
destroyInstance()
{
    MY_LOG("[%s]", __FUNCTION__);
    uninit();
}

Hal3AYuvImp::
Hal3AYuvImp(MUINT32 const i4SensorDevId, MINT32 const i4SensorOpenIndex)
    : m_i4SensorIdx(i4SensorOpenIndex)
    , m_i4SensorDev(i4SensorDevId)
    , m_u4SensorMode(0)
    , m_i4SensorPreviewDelay(0)
    , m_i4AeShutDelayFrame(0)
    , m_i4AeISPGainDelayFrame(0)
    , m_i4CaptureDelayFrame(0)
    , m_pCamIO(NULL)
    , m_pIHalSensor(NULL)
    //, m_pTuning(NULL)
    , m_rParam()
    , m_rAfParam()
    , m_pCbSet(NULL)
    , m_u4MagicNumber(0)
    , m_bForceUpdatParam(MFALSE)
    , m_bIsRecordingFlash(MFALSE)
    , m_bIsCapEnd(MFALSE)
    , m_i4State(E_YUV_STATE_IDLE)
    , m_i4HWSuppportFlash(0)
    , m_i4HbinWidth(1000)
    , m_i4HbinHeight(1000)
    , m_i4TgWidth(1000)
    , m_i4TgHeight(1000)
    , m_TgInfo(CAM_TG_ERR)
    , m_fgAfTrig(MFALSE)
    , m_fgAfTrigMode(MFALSE)
    , m_fgAeLockSupp(0)
    , m_fgAwbLockSupp(0)
    , m_bReadyToCapture(MFALSE)
    , m_i4LEDPartId(1)
    , m_pStrobeDrvObj(NULL)
{
    MY_LOG("[%s] sensorDev(%d) sensorIdx(%d)", __FUNCTION__, i4SensorDevId, i4SensorOpenIndex);
}

MBOOL
Hal3AYuvImp::
init()
{
    m_3ALogEnable = property_get_int32("vendor.debug.camera.log", 0);
    if ( m_3ALogEnable == 0 ) {
        m_3ALogEnable = property_get_int32("vendor.debug.camera.log.hal3a", 0);
    }

    MY_LOG("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // check user count
    MINT32 i4CurrLensId = 0;
    MINT32 i4CurrSensorId = 0;
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("[%s] %d has created \n", __FUNCTION__, m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    //****************** IHalSensor Init ******************//
    CAM_TRACE_BEGIN("IHalSensor Init");
    if (m_pIHalSensor == NULL)
    {
        //IHalSensorList* const pIHalSensorList = IHalSensorList::get();
        IHalSensorList*const pIHalSensorList = MAKE_HalSensorList();
        m_pIHalSensor = pIHalSensorList->createSensor(LOG_TAG, m_i4SensorIdx);

        if (!m_pIHalSensor)
        {
            MY_ERR("m_pIHalSensor create fail");
            return E_3A_ERR;
        }
        else
        {
            // get current Sensor id
            SensorStaticInfo rSensorStaticInfo;
            pIHalSensorList->querySensorStaticInfo(m_i4SensorDev, &rSensorStaticInfo);
            i4CurrSensorId = rSensorStaticInfo.sensorDevID;
            MY_LOG("[%s] m_pIHalSensor(0x%p) create OK. ID(0x%d)", __FUNCTION__, m_pIHalSensor, i4CurrSensorId);

            /*// get TG info
            SensorDynamicInfo senInfo;
            MINT32 i4SensorDevId = m_pIHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);
            MY_LOG("m_i4SensorDev(%d), TgInfo(%d)\n", m_i4SensorDev, senInfo.TgInfo);

            if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
            {
                MY_ERR("YUV sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
                return E_3A_ERR;
            }

            m_TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2*/
        }
    }
    CAM_TRACE_END();

    //****************** Lens init ******************//
    /*CAM_TRACE_BEGIN("Lens Init");
    MCUDrv::lensSearch(m_i4SensorDev, i4CurrSensorId);
    i4CurrLensId = MCUDrv::getCurrLensID(m_i4SensorDev);
    m_fgIsDummyLens = (i4CurrLensId == SENSOR_DRIVE_LENS_ID) ? MFALSE : MTRUE;
    MY_LOG("[%s] i4CurrLensId(0x%08x), m_fgIsDummyLens(%d)\n", __FUNCTION__, i4CurrLensId, m_fgIsDummyLens);
    CAM_TRACE_END();*/

    //****************** Strobe init ******************//
    CAM_TRACE_BEGIN("Strobe init");
    m_pStrobeDrvObj = StrobeDrv::getInstance(m_i4SensorDev);
    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->getPartId(&m_i4LEDPartId);
        m_pStrobeDrvObj->init();
        m_aeFlashlightType = m_pStrobeDrvObj->getFlashlightType();
        if (m_aeFlashlightType == StrobeDrv::FLASHLIGHT_NONE)
        {
            m_pStrobeDrvObj->uninit();
            m_pStrobeDrvObj = NULL;
        }
        MY_LOG("strobe type(%d)", m_aeFlashlightType);
    }
    NSCamCustom::custom_GetYuvFLParam(m_i4LEDPartId, m_rYuvFlParam);
    MY_LOG("[%s] m_pStrobeDrvObj(0x%p), m_aeFlashlightType(%d)\n", __FUNCTION__, m_pStrobeDrvObj, m_aeFlashlightType);

    if (m_pStrobeDrvObj)
    {
        MY_LOG("[%s] ParId(%d) Th(%3.3f) Duty(%d) Step(%d) Frm(%d) PreAF(%d) Gain(%d) HighDuty(%d) TO(%d) AfLamp(%d)", __FUNCTION__,
            m_i4LEDPartId,
            m_rYuvFlParam.dFlashlightThreshold,
            m_rYuvFlParam.i4FlashlightDuty,
            m_rYuvFlParam.i4FlashlightStep,
            m_rYuvFlParam.i4FlashlightFrameCnt,
            m_rYuvFlParam.i4FlashlightPreflashAF,
            m_rYuvFlParam.i4FlashlightGain10X,
            m_rYuvFlParam.i4FlashlightHighCurrentDuty,
            m_rYuvFlParam.i4FlashlightHighCurrentTimeout,
            m_rYuvFlParam.i4FlashlightAfLampSupport
        );
        if (m_pStrobeDrvObj->setStep(m_rYuvFlParam.i4FlashlightStep) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setStep(%d)", __FUNCTION__, m_rYuvFlParam.i4FlashlightStep);
        }
    }
    CAM_TRACE_END();

    //****************** Member init ******************//
    m_fBVThreshold      = m_rYuvFlParam.dFlashlightThreshold;
    m_u4PreFlashFrmCnt  = m_rYuvFlParam.i4FlashlightFrameCnt;
    m_u4StrobeDuty      = m_rYuvFlParam.i4FlashlightDuty;
    m_i4AutoFocus = MFALSE;
    m_i4AutoFocusTimeout = 0;
    m_bAeLimiter = 0;
    m_i4FDFrmCnt = 0;
    m_i4FDApplyCnt = 0;
    m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
    m_fgAfTrig = MFALSE;
    m_bForceUpdatParam = MFALSE;
    m_i4State = E_YUV_STATE_IDLE;

    //****************** ASD init ******************//
    CAM_TRACE_BEGIN("ASD init");
    SENSOR_AE_AWB_REF_STRUCT ref;
    memset(&ref, 0, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN, (MUINTPTR)&ref, 0, 0);
    memcpy(&m_AsdRef, &ref, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    m_AsdRef.SensorLV05LV13EVRef =
        ASDLog2Func(ref.SensorAERef.AeRefLV05Shutter * ref.SensorAERef.AeRefLV05Gain,
                    ref.SensorAERef.AeRefLV13Shutter * ref.SensorAERef.AeRefLV13Gain);
    MY_LOG("[%s] ASD AE Ref: Lv05S(%d) Lv05G(%d) Lv13S(%d) Lv13G(%d) EVRef(%d)\n", __FUNCTION__,
        ref.SensorAERef.AeRefLV05Shutter, ref.SensorAERef.AeRefLV05Gain,
        ref.SensorAERef.AeRefLV13Shutter, ref.SensorAERef.AeRefLV13Gain,
        m_AsdRef.SensorLV05LV13EVRef);
    CAM_TRACE_END();
#if 0
    //****************** TuningMgr init ******************//
    if (m_pTuning == NULL)
    {
        CAM_TRACE_BEGIN("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx))
        {
            MY_ERR("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_3A_HAL("Fail to init IspTuningMgr");
            CAM_TRACE_END();
            return MFALSE;
        }
        CAM_TRACE_END();
    }
#endif
    //****************** Query sensor info ******************//
    CAM_TRACE_BEGIN("querySensorStaticInfo");
    querySensorStaticInfo();
    CAM_TRACE_END();

    MY_LOG("[%s] done\n", __FUNCTION__);
    android_atomic_inc(&m_Users);
    return S_3A_OK;
}

MBOOL
Hal3AYuvImp::
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
    MY_LOG("[%s] m_Users: %d \n", __FUNCTION__, m_Users);

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        //****************** Callback uninit ******************//
        if (m_pCbSet)
        {
            m_pCbSet = NULL;
            MY_ERR("User did not detach callbacks!");
        }
#if 0
        //****************** TuningMgr uninit ******************//
        if (m_pTuning)
        {
            m_pTuning->uninit(m_i4SensorDev);
            m_pTuning = NULL;
        }
#endif
        MY_LOG("[%s] done\n", __FUNCTION__);

    }
    else    // There are still some users.
    {
        MY_LOG_IF(m_3ALogEnable, "[%s] Still %d users \n", __FUNCTION__, m_Users);
    }

    return S_3A_OK;
}

MINT32
Hal3AYuvImp::
config(const ConfigInfo_T& /*rConfigInfo*/)
{
    MY_LOG("[%s]+ ConfigInfo sensorDev(%d), Mode(%d)", __FUNCTION__, m_i4SensorDev, m_u4SensorMode);
    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            MY_ERR("Fail to create NormalPipe");
            return MFALSE;
        }
    }
    MY_LOG("[%s]-", __FUNCTION__);
    return S_3A_OK;
}

MBOOL
Hal3AYuvImp::
start()
{
    MY_LOG("[%s] +", __FUNCTION__);
    // Force reset Parameter
    /*Param_T old_para,rst_para;
    memset(&rst_para, 0, sizeof(Param_T));
    old_para = m_rParam;
    m_rParam = rst_para;*/
    m_bForceUpdatParam = MTRUE;
    setParams(m_rParam, MTRUE);
    m_bForceUpdatParam = MFALSE;

    m_bExifFlashOn = 0;

    // transit state to preview
    if (m_i4State == E_YUV_STATE_IDLE)
    {
        m_i4InitReadyCnt = getDelayFrame(EQueryType_Init);
        MY_LOG("[%s] Start to count down (%d)", __FUNCTION__, m_i4InitReadyCnt);
    }

    m_i4State = E_YUV_STATE_PREVIEW;
    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
stop()
{
    MY_LOG("[%s] +", __FUNCTION__);
    MRESULT err = S_3A_OK;
    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL)
    {
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }
    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
generateP2(MINT32 /*flowType*/, const NSIspTuning::ISP_INFO_T& /*rIspInfo*/, void* /*pTuningBuf*/, ResultP2_T* /*pResultP2*/)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
validateP1(const ParamIspProfile_T& /*rParamIspProfile*/, MBOOL /*fgPerframe*/)
{
    Mutex::Autolock lock(m_ValidateLock);
    MY_LOG_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
    //m_pTuning->setIspProfile(m_i4SensorDev, rParamIspProfile.eIspProfile);
    //m_pTuning->validatePerFrame(m_i4SensorDev, rParamIspProfile.rRequestSet, fgPerframe);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
setParams(Param_T const &rNewParam, MBOOL /*bUpdateScenario*/)
{
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0;
    MINT32 i4SceneModeUpdate = 1;
    MINT32 i4SceneModeChg = 0;

    MY_LOG_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);

    i4SceneModeChg = m_rParam.u4SceneMode != rNewParam.u4SceneMode || m_bForceUpdatParam;

    if (m_rParam.u4EffectMode != rNewParam.u4EffectMode || m_bForceUpdatParam)
    {
        MY_LOG("[FID_COLOR_EFFECT], (%d)->(%d) \n", m_rParam.u4EffectMode, rNewParam.u4EffectMode);
        setYuvFeature(FID_COLOR_EFFECT, rNewParam.u4EffectMode);
    }

    //****************** Scene mode ******************//
    if (i4SceneModeUpdate)
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE], (%d)->(%d) \n", m_rParam.u4SceneMode, rNewParam.u4SceneMode);
            setYuvFeature(FID_SCENE_MODE, rNewParam.u4SceneMode);
        }

        if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_EV], Idx:(%d)->(%d), Step:(%f)->(%f) \n", m_rParam.i4ExpIndex, rNewParam.i4ExpIndex, m_rParam.fExpCompStep, rNewParam.fExpCompStep);
            setYuvFeature(FID_AE_EV, mapAEToEnum(rNewParam.i4ExpIndex, rNewParam.fExpCompStep));
        }

        if (m_rParam.u4AwbMode != rNewParam.u4AwbMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AWB_MODE], (%d)->(%d) \n", m_rParam.u4AwbMode, rNewParam.u4AwbMode);
            setYuvFeature(FID_AWB_MODE, rNewParam.u4AwbMode);
        }

        if (m_rParam.i4BrightnessMode != rNewParam.i4BrightnessMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_BRIGHT], (%d)->(%d) \n", m_rParam.i4BrightnessMode, rNewParam.i4BrightnessMode);
            setYuvFeature(FID_ISP_BRIGHT, rNewParam.i4BrightnessMode);
        }
        if (m_rParam.i4HueMode != rNewParam.i4HueMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_HUE], (%d)->(%d) \n", m_rParam.i4HueMode, rNewParam.i4HueMode);
            setYuvFeature(FID_ISP_HUE, rNewParam.i4HueMode);
        }
        if (m_rParam.i4SaturationMode != rNewParam.i4SaturationMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_SAT], (%d)->(%d) \n", m_rParam.i4SaturationMode, rNewParam.i4SaturationMode);
            setYuvFeature(FID_ISP_SAT, rNewParam.i4SaturationMode);
        }
        if (m_rParam.i4ContrastMode != rNewParam.i4ContrastMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_CONTRAST], (%d)->(%d) \n", m_rParam.i4ContrastMode, rNewParam.i4ContrastMode);
            setYuvFeature(FID_ISP_CONTRAST, rNewParam.i4ContrastMode);
        }
        if (m_rParam.i4EdgeMode != rNewParam.i4EdgeMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_EDGE], (%d)->(%d) \n", m_rParam.i4EdgeMode, rNewParam.i4EdgeMode);
            setYuvFeature(FID_ISP_EDGE, rNewParam.i4EdgeMode);
        }
        if (m_rParam.i4IsoSpeedMode != rNewParam.i4IsoSpeedMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_ISO], (%d)->(%d) \n", m_rParam.i4IsoSpeedMode, rNewParam.i4IsoSpeedMode);
            setYuvFeature(FID_AE_ISO, mapISOToEnum(rNewParam.i4IsoSpeedMode));
        }
    }
/*
    else if (rNewParam.u4SceneMode == SCENE_MODE_HDR)
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE], (%d)->(SCENE_MODE_HDR) \n", m_rParam.u4SceneMode);
            // set scene mode off (backward compatible) first, then set scene mode hdr (for JB4.2)
            setYuvFeature(FID_SCENE_MODE, SCENE_MODE_OFF);
            setYuvFeature(FID_SCENE_MODE, SCENE_MODE_HDR);

        }

        if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_EV], Idx:(%d)->(%d), Step:(%f)->(%f) \n", m_rParam.i4ExpIndex, rNewParam.i4ExpIndex, m_rParam.fExpCompStep, rNewParam.fExpCompStep);
            setYuvFeature(FID_AE_EV, mapAEToEnum(rNewParam.i4ExpIndex,rNewParam.fExpCompStep));
        }

        if (m_rParam.u4AwbMode != rNewParam.u4AwbMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AWB_MODE], (%d)->(%d) \n", m_rParam.u4AwbMode, rNewParam.u4AwbMode);
            setYuvFeature(FID_AWB_MODE, rNewParam.u4AwbMode);
        }
    }
    else
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE], (%d)->(%d) \n", m_rParam.u4SceneMode, rNewParam.u4SceneMode);
            setYuvFeature(FID_SCENE_MODE, rNewParam.u4SceneMode);
        }
    }
*/
    m_rParam = rNewParam;
    MY_LOG_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
setAfParams(AF_Param_T const &rNewParam)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] +", __FUNCTION__);
    setAFMode(rNewParam.u4AfMode);
    m_rAfParam = rNewParam;
    MY_LOG_IF(m_3ALogEnable, "[%s] -", __FUNCTION__);
    return MTRUE;
}


MBOOL
Hal3AYuvImp::
autoFocus()
{
    MY_LOG("[%s()] +", __FUNCTION__);

    Mutex::Autolock lock(m_LockAF);
    MY_LOG("[%s()] dummylens(%d)", __FUNCTION__, m_fgIsDummyLens);

    if (m_fgIsDummyLens == 1 || m_max_af_areas == 0)
    {
        //add for cts
        m_i4AutoFocus = E_YUV_SAF_FOCUSING;
        MY_LOG("[%s] AF Not Support\n", __FUNCTION__);
        return MTRUE;
    }

    if ((m_rAfParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO) && (m_rAfParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE))
    {
        if (m_fgAfTrigMode)
        {
            m_fgAfTrig = MTRUE;
        }
        else
        {
            m_fgAfTrig = MFALSE;
            setFlashLightOnOff(MTRUE, MTRUE);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)m_AFzone,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,0,0,0);
            m_i4AutoFocus = E_YUV_SAF_FOCUSING;
            m_i4AutoFocusTimeout = 30;
        }
        MY_LOG("[%s] Do SAF CMD\n", __FUNCTION__);
    }
    else
    {
        m_i4AutoFocus = E_YUV_SAF_INCAF;
        MY_LOG("[%s] called in AF mode(%d)", __FUNCTION__, m_rAfParam.u4AfMode);
    }
    MY_LOG("[%s()] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
cancelAutoFocus()
{
    MY_LOG("[%s()] +", __FUNCTION__);
    if (m_fgIsDummyLens == 1){return MTRUE;}

    if (m_rAfParam.u4AfMode != AF_MODE_AFS)
    {
        m_rAfParam.u4AfMode = AF_MODE_INFINITY;
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
    }
    m_i4AutoFocus = E_YUV_SAF_DONE;

    MY_LOG("[%s()] -", __FUNCTION__);
    return MTRUE;
}

MVOID
Hal3AYuvImp::
setFDEnable(MBOOL fgEnable)
{
    Mutex::Autolock lock(m_LockAF);

    if (fgEnable)
    {
        MY_LOG("[%s] Start FD", __FUNCTION__);
        m_fgFDEnable = MTRUE;
        m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
    }
    else
    {
        MY_LOG("[%s] Stop FD", __FUNCTION__);
        m_fgFDEnable = MFALSE;
        //m_i4WinState = 2;
    }
}

MBOOL
Hal3AYuvImp::
setFDInfo(MVOID* prFaces, MVOID* /*prAFFaces*/)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] m_fgFDEnable(%d)", __FUNCTION__, m_fgFDEnable);

    MINT32 i4Cnt;
    CameraArea_T rFDArea;
    MtkCameraFaceMetadata *pFaces;

    Mutex::Autolock lock(m_LockAF);

    pFaces = (MtkCameraFaceMetadata *)prFaces;

    if (!m_fgFDEnable && m_i4WinState!=E_YUV_WIN_STATE_RESET_IN_CAF)
    {
        MY_LOG("[%s] FD is disabled, reset window", __FUNCTION__);
        m_i4WinState = E_YUV_WIN_STATE_RESET_IN_CAF;
        return MFALSE;
    }

    if (pFaces == NULL)
    {
        MY_LOG("[%s] Leave", __FUNCTION__);
        m_i4WinState = E_YUV_WIN_STATE_RESET_IN_CAF;
        return MTRUE;
    }

    i4Cnt = pFaces->number_of_faces;
    MY_LOG_IF(m_3ALogEnable, "[%s] Faces(%d)", __FUNCTION__, pFaces->number_of_faces);

    if (i4Cnt)
    {
        m_i4FDFrmCnt -= (m_i4FDFrmCnt > 0 ? 1 : 0);

        rFDArea.i4Left   = pFaces->faces->rect[0];
        rFDArea.i4Top    = pFaces->faces->rect[1];
        rFDArea.i4Right  = pFaces->faces->rect[2];
        rFDArea.i4Bottom = pFaces->faces->rect[3];

        if (rFDArea.i4Right == rFDArea.i4Left || rFDArea.i4Bottom == rFDArea.i4Top)
        {
            i4Cnt = 0;
        }
        MY_LOG_IF(m_3ALogEnable, "[%s] Faces(%d), LTRB(%d,%d,%d,%d)", __FUNCTION__,
            pFaces->number_of_faces, rFDArea.i4Left, rFDArea.i4Top, rFDArea.i4Right, rFDArea.i4Bottom);
    }
    else
    {
        m_i4FDFrmCnt += (m_i4FDFrmCnt < 8 ? 1 : 0);
    }

    MY_LOG_IF(m_3ALogEnable, "[%s] m_i4FDFrmCnt(%d)", __FUNCTION__, m_i4FDFrmCnt);

    if (m_i4FDFrmCnt < 3)
    {
        m_i4FDApplyCnt -= (m_i4FDApplyCnt > 0 ? 1 : 0);
        if (m_i4FDApplyCnt == 0)
        {
            m_i4WinState = E_YUV_WIN_STATE_RESET_IN_FD;
            m_i4FDApplyCnt = 3;
            if (i4Cnt != 0)
            {
                MY_LOG_IF(m_3ALogEnable, "[%s] number_of_faces(%d), LTRB(%d,%d,%d,%d)\n",
                __FUNCTION__, i4Cnt, rFDArea.i4Left, rFDArea.i4Top, rFDArea.i4Right, rFDArea.i4Bottom);

                if (m_max_af_areas > 0)
                {
                    setFocusAreas(i4Cnt, &rFDArea);
                    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)m_AFzone,0,0);
                }
                if (m_max_metering_areas > 0)
                {
                    setMeteringAreas(i4Cnt, &rFDArea);
                }
            }
        }
    }
    else
    {
        m_i4FDApplyCnt = 3;
        if (m_i4WinState == E_YUV_WIN_STATE_RESET_IN_FD)
        {
            m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
            resetAFAEWindow();
        }
    }
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
setOTInfo(MVOID* /*prOT*/, MVOID* /*prAFOT*/)
{
    MY_LOG_IF(m_3ALogEnable, "[%s]", __FUNCTION__);

    return MTRUE;
}

MVOID
Hal3AYuvImp::
setIspSensorInfo2AF(MINT32 , Result_T&)
{
    return ;
}


MBOOL
Hal3AYuvImp::
setFlashLightOnOff(MBOOL bOnOff, MBOOL /*bMainPre*/)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] bOnOff(%d) + ", __FUNCTION__, bOnOff);

    MINT32 i4Ret = S_3A_OK;
    MINT32 i4AfLampSupport = m_rYuvFlParam.i4FlashlightAfLampSupport;

    if (m_pStrobeDrvObj && i4AfLampSupport)
    {
        if (m_rParam.u4StrobeMode != FLASHLIGHT_TORCH)
        {
            MY_LOG("[%s] bOnOff(%d), StrobeMode(%d)\n", __FUNCTION__, bOnOff, m_rParam.u4StrobeMode);
            if (bOnOff)
            {
                MBOOL fgFlashOn = isStrobeBVTrigger();
                if (fgFlashOn)
                {
                    //ON flashlight
                    if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
                    {
                        MY_LOG("setTimeOutTime: 0\n");
                    }
                    if (m_pStrobeDrvObj->setDuty(m_u4StrobeDuty) == MHAL_NO_ERROR)
                    {
                        MY_LOG("setLevel:%d\n", m_u4StrobeDuty);
                    }
                    if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
                    {
                        MY_LOG("[%s] setFire ON\n", __FUNCTION__);
                    }
                }
                else
                {
                    MY_LOG("[%s] No need to turn on AF lamp.\n", __FUNCTION__);
                }
            }
            else
            {
                if (m_pStrobeDrvObj->setOnOff(0) == MHAL_NO_ERROR)
                {
                    MY_LOG("[%s] setFire OFF\n", __FUNCTION__);
                }
            }
        }
        else
        {
            MY_LOG("[%s] StrobeMode(%d), CamMode(%d), skip\n", __FUNCTION__, (MINT32)m_rParam.u4StrobeMode, (MINT32)m_rParam.u4CamMode);
        }
    }
    else
    {
        MY_LOG("[%s] strobe object(0x%p), AfLampSupport(%d)\n", __FUNCTION__, m_pStrobeDrvObj, i4AfLampSupport);
        i4Ret = E_3A_NULL_OBJECT;
    }

    return i4Ret;

    MY_LOG_IF(m_3ALogEnable, "[%s] - ", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
setPreFlashOnOff(MBOOL /*bOnOff*/)
{
    MY_LOG("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
isNeedTurnOnPreFlash() const
{
    MY_LOG("[%s]", __FUNCTION__);
    return MFALSE;
}

MBOOL
Hal3AYuvImp::
chkMainFlashOnCond()
{
    MY_LOG_IF(m_3ALogEnable, "[%s]", __FUNCTION__);
    MBOOL bChkFlash;
    bChkFlash = (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_SINGLE))
                ||
                (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isStrobeBVTrigger()))
                && (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE));

    return bChkFlash && (m_i4HWSuppportFlash != 0);//shouldn't return MTRUE if Hw does not support flash
}

MBOOL
Hal3AYuvImp::
chkPreFlashOnCond()
{
    MY_LOG_IF(m_3ALogEnable, "[%s] IsRecordingFlash(%d)", __FUNCTION__, m_bIsRecordingFlash);
    MBOOL bChkFlash;
    bChkFlash = (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH))
                || m_bIsRecordingFlash;

    return bChkFlash && (m_i4HWSuppportFlash != 0);//shouldn't return MTRUE if Hw does not support flash
}

MBOOL
Hal3AYuvImp::
isStrobeBVTrigger() const
{
    MINT32 rtn = 0;
    MUINT32 u4TrigFlashOn;

    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO, (MUINTPTR)&u4TrigFlashOn,0,0);

    MY_LOG("[%s] m_aeFlashlightType(%d), StrobeMode(%d), u4TrigFlashOn(%d)", __FUNCTION__,
        m_aeFlashlightType,
        m_rParam.u4StrobeMode,
        u4TrigFlashOn);

    if (FLASHLIGHT_LED_PEAK == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType ||
        FLASHLIGHT_LED_CONSTANT == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType)
    {
        if ( (LIB3A_FLASH_MODE_T)LIB3A_FLASH_MODE_FORCE_ON == m_rParam.u4StrobeMode )
        {
            rtn = 1;
        }
        else if ((LIB3A_FLASH_MODE_T)LIB3A_FLASH_MODE_AUTO == m_rParam.u4StrobeMode && u4TrigFlashOn
                 /*(m_fBVThreshold > m_strobeTrigerBV ) */)
        {
            rtn = 1;
        }
    }
    return rtn;
}

MBOOL
Hal3AYuvImp::
chkCapFlash() const
{
    return MFALSE;
}

MINT32
Hal3AYuvImp::
getCurrResult(MUINT32 i4FrmId, Result_T& rResult, MINT32 /*i4SubsampleIndex*/) const
{
    MY_LOG_IF(m_3ALogEnable, "[%s] + i4MagicNum(%d)", __FUNCTION__, i4FrmId);
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

    rResult.u1SceneMode = m_rParam.u4SceneMode;

    // AE
    if(m_i4State == E_YUV_STATE_PRECAPTURE)
    {
        if(m_bReadyToCapture)
            rResult.u1AeState = MTK_CONTROL_AE_STATE_CONVERGED;
        else
            rResult.u1AeState = MTK_CONTROL_AE_STATE_SEARCHING;
    }
    else if (isStrobeBVTrigger())
        rResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else if(m_i4AeStatus == SENSOR_AE_LOCKED)
        rResult.u1AeState = MTK_CONTROL_AE_STATE_LOCKED;
    else
        rResult.u1AeState = MTK_CONTROL_AE_STATE_CONVERGED;

     rResult.i8SensorExposureTime = 0;

    // AWB
    switch(m_i4AwbStatus)
    {
        case SENSOR_AWB_BALANCING:
            rResult.u1AwbState = MTK_CONTROL_AWB_STATE_SEARCHING;
            break;
        case SENSOR_AWB_BALANCED:
            rResult.u1AwbState = MTK_CONTROL_AWB_STATE_CONVERGED;
            break;
        case SENSOR_AWB_LOCKED:
            rResult.u1AwbState = MTK_CONTROL_AWB_STATE_LOCKED;
            break;
        default:
            break;
    }

    // AF
    switch(m_i4AfStatus)
    {
        case SENSOR_AF_IDLE:
        case SENSOR_AF_FOCUSED:
            rResult.u1AfState = MTK_CONTROL_AF_STATE_FOCUSED_LOCKED;
            break;
        case SENSOR_AF_FOCUSING:
            rResult.u1AfState = MTK_CONTROL_AF_STATE_ACTIVE_SCAN;
            break;
    }
    MY_LOG("[%s] #(%d) AWB(%d) AE(%d) AF(%d)", __FUNCTION__, rResult.i4FrmId, rResult.u1AwbState, rResult.u1AeState, rResult.u1AfState);

    // Flash
    /*rResult.u1FlashState =
        (FlashMgr::getInstance().getFlashState(m_i4SensorDev) == MTK_FLASH_STATE_UNAVAILABLE) ?
        MTK_FLASH_STATE_UNAVAILABLE :
        (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev) ? MTK_FLASH_STATE_FIRED : MTK_FLASH_STATE_READY);*/

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
        getP1DbgInfo(rDbg3AInfo, rDbgShadTbl, rDbgIspInfo);
    }

    MY_LOG_IF(m_3ALogEnable, "[%s] - i4MagicNum(%d)", __FUNCTION__, i4FrmId);
    return 0;
}

MINT32
Hal3AYuvImp::
getCurrentHwId() const
{
    MINT32 idx = 0;
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&idx, 0, 0);
    MY_LOG_IF(m_3ALogEnable, "[%s] idx(%d)", __FUNCTION__, idx);
    return idx;
}

MVOID
Hal3AYuvImp::
setSensorMode(MINT32 i4SensorMode)
{
    MY_LOG("[%s] mode(%d)", __FUNCTION__, i4SensorMode);
    if(m_u4SensorMode != (MUINT32)i4SensorMode)
        m_u4SensorMode = (MUINT32)i4SensorMode;
}

MBOOL
Hal3AYuvImp::
postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam)
{
    MY_LOG_IF(m_3ALogEnable, "[%s]+ cmd(%d)", __FUNCTION__, r3ACmd);
    MBOOL bRet = MTRUE;

    if (r3ACmd != ECmd_Update)
    {
        m_i4AFSwitchCtrl = -1;
        MY_LOG("[%s] eCmd(%d)\n", __FUNCTION__, r3ACmd);
    }

    switch (r3ACmd)
    {
    case ECmd_CameraPreviewStart:
        start();
        break;
    case ECmd_CameraPreviewEnd:
        stop();
        break;
    case ECmd_PrecaptureStart:
        onPreCaptureStart();
        break;
    case ECmd_PrecaptureEnd:
        onPreCaptureEnd();
        break;
    case ECmd_CaptureStart:
        onCaptureStart();
        break;
    case ECmd_Update:
        {
            MY_LOG_IF(m_3ALogEnable, "[%s] ECmd_Update (%d)", __FUNCTION__, pParam->i4MagicNum);
            get3AStatusFromSensor();

            switch (m_i4State)
            {
            case E_YUV_STATE_PRECAPTURE:
                updatePreCapture();
                break;
            case E_YUV_STATE_IDLE:
            case E_YUV_STATE_CAPTURE:
                break;
            default:
                break;
            }

            m_i4InitReadyCnt = m_i4InitReadyCnt > 0 ? m_i4InitReadyCnt - 1 : 0;

            validateP1(*pParam, MTRUE);
        }
        break;
    default:
        MY_ERR("[%s] Undefined command", __FUNCTION__);
        break;
    }

    return bRet;
    MY_LOG_IF(m_3ALogEnable, "[%s]-", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const
{
    SENSOR_EXIF_INFO_STRUCT rSensorInfo;
    //EXIF_INFO_T rEXIFInfo;
    //memset(&rEXIFInfo, 0, sizeof(EXIF_INFO_T));
    memset(&rSensorInfo, 0, sizeof(SENSOR_EXIF_INFO_STRUCT));

    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_EXIF_INFO,(MUINTPTR)&rSensorInfo,0,0);

    MY_LOG("[%s] FNumber(%d), AEISOSpeed(%d), AWBMode(%d), CapExposureTime(%d), FlashLightTimeus(%d), RealISOValue(%d)",
            __FUNCTION__,
            rSensorInfo.FNumber, m_rParam.i4IsoSpeedMode, m_rParam.u4AwbMode,
            rSensorInfo.CapExposureTime, m_bExifFlashOn, rSensorInfo.RealISOValue);

    rExifInfo.u4FNumber = rSensorInfo.FNumber>0 ? rSensorInfo.FNumber : 28;
    rExifInfo.u4FocalLength = 350;
    rExifInfo.u4SceneMode = m_rParam.u4SceneMode;
    rExifInfo.u4AWBMode = m_rParam.u4AwbMode;
    rExifInfo.u4CapExposureTime = rSensorInfo.CapExposureTime>0? rSensorInfo.CapExposureTime : 0;
    rExifInfo.u4FlashLightTimeus = m_bExifFlashOn;
    rExifInfo.u4AEISOSpeed = m_rParam.i4IsoSpeedMode;    // in ISO value (not enum)
    rExifInfo.u4RealISOValue = rSensorInfo.RealISOValue; // in ISO value
    rExifInfo.i4AEExpBias = m_rParam.i4ExpIndex;
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
getASDInfo(ASDInfo_T &/*a_rASDInfo*/) const
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
getP1DbgInfo(AAA_DEBUG_INFO1_T& /*rDbg3AInfo1*/, DEBUG_SHAD_ARRAY_INFO_T& /*rDbgShadTbl*/, AAA_DEBUG_INFO2_T& /*rDbg3AInfo2*/) const
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    switch (e3ACtrl)
    {
        case E3ACtrl_GetSensorSyncInfo:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4CaptureDelayFrame;
            break;
        case E3ACtrl_GetSensorPreviewDelay:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4SensorPreviewDelay;
            break;
        case E3ACtrl_GetSensorDelayInfo:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_i4AeShutDelayFrame;
            *(reinterpret_cast<MINT32*>(iArg2)) = m_i4AeISPGainDelayFrame;
            break;
        case E3ACtrl_GetSupportedInfo:
        {
            MINT32 ae_lock=0,awb_lock=0;
            MINT32 max_focus=0,max_meter=0;
            FeatureParam_T* rFeatureParam;
            rFeatureParam = reinterpret_cast<FeatureParam_T*>(iArg1);

            MY_LOG("[%s()] E3ACtrl_GetSupportedInfo\n", __FUNCTION__);

            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_AE_AWB_LOCK,(MUINTPTR)&ae_lock,(MUINTPTR)&awb_lock,0);
            m_fgAeLockSupp = ae_lock==1?1:0;
            m_fgAwbLockSupp = awb_lock==1?1:0;
            rFeatureParam->bExposureLockSupported = m_fgAeLockSupp;
            rFeatureParam->bAutoWhiteBalanceLockSupported = m_fgAwbLockSupp;
            MY_LOG("AE_sup(%d),AWB_sub(%d) \n",m_fgAeLockSupp,m_fgAwbLockSupp);

            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,(MUINTPTR)&max_focus,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,(MUINTPTR)&max_meter,0,0);
            rFeatureParam->u4MaxMeterAreaNum = max_meter>=1?1:0;
            rFeatureParam->u4MaxFocusAreaNum = max_focus>=1?1:0;
            m_max_metering_areas = max_meter;
            m_max_af_areas = max_focus;
            m_fgIsDummyLens = m_max_af_areas>0?MFALSE:MTRUE;
            MY_LOG("FOCUS_max(%d),METER_max(%d) \n",max_focus,max_meter);

            rFeatureParam->bEnableDynamicFrameRate = CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE();
            rFeatureParam->u4FocusLength_100x = 350;
            break;
        }
        default:
            break;
    }
    return MTRUE;
}

MUINT32
Hal3AYuvImp::
queryMagicNumber() const
{
    return m_u4MagicNumber;
}


VOID
Hal3AYuvImp::
queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight)
{
    i4TgWidth = m_i4TgWidth;
    i4TgHeight = m_i4TgHeight;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3AYuvImp::
attachCb(I3ACallBack* cb)
{
    MY_LOG("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    return 0;
}

MINT32
Hal3AYuvImp::
detachCb(I3ACallBack* cb)
{
    MY_LOG("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = NULL;
    return 0;

}

VOID
Hal3AYuvImp::
queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight)
{

    i4HbinWidth = m_i4HbinWidth;
    i4HbinHeight = m_i4HbinHeight;
}

MVOID
Hal3AYuvImp::
querySensorStaticInfo()
{
    MY_LOG("[%s] + SensorDev(%d), SensorOpenIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx);

    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        MY_ERR("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    m_i4SensorPreviewDelay = sensorStaticInfo.previewDelayFrame;
    m_i4AeShutDelayFrame = sensorStaticInfo.aeShutDelayFrame;
    m_i4AeISPGainDelayFrame = sensorStaticInfo.aeISPGainDelayFrame;
    m_i4CaptureDelayFrame = sensorStaticInfo.captureDelayFrame;

    MY_LOG("[%s] - m_i4SensorPreviewDelay(%d), m_i4AeShutDelayFrame(%d), m_i4AeISPGainDelayFrame(%d), m_i4CaptureDelayFrame(%d)", __FUNCTION__, m_i4SensorPreviewDelay, m_i4AeShutDelayFrame, m_i4AeISPGainDelayFrame, m_i4CaptureDelayFrame);

}

MBOOL
Hal3AYuvImp::
notifyPwrOn()
{
    return MTRUE;
}


MBOOL
Hal3AYuvImp::
notifyPwrOff()
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
notifyP1PwrOn()
{
    return MTRUE;
}


MBOOL
Hal3AYuvImp::
notifyP1PwrOff()
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
dumpP1Params(MUINT32 /*u4MagicNum*/)
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
setP2Params(P2Param_T const &/*rNewP2Param*/, ResultP2_T* /*pResultP2*/)
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
getP2Result(P2Param_T const &/*rNewP2Param*/, ResultP2_T* /*pResultP2*/)
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
notifyResult4TG(MINT32 /*i4PreFrmId*/, Result_T* /*rPreResult*/)
{
    return MTRUE;
}

MBOOL
Hal3AYuvImp::
notify4CCU(MUINT32 /*u4PreFrmId*/, Result_T* /*rPreResult*/)
{
    return MTRUE;
}


/* LUT for gain & dEv */
#define ASD_LOG2_LUT_RATIO_BASE 256
#define ASD_LOG2_LUT_NO 101
#define ASD_LOG2_LUT_CENTER 0
#define YUV_EVDELTA_THRESHOLD  10


const MINT32 ASD_LOG2_LUT_RATIO[ASD_LOG2_LUT_NO]={
256,/* 0 */
274, 294, 315, 338, 362, 388, 416, 446, 478, 512,/* 0.1~1.0 */
549, 588, 630, 676, 724, 776, 832, 891, 955, 1024,/* 1.1~2.0 */
1097, 1176, 1261, 1351, 1448, 1552, 1663, 1783, 1911, 2048,/* 2.1~3.0 */
2195, 2353, 2521, 2702, 2896, 3104, 3327, 3566, 3822, 4096,/* 3.1~4.0 */
4390, 4705, 5043, 5405, 5793, 6208, 6654, 7132, 7643, 8192,/* 4.1~5.0 */
8780, 9410, 10086, 10809, 11585, 12417, 13308, 14263, 15287, 16384,/* 5.1~6.0 */
17560, 18820, 20171, 21619, 23170, 24834, 26616, 28526, 30574, 32768,/* 6.1~7.0 */
35120, 37640, 40342, 43238, 46341, 49667, 53232, 57052, 61147, 65536,/* 7.1~8.0 */
70240, 75281, 80864, 86475, 92682, 99334, 106464, 114105, 122295, 131072,/* 8.1~9.0 */
140479, 150562, 161369, 172951, 185364, 198668, 212927, 228210, 244589, 262144/* 9.1~10.0 */
};

MINT32
Hal3AYuvImp::
ASDLog2Func(MUINT32 numerator, MUINT32 denominator) const
{
    MUINT32 temp_p;
    MINT32 x;
    MUINT32 *p_LOG2_LUT_RATIO = (MUINT32*)(&ASD_LOG2_LUT_RATIO[0]);

    temp_p = numerator*p_LOG2_LUT_RATIO[ASD_LOG2_LUT_CENTER];

    if (temp_p>denominator*ASD_LOG2_LUT_RATIO_BASE)
    {
        for (x=ASD_LOG2_LUT_CENTER; x<ASD_LOG2_LUT_NO; x++)
        {
            temp_p = denominator*p_LOG2_LUT_RATIO[x];

            if (temp_p>=numerator*ASD_LOG2_LUT_RATIO_BASE)
            {
                if ((temp_p -numerator*ASD_LOG2_LUT_RATIO_BASE)
                    > (numerator*ASD_LOG2_LUT_RATIO_BASE-denominator*p_LOG2_LUT_RATIO[x-1]))
                {
                    return x-1;
                }
                else
                {
                    return x;
                }
            }
            else if (x==ASD_LOG2_LUT_NO-1)
            {
                return (ASD_LOG2_LUT_NO-1);
            }
        }
    }
    return ASD_LOG2_LUT_CENTER;
}


#define ASD_ABS(val) (((val) < 0) ? -(val) : (val))

void Hal3AYuvImp::calcASDEv(const SENSOR_AE_AWB_CUR_STRUCT& cur)
{
    MINT32 AeEv;
    MY_LOG("[%s] shutter=%d,gain=%d,", __FUNCTION__,
        cur.SensorAECur.AeCurShutter,cur.SensorAECur.AeCurGain);

    //m_i4AELv_x10
    if ((m_AsdRef.SensorAERef.AeRefLV05Shutter * m_AsdRef.SensorAERef.AeRefLV05Gain)
        <= (cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain))
    {
        AeEv = 50;//0*80/IspSensorAeAwbRef.SensorLV05LV13EVRef+50;
    }
    else
    {
        AeEv = ASDLog2Func(m_AsdRef.SensorAERef.AeRefLV05Shutter * m_AsdRef.SensorAERef.AeRefLV05Gain,
                           cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain);
        if (AeEv == 0)
        {
            AeEv = 50;
        }
        else
        {
            if (m_AsdRef.SensorLV05LV13EVRef)
            {
                AeEv = AeEv * 80 / m_AsdRef.SensorLV05LV13EVRef + 50;
            }
            else
            {
                AeEv = 150;
            }
        }
    }

    if (AeEv > 150) // EV range from 50 ~150
    {
        AeEv = 150;
    }

    if (ASD_ABS(m_i4AELv_x10-AeEv) <= YUV_EVDELTA_THRESHOLD)
    {
        m_bAEStable = MTRUE;
    }
    else
    {
        m_bAEStable = MFALSE;
    }

    m_i4AELv_x10 = AeEv;

    MY_LOG("[%s] m_i4AELv_x10=%d", __FUNCTION__, m_i4AELv_x10);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuvImp::getDelayFrame(EQueryType_T const eQueryType) const
{
    MUINT32 ret = 0;
    SENSOR_DELAY_INFO_STRUCT pDelay;

    MY_LOG("[%s()] \n", __FUNCTION__);

    memset(&pDelay,0x0,sizeof(SENSOR_DELAY_INFO_STRUCT));
    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_DELAY_INFO,(MUINTPTR)&pDelay,0,0);
    MY_LOG("Init:%d,effect:%d,awb:%d,af:%d,ev:%d,sat:%d,bright:%d,contrast:%d \n",
           pDelay.InitDelay,pDelay.EffectDelay,pDelay.AwbDelay,pDelay.AFSwitchDelayFrame,
           pDelay.EvDelay,pDelay.SatDelay,pDelay.BrightDelay,pDelay.ContrastDelay);

    switch (eQueryType)
    {
        case EQueryType_Init:
        {
            ret = (pDelay.InitDelay>0 && pDelay.InitDelay<5)?pDelay.InitDelay:0;
            return ret;
        }
        case EQueryType_Effect:
        {
             ret = (pDelay.EffectDelay>0 && pDelay.EffectDelay<5)?pDelay.EffectDelay:0;
             return ret;
        }
        case EQueryType_AWB:
        {
            ret = (pDelay.AwbDelay>0 && pDelay.AwbDelay<5)?pDelay.AwbDelay:0;
            return ret;
        }
        case EQueryType_AF:
        {
            ret = pDelay.AFSwitchDelayFrame;
            ret = ret < 1200 ? ret : 0;
            return ret;
        }
        case EQueryType_Ev:
        {
            ret = (pDelay.EvDelay>0 && pDelay.EvDelay<5)?pDelay.EvDelay:0;
            return ret;
        }
        case EQueryType_Sat:
        {
            ret = (pDelay.SatDelay>0 && pDelay.SatDelay<5)?pDelay.SatDelay:0;
            return ret;
        }
        case EQueryType_Bright:
        {
            ret = (pDelay.BrightDelay>0 && pDelay.BrightDelay<5)?pDelay.BrightDelay:0;
            return ret;
        }
        case EQueryType_Contrast:
        {
            ret = (pDelay.ContrastDelay>0 && pDelay.ContrastDelay<5)?pDelay.ContrastDelay:0;
            return ret;
        }
        default:
            return 0;
    }
}

//******************************************************************************
// Map AE exposure to Enum
//******************************************************************************
MINT32 Hal3AYuvImp::mapAEToEnum(MINT32 mval,MFLOAT mstep)
{
    MINT32 pEv,ret;

    pEv = 100 * mval * mstep;

    if     (pEv <-250) { ret = AE_EV_COMP_n30;}  // EV compensate -3.0
    else if(pEv <-200) { ret = AE_EV_COMP_n25;}  // EV compensate -2.5
    else if(pEv <-170) { ret = AE_EV_COMP_n20;}  // EV compensate -2.0
    else if(pEv <-160) { ret = AE_EV_COMP_n17;}  // EV compensate -1.7
    else if(pEv <-140) { ret = AE_EV_COMP_n15;}  // EV compensate -1.5
    else if(pEv <-130) { ret = AE_EV_COMP_n13;}  // EV compensate -1.3
    else if(pEv < -90) { ret = AE_EV_COMP_n10;}  // EV compensate -1.0
    else if(pEv < -60) { ret = AE_EV_COMP_n07;}  // EV compensate -0.7
    else if(pEv < -40) { ret = AE_EV_COMP_n05;}  // EV compensate -0.5
    else if(pEv < -10) { ret = AE_EV_COMP_n03;}  // EV compensate -0.3
    else if(pEv ==  0) { ret = AE_EV_COMP_00; }  // EV compensate -2.5
    else if(pEv <  40) { ret = AE_EV_COMP_03; }  // EV compensate  0.3
    else if(pEv <  60) { ret = AE_EV_COMP_05; }  // EV compensate  0.5
    else if(pEv <  90) { ret = AE_EV_COMP_07; }  // EV compensate  0.7
    else if(pEv < 110) { ret = AE_EV_COMP_10; }  // EV compensate  1.0
    else if(pEv < 140) { ret = AE_EV_COMP_13; }  // EV compensate  1.3
    else if(pEv < 160) { ret = AE_EV_COMP_15; }  // EV compensate  1.5
    else if(pEv < 180) { ret = AE_EV_COMP_17; }  // EV compensate  1.7
    else if(pEv < 210) { ret = AE_EV_COMP_20; }  // EV compensate  2.0
    else if(pEv < 260) { ret = AE_EV_COMP_25; }  // EV compensate  2.5
    else if(pEv < 310) { ret = AE_EV_COMP_30; }  // EV compensate  3.0
    else               { ret = AE_EV_COMP_00;}

    MY_LOG("[%s()]EV:(%d),Ret:(%d)\n", __FUNCTION__, pEv,ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuvImp::mapISOToEnum(MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret;

    switch(u4NewAEISOSpeed){
        case 0:
            ret = AE_ISO_AUTO;
            break;
        case 100:
            ret = AE_ISO_100;
            break;
        case 200:
            ret = AE_ISO_200;
            break;
        case 400:
            ret = AE_ISO_400;
            break;
        case 800:
             ret = AE_ISO_800;
           break;
        case 1600:
            ret = AE_ISO_1600;
           break;
        default:
            MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);
            ret = AE_ISO_AUTO;
            break;
    }
    MY_LOG("[%s()]ISOVal:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEISOSpeed, ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuvImp::mapEnumToISO(MUINT32 u4NewAEIsoEnum) const
{
    MINT32 ret;

    switch(u4NewAEIsoEnum){
        case AE_ISO_AUTO:
            ret = 100;
            break;
        case AE_ISO_100:
            ret = 100;
            break;
        case AE_ISO_200:
            ret = 200;
            break;
        case AE_ISO_400:
            ret = 400;
            break;
        case AE_ISO_800:
             ret = 800;
           break;
        case AE_ISO_1600:
            ret = 1600;
           break;
        default:
            ret = 100;
            break;
    }
    MY_LOG("[%s()]ISOEnum:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEIsoEnum, ret);

    return ret;
}

MINT32 Hal3AYuvImp::setAFMode(MINT32 AFMode)
{
    switch (AFMode)
    {
    case MTK_CONTROL_AF_MODE_AUTO:
        MY_LOG("[%s] AF_MODE_AFS", __FUNCTION__);
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
        break;
    case MTK_CONTROL_AF_MODE_OFF:
    {
        MY_LOG("[%s] AF_MODE_INFINITY", __FUNCTION__);
        int iYuv3ACmd = SENSOR_3A_AF_INFINITY;
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_3A_CMD,(MUINTPTR)&iYuv3ACmd,0,0);
    }
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
    case MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
        MY_LOG("[%s] AF_MODE_AFC(%d)", __FUNCTION__, AFMode);
        m_i4AFSwitchCtrl = getDelayFrame(EQueryType_AF);
        MY_LOG("[%s] m_i4AFSwitchCtrl(%d)", __FUNCTION__, m_i4AFSwitchCtrl);
        break;
    default:
        break;
    }
    return S_3A_OK;
}

MINT32 Hal3AYuvImp::isFocused()
{
    MINT32 err = MHAL_NO_ERROR;
    MINT32 i4AfStatus = m_i4AfStatus;

    if (m_fgIsDummyLens)
    {
        return SENSOR_AF_FOCUSED;
    }

    if (SENSOR_AF_SCENE_DETECTING == i4AfStatus)
        i4AfStatus = SENSOR_AF_FOCUSING;

    return i4AfStatus;
}

MINT32
Hal3AYuvImp::
get3AStatusFromSensor()
{
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_AF_STATUS, (MUINTPTR)&m_i4AfStatus, 0, 0);
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_AE_STATUS, (MUINTPTR)&m_i4AeStatus, 0, 0);
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_AWB_STATUS, (MUINTPTR)&m_i4AwbStatus, 0, 0);

    MY_LOG_IF(m_3ALogEnable, "[%s] AF(%d), AE(%d), AWB(%d)", __FUNCTION__, m_i4AfStatus, m_i4AeStatus, m_i4AwbStatus);
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuvImp::queryAEFlashlightInfoFromSensor(SENSOR_FLASHLIGHT_AE_INFO_STRUCT& rAeFlashlightInfo)
{
    MINT32 err = MHAL_NO_ERROR;

    memset(&rAeFlashlightInfo, 0, sizeof(SENSOR_FLASHLIGHT_AE_INFO_STRUCT));

    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_STROBE_INFO,
        (MUINTPTR)&rAeFlashlightInfo, 0, 0);

    if (rAeFlashlightInfo.u4Fno == 0)
    {
        MY_ERR("Fail to get F#, set to 28");
        rAeFlashlightInfo.u4Fno = 28;
    }
    if (rAeFlashlightInfo.Exposuretime > 1000000 || rAeFlashlightInfo.Exposuretime == 0)
    {
        MY_ERR("Fail to get T(%d), set to 1000", rAeFlashlightInfo.Exposuretime);
        rAeFlashlightInfo.Exposuretime = 1000;
    }
    if (rAeFlashlightInfo.GAIN_BASE < MIN_ISO)
    {
        MY_ERR("Fail to get GAIN_BASE(%d), set to %d", rAeFlashlightInfo.GAIN_BASE, MIN_ISO);
        rAeFlashlightInfo.GAIN_BASE = MIN_ISO;
    }
    if (rAeFlashlightInfo.Gain == 0)
    {
        MY_ERR("Fail to get GAIN, set to %d", rAeFlashlightInfo.GAIN_BASE);
        rAeFlashlightInfo.Gain = rAeFlashlightInfo.GAIN_BASE;
    }

    MY_LOG("[%s] u4Fno(%d), Exposuretime(%d), Gain(%d), GAIN_BASE(%d)", __FUNCTION__,
        rAeFlashlightInfo.u4Fno,
        rAeFlashlightInfo.Exposuretime,
        rAeFlashlightInfo.Gain,
        rAeFlashlightInfo.GAIN_BASE);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuvImp::setEShutterParam(
       MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 err;

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0)) {
        MY_LOG("setExpParam() error: a_u4ExpTime = %d; a_u4SensorGain = %d; \n", a_u4ExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    MY_LOG("[%s] ExpTime(%d us), SensorGain(%d)\n", __FUNCTION__, a_u4ExpTime, a_u4SensorGain);

    // exposure time in terms of 32us
    a_u4ExpTime = a_u4ExpTime >> 5;
    a_u4SensorGain = a_u4SensorGain << 4;

    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_SENSOR_EXP_TIME,(MUINTPTR)&a_u4ExpTime,0,0);
    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_SENSOR_GAIN,(MUINTPTR)&a_u4SensorGain,0,0);

    return MHAL_NO_ERROR;
}

void Hal3AYuvImp::convertFlashExpPara(MINT32 flashEngRatio_x10, MINT32 minAfeGain_x1024,
             MINT32 bv0_x1024, MINT32 bv1_x1024,
             MINT32  exp1, MINT32  afeGain1_x1024, MINT32  ispGain1_x1024,
             MINT32& exp2, MINT32& afeGain2_x1024, MINT32& ispGain2_x1024) const
{
    MY_LOG("convertFlashExpParaa ratio=%d minG=%d bv0=%d bv1=%d\n",flashEngRatio_x10, minAfeGain_x1024, bv0_x1024, bv1_x1024);
    MY_LOG("convertFlashExpParaa exp=%d afe=%d isp=%d\n",exp1, afeGain1_x1024, ispGain1_x1024);
    if(minAfeGain_x1024==0)
        minAfeGain_x1024=2048;
    double bv0;
    double bv1;
    double engRatio;
    double delEv;
    double rat;
    bv0 = bv0_x1024/1024.0;
    bv1 = bv1_x1024/1024.0;
    engRatio = flashEngRatio_x10/10.0;


    /*
    double m0;
    double m1;
    double rat2;
    m0 = pow(2, bv0);
    m1 = pow(2, bv1);
    double rr;
    rat2 = ( (m0+(m1-m0)*engRatio)/m1);
    */

    if (bv1 < bv0)
    {
        exp2 = exp1 / engRatio;
        afeGain2_x1024 = afeGain1_x1024;
        ispGain2_x1024 = 1024;
        MY_LOG("[%s] bv1 < bv0!\n", __FUNCTION__);
            return;
    }

    rat = ((pow(2, bv1-bv0)-1)*engRatio +1)*pow(2, bv0-bv1);

    double maxGainRatio=1;
    if(afeGain1_x1024>minAfeGain_x1024)
        maxGainRatio = afeGain1_x1024/(double)minAfeGain_x1024;
    maxGainRatio *= ispGain1_x1024/1024.0;

    MY_LOG("[%s] rat(%3.6f), maxGainRatio(%3.6f)\n", __FUNCTION__, rat, maxGainRatio);

    double gainRatio;
    double expRatio;
    if(rat>maxGainRatio)
    {
        exp2 = exp1*(maxGainRatio/rat);
        afeGain2_x1024 = minAfeGain_x1024;
        ispGain2_x1024 = 1024;
    }
    else
    {
        gainRatio = afeGain1_x1024/(double)minAfeGain_x1024;
        MY_LOG("[%s] rat(%3.6f), gainRatio(%3.6f)\n", __FUNCTION__, rat, gainRatio);
        if(rat > gainRatio)
        {
            exp2 = exp1;
            afeGain2_x1024 = minAfeGain_x1024;
            ispGain2_x1024 = ispGain1_x1024/(rat/gainRatio);
        }
        else
        {
            exp2 = exp1;
            afeGain2_x1024 = afeGain1_x1024/rat;
            ispGain2_x1024 = ispGain1_x1024;
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MDOUBLE Hal3AYuvImp::calcBV(const SENSOR_FLASHLIGHT_AE_INFO_STRUCT& rAeFlashlightInfo)
{
    MDOUBLE  AV=0, TV=0, SV=0, BV=0;
    MINT32 ISO = 0;

    AV  = AEFlashlightLog2((MDOUBLE)rAeFlashlightInfo.u4Fno/10)*2;
    TV  = AEFlashlightLog2(1000000/((MDOUBLE)rAeFlashlightInfo.Exposuretime));
    ISO = rAeFlashlightInfo.Gain * MIN_ISO / rAeFlashlightInfo.GAIN_BASE;
    SV  = AEFlashlightLog2(((MDOUBLE)ISO)/3.125);

    BV = AV + TV - SV ;

    MY_LOG("[%s] AV(%3.6f), TV(%3.6f), ISO(%d), SV(%3.6f), BV(%3.6f)", __FUNCTION__, AV, TV, ISO, SV, BV);

    return (BV);
}

/*******************************************************************************
*
********************************************************************************/
MDOUBLE Hal3AYuvImp::AEFlashlightLog2(MDOUBLE x)
{
     return log(x)/log((double)2);
}


MINT32 Hal3AYuvImp::clamp(MINT32 x, MINT32 min, MINT32 max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

MVOID Hal3AYuvImp::mapAeraToZone(
    CameraArea_T *p_area, MINT32 areaW,
    MINT32 areaH, MINT32* p_zone,
    MINT32 zoneW, MINT32 zoneH)
{

    MINT32 left, top, right, bottom;

    p_area->i4Left   = clamp(p_area->i4Left,   0, areaW-1);
    p_area->i4Right  = clamp(p_area->i4Right,  0, areaW-1);
    p_area->i4Top    = clamp(p_area->i4Top,    0, areaH-1);
    p_area->i4Bottom = clamp(p_area->i4Bottom, 0, areaH-1);

    left   = p_area->i4Left   * zoneW / areaW;
    right  = p_area->i4Right  * zoneW / areaW;
    top    = p_area->i4Top    * zoneH / areaH;
    bottom = p_area->i4Bottom * zoneH / areaH;

    *p_zone     = clamp(left,   0, zoneW-1);
    *(p_zone+1) = clamp(top,    0, zoneH-1);
    *(p_zone+2) = clamp(right,  0, zoneW-1);
    *(p_zone+3) = clamp(bottom, 0, zoneH-1);
    *(p_zone+4) = zoneW;
    *(p_zone+5) = zoneH;

    MY_LOG("[%s] LTRBWH(%d,%d,%d,%d,%d,%d) -> LTRBWH(%d,%d,%d,%d,%d,%d)", __FUNCTION__,
        p_area->i4Left, p_area->i4Top, p_area->i4Right, p_area->i4Bottom, areaW, areaH,
        *p_zone, *(p_zone+1), *(p_zone+2), *(p_zone+3), *(p_zone+4), *(p_zone+5));
}

MBOOL Hal3AYuvImp::resetAFAEWindow()
{
    MINT32 ai4Zone[6];

    MY_LOG("[%s]", __FUNCTION__);

    // reset to center point
    ai4Zone[0] = ai4Zone[2] = YUV_IMG_WD/2;
    ai4Zone[1] = ai4Zone[3] = YUV_IMG_HT/2;
    ai4Zone[4] = YUV_IMG_WD;
    ai4Zone[5] = YUV_IMG_HT;

    if (m_max_af_areas > 0)
    {
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)ai4Zone,0,0);
    }

    if (m_max_metering_areas > 0)
    {
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AE_WINDOW,(MUINTPTR)ai4Zone,0,0);
    }

    return MTRUE;
}

MVOID Hal3AYuvImp::setFocusAreas(MINT32 a_i4Cnt, CameraArea_T *a_psFocusArea)
{
    MY_LOG("[%s] +", __FUNCTION__);

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_af_areas))
    {
        return ;
    }
    else  // spot or matrix meter
    {
        m_sAFAREA[0] = *a_psFocusArea;

        m_sAFAREA[0].i4Left   = clamp(m_sAFAREA[0].i4Left   + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAFAREA[0].i4Right  = clamp(m_sAFAREA[0].i4Right  + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAFAREA[0].i4Top    = clamp(m_sAFAREA[0].i4Top    + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);
        m_sAFAREA[0].i4Bottom = clamp(m_sAFAREA[0].i4Bottom + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);
        MY_LOG("[%s] LTRB(%d,%d,%d,%d)", __FUNCTION__,
            m_sAFAREA[0].i4Left, m_sAFAREA[0].i4Top, m_sAFAREA[0].i4Right, m_sAFAREA[0].i4Bottom);
        mapAeraToZone(&m_sAFAREA[0], ANDR_IMG_WD, ANDR_IMG_HT, &m_AFzone[0], YUV_IMG_WD, YUV_IMG_HT);
    }

    MY_LOG("[%s] -", __FUNCTION__);
}

MVOID Hal3AYuvImp::getFocusAreas(MINT32 &a_i4Cnt, CameraArea_T **a_psFocusArea)
{
    MY_LOG("[AF][%s()] \n", __FUNCTION__);

    a_i4Cnt = 1;
    *a_psFocusArea = &m_sAFAREA[0];
}

MVOID Hal3AYuvImp::getMeteringAreas(MINT32 &a_i4Cnt, CameraArea_T **a_psAEArea)
{
    MY_LOG("[AF][%s()] \n", __FUNCTION__);

    a_i4Cnt = 1;
    *a_psAEArea = &m_sAEAREA[0];
}

MVOID Hal3AYuvImp::setMeteringAreas(MINT32 a_i4Cnt, CameraArea_T const *a_psAEArea)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* zone_addr = (MUINT32*)&m_AEzone[0];

    MY_LOG("[%s] +", __FUNCTION__);

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_metering_areas))
    {
        return;
    }
    else  // spot or matrix meter
    {
        m_sAEAREA[0] = *a_psAEArea;

        m_sAEAREA[0].i4Left   = clamp(m_sAEAREA[0].i4Left   + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAEAREA[0].i4Right  = clamp(m_sAEAREA[0].i4Right  + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAEAREA[0].i4Top    = clamp(m_sAEAREA[0].i4Top    + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);
        m_sAEAREA[0].i4Bottom = clamp(m_sAEAREA[0].i4Bottom + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);

        MY_LOG("[%s] LTRB(%d,%d,%d,%d)", __FUNCTION__,
            m_sAEAREA[0].i4Left, m_sAEAREA[0].i4Top, m_sAEAREA[0].i4Right, m_sAEAREA[0].i4Bottom);

        mapAeraToZone(&m_sAEAREA[0], ANDR_IMG_WD, ANDR_IMG_HT, &m_AEzone[0], YUV_IMG_WD, YUV_IMG_HT);
        MY_LOG("[%s] zone_addr(0x%p)", __FUNCTION__, zone_addr);
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AE_WINDOW,(MUINTPTR)zone_addr,0,0);
    }

    MY_LOG("[%s] -", __FUNCTION__);
}

MINT32 Hal3AYuvImp::updateAeFlashCaptureParams()
{
    MINT32 i4StrobeGain = 0;
    MINT32 i4Shutter = 0, i4CfgGain = 0, i4Gain = 0;
    MINT32 i4StrobeWidth = 0;
    MINT32 i4HighcurrentTimeout = 0;

    MINT32 i4TestShutter = 0;
    MINT32 i4TestGain = 0;
    i4TestShutter = property_get_int32("vendor.debug.aaa_hal_yuv.shutter", -1);
    i4TestGain = property_get_int32("vendor.debug.aaa_hal_yuv.gain", -1);

    if (m_pStrobeDrvObj && m_u4StrobeDuty >0 && m_bFlashActive == MTRUE)
    {
        i4StrobeGain = m_rYuvFlParam.i4FlashlightGain10X;
        MY_LOG("[%s] i4StrobeGain(%d), m_fNoFlashBV(%f), m_fPreFlashBV(%f), m_i4PreFlashShutter(%d), m_i4PreFlashGain(%d)",
            __FUNCTION__, i4StrobeGain, m_fNoFlashBV, m_fPreFlashBV, m_i4PreFlashShutter, m_i4PreFlashGain);
        convertFlashExpPara(
            i4StrobeGain, m_rAeInfo.GAIN_BASE,
            m_fNoFlashBV*1024, m_fPreFlashBV*1024,
            m_i4PreFlashShutter, m_i4PreFlashGain, 1024,
            i4Shutter, i4Gain, i4CfgGain);

        m_fNoFlashBV = 0.0;
        i4StrobeWidth = m_u4StrobeDuty;
        if (i4StrobeGain > 10 && m_rParam.u4ShotMode != CAPTURE_MODE_BURST_SHOT)
        {
            MY_LOG("[%s] High current mode, i4Shutter(%d), i4Gain(%d)", __FUNCTION__, i4Shutter, i4Gain);
            // strobe led driver should implement 0xff as 2x.
            i4StrobeWidth = m_rYuvFlParam.i4FlashlightHighCurrentDuty;
            i4HighcurrentTimeout = m_rYuvFlParam.i4FlashlightHighCurrentTimeout;
            // set to sensor
            if (i4TestShutter != -1)
                i4Shutter = i4TestShutter;
            if (i4TestGain != -1)
                i4Gain = i4TestGain;
            setEShutterParam(i4Shutter, i4Gain);
        }

        if (m_pStrobeDrvObj->setTimeOutTime(i4HighcurrentTimeout) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setTimeOutTime(%d) ms", __FUNCTION__, i4HighcurrentTimeout);
        }
        if (m_pStrobeDrvObj->setDuty(i4StrobeWidth) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setLevel(%d)", __FUNCTION__, i4StrobeWidth);
        }
        if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setFire ON", __FUNCTION__);
        }
    }

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuvImp::setAeLock(MBOOL bLock)
{
    int iYuv3ACmd = bLock ? SENSOR_3A_AE_LOCK : SENSOR_3A_AE_UNLOCK;

    MY_LOG("[%s] bLock = %d\n", __FUNCTION__, bLock);

    if (m_fgAeLockSupp == 1)
    {
        MY_LOG("AE Lock supports, send CMD\n");
        m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_3A_CMD, (MUINTPTR)&iYuv3ACmd,0,0);
    }

    return MTRUE;

}

MBOOL Hal3AYuvImp::setAwbLock(MBOOL bLock)
{
    int iYuv3ACmd = bLock ? SENSOR_3A_AWB_LOCK : SENSOR_3A_AWB_UNLOCK;

    MY_LOG("[%s] bLock = %d\n", __FUNCTION__, bLock);

    if (m_fgAwbLockSupp == 1)
    {
        MY_LOG("AWB Lock supports, send CMD\n");
        m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_3A_CMD, (MUINTPTR)&iYuv3ACmd,0,0);
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT
Hal3AYuvImp::
onPreCaptureStart()
{
    const MINT32 i4FlashFrmCnt = m_rYuvFlParam.i4FlashlightFrameCnt;

    MY_LOG("[%s +] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    if (m_pStrobeDrvObj)
    {
        MBOOL fgFlashOn = (eShotMode_ZsdShot == m_rParam.u4ShotMode) ? m_isFlashOnCapture : isStrobeBVTrigger();
        if (fgFlashOn)
        {
            queryAEFlashlightInfoFromSensor(m_rAeInfo);
            m_fNoFlashBV = m_fPreFlashBV = calcBV(m_rAeInfo);

            //ON flashlight
            if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
            {
                MY_LOG("[%s] setTimeOutTime(0)", __FUNCTION__);
            }
            if (m_pStrobeDrvObj->setDuty(m_u4StrobeDuty) == MHAL_NO_ERROR)
            {
                MY_LOG("[%s] setLevel(%d)", __FUNCTION__, m_u4StrobeDuty);
            }
            if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
            {
                MY_LOG("[%s] setFire(1)", __FUNCTION__);
                m_u4PreFlashFrmCnt = i4FlashFrmCnt - 1;
                m_u4PreFlashFrmCnt = (MINT32) m_u4PreFlashFrmCnt < i4FlashFrmCnt ? m_u4PreFlashFrmCnt : 0;
                m_bFlashActive = MTRUE;
                m_bExifFlashOn = 1;
            }

            if (m_rYuvFlParam.i4FlashlightPreflashAF)
            {
                // trigger single AF on pre-flash
                MINT32 i4AfState = isFocused();
                MY_LOG("[%s] i4AfState(%d)\n", __FUNCTION__, i4AfState);

                setAFMode(AF_MODE_AFS);
                m_i4AutoFocus = E_YUV_SAF_FOCUSING;
                m_i4AutoFocusTimeout = 30;
                resetAFAEWindow();
                m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_CANCEL_AF, 0, 0, 0);
                m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE, 0, 0, 0);
            }
        }
        else
        {
            MY_LOG("[%s] No need to flash, ready to capture", __FUNCTION__);
            m_bReadyToCapture = 1;
        }
    }
    else
    {
        MY_LOG("[%s] No Strobe, ready to capture", __FUNCTION__);
        m_bReadyToCapture = 1;
    }

    // transit state to precapture
    m_i4State = E_YUV_STATE_PRECAPTURE;

    MY_LOG("[%s -] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    return S_3A_OK;
}

MRESULT
Hal3AYuvImp::
onPreCaptureEnd()
{
    MY_LOG("[%s] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    resetReadyToCapture();
    m_isFlashOnCapture = 0;
    return S_3A_OK;
}

MRESULT
Hal3AYuvImp::
updatePreCapture()
{
    const MINT32 i4FlashFrmCnt = m_rYuvFlParam.i4FlashlightFrameCnt;

    if (m_pStrobeDrvObj)
    {
        MY_LOG_IF(m_3ALogEnable, "[%s +] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
        if (m_bFlashActive == MTRUE)
        {
            if (m_rYuvFlParam.i4FlashlightPreflashAF)
            {
                if (m_i4AutoFocus)
                {
                    MINT32 i4AfState = isFocused();
                    MY_LOG("[%s] SAF(%d)\n", __FUNCTION__, i4AfState);
                    if (i4AfState == SENSOR_AF_FOCUSED)
                    {
                        m_i4AutoFocus = E_YUV_SAF_DONE;
                        m_u4PreFlashFrmCnt += 2;
                    }
                    else if (m_i4AutoFocusTimeout == 0)
                    {
                        m_i4AutoFocus = E_YUV_SAF_DONE;
                    }
                    m_i4AutoFocusTimeout = m_i4AutoFocusTimeout > 0 ? m_i4AutoFocusTimeout - 1 : 0;
                }
            }

            queryAEFlashlightInfoFromSensor(m_rAeInfo);
            m_fPreFlashBV = calcBV(m_rAeInfo);
            m_u4PreFlashFrmCnt = m_u4PreFlashFrmCnt > 0 ? m_u4PreFlashFrmCnt - 1 : 0;
            MY_LOG_IF(m_3ALogEnable, "[%s] Cnt(%d/%d)", __FUNCTION__, m_u4PreFlashFrmCnt, i4FlashFrmCnt);

            if (0 == m_u4PreFlashFrmCnt && E_YUV_SAF_DONE == m_i4AutoFocus)
            {
                m_i4PreFlashShutter = m_rAeInfo.Exposuretime;
                m_i4PreFlashGain = m_rAeInfo.Gain;
                //OFF flashlight after preflash done.
                if (m_bFlashActive == MTRUE)
                {
                    setAeLock(MTRUE);
                    setAwbLock(MTRUE);
                    if (m_pStrobeDrvObj->setOnOff(0) == MHAL_NO_ERROR)
                    {
                        MY_LOG("[%s] setFire OFF", __FUNCTION__);
                    }
                }
                m_u4PreFlashFrmCnt = i4FlashFrmCnt;
                m_bReadyToCapture = 1;
            }
        }
        MY_LOG_IF(m_3ALogEnable, "[%s -] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    }

    return S_3A_OK;
}

MRESULT
Hal3AYuvImp::
onCaptureStart()
{
    MY_LOG("[%s +] m_i4SensorDev(%d), shotMode(%d)", __FUNCTION__, m_i4SensorDev, (MINT32)m_rParam.u4ShotMode);
    updateAeFlashCaptureParams();

    // transit state to capture
    m_i4State = E_YUV_STATE_CAPTURE;

    MY_LOG("[%s -] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    return S_3A_OK;
}

MVOID
Hal3AYuvImp::
notifyPreStop()
{
    MY_LOG("[%s]", __FUNCTION__);
}
