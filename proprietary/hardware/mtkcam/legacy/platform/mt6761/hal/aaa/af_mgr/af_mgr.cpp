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
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <aaa_log.h>
#include <mtkcam/common/faces.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <pd_algo_if.h>
#include "af_mgr_if.h"
#include "af_mgr.h"
#include "camera_custom_cam_cal.h"  //seanlin 121022 for test
#include "cam_cal_drv.h" //seanlin 121022 for test
#include "nvbuf_util.h"
#include <isp_mgr_af_stat.h>
#include <isp_mgr.h>
#include "aaa_common_custom.h"
#include "aaa_sensor_buf_mgr.h"
#include <pd_mgr_if.h>
#include <laser_mgr_if.h>
#include <laser_drv.h>
#include <af_flow_custom.h>



#include <LogCollector.h>
#include <afo_buf_mgr.h>
// AF v1.2
#include <math.h>
#include <android/sensor.h>             // for g/gyro sensor listener
#define SENSOR_ACCE_POLLING_MS  33
#define SENSOR_GYRO_POLLING_MS  33
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

#define LASER_TOUCH_REGION_W    0
#define LASER_TOUCH_REGION_H    0

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                   trace.
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_CAMERA
#endif
//
#include <utils/Trace.h>
#define CAM_TRACE_NAME_LENGTH                   64
#define CAM_TRACE_CALL()                        ATRACE_CALL()
#define CAM_TRACE_NAME(name)                    ATRACE_NAME(name)
#define CAM_TRACE_BEGIN(name)                   ATRACE_BEGIN(name)
#define CAM_TRACE_END()                         ATRACE_END()
#define CAM_TRACE_FMT_BEGIN(fmt, arg...)                    \
do{                                                         \
    if( ATRACE_ENABLED() )                                  \
    {                                                       \
        char buf[CAM_TRACE_NAME_LENGTH];                    \
        snprintf(buf, CAM_TRACE_NAME_LENGTH, fmt, ##arg);   \
        CAM_TRACE_BEGIN(buf);                               \
    }                                                       \
}while(0)
#define CAM_TRACE_FMT_END()                     CAM_TRACE_END()

#define AAA_TRACE_D(fmt, arg...) do{ CAM_TRACE_FMT_BEGIN(fmt, ##arg); } while(0)
#define AAA_TRACE_END_D do{ CAM_TRACE_FMT_END(); } while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                   Debug.
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define FUNC_BEGIN           CAM_LOGD("AF-%-15s + ", __FUNCTION__)
#define FUNC_END             CAM_LOGD("AF-%-15s - ", __FUNCTION__)
#define FUNC_BEGIN_DEV(dev)  CAM_LOGD("AF-%-15s + Dev %d ", __FUNCTION__, dev)
#define FUNC_END_DEV(dev)    CAM_LOGD("AF-%-15s - Dev %d ", __FUNCTION__, dev)

#define DBG_MSG_BUF_SZ 1024 //byte

// LaunchCamTrigger
#define AESTABLE_TIMEOUT 0
#define VALIDPD_TIMEOUT  0

#define GYRO_THRESHOLD 15
#define MAX_PDMOVING_COUNT 6

#define DBG_MSG_BUF_SZ 800 //byte

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr* AfMgr::s_pAfMgr = MNULL;

template <ESensorDev_T const eSensorDev>
class AfMgrDev : public AfMgr
{
public:
    static AfMgr& getInstance()
    {
        static AfMgrDev<eSensorDev> singleton;
        AfMgr::s_pAfMgr = &singleton;
        return singleton;
    }

    AfMgrDev() : AfMgr( eSensorDev) {}
    virtual ~AfMgrDev() {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr& AfMgr::getInstance( MINT32 const i4SensorDev)
{
#ifdef CAM3_SW_SUPPORT
    switch( i4SensorDev)
    {
    /* Main Sensor*/
    case ESensorDev_Main :
        return AfMgrDev<ESensorDev_Main>::getInstance();

    /* Main Second Sensor */
    case ESensorDev_MainSecond :
        return AfMgrDev<ESensorDev_MainSecond>::getInstance();

    /* Sub Sensor */
    case ESensorDev_Sub :
        return AfMgrDev<ESensorDev_Sub>::getInstance();

    /* Sub Second Sensor */
    case ESensorDev_SubSecond :
        return AfMgrDev<ESensorDev_SubSecond>::getInstance();

    default:
        CAM_LOGD( "i4SensorDev = %d", i4SensorDev);
        if( AfMgr::s_pAfMgr)
        {
            return *AfMgr::s_pAfMgr;
        }
        else
        {
            return AfMgrDev<ESensorDev_Main>::getInstance();
        }
    }
#else
    return *AfMgr::s_pAfMgr;
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::AfMgr( ESensorDev_T eSensorDev)
{
    mEngFileName    = new char[200];
    mEngFileName[0] = 0;

    m_i4CurrSensorDev = (MINT32)eSensorDev;
    m_i4Users     = 0;
    m_CCTUsers    = 0;
    m_pMcuDrv        = NULL;
    m_pIAfAlgo       = NULL;
    m_pAFParam       = NULL;
    m_bLDAFEn     = MFALSE;

    memset( &m_sAFInput,        0, sizeof(AF_INPUT_T)            );
    memset( &m_sAFOutput,       0, sizeof(AF_OUTPUT_T)           );
    m_ptrNVRam = NULL;
    memset( &m_sArea_Center,    0, sizeof(AREA_T));
    memset( &m_sArea_APCmd,     0, sizeof(AREA_T));
    memset( &m_sArea_APCheck,   0, sizeof(AREA_T));
    memset( &m_sArea_OTFD,      0, sizeof(AREA_T));
    memset( &m_sArea_Bokeh,     0, sizeof(AREA_T));
    memset( &m_sCallbackInfo, 0, sizeof(CHECK_AF_CALLBACK_INFO_T));
    memset( &rNormalPipe_HBIN_Info, 0, sizeof(NormalPipe_HBIN_Info));
    memset( &m_AEInfo, 0, sizeof(AE2AFInfo_T));

    m_i4CurrSensorId = 0x1;
    m_i4CurrModuleId = -1;
    m_i4TGSzW        = 0;
    m_i4TGSzH        = 0;
    m_i4CurrLensId   = 0;
    m_eCurAFMode     = MTK_CONTROL_AF_MODE_EDOF; /*force to waitting AP send setting mode command. [Ref : setafmode function]*/
    m_bIsFullScan      = FALSE;
    m_aCurEMAFFlag     = 0;   // for EMAF mode select: FullScan / AdvFullScan / Temperature Calibration
    m_aPreEMAFFlag     = 0;   // for EMAF mode select: FullScan / AdvFullScan / Temperature Calibration
    m_aAdvFSRepeatTime = 10;
    m_i4FullScanStep   = 0;
    m_i4EnableAF       = -1;
    m_AETargetMode   = AE_MODE_NORMAL;
    m_eLIB3A_AFMode    = LIB3A_AF_MODE_AFS;
    m_pAFCallBack      = NULL;
    m_i4DgbLogLv       = 0;
    m_eSensorTG        = ESensorTG_1;
    m_i4SensorIdx      = 0;
    m_i4SensorMode     = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    m_i4BINSzW         = 0;
    m_i4BINSzH         = 0;
    m_ptrIHalSensor          = NULL;
    m_bSensorModeSupportPDAF = MFALSE;
    m_PDPipeCtrl             = FEATURE_PDAF_UNSUPPORT;
    m_i4PDAF_support_type    = PDAF_SUPPORT_NA;
    m_ptrLensNVRam           = NULL;
    m_i4OISDisable           = MFALSE;
    m_i4InitPos              = 0;
    m_i4LensPosExit          = 0;
    m_bNeedCheckSendCallback = MFALSE;
    i4IsLockAERequest = 0;
    // LaunchCamTrigger : disable at the first
    m_i4LaunchCamTriggered_Prv = m_i4LaunchCamTriggered = E_LAUNCH_AF_IDLE;
    m_i4AEStableFrameCount = -1;
    m_i4ValidPDFrameCount = -1;
    m_i4AEStableTriggerTimeout = AESTABLE_TIMEOUT;
    m_i4ValidPDTriggerTimeout = VALIDPD_TIMEOUT;
    m_i4IsSMVR               = MFALSE;
    m_i4AFMaxAreaNum         = -1;
    m_i4IsLockForLaunchCamTrigger = 0;

    m_i4IsCAFWithoutFace = 0;
    m_bNVRAMGot = MFALSE;
    m_i4isAEStable = MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::~AfMgr()
{
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sendAFNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL ret = MFALSE;
    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIdx, LOG_TAG);
    if( pPipe==NULL)
    {
        CAM_LOGE( "Fail to create INormalPipe_FrmB");
    }
    else
    {
        ret = pPipe->sendCommand( cmd, arg1, arg2, arg3);
        pPipe->destroyInstance( LOG_TAG);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getPdInfoForSttCtrl(MINT32 /*i4SensorIdx*/, MINT32 i4SensorMode, FEATURE_PDAF_STATUS &PDAFStatus)
{
    MBOOL bSensorModeSupportPDAF = 0;

    /*
     * Calling timing :
     *    After "3A" and "isp" modules are initialized, calling this function by Hal3ASttCtrl before starting AF module.
     * Note :
     *    Do not use m_ptrIHalSensor.
     *    Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer).
     *    Outputs u4PDOSizeW, u4PDOSizeH, PDAFStatus for each type PD HW path for configuring SttPipe in 3A framework.
     */
    PDAFStatus = FEATURE_PDAF_UNSUPPORT;


    if( property_get_int32("vendor.debug.pdflow.disable", 0)==1)
    {
        CAM_LOGE("PD disabled by property (disablepd)");

        m_PDPipeCtrl             = PDAFStatus;
        m_bSensorModeSupportPDAF = bSensorModeSupportPDAF;

        return S_AF_OK;
    }

    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    if (!pHalSensorList)
    {
        CAM_LOGE("%s IHalSensorList::get()==NULL", __FUNCTION__);

        m_PDPipeCtrl             = PDAFStatus;
        m_bSensorModeSupportPDAF = bSensorModeSupportPDAF;

        return E_3A_ERR;
    }
    IHalSensor* pIHalSensor = pHalSensorList->createSensor( "getPdInfoForSttCtrl", m_i4CurrSensorDev);

    /*
     * Query sensor static informataion PDAF_Support :
     * 0 : NO PDAF
     * 1 : PDAF Raw Data mode
     * 2 : PDAF VC mode(Full)
     * 3 : PDAF VC mode(Binning)
     * 4 : PDAF DualPD Raw Data mode
     * 5 : PDAF DualPD VC mode
     */
    SensorStaticInfo sensorStaticInfo;
    pHalSensorList->querySensorStaticInfo(m_i4CurrSensorDev,&sensorStaticInfo);

    /*
     * Query PDAF capacity from sensor driver
     * This value is by sensor mode.
     */
    if(pIHalSensor)
    {
        pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY, (MINTPTR)&i4SensorMode, (MINTPTR)&bSensorModeSupportPDAF, 0);
    }

    /*
     * Store PDAF related information for flow control.
     */
    m_bSensorModeSupportPDAF = bSensorModeSupportPDAF;
    m_i4PDAF_support_type    = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)sensorStaticInfo.PDAF_Support;


    /*
     * Outputs u4PDOSizeW, u4PDOSizeH, PDAFStatus by PD type.
     */
    if( m_bSensorModeSupportPDAF==PDAF_SUPPORT_NA)
    {
        // Currently, sensor mode is not support PD.
        PDAFStatus = FEATURE_PDAF_UNSUPPORT;

    }
    else if( m_i4PDAF_support_type==PDAF_SUPPORT_RAW)
    {
        // For legacy platform not support PDO type.
        PDAFStatus = FEATURE_PDAF_UNSUPPORT;
    }
    else if( m_i4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)
    {
        PDAFStatus = FEATURE_PDAF_SUPPORT_LEGACY;
    }
    else if( m_i4PDAF_support_type==PDAF_SUPPORT_RAW_DUALPD)
    {
        // For legacy platform not support PBIN type.
        PDAFStatus = FEATURE_PDAF_UNSUPPORT;
    }
    else if( m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV || m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV_LEGACY || m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV_DUALPD)
    {
        PDAFStatus = FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL;
    }
    else
    {
        PDAFStatus = FEATURE_PDAF_UNSUPPORT;
    }


    m_PDPipeCtrl = PDAFStatus;


    CAM_LOGD("[%s] SensorMode(%d), PDAF_Support(%d), bSensorModeSupportPDAF(%d), Info for 3A framework to Cfg sttPipe :PDAFStatus(%d)",
             __FUNCTION__,
             i4SensorMode,
             m_i4PDAF_support_type,
             bSensorModeSupportPDAF,
             PDAFStatus);

    if( pIHalSensor)
    {
        pIHalSensor->destroyInstance( "getPdInfoForSttCtrl");
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::init( MINT32 i4SensorIdx, MINT32 /*isInitMCU*/)
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    MRESULT ret = E_3A_ERR;

    Mutex::Autolock lock(m_Lock);

    if( m_i4Users==0)
    {
        /* Init af_mgr, when no user calls af_mgr init.*/

        // initial property
        m_i4DgbLogLv        = property_get_int32("vendor.debug.af_mgr.enable", 0);
        m_i4DbgPDVerifyEn   = property_get_int32("vendor.debug.pd_verify_flow.enable", 0);

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
        m_i4AFMaxAreaNum   = -1;
        m_bPauseAF         = MFALSE;
        m_i4UnPauseReqNum  = 0;
        m_i4IsEnableFVInFixedFocus = -1;
        m_i4EnableAF  = -1;
        m_bLatchROI           = MFALSE;
        m_bWaitForceTrigger   = MFALSE;
        m_bForceTrigger       = MFALSE;
        m_bTriggerCmdVlid     = MFALSE;
//        m_bMgrTriggerCmd      = MFALSE;
        memset(&m_sHostCmd, 0, sizeof(HostCmdParam_t));

        // Get hybrid AF instance.
#if USE_OPEN_SOURCE_AF
        m_pIAfAlgo=NS3A::IAfAlgo::createInstance<NS3A::EAAAOpt_OpenSource>( m_i4CurrSensorDev);
#else
        m_pIAfAlgo=NS3A::IAfAlgo::createInstance<NS3A::EAAAOpt_MTK>( m_i4CurrSensorDev);
#endif

        SDAF_Init();

        LDAF_Init();

        SensorProvider_Init();

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

    if( m_i4Users==1)
    {
        /* Uninit af_mgr, when last user calls af_mgr uninit.*/


        // uninit hybrid AF
        if( m_pIAfAlgo)
        {
            m_pIAfAlgo->destroyInstance();
            m_pIAfAlgo = NULL;
        }

        // Clear EMAF flag when leaving camera
        m_bIsFullScan = MFALSE;
        m_aCurEMAFFlag = 0;
        m_aPreEMAFFlag = 0;
        m_aAdvFSRepeatTime = 10;
        m_bNVRAMGot = MFALSE;

        SensorProvider_Uninit();

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

    /****************************************************************
     * start function is called as init AF manager is called in HAL1.
     * So mutex cannot add here for HAL1 !!
     ****************************************************************/
    //Mutex::Autolock lock( m_Lock);

    //reset member.
    memset( &m_sAFInput,           0, sizeof(AF_INPUT_T ));
    memset( &m_sAFOutput,          0, sizeof(AF_OUTPUT_T));
    memset( &m_sCropRegionInfo,    0, sizeof(AREA_T     ));
    memset( &m_sArea_Focusing,     0, sizeof(AREA_T     ));
    memset( &m_sArea_Center,       0, sizeof(AREA_T     ));
    memset( &m_sArea_APCmd,        0, sizeof(AREA_T     ));
    memset( &m_sArea_APCheck,      0, sizeof(AREA_T     ));
    memset( &m_sArea_OTFD,         0, sizeof(AREA_T     ));
    memset( &m_sArea_HW,           0, sizeof(AREA_T     ));
    memset( &m_sArea_Bokeh,        0, sizeof(AREA_T     ));
    memset( &m_sPDRois[0],         0, sizeof(PD_CALCULATION_ROI_T)*eIDX_ROI_ARRAY_NUM);
    memset( &m_sPDCalculateWin[0], 0, sizeof(AFPD_BLOCK_ROI_T    )*AF_PSUBWIN_NUM    );
    memset( &m_aAEBlkVal,          0, sizeof(MUINT8              )*25                );
    memset( &m_AEInfo, 0, sizeof(AE2AFInfo_T));
    m_vFrmInfo.clear();
    m_i4EnableAF          = -1;
    m_i4CurLensPos        = 0;
    m_i4PreLensPos        = 0;
    m_i4DZFactor          = 100; /* Initial digital zoom factor. */
    m_sFocusDis.i4LensPos = 0; /* default value, ref: af_tuning_customer.cpp*/
    m_sFocusDis.i4Dist    = 0.33; /* default value, ref: af_tuning_customer.cpp*/
    m_afwin_synccnt       = 0;
    m_next_query_FrmNum   = 0;
    m_i4MvLensTo          = -1;
    m_i4MvLensToPre       = 0;
    m_i4HWBlkNumX         = 0;
    m_i4HWBlkNumY         = 0;
    m_i4HWBlkSizeX        = 0;
    m_i4HWBlkSizeY        = 0;
    m_i4HWEnExtMode       = 0;
    m_i4IsFocused         = 0;
    m_u4ReqMagicNum       = 0;
    m_u4StaMagicNum       = 0;
    m_u4ConfigHWNum       = 0;
    m_u4ConfigLatency     = 2;
    m_bPdInputExpected    = MFALSE;
    m_sArea_TypeSel       = AF_ROI_SEL_NONE;

    m_i4OTFDLogLv         = 0;
    m_eEvent              = EVENT_CMD_START;
    m_i4IsAFSearch_PreState = AF_SEARCH_DONE; /*Force to select ROI to focusing as first in, ref:SelROIToFocusing */
    m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    m_bRunPDEn          = MFALSE;
    m_bSDAFEn            = MFALSE;
    m_bMZAFEn             = MFALSE;
    m_bGetMetaData      = MFALSE;
    m_bPauseAF          = MFALSE;
    m_bNeedPdoResult    = MTRUE;
    m_i4TAFStatus       = TAF_STATUS_RESET;
    m_i4DbgOISPos       = 0;
    m_u4FrameCount      = 0;
    i4IsLockAERequest   = 0;
    m_eCurAFMode        = MTK_CONTROL_AF_MODE_EDOF;
    m_eLIB3A_AFMode     = LIB3A_AF_MODE_OFF;
    m_i4IsCAFWithoutFace = 0;
    m_eCamScenarioMode  = CAM_SCENARIO_PREVIEW;
    m_bLDAFEn           = MFALSE;
    m_eAFStatePre   = m_eAFState = NS3A::E_AF_INACTIVE;
    m_bLock          = MFALSE;
    m_bNeedLock      = MFALSE;
    m_bLatchROI           = MFALSE;
    m_bWaitForceTrigger   = MFALSE;
    m_bForceTrigger       = MFALSE;
    m_bTriggerCmdVlid     = MFALSE;
//    m_bMgrTriggerCmd      = MFALSE;
    // LaunchCamTrigger : disable at the first
    m_i4LaunchCamTriggered_Prv = m_i4LaunchCamTriggered = E_LAUNCH_AF_IDLE;
    m_i4AEStableFrameCount = -1;
    m_i4ValidPDFrameCount = -1;
    m_i4AEStableTriggerTimeout = AESTABLE_TIMEOUT;
    m_i4ValidPDTriggerTimeout = VALIDPD_TIMEOUT;
    m_u8CamTS    = 0;
    m_u8PreCamTS = 0;
    m_u8FrmCnt   = 0;

    m_i4FirsetCalPDFrameCount = -1;
    m_i4isAEStable = MFALSE;

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                            check algo instance
     *
     *--------------------------------------------------------------------------------------------------------*/
    if( !m_pIAfAlgo)
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
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();

    if( !pIHalSensorList)
    {
        CAM_LOGE("AF-%-15s: Dev(0x%04x), pIHalSensorList NULL", __FUNCTION__, m_i4CurrSensorDev);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }

    // get sensor information
    SensorStaticInfo  rSensorStaticInfo;
    SensorDynamicInfo rSensorDynamicInfo;

    if( m_ptrIHalSensor==NULL)
    {
        m_ptrIHalSensor = pIHalSensorList->createSensor( LOG_TAG, m_i4SensorIdx);
    }

    switch( m_i4CurrSensorDev)
    {
    case ESensorDev_Main :
        pIHalSensorList->querySensorStaticInfo(  NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        m_ptrIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
        break;
    case ESensorDev_Sub:
        pIHalSensorList->querySensorStaticInfo(  NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        m_ptrIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
        break;
    case ESensorDev_MainSecond:
        pIHalSensorList->querySensorStaticInfo(  NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        m_ptrIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
        break;
    case ESensorDev_SubSecond:
        pIHalSensorList->querySensorStaticInfo(  NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        m_ptrIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_SUB_2, &rSensorDynamicInfo);
        break;
    default:
        CAM_LOGE( "AF-%-15s: Invalid sensor device: %d", __FUNCTION__, m_i4CurrSensorDev);
        break;
    }

    m_sMetaData = pIHalSensorList->queryStaticInfo( m_i4SensorIdx);

    switch( rSensorDynamicInfo.TgInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
        break;
    default:
        CAM_LOGD("rSensorDynamicInfo.TgInfo = %d", rSensorDynamicInfo.TgInfo);
        //return MFALSE;
    }

    /* 0:BGGR, 1:GBRG, 2GRBG, 3RGGB */
    m_i4PixelId = (MINT32)rSensorStaticInfo.sensorFormatOrder;

    /* Lens search */
    m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
    if( m_pMcuDrv==NULL)
    {
        /*******************************************************
         *
         * lensSearch MUST be done before get data from NVRAM !!
         *
         *******************************************************/
        MUINT32 ModuleID = 0;
        AAA_TRACE_D("GetModuleIDFromCamCal");
        if (m_i4CurrModuleId == -1)
        {
            CAM_LOGD( "AF-%-15s: Dev(0x%04x) Get Module ID +", __FUNCTION__, m_i4CurrSensorDev);
            /* In order to get module id, it must be call CamCal */
            CAM_CAL_DATA_STRUCT pCalData;
            CamCalDrvBase* pCamCalDrvObj = CamCalDrvBase::createInstance();
            CAMERA_CAM_CAL_TYPE_ENUM eCamCalDataType = CAMERA_CAM_CAL_DATA_PART_NUMBER;
            MINT32 ret = pCamCalDrvObj->GetCamCalCalData(m_i4CurrSensorDev, eCamCalDataType, (void*)&pCalData);

            m_i4CurrModuleId = (ret == CAM_CAL_ERR_NO_ERR) ? (pCalData.PartNumber[0] << 8) + pCalData.PartNumber[1] : 0;
            CAM_LOGD( "AF-%-15s: Dev(0x%04x) Module ID (%d), ret (%d) -", __FUNCTION__, m_i4CurrSensorDev, m_i4CurrModuleId, ret);
        }
        AAA_TRACE_END_D;
        ModuleID = (m_i4CurrModuleId > 0) ? m_i4CurrModuleId : 0;

        MCUDrv::lensSearch( m_i4CurrSensorDev, m_i4CurrSensorId, ModuleID);
        m_i4CurrLensId = MCUDrv::getCurrLensID( m_i4CurrSensorDev);
    }

    m_i4EnableAF = (getAFMaxAreaNum() > 0) ? 1 : 0;// isAFSupport == hasAFLens || ForceEnableFVInFixedFocus

    MINT32 IsAFSupport = isAFSupport();

    CAM_LOGD( "AF-%-15s: Dev(0x%04x), TG(%d), PixId (%d)(0:B, 1:GB, 2:GR, 3:R), SensorID (0x%04x), ModuleId (0x%04x), LensId (0x%04x), EnableAF (%d), IsAFSupport(%d), PD_Support(%d)",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eSensorTG,
              m_i4PixelId,
              m_i4CurrSensorId,
              m_i4CurrModuleId,
              m_i4CurrLensId,
              m_i4EnableAF,
              IsAFSupport,
              rSensorStaticInfo.PDAF_Support);

    /*--------------------------------------------------------------------------------------------------------
     *                                get NVRAM data : lensSearch MUST be done before this
     *--------------------------------------------------------------------------------------------------------*/
    MCUDrv::setLensNvramIdx( m_i4CurrSensorDev);
    MINT32 err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrLensNVRam);
    if( (err!=0) || (!m_ptrLensNVRam))
    {
        CAM_LOGE( "AF-%-15s: AfAlgo NvBufUtil get buf fail!", __FUNCTION__);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }
    // Update CamScenarioMode if the new one is different and valid.
    if(m_sHostCmd.a_eCamScenarioMode != m_eCamScenarioMode && m_sHostCmd.a_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
    {
        m_eCamScenarioMode = m_sHostCmd.a_eCamScenarioMode;
    }
    // get AFNVRAM according to CamScenarioMode
    if(m_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
    {
        m_ptrNVRam = &(m_ptrLensNVRam->rLENSNVRAM[m_eCamScenarioMode]);
        CAM_LOGD("AF-%-15s: Dev(0x%04x), Scenario Mode(%d), NVRAM File Path %s",
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_eCamScenarioMode,
                 m_ptrNVRam->rFilePath);
    }
    else
    {
        CAM_LOGE( "AF-%-15s: CamScenarioMode is not be updated by 3A framework!!", __FUNCTION__);
        m_i4EnableAF = 0;
        return E_3A_ERR;
    }
    CAM_LOGD( "AF-%-15s: Dev(0x%04x), ParamInfo : CamScenarioMode(%d), [nvram] ver(%d), Sz(%d, %zu, %zu, %zu, %zu, %zu, %zu),Normal Num(%d) Macro Num(%d) Normal min step(%d) Macro min step(%d)",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eCamScenarioMode,
              m_ptrLensNVRam->Version,
              MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE,
              sizeof(UINT32),
              sizeof(FOCUS_RANGE_T),
              sizeof(AF_NVRAM_T),
              sizeof(PD_NVRAM_T),
              sizeof(DUALCAM_NVRAM_T),
              MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(FOCUS_RANGE_T)-sizeof(AF_NVRAM_T)-sizeof(PD_NVRAM_T)-sizeof(DUALCAM_NVRAM_T),
              m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum,
              m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum,
              m_ptrNVRam->rAFNVRAM.i4AFS_STEP_MIN_NORMAL,
              m_ptrNVRam->rAFNVRAM.i4AFS_STEP_MIN_MACRO);
    // LaunchCamTrigger
    m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10]>0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] : m_i4AEStableTriggerTimeout;
    m_i4ValidPDTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11]>0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] : m_i4ValidPDTriggerTimeout;
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

    if(m_i4DgbLogLv)
    {
        for(int i=0; i<AF_CAM_SCENARIO_NUM; i++)
        {
            CAM_LOGD("LENS NVRAM set[%d] path: %s", i, m_ptrLensNVRam->rLENSNVRAM[i].rFilePath);
        }
    }

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                    get pass1 related information
     *
     *--------------------------------------------------------------------------------------------------------*/
    // Get HBIN1 info
    sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_HBIN_INFO, m_i4SensorIdx, (MINTPTR)&rNormalPipe_HBIN_Info, NULL);
    m_bIsHBINEnabled = rNormalPipe_HBIN_Info.mEnabled;

    // Get sensor information :
    // TG size
    sendAFNormalPipe( NSImageio::NSIspio::EPIPECmd_GET_TG_OUT_SIZE, m_i4SensorIdx, (MINTPTR)(&m_i4TGSzW), (MINTPTR)(&m_i4TGSzH));
    // TG after HBIN Blk size : for HPF coordinate setting.
    m_i4BINSzW = m_bIsHBINEnabled ? (m_i4TGSzW>>1):(m_i4TGSzW);
    m_i4BINSzH = m_i4TGSzH;
    // Checking sensor mode.
    CAM_LOGD( "AF-%-15s: Dev(0x%04x), TGSZ: W %d, H %d, BINSZ: W %d, H %d",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4TGSzW,
              m_i4TGSzH,
              m_i4BINSzW,
              m_i4BINSzH);

    if( (m_sAFInput.sEZoom.i4W == 0) || (m_sAFInput.sEZoom.i4H == 0))
    {
        m_sAFInput.sEZoom.i4W = m_i4TGSzW;
        m_sAFInput.sEZoom.i4H = m_i4TGSzH;
    }

    //check ZSD or not
    MUINT32 isZSD = ( m_i4TGSzW==(MUINT32)rSensorStaticInfo.captureWidth) && ( m_i4TGSzH==(MUINT32)rSensorStaticInfo.captureHeight)? TRUE : FALSE;
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

    if( !m_pAFParam || !ptrHWCfgDef)
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

    m_bNVRAMGot = TRUE;

    // EM : set smooth-step to 1023
    MINT32 i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4CurrSensorDev);
    if(i4OperMode==3)
    {
        CAM_LOGD("EM: set i4BackJump to 0");
        for(int iter=0; iter<JUMP_NUM; iter++)
        {
            m_ptrNVRam->rAFNVRAM.i4BackJump[iter] = 0;
        }
    }

    /* if(m_i4DgbLogLv)
    {
        LogCollector LC_NVRAM(LOG_TAG);
        LC_NVRAM.Add("AF-%-15s: [CamScenarioMode] %d [nvram][Version] %d / ", __FUNCTION__, m_eCamScenarioMode, m_ptrLensNVRam->Version);
        LC_NVRAM.Add("[Size] %d %d %d %d %d %d %d ",
                     MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE, sizeof(UINT32), sizeof(FOCUS_RANGE_T), sizeof(AF_NVRAM_T), sizeof(PD_NVRAM_T), sizeof(DUALCAM_NVRAM_T), sizeof(m_ptrNVRam->rSCENARIONVRAM));
        LC_NVRAM.Add("rest %d / ",
                     MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(FOCUS_RANGE_T)-sizeof(AF_NVRAM_T)-sizeof(PD_NVRAM_T)-sizeof(DUALCAM_NVRAM_T)-sizeof(m_ptrNVRam->rSCENARIONVRAM));
        LC_NVRAM.Add("[Normal Num]%d [Macro Num]%d / ", m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum, m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);
        LC_NVRAM.Add("[Param][Normal min step]%d [Macro min step]%d", m_ptrNVRam->rAFNVRAM.i4AFS_STEP_MIN_NORMAL, m_ptrNVRam->rAFNVRAM.i4AFS_STEP_MIN_MACRO);
        LC_NVRAM.Print();
    } */


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
    //===============
    // for legacy platform calling getPdInfoForSttCtrl instead 3A framework.
    //===============
    FEATURE_PDAF_STATUS PDAFStatus;
    getPdInfoForSttCtrl( m_i4SensorIdx, m_i4SensorMode, PDAFStatus);
    //===============
    // PD profile for getting pd_mgr
    //===============
    SPDProfile_t pdprofile;
    pdprofile.i4CurrSensorId      = m_i4CurrSensorId;
    pdprofile.u4IsZSD             = isZSD;
    pdprofile.u4PDAF_support_type = m_i4PDAF_support_type; // initialized in getPdInfoForSttCtrl.
    pdprofile.uImgXsz             = m_i4TGSzW;
    pdprofile.uImgYsz             = m_i4TGSzH;
    pdprofile.uFullXsz            = (MINT32)rSensorStaticInfo.captureWidth;
    pdprofile.uFullYsz            = (MINT32)rSensorStaticInfo.captureHeight;
    pdprofile.u4IsFrontalBinning  = 0;  //[TODO]((m_i4TGSzW==m_i4BINSzW) && (m_i4TGSzH==m_i4BINSzH)) ? 0 : 1;
    pdprofile.bEnablePBIN         = 0;  //[TODO] m_bEnablePBIN;
    pdprofile.uPdSeparateMode     = 0;  //[TODO] m_u1PdSeparateMode;
    pdprofile.i4SensorMode        = m_i4SensorMode;
    pdprofile.AETargetMode        = m_AETargetMode;
    pdprofile.u4PDPipeCtrl         = m_PDPipeCtrl;
    pdprofile.bSensorModeSupportPD = m_bSensorModeSupportPDAF;

    //===============
    // Start PD manager.
    //===============
    m_PDBuf_Type = IPDMgr::getInstance().start( m_i4CurrSensorDev,
                   m_i4SensorIdx,
                   &pdprofile,
                   &(m_ptrNVRam->rPDNVRAM));

    if( m_PDBuf_Type==EPDBUF_NOTDEF)
    {
        CAM_LOGD( "AF-%-15s: Dev(0x%04x), PDAF flow is not enabled PDAF support type(%d->0)", __FUNCTION__, m_i4CurrSensorDev, m_i4PDAF_support_type);
        m_i4PDAF_support_type = PDAF_SUPPORT_NA;
    }
    else
    {
        //allocate vc DMA buffer.
        if( m_PDBuf_Type==EPDBUF_VC|| m_PDBuf_Type==EPDBUF_VC_OPEN)
        {
            //allocate virtual channel buffer
            m_CamSVBufInfo = NULL;
            m_CamSVBufInfo = new BufInfo_T;
        }
        SPDLibVersion_t PdLibVersion;
        IPDMgr::getInstance().GetVersionOfPdafLibrary( m_i4CurrSensorDev, PdLibVersion);
        CAM_LOGD( "AF-%-15s: Dev(0x%04x), PDAF flow is enabled, PD version %d.%d", __FUNCTION__, m_i4CurrSensorDev, PdLibVersion.MajorVersion, PdLibVersion.MinorVersion);
    }

    CAM_LOGD( "AF-%-15s: Dev(0x%04x), PD Flow (%d), SensorMode(%d), PDAF_Support_Type(%d), SensorModeSupportPDAF(%d), PDPipeCtrl(%d), ImgSz(%d, %d), FullSz(%d, %d), IsZSD %d, AETargetMode(%d)",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4PDAF_support_type,
              m_i4SensorMode,
              pdprofile.u4PDAF_support_type,
              m_bSensorModeSupportPDAF,
              m_PDPipeCtrl,
              pdprofile.uImgXsz,
              pdprofile.uImgYsz,
              pdprofile.uFullXsz,
              pdprofile.uFullYsz,
              isZSD,
              m_AETargetMode);

    /*--------------------------------------------------------------------------------------------------------
     *
     *                                          init laser driver
     *
     *--------------------------------------------------------------------------------------------------------*/
    LDAF_Start();

    // ISP3.0/ISP4.0 compatible
    // SetCropRegion SHOULD be called before ExeSetAFArea to get the correct m_sCropRegionInfo.
    HostCmdQ_T rReq;
    MINT32 i4RequestCount = 0;
    while(getAFRequest(rReq, i4RequestCount))
    {
        i4RequestCount++;
        CAM_LOGD_IF(m_i4DgbLogLv, "[%s] getAFRequest \n", __FUNCTION__);
        switch(rReq.afCmd)
        {
        case AF_CMD_AUTOFOCUS:
            ExeAutoFocus(m_sHostCmd.bIsAutoFocusFromHost);
            break;
        case AF_CMD_CANCELAUTOFOCUS:
            ExeCancelAutoFocus(m_sHostCmd.bIsCancelAutoFocusFromHost);
            break;
        case AF_CMD_SETAFAREA:
            ExeSetAFArea(m_sHostCmd.rFocusAreas);
            break;
        case AF_CMD_SETAFMODE:
            ExeSetAFMode(m_sHostCmd.u4AfMode);
            break;
        case AF_CMD_SETFULLSCANSTEP:
            ExeSetFullScanstep(m_sHostCmd.i4FullScanStep);
            break;
        case AF_CMD_SETMFPOS:
            ExeSetMFPos(m_sHostCmd.i4MFPos);
            break;
        case AF_CMD_SETZOOMWININFO:
            SetCropRegionInfo(m_sHostCmd.rCropRegion.i4X, m_sHostCmd.rCropRegion.i4Y, m_sHostCmd.rCropRegion.i4W, m_sHostCmd.rCropRegion.i4H, AF_CMD_CALLER);
            break;
        case AF_CMD_SETSENSORMODE:
            setCamScenarioMode( m_sHostCmd.a_eCamScenarioMode);
            break;
        case AF_CMD_SETFOCUSDISTANCE:
            ExeSetFocusDistance(m_sHostCmd.fFocusDistance);
            break;
        case AF_CMD_TRIGGERAF:
            triggerAF(AF_MGR_CALLER);
            break;
        default:
            CAM_LOGD("%s getAFRequest invalid cmd");
            break;
        }
    }

    if(m_sCropRegionInfo.i4X==0 && m_sCropRegionInfo.i4Y==0 && m_sCropRegionInfo.i4W==0 && m_sCropRegionInfo.i4H==0)
    {
        CAM_LOGD("NULL CropRegion");
        SetCropRegionInfo(0, 0, (MUINT32)m_i4TGSzW, (MUINT32)m_i4TGSzH, AF_CMD_CALLER);
    }
    /*--------------------------------------------------------------------------------------------------------
     *
     *                                          initial Hybrid AF algorithm
     *
     *--------------------------------------------------------------------------------------------------------*/
    // Currently, multi zone is supported as PDAF is on.
    m_i4DbgMZEn = property_get_int32("vendor.af.mzaf.enable", 0);
    m_bMZAFEn = m_i4DbgMZEn||(m_bMZHostEn==1) ? MTRUE : MFALSE;
    m_sAFInput.i4IsMZ = m_bMZAFEn;
    // default using center ROI. m_sArea_Center is updated after calling SetCropRegionInfo.
    m_sAFInput.sAFArea.i4Count  = 1;
    m_sAFInput.sAFArea.sRect[0] = m_sArea_Center;

    // update non-volatilize information for hybrid AF input
    memcpy( &(m_sAFInput.sLensInfo), &m_sCurLensInfo, sizeof(LENS_INFO_T));

    // set parameters to hybrid AF.
    // should get default HW configuration from af_tuning_custom.
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
    m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

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
    m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);
    m_eAFStatePreForCallback = m_eAFStatePre = m_eAFState;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                            init af_mgr flow control
     *
     *-----------------------------------------------------------------------------------------------------*/
    m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = m_sAFOutput.i4IsAfSearch;
    m_i4IsSelHWROI_PreState   = m_i4IsSelHWROI_CurState   = m_sAFOutput.i4IsSelHWROI;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                            AF Manager initial end
     *
     *-----------------------------------------------------------------------------------------------------*/
    if( m_bLock==MTRUE)
    {
        m_pIAfAlgo->cancel();
        m_pIAfAlgo->lock();
        CAM_LOGD( "AF-%-15s: LockAF", __FUNCTION__);
        m_bLock     = MTRUE;
        m_bNeedLock = MFALSE;

        if(!m_i4IsLockForLaunchCamTrigger)
        {
            // algo locked for preCapture ==> disable LaunchCamTrigger
            m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        }
    }
    m_FirstTimeAFON = 0;
    m_bGetMetaData  = MTRUE;

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::AFThreadStart()
{
    //---------------------------------------- StartPreview speed up -----------------
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    // Get AF calibration data. Should do this after setAFParam is called.
    if( m_ptrNVRam->rAFNVRAM.i4ReadOTP==TRUE)
    {
        readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
    }

    //---------------------------------------- init MCU ------------------------------
    if( m_pMcuDrv == NULL)
    {
        CAM_LOGD( "AF-%-15s: initMCU + Dev %d, moveMCU %d",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_i4InitPos);

        m_pMcuDrv = MCUDrv::createInstance(m_i4CurrLensId);
        if (m_pMcuDrv == NULL)
        {
            CAM_LOGE( "AF-%-15s: McuDrv::createInstance fail", __FUNCTION__);
            m_i4EnableAF = 0;
        }
        else
        {
            m_pMcuDrv->init(m_i4CurrSensorDev, m_i4InitPos);
        }
        CAM_LOGD( "AF-%-15s: initMCU -", __FUNCTION__);
    }
    else
    {
        CAM_LOGD( "AF-%-15s: Fixed Focus", __FUNCTION__);
    }

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::Stop()
{
    /****************************************************************
     * start function is called as init AF manager is called in HAL1.
     * So mutex cannot add here for HAL1 !!
     ****************************************************************/
    //Mutex::Autolock lock( m_Lock);

    //uninitial isp_mgr_af_stat for configure HW
    IspMgrAFStatStop();

    //store Nno-volatilize informaiton.
    getLensInfo( m_sCurLensInfo);
    CAM_LOGD( "AF-%-15s: + Dev %d, Record : Mode (%d)%d, Pos %d",
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eCurAFMode,
              m_eLIB3A_AFMode,
              m_sCurLensInfo.i4CurrentPos);

    //reset parameters.
    m_bRunPDEn = MFALSE;
    m_i4PDAF_support_type =  PDAF_SUPPORT_NA;

    IPDMgr::getInstance().stop( m_i4CurrSensorDev);

    if( m_ptrIHalSensor)
    {
        m_ptrIHalSensor->destroyInstance( LOG_TAG);
        m_ptrIHalSensor = NULL;
    }

    if( m_CamSVBufInfo)
    {
        delete (BufInfo_T*) m_CamSVBufInfo;
        m_CamSVBufInfo = NULL;
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

    memset( &m_sCurLensInfo, 0, sizeof( LENS_INFO_T));

    m_eAFStatePre   = m_eAFState = NS3A::E_AF_INACTIVE;

    m_bLock          = MFALSE;
    m_bNeedLock      = MFALSE;

    m_bLatchROI           = MFALSE;
    m_bWaitForceTrigger   = MFALSE;
    m_bForceTrigger       = MFALSE;
    m_bTriggerCmdVlid     = MFALSE;
    m_ptrLensNVRam        = NULL;

    m_i4MFPos = -1;
    m_i4InitPos = 0;
    m_i4DbgMotorMPos      = 1024;
    m_i4DbgMotorMPosPre   = 1024;
    m_i4DbgMotorDisable   = 0;

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::CamPwrOffState()
{
    //Camera Power Off, call by HAL, MW
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    if( m_pMcuDrv)
    {
        CAM_LOGD( "AF-%-15s: uninitMcuDrv - Dev: %d", __FUNCTION__, m_i4CurrSensorDev);
        m_pMcuDrv->uninit( m_i4CurrSensorDev);
        m_pMcuDrv->destroyInstance();
        m_pMcuDrv = NULL;
    }

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::triggerAF( MUINT32 u4Caller)
{
    if( u4Caller==AF_MGR_CALLER)
        CAM_LOGD( "%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);
    else
        CAM_LOGD( "cmd-%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    return S_AF_OK;
    UpdateState( EVENT_CMD_TRIGGERAF_WITH_AE_STBL);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WaitTriggerAF( MBOOL &bWait)
{
    if( m_i4EnableAF==0)
    {
        return S_AF_OK;
    }
    if( bWait==MTRUE)
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
MRESULT AfMgr::setAFMode(MINT32 eAFMode, MUINT32 u4Caller)
{
    if( m_i4EnableAF==0 )
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "AF_En==0");
        return S_AF_OK;
    }
    else if ( m_eCurAFMode == eAFMode )
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "Same AF Mode");
        return S_AF_OK;
    }

    LogCollector LC(LOG_TAG);
    LC.Add("#(%5d,%5d)", m_u4ReqMagicNum,  m_u4StaMagicNum);
    if(u4Caller==AF_CMD_CALLER)
    {
        LC.Add(" cmd-");
    }
    LC.Add("%s Dev(%d):ctl_afmode(%d -> %d) TG(%d)",
           __FUNCTION__,
           m_i4CurrSensorDev,
           m_sHostCmd.u4AfMode,
           eAFMode,
           m_eSensorTG);
    LC.Print();

    m_sHostCmd.u4AfMode = eAFMode;
    ExeSetAFMode(m_sHostCmd.u4AfMode);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::ExeSetAFMode(MINT32 eAFMode)
{
    LogCollector LC(LOG_TAG);
    LC.Add("#(%5d,%5d) cmd-%s Dev(%d): ",
           m_u4ReqMagicNum,
           m_u4StaMagicNum,
           __FUNCTION__,
           m_i4CurrSensorDev);
    LC.Add( "ctl_afmode(%d) TG(%d) ",
            eAFMode,
            m_eSensorTG);

    m_bIsFullScan  = FALSE;
    m_eCurAFMode = eAFMode;

    //Set mode to hybrid AF algorithm
    if (m_pIAfAlgo)
    {
        LIB3A_AF_MODE_T preLib3A_AfMode = m_eLIB3A_AFMode;
        switch (m_eCurAFMode)
        {
        case MTK_CONTROL_AF_MODE_OFF :
            m_eLIB3A_AFMode = LIB3A_AF_MODE_MF;  /*API2:The auto-focus routine does not control the lens. Lens  is controlled by the application.*/
            break;
        case MTK_CONTROL_AF_MODE_AUTO :
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS; // AF-Single Shot Mode
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
        default:
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
            break;
        }
        LC.Add( "lib_afmode %d->%d ",
                preLib3A_AfMode,
                m_eLIB3A_AFMode);
    }
    else
    {
        LC.Add( "Null algo ptr");
    }
    LC.Print();
    //Set mode to hybrid AF algorithm
    if( m_pIAfAlgo)
        m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);

    //update parameters and status.
    UpdateState( EVENT_CMD_CHANGE_MODE);
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::LockAlgo(MUINT32 u4Caller)
{
    if( u4Caller==AF_MGR_CALLER)
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
    if( u4Caller==AF_MGR_CALLER)
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
NS3A::EAfState_T AfMgr::UpdateStateAutoMode( NS3A::EAfState_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    NS3A::EAfState_T NewState = NS3A::E_AF_INACTIVE;
    switch( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        NewState = NS3A::E_AF_INACTIVE;
        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MTRUE here to wait trigger searching..
         */
        m_bNeedCheckSendCallback = MFALSE;
        m_i4TAFStatus   = TAF_STATUS_RESET;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if(      eInCurSate == NS3A::E_AF_INACTIVE          ) NewState = NS3A::E_AF_ACTIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_FOCUSED_LOCKED    ) NewState = NS3A::E_AF_ACTIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_NOT_FOCUSED_LOCKED) NewState = NS3A::E_AF_ACTIVE_SCAN;
        else                                                  NewState = eInCurSate; /*Should be at E_AF_ACTIVE_SCAN*/

        // get Autofocus when [(CenterROI exist) or (AE locked)] ==> ForceDoAlgo to speed up SEARCH_END
        if(m_i4IsCAFWithoutFace==1 || i4IsLockAERequest==1)
        {
            m_bForceDoAlgo = MTRUE;
        }
        m_sCallbackInfo.isAfterAutoMode = 1;
        m_i4TAFStatus = TAF_STATUS_RECEIVE_AUTOFOCUS;
        break;

    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        if( m_pIAfAlgo)
        {
            m_pIAfAlgo->trigger();
        }
        break;

    case EVENT_CMD_AUTOFOCUS_CANCEL :
        if(      eInCurSate == NS3A::E_AF_ACTIVE_SCAN       ) NewState = NS3A::E_AF_INACTIVE;
        else if( eInCurSate == NS3A::E_AF_FOCUSED_LOCKED    ) NewState = NS3A::E_AF_INACTIVE;
        else if( eInCurSate == NS3A::E_AF_NOT_FOCUSED_LOCKED) NewState = NS3A::E_AF_INACTIVE;
        else                                                  NewState = eInCurSate; /*Should be at E_AF_INACTIVE*/

        if( m_i4TAFStatus == TAF_STATUS_RECEIVE_AUTOFOCUS)
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
        break;

    case EVENT_SEARCHING_START :
        NewState = NS3A::E_AF_ACTIVE_SCAN;
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_END :
        m_bLatchROI = MFALSE;
        if( eInCurSate==NS3A::E_AF_ACTIVE_SCAN)
        {
            if( m_i4IsFocused==1) NewState = NS3A::E_AF_FOCUSED_LOCKED;
            else                  NewState = NS3A::E_AF_NOT_FOCUSED_LOCKED;
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
NS3A::EAfState_T AfMgr::UpdateStateContinuousMode( NS3A::EAfState_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    NS3A::EAfState_T NewState = NS3A::E_AF_INACTIVE;
    switch( sInEvent)
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
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

        m_bNeedCheckSendCallback = MTRUE;

        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;

        if( eInCurSate==NS3A::E_AF_INACTIVE || m_i4LaunchCamTriggered!=E_LAUNCH_AF_DONE)
        {
            // To prevent CTS fail because of the wrong af state while switching mode w/o trigger (original: NS3A::E_AF_INACTIVE)
            NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
            if( m_i4TAFStatus == TAF_STATUS_RECEIVE_CANCELAUTOFOCUS)
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
        else if( eInCurSate==NS3A::E_AF_ACTIVE_SCAN       ) NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
        else if( eInCurSate==NS3A::E_AF_FOCUSED_LOCKED    ) NewState = NS3A::E_AF_PASSIVE_FOCUSED;
        else if( eInCurSate==NS3A::E_AF_NOT_FOCUSED_LOCKED) NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
        else                                                NewState = eInCurSate;

        m_i4TAFStatus = TAF_STATUS_RESET;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if( m_bForceTrigger==MTRUE)
        {
            // force to trigger searching when changing ROI at continuous mode.
            NewState = NS3A::E_AF_PASSIVE_SCAN;
            CAM_LOGD( "Wait force trigger and lock until searching done");
            m_bNeedLock = MTRUE;
        }
        else
        {
            if(      eInCurSate==NS3A::E_AF_INACTIVE          ) NewState = NS3A::E_AF_NOT_FOCUSED_LOCKED;
            else if( eInCurSate==NS3A::E_AF_PASSIVE_SCAN      )
            {
                if( m_eCurAFMode==MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
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
                    NewState = NS3A::E_AF_NOT_FOCUSED_LOCKED;
                }
            }
            else if( eInCurSate==NS3A::E_AF_PASSIVE_FOCUSED   ) NewState = NS3A::E_AF_FOCUSED_LOCKED;
            else if( eInCurSate==NS3A::E_AF_PASSIVE_UNFOCUSED ) NewState = NS3A::E_AF_NOT_FOCUSED_LOCKED;
            else if( eInCurSate==NS3A::E_AF_FOCUSED_LOCKED    ) NewState = NS3A::E_AF_FOCUSED_LOCKED;
            else if( eInCurSate==NS3A::E_AF_NOT_FOCUSED_LOCKED) NewState = NS3A::E_AF_NOT_FOCUSED_LOCKED;
            else                                                NewState = eInCurSate; /*Should not be happened*/

            if( m_eEvent&EVENT_CMD_CHANGE_MODE)
            {
                NewState = NS3A::E_AF_PASSIVE_SCAN;
                CAM_LOGD( "Got changing mode and AF_TRIGGER at same time, force trigger searching");
                if( m_pIAfAlgo)
                {
                    m_pIAfAlgo->cancel();
                    m_pIAfAlgo->trigger();
                }
            }

            if( NewState != NS3A::E_AF_PASSIVE_SCAN)
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
        if (!((m_bForceTrigger==MTRUE && m_bTriggerCmdVlid==MTRUE) && m_bWaitForceTrigger==MFALSE))
        {
            NewState = eInCurSate;
            break;
        }
        else
        {
            // LaunchCamTrigger
            if(m_i4LaunchCamTriggered==E_LAUNCH_AF_WAITING)
            {
                if(m_i4AEStableFrameCount==-1)
                {
                    m_i4AEStableFrameCount = m_u4ReqMagicNum;
                    CAM_LOGD("#(%5d,%5d) %s Dev(%d) LaunchCamTrigger EVENT_AE_IS_STABLE(%d) - BUT NOT TRIGGER YET",
                             m_u4ReqMagicNum, m_u4StaMagicNum,
                             __FUNCTION__,
                             m_i4CurrSensorDev,
                             m_i4AEStableFrameCount);
                }

                if(m_i4AEStableFrameCount != -1 && m_u4ReqMagicNum >= (m_i4AEStableFrameCount + m_i4AEStableTriggerTimeout))
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
        if( m_bForceTrigger==MTRUE && m_bTriggerCmdVlid==MTRUE)
        {
            NewState = eInCurSate;

            m_bTriggerCmdVlid = MFALSE;
            // force to trigger searching when changing ROI at continuous mode.
            CAM_LOGD( "Force trigger searching [NeedLock(%d) LatchROI(%d) isAFSearch(%d)]", m_bNeedLock, m_bLatchROI, m_i4IsAFSearch_CurState);
            if( m_pIAfAlgo)
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
        if( eInCurSate==NS3A::E_AF_FOCUSED_LOCKED)
        {
            NewState = NS3A::E_AF_PASSIVE_FOCUSED;
            UnlockAlgo();
        }
        else if( eInCurSate==NS3A::E_AF_NOT_FOCUSED_LOCKED)
        {
            NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
            UnlockAlgo();
        }
        else if( eInCurSate == NS3A::E_AF_PASSIVE_SCAN)
        {
            NewState = NS3A::E_AF_INACTIVE;
            CAM_LOGD( "Abort search");
            if( m_pIAfAlgo)
            {
                m_pIAfAlgo->cancel();
            }
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
        if( eInCurSate==NS3A::E_AF_PASSIVE_SCAN)
        {
            NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
            if( m_pIAfAlgo)
            {
                m_pIAfAlgo->cancel();
            }
        }
        else
        {
            NewState = eInCurSate;
        }

        if( eInCurSate!=NS3A::E_AF_FOCUSED_LOCKED && eInCurSate!=NS3A::E_AF_NOT_FOCUSED_LOCKED)
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
        if(m_sCallbackInfo.isAfterAutoMode>0)
        {
            m_sCallbackInfo.isAfterAutoMode--;
        }
        if(      eInCurSate == NS3A::E_AF_INACTIVE         ) NewState = NS3A::E_AF_PASSIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_PASSIVE_FOCUSED  ) NewState = NS3A::E_AF_PASSIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_PASSIVE_UNFOCUSED) NewState = NS3A::E_AF_PASSIVE_SCAN;
        else                                                 NewState = eInCurSate;
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;
        CAM_LOGD( "isAFSearch[%d]", m_i4IsAFSearch_CurState);
        if( m_i4IsAFSearch_CurState)
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
        if( m_bTriggerCmdVlid==MTRUE && m_bWaitForceTrigger==MTRUE)
        {
            NewState = NS3A::E_AF_PASSIVE_SCAN;
        }
        else
        {
            //reset parameters
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

            if( m_bNeedLock==MTRUE)
            {
                if( m_i4IsFocused==1) NewState = NS3A::E_AF_FOCUSED_LOCKED;
                else                  NewState = NS3A::E_AF_NOT_FOCUSED_LOCKED;

                //lock
                LockAlgo();
                m_bNeedLock = MFALSE;

            }
            else if( eInCurSate == NS3A::E_AF_PASSIVE_SCAN)
            {
                if( m_i4IsFocused==1) NewState = NS3A::E_AF_PASSIVE_FOCUSED;
                else                  NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
            }
            else
            {
                NewState = eInCurSate;

                if( sInEvent==EVENT_CMD_STOP && eInCurSate == NS3A::E_AF_INACTIVE)
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
        if( m_bTriggerCmdVlid==MTRUE && m_bWaitForceTrigger==MTRUE)
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
        if( eInCurSate!=NS3A::E_AF_FOCUSED_LOCKED && eInCurSate!=NS3A::E_AF_NOT_FOCUSED_LOCKED)
        {
            m_bTriggerCmdVlid = m_bForceTrigger = m_bWaitForceTrigger = MTRUE;
            CAM_LOGD( "Set wait force trigger for preCap");
            if( m_i4IsAFSearch_CurState==AF_SEARCH_DONE)
            {
                LockAlgo();
            }
            // send callback for the next two times
            if(m_sCallbackInfo.isAfterAutoMode==1)
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
NS3A::EAfState_T AfMgr::UpdateStateOFFMode( NS3A::EAfState_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    NS3A::EAfState_T NewState = NS3A::E_AF_INACTIVE;
    switch( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        NewState = NS3A::E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        m_bNeedCheckSendCallback = MFALSE;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        if( m_pIAfAlgo)
        {
            m_pIAfAlgo->trigger();
        }
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
        NewState = NS3A::E_AF_INACTIVE;
        break;
    }

    return NewState;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NS3A::EAfState_T AfMgr::UpdateStateMFMode( NS3A::EAfState_T &eInCurSate, AF_EVENT_T &sInEvent)
{
    NS3A::EAfState_T NewState = NS3A::E_AF_INACTIVE;
    switch( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        m_bNeedCheckSendCallback = MFALSE;
        NewState = NS3A::E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        if( m_pIAfAlgo)
        {
            m_pIAfAlgo->trigger();
        }
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
        NewState = NS3A::E_AF_INACTIVE;
        break;
    }

    return NewState;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NS3A::EAfState_T AfMgr::UpdateState( AF_EVENT_T sInEvent)
{
    NS3A::EAfState_T NewState;

    switch( sInEvent)
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

    switch( m_eCurAFMode)
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

    if( m_eAFState!=NewState)
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
NS3A::EAfState_T AfMgr::getAFState()
{
    NS3A::EAfState_T ret;

    ret = m_eAFState;

    if( m_eAFStatePre!=ret)
    {
        CAM_LOGD( "cmd-%s Dev %d : %d->%d",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_eAFStatePre,
                  ret);
    }

    m_eAFStatePre = ret;


    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
MRESULT AfMgr::getFocusAreaResult( android::Vector<MINT32> &vecOutPos, android::Vector<MUINT8> &vecOutRes, MINT32 &i4OutSzW, MINT32 &i4OutSzH)
{
    // the FocusArea would be updated to the HalMeta
    // FocusArea: (X,Y,W,H) -> (L,T,R,B)
    // (X,Y) = Center point of the ROI

    vecOutPos.clear();
    vecOutRes.clear();
    i4OutSzW = 0;
    i4OutSzH = 0;

    if( m_bMZAFEn)
    {
        MUINT32 num = static_cast<MUINT32>(m_sAFOutput.sROIStatus.i4TotalNum);
        if( num<=MAX_MULTI_ZONE_WIN_NUM)
        {
            MINT32 x,y;
            MUINT8 res;
            for( MUINT32 i=0; i<num; i++)
            {
                x   = m_sAFOutput.sROIStatus.sROI[i].i4X + m_sAFOutput.sROIStatus.sROI[i].i4W/2;
                y   = m_sAFOutput.sROIStatus.sROI[i].i4Y + m_sAFOutput.sROIStatus.sROI[i].i4H/2;
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

    CAM_LOGD_IF( m_i4DgbLogLv&4,
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


    if( m_bMZAFEn)
    {
        MINT32 type = 0;
        vecOut.push_back(type);

        MUINT32 num = static_cast<MUINT32>(m_sAFOutput.sROIStatus.i4TotalNum);
        if( (MAX_MULTI_ZONE_WIN_NUM<num))
        {
            num = 0;
        }
        vecOut.push_back(num);

        for( MUINT32 i=0; i<num; i++)
        {
            vecOut.push_back( m_sAFOutput.sROIStatus.sROI[i].i4X);
            vecOut.push_back( m_sAFOutput.sROIStatus.sROI[i].i4Y);
            vecOut.push_back((m_sAFOutput.sROIStatus.sROI[i].i4X+m_sAFOutput.sROIStatus.sROI[i].i4W));
            vecOut.push_back((m_sAFOutput.sROIStatus.sROI[i].i4Y+m_sAFOutput.sROIStatus.sROI[i].i4H));
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

        MINT32 num  = 1;
        vecOut.push_back(num);

        vecOut.push_back( m_sArea_HW.i4X);
        vecOut.push_back( m_sArea_HW.i4Y);
        vecOut.push_back((m_sArea_HW.i4X+m_sArea_HW.i4W));
        vecOut.push_back((m_sArea_HW.i4Y+m_sArea_HW.i4H));
        vecOut.push_back( m_sArea_TypeSel);
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "%s ROI X(%4d), Y(%4d), W(%4d), H(%4d), Res(%d)",
                     __FUNCTION__,
                     m_sArea_HW.i4X,
                     m_sArea_HW.i4Y,
                     m_sArea_HW.i4W,
                     m_sArea_HW.i4H,
                     m_sArea_TypeSel);
    }

    return S_AF_OK;
}
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFArea(CameraFocusArea_T &sInAFArea)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): [Cnt]%d [L]%d [R]%d [U]%d [B]%d",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              sInAFArea.u4Count,
              sInAFArea.rAreas[0].i4Left,
              sInAFArea.rAreas[0].i4Right,
              sInAFArea.rAreas[0].i4Top,
              sInAFArea.rAreas[0].i4Bottom);

    memcpy( &m_sHostCmd.rFocusAreas, &sInAFArea, sizeof(CameraFocusArea_T));
    ExeSetAFArea(m_sHostCmd.rFocusAreas);

    return S_AF_OK;
}

MRESULT AfMgr::ExeSetAFArea(CameraFocusArea_T sInAFArea)
{
    /* sInAFArea is android defined coordinate (-1000~1000)*/
    MRESULT ret = E_3A_ERR;

    LogCollector LC(LOG_TAG);
    LC.Add( "#(%5d,%5d) cmd-%s : ",
            m_u4ReqMagicNum,
            m_u4StaMagicNum,
            __FUNCTION__);
    LC.Add( "TGSZ %d, %d, [Cnt]%d (L,R,U,B)=(%d,%d,%d,%d) ",
            m_i4TGSzW,
            m_i4TGSzH,
            sInAFArea.u4Count,
            sInAFArea.rAreas[0].i4Left,
            sInAFArea.rAreas[0].i4Right,
            sInAFArea.rAreas[0].i4Top,
            sInAFArea.rAreas[0].i4Bottom);


    if( 1<=sInAFArea.u4Count)
    {
        /* Transform AF area coordinate to TG base*/
        for (MINT32 i=0; i<sInAFArea.u4Count; i++)
        {
            sInAFArea.rAreas[i].i4Left   = (sInAFArea.rAreas[i].i4Left   +1000) * m_sCropRegionInfo.i4W  / 2000 + m_sCropRegionInfo.i4X;
            sInAFArea.rAreas[i].i4Right  = (sInAFArea.rAreas[i].i4Right  +1000) * m_sCropRegionInfo.i4W  / 2000 + m_sCropRegionInfo.i4X;
            sInAFArea.rAreas[i].i4Top    = (sInAFArea.rAreas[i].i4Top    +1000) * m_sCropRegionInfo.i4H  / 2000 + m_sCropRegionInfo.i4Y;
            sInAFArea.rAreas[i].i4Bottom = (sInAFArea.rAreas[i].i4Bottom +1000) * m_sCropRegionInfo.i4H  / 2000 + m_sCropRegionInfo.i4Y;
        }

        if( /* boundary check*/
            (sInAFArea.rAreas[0].i4Left<sInAFArea.rAreas[0].i4Right ) &&
            (sInAFArea.rAreas[0].i4Top <sInAFArea.rAreas[0].i4Bottom) &&
            (0<=sInAFArea.rAreas[0].i4Left && sInAFArea.rAreas[0].i4Right<=(MINT32)m_i4TGSzW) &&
            (0<=sInAFArea.rAreas[0].i4Top  && sInAFArea.rAreas[0].i4Bottom<=(MINT32)m_i4TGSzH))
        {
            AREA_T roi = AREA_T( sInAFArea.rAreas[0].i4Left,
                                 sInAFArea.rAreas[0].i4Top,
                                 sInAFArea.rAreas[0].i4Right  - sInAFArea.rAreas[0].i4Left,
                                 sInAFArea.rAreas[0].i4Bottom - sInAFArea.rAreas[0].i4Top,
                                 AF_MARK_NONE);

            if( memcmp( &roi, &m_sArea_APCheck, sizeof(AREA_T))!=0)
            {
                //store command.
                m_sArea_APCheck = m_sArea_APCmd = roi;

                m_sPDRois[eIDX_ROI_ARRAY_AP].info.sType  = ROI_TYPE_AP;
                m_sPDRois[eIDX_ROI_ARRAY_AP].info.sPDROI = m_sArea_APCmd;

                UpdateState( EVENT_CMD_SET_AF_REGION);

                LC.Add(" =(new ROI)=> (X,Y,W,H)=(%d,%d,%d,%d)",
                       m_sArea_APCmd.i4X,
                       m_sArea_APCmd.i4Y,
                       m_sArea_APCmd.i4W,
                       m_sArea_APCmd.i4H);
                LC.Print();

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
                LC.Add(m_i4DgbLogLv&2, "(same ROI)");
            }
        }
        else
        {
            //command is not vaild, using center window.
            LC.Add(m_i4DgbLogLv&2, "ROI is invalid");
        }
    }
    else
    {
        //command is not vaild, using center window.
        LC.Add(m_i4DgbLogLv&2, "ROI cnt=0!!");
    }

    if( ret==E_3A_ERR)
    {
        LC.Print(m_i4DgbLogLv&2);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCamScenarioMode(MUINT32 a_eCamScenarioMode)
{
    /**********************************************************
     *
     * This function should be called before af mgr is started!!
     *
     **********************************************************/
    // [change mode] or [unvalid mode]
    if( (a_eCamScenarioMode!=m_eCamScenarioMode) || (m_eCamScenarioMode>=AF_CAM_SCENARIO_NUM))
    {
        // if original CamScenarioMode is unvalid, reset it to CAM_SCENARIO_PREVIEW
        if(m_eCamScenarioMode>=AF_CAM_SCENARIO_NUM)
        {
            m_eCamScenarioMode = CAM_SCENARIO_PREVIEW;
        }
        CAM_LOGD("[setCamScenarioMode] %d -> %d", m_eCamScenarioMode, a_eCamScenarioMode);

        if ( a_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
        {
            if( m_ptrLensNVRam != NULL)
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
                    if(m_ptrNVRam->rAFNVRAM.i4EasyTuning[10]>0)  // the timeout of AEStable-wait-Face is not default value
                    {
                        m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10];
                    }
                    if(m_ptrNVRam->rAFNVRAM.i4EasyTuning[11]>0)  // the timeout of ValidPD-wait-Face is not default value
                    {
                        m_i4ValidPDTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11];
                    }

                    // checkSendCallback info : update the target form NVRAM
                    m_sCallbackInfo.CompSet_PDCL.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[20];
                    m_sCallbackInfo.CompSet_FPS.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[21];
                    m_sCallbackInfo.CompSet_ISO.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[22];

                    if( m_pIAfAlgo)
                    {
                        AF_CONFIG_T const *ptrHWCfgDef = NULL;
                        getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
                        m_pIAfAlgo->setAFParam( (*m_pAFParam), (*ptrHWCfgDef), m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);
                        // initial hybrid AF algorithm
                        m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

                        if( m_bLock!=MTRUE)
                        {
                            m_pIAfAlgo->cancel();
                        }
                    }
                    // Get AF calibration data. Should do this after setAFParam is called.
                    if( m_ptrNVRam->rAFNVRAM.i4ReadOTP==TRUE)
                    {
                        readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
                    }
                }
                else
                {
                    CAM_LOGD( "[%s] tuning param is the same, no need update to algo\n", __FUNCTION__);
                }
            }
            else
            {
                CAM_LOGD( "[%s] NVRAM is not initialized, update m_eCamScenarioMode only, no need update to algo\n", __FUNCTION__);
            }

            //
            m_eCamScenarioMode = a_eCamScenarioMode;
        }
        else
        {
            CAM_LOGD( "#(%5d,%5d) %s Camera Scenario mode invalid (%d), no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, a_eCamScenarioMode);
        }

    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetCropRegionInfo( MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height, MUINT32 u4Caller)
{
    if( u4Width == 0 || u4Height == 0 )
    {
        return S_AF_OK;
    }

    /* Digital Zoom : skip crop reggion info durning AF seraching */
    if( m_i4IsAFSearch_CurState!=AF_SEARCH_DONE)
    {
        return S_AF_OK;
    }

    if( (u4XOffset<m_i4TGSzW) &&
            (u4YOffset<m_i4TGSzH) &&
            (u4XOffset+u4Width<=m_i4TGSzW) &&
            (u4YOffset+u4Height<=m_i4TGSzH) )
    {
        // set crop region information and update center ROI automatically.
        if( m_sCropRegionInfo.i4X != (MINT32)u4XOffset ||
                m_sCropRegionInfo.i4Y != (MINT32)u4YOffset ||
                m_sCropRegionInfo.i4W != (MINT32)u4Width   ||
                m_sCropRegionInfo.i4H != (MINT32)u4Height )
        {
            m_sCropRegionInfo.i4X = (MINT32)u4XOffset;
            m_sCropRegionInfo.i4Y = (MINT32)u4YOffset;
            m_sCropRegionInfo.i4W = (MINT32)u4Width;
            m_sCropRegionInfo.i4H = (MINT32)u4Height;

            //calculate zoom information : 1X-> 100, 2X->200, ...
            MUINT32 dzfX = 100*m_i4TGSzW/m_sCropRegionInfo.i4W;
            MUINT32 dzfY = 100*m_i4TGSzH/m_sCropRegionInfo.i4H;
            //Should be the same.
            m_i4DZFactor = dzfX<dzfY ? dzfX : dzfY;

            if( u4Caller==AF_MGR_CALLER)
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
            m_sArea_Focusing = m_sArea_APCheck = m_sArea_APCmd = m_sArea_OTFD = m_sArea_HW = m_sArea_Center;
            m_sArea_TypeSel = ROI_TYPE_CENTER;

            // it is always valid for calculation center roi once crop region is set.
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].valid       = MTRUE;
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sType  = ROI_TYPE_CENTER;
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sPDROI = m_sArea_Center;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv&2, "%s same cmd", __FUNCTION__);
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
    if(      m_i4DzWinCfg<1) m_i4DzWinCfg = 1;
    else if( 4<m_i4DzWinCfg) m_i4DzWinCfg = 4;

    // scale should be same as m_i4DZFactor
    m_i4DzWinCfg *= 100;
    CAM_LOGD_IF( m_i4DgbLogLv&2,
                 "%s [DZ]%d, [Cfg]%d",
                 __FUNCTION__,
                 m_i4DZFactor,
                 m_i4DzWinCfg);


    if( m_i4DzWinCfg==400)
    {
        // i4WinCfg=4-> DigZoomFac>4, AF win align digital effect.
    }
    else if( m_bMZAFEn==MFALSE)
    {
        /**
         * i4WinCfg = 1~3
         * i4WinCfg = 1 : DigZoomFac>1, AF win no change
         * i4WinCfg = 2 : DigZoomFac>2, AF win no change
         * i4WinCfg = 3 : DigZoomFac>3, AF win no change
         */
        if( m_i4DzWinCfg <= m_i4DZFactor) /* fix to upper bound */
        {

            //scale up window.
            AREA_T scaledWin;
            scaledWin.i4W = sOutAFRegion.i4W * m_i4DZFactor/m_i4DzWinCfg;
            scaledWin.i4H = sOutAFRegion.i4H * m_i4DZFactor/m_i4DzWinCfg;
            scaledWin.i4X = (sOutAFRegion.i4X + sOutAFRegion.i4W/2) -  scaledWin.i4W/2;
            scaledWin.i4Y = (sOutAFRegion.i4Y + sOutAFRegion.i4H/2) -  scaledWin.i4H/2;

            CAM_LOGD_IF( m_i4DgbLogLv&2,
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
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCamMode(MINT32 a_eCamMode)
{
    CAM_LOGD("[setCamMode]%d\n", a_eCamMode);
    m_sAFInput.i4IsVDO= FALSE;

    if(a_eCamMode == eAppMode_VideoMode)
    {
        m_sAFInput.i4IsVDO= TRUE;
        m_sAFInput.i4IsZSD = FALSE;
    }
    else
        m_sAFInput.i4IsVDO= FALSE;

    if(m_pAFParam->i4AFS_MODE==2)  //AF auto mode always don't AFC
        m_sAFInput.i4IsVDO= FALSE;

    MINT32 AFtbl_num = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height, MUINT32 u4Caller)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): X(%d) Y(%d) W(%d) H(%d)",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              u4XOffset,
              u4YOffset,
              u4Width,
              u4Height);

    m_sHostCmd.rCropRegion.i4X = u4XOffset;
    m_sHostCmd.rCropRegion.i4Y = u4YOffset;
    m_sHostCmd.rCropRegion.i4W = u4Width;
    m_sHostCmd.rCropRegion.i4H = u4Height;

    SetCropRegionInfo(m_sHostCmd.rCropRegion.i4X, m_sHostCmd.rCropRegion.i4Y, m_sHostCmd.rCropRegion.i4W, m_sHostCmd.rCropRegion.i4H, AF_CMD_CALLER);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setOTFDInfo( MVOID* sInROIs, MINT32 i4Type)
{
    MRESULT ret=E_3A_ERR;

    // sInROIs is android defined coordinate (-1000~1000)
    MtkCameraFaceMetadata *ptrWins = (MtkCameraFaceMetadata *)sInROIs;

    //prepare information and set to hybrid AF
    AF_AREA_T sAreaInfo;
    sAreaInfo.i4Score  = i4Type!=0 ? ptrWins->faces->score : 0;  /*for algorithm to check input is FD or object, 0:face, 100:object*/
    sAreaInfo.i4Count  = 0;
    sAreaInfo.sRect[0] = AREA_T( 0, 0, 0, 0, AF_MARK_NONE);

    if( ptrWins!=NULL)
    {
        if( ptrWins->number_of_faces!=0)
        {
            if( ptrWins->faces!=NULL)
            {
                //TODO : currently always get the first ROI for focusing.

                // android defined coordinate -> TG base coordinate
                MINT32 Left   = (ptrWins->faces->rect[0]+1000) * m_sCropRegionInfo.i4W / 2000 + m_sCropRegionInfo.i4X;
                MINT32 Right  = (ptrWins->faces->rect[2]+1000) * m_sCropRegionInfo.i4W / 2000 + m_sCropRegionInfo.i4X;
                MINT32 Top    = (ptrWins->faces->rect[1]+1000) * m_sCropRegionInfo.i4H / 2000 + m_sCropRegionInfo.i4Y;
                MINT32 Bottom = (ptrWins->faces->rect[3]+1000) * m_sCropRegionInfo.i4H / 2000 + m_sCropRegionInfo.i4Y;
                // rect => 0:left, 1:top, 2:right, 3:bottom
                MINT32 i4X = Left;
                MINT32 i4Y = Top;
                MINT32 i4W = Right - Left;
                MINT32 i4H = Bottom - Top;
                // scale OTFD ROI
                MINT32 i4scale = m_ptrNVRam->rAFNVRAM.i4FDWinPercent&0xFFFF;
                if( i4scale > 150)
                {
                    i4scale = 150;
                }
                else if( i4scale < 0)
                {
                    i4scale = 100;
                }
                // scaling
                MINT32 scaledW = i4W*i4scale/100;
                MINT32 scaledH = i4H*i4scale/100;
                MINT32 scaledX = (i4X+i4W/2) - scaledW/2;
                MINT32 scaledY = (i4Y+i4H/2) - scaledH/2;

                if( (scaledW!=0) || (scaledY!=0))
                {
                    // OTFD data is valid.
                    sAreaInfo.i4Count  = 1;
                    sAreaInfo.sRect[0] = AREA_T( scaledX, scaledY, scaledW, scaledH, AF_MARK_NONE);
                    // new FD info +
                    sAreaInfo.i4Id[0]        = ptrWins->faces[0].id;
                    // fd roi is calculated once a FD region is set
                    m_sPDRois[eIDX_ROI_ARRAY_FD].valid       = MFALSE; //[TODO] MTRUE;
                    m_sPDRois[eIDX_ROI_ARRAY_FD].info.sType  = ROI_TYPE_FD;
                    m_sPDRois[eIDX_ROI_ARRAY_FD].info.sPDROI = sAreaInfo.sRect[0]; // FDROI before extended
#if 0
                    sAreaInfo.i4Type[0]      = ptrWins->faces_type[0];
                    sAreaInfo.i4Motion[0][0] = ptrWins->motion[0][0];
                    sAreaInfo.i4Motion[0][1] = ptrWins->motion[0][1];
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
#endif
                    // new FD info -
                    //latch last vaild FD information.
                    m_sArea_OTFD = sAreaInfo.sRect[0];

                    CAM_LOGD( "[%s]cnt:%d, type %d, scale:%d, [X]%d [Y]%d [W]%d [H]%d -> [X]%d [Y]%d [W]%d [H]%d",
                              __FUNCTION__,
                              sAreaInfo.i4Count,
                              sAreaInfo.i4Score,
                              i4scale,
                              i4X,
                              i4Y,
                              i4W,
                              i4H,
                              scaledX,
                              scaledY,
                              scaledW,
                              scaledH);

                    ret = S_AF_OK;
                }
                else
                {
                    CAM_LOGD_IF( m_i4OTFDLogLv!=1, "[%s] data is not valid", __FUNCTION__);
                    m_i4OTFDLogLv = 1;
                }
            }
            else
            {
                CAM_LOGD_IF( m_i4OTFDLogLv!=2, "[%s] data is NULL ptr", __FUNCTION__);
                m_i4OTFDLogLv = 2;
            }
        }
        else
        {
            CAM_LOGD_IF( m_i4OTFDLogLv!=3, "[%s] num 0", __FUNCTION__);
            m_i4OTFDLogLv = 3;
        }
    }
    else
    {
        CAM_LOGD_IF( m_i4OTFDLogLv!=4, "[%s] Input NULL ptr", __FUNCTION__);
        m_i4OTFDLogLv = 4;
    }
    if(m_pIAfAlgo)
    {
        m_pIAfAlgo->setFDWin( sAreaInfo);
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFDInfo(MVOID* a_sFaces)
{
    MRESULT ret = E_3A_ERR;

    if ( m_pIAfAlgo!=NULL)
    {
        ret = setOTFDInfo( a_sFaces, 0);
    }
    else
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): Null algo ptr", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setOTInfo(MVOID* a_sOT)
{
    MRESULT ret = E_3A_ERR;

    if ( m_pIAfAlgo!=NULL)
    {
        ret = setOTFDInfo( a_sOT, 1);
    }
    else
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): Null algo ptr", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
    }

    return ret;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFMaxAreaNum()
{
    if (m_i4AFMaxAreaNum == -1)
    {
        IHalSensorList* const pIHalSensorList = IHalSensorList::get();

        SensorStaticInfo rSensorStaticInfo;
        switch( m_i4CurrSensorDev)
        {
        case ESensorDev_Main:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
            break;
        case ESensorDev_SubSecond:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
            break;
        default:
            CAM_LOGE( "Invalid sensor device: %d", m_i4CurrSensorDev);
            break;
        }

        m_i4AFMaxAreaNum = MCUDrv::isSupportLens( m_i4CurrSensorDev, rSensorStaticInfo.sensorDevID)==0 ? 0 : AF_WIN_NUM_SPOT;

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
            m_i4IsEnableFVInFixedFocus = property_get_int32("vendor.debug.af_fv.switch", 0 /*ForceEnableFVInFixedFocus(m_i4CurrSensorDev)*/);

            CAM_LOGD( "[%s] IsEnableFVInFixedFocus(%d)", __FUNCTION__, m_i4IsEnableFVInFixedFocus);
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

    if( m_i4EnableAF==0)
    {
        maxLensPos = 0;
    }
    else
    {
        if( m_pMcuDrv)
        {
            mcuMotorInfo MotorInfo;
            m_pMcuDrv->getMCUInfo( &MotorInfo,m_i4CurrSensorDev);
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

    if( m_i4EnableAF==0)
    {
        minLensPos = 0;
    }
    else
    {
        if( m_pMcuDrv)
        {
            mcuMotorInfo MotorInfo;
            m_pMcuDrv->getMCUInfo( &MotorInfo,m_i4CurrSensorDev);
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
    return (m_sAFOutput.i4IsAfSearch==AF_SEARCH_DONE)?1:0;
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
MRESULT AfMgr::setMFPos(MINT32 a_i4Pos, MUINT32 u4Caller)
{
    LogCollector LC(LOG_TAG);
    LC.Add("#(%5d,%5d) ",
           m_u4ReqMagicNum,
           m_u4StaMagicNum);
    if( u4Caller==AF_CMD_CALLER)
    {
        LC.Add("cmd-");
    }
    LC.Add("%s Dev(%d): %d -> %d", __FUNCTION__,
           m_i4CurrSensorDev,
           m_sHostCmd.i4MFPos,
           a_i4Pos);
    LC.Print();

    m_sHostCmd.i4MFPos = a_i4Pos;
    ExeSetMFPos(m_sHostCmd.i4MFPos);

    return S_AF_OK;
}

MRESULT AfMgr::ExeSetMFPos(MINT32 a_i4Pos)
{
    LogCollector LC(LOG_TAG);
    LC.Add("#(%5d,%5d) %s Dev(%d): ",
           m_u4ReqMagicNum,
           m_u4StaMagicNum,
           __FUNCTION__,
           m_i4CurrSensorDev);

    if( /* check condition*/
        (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
        ((m_i4MFPos != a_i4Pos)||(m_FirstTimeAFON==0)) &&
        (0<=a_i4Pos) &&
        (a_i4Pos<=1023) &&
        (m_pIAfAlgo))
    {
        m_FirstTimeAFON = 1;
        LC.Add("MFPos:(%d)->(%d)", m_i4MFPos, a_i4Pos);
        LC.Print();
        m_i4MFPos = a_i4Pos;

        if( m_pIAfAlgo)
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
        LC.Add(m_i4DgbLogLv, "SKIP MFPos:(%d)->(%d), lib_afmode(%d), algo(%p)",
               m_sAFOutput.i4AFPos,
               a_i4Pos,
               m_eLIB3A_AFMode,
               m_pIAfAlgo);
        LC.Print(m_i4DgbLogLv);
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFullScanstep(MINT32 a_i4Step)
{
    a_i4Step = (a_i4Step & 0x0000FFFF);
    if(a_i4Step>=0 && a_i4Step<=1024)
    {
        m_sHostCmd.i4FullScanStep = a_i4Step;
    }
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): %d -> %d",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_sHostCmd.i4FullScanStep,
              a_i4Step);

    m_sHostCmd.i4FullScanStep = a_i4Step;
    ExeSetFullScanstep(m_sHostCmd.i4FullScanStep);
    return S_AF_OK;
}

MRESULT AfMgr::ExeSetFullScanstep(MINT32 a_i4Step)
{
    if( m_i4FullScanStep != a_i4Step)
    {
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "#(%5d,%5d) cmd-%s Dev(%d): %d -> %d",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     m_i4FullScanStep,
                     a_i4Step);

        m_i4FullScanStep = a_i4Step;
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

    CAM_LOGD_IF( (m_i4DgbLogLv&1),
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
        CAM_LOGD_IF( m_i4DgbLogLv,"%s", __FUNCTION__);
        rWinSize.i4Left   = 0;
        rWinSize.i4Right  = 0;
        rWinSize.i4Top    = 0;
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
MVOID AfMgr::setAE2AFInfo( AE2AFInfo_T &rAEInfo)
{
    memcpy( &m_AEInfo, &rAEInfo, sizeof(AE2AFInfo_T));

    if (m_i4EnableAF == 0)
    {
        m_sAFInput.i8GSum       = 50;
        m_sAFInput.i4ISO        = 100;
        m_sAFInput.i4IsAEStable = 1;
        m_sAFInput.i4SceneLV    = 80;
        return;
    }

    m_i4isAEStable = ((rAEInfo.i4IsAEStable==1)||(rAEInfo.i4IsAELocked==1)) ? 1: 0;

    m_sAFInput.i8GSum             = rAEInfo.iYvalue; /* [TODO] :: check should be remove or not*/
    m_sAFInput.i4ISO              = rAEInfo.i4ISO;
    m_sAFInput.i4IsAEStable       = m_i4isAEStable;
    m_sAFInput.i4SceneLV          = rAEInfo.i4SceneLV;
    m_sAFInput.i4ShutterValue     = rAEInfo.ishutterValue;
    m_sAFInput.i4IsFlashFrm       = rAEInfo.i4IsFlashFrm;
    m_sAFInput.i4AEBlockAreaYCnt  = rAEInfo.i4AEBlockAreaYCnt;
    m_sAFInput.pAEBlockAreaYvalue = rAEInfo.pAEBlockAreaYvalue;
    memcpy( m_aAEBlkVal, rAEInfo.aeBlockV, 25);

    if( m_i4isAEStable==1)
    {
        UpdateState( EVENT_AE_IS_STABLE);
    }

    if( m_i4DgbLogLv)
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
MUINT32 AfMgr::GetHybridAFMode()
{
    MUINT32 HybridAFMode = 0;

    //Depth AF
    if ((m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE) && (m_bSDAFEn == MTRUE))
    {
        HybridAFMode |= 1;
    }

    //PDAF
    if(
        ( (m_i4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)   && (m_PDBuf_Type==EPDBUF_RAW_LEGACY)                           ) ||
        ( (m_i4PDAF_support_type==PDAF_SUPPORT_RAW)          && (m_PDBuf_Type==EPDBUF_RAW_LEGACY||m_PDBuf_Type==EPDBUF_PDO) ) ||
        ( (m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV)        && (m_PDBuf_Type==EPDBUF_VC)                            ) ||
        ( (m_i4PDAF_support_type==PDAF_SUPPORT_RAW_DUALPD)   && (m_PDBuf_Type==EPDBUF_DUALPD_RAW)                    ) ||
        ( (m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV_DUALPD) && (m_PDBuf_Type==EPDBUF_DUALPD_VC)                     )  )
    {
        HybridAFMode |= 2;  //2'b 0010
    }
    else if( (m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV) && (m_PDBuf_Type==EPDBUF_VC_OPEN))
    {
        HybridAFMode |= 10; //2'b 1010
    }

    //Laser AF
    if (m_bLDAFEn == MTRUE)
    {
        HybridAFMode |= 4;
    }

    CAM_LOGD_IF( m_i4DgbLogLv&4, "%s:(0x%x)", __FUNCTION__, HybridAFMode);

    return HybridAFMode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::HybridAFPreprocessing()
{
    MINT32 EnablePD = ( ( m_PDBuf_Type!= EPDBUF_NOTDEF ) && ( m_i4PDAF_support_type != PDAF_SUPPORT_NA ) );

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d) PDEn(%d) LaserEn(%d) FDDetect(%d): ",
                              m_u4ReqMagicNum,
                              m_u4StaMagicNum,
                              __FUNCTION__,
                              m_i4CurrSensorDev,
                              EnablePD,
                              m_bLDAFEn,
                              m_sAFOutput.i4FDDetect);
    }


    /* for reference */
    AREA_T LastAFArea   = m_sArea_Focusing;
    MINT32 i4ISO        = m_sAFInput.i4ISO;
    MINT32 i4IsAEStable = m_sAFInput.i4IsAEStable;
    MUINT8 AEBlockV[25];
    memcpy( AEBlockV, m_aAEBlkVal, 25);

    /* do PD data preprocessing */
    if( EnablePD)
    {
        MINT32   PDOut_numRes = m_sAFInput.sPDInfo.i4PDBlockInfoNum;
        MUINT16 pPDAF_DAC = 0;
        MUINT16 pPDAF_Conf = 0;

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[PD (DAC, CL)] org{ ");
            for( MINT32 i=0; i<PDOut_numRes; i++)
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

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "new{ ");
            for( MINT32 i=0; i<PDOut_numRes; i++)
            {
                pPDAF_DAC = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafDacIndex;
                pPDAF_Conf = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC, pPDAF_Conf);
            }
            ptrMsgBuf += sprintf( ptrMsgBuf, "}, ");
        }

        // LaunchCamTrigger
        if(m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            if(m_i4ValidPDFrameCount==-1 && PDOut_numRes>0)
            {
                for(MINT32 i=0; i<PDOut_numRes; i++)
                {
                    pPDAF_Conf = m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                    if(pPDAF_Conf > 0)
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

            if(m_i4ValidPDFrameCount != -1 && m_u4ReqMagicNum >= (m_i4ValidPDFrameCount + m_i4ValidPDTriggerTimeout))
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
    if( m_bLDAFEn == MTRUE)
    {
        MINT32 LaserStatus = ILaserMgr::getInstance().getLaserCurStatus(m_i4CurrSensorDev);

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[Laser %d (DAC, CONF, DIST)] org:(%d, %d, %d) ",
                                  LaserStatus,
                                  m_sAFInput.sLaserInfo.i4CurPosDAC,
                                  m_sAFInput.sLaserInfo.i4Confidence,
                                  m_sAFInput.sLaserInfo.i4CurPosDist);
        }

        /*****************************  Laser result preprocessing (start) *****************************/
        {
            switch( LaserStatus)
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

            //Touch AF : if the ROI isn't in the center, the laser data need to set low confidence.
            if( (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO) )
            {
                if( abs(m_sAFInput.sLaserInfo.i4AfWinPosX) >= LASER_TOUCH_REGION_W ||
                        abs(m_sAFInput.sLaserInfo.i4AfWinPosY) >= LASER_TOUCH_REGION_H ||
                        m_sAFInput.sLaserInfo.i4AfWinPosCnt > 1 )
                {
                    m_sAFInput.sLaserInfo.i4Confidence = 20;
                    m_sAFInput.sLaserInfo.i4CurPosDAC = 0;
                }
            }

            if (1 == m_sAFOutput.i4FDDetect)
            {
                m_sAFInput.sLaserInfo.i4Confidence = 20;
                m_sAFInput.sLaserInfo.i4CurPosDAC = 0;
            }
        }
        /*****************************  Laser result preprocessing (end)  *****************************/


        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[Laser %d (DAC, CONF, DIST)] new:(%d, %d, %d) ",
                                  LaserStatus,
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
    if( (m_i4IsSelHWROI_PreState!=m_i4IsSelHWROI_CurState && m_i4IsSelHWROI_CurState==MTRUE) ||
            (m_bLatchROI==MTRUE) ||
            (m_i4DgbLogLv))
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) latch(%d) type(%d) sel(%d) issel(%d) ROI(X,Y,W,H) : Center(%d, %d, %d, %d), AP(%d, %d, %d, %d), OT(%d, %d, %d, %d), HW(%d, %d, %d, %d)",
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
                  m_sArea_HW.i4X,
                  m_sArea_HW.i4Y,
                  m_sArea_HW.i4W,
                  m_sArea_HW.i4H);
    }


    if( m_bLatchROI==MTRUE) /* Got changing ROI command from host. Should be trigger searching.*/
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

        //apply zoom information.
        ApplyZoomEffect( m_sArea_Focusing);
        //used to control select AF ROI at AFS mode.
    }
    else if( sInAFInfo.i4ROISel==AF_ROI_SEL_NONE)
    {
        /**
         *  Do nothing
         *  This case is just happened after af is inited.
         *  Wait algo to check using FD or center ROI to do focusing.
         *  Should get i4IsMonitorFV==TRUE. i4IsMonitorFV will be FALSE when ROI is selected.
         */
    }
    else if( m_i4IsSelHWROI_CurState==MTRUE) /* Without got changing ROI command from host, and need to do searching*/
    {
        switch( sInAFInfo.i4ROISel)
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
            m_sArea_Focusing = m_sArea_APCmd = m_sArea_Center; //rest AP ROI
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

        //apply zoom information.
        ApplyZoomEffect( m_sArea_Focusing);
    }

    return m_sArea_Focusing;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::doAF(MVOID *pAFStatBuf)
{
    SensorProvider_getData();

    if( m_i4EnableAF==0)
    {
        m_sAFOutput.i4IsAfSearch = AF_SEARCH_DONE;
        m_sAFOutput.i4IsFocused = 0;
        m_sAFOutput.i4AFPos = 0;
        m_bRunPDEn = MTRUE;
        CAM_LOGD_IF( m_i4DgbLogLv, "disableAF");
        return S_AF_OK;
    }


    LogCollector LC_doAF(LOG_TAG);
    LC_doAF.Add("#(%5d,%5d) %s Dev(%d): ", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);


    MUINT32 i4curFrmNum = m_u4ReqMagicNum;

    AAA_TRACE_D("doAF #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);

    //-------------------------------------------------------------------------------------------------------------
    if( m_ptrNVRam->rAFNVRAM.i4SceneMonitorLevel>0)
    {
        m_pIAfAlgo->setAEBlockInfo( m_aAEBlkVal, 25);
    }


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
    //PDAF
    //==========
    if( m_bRunPDEn==MTRUE)
    {

#define PERFRAME_GET_PD_RESULT 1

        // LaunchCamTrigger Search Done ==> get PD every 2 frames
        if(m_i4LaunchCamTriggered!=E_LAUNCH_AF_DONE)
        {
            m_bPdInputExpected = (m_i4FirsetCalPDFrameCount>0)&&(m_u4ReqMagicNum>m_i4FirsetCalPDFrameCount);
        }
        else if(m_ptrNVRam->rAFNVRAM.i4EasyTuning[2]==PERFRAME_GET_PD_RESULT)
        {
            m_bPdInputExpected = MTRUE;
        }
        else
        {
            m_bPdInputExpected = 1 - m_bPdInputExpected;
        }

        //
        m_sAFInput.sPDInfo.i4Valid = 0;
        if( m_bPdInputExpected)
        {
            //
            PD_CALCULATION_OUTPUT *ptrPDRes = nullptr;

            //
            if( IPDMgr::getInstance().getPDTaskResult( m_i4CurrSensorDev, &ptrPDRes) == S_3A_OK)
            {
                if( ptrPDRes)
                {
                    // set pd result to pd algorithm
                    memset( &m_sAFInput.sPDInfo, 0, sizeof(AFPD_INFO_T));

                    m_sAFInput.sPDInfo.i4Valid  = MTRUE;
                    m_sAFInput.sPDInfo.i4FrmNum = ptrPDRes->frmNum;

                    //
                    MINT32 numRes=0;
                    for(MINT32 i=0; i<ptrPDRes->numRes; i++)
                    {
                        if(ptrPDRes->Res[i].sROIInfo.sType==ROI_TYPE_AP)
                        {
                            if( /* for multi touch case*/
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4X != m_sArea_APCmd.i4X) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4Y != m_sArea_APCmd.i4Y) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4W != m_sArea_APCmd.i4W) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4H != m_sArea_APCmd.i4H))
                            {
                                continue;
                            }
                        }

                        m_sAFInput.sPDInfo.sPDBlockInfo[numRes].sBlockROI.sType   = ptrPDRes->Res[i].sROIInfo.sType;
                        m_sAFInput.sPDInfo.sPDBlockInfo[numRes].sBlockROI.sPDROI  = ptrPDRes->Res[i].sROIInfo.sPDROI;
                        m_sAFInput.sPDInfo.sPDBlockInfo[numRes].i4PDafDacIndex    = ptrPDRes->Res[i].Defocus;
                        m_sAFInput.sPDInfo.sPDBlockInfo[numRes].i4PDafConverge    = ptrPDRes->Res[i].PhaseDifference;
                        m_sAFInput.sPDInfo.sPDBlockInfo[numRes].i4PDafConfidence  = (MUINT32)ptrPDRes->Res[i].DefocusConfidenceLevel;
                        m_sAFInput.sPDInfo.sPDBlockInfo[numRes].i4SatPercent      = ptrPDRes->Res[i].SaturationPercent;

                        numRes++;
                    }
                    m_sAFInput.sPDInfo.i4PDBlockInfoNum = numRes;

                    //print log if necessary
                    if( m_i4DgbLogLv&2)
                    {
                        CAM_LOGD_IF( m_i4DgbLogLv&2,
                                     "%s PD Res vd(%d) frmNum(#%d) curPos(%d) winNum(%d) winInfo(fmt,x,y,w,h,targetPos,pd,cl,satPercent):",
                                     __FUNCTION__,
                                     m_sAFInput.sPDInfo.i4Valid,
                                     m_sAFInput.sPDInfo.i4FrmNum,
                                     ptrPDRes->curLensPos,
                                     m_sAFInput.sPDInfo.i4PDBlockInfoNum);

                        for(MINT32 i=0; i<ptrPDRes->numRes; )
                        {
                            char  dbgMsgBufPD[DBG_MSG_BUF_SZ];
                            char* ptrMsgBufPD = dbgMsgBufPD;

                            for(MINT32 j=0; j<3 && i < ptrPDRes->numRes; j++)
                            {
                                ptrMsgBufPD += sprintf( ptrMsgBufPD,
                                                        " #%d(%d,%4d,%4d,%4d,%4d,%4d,%6d,%3d,%d)\n",
                                                        i,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].sBlockROI.sType,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].sBlockROI.sPDROI.i4X,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].sBlockROI.sPDROI.i4Y,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].sBlockROI.sPDROI.i4W,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].sBlockROI.sPDROI.i4H,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafDacIndex,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConverge,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].i4PDafConfidence,
                                                        m_sAFInput.sPDInfo.sPDBlockInfo[i].i4SatPercent);
                                i++;
                            }
                            CAM_LOGD_IF( m_i4DgbLogLv&2, "%s", dbgMsgBufPD);

                        }
                    }

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

        if( m_i4DgbLogLv)
        {
            LC_doAF.Add( "PDExpt(%d %d)/PDVd(%d)/LaunchCam(%d)/FirsetCalPDFrameCount(%d)/",
                         m_bPdInputExpected,
                         m_ptrNVRam->rAFNVRAM.i4EasyTuning[2],
                         m_sAFInput.sPDInfo.i4Valid,
                         m_i4LaunchCamTriggered,
                         m_i4FirsetCalPDFrameCount);
        }
    }

    //---------------------------------------Run hybrid AF core flow---------------------------------------------
    if( IspMgrAFStatHWPreparing())
    {
        /* No need execute handleAF */
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
#if 0
        //get ae relate information
        ISP_SENSOR_INFO_T* ptrSensorSetting = getMatchedISInfoFromFrameId(m_u4StaMagicNum);
        if(ptrSensorSetting->i4FrameId!=-1)
        {
            m_sAFInput.i4DeltaBV          = ptrSensorSetting->i4deltaIndex;
            m_sAFInput.i4IsAEidxReset     = ptrSensorSetting->bAEScenarioChange;
            m_sAFInput.u4AEFinerEVIdxBase = ptrSensorSetting->u4AEFinerEVIdxBase;
            m_sAFInput.u4AEidxCurrentF    = ptrSensorSetting->u4AEidxCurrentF;
        }
#endif
        //calculate AF statist value
        passAFBuffer(pAFStatBuf);

        AAA_TRACE_D("handleAF");
        m_pIAfAlgo->handleAF( m_sAFInput, m_sAFOutput);
        AAA_TRACE_END_D;
        //Move lens position.
        AAA_TRACE_D("Move Lens (%d)", m_sAFOutput.i4AFPos);
        m_u8MvLensTS = MoveLensTo( m_sAFOutput.i4AFPos, AF_MGR_CALLER);
        AAA_TRACE_END_D;
        m_i4LensPosExit = m_sAFOutput.i4AFPos;
        //Update parameter for flow controlling
        m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState;
        m_i4IsSelHWROI_CurState = m_sAFOutput.i4IsSelHWROI;
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState;
        m_i4IsAFSearch_CurState = m_sAFOutput.i4IsAfSearch;

        //Update LockAERequest: lockAE when Contrast AF
        i4IsLockAERequest = (m_sAFOutput.i4IsAfSearch<=AF_SEARCH_TARGET_MOVE)? 0:1;
#if 0
        // Warning for AdpAlarm
        if(m_sAFOutput.i4AdpAlarm)
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
#endif
    }

    //------------------------------------Configure/update HW setting----------------------------------
    IspMgrAFStatUpdateHw();

    //------------------------------------Update parameter for depth AF----------------------------------
    SDAF_Result();

    //------------------------------------store debug information while searching----------------------
    if( m_i4IsAFSearch_CurState!=AF_SEARCH_DONE)
    {
        MUINT64 ts = getTimeStamp_us();
        WDBGTSInfo( MGR_TS, (MINT32)ts, 0);
    }

    //------------------------------------Update parameter for flow controlling----------------------------------

    m_i4IsFocused = m_sAFOutput.i4IsFocused;

    UpdatePDParam( m_sAFOutput.i4ROISel);

    //Event
    if(  m_i4IsAFSearch_PreState!=m_i4IsAFSearch_CurState)
    {
        CAM_LOGD("#(%5d,%5d) %s Dev(%d) isAFSearch changed : %d -> %d",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_i4IsAFSearch_PreState,
                 m_i4IsAFSearch_CurState);
        if( m_i4IsAFSearch_CurState!=AF_SEARCH_DONE)
        {
            // Positive Edge: Searching Start
            if (m_pMcuDrv)
            {
                m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MTRUE, m_i4CurrSensorDev);
            }
            CleanMgrDbgInfo();
            CleanTSDbgInfo();

            // CONTINUOUS_VIDEO or CONTINUOUS_PICTURE ==> need to check if send callback
            if(m_bNeedCheckSendCallback)
            {
                if(m_bNeedLock)
                {
                    /* It have to send callback once got autofocus command from host */
                    m_bNeedSendCallback = 1;
                }
                else
                {
                    m_sCallbackInfo.isSearching        = m_i4IsAFSearch_CurState;
                    m_sCallbackInfo.CompSet_PDCL.Value = m_sAFOutput.i4PdTrigUiConf;
                    m_sCallbackInfo.CompSet_ISO.Value  = m_sAFInput.i4ISO;
                    m_sCallbackInfo.CompSet_FPS.Value  = (m_sAFInput.TS_AFDone - TS_AFDone_Pre > 0) ? 1000000/(m_sAFInput.TS_AFDone - TS_AFDone_Pre) : 0;
                    m_bNeedSendCallback |= checkSendCallback(m_sCallbackInfo); // isSearch switch between 1~3 still need SendCallback
                    CAM_LOGD("#(%5d,%5d) %s Dev(%d) checkSendCallback: m_bNeedSendCallback(%d), AfterAutoMode(%d), Searching(%d), PDCL(%d, %d), ISO(%d, %d), FPS(%d, %d)",
                             m_u4ReqMagicNum,
                             m_u4StaMagicNum,
                             __FUNCTION__,
                             m_i4CurrSensorDev,
                             m_bNeedSendCallback,
                             m_sCallbackInfo.isAfterAutoMode,
                             m_sCallbackInfo.isSearching,
                             m_sCallbackInfo.CompSet_PDCL.Value,m_sCallbackInfo.CompSet_PDCL.Target,
                             m_sCallbackInfo.CompSet_ISO.Value,m_sCallbackInfo.CompSet_ISO.Target,
                             m_sCallbackInfo.CompSet_FPS.Value,m_sCallbackInfo.CompSet_FPS.Target);
                }
                if(m_bNeedSendCallback==1 && m_sAFOutput.i4ROISel!=AF_ROI_SEL_FD)
                {
                    m_i4IsCAFWithoutFace = 1;   // CAFWithoutFace && NeedSendCallback
                }
            }
            else // else ==> always send callback
            {
                m_bNeedSendCallback = 1;
            }

            if(m_bNeedSendCallback)
            {
                UpdateState( EVENT_SEARCHING_START);
            }
        }
        else if(m_i4IsAFSearch_CurState==AF_SEARCH_DONE)
        {
            // Negative Edge: Searching End
            if (m_pMcuDrv)
            {
                m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, m_i4OISDisable, m_i4CurrSensorDev);
            }
            SetMgrDbgInfo();

            if(m_bNeedLock)
            {
                /* It have to send callback once got autofocus command from host */
                m_bNeedSendCallback = 1;
            }

            if(m_bNeedSendCallback)
            {
                UpdateState( EVENT_SEARCHING_END);
                m_bNeedSendCallback=0;
                m_i4IsCAFWithoutFace = 0;
                m_bForceDoAlgo = MFALSE;
            }
            else
            {
                UpdateState( EVENT_SEARCHING_DONE_RESET_PARA); /* Hybrid AF : PDAF, LDAF, ... */
            }
            // The af search for LaunchCamTrigger is done
            if(m_i4LaunchCamTriggered==E_LAUNCH_AF_TRIGGERED)
            {
                m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
            }
        }
    }

    //----------------------------- send callback only when AFState change ----------------------------------
    if(m_eAFStatePreForCallback != m_eAFState)
    {
        CallbackNotify_BasedOnAFState();
        m_eAFStatePreForCallback = m_eAFState;
    }

    if( (m_i4IsAFSearch_PreState!=m_i4IsAFSearch_CurState) ||
            (m_i4IsSelHWROI_PreState!=m_i4IsSelHWROI_CurState) ||
            (m_i4DgbLogLv))
    {
        LC_doAF.Add("[Status] IsAFSearch(%d->%d), IsSelHWROI(%d->%d), ROISel(%d), Event(0x%x)",
                    m_i4IsAFSearch_PreState,
                    m_i4IsAFSearch_CurState,
                    m_i4IsSelHWROI_PreState,
                    m_i4IsSelHWROI_CurState,
                    m_sAFOutput.i4ROISel,
                    m_eEvent);
        LC_doAF.Print();
    }

    if( m_sAFOutput.i4IsAfSearch==AF_SEARCH_DONE)
    {
        m_i4DbgMotorDisable = property_get_int32("vendor.debug.af_motor.disable", 0);
        m_i4DbgMotorMPos    = property_get_int32("vendor.debug.af_motor.position", 1024);
    }

    AAA_TRACE_END_D;

    if(m_i4LaunchCamTriggered_Prv==E_LAUNCH_AF_IDLE && m_i4LaunchCamTriggered==E_LAUNCH_AF_WAITING)
    {
        // positive edge
        AAA_TRACE_D("LaunchCamTrigger Start: #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);
        AAA_TRACE_END_D;
    }
    else if(m_i4LaunchCamTriggered_Prv==E_LAUNCH_AF_TRIGGERED && m_i4LaunchCamTriggered==E_LAUNCH_AF_DONE)
    {
        // negetive edge
        AAA_TRACE_D("LaunchCamTrigger End: #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);
        AAA_TRACE_END_D;
    }
    m_i4LaunchCamTriggered_Prv = m_i4LaunchCamTriggered;

    //---------------------------------------------------------------------------------------------------
    //get current sensor's temperature
    m_sAFInput.u4CurTemperature = getSensorTemperature();

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::passPDBuffer( MVOID *ptrInPDData)
{
    Mutex::Autolock lock( m_Lock);

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "%s : ", __FUNCTION__);
    }

    //return directly during camera is opening(init)
    if( (m_bRunPDEn == MFALSE) || (m_i4EnableAF!= 1))
    {
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to m_bRunPDEn(%d) m_i4EnableAF(%d)", m_bRunPDEn, m_i4EnableAF);
            CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
        }
        return S_AF_OK;
    }

    //return directly once af_mgr is uninitated
    if( m_bRunPDEn==MFALSE)
    {
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to m_bRunPDEn(%d)", m_bRunPDEn);
            CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
        }
        return S_AF_OK;
    }

    //store lens position.
    LENS_INFO_T lensInfo;
    lensInfo = m_sCurLensInfo;
    getLensInfo( lensInfo);
    if( 0<=lensInfo.i4CurrentPos && lensInfo.i4CurrentPos<=1023)
    {
        m_i4PreLensPos = m_i4CurLensPos;
        m_i4CurLensPos = lensInfo.i4CurrentPos;
    }
    else
    {
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to incorrect lensInfo: m_bRunPDEn=%d", m_bRunPDEn);
            CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
        }
        return S_AF_OK;
    }

    MUINT32 bufSize;
    MUINT32 bufStride;
    MUINT8* pBufAddr;
    MINT32  magicNum  = m_u4ReqMagicNum - 2; //default value if no magicnumber from buffer info
    MINT32  lensPos   = m_i4PreLensPos;


    //get pd input buffer info
    if( m_i4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)
    {
        bufSize     = m_pdaf_h*m_pdaf_w*10/8;
        bufStride   = m_pdaf_stride;
        pBufAddr    = reinterpret_cast<MUINT8 *>(m_pdaf_rawbuf);
    }
    else
    {
        BufInfo_T *rBufInfo = (BufInfo_T*)(m_CamSVBufInfo);

        bufSize   = rBufInfo->size;
        bufStride = 0;
        pBufAddr  = reinterpret_cast<MUINT8 *>(rBufInfo->virtAddr);

        //
        if( (m_i4PDAF_support_type==PDAF_SUPPORT_RAW) || (m_i4PDAF_support_type==PDAF_SUPPORT_RAW_DUALPD) || (m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV) || (m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV_DUALPD))
        {
            //do nothing
        }
        else
        {
            if( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to No correct PD task [Dev %d]: %d, %d", m_i4CurrSensorDev, m_i4EnableAF, m_i4PDAF_support_type);
                CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
            }
            return S_AF_OK;
        }
    }

#define PERFRAME_CALCULATE_PD 1
    //
    MBOOL fdexist = MFALSE;
    for( MUINT32 i=0; i<m_i4PDCalculateWinNum; i++)
    {
        if((m_sPDCalculateWin[i].sType==ROI_TYPE_OT) || (m_sPDCalculateWin[i].sType==ROI_TYPE_FD))
        {
            fdexist = MTRUE;
            break;
        }
    }

    //
    if( /* checkin pd calculation task can be executed or not */
        //[TODO] (m_i4LaunchCamTriggered!=E_LAUNCH_AF_DONE) /* quick lunch */||
        //[TODO] (m_ptrNVRam->rAFNVRAM.i4EasyTuning[2]==PERFRAME_CALCULATE_PD) /*perframe calculating pd*/ ||
        (fdexist==MTRUE) /*fd is exist in the calculation list*/)
    {
        m_bNeedPdoResult = MTRUE;
    }
    else
    {
        if( m_i4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)
        {
            m_bNeedPdoResult = m_pdaf_raw_fmt==1 ? 1 : 0;
        }
        else if( m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV || m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV_LEGACY)
        {
            // Normally, do pd sw path per 2 frame
            m_bNeedPdoResult = 1-m_bNeedPdoResult;
        }
        else
        {
            m_bNeedPdoResult = MFALSE;
        }
    }

    //
    if(m_bNeedPdoResult)
    {

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, " buf#%d Sz(%d) stride(%d) Addr(%p), ", magicNum, bufSize, bufStride, (void*)pBufAddr);
        }

        //allocate pd input data
        PD_CALCULATION_INPUT PDInput( m_i4PDCalculateWinNum, bufSize, bufStride, pBufAddr);

        //match lens position by time stamp
        if( m_u8PreCamTS!=0)
        {
            MBOOL seachDone = MFALSE;
            vector<AF_FRM_INFO_T>::iterator itor;

            for( itor=m_vFrmInfo.end()-1; ; )
            {
                if( 7000+itor->u8MvLensTS<m_u8PreCamTS)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf,
                                          "matching lens position by time stamp : Raw#(%d) PreVSTS(%12llu) CurVSTS(%12llu), MvLensTS(%12llu) Buffer-LensPos(%d) CurLensPos(%d) ",
                                          m_pdaf_raw_frmnum,
                                          m_u8PreCamTS,
                                          m_u8CamTS,
                                          itor->u8MvLensTS,
                                          itor->i4LensPos,
                                          m_i4CurLensPos);
                    lensPos   = itor->i4LensPos;
                    seachDone = MTRUE;
                    break;
                }
                if( itor == m_vFrmInfo.begin())
                {
                    break;
                }
                itor--;
            }

            //lose frame.
            if( seachDone == MFALSE)
            {
                CAM_LOGE( "%s Can not find lens postion to describe PD buffer, Raw#(%d) PreVSTS(%12llu) CurVSTS(%12llu), MvLensTS(%12llu) CurLensPos(%d) ",
                          __FUNCTION__,
                          m_pdaf_raw_frmnum,
                          m_u8PreCamTS,
                          m_u8CamTS,
                          itor->u8MvLensTS,
                          m_i4CurLensPos);
                return S_AF_OK;
            }
            else
            {
                m_vFrmInfo.erase( m_vFrmInfo.begin(), itor);
            }
        }
        else
        {
            m_vFrmInfo.clear();
        }

        PDInput.frmNum       = m_u4ReqMagicNum;
        PDInput.magicNumber  = magicNum;
        PDInput.curLensPos   = lensPos;
        PDInput.XSizeOfImage = m_i4TGSzW;
        PDInput.YSizeOfImage = m_i4TGSzH;

        MUINT64 timestamp = getTimeStamp_us();

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "curPos %4d, TS mvMCU->PD %3d, ", PDInput.curLensPos, (MINT32)(timestamp-m_u8MvLensTS));
        }


        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "CalculateWin (fmt,x,y,w,h,info): ");
        }
        for( MUINT32 i=0; i<PDInput.numROI; i++)
        {
            PDInput.ROI[i] = m_sPDCalculateWin[i];

            if( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#%d(%d,%d,%d,%d,%d,%d) ",
                                      i,
                                      PDInput.ROI[i].sType,
                                      PDInput.ROI[i].sPDROI.i4X,
                                      PDInput.ROI[i].sPDROI.i4Y,
                                      PDInput.ROI[i].sPDROI.i4W,
                                      PDInput.ROI[i].sPDROI.i4H,
                                      PDInput.ROI[i].sPDROI.i4Info);
            }
        }
        //update information of current buffer
        PDInput.afeGain    = m_AEInfo.u4AfeGain; //[TODO] pAfeGain->u4AfeGain;
        PDInput.hlrEn      = 0; //[TODO] pAfeGain->bHLREnable;
        PDInput.iso        = m_AEInfo.i4ISO; //[TODO] pAfeGain->u4RealISOValue;
        PDInput.isAEStable = m_AEInfo.i4IsAEStable;

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "afeGain(%d) hlrEn(%d) iso(%d) isAEStable(%d)",
                                  PDInput.afeGain,
                                  PDInput.hlrEn,
                                  PDInput.iso,
                                  PDInput.isAEStable,
                                  m_eEvent);
        }

        AAA_TRACE_D("PostPD");
        IPDMgr::getInstance().postToPDTask( m_i4CurrSensorDev, &PDInput);
        AAA_TRACE_END_D;
    }
    else
    {
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "skip 1 of 2 frames");
        }

    }

    CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::UpdateCenterROI( AREA_T &sOutAreaCenter)
{
    MUINT32 scalex = 100;
    MUINT32 scaley = 100;

    if( m_bMZAFEn)
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4MultiAFCoefs[0];
        scaley = m_ptrNVRam->rAFNVRAM.i4MultiAFCoefs[1];
    }
    else if( m_i4PDAF_support_type!=PDAF_SUPPORT_NA)
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[1];
        scaley = m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[2];
    }
    else
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4SPOT_PERCENT_W;
        scaley = m_ptrNVRam->rAFNVRAM.i4SPOT_PERCENT_H;
    }

    if(     100<scalex) scalex=100;
    else if( scalex<=0) scalex=15;

    if(     100<scaley) scaley=100;
    else if( scaley<=0) scaley=15;

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

    MUINT32 croiw = m_sCropRegionInfo.i4W*scalex/100;
    MUINT32 croih = m_sCropRegionInfo.i4H*scaley/100;
    MUINT32 croix = m_sCropRegionInfo.i4X + (m_sCropRegionInfo.i4W-croiw)/2;
    MUINT32 croiy = m_sCropRegionInfo.i4Y + (m_sCropRegionInfo.i4H-croih)/2;

    //updateing.
    sOutAreaCenter = AREA_T( croix, croiy, croiw, croih, AF_MARK_NONE);

    CAM_LOGD( "ROI-C [X]%d [Y]%d [W]%d [H]%d",
              sOutAreaCenter.i4X,
              sOutAreaCenter.i4Y,
              sOutAreaCenter.i4W,
              sOutAreaCenter.i4H);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::UpdatePDParam( MINT32 &i4InROISel)
{
    if( (m_PDBuf_Type!=EPDBUF_NOTDEF) && (m_i4PDAF_support_type!=PDAF_SUPPORT_NA))
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "#(%5d,%5d) %s PDBufType(0x%x) roiSel(%d) nFmt(%d) Info(vd,Fmt,X,Y,W,H,Info): ",
                                  m_u4ReqMagicNum,
                                  m_u4StaMagicNum,
                                  __FUNCTION__,
                                  m_PDBuf_Type,
                                  i4InROISel,
                                  eIDX_ROI_ARRAY_NUM);
        }


        MINT32 PDRoisNum = 0;
        AFPD_BLOCK_ROI_T PDRois[eIDX_ROI_ARRAY_NUM];

        for( MUINT16 i=0; i<eIDX_ROI_ARRAY_NUM; i++)
        {
            MBOOL select = MFALSE;
            MBOOL roi_vd = MFALSE;

            if( m_i4DgbLogLv)
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

            switch(i)
            {
            case eIDX_ROI_ARRAY_AP:
                select = m_sPDRois[i].info.sType==ROI_TYPE_AP ? MTRUE : MFALSE;
                roi_vd = i4InROISel==AF_ROI_SEL_AP ? MTRUE : m_sPDRois[i].valid; // force calculating AP window when hybrid AF need PD result of AP roi.
                break;

            case eIDX_ROI_ARRAY_FD:
                select = m_sPDRois[i].info.sType==ROI_TYPE_FD ? MTRUE : MFALSE;
                roi_vd = ((i4InROISel==AF_ROI_SEL_FD) || (i4InROISel==AF_ROI_SEL_OT)) ? MTRUE : m_sPDRois[i].valid; // force calculating FD window when hybrid AF is at FDAF stage.
                break;

            case eIDX_ROI_ARRAY_CENTER:
                select = m_sPDRois[i].info.sType==ROI_TYPE_CENTER ? MTRUE : MFALSE;
                roi_vd = m_sPDRois[i].valid;
                break;

            default :
                select = MFALSE;
                roi_vd = MFALSE;
                break;

            }

            // Only center ROI is calculated for verification flow.
            if( m_i4DbgPDVerifyEn)
            {
                roi_vd = (i==eIDX_ROI_ARRAY_CENTER) ? MTRUE : MFALSE;
            }

            if( select && roi_vd)
            {
                //
                PDRois[PDRoisNum] = m_sPDRois[i].info;

                //apply zoom information.
                ApplyZoomEffect( PDRois[PDRoisNum].sPDROI);

                //debug log
                if( m_i4DgbLogLv)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf,
                                          "->(%d,%d,%d,%d,%d,%d,%d) ",
                                          roi_vd,
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
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "PDCalculateWin(total%d) Info(Fmt,X,Y,W,H,Info): ", m_i4PDCalculateWinNum);
        }

        for( MINT32 i=0; i<m_i4PDCalculateWinNum; i++)
        {
            if( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#%d(%d,%d,%d,%d,%d) ",
                                      i,
                                      m_sPDCalculateWin[i].sType,
                                      m_sPDCalculateWin[i].sPDROI.i4X,
                                      m_sPDCalculateWin[i].sPDROI.i4Y,
                                      m_sPDCalculateWin[i].sPDROI.i4W,
                                      m_sPDCalculateWin[i].sPDROI.i4H);
            }

            if( /* Boundary checking */
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

        if( res!=S_3A_OK)
        {
            m_i4PDCalculateWinNum = 1;
            m_sPDCalculateWin[0]  = m_sPDRois[eIDX_ROI_ARRAY_CENTER].info;
        }
        else if( m_i4DbgPDVerifyEn && (m_i4PDCalculateWinNum<AF_PSUBWIN_NUM))
        {
            m_i4PDCalculateWinNum++;
            m_sPDCalculateWin[m_i4PDCalculateWinNum-1] = m_sPDRois[eIDX_ROI_ARRAY_CENTER].info;
        }

        m_bRunPDEn = MTRUE;

        CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);

        /* exectuing from first request */
        AAA_TRACE_D("UpdatePDParam (%d)", m_u4StaMagicNum);
        IPDMgr::getInstance().UpdatePDParam( m_i4CurrSensorDev, m_u4StaMagicNum, m_i4PDCalculateWinNum, m_sPDCalculateWin, m_sAFOutput.i4SearchRangeInf, m_sAFOutput.i4SearchRangeMac);
        AAA_TRACE_END_D;

        /*****************************************************************************************************************
         * FD roi is calculated once a FD region is set
         * So reset status once region is read out
         *****************************************************************************************************************/
        if( m_sPDRois[eIDX_ROI_ARRAY_FD].valid)
        {
            m_sPDRois[eIDX_ROI_ARRAY_FD].valid = MFALSE;
        }

    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::doPDBuffer(MVOID * buffer, MINT32 w,MINT32 h, MINT32 stride, MUINT32 i4FrmNum, MUINT32 i4RawFmt, IImageBuffer *imgbuf)
{
    if(imgbuf)
{
    AAA_TRACE_D("doPDBuffer");
    m_pdaf_rawbuf     = buffer;
    m_pdaf_w          = w;
    m_pdaf_h          = h;
    m_pdaf_stride     = stride;
    m_pdaf_raw_frmnum = i4FrmNum;
    m_pdaf_raw_fmt    = i4RawFmt; /* 0 represe precess raw, 1 represent pure raw */
    CAM_LOGD_IF( m_i4DgbLogLv,
                     "%s raw info for pd processing : frmNum(%d) w(%d/%d) h(%d/%d) stride(0x%x/0x%x BYTE) va(%p/%p) format(%d), sz(0x%x BYTE), bit depth per pixel(%d)",
                 __FUNCTION__,
                 m_pdaf_raw_frmnum,
                 m_pdaf_w,
                     imgbuf->getImgSize().w,
                 m_pdaf_h,
                     imgbuf->getImgSize().h,
                 m_pdaf_stride,
                     imgbuf->getBufStridesInBytes(0),
                 m_pdaf_rawbuf,
                     imgbuf->getBufVA(0),
                     m_pdaf_raw_fmt,
                     imgbuf->getBufSizeInBytes(0),
                     imgbuf->getImgBitsPerPixel());

    //
    if( m_i4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)
    {
            MRESULT res = passPDBuffer( reinterpret_cast<IImageBuffer *>( imgbuf));
    }
    AAA_TRACE_END_D;
    }
    else
    {

    }



}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::VSyncIrq()
{
    m_u8FrmCnt++;

    //------------------------------------update HW setting----------------------------------
    {
        Mutex::Autolock lock( m_CmdQLock);
        if (m_i4VsyncUpdate != MFALSE)
        {
            CAM_LOGD_IF(m_i4DgbLogLv, "[%s] update HW setting", __FUNCTION__);
            ConfigHWReg( m_sHWCfg, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);
            updateAFWinStatus( m_sArea_HW, m_sArea_HW_Previous);
            m_i4VsyncUpdate = MFALSE;
        }
    }

    m_u8PreCamTS = m_u8CamTS;
    m_u8CamTS    = getTimeStamp_us();

    MINT32 imgo_raw_type = 0; /* 0 represe precess raw, 1 represent pure raw */
    if( MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_CUR_FRM_RAWFMT, (MINTPTR)(&imgo_raw_type), 0,0))
    {
        CAM_LOGE("EPIPECmd_GET_CUR_FRM_RAWFMT fail");
    }

    //queue previous 30 frames' information
    getLensInfo( m_sCurLensInfo);
    {
        Mutex::Autolock lock( m_Lock);
        if( m_vFrmInfo.size()>30)
        {
            m_vFrmInfo.erase( m_vFrmInfo.begin(), m_vFrmInfo.end()-30);
        }
    }
    //queue current frame's information
    AF_FRM_INFO_T frminfo( m_u4ReqMagicNum, imgo_raw_type, m_sCurLensInfo.i4CurrentPos, m_u8MvLensTS);
    m_vFrmInfo.push_back( frminfo);

    //store lens position.
    if( 0<=m_sCurLensInfo.i4CurrentPos && m_sCurLensInfo.i4CurrentPos<=1023)
    {
        m_i4PreLensPos = m_i4CurLensPos;
        m_i4CurLensPos = m_sCurLensInfo.i4CurrentPos;
    }

    CAM_LOGD_IF( m_i4DgbLogLv,
                 "%s Req#(%d) Fmt(%d) [Info] PreVSTS(%12llu) CurVSTS(%12llu) moveMCU->VSync(%3d) curPos(%4d) prePos(%d)",
                 __FUNCTION__,
                 m_u4ReqMagicNum,
                 imgo_raw_type,
                 m_u8PreCamTS,
                 m_u8CamTS,
                 (MINT32)(m_u8CamTS-m_u8MvLensTS),
                 m_i4CurLensPos,
                 m_i4PreLensPos);


    //
    if( (m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV || m_i4PDAF_support_type==PDAF_SUPPORT_CAMSV_LEGACY))
    {
        //checking DMA is inited or not.
        MBOOL isDMAinited = MFALSE;
        IAEBufMgr::getInstance().isDMAInit( m_i4CurrSensorDev, isDMAinited);

        MBOOL ret = MFALSE;
        BufInfo_T *rBufInfo = (BufInfo_T*)(m_CamSVBufInfo);

        // CamSv buffer control
        if( rBufInfo && isDMAinited)
        {
            if(1<m_u8FrmCnt)
            {
                // Dequeue DMA buffer
                AAA_TRACE_D("dequeu");
                ret = IAEBufMgr::getInstance().dequeueHwBuf( m_i4CurrSensorDev, (*rBufInfo), _PD_BUF_SEL_, m_u4ReqMagicNum);
                AAA_TRACE_END_D;

                // Update DMA base address for next frame
                AAA_TRACE_D("update");
                ret = IAEBufMgr::getInstance().updateDMABaseAddr( m_i4CurrSensorDev, _PD_BUF_SEL_);
                AAA_TRACE_END_D;

                MRESULT res = passPDBuffer( NULL);

                // Enqueue DMA buffer
                AAA_TRACE_D("enqueue");
                ret = IAEBufMgr::getInstance().enqueueHwBuf( m_i4CurrSensorDev, (*rBufInfo), _PD_BUF_SEL_);
                AAA_TRACE_END_D;
            }
            else if( 1==m_u8FrmCnt)
            {
                //first SOF
                CAM_LOGD("%s First SOF, update DMA buffer address directly for double buffer control",__FUNCTION__);
                AAA_TRACE_D("update");
                ret = IAEBufMgr::getInstance().updateDMABaseAddr( m_i4CurrSensorDev, _PD_BUF_SEL_);
                AAA_TRACE_END_D;
            }

        }
        else
        {
            CAM_LOGE( "abort due to CamSv is not initialized for PDAF_support type(%d), BufInfo(%p)", m_i4PDAF_support_type, m_CamSVBufInfo);
            return;
        }
    }
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
#if 1
    m_u4StaMagicNum = m_u4ReqMagicNum-2; //[TODO]
#else
    //---------------------------------------Statistic data information--------------------------------------------
    StatisticBufInfo *ptrStatInfo = reinterpret_cast<StatisticBufInfo *>( ptrInAFData);
    m_u4StaMagicNum   = ptrStatInfo->mMagicNumber;
    m_u4FrameCount    = ptrStatInfo->mFrameCount;
    m_u4ConfigHWNum   = ptrStatInfo->mConfigNum;
    m_u4ConfigLatency = ptrStatInfo->mConfigLatency;

    m_sAFInput.i4IsSupportN2Frame = (m_u4ConfigLatency == 3) ? 0 : 1;
    // m_sAFInput.i4Ready4EnableDS   = ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).sendAFConfigCtrl(EAFConfigCtrl_IsAF_DSSupport, NULL, NULL);
#endif

    //Got AF statistic from DMA buffer.
    AAA_TRACE_D("ConvertBufToStat (%d)", m_u4StaMagicNum);
    ConvertDMABufToStat( m_sAFOutput.i4AFPos, ptrInAFData, m_sAFInput.sStatProfile);
    AAA_TRACE_END_D;

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::ConvertDMABufToStat( MINT32 &i4CurPos, MVOID *ptrInStatBuf, AF_STAT_PROFILE_T &sOutSata)
{
    // Input : HW DMA Buffer
    MUINT8  *pDMABuf = reinterpret_cast<MUINT8*>(ptrInStatBuf);
    MUINT32 u4Stride = ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getAFStride();
    MUINT32 u4HwStatSize    = sizeof(AF_HW_STAT_T) / 2; /* Statistics Data : 128bits */

    if( m_i4HWEnExtMode != 0)
    {
        u4HwStatSize *= 2; /* Statistics Data : 256bits */
    }

    CAM_LOGD_IF( m_i4DgbLogLv,
                 "#(%5d,%5d) %s BlkW(%d) BlkH(%d) SzW(%d) SzH(%d) Str(%d) off(%d)",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4HWBlkNumX,
                 m_i4HWBlkNumY,
                 m_i4HWBlkSizeX,
                 m_i4HWBlkSizeY,
                 u4Stride,
                 u4Stride/u4HwStatSize);


    //reset last time data
    memset( &sOutSata, 0, sizeof(AF_STAT_PROFILE_T));

    if( m_i4HWBlkNumX<=0 || m_i4HWBlkNumY<=0 || m_i4HWBlkNumX>MAX_AF_HW_WIN_X || m_i4HWBlkNumY>MAX_AF_HW_WIN_Y)
    {
        //Should not be happened.
        CAM_LOGE( "HW-Setting Fail");
    }
    else if( pDMABuf==NULL)
    {
        //Should not be happened.
        CAM_LOGE( "AFO Buffer NULL");
    }
    else
    {
        //number of AF statistic blocks.
        MUINT32 nblkW = m_i4HWBlkNumX;
        MUINT32 nblkH = m_i4HWBlkNumY;

        //Debug only.
        MUINT64 FV_H0=0;
        MUINT64 FV_H1=0;
        MUINT64 FV_H2=0;
        MUINT64 FV_V=0;


        //==========
        // Outputs
        //==========
        // AF extend mode
        sOutSata.u4AfExtValid = m_i4HWEnExtMode;
        sOutSata.u4ConfigNum  = m_u4StaMagicNum;
        sOutSata.i4AFPos      = i4CurPos;

        //statistic information.
        sOutSata.u4NumBlkX = nblkW;
        sOutSata.u4NumBlkY = nblkH;
        sOutSata.u4SizeBlkX = m_i4HWBlkSizeX;
        sOutSata.u4SizeBlkY = m_i4HWBlkSizeY;
        //AF statistic
        AF_STAT_T *ptrSata = sOutSata.ptrStat;

        for( MUINT32 j=0; j<nblkH; j++)
        {
            MUINT8 *pHwStatBuf_W = reinterpret_cast<MUINT8 *>( pDMABuf);

            for( MUINT32 i=0; i<nblkW; i++)
            {
                AF_HW_STAT_T *ptrDMABuf = reinterpret_cast<AF_HW_STAT_T *>( pHwStatBuf_W);

                (*ptrSata).u4FILV  = (*ptrDMABuf).byte_00_03 & 0x007FFFFF;
                (*ptrSata).u4FILH0 = (*ptrDMABuf).byte_04_07 & 0x3FFFFFFF;
                (*ptrSata).u4FILH1 = (*ptrDMABuf).byte_08_11 & 0x3FFFFFFF;
                (*ptrSata).u4GSum  = (*ptrDMABuf).byte_12_15 & 0x003FFFFF;
                (*ptrSata).u4SCnt  = (((*ptrDMABuf).byte_00_03 >> 23      ) & 0x01FF) |
                                     (((*ptrDMABuf).byte_04_07 >> 30 <<  9) & 0x0600) |
                                     (((*ptrDMABuf).byte_12_15 >> 24 << 11) & 0x3800) ;

                FV_H0 += (*ptrSata).u4FILH0;
                FV_H1 += (*ptrSata).u4FILH1;
                FV_V  += (*ptrSata).u4FILV;

                if( m_i4HWEnExtMode != 0)
                {
                    (*ptrSata).u4FILH0Max = (*ptrDMABuf).byte_20_23 & 0x0000FFFF;
                    (*ptrSata).u4FILH2    = (*ptrDMABuf).byte_16_19 & 0x3FFFFFFF;
                    (*ptrSata).u4FILH2Cnt = ((*ptrDMABuf).byte_20_23 >> 16) & 0x7FFF;
                    (*ptrSata).u4RSum     = (*ptrDMABuf).byte_24_27 & 0x001FFFFF;
                    (*ptrSata).u4RSCnt    = (((*ptrDMABuf).byte_24_27 >> 21      ) & 0x07FF) |
                                            (((*ptrDMABuf).byte_16_19 >> 30 << 11) & 0x1800) ;
                    (*ptrSata).u4BSum     = (*ptrDMABuf).byte_28_31 & 0x001FFFFF;
                    (*ptrSata).u4BSCnt    = (((*ptrDMABuf).byte_28_31 >> 21      ) & 0x07FF) |
                                            (((*ptrDMABuf).byte_12_15 >> 22 << 11) & 0x1800) ;

                    FV_H2 += (*ptrSata).u4FILH2;
                }

                ptrSata++;
                pHwStatBuf_W += u4HwStatSize;
            }

            pDMABuf += u4Stride;
        }

        // For debug FV only.
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "#(%5d,%5d) %s [Pos]%4d [H0]%10llu [H1]%10llu [H2_EXT]%10llu [V]%llu",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     i4CurPos,
                     (unsigned long long)FV_H0,
                     (unsigned long long)FV_H1,
                     (unsigned long long)FV_H2,
                     (unsigned long long)FV_V);

        //debug print.
        if( (m_i4DgbLogLv&0x2)!=0)
        {
            for( MUINT32 j=0; j<nblkH; j++)
            {
                for( MUINT32 i=0; i<nblkW; i++)
                {
                    CAM_LOGD_IF( m_i4DgbLogLv&0x2,
                                 "[%3d][%3d] : [V]%8d, [H0]%8d, [H1]%8d, [GSum]%8d, [SCnt]%8d\n",
                                 i,
                                 j,
                                 sOutSata.ptrStat[i].u4FILV,
                                 sOutSata.ptrStat[i].u4FILH0,
                                 sOutSata.ptrStat[i].u4FILH1,
                                 sOutSata.ptrStat[i].u4GSum,
                                 sOutSata.ptrStat[i].u4SCnt);

                }
            }
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocusFinish()
{
    MINT32 ret = 1;
    CAM_LOGD_IF(m_i4DgbLogLv, "%s, m_eAFState=%d", __FUNCTION__, m_eAFState);
    if( (m_eAFState == NS3A::E_AF_PASSIVE_SCAN) ||
            (m_eAFState == NS3A::E_AF_ACTIVE_SCAN) )
    {
        ret = 0;
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocused()
{
    MINT32 ret = 0;

    if( (m_eAFState == NS3A::E_AF_PASSIVE_FOCUSED) ||
            (m_eAFState == NS3A::E_AF_FOCUSED_LOCKED))
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
MRESULT AfMgr::getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo)
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( m_i4DgbLogLv&4,
                 "%s Dev %d",
                 __FUNCTION__,
                 m_i4CurrSensorDev);

    //reset.
    memset( &rAFDebugInfo, 0, sizeof(AF_DEBUG_INFO_T));

    /* Do not modify following oder: */

    //1. Hybrid AF library
    if (m_pIAfAlgo)
    {
        ret =  m_pIAfAlgo->getDebugInfo(rAFDebugInfo);
        m_pIAfAlgo->SaveAfCoreInfo(mEngFileName);
    }

    //2. PD library
    if( m_i4PDAF_support_type!=PDAF_SUPPORT_NA)
    {
        //Not support open pd library.
        if( m_PDBuf_Type==EPDBUF_RAW_LEGACY || m_PDBuf_Type==EPDBUF_VC || m_PDBuf_Type==EPDBUF_PDO)
        {
            ret = IPDMgr::getInstance().GetDebugInfo( m_i4CurrSensorDev, rAFDebugInfo);
        }
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
    for( MUINT32 i=0; i<AF_DEBUG_TAG_SIZE; i++)
    {
        if( sOutMgrDebugInfo.Tag[i].u4FieldID==0)
        {
            idx = i;
            break;
        }
    }

    CAM_LOGD_IF( m_i4DgbLogLv&4,
                 "%s %d %d %d %d %d %d",
                 __FUNCTION__,
                 AF_DEBUG_TAG_SIZE,
                 idx,
                 MGR_EXIF_SIZE,
                 m_i4MgrExifSz,
                 MGR_CAPTURE_EXIF_SIZE,
                 m_i4MgrCapExifSz);

    if( ((AF_DEBUG_TAG_SIZE-idx)>m_i4MgrExifSz) && (0<m_i4MgrExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrExifSz;
    }

    if( (AF_DEBUG_TAG_SIZE-idx)>m_i4MgrCapExifSz && (0<m_i4MgrCapExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrCapExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrCapExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrCapExifSz;
    }

    if( (AF_DEBUG_TAG_SIZE-idx)>m_i4MgrTsExifSz && (0<m_i4MgrTsExifSz))
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

    if( m_i4MgrTsExifSz<MGR_TS_EXIF_SIZE)
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

    if( m_i4MgrExifSz<MGR_EXIF_SIZE)
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
    if( mzNum<=MAX_MULTI_ZONE_WIN_NUM)
    {
        ret = WDBGInfo( MZ_WIN_NUM, m_sAFOutput.sROIStatus.i4TotalNum, 0);
        ret = WDBGInfo( MZ_WIN_W, m_sAFOutput.sROIStatus.sROI[0].i4W, 0);
        ret = WDBGInfo( MZ_WIN_H, m_sAFOutput.sROIStatus.sROI[0].i4H, 1);

        for( MUINT32 i=0; i<mzNum; i++)
        {
            ret = WDBGInfo( MZ_WIN_X, m_sAFOutput.sROIStatus.sROI[i].i4X, 0);
            ret = WDBGInfo( MZ_WIN_Y, m_sAFOutput.sROIStatus.sROI[i].i4Y, 1);
            ret = WDBGInfo( MZ_WIN_RES, m_sAFOutput.sROIStatus.sROI[i].i4Info, 1);
            if( ret==E_3A_ERR)
            {
                break;
            }
        }
    }

    if( ret==S_3A_OK)
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
        ret = WDBGInfo( MGR_PD_BUF_TYPE, m_PDBuf_Type, 0);
        ret = WDBGInfo( MGR_PD_SEN_TYPE, m_i4PDAF_support_type, 0);
        ret = WDBGInfo( MGR_LASER_VAL, m_sAFInput.sLaserInfo.i4CurPosDist, 0);
        ret = WDBGInfo( MGR_FOCUSING_POS, m_sFocusDis.i4LensPos, 0);
        ret = WDBGInfo( MGR_FOCUSING_DST, m_sFocusDis.i4Dist, 0);
    }

    return ret;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WDBGCapInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep)
{
    MRESULT ret = E_3A_ERR;

    if( m_i4MgrCapExifSz<MGR_CAPTURE_EXIF_SIZE)
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
    CAM_LOGD_IF( m_i4DgbLogLv&4, "%s", __FUNCTION__);
    memset( &m_sMgrCapExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_CAPTURE_EXIF_SIZE);
    m_i4MgrCapExifSz = 0;
    return S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetMgrCapDbgInfo()
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( m_i4DgbLogLv&4, "%s", __FUNCTION__);

    ret = CleanMgrCapDbgInfo();

    if( ret==S_3A_OK)
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

    if( m_pMcuDrv)
    {
        ret = m_pMcuDrv->getMCUInfo(&rMotorInfo, m_i4CurrSensorDev);

        if( a_rLensInfo.i4CurrentPos!=(MINT32)rMotorInfo.u4CurrentPosition)
        {
            CAM_LOGD_IF( m_i4DgbLogLv&2,
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

        CAM_LOGD_IF( m_i4DgbLogLv&2,
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
MRESULT AfMgr::calBestShotValue(MVOID *pAFStatBuf)
{
    pAFStatBuf=NULL;

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
MINT32 AfMgr::Boundary(MINT32 a_i4Min, MINT32 a_i4Vlu, MINT32 a_i4Max)
{
    if (a_i4Max < a_i4Min)
    {
        a_i4Max = a_i4Min;
    }
    if (a_i4Vlu < a_i4Min)
    {
        a_i4Vlu = a_i4Min;
    }
    if (a_i4Vlu > a_i4Max)
    {
        a_i4Vlu = a_i4Max;
    }
    return a_i4Vlu;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::enableAF(MINT32 a_i4En)
{
    CAM_LOGD("[enableAF]%d\n", a_i4En);
    m_i4EnableAF = a_i4En;
    return S_AF_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::setCallbacks(I3ACallBack* cb)
{
    m_pAFCallBack = cb;
    return TRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CallbackNotify_BasedOnAFState()
{
    MINT32 msg = I3ACallBack::eID_NOTIFY_AF_FOCUSED;
    MINT32 msgExt = 0;
    MBOOL  bNeedCallback = MFALSE;
    switch(m_eAFState)
    {
    case NS3A::E_AF_INACTIVE:
    case NS3A::E_AF_ACTIVE_SCAN:
        //No need to callback
        break;
    case NS3A::E_AF_FOCUSED_LOCKED:
    case NS3A::E_AF_NOT_FOCUSED_LOCKED:
        // Auto mode
        msg = I3ACallBack::eID_NOTIFY_AF_FOCUSED;
        msgExt = (m_eAFState == MTK_CONTROL_AF_STATE_FOCUSED_LOCKED) ? 1 : 0;
        bNeedCallback = MTRUE;
        break;
    case NS3A::E_AF_PASSIVE_SCAN:
    case NS3A::E_AF_PASSIVE_FOCUSED:
    case NS3A::E_AF_PASSIVE_UNFOCUSED:
        // Continuous mode
        msg = I3ACallBack::eID_NOTIFY_AF_MOVING;
        msgExt = (m_eAFState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN) ? 1 : 0;
        bNeedCallback = MTRUE;
        break;
    }

    if(bNeedCallback == MTRUE)
    {
        if (m_pAFCallBack)
        {
            m_pAFCallBack->doNotifyCb(msg, msgExt, 0, 0);
            CAM_LOGD("#(%5d,%5d) Dev(%d): Callback -- msg %d, msgExt %d", m_u4ReqMagicNum, m_u4StaMagicNum, m_i4CurrSensorDev, msg, msgExt);
        }
    }
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SingleAF_CallbackNotify()
{
    FUNC_BEGIN;
    // obsolete function
    // leave empty shell for hal 3a
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CallbackNotify_Timeout()
{
    FUNC_BEGIN;
    m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
    CAM_LOGD("#(%5d,%5d) Dev(%d): Callback -- msg %d, msgExt %d", m_u4ReqMagicNum, m_u4StaMagicNum, m_i4CurrSensorDev, I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AfMgr::getSensorTemperature()
{
    MUINT32 u4temperature = 0;

    if( m_ptrIHalSensor)
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

    MINT64 timestamp =((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000;
    return timestamp; // from nano to us
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfMgr::MoveLensTo( MINT32 &i4TargetPos, MUINT32 u4Caller)
{
    MUINT64 TS_BeforeMoveMCU = 0; // timestamp of before moveMCU
    MUINT64 TS_AfterMoveMCU = 0;  // timestamp of after moveMCU

    if( m_pMcuDrv)
    {
        if( m_i4DbgMotorDisable==1)
        {
            if( m_i4DbgMotorMPos<1024 && m_i4DbgMotorMPos!=m_i4DbgMotorMPosPre)
            {
                m_i4DbgMotorMPosPre = m_i4DbgMotorMPos;
                m_pMcuDrv->moveMCU( m_i4DbgMotorMPos, m_i4CurrSensorDev);
            }
        }
        else if( m_i4MvLensTo!=i4TargetPos)
        {
            if( u4Caller==AF_MGR_CALLER)
            {
                CAM_LOGD("#(%5d,%5d) %s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }
            else
            {
                CAM_LOGD("#(%5d,%5d) cmd-%s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }

            CAM_LOGD_IF( m_i4DgbLogLv&4, "%s+", __FUNCTION__);
            TS_BeforeMoveMCU = getTimeStamp_us();
            if (m_pMcuDrv->moveMCU( i4TargetPos, m_i4CurrSensorDev))
            {
                m_i4MvLensTo = i4TargetPos;
            }
            TS_AfterMoveMCU = getTimeStamp_us();
            CAM_LOGD_IF( m_i4DgbLogLv&4, "%s-", __FUNCTION__);
        }

        if( m_i4DbgOISDisable==1)
        {
            MUINT32 DbgOISPos = property_get_int32("vendor.debug.af_ois.position", 0);
            // DbgOISPos(XXXXYYYY) = X * 10000 + Y; Ex: 10000200
            if (m_i4DbgOISPos!=DbgOISPos)
            {
                CAM_LOGD("Set OIS Position %d", DbgOISPos);
                m_pMcuDrv->setMCUParam(0x02, DbgOISPos, m_i4CurrSensorDev);
                m_i4DbgOISPos = DbgOISPos;
            }
        }
    }
    else
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "%s Fail, Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    }

    if (TS_BeforeMoveMCU!=0)
    {
        // update time stamp of moveMCU
        return (MUINT64)((TS_BeforeMoveMCU + TS_AfterMoveMCU)/2);
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
    MINT32 i4SensorDevID;

    CAM_LOGD("Read (%d) calibration data from EEPROM by camcal", enCamCalEnum);
    switch( m_i4CurrSensorDev)
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
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        return S_AWB_OK;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AWB_OK;
    }

    result = pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);
    CAM_LOGD_IF( m_i4DgbLogLv, "(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);

    if( enCamCalEnum == CAMERA_CAM_CAL_DATA_3A_GAIN)
    {
        if (result&CamCalReturnErr[enCamCalEnum])
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

        if( 0<i4MacroPos && i4MacroPos<1024 && i4MacroPos>i4InfPos )
        {
            AF_STEP_T sAFTbl;
            memset(&sAFTbl, 0, sizeof(AF_STEP_T));

            if( (i4MacroPos>i450cmPos) && (i450cmPos>i4InfPos) )
            {
                if (m_bLDAFEn == MTRUE)
                {
                    ILaserMgr::getInstance().setLensCalibrationData(m_i4CurrSensorDev, i4MacroPos, i450cmPos);
                }

                CAM_LOGD( "OTP [50cm]%d", i450cmPos);
            }

            if( i4MiddlePos>i4InfPos && i4MiddlePos<i4MacroPos)
            {
                CAM_LOGD( "Middle OTP cal:%d\n", i4MiddlePos);
            }
            else
            {
                i4MiddlePos = i4InfPos + (i4MacroPos - i4InfPos) * m_ptrNVRam->rAFNVRAM.i4DualAFCoefs[1] / 100;
                CAM_LOGD( "Middle OTP adjust:%d NVRAM:%d INF:%d Mac:%d\n", i4MiddlePos, m_ptrNVRam->rAFNVRAM.i4DualAFCoefs[1],i4InfPos,i4MacroPos);
            }

            if( m_pIAfAlgo)
            {
                //sAFTbl = m_pIAfAlgo->updateAFtableBoundary( i4InfPos, i4MacroPos);
                m_pIAfAlgo->updateAFtableBoundary( i4InfPos, i4MacroPos);
                m_pIAfAlgo->updateMiddleAFPos(i4MiddlePos);
            }

            if(m_ptrNVRam->rAFNVRAM.i4EasyTuning[0] == 10000 && m_i4LensPosExit != 0)
            {
                m_i4InitPos = m_i4LensPosExit;
            }
            else
            {
                m_i4InitPos = i4InfPos + (i4MacroPos - i4InfPos) * m_ptrNVRam->rAFNVRAM.i4EasyTuning[0] / 100;
                m_i4InitPos = (m_i4InitPos < 0) ? 0 : ((m_i4InitPos > 1023) ? 1023 : m_i4InitPos);
            }

            // adjust depth dac_min, dac_max
            m_sDAF_TBL.af_dac_inf        = i4InfPos;
            m_sDAF_TBL.af_dac_marco      = i4MacroPos;

            if( sAFTbl.i4Num>0)
            {
                m_sDAF_TBL.af_dac_min    = sAFTbl.i4Pos[0];
                m_sDAF_TBL.af_dac_max    = sAFTbl.i4Pos[sAFTbl.i4Num-1];
            }
            else
            {
                m_sDAF_TBL.af_dac_min = i4InfPos;
                m_sDAF_TBL.af_dac_max = i4MacroPos;
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
        }
        else
        {
            if( m_pMcuDrv)
            {
                mcuMotorCalPos rMotorCalPos;
                m_pMcuDrv->getMCUCalPos( &rMotorCalPos, m_i4CurrSensorDev);

                i4InfPos   = rMotorCalPos.u4InfPos;
                i4MacroPos = rMotorCalPos.u4MacroPos;

                if( 0<i4MacroPos && i4MacroPos<1024 && i4MacroPos>i4InfPos)
                {
                    CAM_LOGD( "OTP from driver [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);

                    if( m_pIAfAlgo)
                    {
                        m_pIAfAlgo->updateAFtableBoundary( i4InfPos, i4MacroPos);
                    }
                }
            }

            CAM_LOGD_IF( m_i4DgbLogLv,
                         "%s : cam_cal is abnormal, read calibration data from AF driver [Inf]%d [Macro]%d",
                         __FUNCTION__,
                         i4InfPos,
                         i4MacroPos);
        }
    }
    else if (enCamCalEnum == CAMERA_CAM_CAL_DATA_PDAF)
    {
        if (result==CAM_CAL_ERR_NO_ERR)
        {
            memcpy( m_ptrAfPara->rPdCaliData.uData, GetCamCalData.PDAF.Data, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
            m_ptrAfPara->rPdCaliData.i4Size = GetCamCalData.PDAF.Size_of_PDAF;

            CAM_LOGD("Size of PD calibration data = %d", m_ptrAfPara->rPdCaliData.i4Size);
            CAM_LOGD_IF(m_i4DgbLogLv&2, "pd calibration data:");
            for (int index = 0; index < m_ptrAfPara->rPdCaliData.i4Size;)
            {
                CAM_LOGD_IF(m_i4DgbLogLv&2, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                            m_ptrAfPara->rPdCaliData.uData[index+ 0], m_ptrAfPara->rPdCaliData.uData[index+ 1],
                            m_ptrAfPara->rPdCaliData.uData[index+ 2], m_ptrAfPara->rPdCaliData.uData[index+ 3],
                            m_ptrAfPara->rPdCaliData.uData[index+ 4], m_ptrAfPara->rPdCaliData.uData[index+ 5],
                            m_ptrAfPara->rPdCaliData.uData[index+ 6], m_ptrAfPara->rPdCaliData.uData[index+ 7],
                            m_ptrAfPara->rPdCaliData.uData[index+ 8], m_ptrAfPara->rPdCaliData.uData[index+ 9],
                            m_ptrAfPara->rPdCaliData.uData[index+10], m_ptrAfPara->rPdCaliData.uData[index+11],
                            m_ptrAfPara->rPdCaliData.uData[index+12], m_ptrAfPara->rPdCaliData.uData[index+13],
                            m_ptrAfPara->rPdCaliData.uData[index+14], m_ptrAfPara->rPdCaliData.uData[index+15]);
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
    if(m_i4EnableAF == 0)
    {
        CAM_LOGD("autofocus : disableAF or dummy lens");
        CallbackNotify_Timeout();
        return;
    }
    m_sHostCmd.bIsAutoFocusFromHost = MTRUE;
    ExeAutoFocus(m_sHostCmd.bIsAutoFocusFromHost);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::ExeAutoFocus(MBOOL &bIsAutoFocusFromHost)
{
    if(bIsAutoFocusFromHost)
    {
        // reset the host command
        bIsAutoFocusFromHost = MFALSE;
        CAM_LOGD("cmd-%s TG(%d) : lib_afmode(%d)", __FUNCTION__, m_eSensorTG, m_eLIB3A_AFMode);

        if(m_i4EnableAF == 0)
        {
            CAM_LOGD("autofocus : dummy lens");
            return;
        }

        UpdateState( EVENT_CMD_AUTOFOCUS);

        if( m_pIAfAlgo)
        {
            m_pIAfAlgo->targetAssistMove();
        }

        //calibration flow testing
        if (m_bLDAFEn == MTRUE)
        {
            int Offset = 0;
            int XTalk = 0;

            int Mode = property_get_int32("vendor.laser.calib.mode", 0);

            if( Mode == 1 )
            {
                CAM_LOGD("LaserCali : getLaserOffsetCalib Start");
                Offset = ILaserMgr::getInstance().getLaserOffsetCalib(m_i4CurrSensorDev);
                CAM_LOGD("LaserCali : getLaserOffsetCalib : %d", Offset);
                CAM_LOGD("LaserCali : getLaserOffsetCalib End");
            }

            if( Mode == 2 )
            {
                CAM_LOGD("LaserCali : getLaserXTalkCalib Start");
                XTalk = ILaserMgr::getInstance().getLaserXTalkCalib(m_i4CurrSensorDev);
                CAM_LOGD("LaserCali : getLaserXTalkCalib : %d", XTalk);
                CAM_LOGD("LaserCali : getLaserXTalkCalib End");
            }
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::DoCallback()
{
    // obsolete callback function
    // empty shell for hal 3a
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::cancelAutoFocus()
{
    m_sHostCmd.bIsCancelAutoFocusFromHost=MTRUE;
    ExeCancelAutoFocus(m_sHostCmd.bIsCancelAutoFocusFromHost);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::ExeCancelAutoFocus(MBOOL &bIsCancelAutoFocusFromHost)
{
    if(bIsCancelAutoFocusFromHost)
    {
        bIsCancelAutoFocusFromHost = MFALSE;
        CAM_LOGD("#(%5d,%5d) cmd-%s TG(%d)  lib_afmode%d",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_eSensorTG,
                 m_eLIB3A_AFMode);
        //update parameters and status.
        UpdateState(EVENT_CMD_AUTOFOCUS_CANCEL);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::TimeOutHandle()
{
    CAM_LOGD( "#(%5d,%5d) SPECIAL_EVENT cmd-%s Dev(%d)",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev);
    if(m_eAFStatePreForCallback != m_eAFState)
    {
        CallbackNotify_BasedOnAFState();
        m_eAFStatePreForCallback = m_eAFState;
    }

    if( m_pIAfAlgo)
    {
        m_pIAfAlgo->cancel();
        UpdateState(EVENT_SEARCHING_END);
    }
    else
    {
        CAM_LOGD( "%s AF mgr is not initialed, ptr(%p)", __FUNCTION__, m_pIAfAlgo);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAndroidServiceState(MBOOL a_state)
{
    CAM_LOGD_IF( m_i4DgbLogLv,
                 "#(%5d,%5d) cmd-%s Dev(%d): %d ",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 a_state);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Camera 3.0
MVOID AfMgr::setAperture( MFLOAT /*lens_aperture*/)
{}

MFLOAT AfMgr::getAperture()
{
    MFLOAT lens_aperture=0;
    if( m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for(MUINT8 ii=0; ii< m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_APERTURES).count(); ii++)
    {
        lens_aperture= m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_APERTURES).itemAt(ii, Type2Type< MFLOAT >());
        CAM_LOGD("AFmeta APERTURES %d  %f",ii, lens_aperture);
    }
    return  lens_aperture;
}
MVOID AfMgr::setFilterDensity(MFLOAT /*lens_filterDensity*/)
{}

MFLOAT AfMgr::getFilterDensity()
{
    MFLOAT lens_filterDensity=0;
    if(m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for(MUINT8 ii=0; ii< m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES).count(); ii++)
    {
        lens_filterDensity= m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES).itemAt(ii, Type2Type< MFLOAT >());
        CAM_LOGD("AFmeta FILTER_DENSITIES %d  %f",ii, lens_filterDensity);
    }
    return  lens_filterDensity;
}
MVOID AfMgr::setFocalLength( MFLOAT /*lens_focalLength*/)
{}

MFLOAT AfMgr::getFocalLength ()
{
    MFLOAT lens_focalLength =34.0;
    if( m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for(MUINT8 ii=0; ii< m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS).count(); ii++)
    {
        lens_focalLength= m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS).itemAt(ii, Type2Type< MFLOAT >());
        CAM_LOGD("AFmeta APERTURES %d  %f",ii, lens_focalLength);
    }
    return  lens_focalLength;
}

MVOID AfMgr::setFocusDistance( MFLOAT lens_focusDistance)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): lens_focusDistance = %f",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              lens_focusDistance);

    if( lens_focusDistance<0 || !m_pAFParam)
    {
        return;
    }


    m_sHostCmd.fFocusDistance = lens_focusDistance;
    ExeSetFocusDistance(m_sHostCmd.fFocusDistance);
}

MVOID AfMgr::ExeSetFocusDistance( MFLOAT lens_focusDistance)
{
    LogCollector LC(LOG_TAG);
    LC.Add("#(%5d,%5d) %s Dev(%d): lens_focusDistance = %f",
           m_u4ReqMagicNum,
           m_u4StaMagicNum,
           __FUNCTION__,
           m_i4CurrSensorDev,
           lens_focusDistance);

    MINT32 fdistidx = 0;
    MINT32 i4distmm;
    MINT32 i4tblLL;
    MINT32 i4ResultDac;
    //lens_focusDistance is in unit dipoter, means 1/distance,
    //if distance is 100 cm,  then the value is  1/(0.1M) =10,
    // 10 cm => dipoter 100
    // 1 cm => dipoter 1000
    i4tblLL = m_pAFParam->i4TBLL;
    if( lens_focusDistance <= (1000.0f/m_pAFParam->i4Dist[i4tblLL-1])) /*infinity*/
    {
        i4ResultDac=m_pAFParam->i4Dacv[i4tblLL-1];
    }
    else if( (1000.0f/m_pAFParam->i4Dist[0]) <= lens_focusDistance)  /*marco*/
    {
        i4ResultDac = m_pAFParam->i4Dacv[0];
    }
    else
    {
        i4distmm = (MINT32)(1000/lens_focusDistance);

        for( fdistidx=0; fdistidx< i4tblLL ; fdistidx++)
        {
            if( i4distmm<m_pAFParam->i4Dist[fdistidx])
                break;

        }

        if( fdistidx==0)
        {
            i4ResultDac = m_pAFParam->i4Dacv[0];
        }
        else
        {
            i4ResultDac=
                ( m_pAFParam->i4Dacv[fdistidx]   * (i4distmm - m_pAFParam->i4Dist[fdistidx-1])
                  + m_pAFParam->i4Dacv[fdistidx-1] * (m_pAFParam->i4Dist[fdistidx] - i4distmm ))
                /(m_pAFParam->i4Dist[fdistidx] - m_pAFParam->i4Dist[fdistidx-1] );
        }
    }

    // API2: At MTK_CONTROL_AF_MODE_OFF mode, configure algorithm as MF mode.
    //          The auto-focus routine does not control the lens. Lens is controlled by the application.
    if( (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
            (m_sAFOutput.i4AFPos != i4ResultDac) &&
            (0<=i4ResultDac) &&
            (i4ResultDac<=1023))
    {
        LC.Add("DAC(%d->%d) Dist(%f)",
               m_sAFOutput.i4AFPos,
               i4ResultDac,
               lens_focusDistance);

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
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d) !!skip!! DAC(%d->%d) Dist(%f) lib_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_sAFOutput.i4AFPos,
                  i4ResultDac,
                  lens_focusDistance,
                  m_eLIB3A_AFMode);
    }
    LC.Print();
}

MFLOAT AfMgr::getFocusDistance()
{
    MINT32 fdacidx=0;
    MINT32 i4tblLL;
    MINT32 i4ResultDist  = 100;

    if (m_i4EnableAF == 0)
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "cmd-%s no AF", __FUNCTION__);
        return 0.0;
    }


    MINT32 LensPos;
    if( m_eLIB3A_AFMode == LIB3A_AF_MODE_MF)
    {
        /* In MF mode, algorithm will take some step to target position.
                  So directly using MFpos instead of using m_sAFOutput.i4AFPos.*/
        LensPos = m_i4MFPos;
    }
    else
    {
        LensPos = m_sAFOutput.i4AFPos;
    }

    if (m_pAFParam == NULL)
        return 0.0;

    if (m_pAFParam->i4TBLL == 0)
        return 0.0;

    // Lens position is changed, calculating new focus distance :
    if( m_sFocusDis.i4LensPos!=LensPos)
    {
        MUINT32 DAC_Inf        = m_sDAF_TBL.af_dac_inf;
        MUINT32 DAC_Marco      = m_sDAF_TBL.af_dac_marco;
        MUINT32 Distance_Inf   = m_sDAF_TBL.af_distance_inf;
        MUINT32 Distance_Marco = m_sDAF_TBL.af_distance_marco;
        MUINT32 NewLensPos     = LensPos;

        i4tblLL = m_pAFParam->i4TBLL;

        CAM_LOGD_IF(m_i4DgbLogLv, "cmd-%s [%d] [%d] [%d] [%d] - [%d] [%d]\n",
                    __FUNCTION__, DAC_Inf, DAC_Marco, Distance_Inf, Distance_Marco, m_pAFParam->i4Dist[i4tblLL-1], m_pAFParam->i4Dist[0]);

        if ((Distance_Marco == m_pAFParam->i4Dist[0]) && (Distance_Inf == m_pAFParam->i4Dist[i4tblLL-1]))
        {

            if (DAC_Marco - DAC_Inf > 0)
            {
                NewLensPos = ((LensPos - DAC_Inf) * (m_pAFParam->i4Dacv[0] - m_pAFParam->i4Dacv[i4tblLL-1])) / (DAC_Marco - DAC_Inf) + m_pAFParam->i4Dacv[i4tblLL-1];
            }

            CAM_LOGD_IF(m_i4DgbLogLv, "cmd-%s Pos remapping %d -> %d\n", __FUNCTION__, LensPos, NewLensPos);
        }

        if( NewLensPos <= m_pAFParam->i4Dacv[i4tblLL-1]) /*infinity*/
        {
            i4ResultDist = m_pAFParam->i4Dist[i4tblLL-1];
        }
        else if( m_pAFParam->i4Dacv[0] <= NewLensPos) /*macro*/
        {
            i4ResultDist = m_pAFParam->i4Dist[0];

        }
        else
        {
            for(fdacidx=0; fdacidx< i4tblLL ; fdacidx++)
            {
                if( NewLensPos >  m_pAFParam->i4Dacv[fdacidx])
                    break;
            }
            if(fdacidx==0)
            {
                i4ResultDist = m_pAFParam->i4Dist[0];
            }
            else
            {
                i4ResultDist=
                    ( m_pAFParam->i4Dist[fdacidx]   * (m_pAFParam->i4Dacv[fdacidx-1] - NewLensPos)
                      + m_pAFParam->i4Dist[fdacidx-1] * (LensPos - m_pAFParam->i4Dacv[fdacidx]))
                    /(m_pAFParam->i4Dacv[fdacidx-1] - m_pAFParam->i4Dacv[fdacidx] );
            }
            if( i4ResultDist<=0)
            {
                i4ResultDist = m_pAFParam->i4Dist[i4tblLL-1];
            }
        }

        MFLOAT i4Dist = 1000.0/ ((MFLOAT)i4ResultDist);

        CAM_LOGD_IF(m_i4DgbLogLv, "cmd-%s Pos %d->%d, Dis(%d) %f->%f\n",
                    __FUNCTION__,
                    m_sFocusDis.i4LensPos,
                    LensPos,
                    i4ResultDist,
                    m_sFocusDis.i4Dist,
                    i4Dist);

        //record.
        m_sFocusDis.i4LensPos = LensPos;
        m_sFocusDis.i4Dist = i4Dist;
    }

    return m_sFocusDis.i4Dist;
}

MVOID AfMgr::setOpticalStabilizationMode( MINT32 ois_OnOff)
{
    if( m_pMcuDrv)
    {
        if( ois_OnOff == TRUE)
        {
            m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MFALSE, m_i4CurrSensorDev);
        }
        else
        {
            m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MTRUE, m_i4CurrSensorDev);
        }
    }

    MUINT8 oismode=0;
    if( m_bGetMetaData==MFALSE)
    {
        return;
    }

    for( MUINT8 ii=0; ii<m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).count(); ii++)
    {
        oismode = m_sMetaData.entryFor(MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).itemAt( ii, Type2Type<MUINT8>());
        CAM_LOGD( "AFmeta OPTICAL_STABILIZATION %d  %d",ii, oismode);
        if(oismode==1)
        {
            //set_ois_drv_on(ois_OnOff);
            break;
        }
    }
}

MINT32 AfMgr::getOpticalStabilizationMode()
{
    MUINT8 oismode=0;
    if( m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for( MUINT8 ii=0; ii<m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).count(); ii++)
    {
        oismode = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION).itemAt( ii, Type2Type< MUINT8 >());
        CAM_LOGD( "AFmeta OPTICAL_STABILIZATION %d  %d", ii, oismode);
    }
    return oismode;/*OFF, ON */
}

MVOID AfMgr::getFocusRange( MFLOAT *vnear, MFLOAT *vfar)
{
    MINT32 i4tblLL;
    MINT32 fdacidx       =0;
    MINT32 i4ResultDist  = 100;
    MINT32 i4ResultRange = 100;
    MINT32 i4ResultNear  = 100;
    MINT32 i4ResultFar   = 100;
    MFLOAT retDist       = 0.0;

    if( m_i4EnableAF==0)
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "no focus lens");
        *vnear = 1/0.6;
        *vfar  = 1/3.0;
        return;
    }

    i4tblLL = m_pAFParam->i4TBLL;
    for( fdacidx=0; fdacidx<i4tblLL; fdacidx++)
    {
        if( m_sAFOutput.i4AFPos>m_pAFParam->i4Dacv[fdacidx])
            break;
    }

    if(fdacidx==0)
    {
        i4ResultDist = m_pAFParam->i4Dist[0];
    }
    else
    {
        i4ResultDist=
            ( m_pAFParam->i4Dist[fdacidx  ] * (m_pAFParam->i4Dacv[fdacidx-1] - m_sAFOutput.i4AFPos       )  +
              m_pAFParam->i4Dist[fdacidx-1] * (m_sAFOutput.i4AFPos          - m_pAFParam->i4Dacv[fdacidx])) /
            ( m_pAFParam->i4Dacv[fdacidx-1] - m_pAFParam->i4Dacv[fdacidx]);

        i4ResultRange=
            ( m_pAFParam->i4FocusRange[fdacidx  ] * (m_pAFParam->i4Dacv[fdacidx-1] - m_sAFOutput.i4AFPos       )  +
              m_pAFParam->i4FocusRange[fdacidx-1] * (m_sAFOutput.i4AFPos          - m_pAFParam->i4Dacv[fdacidx])) /
            ( m_pAFParam->i4Dacv[fdacidx-1] - m_pAFParam->i4Dacv[fdacidx]);
    }
    if( i4ResultDist <=0)  i4ResultDist= m_pAFParam->i4Dist[i4tblLL-1];
    if( i4ResultRange<=0) i4ResultRange= m_pAFParam->i4Dist[i4tblLL-1];

    i4ResultNear = i4ResultDist - (i4ResultRange/2);
    i4ResultFar  = i4ResultDist + (i4ResultRange/2);

    *vnear = 1000.0/ ((MFLOAT)i4ResultNear);
    *vfar  = 1000.0/ ((MFLOAT)i4ResultFar);

    //CAM_LOGD("[getFocusRange] [%f, %f]", *vnear,*vfar);
}

MINT32 AfMgr::getLensState  ()
{
    MINT32 ret = 0;
    if( m_i4MvLensToPre!=m_i4MvLensTo)
    {
        /* MOVING */
        ret = 1;
    }
    else
    {
        /* STATIONARY */
        ret = 0;
    }

    m_i4MvLensToPre = m_i4MvLensTo;

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID AfMgr::setSharpnessMapMode(MINT32 SharpMapOnOff)
{
    SharpMapOnOff=0;
    //check available table, set lens_focalLength
}
MINT32 AfMgr::getSharpnessMapMode()
{
    return    1;
}
MINT32 AfMgr::getMaxSharpnessMapValue ()
{
    return    0xFFFFFFFF;
}
MINT32 AfMgr::getSharpnessMapSize()
{
    return  32;
}
MVOID AfMgr::getSharpnessMap(MVOID* a_sAFFullStat)
{
    a_sAFFullStat=NULL;
}

MVOID AfMgr::SaveAfMgrInfo(const char * fname)
{
    MUINT32 length=0;
    while(fname[length]!='\n' && length<200)
    {
        length++;
    }
    length = length>199?199:length;
    strncpy(mEngFileName, fname, (length+1));
}

MBOOL AfMgr::IsFullScanMode(void)
{
    return m_bIsFullScan;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::setSensorMode(MINT32 i4NewSensorMode)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): %d -> %d",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4SensorMode,
              i4NewSensorMode);

    m_i4SensorMode = i4NewSensorMode;

    switch( m_i4SensorMode)
    {
    case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
        m_sHostCmd.a_eCamScenarioMode = CAM_SCENARIO_PREVIEW;
        break;
    case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
        m_sHostCmd.a_eCamScenarioMode = CAM_SCENARIO_CAPTURE;
        break;
    case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
    case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
    case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
        m_sHostCmd.a_eCamScenarioMode = CAM_SCENARIO_VIDEO;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM1:
    case SENSOR_SCENARIO_ID_CUSTOM5:
        m_sHostCmd.a_eCamScenarioMode = CAM_SCENARIO_CUSTOM1;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM2:
        m_sHostCmd.a_eCamScenarioMode = CAM_SCENARIO_CUSTOM2;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM3:
    case SENSOR_SCENARIO_ID_CUSTOM4:
        m_sHostCmd.a_eCamScenarioMode = CAM_SCENARIO_CUSTOM3;
        break;
    default:
        CAM_LOGE("UNSUPPORT_SENSOR_MODE: %d\n", i4NewSensorMode);
        return MFALSE;
    }

    if(m_bNVRAMGot)
    {
        setCamScenarioMode( m_sHostCmd.a_eCamScenarioMode);
    }
    else
    {
        sendAFRequest(AF_CMD_SETSENSORMODE, (MINTPTR)&m_sHostCmd);
    }

    return MTRUE;
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
                 "#(%5d,%5d) %s Dev(%d) TG(%d): set(X,Y,W,H)=( %d, %d, %d, %d)",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_eSensorTG,
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
    MINT32 i4FirstTimeConfig = (m_eEvent&EVENT_CMD_START) ? 1 : 0;
    ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).config( m_eSensorTG, sInHWCfg, sOutHWROI, i4OutHWBlkNumX, i4OutHWBlkNumY, i4OutHWBlkSizeX, i4OutHWBlkSizeY);

    m_i4HWEnExtMode = sInHWCfg.AF_EXT_ENABLE;

    // error log : should not be happened.
    if( sInHWCfg.AF_BLK_XNUM != i4OutHWBlkNumX  ||
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
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatStart()
{
    MINT32 ReadOutTimePerLine = 0;
    MINT32 P1TimeReadOut      = 0;

    if (m_sAFInput.PixelClk > 0)
    {
        ReadOutTimePerLine = (MINT32)(((MINT64)m_sAFInput.PixelInLine * 1000000000) / m_sAFInput.PixelClk); // unit: ns
        P1TimeReadOut      = (MINT32)(ReadOutTimePerLine * m_i4TGSzH / 1000); // unit:us
    }
    CAM_LOGD( "AF-%-15s: ReadOutTimePerLine(%d), P1DoneTime(%d)", __FUNCTION__, ReadOutTimePerLine, P1TimeReadOut);

    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).start(m_i4CurrSensorDev, m_i4SensorIdx);
    if(m_eSensorTG == ESensorTG_1)
    {
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AFO, 1, MNULL))
        {
            CAM_LOGE("Enable EModule_AFO fail");
        }
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_EN, 1,-1))
        {
            CAM_LOGE("EPipe_P1Sel_SGG_EN fail");
        }
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG, 1,-1))
        {
            CAM_LOGE("EPipe_P1Sel_SGG fail");
        }
    }
    else if(m_eSensorTG == ESensorTG_2)
    {
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AFO_D, 1, MNULL))
        {
            CAM_LOGE("Enable EModule_AFO_D fail");
        }
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_EN_D, 1,-1))
        {
            CAM_LOGE("EPipe_P1Sel_SGG_EN_D fail");
        }
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_D, 1,-1))
        {
            CAM_LOGE("EPipe_P1Sel_SGG_D fail");
        }
    }

    // First time HW setting is got after calling initAF.
    ConfigHWReg( m_sAFOutput.sAFStatConfig, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);

    {
        Mutex::Autolock lock( m_CmdQLock);
        m_i4VsyncUpdate = MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatStop()
{
    //uninitial isp_mgr_af_stat for configure HW
    {
        Mutex::Autolock lock( m_CmdQLock);
        m_i4VsyncUpdate = MFALSE;
    }

    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).stop();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::IspMgrAFStatHWPreparing()
{
    return (m_afwin_synccnt>=2 ? MFALSE : MTRUE);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatUpdateHw()
{
#if 1 //defined(MTKCAM_CMDQ_SUPPORT)
    //Configure/update HW setting during vsync
    Mutex::Autolock lock( m_CmdQLock);
    memcpy( &m_sHWCfg, &m_sAFOutput.sAFStatConfig, sizeof(AF_CONFIG_T));
    m_i4VsyncUpdate = MTRUE;
    CAM_LOGD_IF(m_i4DgbLogLv, "[%s]", __FUNCTION__);
#else
    // AF control on CCU - per-frame sequence limit : Dequeue AFO -> set AF HW -> Enqueue AFO
    ConfigHWReg( m_sAFOutput.sAFStatConfig, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setCurFrmNum(MUINT32 frm_num)
{
    m_eEvent = EVENT_NONE;

    Mutex::Autolock lock(m_Lock);

    m_u4ReqMagicNum = frm_num;

    //depth AF
    m_AFStatusTbl[m_u4ReqMagicNum%16][0]=m_u4ReqMagicNum;
    m_AFStatusTbl[m_u4ReqMagicNum%16][1]=(MUINT32)(m_sAFOutput.i4IsAfSearch==0);

    if( m_i4DgbLogLv)
    {
        CAM_LOGD( "cmd-%s ReqMagicNum #%d", __FUNCTION__, m_u4ReqMagicNum);
    }
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
    if( (bEn==0||bEn==1) && m_bMZHostEn!=bEn)
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_bMZHostEn, bEn);

        m_bMZHostEn = bEn;
        m_bMZAFEn     = m_i4DbgMZEn||(m_bMZHostEn==1) ? MTRUE : MFALSE;

        //Accroding to option, updating center ROI coordinate automatically
        UpdateCenterROI( m_sArea_Center);

        //Algo limit : algo need to be retriggered if mz is enabled while af searching
        if(bEn==1 && m_pIAfAlgo)
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
        if (ILaserMgr::getInstance().init(m_i4CurrSensorDev) == 1)
        {
            m_bLDAFEn = MTRUE;
            CAM_LOGD( "[%s] ILaserMgr init() done", __FUNCTION__);
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "[%s] ILaserMgr init() fail", __FUNCTION__);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_Start()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);

    if(m_bLDAFEn == MTRUE)
    {
        if( property_get_int32("vendor.laser.calib.disable", 0) == 0 )
        {
            MUINT32 OffsetData = (MUINT32)m_ptrNVRam->rAFNVRAM.i4Revs[201];
            MUINT32 XTalkData  = (MUINT32)m_ptrNVRam->rAFNVRAM.i4Revs[202];
            ILaserMgr::getInstance().setLaserCalibrationData(m_i4CurrSensorDev, OffsetData, XTalkData);
        }

        MUINT32 LaserMaxDistance = (MUINT32)m_ptrNVRam->rAFNVRAM.i4Revs[203];
        MUINT32 LaserTableNum    = (MUINT32)m_ptrNVRam->rAFNVRAM.i4Revs[204];
        ILaserMgr::getInstance().setLaserGoldenTable(m_i4CurrSensorDev, (MUINT32*)&m_ptrNVRam->rAFNVRAM.i4Revs[205], LaserTableNum, LaserMaxDistance);

        m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;

        if( ILaserMgr::getInstance().checkHwSetting(m_i4CurrSensorDev)==0 || property_get_int32("vendor.laser.disable", 0)==1)
        {
            ILaserMgr::getInstance().uninit(m_i4CurrSensorDev);
            m_bLDAFEn = MFALSE;
            CAM_LOGE( "AF-%-15s: ILaserMgr checkHwSetting() fail", __FUNCTION__);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_CamPwrOffState()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s]", __FUNCTION__);

    if (m_bLDAFEn == MTRUE)
    {
        CAM_LOGD( "[%s] ILaserMgr uninit() done", __FUNCTION__);
        ILaserMgr::getInstance().uninit(m_i4CurrSensorDev);
        m_bLDAFEn = MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::LDAF_Update()
{
    if (m_bLDAFEn == MTRUE)
    {
        MINT32 LaserStatus = ILaserMgr::getInstance().getLaserCurStatus(m_i4CurrSensorDev);

        if( LaserStatus == STATUS_RANGING_VALID)
        {
            m_sAFInput.sLaserInfo.i4CurPosDAC   = ILaserMgr::getInstance().getLaserCurDac(m_i4CurrSensorDev);
            m_sAFInput.sLaserInfo.i4CurPosDist  = ILaserMgr::getInstance().getLaserCurDist(m_i4CurrSensorDev);
        }
        else
        {
            m_sAFInput.sLaserInfo.i4CurPosDAC   = ILaserMgr::getInstance().predictAFStartPosDac(m_i4CurrSensorDev);
            m_sAFInput.sLaserInfo.i4CurPosDist  = ILaserMgr::getInstance().predictAFStartPosDist(m_i4CurrSensorDev);
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

    if( m_i4CurrLensId != 0xffff)
    {
        MINT32 AFtbl_Num;
        MINT32 AFtbl_Marco;
        MINT32 AFtbl_Inf;

        m_bSDAFEn = (m_ptrNVRam->rAFNVRAM.i4SDAFCoefs[1]>0) ? MTRUE : MFALSE;

        AFtbl_Num   = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
        AFtbl_Inf   = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
        AFtbl_Marco = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[AFtbl_Num-1];

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
    if( m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE)
    {
        MUINT32 i4curFrmNum = m_u4ReqMagicNum;

        //Input Depth Info
        MUINT32 QueryDafTblIdx = m_next_query_FrmNum % DAF_TBL_QLEN;

        m_sDAF_TBL.curr_p1_frm_num = i4curFrmNum;

        // af_mgr -> af_algo
        m_sAFInput.i4CurrP1FrmNum = i4curFrmNum;
        if(m_next_query_FrmNum == 0xFFFFFFFF)
        {
            m_sAFInput.i4DafDacIndex = 0;
            m_sAFInput.i4DafConfidence = 0;
        }
        else
        {
            m_sAFInput.i4DafDacIndex   = m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_dac_index;
            m_sAFInput.i4DafConfidence = m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_confidence;
        }

        CAM_LOGD( "#(%5d,%5d) DAF--[Mode]%d [cp1#]%d [cp2#]%d [nextF#]%d [DafDac]%d [DafConf]%d [daf_dist]%d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  (MINT32)m_sAFInput.i4HybridAFMode,
                  (MINT32)m_sAFInput.i4CurrP1FrmNum,
                  (MINT32)m_sDAF_TBL.curr_p2_frm_num,
                  (MINT32)m_next_query_FrmNum,
                  (MINT32)m_sAFInput.i4DafDacIndex,
                  (MINT32)m_sAFInput.i4DafConfidence,
                  m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_distance);

        if( m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_confidence)
        {
            CAM_LOGD( "#(%5d,%5d) DAFAA-%d %d",
                      m_u4ReqMagicNum,
                      m_u4StaMagicNum,
                      m_daf_distance,
                      m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_confidence);

            m_daf_distance = (MINT32)m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_distance;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SDAF_Result()
{
    //==========
    // Prepare AF info to Vsdof
    //==========
    if( m_sDAF_TBL.is_daf_run & E_DAF_RUN_STEREO)
    {
        MUINT32 MagicNum, CurDafTblIdx;

        MagicNum = m_u4ReqMagicNum; // m_u4StaMagicNum + 1; // m_sAFOutput can match next image status

        CurDafTblIdx = MagicNum % DAF_TBL_QLEN;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].frm_mun          = MagicNum;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_mode          = m_eCurAFMode;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_roi_sel       = m_sAFOutput.i4ROISel;
        // Contrast AF output info
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_pos       = m_sAFOutput.i4AFPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].thermal_lens_pos = m_sAFOutput.i4ThermalLensPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].posture_dac      = m_sAFOutput.i4PostureDac;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_index     = m_sAFOutput.i4AfDacIndex;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_af_stable     = ((m_sAFOutput.i4IsAfSearch==AF_SEARCH_DONE) && m_sCurLensInfo.bIsMotorOpen)?1:0;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_scene_stable  = !m_sAFOutput.i4IsSceneChange;

#if 0
        if (m_sAFOutput.i4IsAfSearch != AF_SEARCH_DONE)
        {
            // use the current face roi to avoid the finally inconsistency between depth point and focusing roi
            if (m_sAFOutput.i4ROISel == AF_ROI_SEL_FD)
            {
                m_sArea_Bokeh = m_sArea_OTFD;
            }
            else
            {
                if (m_sAFOutput.i4IsAfSearch == AF_SEARCH_CONTRAST)
                {
                    m_sArea_Bokeh = m_sArea_Focusing;
                }
                else
                {
                    m_sArea_Bokeh = m_sAFOutput.sPDWin;
                }
            }
        }
#else
        m_sArea_Bokeh = m_sArea_Focusing;
#endif

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

    //==========
    // Prepare depth AF data to DEPTH engine
    //==========
    if( m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE)
    {
        MUINT32 MagicNum, CurDafTblIdx;

        MagicNum = m_u4StaMagicNum + 1;

        CurDafTblIdx = MagicNum % DAF_TBL_QLEN;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].frm_mun        = MagicNum;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_learning    = m_sAFOutput.i4IsLearning;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_querying    = m_sAFOutput.i4IsQuerying;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_valid       = m_sAFOutput.i4AfValid;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_index   = m_sAFOutput.i4AfDacIndex;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_confidence  = m_sAFOutput.i4AfConfidence;

        m_sDAF_TBL.daf_vec[CurDafTblIdx].daf_dac_index  = 0;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].daf_confidence = 0;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].daf_distance   = 0;

        m_sDAF_TBL.is_query_happen = m_sAFOutput.i4QueryFrmNum;

        m_next_query_FrmNum = m_sAFOutput.i4QueryFrmNum;

        CAM_LOGD( "DAF--[islrn]%d [isqry]%d [afVld]%d [af_dac]%d [afConf]%d [af_win]%d %d %d %d [nextF#]%d\n",
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].is_learning,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].is_querying,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_valid,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_index,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_confidence,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x,
                  (MINT32)m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y,
                  (MINT32)m_next_query_FrmNum);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::queryAFStatusByMagicNum(MUINT32 i4MagicNum)
{
    if(m_AFStatusTbl[i4MagicNum%16][0]==i4MagicNum)
        return (MINT32)m_AFStatusTbl[i4MagicNum%16][1];
    else
    {
        CAM_LOGD("[query_frm_num] %d [table_frm_num] %d, not match !!", i4MagicNum, m_AFStatusTbl[i4MagicNum%16][0] );
        return -1;
    }
    return -1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::updateAFWinStatus( AREA_T &sNewHWArea, AREA_T &sPreHWArea)
{
    //compare
    if(
        sNewHWArea.i4X == sPreHWArea.i4X &&
        sNewHWArea.i4Y == sPreHWArea.i4Y &&
        sNewHWArea.i4W == sPreHWArea.i4W &&
        sNewHWArea.i4H == sPreHWArea.i4H )
    {
        if( m_afwin_synccnt<2) //avoid overflow
        {
            m_afwin_synccnt++;
        }
    }
    else
    {
        m_afwin_synccnt = 0;
    }

    //record
    memcpy( &sPreHWArea, &sNewHWArea, sizeof(AREA_T));
}

/******************************************************************************/
MBOOL AfMgr::sendAFRequest(AF_CMD_T const r3ACmd, MUINTPTR const i4Arg)
{
    HostCmdQ_T rReq;
    rReq.afCmd = r3ACmd;
    if (i4Arg != 0)
        rReq.rParam = *reinterpret_cast<HostCmdParam_t*>(i4Arg);
    CAM_LOGD_IF(m_i4DgbLogLv, "[%s] rAFHostCmd(%d)\n", __FUNCTION__, r3ACmd);

    addAFRequestQ(rReq);
    return MTRUE;
}
// add rReq to mHostReqQ
MVOID AfMgr::addAFRequestQ(HostCmdQ_T &rReq)
{
    Mutex::Autolock autoLock(mReqQMtx);

    mHostReqQ.push_back(rReq);

    CAM_LOGD_IF(m_i4DgbLogLv, "[%s] mHostReqQ size(%d) + cmd(%d)", __FUNCTION__, (MINT32)mHostReqQ.size(), rReq.afCmd);
}

MVOID AfMgr::clearAFRequestQ()
{
    FUNC_BEGIN;
    Mutex::Autolock autoLock(mReqQMtx);

    MINT32 Qsize = mHostReqQ.size();

    for (List<HostCmdQ_t>::iterator it = mHostReqQ.begin(); it != mHostReqQ.end();)
    {
        AF_CMD_T afCmd = it->afCmd;
        CAM_LOGD("[%s] cmd size(%d), clear(%d)", __FUNCTION__, (MINT32)mHostReqQ.size(), afCmd);
        it = mHostReqQ.erase(it);
    }
    CAM_LOGD("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
}

MBOOL AfMgr::getAFRequest(HostCmdQ_T &rReq, MINT32 i4Count)
{
    LogCollector LC_getAFRequest(LOG_TAG);

    if(m_i4DgbLogLv)
    {
        LC_getAFRequest.Add("#(%5d,%5d) %s Dev(%d): mCmdQ.size(%d) ", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, (MINT32)mHostReqQ.size());
    }

    Mutex::Autolock autoLock(mReqQMtx);

    if (mHostReqQ.size() == 0) //this is only for en_timeout == 1 & timeout case
    {
        LC_getAFRequest.Print();
        return MFALSE;
    }

    rReq = *mHostReqQ.begin();

    mHostReqQ.erase(mHostReqQ.begin());

    if(m_i4DgbLogLv)
    {
        LC_getAFRequest.Add("eCmd(%d)", rReq.afCmd);
        LC_getAFRequest.Print();
    }

    return MTRUE;
}


