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
#include <isp_tuning_mgr.h>
#include <af_feature.h>
#include <af_define.h>

#include "af_mgr.h"
#include <nvbuf_util.h>
#include <isp_mgr.h>
#include "aaa_common_custom.h"
#include <pd_mgr_if.h>
#include "private/PDTblGen.h"

//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

//
#include "mtkcam/utils/metadata/client/mtk_metadata_tag.h"

//configure HW
#include <isp_mgr_af_stat.h>
#include <StatisticBuf.h>

#include <private/aaa_utils.h>
#include <array>

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

#define DBG_MSG_BUF_SZ 1024 //byte

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

/************************************************************************/
/* Systrace                                                             */
/************************************************************************/

#define TRACE_LEVEL_DBG 1
#define TRACE_LEVEL_DEF 0

#ifdef AF_TRACE_FMT_BEGIN
#undef AF_TRACE_FMT_BEGIN
#endif
#define AF_TRACE_FMT_BEGIN(lv, fmt, arg...)      \
    do {                                         \
        if (m_i4DbgSystraceLevel >= lv) {        \
            CAM_TRACE_FMT_BEGIN(fmt, ##arg);     \
        }                                        \
    } while(0)

#ifdef AF_TRACE_FMT_END
#undef AF_TRACE_FMT_END
#endif
#define AF_TRACE_FMT_END(lv)                     \
    do {                                         \
        if (m_i4DbgSystraceLevel >= lv) {        \
            CAM_TRACE_FMT_END();                 \
        }                                        \
    } while(0)

// LaunchCamTrigger
#define AESTABLE_TIMEOUT 0
#define VALIDPD_TIMEOUT  0

#define GYRO_THRESHOLD 15
#define MAX_PDMOVING_COUNT 6

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSCam::Utils;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MY_INST_AFMGR NS3Av3::INST_T<AfMgr>
static std::array<MY_INST_AFMGR, SENSOR_IDX_MAX> gMultitonAFMgr;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr& AfMgr::getInstance( MINT32 const i4SensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_AFMGR& rSingleton = gMultitonAFMgr[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AfMgr>(i4SensorDev);
    } );

    // CAM_LOGD("[%s] SensorDev(%d), SensorIdx(%d) instance(%p)\n", __FUNCTION__, i4SensorDev, i4SensorIdx, &(*(rSingleton.instance)));

    return *(rSingleton.instance);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::AfMgr(MINT32 const i4SensorDev)
{
    m_i4CurrSensorDev = i4SensorDev;
    m_i4Users   = 0;
    m_CCTUsers  = 0;
    m_pMcuDrv   = NULL;
    m_pIAfAlgo  = NULL;
    m_pAFParam  = NULL;
    m_bLDAFEn    = MFALSE;

    memset( &m_sAFInput,      0, sizeof(AF_INPUT_T));
    memset( &m_sAFOutput,     0, sizeof(AF_OUTPUT_T));
    m_ptrNVRam = NULL;
    memset( &m_sArea_Center,  0, sizeof(AREA_T));
    memset( &m_sArea_APCmd,   0, sizeof(AREA_T));
    memset( &m_sArea_APCheck, 0, sizeof(AREA_T));
    memset( &m_sArea_OTFD,    0, sizeof(AREA_T));
    memset( &m_sArea_OTFD_No_Extend,    0, sizeof(AREA_T));
    memset( &m_sArea_Bokeh,   0, sizeof(AREA_T));
    memset( &m_sCallbackInfo, 0, sizeof(CHECK_AF_CALLBACK_INFO_T));
    memset( &m_sIspSensorInfo, 0, sizeof(ISP_SENSOR_INFO_T));

    m_i4CurrSensorId = 0x1;
    m_i4CurrModuleId = -1;
    m_i4TGSzW        = 0;
    m_i4TGSzH        = 0;
    m_i4CurrLensId   = 0;
    m_eCurAFMode     = MTK_CONTROL_AF_MODE_EDOF; /*force to waitting AP send setting mode command. [Ref : setafmode function]*/
    m_bIsFullScan    = FALSE;
    m_aCurEMAFFlag = 0;   // for EMAF mode select: FullScan / AdvFullScan / Temperature Calibration
    m_aPreEMAFFlag = 0;   // for EMAF mode select: FullScan / AdvFullScan / Temperature Calibration
    m_aAdvFSRepeatTime = 10;
    m_i4FullScanStep = 0;
    m_i4EnableAF     = -1;
    m_AETargetMode   = AE_MODE_NORMAL;
    m_eLIB3A_AFMode  = LIB3A_AF_MODE_AFS;
    m_i4DgbLogLv     = 0;
    m_i4SensorIdx    = 0;
    m_i4SensorMode   = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    m_i4BINSzW       = 0;
    m_i4BINSzH       = 0;
    m_ptrIHalSensor          = NULL;
    m_bSensorModeSupportPDAF = MFALSE;
    m_PDPipeCtrl             = FEATURE_PDAF_UNSUPPORT;
    m_ptrLensNVRam           = NULL;
    m_eCamScenarioMode       = AF_CAM_SCENARIO_NUM;
    m_eNVRAMIndex            = AF_CAM_SCENARIO_NUM_2;
    m_i4OISDisable           = MFALSE;
    m_i4InitPos              = 0;
    m_i4LensPosExit          = 0;
    mpSensorProvider         = nullptr;
    m_bNeedCheckSendCallback = MFALSE;
    i4IsLockAERequest = 0;
    m_i4DbgSystraceLevel     = TRACE_LEVEL_DEF;
    // LaunchCamTrigger : disable at the first
    m_i4LaunchCamTriggered_Prv = m_i4LaunchCamTriggered = E_LAUNCH_AF_IDLE;
    m_i4AEStableFrameCount = -1;
    m_i4ValidPDFrameCount = -1;
    m_i4AEStableTriggerTimeout = AESTABLE_TIMEOUT;
    m_i4ValidPDTriggerTimeout = VALIDPD_TIMEOUT;
    m_i4EnThermalComp = 0;
    m_i4IsSMVR = MFALSE;
    m_i4AFMaxAreaNum = -1;
    m_i4IsLockForLaunchCamTrigger = 0;
    m_bGryoVd = MFALSE;
    m_bAcceVd = MFALSE;

    m_i4IsCAFWithoutFace = 0;

    m_bForceDoAlgo = MFALSE;
    m_i4isAEStable = MFALSE;
    m_vRawBufCtrl.clear();
    m_bIsAPROITooLarge = MFALSE;
    m_bRunAFByTwoFrame = MFALSE;

    m_pLaserMgr = NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::~AfMgr()
{}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sendAFNormalPipe( MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL ret = MFALSE;

    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe( m_i4SensorIdx, LOG_TAG);

    if ( pPipe == NULL)
    {
        CAM_LOGE( "Fail to create NormalPipe");
    }
    else
    {
        ret = pPipe->sendCommand( cmd, arg1, arg2, arg3);
        pPipe->destroyInstance( LOG_TAG);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getPdInfoForSttCtrl(MINT32 i4SensorIdx, MINT32 i4SensorMode, MUINT32 &u4PDOSizeW, MUINT32 &u4PDOSizeH, FEATURE_PDAF_STATUS &PDAFStatus)
{
    return IPDMgr::getInstance().getPDInfoForSttCtrl(m_i4CurrSensorDev, i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::init( MINT32 i4SensorIdx, MINT32 /*isInitMCU*/)
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    MRESULT ret = E_3A_ERR;

    Mutex::Autolock lock(m_Lock);

    if ( m_i4Users == 0)
    {
        /* Init af_mgr, when no user calls af_mgr init.*/

        // initial property
        m_i4DgbLogLv         = property_get_int32("vendor.debug.af_mgr.enable", 0);
        m_i4DbgAfegainQueue  = property_get_int32("vendor.debug.afegainqueue", 0);
        m_i4DbgPDVerifyEn    = property_get_int32("vendor.debug.pd_verify_flow.enable", 0);
        m_i4DbgSystraceLevel = property_get_int32("vendor.debug.afsystrace", TRACE_LEVEL_DEF);
        m_i4DbgAutoBitTrue   = property_get_int32("vendor.afo.autobittrue.enable", 0);

        /**
         * initial nonvolatilize data :
         * Which meas that the following parameters will not be changed once sensor mode is changed.
         * When sensor mode is changed, stop/start will be executed.
         */

        memset( &m_sMgrExif[0],    0, sizeof( AAA_DEBUG_TAG_T)*MGR_EXIF_SIZE);
        memset( &m_sMgrCapExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_CAPTURE_EXIF_SIZE);
        memset( &m_sMgrTSExif[0],  0, sizeof( AAA_DEBUG_TAG_T)*MGR_TS_EXIF_SIZE);

        // initial flow control parameters.
        m_i4MgrExifSz      = 0;
        m_i4MgrCapExifSz   = 0;
        m_i4MgrTsExifSz    = 0;
        m_i4SensorIdx      = i4SensorIdx;
        m_bMZHostEn        = 0;
        m_eCamScenarioMode = AF_CAM_SCENARIO_NUM; /* update in setCamScenarioMode function*/
        m_i4IsSMVR         = MFALSE;
        m_eNVRAMIndex      = AF_CAM_SCENARIO_NUM_2;
        m_i4AFMaxAreaNum   = -1;
        m_bIsSecureCam     = MFALSE;
        m_bPauseAF         = MFALSE;
        m_i4UnPauseReqNum  = 0;
        m_i4IsEnableFVInFixedFocus = -1;

        // Get hybrid AF instance.
#if USE_OPEN_SOURCE_AF
        m_pIAfAlgo = NS3A::IAfAlgo::createInstance<NS3A::EAAAOpt_OpenSource>( m_i4CurrSensorDev);
#else
        m_pIAfAlgo = NS3A::IAfAlgo::createInstance<NS3A::EAAAOpt_MTK>( m_i4CurrSensorDev);
#endif

        SDAF_Init();

        LDAF_Init();

        // --- Create SensorProvider Object ---
        AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "SensorProvider");
        if ( mpSensorProvider == NULL)
        {
            mpSensorProvider = SensorProvider::createInstance(LOG_TAG);

            if ( mpSensorProvider != nullptr)
            {
                MUINT32 interval = 30;

                interval = property_get_int32("vendor.debug.af_mgr.gyrointerval", SENSOR_GYRO_POLLING_MS);
                if ( mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval))
                {
                    m_bGryoVd = MTRUE;
                    CAM_LOGD_IF(m_i4DgbLogLv, "Dev(%d):enable SensorProvider success for Gyro ", m_i4CurrSensorDev);
                }
                else
                {
                    m_bGryoVd = MFALSE;
                    CAM_LOGE("Enable SensorProvider fail for Gyro");
                }

                interval = property_get_int32("vendor.debug.af_mgr.gyrointerval", SENSOR_ACCE_POLLING_MS);
                if ( mpSensorProvider->enableSensor(SENSOR_TYPE_ACCELERATION, interval))
                {
                    m_bAcceVd = MTRUE;
                    CAM_LOGD_IF(m_i4DgbLogLv, "Dev(%d):enable SensorProvider success for Acce ", m_i4CurrSensorDev);
                }
                else
                {
                    m_bAcceVd = MFALSE;
                    CAM_LOGE("Enable SensorProvider fail for Acce");
                }

            }
        }
        AF_TRACE_FMT_END(TRACE_LEVEL_DBG);

        ret = S_3A_OK;
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
    CAM_LOGD( "AF-%-15s: EnAF %d, Algo(%p), users %d", __FUNCTION__, m_i4EnableAF, (void*)m_pIAfAlgo, m_i4Users);

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::uninit( MINT32 /*isInitMCU*/)
{
    CAM_LOGD( "AF-%-15s: +", __FUNCTION__);

    MRESULT ret = E_3A_ERR;

    Mutex::Autolock lock(m_Lock);

    if ( m_i4Users == 1)
    {
        /* Uninit af_mgr, when last user calls af_mgr uninit.*/

        // restore AF-sync parameters
        //SyncAFWriteDatabase();

        // uninit hybrid AF
        if ( m_pIAfAlgo)
        {
            //m_pIAfAlgo->destroyInstance();
#if USE_OPEN_SOURCE_AF
            NS3A::IAfAlgo::destroyInstance<NS3A::EAAAOpt_OpenSource>( m_i4CurrSensorDev);
#else
            NS3A::IAfAlgo::destroyInstance<NS3A::EAAAOpt_MTK>( m_i4CurrSensorDev);
#endif
            m_pIAfAlgo = NULL;
        }

        // Clear EMAF flag when leaving camera
        m_bIsFullScan = MFALSE;
        m_aCurEMAFFlag = 0;
        m_aPreEMAFFlag = 0;
        m_aAdvFSRepeatTime = 10;

        if ( mpSensorProvider != NULL)
        {
            mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
            mpSensorProvider->disableSensor(SENSOR_TYPE_ACCELERATION);
            mpSensorProvider = NULL;
        }

        ret = S_3A_OK;
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
    CAM_LOGD( "AF-%-15s: Algo(%p), users %d", __FUNCTION__, (void*)m_pIAfAlgo, m_i4Users);

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::Start()
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    Mutex::Autolock lock( m_Lock);

    //reset member.
    memset( &m_sAFInput,           0, sizeof(AF_INPUT_T ));
    memset( &m_sAFOutput,          0, sizeof(AF_OUTPUT_T));
    memset( &m_sCropRegionInfo,    0, sizeof(AREA_T     ));
    memset( &m_sArea_Focusing,     0, sizeof(AREA_T     ));
    memset( &m_sArea_Center,       0, sizeof(AREA_T     ));
    memset( &m_sArea_APCmd,        0, sizeof(AREA_T     ));
    memset( &m_sArea_APCheck,      0, sizeof(AREA_T     ));
    memset( &m_sArea_APForContiMode,    0, sizeof(AREA_T));
    memset( &m_sArea_OTFD,         0, sizeof(AREA_T     ));
    memset( &m_sArea_OTFD_No_Extend,    0, sizeof(AREA_T));
    memset( &m_sArea_HW,           0, sizeof(AREA_T     ));
    memset( &m_sArea_Bokeh,        0, sizeof(AREA_T     ));
    memset( &m_sPDRois[0],         0, sizeof(PD_CALCULATION_ROI_T)*eIDX_ROI_ARRAY_NUM);
    memset( &m_sPDCalculateWin[0], 0, sizeof(AFPD_BLOCK_ROI_T    )*AF_PSUBWIN_NUM    );
    memset( &m_aAEBlkVal,          0, sizeof(MUINT8              ) * 25                );
    memset( &m_sIspSensorInfo,     0, sizeof(ISP_SENSOR_INFO_T));
    memset( &m_sFRMInfo,           0, sizeof(AF_FRAME_INFO_T));
    {
        Mutex::Autolock lock(m_DACListLock);
        DACList.clear();
    }

    m_vFrmInfo.clear();
    m_vISQueue.clear();
    m_i4EnableAF    = -1;
    m_i4PDCalculateWinNum = 0;
    m_i4DZFactor    = 100; /* Initial digital zoom factor. */
    m_sFocusDis.i4LensPos = 0; /* default value, ref: af_tuning_customer.cpp*/
    m_sFocusDis.fDist    = 0.33; /* default value, ref: af_tuning_customer.cpp*/
    m_i4MvLensTo    = -1;
    m_i4MvLensToPre = 0;
    m_i4HWBlkNumX   = 0;
    m_i4HWBlkNumY   = 0;
    m_i4HWBlkSizeX  = 0;
    m_i4HWBlkSizeY  = 0;
    m_i4HWEnExtMode = 0;
    m_i4IsFocused   = 0;
    m_u4ReqMagicNum = 0;
    m_u4StaMagicNum = 0;
    m_u4ConfigHWNum = 0;
    m_u4ConfigLatency = 3;
    m_bPdInputExpected = MFALSE;
    m_sArea_TypeSel = AF_ROI_SEL_NONE;
    m_sArea_APValid = 0;

    m_i4FlashPartialCount = 0;

    m_i4OTFDLogLv   = 0;
    m_eEvent        = EVENT_CMD_START;
    m_i4IsAFSearch_PreState = AF_SEARCH_DONE; /*Force to select ROI to focusing as first in, ref:SelROIToFocusing */
    m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    m_bRunPDEn          = MFALSE;
    m_bSDAFEn           = MFALSE;
    m_bMZAFEn           = MFALSE;
    m_bGetMetaData      = MFALSE;
    m_bNeedPdoResult    = MTRUE;
    m_i4TAFStatus       = TAF_STATUS_RESET;
    m_i4DbgOISPos       = 0;
    i4IsLockAERequest   = 0;
    m_i4IsCAFWithoutFace = 0;
    m_bForceDoAlgo = MFALSE;
    if (m_eAFState != E_AF_FOCUSED_LOCKED && m_eAFState != E_AF_NOT_FOCUSED_LOCKED)
    {
        CAM_LOGD("AF-%-15s: Dev(0x%04x), set CurAFMode to EDOF to enable LaunchCamTrigger",
                 __FUNCTION__,
                 m_i4CurrSensorDev);
        m_eCurAFMode = MTK_CONTROL_AF_MODE_EDOF;
    }
    // LaunchCamTrigger : disable at the first
    m_i4LaunchCamTriggered_Prv = m_i4LaunchCamTriggered = E_LAUNCH_AF_IDLE;
    m_i4AEStableFrameCount = -1;
    m_i4ValidPDFrameCount = -1;
    m_i4AEStableTriggerTimeout = AESTABLE_TIMEOUT;
    m_i4ValidPDTriggerTimeout = VALIDPD_TIMEOUT;
    m_i4EnThermalComp = 0;
    m_i4FirsetCalPDFrameCount = -1;
    m_i4isAEStable = MFALSE;
    m_i4ContinuePDMovingCount = 0;
    m_pdaf_raw_fmt = -1;
    m_vRawBufCtrl.clear();
    sem_init( &m_semSWPDE, 0, 0);
    m_fMfFocusDistance = -1;
    m_bIsAPROITooLarge = MFALSE;
    m_bRunAFByTwoFrame = MFALSE;
    /*--------------------------------------------------------------------------------------------------------
     *
     *                                            check algo instance
     *
     *--------------------------------------------------------------------------------------------------------*/
    if ( !m_pIAfAlgo)
    {
        CAM_LOGE("AF-%-15s: Dev(0x%04x), AfAlgo pointer NULL", __FUNCTION__, m_i4CurrSensorDev);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                       get sensor related information
     *
     *--------------------------------------------------------------------------------------------------------*/
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();

    if ( !pIHalSensorList)
    {
        CAM_LOGE("AF-%-15s: Dev(0x%04x), pIHalSensorList NULL", __FUNCTION__, m_i4CurrSensorDev);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }

    // get sensor information
    SensorStaticInfo  rSensorStaticInfo;
    SensorDynamicInfo rSensorDynamicInfo;

    if ( m_ptrIHalSensor == NULL)
    {
        m_ptrIHalSensor = pIHalSensorList->createSensor( LOG_TAG, m_i4SensorIdx);
    }

    MUINT32 i4HalSensorDev = mapSensorIdxToDev(m_i4SensorIdx);

    pIHalSensorList->querySensorStaticInfo(i4HalSensorDev, &rSensorStaticInfo);
    m_ptrIHalSensor->querySensorDynamicInfo(i4HalSensorDev, &rSensorDynamicInfo);

    m_sMetaData = pIHalSensorList->queryStaticInfo( m_i4SensorIdx);

    /* 0:BGGR, 1:GBRG, 2GRBG, 3RGGB */
    m_i4PixelId = (MINT32)rSensorStaticInfo.sensorFormatOrder;

    /* Lens search */
    m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
    if ( m_pMcuDrv == NULL)
    {
        /*******************************************************
         *
         * lensSearch MUST be done before get data from NVRAM !!
         *
         *******************************************************/
        MUINT32 ModuleID = 0;

        ModuleID = m_i4CurrModuleId = 0;

        m_pMcuDrv = MCUDrv::getInstance(m_i4CurrSensorDev);
        if (m_pMcuDrv)
        {
            m_pMcuDrv->lensSearch(m_i4CurrSensorId, ModuleID);
            m_i4CurrLensId = m_pMcuDrv->getCurrLensID();
            /* m_pMcuDrv->getCurrLensName(m_u1LensFileName); */
            m_pMcuDrv->init(0, (MUINT32)m_i4SensorIdx, 0);
        }
        else
        {
            CAM_LOGE( "AF-%-15s: McuDrv::createInstance fail", __FUNCTION__);
        }
    }

    m_i4EnableAF = (getAFMaxAreaNum() > 0) ? 1 : 0;// isAFSupport == hasAFLens || ForceEnableFVInFixedFocus

    MINT32 IsAFSupport = isAFSupport();

    CAM_LOGD( "AF-%-15s: Dev(0x%04x), PixId (%d)(0:B, 1:GB, 2:GR, 3:R), SensorID (0x%04x), ModuleId (0x%04x), LensId (0x%04x), EnableAF (%d), IsAFSupport(%d), PD_Support(%d)",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4PixelId,
              m_i4CurrSensorId,
              m_i4CurrModuleId,
              m_i4CurrLensId,
              m_i4EnableAF,
              IsAFSupport,
              rSensorStaticInfo.PDAF_Support);

    // Get info from SensorInfo for AdptComp
    MINT32 PixelClk = 0;
    m_ptrIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_PIXEL_CLOCK_FREQ, (MINTPTR)&PixelClk, 0, 0); // (Hz)
    m_sAFInput.PixelClk = PixelClk; //(Hz)
    MINT32 FrameSyncPixelLineNum = 0;
    m_ptrIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MINTPTR)&FrameSyncPixelLineNum, 0, 0); // (Pixel)
    m_sAFInput.PixelInLine = 0x0000FFFF & FrameSyncPixelLineNum; //(Pixel)

    /*--------------------------------------------------------------------------------------------------------
     *                                get NVRAM data : lensSearch MUST be done before this
     *--------------------------------------------------------------------------------------------------------*/
    if (m_pMcuDrv)
    {
        m_pMcuDrv->setLensNvramIdx();
    }
    MINT32 err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrLensNVRam);
    if ( (err != 0) || (!m_ptrLensNVRam))
    {
        CAM_LOGE( "AF-%-15s: AfAlgo NvBufUtil get buf fail!", __FUNCTION__);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }
    if ( m_eNVRAMIndex < AF_CAM_SCENARIO_NUM_2)
    {
        m_ptrNVRam = &(m_ptrLensNVRam->AF[m_eNVRAMIndex]);
        CAM_LOGD("AF-%-15s: Dev(0x%04x),  Nvram Idx(%d), NVRAM File Path %s",
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_eNVRAMIndex,
                 m_ptrNVRam->rFilePath);
    }
    else
    {
        CAM_LOGE( "AF-%-15s: CamScenarioMode is not be updated by 3A framework!!", __FUNCTION__);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }
    CAM_LOGD( "AF-%-15s: Dev(0x%04x), ParamInfo : Nvram Idx(%d), [nvram] ver(%d), Sz(%d, %zu, %zu, %zu, %zu, %zu, %zu),Normal Num(%d) Macro Num(%d) Normal min step(%d) Macro min step(%d)",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eNVRAMIndex,
              m_ptrLensNVRam->Version,
              MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE,
              sizeof(UINT32),
              sizeof(FOCUS_RANGE_T),
              sizeof(AF_NVRAM_T),
              sizeof(PD_NVRAM_T),
              sizeof(DUALCAM_NVRAM_T),
              MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE - sizeof(UINT32) - sizeof(FOCUS_RANGE_T) - sizeof(AF_NVRAM_T) - sizeof(PD_NVRAM_T) - sizeof(DUALCAM_NVRAM_T),
              m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum,
              m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum,
              m_ptrNVRam->rAFNVRAM.i4AFS_STEP_MIN_NORMAL,
              m_ptrNVRam->rAFNVRAM.i4AFS_STEP_MIN_MACRO);

    // Enable Thermal Compensation
    m_i4EnThermalComp = m_ptrNVRam->rAFNVRAM.i4TempErr[0];
    // LaunchCamTrigger
    m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] > 0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] : m_i4AEStableTriggerTimeout;
    m_i4ValidPDTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] > 0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] : m_i4ValidPDTriggerTimeout;
    // checkSendCallback info : update the target form NVRAM
    m_sCallbackInfo.isAfterAutoMode = 0;
    m_sCallbackInfo.isSearching = AF_SEARCH_DONE;
    m_sCallbackInfo.CompSet_PDCL.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[20];
    m_sCallbackInfo.CompSet_FPS.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[21];
    m_sCallbackInfo.CompSet_ISO.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[22];
    CAM_LOGD("AF-%-15s: Dev(0x%04x), needCheckSendCallback %d, CallbackInfo.Targets: PDCL %d, FPS %d, ISO %d",
             __FUNCTION__,
             m_i4CurrSensorDev,
             m_bNeedCheckSendCallback,
             m_sCallbackInfo.CompSet_PDCL.Target,
             m_sCallbackInfo.CompSet_FPS.Target,
             m_sCallbackInfo.CompSet_ISO.Target);

    if (m_i4DgbLogLv)
    {
        for (int i = 0; i < AF_CAM_SCENARIO_NUM_2; i++)
        {
            CAM_LOGD("LENS NVRAM set[%d] path: %s", i, m_ptrLensNVRam->AF[i].rFilePath);
        }
    }

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                    get pass1 related information
     *
     *--------------------------------------------------------------------------------------------------------*/
    // Get sensor information :
    // TG size
    sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&m_i4TGSzW), (MINTPTR)(&m_i4TGSzH), 0);
    // TG after BIN Blk size : for HPF coordinate setting.
    sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO, (MINTPTR)(&m_i4BINSzW), (MINTPTR)(&m_i4BINSzH), 0);
    // Checking sensor mode.
    CAM_LOGD( "AF-%-15s: Dev(0x%04x), TGSZ: W %d, H %d, BINSZ: W %d, H %d",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4TGSzW,
              m_i4TGSzH,
              m_i4BINSzW,
              m_i4BINSzH);

    if ( (m_sAFInput.sEZoom.i4W == 0) || (m_sAFInput.sEZoom.i4H == 0))
    {
        m_sAFInput.sEZoom.i4W = m_i4TGSzW;
        m_sAFInput.sEZoom.i4H = m_i4TGSzH;
    }

    //check ZSD or not
    MUINT32 isZSD = ( m_i4TGSzW == (MUINT32)rSensorStaticInfo.captureWidth) && ( m_i4TGSzH == (MUINT32)rSensorStaticInfo.captureHeight) ? TRUE : FALSE;
    m_sAFInput.i4IsZSD = isZSD;


    /*--------------------------------------------------------------------------------------------------------
     *
     *                                                Get parameters
     *
     *--------------------------------------------------------------------------------------------------------*/
    //===============
    // Get parameters from af_tuning_custom : default HW configure and distance-DAC table.
    //===============
    //distance-DAC table
    getAFParam( m_i4CurrSensorDev, &m_pAFParam);

    //default HW configure
    AF_CONFIG_T const *ptrHWCfgDef = NULL;
    getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);

    if ( !m_pAFParam || !ptrHWCfgDef)
    {
        CAM_LOGE( "AF-%-15s: get af_tuning_custom parameters fail !! AFParam(%p), AFConfig(%p)",
                  __FUNCTION__,
                  m_pAFParam,
                  ptrHWCfgDef);

        m_i4EnableAF = 0;

        return E_3A_ERR;
    }

    // set as default HW configure.
    memcpy( &m_sHWCfg, ptrHWCfgDef, sizeof(AF_CONFIG_T));
    CAM_LOGD_IF(m_i4DgbLogLv, "AF-%-15s: Dev(0x%04x), [AFParam] AFS_MODE(%d), AFC_MODE(%d), VAFC_MODE(%d), TBLL(%d), [DefAFConfig] AF_EXT_ENABLE(%d)",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_pAFParam->i4AFS_MODE,
              m_pAFParam->i4AFC_MODE,
              m_pAFParam->i4VAFC_MODE,
              m_pAFParam->i4TBLL,
              m_sHWCfg.AF_EXT_ENABLE);

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                           Set Depth AF Info
     *
     *--------------------------------------------------------------------------------------------------------*/
    SDAF_Start();

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                                 init PD mgr
     *         m_bSensorModeSupportPDAF & m_PDPipeCtrl set by getPdInfoForSttCtrl() before af_mgr::start()
     *   Which means before af_mgr::getPdInfoForSttCtrl should be executed before af_mgr::start() is executed.
     *
     *--------------------------------------------------------------------------------------------------------*/
    // Start PD manager.
    MRESULT err_ = IPDMgr::getInstance().config(m_i4CurrSensorDev, m_i4SensorIdx, m_i4SensorMode, m_eNVRAMIndex);
    if( err_==S_3A_OK)
    {
        SPDLibVersion_t PdLibVersion;
        IPDMgr::getInstance().GetVersionOfPdafLibrary( m_i4CurrSensorDev, PdLibVersion);
        CAM_LOGD( "AF-%-15s: Dev(0x%04x), config PD HAL done, PD version %d.%d", __FUNCTION__, m_i4CurrSensorDev, (MINT32)PdLibVersion.MajorVersion, (MINT32)PdLibVersion.MinorVersion);
    }
    else
    {
        CAM_LOGD( "AF-%-15s: Dev(0x%04x), config PD HAL fail", __FUNCTION__, m_i4CurrSensorDev);
    }

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                          init laser driver
     *
     *--------------------------------------------------------------------------------------------------------*/
    LDAF_Start();


    /*--------------------------------------------------------------------------------------------------------
     *
     *                                          initial Hybrid AF algorithm
     *
     *--------------------------------------------------------------------------------------------------------*/
    // Currently, multi zone is supported as PDAF is on.
    m_i4DbgMZEn = property_get_int32("vendor.af.mzaf.enable", 0);
    m_bMZAFEn = m_i4DbgMZEn || (m_bMZHostEn == 1) ? MTRUE : MFALSE;
    m_sAFInput.i4IsMZ = m_bMZAFEn;
    // Initial crop region information and center ROI coordinate will be updated automatically in SetCropRegionInfo.
    // All ROI parameters will be reset.
    SetCropRegionInfo( 0, 0, (MUINT32)m_i4TGSzW, (MUINT32)m_i4TGSzH, AF_MGR_CALLER);
    // default using center ROI. m_sArea_Center is updated after calling SetCropRegionInfo.
    m_sAFInput.sAFArea.i4Count  = 1;
    m_sAFInput.sAFArea.sRect[0] = m_sArea_Center;

    // update non-volatilize information for hybrid AF input
    memcpy( &(m_sAFInput.sLensInfo), &m_sCurLensInfo, sizeof( LENS_INFO_T));

    // set AF-sync parameter to hybrid AF
    SyncAFReadDatabase();

    // set parameters to hybrid AF.
    // should get default HW configuration from af_tuning_custom.
    if(m_pIAfAlgo)
        m_pIAfAlgo->setAFParam( (*m_pAFParam), m_sHWCfg, m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);

    // input TG and HW coordinate to hybridAF
    m_sAFInput.sTGSz = AF_COORDINATE_T( m_i4TGSzW,  m_i4TGSzH);
    m_sAFInput.sHWSz = AF_COORDINATE_T( m_i4BINSzW, m_i4BINSzH);

    // set firstCAF flag to hybrid AF
    m_sAFInput.i4IsEnterCam = 1; // default

    // set AdvFSRepeatTime to AFInput
    m_sAFInput.i2AdvFSRepeat = m_aAdvFSRepeatTime;

    // Ext mode setup : the platform check whether the ext mode is support or not
    m_sAFInput.i4IsExtStatMode = 1;

    // full scan related information
    m_sAFInput.i4FullScanStep = m_i4FullScanStep;

    // initial hybrid AF algorithm
    if(m_pIAfAlgo)
        m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

    if (m_i4IsEnableFVInFixedFocus)
    {
        m_sAFOutput.sAFStatConfig.sRoi.i4X = 0;
        m_sAFOutput.sAFStatConfig.sRoi.i4Y = 0;
        m_sAFOutput.sAFStatConfig.sRoi.i4W = m_i4TGSzW;
        m_sAFOutput.sAFStatConfig.sRoi.i4H = m_i4TGSzH;
    }

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                            Configure AF HW
     *
     *-----------------------------------------------------------------------------------------------------*/
    //initial isp_mgr_af_stat for configure HW after calling initAF.
    IspMgrAFStatStart();

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                            Control Hybrid AF
     *
     *-----------------------------------------------------------------------------------------------------*/
    // set mode to AF algo to start AF state machine
    if(m_pIAfAlgo)
        m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);
    m_eAFStatePre = m_eAFState;


    /*-----------------------------------------------------------------------------------------------------
     *
     *                                            init af_mgr flow control
     *
     *-----------------------------------------------------------------------------------------------------*/
    m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = m_sAFOutput.i4IsAfSearch;
    m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState = m_sAFOutput.i4IsSelHWROI;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                            AF Manager initial end
     *
     *-----------------------------------------------------------------------------------------------------*/
    if ( m_bLock == MTRUE)
    {
        if(m_pIAfAlgo)
        {
            m_pIAfAlgo->cancel();
            m_pIAfAlgo->lock();
        }
        CAM_LOGD( "AF-%-15s: LockAF", __FUNCTION__);
        m_bLock     = MTRUE;
        m_bNeedLock = MFALSE;

        if (!m_i4IsLockForLaunchCamTrigger)
        {
            // algo locked for preCapture ==> disable LaunchCamTrigger
            m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        }
    }

    m_bGetMetaData = MTRUE;

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::AFThreadStart()
{
    //---------------------------------------- StartPreview speed up -----------------
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    // Get AF calibration data. Should do this after setAFParam is called.
    if (m_ptrNVRam)
    {
        if (m_ptrNVRam->rAFNVRAM.i4ReadOTP == TRUE)
        {
            readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
        }
    }
    else
    {
        CAM_LOGE( "AF-%-15s: Dev %d, NVRAM init fail", __FUNCTION__, m_i4CurrSensorDev);
    }

    //---------------------------------------- init MCU ------------------------------
    if (m_pMcuDrv)
    {
        if (m_pMcuDrv->setInitPos(m_i4InitPos))
        {
            CAM_LOGD( "AF-%-15s: initMCU Dev %d, [MoveLensTo]m_i4InitPos %d", __FUNCTION__, m_i4CurrSensorDev, m_i4InitPos);
        }
    }

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);

    //
    IPDMgr::getInstance().isModuleEnable( m_i4CurrSensorDev, m_bEnablePD);

    // AFArea, force to update center ROI after get PD supporting info
    SetCropRegionInfo( 0, 0, (MUINT32)m_i4TGSzW, (MUINT32)m_i4TGSzH, AF_MGR_CALLER, MTRUE);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::Stop()
{
    Mutex::Autolock lock( m_Lock);

    //uninitial isp_mgr_af_stat for configure HW
    IspMgrAFStatStop();

    //store Nno-volatilize informaiton.
    getLensInfo( m_sCurLensInfo);
    CAM_LOGD( "AF-%-15s: + Dev %d, Record : Mode (%d)%d, Pos %d",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eCurAFMode,
              m_eLIB3A_AFMode,
              m_sCurLensInfo.i4CurrentPos );

    //reset parameters.
    m_bRunPDEn       = MFALSE;

    IPDMgr::getInstance().stop( m_i4CurrSensorDev);

    if ( m_ptrIHalSensor)
    {
        m_ptrIHalSensor->destroyInstance( LOG_TAG);
        m_ptrIHalSensor = NULL;
    }

    UpdateState( EVENT_CMD_STOP);

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::CamPwrOnState()
{
    //Camera Power On, call by HAL, MW
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    memset( &m_sCurLensInfo,   0, sizeof( LENS_INFO_T));

    m_eCurAFMode    = MTK_CONTROL_AF_MODE_EDOF;
    m_eLIB3A_AFMode = LIB3A_AF_MODE_OFF;
    m_eAFStatePre   = m_eAFState = E_AF_INACTIVE;

    m_bLock          = MFALSE;
    m_bNeedLock      = MFALSE;

    m_bLatchROI           = MFALSE;
    m_bWaitForceTrigger   = MFALSE;
    m_bForceTrigger       = MFALSE;
    m_bTriggerCmdVlid     = MFALSE;
    m_ptrLensNVRam        = NULL;

    m_i4MFPos           = -1;
    m_i4InitPos = 0;
    m_i4DbgMotorMPos      = 1024;
    m_i4DbgMotorMPosPre   = 1024;
    m_i4DbgMotorDisable   = 0;

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);

    //
    IPDMgr::getInstance().CamPwrOnState(m_i4CurrSensorDev);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::CamPwrOffState()
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

    //
    IPDMgr::getInstance().CamPwrOffState( m_i4CurrSensorDev);

    LDAF_CamPwrOffState();

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetPauseAF( MBOOL &bIsPause)
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
                if (m_u4StaMagicNum >= (MUINT32)m_i4UnPauseReqNum) // The unpause event takes effect from ReqMagNum.
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::triggerAF( MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
        CAM_LOGD( "%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);
    else
        CAM_LOGD( "cmd-%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    UpdateState( EVENT_CMD_TRIGGERAF_WITH_AE_STBL);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WaitTriggerAF( MBOOL &bWait)
{
    if ( m_i4EnableAF == 0)
    {
        return S_AF_OK;
    }
    if ( bWait == MTRUE)
    {
        UpdateState( EVENT_SET_WAIT_FORCE_TRIGGER);
    }
    else
    {
        UpdateState( EVENT_CANCEL_WAIT_FORCE_TRIGGER);
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    else
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d):ctl_afmode(%d)",
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
            else if (m_aCurEMAFFlag == (EM_AF_FLAG_FULLSCAN_NORMAL | EM_AF_FLAG_FULLSCAN_ADVANCE)) // Advanced Full Scan
            {
                m_eLIB3A_AFMode = LIB3A_AF_MODE_ADVFULLSCAN;
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

    //Set mode to hybrid AF algorithm
    if ( m_pIAfAlgo)
        m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);
    else
        CAM_LOGD( "Null m_pIAfAlgo");


    //update parameters and status.
    UpdateState( EVENT_CMD_CHANGE_MODE);
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::LockAlgo(MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
    {
        m_bLock = MTRUE;
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }
    else
    {
        m_bPauseAF = MTRUE;
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }

    if (m_bLock || m_bPauseAF)
    {
        if (m_pIAfAlgo)
        {
            m_pIAfAlgo->cancel();
            m_pIAfAlgo->lock();
            CAM_LOGD( "%s", __FUNCTION__);
        }
    }

    return m_bLock || m_bPauseAF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::UnlockAlgo(MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
    {
        m_bLock = MFALSE;
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }
    else
    {
        m_bPauseAF = MFALSE;
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }

    if (m_bLock == MFALSE && m_bPauseAF == MFALSE)
    {
        if (m_pIAfAlgo)
        {
            m_pIAfAlgo->unlock();
            CAM_LOGD( "%s", __FUNCTION__);
        }
    }

    return m_bLock || m_bPauseAF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateAutoMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        NewState = E_AF_INACTIVE;
        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MTRUE here to wait trigger searching..
         */
        m_bNeedCheckSendCallback = MFALSE;
        m_i4TAFStatus   = TAF_STATUS_RESET;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            UnlockAlgo();
        }
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if (      eInCurSate == E_AF_INACTIVE          ) NewState = E_AF_ACTIVE_SCAN;
        else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_ACTIVE_SCAN;
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_ACTIVE_SCAN;
        else                                                  NewState = eInCurSate; /*Should be at E_AF_ACTIVE_SCAN*/

        // get Autofocus when [(CenterROI exist) or (AE locked)] ==> ForceDoAlgo to speed up SEARCH_END
        if (m_i4IsCAFWithoutFace == 1 || i4IsLockAERequest == 1)
        {
            m_bForceDoAlgo = MTRUE;
        }
        m_sCallbackInfo.isAfterAutoMode = 1;
        m_i4TAFStatus = TAF_STATUS_RECEIVE_AUTOFOCUS;
        break;

    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        if (m_pIAfAlgo)
        {
            m_pIAfAlgo->trigger();
        }
        break;

    case EVENT_CMD_AUTOFOCUS_CANCEL :
        if (      eInCurSate == E_AF_ACTIVE_SCAN       ) NewState = E_AF_INACTIVE;
        else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_INACTIVE;
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_INACTIVE;
        else                                            NewState = eInCurSate; /*Should be at E_AF_INACTIVE*/

        if ( m_i4TAFStatus == TAF_STATUS_RECEIVE_AUTOFOCUS)
        {
            m_i4TAFStatus = TAF_STATUS_RECEIVE_CANCELAUTOFOCUS;
            CAM_LOGD( "receive cancelautofocus after seraching end");
        }

        if (m_pIAfAlgo)
            m_pIAfAlgo->cancel();

        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_SET_AF_REGION :
        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MTRUE here to wait trigger searching..
         */
        m_bLatchROI = MTRUE;
        NewState = eInCurSate;
        m_bIsAPROITooLarge = (m_sArea_APCmd.i4H > (m_i4TGSzH / 2)) ? MTRUE : MFALSE;
        CAM_LOGD_IF(m_bIsAPROITooLarge, "[%s] AP ROI is too large", __FUNCTION__);
        break;

    case EVENT_SEARCHING_START :
        NewState = E_AF_ACTIVE_SCAN;
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_END :
        m_bIsAPROITooLarge = m_bLatchROI = MFALSE;
        if ( eInCurSate == E_AF_ACTIVE_SCAN)
        {
            if ( m_i4IsFocused == 1) NewState = E_AF_FOCUSED_LOCKED;
            else                  NewState = E_AF_NOT_FOCUSED_LOCKED;
        }
        else
        {
            NewState = eInCurSate;
        }
        break;

    case EVENT_SET_WAIT_FORCE_TRIGGER :
        NewState = eInCurSate;
        break;

    case EVENT_AE_IS_STABLE :
    default : /*Should not be happened*/
        NewState = eInCurSate;
        break;
    }

    return NewState;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateContinuousMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        /**
         *  For normal continuous AF flow, AF will do lock once got autofocus command from host.
         *  Reset flags.
         */
        m_bLock = m_bNeedLock = MFALSE;

        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MFALSE here to wait trigger searching(change ROI only).
         */
        m_bIsAPROITooLarge = m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

        m_bNeedCheckSendCallback = MTRUE;

        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;

        if ( eInCurSate == E_AF_INACTIVE || m_i4LaunchCamTriggered != E_LAUNCH_AF_DONE)
        {
            // To prevent C T S fail because of the wrong af state while switching mode w/o trigger (original: E_AF_INACTIVE)
            NewState = E_AF_PASSIVE_UNFOCUSED;
            if ( m_i4TAFStatus == TAF_STATUS_RECEIVE_CANCELAUTOFOCUS)
            {
                CAM_LOGD( "Don't do AF searching after TAF");
            }
            else
            {
                CAM_LOGD( "[%s] LaunchCamTrigger, LockAlgo for waiting (AEStable or PDValid) + FDinfo", __FUNCTION__);
                m_bForceTrigger = m_bTriggerCmdVlid = MTRUE;
                // LaunchCamTrigger get enabled while force trigger searching
                m_i4LaunchCamTriggered = E_LAUNCH_AF_WAITING;
                m_i4AEStableFrameCount = -1;
                m_i4ValidPDFrameCount = -1;
                LockAlgo(); // to stop continuous af triggered by algo while launching camera
                m_i4IsLockForLaunchCamTrigger = 1;
            }
        }
        else if ( eInCurSate == E_AF_ACTIVE_SCAN       ) NewState = E_AF_PASSIVE_UNFOCUSED;
        else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_PASSIVE_FOCUSED;
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_PASSIVE_UNFOCUSED;
        else                                                NewState = eInCurSate;

        m_i4TAFStatus = TAF_STATUS_RESET;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if ( m_bForceTrigger == MTRUE)
        {
            if (m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO)
            {
                m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

                if (m_pIAfAlgo)
                    m_pIAfAlgo->cancel();

                NewState = E_AF_NOT_FOCUSED_LOCKED;
                CAM_LOGD( "During continuous-video mode : Immediate transition to lock state");
            }
            else
            {
                // force to trigger searching when changing ROI at continuous mode.
                NewState = E_AF_PASSIVE_SCAN;
                CAM_LOGD( "Wait force trigger and lock until searching done");
                m_bNeedLock = MTRUE;
            }
        }
        else
        {
            if (      eInCurSate == E_AF_INACTIVE          ) NewState = E_AF_NOT_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_PASSIVE_SCAN      )
            {
                if ( m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                {
                    /**
                     * During continuous-picture mode :
                     * Eventual transition once the focus is good.
                     * If autofocus(AF_TRIGGER) command is sent during searching, AF will be locked once searching done.
                     */
                    NewState = eInCurSate;
                }
                else
                {
                    /**
                     * During continuous-video mode :
                     * Immediate transition to lock state
                     */
                    NewState = E_AF_NOT_FOCUSED_LOCKED;
                }
            }
            else if ( eInCurSate == E_AF_PASSIVE_FOCUSED   ) NewState = E_AF_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_PASSIVE_UNFOCUSED ) NewState = E_AF_NOT_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_NOT_FOCUSED_LOCKED;
            else                                                NewState = eInCurSate; /*Should not be happened*/

            if ( m_eEvent & EVENT_CMD_CHANGE_MODE)
            {
                NewState = E_AF_PASSIVE_SCAN;
                CAM_LOGD( "Got changing mode and AF_TRIGGER at same time, force trigger searching");
                if (m_pIAfAlgo)
                {
                    m_pIAfAlgo->cancel();
                    m_pIAfAlgo->trigger();
                }
            }

            if ( NewState != E_AF_PASSIVE_SCAN)
            {
                LockAlgo();
            }
            else
            {
                // It will change AF state
                CAM_LOGD( "LockAF until searching done");
                m_bNeedLock = MTRUE;
            }
        }
        break;

    case EVENT_AE_IS_STABLE :
        /*
            if (NO_TRIGGER_CMD) keep previous AF state.  -> NO_TRIGGER_CMD = !(TRIGGER_CMD)
            else                trigger AF
        */
        if (!((m_bForceTrigger == MTRUE && m_bTriggerCmdVlid == MTRUE) && m_bWaitForceTrigger == MFALSE))
        {
            NewState = eInCurSate;
            break;
        }
        else
        {
            // LaunchCamTrigger
            if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
            {
                if (m_i4AEStableFrameCount == -1)
                {
                    m_i4AEStableFrameCount = m_u4ReqMagicNum;
                    CAM_LOGD("#(%5d,%5d) %s Dev(%d) LaunchCamTrigger EVENT_AE_IS_STABLE(%d) - BUT NOT TRIGGER YET",
                             m_u4ReqMagicNum, m_u4StaMagicNum,
                             __FUNCTION__,
                             m_i4CurrSensorDev,
                             m_i4AEStableFrameCount);
                }

                if (m_i4AEStableFrameCount != -1 && m_u4ReqMagicNum >= (MUINT32)(m_i4AEStableFrameCount + m_i4AEStableTriggerTimeout))
                {
                    // AE stable and AEStableTimeout(for waiting Face)
                    m_i4LaunchCamTriggered = E_LAUNCH_AF_TRIGGERED;
                    CAM_LOGD( "#(%5d,%5d) %s Dev(%d) LaunchCamTrigger EVENT_AE_IS_STABLE(%d + %d) - UnlockAlgo + TRIGGERAF lib_afmode(%d)",
                              m_u4ReqMagicNum, m_u4StaMagicNum,
                              __FUNCTION__,
                              m_i4CurrSensorDev,
                              m_i4AEStableFrameCount, m_i4AEStableTriggerTimeout,
                              m_eLIB3A_AFMode);
                    UnlockAlgo();
                    m_i4IsLockForLaunchCamTrigger = 0;
                    // no break : Intentionally fall through to triggerAF
                }
                else
                {
                    NewState = eInCurSate;
                    break; // AE stable but not timeout yet ==> skip triggerAF
                }
            }
            else
            {
                NewState = eInCurSate;
                break; // no break : Intentionally fall through to triggerAF
            }
        }
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        if ( m_bForceTrigger == MTRUE && m_bTriggerCmdVlid == MTRUE)
        {
            NewState = eInCurSate;

            m_bTriggerCmdVlid = MFALSE;
            // force to trigger searching when changing ROI at continuous mode.
            CAM_LOGD( "Force trigger searching [NeedLock(%d) LatchROI(%d) isAFSearch(%d)]", m_bNeedLock, m_bLatchROI, m_i4IsAFSearch_CurState);
            if (m_pIAfAlgo)
            {
                m_pIAfAlgo->cancel();
                m_pIAfAlgo->trigger();
            }
            /* HAL trigger AF during seraching without callback */
            m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        }
        else
        {
            NewState = eInCurSate;
        }
        break;

    case EVENT_CMD_AUTOFOCUS_CANCEL :
        if ( eInCurSate == E_AF_FOCUSED_LOCKED)
        {
            NewState = E_AF_PASSIVE_FOCUSED;
            UnlockAlgo();
        }
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED)
        {
            NewState = E_AF_PASSIVE_UNFOCUSED;
            UnlockAlgo();
        }
        else if ( eInCurSate == E_AF_PASSIVE_SCAN)
        {
            NewState = E_AF_INACTIVE;
            CAM_LOGD( "Abort search");
            if (m_pIAfAlgo)
                m_pIAfAlgo->cancel();
        }
        else
        {
            NewState = eInCurSate;
        }
        m_bNeedLock = MFALSE;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_SET_AF_REGION :
        if ( eInCurSate == E_AF_PASSIVE_SCAN)
        {
            NewState = E_AF_PASSIVE_UNFOCUSED;
            if (m_pIAfAlgo)
                m_pIAfAlgo->cancel();
        }
        else
        {
            NewState = eInCurSate;
        }

        if ( eInCurSate != E_AF_FOCUSED_LOCKED && eInCurSate != E_AF_NOT_FOCUSED_LOCKED)
        {
            /**
             *  For normal TAF flow, AF HAL got both auto mode and AF region information.
             *  So both m_bForceTrigger and m_bLatchROI will be set MTURE under normal TAF flow.
             *  If TAF is processed under continuous mode, setting m_bForceTrigger and m_bLatchROI as MTRUE here to force trigger searching.
             */
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = MTRUE;

            CAM_LOGD( "Force trigger with setting ROI");
        }
        break;

    case EVENT_SEARCHING_START :
        CAM_LOGD( "[%d]", eInCurSate);
        if (m_sCallbackInfo.isAfterAutoMode > 0)
        {
            m_sCallbackInfo.isAfterAutoMode--;
        }
        if (      eInCurSate == E_AF_INACTIVE         ) NewState = E_AF_PASSIVE_SCAN;
        else if ( eInCurSate == E_AF_PASSIVE_FOCUSED  ) NewState = E_AF_PASSIVE_SCAN;
        else if ( eInCurSate == E_AF_PASSIVE_UNFOCUSED) NewState = E_AF_PASSIVE_SCAN;
        else                                                 NewState = eInCurSate;
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;
        CAM_LOGD( "isAFSearch[%d]", m_i4IsAFSearch_CurState);
        if ( m_i4IsAFSearch_CurState)
        {
            NewState = eInCurSate;
            break;
        }
    case EVENT_CMD_STOP : // intentionally fall through
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;
    // intentionally fall through
    case EVENT_SEARCHING_END :
        CAM_LOGD( "TriggerCmdValid(%d)/WaitForceTrigger(%d)/bNeedLock(%d)/IsFocused(%d)/eInCurSate(%d)",
                  m_bTriggerCmdVlid,
                  m_bWaitForceTrigger,
                  m_bNeedLock,
                  m_i4IsFocused,
                  eInCurSate);
        if ( m_bTriggerCmdVlid == MTRUE && m_bWaitForceTrigger == MTRUE)
        {
            NewState = E_AF_PASSIVE_SCAN;
        }
        else
        {
            //reset parameters
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

            if ( m_bNeedLock == MTRUE)
            {
                if ( m_i4IsFocused == 1) NewState = E_AF_FOCUSED_LOCKED;
                else                  NewState = E_AF_NOT_FOCUSED_LOCKED;

                //lock
                LockAlgo();
                m_bNeedLock = MFALSE;

            }
            else if ( eInCurSate == E_AF_PASSIVE_SCAN)
            {
                if ( m_i4IsFocused == 1) NewState = E_AF_PASSIVE_FOCUSED;
                else                  NewState = E_AF_PASSIVE_UNFOCUSED;
            }
            else
            {
                NewState = eInCurSate;

                if ( sInEvent == EVENT_CMD_STOP && eInCurSate == E_AF_INACTIVE)
                {
                    // force doing one time searching when next time start preview.
                    m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = MTRUE;
                    CAM_LOGD( "Do one time searching when next time start preview.");
                }
            }
        }
        break;

    case EVENT_SEARCHING_DONE_RESET_PARA:
        NewState = eInCurSate;
        if ( m_bTriggerCmdVlid == MTRUE && m_bWaitForceTrigger == MTRUE)
        {
            /* For capture with flash */
            CAM_LOGD( "hybrid AF searching durig capture state");
            /*    _________            _____________
                           |__________|                     First time searching with PD doesn't change state
                  ___________________            ________
                                     |__________|           capture flow during first time searching with PD
             */
        }
        else
        {
            //reset parameters
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = MFALSE;
        }
        break;

    case EVENT_SET_WAIT_FORCE_TRIGGER :
        if ( m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO)
        {
            CAM_LOGW("EVENT_SET_WAIT_FORCE_TRIGGER in MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO");
        }
        else if ( eInCurSate != E_AF_FOCUSED_LOCKED && eInCurSate != E_AF_NOT_FOCUSED_LOCKED)
        {
            m_bTriggerCmdVlid = m_bForceTrigger = m_bWaitForceTrigger = MTRUE;
            CAM_LOGD( "Set wait force trigger for preCap");
            if ( m_i4IsAFSearch_CurState == AF_SEARCH_DONE)
            {
                LockAlgo();
            }
            // send callback for the next two times
            if (m_sCallbackInfo.isAfterAutoMode == 1)
            {
                m_sCallbackInfo.isAfterAutoMode = 2;
            }
        }

        NewState = eInCurSate;
        break;

    default : /*Should not be happened*/
        NewState = eInCurSate;
        break;
    }

    return NewState;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateOFFMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        NewState = E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        m_bNeedCheckSendCallback = MFALSE;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            UnlockAlgo();
        }
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        if (m_pIAfAlgo)
            m_pIAfAlgo->trigger();
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        NewState = eInCurSate;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL :
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    // intentionally fall through
    case EVENT_CMD_SET_AF_REGION :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_START :
    case EVENT_SEARCHING_END :
    case EVENT_AE_IS_STABLE :
    default :
        m_sCallbackInfo.isAfterAutoMode = 0;
        NewState = E_AF_INACTIVE;
        break;
    }

    return NewState;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateMFMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        m_bNeedCheckSendCallback = MFALSE;
        NewState = E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            UnlockAlgo();
        }
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        if (m_pIAfAlgo)
            m_pIAfAlgo->trigger();
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        NewState = eInCurSate;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL :
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    // intentionally fall through
    case EVENT_CMD_SET_AF_REGION :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_START :
    case EVENT_SEARCHING_END :
    case EVENT_AE_IS_STABLE :
    default :
        m_sCallbackInfo.isAfterAutoMode = 0;
        NewState = E_AF_INACTIVE;
        break;
    }

    return NewState;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateState( AF_EVENT_T sInEvent)
{
    Mutex::Autolock lock( m_AFStateLock);

    E_AF_STATE_T NewState;

    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_CHANGE_MODE", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_CHANGE_MODE;
        break;
    case EVENT_CMD_AUTOFOCUS:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_AUTOFOCUS", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_AUTOFOCUS;
        break;
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_TRIGGERAF_WITH_AE_STBL", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_TRIGGERAF_WITH_AE_STBL;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_AUTOFOCUS_CANCEL", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_AUTOFOCUS_CANCEL;
        break;
    case EVENT_CMD_SET_AF_REGION:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_SET_AF_REGION", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_SET_AF_REGION;
        break;
    case EVENT_CMD_STOP:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_STOP", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_STOP;
        break;
    case EVENT_SEARCHING_START:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SEARCHING_START", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_START;
        break;
    case EVENT_SEARCHING_END:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SEARCHING_END", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_END;
        break;
    case EVENT_SEARCHING_DONE_RESET_PARA:
        m_eEvent |= EVENT_SEARCHING_DONE_RESET_PARA;
        break;
    case EVENT_AE_IS_STABLE:
        //CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_AE_IS_STABLE", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_AE_IS_STABLE;
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SET_WAIT_FORCE_TRIGGER", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SET_WAIT_FORCE_TRIGGER;
        break;
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CANCEL_WAIT_FORCE_TRIGGER", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CANCEL_WAIT_FORCE_TRIGGER;
        break;
    default:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_WRONG", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        break;
    }

    switch ( m_eCurAFMode)
    {
    case MTK_CONTROL_AF_MODE_OFF :
        NewState = UpdateStateMFMode( m_eAFState, sInEvent);
        break;
    case MTK_CONTROL_AF_MODE_AUTO :
    case MTK_CONTROL_AF_MODE_MACRO :
        NewState = UpdateStateAutoMode( m_eAFState, sInEvent);
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO :
    case MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE :
        NewState = UpdateStateContinuousMode( m_eAFState, sInEvent);
        break;
    case MTK_CONTROL_AF_MODE_EDOF :
    default :
        NewState = UpdateStateOFFMode( m_eAFState, sInEvent);
        break;
    }

    if ( m_eAFState != NewState)
    {
        CAM_LOGD( "#(%5d,%5d) %s  Dev(%d) : %d->%d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_eAFState,
                  NewState);
    }

    m_eAFState = NewState;

    return NewState;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::getAFState()
{
    E_AF_STATE_T ret;

    if ( m_u4ReqMagicNum <= AF_START_MAGIC_NUMBER && m_eAFState == E_AF_PASSIVE_SCAN)
    {
        /**
         * If AF starts searching at first request.
         * Force to return inactive state as initial state.
         */
        ret = E_AF_INACTIVE;
    }
    else
    {
        ret = m_eAFState;
    }

    MINT32 IsAFSuspend = ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsAFSuspend, NULL, NULL);

    if (IsAFSuspend != MFALSE && ret == E_AF_PASSIVE_SCAN)
    {
        if (m_bNeedLock)
        {
            ret = E_AF_FOCUSED_LOCKED;
        }
        else
        {
            ret = E_AF_PASSIVE_FOCUSED;
        }
        if ( m_eAFStatePre != ret)
        {
            CAM_LOGD("cmd-%s Dev %d : ret = %d, CCU Supend",
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     ret);
        }
    }
    else if (AP_NOT_SHOW_FDROI_AUTO && m_i4IsCAFWithoutFace == 1 && ret == NS3A::E_AF_PASSIVE_SCAN && m_sAFOutput.i4ROISel == AF_ROI_SEL_FD)
    {
        // For AP which WOULD NOT show Face ROI automatically while CAFing
        // faces show up while CAFing
        // overwrite AfState for MW to send callback
        if (m_bNeedLock)
        {
            ret = E_AF_FOCUSED_LOCKED;
        }
        else
        {
            ret = E_AF_PASSIVE_FOCUSED;
        }
        if ( m_eAFStatePre != ret)
        {
            CAM_LOGD("cmd-%s Dev %d : ret = %d, Face shows up while CAFing => Send callback to clear CAF ROI",
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     ret);
        }
    }
    else
    {
        if ( m_eAFStatePre != ret)
        {
            CAM_LOGD( "cmd-%s Dev %d : %d->%d",
                      __FUNCTION__,
                      m_i4CurrSensorDev,
                      m_eAFStatePre,
                      ret);
        }
    }


    m_eAFStatePre = ret;


    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getFocusAreaResult( android::Vector<MINT32> &vecOutPos, android::Vector<MUINT8> &vecOutRes, MINT32 &i4OutSzW, MINT32 &i4OutSzH)
{
    // the FocusArea would be updated to the HalMeta
    // FocusArea: (X,Y,W,H) -> (L,T,R,B)
    // (X,Y) = Center point of the ROI

    vecOutPos.clear();
    vecOutRes.clear();
    i4OutSzW = 0;
    i4OutSzH = 0;

    if ( m_bMZAFEn)
    {
        MUINT32 num = static_cast<MUINT32>(m_sAFOutput.sROIStatus.i4TotalNum);
        if ( num <= MAX_MULTI_ZONE_WIN_NUM)
        {
            MINT32 x, y;
            MUINT8 res;
            for ( MUINT32 i = 0; i < num; i++)
            {
                x   = m_sAFOutput.sROIStatus.sROI[i].i4X + m_sAFOutput.sROIStatus.sROI[i].i4W / 2;
                y   = m_sAFOutput.sROIStatus.sROI[i].i4Y + m_sAFOutput.sROIStatus.sROI[i].i4H / 2;
                res = m_sAFOutput.sROIStatus.sROI[i].i4Info;

                /*The order of vector vecOutPos is (x1)->(y1)->(x2)->(y2)*/
                vecOutPos.push_back(x);
                vecOutPos.push_back(y);
                vecOutRes.push_back(res);

                CAM_LOGD_IF( m_i4DgbLogLv,
                             "%s [%d] X(%4d), Y(%4d), W(%4d), H(%4d), Res(%d)",
                             __FUNCTION__,
                             i,
                             m_sAFOutput.sROIStatus.sROI[i].i4X,
                             m_sAFOutput.sROIStatus.sROI[i].i4Y,
                             m_sAFOutput.sROIStatus.sROI[i].i4W,
                             m_sAFOutput.sROIStatus.sROI[i].i4H,
                             m_sAFOutput.sROIStatus.sROI[i].i4Info);
            }
            /* All W and H should be the same*/
            i4OutSzW = m_sAFOutput.sROIStatus.sROI[0].i4W;
            i4OutSzH = m_sAFOutput.sROIStatus.sROI[0].i4H;
        }
    }

    CAM_LOGD_IF( m_i4DgbLogLv & 4,
                 "%s %zu %zu",
                 __FUNCTION__,
                 vecOutPos.size(),
                 vecOutRes.size());

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getFocusArea( android::Vector<MINT32> &vecOut)
{
    // the FocusArea would be updated to the AppMeta
    // FocusArea: (X,Y,W,H) -> (L,T,R,B)
    // (X,Y) = LeftTop point of the ROI

    vecOut.clear();

    /*
     * The order of vector vecOut is :
     * (type)->(number of ROI)->(left_1)->(top_1)->(right_1)->(bottom_1)->(result_1)->(left_2)->(top_2)->(right_2)->(bottom_2)...
     */


    if ( m_bMZAFEn)
    {
        MINT32 type = 0;
        vecOut.push_back(type);

        MUINT32 num = static_cast<MUINT32>(m_sAFOutput.sROIStatus.i4TotalNum);
        if ( (MAX_MULTI_ZONE_WIN_NUM < num))
        {
            num = 0;
        }
        vecOut.push_back(num);

        for ( MUINT32 i = 0; i < num; i++)
        {
            vecOut.push_back( m_sAFOutput.sROIStatus.sROI[i].i4X);
            vecOut.push_back( m_sAFOutput.sROIStatus.sROI[i].i4Y);
            vecOut.push_back((m_sAFOutput.sROIStatus.sROI[i].i4X + m_sAFOutput.sROIStatus.sROI[i].i4W));
            vecOut.push_back((m_sAFOutput.sROIStatus.sROI[i].i4Y + m_sAFOutput.sROIStatus.sROI[i].i4H));
            vecOut.push_back( m_sAFOutput.sROIStatus.sROI[i].i4Info);

            CAM_LOGD_IF( m_i4DgbLogLv,
                         "%s [%d] X(%4d), Y(%4d), W(%4d), H(%4d), Res(%d)",
                         __FUNCTION__,
                         i,
                         m_sAFOutput.sROIStatus.sROI[i].i4X,
                         m_sAFOutput.sROIStatus.sROI[i].i4Y,
                         m_sAFOutput.sROIStatus.sROI[i].i4W,
                         m_sAFOutput.sROIStatus.sROI[i].i4H,
                         m_sAFOutput.sROIStatus.sROI[i].i4Info);
        }

    }
    else
    {
        MINT32 type = 0;
        vecOut.push_back(type);

        AREA_T AreaOnUI;
        MUINT32 TwinStatus = 0;

        // check if TwinMode on
        MINT32 num  = 1;
        MINT32 DbgTwinMode = (m_i4DgbLogLv) ? property_get_int32("debug.af_twin.enable", 0) : 0;
        if (DbgTwinMode)
        {
            IspMgrAFGetROIFromHw(AreaOnUI, TwinStatus);
            CAM_LOGD_IF( m_i4DgbLogLv,
                         "%s isTwin(%d), ROI from CAM_A Reg X(%4d), Y(%4d), W(%4d), H(%4d), Res(%d)",
                         __FUNCTION__,
                         TwinStatus,
                         AreaOnUI.i4X,
                         AreaOnUI.i4Y,
                         AreaOnUI.i4W,
                         AreaOnUI.i4H,
                         m_sArea_TypeSel);

            num = (TwinStatus == 0) ? 1 : 2;
        }

        vecOut.push_back(num);
        vecOut.push_back( m_sArea_HW.i4X);
        vecOut.push_back( m_sArea_HW.i4Y);
        vecOut.push_back((m_sArea_HW.i4X + m_sArea_HW.i4W));
        vecOut.push_back((m_sArea_HW.i4Y + m_sArea_HW.i4H));
        vecOut.push_back( m_sArea_TypeSel);
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "%s ROI X(%4d), Y(%4d), W(%4d), H(%4d), Res(%d)",
                     __FUNCTION__,
                     m_sArea_HW.i4X,
                     m_sArea_HW.i4Y,
                     m_sArea_HW.i4W,
                     m_sArea_HW.i4H,
                     m_sArea_TypeSel);
        if (TwinStatus && DbgTwinMode)
        {
            vecOut.push_back( AreaOnUI.i4X);
            vecOut.push_back( AreaOnUI.i4Y);
            vecOut.push_back((AreaOnUI.i4X + AreaOnUI.i4W));
            vecOut.push_back((AreaOnUI.i4Y + AreaOnUI.i4H));
            vecOut.push_back( m_sArea_TypeSel);
        }
    }
    return S_AF_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFArea( CameraFocusArea_T &sInAFArea)
{
    /* sInAFArea is TG base coordinate */

    MRESULT ret = E_3A_ERR;

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if ( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) cmd-%s : [Cnt]%d [L]%d [R]%d [U]%d [B]%d, ",
                              m_u4ReqMagicNum,
                              m_u4StaMagicNum,
                              __FUNCTION__,
                              sInAFArea.u4Count,
                              sInAFArea.rAreas[0].i4Left,
                              sInAFArea.rAreas[0].i4Right,
                              sInAFArea.rAreas[0].i4Top,
                              sInAFArea.rAreas[0].i4Bottom);
    }

    if ( 1 <= sInAFArea.u4Count)
    {
        if ( // boundary check.
            (sInAFArea.rAreas[0].i4Left < sInAFArea.rAreas[0].i4Right ) &&
            (sInAFArea.rAreas[0].i4Top  < sInAFArea.rAreas[0].i4Bottom) &&
            (0 <= sInAFArea.rAreas[0].i4Left && sInAFArea.rAreas[0].i4Right <= (MINT32)m_i4TGSzW) &&
            (0 <= sInAFArea.rAreas[0].i4Top  && sInAFArea.rAreas[0].i4Bottom <= (MINT32)m_i4TGSzH))
        {
            // To remain the center
            MINT32 InWidth  = sInAFArea.rAreas[0].i4Right - sInAFArea.rAreas[0].i4Left;
            MINT32 InHeight = sInAFArea.rAreas[0].i4Bottom - sInAFArea.rAreas[0].i4Top;
            MINT32 Center_X = sInAFArea.rAreas[0].i4Left + InWidth / 2;
            MINT32 Center_Y = sInAFArea.rAreas[0].i4Top + InHeight / 2;

            MINT32 Percent_TouchAFROI = 100;
            // To resize the Touch AF ROI if it is required from users.
            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[3] > 0 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[3] <= 200)
            {
                Percent_TouchAFROI = m_ptrNVRam->rAFNVRAM.i4EasyTuning[3];
            }
            // To override the Percent_TouchAFROI based on ISO target
            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[5] > 0 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[5] <= 200 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[4] > 0)
            {
                if (m_sAFInput.i4ISO > m_ptrNVRam->rAFNVRAM.i4EasyTuning[4])
                {
                    Percent_TouchAFROI = m_ptrNVRam->rAFNVRAM.i4EasyTuning[5];
                }
            }
            // To resize the Touch AF ROI if it is required from users.
            if (Percent_TouchAFROI != 100)
            {
                InWidth =  InWidth * Percent_TouchAFROI / 100.0;
                InHeight = InHeight * Percent_TouchAFROI / 100.0;
                // To resize the ROI from tuning parameter
                MINT32 TempLeft   = Center_X - (InWidth / 2);
                MINT32 TempRight  = TempLeft + InWidth;
                MINT32 TempTop    = Center_Y - (InHeight / 2);
                MINT32 TempBottom = TempTop  + InHeight;
                // Left Boundary Check
                if (TempLeft < 0)
                {
                    TempLeft  = 0;
                    TempRight = InWidth;
                }
                // Right Boundary Check
                if (TempRight >= (MINT32)m_i4TGSzW)
                {
                    TempRight = (m_i4TGSzW - 1);
                    TempLeft  = TempRight - InWidth;
                }
                // Top Boundary Check
                if (TempTop < 0)
                {
                    TempTop    = 0;
                    TempBottom = InHeight;
                }
                // Bottom Boundary Check
                if (TempBottom >= (MINT32)m_i4TGSzH)
                {
                    TempBottom = (m_i4TGSzH - 1);
                    TempTop    = TempBottom - InHeight;
                }
                CAM_LOGD_IF(m_i4DgbLogLv,
                            "#(%5d,%5d) cmd-%s InAFArea is resized from users' requirement, Resize Percent(%d), ROI(L,R,U,B): Ori(%d,%d,%d,%d)->Rescale(%d,%d,%d,%d)",
                            m_u4ReqMagicNum,
                            m_u4StaMagicNum,
                            __FUNCTION__,
                            Percent_TouchAFROI,
                            sInAFArea.rAreas[0].i4Left,
                            sInAFArea.rAreas[0].i4Right,
                            sInAFArea.rAreas[0].i4Top,
                            sInAFArea.rAreas[0].i4Bottom,
                            TempLeft, TempRight, TempTop, TempBottom );
                // To update the resized touch AF ROI
                sInAFArea.rAreas[0].i4Left   = TempLeft;
                sInAFArea.rAreas[0].i4Right  = TempRight;
                sInAFArea.rAreas[0].i4Top    = TempTop;
                sInAFArea.rAreas[0].i4Bottom = TempBottom;
            }
            /*
            // If InAFArea.W < ROIWidth || InAFArea.H < ROIWidth, resize the AreaSize
            // Resize rule : the rectangular centered by the touch position
            //  - Center = InAFArea.Center
            //  - Width = ShortLine * PERCENT_AFROI / 100.0;
            */
            MINT32 Percent_AFROI = 10; // default : 20%
            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[1] >= 1 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[1] <= 100)
            {
                Percent_AFROI = m_ptrNVRam->rAFNVRAM.i4EasyTuning[1];
            }
            MDOUBLE ShortLine = min(m_i4TGSzW, m_i4TGSzH);
            MINT32 ROIWidth  = (MINT32)(ShortLine * Percent_AFROI / 100.0);
            if (InWidth < ROIWidth || InHeight < ROIWidth)
            {
                // To resize the rectangular
                MINT32 TempLeft   = Center_X - ROIWidth / 2;
                MINT32 TempRight  = TempLeft + ROIWidth;
                MINT32 TempTop    = Center_Y - ROIWidth / 2;
                MINT32 TempBottom = TempTop  + ROIWidth;
                // Left Boundary Check
                if (TempLeft < 0)
                {
                    TempLeft  = 0;
                    TempRight = ROIWidth;
                }
                // Right Boundary Check
                if (TempRight >= (MINT32)m_i4TGSzW)
                {
                    TempRight = m_i4TGSzW - 1;
                    TempLeft  = TempRight - ROIWidth;
                }
                // Top Boundary Check
                if (TempTop < 0)
                {
                    TempTop    = 0;
                    TempBottom = ROIWidth;
                }
                // Bottom Boundary Check
                if (TempBottom >= (MINT32)m_i4TGSzH)
                {
                    TempBottom = (m_i4TGSzH - 1);
                    TempTop    = TempBottom - ROIWidth;
                }
                CAM_LOGD("#(%5d,%5d) cmd-%s InAFArea is too small, MinROIWidth(%d), Ori(%d,%d,%d,%d)->rescale(%d,%d,%d,%d)",
                         m_u4ReqMagicNum,
                         m_u4StaMagicNum,
                         __FUNCTION__,
                         ROIWidth,
                         sInAFArea.rAreas[0].i4Left,
                         sInAFArea.rAreas[0].i4Right,
                         sInAFArea.rAreas[0].i4Top,
                         sInAFArea.rAreas[0].i4Bottom,
                         TempLeft, TempRight, TempTop, TempBottom );
                // update the InAFArea
                sInAFArea.rAreas[0].i4Left   = TempLeft;
                sInAFArea.rAreas[0].i4Right  = TempRight;
                sInAFArea.rAreas[0].i4Top    = TempTop;
                sInAFArea.rAreas[0].i4Bottom = TempBottom;
            }

            AREA_T roi = AREA_T( sInAFArea.rAreas[0].i4Left,
                                 sInAFArea.rAreas[0].i4Top,
                                 sInAFArea.rAreas[0].i4Right  - sInAFArea.rAreas[0].i4Left,
                                 sInAFArea.rAreas[0].i4Bottom - sInAFArea.rAreas[0].i4Top,
                                 AF_MARK_NONE);

            m_sArea_APForContiMode = roi;
            m_sArea_APValid = 1;

            if ( memcmp( &roi, &m_sArea_APCheck, sizeof(AREA_T)) != 0)
            {
                //store command.
                m_sArea_APCheck = m_sArea_APCmd = roi;

                m_sPDRois[eIDX_ROI_ARRAY_AP].info.sType  = ROI_TYPE_AP;
                m_sPDRois[eIDX_ROI_ARRAY_AP].info.sPDROI = m_sArea_APCmd;

                UpdateState( EVENT_CMD_SET_AF_REGION);

                CAM_LOGD( "#(%5d,%5d) cmd-%s Got ROI changed cmd. [Cnt]%d (L,R,U,B)=(%d,%d,%d,%d) => (X,Y,W,H)=(%d,%d,%d,%d)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          sInAFArea.u4Count,
                          sInAFArea.rAreas[0].i4Left,
                          sInAFArea.rAreas[0].i4Right,
                          sInAFArea.rAreas[0].i4Top,
                          sInAFArea.rAreas[0].i4Bottom,
                          m_sArea_APCmd.i4X,
                          m_sArea_APCmd.i4Y,
                          m_sArea_APCmd.i4W,
                          m_sArea_APCmd.i4H);

                // control laser AF touch behavior.
                if (m_bLDAFEn == MTRUE)
                {
                    MINT32 Centr_X = m_sCropRegionInfo.i4X + (m_sCropRegionInfo.i4W / 2);
                    MINT32 Centr_Y = m_sCropRegionInfo.i4Y + (m_sCropRegionInfo.i4H / 2);
                    MINT32 Touch_X = (sInAFArea.rAreas[0].i4Left + sInAFArea.rAreas[0].i4Right) / 2;
                    MINT32 Touch_Y = (sInAFArea.rAreas[0].i4Top  + sInAFArea.rAreas[0].i4Bottom) / 2;

                    m_sAFInput.sLaserInfo.i4AfWinPosX = abs(Centr_X - Touch_X);
                    m_sAFInput.sLaserInfo.i4AfWinPosY = abs(Centr_Y - Touch_Y);

                    if ((m_sAFInput.sLaserInfo.i4AfWinPosX < LASER_TOUCH_REGION_W) && (m_sAFInput.sLaserInfo.i4AfWinPosY < LASER_TOUCH_REGION_H))
                    {
                        if ((m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO))
                        {
                            m_sAFInput.sLaserInfo.i4AfWinPosCnt++;
                        }
                    }
                    else
                    {
                        m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;
                    }
                }

                ret = S_AF_OK;
            }
            else
            {
                if ( m_i4DgbLogLv)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cmd is same");
                }
            }
        }
        else
        {
            memset( &m_sArea_APForContiMode, 0, sizeof(AREA_T));
            m_sArea_APValid = 0;

            //command is not valid, using center window.
            if ( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cmd is not correct");
            }
        }
    }
    else
    {
        memset( &m_sArea_APForContiMode, 0, sizeof(AREA_T));
        m_sArea_APValid = 0;

        //command is not valid, using center window.
        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cnt=0!!");
        }
    }

    if ( ret == E_3A_ERR)
    {
        CAM_LOGD_IF( m_i4DgbLogLv & 2, "%s", dbgMsgBuf);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCamScenarioMode(MUINT32 a_eCamScenarioMode __unused)
{
#if 0 // no use in New NVRAM design (ISP50)
    /**********************************************************
     *
     * This function should be called before af mgr is started!!
     *
     **********************************************************/
    // [change mode] or [unvalid mode]
    if ( (a_eCamScenarioMode != m_eCamScenarioMode) || (m_eCamScenarioMode >= AF_CAM_SCENARIO_NUM))
    {
        // if original CamScenarioMode is unvalid, reset it to CAM_SCENARIO_PREVIEW
        if (m_eCamScenarioMode >= AF_CAM_SCENARIO_NUM)
        {
            m_eCamScenarioMode = CAM_SCENARIO_PREVIEW;
        }
        CAM_LOGD( "#(%5d,%5d) %s %d -> %d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_eCamScenarioMode,
                  a_eCamScenarioMode);

        if ( a_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
        {
            if ( m_ptrLensNVRam != NULL)
            {
                /*************************************************************************************************
                 *
                 * m_ptrLensNVRam is initialized in af mgr start function.
                 * m_ptrLensNVRam!=NULL means AF NVRAM data is valid, so NVRAM data should be updated in algroithm.
                 *
                 *************************************************************************************************/
                if (AFNVRAMMapping[a_eCamScenarioMode] != AFNVRAMMapping[m_eCamScenarioMode])
                {
                    MUINT32 u4AFNVRAMIdx = AFNVRAMMapping[a_eCamScenarioMode];

                    m_ptrNVRam = &(m_ptrLensNVRam->rLENSNVRAM[u4AFNVRAMIdx]);

                    CAM_LOGD_IF( m_i4DgbLogLv,
                                 "#(%5d,%5d) %s [nvram][Offset]%d [Normal Num]%d [Macro Num]%d",
                                 m_u4ReqMagicNum,
                                 m_u4StaMagicNum,
                                 __FUNCTION__,
                                 m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset,
                                 m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum,
                                 m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);

                    // LaunchCamTrigger
                    if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] > 0) // the timeout of AEStable-wait-Face is not default value
                    {
                        m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10];
                    }
                    if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] > 0) // the timeout of ValidPD-wait-Face is not default value
                    {
                        m_i4ValidPDTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11];
                    }

                    // checkSendCallback info : update the target form NVRAM
                    m_sCallbackInfo.CompSet_PDCL.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[20];
                    m_sCallbackInfo.CompSet_FPS.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[21];
                    m_sCallbackInfo.CompSet_ISO.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[22];

                    if ( m_pIAfAlgo)
                    {
                        AF_CONFIG_T const *ptrHWCfgDef = NULL;
                        getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
                        m_pIAfAlgo->setAFParam( (*m_pAFParam), (*ptrHWCfgDef), m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);
                        // initial hybrid AF algorithm
                        m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

                        if ( m_bLock != MTRUE)
                        {
                            m_pIAfAlgo->cancel();
                        }
                    }
                    // Get AF calibration data. Should do this after setAFParam is called.
                    if ( m_ptrNVRam->rAFNVRAM.i4ReadOTP == TRUE)
                    {
                        readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
                    }
                }
                else
                {
                    CAM_LOGD( "#(%5d,%5d) %s] tuning param is the same, no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__);
                }
            }
            else
            {
                CAM_LOGD( "#(%5d,%5d) %s NVRAM is not initialized, update m_eCamScenarioMode only, no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__);
            }

            //
            m_eCamScenarioMode = a_eCamScenarioMode;
        }
        else
        {
            CAM_LOGD( "#(%5d,%5d) %s Camera Scenario mode invalid (%d), no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, a_eCamScenarioMode);
        }

    }
#endif
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setNVRAMIndex(MUINT32 a_eNVRAMIndex)
{
    /**********************************************************
     *
     * This function should be called before af mgr is started!!
     *
     **********************************************************/
    // [change mode] or [unvalid mode]
    if ( (a_eNVRAMIndex != m_eNVRAMIndex) || (m_eNVRAMIndex >= AF_CAM_SCENARIO_NUM_2))
    {
        // if original CamScenarioMode is unvalid, reset it to CAM_SCENARIO_PREVIEW
        if (m_eNVRAMIndex >= AF_CAM_SCENARIO_NUM_2)
        {
            m_eNVRAMIndex = 0;
        }
        CAM_LOGD( "#(%5d,%5d) Dev(0x%04x), %s %d -> %d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  m_i4CurrSensorDev,
                  __FUNCTION__,
                  m_eNVRAMIndex,
                  a_eNVRAMIndex);

        if ( a_eNVRAMIndex < AF_CAM_SCENARIO_NUM_2)
        {
            if ( m_ptrLensNVRam != NULL)
            {
                /*************************************************************************************************
                 *
                 * m_ptrLensNVRam is initialized in af mgr start function.
                 * m_ptrLensNVRam!=NULL means AF NVRAM data is valid, so NVRAM data should be updated in algroithm.
                 *
                 *************************************************************************************************/
                m_ptrNVRam = &(m_ptrLensNVRam->AF[a_eNVRAMIndex]);
                CAM_LOGD_IF( m_i4DgbLogLv,
                             "#(%5d,%5d) %s [Path]%s [nvram][Offset]%d [Normal Num]%d [Macro Num]%d",
                             m_u4ReqMagicNum,
                             m_u4StaMagicNum,
                             __FUNCTION__,
                             m_ptrNVRam->rFilePath,
                             m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset,
                             m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum,
                             m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);

                // LaunchCamTrigger
                if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] > 0) // the timeout of AEStable-wait-Face is not default value
                {
                    m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10];
                }
                if(m_ptrNVRam->rAFNVRAM.i4EasyTuning[11]>0)  // the timeout of ValidPD-wait-Face is not default value
                {
                    m_i4ValidPDTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11];
                }

                // checkSendCallback info : update the target form NVRAM
                m_sCallbackInfo.CompSet_PDCL.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[20];
                m_sCallbackInfo.CompSet_ISO.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[22];
                m_sCallbackInfo.CompSet_FPS.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[21];

                if ( m_pIAfAlgo)
                {
                    AF_CONFIG_T const *ptrHWCfgDef = NULL;
                    getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
                    m_pIAfAlgo->setAFParam( (*m_pAFParam), (*ptrHWCfgDef), m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);
                    // initial hybrid AF algorithm
                    m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

                    if ( m_bLock != MTRUE)
                    {
                        m_pIAfAlgo->cancel();
                    }
                }
                // Get AF calibration data. Should do this after setAFParam is called.
                if ( m_ptrNVRam->rAFNVRAM.i4ReadOTP == TRUE)
                {
                    readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
                }
            }
            else
            {
                CAM_LOGD( "#(%5d,%5d) %s NVRAM is not initialized, update m_eCamScenarioMode only, no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__);
            }

            //
            MRESULT ret = IPDMgr::getInstance().setNVRAMIndex(m_i4CurrSensorDev, a_eNVRAMIndex);

            //
            m_eNVRAMIndex = a_eNVRAMIndex;
        }
        else
        {
            CAM_LOGD( "#(%5d,%5d) %s Camera NVRAM index invalid (%d), no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, a_eNVRAMIndex);
        }

    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetCropRegionInfo( MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height, MUINT32 u4Caller, MUINT32 u4ForceUpdateCenterROI)
{
    if ( u4Width == 0 || u4Height == 0 )
    {
        return S_AF_OK;
    }

    /* Digital Zoom : skip crop reggion info durning AF seraching */
    if ( m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
    {
        return S_AF_OK;
    }

    if ( (u4XOffset < m_i4TGSzW) &&
            (u4YOffset < m_i4TGSzH) &&
            (u4XOffset + u4Width <= m_i4TGSzW) &&
            (u4YOffset + u4Height <= m_i4TGSzH) )
    {
        // set crop region information and update center ROI automatically.
        if ( m_sCropRegionInfo.i4X != (MINT32)u4XOffset ||
                m_sCropRegionInfo.i4Y != (MINT32)u4YOffset ||
                m_sCropRegionInfo.i4W != (MINT32)u4Width   ||
                m_sCropRegionInfo.i4H != (MINT32)u4Height ||
                u4ForceUpdateCenterROI == MTRUE)
        {
            m_sCropRegionInfo.i4X = (MINT32)u4XOffset;
            m_sCropRegionInfo.i4Y = (MINT32)u4YOffset;
            m_sCropRegionInfo.i4W = (MINT32)u4Width;
            m_sCropRegionInfo.i4H = (MINT32)u4Height;

            //calculate zoom information : 1X-> 100, 2X->200, ...
            MUINT32 dzfX = 100 * m_i4TGSzW / m_sCropRegionInfo.i4W;
            MUINT32 dzfY = 100 * m_i4TGSzH / m_sCropRegionInfo.i4H;
            //Should be the same.
            m_i4DZFactor = dzfX < dzfY ? dzfX : dzfY;

            if ( u4Caller == AF_MGR_CALLER)
            {
                CAM_LOGD( "#(%5d,%5d) %s (x,y,w,h)=(%d, %d, %d, %d), dzX(%d), dzY(%d), Fac(%d)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          u4XOffset,
                          u4YOffset,
                          u4Width,
                          u4Height,
                          dzfX,
                          dzfY,
                          m_i4DZFactor);
            }
            else
            {
                CAM_LOGD( "#(%5d,%5d) cmd-%s (x,y,w,h)=(%d, %d, %d, %d), dzX(%d), dzY(%d), Fac(%d)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          u4XOffset,
                          u4YOffset,
                          u4Width,
                          u4Height,
                          dzfX,
                          dzfY,
                          m_i4DZFactor);
            }

            //Accroding to crop region, updating center ROI coordinate automatically
            UpdateCenterROI( m_sArea_Center);

            //Reset all focusing window.
            m_sArea_APCheck = m_sArea_APCmd = m_sArea_OTFD = m_sArea_OTFD_No_Extend = m_sArea_HW = m_sArea_Center;
            m_sArea_TypeSel = ROI_TYPE_CENTER;

            // it is always valid for calculation center roi once crop region is set.
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].valid       = MTRUE;
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sType  = ROI_TYPE_CENTER;
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sPDROI = m_sArea_Center;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv & 2, "%s same cmd", __FUNCTION__);
        }
    }
    else
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "%s not valid", __FUNCTION__);
    }
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::ApplyZoomEffect( AREA_T &sOutAFRegion)
{
    // zoom effect of ROI is configured from customized parameter
    m_i4DzWinCfg = (MUINT32)m_ptrNVRam->rAFNVRAM.i4ZoomInWinChg;

    // error check
    if (      m_i4DzWinCfg < 1) m_i4DzWinCfg = 1;
    else if ( 4 < m_i4DzWinCfg) m_i4DzWinCfg = 4;

    // scale should be same as m_i4DZFactor
    m_i4DzWinCfg *= 100;
    CAM_LOGD_IF( m_i4DgbLogLv & 2,
                 "%s [DZ]%d, [Cfg]%d",
                 __FUNCTION__,
                 m_i4DZFactor,
                 m_i4DzWinCfg);


    if ( m_i4DzWinCfg == 400)
    {
        // i4WinCfg=4-> DigZoomFac>4, AF win align digital effect.
    }
    else if ( m_bMZAFEn == MFALSE)
    {
        /**
         * i4WinCfg = 1~3
         * i4WinCfg = 1 : DigZoomFac>1, AF win no change
         * i4WinCfg = 2 : DigZoomFac>2, AF win no change
         * i4WinCfg = 3 : DigZoomFac>3, AF win no change
         */
        if ( m_i4DzWinCfg <= m_i4DZFactor) /* fix to upper bound */
        {

            //scale up window.
            AREA_T scaledWin;
            scaledWin.i4W = sOutAFRegion.i4W * m_i4DZFactor / m_i4DzWinCfg;
            scaledWin.i4H = sOutAFRegion.i4H * m_i4DZFactor / m_i4DzWinCfg;
            scaledWin.i4X = (sOutAFRegion.i4X + sOutAFRegion.i4W / 2) -  scaledWin.i4W / 2;
            scaledWin.i4Y = (sOutAFRegion.i4Y + sOutAFRegion.i4H / 2) -  scaledWin.i4H / 2;

            CAM_LOGD_IF( m_i4DgbLogLv & 2,
                         "%s [W]%d [H]%d [X]%d [Y]%d -> [W]%d [H]%d [X]%d [Y]%d",
                         __FUNCTION__,
                         scaledWin.i4W,
                         scaledWin.i4H,
                         scaledWin.i4X,
                         scaledWin.i4Y,
                         sOutAFRegion.i4W,
                         sOutAFRegion.i4H,
                         sOutAFRegion.i4X,
                         sOutAFRegion.i4Y );

            sOutAFRegion = scaledWin;
        }
        else /* (i4DzFactor < i4WinCfg*100), AF win change aligning to digital zoom factor */
        {
            //CAM_LOGD("[applyZoomInfo] <bound2DZ, DZ=%d, Bound=%d \n",i4DzFactor,i4WinCfg*100);
        }
    }


    //CAM_LOGD("[applyZoomInfo] >bound2fix, DZ=%d, Bound=%d\n",i4DzFactor,i4WinCfg*100);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setOTFDInfo( MVOID* sInROIs, MINT32 i4Type)
{
    MRESULT ret = E_3A_ERR;

    /*TG base coordinate*/
    MtkCameraFaceMetadata *ptrWins = (MtkCameraFaceMetadata *)sInROIs;

    //prepare information and set to hybrid AF
    AREA_T OriFDArea = AREA_T( 0, 0, 0, 0, AF_MARK_NONE);

    AF_AREA_T sAreaInfo;
    sAreaInfo.i4Score  = 0;
    sAreaInfo.i4Count  = 0;
    sAreaInfo.sRect[0] = AREA_T( 0, 0, 0, 0, AF_MARK_NONE);

    if ( ptrWins != NULL)
    {
        //=== Portrait ===//
        sAreaInfo.i4PortEnable = (MINT32)(ptrWins->CNNFaces.PortEnable);
        sAreaInfo.i4IsTrueFace = (MINT32)(ptrWins->CNNFaces.IsTrueFace);
        sAreaInfo.f4CnnResult0 = (MFLOAT)(ptrWins->CNNFaces.CnnResult0);
        sAreaInfo.f4CnnResult1 = (MFLOAT)(ptrWins->CNNFaces.CnnResult1);
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "#(%5d,%5d) %s Dev(%d) Portrait ==> i4PortEnable: %d, i4IsTrueFace: %d, f4CnnResult: (%d, %d)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     sAreaInfo.i4PortEnable,
                     sAreaInfo.i4IsTrueFace,
                     (MINT32)(sAreaInfo.f4CnnResult0 * 1000),
                     (MINT32)(sAreaInfo.f4CnnResult1 * 1000));

        sAreaInfo.i4Score  = i4Type != 0 ? ptrWins->faces->score : 0; /*for algorithm to check input is FD or object, 0:face, 100:object*/
        if ( ptrWins->number_of_faces != 0)
        {
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
                    sAreaInfo.sRect[0] = OriFDArea;
                    // new FD info +
                    sAreaInfo.i4Id[0]        = ptrWins->faces[0].id;
                    sAreaInfo.i4Type[0]      = ptrWins->faces_type[0];
                    sAreaInfo.i4Motion[0][0] = ptrWins->motion[0][0];
                    sAreaInfo.i4Motion[0][1] = ptrWins->motion[0][1];
                    // fd roi is calculated once a FD region is set
                    m_sPDRois[eIDX_ROI_ARRAY_FD].valid       = MTRUE;
                    m_sPDRois[eIDX_ROI_ARRAY_FD].info.sType  = ROI_TYPE_FD;
                    m_sPDRois[eIDX_ROI_ARRAY_FD].info.sPDROI = sAreaInfo.sRect[0]; // FDROI before extended

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
                    CAM_LOGD_IF( m_i4DgbLogLv,
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
    if (m_i4IsEnableFVInFixedFocus == 0)
    {
        if(m_pIAfAlgo)
            m_pIAfAlgo->extendFDWin(sAreaInfo);
    }
    if (sAreaInfo.i4Count != 0)
    {
        CAM_LOGD( "[%s]cnt:%d, type %d, FD extend = [X]%d [Y]%d [W]%d [H]%d -> [X]%d [Y]%d [W]%d [H]%d",
                  __FUNCTION__,
                  sAreaInfo.i4Count,
                  sAreaInfo.i4Score,
                  OriFDArea.i4X, OriFDArea.i4Y, OriFDArea.i4W, OriFDArea.i4H,
                  sAreaInfo.sRect[0].i4X, sAreaInfo.sRect[0].i4Y, sAreaInfo.sRect[0].i4W, sAreaInfo.sRect[0].i4H);
    }
    // new FD info -
    // latch last valid FD information.
    m_sArea_OTFD = sAreaInfo.sRect[0];
    m_sArea_OTFD_No_Extend = OriFDArea;

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFDInfo( MVOID *a_sFaces)
{
    MRESULT ret = E_3A_ERR;

    if ( m_pIAfAlgo != NULL)
    {
        ret = setOTFDInfo( a_sFaces, 0);
    }
    else
    {
        CAM_LOGD( "[%s] Null algo ptr", __FUNCTION__);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setOTInfo( MVOID *a_sOT)
{
    MRESULT ret = E_3A_ERR;

    if ( m_pIAfAlgo != NULL)
    {
        ret = setOTFDInfo( a_sOT, 1);
    }
    else
    {
        CAM_LOGD( "[%s] Null algo ptr", __FUNCTION__);
    }

    return ret;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFMaxAreaNum()
{
    if (m_i4AFMaxAreaNum == -1)
    {
        IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();

        SensorStaticInfo rSensorStaticInfo;

        MUINT32 i4HalSensorDev = mapSensorIdxToDev(m_i4SensorIdx);

        pIHalSensorList->querySensorStaticInfo(i4HalSensorDev, &rSensorStaticInfo);

        MCUDrv *pMcuDrv = MCUDrv::getInstance(m_i4CurrSensorDev);
        m_i4AFMaxAreaNum = pMcuDrv->isLensSupport(rSensorStaticInfo.sensorDevID) == 0 ? 0 : AF_WIN_NUM_SPOT;

        CAM_LOGD( "[%s] (%d)", __FUNCTION__, m_i4AFMaxAreaNum);

        if (property_get_int32("vendor.debug.af_motor.fixedfocus", 0) > 0)
        {
            m_i4AFMaxAreaNum = 0;
            CAM_LOGD( "[%s] (%d) by ADB", __FUNCTION__, m_i4AFMaxAreaNum);
        }
    }

    return (m_i4AFMaxAreaNum > 0) ? AF_WIN_NUM_SPOT : 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isAFSupport()
{
    MINT32 isAFSupport = getAFMaxAreaNum();

    if (m_i4IsEnableFVInFixedFocus == -1)
    {
        if (isAFSupport == 0)
        {
            m_i4IsEnableFVInFixedFocus = ForceEnableFVInFixedFocus(m_i4CurrSensorDev);

            MINT32 type = property_get_int32("vendor.debug.af_fv.switch", 0);

            if (m_i4IsEnableFVInFixedFocus == 0 && type == 1)
                m_i4IsEnableFVInFixedFocus = 1;

            if (m_i4IsEnableFVInFixedFocus == 1 && type == 2)
                m_i4IsEnableFVInFixedFocus = 0;
        }
        else
        {
            m_i4IsEnableFVInFixedFocus = 0;
        }
    }

    isAFSupport |= m_i4IsEnableFVInFixedFocus;

    return isAFSupport;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMaxLensPos()
{
    MINT32 maxLensPos = 0;

    if ( m_i4EnableAF == 0)
    {
        maxLensPos = 0;
    }
    else
    {
        if ( m_pMcuDrv)
        {
            mcuMotorInfo MotorInfo;
            m_pMcuDrv->getMCUInfo( &MotorInfo);
            maxLensPos = (MINT32)MotorInfo.u4MacroPosition;
        }
        else
        {
            maxLensPos = 1023;
        }
    }

    CAM_LOGD_IF( m_i4DgbLogLv, "[%s] EnableAF(%d), MCU (%p), MaxLensPos(%d)", __FUNCTION__, m_i4EnableAF, (void*)m_pMcuDrv, maxLensPos);

    return maxLensPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMinLensPos()
{
    MINT32 minLensPos = 0;

    if ( m_i4EnableAF == 0)
    {
        minLensPos = 0;
    }
    else
    {
        if ( m_pMcuDrv)
        {
            mcuMotorInfo MotorInfo;
            m_pMcuDrv->getMCUInfo( &MotorInfo);
            minLensPos = (MINT32)MotorInfo.u4InfPosition;
        }
        else
        {
            minLensPos = 0;
        }
    }

    CAM_LOGD_IF( m_i4DgbLogLv, "[%s] EnableAF(%d), MCU (%p), MinLensPos(%d)", __FUNCTION__, m_i4EnableAF, (void*)m_pMcuDrv, minLensPos);

    return minLensPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFBestPos()
{
    return m_sAFOutput.i4AFBestPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFPos()
{
    return m_sAFOutput.i4AFPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFStable()
{
    return (m_sAFOutput.i4IsAfSearch == AF_SEARCH_DONE) ? 1 : 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getLensMoving()
{
    return m_sAFInput.sLensInfo.bIsMotorMoving;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableOffset()
{
    return m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableMacroIdx()
{
    return m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableIdxNum()
{
    return AF_TABLE_NUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* AfMgr::getAFTable()
{
    return (MVOID*)m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setMFPos( MINT32 a_i4Pos, MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
    {
        CAM_LOGD( "%s Dev %d : %d",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  a_i4Pos);
    }
    else
    {
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "cmd-%s Dev %d : %d",
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     a_i4Pos);
    }



    if ( (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
            (m_i4MFPos != a_i4Pos) &&
            (0 <= a_i4Pos) &&
            (a_i4Pos <= 1023))
    {
        if ( u4Caller == AF_MGR_CALLER)
        {
            CAM_LOGD( "%s set MF pos (%d)->(%d)",
                      __FUNCTION__,
                      m_sAFOutput.i4AFPos,
                      a_i4Pos);
        }
        else
        {
            CAM_LOGD( "cmd-%s set MF pos (%d)->(%d)",
                      __FUNCTION__,
                      m_sAFOutput.i4AFPos,
                      a_i4Pos);
        }

        m_i4MFPos = a_i4Pos;

        if ( m_pIAfAlgo)
        {
            m_pIAfAlgo->setMFPos( m_i4MFPos);
            m_pIAfAlgo->trigger();
        }
        else
        {
            CAM_LOGD( "Null m_pIAfAlgo");
        }
    }
    else
    {
        if ( u4Caller == AF_MGR_CALLER)
        {
            CAM_LOGD( "%s skip set MF pos (%d)->(%d), lib_afmode(%d)",
                      __FUNCTION__,
                      m_sAFOutput.i4AFPos,
                      a_i4Pos,
                      m_eLIB3A_AFMode);
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv,
                         "cmd-%s skip set MF pos (%d)->(%d), lib_afmode(%d)",
                         __FUNCTION__,
                         m_sAFOutput.i4AFPos,
                         a_i4Pos,
                         m_eLIB3A_AFMode);
        }
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFullScanstep( MINT32 a_i4Step)
{
    //if (m_i4FullScanStep != a_i4Step)
    {
        CAM_LOGD( "[setFullScanstep]%x", a_i4Step);

        m_i4FullScanStep = a_i4Step;
        m_aCurEMAFFlag = EM_AF_FLAG_NONE; // clear FullScan flag
        if ( m_i4FullScanStep > 0) /* Step > 0 , set Full Scan Mode */
        {
            MINT32 FullScanStep = property_get_int32("vendor.debug.af_fullscan.step", 0);
            if (FullScanStep > 0)
            {
                m_i4FullScanStep = (m_i4FullScanStep & 0xFFFF0000) | (FullScanStep & 0x0000FFFF);
                CAM_LOGD( "[adb prop][setFullScanstep] %x", m_i4FullScanStep);
            }
            m_bIsFullScan = MTRUE;
            m_sAFInput.i4FullScanStep = m_i4FullScanStep;

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T AfMgr::getFLKStat()
{
    AF_FULL_STAT_T sFullStat;
    memset( &sFullStat, 0, sizeof( AF_FULL_STAT_T));

    // auto detecting flk is always on
    sFullStat.bValid = MTRUE;

    CAM_LOGD_IF( (m_i4DgbLogLv & 1),
                 "[%s] FLK_bValid %d",
                 __FUNCTION__,
                 sFullStat.bValid);

    return sFullStat;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::getAFRefWin( CameraArea_T &rWinSize)
{

    //The coordinate is base on TG coordinate which is applied binning information.
    if (m_i4EnableAF == 0)
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "%s", __FUNCTION__);
        rWinSize.i4Left = 0;
        rWinSize.i4Right = 0;
        rWinSize.i4Top = 0;
        rWinSize.i4Bottom = 0;
        return;
    }
    else
    {
        rWinSize.i4Left = m_sArea_HW.i4X;
        rWinSize.i4Right = m_sArea_HW.i4X + m_sArea_HW.i4W;
        rWinSize.i4Top = m_sArea_HW.i4Y;
        rWinSize.i4Bottom = m_sArea_HW.i4Y + m_sArea_HW.i4H;

        rWinSize.i4Left = rWinSize.i4Left * m_i4BINSzW / m_i4TGSzW;
        rWinSize.i4Right = rWinSize.i4Right * m_i4BINSzW / m_i4TGSzW;
        rWinSize.i4Top = rWinSize.i4Top * m_i4BINSzH / m_i4TGSzH;
        rWinSize.i4Bottom = rWinSize.i4Bottom * m_i4BINSzH / m_i4TGSzH;
    }


}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::getAF2AEInfo(AF2AEInfo_T &rAFInfo)
{
    rAFInfo.i4IsAFDone      = (m_sAFOutput.i4IsAfSearch == AF_SEARCH_DONE) ? 1 : 0;
    rAFInfo.i4AfDac         = m_sCurLensInfo.i4CurrentPos;
    rAFInfo.i4IsSceneStable = m_sAFOutput.i4IsSceneStable;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAE2AFInfo( AE2AFInfo_T &rAEInfo)
{
    if (m_i4EnableAF == 0)
    {
        m_sAFInput.i8GSum       = 50;
        m_sAFInput.i4ISO        = 100;
        m_sAFInput.i4IsAEStable = 1;
        m_sAFInput.i4SceneLV    = 80;
        return;
    }

    m_i4isAEStable = ((rAEInfo.i4IsAEStable == 1) || (rAEInfo.i4IsAELocked == 1)) ? 1 : 0;

    m_sAFInput.i8GSum             = rAEInfo.iYvalue;
    m_sAFInput.i4ISO              = rAEInfo.i4ISO;
    m_sAFInput.i4IsAEStable       = m_i4isAEStable;
    m_sAFInput.i4SceneLV          = rAEInfo.i4SceneLV;
    m_sAFInput.i4ShutterValue     = rAEInfo.ishutterValue;
    m_sAFInput.i4IsFlashFrm       = rAEInfo.i4IsFlashFrm;
    //m_sAFInput.i4AEBlockAreaYCnt  = rAEInfo.i4AEBlockAreaYCnt;
    //m_sAFInput.pAEBlockAreaYvalue = rAEInfo.pAEBlockAreaYvalue;
    if( m_ptrNVRam->rAFNVRAM.i4SceneMonitorLevel>0)
    {
        m_pIAfAlgo->setAEBlockInfo( rAEInfo.aeBlockV, 25);
    }

    if ( m_i4isAEStable == 1)
    {
        UpdateState( EVENT_AE_IS_STABLE);
    }

    if ( m_i4DgbLogLv)
    {
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "#(%5d,%5d) cmd-%s Dev(%d) GSum(%lld) ISO(%d) AEStb(%d,%d) AELock(%d) LV(%d) shutter(%d) flash(%d) DeltaBV(%d) AEBlk(%d)(%p)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     (long long)m_sAFInput.i8GSum,
                     m_sAFInput.i4ISO,
                     m_sAFInput.i4IsAEStable,
                     rAEInfo.i4IsAEStable,
                     rAEInfo.i4IsAELocked,
                     rAEInfo.i4SceneLV,
                     m_sAFInput.i4ShutterValue,
                     m_sAFInput.i4IsFlashFrm,
                     m_sAFInput.i4DeltaBV,
                     m_sAFInput.i4AEBlockAreaYCnt,
                     (void*)m_sAFInput.pAEBlockAreaYvalue);

    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAdptCompInfo( const AdptCompTimeData_T &AdptCompTime)
{
    // info from AfMgr
    m_sAFInput.TS_MLStart  = m_u8MvLensTS;          // unit: us
    // info from Hal3A
    TS_AFDone_Pre = m_sAFInput.TS_AFDone;           // previous TS_AFDone unit: us
    m_sAFInput.TS_AFDone  = AdptCompTime.TS_AFDone; // current TS_AFDone, unit: us
    // debug
    if (m_i4DgbLogLv)
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) cmd-%s Dev(%d): ", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        ptrMsgBuf += sprintf( ptrMsgBuf, "TS_AFDone = %" PRId64 " \\ ", m_sAFInput.TS_AFDone);
        ptrMsgBuf += sprintf( ptrMsgBuf, "TS_MLStart= %" PRId64 " \\ ", m_sAFInput.TS_MLStart);
        ptrMsgBuf += sprintf( ptrMsgBuf, "PixelClk = %d \\ ", m_sAFInput.PixelClk);
        ptrMsgBuf += sprintf( ptrMsgBuf, "PixelInLine = %d \\ ", m_sAFInput.PixelInLine);
        CAM_LOGD("%s", dbgMsgBuf);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setIspSensorInfo2AF(ISP_SENSOR_INFO_T ispSensorInfo)
{
    CAM_LOGD_IF( m_i4DgbLogLv,
                 "cmd-%s FrameId(%5d), AfeGain(%5d), IspGain(%4d), AEStable(%d), AELock(%d), hlrEnable(%d), Eposuretime(%d), DeltaIdx(%4d), RealISO(%d), MaxISO(%d), AEStableCnt(%d), bAEScenarioChange(%d), u4AEFinerEVIdxBase(%d), u4AEidxCurrentF(%d)",
                 __FUNCTION__,
                 ispSensorInfo.i4FrameId,
                 ispSensorInfo.u4AfeGain,
                 ispSensorInfo.u4IspGain,
                 ispSensorInfo.bAEStable,
                 ispSensorInfo.bAELock,
                 ispSensorInfo.bHLREnable,
                 ispSensorInfo.u4Eposuretime,
                 ispSensorInfo.i4deltaIndex,
                 ispSensorInfo.u4RealISOValue,
                 ispSensorInfo.u4MaxISO,
                 ispSensorInfo.u4AEStableCnt,
                 ispSensorInfo.bAEScenarioChange,
                 ispSensorInfo.u4AEFinerEVIdxBase,
                 ispSensorInfo.u4AEidxCurrentF);

    m_vISQueue.pushHeadAnyway(ispSensorInfo);

    if (m_i4DbgAfegainQueue & 0x1)
    {
        MUINT32 front = m_vISQueue.head;
        MUINT32 end = m_vISQueue.tail;

        CAM_LOGD("--> Head:%d, Tail:%d, FrameId:%d, AfeGain:%d, IspGain:%d, hlrEnable:%d",
                 front,
                 end,
                 ispSensorInfo.i4FrameId,
                 ispSensorInfo.u4AfeGain,
                 ispSensorInfo.u4IspGain,
                 ispSensorInfo.bHLREnable);

        if (end > front)
        {
            front += m_vISQueue.queueSize;
        }

        for (MUINT32 i = front; i > end; i--)
        {
            MUINT32 idx = i % m_vISQueue.queueSize;
            CAM_LOGD("AfeGain qIdx(%d): frameId(%d), afeGain(%d), ispGain(%d), hlrEnable:%d",
                     idx,
                     m_vISQueue.content[idx].i4FrameId,
                     m_vISQueue.content[idx].u4AfeGain,
                     m_vISQueue.content[idx].u4IspGain,
                     m_vISQueue.content[idx].bHLREnable);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetAETargetMode(eAETargetMODE eAETargetMode)
{
    m_AETargetMode = eAETargetMode;

    CAM_LOGD_IF( m_i4DgbLogLv, "m_AETargetMode = %d", m_AETargetMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AfMgr::GetHybridAFMode()
{
    MUINT32 HybridAFMode = 0;

    //Depth AF
    if ((m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE) && (m_bSDAFEn == MTRUE))
    {
        HybridAFMode |= 1;
    }

    //PDAF
    if (m_bEnablePD)
    {
        HybridAFMode |= 2;  //2'b 0010
    }

    //Laser AF
    if (m_bLDAFEn == MTRUE)
    {
        HybridAFMode |= 4;
    }

    CAM_LOGD_IF( m_i4DgbLogLv & 4, "%s:(0x%x)", __FUNCTION__, HybridAFMode);

    return HybridAFMode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::HybridAFPreprocessing()
{
    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if ( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d) PDEn(%d) LaserEn(%d) FDDetect(%d): ",
                              m_u4ReqMagicNum,
                              m_u4StaMagicNum,
                              __FUNCTION__,
                              m_i4CurrSensorDev,
                              m_bEnablePD,
                              m_bLDAFEn,
                              m_sAFOutput.i4FDDetect);
    }


    /* for reference */

    /* do PD data preprocessing */
    if( m_bEnablePD)
    {
        MINT32   PDOut_numRes = m_sAFInput.sPDInfo.i4PDBlockInfoNum;
        MUINT16 pPDAF_DAC = 0;
        MUINT16 pPDAF_Conf = 0;

        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[PD (DAC, CL)] org{ ");
            for ( MINT32 i = 0; i < PDOut_numRes; i++)
            {
                pPDAF_DAC = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafDacIndex;
                pPDAF_Conf = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC, pPDAF_Conf);
            }
            ptrMsgBuf += sprintf( ptrMsgBuf, "}, ");
        }

        /*****************************  PD result preprocessing (start) *****************************/
#if 0
        {
            // To Do :
            // customer can process PD result here for customization.
        }
#endif
        /*****************************  PD result preprocessing (end)  *****************************/

        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "new{ ");
            for ( MINT32 i = 0; i < PDOut_numRes; i++)
            {
                pPDAF_DAC = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafDacIndex;
                pPDAF_Conf = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC, pPDAF_Conf);
            }
            ptrMsgBuf += sprintf( ptrMsgBuf, "}, ");
        }

        // LaunchCamTrigger
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            if (m_i4ValidPDFrameCount == -1 && PDOut_numRes > 0)
            {
                for (MINT32 i = 0; i < PDOut_numRes; i++)
                {
                    pPDAF_Conf = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                    if (pPDAF_Conf > 0)
                    {
                        m_i4ValidPDFrameCount = m_u4ReqMagicNum;
                        CAM_LOGD("#(%5d,%5d) %s Dev(%d) LaunchCamTrigger VALID PD(%d) - BUT NOT TRIGGER YET",
                                 m_u4ReqMagicNum, m_u4StaMagicNum,
                                 __FUNCTION__,
                                 m_i4CurrSensorDev,
                                 m_i4ValidPDFrameCount);
                        break;
                    }
                }
            }

            if (m_i4ValidPDFrameCount != -1 && m_u4ReqMagicNum >= (MUINT32)(m_i4ValidPDFrameCount + m_i4ValidPDTriggerTimeout))
            {
                // Valid PD and m_i4ValidPDTriggerTimeout(for waiting Face)
                m_i4LaunchCamTriggered = E_LAUNCH_AF_TRIGGERED;
                CAM_LOGD( "#(%5d,%5d) %s Dev(%d) LaunchCamTrigger VALID PD(%d + %d) - UnlockAlgo + TRIGGERAF lib_afmode(%d)",
                          m_u4ReqMagicNum, m_u4StaMagicNum,
                          __FUNCTION__,
                          m_i4CurrSensorDev,
                          m_i4ValidPDFrameCount, m_i4ValidPDTriggerTimeout,
                          m_eLIB3A_AFMode);
                UnlockAlgo();
                triggerAF(AF_MGR_CALLER);
                m_i4IsLockForLaunchCamTrigger = 0;
            }

        }
    }
    else
    {
        // To set the target to make sure the ROI always be drawn with Contrast AF.
        m_sCallbackInfo.CompSet_PDCL.Target = 101;  // value > target ==> no draw
    }

    /* do laser data preprocessing */
    if (m_bLDAFEn == MTRUE && m_pLaserMgr != NULL)
    {
        MINT32 i4LaserStatus = m_pLaserMgr->getLaserCurStatus();

        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[Laser %d (DAC, CONF, DIST)] org:(%d, %d, %d) ",
                                  i4LaserStatus,
                                  m_sAFInput.sLaserInfo.i4CurPosDAC,
                                  m_sAFInput.sLaserInfo.i4Confidence,
                                  m_sAFInput.sLaserInfo.i4CurPosDist);
        }

        /*****************************  Laser result preprocessing (start) *****************************/
        {
            switch (i4LaserStatus)
            {
            case STATUS_RANGING_VALID:
                m_sAFInput.sLaserInfo.i4Confidence = 80;
                break;

            case STATUS_MOVE_DMAX:
            case STATUS_MOVE_MAX_RANGING_DIST:
                m_sAFInput.sLaserInfo.i4Confidence = 49;
                break;

            default:
                m_sAFInput.sLaserInfo.i4Confidence = 20;
                break;
            }

            if (m_sAFInput.sLaserInfo.i4CurPosDist < 70)
            {
                m_sAFInput.sLaserInfo.i4Confidence = 20;
            }

            //Touch AF : if the ROI isn't in the center, the laser data need to set low confidence.
            if ( (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO))
            {
                #if 1
                m_sAFInput.sLaserInfo.i4Confidence = 0;
                m_sAFInput.sLaserInfo.i4CurPosDAC  = 0;
                #else
                if (m_sAFInput.sLaserInfo.i4AfWinPosX >= LASER_TOUCH_REGION_W ||
                    m_sAFInput.sLaserInfo.i4AfWinPosY >= LASER_TOUCH_REGION_H ||
                    m_sAFInput.sLaserInfo.i4AfWinPosCnt > 1 )
                {
                    m_sAFInput.sLaserInfo.i4Confidence = 0;
                    m_sAFInput.sLaserInfo.i4CurPosDAC  = 0;
                }
                #endif
            }

            if ( 1 == m_sAFOutput.i4FDDetect)
            {
                m_sAFInput.sLaserInfo.i4Confidence = 0;
                m_sAFInput.sLaserInfo.i4CurPosDAC  = 0;
            }
        }
        /*****************************  Laser result preprocessing (end)  *****************************/


        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[Laser %d (DAC, CONF, DIST)] new:(%d, %d, %d) ",
                                  i4LaserStatus,
                                  m_sAFInput.sLaserInfo.i4CurPosDAC,
                                  m_sAFInput.sLaserInfo.i4Confidence,
                                  m_sAFInput.sLaserInfo.i4CurPosDist);
        }
    }

    CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AREA_T& AfMgr::SelROIToFocusing( AF_OUTPUT_T &sInAFInfo)
{
    if ( (m_i4IsSelHWROI_PreState != m_i4IsSelHWROI_CurState && m_i4IsSelHWROI_CurState == MTRUE) ||
            (m_bLatchROI == MTRUE) ||
            (m_i4DgbLogLv))
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) latch(%d) type(%d) sel(%d) issel(%d) ROI(X,Y,W,H) : Center(%d, %d, %d, %d), AP(%d, %d, %d, %d), OT(%d, %d, %d, %d), APForContiMode(%d, %d, %d, %d), HW(%d, %d, %d, %d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_bLatchROI,
                  m_sArea_TypeSel,
                  sInAFInfo.i4ROISel,
                  m_i4IsSelHWROI_CurState,
                  m_sArea_Center.i4X,
                  m_sArea_Center.i4Y,
                  m_sArea_Center.i4W,
                  m_sArea_Center.i4H,
                  m_sArea_APCmd.i4X,
                  m_sArea_APCmd.i4Y,
                  m_sArea_APCmd.i4W,
                  m_sArea_APCmd.i4H,
                  m_sArea_OTFD.i4X,
                  m_sArea_OTFD.i4Y,
                  m_sArea_OTFD.i4W,
                  m_sArea_OTFD.i4H,
                  m_sArea_APForContiMode.i4X,
                  m_sArea_APForContiMode.i4Y,
                  m_sArea_APForContiMode.i4W,
                  m_sArea_APForContiMode.i4H,
                  m_sArea_HW.i4X,
                  m_sArea_HW.i4Y,
                  m_sArea_HW.i4W,
                  m_sArea_HW.i4H);
    }


    if ( m_bLatchROI == MTRUE) /* Got changing ROI command from host. Should be trigger searching.*/
    {
        /**
         *   force using the new ROI which is sent from host, and do one time searching :
         *   @LIB3A_AF_MODE_AFS -> wiait autofocus command.
         *   @LIB3A_AF_MODE_AFC_VIDEO, LIB3A_AF_MODE_AFC ->  focuse to trigger searching by switching af mode to auto mode in AF HAL.
         */
        m_sArea_Focusing = m_sArea_APCmd;
        m_sArea_TypeSel  = AF_ROI_SEL_AP;

        CAM_LOGD("#(%5d,%5d) %s [CMD] %d (X,Y,W,H)=(%d, %d, %d, %d)",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_sArea_TypeSel,
                 m_sArea_Focusing.i4X,
                 m_sArea_Focusing.i4Y,
                 m_sArea_Focusing.i4W,
                 m_sArea_Focusing.i4H);

        //used to control select AF ROI at AFS mode.
    }
    else if ( sInAFInfo.i4ROISel == AF_ROI_SEL_NONE)
    {
        /**
         *  Do nothing
         *  This case is just happened after af is inited.
         *  Wait algo to check using FD or center ROI to do focusing.
         *  Should get i4IsMonitorFV==TRUE. i4IsMonitorFV will be FALSE when ROI is selected.
         */
    }
    else if ( m_i4IsSelHWROI_CurState == MTRUE) /* Without got changing ROI command from host, and need to do searching*/
    {
        switch ( sInAFInfo.i4ROISel)
        {
        case AF_ROI_SEL_NONE :
            //This case cannot be happened.
            break;
        case AF_ROI_SEL_AP :
            //This case cannot be happened.
            m_sArea_Focusing = m_sArea_APCmd;
            break;
        case AF_ROI_SEL_OT :
        case AF_ROI_SEL_FD :
            m_sArea_Focusing = m_sArea_APCmd = m_sArea_OTFD; //rest AP ROI
            break;
        case AF_ROI_SEL_CENTER :
        case AF_ROI_SEL_DEFAULT :
        default :
            if (m_sArea_APValid == 1)
            {
                m_sArea_Focusing = m_sArea_APCmd = m_sArea_APForContiMode; //rest AP ROI
                m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sPDROI = m_sArea_APForContiMode;
            }
            else
            {
                m_sArea_Focusing = m_sArea_APCmd = m_sArea_Center; //rest AP ROI
                m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sPDROI = m_sArea_Center;
            }
            break;
        }
        m_sArea_TypeSel = sInAFInfo.i4ROISel;

        CAM_LOGD( "#(%5d,%5d) %s [SEL] %d (X,Y,W,H)=(%d, %d, %d, %d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_sArea_TypeSel,
                  m_sArea_Focusing.i4X,
                  m_sArea_Focusing.i4Y,
                  m_sArea_Focusing.i4W,
                  m_sArea_Focusing.i4H);
    }

    return m_sArea_Focusing;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::doAF( MVOID* /*ptrInAFData*/)
{
    SensorProvider_getData();

    if ( m_i4EnableAF == 0)
    {
        m_sAFOutput.i4IsAfSearch = AF_SEARCH_DONE;
        m_sAFOutput.i4IsFocused = 0;
        m_sAFOutput.i4AFPos = 0;
        m_bRunPDEn = MTRUE;
        CAM_LOGD_IF( m_i4DgbLogLv, "disableAF");

        if (m_i4IsEnableFVInFixedFocus)
        {
            m_sArea_Focusing = m_sArea_OTFD;
            m_sArea_TypeSel  = (m_sArea_Focusing.i4W != 0 || m_sArea_Focusing.i4H != 0) ? AF_ROI_SEL_FD : AF_ROI_SEL_NONE;
        }

        return S_AF_OK;
    }

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;
    if ( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d): ", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
    }

    AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "doAF #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);

    //----------------------------------Prepare AF Algorithm input data------------------------------------------------
    //==========
    // Stereo depth AF
    //==========
    // SDAF_Update();

    //==========
    // Laser distance AF
    //==========
    LDAF_Update();

    //==========
    //   PDAF
    //==========
    if ( m_bRunPDEn == MTRUE)
    {
#define PERFRAME_GET_PD_RESULT 1
        // LaunchCamTrigger Search Done ==> get PD every 2 frames
        if ( m_i4LaunchCamTriggered != E_LAUNCH_AF_DONE)
        {
            m_bPdInputExpected = (m_i4FirsetCalPDFrameCount > 0) && (m_u4ReqMagicNum > (MUINT32)m_i4FirsetCalPDFrameCount);
        }
        else if ( m_ptrNVRam->rAFNVRAM.i4EasyTuning[2] == PERFRAME_GET_PD_RESULT)
        {
            m_bPdInputExpected = MTRUE;
        }
        else
        {
            m_bPdInputExpected = 1 - m_bPdInputExpected;
        }

        m_sAFInput.sPDInfo.i4Valid = 0;
        if ( m_bPdInputExpected)
        {
            PD_CALCULATION_OUTPUT *ptrPDRes = nullptr;
            if ( IPDMgr::getInstance().getPDTaskResult( m_i4CurrSensorDev, &ptrPDRes) == S_3A_OK)
            {
                if ( ptrPDRes)
                {
                    // set pd result to pd algorithm
                    memset( &m_sAFInput.sPDInfo, 0, sizeof(AFPD_INFO_T));
                    m_sAFInput.sPDInfo.i4Valid  = MTRUE;
                    m_sAFInput.sPDInfo.i4FrmNum = ptrPDRes->frmNum;

                    MINT32 _idx=0;
                    for (MINT32 i = 0; i < ptrPDRes->numRes; i++)
                    {
                        if (ptrPDRes->Res[i].sROIInfo.sType == ROI_TYPE_AP)
                        {
                            if ( /* for multi touch case*/
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4X != m_sArea_APCmd.i4X) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4Y != m_sArea_APCmd.i4Y) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4W != m_sArea_APCmd.i4W) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4H != m_sArea_APCmd.i4H))
                            {
                                continue;
                            }
                        }

                        m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sType   = ptrPDRes->Res[i].sROIInfo.sType;
                        m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI  = ptrPDRes->Res[i].sROIInfo.sPDROI;
                        m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4PDafDacIndex    = ptrPDRes->Res[i].DesLensPos;
                        m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4PDafConverge    = ptrPDRes->Res[i].PhaseDifference;
                        m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4PDafConfidence  = (MUINT32)ptrPDRes->Res[i].ConfidenceLevel;
                        m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4SatPercent      = ptrPDRes->Res[i].SaturationPercent;

                        //print log if necessary
                        if ( m_i4DgbLogLv)
                        {
                            CAM_LOGD("[%s] #%d (%d,%4d,%4d,%4d,%4d,%3d->%3d,%6d,%3d,%d)",
                                     __FUNCTION__,
                                     _idx,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sType,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4X,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4Y,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4W,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4H,
                                     ptrPDRes->Res[i].CurLensPos,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4PDafDacIndex,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4PDafConverge,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4PDafConfidence,
                                     m_sAFInput.sPDInfo.sPDBlockInfo[_idx].i4SatPercent);
                        }

                        //
                        _idx++;
                    }
                    m_sAFInput.sPDInfo.i4PDBlockInfoNum = _idx;

                    //release resource
                    delete ptrPDRes;
                    ptrPDRes = nullptr;
                }
                else
                {
                    m_sAFInput.sPDInfo.i4Valid = 0;
                    CAM_LOGE( "get null pointer result from pd manager, should not be happened !!");
                }
            }
            else
            {
                m_sAFInput.sPDInfo.i4Valid = 0;
                CAM_LOGW( "pd result is not ready! statMagicNum(%d)", m_u4StaMagicNum);
            }
        }

        //direct control
        m_sAFInput.sPDInfo.i4PDPureRawfrm = (MINT32)m_bPdInputExpected;

        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "PDExpt(%d %d)/PDVd(%d)/LaunchCam(%d)/FirsetCalPDFrameCount(%d)/",
                                  m_bPdInputExpected,
                                  m_ptrNVRam->rAFNVRAM.i4EasyTuning[2],
                                  m_sAFInput.sPDInfo.i4Valid,
                                  m_i4LaunchCamTriggered,
                                  m_i4FirsetCalPDFrameCount);
        }

    }

    //---------------------------------------Run hybrid AF core flow---------------------------------------------
    if (IspMgrAFStatHWPreparing())
    {
        /* No need execute handleAF */
    }
    else if(m_i4FlashPartialCount>0)
    {
        m_i4FlashPartialCount--;
        CAM_LOGD_IF(m_i4DgbLogLv, "Skip handleAF because flashstate not READY");
    }
    else if(m_i4DbgPDVerifyEn)
    {
        i4IsLockAERequest   = property_get_int32("vendor.debug.pdmgr.lockae", 0);
        m_i4DbgMotorDisable = property_get_int32("vendor.debug.af_motor.disable", 0);
        m_i4DbgMotorMPos    = property_get_int32("vendor.debug.af_motor.position", 1024);
        m_u8MvLensTS        = MoveLensTo( m_i4DbgMotorMPos, AF_MGR_CALLER);
    }
    else
    {
        //get current lens position.
        getLensInfo( m_sCurLensInfo);
        //Do AF is triggered when Vsync is came, so use previours lens information.
        m_sAFInput.sLensInfo = m_sCurLensInfo;
        if (m_i4DbgMotorDisable || m_i4DbgPDVerifyEn)
            m_sAFInput.sLensInfo.i4CurrentPos = m_sAFOutput.i4AFPos;
        //Pre-processing input data for hybrid AF.
        HybridAFPreprocessing();
        //select focusing ROI.
        m_sAFInput.sAFArea.i4Count  = 1;
        m_sAFInput.sAFArea.sRect[0] = SelROIToFocusing( m_sAFOutput);
        //set MZ infotmation.
        m_sAFInput.i4IsMZ = m_bMZAFEn;
        //select hybrid af behavior.
        m_sAFInput.i4HybridAFMode = GetHybridAFMode();
        //get ae relate information
        ISP_SENSOR_INFO_T* ptrSensorSetting = getMatchedISInfoFromFrameId(m_u4StaMagicNum);
        if (ptrSensorSetting->i4FrameId != -1)
        {
            m_sAFInput.i4DeltaBV          = ptrSensorSetting->i4deltaIndex;
            m_sAFInput.i4IsAEidxReset     = ptrSensorSetting->bAEScenarioChange;
            m_sAFInput.u4AEFinerEVIdxBase = ptrSensorSetting->u4AEFinerEVIdxBase;
            m_sAFInput.u4AEidxCurrentF    = ptrSensorSetting->u4AEidxCurrentF;
        }
        //Run algorithm
        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "MZen(%d)/temp(%d)/hybrid mode(0x%x)/curPos(%4d)/",
                                  m_sAFInput.i4IsMZ,
                                  m_sAFInput.u4CurTemperature,
                                  m_sAFInput.i4HybridAFMode,
                                  m_sAFInput.sLensInfo.i4CurrentPos);
        }
        AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "handleAF");
        if(m_pIAfAlgo)
            m_pIAfAlgo->handleAF( m_sAFInput, m_sAFOutput);
        AF_TRACE_FMT_END(TRACE_LEVEL_DBG);
        //Move lens position.
        AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "Move Lens (%d)", m_sAFOutput.i4AFPos);
        m_u8MvLensTS = MoveLensTo( m_sAFOutput.i4AFPos, AF_MGR_CALLER);
        AF_TRACE_FMT_END(TRACE_LEVEL_DBG);
        m_i4LensPosExit = m_sAFOutput.i4AFPos;
        //Update parameter for flow controlling
        m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState;
        m_i4IsSelHWROI_CurState = m_sAFOutput.i4IsSelHWROI;
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState;
        m_i4IsAFSearch_CurState = m_sAFOutput.i4IsAfSearch;

        //Update LockAERequest: lockAE when Contrast AF
        i4IsLockAERequest = (m_sAFOutput.i4IsAfSearch <= AF_SEARCH_TARGET_MOVE) ? 0 : 1;

        // Warning for AdpAlarm
        if (m_sAFOutput.i4AdpAlarm)
        {
            // error log
            CAM_LOGE("#(%5d,%5d) %s Dev(%d) i4AdpAlarm(%d)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     m_sAFOutput.i4AdpAlarm);
            if (property_get_int32("vendor.debug.af_mgr.adpalarm", 0) > 0)
            {
                // yellow screen (only enable after SQC1)
                AEE_ASSERT_AF("AF AdpAlarm");
            }
        }
    }

    //------------------------------------Configure/update HW setting----------------------------------
    if (!m_i4DbgAutoBitTrue) // Do Not Change Config while Auto Bit-True
    {
        IspMgrAFStatUpdateHw();
    }
    //------------------------------------Update parameter for depth AF----------------------------------
    SDAF_Result();

    //------------------------------------store debug information while searching----------------------
    if ( m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
    {
        MUINT64 ts = getTimeStamp_us();
        WDBGTSInfo( MGR_TS, (MINT32)ts, 0);
    }

    //------------------------------------Update parameter for flow controlling----------------------------------

    m_i4IsFocused = m_sAFOutput.i4IsFocused;

    UpdatePDParam( m_sAFOutput.i4ROISel);

    //Event
    if (  m_i4IsAFSearch_PreState != m_i4IsAFSearch_CurState)
    {
        CAM_LOGD("#(%5d,%5d) %s Dev(%d) isAFSearch changed : %d -> %d",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_i4IsAFSearch_PreState,
                 m_i4IsAFSearch_CurState);
        if ( m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
        {
            // Positive Edge: Searching Start
            if (m_pMcuDrv)
            {
                m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MTRUE);
            }
            CleanMgrDbgInfo();
            CleanTSDbgInfo();

            // CONTINUOUS_VIDEO or CONTINUOUS_PICTURE ==> need to check if send callback
            if (m_bNeedCheckSendCallback)
            {
                if (m_bNeedLock)
                {
                    /* It have to send callback once got autofocus command from host */
                    m_bNeedSendCallback = 1;
                }
                else
                {
                    m_sCallbackInfo.isSearching        = m_i4IsAFSearch_CurState;
                    m_sCallbackInfo.CompSet_PDCL.Value = m_sAFOutput.i4PdTrigUiConf;
                    m_sCallbackInfo.CompSet_ISO.Value  = m_sAFInput.i4ISO;
                    m_sCallbackInfo.CompSet_FPS.Value  = (m_sAFInput.TS_AFDone - TS_AFDone_Pre > 0) ? 1000000 / (m_sAFInput.TS_AFDone - TS_AFDone_Pre) : 0;
                    m_bNeedSendCallback |= checkSendCallback(m_sCallbackInfo); // isSearch switch between 1~3 still need SendCallback
                    CAM_LOGD("#(%5d,%5d) %s Dev(%d) checkSendCallback: m_bNeedSendCallback(%d), AfterAutoMode(%d), Searching(%d), PDCL(%d, %d), ISO(%d, %d), FPS(%d, %d)",
                             m_u4ReqMagicNum,
                             m_u4StaMagicNum,
                             __FUNCTION__,
                             m_i4CurrSensorDev,
                             m_bNeedSendCallback,
                             m_sCallbackInfo.isAfterAutoMode,
                             m_sCallbackInfo.isSearching,
                             m_sCallbackInfo.CompSet_PDCL.Value, m_sCallbackInfo.CompSet_PDCL.Target,
                             m_sCallbackInfo.CompSet_ISO.Value, m_sCallbackInfo.CompSet_ISO.Target,
                             m_sCallbackInfo.CompSet_FPS.Value, m_sCallbackInfo.CompSet_FPS.Target);
                }
                if (m_bNeedSendCallback == 1 && m_sAFOutput.i4ROISel != AF_ROI_SEL_FD)
                {
                    m_i4IsCAFWithoutFace = 1;   // CAFWithoutFace && NeedSendCallback
                }
            }
            else // else ==> always send callback
            {
                m_bNeedSendCallback = 1;
            }

            if (m_bNeedSendCallback)
            {
                UpdateState( EVENT_SEARCHING_START);
            }
        }
        else if (m_i4IsAFSearch_CurState == AF_SEARCH_DONE)
        {
            // Negative Edge: Searching End
            if (m_pMcuDrv)
            {
                m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, m_i4OISDisable);
            }
            SetMgrDbgInfo();

            if (m_bNeedLock)
            {
                /* It have to send callback once got autofocus command from host */
                m_bNeedSendCallback = 1;
            }

            if (m_bNeedSendCallback)
            {
                UpdateState( EVENT_SEARCHING_END);
                m_bNeedSendCallback = 0;
                m_i4IsCAFWithoutFace = 0;
                m_bForceDoAlgo = MFALSE;
            }
            else
            {
                UpdateState( EVENT_SEARCHING_DONE_RESET_PARA); /* Hybrid AF : PDAF, LDAF, ... */
            }
            // The af search for LaunchCamTrigger is done
            if (m_i4LaunchCamTriggered == E_LAUNCH_AF_TRIGGERED)
            {
                m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
            }
        }
    }


    if ( (m_i4IsAFSearch_PreState != m_i4IsAFSearch_CurState) ||
            (m_i4IsSelHWROI_PreState != m_i4IsSelHWROI_CurState) ||
            (m_i4DgbLogLv))
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): [Status] IsAFSearch(%d->%d), IsSelHWROI(%d->%d), ROISel(%d), Event(0x%x)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_i4IsAFSearch_PreState,
                  m_i4IsAFSearch_CurState,
                  m_i4IsSelHWROI_PreState,
                  m_i4IsSelHWROI_CurState,
                  m_sAFOutput.i4ROISel,
                  m_eEvent);
    }

    if (m_i4DgbLogLv)
    {
        m_i4DbgMotorDisable = property_get_int32("vendor.debug.af_motor.disable", 0);
        m_i4DbgMotorMPos    = property_get_int32("vendor.debug.af_motor.position", 1024);
    }

    AF_TRACE_FMT_END(TRACE_LEVEL_DBG);

    if (m_i4LaunchCamTriggered_Prv == E_LAUNCH_AF_IDLE && m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
    {
        // positive edge
        AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "LaunchCamTrigger Start: #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);
        AF_TRACE_FMT_END(TRACE_LEVEL_DBG);
    }
    else if (m_i4LaunchCamTriggered_Prv == E_LAUNCH_AF_TRIGGERED && m_i4LaunchCamTriggered == E_LAUNCH_AF_DONE)
    {
        // negetive edge
        AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "LaunchCamTrigger End: #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);
        AF_TRACE_FMT_END(TRACE_LEVEL_DBG);
    }
    m_i4LaunchCamTriggered_Prv = m_i4LaunchCamTriggered;

    //---------------------------------------------------------------------------------------------------
    //get current sensor's temperature
    if (m_i4EnThermalComp)
        m_sAFInput.u4CurTemperature = getSensorTemperature();

    CAM_LOGD_IF( m_i4DgbLogLv, "%s %d", dbgMsgBuf, (MINT32)(ptrMsgBuf - dbgMsgBuf));
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_SENSOR_INFO_T* AfMgr::getMatchedISInfoFromFrameId(MINT32 frameId)
{
    m_sIspSensorInfo.i4FrameId = -1;

    MUINT32 front = m_vISQueue.head;
    MUINT32 end = m_vISQueue.tail;

    if (end > front)
        front += m_vISQueue.queueSize;

    // assume frameid is continuous, calculate idx by frameid difference
    MUINT32 offset = m_vISQueue.content[(front % m_vISQueue.queueSize)].i4FrameId - frameId;
    MUINT32 idx = (front - offset) % m_vISQueue.queueSize;

    int enableDebug = m_i4DbgAfegainQueue & 0x2;
    if (enableDebug)
    {
        CAM_LOGD("search afegain queue... magic: %d, head frameid: %d, idx: %d, indexed frameid: %d",
                 frameId, m_vISQueue.content[(front % m_vISQueue.queueSize)].i4FrameId, idx, m_vISQueue.content[idx].i4FrameId);
    }

    if (idx < m_vISQueue.queueSize && m_vISQueue.content[idx].i4FrameId == frameId) //found
    {
        memcpy(&m_sIspSensorInfo, &(m_vISQueue.content[idx]), sizeof(ISP_SENSOR_INFO_T));
    }
    else // fail by indexing, search one by one
    {
        if (enableDebug)
        {
            CAM_LOGD("search afegain queue... Failed by indexing, search one by one, found idx = %d", idx);
        }
        for (MUINT32 i = front; i > end; i--)
        {
            idx = i % m_vISQueue.queueSize;
            if (m_vISQueue.content[idx].i4FrameId == frameId) //found
                memcpy(&m_sIspSensorInfo, &(m_vISQueue.content[idx]), sizeof(ISP_SENSOR_INFO_T));
        }
    }
    // cannot get matched afegain from queue
    if (m_sIspSensorInfo.i4FrameId == -1)
    {
        CAM_LOGW("Cannot get matched afegain from m_vISQueue!");

        if (enableDebug)
        {
            front = m_vISQueue.head;
            end = m_vISQueue.tail;

            if (end > front)
                front += m_vISQueue.queueSize;
            for (MUINT32 i = front; i > end; i--)
            {
                MUINT32 idx = i % m_vISQueue.queueSize;
                CAM_LOGD("AfeGain qIdx(%d): frameId(%d), afeGain(%d), ispGain(%d), AEIdxCurrentF(%d)",
                         idx, m_vISQueue.content[idx].i4FrameId, m_vISQueue.content[idx].u4AfeGain, m_vISQueue.content[idx].u4IspGain, m_vISQueue.content[idx].u4AEidxCurrentF);
            }
        }
    }
    return &m_sIspSensorInfo;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::passPDBuffer( MVOID *ptrInPDData, mcuMotorInfo *pLensInfo)
{
    return IPDMgr::getInstance().postToPDTask(m_i4CurrSensorDev, reinterpret_cast<StatisticBufInfo *>( ptrInPDData), pLensInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::enablePBIN(MBOOL bEnable)
{
    return IPDMgr::getInstance().setPBNen(m_i4CurrSensorDev, bEnable);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setPdSeparateMode(MUINT8 i4SMode)
{
    m_u1PdSeparateMode = i4SMode;
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::UpdateCenterROI( AREA_T &sOutAreaCenter)
{
    MUINT32 scalex = 100;
    MUINT32 scaley = 100;

    if (m_ptrNVRam == NULL)
    {
        CAM_LOGE( "[%s] Dev(%d) NVRAM init fai", __FUNCTION__, m_i4CurrSensorDev);
        return;
    }

    if ( m_bMZAFEn)
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4MultiAFCoefs[0];
        scaley = m_ptrNVRam->rAFNVRAM.i4MultiAFCoefs[1];
    }
    else if( m_bEnablePD)
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[1];
        scaley = m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[2];
    }
    else
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4SPOT_PERCENT_W;
        scaley = m_ptrNVRam->rAFNVRAM.i4SPOT_PERCENT_H;
    }

    if (     100 < scalex) scalex = 100;
    else if ( scalex <= 0) scalex = 15;

    if (     100 < scaley) scaley = 100;
    else if ( scaley <= 0) scaley = 15;

    CAM_LOGD( "%s %d %d %d %d %d %d - scale %d %d",
              __FUNCTION__,
              m_ptrNVRam->rAFNVRAM.i4MultiAFCoefs[0],
              m_ptrNVRam->rAFNVRAM.i4MultiAFCoefs[1],
              m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[1],
              m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[2],
              m_ptrNVRam->rAFNVRAM.i4SPOT_PERCENT_W,
              m_ptrNVRam->rAFNVRAM.i4SPOT_PERCENT_H,
              scalex,
              scaley);

    MUINT32 cropRegionCenter_X = m_sCropRegionInfo.i4X + (m_sCropRegionInfo.i4W / 2);
    MUINT32 cropRegionCenter_Y = m_sCropRegionInfo.i4Y + (m_sCropRegionInfo.i4H / 2);
    MUINT32 croiw = m_i4TGSzW * scalex / 100;
    MUINT32 croih = m_i4TGSzH * scaley / 100;
    MUINT32 croix = (cropRegionCenter_X > (croiw / 2)) ? cropRegionCenter_X - (croiw / 2) : 0;
    MUINT32 croiy = (cropRegionCenter_Y > (croih / 2)) ? cropRegionCenter_Y - (croih / 2) : 0;

    //updateing.
    sOutAreaCenter = AREA_T( croix, croiy, croiw, croih, AF_MARK_NONE);

    CAM_LOGD( "[%s] Dev(%d) ROI-C [X]%d [Y]%d [W]%d [H]%d",
              __FUNCTION__,
              m_i4CurrSensorDev,
              sOutAreaCenter.i4X,
              sOutAreaCenter.i4Y,
              sOutAreaCenter.i4W,
              sOutAreaCenter.i4H);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::UpdatePDParam( MINT32 &i4InROISel)
{
    if(m_bEnablePD)
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;

        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "#(%5d,%5d) %s:roiSel(%d) nFmt(%d) Info(vd,Fmt,X,Y,W,H,Info): ",
                                  m_u4ReqMagicNum,
                                  m_u4StaMagicNum,
                                  __FUNCTION__,
                                  i4InROISel,
                                  eIDX_ROI_ARRAY_NUM);
        }


        MINT32 PDRoisNum = 0;
        AFPD_BLOCK_ROI_T PDRois[eIDX_ROI_ARRAY_NUM];

        for ( MUINT16 i = 0; i < eIDX_ROI_ARRAY_NUM; i++)
        {
            MBOOL select = MFALSE;

            if ( m_i4DgbLogLv)
            {

                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#%d(%d,%d,%d,%d,%d,%d,%d) ",
                                      i,
                                      m_sPDRois[i].valid,
                                      m_sPDRois[i].info.sType,
                                      m_sPDRois[i].info.sPDROI.i4X,
                                      m_sPDRois[i].info.sPDROI.i4Y,
                                      m_sPDRois[i].info.sPDROI.i4W,
                                      m_sPDRois[i].info.sPDROI.i4H,
                                      m_sPDRois[i].info.sPDROI.i4Info);
            }

            switch (i)
            {
                case eIDX_ROI_ARRAY_AP:
                    select = m_sPDRois[i].info.sType==ROI_TYPE_AP ? MTRUE : MFALSE;
                    m_sPDRois[i].valid = i4InROISel == AF_ROI_SEL_AP ? MTRUE : m_sPDRois[i].valid; // force calculating FD window when hybrid AF need PD result of AP roi.
                    break;

                case eIDX_ROI_ARRAY_FD:
                    select = m_sPDRois[i].info.sType==ROI_TYPE_FD ? MTRUE : MFALSE;
                    m_sPDRois[i].valid = ((i4InROISel == AF_ROI_SEL_FD) || (i4InROISel == AF_ROI_SEL_OT)) ? MTRUE : m_sPDRois[i].valid; // force calculating FD window when hybrid AF is at FDAF stage.
                    break;

                case eIDX_ROI_ARRAY_CENTER:
                    select = m_sPDRois[i].info.sType==ROI_TYPE_CENTER ? MTRUE : MFALSE;
                    break;

                default :
                    select = MFALSE;
                    break;

            }

            // Only center ROI is calculated for verification flow.
            if ( m_i4DbgPDVerifyEn)
            {
                m_sPDRois[i].valid = (i == eIDX_ROI_ARRAY_CENTER) ? MTRUE : MFALSE;
            }

            if ( select && m_sPDRois[i].valid)
            {
                //
                PDRois[PDRoisNum] = m_sPDRois[i].info;

                //debug log
                if ( m_i4DgbLogLv)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf,
                                          "->(%d,%d,%d,%d,%d,%d,%d) ",
                                          m_sPDRois[i].valid,
                                          PDRois[PDRoisNum].sType,
                                          PDRois[PDRoisNum].sPDROI.i4X,
                                          PDRois[PDRoisNum].sPDROI.i4Y,
                                          PDRois[PDRoisNum].sPDROI.i4W,
                                          PDRois[PDRoisNum].sPDROI.i4H,
                                          PDRois[PDRoisNum].sPDROI.i4Info);
                }

                //
                PDRoisNum++;
            }
        }

        MRESULT res = m_pIAfAlgo->getFocusWindows( &PDRois[0],
                                               PDRoisNum,
                                               AF_PSUBWIN_NUM, /* Size of m_sPDCalculateWin */
                                               &m_sPDCalculateWin[0],
                                               &m_i4PDCalculateWinNum /* How many windows are stored in m_sPDCalculateWin*/);

        // error check
        if ( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "PDCalculateWin(total%d) Info(Fmt,X,Y,W,H,Info): ", m_i4PDCalculateWinNum);
        }

        for ( MINT32 i = 0; i < m_i4PDCalculateWinNum; i++)
        {
            if ( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#%d(%d,%d,%d,%d,%d,%d) ",
                                      i,
                                      m_sPDCalculateWin[i].sType,
                                      m_sPDCalculateWin[i].sPDROI.i4X,
                                      m_sPDCalculateWin[i].sPDROI.i4Y,
                                      m_sPDCalculateWin[i].sPDROI.i4W,
                                      m_sPDCalculateWin[i].sPDROI.i4H,
                                      m_sPDCalculateWin[i].sPDROI.i4Info);
            }

            if ( /* Boundary checking */
                (m_sPDCalculateWin[i].sPDROI.i4W <= 0) ||
                (m_sPDCalculateWin[i].sPDROI.i4H <= 0) ||
                (m_sPDCalculateWin[i].sPDROI.i4X <  0) ||
                (m_sPDCalculateWin[i].sPDROI.i4Y <  0) )
            {
                CAM_LOGE( "PD Calculation window is wrong, please check function(getFocusWindows) behavior!!");
                res = E_3A_ERR;
                break;
            }

        }

        if ( res != S_3A_OK)
        {
            m_i4PDCalculateWinNum = 1;
            m_sPDCalculateWin[0]  = m_sPDRois[eIDX_ROI_ARRAY_CENTER].info;
        }
        else if ( m_i4DbgPDVerifyEn && (m_i4PDCalculateWinNum < AF_PSUBWIN_NUM))
        {
            m_i4PDCalculateWinNum++;
            m_sPDCalculateWin[m_i4PDCalculateWinNum - 1] = m_sPDRois[eIDX_ROI_ARRAY_CENTER].info;
        }

        m_bRunPDEn = MTRUE;

        CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);

        /* exectuing from first request */
        MBOOL isForceCalPD = (MBOOL)((m_i4LaunchCamTriggered != E_LAUNCH_AF_DONE) || (m_sPDRois[eIDX_ROI_ARRAY_FD].valid));
        AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "UpdatePDParam (%d)", m_u4StaMagicNum);
        IPDMgr::getInstance().UpdatePDParam( m_i4CurrSensorDev,
                                             m_u4StaMagicNum,
                                             m_i4PDCalculateWinNum,
                                             m_sPDCalculateWin,
                                             m_sAFOutput.i4SearchRangeInf,
                                             m_sAFOutput.i4SearchRangeMac,
                                             isForceCalPD);
        AF_TRACE_FMT_END(TRACE_LEVEL_DBG);

        /*****************************************************************************************************************
         * FD roi is calculated once a FD region is set
         * So reset status once region is read out
         *****************************************************************************************************************/
        if ( m_sPDRois[eIDX_ROI_ARRAY_FD].valid)
        {
            m_sPDRois[eIDX_ROI_ARRAY_FD].valid = MFALSE;
        }

        //
        if ((m_i4FirsetCalPDFrameCount == -1) && (m_u4ReqMagicNum > 0))
        {
            m_i4FirsetCalPDFrameCount = m_u4ReqMagicNum;
        }
    }

}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::doSWPDE(MVOID *iHalMetaData, MVOID *iImgbuf)
{
    CAM_LOGD_IF( m_i4DgbLogLv, "%s +", __FUNCTION__);
    AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "doSWPDE");
    MRESULT res = IPDMgr::getInstance().doSWPDE(m_i4CurrSensorDev, iHalMetaData, iImgbuf);
    CAM_LOGD_IF( m_i4DgbLogLv, "%s - %d", __FUNCTION__, res);
    AF_TRACE_FMT_END(TRACE_LEVEL_DBG);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::PDPureRawInterval ()
{
#ifdef PD_PURE_RAW_INTERVAL
    return PD_PURE_RAW_INTERVAL;
#else
    return 1;
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setSGGPGN( MINT32 /*i4SGG_PGAIN*/)
{
    // workaround for iVHDR, no use
#if 0
    MINT32 i4AESetPGN = i4SGG_PGAIN;
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::passAFBuffer( MVOID *ptrInAFData)
{
    //---------------------------------------Statistic data information--------------------------------------------
    StatisticBufInfo *ptrStatInfo = reinterpret_cast<StatisticBufInfo *>( ptrInAFData);
    m_u4StaMagicNum   = ptrStatInfo->mMagicNumber;
    m_u4ConfigHWNum   = ptrStatInfo->mConfigNum;
    m_u4ConfigLatency = ptrStatInfo->mConfigLatency;

    if(ptrStatInfo->mFlashState==MTK_FLASH_STATE_PARTIAL)
    {
        m_i4FlashPartialCount = 2;
    }

    m_sAFInput.i4IsSupportN2Frame = (m_u4ConfigLatency == 3) ? 0 : 1;
    // m_sAFInput.i4Ready4EnableDS   = ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsAF_DSSupport, NULL, NULL);

    //Got AF statistic from DMA buffer.
    AF_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "ConvertBufToStat (%d)", m_u4StaMagicNum);
    MINT32 CurPos = m_i4DbgMotorDisable ? m_i4DbgMotorMPos : m_sAFOutput.i4AFPos;
    ConvertDMABufToStat(CurPos, ptrInAFData, m_sAFInput.sStatProfile);
    AF_TRACE_FMT_END(TRACE_LEVEL_DBG);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::ConvertDMABufToStat( MINT32 &i4CurPos, MVOID *ptrInStatBuf, AF_STAT_PROFILE_T &sOutSata)
{
    StatisticBufInfo *ptrStatInfo = reinterpret_cast<StatisticBufInfo *>( ptrInStatBuf);
    AF_STAT_T* pDMABuf            = reinterpret_cast<AF_STAT_T*>( ptrStatInfo->mVa);
    MUINT32 u4BlockStatSize       = sizeof(AF_STAT_T);                                      // (unit:Bytes) Statistics Data : 288bits=36bytes
    MUINT32 u4BlockStrideOfDMABuf = ptrStatInfo->mStride / u4BlockStatSize;                 // (unit:Blocks)BlockNumX + 1 (1 comes from the requirement of twin mode)
    MUINT32 u4FullStatSize        = ptrStatInfo->mSize;                                     // (unit:Bytes) MAX_AF_HW_WIN_X*MAX_AF_HW_WIN_Y*u4StatSizePerBlock

    CAM_LOGD_IF( m_i4DgbLogLv,
                 "#(%5d,%5d) %s(%d) Latency(%d) BlkW,H(%3d,%3d blocks) Str(%3d blocks) BlkSzW,H(%4d,%4d pixels) FullSz(%d bytes)",
                 m_u4ReqMagicNum, m_u4StaMagicNum,
                 __FUNCTION__, m_u4ConfigHWNum, m_u4ConfigLatency,
                 m_i4HWBlkNumX, m_i4HWBlkNumY,   // Width, Height (blocks)
                 u4BlockStrideOfDMABuf,          // Stride (blocks)
                 m_i4HWBlkSizeX, m_i4HWBlkSizeY, // Width, Height (pixels)
                 u4FullStatSize);                // (bytes)

    //reset last time data
    memset( &sOutSata, 0, sizeof(AF_STAT_PROFILE_T));

    if ( m_i4HWBlkNumX <= 0 || m_i4HWBlkNumY <= 0 || m_i4HWBlkNumX > MAX_AF_HW_WIN_X || m_i4HWBlkNumY > MAX_AF_HW_WIN_Y)
    {
        //Should not be happened.
        CAM_LOGE( "HW-Setting Fail");
    }
    else if ( pDMABuf == NULL)
    {
        //Should not be happened.
        CAM_LOGE( "AFO Buffer NULL");
    }
    else if ( u4FullStatSize < m_i4HWBlkNumX * m_i4HWBlkNumY * u4BlockStatSize)
    {
        //Should not be happened.
        CAM_LOGE( "AFO Size Fail");
    }
    else
    {
        //number of AF statistic blocks.
        MUINT32 nblkW __unused = m_i4HWBlkNumX;
        MUINT32 nblkH __unused = m_i4HWBlkNumY;

        //==========
        // Outputs
        //==========
        // AF extend mode
        sOutSata.u4AfExtValid = m_i4HWEnExtMode;
        sOutSata.u4ConfigNum  = m_u4ConfigHWNum;
        sOutSata.i4AFPos      = i4CurPos;

        //statistic information.
        sOutSata.u4NumBlkX   = m_i4HWBlkNumX;        // unit: blocks
        sOutSata.u4NumBlkY   = m_i4HWBlkNumY;        // unit: blocks
        sOutSata.u4NumStride = u4BlockStrideOfDMABuf; // unit: blocks
        sOutSata.u4SizeBlkX  = m_i4HWBlkSizeX;       // unit: pixels
        sOutSata.u4SizeBlkY  = m_i4HWBlkSizeY;       // unit: pixels

        //AF statistic - ISP5.0
        sOutSata.ptrStat = pDMABuf;


        MINT32 Blk_FaceLeft = 0;
        MINT32 Blk_FaceRight = 0;
        MINT32 Blk_FaceTop = 0;
        MINT32 Blk_FaceBottom = 0;

        if (m_i4IsEnableFVInFixedFocus)
        {
            MINT32 FaceLeft    = m_sArea_OTFD.i4X;
            MINT32 FaceRight   = m_sArea_OTFD.i4X + m_sArea_OTFD.i4W;
            MINT32 FaceTop     = m_sArea_OTFD.i4Y;
            MINT32 FaceBottom  = m_sArea_OTFD.i4Y + m_sArea_OTFD.i4H;

            Blk_FaceLeft    = FaceLeft   / m_i4HWBlkSizeX;
            Blk_FaceRight   = FaceRight  / m_i4HWBlkSizeX;
            Blk_FaceTop     = FaceTop    / m_i4HWBlkSizeY;
            Blk_FaceBottom  = FaceBottom / m_i4HWBlkSizeY;

            CAM_LOGD_IF(m_i4DgbLogLv, "#(%5d,%5d) %s FVinFixedFocus FaceROI(L,R,T,B)=(%d,%d,%d,%d), Blk_FaceROI(L,R,T,B)=(%d,%d,%d,%d), BlkNum(%d,%d), BlkSize(%d,%d)",
                        m_u4ReqMagicNum,
                        m_u4StaMagicNum,
                        __FUNCTION__,
                        FaceLeft, FaceRight, FaceTop, FaceBottom,
                        Blk_FaceLeft, Blk_FaceRight, Blk_FaceTop, Blk_FaceBottom,
                        m_i4HWBlkNumX, m_i4HWBlkNumY,
                        m_i4HWBlkSizeX, m_i4HWBlkSizeY);
        }

        if (m_i4DgbLogLv || m_i4IsEnableFVInFixedFocus)
        {
            MUINT64 FV_H0 = 0, FV_H1 = 0, FV_H2 = 0, FV_V = 0;
            MUINT64 FV_H0_FVinFixedFocus = 0;
            // block stride = BlockNumberX + 1, 1 comes from the requirement of twin mode
            MUINT32 NumBlkStride = sOutSata.u4NumStride;
            for (MUINT32 row = 0; row < sOutSata.u4NumBlkY; row++)
            {
                for (MUINT32 col = 0; col < sOutSata.u4NumBlkX; col++)
                {
                    AF_STAT_T* ptrStat = sOutSata.ptrStat + (row * NumBlkStride + col);
                    FV_H0 += ptrStat->u4FILH0;
                    FV_H1 += ptrStat->u4FILH1;
                    FV_H2 += ptrStat->u4FILH2;
                    FV_V  += ptrStat->u4FILV;

                    if (m_i4IsEnableFVInFixedFocus)
                    {
                        if (row >= (MUINT32)Blk_FaceTop && row <= (MUINT32)Blk_FaceBottom && col >= (MUINT32)Blk_FaceLeft && col <= (MUINT32)Blk_FaceRight)
                        {
                            FV_H0_FVinFixedFocus += ptrStat->u4FILH0;
                        }
                        m_sAFOutput.i8AFValue = FV_H0_FVinFixedFocus;
                    }
                    // debug : stt for each block
                    CAM_LOGD_IF( m_i4DgbLogLv & 0x2,
                                 "[%3d][%3d] : [H0]%8d, [H1]%8d, [H2]%8d, [V]%8d, [GSum]%8d",
                                 col, row,
                                 ptrStat->u4FILH0,
                                 ptrStat->u4FILH1,
                                 ptrStat->u4FILH2,
                                 ptrStat->u4FILV,
                                 ptrStat->u4GSum);
                }
            }
            CAM_LOGD_IF((m_i4IsEnableFVInFixedFocus && m_i4DgbLogLv), "#(%5d,%5d) %s FVinFixedFocus = %10llu",
                        m_u4ReqMagicNum,
                        m_u4StaMagicNum,
                        __FUNCTION__,
                        (unsigned long long)FV_H0_FVinFixedFocus);
            // debug : stt for sum of blocks excluding the spare block.
            CAM_LOGD_IF(m_i4DgbLogLv,
                        "#(%5d,%5d) %s(%d) [Pos]%4d [H0]%10llu [H1]%10llu [H2_EXT]%10llu [V]%llu",
                        m_u4ReqMagicNum,
                        m_u4StaMagicNum,
                        __FUNCTION__,
                        m_u4ConfigHWNum,
                        i4CurPos,
                        (unsigned long long)FV_H0,
                        (unsigned long long)FV_H1,
                        (unsigned long long)FV_H2,
                        (unsigned long long)FV_V);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocusFinish()
{
    MINT32 ret = 1;
    CAM_LOGD_IF(m_i4DgbLogLv, "%s, m_eAFState=%d", __FUNCTION__, m_eAFState);
    if ( (m_eAFState == E_AF_PASSIVE_SCAN) ||
            (m_eAFState == E_AF_ACTIVE_SCAN) )
    {
        ret = 0;
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocused()
{
    MINT32 ret = 0;

    if ( (m_eAFState == E_AF_PASSIVE_FOCUSED) ||
            (m_eAFState == E_AF_FOCUSED_LOCKED))
    {
        ret = 1;
    }

    CAM_LOGD_IF( m_i4DgbLogLv,
                 "%s %d %d",
                 __FUNCTION__,
                 m_eAFState,
                 ret);


    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isLockAE()
{
    return i4IsLockAERequest;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getDebugInfo( AF_DEBUG_INFO_T &rAFDebugInfo)
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( m_i4DgbLogLv & 4,
                 "%s Dev %d",
                 __FUNCTION__,
                 m_i4CurrSensorDev);

    //reset.
    memset( &rAFDebugInfo, 0, sizeof(AF_DEBUG_INFO_T));

    /* Do not modify following oder: */

    //1. Hybrid AF library
    if ( m_pIAfAlgo)
    {
        ret = m_pIAfAlgo->getDebugInfo( rAFDebugInfo);
    }

    //2. PD library
    if( m_bEnablePD)
    {
        ret = IPDMgr::getInstance().GetDebugInfo( m_i4CurrSensorDev, rAFDebugInfo);
    }

    //3. af mgr
    ret = GetMgrDbgInfo( rAFDebugInfo);



    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::GetMgrDbgInfo( AF_DEBUG_INFO_T &sOutMgrDebugInfo)
{
    MRESULT ret = E_3A_ERR;

    /* Store current status to EXIF*/
    SetMgrCapDbgInfo();


    /* Output */
    MUINT32 idx = AF_DEBUG_TAG_SIZE;
    for ( MUINT32 i = 0; i < AF_DEBUG_TAG_SIZE; i++)
    {
        if ( sOutMgrDebugInfo.Tag[i].u4FieldID == 0)
        {
            idx = i;
            break;
        }
    }

    CAM_LOGD_IF( m_i4DgbLogLv & 4,
                 "%s %d %d %d %d %d %d",
                 __FUNCTION__,
                 AF_DEBUG_TAG_SIZE,
                 idx,
                 MGR_EXIF_SIZE,
                 m_i4MgrExifSz,
                 MGR_CAPTURE_EXIF_SIZE,
                 m_i4MgrCapExifSz);

    if ( ((AF_DEBUG_TAG_SIZE - idx) >= m_i4MgrExifSz) && (0 < m_i4MgrExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrExifSz;
    }

    if ( (AF_DEBUG_TAG_SIZE - idx) >= m_i4MgrCapExifSz && (0 < m_i4MgrCapExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrCapExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrCapExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrCapExifSz;
    }

    if ( (AF_DEBUG_TAG_SIZE - idx) >= m_i4MgrTsExifSz && (0 < m_i4MgrTsExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrTSExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrTsExifSz);
        ret = S_3A_OK;
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WDBGTSInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep)
{
    MRESULT ret = E_3A_ERR;

    if ( m_i4MgrTsExifSz < MGR_TS_EXIF_SIZE)
    {
        m_sMgrTSExif[ m_i4MgrTsExifSz].u4FieldID    = AAATAG( AAA_DEBUG_AF_MODULE_ID, i4InTag, i4InLineKeep);
        m_sMgrTSExif[ m_i4MgrTsExifSz].u4FieldValue = i4InVal;
        m_i4MgrTsExifSz++;
        ret = S_3A_OK;
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CleanTSDbgInfo()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "%s", __FUNCTION__);
    memset( &m_sMgrTSExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_TS_EXIF_SIZE);
    m_i4MgrTsExifSz = 0;
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WDBGInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep)
{
    MRESULT ret = E_3A_ERR;

    if ( m_i4MgrExifSz < MGR_EXIF_SIZE)
    {
        m_sMgrExif[ m_i4MgrExifSz].u4FieldID    = AAATAG( AAA_DEBUG_AF_MODULE_ID, i4InTag, i4InLineKeep);
        m_sMgrExif[ m_i4MgrExifSz].u4FieldValue = i4InVal;
        m_i4MgrExifSz++;
        ret = S_3A_OK;
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CleanMgrDbgInfo()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "%s", __FUNCTION__);
    memset( &m_sMgrExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_EXIF_SIZE);
    m_i4MgrExifSz = 0;
    return S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetMgrDbgInfo()
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( m_i4DgbLogLv, "%s", __FUNCTION__);

    ret = CleanMgrDbgInfo();

    MUINT32 mzNum = static_cast<MUINT32>(m_sAFOutput.sROIStatus.i4TotalNum);
    if ( mzNum <= MAX_MULTI_ZONE_WIN_NUM)
    {
        ret = WDBGInfo( MZ_WIN_NUM, m_sAFOutput.sROIStatus.i4TotalNum, 0);
        ret = WDBGInfo( MZ_WIN_W, m_sAFOutput.sROIStatus.sROI[0].i4W, 0);
        ret = WDBGInfo( MZ_WIN_H, m_sAFOutput.sROIStatus.sROI[0].i4H, 1);

        for ( MUINT32 i = 0; i < mzNum; i++)
        {
            ret = WDBGInfo( MZ_WIN_X, m_sAFOutput.sROIStatus.sROI[i].i4X, 0);
            ret = WDBGInfo( MZ_WIN_Y, m_sAFOutput.sROIStatus.sROI[i].i4Y, 1);
            ret = WDBGInfo( MZ_WIN_RES, m_sAFOutput.sROIStatus.sROI[i].i4Info, 1);
            if ( ret == E_3A_ERR)
            {
                break;
            }
        }
    }

    if ( ret == S_3A_OK)
    {
        ret = WDBGInfo( MGR_TG_W, m_i4TGSzW, 0);
        ret = WDBGInfo( MGR_TG_H, m_i4TGSzH, 1);
        ret = WDBGInfo( MGR_BIN_W, m_i4BINSzW, 0);
        ret = WDBGInfo( MGR_BIN_H, m_i4BINSzH, 1);
        ret = WDBGInfo( MGR_CROP_WIN_X, m_sCropRegionInfo.i4X, 0);
        ret = WDBGInfo( MGR_CROP_WIN_Y, m_sCropRegionInfo.i4Y, 1);
        ret = WDBGInfo( MGR_CROP_WIN_W, m_sCropRegionInfo.i4W, 1);
        ret = WDBGInfo( MGR_CROP_WIN_H, m_sCropRegionInfo.i4H, 1);
        ret = WDBGInfo( MGR_DZ_CFG, m_i4DzWinCfg, 0);
        ret = WDBGInfo( MGR_DZ_FACTOR, m_i4DZFactor, 1);
        ret = WDBGInfo( MGR_FOCUSING_WIN_X, m_sArea_Focusing.i4X, 0);
        ret = WDBGInfo( MGR_FOCUSING_WIN_Y, m_sArea_Focusing.i4Y, 1);
        ret = WDBGInfo( MGR_FOCUSING_WIN_W, m_sArea_Focusing.i4W, 1);
        ret = WDBGInfo( MGR_FOCUSING_WIN_H, m_sArea_Focusing.i4H, 1);
        ret = WDBGInfo( MGR_OTFD_WIN_X, m_sArea_OTFD.i4X, 0);
        ret = WDBGInfo( MGR_OTFD_WIN_Y, m_sArea_OTFD.i4Y, 1);
        ret = WDBGInfo( MGR_OTFD_WIN_W, m_sArea_OTFD.i4W, 1);
        ret = WDBGInfo( MGR_OTFD_WIN_H, m_sArea_OTFD.i4H, 1);
        ret = WDBGInfo( MGR_CENTER_WIN_X, m_sArea_Center.i4X, 0);
        ret = WDBGInfo( MGR_CENTER_WIN_Y, m_sArea_Center.i4Y, 1);
        ret = WDBGInfo( MGR_CENTER_WIN_W, m_sArea_Center.i4W, 1);
        ret = WDBGInfo( MGR_CENTER_WIN_H, m_sArea_Center.i4H, 1);
        ret = WDBGInfo( MGR_CMD_WIN_X, m_sArea_APCmd.i4X, 0);
        ret = WDBGInfo( MGR_CMD_WIN_Y, m_sArea_APCmd.i4Y, 1);
        ret = WDBGInfo( MGR_CMD_WIN_W, m_sArea_APCmd.i4W, 1);
        ret = WDBGInfo( MGR_CMD_WIN_H, m_sArea_APCmd.i4H, 1);
        ret = WDBGInfo( MGR_LASER_VAL, m_sAFInput.sLaserInfo.i4CurPosDist, 0);
        //ret = WDBGInfo( MGR_FOCUSING_POS, m_sFocusDis.i4LensPos, 0);
        //ret = WDBGInfo( MGR_FOCUSING_DST, m_sFocusDis.fDist, 0);
    }

    return ret;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WDBGCapInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep)
{
    MRESULT ret = E_3A_ERR;

    if ( m_i4MgrCapExifSz < MGR_CAPTURE_EXIF_SIZE)
    {
        m_sMgrCapExif[ m_i4MgrCapExifSz].u4FieldID    = AAATAG( AAA_DEBUG_AF_MODULE_ID, i4InTag, i4InLineKeep);
        m_sMgrCapExif[ m_i4MgrCapExifSz].u4FieldValue = i4InVal;
        m_i4MgrCapExifSz++;
        ret = S_3A_OK;
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CleanMgrCapDbgInfo()
{
    CAM_LOGD_IF( m_i4DgbLogLv & 4, "%s", __FUNCTION__);
    memset( &m_sMgrCapExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_CAPTURE_EXIF_SIZE);
    m_i4MgrCapExifSz = 0;
    return S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetMgrCapDbgInfo()
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( m_i4DgbLogLv & 4, "%s", __FUNCTION__);

    ret = CleanMgrCapDbgInfo();

    if ( ret == S_3A_OK)
    {
        ret = WDBGCapInfo( MGR_CURRENT_POS, m_sCurLensInfo.i4CurrentPos, 0);
        ret = WDBGCapInfo( MGR_GYRO_SENSOR_X, m_i4GyroInfo[0], 0);
        ret = WDBGCapInfo( MGR_GYRO_SENSOR_Y, m_i4GyroInfo[1], 1);
        ret = WDBGCapInfo( MGR_GYRO_SENSOR_Z, m_i4GyroInfo[2], 1);
    }

    return ret;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getLensInfo( LENS_INFO_T &a_rLensInfo)
{
    MRESULT ret = E_3A_ERR;
    mcuMotorInfo rMotorInfo;
    mcuMotorInfo *pMotorInfo = nullptr;

    if ( m_pMcuDrv)
    {
        pMotorInfo = &rMotorInfo;
        ret = m_pMcuDrv->getMCUInfo(&rMotorInfo);

        if ( a_rLensInfo.i4CurrentPos != (MINT32)rMotorInfo.u4CurrentPosition)
        {
            CAM_LOGD_IF( m_i4DgbLogLv & 2,
                         "%s Dev %d, curPos %d, ",
                         __FUNCTION__,
                         m_i4CurrSensorDev,
                         (MINT32)rMotorInfo.u4CurrentPosition);
        }

        a_rLensInfo.i4CurrentPos   = (MINT32)rMotorInfo.u4CurrentPosition;
        a_rLensInfo.bIsMotorOpen   = rMotorInfo.bIsMotorOpen;
        a_rLensInfo.bIsMotorMoving = rMotorInfo.bIsMotorMoving;
        a_rLensInfo.i4InfPos       = (MINT32)rMotorInfo.u4InfPosition;
        a_rLensInfo.i4MacroPos     = (MINT32)rMotorInfo.u4MacroPosition;
        a_rLensInfo.bIsSupportSR   = rMotorInfo.bIsSupportSR;

        CAM_LOGD_IF( m_i4DgbLogLv & 2,
                     "%s Dev %d, %d, %d, %d, %d, %d, %d",
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     a_rLensInfo.i4CurrentPos,
                     a_rLensInfo.bIsMotorOpen,
                     a_rLensInfo.bIsMotorMoving,
                     a_rLensInfo.i4InfPos,
                     a_rLensInfo.i4MacroPos,
                     a_rLensInfo.bIsSupportSR);

        ret = S_AF_OK;
    }
    else
    {
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "%s Fail, Dev %d",
                     __FUNCTION__,
                     m_i4CurrSensorDev);
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setBestShotConfig()
{
    CAM_LOGD( "[setBestShotConfig] Not use");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::calBestShotValue( MVOID *pAFStatBuf)
{
    pAFStatBuf = NULL;

    CAM_LOGD( "[calBestShotValue] Not use");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::getBestShotValue()
{
    CAM_LOGD( "[getBestShotValue] Not use");
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AfMgr::getSensorTemperature()
{
    MUINT32 u4temperature = 0;

    if ( m_ptrIHalSensor)
    {
        m_ptrIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_TEMPERATURE_VALUE, (MINTPTR)& u4temperature, 0, 0);
    }
    else
    {
        CAM_LOGE( "%s m_ptrIHalSensor is NULL", __FUNCTION__);
    }

    return u4temperature;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfMgr::getTimeStamp_us()
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp = ((t.tv_sec) * 1000000000LL + t.tv_nsec) / 1000;
    return timestamp; // from nano to us
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfMgr::MoveLensTo( MINT32 &i4TargetPos, MUINT32 u4Caller)
{
    MUINT64 TS_BeforeMoveMCU = 0; // timestamp of before moveMCU
    MUINT64 TS_AfterMoveMCU = 0;  // timestamp of after moveMCU

    if ( m_pMcuDrv)
    {
        {
            Mutex::Autolock lock(m_DACListLock);
            DACList.push_front(i4TargetPos);
            if (DACList.size() > 3)
            {
                DACList.pop();
            }
        }

        if ( m_i4DbgMotorDisable == 1)
        {
            if ( m_i4DbgMotorMPos < 1024 && m_i4DbgMotorMPos != m_i4DbgMotorMPosPre)
            {
                if (m_pMcuDrv->moveMCU( m_i4DbgMotorMPos))
                {
                    m_i4DbgMotorMPosPre = m_i4DbgMotorMPos;
                }
            }
        }
        else if ( m_i4MvLensTo != i4TargetPos)
        {
            if ( u4Caller == AF_MGR_CALLER)
            {
                CAM_LOGD("#(%5d,%5d) %s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }
            else
            {
                CAM_LOGD("#(%5d,%5d) cmd-%s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }

            CAM_LOGD_IF( m_i4DgbLogLv & 4, "%s+", __FUNCTION__);
            TS_BeforeMoveMCU = getTimeStamp_us();
            if (m_pMcuDrv->moveMCU( i4TargetPos))
            {
                m_i4MvLensTo = i4TargetPos;
            }
            TS_AfterMoveMCU = getTimeStamp_us();
            CAM_LOGD_IF( m_i4DgbLogLv & 4, "%s-", __FUNCTION__);
        }

        if ( m_i4DbgOISDisable == 1)
        {
            MUINT32 DbgOISPos = property_get_int32("vendor.debug.af_ois.position", 0);
            // DbgOISPos(XXXXYYYY) = X * 10000 + Y; Ex: 10000200
            if (m_i4DbgOISPos != DbgOISPos)
            {
                CAM_LOGD("Set OIS Position %d", DbgOISPos);
                m_pMcuDrv->setMCUParam(0x02, DbgOISPos);
                m_i4DbgOISPos = DbgOISPos;
            }
        }
    }
    else
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "%s Fail, Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    }

    if (TS_BeforeMoveMCU != 0)
    {
        // update time stamp of moveMCU
        return (MUINT64)((TS_BeforeMoveMCU + TS_AfterMoveMCU) / 2);
    }
    else
    {
        // inherit the original one
        return m_u8MvLensTS;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::readOTP(CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum)
{
    MUINT32 result = 0;
    CAM_CAL_DATA_STRUCT GetCamCalData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MUINT32 i4HalSensorDev = mapSensorIdxToDev(m_i4SensorIdx);

    CAM_LOGD("Read (%d) calibration data from EEPROM by camcal", enCamCalEnum);

    result = pCamCalDrvObj->GetCamCalCalData(i4HalSensorDev, enCamCalEnum, (void *)&GetCamCalData);
    CAM_LOGD_IF( m_i4DgbLogLv, "(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);

    if (enCamCalEnum == CAMERA_CAM_CAL_DATA_3A_GAIN)
    {
        if (result & CamCalReturnErr[enCamCalEnum])
        {
            CAM_LOGD( "err (%s)", CamCalErrString[enCamCalEnum]);
            //return E_AF_NOSUPPORT;
        }

        CAM_LOGD_IF( m_i4DgbLogLv,
                     "OTP data [S2aBitEn]%d [S2aAfBitflagEn]%d [S2aAf0]%d [S2aAf1]%d",
                     GetCamCalData.Single2A.S2aBitEn,
                     GetCamCalData.Single2A.S2aAfBitflagEn,
                     GetCamCalData.Single2A.S2aAf[0],
                     GetCamCalData.Single2A.S2aAf[1]);

        MINT32 i4InfPos, i4MacroPos, i450cmPos, i4MiddlePos;

        i4InfPos    = GetCamCalData.Single2A.S2aAf[0];
        i4MacroPos  = GetCamCalData.Single2A.S2aAf[1];
        i450cmPos   = GetCamCalData.Single2A.S2aAf[3];
        i4MiddlePos = GetCamCalData.Single2A.S2aAF_t.AF_Middle_calibration;

        if ( 0 < i4InfPos && i4MacroPos < 1024 && i4MacroPos > i4InfPos )
        {
            if( i4MiddlePos>i4InfPos && i4MiddlePos<i4MacroPos)
            {
                CAM_LOGD( "Middle OTP cal:%d\n", i4MiddlePos);
            }
            else
            {
                i4MiddlePos = i4InfPos + (i4MacroPos - i4InfPos) * m_ptrNVRam->rAFNVRAM.i4DualAFCoefs[1] / 100;
                CAM_LOGD( "Middle OTP adjust:%d NVRAM:%d INF:%d Mac:%d\n", i4MiddlePos, m_ptrNVRam->rAFNVRAM.i4DualAFCoefs[1], i4InfPos, i4MacroPos);
            }

            m_i4AFTabStr = 0; // get AF table start & end from updateAFtableBoundary
            m_i4AFTabEnd = 0;
            if ( m_pIAfAlgo)
            {
                m_pIAfAlgo->updateAFtableBoundary( i4InfPos, i4MacroPos, &m_i4AFTabStr, &m_i4AFTabEnd);
                m_pIAfAlgo->updateMiddleAFPos(i4MiddlePos);
            }

            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[0] == 10000 && m_i4LensPosExit != 0)
            {
                m_i4InitPos = m_i4LensPosExit;
            }
            else
            {
                MINT32 ratio = m_ptrNVRam->rAFNVRAM.i4EasyTuning[0] % 10000;
                m_i4InitPos = i4InfPos + (i4MacroPos - i4InfPos) * ratio / 100;
                m_i4InitPos = (m_i4InitPos < 0) ? 0 : ((m_i4InitPos > 1023) ? 1023 : m_i4InitPos);
            }

            MINT32 InitPosB4PostureComp = m_i4InitPos;
            if(m_pIAfAlgo)
            {
                m_pIAfAlgo->updatePosturedPos(m_i4InitPos, m_i4AcceInfo, SENSOR_ACCE_SCALE);
                CAM_LOGD_IF(m_i4DgbLogLv, "Posture Compensated InitPos : %d -> %d", InitPosB4PostureComp, m_i4InitPos);
            }
            else
            {
                CAM_LOGE("m_pIAfAlgo NULL");
            }
            // adjust depth dac_min, dac_max
            m_sDAF_TBL.af_dac_inf        = i4InfPos;
            m_sDAF_TBL.af_dac_marco      = i4MacroPos;
            m_sDAF_TBL.af_dac_min    = i4InfPos;
            m_sDAF_TBL.af_dac_max    = i4MacroPos;

            if ( (m_i4AFTabStr > 0) && ( m_i4AFTabEnd > m_i4AFTabStr))
            {
                m_sDAF_TBL.af_dac_min    = m_i4AFTabStr;
                m_sDAF_TBL.af_dac_max    = m_i4AFTabEnd;
            }

            m_sDAF_TBL.af_distance_inf   = GetCamCalData.Single2A.S2aAF_t.AF_infinite_pattern_distance;
            m_sDAF_TBL.af_distance_marco = GetCamCalData.Single2A.S2aAF_t.AF_Macro_pattern_distance;
            CAM_LOGD( "AF-%-15s: calibrated data [af_dac_inf]%d [af_dac_marco]%d [af_dac_min]%d [af_dac_max]%d [af_distance_inf]%d [af_distance_marco]%d\n",
                      __FUNCTION__,
                      (MINT32)m_sDAF_TBL.af_dac_inf,
                      (MINT32)m_sDAF_TBL.af_dac_marco,
                      (MINT32)m_sDAF_TBL.af_dac_min,
                      (MINT32)m_sDAF_TBL.af_dac_max,
                      (MINT32)m_sDAF_TBL.af_distance_inf,
                      (MINT32)m_sDAF_TBL.af_distance_marco);

            CAM_LOGD( "%s : [Inf]%d [Macro]%d [50cm]%d [InitPos]%d", __FUNCTION__, i4InfPos, i4MacroPos, i450cmPos, m_i4InitPos);

            if (m_bLDAFEn == MTRUE && m_pLaserMgr != NULL)
            {
                m_pLaserMgr->setAFCaliData(m_sDAF_TBL.af_dac_inf, m_sDAF_TBL.af_distance_inf,
                                           m_sDAF_TBL.af_dac_marco, m_sDAF_TBL.af_distance_marco,
                                           0, 0);
            }
        }
        else
        {
            // data from otp is abnormal
            i4InfPos   = 0;
            i4MacroPos = 1023;

            CAM_LOGD_IF( m_i4DgbLogLv,
                         "%s : cam_cal is abnormal, read calibration data from AF driver [Inf]%d [Macro]%d",
                         __FUNCTION__,
                         i4InfPos,
                         i4MacroPos);
        }
    }
    else if (enCamCalEnum == CAMERA_CAM_CAL_DATA_PDAF)
    {
        if (result == CAM_CAL_ERR_NO_ERR)
        {
            memcpy( m_ptrAfPara->rPdCaliData.uData, GetCamCalData.PDAF.Data, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
            m_ptrAfPara->rPdCaliData.i4Size = GetCamCalData.PDAF.Size_of_PDAF;

            CAM_LOGD("Size of PD calibration data = %d", m_ptrAfPara->rPdCaliData.i4Size);
            CAM_LOGD_IF(m_i4DgbLogLv & 2, "pd calibration data:");
            for (int index = 0; index < m_ptrAfPara->rPdCaliData.i4Size;)
            {
                CAM_LOGD_IF(m_i4DgbLogLv & 2, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                            m_ptrAfPara->rPdCaliData.uData[index + 0], m_ptrAfPara->rPdCaliData.uData[index + 1],
                            m_ptrAfPara->rPdCaliData.uData[index + 2], m_ptrAfPara->rPdCaliData.uData[index + 3],
                            m_ptrAfPara->rPdCaliData.uData[index + 4], m_ptrAfPara->rPdCaliData.uData[index + 5],
                            m_ptrAfPara->rPdCaliData.uData[index + 6], m_ptrAfPara->rPdCaliData.uData[index + 7],
                            m_ptrAfPara->rPdCaliData.uData[index + 8], m_ptrAfPara->rPdCaliData.uData[index + 9],
                            m_ptrAfPara->rPdCaliData.uData[index + 10], m_ptrAfPara->rPdCaliData.uData[index + 11],
                            m_ptrAfPara->rPdCaliData.uData[index + 12], m_ptrAfPara->rPdCaliData.uData[index + 13],
                            m_ptrAfPara->rPdCaliData.uData[index + 14], m_ptrAfPara->rPdCaliData.uData[index + 15]);
                index += 16;
            }
        }
        else
        {
            CAM_LOGE("FAILED getting pd calibration data from GetCamCalCalData()!");
        }
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::autoFocus()
{
    CAM_LOGD( "cmd-%s Dev %d : lib_afmode %d", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    if ( m_i4EnableAF == 0)
    {
        CAM_LOGD( "autofocus : dummy lens");
        return;
    }

    UpdateState( EVENT_CMD_AUTOFOCUS);

    if ( m_pIAfAlgo)
    {
        m_pIAfAlgo->targetAssistMove();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::cancelAutoFocus()
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d) lib_afmode(%d)",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eLIB3A_AFMode);

    //update parameters and status.
    UpdateState(EVENT_CMD_AUTOFOCUS_CANCEL);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::TimeOutHandle()
{
    CAM_LOGD( "#(%5d,%5d) SPECIAL_EVENT cmd-%s Dev(%d)",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev);
    if(m_pIAfAlgo)
        m_pIAfAlgo->cancel();
    UpdateState(EVENT_SEARCHING_END);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAperture( MFLOAT /*lens_aperture*/)
{}

MFLOAT AfMgr::getAperture()
{
    MFLOAT lens_aperture = 0;
    if ( m_bGetMetaData == MFALSE)
    {
        return 0;
    }

    for ( MUINT8 ii = 0; ii < m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_APERTURES).count(); ii++)
    {
        lens_aperture = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_APERTURES).itemAt( ii, Type2Type<MFLOAT>());
        CAM_LOGD( "AFmeta APERTURES %d  %f", ii, lens_aperture);
    }
    return lens_aperture;
}
MVOID AfMgr::setFilterDensity( MFLOAT /*lens_filterDensity*/)
{}

MFLOAT AfMgr::getFilterDensity()
{
    MFLOAT lens_filterDensity = 0;
    if ( m_bGetMetaData == MFALSE)
    {
        return 0;
    }

    for (MUINT8 ii = 0; ii < m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES).count(); ii++)
    {
        lens_filterDensity = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES).itemAt( ii, Type2Type<MFLOAT>());
        CAM_LOGD( "AFmeta FILTER_DENSITIES %d  %f", ii, lens_filterDensity);
    }
    return lens_filterDensity;
}
MVOID AfMgr::setFocalLength( MFLOAT /*lens_focalLength*/)
{}

MFLOAT AfMgr::getFocalLength ()
{
    MFLOAT lens_focalLength = 34.0;
    if ( m_bGetMetaData == MFALSE)
    {
        return 0;
    }

    for ( MUINT8 ii = 0; ii < m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS).count(); ii++)
    {
        lens_focalLength = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS).itemAt( ii, Type2Type<MFLOAT>());
        CAM_LOGD( "AFmeta APERTURES %d  %f", ii, lens_focalLength);
    }
    return lens_focalLength;
}

MVOID AfMgr::setFocusDistance( MFLOAT lens_focusDistance)
{
    if ( lens_focusDistance < 0)
    {
        return;
    }
    MINT32 i4ResultDac = 0;
    // data from OTP
    MFLOAT fDAC_Inf        = (MFLOAT)m_sDAF_TBL.af_dac_inf; // unit:DAC
    MFLOAT fDAC_Macro      = (MFLOAT)m_sDAF_TBL.af_dac_marco; // unit:DAC
    MFLOAT fDistance_Inf   = (MFLOAT)m_sDAF_TBL.af_distance_inf; // unit:mm
    MFLOAT fDistance_Macro = (MFLOAT)m_sDAF_TBL.af_distance_marco; // unit:mm
    MFLOAT fDist_Inf_rev   = 0.0;
    MFLOAT fDist_Macro_rev = 0.0;
    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF)
    {
        m_fMfFocusDistance = lens_focusDistance;

        // set fd_inf and fd_macro to default if there's no data in OTP
        if (fDistance_Inf <= fDistance_Macro)
        {
            fDistance_Inf = 5000;
            fDistance_Macro = 100;
        }
        fDist_Inf_rev = 1000.0 / fDistance_Inf;
        fDist_Macro_rev = 1000.0 / fDistance_Macro;

        // focusDistance -> DAC
        i4ResultDac = (MINT32)(fDAC_Macro + ((lens_focusDistance - fDist_Macro_rev) / (fDist_Inf_rev - fDist_Macro_rev) * (fDAC_Inf - fDAC_Macro)));
    }
    else
    {
        m_fMfFocusDistance = -1;
    }

    // API2: At MTK_CONTROL_AF_MODE_OFF mode, configure algorithm as MF mode.
    //          The auto-focus routine does not control the lens. Lens is controlled by the application.
    if ( (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
            (m_sAFOutput.i4AFPos != i4ResultDac) &&
            (0 <= i4ResultDac) &&
            (i4ResultDac <= 1023))
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d) DAC(%d->%d) Dist(%f); fDist_Macro_rev %f, fDist_Inf_rev %f, fDAC_Macro %f, fDAC_Inf %f",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_sAFOutput.i4AFPos,
                  i4ResultDac,
                  lens_focusDistance,
                  fDist_Macro_rev,
                  fDist_Inf_rev,
                  fDAC_Macro,
                  fDAC_Inf);

        m_i4MFPos = i4ResultDac;

        if (m_pIAfAlgo)
        {
            m_pIAfAlgo->setMFPos(m_i4MFPos);
            m_pIAfAlgo->trigger();
        }
        else
        {
            CAM_LOGD("Null m_pIAfAlgo");
        }
    }
    else
    {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF)
        {
            CAM_LOGD("#(%5d,%5d) cmd-%s Dev(%d) !!SKIP!! DAC(%d->%d) Dist(%f) lib_afmode(%d)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     m_sAFOutput.i4AFPos,
                     i4ResultDac,
                     lens_focusDistance,
                     m_eLIB3A_AFMode);
        }
    }
}

MFLOAT AfMgr::getFocusDistance()
{
    if ( m_i4EnableAF == 0)
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "cmd-%s no AF", __FUNCTION__);
        return 0.0;
    }

    MINT32 LensPos;
    if ( m_eLIB3A_AFMode == LIB3A_AF_MODE_MF)
    {
        /* In MF mode, algorithm will take some step to target position.
                  So directly using MFpos instead of using m_sAFOutput.i4AFPos.*/
        LensPos = m_i4MFPos;

        // setFocusDistance followed by getFocusDistance in MFMode
        if (m_fMfFocusDistance != -1)
        {
            return m_fMfFocusDistance;
        }
    }
    else
    {
        LensPos = m_sAFOutput.i4AFPos;
    }

    // Lens position is changed, calculating new focus distance :
    if ( m_sFocusDis.i4LensPos != LensPos)
    {
        // data from OTP
        MFLOAT fDAC_Inf        = (MFLOAT)m_sDAF_TBL.af_dac_inf; // unit:DAC
        MFLOAT fDAC_Macro      = (MFLOAT)m_sDAF_TBL.af_dac_marco; // unit:DAC
        MFLOAT fDistance_Inf   = (MFLOAT)m_sDAF_TBL.af_distance_inf; // unit:mm
        MFLOAT fDistance_Macro = (MFLOAT)m_sDAF_TBL.af_distance_marco; // unit:mm
        // current lensPos in DAC
        MFLOAT fCurLensPos     = (MFLOAT)LensPos;

        // set fd_inf and fd_macro to default if there's no data in OTP
        if (fDistance_Inf <= fDistance_Macro)
        {
            fDistance_Inf = 5000;
            fDistance_Macro = 100;
        }
        // (Linear Interpolation) : distance_xxx_rev is linear to DAC
        MFLOAT fDist_Inf_rev = 1000.0 / fDistance_Inf;
        MFLOAT fDist_Macro_rev = 1000.0 / fDistance_Macro;
        // DAC -> FocusDistance
        MFLOAT fDist_rev = fDist_Macro_rev + (fCurLensPos - fDAC_Macro) / (fDAC_Inf - fDAC_Macro) * (fDist_Inf_rev - fDist_Macro_rev);

        CAM_LOGD("cmd-%s Pos %d->%d, Dis %f->%f; fDist_Macro_rev %f, fDist_Inf_rev %f, fDAC_Macro %f, fDAC_Inf %f",
                 __FUNCTION__,
                 m_sFocusDis.i4LensPos,
                 LensPos,
                 m_sFocusDis.fDist,
                 fDist_rev,
                 fDist_Macro_rev,
                 fDist_Inf_rev,
                 fDAC_Macro,
                 fDAC_Inf);

        //record.
        m_sFocusDis.i4LensPos = LensPos;
        m_sFocusDis.fDist = fDist_rev;
    }

    return m_sFocusDis.fDist;
}

MVOID AfMgr::setOpticalStabilizationMode( MINT32 ois_OnOff)
{
    if ( m_pMcuDrv)
    {
        if ( ois_OnOff == TRUE)
        {
            m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MFALSE);
        }
        else
        {
            m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MTRUE);
        }
    }

    MUINT8 oismode = 0;
    if ( m_bGetMetaData == MFALSE)
    {
        return;
    }

    for ( MUINT8 ii = 0; ii < m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).count(); ii++)
    {
        oismode = m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).itemAt( ii, Type2Type<MUINT8>());
        CAM_LOGD( "AFmeta OPTICAL_STABILIZATION %d  %d", ii, oismode);
        if (oismode == 1)
        {
            //set_ois_drv_on(ois_OnOff);
            break;
        }
    }
}

MINT32 AfMgr::getOpticalStabilizationMode()
{
    MUINT8 oismode = 0;
    if ( m_bGetMetaData == MFALSE)
    {
        return 0;
    }

    for ( MUINT8 ii = 0; ii < m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).count(); ii++)
    {
        oismode = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).itemAt( ii, Type2Type< MUINT8 >());
        CAM_LOGD( "AFmeta OPTICAL_STABILIZATION %d  %d", ii, oismode);
    }
    return oismode;/*OFF, ON */
}

MVOID AfMgr::getFocusRange( MFLOAT *vnear, MFLOAT *vfar)
{
    MINT32 i4tblLL;
    MINT32 fdacidx       = 0;
    MINT32 i4ResultDist  = 100;
    MINT32 i4ResultRange = 100;
    MINT32 i4ResultNear  = 100;
    MINT32 i4ResultFar   = 100;
    MFLOAT retDist __unused       = 0.0;

    if ( m_i4EnableAF == 0)
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "no focus lens");
        *vnear = 1 / 0.6;
        *vfar  = 1 / 3.0;
        return;
    }

    i4tblLL = m_pAFParam->i4TBLL;
    for ( fdacidx = 0; fdacidx < i4tblLL; fdacidx++)
    {
        if ( m_sAFOutput.i4AFPos > m_pAFParam->i4Dacv[fdacidx])
            break;
    }

    if (fdacidx == 0)
    {
        i4ResultDist = m_pAFParam->i4Dist[0];
    }
    else
    {
        i4ResultDist =
            ( m_pAFParam->i4Dist[fdacidx  ] * (m_pAFParam->i4Dacv[fdacidx - 1] - m_sAFOutput.i4AFPos       )  +
              m_pAFParam->i4Dist[fdacidx - 1] * (m_sAFOutput.i4AFPos          - m_pAFParam->i4Dacv[fdacidx])) /
            ( m_pAFParam->i4Dacv[fdacidx - 1] - m_pAFParam->i4Dacv[fdacidx]);

        i4ResultRange =
            ( m_pAFParam->i4FocusRange[fdacidx  ] * (m_pAFParam->i4Dacv[fdacidx - 1] - m_sAFOutput.i4AFPos       )  +
              m_pAFParam->i4FocusRange[fdacidx - 1] * (m_sAFOutput.i4AFPos          - m_pAFParam->i4Dacv[fdacidx])) /
            ( m_pAFParam->i4Dacv[fdacidx - 1] - m_pAFParam->i4Dacv[fdacidx]);
    }
    if ( i4ResultDist <= 0)  i4ResultDist = m_pAFParam->i4Dist[i4tblLL - 1];
    if ( i4ResultRange <= 0) i4ResultRange = m_pAFParam->i4Dist[i4tblLL - 1];

    i4ResultNear = i4ResultDist - (i4ResultRange / 2);
    i4ResultFar  = i4ResultDist + (i4ResultRange / 2);

    *vnear = 1000.0 / ((MFLOAT)i4ResultNear);
    *vfar  = 1000.0 / ((MFLOAT)i4ResultFar);

    //CAM_LOGD("[getFocusRange] [%f, %f]", *vnear,*vfar);
}

MINT32 AfMgr::getLensState  ()
{
    MINT32 ret = 0;
    MINT32 IsAFSuspend = ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsAFSuspend, NULL, NULL);

    if (IsAFSuspend != MFALSE)
    {
        ret = 0;
        CAM_LOGW("%s CCU Suspend: LensState = %d", __FUNCTION__, ret);
    }
    else if (m_i4GyroValue > GYRO_THRESHOLD && m_sAFOutput.i4IsAfSearch == AF_SEARCH_TARGET_MOVE)
    {
        // FAKE STATIONARY : Phone Moving while TRACKING
        ret = 0;
        if (m_bForceCapture == 0)
        {
            CAM_LOGW("%s Phone Moving while TRACKING, ForceCapture: LensState = %d", __FUNCTION__, ret);
        }
        m_bForceCapture = 1;
        m_i4ContinuePDMovingCount = 0;
    }
    else if (m_sAFOutput.i4IsAfSearch == AF_SEARCH_TARGET_MOVE)
    {
        m_i4ContinuePDMovingCount += (m_i4ContinuePDMovingCount < MAX_PDMOVING_COUNT) ? 1 : 0;
        if (m_i4ContinuePDMovingCount >= MAX_PDMOVING_COUNT)
        {
            // FAKE STATIONARY : Keep PDMoving with the stable phone for more than 6 frames
            ret = 0;
            if (m_bForceCapture == 0)
            {
                CAM_LOGW("%s Keep PDMoving for more than %d frames, ForceCapture: LensState = %d", __FUNCTION__, MAX_PDMOVING_COUNT, ret);
            }
            m_bForceCapture = 1;
        }
        else
        {
            // MOVING : PDMoving
            ret = 1;
            m_bForceCapture = 0;
        }
    }
    else if (m_sAFOutput.i4IsAfSearch != AF_SEARCH_DONE)
    {
        // MOVING : FineSearch or ContrastAF
        ret = 1;
        m_i4ContinuePDMovingCount = 0;
        m_bForceCapture = 0;
    }
    else
    {
        // STATIONARY
        ret = 0;
        m_i4ContinuePDMovingCount = 0;
        m_bForceCapture = 0;
    }


    m_i4MvLensToPre = m_i4MvLensTo;

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::setSensorMode( MINT32 &i4NewSensorMode, MUINT32 &i4BINInfo_SzW, MUINT32 &i4BINInfo_SzH)
{
    m_i4SensorMode = i4NewSensorMode;
    m_i4BINSzW = i4BINInfo_SzW;
    m_i4BINSzH = i4BINInfo_SzH;

    CAM_LOGD( "cmd-%s Dev %d, Mode %d, BINSZ(%d, %d)\n",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4SensorMode,
              m_i4BINSzW,
              m_i4BINSzH);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::PrintHWRegSetting( AF_CONFIG_T &sAFHWCfg)
{
    CAM_LOGD( "HW-TGSZ %d, %d, BINSZ %d, %d",
              sAFHWCfg.sTG_SZ.i4W,
              sAFHWCfg.sTG_SZ.i4H,
              sAFHWCfg.sBIN_SZ.i4W,
              sAFHWCfg.sBIN_SZ.i4H);

    CAM_LOGD( "HW-sROI %d, %d %d, %d %d",
              sAFHWCfg.sRoi.i4X,
              sAFHWCfg.sRoi.i4Y,
              sAFHWCfg.sRoi.i4W,
              sAFHWCfg.sRoi.i4H,
              sAFHWCfg.sRoi.i4Info);

    CAM_LOGD( "HW-nBLK %d, %d",
              sAFHWCfg.AF_BLK_XNUM,
              sAFHWCfg.AF_BLK_YNUM);

    CAM_LOGD( "HW-SGG %d, %d, %d, %d, %d, %d, %d, %d",
              sAFHWCfg.i4SGG_GAIN,
              sAFHWCfg.i4SGG_GMR1,
              sAFHWCfg.i4SGG_GMR2,
              sAFHWCfg.i4SGG_GMR3,
              sAFHWCfg.i4SGG_GMR4,
              sAFHWCfg.i4SGG_GMR5,
              sAFHWCfg.i4SGG_GMR6,
              sAFHWCfg.i4SGG_GMR7);


    CAM_LOGD( "HW-HVGL %d, %d, %d",
              sAFHWCfg.AF_H_GONLY,
              sAFHWCfg.AF_V_GONLY,
              sAFHWCfg.AF_V_AVG_LVL);

    CAM_LOGD( "HW-BLF %d, %d, %d, %d",
              sAFHWCfg.AF_BLF[0],
              sAFHWCfg.AF_BLF[1],
              sAFHWCfg.AF_BLF[2],
              sAFHWCfg.AF_BLF[3]);

    CAM_LOGD( "HW-TH %d, %d, %d, %d",
              sAFHWCfg.AF_TH_H[0],
              sAFHWCfg.AF_TH_H[1],
              sAFHWCfg.AF_TH_V,
              sAFHWCfg.AF_TH_G_SAT);

    CAM_LOGD( "HW-FIL0 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
              sAFHWCfg.AF_FIL_H0[0],
              sAFHWCfg.AF_FIL_H0[1],
              sAFHWCfg.AF_FIL_H0[2],
              sAFHWCfg.AF_FIL_H0[3],
              sAFHWCfg.AF_FIL_H0[4],
              sAFHWCfg.AF_FIL_H0[5],
              sAFHWCfg.AF_FIL_H0[6],
              sAFHWCfg.AF_FIL_H0[7],
              sAFHWCfg.AF_FIL_H0[8],
              sAFHWCfg.AF_FIL_H0[9],
              sAFHWCfg.AF_FIL_H0[10],
              sAFHWCfg.AF_FIL_H0[11]);

    CAM_LOGD( "HW-FIL1 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
              sAFHWCfg.AF_FIL_H1[0],
              sAFHWCfg.AF_FIL_H1[1],
              sAFHWCfg.AF_FIL_H1[2],
              sAFHWCfg.AF_FIL_H1[3],
              sAFHWCfg.AF_FIL_H1[4],
              sAFHWCfg.AF_FIL_H1[5],
              sAFHWCfg.AF_FIL_H1[6],
              sAFHWCfg.AF_FIL_H1[7],
              sAFHWCfg.AF_FIL_H1[8],
              sAFHWCfg.AF_FIL_H1[9],
              sAFHWCfg.AF_FIL_H1[10],
              sAFHWCfg.AF_FIL_H1[11]);

    CAM_LOGD( "HW-FILV %d, %d, %d, %d",
              sAFHWCfg.AF_FIL_V[0],
              sAFHWCfg.AF_FIL_V[1],
              sAFHWCfg.AF_FIL_V[2],
              sAFHWCfg.AF_FIL_V[3]);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::ConfigHWReg( AF_CONFIG_T &sInHWCfg, AREA_T &sOutHWROI, MINT32 &i4OutHWBlkNumX, MINT32 &i4OutHWBlkNumY, MINT32 &i4OutHWBlkSizeX, MINT32 &i4OutHWBlkSizeY)
{
    //-------------
    //AF HAL control flow :
    //-------------
    if (m_i4BINSzW != 0 && m_i4TGSzW != 0)
    {
        sInHWCfg.sTG_SZ.i4W  = m_i4TGSzW;
        sInHWCfg.sTG_SZ.i4H  = m_i4TGSzH;
        sInHWCfg.sBIN_SZ.i4W = m_i4BINSzW;
        sInHWCfg.sBIN_SZ.i4H = m_i4BINSzH;
    }
    else
    {
        CAM_LOGE( "%s : [TG_SZ]%d %d [BIN_SZ]%d %d -> [TG_SZ]%d %d [BIN_SZ]%d %d",
                  __FUNCTION__,
                  m_i4TGSzW,
                  m_i4TGSzH,
                  m_i4BINSzW,
                  m_i4BINSzH,
                  sInHWCfg.sTG_SZ.i4W,
                  sInHWCfg.sTG_SZ.i4H,
                  sInHWCfg.sBIN_SZ.i4W,
                  sInHWCfg.sBIN_SZ.i4H);
    }

    CAM_LOGD_IF( m_i4DgbLogLv,
                 "#(%5d,%5d) %s Dev(%d): set(X,Y,W,H)=( %d, %d, %d, %d)",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 sInHWCfg.sRoi.i4X,
                 sInHWCfg.sRoi.i4Y,
                 sInHWCfg.sRoi.i4W,
                 sInHWCfg.sRoi.i4H);

    /**
     * configure HW :
     * Output parameters :
     * Because HW constraint is applied, HW setting maybe be changed.
     * sOutHWROI is used to align HW analyzed region to any other algorithm, for example, phase difference algorithm.
     */
    MINT32 i4FirstTimeConfig = (m_eEvent & EVENT_CMD_START) ? 1 : 0;
    ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).config(sInHWCfg, sOutHWROI, i4OutHWBlkNumX, i4OutHWBlkNumY, i4OutHWBlkSizeX, i4OutHWBlkSizeY, i4FirstTimeConfig);

    m_i4HWEnExtMode = sInHWCfg.AF_EXT_ENABLE;

    // error log : should not be happened.
    if ( sInHWCfg.AF_BLK_XNUM != i4OutHWBlkNumX  ||
            sInHWCfg.AF_BLK_YNUM != i4OutHWBlkNumY )
    {
        CAM_LOGE( "WAR-ROI : [X]%d [Y]%d [W]%d [H]%d -> [X]%d [Y]%d [W]%d [H]%d, [XNUM] %d->%d, [YNUM] %d->%d",
                  sInHWCfg.sRoi.i4X,
                  sInHWCfg.sRoi.i4Y,
                  sInHWCfg.sRoi.i4W,
                  sInHWCfg.sRoi.i4H,
                  sOutHWROI.i4X,
                  sOutHWROI.i4Y,
                  sOutHWROI.i4W,
                  sOutHWROI.i4H,
                  sInHWCfg.AF_BLK_XNUM,
                  i4OutHWBlkNumX,
                  sInHWCfg.AF_BLK_YNUM,
                  i4OutHWBlkNumY);
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatStart()
{
    m_i4ReadOutTimePerLine = 0;
    m_i4P1TimeReadOut      = 0;

    if (m_sAFInput.PixelClk > 0)
    {
        m_i4ReadOutTimePerLine = (MINT32)(((MINT64)m_sAFInput.PixelInLine * 1000000000) / m_sAFInput.PixelClk); // unit: ns
        m_i4P1TimeReadOut      = (MINT32)(m_i4ReadOutTimePerLine * m_i4TGSzH / 1000); // unit:us
    }
    CAM_LOGD( "AF-%-15s: ReadOutTimePerLine(%d), P1DoneTime(%d)", __FUNCTION__, m_i4ReadOutTimePerLine, m_i4P1TimeReadOut);

    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).start(m_i4CurrSensorDev, m_i4SensorIdx);

    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_SetTimeReadOutPerLine, m_i4ReadOutTimePerLine, m_i4P1TimeReadOut);

    // First time HW setting is got after calling initAF.
    ConfigHWReg( m_sAFOutput.sAFStatConfig, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatStop()
{
    //uninitial isp_mgr_af_stat for configure HW
    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).stop();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::IspMgrAFStatHWPreparing()
{
    MBOOL bSkip   = MTRUE;
    MBOOL isHWRdy = MTRUE;

    if( m_eCurAFMode == MTK_CONTROL_AF_MODE_OFF)
    {
        /*
         * AF off mode don't need to check statistic data, but need run handleAF.
         */
        bSkip = MFALSE;
    }
    else if (m_bForceDoAlgo == MTRUE)
    {
        bSkip = MFALSE;
        CAM_LOGD("%s ForceDoAlgo to speed up SEARCH_END", __FUNCTION__);
    }
    else if (m_bIsAPROITooLarge == MTRUE)
    {
        m_bRunAFByTwoFrame = 1 - m_bRunAFByTwoFrame;
        bSkip = m_bRunAFByTwoFrame;
    }
    else
    {
        /**
         * The condition AF_ROI_SEL_NONE is for the first time lunch camera only. [Ref] SelROIToFocusing
         */
        isHWRdy = ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).isHWRdy( m_u4ConfigHWNum);
        if (m_sAFOutput.i4ZECChg == 1 || m_sAFOutput.i4IsTargetAssitMove == 1)
        {
            // algo requirement : do algo
            bSkip = MFALSE;
        }
        else if (isHWRdy || m_sAFOutput.i4ROISel == AF_ROI_SEL_NONE)
        {
            // flow requirement : do algo
            bSkip = MFALSE;
        }
    }


    if ( m_i4DgbLogLv || bSkip == MTRUE)
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;

        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d) Config(%d) ", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, m_u4ConfigHWNum);

        ptrMsgBuf += sprintf( ptrMsgBuf,
                              "bSkip(%d) PauseAF(%d) Afmode(%d) VsyncUpdate(%d) IsTargetAssitMove(%d) isHWRdy(%d) ROISel(%d) ZECChg(%d) DbgPDVerifyEn(%d) IsAPROITooLarge(%d)",
                              bSkip,
                              m_bPauseAF,
                              m_eCurAFMode,
                              m_i4VsyncUpdate,
                              m_sAFOutput.i4IsTargetAssitMove,
                              isHWRdy,
                              m_sAFOutput.i4ROISel,
                              m_sAFOutput.i4ZECChg,
                              m_i4DbgPDVerifyEn,
                              m_bIsAPROITooLarge);
        CAM_LOGD( "%s", dbgMsgBuf);
    }



    return bSkip;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatUpdateHw()
{
    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsZoomEffectChange, m_sAFOutput.i4ZECChg, NULL);

    ConfigHWReg( m_sAFOutput.sAFStatConfig, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);
}

MVOID AfMgr::IspMgrAFGetROIFromHw(AREA_T &Area, MUINT32 &isTwin)
{
    ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getROIFromHw(Area, isTwin);
    // transfer AFROI(BINSize) to AFROI(TGSize)
    if (m_i4BINSzW != 0)
    {
        Area.i4X *= m_i4TGSzW / m_i4BINSzW;
        Area.i4Y *= m_i4TGSzW / m_i4BINSzW;
        Area.i4W *= m_i4TGSzW / m_i4BINSzW;
        Area.i4H *= m_i4TGSzW / m_i4BINSzW;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::VsyncUpdate()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info)
{
    CAM_LOGD_IF(m_i4DgbLogLv & 4, "[%s] pSem(%p), reltime(%lld), info(%s)", __FUNCTION__, pSem, (long long)reltime, info);
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        CAM_LOGE("error in clock_gettime! Please check\n");

    ts.tv_sec  += reltime / 1000000000;
    ts.tv_nsec += reltime % 1000000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }

    int s = sem_timedwait(pSem, &ts);
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
        {
            return MFALSE;
        }
        else
        {
            CAM_LOGD_IF(m_i4DgbLogLv, "[%s][%s]sem_timedwait() errno = %d\n", __FUNCTION__, info, errno);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::updateSensorListenerParams( MINT32 *i4SensorInfo __unused)
{
#if 0
    GyroSensor_Param_T *pGyroSensorParam = reinterpret_cast<GyroSensor_Param_T *>(i4SensorInfo);

    m_u8PreAcceTS = m_u8AcceTS;
    m_i4AcceInfo[0] = pGyroSensorParam->i4AcceInfo[0];
    m_i4AcceInfo[1] = pGyroSensorParam->i4AcceInfo[1];
    m_i4AcceInfo[2] = pGyroSensorParam->i4AcceInfo[2];
    m_u8AcceTS = pGyroSensorParam->u8AcceTS;

    m_u8PreGyroTS = m_u8GyroTS;
    m_i4GyroInfo[0] = pGyroSensorParam->i4GyroInfo[0];
    m_i4GyroInfo[1] = pGyroSensorParam->i4GyroInfo[1];
    m_i4GyroInfo[2] = pGyroSensorParam->i4GyroInfo[2];
    m_u8GyroTS = pGyroSensorParam->u8GyroTS;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SetCurFrmNum( MUINT32 u4FrmNum, MUINT32 u4FrmNumCur __unused)
{
    // Request
    m_u4ReqMagicNum = u4FrmNum;
    // Statistic - u4FrmNumCur
    m_eEvent = EVENT_NONE;
    //
    MRESULT res = IPDMgr::getInstance().setRequest(m_i4CurrSensorDev, u4FrmNum);
    CAM_LOGD_IF( m_i4DgbLogLv&2, "Req #%5d, %d", m_u4ReqMagicNum, res);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getDAFtbl( MVOID **ptbl)
{
    *ptbl = &m_sDAF_TBL;
    return DAF_TBL_QLEN;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::setMultiZoneEnable( MUINT8 bEn)
{
    if ( (bEn == 0 || bEn == 1) && m_bMZHostEn != bEn)
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_bMZHostEn, bEn);

        m_bMZHostEn = bEn;
        m_bMZAFEn     = m_i4DbgMZEn || (m_bMZHostEn == 1) ? MTRUE : MFALSE;

        //Accroding to option, updating center ROI coordinate automatically
        UpdateCenterROI( m_sArea_Center);

        //Algo limit : algo need to be retriggered if mz is enabled while af searching
        if (bEn == 1 && m_pIAfAlgo)
        {
            CAM_LOGD("retrigger algo if mz is enabled while af searching");
            m_pIAfAlgo->cancel();
            m_pIAfAlgo->trigger();
        }

        //Reset all focusing window.
        m_sArea_Focusing = m_sArea_APCheck = m_sArea_APCmd = m_sArea_OTFD = m_sArea_HW = m_sArea_Center;
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_Init()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);
    // --- init Laser ---
    if (m_bLDAFEn == MFALSE)
    {
        m_pLaserMgr = LaserMgr::getInstance(m_i4CurrSensorDev);

        if (m_pLaserMgr != NULL)
        {
            if (m_pLaserMgr->init() != MFALSE)
            {
                m_bLDAFEn = MTRUE;
                CAM_LOGD("[%s] LaserMgr init() done", __FUNCTION__);
            }
            else
            {
                CAM_LOGD("[%s] LaserMgr init() fail", __FUNCTION__);
            }
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_Start()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);

    if (m_bLDAFEn == MTRUE && m_pLaserMgr != NULL)
    {
        if ( property_get_int32("vendor.laser.calib.disable", 0) == 0 )
        {
            MINT32 i4OffsetData = m_ptrNVRam->rAFNVRAM.i4Revs[201];
            MINT32 i4XTalkData  = m_ptrNVRam->rAFNVRAM.i4Revs[202];
            m_pLaserMgr->setLaserCaliData(i4OffsetData, i4XTalkData);
        }

        MINT32 i4LaserMaxDisRatio = m_ptrNVRam->rAFNVRAM.i4Revs[200];
        MINT32 i4LaserMaxDistance = m_ptrNVRam->rAFNVRAM.i4Revs[203];
        MINT32 i4LaserTableNum    = m_ptrNVRam->rAFNVRAM.i4Revs[204];
        m_pLaserMgr->setLaserGoldenTable(&m_ptrNVRam->rAFNVRAM.i4Revs[205], i4LaserTableNum, i4LaserMaxDistance, i4LaserMaxDisRatio);

        m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;

        if (m_pLaserMgr->checkHwSetting() == 0 || property_get_int32("vendor.laser.disable", 0) == 1)
        {
            m_pLaserMgr->uninit();
            m_pLaserMgr = NULL;
            m_bLDAFEn = MFALSE;
            CAM_LOGE( "AF-%-15s: LaserMgr checkHwSetting() fail", __FUNCTION__);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_CamPwrOffState()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);

    if (m_bLDAFEn == MTRUE && m_pLaserMgr != NULL)
    {
        CAM_LOGD( "[%s] LaserMgr uninit() done", __FUNCTION__);
        m_pLaserMgr->uninit();
        m_pLaserMgr = NULL;
        m_bLDAFEn = MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_Update()
{
    if (m_bLDAFEn == MTRUE && m_pLaserMgr != NULL)
    {
        MINT32 i4LaserStatus = m_pLaserMgr->getLaserCurStatus();

        if (i4LaserStatus == STATUS_RANGING_VALID)
        {
            m_sAFInput.sLaserInfo.i4CurPosDAC   = m_pLaserMgr->getLaserCurDac();
            m_sAFInput.sLaserInfo.i4CurPosDist  = m_pLaserMgr->getLaserCurDist();
        }
        else
        {
            m_sAFInput.sLaserInfo.i4CurPosDAC   = m_pLaserMgr->predictAFStartPosDac();
            m_sAFInput.sLaserInfo.i4CurPosDist  = m_pLaserMgr->predictAFStartPosDist();
            m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SDAF_Init()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);
    m_sDAF_TBL.is_daf_run = E_DAF_OFF;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SDAF_Start()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);

    if ( m_i4CurrLensId != 0xffff)
    {
        MINT32 AFtbl_Num;
        MINT32 AFtbl_Marco;
        MINT32 AFtbl_Inf;

        m_bSDAFEn = (m_ptrNVRam->rAFNVRAM.i4SDAFCoefs[1] > 0) ? MTRUE : MFALSE;

        AFtbl_Num   = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
        AFtbl_Inf   = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
        AFtbl_Marco = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[AFtbl_Num - 1];

        m_sDAF_TBL.af_dac_min        = AFtbl_Inf;
        m_sDAF_TBL.af_dac_max        = AFtbl_Marco;
        m_sDAF_TBL.af_dac_inf        = AFtbl_Inf;
        m_sDAF_TBL.af_dac_marco      = AFtbl_Marco;
        m_sDAF_TBL.af_distance_inf   = 0;
        m_sDAF_TBL.af_distance_marco = 0;

        CAM_LOGD( "AF-%-15s: default data [af_dac_inf]%d [af_dac_marco]%d [af_dac_min]%d [af_dac_max]%d [af_distance_inf]%d [af_distance_marco]%d\n",
                  __FUNCTION__,
                  (MINT32)m_sDAF_TBL.af_dac_inf,
                  (MINT32)m_sDAF_TBL.af_dac_marco,
                  (MINT32)m_sDAF_TBL.af_dac_min,
                  (MINT32)m_sDAF_TBL.af_dac_max,
                  (MINT32)m_sDAF_TBL.af_distance_inf,
                  (MINT32)m_sDAF_TBL.af_distance_marco);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SDAF_Update()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SDAF_Result()
{
    //==========
    // Prepare AF info to Vsdof
    //==========
    if ( m_sDAF_TBL.is_daf_run & E_DAF_RUN_STEREO)
    {
        MUINT32 MagicNum, CurDafTblIdx;

        MagicNum = m_u4StaMagicNum + 1; // m_sAFOutput can match next image status

        CurDafTblIdx = MagicNum % DAF_TBL_QLEN;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].frm_mun          = MagicNum;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_mode          = m_eCurAFMode;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_roi_sel       = m_sAFOutput.i4ROISel;
        // Contrast AF output info
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_pos       = m_sAFOutput.i4AFPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].thermal_lens_pos = m_sAFOutput.i4ThermalLensPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].posture_dac      = m_sAFOutput.i4PostureDac;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_index     = m_sAFOutput.i4AfDacIndex;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_af_stable     = ((m_i4IsAFSearch_CurState == AF_SEARCH_DONE) && m_sCurLensInfo.bIsMotorOpen) ? 1 : 0;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_scene_stable  = !m_sAFOutput.i4IsSceneChange;

        if (m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
        {
            if (m_sArea_TypeSel == AF_ROI_SEL_AP)
            {
                m_sArea_Bokeh = m_sArea_Focusing;
            }
            else if (m_sArea_TypeSel == AF_ROI_SEL_FD)
            {
                // use the current face roi to avoid the finally inconsistency between depth point and focusing roi
                m_sArea_Bokeh = m_sArea_OTFD_No_Extend; //Passing FDROI w/o extend instead of m_sArea_OTFD to Bokeh;
            }
            else
            {
                if (m_i4IsAFSearch_CurState == AF_SEARCH_CONTRAST)
                {
                    m_sArea_Bokeh = m_sArea_Focusing;
                }
                else
                {
                    m_sArea_Bokeh = m_sAFOutput.sPDWin;
                }
            }
        }

        if ((m_sArea_Bokeh.i4W == 0) || (m_sArea_Bokeh.i4H == 0))
        {
            m_sArea_Bokeh = m_sArea_Center;
        }

        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x = (MUINT16)m_sArea_Bokeh.i4X;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y = (MUINT16)m_sArea_Bokeh.i4Y;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x   = (MUINT16)m_sArea_Bokeh.i4X + m_sArea_Bokeh.i4W;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y   = (MUINT16)m_sArea_Bokeh.i4Y + m_sArea_Bokeh.i4H;

        CAM_LOGD_IF(m_i4DgbLogLv, "[%s] #%d X1(%d) X2(%d) Y1(%d) Y2(%d), isAFStable(%d), DAC(%d)", __FUNCTION__, CurDafTblIdx,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].is_af_stable, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_pos);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SensorProvider_getData()
{
    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    //-------------------------------------------------------------------------------------------------------------
    MUINT32 u4ACCEScale = 0, u4GyroScale = 0;

    if (m_ptrNVRam->rAFNVRAM.i4SensorEnable > 0)
    {
        // get gyro/acceleration data
        SensorData gyroDa;
        MBOOL gyroDaVd = m_bGryoVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_GYRO, gyroDa) : MFALSE;

        if ( gyroDaVd && gyroDa.timestamp)
        {
            m_u8PreGyroTS = m_u8GyroTS;
            m_i4GyroInfo[0] = gyroDa.gyro[0] * SENSOR_GYRO_SCALE;
            m_i4GyroInfo[1] = gyroDa.gyro[1] * SENSOR_GYRO_SCALE;
            m_i4GyroInfo[2] = gyroDa.gyro[2] * SENSOR_GYRO_SCALE;
            m_u8GyroTS = gyroDa.timestamp;

            u4GyroScale = m_u8GyroTS != m_u8PreGyroTS ? SENSOR_GYRO_SCALE : 0;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "Gyro InValid!");
            u4GyroScale = 0; // set scale 0 means invalid to algo
        }

        SensorData acceDa;
        MBOOL acceDaVd = m_bAcceVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_ACCELERATION, acceDa) : MFALSE;

        if ( acceDaVd && acceDa.timestamp)
        {
            m_u8PreAcceTS = m_u8AcceTS;
            m_i4AcceInfo[0] = acceDa.acceleration[0] * SENSOR_ACCE_SCALE;
            m_i4AcceInfo[1] = acceDa.acceleration[1] * SENSOR_ACCE_SCALE;
            m_i4AcceInfo[2] = acceDa.acceleration[2] * SENSOR_ACCE_SCALE;
            m_u8AcceTS = acceDa.timestamp;

            u4ACCEScale = m_u8AcceTS != m_u8PreAcceTS ? SENSOR_ACCE_SCALE : 0;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "Acce InValid!");
            u4ACCEScale = 0; // set scale 0 means invalid to algo
        }
    }

    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->setAcceSensorInfo( m_i4AcceInfo, u4ACCEScale);
        m_pIAfAlgo->setGyroSensorInfo( m_i4GyroInfo, u4GyroScale);
    }

    if ( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf,
                              "Acce preTS(%11lld) TS(%11lld) Info(%4d %4d %4d)/Gyro preTS(%11lld) TS(%11lld) Info(%4d %4d %4d)/",
                              (unsigned long long)m_u8PreAcceTS,
                              (unsigned long long)m_u8AcceTS,
                              m_i4AcceInfo[0],
                              m_i4AcceInfo[1],
                              m_i4AcceInfo[2],
                              (unsigned long long)m_u8PreGyroTS,
                              (unsigned long long)m_u8GyroTS,
                              m_i4GyroInfo[0],
                              m_i4GyroInfo[1],
                              m_i4GyroInfo[2]);
    }

    MINT32 SqrGyroX = m_i4GyroInfo[0] * m_i4GyroInfo[0];
    MINT32 SqrGyroY = m_i4GyroInfo[1] * m_i4GyroInfo[1];
    MINT32 SqrGyroZ = m_i4GyroInfo[2] * m_i4GyroInfo[2];
    m_i4GyroValue = (MINT32)(sqrt((double)(SqrGyroX + SqrGyroY + SqrGyroZ)));

    if ( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf,
                              " Gyro VectorValue(%4d)",
                              m_i4GyroValue);
    }

    CAM_LOGD_IF( m_i4DgbLogLv, "%s %d", dbgMsgBuf, (MINT32)(ptrMsgBuf - dbgMsgBuf));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::sendAFCtrl(MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = MTRUE;
    switch (eAFCtrl)
    {
    case EAFMgrCtrl_GetIsSMVR:
        *(reinterpret_cast<MINT32*>(iArg1)) = m_i4IsSMVR;
        break;
    case EAFMgrCtrl_SetSMVRMode:
        m_i4IsSMVR = (MINT32)iArg1;
        i4Ret = ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_SetSMVRMode, iArg1, iArg2);
        break;

    case EAFMgrCtrl_NotifyPreStop:
        ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).notifyPreStop();
        break;

    case EAFMgrCtrl_GetFocusValue:
        m_sFRMInfo.i8FocusValue = m_sAFOutput.i8AFValue;
        m_sFRMInfo.i4LensPos    = m_sAFOutput.i4AFPos;
        for (int i = 0; i < 3; i++)
        {
            m_sFRMInfo.GyroValue[i] = m_i4GyroInfo[i];
        }
        m_sFRMInfo.AFROI[0]     = m_sArea_Focusing.i4X;
        m_sFRMInfo.AFROI[1]     = m_sArea_Focusing.i4Y;
        m_sFRMInfo.AFROI[2]     = m_sArea_Focusing.i4W;
        m_sFRMInfo.AFROI[3]     = m_sArea_Focusing.i4H;
        m_sFRMInfo.AFROI[4]     = (m_sArea_TypeSel == AF_ROI_SEL_FD) ? 1 : 0;
        *(reinterpret_cast<MINT32*>(iArg1)) = m_u4StaMagicNum;
        *(reinterpret_cast<AF_FRAME_INFO_T*>(iArg2)) = m_sFRMInfo;
        //
        if (m_i4DgbLogLv)
        {
            CAM_LOGD_IF(m_i4DgbLogLv,
                        "[%s] sttNum(#%d) FV(%d) DAC(%d) Gyro(%d, %d, %d) ROI(%d,%d,%d,%d,%d)",
                        __FUNCTION__,
                        m_u4StaMagicNum,
                        (MUINT32)m_sFRMInfo.i8FocusValue,
                        m_sFRMInfo.i4LensPos,
                        m_sFRMInfo.GyroValue[0], m_sFRMInfo.GyroValue[1], m_sFRMInfo.GyroValue[2],
                        m_sFRMInfo.AFROI[0], m_sFRMInfo.AFROI[1], m_sFRMInfo.AFROI[2], m_sFRMInfo.AFROI[3], m_sFRMInfo.AFROI[4]);
        }
        break;

    case EAFMgrCtrl_SetHighFPSCCUSuspend:
        ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_SetHighFPSCCUSuspend, iArg1, iArg2);
        break;

    case EAFMgrCtrl_CheckFPSCCUResume:
        ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_CheckFPSCCUResume, iArg1, iArg2);
        break;

    case EAFMgrCtrl_SetFrameRateInfo:
        ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_SetFrameRateInfo, iArg1, iArg2);
        break;

    case EAFMgrCtrl_GetFrameRateInfo:
        ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_GetFrameRateInfo, iArg1, iArg2);
        break;

    case EAFMgrCtrl_IsCCUSupport:
        i4Ret = ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsCCUSupport, iArg1, iArg2);
        break;

    case EAFMgrCtrl_IsAFSuspend:
        i4Ret = ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsAFSuspend, iArg1, iArg2);
        break;

    case EAFMgrCtrl_GetFSCInitInfo:
        if (iArg1)
        {
            ptrFSCInitInfo = reinterpret_cast<FSC_DAC_INIT_INFO_T*>(iArg1);
            ptrFSCInitInfo->macro_To_Inf_Ratio   = m_ptrNVRam->rAFNVRAM.i4ZoEffect[5];
            ptrFSCInitInfo->dac_Inf              = (MINT32)m_sDAF_TBL.af_dac_inf;
            ptrFSCInitInfo->dac_Macro            = (MINT32)m_sDAF_TBL.af_dac_marco;
            ptrFSCInitInfo->damping_Time         = 12;
            ptrFSCInitInfo->readout_Time_us      = m_i4P1TimeReadOut;
            ptrFSCInitInfo->init_DAC             = m_i4InitPos;
            ptrFSCInitInfo->af_Table_Start     = m_i4AFTabStr;
            ptrFSCInitInfo->af_Table_End       = m_i4AFTabEnd;
            CAM_LOGD_IF(m_i4DgbLogLv,
                        "[%s] EAFMgrCtrl_GetFSCInitInfo %d,%d,%d,%d,%d,%d,%d,%d",
                        __FUNCTION__,
                        ptrFSCInitInfo->macro_To_Inf_Ratio,
                        ptrFSCInitInfo->dac_Inf,
                        ptrFSCInitInfo->dac_Macro,
                        ptrFSCInitInfo->damping_Time,
                        ptrFSCInitInfo->readout_Time_us,
                        ptrFSCInitInfo->init_DAC,
                        ptrFSCInitInfo->af_Table_Start,
                        ptrFSCInitInfo->af_Table_End);
        }
        else
        {
            CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInitInfo, input arg is NULL", __FUNCTION__);
        }
        break;
    case EAFMgrCtrl_GetFSCInfo:
        if (iArg1)
        {
            Mutex::Autolock lock(m_DACListLock);
            ptrFSCInfo = reinterpret_cast<FSC_FRM_INFO_T*>(iArg1);
            ptrFSCInfo->SttNum = m_u4StaMagicNum;
            ptrFSCInfo->SetCount = 0;
            MINT32 needUpdateFSCInfo = 0;
            needUpdateFSCInfo |= ((DACList.size() > 1) && (DACList[0] != DACList[1]));
            needUpdateFSCInfo |= ((DACList.size() > 2) && (DACList[1] != DACList[2]));
            if (needUpdateFSCInfo)
            {
                MINT32 A = m_sAFInput.i4ShutterValue;   // unit : us
                MINT32 B = (m_sArea_HW.i4Y + m_sArea_HW.i4H) * m_i4ReadOutTimePerLine / 1000; // unit : us
                MINT32 C = m_i4P1TimeReadOut - B;       // unit : us
                MINT32 D = m_sAFInput.TS_AFDone - m_sAFInput.TS_MLStart;
                ptrFSCInfo->SetCount = ((D > 0) && (D <= A + B) && (DACList.size() > 2)) ? 2 : 1;

                // To prevent from updating only 1 set with the same DAC.
                if ((ptrFSCInfo->SetCount == 1) && (DACList[0] == DACList[1]))
                {
                    ptrFSCInfo->SetCount = 0;
                }

                for (int i = 0; i < ptrFSCInfo->SetCount; i++)
                {
                    ptrFSCInfo->DACInfo[i].DAC_To    = DACList[i + 0];
                    ptrFSCInfo->DACInfo[i].DAC_From  = DACList[i + 1];
                    ptrFSCInfo->DACInfo[i].Percent   = ((A + B - (D * i)) * 100) / (A + B + C);
                }
                if (m_i4DgbLogLv)
                {
                    CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo FSC (A,B,C,D) = (%d, %d, %d, %d)",
                             __FUNCTION__,
                             A, B, C, D);
                    CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo SttNum(%d) setCount(%d)",
                             __FUNCTION__,
                             ptrFSCInfo->SttNum,
                             ptrFSCInfo->SetCount);
                    for (int i = 0; i < ptrFSCInfo->SetCount; i++)
                    {
                        CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo Set %d : DAC (%d)->(%d), Percent (%d)",
                                 __FUNCTION__,
                                 i,
                                 ptrFSCInfo->DACInfo[i].DAC_From,
                                 ptrFSCInfo->DACInfo[i].DAC_To,
                                 ptrFSCInfo->DACInfo[i].Percent);
                    }
                }
            }
        }
        else
        {
            CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo, input arg is NULL", __FUNCTION__);
        }
        break;

    case EAFMgrCtrl_SetSecureFlag:
        m_bIsSecureCam = MTRUE;
        i4Ret = ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_SetSecureFlag, MTRUE, iArg2);
        break;

    case EAFMgrCtrl_GetSecureFlag:
        *(reinterpret_cast<MBOOL*>(iArg1)) = m_bIsSecureCam;
        break;
    }

    return i4Ret;
}