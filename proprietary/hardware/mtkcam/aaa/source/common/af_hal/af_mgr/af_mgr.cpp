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
#define LOG_TAG "af_mgr_v3"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <sys/stat.h>
#include <sys/time.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <faces.h>
#include <private/aaa_hal_private.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <cct_feature.h>
#include <af_feature.h>
#include <af_define.h>
#include <af_tuning_custom.h>

#include "af_mgr.h"
#include <nvbuf_util.h>
//#include <isp_mgr.h>
#include "aaa_common_custom.h"
#include <pd_mgr_if.h>
#include "private/PDTblGen.h"
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

//
#include "mtkcam/utils/metadata/client/mtk_metadata_tag.h"

//configure HW
#include <StatisticBuf.h>

#include <math.h>
#include <android/sensor.h>             // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

#define LASER_TOUCH_REGION_W    0
#define LASER_TOUCH_REGION_H    0

#define AF_ENLOG_STATISTIC 2
#define AF_ENLOG_ROI 4

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_AF(String) \
          do { \
              aee_system_warning( \
                  "af_mgr", \
                  NULL, \
                  DB_OPT_DEFAULT|DB_OPT_FTRACE, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_AF(String)
#endif

// LaunchCamTrigger
#define AESTABLE_TIMEOUT 0
#define VALIDPD_TIMEOUT  0

#define GYRO_THRESHOLD 15

using namespace NS3Av3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSCam::Utils;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr& AfMgr::getInstance( MINT32 sensorDev)
{
    if (sensorDev == ESensorDev_MainSecond)
    {
        static AfMgr singletonMain2(sensorDev);
        return singletonMain2;
    }
    if (sensorDev == ESensorDev_MainThird)
    {
        static AfMgr singletonMain3(sensorDev);
        return singletonMain3;
    }
    else if (sensorDev == ESensorDev_Sub)
    {
        static AfMgr singletonSub1(sensorDev);
        return singletonSub1;
    }
    else if (sensorDev == ESensorDev_SubSecond)
    {
        static AfMgr singletonSub2(sensorDev);
        return singletonSub2;
    }
    else
    {
        if (sensorDev != ESensorDev_Main)
        {
            CAM_LOGE("%s sensorDev invalid : %d", __FUNCTION__, sensorDev);
        }
        static AfMgr singletonMain1(sensorDev);
        return singletonMain1;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::AfMgr( MINT32 eSensorDev) :
    m_sAFInput(&m_sDoAFInput.afInput),
    m_sAFOutput(&m_sDoAFOutput.afOutput),
    s_pIAfCxU(MNULL),
    m_i4IsCCUAF(CCU_AF_FEATURE_EN), // init with default value defined in makefile
    m_pAfoBuf(MNULL),
    m_i4Users(0),
    m_CCTUsers(0),
    m_i4EnableAF(-1),
    m_bLDAFEn(MFALSE),
    m_ptrNVRam(MNULL),
    m_i4DgbLogLv(0),
    m_pAFParam(MNULL),
    m_bGryoVd(MFALSE),
    m_bAcceVd(MFALSE),
    m_i4CurrSensorDev(eSensorDev),
    m_i4CurrSensorId(0x1),
    m_i4CurrModuleId(-1),
    m_i4CurrLensId(0),
    m_pMcuDrv(MNULL),
    m_pLaserMgr(MNULL)
{
    memset( &m_sDoAFInput,     0, sizeof(DoAFInput_T));
    memset( &m_sDoAFOutput,    0, sizeof(DoAFOutput_T));
    memset( &m_sAFResultConfig, 0, sizeof(AFRESULT_ISPREG_T));
    memset( &m_sArea_Focusing, 0, sizeof(AREA_T));
    memset( &m_sArea_Center,   0, sizeof(AREA_T));
    memset( &m_sArea_APCmd,    0, sizeof(AREA_T));
    memset( &m_sArea_APCheck,  0, sizeof(AREA_T));
    memset( &m_sArea_OTFD,     0, sizeof(AREA_T));
    memset( &m_sArea_HW,       0, sizeof(AREA_T));
    memset( &m_sArea_Bokeh,    0, sizeof(AREA_T));
    memset( &m_sCallbackInfo,  0, sizeof(CHECK_AF_CALLBACK_INFO_T));
    memset( &m_sIspAeInfo,     0, sizeof(ISPAEInfo_T));
    memset( &m_sCurLensInfo,   0, sizeof(LENS_INFO_T));
    memset( &m_sConfigInput,   0, sizeof(ConfigAFInput_T));
    memset( &m_sConfigOutput,  0, sizeof(ConfigAFOutput_T));
    memset( &m_sStartInput,    0, sizeof(StartAFInput_T));
    memset( &m_sStartOutput,   0, sizeof(StartAFOutput_T));

    m_i4TGSzW = m_i4TGSzH = 0;
    m_i4BINSzW = m_i4BINSzH = 0;
    m_eCurAFMode     = MTK_CONTROL_AF_MODE_EDOF; /*force to waitting AP send setting mode command. [Ref : setafmode function]*/
    m_bIsFullScan    = FALSE;
    m_aCurEMAFFlag   = m_aPreEMAFFlag = 0;   // for EMAF mode select: FullScan / AdvFullScan / Temperature Calibration
    m_aAdvFSRepeatTime = 10;
    m_i4FullScanStep = 0;
    m_eLIB3A_AFMode  = LIB3A_AF_MODE_AFS;
    m_i4SensorIdx    = 0;
    m_ptrIHalSensor          = NULL;
    m_bEnablePD      = MFALSE;
    m_i4PDAF_support_type    = PDAF_SUPPORT_NA;
    m_ptrLensNVRam           = NULL;
    m_eCamScenarioMode       = AF_CAM_SCENARIO_NUM;
    m_eNVRAMIndex            = AF_CAM_SCENARIO_NUM_2;
    m_i4OISDisable           = MFALSE;
    m_i4InitPos              = 0;
    m_i4LensPosExit          = 0;
    mpSensorProvider         = nullptr;
    m_bNeedCheckSendCallback = MFALSE;
    i4IsLockAERequest = E_AF_LOCK_AE_REQ_NO_LOCK;
    m_bForceUnlockAE = MFALSE;
    m_i4SkipAlgoDueToHwConstaint = 0;
    // LaunchCamTrigger : disable at the first
    m_i4LaunchCamTriggered = E_LAUNCH_AF_IDLE;
    m_i4AEStableFrameCount = -1;
    m_i4ValidPDFrameCount = -1;
    m_i4AEStableTriggerTimeout = AESTABLE_TIMEOUT;
    m_i4ValidPDTriggerTimeout = VALIDPD_TIMEOUT;
    m_i4LensSupport = -1;
    m_i4IsLockForLaunchCamTrigger = 0;

    m_i4IsCAFWithoutFace = 0;

    m_bForceDoAlgo = MFALSE;
    m_i4isAEStable = MFALSE;
    memset(m_u1LensFileName, 0, sizeof(MUINT8) * 32);
    m_u1LensFileName[0] = '\0';
    m_u4AfoStatMode = 0;
    m_u4AfoBlkSzByte = 0;
    c_sensorDev = ext_ESensorDev_None;
    m_sArea_TypeSel = 0;
    m_i4MvLensTo = 0;
    m_i4MvLensToPre = 0;
    m_pPipe = NULL;
    m_bNeedSendCallback = 0;
    TS_SOF_Pre = 0;
    m_i4FirsetCalPDFrameCount = 0;
    m_i4IsCctOper = 0;
    m_i4DgbRtvEnable = 0;
    m_i4ForceCPU = 0;
    memset(&m_sDAF_TBL, 0, sizeof(DAF_TBL_STRUCT));
    memset(&m_sCropRegionInfo, 0, sizeof(AREA_T));
    m_bPdInputExpected = MTRUE;
    m_i4HWBlkNumX = 0;
    m_i4HWBlkNumY = 0;
    m_i4HWBlkSizeX = 0;
    m_i4HWBlkSizeY = 0;
    memset(m_sMgrExif, 0, sizeof(AAA_DEBUG_TAG_T)*MGR_EXIF_SIZE);
    memset(m_sMgrCapExif, 0, sizeof(AAA_DEBUG_TAG_T)*MGR_CAPTURE_EXIF_SIZE);
    m_i4DgbDisableAF = 0;
    m_i4DgbLogLv = 0;
    m_i4DbgAfegainQueue = 0;
    m_i4DbgOISDisable = 0;
    m_i4DbgOISPos = 0;
    m_i4DbgPDVerifyEn = 0;
    m_i4DbgAdpAlarm = 0;
    m_i4DgbRtvEnable = 0;
    m_i4OTFDLogLv = 0;
    m_i4MgrExifSz = 0;
    m_i4MgrCapExifSz = 0;
    m_u4ACCEScale = 0;
    m_u4GyroScale = 0;
    m_u8AcceTS = 0;
    m_u8PreAcceTS = 0;
    m_u8GyroTS = 0;
    m_u8PreGyroTS = 0;
    m_bRunPDEn = MFALSE;
    m_i4PDCalculateWinNum = 0;
    memset(m_sPDCalculateWin, 0, sizeof(AFPD_BLOCK_ROI_T)*AF_PSUBWIN_NUM);

    m_i4IsAFSearch_PreState = AF_SEARCH_DONE;
    m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    m_i4IsSelHWROI_CurState = 0;
    m_i4IsSelHWROI_PreState = 0;
    m_i4TAFStatus = TAF_STATUS_RESET;
    m_i4IsFocused = 0;
    m_i4EnThermalComp = 0;
    m_i4MFPos = 0;
    m_i4InfPos = 0;
    m_i4MacroPos = 1023;
    m_i450cmPos = 0;
    m_i4MiddlePos = 0;
    m_i4AFTabStr = 0;
    m_i4AFTabEnd = 0;
    m_u8MvLensTS = 0;
    m_i4GyroValue = 0;
    m_bForceCapture = 0;
    m_i4ContinuePDMovingCount = 0;
    memset(&m_sFRMInfo, 0, sizeof(AF_FRAME_INFO_T));
    m_i4IsEnableFVInFixedFocus = 0;
    m_i4ReadOutTimePerLine = 0;
    m_i4P1TimeReadOut = 0;
    m_fMfFocusDistance = -1;
    m_i4CctMfPos = 0;
    m_i4isAFStarted = 0;
    m_bSDAFEn = MFALSE;
    m_eAFStatePre = m_eAFState = E_AF_INACTIVE;
    m_lensState = 0;
    m_bForceTrigger = MFALSE;
    m_bTriggerCmdVlid = MFALSE;
    m_bLatchROI = MFALSE;
    m_bLock = MFALSE;
    m_bNeedLock = MFALSE;
    m_i4IsEnterCam = 0;
    m_i4IsZSD = 0;
    m_bWaitForceTrigger = MFALSE;
    m_bPauseAF = MFALSE;
    m_i4UnPauseReqNum = 0;
    m_eEvent = EVENT_NONE;
    m_u4ReqMagicNum = m_u4StaMagicNum = m_u4ConfigHWNum = m_u4LatestResNum = 0;
    m_u4ConfigLatency = 0;
    m_u8SofTimeStamp = 0;
    m_i4DbgAutoBitTrue = 0;
    m_i4CctAfMode = 0;
    m_i4CctTriggerAf = 0;
    m_u8RecvFDTS = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::~AfMgr()
{}

/// init
MINT32 AfMgr::init( MINT32 i4SensorIdx)
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    c_sensorDev = (ext_ESensorDev_T)m_i4CurrSensorDev;

    MINT32 ret = 1;

    Mutex::Autolock lock(m_Lock);

    if ( m_i4Users == 0)
    {
        // initial property
        memset(&(m_sStartInput.initAlgoInput.sProperty), AF_NONE_PROPERTY, sizeof(AF_DBG_S_PROPERTY_T));
        checkStaticProperty(m_sStartInput.initAlgoInput.sProperty);

        /**
         * initial nonvolatilize data :
         * Which meas that the following parameters will not be changed once sensor mode is changed.
         * When sensor mode is changed, stop/start will be executed.
         */

        memset( &m_sMgrExif[0],    0, sizeof( AAA_DEBUG_TAG_T)*MGR_EXIF_SIZE);
        memset( &m_sMgrCapExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_CAPTURE_EXIF_SIZE);

        // initial flow control parameters.
        m_i4MgrExifSz      = 0;
        m_i4MgrCapExifSz   = 0;
        m_i4SensorIdx      = i4SensorIdx;
        m_eCamScenarioMode = AF_CAM_SCENARIO_NUM; /* update in setCamScenarioMode function*/
        m_eNVRAMIndex      = AF_CAM_SCENARIO_NUM_2;
        m_i4LensSupport    = -1;
        m_bPauseAF         = MFALSE;
        m_i4UnPauseReqNum  = 0;
        m_i4IsEnableFVInFixedFocus = -1;
        s_pIAfCxU = MNULL;
        m_i4IsEnterCam = 1;
        m_i4isAFStarted = 0;
        m_i4FullScanStep = 0;
        m_i4ForceCPU = 0;

        initSD(); // stereo
        initLD(); // laser
        initSP(); // sensor provider
        ret = 0;
    }
    else
    {
        /**
         *  Do not init af_mgr :
         *  1. User Cnt >= 1 : af_mgr is still used.
         *  2. User Cnt   < 0  : wrong host flow.
         */
        CAM_LOGD( "AF-%-15s: no init, user %d", __FUNCTION__, m_i4Users);
    }

    android_atomic_inc( &m_i4Users);
    CAM_LOGD( "AF-%-15s: EnAF %d, users %d", __FUNCTION__, m_i4EnableAF, m_i4Users);

    return ret;
}
MINT32 AfMgr::camPwrOn()
{
    // To reset parameter for starting a camera.
    CAM_LOGD( "AF-%-15s: Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    memset( &m_sCurLensInfo,   0, sizeof( LENS_INFO_T));
    m_eCurAFMode            = MTK_CONTROL_AF_MODE_EDOF;
    m_eLIB3A_AFMode         = LIB3A_AF_MODE_OFF;
    m_eAFStatePre           = m_eAFState = E_AF_INACTIVE;
    m_bLock                 = MFALSE;
    m_bNeedLock             = MFALSE;
    m_bLatchROI             = MFALSE;
    m_bWaitForceTrigger     = MFALSE;
    m_bForceTrigger         = MFALSE;
    m_bTriggerCmdVlid       = MFALSE;
    m_ptrLensNVRam          = NULL;
    m_i4MFPos               = -1;
    m_i4InitPos             = 0;

    return 0;
}
MINT32 AfMgr::config() // before start preview
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    Mutex::Autolock lock( m_Lock);
    MINT32 err = 0;

    if (m_i4DgbDisableAF)
    {
        err = 1;
    }

    // flow of config() ///////////////////////////////
    // 0. reset parameters
    // 1. get cxu instance
    // 2. get sensorId
    // 3. get lensId according to sensorId
    // 4. get NVRAM according to lensId
    // 5. get TG/BIN size
    // 6. config isp with default setting
    // 7. init pdmgr (including isp_pdo config)
    //////////////////////////////////////////////////

    if (!err)
    {
        // 0. reset parameters
        resetAFParams();
        // 1. get cxu instance
        if (!s_pIAfCxU)
        {
            CAM_LOGD("%s IAfCxU::getInstance", __FUNCTION__);
            s_pIAfCxU = IAfCxU::getInstance(m_i4CurrSensorDev, m_i4SensorIdx, m_i4ForceCPU);
        }
        else if (s_pIAfCxU->isCCUAF() == 0)
        {
            // AfCxU::getInstance again if current instance is CPUAF
            s_pIAfCxU = IAfCxU::getInstance(m_i4CurrSensorDev, m_i4SensorIdx, m_i4ForceCPU);
        }

        if (!s_pIAfCxU)
        {
            CAM_LOGE("AF-%-15s: Dev(0x%04x), IAfCxU pointer NULL", __FUNCTION__, m_i4CurrSensorDev);
            err = 1;
        }
        else
        {
            m_i4IsCCUAF = s_pIAfCxU->isCCUAF();
            CAM_LOGD("AF-%-15s: Dev(0x%04x), sCCUAF = %d", __FUNCTION__, m_i4CurrSensorDev, m_i4IsCCUAF);
        }
    }
    // 2. [SensorId] get sensor related information
    if (!err)
    {
        IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
        if ( !pIHalSensorList)
        {
            CAM_LOGE("AF-%-15s: Dev(0x%04x), pIHalSensorList NULL", __FUNCTION__, m_i4CurrSensorDev);
            err = 1;
        }
        else
        {
            // To get sensor information
            if ( m_ptrIHalSensor == NULL)
            {
                m_ptrIHalSensor = pIHalSensorList->createSensor( LOG_TAG, m_i4SensorIdx);
            }
            switch ( m_i4CurrSensorDev)
            {
            case ESensorDev_Main :
                pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
                break;
            case ESensorDev_Sub:
                pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
                break;
            case ESensorDev_MainSecond:
                pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
                break;
            case ESensorDev_SubSecond:
                pIHalSensorList->querySensorStaticInfo(  NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
                break;
            case ESensorDev_MainThird:
                pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN_3, &rSensorStaticInfo);
                break;
            default:
                CAM_LOGE( "AF-%-15s: Invalid sensor device: %d", __FUNCTION__, m_i4CurrSensorDev);
                err = 1;
                break;
            }
        }

        if (!err)
        {
            // To get sensorId from the sensorStaticInfo
            m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
            // To get info from SensorInfo for AdptComp
            MINT32 PixelClk = 0;
            m_ptrIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_PIXEL_CLOCK_FREQ, (MINTPTR)&PixelClk, 0, 0); // (Hz)
            m_sAFInput->PixelClk = PixelClk; //(Hz)
            MINT32 FrameSyncPixelLineNum = 0;
            m_ptrIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MINTPTR)&FrameSyncPixelLineNum, 0, 0); // (Pixel)
            m_sAFInput->PixelInLine = 0x0000FFFF & FrameSyncPixelLineNum; //(Pixel)


        }
    }
    // 3. [LensId] get lensId according to sensorId
    if (!err)
    {
        /*******************************************************
         * lensSearch MUST be done before get data from NVRAM !!
         *******************************************************/
        if ( m_pMcuDrv == NULL)
        {
            MUINT32 ModuleID = 0;
            AAA_TRACE_D("GetModuleIDFromCamCal");
            if (m_i4CurrModuleId == -1)
            {
                /* In order to get module id, it must be call CamCal */
                CAM_CAL_PART_NUM_STRUCT partnum_data;
                CamCalDrvBase* pCamCalDrvObj = CamCalDrvBase::createInstance();
                CAMERA_CAM_CAL_TYPE_ENUM eCamCalDataType = CAMERA_CAM_CAL_DATA_PART_NUMBER;
                MINT32 i4SensorDevID = -1;
                switch (m_i4CurrSensorDev)
                {
                case ESensorDev_Main:
                    i4SensorDevID = SENSOR_DEV_MAIN;
                    break;
                case ESensorDev_Sub:
                    i4SensorDevID = SENSOR_DEV_SUB;
                    break;
                case ESensorDev_MainSecond:
                    i4SensorDevID = SENSOR_DEV_MAIN_2;
                    break;
                case ESensorDev_SubSecond:
                    i4SensorDevID = SENSOR_DEV_SUB_2;
                    break;
                case ESensorDev_MainThird:
                    i4SensorDevID = SENSOR_DEV_MAIN_3;
                    break;
                default:
                    break;
                }
                if (i4SensorDevID != -1)
                {
                    MINT32 ret = pCamCalDrvObj->GetCamCalCalDataV2(m_i4CurrSensorDev, eCamCalDataType, &partnum_data, sizeof(CAM_CAL_PART_NUM_STRUCT));
                    m_i4CurrModuleId = (ret == CAM_CAL_ERR_NO_ERR) ? (partnum_data.PartNumber[1] << 8) + partnum_data.PartNumber[0] : 0;
                    CAM_LOGD( "AF-%-15s: Dev(0x%04x) Module ID (%d), ret (%d)", __FUNCTION__, m_i4CurrSensorDev, m_i4CurrModuleId, ret);
                }
            }
            AAA_TRACE_END_D;
            ModuleID = (m_i4CurrModuleId > 0) ? m_i4CurrModuleId : 0;

            m_pMcuDrv = MCUDrv::getInstance(m_i4CurrSensorDev);
            if (m_pMcuDrv)
            {
                m_pMcuDrv->lensSearch(m_i4CurrSensorId, ModuleID);
                m_i4CurrLensId = m_pMcuDrv->getCurrLensID();
                m_pMcuDrv->getCurrLensName(m_u1LensFileName);
                m_pMcuDrv->init(0, (MUINT32)m_i4SensorIdx, m_i4ForceCPU);
            }
            else
            {
                CAM_LOGE( "AF-%-15s: McuDrv::createInstance fail", __FUNCTION__);
                err = 1;
            }
        }
        err = ((getAfSupport() == 0) || (m_u1LensFileName[0] == '\0') ) ? 1 : 0;
        CAM_LOGW( "AF-%-15s: Dev(0x%04x), SensorID (0x%04x), ModuleId (0x%04x), LensId (0x%04x), LensFileName(%s)",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_i4CurrSensorId,
                  m_i4CurrModuleId,
                  m_i4CurrLensId,
                  m_u1LensFileName);
    }
    // 4. [NVRAM] get NVRAM according to lensId
    if (!err)
    {
        if (m_pMcuDrv)
        {
            m_pMcuDrv->setLensNvramIdx();
        }
        MINT32 err1 = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrLensNVRam);
        if (err1 == 0 && m_ptrLensNVRam)
        {
            for (int i = 0; i < AF_CAM_SCENARIO_NUM_2; i++)
            {
                if (*(m_ptrLensNVRam->AF[i].rFilePath) == 0)
                {
                    break;
                }
                CAM_LOGD_IF(LEVEL_FLOW, "AF-%-15s: Dev(0x%04x), NVRAM set[%d] path: %s", __FUNCTION__, m_i4CurrSensorDev, i, m_ptrLensNVRam->AF[i].rFilePath);
            }

            updateNVRAM(m_sAFIrgCommand.curNvramIndex);
            if ( m_eNVRAMIndex < AF_CAM_SCENARIO_NUM_2)
            {
                m_ptrNVRam = &(m_ptrLensNVRam->AF[m_eNVRAMIndex]);
                CAM_LOGD("AF-%-15s: Dev(0x%04x),  Nvram Idx(%d), NVRAM File Path %s",
                         __FUNCTION__,
                         m_i4CurrSensorDev,
                         m_eNVRAMIndex,
                         m_ptrNVRam->rFilePath);
                CAM_LOGD( "AF-%-15s: Dev(0x%04x), ParamInfo : Nvram Idx(%d), [nvram] ver(%d), Sz(%d, %zu, %zu, %zu, %zu, %zu),Normal Num(%d)",
                          __FUNCTION__,
                          m_i4CurrSensorDev,
                          m_eNVRAMIndex,
                          m_ptrLensNVRam->Version,
                          MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE,
                          sizeof(UINT32),
                          sizeof(FOCUS_RANGE_T),
                          sizeof(AF_NVRAM_T),
                          sizeof(PD_NVRAM_T),
                          MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE - sizeof(UINT32) - sizeof(FOCUS_RANGE_T) - sizeof(AF_NVRAM_T) - sizeof(PD_NVRAM_T),
                          m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum);
                CAM_LOGD_IF(LEVEL_VERBOSE, "AF-%-15s: Dev(0x%04x), i4EasyTuning: [0]%d,[1]%d,[2]%d,[3]%d,[4]%d,[5]%d,[6]%d,[10]%d,[11]%d,[20]%d,[21]%d,[22]%d",
                            __FUNCTION__,
                            m_i4CurrSensorDev,
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[0],  // initpos
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[1],  // TAF ROI resize if it is too small
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[2],  // preframe calculating pd result
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[3],  // TAF ROI resize if it is users desired
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[4],  // TAF ROI resize if ISO > ISO_Threshold
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[5],  // TAF ROI resize if ISO > ISO_Threshold
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[6],  // Threshold for PDMoving too long
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[10], // LaunchCamTrigger : m_i4AEStableTriggerTimeout
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[11], // LaunchCamTrigger : m_i4ValidPDTriggerTimeout
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[20], // CheckSendCallback : CompSet_PDCL.Target
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[21], // CheckSendCallback : CompSet_FPS.Target
                            m_ptrNVRam->rAFNVRAM.i4EasyTuning[22]);// CheckSendCallback : CompSet_ISO.Target
            }
            else
            {
                CAM_LOGE( "AF-%-15s: Dev(0x%04x), CamScenarioMode is not be updated by 3A framework!!", __FUNCTION__, m_i4CurrSensorDev);
                err = 1;
            }

        }
        else
        {
            CAM_LOGE( "AF-%-15s: Dev(0x%04x), AfAlgo NvBufUtil get buf fail!", __FUNCTION__, m_i4CurrSensorDev);
            err = 1;
        }
    }
    // 5. [TG/BIN size] get isp info
    if (!err)
    {
        // Get sensor information :
        // TG/BIN size
        sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&m_i4TGSzW),  (MINTPTR)(&m_i4TGSzH), 0);
        sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO,    (MINTPTR)(&m_i4BINSzW), (MINTPTR)(&m_i4BINSzH), 0);
        CAM_LOGD( "AF-%-15s: Dev(0x%04x), TGSZ: W %d, H %d, BINSZ: W %d, H %d",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_i4TGSzW,
                  m_i4TGSzH,
                  m_i4BINSzW,
                  m_i4BINSzH);
        if ( (m_sAFInput->sEZoom.i4W == 0) || (m_sAFInput->sEZoom.i4H == 0))
        {
            m_sAFInput->sEZoom.i4W = m_i4TGSzW;
            m_sAFInput->sEZoom.i4H = m_i4TGSzH;
        }
        //check ZSD or not
        m_i4IsZSD = ( m_i4TGSzW == (MUINT32)rSensorStaticInfo.captureWidth) && ( m_i4TGSzH == (MUINT32)rSensorStaticInfo.captureHeight) ? TRUE : FALSE;
        if (m_i4BINSzW == 0 || m_i4BINSzH == 0 || m_i4TGSzW == 0 || m_i4TGSzH == 0)
        {
            err = 1;
            CAM_LOGE( "AF-%-15s: WRONG TG/BIN size : TG[%d,%d], BIN[%d,%d]",
                      __FUNCTION__,
                      m_i4TGSzW, m_i4TGSzH,
                      m_i4BINSzW, m_i4BINSzH);
        }

        m_i4ReadOutTimePerLine = 0;
        m_i4P1TimeReadOut      = 0;
        if (m_sAFInput->PixelClk > 0)
        {
            m_i4ReadOutTimePerLine = (MINT32)(((MINT64)m_sAFInput->PixelInLine * 1000000000) / m_sAFInput->PixelClk); // unit: ns
            m_i4P1TimeReadOut      = (MINT32)(m_i4ReadOutTimePerLine * m_i4TGSzH / 1000); // unit:us
        }
        CAM_LOGD( "AF-%-15s: ReadOutTimePerLine(%d), P1DoneTime(%d)", __FUNCTION__, m_i4ReadOutTimePerLine, m_i4P1TimeReadOut);
    }
    // 6. config isp with default setting
    if (!err)
    {
        s_pIAfCxU->init(m_i4CurrSensorDev, m_i4SensorIdx, &m_u1LensFileName[0]);

        // To get m_bEnablePD before setCropRegion to get the collect centerROI.
        IPDMgr::getInstance().isModuleEnable( m_i4CurrSensorDev, m_bEnablePD);

        // Initial crop region information and center ROI coordinate will be updated automatically in SetCropRegionInfo.
        // NVRAM and TGSize are required for this.
        setCropRegionInfo( 0, 0, (MUINT32)m_i4TGSzW, (MUINT32)m_i4TGSzH, AF_MGR_CALLER);
        // get default AFParam
        getAFParam( m_i4CurrSensorDev, (AF_PARAM_T const **)&m_pAFParam);
        // get default AFParam/AFConfig and doISPConfig
        AF_CONFIG_T const *ptrHWCfgDef = NULL;
        getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
        if (ptrHWCfgDef)
        {
            // To reset initHWConfigInput
            m_sConfigInput.initHWConfigInput.enableLPB = 1;
            m_sConfigInput.initHWConfigInput.enableH3 = 1;
            m_sConfigInput.initHWConfigInput.enableRBSAT = 1;
            AF_CONFIG_T *ptrHWCfg = &(m_sConfigInput.initHWConfigInput.sDefaultHWCfg);
            memcpy( ptrHWCfg, ptrHWCfgDef, sizeof(AF_CONFIG_T));
            ptrHWCfg->sTG_SZ.i4W  = m_i4TGSzW;
            ptrHWCfg->sTG_SZ.i4H  = m_i4TGSzH;
            ptrHWCfg->sBIN_SZ.i4W = m_i4BINSzW;
            ptrHWCfg->sBIN_SZ.i4H = m_i4BINSzH;
            memcpy(&ptrHWCfg->sRoi, &m_sArea_Center, sizeof(AREA_T));
            memcpy(&m_sArea_Focusing, &m_sArea_Center, sizeof(AREA_T));   // To take the initial center ROI as m_sArea_Focusing

            // To set reference of configOutput
            m_sConfigOutput.resultConfig = &m_sAFResultConfig; // register map
            s_pIAfCxU->config(m_sConfigInput, m_sConfigOutput);
            m_u4AfoStatMode = m_sConfigOutput.initHWConfigOutput.sInitHWCfg.u4AfoStatMode;
            m_u4AfoBlkSzByte = m_sConfigOutput.initHWConfigOutput.sInitHWCfg.u4AfoBlkSzByte;

            if (!s_pIAfCxU->isCCUAF())
            {
                // NOT REQUIRED for CCUAF ===============================================================
                // update params for flow control
                memcpy(&m_sArea_HW, &m_sConfigOutput.hwConfigInfo.hwArea, sizeof(AREA_T)); // output info
                // data below SHOULD be move the afobufmgr (read from register)
                m_i4HWBlkNumX = m_sConfigOutput.hwConfigInfo.hwBlkNumX;     // for parsing AFO
                m_i4HWBlkNumY = m_sConfigOutput.hwConfigInfo.hwBlkNumY;     // for parsing AFO
                m_i4HWBlkSizeX = m_sConfigOutput.hwConfigInfo.hwBlkSizeX;   // for parsing AFO
                m_i4HWBlkSizeY = m_sConfigOutput.hwConfigInfo.hwBlkSizeY;   // for parsing AFO
                // NOT REQUIRED for CCUAF ===============================================================
            }
        }
        else
        {
            CAM_LOGE( "AF-%-15s: get af_tuning_custom parameters fail !! AFParam(%p), AFConfig(%p)",
                      __FUNCTION__,
                      m_pAFParam,
                      ptrHWCfgDef);
            err = 1;
        }
    }

    // 7. [LensInit] wait MCU init done
    if (!err)
    {
        if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[25] > 0)
        {
            if (m_pMcuDrv)
            {
                m_pMcuDrv->waitInitDone();
            }
        }
    }

    m_i4EnableAF = (err == 0) ? 1 : 0;
    CAM_LOGD( "AF-%-15s: -, m_i4EnableAF: %d", __FUNCTION__, m_i4EnableAF);

    return err;
}
MINT32 AfMgr::start() // after start preview
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    Mutex::Autolock lock( m_Lock);

    MINT32 err = 0;

    if (!m_i4EnableAF | m_i4DgbDisableAF)
    {
        err = 1;
    }
    // flow of start() ///////////////////////////////
    // 0. reset flow control parameters (with NVRAM)
    // 1. readOTP
    // 2. create instance of MCU and get mcuName
    // 3. reset algo parameters
    // 4. start cxu (init algo)
    // 5. init MCU
    //////////////////////////////////////////////////

    // 0. reset flow control parameters (with NVRAM)
    if (!err)
    {
        startSD(); // Set Depth AF Info
        startLD(); // init laser driver

        // LaunchCamTrigger
        m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] > 0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] : m_i4AEStableTriggerTimeout;
        m_i4ValidPDTriggerTimeout  = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] > 0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] : m_i4ValidPDTriggerTimeout;

        // Enable Thermal Compensation
        m_i4EnThermalComp = m_ptrNVRam->rAFNVRAM.i4TempErr[0];

        // checkSendCallback info : update the target form NVRAM
        m_sCallbackInfo.isAfterAutoMode     = 0;
        m_sCallbackInfo.isSearching         = AF_SEARCH_DONE;
        m_sCallbackInfo.CompSet_PDCL.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[20];
        m_sCallbackInfo.CompSet_FPS.Target  = m_ptrNVRam->rAFNVRAM.i4EasyTuning[21];
        m_sCallbackInfo.CompSet_ISO.Target  = m_ptrNVRam->rAFNVRAM.i4EasyTuning[22];
        CAM_LOGD("AF-%-15s: Dev(0x%04x), needCheckSendCallback %d, CallbackInfo.Targets: PDCL %d, FPS %d, ISO %d",
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_bNeedCheckSendCallback,
                 m_sCallbackInfo.CompSet_PDCL.Target,
                 m_sCallbackInfo.CompSet_FPS.Target,
                 m_sCallbackInfo.CompSet_ISO.Target);

        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = m_sAFOutput->i4IsAfSearch;
        m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState = m_sAFOutput->i4IsSelHWROI;

        COMMAND_CLEAR(m_sStartInput.initAlgoCommand);

        if ( m_bLock == MTRUE)
        {
            CAM_LOGD( "AF-%-15s: LockAF", __FUNCTION__);
            LockAlgo();
            m_sStartInput.initAlgoCommand.eLockAlgo = AfCommand_Start;
            m_bLock     = MTRUE;
            m_bNeedLock = MFALSE;
            // algo locked for preCapture ==> disable LaunchCamTrigger
            m_i4LaunchCamTriggered = (!m_i4IsLockForLaunchCamTrigger) ? E_LAUNCH_AF_DONE  : m_i4LaunchCamTriggered;
        }
    }
    // 1. readOTP : time consuming
    if (!err)
    {
        // Get AF calibration data and initPos. Should do this after setAFParam is called.
        if ( m_ptrNVRam->rAFNVRAM.i4Common[0] == TRUE)
        {
            readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
        }
    }

    // 3. reset algo parameters
    if (!err)
    {
        // OTP
        m_sStartInput.initAlgoInput.otpInfPos       = m_i4InfPos;
        m_sStartInput.initAlgoInput.otpMacroPos     = m_i4MacroPos;
        m_sStartInput.initAlgoInput.otpMiddlePos    = m_i4MiddlePos;
        // To get g/gyro data to get the initPos with posture compensation
        prepareSPData();
        memcpy(&(m_sStartInput.initAlgoInput.i4Acce), &(m_i4AcceInfo), sizeof(MINT32) * 3);
        m_sStartInput.initAlgoInput.i4AcceScale     = m_u4ACCEScale;

        m_sStartInput.initAlgoInput.i4IsEnterCam    = m_i4IsEnterCam;
        m_sStartInput.initAlgoInput.i4ISO           = m_sAFInput->i4ISO;
        m_sStartInput.initAlgoInput.eAFMode         = m_eLIB3A_AFMode;
        // TG/BIN size
        m_sStartInput.initAlgoInput.sTGSz.i4W       = m_i4TGSzW;
        m_sStartInput.initAlgoInput.sTGSz.i4H       = m_i4TGSzH;
        m_sStartInput.initAlgoInput.sHWSz.i4W       = m_i4BINSzW;
        m_sStartInput.initAlgoInput.sHWSz.i4H       = m_i4BINSzH;
        // AFArea
        m_sStartInput.initAlgoInput.sAFArea.i4Count = 1;
        memcpy(&m_sStartInput.initAlgoInput.sAFArea.sRect[0], &m_sArea_Focusing, sizeof(AREA_T));
        memcpy(&m_sStartInput.initAlgoInput.sEZoom, &m_sAFInput->sEZoom, sizeof(AREA_T));
        // LensInfo
        m_sStartInput.initAlgoInput.sLensInfo.i4CurrentPos = m_i4InitPos; // m_i4InitPos comes from readOTP
        // config/nvram
        memcpy(&m_sStartInput.initAlgoInput.sAFNvram, &(m_ptrNVRam->rAFNVRAM), sizeof(AF_NVRAM_T));
        memcpy(&m_sStartInput.initAlgoInput.sInitHwConfig, &m_sConfigOutput.initHWConfigOutput.sInitHWCfg, sizeof(AF_CONFIG_T));
        // MCU
        m_sStartInput.ptrMCUDrv                     = (MVOID*)m_pMcuDrv;
        // To pass current sensorDev to algo (for syncAF)
        m_sStartInput.initAlgoInput.i4RealSensorDev = m_i4CurrSensorDev;
        // TODO : BSS
        if (m_i4IsEnableFVInFixedFocus)
        {
            m_sAFOutput->sAFStatConfig.sRoi.i4X = 0;
            m_sAFOutput->sAFStatConfig.sRoi.i4Y = 0;
            m_sAFOutput->sAFStatConfig.sRoi.i4W = m_i4TGSzW;
            m_sAFOutput->sAFStatConfig.sRoi.i4H = m_i4TGSzH;
        }
    }
    // 4. start cxu (init algo)
    if (!err)
    {
        err = s_pIAfCxU->start(m_sStartInput, m_sStartOutput);

        m_i4IsEnterCam = 0;
        m_i4isAFStarted = 1;
        // get AF table start & end from updateAFtableBoundary
        m_i4AFTabStr = m_sStartOutput.initAlgoOutput.otpAfTableStr;
        m_i4AFTabEnd = m_sStartOutput.initAlgoOutput.otpAfTableEnd;
        m_i4InitPos = m_sStartOutput.initAlgoOutput.posturedInitPos;
        if ( (m_i4AFTabStr > 0) && ( m_i4AFTabEnd > m_i4AFTabStr))
        {
            m_sDAF_TBL.af_dac_min    = m_i4AFTabStr;
            m_sDAF_TBL.af_dac_max    = m_i4AFTabEnd;
        }

        // To clear algoCommand
        COMMAND_CLEAR(m_sDoAFInput.algoCommand);
        // clear dynamic property
        memset(&(m_sDoAFInput.afInput.dProperty), AF_NONE_PROPERTY, sizeof(AF_DBG_D_PROPERTY_T));
        // To set reference of doAFInput/doAFOutput
        memcpy(&m_sDoAFOutput.afOutput.sAFStatConfig, &m_sStartInput.initAlgoInput.sInitHwConfig, sizeof(AF_CONFIG_T));
        memcpy(&m_sDoAFOutput.ispOutput, &m_sConfigOutput, sizeof(ConfigAFOutput_T));
        m_sDoAFOutput.mvLensTS = m_u8MvLensTS;

    }
    // 5. set MCU init pos
    if (!err)
    {
        if (m_pMcuDrv)
        {
            if (m_pMcuDrv->setInitPos(m_sStartOutput.initAlgoOutput.posturedInitPos))
            {
                CAM_LOGD( "AF-%-15s: initMCU Dev %d, [MoveLensTo]posturedInitPos %d", __FUNCTION__, m_i4CurrSensorDev, m_sStartOutput.initAlgoOutput.posturedInitPos);
            }
        }
    }

    m_i4EnableAF = (err == 0) ? 1 : 0;

    CAM_LOGD( "AF-%-15s: -, m_i4EnableAF = %d", __FUNCTION__, m_i4EnableAF);
    return err;
}

/// uninit
MINT32 AfMgr::stop()
{
    Mutex::Autolock lock( m_Lock);
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    //uninitial isp_mgr_af_stat for configure HW
    if (s_pIAfCxU)
    {
        s_pIAfCxU->stop(m_sCurLensInfo);
        s_pIAfCxU->uninit();
        s_pIAfCxU = MNULL;
    }

    // clear pd parameters.
    m_bRunPDEn  = MFALSE;
    m_bEnablePD = MFALSE;
    m_i4PDAF_support_type = PDAF_SUPPORT_NA;

    if ( m_ptrIHalSensor)
    {
        m_ptrIHalSensor->destroyInstance( LOG_TAG);
        m_ptrIHalSensor = NULL;
    }

    UpdateState( EVENT_CMD_STOP);
    m_i4isAFStarted = 0;

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return 0;
}
MINT32 AfMgr::camPwrOff()
{
    //Camera Power Off, call by HAL, MW
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    if ( m_pMcuDrv)
    {
        CAM_LOGD( "AF-%-15s: uninitMcuDrv - Dev: %d", __FUNCTION__, m_i4CurrSensorDev);
        m_pMcuDrv->uninit();
        m_pMcuDrv->destroyInstance();
        m_pMcuDrv = NULL;
    }

    camPwrOffLD();

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return 0;
}
MINT32 AfMgr::uninit()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    MINT32 ret = 1;

    if ( m_i4Users == 1)
    {
        // Clear EMAF flag when leaving camera
        m_bIsFullScan = MFALSE;
        m_aCurEMAFFlag = 0;
        m_aPreEMAFFlag = 0;
        m_aAdvFSRepeatTime = 10;

        /* Sensor Provider */
        unintSP();

        ret = 0;
    }
    else
    {
        /**
         *  Do not uninit af_mgr :
         *  1. User Cnt   >1 : af_mgr is still used.
         *  2. User Cnt <=0  : wrong host flow.
         */
        CAM_LOGD( "AF-%-15s: no uninit, user %d", __FUNCTION__, m_i4Users);
    }

    android_atomic_dec( &m_i4Users);
    CAM_LOGD( "AF-%-15s: users %d", __FUNCTION__, m_i4Users);

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return ret;
}

/// process
MINT32 AfMgr::process(AFInputData_T data, AFCommand_T command)
{
    MINT32 err = 0;

    // 0. check if AF enabled
    if ( m_i4EnableAF == 0)
    {
        m_sAFOutput->i4IsAfSearch = AF_SEARCH_DONE;
        m_sAFOutput->i4IsFocused = 0;
        m_sAFOutput->i4AFPos = 0;
        m_bRunPDEn = MTRUE;
        CAM_LOGD_IF( LEVEL_FLOW, "disableAF");
        if (m_i4IsEnableFVInFixedFocus)
        {
            m_sArea_Focusing = m_sArea_OTFD;
            m_sArea_TypeSel  = (m_sArea_Focusing.i4W != 0 || m_sArea_Focusing.i4H != 0) ? AF_ROI_SEL_FD : AF_ROI_SEL_NONE;
        }
        err = 1;
    }

    // 0. doStt: processing AF statistics buffer
    if (!err)
    {
        err = process_doStt(command.requestNum, data.ptrAFStt);
    }

    AAA_TRACE_D("%s #(%d,%d)", __FUNCTION__, m_u4ReqMagicNum, m_u4StaMagicNum);

    // 1. doData: collecting all data for algo
    if (!err)
    {
        err = process_doData(data);
    }
    // 2. doCommand: metaCommand->algoCommand
    if (!err)
    {
        err = process_doCommand(command);
    }
    // 3. doAF: handleAF + move lens + config isp
    if (!err)
    {
        err = process_doAF();
    }
    // 4. updateOutput: To get algo output
    if (!err)
    {
        err = process_doOutput();
    }
    AAA_TRACE_END_D;

    return err;
}

/// get
MINT32 AfMgr::getStaticInfo(AFStaticInfo_T &staticInfo, char const * caller)
{
    staticInfo.isAfSupport        = getAfSupport();
    staticInfo.isAfThreadEnable   = getAfThreadEnable();

    staticInfo.isAfHwEnableByP1   = getAfHwEnableByP1();
    staticInfo.isCCUAF            = m_i4IsCCUAF;

    CAM_LOGD_IF(LEVEL_VERBOSE, "[%s] Dev(%d) by[%s] afSupport %d, afThreadEnable %d, afHwEnableByP1 %d, isCCUAF %d",
                __FUNCTION__,
                m_i4CurrSensorDev,
                caller,
                staticInfo.isAfSupport,
                staticInfo.isAfThreadEnable,
                staticInfo.isAfHwEnableByP1,
                staticInfo.isCCUAF);

    staticInfo.lastFocusPos   = m_sAFOutput->i4AFBestPos;
    if (m_ptrNVRam)
    {
        staticInfo.afTable          = getAFTable();
        staticInfo.afTableStepCount = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
    }
    staticInfo.minAfTablePos = m_sDAF_TBL.af_dac_min;
    staticInfo.maxAfTablePos = m_sDAF_TBL.af_dac_max;

    // FSC init info
    if (m_ptrNVRam)
        staticInfo.fscInitInfo.macro_To_Inf_Ratio = m_ptrNVRam->rAFNVRAM.i4ZoEffect[5];
    staticInfo.fscInitInfo.dac_Inf            = m_i4InfPos;
    staticInfo.fscInitInfo.dac_Macro          = m_i4MacroPos;
    staticInfo.fscInitInfo.damping_Time       = 12;
    staticInfo.fscInitInfo.readout_Time_us    = m_i4P1TimeReadOut;
    staticInfo.fscInitInfo.init_DAC           = m_i4InitPos;
    staticInfo.fscInitInfo.af_Table_Start     = m_i4AFTabStr;
    staticInfo.fscInitInfo.af_Table_End       = m_i4AFTabEnd;
    CAM_LOGD_IF(LEVEL_VERBOSE, "[%s] Get_AF_FSC_INIT_INFO %d, %d, %d, %d, %d, %d, %d, %d",
                __FUNCTION__,
                staticInfo.fscInitInfo.macro_To_Inf_Ratio,
                staticInfo.fscInitInfo.dac_Inf,
                staticInfo.fscInitInfo.dac_Macro,
                staticInfo.fscInitInfo.damping_Time,
                staticInfo.fscInitInfo.readout_Time_us,
                staticInfo.fscInitInfo.init_DAC,
                staticInfo.fscInitInfo.af_Table_Start,
                staticInfo.fscInitInfo.af_Table_End);

    staticInfo.crpR1_Sel_1 = 1; // default 1: RAW for AF without LTM

    return 1;
}
MINT32 AfMgr::getResult(AFResult_T &result)
{
    MINT32 err = 0;
    result.resultNum         = m_u4ReqMagicNum;
    result.afMode            = m_eCurAFMode;
    result.afState           = m_eAFState;
    result.lensState         = m_lensState;
    result.lensFocusDistance = getFocusDistance();
    getFocusRange(&(result.lensFocusRange_near), &(result.lensFocusRange_far));
    result.lockAERequest     = i4IsLockAERequest;
    result.isFocusFinish     = (m_eAFState != E_AF_PASSIVE_SCAN)    && (m_eAFState != E_AF_ACTIVE_SCAN);
    result.isFocused         = (m_eAFState == E_AF_PASSIVE_FOCUSED) || (m_eAFState == E_AF_FOCUSED_LOCKED);
    result.afSceneChange     = m_sAFOutput->i4IsSceneChange;
    result.lensPosition      = m_sAFOutput->i4AFPos;
    result.focusValue        = m_sAFOutput->i8AFValue;

    // type, num, left, top, right, bottom, isFDAF
    result.afRegions[0] = 0;
    result.afRegions[1] = 1;
    result.afRegions[2] = m_sArea_Focusing.i4X;
    result.afRegions[3] = m_sArea_Focusing.i4Y;
    result.afRegions[4] = m_sArea_Focusing.i4X + m_sArea_Focusing.i4W;
    result.afRegions[5] = m_sArea_Focusing.i4Y + m_sArea_Focusing.i4H;
    result.afRegions[6] = (m_sArea_TypeSel == AF_ROI_SEL_FD) ? 1 : 0;

    memcpy(result.gyroValue, m_i4GyroInfo, sizeof(MINT32) * 3);

    // fsc perframe info
    memcpy(&result.fscInfo, &fscInfo, sizeof(FSC_FRM_INFO_T));

    MBOOL isShowLog = 0;
    if (LEVEL_FLOW) // show once per frame
    {
        isShowLog = (m_u4LatestResNum != result.resultNum) ? 1 : 0;
    }
    else if (LEVEL_VERBOSE) // show everytime
    {
        isShowLog = 1;
    }

    CAM_LOGD_IF(isShowLog || (m_eAFState != m_eAFStatePre), "[%s] resultNum = %d, afMode = %d, afState = %d, lensState = %d, lockAEReq = %d, lensPosition = %d, isFocusFinish = %d, isFocused = %d, focusDistance = %f, eEvent = 0x%x",
                __FUNCTION__,
                result.resultNum,
                result.afMode,
                result.afState,
                result.lensState,
                result.lockAERequest,
                result.lensPosition,
                result.isFocusFinish,
                result.isFocused,
                result.lensFocusDistance,
                m_eEvent);

    m_eAFStatePre = m_eAFState;
    m_u4LatestResNum = result.resultNum;

    return err;
}
MVOID  AfMgr::getHWCfgReg(AFResultConfig_T * const pResultConfig)
{
    memcpy(&(pResultConfig->afIspRegInfo), &m_sAFResultConfig, sizeof(AFRESULT_ISPREG_T));
    CAM_LOGD_IF(LEVEL_FLOW, "HW-%s Enable(%d)", __FUNCTION__, m_sAFResultConfig.enableAFHw);
}
MINT32 AfMgr::getDAFTbl(AFStaticInfo_T &staticInfo)
{
    CAM_LOGD_IF(LEVEL_VERBOSE, "%s", __FUNCTION__);
    staticInfo.dafTbl = &m_sDAF_TBL;

    return 1;
}
MRESULT AfMgr::getDebugInfo( AF_DEBUG_INFO_T &rAFDebugInfo)
{
    MRESULT ret = E_3A_ERR;

    FUNC_START_DEV(LEVEL_VERBOSE)

    //reset.
    memset( &rAFDebugInfo, 0, sizeof(AF_DEBUG_INFO_T));

    /* Do not modify following oder: */

    //1. Hybrid AF library
    if (s_pIAfCxU)
        s_pIAfCxU->getDebugInfo(rAFDebugInfo);

    //2. PD library
    if ( m_i4PDAF_support_type != PDAF_SUPPORT_NA)
    {
        //Not support open pd library.
        if ( m_bEnablePD)
        {
            ret = IPDMgr::getInstance().GetDebugInfo( m_i4CurrSensorDev, rAFDebugInfo);
        }
    }

    //3. af mgr
    ret = GetMgrDbgInfo( rAFDebugInfo);
    return ret;
}

/// set
MVOID AfMgr::timeOutHandle()
{
    CAM_LOGD( "#(%5d,%5d) SPECIAL_EVENT cmd-%s Dev(%d)",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev);

    m_sDoAFInput.algoCommand.bCancel = MTRUE;
    UpdateState(EVENT_SEARCHING_END);
}
MRESULT AfMgr::setOTFDInfo( MVOID* sInROIs, MINT32 i4Type)
{
    MRESULT ret = E_3A_ERR;

    /*for algorithm to check input is FD or object, 0:face, 100:object*/
    if (i4Type != 0)
    {
        return ret;
    }
    /*TG base coordinate*/
    MtkCameraFaceMetadata *ptrWins = (MtkCameraFaceMetadata *)sInROIs;

    //prepare information and set to hybrid AF
    AREA_T OriFDArea;

    OriFDArea.i4X =  0;
    OriFDArea.i4Y =  0;
    OriFDArea.i4W =  0;
    OriFDArea.i4H =  0;
    OriFDArea.i4Info = AF_MARK_NONE;

    FD_INFO_T sAreaInfo;
    sAreaInfo.i4IsFdValid = 1;
    sAreaInfo.i4Score = 0;
    sAreaInfo.i4Count = 0;
    sAreaInfo.sRect[0].i4X =  0;
    sAreaInfo.sRect[0].i4Y =  0;
    sAreaInfo.sRect[0].i4W =  0;
    sAreaInfo.sRect[0].i4H =  0;
    sAreaInfo.sRect[0].i4Info = AF_MARK_NONE;

    if ( ptrWins != NULL)
    {
        //=== Portrait ===//
        sAreaInfo.i4PortEnable = (MINT32)(ptrWins->CNNFaces.PortEnable);
        sAreaInfo.i4IsTrueFace = (MINT32)(ptrWins->CNNFaces.IsTrueFace);
        sAreaInfo.f4CnnResult0 = (MFLOAT)(ptrWins->CNNFaces.CnnResult0);
        sAreaInfo.f4CnnResult1 = (MFLOAT)(ptrWins->CNNFaces.CnnResult1);
        CAM_LOGD_IF( LEVEL_FLOW,
                     "#(%5d,%5d) %s Dev(%d) Portrait ==> i4PortEnable: %d, i4IsTrueFace: %d, f4CnnResult: (%d, %d)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     sAreaInfo.i4PortEnable,
                     sAreaInfo.i4IsTrueFace,
                     (MINT32)(sAreaInfo.f4CnnResult0 * 1000),
                     (MINT32)(sAreaInfo.f4CnnResult1 * 1000));

        sAreaInfo.i4Score = 0;
        if ( ptrWins->number_of_faces != 0)
        {
            if ( ptrWins->faces != NULL)
            {
                // LaunchCamTrigger
                // there's at least one face ==> clear the TimeoutCount for LaunchCamTrigger
                m_i4AEStableTriggerTimeout = 0;
                m_i4ValidPDTriggerTimeout = 0;

                // rect => 0:left, 1:top, 2:right, 3:bottom
                OriFDArea.i4X = ptrWins->faces[0].rect[0];
                OriFDArea.i4Y = ptrWins->faces[0].rect[1];
                OriFDArea.i4W = ptrWins->faces[0].rect[2] - ptrWins->faces[0].rect[0];
                OriFDArea.i4H = ptrWins->faces[0].rect[3] - ptrWins->faces[0].rect[1];

                if ( (OriFDArea.i4W != 0) || (OriFDArea.i4H != 0))
                {
                    // OTFD data is valid.
                    sAreaInfo.i4Count  = 1;
                    memcpy(&sAreaInfo.sRect[0], &OriFDArea, sizeof(AREA_T));
                    // new FD info +
                    sAreaInfo.i4Id[0]        = ptrWins->faces[0].id;
                    sAreaInfo.i4Type[0]      = ptrWins->faces_type[0];
                    sAreaInfo.i4Motion[0][0] = ptrWins->motion[0][0];
                    sAreaInfo.i4Motion[0][1] = ptrWins->motion[0][1];
                    // update the timestamp of recieving FDInfo
                    m_u8RecvFDTS = getTimeStamp_us();
                    //=== Landmark ===//
                    // landmark CV
                    sAreaInfo.i4LandmarkCV[0] = ptrWins->fa_cv[0];
                    // left eye
                    sAreaInfo.i4Landmark[0][0][0] = ptrWins->leyex0[0];
                    sAreaInfo.i4Landmark[0][0][1] = ptrWins->leyey0[0];
                    sAreaInfo.i4Landmark[0][0][2] = ptrWins->leyex1[0];
                    sAreaInfo.i4Landmark[0][0][3] = ptrWins->leyey1[0];
                    // right eye
                    sAreaInfo.i4Landmark[0][1][0] = ptrWins->reyex0[0];
                    sAreaInfo.i4Landmark[0][1][1] = ptrWins->reyey0[0];
                    sAreaInfo.i4Landmark[0][1][2] = ptrWins->reyex1[0];
                    sAreaInfo.i4Landmark[0][1][3] = ptrWins->reyey1[0];
                    // mouth
                    sAreaInfo.i4Landmark[0][2][0] = ptrWins->mouthx0[0];
                    sAreaInfo.i4Landmark[0][2][1] = ptrWins->mouthy0[0];
                    sAreaInfo.i4Landmark[0][2][2] = ptrWins->mouthx1[0];
                    sAreaInfo.i4Landmark[0][2][3] = ptrWins->mouthy1[0];
                    // rip/rop info of FLD
                    sAreaInfo.i4LandmarkRIP[0] = ptrWins->fld_rip[0];
                    sAreaInfo.i4LandmarkROP[0] = ptrWins->fld_rop[0];
                    CAM_LOGD_IF( LEVEL_FLOW,
                                 "[%s] Landmark ==> CV: %d, left eye: (%d,%d,%d,%d), right eye: (%d,%d,%d,%d), mouth: (%d,%d,%d,%d), rip/rop: (%d,%d)",
                                 __FUNCTION__,
                                 sAreaInfo.i4LandmarkCV[0],     // CV
                                 sAreaInfo.i4Landmark[0][0][0], // left eye
                                 sAreaInfo.i4Landmark[0][0][1],
                                 sAreaInfo.i4Landmark[0][0][2],
                                 sAreaInfo.i4Landmark[0][0][3],
                                 sAreaInfo.i4Landmark[0][1][0], // right eye
                                 sAreaInfo.i4Landmark[0][1][1],
                                 sAreaInfo.i4Landmark[0][1][2],
                                 sAreaInfo.i4Landmark[0][1][3],
                                 sAreaInfo.i4Landmark[0][2][0], // mouth
                                 sAreaInfo.i4Landmark[0][2][1],
                                 sAreaInfo.i4Landmark[0][2][2],
                                 sAreaInfo.i4Landmark[0][2][3],
                                 sAreaInfo.i4LandmarkRIP[0],    // rip/rop
                                 sAreaInfo.i4LandmarkROP[0]);

                    ret = S_AF_OK;
                }
                else
                {
                    CAM_LOGD_IF( m_i4OTFDLogLv != 1, "[%s] data is not valid", __FUNCTION__);
                    m_i4OTFDLogLv = 1;
                }
            }
            else
            {
                CAM_LOGD_IF( m_i4OTFDLogLv != 2, "[%s] data is NULL ptr", __FUNCTION__);
                m_i4OTFDLogLv = 2;
            }
        }
        else
        {
            CAM_LOGD_IF( m_i4OTFDLogLv != 3, "[%s] num 0", __FUNCTION__);
            m_i4OTFDLogLv = 3;
        }
    }
    else
    {
        CAM_LOGD_IF( m_i4OTFDLogLv != 4, "[%s] Input NULL ptr", __FUNCTION__);
        m_i4OTFDLogLv = 4;
    }

    // 1. sAreaInfo is passed to AfAlgo
    // 2. FaceROI is extended by algo for HW setting
    memcpy(&m_sAFIrgCommand.FDROI, &sAreaInfo, sizeof(FD_INFO_T));

    if (sAreaInfo.i4Count != 0)
    {
        CAM_LOGD( "[%s]cnt:%d, type %d, FD = [X]%d [Y]%d [W]%d [H]%d ",
                  __FUNCTION__,
                  sAreaInfo.i4Count,
                  sAreaInfo.i4Score,
                  sAreaInfo.sRect[0].i4X, sAreaInfo.sRect[0].i4Y, sAreaInfo.sRect[0].i4W, sAreaInfo.sRect[0].i4H);
    }

    return ret;
}
MVOID AfMgr::notify(E_AF_NOTIFY flag)
{
    switch (flag)
    {
    case E_AFNOTIFY_SW:
        CAM_LOGD("%s E_AFNOTIFY_SW", __FUNCTION__);
        break;

    case E_AFNOTIFY_ABORT:
        CAM_LOGD("%s E_AFNOTIFY_ABORT", __FUNCTION__);
        if (s_pIAfCxU)
            s_pIAfCxU->abort();
        break;

    case E_AFNOTIFY_UNLOCKAE:
        CAM_LOGD("%s E_AFNOTIFY_UNLOCKAE", __FUNCTION__);
        // cancel the current search from taskFlashFront
        m_bForceUnlockAE = MTRUE;
        break;

    default :
        break;
    }
}
MRESULT AfMgr::setNVRAMIndex(MUINT32 a_eNVRAMIndex)
{
    m_sAFIrgCommand.curNvramIndex = a_eNVRAMIndex;
    return S_AF_OK;
}

/// ACDK
MRESULT AfMgr::setAFMode( MINT32 eAFMode, MUINT32 u4Caller)
{
    if ( m_i4EnableAF == 0)
    {
        return S_AF_OK;
    }

    if ( m_eCurAFMode == eAFMode)
    {
        return S_AF_OK;
    }

    /**
     *  Before new af mode is set, setting af area command is sent.
     */
    if ( u4Caller == AF_MGR_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  eAFMode);
    }
    else if (u4Caller == AF_CMD_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  eAFMode);
    }
    else if (u4Caller == AF_SYNC_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) sync-%s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  eAFMode);
    }

    m_eCurAFMode = eAFMode;

    LIB3A_AF_MODE_T preLib3A_AfMode = m_eLIB3A_AFMode;
    switch ( m_eCurAFMode)
    {
    case MTK_CONTROL_AF_MODE_OFF :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_MF; /*API2:The auto-focus routine does not control the lens. Lens  is controlled by the application.*/
        break;
    case MTK_CONTROL_AF_MODE_AUTO :
        if (m_aCurEMAFFlag == 0) // Auto Focus for Single shot
        {
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
        }
        else
        {
            // Engineer Mode: Full Scan
            if (m_aCurEMAFFlag == EM_AF_FLAG_FULLSCAN_NORMAL) // Full Scan
            {
                m_eLIB3A_AFMode = LIB3A_AF_MODE_FULLSCAN;
            }
            else if (m_aCurEMAFFlag == (EM_AF_FLAG_FULLSCAN_NORMAL | EM_AF_FLAG_TEMP_CALI)) // Temperature Calibration
            {
                m_eLIB3A_AFMode = LIB3A_AF_MODE_TEMP_CALI;
            }
        }
        break;
    case MTK_CONTROL_AF_MODE_MACRO :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_MACRO;
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_AFC_VIDEO;
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_AFC;
        break;
    case MTK_CONTROL_AF_MODE_EDOF :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_OFF;
        break;
    default :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
        break;
    }

    // checking whether it is valid for calculation ap roi.
    if ( m_eCurAFMode == MTK_CONTROL_AF_MODE_AUTO)
    {
        // ap roi is calculated during af mode is configured as auto mode.
        m_sPDRois[eIDX_ROI_ARRAY_AP].valid = MTRUE;
    }
    else
    {
        m_sPDRois[eIDX_ROI_ARRAY_AP].valid = MFALSE;
    }


    // log only.
    if ( u4Caller == AF_MGR_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d):lib_afmode %d->%d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  preLib3A_AfMode,
                  m_eLIB3A_AFMode);
    }
    else
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d):lib_afmode %d->%d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  preLib3A_AfMode,
                  m_eLIB3A_AFMode);
    }

    //update parameters and status.
    UpdateState( EVENT_CMD_CHANGE_MODE);
    return S_AF_OK;
}
MRESULT AfMgr::setFullScanstep( MINT32 a_i4Step)
{
    //if (m_i4FullScanStep != a_i4Step)
    {
        CAM_LOGD( "[setFullScanstep]%x", a_i4Step);

        m_i4FullScanStep = a_i4Step;
        m_aCurEMAFFlag = EM_AF_FLAG_NONE; // clear FullScan flag
        if ( m_i4FullScanStep > 0) /* Step > 0 , set Full Scan Mode */
        {
            m_bIsFullScan = MTRUE;
            m_sAFInput->i4FullScanStep = m_i4FullScanStep;

            // for Advanced Full Scan
            m_aCurEMAFFlag |= EM_AF_FLAG_FULLSCAN_NORMAL;
            m_aAdvFSRepeatTime = property_get_int32("vendor.mtk.client.em.af_advfs.rpt", 10);

            if (property_get_int32("vendor.mtk.client.em.af_advfs.enable", 0) == 1)
            {
                m_aCurEMAFFlag |= EM_AF_FLAG_FULLSCAN_ADVANCE;
            }
            else if (property_get_int32("vendor.mtk.client.em.af_cali_flag", 0) == 1)
            {
                m_aCurEMAFFlag |= EM_AF_FLAG_TEMP_CALI;
            }
        }
        else
        {
            m_bIsFullScan = MFALSE;
        }

        if (m_aCurEMAFFlag != m_aPreEMAFFlag)
        {
            setAFMode(MTK_CONTROL_AF_MODE_EDOF, AF_MGR_CALLER);
            setAFMode(MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
            m_aPreEMAFFlag = m_aCurEMAFFlag;
        }
    }
    return S_AF_OK;
}
MRESULT AfMgr::setPauseAF( MBOOL bIsPause)
{
    if (m_bPauseAF != bIsPause)
    {
        if (bIsPause == MTRUE)
        {
            CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d (PAUSE)",
                      m_u4ReqMagicNum,
                      m_u4StaMagicNum,
                      __FUNCTION__,
                      m_bPauseAF,
                      bIsPause);

            LockAlgo(AF_CMD_CALLER);
        }
        else
        {
            if (m_i4UnPauseReqNum == 0)
            {
                m_i4UnPauseReqNum = m_u4ReqMagicNum; // The unpause event wait to take effect.

                CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d (WAIT TO UNPAUSE)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          m_bPauseAF,
                          bIsPause);
            }
            else
            {
                if (m_u4StaMagicNum >= m_i4UnPauseReqNum) // The unpause event takes effect from ReqMagNum.
                {
                    CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d (CONTINUE)",
                              m_u4ReqMagicNum,
                              m_u4StaMagicNum,
                              __FUNCTION__,
                              m_bPauseAF,
                              bIsPause);

                    UnlockAlgo(AF_CMD_CALLER);
                    m_i4UnPauseReqNum = 0;
                }
            }
        }
    }

    return S_AF_OK;
}
