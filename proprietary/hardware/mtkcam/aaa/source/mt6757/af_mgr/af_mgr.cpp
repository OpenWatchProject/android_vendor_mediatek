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
#include <mtkcam/aaa/drv/mcu_drv.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include <cct_feature.h>
#include <isp_tuning_mgr.h>
#include <af_feature.h>

#include "af_mgr.h"
#include <nvbuf_util.h>
#include <isp_mgr.h>
#include "aaa_common_custom.h"
#include <pd_mgr_if.h>
#include <laser_mgr_if.h>

//configure HW
#include <isp_mgr_af_stat.h>
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

#define DBG_MSG_BUF_SZ 800 //byte

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
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::AfMgr( ESensorDev_T eSensorDev) :
    m_bGryoVd( MFALSE),
    m_bAcceVd( MFALSE)
{
    m_i4CurrSensorDev = (MINT32)eSensorDev;
    m_i4Users   = 0;
    m_CCTUsers  = 0;
    m_pMcuDrv   = NULL;
    m_pIAfAlgo  = NULL;
    m_bLaserAFEn    = MFALSE;
    memset( &m_sAFInput,      0, sizeof(AF_INPUT_T));
    memset( &m_sAFOutput,     0, sizeof(AF_OUTPUT_T));
    memset( &m_sAFParam,      0, sizeof(AF_PARAM_T));
    memset( &m_sNVRam,        0, sizeof(NVRAM_LENS_PARA_STRUCT));
    memset( &m_sArea_Center,  0, sizeof(AREA_T));
    memset( &m_sArea_APCmd,   0, sizeof(AREA_T));
    memset( &m_sArea_APCheck, 0, sizeof(AREA_T));
    memset( &m_sArea_OTFD,    0, sizeof(AREA_T));

    m_i4CurrSensorId = 0x1;
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
    pIHalSensorInfo  = NULL;

    m_bPdSupport     = MFALSE;
    m_PDPipeCtrl     = FEATURE_PDAF_UNSUPPORT;
    m_i4PDSensorType = SensorType_NO_PDAF;
    m_bFirstInitAF   = MFALSE;
    m_i4InitPos      = 0;
    mpSensorProvider = nullptr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::~AfMgr()
{}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sendAFNormalPipe( MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL ret = MFALSE;

    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe( m_i4SensorIdx, LOG_TAG);

    if( pPipe==NULL)
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
    MBOOL bSensorModeSupportPDAF = 0;

    /*
     * Calling timing :
     *    After "3A" and "isp" modules are initialized, calling this function by Hal3ASttCtrl before starting AF module.
     * Note :
     *    Do not use pIHalSensorInfo.
     *    Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer).
     *    Outputs u4PDOSizeW, u4PDOSizeH, PDAFStatus for each type PD HW path for configuring SttPipe in 3A framework.
     */
    u4PDOSizeW = 0;
    u4PDOSizeH = 0;
    PDAFStatus = FEATURE_PDAF_UNSUPPORT;


    MINT32 i4DisablePD = property_get_int32("vendor.disablepd", 0);
    if (i4DisablePD)
    {
        CAM_LOGE("PD disabled by property (disablepd)");
        m_PDPipeCtrl = PDAFStatus;
        m_bPdSupport = bSensorModeSupportPDAF;
        return S_AF_OK;
    }

    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        m_PDPipeCtrl = PDAFStatus;
        m_bPdSupport = bSensorModeSupportPDAF;
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
     * Outputs u4PDOSizeW, u4PDOSizeH, PDAFStatus by PD type.
     */
    if( bSensorModeSupportPDAF == 0)
    {
        u4PDOSizeW = 0;
        u4PDOSizeH = 0;
        PDAFStatus = FEATURE_PDAF_UNSUPPORT;

    }
    else if( sensorStaticInfo.PDAF_Support == 1)
    {
        MUINT32 u4TGSzW=0, u4TGSzH=0, u4BINSzW=0, u4BINSzH=0;
        // TG size
        sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&u4TGSzW), (MINTPTR)(&u4TGSzH),0);
        // TG after BIN Blk size : for HPF coordinate setting.
        sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO, (MINTPTR)(&u4BINSzW), (MINTPTR)(&u4BINSzH), 0);
        //
        MBOOL isTGInfoValid       = (0<u4TGSzW) && (u4TGSzW<=(MUINT32)sensorStaticInfo.captureWidth) && (0<u4TGSzH) && (u4TGSzH<=(MUINT32)sensorStaticInfo.captureHeight);
        MBOOL isFrontalBINDisable = (u4TGSzW==u4BINSzW) && (u4TGSzH==u4BINSzH);
        MBOOL isFullMode          = (u4TGSzW==(MUINT32)sensorStaticInfo.captureWidth) && (u4TGSzH==(MUINT32)sensorStaticInfo.captureHeight);
        // Read BPCI table related information from BNR2 ISP module.
        MUINT32  u4Bpci_xsz=0, u4Bpci_ysz=0, u4Pdo_xsz=0, u4Pdo_ysz=0;
        MUINTPTR phyAddrBpci_tbl, virAddrBpci_tbl;

        MBOOL isBPCItbl = ISP_MGR_BNR2::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getBPCItable(
                              u4Bpci_xsz,
                              u4Bpci_ysz,
                              u4Pdo_xsz,
                              u4Pdo_ysz,
                              phyAddrBpci_tbl,
                              virAddrBpci_tbl);

        MBOOL isBPCItblValid = (isBPCItbl) && (0<(u4Pdo_xsz+1)) && ((u4Pdo_xsz+1)<=u4BINSzW) && (0<(u4Pdo_ysz+1)) && ((u4Pdo_ysz+1)<=u4BINSzH) && ((u4Bpci_ysz+1)==1) && (0<u4Bpci_xsz);

        //
        CAM_LOGD( "[%s] Dev 0x%04x, TGSZ: W(%d) H(%d), BINSZ: W(%d) H(%d), isTGInfoValid(%d)/isFrontalBINDisable(%d)/isFullMode(%d)/isBPCItblValid(%d)/BPCITbl: xSz(%d) ySz(%d) pa(0x%x) va(0x%x)/PDO: xSz(0x%x) ySz(0x%x)",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  u4TGSzW,
                  u4TGSzH,
                  u4BINSzW,
                  u4BINSzH,
                  isTGInfoValid,
                  isFrontalBINDisable,
                  isFullMode,
                  isBPCItblValid,
                  u4Bpci_xsz,
                  u4Bpci_ysz,
                  phyAddrBpci_tbl,
                  virAddrBpci_tbl,
                  u4Pdo_xsz,
                  u4Pdo_ysz);

        if( (isTGInfoValid) && (isFrontalBINDisable) && (isFullMode) && (isBPCItblValid))
        {
            /*
             * PDAF Raw Data mode is supported once frontal binning is not enalbed in full mode.
             */
            u4PDOSizeW = u4TGSzW;
            u4PDOSizeH = u4TGSzH;
            PDAFStatus = FEATURE_PDAF_SUPPORT_BNR_PDO;
        }
        else
        {
            u4PDOSizeW = 0;
            u4PDOSizeH = 0;
            PDAFStatus = FEATURE_PDAF_UNSUPPORT;
        }


    }
    else if( sensorStaticInfo.PDAF_Support == 4)
    {
        MUINT32 u4TGSzW=0, u4TGSzH=0;
        // TG size
        sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&u4TGSzW), (MINTPTR)(&u4TGSzH),0);

        u4PDOSizeW = u4TGSzW; // no use for dual pd raw type case, set TG size just for avoid P1 pipe check fail.
        u4PDOSizeH = u4TGSzH;
        PDAFStatus = FEATURE_PDAF_SUPPORT_PBN_PDO;
    }
    else if(sensorStaticInfo.PDAF_Support == 2 || sensorStaticInfo.PDAF_Support == 3 || sensorStaticInfo.PDAF_Support == 5)
    {
        u4PDOSizeW = 0; // no use for camsv case.
        u4PDOSizeH = 0;
        PDAFStatus = FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL;
    }
    else
    {
        u4PDOSizeW = 0;
        u4PDOSizeH = 0;
        PDAFStatus = FEATURE_PDAF_UNSUPPORT;
    }


    m_PDPipeCtrl = PDAFStatus;
    m_bPdSupport = bSensorModeSupportPDAF;

    CAM_LOGD("[%s] SensorMode(%d), PDAF_Support(%d), bSensorModeSupportPDAF(%d), Info for 3A framework to Cfg sttPipe :PDAFStatus(%d), PDOSzW(%d), PDOSzH(%d)",
             __FUNCTION__,
             i4SensorMode,
             sensorStaticInfo.PDAF_Support,
             bSensorModeSupportPDAF,
             PDAFStatus,
             u4PDOSizeW,
             u4PDOSizeH);

    if( pIHalSensor)
    {
        pIHalSensor->destroyInstance( "getPdInfoForSttCtrl");
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::init( MINT32 i4SensorIdx, MINT32 isInitMCU)
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    MRESULT ret = E_3A_ERR;

    Mutex::Autolock lock(m_Lock);

    if( m_i4Users==0)
    {
        /* Init af_mgr, when no user calls af_mgr init.*/

        // initial property
        m_i4DgbLogLv        = property_get_int32("vendor.debug.af_mgr.enable", 0);
        m_bPDVCTest         = property_get_int32("vendor.debug.pd_vc.enable", 0);
        m_i4DbgAfegainQueue = property_get_int32("vendor.debug.afegainqueue", 0);

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

        // Get hybrid AF instance.
#if USE_OPEN_SOURCE_AF
        m_pIAfAlgo=NS3A::IAfAlgo::createInstance<NS3A::EAAAOpt_OpenSource>( m_i4CurrSensorDev);
#else
        m_pIAfAlgo=NS3A::IAfAlgo::createInstance<NS3A::EAAAOpt_MTK>( m_i4CurrSensorDev);
#endif

        m_sDAF_TBL.is_daf_run = E_DAF_OFF;

        // --- init Laser ---
        if (m_bLaserAFEn == MFALSE)
        {
            if (ILaserMgr::getInstance().init(m_i4CurrSensorDev) == 1)
            {
                m_bLaserAFEn = MTRUE;
                CAM_LOGD( "[%s] ILaserMgr init() done", __FUNCTION__);
            }
            else
            {
                CAM_LOGD_IF( m_i4DgbLogLv, "[%s] ILaserMgr init() fail", __FUNCTION__);
            }
        }

        // --- Create SensorProvider Object ---
        AAA_TRACE_D("SensorProvider");
        if( mpSensorProvider == NULL)
        {
            mpSensorProvider = SensorProvider::createInstance(LOG_TAG);

            if( mpSensorProvider!=nullptr)
            {
                MUINT32 interval= 30;

                interval = property_get_int32("vendor.debug.af_mgr.gyrointerval", SENSOR_GYRO_POLLING_MS);
                if( mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval))
                {
                    m_bGryoVd = MTRUE;
                    CAM_LOGD("Dev(%d):enable SensorProvider success (%d)", m_i4CurrSensorDev, m_bGryoVd);
                }
                else
                {
                    m_bGryoVd = MFALSE;
                    CAM_LOGE("Enable SensorProvider fail");
                }

                interval = property_get_int32("vendor.debug.af_mgr.gyrointerval", SENSOR_ACCE_POLLING_MS);
                if( mpSensorProvider->enableSensor(SENSOR_TYPE_ACCELERATION, interval))
                {
                    m_bAcceVd = MTRUE;
                    CAM_LOGD("Dev(%d):enable SensorProvider success (%d)", m_i4CurrSensorDev, m_bGryoVd);
                }
                else
                {
                    m_bAcceVd = MFALSE;
                    CAM_LOGE("Enable SensorProvider fail");
                }

            }
        }
        AAA_TRACE_END_D;

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
    CAM_LOGD( "AF-%-15s: EnAF %d, Algo(0x%x), users %d", __FUNCTION__, m_i4EnableAF, m_pIAfAlgo, m_i4Users);

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::uninit( MINT32 isInitMCU)
{
    CAM_LOGD( "AF-%-15s: +", __FUNCTION__);

    MRESULT ret = E_3A_ERR;

    Mutex::Autolock lock(m_Lock);

    if( m_i4Users==1)
    {
        /* Uninit af_mgr, when last user calls af_mgr uninit.*/

        // restore AF-sync parameters
        //SyncAFWriteDatabase();

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

        if( mpSensorProvider != NULL)
        {
            mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
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
    CAM_LOGD( "AF-%-15s: Algo(0x%x), users %d", __FUNCTION__, m_pIAfAlgo, m_i4Users);

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::Start()
{
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    Mutex::Autolock lock( m_Lock);

    //reset member.
    memset( &m_sAFInput,        0, sizeof(m_sAFInput));
    memset( &m_sAFOutput,       0, sizeof(m_sAFOutput));
    memset( &m_sHWCfg,          0, sizeof(AF_CONFIG_T));
    memset( &m_sCropRegionInfo, 0, sizeof(AREA_T));
    memset( &m_sArea_Focusing,  0, sizeof(AREA_T));
    memset( &m_sArea_Center,    0, sizeof(AREA_T));
    memset( &m_sArea_APCmd,     0, sizeof(AREA_T));
    memset( &m_sArea_APCheck,   0, sizeof(AREA_T));
    memset( &m_sArea_OTFD,      0, sizeof(AREA_T));
    memset( &m_sArea_HW,        0, sizeof(AREA_T));
    memset( &m_sArea_PD[0],     0, sizeof(AREA_T)*AF_PSUBWIN_NUM);
    memset( &m_aAEBlkVal,        0, sizeof(MUINT8)*25);

    m_vFrmInfo.clear();
    m_vISQueue.clear();
    m_i4EnableAF    = -1;
    m_i4PDAreaNum   = 0;
    m_i4CurLensPos  = 0;
    m_i4PreLensPos  = 0;
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
    m_u4HalMagicNum = 0;
    m_bPdInputExpected = MFALSE;
    m_i4PDResNum    = 0;
    m_i4OTFDLogLv   = 0;
    m_eEvent        = EVENT_NONE;
    m_i4IsSearchDone_PreState = MTRUE; /*Force to select ROI to focusing as first in, ref:SelROIToFocusing */
    m_i4IsSearchDone_CurState = MTRUE;
    m_i4IsAFDone_CurState     = MTRUE;
    m_i4IsAFDone_PreState     = MTRUE;
    m_i4IsMonitorFV_CurState  = MTRUE;
    m_i4IsMonitorFV_PreState  = MTRUE;
    m_bCheckStateByAFDone     = MFALSE;
    m_bRunPDEn          = MFALSE;
    m_bDAFEn            = MFALSE;
    m_bMZEn             = MFALSE;
    m_bGetMetaData      = MFALSE;
    m_bPauseAF          = MFALSE;
    m_ptrNVRam          = NULL;
    m_bNeedPdoResult    = MFALSE;
    m_i4TAFStatus       = TAF_STATUS_RESET;
    m_eCamScenarioMode  = CAM_SCENARIO_PREVIEW;
    m_i4DbgOISPos       = 0;
    m_u4FrameCount      = 0;
    m_bIsDoAF           = MFALSE;
    m_fMfFocusDistance  = -1;

    //initial isp_mgr_af_stat for configure HW
    IspMgrAFStatInit();

    //------------------------------------------- check algo instance-----------------------------------------
    if( !m_pIAfAlgo)
    {
        CAM_LOGE("AF-%-15s: AfAlgo pointer NULL", __FUNCTION__);
        m_i4EnableAF = 0;
        return S_AF_OK;
    }
    //-------------------------------------get sensor related information-----------------------------------
    SensorStaticInfo rSensorStaticInfo;
    if( m_i4EnableAF == -1)
    {
        IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
        IHalSensor* pIHalSensor = pIHalSensorList->createSensor( "af_mgr", m_i4SensorIdx);
        SensorDynamicInfo rSensorDynamicInfo;
        m_sMetaData = pIHalSensorList->queryStaticInfo( m_i4SensorIdx);
        m_bGetMetaData = MTRUE;

        switch( m_i4CurrSensorDev)
        {
        case ESensorDev_Main :
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
            pIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
            pIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
            pIHalSensor->querySensorDynamicInfo( NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
            break;
        default:
            CAM_LOGE( "AF-%-15s: Invalid sensor device: %d", __FUNCTION__, m_i4CurrSensorDev);
            break;
        }

        if( pIHalSensor)
        {
            pIHalSensor->destroyInstance( "af_mgr");
        }

        /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning)*/
        CAM_LOGD( "AF-%-15s: PD sensor support info %d",
                  __FUNCTION__,
                  rSensorStaticInfo.PDAF_Support);

        /* 0:BGGR, 1:GBRG, 2GRBG, 3RGGB */
        m_i4PixelId = (MINT32)rSensorStaticInfo.sensorFormatOrder;
        CAM_LOGD_IF( m_i4DgbLogLv, "AF-%-15s: PixId %d (0:B, 1:GB, 2:GR, 3:R)",
                     __FUNCTION__,
                     m_i4PixelId);

        /* Lens search */
        m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
        if( m_pMcuDrv == NULL)
        {
            /* This part MUST be done before [Get Data from NVRAM] */
            MCUDrv::lensSearch( m_i4CurrSensorDev, m_i4CurrSensorId);
            m_i4CurrLensId = MCUDrv::getCurrLensID( m_i4CurrSensorDev);
        }
        CAM_LOGD( "AF-%-15s: Dev 0x%04x, SensorID 0x%04x, LensId 0x%04x",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_i4CurrSensorId,
                  m_i4CurrLensId);

        if( m_i4CurrLensId == 0xFFFF) m_i4EnableAF = 0;
        else                          m_i4EnableAF = 1;

    }

    //---------------------------------------- init sensor related information -------------------------------------------
    // Get sensor information :
    // TG size
    sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&m_i4TGSzW), (MINTPTR)(&m_i4TGSzH),0);
    // TG after BIN Blk size : for HPF coordinate setting.
    sendAFNormalPipe( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO, (MINTPTR)(&m_i4BINSzW), (MINTPTR)(&m_i4BINSzH), 0);
    // Checking sensor mode.
    CAM_LOGD( "AF-%-15s: Dev 0x%04x, TGSZ: W %d, H %d, BINSZ: W %d, H %d",
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


    //------------------------------------------- Get parameters -----------------------------------------
    //===============
    // Get parameters
    //===============
    m_sAFParam = getAFParam();

    //===============
    // Get data from NVRam
    //===============
    MCUDrv::setLensNvramIdx(m_i4CurrSensorDev);
    MINT32 err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrNVRam);

    if( err!=0)
    {
        CAM_LOGE( "AF-%-15s: AfAlgo NvBufUtil get buf fail!", __FUNCTION__);
    }
    memcpy( &m_sNVRam, m_ptrNVRam, sizeof(NVRAM_LENS_PARA_STRUCT));

    if (0 <= m_eCamScenarioMode && m_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
    {
        memcpy( &m_sNVRam.rAFNVRAM, &m_sNVRam.rSCENARIONVRAM[AFNVRAMMapping[m_eCamScenarioMode]], sizeof(m_sNVRam.rSCENARIONVRAM[AFNVRAMMapping[m_eCamScenarioMode]]));
        CAM_LOGD( "AF-%-15s: Scenario mode %d", __FUNCTION__, m_eCamScenarioMode);
    }

    CAM_LOGD_IF( m_i4DgbLogLv, "AF-%-15s: [nvram][Version] %d %d",
                 __FUNCTION__,
                 m_sNVRam.Version,
                 m_ptrNVRam->Version);

    CAM_LOGD( "AF-%-15s: [nvram][Size] %d %d %d %d %d %d %d %d",
              __FUNCTION__,
              MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE,
              sizeof(UINT32),
              sizeof(FOCUS_RANGE_T),
              sizeof(AF_NVRAM_T),
              sizeof(PD_NVRAM_T),
              sizeof(DUALCAM_NVRAM_T),
              sizeof(m_sNVRam.rSCENARIONVRAM),
              MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(FOCUS_RANGE_T)-sizeof(AF_NVRAM_T)-sizeof(PD_NVRAM_T)-sizeof(DUALCAM_NVRAM_T)-sizeof(m_sNVRam.rSCENARIONVRAM));
    //CAM_LOGD("[nvram][THRES_MAIN]%d", m_sNVRam.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    //CAM_LOGD("[nvram][SUB_MAIN]%d", m_sNVRam.rAFNVRAM.sAF_Coef.i4THRES_SUB);
    CAM_LOGD_IF( m_i4DgbLogLv, "AF-%-15s: [nvram][Normal Num]%d [Macro Num]%d",
                 __FUNCTION__,
                 m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum,
                 m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);
    //CAM_LOGD("[nvram][VAFC Fail Cnt]%d", m_sNVRam.rAFNVRAM.i4VAFC_FAIL_CNT);
    //CAM_LOGD("[nvram][LV thres]%d", m_sNVRam.rAFNVRAM.i4LV_THRES);
    //CAM_LOGD("[nvram][PercentW]%d [PercentH]%d", m_sNVRam.rAFNVRAM.i4SPOT_PERCENT_W, m_sNVRam.rAFNVRAM.i4SPOT_PERCENT_H);
    //CAM_LOGD("[nvram][AFC step]%d", m_sNVRam.rAFNVRAM.i4AFC_STEP_SIZE);
    //CAM_LOGD("[nvram][InfPos]%d", m_sNVRam.rFocusRange.i4InfPos);
    //CAM_LOGD("[nvram][MacroPos]%d", m_sNVRam.rFocusRange.i4MacroPos);
    CAM_LOGD_IF( m_i4DgbLogLv, "AF-%-15s: [Param][Normal min step]%d [Macro min step]%d",
                 __FUNCTION__,
                 m_sNVRam.rAFNVRAM.i4AFS_STEP_MIN_NORMAL,
                 m_sNVRam.rAFNVRAM.i4AFS_STEP_MIN_MACRO);



#ifdef AFEXTENDCOEF
    //===============
    // Get extend parameters
    //===============
    CustAF_THRES_T sAFExtCoef = getAFExtendCoef( m_i4TGSzW, m_i4TGSzH, m_i4CurrSensorId);
    if( sAFExtCoef.i4ISONum)
    {
        memcpy( &m_sNVRam.rAFNVRAM.sREV2_AF_TH, &sAFExtCoef, sizeof(sAFExtCoef));
        m_sAFInput.i4IsRevMode2 = MTRUE;
    }
    else
    {
        m_sAFInput.i4IsRevMode2 = MFALSE;
    }
    CAM_LOGD_IF( m_i4DgbLogLv, "AF-%-15s: AFExtCoef %d",
                 __FUNCTION__,
                 m_sAFInput.i4IsRevMode2);
#endif

    //------------------------------------------- Set Depth AF Info ----------------------------------------
    if( m_i4CurrLensId != 0xffff)
    {
        MINT32 AFtbl_Num;
        MINT32 AFtbl_Marco;
        MINT32 AFtbl_Inf;

        m_bDAFEn = (m_sNVRam.rAFNVRAM.i4SDAFCoefs[1]>0) ? MTRUE : MFALSE;

        if( (m_sAFInput.i4IsVDO==TRUE) && (m_eLIB3A_AFMode==LIB3A_AF_MODE_AFC_VIDEO) )
        {
            AFtbl_Num   = m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4NormalNum;
            AFtbl_Inf   = m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset + m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos[0];
            AFtbl_Marco = m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset + m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos[AFtbl_Num-1];
        }
        else
        {
            AFtbl_Num   = m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
            AFtbl_Inf   = m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
            AFtbl_Marco = m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[AFtbl_Num-1];
        }

        m_sDAF_TBL.af_dac_min = AFtbl_Inf;
        m_sDAF_TBL.af_dac_max = AFtbl_Marco;
    }

    //-------------------------------------------- init PD mgr ---------------------------------------------
    //===============
    // pd profile for getting pd_mgr
    //===============
    SPDProfile_t pdprofile;
    pdprofile.i4CurrSensorId = m_i4CurrSensorId;
    pdprofile.u4IsZSD        = isZSD;
    m_i4PDSensorType         = (SensorType_t)rSensorStaticInfo.PDAF_Support; /*0: NO PDAF, 1: Raw, 2: VC(Full), 3: VC(Binning), 4: DualPD Raw, 5: DualPD VC*/
    pdprofile.uSensorType        = m_i4PDSensorType;
    pdprofile.uImgXsz            = m_i4TGSzW;
    pdprofile.uImgYsz            = m_i4TGSzH;
    pdprofile.uFullXsz           = (MINT32)rSensorStaticInfo.captureWidth;
    pdprofile.uFullYsz           = (MINT32)rSensorStaticInfo.captureHeight;
    pdprofile.u4IsFrontalBinning = ((m_i4TGSzW==m_i4BINSzW) && (m_i4TGSzH==m_i4BINSzH)) ? 0 : 1;
    pdprofile.bEnablePBIN        = m_bEnablePBIN;
    pdprofile.uPdSeparateMode    = m_u1PdSeparateMode;
    pdprofile.i4SensorMode       = m_i4SensorMode;
    pdprofile.AETargetMode       = m_AETargetMode;

    CAM_LOGD( "AF-%-15s: PD sensor type %d, ImgSz(%d, %d), FullSz(%d, %d), IsZSD %d",
              __FUNCTION__,
              m_i4PDSensorType,
              pdprofile.uImgXsz,
              pdprofile.uImgYsz,
              pdprofile.uFullXsz,
              pdprofile.uFullYsz,
              isZSD);

    //get pd_mgr
    //m_bPdSupport & m_PDPipeCtrl set by getPdInfoForSttCtrl() before af_mgr::start()
    m_PDBuf_Type = m_bPdSupport ?
                   IPDMgr::getInstance().start( m_i4CurrSensorDev, m_i4SensorIdx, &pdprofile, m_PDPipeCtrl) :
                   EPDBuf_NotDef;

    if( m_PDBuf_Type==EPDBuf_NotDef)
    {
        m_i4PDSensorType = SensorType_NO_PDAF;
        CAM_LOGD( "AF-%-15s: PD buf type isn't defined", __FUNCTION__);
    }

    //===============
    // initial pd_mgr
    //===============
    if( m_i4PDSensorType != SensorType_NO_PDAF)
    {
        MRESULT ret=E_3A_ERR;

        //Get PD information from PD mgr and set  to Hybird AF.
        MINT32  PDInfoSz=0;
        MINT32 *PDInfo=NULL;

        NvBufUtil::getInstance().getBufAndReadNoDefault( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev, (void*&)m_ptrAfPara);
        if( m_ptrAfPara->rPdCaliData.i4Size==0)
        {
            CAM_LOGD( "AF-%-15s: NVRAM NO PDAF calib data, read from EEPROM !!", __FUNCTION__);
            readOTP(CAMERA_CAM_CAL_DATA_PDAF);
            NvBufUtil::getInstance().write( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev);
        }
        else
        {
            CAM_LOGD( "AF-%-15s: NVRAM Have PDAF calib data %d",
                      __FUNCTION__,
                      m_ptrAfPara->rPdCaliData.i4Size);
        }

        //set pd calibration data to PD_mgr.
        ret = IPDMgr::getInstance().setPDCaliData(
                  m_i4CurrSensorDev,
                  reinterpret_cast<MVOID *>(&m_sNVRam.rPDNVRAM.rTuningData),
                  reinterpret_cast<MVOID *>(&m_ptrAfPara->rPdCaliData),
                  PDInfoSz,
                  &PDInfo);

        //PD information to hybrid AF
        if( ret==S_3A_OK)
        {
            memcpy( m_sAFInput.i4PDInfo, PDInfo, PDInfoSz);
            CAM_LOGD( "AF-%-15s: PD info (Sz=%d) to Hybrid AF after PD_mgr initailed %d %d",
                      __FUNCTION__,
                      PDInfoSz,
                      m_sAFInput.i4PDInfo[0],
                      m_sAFInput.i4PDInfo[1]);
        }
        else
        {
            CAM_LOGD( "AF-%-15s: Set PD calibration to PD mgr fail, pd buffer type (0x%x)",
                      __FUNCTION__,
                      m_PDBuf_Type);
            m_i4PDSensorType = SensorType_NO_PDAF;
            m_PDBuf_Type = EPDBuf_NotDef;
        }

        SPDLibVersion_t PdLibVersion;
        IPDMgr::getInstance().GetVersionOfPdafLibrary( m_i4CurrSensorDev, PdLibVersion);

        if( property_get_int32("vendor.debug.dump_pdaf_cali.enable", 0) == 1)
        {
            FILE *fp;
            fp = fopen("/sdcard/pdaf_calibration_data.bin","wb");
            fwrite((void *)m_sNVRam.rPDNVRAM.rCaliData.uData, sizeof(char), m_sNVRam.rPDNVRAM.rCaliData.i4Size, fp);
            fclose(fp);
        }
    }
    else if( m_bPDVCTest)
    {
        //for engineer checking DMA data only.
        CAM_LOGD( "AF-%-15s: Debug virtual channel only : for engineer testing only!!", __FUNCTION__);
    }

    //config pdo hw: configHW need to be called after start and setPDCaliData for m_sPDOHWInfo should be set already
    //need to config PDO HW once when uPdPipeCtrl = 1 or system crash
    //no matter if m_pPDBufMgrCore or m_pIPdAlgo is NULL
    if (m_PDPipeCtrl == FEATURE_PDAF_SUPPORT_BNR_PDO || m_PDPipeCtrl == FEATURE_PDAF_SUPPORT_PBN_PDO)
        IPDMgr::getInstance().configPdoHW(m_i4CurrSensorDev);

    CAM_LOGD( "AF-%-15s: PD Sensor support mode %d\n", __FUNCTION__, m_i4PDSensorType);

    //-------------------------------------------------- init laser driver  ----------------------------------------------------
    if(m_bLaserAFEn == MTRUE)
    {
        if( property_get_int32("vendor.laser.calib.disable", 0) == 0 )
        {
            MUINT32 OffsetData = (MUINT32)m_sNVRam.rAFNVRAM.i4Revs[201];
            MUINT32 XTalkData  = (MUINT32)m_sNVRam.rAFNVRAM.i4Revs[202];
            ILaserMgr::getInstance().setLaserCalibrationData(m_i4CurrSensorDev, OffsetData, XTalkData);
        }

        MUINT32 LaserMaxDistance = (MUINT32)m_sNVRam.rAFNVRAM.i4Revs[203];
        MUINT32 LaserTableNum    = (MUINT32)m_sNVRam.rAFNVRAM.i4Revs[204];
        ILaserMgr::getInstance().setLaserGoldenTable(m_i4CurrSensorDev, (MUINT32*)&m_sNVRam.rAFNVRAM.i4Revs[205], LaserTableNum, LaserMaxDistance);

        m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;

        if( ILaserMgr::getInstance().checkHwSetting(m_i4CurrSensorDev)==0 || property_get_int32("vendor.laser.disable", 0)==1)
        {
            ILaserMgr::getInstance().uninit(m_i4CurrSensorDev);
            m_bLaserAFEn = MFALSE;
            CAM_LOGE( "AF-%-15s: ILaserMgr checkHwSetting() fail", __FUNCTION__);
        }
    }

    //------------------------------------------- initial Hybrid AF algorithm ----------------------------------
    if (m_i4SensorMode == SENSOR_SCENARIO_ID_NORMAL_VIDEO || m_i4SensorMode == SENSOR_SCENARIO_ID_CUSTOM2)
    {
        m_sAFInput.i4IsVDO = TRUE;
        m_sAFInput.i4IsZSD = FALSE;
    }
    else
    {
        m_sAFInput.i4IsVDO = FALSE;
    }

    if( m_sAFParam.i4AFS_MODE==2)  //AF auto mode always don't AFC
    {
        m_sAFInput.i4IsVDO= FALSE;
    }
    // Currently, multi zone is supported as PDAF is on.
    m_i4DbgMZEn = property_get_int32("vendor.af.mzaf.enable", 0);
    m_bMZEn = m_i4DbgMZEn||(m_bMZHostEn==1) ? MTRUE : MFALSE;
    m_sAFInput.i4IsMZ = m_bMZEn;
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

    // set parameters to hybrid AF
    m_pIAfAlgo->setAFParam( m_sAFParam, getAFConfig(), m_sNVRam.rAFNVRAM, m_sNVRam.rDualCamNVRAM);

    // input TG and HW coordinate to hybridAF
    m_sAFInput.sTGSz = AF_COORDINATE_T( m_i4TGSzW,  m_i4TGSzH);
    m_sAFInput.sHWSz = AF_COORDINATE_T( m_i4BINSzW, m_i4BINSzH);

    // set firstCAF flag to hybrid AF
    m_sAFInput.i4IsEnterCam = m_bFirstInitAF;

    // set AdvFSRepeatTime to AFInput
    m_sAFInput.i2AdvFSRepeat = m_aAdvFSRepeatTime;

    // Ext mode setup : the platform check whether the ext mode is support or not
#if defined(MTKCAM_AF_EXTMODE_SUPPORT)
    m_sAFInput.i4IsExtStatMode = 0;
#endif

#if defined(MTKCAM_CMDQ_SUPPORT)
    m_sAFInput.i4IsSupportN2Frame = 0;
#endif

    // initial hybrid AF algorithm
    m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

    // get sensor info for AF algorithm
    if( pIHalSensorInfo == NULL)
    {
        IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
        pIHalSensorInfo = pIHalSensorList->createSensor( "AfAlgo-af_mgr", m_i4SensorIdx);
    }

    //------------------------------------------- Configure AF HW -----------------------------------------
    // HW setting is got after calling initAF.
    ConfigHWReg( m_sAFOutput.sAFStatConfig, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);

    //------------------------------------------- Control Hybrid AF -----------------------------------------
    // set mode to AF algo to start AF state machine
    m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);
    m_eAFStatePre = m_eAFState;

    //------------------------------------------- init af_mgr flow control ------------------------------------
    m_i4IsMonitorFV_PreState  = m_i4IsMonitorFV_CurState  = m_sAFOutput.i4IsMonitorFV;
    m_i4IsAFDone_CurState     = m_i4IsAFDone_PreState     = m_sAFOutput.i4IsAFDone;
    m_i4IsSearchDone_PreState = m_i4IsSearchDone_CurState = m_sAFOutput.i4IsMonitorFV;
    m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState = m_sAFOutput.i4IsSelHWROI;

    //---------------------------------------------------------------------------------------------------------
    if( m_bLock==MTRUE)
    {
        m_pIAfAlgo->cancel();
        m_pIAfAlgo->lock();
        CAM_LOGD( "AF-%-15s: LockAF", __FUNCTION__);
        m_bLock     = MTRUE;
        m_bNeedLock = MFALSE;
    }

    m_sAFInput.i4FullScanStep = m_i4FullScanStep;
    m_bFirstInitAF = MFALSE;

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::AFThreadStart()
{
    //---------------------------------------- StartPreview speed up -----------------
    CAM_LOGD( "AF-%-15s: + Dev %d", __FUNCTION__, m_i4CurrSensorDev);

    // Get AF calibration data. Should do this after setAFParam is called.
    if( m_sNVRam.rAFNVRAM.i4ReadOTP==TRUE)
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
        if( !m_pMcuDrv)
        {
            CAM_LOGE( "AF-%-15s: McuDrv::createInstance fail", __FUNCTION__);
            m_i4EnableAF = 0;
        }

        if( m_pMcuDrv->init(m_i4CurrSensorDev) < 0)
        {
            CAM_LOGE( "AF-%-15s: m_pMcuDrv->init() fail", __FUNCTION__);
            m_i4EnableAF = 0;
        }
        else
        {
            m_i8MvLensTS = MoveLensTo( m_i4InitPos, AF_MGR_CALLER);

            m_i4DbgOISDisable = property_get_int32("vendor.debug.af_ois.disable", 0);
            m_pMcuDrv->setMCUParam(0x01, m_i4DbgOISDisable, m_i4CurrSensorDev);

            getLensInfo(m_sCurLensInfo);
        }

        CAM_LOGD( "AF-%-15s: initMCU -", __FUNCTION__);
    }

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::Stop()
{
    Mutex::Autolock lock( m_Lock);

    {
        Mutex::Autolock lock( m_AFBufLock);
        m_bIsDoAF = MTRUE;
    }

    //uninitial isp_mgr_af_stat for configure HW
    IspMgrAFStatUninit();

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
    m_i4PDSensorType = SensorType_NO_PDAF;
    IPDMgr::getInstance().stop( m_i4CurrSensorDev);

    if( pIHalSensorInfo)
    {
        pIHalSensorInfo->destroyInstance( "AfAlgo-af_mgr");
        pIHalSensorInfo = NULL;
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
    m_eAFStatePre   = m_eAFState = NS3A::E_AF_INACTIVE;

    m_bLock          = MFALSE;
    m_bNeedLock      = MFALSE;

    m_bLatchROI           = MFALSE;
    m_bWaitForceTrigger   = MFALSE;
    m_bForceTrigger       = MFALSE;
    m_bTriggerCmdVlid     = MFALSE;
    m_bCheckStateByAFDone = MFALSE;
    m_bMgrTriggerCmd      = MFALSE;
    m_bFirstInitAF        = MTRUE;

    m_i4MFPos           = -1;
    m_i4InitPos         = 0;

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

    if (m_bLaserAFEn == MTRUE)
    {
        CAM_LOGD( "[%s] ILaserMgr uninit() done", __FUNCTION__);
        ILaserMgr::getInstance().uninit(m_i4CurrSensorDev);
        m_bLaserAFEn = MFALSE;
    }

    CAM_LOGD( "AF-%-15s: -", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTMCUNameinit( MINT32 i4SensorIdx)
{
    CAM_LOGD( "%s +", __FUNCTION__);
    Mutex::Autolock lock( m_Lock);

    if( m_CCTUsers > 0)
    {
        CAM_LOGD( "[CCTMCUNameinit] no init, %d has created", m_CCTUsers);
        android_atomic_inc( &m_CCTUsers);
        return S_3A_OK;
    }
    android_atomic_inc(&m_CCTUsers);
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    switch( m_i4CurrSensorDev)
    {
    case ESensorDev_Main:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;
    case ESensorDev_Sub:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;
    case ESensorDev_MainSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;
    default:
        CAM_LOGE( "Invalid sensor device: %d", m_i4CurrSensorDev);
        break;
    }
    m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
    MCUDrv::lensSearch( m_i4CurrSensorDev, m_i4CurrSensorId);
    m_i4CurrLensId   = MCUDrv::getCurrLensID( m_i4CurrSensorDev);
    CAM_LOGD( "[lens][SensorDev]0x%04x, [SensorId]0x%04x, [CurrLensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

    if( m_i4CurrLensId==0xFFFF) m_i4EnableAF = 0;
    else                        m_i4EnableAF = 1;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrNVRam);
    if(err!=0)
    {
        CAM_LOGE( "AfAlgo NvBufUtil get buf fail!");
    }


    CAM_LOGD( "%s -", __FUNCTION__);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTMCUNameuninit()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "%s +", __FUNCTION__);
    Mutex::Autolock lock( m_Lock);

    if( m_CCTUsers<=0)
    {
        return S_3A_OK;
    }

    android_atomic_dec( &m_CCTUsers);
    if( m_CCTUsers!=0)
    {
        CAM_LOGD( "[CCTMCUNameuninit] Still %d users", m_CCTUsers);
        return S_AF_OK;
    }

    m_i4EnableAF = -1;
    CAM_LOGD( "%s - %d %d", __FUNCTION__, m_i4EnableAF, m_CCTUsers);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFOpeartion()
{
    CAM_LOGD( "%s", __FUNCTION__);
    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    triggerAF( AF_MGR_CALLER);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPMFOpeartion( MINT32 a_i4MFpos)
{
    MINT32 i4TimeOutCnt = 0;

    CAM_LOGD( "%s %d", __FUNCTION__, a_i4MFpos);

    setAFMode( MTK_CONTROL_AF_MODE_OFF, AF_MGR_CALLER);
    triggerAF( AF_MGR_CALLER);
    setMFPos(  a_i4MFpos, AF_MGR_CALLER);

    while( !isFocusFinish())
    {
        usleep( 5000); // 5ms
        i4TimeOutCnt++;
        if( i4TimeOutCnt>100)
        {
            break;
        }
    }

    //[TODO]:CAM_LOGD("[MF]pos:%d, value:%lld\n", a_i4MFpos, m_sAFInput.sAFStat.i8Stat24);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPCheckAutoFocusDone()
{
    return (m_sAFOutput.i4IsAFDone) ? 1 : 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetAFInfo( MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    ACDK_AF_INFO_T *pAFInfo = (ACDK_AF_INFO_T *)a_pAFInfo;

    CAM_LOGD_IF( m_i4DgbLogLv, "%s", __FUNCTION__);

    pAFInfo->i4AFMode  = m_eLIB3A_AFMode;
    pAFInfo->i4AFMeter = LIB3A_AF_METER_SPOT;
    pAFInfo->i4CurrPos = m_sAFOutput.i4AFPos;

    *a_pOutLen = sizeof( ACDK_AF_INFO_T);

    CAM_LOGD( "[AF Mode] = %d", pAFInfo->i4AFMode);
    CAM_LOGD( "[AF Meter] = %d", pAFInfo->i4AFMeter);
    CAM_LOGD( "[AF Current Pos] = %d", pAFInfo->i4CurrPos);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetBestPos( MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s %d", __FUNCTION__, m_sAFOutput.i4AFBestPos);
    *a_pAFBestPos = m_sAFOutput.i4AFBestPos;
    *a_pOutLen    = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFCaliOperation( MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    ACDK_AF_CALI_DATA_T *pAFCaliData = (ACDK_AF_CALI_DATA_T *)a_pAFCaliData;
    AF_DEBUG_INFO_T rAFDebugInfo;
    MUINT32 aaaDebugSize;
    MINT32 i4TimeOutCnt = 0;

    CAM_LOGD( "%s", __FUNCTION__);

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    usleep( 500000);    // 500ms
    m_eLIB3A_AFMode = LIB3A_AF_MODE_CALIBRATION;
    m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);

    usleep( 500000);    // 500ms
    while( !isFocusFinish())
    {
        usleep( 30000); // 30ms
        i4TimeOutCnt++;
        if( i4TimeOutCnt>2000)
        {
            break;
        }
    }

    getDebugInfo( rAFDebugInfo);
    pAFCaliData->i4Gap = (MINT32)rAFDebugInfo.Tag[3].u4FieldValue;

    for( MINT32 i=0; i<512; i++)
    {
        if( rAFDebugInfo.Tag[i+4].u4FieldValue != 0)
        {
            pAFCaliData->i8Vlu[i] = (MINT64)rAFDebugInfo.Tag[i+4].u4FieldValue;  // need fix it
            pAFCaliData->i4Num = i+1;
        }
        else
        {
            break;
        }
    }

    pAFCaliData->i4BestPos = m_sAFOutput.i4AFBestPos;

    CAM_LOGD( "[AFCaliData] Num = %d", pAFCaliData->i4Num);
    CAM_LOGD( "[AFCaliData] Gap = %d", pAFCaliData->i4Gap);

    for( MINT32 i=0; i<pAFCaliData->i4Num; i++)
    {
        CAM_LOGD( "[AFCaliData] Vlu %d = %lld", i, pAFCaliData->i8Vlu[i]);
    }

    CAM_LOGD( "[AFCaliData] Pos = %d", pAFCaliData->i4BestPos);

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    *a_pOutLen = sizeof(MINT32);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSetFocusRange( MVOID *a_pFocusRange)
{
    CAM_LOGD( "%s", __FUNCTION__);
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    m_sNVRam.rFocusRange = *pFocusRange;

    if( m_pMcuDrv)
    {
        m_pMcuDrv->setMCUInfPos( m_sNVRam.rFocusRange.i4InfPos,m_i4CurrSensorDev);
        m_pMcuDrv->setMCUMacroPos( m_sNVRam.rFocusRange.i4MacroPos,m_i4CurrSensorDev);
    }

    if( m_pIAfAlgo)
    {
        m_pIAfAlgo->setAFParam( m_sAFParam, getAFConfig(), m_sNVRam.rAFNVRAM, m_sNVRam.rDualCamNVRAM);
    }

    CAM_LOGD( "[Inf Pos] = %d", m_sNVRam.rFocusRange.i4InfPos);
    CAM_LOGD( "[Marco Pos] = %d", m_sNVRam.rFocusRange.i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFocusRange( MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s", __FUNCTION__);
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    *pFocusRange = m_sNVRam.rFocusRange;
    *a_pOutLen   = sizeof(FOCUS_RANGE_T);

    CAM_LOGD( "[Inf Pos] = %d",   pFocusRange->i4InfPos);
    CAM_LOGD( "[Marco Pos] = %d", pFocusRange->i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s", __FUNCTION__);

    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = reinterpret_cast<NVRAM_LENS_PARA_STRUCT*>(a_pAFNVRAM);

    /**
     * The last parameters 1 :
     * force read NVRam data from EMMC
     */
    MINT32 err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrNVRam, 1);

    if( err!=0)
    {
        CAM_LOGE( "CCTOPAFGetNVRAMParam NvBufUtil get buf fail!");
    }

    memcpy( &m_sNVRam, m_ptrNVRam, sizeof(NVRAM_LENS_PARA_STRUCT));

    *pAFNVRAM  = m_sNVRam;
    *a_pOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFApplyNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode)
{
    CAM_LOGD( "%s", __FUNCTION__);

    NVRAM_LENS_PARA_STRUCT *ptrNewNVRAM = (NVRAM_LENS_PARA_STRUCT *)a_pAFNVRAM;

    memcpy( &m_sNVRam, ptrNewNVRAM, sizeof(NVRAM_LENS_PARA_STRUCT));

    if (0 <= m_eCamScenarioMode && m_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
    {
        memcpy( &m_sNVRam.rSCENARIONVRAM[u4CamScenarioMode], &m_sNVRam.rAFNVRAM, sizeof(m_sNVRam.rSCENARIONVRAM[u4CamScenarioMode]));
        CAM_LOGD( "Apply to Phone[Scenario mode] %d", __FUNCTION__, u4CamScenarioMode);
    }

    CAM_LOGD( "Apply to Phone[Thres Main] %d", m_sNVRam.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    CAM_LOGD( "Apply to Phone[Thres Sub] %d",  m_sNVRam.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    CAM_LOGD( "Apply to Phone[HW_TH] %d",      m_sNVRam.rAFNVRAM.sAF_TH.i4HW_TH[0]);
    CAM_LOGD( "Apply to Phone[Statgain] %d",   m_sNVRam.rAFNVRAM.i4StatGain);

    if( m_pIAfAlgo)
    {
        m_pIAfAlgo->setAFParam( m_sAFParam, getAFConfig(), m_sNVRam.rAFNVRAM, m_sNVRam.rDualCamNVRAM);
    }
    if( m_pMcuDrv)
    {
        m_pMcuDrv->setMCUInfPos( m_sNVRam.rFocusRange.i4InfPos,m_i4CurrSensorDev);
        m_pMcuDrv->setMCUMacroPos( m_sNVRam.rFocusRange.i4MacroPos,m_i4CurrSensorDev);
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSaveNVRAMParam()
{
    CAM_LOGD( "%s", __FUNCTION__);

    CAM_LOGD( "WriteNVRAM from Phone[Thres Main]%d\n", m_sNVRam.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    CAM_LOGD( "WriteNVRAM from Phone[Thres Sub]%d\n",  m_sNVRam.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    CAM_LOGD( "WriteNVRAM from Phone[HW_TH]%d\n",      m_sNVRam.rAFNVRAM.sAF_TH.i4HW_TH[0]);
    CAM_LOGD( "WriteNVRAM from Phone[Statgain]%d\n",   m_sNVRam.rAFNVRAM.i4StatGain);

    NVRAM_LENS_PARA_STRUCT *ptrNVRAMBuf;

    MINT32 err1 = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)ptrNVRAMBuf);

    memcpy( ptrNVRAMBuf, &m_sNVRam, sizeof(NVRAM_LENS_PARA_STRUCT));

    MINT32 err2 = NvBufUtil::getInstance().write( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev);

    if( err1!=0)
        return err1;
    else
        return err2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFV( MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    ACDK_AF_POS_T *pAFPos   = (ACDK_AF_POS_T *) a_pAFPosIn;
    ACDK_AF_VLU_T *pAFValue = (ACDK_AF_VLU_T *) a_pAFValueOut;

    CAM_LOGD( "%s", __FUNCTION__);

    pAFValue->i4Num = pAFPos->i4Num;
    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    usleep( 500000); // 500ms
    setAFMode( MTK_CONTROL_AF_MODE_OFF, AF_MGR_CALLER);

    for( MINT32 i=0; i<pAFValue->i4Num; i++)
    {
        setMFPos( pAFPos->i4Pos[i], AF_MGR_CALLER);
        usleep( 500000); // 500ms
        pAFValue->i8Vlu[i] = TransStatProfileToAlgo( m_sAFInput.sStatProfile);
        CAM_LOGD( "[FV]pos = %d, value = %lld\n", pAFPos->i4Pos[i], pAFValue->i8Vlu[i]);
    }

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    *a_pOutLen = sizeof( ACDK_AF_VLU_T);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFEnable()
{
    CAM_LOGD( "%s", __FUNCTION__);
    m_i4EnableAF = 1;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFDisable()
{
    CAM_LOGD( "%s", __FUNCTION__);
    m_i4EnableAF = 0;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetEnableInfo( MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s %d", __FUNCTION__, m_i4EnableAF);

    MINT32 *pEnableAF = (MINT32 *)a_pEnableAF;
    *pEnableAF = m_i4EnableAF;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTOPAFSetAfArea(MUINT32 a_iPercent)
{
    CAM_LOGD( "%s %d", __FUNCTION__, a_iPercent);

    MINT32 AfAreaW = (MINT32)(m_i4TGSzW*a_iPercent/100);
    MINT32 AfAreaH = (MINT32)(m_i4TGSzH*a_iPercent/100);
    MINT32 AfAreaX = (MINT32)((m_i4TGSzW - AfAreaW)/2);
    MINT32 AfAreaY = (MINT32)((m_i4TGSzH - AfAreaH)/2);

    CameraFocusArea_T area;
    area.u4Count = 1;
    area.rAreas[0].i4Left = AfAreaX;
    area.rAreas[0].i4Top = AfAreaY;
    area.rAreas[0].i4Right = AfAreaX+AfAreaW;
    area.rAreas[0].i4Bottom = AfAreaY+AfAreaH;
    area.rAreas[0].i4Weight = 1;

    setAFArea(area);

    CAM_LOGD( "TG size(%d,%d), Af Area(l,t,r,b)=(%d,%d,%d,%d) (w,h)=(%d,%d)\n",
              m_i4TGSzW,m_i4TGSzH,
              area.rAreas[0].i4Left,
              area.rAreas[0].i4Top,
              area.rAreas[0].i4Right,
              area.rAreas[0].i4Bottom,
              AfAreaW,AfAreaH);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetPauseAF( MBOOL &bIsPause)
{
    if( m_bPauseAF!=bIsPause)
    {
        if( bIsPause==MTRUE)
        {
            CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d (PAUSE)",
                      m_u4ReqMagicNum,
                      m_u4HalMagicNum,
                      __FUNCTION__,
                      m_bPauseAF,
                      bIsPause);
        }
        else
        {
            CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d (CONTINUE)",
                      m_u4ReqMagicNum,
                      m_u4HalMagicNum,
                      __FUNCTION__,
                      m_bPauseAF,
                      bIsPause);
        }
    }
    m_bPauseAF = bIsPause;

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::triggerAF( MUINT32 u4Caller)
{
    if( u4Caller==AF_MGR_CALLER)
        CAM_LOGD( "%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);
    else
        CAM_LOGD( "cmd-%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    UpdateState( EVENT_CMD_TRIGGERAF_WITH_AE_STBL);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WaitTriggerAF( MBOOL &bWait)
{
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
MRESULT AfMgr::setAFMode( MINT32 eAFMode, MUINT32 u4Caller)
{
    if( m_i4EnableAF==0)
    {
        return S_AF_OK;
    }

    if( m_eCurAFMode==eAFMode)
    {
        return S_AF_OK;
    }

    /**
     *  Before new af mode is set, setting af area command is sent.
     */
    if( u4Caller==AF_MGR_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4HalMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  eAFMode);
    }
    else
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4HalMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  eAFMode);
    }

    m_eCurAFMode = eAFMode;

    LIB3A_AF_MODE_T preLib3A_AfMode = m_eLIB3A_AFMode;
    switch( m_eCurAFMode)
    {
    case MTK_CONTROL_AF_MODE_OFF :
        m_eLIB3A_AFMode = LIB3A_AF_MODE_MF; /*API2:The auto-focus routine does not control the lens. Lens  is controlled by the application.*/
        break;
    case MTK_CONTROL_AF_MODE_AUTO :
        if(m_aCurEMAFFlag==0)// Auto Focus for Single shot
        {
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
        }
        else
        {
            // Engineer Mode: Full Scan
            if(m_aCurEMAFFlag == EM_AF_FLAG_FULLSCAN_NORMAL) // Full Scan
            {
                m_eLIB3A_AFMode = LIB3A_AF_MODE_FULLSCAN;
            }
            else if(m_aCurEMAFFlag == (EM_AF_FLAG_FULLSCAN_NORMAL|EM_AF_FLAG_FULLSCAN_ADVANCE))// Advanced Full Scan
            {
                m_eLIB3A_AFMode = LIB3A_AF_MODE_ADVFULLSCAN;
            }
            else if(m_aCurEMAFFlag == (EM_AF_FLAG_FULLSCAN_NORMAL|EM_AF_FLAG_TEMP_CALI))// Temperature Calibration
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

    // log only.
    if( u4Caller==AF_MGR_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d):lib_afmode %d->%d",
                  m_u4ReqMagicNum,
                  m_u4HalMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  preLib3A_AfMode,
                  m_eLIB3A_AFMode);
    }
    else
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d):lib_afmode %d->%d",
                  m_u4ReqMagicNum,
                  m_u4HalMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  preLib3A_AfMode,
                  m_eLIB3A_AFMode);
    }

    //Set mode to hybrid AF algorithm
    if( m_pIAfAlgo)
        m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);
    else
        CAM_LOGD( "Null m_pIAfAlgo");


    //update parameters and status.
    UpdateState( EVENT_CMD_CHANGE_MODE);
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::LockAlgo()
{
    m_pIAfAlgo->cancel();
    m_pIAfAlgo->lock();
    CAM_LOGD( "%s", __FUNCTION__);
    m_bLock = MTRUE;
    return m_bLock;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::UnlockAlgo()
{
    m_pIAfAlgo->unlock();
    CAM_LOGD( "%s", __FUNCTION__);
    m_bLock = MFALSE;
    return m_bLock;
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
        m_bForceTrigger = MTRUE;
        m_bLatchROI     = MTRUE;
        m_i4TAFStatus   = TAF_STATUS_RESET;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if(      eInCurSate == NS3A::E_AF_INACTIVE          ) NewState = NS3A::E_AF_ACTIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_FOCUSED_LOCKED    ) NewState = NS3A::E_AF_ACTIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_NOT_FOCUSED_LOCKED) NewState = NS3A::E_AF_ACTIVE_SCAN;
        else                                                  NewState = eInCurSate; /*Should be at E_AF_ACTIVE_SCAN*/

        m_i4TAFStatus = TAF_STATUS_RECEIVE_AUTOFOCUS;
        break;

    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        m_pIAfAlgo->trigger();
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
        NewState = NS3A::E_AF_ACTIVE_SCAN;
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
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = m_bMgrTriggerCmd = m_bCheckStateByAFDone = MFALSE;

        if( eInCurSate==NS3A::E_AF_INACTIVE)
        {
            NewState = NS3A::E_AF_PASSIVE_UNFOCUSED; // To prevent CTS fail because of the wrong af state while switching mode w/o trigger (original: NS3A::E_AF_INACTIVE)
            if( m_i4TAFStatus == TAF_STATUS_RECEIVE_CANCELAUTOFOCUS)
            {
                CAM_LOGD( "Don't do AF searching after TAF");
            }
            else
            {
                CAM_LOGD( "[%s] Set force trigger searching", __FUNCTION__);
                m_bForceTrigger = m_bTriggerCmdVlid = m_bMgrTriggerCmd = MTRUE;
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
                m_pIAfAlgo->cancel();
                m_pIAfAlgo->trigger();
            }

            if( NewState != NS3A::E_AF_PASSIVE_SCAN)
            {
                LockAlgo();
            }
            else
            {
                CAM_LOGD( "LockAF until searching done");
                m_bNeedLock = MTRUE;
            }
        }
        break;

    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        if( m_bForceTrigger==MTRUE && m_bTriggerCmdVlid==MTRUE)
        {
            NewState = eInCurSate;

            m_bTriggerCmdVlid = MFALSE;
            // force to trigger searching when changing ROI at continuous mode.
            CAM_LOGD( "Force trigger searching [%d %d %d %d]", m_bCheckStateByAFDone, m_bMgrTriggerCmd, m_bLatchROI, m_i4IsAFDone_CurState);
            m_pIAfAlgo->cancel();
            m_pIAfAlgo->trigger();

            // Mgr trigger Algo to search
            if( m_bMgrTriggerCmd == MTRUE)
            {
                // m_bForceTrigger - select ROI from algo
                m_bCheckStateByAFDone = m_bForceTrigger = m_bMgrTriggerCmd = MFALSE;
            }
            else
            {
                m_bCheckStateByAFDone = MTRUE;

                /* AF state by using IsMonitorFV during AF searching */
                if (m_i4IsAFDone_CurState == MFALSE)
                {
                    CAM_LOGD( "During AF seraching");
                    m_bCheckStateByAFDone = MFALSE;
                }
            }
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
            m_pIAfAlgo->cancel();
        }
        else
        {
            NewState = eInCurSate;
        }
        m_bNeedLock = MFALSE;
        break;

    case EVENT_CMD_SET_AF_REGION :
        if( eInCurSate==NS3A::E_AF_PASSIVE_SCAN)
        {
            NewState = NS3A::E_AF_PASSIVE_UNFOCUSED;
            m_pIAfAlgo->cancel();
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
        if(      eInCurSate == NS3A::E_AF_INACTIVE         ) NewState = NS3A::E_AF_PASSIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_PASSIVE_FOCUSED  ) NewState = NS3A::E_AF_PASSIVE_SCAN;
        else if( eInCurSate == NS3A::E_AF_PASSIVE_UNFOCUSED) NewState = NS3A::E_AF_PASSIVE_SCAN;
        else                                                 NewState = eInCurSate;
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;
        CAM_LOGD( "[%d]", m_i4IsSearchDone_CurState);
        if( m_i4IsSearchDone_CurState==MFALSE)
        {
            NewState = eInCurSate;
            break;
        }
    case EVENT_CMD_STOP :
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = m_bCheckStateByAFDone = MFALSE;
    case EVENT_SEARCHING_END :
    {
        Mutex::Autolock lock( m_SearchingEndLock);

        CAM_LOGD( "TriggerCmdValid(%d)/WaitForceTrigger(%d)/bNeedLock(%d)/IsFocused(%d)/CheckStateByAFDone(%d)/eInCurSate(%d)",
                  m_bTriggerCmdVlid,
                  m_bWaitForceTrigger,
                  m_bNeedLock,
                  m_i4IsFocused,
                  m_bCheckStateByAFDone,
                  eInCurSate);
        if( m_bTriggerCmdVlid==MTRUE && m_bWaitForceTrigger==MTRUE)
        {
            NewState = NS3A::E_AF_PASSIVE_SCAN;
        }
        else
        {
            //reset parameters
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = m_bCheckStateByAFDone = MFALSE;

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

    }
    break;

    case EVENT_AE_IS_STABLE :
        if( (m_bForceTrigger==MTRUE && m_bTriggerCmdVlid==MTRUE) && m_bWaitForceTrigger==MFALSE)
        {
            /* Trigger AF searching in af_mgr*/
            triggerAF( AF_MGR_CALLER);
        }
        NewState = eInCurSate;
        break;

    case EVENT_SET_WAIT_FORCE_TRIGGER :
        if( m_eCurAFMode==MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO)
        {
            CAM_LOGW("EVENT_SET_WAIT_FORCE_TRIGGER in MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO");
        }
        else if( eInCurSate!=NS3A::E_AF_FOCUSED_LOCKED &&
                 eInCurSate!=NS3A::E_AF_NOT_FOCUSED_LOCKED)
        {
            m_bLatchROI = m_bTriggerCmdVlid = m_bForceTrigger = m_bWaitForceTrigger = MTRUE;
            CAM_LOGD( "Set wait force trigger for preCap");

            if( m_i4IsSearchDone_CurState==MTRUE)
            {
                LockAlgo();
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
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        m_pIAfAlgo->trigger();
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL :
    case EVENT_CMD_SET_AF_REGION :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_START :
    case EVENT_SEARCHING_END :
    case EVENT_AE_IS_STABLE :
    case EVENT_SET_WAIT_FORCE_TRIGGER :
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
    default :
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
        NewState = NS3A::E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        m_pIAfAlgo->trigger();
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL :
    case EVENT_CMD_SET_AF_REGION :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_START :
    case EVENT_SEARCHING_END :
    case EVENT_AE_IS_STABLE :
    case EVENT_SET_WAIT_FORCE_TRIGGER :
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
    default :
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
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_CHANGE_MODE", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_CHANGE_MODE;
        break;
    case EVENT_CMD_AUTOFOCUS:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_AUTOFOCUS", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_AUTOFOCUS;
        break;
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_TRIGGERAF_WITH_AE_STBL", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_TRIGGERAF_WITH_AE_STBL;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_AUTOFOCUS_CANCEL", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_AUTOFOCUS_CANCEL;
        break;
    case EVENT_CMD_SET_AF_REGION:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_SET_AF_REGION", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_SET_AF_REGION;
        break;
    case EVENT_CMD_STOP:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_STOP", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_STOP;
        break;
    case EVENT_SEARCHING_START:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SEARCHING_START", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_START;
        break;
    case EVENT_SEARCHING_END:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SEARCHING_END", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_END;
        break;
    case EVENT_AE_IS_STABLE:
        //CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_AE_IS_STABLE", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_END;
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SET_WAIT_FORCE_TRIGGER", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SET_WAIT_FORCE_TRIGGER;
        break;
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CANCEL_WAIT_FORCE_TRIGGER", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CANCEL_WAIT_FORCE_TRIGGER;
        break;
    default:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_WRONG", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
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
                  m_u4HalMagicNum,
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

    if( m_u4ReqMagicNum<=AF_START_MAGIC_NUMBER && m_eAFState==NS3A::E_AF_PASSIVE_SCAN)
    {
        /**
         * If AF starts searching at first request.
         * Force to return inactive state as initial state.
         */
        ret = NS3A::E_AF_INACTIVE;
    }
    else
    {
        ret = m_eAFState;
    }


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
MRESULT AfMgr::getFocusAreaResult( android::Vector<MINT32> &vecOutPos, android::Vector<MUINT8> &vecOutRes, MINT32 &i4OutSzW, MINT32 &i4OutSzH)
{
    vecOutPos.clear();
    vecOutRes.clear();
    i4OutSzW = 0;
    i4OutSzH = 0;

    if( m_bMZEn)
    {
        //TODO: check the result from algo.
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
                 "%s %d %d",
                 __FUNCTION__,
                 vecOutPos.size(),
                 vecOutRes.size());

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getFocusArea( android::Vector<MINT32> &vecOut)
{
    vecOut.clear();

    /*
     * The order of vector vecOut is :
     * (type)->(number of ROI)->(left_1)->(top_1)->(right_1)->(bottom_1)->(result_1)->(left_2)->(top_2)->(right_2)->(bottom_2)...
     */


    if( m_bMZEn)
    {
        MINT32 type = 0;
        vecOut.push_back(type);

        MUINT32 num = static_cast<MUINT32>(m_sAFOutput.sROIStatus.i4TotalNum);
        if( (num<0) || (MAX_MULTI_ZONE_WIN_NUM<num))
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
        vecOut.push_back( m_sArea_HW.i4Info);

        CAM_LOGD_IF( m_i4DgbLogLv,
                     "%s X(%4d), Y(%4d), W(%4d), H(%4d), Res(%d)",
                     __FUNCTION__,
                     m_sArea_HW.i4X,
                     m_sArea_HW.i4Y,
                     m_sArea_HW.i4W,
                     m_sArea_HW.i4H,
                     m_sArea_HW.i4Info);
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

    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) cmd-%s : [Cnt]%d [L]%d [R]%d [U]%d [B]%d, ",
                              m_u4ReqMagicNum,
                              m_u4HalMagicNum,
                              __FUNCTION__,
                              sInAFArea.u4Count,
                              sInAFArea.rAreas[0].i4Left,
                              sInAFArea.rAreas[0].i4Right,
                              sInAFArea.rAreas[0].i4Top,
                              sInAFArea.rAreas[0].i4Bottom);
    }

    if( 1<=sInAFArea.u4Count)
    {
        //boundary check.
        if( (sInAFArea.rAreas[0].i4Left<sInAFArea.rAreas[0].i4Right ) &&
                (sInAFArea.rAreas[0].i4Top <sInAFArea.rAreas[0].i4Bottom) &&
                (0<=sInAFArea.rAreas[0].i4Left && sInAFArea.rAreas[0].i4Right<=m_i4TGSzW) &&
                (0<=sInAFArea.rAreas[0].i4Top  && sInAFArea.rAreas[0].i4Bottom<=m_i4TGSzH))
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

                UpdateState( EVENT_CMD_SET_AF_REGION);

                CAM_LOGD( "#(%5d,%5d) cmd-%s Got ROI changed cmd. [Cnt]%d (L,R,U,B)=(%d,%d,%d,%d) => (X,Y,W,H)=(%d,%d,%d,%d)",
                          m_u4ReqMagicNum,
                          m_u4HalMagicNum,
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
                if (m_bLaserAFEn == MTRUE)
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
                if( m_i4DgbLogLv)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cmd is same");
                }
            }
        }
        else
        {
            //command is not vaild, using center window.
            if( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cmd is not correct");
            }
        }
    }
    else
    {
        //command is not vaild, using center window.
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cnt=0!!");
        }
    }

    if( ret==E_3A_ERR)
    {
        CAM_LOGD_IF( m_i4DgbLogLv&2, "%s", dbgMsgBuf);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCamScenarioMode(MUINT32 a_eCamScenarioMode)
{
    if (a_eCamScenarioMode != m_eCamScenarioMode)
    {
        CAM_LOGD("[setCamScenarioMode] %d -> %d", m_eCamScenarioMode, a_eCamScenarioMode);

        if (0 <= a_eCamScenarioMode && a_eCamScenarioMode < AF_CAM_SCENARIO_NUM)
        {
            if (AFNVRAMMapping[a_eCamScenarioMode] != AFNVRAMMapping[m_eCamScenarioMode])
            {
                MUINT32 MappingIdx = AFNVRAMMapping[a_eCamScenarioMode];

                memcpy( &m_sNVRam.rAFNVRAM, &m_sNVRam.rSCENARIONVRAM[MappingIdx], sizeof(m_sNVRam.rSCENARIONVRAM[MappingIdx]));

                if (a_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    m_sAFInput.i4IsVDO = TRUE;
                    m_sAFInput.i4IsZSD = FALSE;
                }
                else
                {
                    m_sAFInput.i4IsVDO = FALSE;
                }
                if( m_sAFParam.i4AFS_MODE==2)  //AF auto mode always don't AFC
                {
                    m_sAFInput.i4IsVDO= FALSE;
                }
                if( m_pIAfAlgo)
                {
                    m_pIAfAlgo->setAFParam( m_sAFParam, m_sAFOutput.sAFStatConfig, m_sNVRam.rAFNVRAM, m_sNVRam.rDualCamNVRAM);
                    // initial hybrid AF algorithm
                    m_pIAfAlgo->initAF( m_sAFInput, m_sAFOutput);

                    if( m_bLock!=MTRUE)
                    {
                        m_pIAfAlgo->cancel();
                    }
                }
                // Get AF calibration data. Should do this after setAFParam is called.
                if( m_sNVRam.rAFNVRAM.i4ReadOTP==TRUE)
                {
                    readOTP(CAMERA_CAM_CAL_DATA_3A_GAIN);
                }
            }
            else
            {
                CAM_LOGD( "[%s] tuning param is the same\n", __FUNCTION__);
            }
        }
        else
        {
            CAM_LOGD( "[%s] Camera Scenario mode invalid\n", __FUNCTION__);
        }

        m_eCamScenarioMode = a_eCamScenarioMode;
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

    if( m_i4IsSearchDone_CurState==MFALSE)
    {
        /* Digital Zoom : skip crop reggion info durning AF seraching */
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
                          m_u4HalMagicNum,
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
                          m_u4HalMagicNum,
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
            m_sArea_APCheck = m_sArea_APCmd = m_sArea_OTFD = m_sArea_HW = m_sArea_Center;
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
    m_i4DzWinCfg = (MUINT32)m_sNVRam.rAFNVRAM.i4ZoomInWinChg;

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
    else if( m_bMZEn==MFALSE)
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
            scaledWin.i4W = sOutAFRegion.i4W*m_i4DZFactor/m_i4DzWinCfg;
            scaledWin.i4H = sOutAFRegion.i4H*m_i4DZFactor/m_i4DzWinCfg;
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
    MRESULT ret=E_3A_ERR;

    /*TG base coordinate*/
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
            {
                //TODO : currently always get the first ROI for focusing.
                // rect => 0:left, 1:top, 2:right, 3:bottom
                MINT32 i4X = ptrWins->faces[0].rect[0];
                MINT32 i4Y = ptrWins->faces[0].rect[1];
                MINT32 i4W = ptrWins->faces[0].rect[2] - ptrWins->faces[0].rect[0];
                MINT32 i4H = ptrWins->faces[0].rect[3] - ptrWins->faces[0].rect[1];
                // scale OTFD ROI
                MINT32 i4scale = m_sNVRam.rAFNVRAM.i4FDWinPercent&0xFFFF;
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

    m_pIAfAlgo->setFDWin( sAreaInfo);
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFDInfo( MVOID *a_sFaces)
{
    MRESULT ret = E_3A_ERR;

    if ( m_pIAfAlgo!=NULL)
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

    if ( m_pIAfAlgo!=NULL)
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
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    SensorStaticInfo rSensorStaticInfo;
    MINT32 isSupportLens;

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
    default:
        CAM_LOGE( "Invalid sensor device: %d", m_i4CurrSensorDev);
    }
    m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;

    isSupportLens = MCUDrv::isSupportLens( m_i4CurrSensorDev, m_i4CurrSensorId);

    if( isSupportLens)
    {
        if( m_i4EnableAF==0)
        {
            CAM_LOGE( "Open AF driver fail!");
            return 0;
        }
        else
        {
            CAM_LOGD( "[getAFMaxAreaNum]%d", AF_WIN_NUM_SPOT);
            return AF_WIN_NUM_SPOT;
        }
    }
    else
    {
        CAM_LOGD( "[getAFMaxAreaNum][AF disable]%d", 0);
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMaxLensPos()
{
    if( m_i4EnableAF==0)
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "[getMaxLensPos]disable AF");
        return 0;
    }
    else
    {
        if( m_pMcuDrv)
        {
            mcuMotorInfo MotorInfo;
            m_pMcuDrv->getMCUInfo( &MotorInfo,m_i4CurrSensorDev);
            CAM_LOGD_IF( m_i4DgbLogLv, "[getMaxLensPos]%d", (MINT32)MotorInfo.u4MacroPosition);
            return (MINT32)MotorInfo.u4MacroPosition;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "[getMaxLensPos]m_pMcuDrv NULL");
            return 1023;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMinLensPos()
{
    if( m_i4EnableAF==0)
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "[getMinLensPos]disable AF");
        return 0;
    }
    else
    {
        if( m_pMcuDrv)
        {
            mcuMotorInfo MotorInfo;
            m_pMcuDrv->getMCUInfo( &MotorInfo,m_i4CurrSensorDev);
            CAM_LOGD_IF( m_i4DgbLogLv, "[getMinLensPos]%d", (MINT32)MotorInfo.u4InfPosition);
            return (MINT32)MotorInfo.u4InfPosition;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "[getMinLensPos]m_pMcuDrv NULL");
            return 0;
        }
    }
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
    return m_sAFOutput.i4IsAFDone;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getLensMoving()
{
    return m_sAFInput.sLensInfo.bIsMotorMoving;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableOffset()
{
    if( m_eLIB3A_AFMode==LIB3A_AF_MODE_AFC_VIDEO)
        return m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset;
    else
        return m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4Offset;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableMacroIdx()
{
    if( m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
        return m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4NormalNum;
    else
        return m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableIdxNum()
{
    return AF_TABLE_NUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* AfMgr::getAFTable()
{
    if( m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
        return (MVOID*)m_sNVRam.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos;

    else
        return (MVOID*)m_sNVRam.rAFNVRAM.sAF_Coef.sTABLE.i4Pos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setMFPos( MINT32 a_i4Pos, MUINT32 u4Caller)
{
    if( u4Caller==AF_MGR_CALLER)
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



    if( (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
            (m_i4MFPos != a_i4Pos) &&
            (0<=a_i4Pos) &&
            (a_i4Pos<=1023))
    {
        if( u4Caller==AF_MGR_CALLER)
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
        if( u4Caller==AF_MGR_CALLER)
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
        if( m_i4FullScanStep>0) /* Step > 0 , set Full Scan Mode */
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

            if(property_get_int32("vendor.mtk.client.em.af_advfs.enable", 0) == 1)
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

        if(m_aCurEMAFFlag != m_aPreEMAFFlag)
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
MVOID AfMgr::setAE2AFInfo( AE2AFInfo_T &rAEInfo)
{

    if (m_i4EnableAF == 0)
    {
        m_sAFInput.i8GSum = 50;
        m_sAFInput.i4ISO = 100;
        m_sAFInput.i4IsAEStable = 1;
        m_sAFInput.i4SceneLV = 80;
        return;
    }

    MINT32 isAEStable = ((rAEInfo.i4IsAEStable==1)||(rAEInfo.i4IsAELocked==1)) ? 1: 0;

    m_sAFInput.i8GSum         = rAEInfo.iYvalue; /* [TODO] :: check should be remove or not*/
    m_sAFInput.i4ISO          = rAEInfo.i4ISO;
    m_sAFInput.i4IsAEStable   = isAEStable;
    m_sAFInput.i4SceneLV      = rAEInfo.i4SceneLV;
    m_sAFInput.i4ShutterValue = rAEInfo.ishutterValue;
    m_sAFInput.i4DeltaBV      = rAEInfo.i4DeltaBV;
    m_sAFInput.i4IsFlashFrm       = rAEInfo.i4IsFlashFrm;
    m_sAFInput.i4AEBlockAreaYCnt  = rAEInfo.i4AEBlockAreaYCnt;
    m_sAFInput.pAEBlockAreaYvalue = rAEInfo.pAEBlockAreaYvalue;
    memcpy( m_aAEBlkVal, rAEInfo.aeBlockV, 25);

    if( isAEStable==1)
    {
        UpdateState( EVENT_AE_IS_STABLE);
    }

    if( m_i4DgbLogLv)
    {
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "#(%5d,%5d) cmd-%s Dev(%d) GSum(%lld) ISO(%d) AEStb(%d,%d) AELock(%d) LV(%d) shutter(%d) flash(%d) DeltaBV(%d) AEBlk(%d %d)",
                     m_u4ReqMagicNum,
                     m_u4HalMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     m_sAFInput.i8GSum,
                     m_sAFInput.i4ISO,
                     m_sAFInput.i4IsAEStable,
                     rAEInfo.i4IsAEStable,
                     rAEInfo.i4IsAELocked,
                     rAEInfo.i4SceneLV,
                     m_sAFInput.i4ShutterValue,
                     m_sAFInput.i4IsFlashFrm,
                     m_sAFInput.i4DeltaBV,
                     m_sAFInput.i4AEBlockAreaYCnt,
                     m_sAFInput.pAEBlockAreaYvalue);

    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAdptCompInfo( const AdptCompTimeData_T &AdptCompTime)
{
    // info from AfMgr
    m_sAFInput.TS_MLStart  = m_i8MvLensTS; // unit: us
    // info from Hal3A
    m_sAFInput.TS_AFDone  = AdptCompTime.TS_AFDone; // unit: us
    // info from SensorInfo
    MINT32 PixelClk=0;
    MINT32 FrameSyncPixelLineNum=0;
    pIHalSensorInfo->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_PIXEL_CLOCK_FREQ, (MINTPTR)&PixelClk, 0,0); // (Hz)
    m_sAFInput.PixelClk = PixelClk; //(Hz)
    pIHalSensorInfo->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MINTPTR)&FrameSyncPixelLineNum, 0,0); // (Pixel)
    m_sAFInput.PixelInLine = 0x0000FFFF & FrameSyncPixelLineNum; //(Pixel)

    // debug
    if(m_i4DgbLogLv)
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d): ", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
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
    CAM_LOGD_IF( m_i4DgbLogLv, "[%s] FrameId:%d, AfeGain:%d, IspGain:%d, hlrEnable:%d",
                 __FUNCTION__, ispSensorInfo.i4FrameId, ispSensorInfo.u4AfeGain, ispSensorInfo.u4IspGain, ispSensorInfo.bHLREnable);

    m_vISQueue.pushHeadAnyway(ispSensorInfo);

    if (m_i4DbgAfegainQueue & 0x1)
    {
        MUINT32 front = m_vISQueue.head;
        MUINT32 end = m_vISQueue.tail;

        CAM_LOGD("--> Head:%d, Tail:%d, FrameId:%d, AfeGain:%d, IspGain:%d, hlrEnable:%d",
                 front, end, ispSensorInfo.i4FrameId, ispSensorInfo.u4AfeGain, ispSensorInfo.u4IspGain, ispSensorInfo.bHLREnable);
        if (end > front)
            front += m_vISQueue.queueSize;
        for (int i=front; i>end; i--)
        {
            MUINT32 idx = i % m_vISQueue.queueSize;
            CAM_LOGD("AfeGain qIdx(%d): frameId(%d), afeGain(%d), ispGain(%d), hlrEnable:%d",
                     idx, m_vISQueue.content[idx].i4FrameId, m_vISQueue.content[idx].u4AfeGain, m_vISQueue.content[idx].u4IspGain, m_vISQueue.content[idx].bHLREnable);
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
    if ((m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE) && (m_bDAFEn == MTRUE))
    {
        HybridAFMode |= 1;
    }

    //PDAF
    if( ((m_i4PDSensorType==SensorType_Pure_Raw)    && (m_PDBuf_Type==EPDBuf_Raw)) ||
            ((m_i4PDSensorType==SensorType_PDAF_Raw)    && (m_PDBuf_Type==EPDBuf_Raw||m_PDBuf_Type==EPDBuf_PDO)) ||
            ((m_i4PDSensorType==SensorType_PDAF_VC_HDR) && (m_PDBuf_Type==EPDBuf_VC)) ||
            ((m_i4PDSensorType==SensorType_DualPD_Raw)  && (m_PDBuf_Type==EPDBuf_DualPD_Raw)) ||
            ((m_i4PDSensorType==SensorType_DualPD_VC)   && (m_PDBuf_Type==EPDBuf_DualPD_VC)))
    {
        HybridAFMode |= 2;  //2'b 0010
    }
    else if( (m_i4PDSensorType==SensorType_PDAF_VC_HDR) && (m_PDBuf_Type==EPDBuf_VC_Open))
    {
        HybridAFMode |= 10; //2'b 1010
    }

    //Laser AF
    if (m_bLaserAFEn == MTRUE)
    {
        HybridAFMode |= 4;
    }

    CAM_LOGD_IF( m_i4DgbLogLv&4, "%s:(0x%x)", __FUNCTION__, HybridAFMode);

    return HybridAFMode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::HybridAFPreprocessing()
{
    MINT32 EnablePD = ( ( m_PDBuf_Type!= EPDBuf_NotDef ) && ( m_i4PDSensorType != SensorType_NO_PDAF ) );

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d) PDEn(%d) LaserEn(%d) FDDetect(%d): ",
                              m_u4ReqMagicNum,
                              m_u4HalMagicNum,
                              __FUNCTION__,
                              m_i4CurrSensorDev,
                              EnablePD,
                              m_bLaserAFEn,
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
        MINT32   PDOut_numRes = m_i4PDResNum;
        MUINT16* pPDAF_DAC    = m_sAFInput.i4PDafDacIndex;
        MUINT16* pPDAF_Conf   = m_sAFInput.i4PDafConfidence;

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[PD (DAC, CL)] org{ ");
            for( MINT32 i=0; i<PDOut_numRes; i++)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC[i], pPDAF_Conf[i]);
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
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC[i], pPDAF_Conf[i]);
            }
            ptrMsgBuf += sprintf( ptrMsgBuf, "}, ");
        }
    }


    /* do laser data preprocessing */
    if (m_bLaserAFEn == MTRUE)
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
            switch(LaserStatus)
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
            if ((m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO))
            {
                if( m_sAFInput.sLaserInfo.i4AfWinPosX >= LASER_TOUCH_REGION_W ||
                        m_sAFInput.sLaserInfo.i4AfWinPosY >= LASER_TOUCH_REGION_H ||
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
            (m_bForceTrigger==MTRUE && m_bLatchROI==MTRUE) ||
            (m_i4DgbLogLv))
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) force(%d) sel(%d) ROI(X,Y,W,H) : Center(%d, %d, %d, %d), AP(%d, %d, %d, %d), OT(%d, %d, %d, %d), HW(%d, %d, %d, %d)",
                  m_u4ReqMagicNum,
                  m_u4HalMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_bForceTrigger,
                  sInAFInfo.i4ROISel,
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


    if( m_bForceTrigger==MTRUE) /* Got changing ROI command from host. Should be trigger searching.*/
    {
        /**
         *   force using the new ROI which is sent from host, and do one time searching :
         *   @LIB3A_AF_MODE_AFS -> wiait autofocus command.
         *   @LIB3A_AF_MODE_AFC_VIDEO, LIB3A_AF_MODE_AFC ->  focuse to trigger searching by switching af mode to auto mode in AF HAL.
         */

        //Latch ROI to do searching.
        if( m_bLatchROI==MTRUE)
        {
            m_sArea_Focusing = m_sArea_APCmd;
            CAM_LOGD("#(%5d,%5d) %s [CMD] %d (X,Y,W,H)=(%d, %d, %d, %d)",
                     m_u4ReqMagicNum,
                     m_u4HalMagicNum,
                     __FUNCTION__,
                     sInAFInfo.i4ROISel,
                     m_sArea_Focusing.i4X,
                     m_sArea_Focusing.i4Y,
                     m_sArea_Focusing.i4W,
                     m_sArea_Focusing.i4H);

            //apply zoom information.
            ApplyZoomEffect( m_sArea_Focusing);
            //used to control select AF ROI at AFS mode.
            m_bLatchROI = MFALSE;
        }
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
    else if( m_i4IsSelHWROI_PreState!=m_i4IsSelHWROI_CurState) /* Without got changing ROI command from host, and need to do searching*/
    {
        //ready to do searching
        if( m_i4IsSelHWROI_CurState==MTRUE)
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
            CAM_LOGD( "#(%5d,%5d) %s [SEL] %d (X,Y,W,H)=(%d, %d, %d, %d)",
                      m_u4ReqMagicNum,
                      m_u4HalMagicNum,
                      __FUNCTION__,
                      sInAFInfo.i4ROISel,
                      m_sArea_Focusing.i4X,
                      m_sArea_Focusing.i4Y,
                      m_sArea_Focusing.i4W,
                      m_sArea_Focusing.i4H);


            //apply zoom information.
            ApplyZoomEffect( m_sArea_Focusing);
        }
    }

    return m_sArea_Focusing;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::doAF( MVOID *ptrInAFData)
{
    if( m_i4EnableAF==0)
    {
        m_sAFOutput.i4IsAFDone = MTRUE;
        m_sAFOutput.i4IsFocused = 0;
        m_sAFOutput.i4AFPos = 0;
        m_bRunPDEn = MTRUE;
        CAM_LOGD_IF( m_i4DgbLogLv, "disableAF");
        return S_AF_OK;
    }

    {
        Mutex::Autolock lock( m_AFBufLock);
        m_bIsDoAF = MTRUE;
    }

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;
    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d): ", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev);
    }

    //store debug information
    if( m_i4IsSearchDone_CurState==MFALSE)
    {
        MINT64 ts = getTimeStamp();
        WDBGTSInfo( MGR_TS, (MINT32)ts, 0);
    }


    MUINT32 i4curFrmNum = m_u4ReqMagicNum;

    if( (m_sAFInput.i4IsVDO!=MTRUE) || (m_sAFOutput.i4IsAFDone==MFALSE ))
    {
        m_i4DbgMotorDisable = property_get_int32("vendor.debug.af_motor.disable", 0);
        m_i4DbgMotorMPos    = property_get_int32("vendor.debug.af_motor.position", 0);
    }

    AAA_TRACE_D("doAF #(%d,%d)", m_u4ReqMagicNum, m_u4StaMagicNum);

    //-------------------------------------------------------------------------------------------------------------
    // i4Coef[7] > 0: enable new scenechange , else: disable
    if( (m_sNVRam.rAFNVRAM.i4SceneMonitorLevel>0) || (m_sNVRam.rAFNVRAM.i4VdoSceneMonitorLevel>0))
    {
        m_pIAfAlgo->setAEBlockInfo( m_aAEBlkVal, 25);
    }
    //-------------------------------------------------------------------------------------------------------------
    // i4Coef[5] == 1:enable SensorListener, 0: disable
    if(m_sNVRam.rAFNVRAM.i4SensorEnable > 0)
    {


        // get gyro/acceleration data
        SensorData gyroDa;
        MBOOL gyroDaVd = m_bGryoVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_GYRO, gyroDa) : MFALSE;

        if( gyroDaVd && gyroDa.timestamp)
        {
            m_u8PreGyroTS = m_u8GyroTS;
            m_i4GyroInfo[0] = gyroDa.gyro[0] * SENSOR_GYRO_SCALE;
            m_i4GyroInfo[1] = gyroDa.gyro[1] * SENSOR_GYRO_SCALE;
            m_i4GyroInfo[2] = gyroDa.gyro[2] * SENSOR_GYRO_SCALE;
            m_u8GyroTS = gyroDa.timestamp;

            m_pIAfAlgo->setGyroSensorInfo( m_i4GyroInfo, SENSOR_GYRO_SCALE);
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "Gyro InValid!");
            m_pIAfAlgo->setGyroSensorInfo( m_i4GyroInfo, 0); // set scale 0 means invalid to algo
        }

        SensorData acceDa;
        MBOOL acceDaVd = m_bAcceVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_ACCELERATION, acceDa) : MFALSE;

        if( acceDaVd && acceDa.timestamp)
        {
            m_u8PreAcceTS = m_u8AcceTS;
            m_i4AcceInfo[0] = acceDa.acceleration[0] * SENSOR_ACCE_SCALE;
            m_i4AcceInfo[1] = acceDa.acceleration[1] * SENSOR_ACCE_SCALE;
            m_i4AcceInfo[2] = acceDa.acceleration[2] * SENSOR_ACCE_SCALE;
            m_u8AcceTS = acceDa.timestamp;

            m_pIAfAlgo->setAcceSensorInfo( m_i4AcceInfo, SENSOR_ACCE_SCALE);

        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "Acce InValid!");
            m_pIAfAlgo->setAcceSensorInfo(m_i4AcceInfo, 0); // set scale 0 means invalid to algo
        }


        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "Acce preTS(%11lld) TS(%11lld) Info(%4d %4d %4d)/Gyro preTS(%11lld) TS(%11lld) Info(%4d %4d %4d)/",
                                  m_u8PreAcceTS,
                                  m_u8AcceTS,
                                  m_i4AcceInfo[0],
                                  m_i4AcceInfo[1],
                                  m_i4AcceInfo[2],
                                  m_u8PreGyroTS,
                                  m_u8GyroTS,
                                  m_i4GyroInfo[0],
                                  m_i4GyroInfo[1],
                                  m_i4GyroInfo[2]);
        }
    }
    //----------------------------------Prepare AF Algorithm input data------------------------------------------------
    //==========
    // depth AF
    //==========
    if( m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE)
    {
        //Input Depth Info
        MUINT32 QueryDafTblIdx = m_next_query_FrmNum % DAF_TBL_QLEN;

        m_sDAF_TBL.curr_p1_frm_num = m_u4StaMagicNum;

        // af_mgr -> af_algo
        m_sAFInput.i4CurrP1FrmNum = m_u4StaMagicNum;
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
                  m_u4HalMagicNum,
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
                      m_u4HalMagicNum,
                      m_daf_distance,
                      m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_confidence);

            m_daf_distance = (MINT32)m_sDAF_TBL.daf_vec[QueryDafTblIdx].daf_distance;
        }
    }
    //==========
    // LaserAF
    //==========
    if (m_bLaserAFEn == MTRUE)
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
    //==========
    //   PDAF
    //==========
    if(m_bRunPDEn == MTRUE)
    {
        SPDOutputData_t PDOutput( m_i4PDAreaNum);
        MBOOL bGetPdFromThread = MFALSE;

        if (m_bPdInputExpected)
        {
            if (IPDMgr::getInstance().getPDTaskResult( m_i4CurrSensorDev, &PDOutput) == S_3A_OK)
            {
                bGetPdFromThread = MTRUE;
            }
            else
            {
                //pd input expected with no pd output, leverage the latest pd output
                if (m_u4StaMagicNum >= (AF_START_MAGIC_NUMBER + 1))
                {
                    // 1 frame buffer for ignoring boundary situation, i.e. ignore frameNum < 5
                    CAM_LOGW( "PD res not rdy! statMagicNum(%d)", m_u4StaMagicNum);
                }
            }
        }

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "PDExpected(%d)/bGetPD(%d)/StaNum(#%5d)/",
                                  m_bPdInputExpected,
                                  bGetPdFromThread,
                                  m_u4StaMagicNum);
        }

        if (bGetPdFromThread)
        {
            m_i4PDResNum = PDOutput.numRes;

            char  dbgMsgBufPD[DBG_MSG_BUF_SZ];
            char* ptrMsgBufPD = dbgMsgBufPD;

            if( m_i4DgbLogLv)
            {
                ptrMsgBufPD += sprintf( ptrMsgBufPD, "%s PD Res : ", __FUNCTION__);
            }

            for(MINT32 i = 0 ; i < PDOutput.numRes ; i++)
            {
                m_sAFInput.i4PDafDacIndex[i]   = PDOutput.Res[i].Defocus;
                m_sAFInput.i4PDafConfidence[i] = (MUINT32)PDOutput.Res[i].DefocusConfidenceLevel;
                m_sAFInput.i4PDafConverge[i]   = PDOutput.Res[i].PhaseDifference;

                if( m_i4DgbLogLv)
                {
                    ptrMsgBufPD += sprintf( ptrMsgBufPD, "#%d Target Pos(%4d) PD(%4d) CL(%4d)/",
                                            i,
                                            m_sAFInput.i4PDafDacIndex[i],
                                            m_sAFInput.i4PDafConverge[i],
                                            m_sAFInput.i4PDafConfidence[i]);
                }
            }

            //Show pd result during searching.
            if( m_i4IsSearchDone_CurState==MFALSE)
            {
                CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBufPD);
            }
        }

        //direct control
        m_sAFInput.i4PDPureRawfrm = (MINT32)m_bPdInputExpected;

        m_bPdInputExpected = 1-m_bPdInputExpected;
    }

    //---------------------------------------Run hybrid AF core flow---------------------------------------------
    if(IspMgrAFStatHWPreparing(m_u4FrameCount))
    {
        /**
         * The condition AF_ROI_SEL_NONE is for the first time lunch camera only. [Ref] SelROIToFocusing
         */
        CAM_LOGD( "#(%5d,%5d) HW-Preparing", m_u4ReqMagicNum, m_u4HalMagicNum);
    }
    else if( m_bPauseAF)
    {
        CAM_LOGD_IF( m_i4DgbLogLv, "#(%5d,%5d) By pass AF", m_u4ReqMagicNum, m_u4HalMagicNum);
    }
    else
    {
        //get current lens position.
        getLensInfo( m_sCurLensInfo);
        //Do AF is triggered when Vsync is came, so use previours lens information.
        m_sAFInput.sLensInfo = m_sCurLensInfo;
        //Pre-processing input data for hybrid AF.
        HybridAFPreprocessing();
        //select focusing ROI.
        m_sAFInput.sAFArea.i4Count  = 1;
        m_sAFInput.sAFArea.sRect[0] = SelROIToFocusing( m_sAFOutput);
        //set MZ infotmation.
        m_sAFInput.i4IsMZ = m_bMZEn;
        //get current sensor's temperature
        m_sAFInput.u4CurTemperature = getSensorTemperature();
        //select hybrid af behavior.
        m_sAFInput.i4HybridAFMode = GetHybridAFMode();
        //Run algorithm
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "MZen(%d)/temp(%d)/hybrid mode(0x%x)/curPos(%4d)",
                                  m_sAFInput.i4IsMZ,
                                  m_sAFInput.u4CurTemperature,
                                  m_sAFInput.i4HybridAFMode,
                                  m_sAFInput.sLensInfo.i4CurrentPos);
        }
        AAA_TRACE_D("handleAF");
        m_pIAfAlgo->handleAF( m_sAFInput, m_sAFOutput);
        AAA_TRACE_END_D;
        //Configure/update HW setting
        IspMgrAFStatUpdateHw();
        //Move lens position.
        AAA_TRACE_D("Move Lens (%d)", m_sAFOutput.i4AFPos);
        m_i8MvLensTS = MoveLensTo( m_sAFOutput.i4AFPos, AF_MGR_CALLER);
        AAA_TRACE_END_D;
        //Update parameter for flow controlling
        m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState;
        m_i4IsSelHWROI_CurState = m_sAFOutput.i4IsSelHWROI;
    }

    //------------------------------------Update parameter for depth AF----------------------------------
    setDAFInfo();

    //------------------------------------Update parameter for flow controlling----------------------------------
    m_i4IsAFDone_PreState    = m_i4IsAFDone_CurState;
    m_i4IsAFDone_CurState    = m_sAFOutput.i4IsAFDone;
    m_i4IsMonitorFV_PreState = m_i4IsMonitorFV_CurState;
    m_i4IsMonitorFV_CurState = m_sAFOutput.i4IsMonitorFV;
    if (m_bCheckStateByAFDone != MFALSE)
    {
        m_i4IsSearchDone_PreState = m_i4IsAFDone_PreState;
        m_i4IsSearchDone_CurState = m_i4IsAFDone_CurState;
    }
    else
    {
        m_i4IsSearchDone_PreState = m_i4IsMonitorFV_PreState;
        m_i4IsSearchDone_CurState = m_i4IsMonitorFV_CurState;
    }

    m_i4IsFocused = m_sAFOutput.i4IsFocused;

    UpdatePDParam( m_sAFOutput.i4ROISel);

    //Event
    if(  m_i4IsSearchDone_PreState!=m_i4IsSearchDone_CurState)
    {
        if( m_i4IsSearchDone_CurState==MFALSE)
        {
            CleanMgrDbgInfo();
            CleanTSDbgInfo();
            UpdateState( EVENT_SEARCHING_START);
        }
        else
        {
            SetMgrDbgInfo();
            UpdateState( EVENT_SEARCHING_END);
        }
    }


    if( (m_i4IsSearchDone_PreState!=m_i4IsSearchDone_CurState) ||
            (m_i4IsSelHWROI_PreState!=m_i4IsSelHWROI_CurState) ||
            (m_i4DgbLogLv))
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): [Status] IsSearchDn(%d->%d), IsSelHWROI(%d->%d), ROISel(%d), IsAFDone(%d->%d), IsMonitorFV(%d->%d)",
                  m_u4ReqMagicNum,
                  m_u4HalMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_i4IsSearchDone_PreState,
                  m_i4IsSearchDone_CurState,
                  m_i4IsSelHWROI_PreState,
                  m_i4IsSelHWROI_CurState,
                  m_sAFOutput.i4ROISel,
                  m_i4IsAFDone_PreState,
                  m_i4IsAFDone_CurState,
                  m_i4IsMonitorFV_PreState,
                  m_i4IsMonitorFV_CurState);

    }

    //---------------------------------------------------------------------------------------------------
    // set AF info
    AF_INFO_T sAFInfo;
    sAFInfo.i4AFPos = m_sAFOutput.i4AFPos;
    IspTuningMgr::getInstance().setAFInfo(m_i4CurrSensorDev, sAFInfo);
    AAA_TRACE_END_D;

    {
        Mutex::Autolock lock( m_AFBufLock);
        m_bIsDoAF = MFALSE;
    }

    CAM_LOGD_IF( m_i4DgbLogLv, "%s %d", dbgMsgBuf, ptrMsgBuf-dbgMsgBuf);
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_SENSOR_INFO_T* AfMgr::getMatchedISInfoFromFrameId(MINT32 frameId)
{
    static ISP_SENSOR_INFO_T ret;
    ret.i4FrameId = -1;

    MUINT32 front = m_vISQueue.head;
    MUINT32 end = m_vISQueue.tail;

    if (end > front)
        front += m_vISQueue.queueSize;

    // assume frameid is continuous, calculate idx by frameid difference
    MUINT32 offset = m_vISQueue.content[(front % m_vISQueue.queueSize)].i4FrameId - frameId;
    MUINT32 idx = (front-offset) % m_vISQueue.queueSize;

    int enableDebug = m_i4DbgAfegainQueue & 0x2;
    if (enableDebug)
    {
        CAM_LOGD("search afegain queue... magic: %d, head frameid: %d, idx: %d, indexed frameid: %d",
                 frameId, m_vISQueue.content[(front % m_vISQueue.queueSize)].i4FrameId, idx, m_vISQueue.content[idx].i4FrameId);
    }

    if (idx < m_vISQueue.queueSize && m_vISQueue.content[idx].i4FrameId == frameId) //found
    {
        memcpy(&ret, &(m_vISQueue.content[idx]), sizeof(ISP_SENSOR_INFO_T));
    }
    else // fail by indexing, search one by one
    {
        if (enableDebug)
        {
            CAM_LOGD("search afegain queue... Failed by indexing, search one by one, found idx = %d", idx);
        }
        for (int i=front; i>end; i--)
        {
            idx = i % m_vISQueue.queueSize;
            if (m_vISQueue.content[idx].i4FrameId == frameId) //found
                memcpy(&ret, &(m_vISQueue.content[idx]), sizeof(ISP_SENSOR_INFO_T));
        }
    }
    // cannot get matched afegain from queue
    if (ret.i4FrameId == -1)
    {
        CAM_LOGE("Cannot get matched afegain from m_vISQueue!");

        front = m_vISQueue.head;
        end = m_vISQueue.tail;

        if (end > front)
            front += m_vISQueue.queueSize;
        for (int i=front; i>end; i--)
        {
            MUINT32 idx = i % m_vISQueue.queueSize;
            CAM_LOGD("AfeGain qIdx(%d): frameId(%d), afeGain(%d), ispGain(%d)",
                     idx, m_vISQueue.content[idx].i4FrameId, m_vISQueue.content[idx].u4AfeGain, m_vISQueue.content[idx].u4IspGain);
        }
    }
    return &ret;
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
            ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to incorrect lensInfo", m_bRunPDEn);
            CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
        }
        return S_AF_OK;
    }

    MUINT32 bufSize;
    MUINT32 bufStride;
    MUINT8* pBufAddr;
    MUINT32 magicNum = m_u4ReqMagicNum - 2; //default value if no magicnumber from buffer info

    IImageBuffer* pPureRawBuf; //for SensorType_Pure_Raw only

    //get pd input buffer info
    if (m_i4PDSensorType == SensorType_Pure_Raw)
    {
        pPureRawBuf = reinterpret_cast<IImageBuffer *>( ptrInPDData);
        bufSize =   pPureRawBuf->getImageBufferHeap()->getBufSizeInBytes(0);
        bufStride = pPureRawBuf->getImageBufferHeap()->getBufStridesInBytes(0);
        pBufAddr =  (MUINT8*)pPureRawBuf->getImageBufferHeap()->getBufVA(0);
    }
    else
    {
        StatisticBufInfo *ptrPDHWData = reinterpret_cast<StatisticBufInfo *>( ptrInPDData);
        bufSize = ptrPDHWData->mSize;
        bufStride = ptrPDHWData->mStride;
        pBufAddr = (MUINT8*)ptrPDHWData->mVa;

        if( m_bPDVCTest)
        {
            //do nothing
        }
        else if (m_i4PDSensorType == SensorType_PDAF_Raw || m_i4PDSensorType == SensorType_DualPD_Raw)
        {
            magicNum = ptrPDHWData->mMagicNumber; // has magicnumber from buffer info
        }
        else if (m_i4PDSensorType == SensorType_PDAF_VC_HDR || m_i4PDSensorType == SensorType_DualPD_VC)
        {
            //do nothing
        }
        else
        {
            if( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to No correct PD task [Dev %d]: %d, %d", m_i4CurrSensorDev, m_i4EnableAF, m_i4PDSensorType);
                CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
            }
            return S_AF_OK;
        }
    }

    //abort if magicNumber < AF_START_MAGIC_NUMBER
    if (magicNum < AF_START_MAGIC_NUMBER)
    {
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "abort due to magicNumber(%d) < AF_START_MAGIC_NUMBER", magicNum);
            CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
        }
        return S_AF_OK;
    }

    //do pd sw path per 2 frame
    m_bNeedPdoResult = 1-m_bNeedPdoResult;
    if (!m_bNeedPdoResult)
    {
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "skip 1 of 2 frames");
            CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
        }
        return S_AF_OK;
    }

    if( m_bPDVCTest)
    {
        //for engineer check only.
        CAM_LOGD_IF( m_i4DgbLogLv, "Debug virtual channel only : for engineer testing only!!");

        char fileName[64];
        sprintf( fileName, "/sdcard/vc/ae%d.raw", magicNum);
        FILE *fp = fopen( fileName, "w");
        if( NULL==fp)
        {
            MY_ERR( "fail to open file to save img: %s", fileName);
            MINT32 err = mkdir( "/sdcard/vc", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD( "err = %d", err);
            return S_AF_OK;
        }
        fwrite( pBufAddr, 1, bufSize, fp);
        fclose( fp);

        return S_AF_OK;
    }

    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, " buf#%d Sz(%d) stride(%d) Addr(0x%x), ", magicNum, bufSize, bufStride, pBufAddr);
    }

    //allocate pd input data
    SPDInputData_t PDInput( m_i4PDAreaNum, bufSize, bufStride, pBufAddr);

    PDInput.frmNum = m_u4ReqMagicNum;
    PDInput.magicNumber = magicNum;
    PDInput.curLensPos   = m_i4PreLensPos;
    PDInput.XSizeOfImage = m_i4TGSzW;
    PDInput.YSizeOfImage = m_i4TGSzH;

    MINT64 timestamp = getTimeStamp();
    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "curPos %4d, TS mvMCU->PD %3d, ", PDInput.curLensPos, (MINT32)(timestamp-m_i8MvLensTS));
    }


    if( m_i4DgbLogLv)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "PDAF win (X1,Y1,X2,Y2): ");
    }
    for( MUINT32 i=0; i<PDInput.numROI; i++)
    {
        PDInput.ROI[i].i4XStart = m_sArea_PD[i].i4X;
        PDInput.ROI[i].i4YStart = m_sArea_PD[i].i4Y;
        PDInput.ROI[i].i4XEnd = m_sArea_PD[i].i4X + m_sArea_PD[i].i4W;
        PDInput.ROI[i].i4YEnd = m_sArea_PD[i].i4Y + m_sArea_PD[i].i4H;
        PDInput.ROI[i].i4Info = m_sArea_PD[i].i4Info;

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d, %d, %d) ", i, PDInput.ROI[i].i4XStart, PDInput.ROI[i].i4YStart, PDInput.ROI[i].i4XEnd, PDInput.ROI[i].i4YEnd);
        }
    }
    ISP_SENSOR_INFO_T* pAfeGain = getMatchedISInfoFromFrameId(PDInput.magicNumber);
    PDInput.afeGain = pAfeGain->u4AfeGain;
    PDInput.hlrEn = pAfeGain->bHLREnable;

    IPDMgr::getInstance().postToPDTask( m_i4CurrSensorDev, &PDInput);

    CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::enablePBIN(MBOOL bEnable)
{
    m_bEnablePBIN = bEnable;
    return S_AF_OK;
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

    if( m_bMZEn)
    {
        scalex = m_sNVRam.rAFNVRAM.i4MultiAFCoefs[0];
        scaley = m_sNVRam.rAFNVRAM.i4MultiAFCoefs[1];
    }
    else if( m_i4PDSensorType != SensorType_NO_PDAF)
    {
        scalex = m_sNVRam.rAFNVRAM.i4HybridAFCoefs[1];
        scaley = m_sNVRam.rAFNVRAM.i4HybridAFCoefs[2];
    }
    else
    {
        scalex = m_sNVRam.rAFNVRAM.i4SPOT_PERCENT_W;
        scaley = m_sNVRam.rAFNVRAM.i4SPOT_PERCENT_H;
    }

    if(     100<scalex) scalex=100;
    else if( scalex<=0) scalex=15;

    if(     100<scaley) scaley=100;
    else if( scaley<=0) scaley=15;

    CAM_LOGD( "%s %d %d %d %d %d %d - scale %d %d",
              __FUNCTION__,
              m_sNVRam.rAFNVRAM.i4MultiAFCoefs[0],
              m_sNVRam.rAFNVRAM.i4MultiAFCoefs[1],
              m_sNVRam.rAFNVRAM.i4HybridAFCoefs[1],
              m_sNVRam.rAFNVRAM.i4HybridAFCoefs[2],
              m_sNVRam.rAFNVRAM.i4SPOT_PERCENT_W,
              m_sNVRam.rAFNVRAM.i4SPOT_PERCENT_H,
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
    if( (m_PDBuf_Type!=EPDBuf_NotDef) && (m_i4PDSensorType != SensorType_NO_PDAF))
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;

        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s 0x%x ", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_PDBuf_Type);
        }

        AREA_T PDRoi;
        switch( i4InROISel)
        {
        case AF_ROI_SEL_AP :
            PDRoi = m_sArea_APCmd;
            break;
        case AF_ROI_SEL_OT :
        case AF_ROI_SEL_FD :
            PDRoi = m_sArea_OTFD;
            break;
        case AF_ROI_SEL_NONE :
        case AF_ROI_SEL_CENTER :
        case AF_ROI_SEL_DEFAULT :
        default :
            PDRoi = m_sArea_Center;
            break;
        }
        //apply zoom information.
        ApplyZoomEffect( PDRoi);

        PDRoi.i4Info = 0;

        MRESULT res = m_pIAfAlgo->getFocusWindows( PDRoi, AF_PSUBWIN_NUM, &m_sArea_PD[0], &m_i4PDAreaNum);
        if( res!=S_3A_OK)
        {
            m_i4PDAreaNum = 1;
            m_sArea_PD[0] = PDRoi;
            m_sArea_PD[0].i4Info = 1;
        }


        // error check
        if( m_i4DgbLogLv)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "(X,Y,W,H) : ");
        }

        for( MINT32 i=0; i<m_i4PDAreaNum; i++)
        {
            if( m_i4DgbLogLv)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d, %d, %d), ", i, m_sArea_PD[i].i4X, m_sArea_PD[i].i4Y, m_sArea_PD[i].i4W, m_sArea_PD[i].i4H);
            }

            if( (m_sArea_PD[i].i4W <= 0) ||
                    (m_sArea_PD[i].i4H <= 0) ||
                    (m_sArea_PD[i].i4X <  0) ||
                    (m_sArea_PD[i].i4Y <  0) )
            {
                CAM_LOGE( "ROI is wrong !!");
                m_i4PDAreaNum = 0;
                break;
            }

        }
        CAM_LOGD_IF( m_i4DgbLogLv, "%s", dbgMsgBuf);

        if( m_i4PDAreaNum==0)
        {
            m_i4PDAreaNum = 1;
            m_sArea_PD[0] = PDRoi;
        }

        m_bRunPDEn = MTRUE;
    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::doPDBuffer( MVOID * buffer, MINT32 w,MINT32 h, MINT32 stride, MUINT32 i4FrmNum)
{
    m_pdaf_rawbuf=buffer;
    m_pdaf_w=w;
    m_pdaf_h=h;
    m_pdaf_stride=stride;
    m_pdaf_raw_frmnum = i4FrmNum;
    CAM_LOGD( "[doPDBuffer][%d] %d %d %d\n", m_pdaf_raw_frmnum, m_pdaf_w, m_pdaf_h, m_pdaf_stride);
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
MVOID AfMgr::setSGGPGN( MINT32 i4SGG_PGAIN)
{
    // workaround for iVHDR, no use
#if 0
    MINT32 i4AESetPGN = i4SGG_PGAIN;
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::passAFBuffer( MVOID *ptrInAFData)
{
    {
        Mutex::Autolock lock( m_AFBufLock);
        if (m_bIsDoAF == MTRUE)
        {
            CAM_LOGW( "Skip AF statistic data(%d)", m_u4StaMagicNum);
            return S_AF_OK;
        }
    }

    //---------------------------------------Statistic data information--------------------------------------------
    StatisticBufInfo *ptrStatInfo = reinterpret_cast<StatisticBufInfo *>( ptrInAFData);
    m_u4StaMagicNum = ptrStatInfo->mMagicNumber;
    m_u4FrameCount  = ptrStatInfo->mFrameCount;

    //Got AF statistic from DMA buffer.
    AAA_TRACE_D("ConvertBufToStat (%d)", m_u4StaMagicNum);
    ConvertDMABufToStat( m_sAFInput.sLensInfo.i4CurrentPos, ptrInAFData, m_sAFInput.sStatProfile);
    AAA_TRACE_END_D;

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::ConvertDMABufToStat( MINT32 &i4CurPos, MVOID *ptrInStatBuf, AF_STAT_PROFILE_T &sOutSata)
{
    StatisticBufInfo *ptrStatInfo = reinterpret_cast<StatisticBufInfo *>( ptrInStatBuf);
    MUINT8  *pDMABuf        = reinterpret_cast<MUINT8 *>( ptrStatInfo->mVa);
    MUINT32 u4StatMagicNum  = ptrStatInfo->mMagicNumber;
    MUINT32 u4StatSize      = ptrStatInfo->mSize;
    MUINT32 u4Stride        = ptrStatInfo->mStride;
    MUINT32 u4HwStatSize    = sizeof(AF_HW_STAT_T) / 2; /* Statistics Data : 128bits */

#if defined(MTKCAM_AF_EXTMODE_SUPPORT)
    if( m_i4HWEnExtMode != 0)
    {
        u4HwStatSize *= 2; /* Statistics Data : 256bits */
    }
#endif

    CAM_LOGD_IF( m_i4DgbLogLv,
                 "#(%5d,%5d) %s(%d) Sz(%d) BlkW(%d) BlkH(%d) SzW(%d) SzH(%d) Str(%d) off(%d)",
                 m_u4ReqMagicNum,
                 m_u4HalMagicNum,
                 __FUNCTION__,
                 u4StatMagicNum,
                 u4StatSize,
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
    else if( u4StatSize<m_i4HWBlkNumX*u4HwStatSize*m_i4HWBlkNumY)
    {
        //Should not be happened.
        CAM_LOGE( "AFO Size Fail");
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
#if defined(MTKCAM_AF_EXTMODE_SUPPORT)
        // AF extend mode
        sOutSata.u4AfExtValid = m_i4HWEnExtMode;
#endif
#if defined(MTKCAM_CMDQ_SUPPORT)
        sOutSata.u4ConfigNum  = u4StatMagicNum;
        sOutSata.i4AFPos      = i4CurPos;
#endif
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


#if defined(MTKCAM_AF_EXTMODE_SUPPORT)
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
#endif

                ptrSata++;
                pHwStatBuf_W += u4HwStatSize;
            }

            pDMABuf += u4Stride;
        }

        // For debug FV only.
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "#(%5d,%5d) %s(%d) [Pos]%4d [H0]%10lld [H1]%10lld [H2_EXT]%10lld [V]%lld",
                     m_u4ReqMagicNum,
                     m_u4HalMagicNum,
                     __FUNCTION__,
                     u4StatMagicNum,
                     i4CurPos,
                     FV_H0,
                     FV_H1,
                     FV_H2,
                     FV_V);

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
MINT64 AfMgr::TransStatProfileToAlgo( AF_STAT_PROFILE_T &sInSataProfile)
{
    /*[TODO] For preview IT only.*/
    MINT64 i8SataH0=0;

    //analyze center 6x6 blocks
    MUINT32 ablkW = 6;
    MUINT32 ablkH = 6;
    //total blocks
    MUINT32 tblkW = sInSataProfile.u4NumBlkX;
    MUINT32 tblkH = sInSataProfile.u4NumBlkY;
    //start address of block index.
    MUINT32 startX = (tblkW-ablkW)/2;
    MUINT32 startY = (tblkH-ablkH)/2;


    for( MUINT32 j=0; j<ablkH; j++)
    {
        for( MUINT32 i=0; i<ablkW; i++)
        {
            MUINT32 idx = (startY+j)*tblkW + (startX+i);
            i8SataH0 += sInSataProfile.ptrStat[idx].u4FILH0;
        }
    }

    //[TODO] m_pIAfAlgo->setAFStats(&statsH[0], &statsV[0], MAX_AF_HW_WIN);

    return i8SataH0;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocusFinish()
{
    MINT32 ret = 1;

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
MINT32 AfMgr::isLockAE()
{
    return m_sAFOutput.i4IsLockAERequest;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getDebugInfo( AF_DEBUG_INFO_T &rAFDebugInfo)
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
    if( m_pIAfAlgo)
    {
        ret = m_pIAfAlgo->getDebugInfo( rAFDebugInfo);
    }

    //2. PD library
    if( m_i4PDSensorType != SensorType_NO_PDAF)
    {
        //Not support open pd library.
        if( m_PDBuf_Type==EPDBuf_Raw || m_PDBuf_Type==EPDBuf_VC)
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

    if( ((AF_DEBUG_TAG_SIZE-idx)>=m_i4MgrExifSz) && (0<m_i4MgrExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrExifSz;
    }

    if( (AF_DEBUG_TAG_SIZE-idx)>=m_i4MgrCapExifSz && (0<m_i4MgrCapExifSz))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrCapExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrCapExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrCapExifSz;
    }

    if( (AF_DEBUG_TAG_SIZE-idx)>=m_i4MgrTsExifSz && (0<m_i4MgrTsExifSz))
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
        ret = WDBGInfo( MGR_PD_SEN_TYPE, m_i4PDSensorType, 0);
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
MRESULT AfMgr::calBestShotValue( MVOID *pAFStatBuf)
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
MUINT32 AfMgr::getSensorTemperature()
{
    MUINT32 u4temperature = 0;

    if ( pIHalSensorInfo)
    {
        pIHalSensorInfo->sendCommand(m_i4CurrSensorDev, SENSOR_CMD_GET_TEMPERATURE_VALUE,(MINTPTR)& u4temperature, 0, 0);
    }

    return u4temperature;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::getTimeStamp()
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp =((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000;
    return timestamp; // from nano to ms
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::getTimeStamp_us()
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp =((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000;
    return timestamp; // from nano to us
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::MoveLensTo( MINT32 &i4TargetPos, MUINT32 u4Caller)
{
    MINT64 TS_BeforeMoveMCU = 0; // timestamp of before moveMCU
    MINT64 TS_AfterMoveMCU = 0;  // timestamp of after moveMCU
    if( m_pMcuDrv)
    {
        if( m_i4DbgMotorDisable==1)
        {
            if( m_i4MvLensTo!=m_i4DbgMotorMPos)
            {
                m_i4MvLensTo = m_i4DbgMotorMPos;
                m_pMcuDrv->moveMCU( m_i4DbgMotorMPos, m_i4CurrSensorDev);
            }
        }
        else if( m_i4MvLensTo!=i4TargetPos)
        {
            if( u4Caller==AF_MGR_CALLER)
            {
                CAM_LOGD("#(%5d,%5d) %s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }
            else
            {
                CAM_LOGD("#(%5d,%5d) cmd-%s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }

            m_i4MvLensTo = i4TargetPos;

            CAM_LOGD_IF( m_i4DgbLogLv&4, "%s+", __FUNCTION__);
            TS_BeforeMoveMCU = getTimeStamp_us();
            m_pMcuDrv->moveMCU( m_i4MvLensTo, m_i4CurrSensorDev);
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
        return (MINT64)((TS_BeforeMoveMCU + TS_AfterMoveMCU)/2);
    }
    else
    {
        // inherit the original one
        return m_i8MvLensTS;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::readOTP(CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum)
{
    MUINT32 result = 0;
    CAM_CAL_DATA_STRUCT GetCamCalData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MINT32 i4SensorDevID;

    CAM_LOGD("read (%d) calibration data from EEPROM by camcal", enCamCalEnum);
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
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        return S_AWB_OK;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AWB_OK;
    }

    result = pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);
    CAM_LOGD_IF( m_i4DgbLogLv, "(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);

    if (enCamCalEnum == CAMERA_CAM_CAL_DATA_3A_GAIN)
    {
        if (result&CamCalReturnErr[enCamCalEnum])
        {
            CAM_LOGD( "err (%s)", CamCalErrString[enCamCalEnum]);
            //return E_AF_NOSUPPORT;
        }

        CAM_LOGD_IF( m_i4DgbLogLv, "OTP data [S2aBitEn]%d [S2aAfBitflagEn]%d [S2aAf0]%d [S2aAf1]%d",
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
            CAM_LOGD( "OTP [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);

            if( (i4MacroPos>i450cmPos) && (i450cmPos>i4InfPos) )
            {
                if (m_bLaserAFEn == MTRUE)
                {
                    ILaserMgr::getInstance().setLensCalibrationData(m_i4CurrSensorDev, i4MacroPos, i450cmPos);
                }

                CAM_LOGD( "OTP [50cm]%d", i450cmPos);
            }

            if( i4MiddlePos>i4InfPos && i4MiddlePos<i4MacroPos)
            {
                if( m_pIAfAlgo)
                {
                    m_pIAfAlgo->updateMiddleAFPos(i4MiddlePos);
                }
            }

            if( m_pIAfAlgo)
            {
                m_pIAfAlgo->updateAFtableBoundary( i4InfPos, i4MacroPos);
            }

            m_i4InitPos = i4InfPos + (i4MacroPos - i4InfPos) * m_sNVRam.rAFNVRAM.i4Coefs[10] / 100;
            m_i4InitPos = (m_i4InitPos < 0) ? 0 : ((m_i4InitPos > 1023) ? 1023 : m_i4InitPos);
            CAM_LOGD( "[InitPos]%d ", m_i4InitPos);

            // adjust depth dac_min, dac_max
            m_sDAF_TBL.af_dac_min = i4InfPos;
            m_sDAF_TBL.af_dac_max = i4MacroPos;
        }
        else
        {
            CAM_LOGD_IF( m_i4DgbLogLv, "OTP abnormal return [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);

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
        }
    }
    else if (enCamCalEnum == CAMERA_CAM_CAL_DATA_PDAF)
    {
        if (result==CAM_CAL_ERR_NO_ERR)
        {
            memcpy( m_ptrAfPara->rPdCaliData.uData, GetCamCalData.PDAF.Data, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
            m_ptrAfPara->rPdCaliData.i4Size = GetCamCalData.PDAF.Size_of_PDAF;

            CAM_LOGD("Size of PD calibration data = %d\n", m_ptrAfPara->rPdCaliData.i4Size);
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
    CAM_LOGD( "cmd-%s Dev %d : lib_afmode %d", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    if( m_i4EnableAF==0)
    {
        CAM_LOGD( "autofocus : dummy lens");
        return;
    }

    UpdateState( EVENT_CMD_AUTOFOCUS);

    if( m_pIAfAlgo)
    {
        m_pIAfAlgo->targetAssistMove();
    }

    //calibration flow testing
    if (m_bLaserAFEn == MTRUE)
    {
        int Offset = 0;
        int XTalk = 0;

        int Mode = property_get_int32("vendor.laser.calib.mode", 0);

        if( Mode == 1 )
        {
            CAM_LOGD( "LaserCali : getLaserOffsetCalib Start");
            Offset = ILaserMgr::getInstance().getLaserOffsetCalib(m_i4CurrSensorDev);
            CAM_LOGD( "LaserCali : getLaserOffsetCalib : %d", Offset);
            CAM_LOGD( "LaserCali : getLaserOffsetCalib End");
        }

        if( Mode == 2 )
        {
            CAM_LOGD( "LaserCali : getLaserXTalkCalib Start");
            XTalk = ILaserMgr::getInstance().getLaserXTalkCalib(m_i4CurrSensorDev);
            CAM_LOGD( "LaserCali : getLaserXTalkCalib : %d", XTalk);
            CAM_LOGD( "LaserCali : getLaserXTalkCalib End");
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::cancelAutoFocus()
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d) lib_afmode(%d)",
              m_u4ReqMagicNum,
              m_u4HalMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eLIB3A_AFMode);

    //update parameters and status.
    UpdateState(EVENT_CMD_AUTOFOCUS_CANCEL);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::TimeOutHandle()
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d)",
              m_u4ReqMagicNum,
              m_u4HalMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev);

    m_pIAfAlgo->cancel();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAperture( MFLOAT lens_aperture)
{

}
MFLOAT AfMgr::getAperture()
{
    MFLOAT lens_aperture=0;
    if( m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for( MUINT8 ii=0; ii<m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_APERTURES).count(); ii++)
    {
        lens_aperture = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_APERTURES).itemAt( ii, Type2Type<MFLOAT>());
        CAM_LOGD( "AFmeta APERTURES %d  %f", ii, lens_aperture);
    }
    return lens_aperture;
}
MVOID AfMgr::setFilterDensity( MFLOAT lens_filterDensity) {}
MFLOAT AfMgr::getFilterDensity()
{
    MFLOAT lens_filterDensity=0;
    if( m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for(MUINT8 ii=0; ii<m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES).count(); ii++)
    {
        lens_filterDensity = m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES).itemAt( ii, Type2Type<MFLOAT>());
        CAM_LOGD( "AFmeta FILTER_DENSITIES %d  %f", ii, lens_filterDensity);
    }
    return lens_filterDensity;
}
MVOID AfMgr::setFocalLength( MFLOAT lens_focalLength) {}
MFLOAT AfMgr::getFocalLength ()
{
    MFLOAT lens_focalLength =34.0;
    if( m_bGetMetaData==MFALSE)
    {
        return 0;
    }

    for( MUINT8 ii=0; ii<m_sMetaData.entryFor( MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS).count(); ii++)
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

    i4tblLL = m_sAFParam.i4TBLL;
    for( fdacidx=0; fdacidx<i4tblLL; fdacidx++)
    {
        if( m_sAFOutput.i4AFPos>m_sAFParam.i4Dacv[fdacidx])
            break;
    }

    if(fdacidx==0)
    {
        i4ResultDist = m_sAFParam.i4Dist[0];
    }
    else
    {
        i4ResultDist=
            ( m_sAFParam.i4Dist[fdacidx  ] * (m_sAFParam.i4Dacv[fdacidx-1] - m_sAFOutput.i4AFPos       )  +
              m_sAFParam.i4Dist[fdacidx-1] * (m_sAFOutput.i4AFPos          - m_sAFParam.i4Dacv[fdacidx])) /
            ( m_sAFParam.i4Dacv[fdacidx-1] - m_sAFParam.i4Dacv[fdacidx]);

        i4ResultRange=
            ( m_sAFParam.i4FocusRange[fdacidx  ] * (m_sAFParam.i4Dacv[fdacidx-1] - m_sAFOutput.i4AFPos       )  +
              m_sAFParam.i4FocusRange[fdacidx-1] * (m_sAFOutput.i4AFPos          - m_sAFParam.i4Dacv[fdacidx])) /
            ( m_sAFParam.i4Dacv[fdacidx-1] - m_sAFParam.i4Dacv[fdacidx]);
    }
    if( i4ResultDist <=0)  i4ResultDist= m_sAFParam.i4Dist[i4tblLL-1];
    if( i4ResultRange<=0) i4ResultRange= m_sAFParam.i4Dist[i4tblLL-1];

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
                 m_u4HalMagicNum,
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
    ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).config(sInHWCfg, sOutHWROI, i4OutHWBlkNumX, i4OutHWBlkNumY, i4OutHWBlkSizeX, i4OutHWBlkSizeY);

#if defined(MTKCAM_AF_EXTMODE_SUPPORT)
    m_i4HWEnExtMode = sInHWCfg.AF_EXT_ENABLE;
#endif

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

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatInit()
{
#if defined(MTKCAM_CMDQ_SUPPORT)
    sem_init(&m_SemCmdQ, 0, 0);
    {
        Mutex::Autolock lock( m_CmdQLock);
        m_i4VsyncUpdate = MFALSE;
    }
#endif
    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).init(m_i4SensorIdx);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatUninit()
{
    //uninitial isp_mgr_af_stat for configure HW
#if defined(MTKCAM_CMDQ_SUPPORT)
    {
        Mutex::Autolock lock( m_CmdQLock);
        m_i4VsyncUpdate = MFALSE;
    }
#endif
    ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4CurrSensorDev)).uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::IspMgrAFStatHWPreparing(MUINT32 u4FrameCount)
{
    MBOOL ret = MTRUE;

    if (m_eCurAFMode == MTK_CONTROL_AF_MODE_OFF) // AF off mode don't need to check statistic data.
    {
        ret = MFALSE;
    }
    else if (u4FrameCount < AF_START_MAGIC_NUMBER) //skip 3 frame to avoid unstable statistic data
    {
        ret = MTRUE;
    }
    else
    {
#if defined(MTKCAM_CMDQ_SUPPORT)
        {
            Mutex::Autolock lock( m_CmdQLock);
            m_i4VsyncUpdate = MTRUE;
        }
        ret = MFALSE;
#else
        ret = ((ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).isHWRdy( m_u4StaMagicNum)==MFALSE) &&
               (m_sAFOutput.i4ROISel!=AF_ROI_SEL_NONE)&&
               (m_sAFOutput.i4ZECChg!=1)&& //ZEC chg
               (m_sAFOutput.i4IsTargetAssitMove != 1));
#endif
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::IspMgrAFStatUpdateHw()
{
#if defined(MTKCAM_CMDQ_SUPPORT)
    int val;
    //Configure/update HW setting during vsync
    sem_getvalue(&m_SemCmdQ, &val);
    if (val == 0)
    {
        Mutex::Autolock lock( m_CmdQLock);
        memcpy( &m_sHWCfg, &m_sAFOutput.sAFStatConfig, sizeof(AF_CONFIG_T));
        sem_post(&m_SemCmdQ);
    }
    else
    {
        CAM_LOGD( "CmdQ Timing E");
    }
#else
    ConfigHWReg( m_sAFOutput.sAFStatConfig, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::VsyncUpdate()
{
#if defined(MTKCAM_CMDQ_SUPPORT)
#define SEM_WAIT_TIME_15MS       15000000L

    MBOOL bRet = MTRUE;

    if (m_i4VsyncUpdate != MFALSE)
    {
        bRet = sem_wait_rt(&m_SemCmdQ, SEM_WAIT_TIME_15MS, "ComQ::ConfigHWReg");
        if (bRet)
        {
            Mutex::Autolock lock( m_CmdQLock);
            ConfigHWReg( m_sHWCfg, m_sArea_HW, m_i4HWBlkNumX, m_i4HWBlkNumY, m_i4HWBlkSizeX, m_i4HWBlkSizeY);
        }
    }
#endif

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info)
{
    CAM_LOGD_IF(m_i4DgbLogLv&4, "[%s] pSem(%p), reltime(%lld), info(%s)", __FUNCTION__, pSem, reltime, info);
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        CAM_LOGE("error in clock_gettime! Please check\n");

    ts.tv_sec  += reltime/1000000000;
    ts.tv_nsec += reltime%1000000000;
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
MVOID AfMgr::updateSensorListenerParams( MINT32 *i4SensorInfo)
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
MVOID AfMgr::SetCurFrmNum( MUINT32 u4FrmNum, MUINT32 u4FrmNumCur)
{
    // Request
    m_u4ReqMagicNum = u4FrmNum;
    // Statistic
    m_u4HalMagicNum = u4FrmNumCur;
    m_eEvent = EVENT_NONE;
    CAM_LOGD_IF( m_i4DgbLogLv&2, "Req #%5d, Stat #%5d", m_u4ReqMagicNum, m_u4HalMagicNum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setDAFInfo()
{
    //==========
    // Prepare AF info to Vsdof
    //==========
    if( m_sDAF_TBL.is_daf_run & E_DAF_RUN_STEREO)
    {
        MUINT32 MagicNum, CurDafTblIdx;

#if defined(MTKCAM_CMDQ_SUPPORT)
        MagicNum = m_u4HalMagicNum;
#else
        MagicNum = m_u4StaMagicNum;
#endif
        CurDafTblIdx = MagicNum % DAF_TBL_QLEN;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].frm_mun        = MagicNum;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_mode        = m_eCurAFMode;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_roi_sel     = m_sAFOutput.i4ROISel;
        // Contrast AF output info
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_pos     = m_sCurLensInfo.i4CurrentPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_af_stable   = m_sAFOutput.i4IsAFDone;

        // use the current face roi to avoid the finally inconsistency between depth point and focusing roi
        if( m_sAFOutput.i4ROISel == AF_ROI_SEL_FD)
        {
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x = (MUINT16)m_sArea_OTFD.i4X;
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y = (MUINT16)m_sArea_OTFD.i4Y;
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x   = (MUINT16)m_sArea_OTFD.i4X + m_sArea_OTFD.i4W;
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y   = (MUINT16)m_sArea_OTFD.i4Y + m_sArea_OTFD.i4H;
        }
        else
        {
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x = (MUINT16)m_sArea_Focusing.i4X;
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y = (MUINT16)m_sArea_Focusing.i4Y;
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x   = (MUINT16)m_sArea_Focusing.i4X + m_sArea_Focusing.i4W;
            m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y   = (MUINT16)m_sArea_Focusing.i4Y + m_sArea_Focusing.i4H;
        }

        CAM_LOGD_IF(m_i4DgbLogLv, "[%s] #%d X1(%d) X2(%d) Y1(%d) Y2(%d)", __FUNCTION__, CurDafTblIdx,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y);
    }

    //==========
    // Prepare depth AF data to DEPTH engine
    //==========
    if( m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE)
    {
        MUINT32 MagicNum, CurDafTblIdx;

#if defined(MTKCAM_CMDQ_SUPPORT)
        MagicNum = m_u4HalMagicNum;
#else
        MagicNum = m_u4StaMagicNum;
#endif
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
        CAM_LOGD( "#(%5d,%5d) cmd-%s %d->%d", m_u4ReqMagicNum, m_u4HalMagicNum, __FUNCTION__, m_bMZHostEn, bEn);

        m_bMZHostEn = bEn;
        m_bMZEn     = m_i4DbgMZEn||(m_bMZHostEn==1) ? MTRUE : MFALSE;

        //Accroding to option, updating center ROI coordinate automatically
        UpdateCenterROI( m_sArea_Center);

        //Reset all focusing window.
        m_sArea_Focusing = m_sArea_APCheck = m_sArea_APCmd = m_sArea_OTFD = m_sArea_HW = m_sArea_Center;
    }
    return S_AF_OK;
}

