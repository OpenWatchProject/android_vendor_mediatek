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
#define LOG_TAG "pd_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                   trace.
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_CAMERA
#endif
//
#if 1
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
#else
#define CAM_TRACE_CALL()
#define CAM_TRACE_NAME(name)
#define CAM_TRACE_BEGIN(name)
#define CAM_TRACE_END()
#define CAM_TRACE_FMT_BEGIN(fmt, arg)
#define CAM_TRACE_FMT_END()
#endif


#define AAA_TRACE_D(fmt, arg...) do{ CAM_TRACE_FMT_BEGIN(fmt, ##arg); } while(0)
#define AAA_TRACE_END_D do{ CAM_TRACE_FMT_END(); } while(0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <sys/stat.h>
#include <stdio.h>

// android related
#include <cutils/properties.h>
#include <utils/threads.h>

// driver related
#include <kd_camera_feature.h>
#include "kd_imgsensor.h"

#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_sensor_buf_mgr.h"
#include <isp_tuning.h>
#include <mtkcam/drv/IHalSensor.h>
#include <kd_imgsensor_define.h>
#include "camera_custom_cam_cal.h"
#include "cam_cal_drv.h"

// pd hal
#include "pd_mgr.h"
#include <pd_buf_mgr_open.h>
#include <pd_buf_mgr.h>
#include <af_feature.h>
#include <Local.h>
#include <camera_custom_nvram.h>
#include <nvbuf_util.h>
#include <camera_custom_raw.h>

//ae_target mode def.
#include <hal/inc/custom/aaa/ae_param.h>

// algorithm
#include <pd_algo_if.h>

// thread
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
#include <mtkcam/utils/common.h>

using namespace NSCam::Utils;
using namespace NSCam;

#define DBG_MSG_BUF_SZ 1024 //byte


namespace NS3A
{
#define DBG_MOVE_LENS_STABLE_FRAME_NUM (5)

/************************************************************************/
/* Calculation                                                          */
/************************************************************************/
//
#define _GET_TIME_STAMP_US_() ({\
    MINT64 _ts = 0;\
    struct timespec t;\
    t.tv_sec = t.tv_nsec = 0;\
    clock_gettime(CLOCK_MONOTONIC, &t);\
    _ts = ((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000;\
    _ts;\
})

//
MINT32 PDMgr::Boundary(MINT32 a_i4Min, MINT32 a_i4Vlu, MINT32 a_i4Max)
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

/************************************************************************/
/* Multi-instance                                                       */
/************************************************************************/
//
PDMgr* PDMgr::s_pPDMgr = MNULL;

//
template <ESensorDev_T const eSensorDev>
class PDMgrDev : public PDMgr
{
public:
    static PDMgr& getInstance()
    {
        static PDMgrDev<eSensorDev> singleton;
        PDMgr::s_pPDMgr = &singleton;
        return singleton;
    }

    PDMgrDev() : PDMgr(eSensorDev) {}
    virtual ~PDMgrDev() {}
};

//
PDMgr &PDMgr::getInstance(MINT32 const i4SensorDev)
{
    switch ( i4SensorDev)
    {
    case ESensorDev_Main:
        return  PDMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond:
        return  PDMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub:
        return  PDMgrDev<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond:
        return  PDMgrDev<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("unknow seneor dev(%d)", i4SensorDev);
        if ( PDMgr::s_pPDMgr)
        {
            return  *PDMgr::s_pPDMgr;
        }
        else
        {
            return  PDMgrDev<ESensorDev_Main>::getInstance();
        }
    }
}

/************************************************************************/
/* Initialization and uninitialization                                  */
/************************************************************************/
PDMgr::PDMgr(ESensorDev_T eSensorDev) :
    m_i4CurrSensorDev((MINT32)eSensorDev),
    m_pIHalSensor(nullptr),
    m_i4CaliAFPos_Inf(0),
    m_i4CaliAFPos_Macro(0),
    m_i4CaliAFPos_50cm(0),
    m_i4CaliAFPos_Middle(0),
    m_databuf_size(0),
    m_databuf_stride(0),
    m_databuf(nullptr),
    m_minLensPos(0),
    m_maxLensPos(0),
    m_u1CaliDaSrc(0)
{
    memset( &m_sCaliData, 0, sizeof(PD_CALIBRATION_DATA_T));
    memset( &m_profile,   0, sizeof(SPDProfile_t));
}
//
PDMgr::~PDMgr()
{}

//
MBOOL PDMgr::CamPwrOnState()
{
    CAM_LOGD("%s+", __FUNCTION__);
    CAM_LOGD("%s-", __FUNCTION__);
    return MTRUE;
}

//
MBOOL PDMgr::CamPwrOffState()
{
    CAM_LOGD("%s+", __FUNCTION__);
    CAM_LOGD("%s-", __FUNCTION__);
    return MTRUE;
}

//
EPDBUF_TYPE_t PDMgr::start(MINT32 i4SensorIdx, SPDProfile_t *iPDProfile, PD_NVRAM_T *ptrInPDNvRam)
{
    PD_NVRAM_T *ptrPDNvRamData   = reinterpret_cast<PD_NVRAM_T *>(ptrInPDNvRam);

    //
    m_i4DbgPDVerifyEn           = property_get_int32("vendor.debug.pd_verify_flow.enable", 0);
    m_i4DbgPdDump               = (MINT32)( (m_i4DbgPDVerifyEn) || (property_get_int32("vendor.pd.dump.enable", 0)));
    m_bDebugEnable              = (MINT32)( (m_i4DbgPdDump) || (m_i4DbgPDVerifyEn) || (property_get_int32("vendor.debug.af_mgr.enable", 0)!=0));
    m_i4DbgDisPdHandle          = property_get_int32("vendor.debug.handlepd.disable", 0);

    CAM_LOGD_IF( m_bDebugEnable,"[%s] +", __FUNCTION__);

    //create folder for saving debug information as m_i4DbgPdDump is set.
    if( m_i4DbgPdDump || m_bDebugEnable)
    {
        FILE *fp = fopen("/sdcard/pdo/pd_mgr_info", "w");
        if( NULL == fp)
        {
            MINT32 err = mkdir( "/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD( "create folder /sdcard/pdo (%d)", err);
        }
        else
        {
            fprintf( fp, "folder /sdcard/pdo is exist");
            fclose( fp);
        }
    }

    //
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    m_pIHalSensor = pIHalSensorList->createSensor( LOG_TAG, m_i4CurrSensorDev);

    //initial member.
    m_frmCnt         = 0;
    m_Status         = EPD_Not_Enabled;
    m_databuf_size   = 0;
    m_databuf_stride = 0;
    m_databuf        = nullptr;
    m_pPDBufMgrCore  = NULL;
    m_pPDBufMgrOpen  = NULL;
    m_pIPdAlgo       = NULL;
    m_i4SensorIdx    = i4SensorIdx;
    m_minLensPos      = 0;
    m_maxLensPos      = 0;

    memset( &m_sAnalysisData,           0, sizeof(S_ANALYSIS_DATA_T));
    memset( &m_sAnalysisDataList,       0, sizeof(S_ANALYSIS_DATA_OPEN_T));
    memset( &m_PDBlockInfo,             0, sizeof(SET_PD_BLOCK_INFO_T));
    memset( &m_SensorCropInfo,          0, sizeof(SensorCropWinInfo));
    memset( &m_SensorVCInfo,            0, sizeof(SensorVCInfo));
    memset( &m_sPDOHWInfo,              0, sizeof(SPDOHWINFO_T));

    m_vPDOutput.clear();

    //get main information from host
    memcpy( &m_profile, iPDProfile, sizeof(SPDProfile_t));

    // initial profile in pd mgr.
    m_profile.BufType = EPDBUF_NOTDEF;

    // get sensor information
    if( m_pIHalSensor)
    {
        MINT32 err1 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_INFO,     (MINTPTR )&(m_profile.i4SensorMode), (MINTPTR )&(m_PDBlockInfo),                  0);
        MINT32 err2 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY, (MINTPTR )&(m_profile.i4SensorMode), (MINTPTR )&(m_profile.bSensorModeSupportPD), 0);
        MINT32 err3 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_VC_INFO,       (MUINTPTR)&(m_SensorVCInfo),         (MUINTPTR)&(m_profile.i4SensorMode),         0);
        MINT32 err4 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&(m_profile.i4SensorMode), (MUINTPTR)&(m_SensorCropInfo),               0);

        if( err1 || err2 || err3 || err4)
        {
            CAM_LOGE( "[%s] sensor command fail (%d/%d/%d/%d)",
                      __FUNCTION__,
                      err1,
                      err2,
                      err3,
                      err4);
        }
    }
    else
    {
        CAM_LOGE("[%s] pIHalSensor is NULL!! Can not get PD_block_info and vc_info from sensor kernel driver", __FUNCTION__);
    }

    CAM_LOGD( "[%s] SensorMode(%d) SensorInfo(offset[%d,%d], pitch[%d,%d], pairNum[%d], subBlkSz[%d,%d], blockNum[%d,%d], longExpFirst[%d], mirrorFlip[%d]) VC2 Info(DataType[0x%x], ID[0x%x], SIZEH[0x%x], SIZEV[0x%x]) CropInfo(FullImgSz[%d, %d], Crop[%d, %d, %d, %d])",
              __FUNCTION__,
              m_profile.i4SensorMode,
              m_PDBlockInfo.i4OffsetX,
              m_PDBlockInfo.i4OffsetY,
              m_PDBlockInfo.i4PitchX,
              m_PDBlockInfo.i4PitchY,
              m_PDBlockInfo.i4PairNum,
              m_PDBlockInfo.i4SubBlkW,
              m_PDBlockInfo.i4SubBlkH,
              m_PDBlockInfo.i4BlockNumX,
              m_PDBlockInfo.i4BlockNumY,
              m_PDBlockInfo.i4LeFirst,
              m_PDBlockInfo.iMirrorFlip,
              m_SensorVCInfo.VC2_DataType,
              m_SensorVCInfo.VC2_ID,
              m_SensorVCInfo.VC2_SIZEH,
              m_SensorVCInfo.VC2_SIZEV,
              m_SensorCropInfo.full_w,
              m_SensorCropInfo.full_h,
              m_SensorCropInfo.x0_offset,
              m_SensorCropInfo.y0_offset,
              m_SensorCropInfo.w0_size,
              m_SensorCropInfo.h0_size);

    // Initial PD related HW module.
    ConfigurePDHWSetting( &m_profile, &(ptrPDNvRamData->rTuningData));

    // inital pd buffer manager and pd algo once PD is supported in current sensor mode.
    if( m_profile.bSensorModeSupportPD)
    {

        //get pd buffer type from custom setting.
        MUINT32 pdBufType = GetPDBuf_Type( m_i4CurrSensorDev, m_profile.i4CurrSensorId);

        //
        IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM PDAF_support_type = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)m_profile.u4PDAF_support_type;

        /*--------------------------------------------------------------------------------------------------------
         *                                                  (1)
         *               Maching PD buffer manager setting and kernel image sensor driver setting
         *--------------------------------------------------------------------------------------------------------*/
        m_profile.BufType = EPDBUF_NOTDEF;
        switch( PDAF_support_type)
        {
        case PDAF_SUPPORT_RAW_LEGACY:
            pdBufType &= (MSK_CATEGORY | MSK_PDBUF_TYPE_RAW_LEGACY);
            if( pdBufType == EPDBUF_RAW_LEGACY)
            {
                m_profile.BufType = (EPDBUF_TYPE_t)pdBufType;
            }
            break;

        case PDAF_SUPPORT_RAW:
            pdBufType &= (MSK_CATEGORY | MSK_PDBUF_TYPE_PDO);
            if( pdBufType == EPDBUF_PDO)
            {
                // checking dependency module constraint
                /*if( m_profile.u4IsFrontalBinning == 1)
                {
                    CAM_LOGD( "Frontal binning should be disabled.");
                }
                else */if( m_profile.u4PDPipeCtrl != FEATURE_PDAF_SUPPORT_BNR_PDO)
                {
                    CAM_LOGE( "[%s] support PDO but pipe(stt) didn't initialized.", __FUNCTION__);
                }
                else
                {
                    m_profile.BufType = (EPDBUF_TYPE_t)pdBufType;
                }
            }
            break;

        case PDAF_SUPPORT_CAMSV:
            pdBufType &= (MSK_CATEGORY | MSK_PDBUF_TYPE_VC);
            if( pdBufType == EPDBUF_VC || pdBufType == EPDBUF_VC_OPEN)
            {
                // checking dependency module constraint
                if( m_profile.u4PDPipeCtrl != FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
                {
                    CAM_LOGE( "[%s] support PDVC but pipe(camsv) didn't initialized.", __FUNCTION__);
                }
                else
                {
                    if( pdBufType == EPDBUF_VC_OPEN)
                    {
                        //
                        if( m_databuf)
                        {
                            CAM_LOGE( "[%s] should not be happened!! someting wrong during last time stop pd_mgr flow", __FUNCTION__);
                            delete m_databuf;
                        }

                        //
                        m_databuf        = nullptr;
                        m_databuf_size   = 0;
                        m_databuf_stride = 0;
                    }
                    m_profile.BufType = (EPDBUF_TYPE_t)pdBufType;
                }
            }
            break;

        case PDAF_SUPPORT_RAW_DUALPD:
            pdBufType &= (MSK_CATEGORY | EPDBUF_PDO);
            if( pdBufType == EPDBUF_DUALPD_RAW)
            {
                // checking dependency module constraint
                if( m_profile.u4PDPipeCtrl != FEATURE_PDAF_SUPPORT_PBN_PDO)
                {
                    CAM_LOGE( "[%s] support DualPD_Raw but pipe(stt) didn't initialized.", __FUNCTION__);
                }
                else if( !m_profile.bEnablePBIN)
                {
                    CAM_LOGE( "[%s] PBIN from 3A (%d) is not match with current PD sensor type (%d)", __FUNCTION__, m_profile.bEnablePBIN, PDAF_support_type);
                }
                else
                {
                    m_profile.BufType = (EPDBUF_TYPE_t)pdBufType;
                }
            }
            break;

        case PDAF_SUPPORT_CAMSV_DUALPD:
            pdBufType &= (MSK_CATEGORY | MSK_PDBUF_TYPE_VC);
            if( pdBufType == EPDBUF_DUALPD_VC)
            {
                // checking dependency module constraint
                if( m_profile.u4PDPipeCtrl != FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
                {
                    CAM_LOGE( "[%s] support DualPD_VC but pipe(camsv) didn't initialized.", __FUNCTION__);
                }
                else
                {
                    m_profile.BufType = (EPDBUF_TYPE_t)pdBufType;
                }
            }
            break;

        default:
            CAM_LOGE( "[%s] Unknown PD sensor type (%d)", __FUNCTION__, PDAF_support_type);
            break;
        }


        /*--------------------------------------------------------------------------------------------------------
         *                                                  (2)
         *                                        Create related instance
         *--------------------------------------------------------------------------------------------------------*/
        if( m_profile.BufType == EPDBUF_NOTDEF)
        {
            // related instance is initialized, do nothing.
        }
        else if( m_profile.BufType & MSK_CATEGORY_OPEN)
        {
            m_pPDBufMgrOpen = ::PDBufMgrOpen::createInstance( m_profile);
        }
        else if( m_profile.BufType & MSK_CATEGORY_DUALPD)
        {
            m_pPDBufMgrCore = ::PDBufMgr::createInstance( m_profile);
            m_pIPdAlgo      = NS3A::IPdAlgo::createInstance( m_i4CurrSensorDev, PDType_DualPD);
        }
        else
        {
            m_pPDBufMgrCore = ::PDBufMgr::createInstance( m_profile);
            m_pIPdAlgo      = NS3A::IPdAlgo::createInstance( m_i4CurrSensorDev, PDType_Legacy);
        }

        //
        MBOOL instanceRdy = (m_pPDBufMgrOpen) || (m_pPDBufMgrCore && m_pIPdAlgo) ? MTRUE : MFALSE;

        /*--------------------------------------------------------------------------------------------------------
         *                                                  (3)
         *                     Check PD mgr initial status to deside thread should be created or not.
         *                     After related instance and PD thread is created, load calibration data.
         *--------------------------------------------------------------------------------------------------------*/
        if( instanceRdy)
        {
            //start calculating thread.
            createThread();

            if( m_bEnPDBufThd)
            {
                // Set PD calibration data and tuning data.
                if( setPDCaliData( ptrPDNvRamData)==S_3A_OK)
                {
                    // Get parameters for verification flow :
                    // switch for executing flow.
                    m_i4DbgPDVerifyRun          = property_get_int32("vendor.debug.pd_verify_flow.run", 0);
                    // calculate one step size.
                    m_i4DbgPDVerifyStepSize     = (m_i4CaliAFPos_Macro-m_i4CaliAFPos_Inf)/( Boundary( 10, property_get_int32("vendor.debug.pd_verify_flow.StepNum", 10), 30));
                    // calculation times for each step
                    m_i4DbgPDVerifyCalculateNum = (DBG_MOVE_LENS_STABLE_FRAME_NUM)+( Boundary( 10, property_get_int32("vendor.debug.pd_verify_flow.CalNum", 10), 20));
                    // reset parameter
                    m_i4DbgPDVerifyCalculateCnt = 0;

                    if( /* checking pd verification can be enabled or not*/
                        (m_i4DbgPDVerifyEn                     )&&
                        (0<m_i4DbgPDVerifyCalculateNum         )&&
                        (m_i4CaliAFPos_Inf!=m_i4CaliAFPos_Macro)&&
                        (m_i4DbgPDVerifyStepSize!=0            )&&
                        (0<m_i4DbgPDVerifyCalculateNum         )&&
                        (m_i4DbgPDVerifyCalculateNum<1023      ))
                    {
                        property_set("vendor.debug.pdmgr.lockae", "0");
                        property_set("vendor.debug.af_motor.disable", "1");

                        /* m_i4CaliAFPos are initialized and checked in setPDCaliData function. Set m_i4CaliAFPos_Inf as initial position*/
#define PROPERTY_VALUE_MAX 10
                        char value[PROPERTY_VALUE_MAX] = {'\0'};
                        snprintf( value,
                                  PROPERTY_VALUE_MAX,
                                  "%d",
                                  m_i4CaliAFPos_Inf + (m_i4DbgPDVerifyCalculateCnt/m_i4DbgPDVerifyCalculateNum)*m_i4DbgPDVerifyStepSize);
                        property_set("vendor.debug.af_motor.position", value);

                        CAM_LOGD( "[%s] DBG (verification flow) :  check initial value lockae(%d) Motordisable(%d) motorPos(%d)",
                                  __FUNCTION__,
                                  property_get_int32("vendor.debug.pdmgr.lockae", 0),
                                  property_get_int32("vendor.debug.af_motor.disable", 0),
                                  property_get_int32("vendor.debug.af_motor.position", 0));


                    }
                    else
                    {
                        m_i4DbgPDVerifyEn = 0;
                    }
                    CAM_LOGD( "[%s] DBG (verification flow) :  enable(%d) step size(%d) calculation frame interval (%d) frames",
                              __FUNCTION__,
                              m_i4DbgPDVerifyEn,
                              m_i4DbgPDVerifyStepSize,
                              m_i4DbgPDVerifyCalculateNum);
                }
                else
                {
                    CAM_LOGE( "[%s] set PD calibration and tuning data fail, close PD thread !! Please check previous error log !!", __FUNCTION__);
                    closeThread();
                    m_profile.BufType = EPDBUF_NOTDEF;
                }
            }
        }
        else
        {
            m_profile.BufType = EPDBUF_NOTDEF;
            CAM_LOGD( "[%s] Instance is not ready : PDBufMgrCore(%p) PDAlgo(%p) PDBufMgrOpen(%p)", __FUNCTION__, (void*)m_pPDBufMgrCore, (void*)m_pIPdAlgo, (void*)m_pPDBufMgrOpen);
        }
    }
    else
    {
        m_profile.BufType = EPDBUF_NOTDEF;
        CAM_LOGD("[%s] Current sensor mode is not support PD", __FUNCTION__);
    }

    CAM_LOGD( "[%s] SensorID(0x%x), PDAF_Support(%d), SensorModeSupportPD(%d), BufType(0x%02x) Note:(NOTDEF[0x00] VC[0x01] VC_OPEN[0x11] RAW_LEGACY[0x02] RAW_LEGACY_OPEN[0x12] PDO[0x04] PDO_OPEN[0x14] DUALPD_VC[0x21] DUALPD_RAW[0x24]",
              __FUNCTION__,
              m_profile.i4CurrSensorId,
              m_profile.u4PDAF_support_type,
              m_profile.bSensorModeSupportPD,
              m_profile.BufType);

    //==========
    // Output
    //==========
    iPDProfile->BufType = m_profile.BufType;

    CAM_LOGD_IF( m_bDebugEnable,"[%s] - return pd buffer manager type[0x%02x]", __FUNCTION__, m_profile.BufType);

    return m_profile.BufType;
}

//
MRESULT PDMgr::stop()
{
    CAM_LOGD("%s+", __FUNCTION__);

    if( m_pIHalSensor)
    {
        m_pIHalSensor->destroyInstance(LOG_TAG);
        m_pIHalSensor = nullptr;
    }

    if( m_bEnPDBufThd)
    {
        //close thread
        closeThread();
    }

    //
    while(1)
    {
        Mutex::Autolock lock(m_Lock_PDWinCfg);

        S_ANALYSIS_DATA_OPEN_T *del_element = m_sAnalysisDataList.head;

        if( del_element==NULL)
        {
            m_sAnalysisDataList.tail = NULL;
            CAM_LOGD( "[%s] all resource in data list are destroyed : sz(%d) prv(%p) nxt(%p) head(%p) tail(%p)",
                      __FUNCTION__,
                      m_sAnalysisDataList.totalsz,
                      m_sAnalysisDataList.prv,
                      m_sAnalysisDataList.nxt,
                      m_sAnalysisDataList.head,
                      m_sAnalysisDataList.tail);
            break;
        }

        m_sAnalysisDataList.head = del_element->nxt;
        m_sAnalysisDataList.nxt  = del_element->nxt;
        m_sAnalysisDataList.totalsz--;

        delete del_element;
    }

    //uninit member
    m_databuf_size=0;
    m_databuf_stride = 0;

    if( m_databuf)
        delete m_databuf;
    m_databuf=nullptr;

    m_pPDBufMgrCore = NULL;
    m_pPDBufMgrOpen = NULL;

    m_pIPdAlgo = NULL;

    {
        Mutex::Autolock lock(m_Lock_Result);
        m_vPDOutput.clear();
    }

    if( m_i4DbgPDVerifyEn)
    {
        property_set("vendor.debug.pd_verify_flow.run", "0");
        property_set("vendor.debug.pd_verify_flow.enable", "0");
        property_set("vendor.debug.pdmgr.lockae", "0");
        property_set("vendor.debug.af_motor.disable", "0");
    }

    CAM_LOGD("%s-", __FUNCTION__);

    return S_3A_OK;
}


//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::ConfigurePDHWSetting(SPDProfile_t *iPDProfile, PD_ALGO_TUNING_T *ptrInTuningData)
{
    MRESULT ret=E_3A_ERR;

    if( iPDProfile->u4PDPipeCtrl==FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
    {
        if( iPDProfile->BufType==EPDBUF_VC_OPEN)
        {
            S_ANALYSIS_DATA_OPEN_T *last_cfg = m_sAnalysisDataList.tail;
            if( last_cfg)
            {
                // query sensor driver setting for 3rd party PD library.
                MUINT32  arg2=0;
                MUINT16 *arg3=NULL;
                MBOOL    res  = MFALSE;

                if( m_pPDBufMgrOpen)
                {
                    AAA_TRACE_D("GetRegSetting");

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "%s : send command PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING to pd buffer manager (open) +",
                                 __FUNCTION__);

                    res  = m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING,
                                                         (MVOID *)(last_cfg),
                                                         (MVOID *)(&arg2),
                                                         (MVOID *)(&arg3));

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "%s : send command PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING to pd buffer manager (open) - result(%d) size(%d) addr(%p)",
                                 __FUNCTION__,
                                 res,
                                 arg2,
                                 arg3);

                    AAA_TRACE_END_D;
                }

                if( res && m_pIHalSensor && arg2 && arg3)
                {
                    AAA_TRACE_D("SendCmd2Sensor");

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "%s : send command SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING to IHalSensor +",
                                 __FUNCTION__);

                    MINT64 timestamp0 = _GET_TIME_STAMP_US_();
                    m_pIHalSensor->sendCommand( m_i4CurrSensorDev,
                                                SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING,
                                                (MUINTPTR)&arg2,
                                                (MUINTPTR)&arg3,
                                                0);
                    MINT64 timestamp1 = _GET_TIME_STAMP_US_();

                    if( timestamp1-timestamp0 > 10*1000)
                    {
                        CAM_LOGW( "%s : performance issue !! Time duration of command SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING is over 10ms, Please check that burst writing I2C mode is used in command SENSOR_FEATURE_SET_PDAF_REG_SETTING int image sensor kernel driver !!", __FUNCTION__);
                    }

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "%s : send command SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING to IHalSensor -",
                                 __FUNCTION__);

                    AAA_TRACE_END_D;
                }
                else
                {
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "%s Can not get PD window HW setting. Use setting in kernel driver. Send command result(%d) : size(%d), addr(%p)",
                                 __FUNCTION__,
                                 res,
                                 arg2,
                                 arg3);
                }
            }
            else
            {
                CAM_LOGE( "%s pd configuration to sensor is not vaild", __FUNCTION__);
            }
        }
        else
        {
            // Do nothing.
            ret = E_3A_ERR;
        }
    }
    else if( (iPDProfile->u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_PBN_PDO) || (iPDProfile->u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_BNR_PDO))
    {
        //
        IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM PDAF_support_type = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)iPDProfile->u4PDAF_support_type;

        if( PDAF_support_type == PDAF_SUPPORT_RAW)
        {
#if 0
            // Get BPCI table information for PDO
            Tbl bpci;
            if( ::IPDTblGen::getInstance()->getTbl( m_i4CurrSensorDev, iPDProfile->i4SensorMode, bpci) == MFALSE)
            {
                CAM_LOGE("Failed acquiring BPCI table!");
                ret = E_3A_ERR;
            }
            else
            {
                m_sPDOHWInfo.u1IsDualPD      = 0;
                m_sPDOHWInfo.u4BitDepth      = 10; /* Expect bit depth of PDO port*/
                m_sPDOHWInfo.u4Bpci_xsz      = bpci.tbl.tbl_xsz;
                m_sPDOHWInfo.u4Bpci_ysz      = bpci.tbl.tbl_ysz;
                m_sPDOHWInfo.u4Pdo_xsz       = bpci.tbl.pdo_xsz;
                m_sPDOHWInfo.u4Pdo_ysz       = bpci.tbl.pdo_ysz;
                m_sPDOHWInfo.phyAddrBpci_tbl = (MUINTPTR)bpci.tbl.tbl_pa;
                m_sPDOHWInfo.virAddrBpci_tbl = (MUINTPTR)bpci.tbl.tbl_va;
                m_sPDOHWInfo.i4memID         = bpci.tbl.memID;

                CAM_LOGD( "%s BPCI table first 4 dword: %x %x %x %x",
                          __FUNCTION__,
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[0],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[1],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[2],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[3]);

                ret = S_3A_OK;
            }

            // TODO : remove it
            // Read BPCI table related information from BNR2 ISP module.
            MUINT32  u4Bpci_xsz=0, u4Bpci_ysz=0, u4Pdo_xsz=0, u4Pdo_ysz=0;
            MUINTPTR phyAddrBpci_tbl, virAddrBpci_tbl;
            MINT32   memID=0;
            MBOOL isBPCItbl = ISP_MGR_BNR2::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getBPCItable(
                                  u4Bpci_xsz,
                                  u4Bpci_ysz,
                                  u4Pdo_xsz,
                                  u4Pdo_ysz,
                                  phyAddrBpci_tbl,
                                  virAddrBpci_tbl,
                                  memID,
                                  eIDX_PDE);


            if( bpci.tbl.tbl_xsz != u4Bpci_xsz)
            {
                CAM_LOGD("error: tbl_xsz is not the same %d %d\n", bpci.tbl.tbl_xsz, u4Bpci_xsz);
            }
            else if( bpci.tbl.tbl_ysz != u4Bpci_ysz)
            {
                CAM_LOGD("error: tbl_ysz is not the same %d %d\n", bpci.tbl.tbl_ysz, u4Bpci_ysz);
            }
            else if( bpci.tbl.pdo_xsz != u4Pdo_xsz)
            {
                CAM_LOGD("error: pdo_xsz is not the same %d %d\n", bpci.tbl.pdo_xsz, u4Pdo_xsz);
            }
            else if( bpci.tbl.pdo_ysz != u4Pdo_ysz)
            {
                CAM_LOGD("error: pdo_ysz is not the same %d %d\n", bpci.tbl.pdo_ysz, u4Pdo_ysz);
            }
            else
            {
                bool isMatch = true;
                char *ptr0 = (char*)(bpci.tbl.tbl_va);
                char *ptr1 = (char*)(virAddrBpci_tbl);
                for( int i=0; i<(bpci.tbl.tbl_xsz)*(bpci.tbl.tbl_ysz); i++)
                {
                    if( ptr1[i]!=ptr0[i])
                    {
                        isMatch = false;
                        CAM_LOGD("error: element %d is not the same %d %d\n", i, ptr0[i], ptr1[i]);
                    }
                }

                if( isMatch)
                {
                    CAM_LOGD("!! Match !!\n");
                }
            }
#endif
        }
        else if( PDAF_support_type == PDAF_SUPPORT_RAW_DUALPD)
        {
            MINT32 binningX     = 0;
            MINT32 binningY     = 0;
            MINT32 needShortExp = 0;

            if( ptrInTuningData)
            {
                binningX     = ptrInTuningData->i4Reserved[0];
                binningY     = ptrInTuningData->i4Reserved[1];
                needShortExp = ptrInTuningData->i4Reserved[2];
            }
            if( /* error check */
                (binningX > 8) ||
                (binningY > 8) ||
                (needShortExp > 2) ||
                (needShortExp < 0) ||
                (binningX < 1) ||
                (binningY < 1))
            {
                CAM_LOGW( "%s, dualpd tuning data is wrong !! binningX(%d) binningY(%d) needShortExp(%d), Please check PD tuning parameters at index 0,1,2 in reserved array",
                          __FUNCTION__,
                          binningX,
                          binningY,
                          needShortExp);
                binningX     = 8;
                binningY     = 8;
                needShortExp = 0;
            }

            m_sPDOHWInfo.u1IsDualPD  = 1;
            m_sPDOHWInfo.u4BitDepth  = 12; /* Expect bit depth of PDO port*/
            m_sPDOHWInfo.u1PBinType  = (iPDProfile->AETargetMode == AE_MODE_IVHDR_TARGET) ? 0 : 1; /*0:1x4, 1:4x4*/
            m_sPDOHWInfo.u4BinRatioX = binningX;
            m_sPDOHWInfo.u4BinRatioY = binningY;

            if( iPDProfile->uPdSeparateMode==1)
            {
                m_sPDOHWInfo.u1PdSeparateMode = 1;
                m_sPDOHWInfo.u4Pdo_xsz        = ((iPDProfile->uImgXsz / m_sPDOHWInfo.u4BinRatioX)*sizeof(MUINT16))-1;
                m_sPDOHWInfo.u4Pdo_ysz        = ((iPDProfile->uImgYsz / m_sPDOHWInfo.u4BinRatioY)*2)- 1; // 2 : L and R
            }
            else if( iPDProfile->uPdSeparateMode==0)
            {
                m_sPDOHWInfo.u1PdSeparateMode = 0;
                m_sPDOHWInfo.u4Pdo_xsz        = ((iPDProfile->uImgXsz / m_sPDOHWInfo.u4BinRatioX)*sizeof(MUINT16)*2)-1; //2 : L and R
                m_sPDOHWInfo.u4Pdo_ysz        = ((iPDProfile->uImgYsz / m_sPDOHWInfo.u4BinRatioY)- 1);
            }
            else
            {
                CAM_LOGE("[%s]- separate mode setting ERROR!", __FUNCTION__);
            }

            m_sPDOHWInfo.u1PBinStartLine = (needShortExp                   ) ?
                                           (m_PDBlockInfo.i4LeFirst ? 1 : 0) :
                                           (m_PDBlockInfo.i4LeFirst ? 0 : 1);
            m_sPDOHWInfo.u1PBinStartLine +=  (m_sPDOHWInfo.u1PBinType == 1) ? 0 : 2; // in 1x4, start line + 2

        }


        //configure ISP PD related HW module, if PDO port is configured to sttPipe by 3A framework.
        MUINT32 expect_bitDepth = m_sPDOHWInfo.u4BitDepth;


        /**********************************************************************************************************************************************
         *
         * Notice :
         *   No matter if m_pPDBufMgrCore or m_pIPdAlgo is NULL, need to config PDO HW once
         *   when u4PDPipeCtrl is FEATURE_PDAF_SUPPORT_PBN_PDO or FEATURE_PDAF_SUPPORT_BNR_PDO, otherwise system crash.
         *
         *   Real bitDepth of PD pixel data will be judged inside ISP_MGR_PDO_CONFIG_T.
         *
         ***********************************************************************************************************************************************/
        CAM_LOGD( "%s pdaf_support_type(%d), PDOHWInfo : bpci_tbl_sz(0x%x,0x%x) bpci_tbl_info(pa[%p],va[%p],memID[%d]) pdo_sz(0x%x,0x%x) bitDepth(%d->%d) IsDualPD(%d) PBinType(%d - 0:1x4, 1:4x4) PBinStartLine(%d) PdSeparateMode(%d)",
                  __FUNCTION__,
                  PDAF_support_type,
                  m_sPDOHWInfo.u4Bpci_xsz,
                  m_sPDOHWInfo.u4Bpci_ysz,
                  (void*)m_sPDOHWInfo.phyAddrBpci_tbl,
                  (void*)m_sPDOHWInfo.virAddrBpci_tbl,
                  m_sPDOHWInfo.i4memID,
                  m_sPDOHWInfo.u4Pdo_xsz,
                  m_sPDOHWInfo.u4Pdo_ysz,
                  expect_bitDepth,
                  m_sPDOHWInfo.u4BitDepth,
                  m_sPDOHWInfo.u1IsDualPD,
                  m_sPDOHWInfo.u1PBinType,
                  m_sPDOHWInfo.u1PBinStartLine,
                  m_sPDOHWInfo.u1PdSeparateMode);
    }
    else
    {
        // Do nothing.
        ret = E_3A_ERR;
    }

    return ret;
}

/************************************************************************/
/* PD calculation thread                                                */
/************************************************************************/
//
MVOID PDMgr::createThread()
{
    CAM_LOGD( "create PD buffer calculation thread");
    //create thread
    m_bEnPDBufThd = MTRUE;
    sem_init(&m_semPDBuf, 0, 0);
    sem_init(&m_semPDBufThdEnd, 0, 1);
    pthread_create( &m_PDBufThread, NULL, PDBufThreadLoop, this);
}

//
MVOID PDMgr::closeThread()
{
    CAM_LOGD("close PD buffer calculation thread");
    //close thread
    m_bEnPDBufThd = MFALSE;
    ::sem_post(&m_semPDBuf);
    pthread_join( m_PDBufThread, NULL);
}

//
MVOID PDMgr::changePDBufThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"PDBufThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AF;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //

        CAM_LOGD_IF( m_bDebugEnable,
                     "[PDMgr::PDBufThreadLoop] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)",
                     expect_policy,
                     policy,
                     expect_priority,
                     priority);
    }

}

//
MVOID* PDMgr::PDBufThreadLoop(MVOID *arg)
{
    MRESULT ret=E_3A_ERR;

    PDMgr *_this = reinterpret_cast<PDMgr*>(arg);

    CAM_LOGD( "[%s] thread start", __FUNCTION__);

    // (1) change thread setting
    _this->changePDBufThreadSetting();
    _this->m_Status = EPD_Init;


    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait(&_this->m_semPDBuf);
        if ( ! _this->m_bEnPDBufThd)
        {
            break;
        }

        _this->m_Status = EPD_BZ;

        CAM_LOGD_IF( _this->m_bDebugEnable, "#(%d) [%s] + run calculation task, dev:%d", _this->m_MagicNumber, __FUNCTION__, _this->m_i4CurrSensorDev);

        AAA_TRACE_D( "PDCalculation #(%d)", _this->m_MagicNumber);
        //run core pd flow to get pd algorithm result.
        switch( _this->m_profile.BufType)
        {
        case EPDBUF_VC :
        case EPDBUF_RAW_LEGACY :
        case EPDBUF_PDO :
        case EPDBUF_DUALPD_VC:
        case EPDBUF_DUALPD_RAW:
            ret = _this->PDCoreFlow();
            break;

        case EPDBUF_VC_OPEN :
        case EPDBUF_RAW_LEGACY_OPEN :
        case EPDBUF_PDO_OPEN :
            ret = _this->PDOpenCoreFlow();
            break;

        default :
            break;

        }
        CAM_LOGD_IF( _this->m_bDebugEnable, "#(%d) [%s] - run calculation task, dev:%d", _this->m_MagicNumber, __FUNCTION__, _this->m_i4CurrSensorDev);


        //verification flow
        if( _this->m_i4DbgPDVerifyEn)
        {
            _this->PDVerificationFlow();
        }

        //thread control
        {
            int Val;
            ::sem_getvalue(&_this->m_semPDBufThdEnd, &Val);

            if( Val==0) //should be 0. 1 means PD task done(abnormal case).
            {
                CAM_LOGD_IF( _this->m_bDebugEnable,
                             "#(%d) [%s] semPDThdEnd before post is [0] : normal case",
                             _this->m_MagicNumber,
                             __FUNCTION__,
                             Val);

                //
                ::sem_post(&_this->m_semPDBufThdEnd);
            }
            else
            {
                CAM_LOGE( "#(%d) [%s] semPDThdEnd before post is [%d] : Abnormal case. Something wrong is happened during about multi thread communication !!",
                          _this->m_MagicNumber,
                          __FUNCTION__,
                          Val);
            }
        }

        //ready output data
        _this->m_Status = EPD_Data_Ready;

        //check pd flow status.
        if(ret==E_3A_ERR)
        {
            CAM_LOGE( "#(%d) [%s] PD calculation thread is colsed because of some is happened during calculation !!!", _this->m_MagicNumber, __FUNCTION__);
            break;
        }
        AAA_TRACE_END_D;
    }

    _this->m_bEnPDBufThd = MFALSE;
    _this->m_Status = EPD_Not_Enabled;


    CAM_LOGD( "[%s] thread end", __FUNCTION__);
    return NULL;
}

/************************************************************************/
/* Data path 1 : using protect PD algorithm                             */
/************************************************************************/
MRESULT PDMgr::SetCaliData2PDCore(PD_ALGO_TUNING_T* ptrInTuningData, PD_CALIBRATION_DATA_T* ptrInCaliData)
{
    CAM_LOGD( "[%s] +", __FUNCTION__);

    //
    MRESULT ret = E_3A_ERR;
    MRESULT retAll = S_3A_OK;

    //
    PD_INIT_T initPdData;

    //get calibration data
    initPdData.rPDNVRAM.rCaliData.i4Size = ptrInCaliData->i4Size;
    memcpy( initPdData.rPDNVRAM.rCaliData.uData, ptrInCaliData->uData, ptrInCaliData->i4Size);

    //get tuning data from host
    memcpy( &initPdData.rPDNVRAM.rTuningData, ptrInTuningData, sizeof(PD_ALGO_TUNING_T));

    //
    if( m_bDebugEnable)
    {
        FILE *fp = NULL;

        fp = fopen("/sdcard/pdo/__pd_cali_data.bin", "w");
        fwrite( reinterpret_cast<void *>(initPdData.rPDNVRAM.rCaliData.uData), 1, PD_CALI_DATA_SIZE, fp);
        fclose( fp);

        fp = fopen("/sdcard/pdo/__pd_tuning_data.bin", "w");
        fwrite( reinterpret_cast<void *>(&initPdData.rPDNVRAM.rTuningData), 1, sizeof(PD_ALGO_TUNING_T), fp);
        fclose( fp);
    }


    CAM_LOGD( "[%s] tuning data(size=%zu) : ConfThr=%d, SaturateLevel=%d, i4SaturateThr=%d, FocusPDSizeX=%d, FocusPDSizeY=%d",
              __FUNCTION__,
              sizeof(NVRAM_LENS_PARA_STRUCT),
              initPdData.rPDNVRAM.rTuningData.i4ConfThr,
              initPdData.rPDNVRAM.rTuningData.i4SaturateLevel,
              initPdData.rPDNVRAM.rTuningData.i4SaturateThr,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeX,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeY);

    char strMsgBuf[256];
    char* strTarget = strMsgBuf;

    strTarget += sprintf(strTarget, "[%s] ConfIdx1( ", __FUNCTION__);
    for (MINT32 i=0; i<PD_CONF_IDX_SIZE; i++)
        strTarget += sprintf(strTarget, " %d", initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[i]);
    strTarget += sprintf(strTarget, ") ");

    strTarget += sprintf(strTarget, "[%s] ConfIdx2( ", __FUNCTION__);
    for (MINT32 i=0; i<PD_CONF_IDX_SIZE; i++)
        strTarget += sprintf(strTarget, " %d", initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[i]);
    strTarget += sprintf(strTarget, ") ");
    CAM_LOGD( "%s", strMsgBuf);

    for( MINT32 i=0; i<(PD_CONF_IDX_SIZE+1); i++)
    {
        strTarget = strMsgBuf;
        strTarget += sprintf(strTarget, "[%s] ConfTbl %02d( ",__FUNCTION__, i);
        for (MINT32 j=0; j<(PD_CONF_IDX_SIZE+1); j++)
            strTarget += sprintf(strTarget, "%3d ", initPdData.rPDNVRAM.rTuningData.i4ConfTbl[i][j]);
        strTarget += sprintf(strTarget, ")");
        CAM_LOGD( "%s", strMsgBuf);
    }


    //(1) set calibration data to PD core.
    if( (ret = m_pIPdAlgo->initPD(initPdData)) != S_3A_OK)
    {
        CAM_LOGE( "[%s] initPD fail. PD block info which is described in alibration data and kernel driver is mismatch. Please check image sensor kernel driver setting or cam_cal setting!!", __FUNCTION__);

        for( MINT32 kidx=0; kidx<PD_CALI_DATA_SIZE; kidx=kidx+32)
        {
            strTarget = strMsgBuf;
            for (MINT i=0; i<32; i++)
                strTarget += sprintf(strTarget, "0x%02x ", initPdData.rPDNVRAM.rCaliData.uData[kidx+i]);
            CAM_LOGE( "%s", strMsgBuf);
        }
        retAll = E_3A_ERR;
    }

    //(2) configure pd orientation information.
    ePDWIN_ORIENTATION_T PDOrientation;
    switch( m_PDBlockInfo.iMirrorFlip)
    {
    /* 0:IMAGE_NORMAL,1:IMAGE_H_MIRROR,2:IMAGE_V_MIRROR,3:IMAGE_HV_MIRROR*/
    case IMAGE_HV_MIRROR :
        PDOrientation = ePDWIN_M1F1;
        break;
    case IMAGE_V_MIRROR:
        PDOrientation = ePDWIN_M0F1;
        break;
    case IMAGE_H_MIRROR:
        PDOrientation = ePDWIN_M1F0;
        break;
    case IMAGE_NORMAL:
    default :
        PDOrientation = ePDWIN_M0F0;
        break;
    }
    CAM_LOGD( "[%s] mirror flip info : kernel(%d):{[0]IMAGE_NORMAL [1]IMAGE_H_MIRROR [2]IMAGE_V_MIRROR [3]IMAGE_HV_MIRROR} set to algo(%d):{[0]ePDWIN_M0F0 [1]ePDWIN_M0F1 [2]ePDWIN_M1F0 [3]ePDWIN_M1F1}",
              __FUNCTION__,
              m_PDBlockInfo.iMirrorFlip,
              PDOrientation);

    if ((ret = m_pIPdAlgo->setPDOrientation(PDOrientation)) != S_3A_OK)
        retAll = E_3A_ERR;


    //(3) configure pd algorithm.
    PD_CONFIG_T algCfg;
    memset (&algCfg, 0, sizeof(PD_CONFIG_T));

    algCfg.i4RawWidth       = m_profile.uImgXsz;
    algCfg.i4RawHeight      = m_profile.uImgYsz;
    algCfg.i4FullRawWidth   = m_SensorCropInfo.full_w;
    algCfg.i4FullRawHeight  = m_SensorCropInfo.full_h;
    algCfg.i4FullRawXOffset = m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][0];
    algCfg.i4FullRawYOffset = m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][1];

#define USE_DRV_PD_COORDINATOR 1  //[TODO] set as 0
#if USE_DRV_PD_COORDINATOR
    algCfg.i4DRVBlockInfoUseEn = 1;
#else
    algCfg.i4DRVBlockInfoUseEn = 0;
#endif

    switch( m_profile.u4PDAF_support_type)
    {
    case PDAF_SUPPORT_RAW_DUALPD:
    {
        //
        algCfg.sDPdFormat.i4Bits     = m_sPDOHWInfo.u4BitDepth;
        algCfg.sDPdFormat.i4BinningX = m_sPDOHWInfo.u4BinRatioX;
        algCfg.sDPdFormat.i4BinningY = m_sPDOHWInfo.u4BinRatioY;
        algCfg.sDPdFormat.i4BufFmt   = (m_sPDOHWInfo.u1PdSeparateMode == 1) ? 1 : 0;
        //
        algCfg.sDPdFormat.i4BufStride = _PDO_STRIDE_ALIGN_(m_sPDOHWInfo.u4Pdo_xsz+1);
        algCfg.sDPdFormat.i4BufHeight = m_sPDOHWInfo.u4Pdo_ysz + 1;
    }
    break;

    case PDAF_SUPPORT_CAMSV_DUALPD:
    {
        //
        m_pPDBufMgrCore->GetDualPDVCInfo( 0, m_sDualPDVCInfo, m_profile.AETargetMode);

        algCfg.sDPdFormat.i4Bits = 10;
        //
        algCfg.sDPdFormat.i4BinningX = m_sDualPDVCInfo.u4VCBinningX;
        algCfg.sDPdFormat.i4BinningY = m_sDualPDVCInfo.u4VCBinningY;
        algCfg.sDPdFormat.i4BufFmt   = m_sDualPDVCInfo.u4VCBufFmt;
        //
        algCfg.sDPdFormat.i4BufStride = m_SensorVCInfo.VC2_SIZEH;
        algCfg.sDPdFormat.i4BufHeight = m_SensorVCInfo.VC2_SIZEV;
    }
    break;

    case PDAF_SUPPORT_RAW:
    case PDAF_SUPPORT_CAMSV:
    case PDAF_SUPPORT_CAMSV_LEGACY:
    {
        algCfg.i4Bits = 10;
    }
    case PDAF_SUPPORT_RAW_LEGACY:
    {
        algCfg.i4Bits = get_raw_bit_depth();
    }
    default:
    {
        /* Condition checking */
        if( m_PDBlockInfo.i4BlockNumX == 0 && m_PDBlockInfo.i4BlockNumY == 0)
        {
            CAM_LOGE( "Please check block number setting in kernel driver: i4BlockNumX(%d) i4BlockNumY(%d)", m_PDBlockInfo.i4BlockNumX, m_PDBlockInfo.i4BlockNumY);
            retAll = E_3A_ERR;
        }

        if( m_PDBlockInfo.i4PairNum > MAX_PD_PAIR_NUM)
        {
            CAM_LOGE( "Please check pair number setting in kernel driver: i4PairNum(%d)", m_PDBlockInfo.i4PairNum);
            m_PDBlockInfo.i4PairNum = MAX_PD_PAIR_NUM;
        }

        algCfg.i4IsPacked  = 1;
        algCfg.i4RawStride = m_profile.uImgXsz*algCfg.i4Bits/8;
        algCfg.sPdBlockInfo.i4BlockNumX = m_PDBlockInfo.i4BlockNumX;
        algCfg.sPdBlockInfo.i4BlockNumY = m_PDBlockInfo.i4BlockNumY;
        algCfg.sPdBlockInfo.i4PairNum = m_PDBlockInfo.i4PairNum;
        algCfg.sPdBlockInfo.i4PitchX  = m_PDBlockInfo.i4PitchX;
        algCfg.sPdBlockInfo.i4PitchY  = m_PDBlockInfo.i4PitchY;
        algCfg.sPdBlockInfo.i4SubBlkH = m_PDBlockInfo.i4SubBlkH;
        algCfg.sPdBlockInfo.i4SubBlkW = m_PDBlockInfo.i4SubBlkW;

#if USE_DRV_PD_COORDINATOR
        /*******************************************************************************
         * Reference crop region and mirrorflip inction to modify PD block information.
         *******************************************************************************/
        unsigned int pitch_x    = algCfg.sPdBlockInfo.i4PitchX;
        unsigned int pitch_y    = algCfg.sPdBlockInfo.i4PitchY;
        unsigned int nblk_x     = algCfg.sPdBlockInfo.i4BlockNumX;
        unsigned int nblk_y     = algCfg.sPdBlockInfo.i4BlockNumY;
        unsigned int full_lOffx = m_PDBlockInfo.i4OffsetX;
        unsigned int full_lOffy = m_PDBlockInfo.i4OffsetY;

        // Current image related full size coordinate
        unsigned int crop_x = algCfg.i4FullRawXOffset;
        unsigned int crop_y = algCfg.i4FullRawYOffset;

        // Current pd block offset related to full size coordinate
        int shift_x = crop_x-full_lOffx;
        if( shift_x<=0)
        {
            shift_x = -crop_x;
        }
        else if( shift_x%pitch_x)
        {
            shift_x = ( (shift_x+(pitch_x-1)) / pitch_x) * pitch_x - crop_x;
        }
        else
        {
            shift_x = ( shift_x / pitch_x) * pitch_x - crop_x;
        }


        int shift_y = crop_y-full_lOffy;
        if( shift_y<=0)
        {
            shift_y = -crop_y;
        }
        else if( shift_y%pitch_y)
        {
            shift_y = ( (shift_y+(pitch_y-1)) / pitch_y) * pitch_y - crop_y;
        }
        else
        {
            shift_y = ( shift_y / pitch_y) * pitch_y - crop_y;
        }

        CAM_LOGD( "coordinate shift in current sensor mode (%d, %d)\n", shift_x, shift_y);

        /*******************************************************************************
         * calculate pd pixels' position by orientation and crop information for general separate function
         *******************************************************************************/
        unsigned int cur_lOffx = full_lOffx + shift_x;
        unsigned int cur_lOffy = full_lOffy + shift_y;
        unsigned int cur_rOffx = m_profile.uImgXsz - cur_lOffx - pitch_x * nblk_x;
        unsigned int cur_rOffy = m_profile.uImgYsz - cur_lOffy - pitch_y * nblk_y;
        unsigned int offx  = (m_PDBlockInfo.iMirrorFlip & 0x1) ? cur_rOffx : cur_lOffx;
        unsigned int offy  = (m_PDBlockInfo.iMirrorFlip & 0x2) ? cur_rOffy : cur_lOffy;
        //
        algCfg.sPdBlockInfo.i4OffsetX   = offx;
        algCfg.sPdBlockInfo.i4OffsetY   = offy;

        CAM_LOGD("mirror_flip(%x), block offset : left side(%d, %d) right sied(%d, %d), offset set to algo(%d, %d)",
                 m_PDBlockInfo.iMirrorFlip,
                 cur_lOffx,
                 cur_lOffy,
                 cur_rOffx,
                 cur_rOffy,
                 algCfg.sPdBlockInfo.i4OffsetX,
                 algCfg.sPdBlockInfo.i4OffsetY);

        //
        MUINT32 pairNum = algCfg.sPdBlockInfo.i4PairNum;
        MUINT32 pdPixels[MAX_PAIR_NUM * 2][2];
        for( MUINT32 Pidx=0; Pidx<pairNum; Pidx++)
        {
            MUINT32 PosL_X = m_PDBlockInfo.i4PosL[Pidx][0] + shift_x;
            MUINT32 PosL_Y = m_PDBlockInfo.i4PosL[Pidx][1] + shift_y;
            MUINT32 PosR_X = m_PDBlockInfo.i4PosR[Pidx][0] + shift_x;
            MUINT32 PosR_Y = m_PDBlockInfo.i4PosR[Pidx][1] + shift_y;

            if (m_PDBlockInfo.iMirrorFlip & 0x1) //mirror
            {
                PosL_X = pitch_x - (PosL_X - cur_lOffx) - 1 + cur_rOffx;
                PosR_X = pitch_x - (PosR_X - cur_lOffx) - 1 + cur_rOffx;
            }
            if (m_PDBlockInfo.iMirrorFlip & 0x2) //flip
            {
                PosL_Y = pitch_y - (PosL_Y - cur_lOffy) - 1 + cur_rOffy;
                PosR_Y = pitch_y - (PosR_Y - cur_lOffy) - 1 + cur_rOffy;
            }
            pdPixels[Pidx][0] = PosR_X;
            pdPixels[Pidx][1] = PosR_Y;
            pdPixels[Pidx + pairNum][0] = PosL_X;
            pdPixels[Pidx + pairNum][1] = PosL_Y;

            CAM_LOGD( "[%s] pd coordinator [L](%3d, %3d)->(%3d, %3d), [R](%3d, %3d)->(%3d, %3d)",
                      __FUNCTION__,
                      m_PDBlockInfo.i4PosL[Pidx][0],
                      m_PDBlockInfo.i4PosL[Pidx][1],
                      PosL_X,
                      PosL_Y,
                      m_PDBlockInfo.i4PosR[Pidx][0],
                      m_PDBlockInfo.i4PosR[Pidx][1],
                      PosR_X,
                      PosR_Y);

            MUINT32 idx = (m_PDBlockInfo.iMirrorFlip & 0x2) ? ((pairNum - 1) - Pidx) : Pidx;
            algCfg.sPdBlockInfo.i4PosR[idx][0] = m_pdoPixels[idx][0] =           pdPixels[Pidx][0];
            algCfg.sPdBlockInfo.i4PosR[idx][1] = m_pdoPixels[idx][1] =           pdPixels[Pidx][1];
            algCfg.sPdBlockInfo.i4PosL[idx][0] = m_pdoPixels[idx + pairNum][0] = pdPixels[Pidx + pairNum][0];
            algCfg.sPdBlockInfo.i4PosL[idx][1] = m_pdoPixels[idx + pairNum][1] = pdPixels[Pidx + pairNum][1];
        }

#else
        for( MUINT32 Pidx=0; Pidx<algCfg.sPdBlockInfo.i4PairNum; Pidx++)
        {
            algCfg.sPdBlockInfo.i4PosR[Pidx][0] = m_PDBlockInfo.i4PosR[Pidx][0];
            algCfg.sPdBlockInfo.i4PosR[Pidx][1] = m_PDBlockInfo.i4PosR[Pidx][1];
            algCfg.sPdBlockInfo.i4PosL[Pidx][0] = m_PDBlockInfo.i4PosL[Pidx][0];
            algCfg.sPdBlockInfo.i4PosL[Pidx][1] = m_PDBlockInfo.i4PosL[Pidx][1];

            CAM_LOGD( "[%s] pd coordinator [L](%3d, %3d), [R](%3d, %3d)",
                      __FUNCTION__,
                      m_PDBlockInfo.i4PosL[Pidx][0],
                      m_PDBlockInfo.i4PosL[Pidx][1],
                      m_PDBlockInfo.i4PosR[Pidx][0],
                      m_PDBlockInfo.i4PosR[Pidx][1]);
        }
#endif
    }
    break;
    }

    CAM_LOGD( "[%s] configuration for algorithm : rawInfo(w[%d],h[%d],stride[%d],bits[%d],isPack[%d]) BlockInfo(blknum[%d,%d],offset[%d,%d],pairNum[%d],pitchSz[%d,%d],subblkSz[%d,%d]) fullRawCrop(%d %d %d %d) DualPdFormat(bufStride[0x%x],bufHeight(0x%x),bufFmt[%d],bits[%d],binning[%d,%d])",
              __FUNCTION__,
              algCfg.i4RawWidth,
              algCfg.i4RawHeight,
              algCfg.i4RawStride,
              algCfg.i4Bits,
              algCfg.i4IsPacked,
              algCfg.sPdBlockInfo.i4BlockNumX,
              algCfg.sPdBlockInfo.i4BlockNumY,
              algCfg.sPdBlockInfo.i4OffsetX,
              algCfg.sPdBlockInfo.i4OffsetY,
              algCfg.sPdBlockInfo.i4PairNum,
              algCfg.sPdBlockInfo.i4PitchX,
              algCfg.sPdBlockInfo.i4PitchY,
              algCfg.sPdBlockInfo.i4SubBlkH,
              algCfg.sPdBlockInfo.i4SubBlkW,
              algCfg.i4FullRawXOffset,
              algCfg.i4FullRawYOffset,
              algCfg.i4FullRawWidth,
              algCfg.i4FullRawHeight,
              algCfg.sDPdFormat.i4BufStride,
              algCfg.sDPdFormat.i4BufHeight,
              algCfg.sDPdFormat.i4BufFmt,
              algCfg.sDPdFormat.i4Bits,
              algCfg.sDPdFormat.i4BinningX,
              algCfg.sDPdFormat.i4BinningY);


    // General separate function needs block information.
    if( m_pPDBufMgrCore != NULL)
    {
        m_pPDBufMgrCore->SetPDInfo( m_PDBlockInfo, m_profile, m_sPDOHWInfo);
    }

    // (4) check configure core PD algorithm is correct or not.
    if ((ret = m_pIPdAlgo->setPDBlockInfo(algCfg)) != S_3A_OK)
    {
        CAM_LOGD("[Core] PD init data error");
        retAll = E_3A_ERR;
    }

    if (retAll == S_3A_OK)
        CAM_LOGD( "[%s] - configure PD algo done", __FUNCTION__);
    else
        CAM_LOGE( "[%s] - configure PD algo failed !! close PD HAL flow !!", __FUNCTION__);

    return retAll;
}

//
MRESULT PDMgr::PDCoreFlow()
{
    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] +", m_MagicNumber, __FUNCTION__);

    MRESULT ret=S_3A_OK;

    if ((m_pIPdAlgo!=NULL))
    {
        SPDResult_T PDRes;
        //reset output result.
        memset( &PDRes, 0, sizeof(SPDResult_T));

        if( /* output all 0  result directly once current lens position is 0 */
            ( m_curLensPos                  ) &&
            ( 0<m_sAnalysisData.numDa                    ) &&
            (   m_sAnalysisData.numDa<=MAX_SIZE_OF_PD_ROI) )
        {

            if (!m_i4DbgDisPdHandle)
            {
                AAA_TRACE_D("handlePD");

                // set output data puffer.
                PDRes.magicNumber = m_MagicNumber;
                PDRes.curLensPos  = m_curLensPos;
                PDRes.numROIs     = m_sAnalysisData.numDa;

                // calculating multi PD windowns
                for( MUINT32 i=0; i<m_sAnalysisData.numDa; i++)
                {
                    if( m_sAnalysisData.Da[i].sHandlePDInput.i4CurLensPos != m_curLensPos)
                    {
                        CAM_LOGE( "#(%d) [%s] lens position is mismatch (%3d != %3d), Something wrong of multi thread mechanism!!, Please reproduce issue with setting property vendor.debug.af_mgr.enable as 1 and provied log file to RD.",
                                  m_MagicNumber,
                                  __FUNCTION__);

                    }

                    PD_OUTPUT_T sPDhandleOutput;

                    // input argument is calcuated in function postToPDTask.
                    memset( &sPDhandleOutput, 0, sizeof(PD_OUTPUT_T));
                    ret = m_pIPdAlgo->handlePD( m_sAnalysisData.Da[i].sHandlePDInput, sPDhandleOutput);

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "#(%d) [%s] handlePD : win#%d Confidence(%3d) LensPos[%4d(%4d)->%4d] PD value(%7d), SatPercent(%d), SensorAGain(%d), ROI(x[%4d->%4d],y[%4d->%4d],w[%4d->%4d],h[%4d->%4d]), BufferAddress(%p, %p, %p, %p)",
                                 m_MagicNumber,
                                 __FUNCTION__,
                                 i,
                                 sPDhandleOutput.i4ConfidenceLevel,
                                 m_sAnalysisData.Da[i].sHandlePDInput.i4CurLensPos,
                                 m_curLensPos,
                                 sPDhandleOutput.i4FocusLensPos,
                                 (MINT32)(sPDhandleOutput.fPdValue*1000),
                                 sPDhandleOutput.i4SatPercent,
                                 m_sAnalysisData.Da[i].sHandlePDInput.i4SensorAGain,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4X,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4X,//refined focus PD window (refer to PD image coordinate));
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4Y,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4Y,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4W,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4W,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4H,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4H,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDLData,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDRData,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDLPos,
                                 m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDRPos);

                    if( ret==E_3A_ERR)
                    {
                        CAM_LOGE("#(%d) [%s] handlePD error at window#%d !! Please contact pdalgo owner !!", m_MagicNumber, __FUNCTION__, i);
                    }

                    //set output data puffer.
                    PDRes.ROIRes[i].Defocus                = Boundary(1, sPDhandleOutput.i4FocusLensPos, 1023);
                    PDRes.ROIRes[i].DefocusConfidence      = sPDhandleOutput.i4ConfidenceLevel;
                    PDRes.ROIRes[i].DefocusConfidenceLevel = sPDhandleOutput.i4ConfidenceLevel;
                    PDRes.ROIRes[i].PhaseDifference        = (MINT32)(sPDhandleOutput.fPdValue*1000);
                    PDRes.ROIRes[i].sROIInfo               = m_sAnalysisData.Da[i].sRoiInfo;
                    PDRes.ROIRes[i].SaturationPercent      = sPDhandleOutput.i4SatPercent;
                }
                AAA_TRACE_END_D;
            }
            else
            {
                CAM_LOGW( "#(%d) [%s] handlePD is disabled by debug command vendor.debug.handlepd.disable.", m_MagicNumber, __FUNCTION__);
                ret = S_3A_OK;
            }

        }
        else
        {
            CAM_LOGW( "#(%d) [%s] Calculation task is not executed, lens pos:%d(should be:0<value<1024), number of ROI:%d(should be:0<value<=MAX_SIZE_OF_PD_ROI(%d))",
                      m_MagicNumber,
                      __FUNCTION__,
                      m_curLensPos,
                      m_sAnalysisData.numDa,
                      MAX_SIZE_OF_PD_ROI);
        }

        if( ret==S_3A_OK)
        {
            Mutex::Autolock lock(m_Lock_Result);
            m_vPDOutput.push_back( PDRes);
            if( 1<m_vPDOutput.size())
            {
                //pop_front and keep only last 2 result
                m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                CAM_LOGD("pop_front, sz=%zu", m_vPDOutput.size());
            }
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] -", m_MagicNumber, __FUNCTION__);
    return ret;

}

/************************************************************************/
/* Data path 2 : using 3rd party PD algorithm                           */
/************************************************************************/
MRESULT PDMgr::SetCaliData2PDOpenCore(PD_ALGO_TUNING_T* /*ptrInTuningData*/, PD_CALIBRATION_DATA_T* ptrInCaliData)
{
    CAM_LOGD("%s +", __FUNCTION__);

    MRESULT ret=E_3A_ERR;

    if( m_pPDBufMgrOpen)
    {

        AAA_TRACE_D("SetCaliData2PDOpenCore");

        // (1) query sensor driver setting for 3rd party PD library.
        MUINT32  arg1=0;
        MUINT16 *arg2=NULL;
        if( m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST, (MVOID *)(&arg1), (MVOID *)(&arg2), NULL))
        {
            if( m_pIHalSensor)
            {
                m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING, (MUINTPTR)&arg1, (MUINTPTR)&arg2, 0);
            }
        }

        // (2) set calibration to PD open core.
        m_pPDBufMgrOpen->SetCalibrationData( ptrInCaliData->i4Size, ptrInCaliData->uData);

        if( m_bDebugEnable)
        {
            FILE *fp = NULL;

            fp = fopen("/sdcard/pdo/__pd_cali_data_open.bin", "w");
            fwrite( reinterpret_cast<void *>(ptrInCaliData->uData), 1, ptrInCaliData->i4Size, fp);
            fclose( fp);
        }

        //
        ret = S_3A_OK;

        AAA_TRACE_END_D;
    }

    CAM_LOGD("%s -", __FUNCTION__);
    return ret;
}

//
MRESULT PDMgr::PDOpenCoreFlow()
{
    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] +", m_MagicNumber, __FUNCTION__);

    SPDROIInput_T iPDInputData;
    SPDROIResult_T oPdOutputData;

    AAA_TRACE_D("DaBuf2PDBufMgr");
    m_pPDBufMgrOpen->SetDataBuf( m_databuf_size, m_databuf, m_frmCnt);
    AAA_TRACE_END_D;

    SPDResult_T PDRes;

    //reset output result.
    memset( &PDRes, 0, sizeof(SPDResult_T));


    if( /* Output all 0  result directly once current lens position is not vaild */
        ( 0<=m_curLensPos      ) &&
        (    m_curLensPos<=1023))
    {
        // query sensor driver setting for 3rd party PD library.
        MINT32  mode = 0;
        MUINT32 cfgNum = 0;

        AAA_TRACE_D("SendCmd(%d)ToPDBufMgr", PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO);
        MBOOL res  = m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO, (MVOID *)(&mode), (MVOID *)(&cfgNum));
        AAA_TRACE_END_D;

        AAA_TRACE_D("GetDefocus");
        {
            Mutex::Autolock lock(m_Lock_PDWinCfg);

            // Debug only
            if( m_bDebugEnable)
            {
                CAM_LOGD( "DBG : dump configuration +, total size(%d)", m_sAnalysisDataList.totalsz);
                for( S_ANALYSIS_DATA_OPEN_T *ptr = m_sAnalysisDataList.tail; ptr!=NULL; ptr=ptr->prv)
                {
                    CAM_LOGD( "DBG : configuration(frmNum[%3d], cfgNum[0x%04x], number of ROIs[%d]",
                              ptr->frmNum,
                              ptr->cfgNum,
                              ptr->numROIs);
                }
                CAM_LOGD( "DBG : dump configuration -");
            }

            CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] res(%d) mode(%d)", m_MagicNumber, __FUNCTION__, res, mode);

            //
            if( /* sensor is configured as fiexible window mode*/
                (res) &&
                (mode==2))
            {
                if( m_sAnalysisDataList.totalsz)
                {
                    // Find matched setting by configuration number.
                    S_ANALYSIS_DATA_OPEN_T *cfg = NULL;
                    for( cfg = m_sAnalysisDataList.tail; cfg!=NULL; cfg=cfg->prv)
                    {
                        if( cfg->cfgNum==cfgNum)
                        {
                            break;
                        }
                    }

                    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] cfg(%p), current buffer setting : mode(%d), cfgNum(%d)", m_MagicNumber, __FUNCTION__, cfg, mode, cfgNum);

                    if( cfg) /* match */
                    {
                        CAM_LOGD_IF( m_bDebugEnable,
                                     "#(%d) [%s] mode(0x%x), buffer's cfgNum(0x%x) is found in configuration list(0x%04x), number of ROIs (%d) is configured",
                                     m_MagicNumber,
                                     __FUNCTION__,
                                     mode,
                                     cfgNum,
                                     cfg->cfgNum,
                                     cfg->numROIs);

                        PDRes.magicNumber = m_MagicNumber;
                        PDRes.curLensPos  = m_curLensPos;
                        PDRes.numROIs     = cfg->numROIs;

                        for( MUINT32 i=0; i<cfg->numROIs; i++)
                        {
                            iPDInputData.curLensPos   = m_curLensPos;
                            iPDInputData.XSizeOfImage = m_profile.uImgXsz;
                            iPDInputData.YSizeOfImage = m_profile.uImgYsz;
                            iPDInputData.ROI.sPDROI.i4X = cfg->ROI[i].sRoiInfo.sPDROI.i4X;
                            iPDInputData.ROI.sPDROI.i4Y = cfg->ROI[i].sRoiInfo.sPDROI.i4Y;
                            iPDInputData.ROI.sPDROI.i4W = cfg->ROI[i].sRoiInfo.sPDROI.i4W;
                            iPDInputData.ROI.sPDROI.i4H = cfg->ROI[i].sRoiInfo.sPDROI.i4H;
                            iPDInputData.ROI.sType    = cfg->ROI[i].sRoiInfo.sType;
                            iPDInputData.cfgInfo      = cfg->ROI[i].cfgInfo;
                            iPDInputData.afegain      = m_afeGain;
                            iPDInputData.mode         = mode;

                            memset( &oPdOutputData, 0, sizeof(SPDROIResult_T));
                            m_pPDBufMgrOpen->GetDefocus( iPDInputData, oPdOutputData);

                            PDRes.ROIRes[i].Defocus                = Boundary(1, oPdOutputData.Defocus, 1023);
                            PDRes.ROIRes[i].DefocusConfidence      = oPdOutputData.DefocusConfidence;
                            PDRes.ROIRes[i].DefocusConfidenceLevel = oPdOutputData.DefocusConfidenceLevel;
                            PDRes.ROIRes[i].PhaseDifference        = oPdOutputData.PhaseDifference;
                            PDRes.ROIRes[i].sROIInfo               = cfg->ROI[i].sRoiInfo;
                            PDRes.ROIRes[i].SaturationPercent      = oPdOutputData.SaturationPercent;

                            CAM_LOGD_IF( m_bDebugEnable,
                                         "#(%d) [GetDefocus] win#%d Confidence(%3d) LensPos(%4d->%4ld) PD value(%7ld) SaturationPercent(%d)",
                                         m_MagicNumber,
                                         i,
                                         PDRes.ROIRes[i].DefocusConfidence,
                                         m_curLensPos,
                                         PDRes.ROIRes[i].Defocus,
                                         PDRes.ROIRes[i].PhaseDifference,
                                         PDRes.ROIRes[i].SaturationPercent);
                        }

                        if( cfg==m_sAnalysisDataList.tail)
                        {
                            /* last element is found, clean all resource inside data list */
                            m_sAnalysisDataList.nxt  = NULL;
                            m_sAnalysisDataList.head = NULL;
                            m_sAnalysisDataList.tail = NULL;
                        }
                        else
                        {
                            /* clean resource from head to the matched element */
                            m_sAnalysisDataList.nxt  = cfg->nxt;
                            m_sAnalysisDataList.head = cfg->nxt;
                            m_sAnalysisDataList.head->prv = NULL;
                        }

                        /* clean resource */
                        while(1)
                        {
                            if( cfg==NULL)
                            {
                                CAM_LOGD_IF( m_bDebugEnable,
                                             "#(%d) [%s] delete resource, current list size(%d)",
                                             m_MagicNumber,
                                             __FUNCTION__,
                                             m_sAnalysisDataList.totalsz);
                                break;
                            }

                            S_ANALYSIS_DATA_OPEN_T *nxt_del_element = cfg->prv;
                            delete cfg;
                            m_sAnalysisDataList.totalsz--;
                            cfg = nxt_del_element;
                        }

                        {
                            Mutex::Autolock lock( m_Lock_Result);
                            m_vPDOutput.push_back( PDRes);
                            if( 1<m_vPDOutput.size())
                            {
                                //pop_front and keep only last 2 result
                                m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                                CAM_LOGD("#(%d) pop_front, sz=%zu", m_MagicNumber, m_vPDOutput.size());
                            }
                        }
                    }
                }
                else
                {
                    CAM_LOGD("[W] Timing of updateing setting(AF_done) and receiving PD buffer(CAMSV_done) is not the same");
                }
            }
            else
            {
                CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] numDa(%d) numDa(%d)", m_MagicNumber, __FUNCTION__, m_sAnalysisData.numDa, m_sAnalysisData.numDa);
                if( /* checking ROI numbers */
                    ( 0<m_sAnalysisData.numDa                    ) &&
                    (   m_sAnalysisData.numDa<=MAX_SIZE_OF_PD_ROI) )
                {

                    PDRes.magicNumber = m_MagicNumber;
                    PDRes.curLensPos  = m_curLensPos;
                    PDRes.numROIs     = m_sAnalysisData.numDa;

                    for( MUINT32 i=0; i<m_sAnalysisData.numDa; i++)
                    {
                        iPDInputData.curLensPos   = m_curLensPos;
                        iPDInputData.XSizeOfImage = m_profile.uImgXsz;
                        iPDInputData.YSizeOfImage = m_profile.uImgYsz;
                        iPDInputData.ROI.sPDROI.i4X = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4X;
                        iPDInputData.ROI.sPDROI.i4Y = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4Y;
                        iPDInputData.ROI.sPDROI.i4W = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4W;
                        iPDInputData.ROI.sPDROI.i4H = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4H;
                        iPDInputData.ROI.sType      = m_sAnalysisData.Da[i].sRoiInfo.sType;
                        iPDInputData.cfgInfo      = 0xFF;
                        iPDInputData.afegain      = m_afeGain;
                        iPDInputData.mode         = 0x0;

                        memset( &oPdOutputData, 0, sizeof(SPDROIResult_T));
                        m_pPDBufMgrOpen->GetDefocus( iPDInputData, oPdOutputData);


                        PDRes.ROIRes[i].Defocus                = Boundary(1, oPdOutputData.Defocus, 1023);
                        PDRes.ROIRes[i].DefocusConfidence      = oPdOutputData.DefocusConfidence;
                        PDRes.ROIRes[i].DefocusConfidenceLevel = oPdOutputData.DefocusConfidenceLevel;
                        PDRes.ROIRes[i].PhaseDifference        = oPdOutputData.PhaseDifference;
                        PDRes.ROIRes[i].sROIInfo               = m_sAnalysisData.Da[i].sRoiInfo;
                        PDRes.ROIRes[i].SaturationPercent      = oPdOutputData.SaturationPercent;

                        CAM_LOGD_IF( m_bDebugEnable,
                                     "#(%d) [GetDefocus] win#%d Confidence(%3d) LensPos(%4d->%4ld) PD value(%7ld) SaturationPercent(%d)",
                                     m_MagicNumber,
                                     i,
                                     PDRes.ROIRes[i].DefocusConfidence,
                                     m_curLensPos,
                                     PDRes.ROIRes[i].Defocus,
                                     PDRes.ROIRes[i].PhaseDifference,
                                     PDRes.ROIRes[i].SaturationPercent);
                    }

                    {
                        Mutex::Autolock lock( m_Lock_Result);
                        m_vPDOutput.push_back( PDRes);
                        if( 1<m_vPDOutput.size())
                        {
                            //pop_front and keep only last 2 result
                            m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                            CAM_LOGD("#(%d) pop_front, sz=%zu", m_MagicNumber, m_vPDOutput.size());
                        }
                    }
                }
            }
        }
        AAA_TRACE_END_D;
    }




    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] -", m_MagicNumber, __FUNCTION__);
    return S_3A_OK;

}

/************************************************************************/
/* Data path  : verification flow                                       */
/************************************************************************/
MVOID PDMgr::PDVerificationFlow()
{
#define PROPERTY_VALUE_MAX 10
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    MUINT32 pos = m_i4CaliAFPos_Inf;

    if( m_i4DbgPDVerifyRun && m_bAEStable)
    {
        // lock AE during verification
        property_set("vendor.debug.pdmgr.lockae", "1");

        // verification flow
        m_i4DbgPDVerifyCalculateCnt++;

        pos = m_i4CaliAFPos_Inf + (m_i4DbgPDVerifyCalculateCnt/m_i4DbgPDVerifyCalculateNum)*(m_i4DbgPDVerifyStepSize);

        if( pos<= m_i4CaliAFPos_Macro)
        {
            /* Dump data once VCM is stable */
            if( DBG_MOVE_LENS_STABLE_FRAME_NUM<(m_i4DbgPDVerifyCalculateCnt%m_i4DbgPDVerifyCalculateNum))
            {
                FILE *fp = NULL;

                fp = fopen("/sdcard/pdo/__pd_verification_dump.pd", "a+");

                if( fp)
                {
                    if( m_vPDOutput.size())
                    {
#define MAX_DUMP_DATA_BUFFER_SIZE 1024
                        char  dumpDataBuf[MAX_DUMP_DATA_BUFFER_SIZE] = {'\0'};
                        char *ptrDumpDataBuf = dumpDataBuf;
                        unsigned int validBufSz = MAX_DUMP_DATA_BUFFER_SIZE;
                        unsigned int storedSz   = 0;

#define PD_VERIFICATION_WINDOW (9+1)
                        vector<SPDResult_T>::iterator itr = m_vPDOutput.begin();

                        if( itr->numROIs<=PD_VERIFICATION_WINDOW)
                        {
                            storedSz = snprintf( ptrDumpDataBuf,
                                                 validBufSz,
                                                 "FrmID_%d__CurLens_%d__ISO_%d__GAIN_%d__NumData_%d_",
                                                 itr->magicNumber,
                                                 itr->curLensPos,
                                                 m_iso,
                                                 m_afeGain,
                                                 itr->numROIs);
                            validBufSz     -= storedSz;
                            ptrDumpDataBuf += storedSz;

                            for( MUINT32 i=0; i<itr->numROIs; i++)
                            {
                                storedSz = snprintf( ptrDumpDataBuf,
                                                     validBufSz,
                                                     "WIN_%d_PD_%d_CL_%d_TARGETPOS_%d_",
                                                     i,
                                                     itr->ROIRes[i].PhaseDifference,
                                                     itr->ROIRes[i].DefocusConfidence,
                                                     itr->ROIRes[i].Defocus);
                                validBufSz     -= storedSz;
                                ptrDumpDataBuf += storedSz;
                            }
                            fprintf (fp, "%s\n", dumpDataBuf);
                            CAM_LOGD( "#(%d) [%s] DBG (verification flow) : dump info (valid buffer size %d) %s", m_MagicNumber, __FUNCTION__, validBufSz, dumpDataBuf);
                        }
                        else
                        {
                            CAM_LOGE( "#(%d) [%s] DBG (verification flow) : fail. something is wrong", m_MagicNumber, __FUNCTION__);
                        }
                    }

                    fclose( fp);
                }
            }
        }
        else
        {
            /******************************************************
             * A verification flow is finished.
             * Clear all status and waiting host trigger command.
             ******************************************************/
            property_set("vendor.debug.pdmgr.lockae", "0");
            property_set("vendor.debug.pd_verify_flow.run", "0");
            m_i4DbgPDVerifyRun = 0;
            m_i4DbgPDVerifyCalculateCnt = 0;
            pos = m_i4CaliAFPos_Inf;
        }
    }
    else
    {
        property_set("vendor.debug.pdmgr.lockae", "0");

        m_i4DbgPDVerifyCalculateCnt = 0;
        pos = m_i4CaliAFPos_Inf;

        /* wait host trigger command*/
        m_i4DbgPDVerifyRun = property_get_int32("vendor.debug.pd_verify_flow.run", 0);
    }

    snprintf(value, PROPERTY_VALUE_MAX, "%d", pos);
    property_set("vendor.debug.af_motor.position", value);

    CAM_LOGD( "#(%d) [%s] DBG (verification flow) : dev(%d) En(%d) RunFlow(%d) bAEStable(%d) cnt(%d) calNum(%d) step size(%d) move lens to(%s)",
              m_MagicNumber,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4DbgPDVerifyEn,
              m_i4DbgPDVerifyRun,
              m_bAEStable,
              m_i4DbgPDVerifyCalculateCnt,
              m_i4DbgPDVerifyCalculateNum,
              m_i4DbgPDVerifyStepSize,
              value);
}


/************************************************************************/
/* Interface to host                                                    */
/************************************************************************/
//
MRESULT PDMgr::GetVersionOfPdafLibrary(SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret=E_3A_ERR;

    tOutSWVer.MajorVersion = 0;
    tOutSWVer.MinorVersion = 0;

    if( m_bEnPDBufThd)
    {
        if (m_profile.BufType & MSK_CATEGORY_OPEN)
        {
            if( m_pPDBufMgrOpen)
            {
                ret = m_pPDBufMgrOpen->GetVersionOfPdafLibrary( tOutSWVer);
            }
        }
        else
        {
            if( m_pIPdAlgo)
            {
                PD_LIB_VERSION_T tSWVer;
                ret = m_pIPdAlgo->getVersion( tSWVer);
                tOutSWVer.MajorVersion = tSWVer.u4MajorVersion;
                tOutSWVer.MinorVersion = tSWVer.u4MinorVersion;
            }
        }

        CAM_LOGD("%s = %d.%d", __FUNCTION__, (int)tOutSWVer.MajorVersion, (int)tOutSWVer.MinorVersion);
    }
    return ret;
}

//
MRESULT PDMgr::setPDCaliData( PD_NVRAM_T *ptrInPDNvRam)
{
    //This function is used to set PD calibration data and output PD related data for Hybrid AF.
    MRESULT ret = E_3A_ERR;
    MUINT32 result;
    MINT32  i4SensorDevID;
    CAM_CAL_DATA_STRUCT CamCalData;

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
        break;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        break;
    }

#if 0
    // Read PDAF calibration data from NVRAM.
    NVRAM_AF_PARA_STRUCT *pNvramDataAF;   /* nvram for managers*/
    NvBufUtil::getInstance().getBufAndReadNoDefault( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev, (void*&)pNvramDataAF);
    // Storing PDAF calibration data into NVRAM.
    NvBufUtil::getInstance().write( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev);
#endif


    /**********************************************************
     * Read AF calibration data.
     * AF calibration data is used for verification flow only.
     **********************************************************/
    result = CamCalDrvBase::createInstance()->GetCamCalCalData(i4SensorDevID, CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&CamCalData);
    if( /* Verify AF calibration data */
        (result==CAM_CAL_ERR_NO_ERR) &&
        (CamCalData.Single2A.S2aAf[0]<CamCalData.Single2A.S2aAf[1]) &&
        (  CamCalData.Single2A.S2aAf[0]<1023) &&
        (0<CamCalData.Single2A.S2aAf[0]     ) &&
        (  CamCalData.Single2A.S2aAf[1]<1023) &&
        (0<CamCalData.Single2A.S2aAf[1]     ))
    {
        m_i4CaliAFPos_Inf    = CamCalData.Single2A.S2aAf[0];
        m_i4CaliAFPos_Macro  = CamCalData.Single2A.S2aAf[1];
        m_i4CaliAFPos_50cm   = CamCalData.Single2A.S2aAf[3];
        m_i4CaliAFPos_Middle = CamCalData.Single2A.S2aAF_t.AF_Middle_calibration;
    }
    else
    {
        m_i4CaliAFPos_Inf   = 0;
        m_i4CaliAFPos_Macro = 1023;
        CAM_LOGE("[%s] Getting CAMERA_CAM_CAL_DATA_3A_GAIN from cam_cal driver fail !!! Please contact cam_cal driver porting owner!! result(0x%x) value(%d %d %d %d)",
                 __FUNCTION__,
                 result,
                 CamCalData.Single2A.S2aAf[0],
                 CamCalData.Single2A.S2aAf[1],
                 CamCalData.Single2A.S2aAf[3],
                 CamCalData.Single2A.S2aAF_t.AF_Middle_calibration);
    }



    /************************************************************************************************
     * Read PD calibration data
     * If calibration data hasn't read out from EEPROM or something wrong of calibration data buffer.
     * Reading calibration data from EEPROM by using cam_cal driver.
     * Please notice that m_sCaliData is only initialized druing construct pd_mgr.
     ************************************************************************************************/
#define _PDCALIDA_READ_ERROR_ 0
#define _PDCALIDA_READ_FROM_DEFAULT_ 1
#define _PDCALIDA_READ_FROM_EEPROM_  2

    // data valid of default calibration data which is stored in file system (/sdcard/pdo/pd_cali_data.bin)
    MINT32 cali_da_vd_F = property_get_int32("debug.pdcalidata.enable", 0)!=0;
    // data valid of default calibration data which is stored in the tunning data
    MBOOL  cali_da_vd_T = ptrInPDNvRam->rCaliData.i4Size!=0;

    // checking using default calibration data.
    MBOOL enable_cali_data_from_default = cali_da_vd_F || cali_da_vd_T;

    CAM_LOGD( "[%s] Load PD default calibration data(%d) : from file system(%d), from tuning file(%d)",
              __FUNCTION__,
              enable_cali_data_from_default,
              cali_da_vd_F,
              cali_da_vd_T);

    if( enable_cali_data_from_default==0)
    {
        if( (m_sCaliData.i4Size<=0) || (PD_CALI_DATA_SIZE<m_sCaliData.i4Size) || (m_u1CaliDaSrc!=_PDCALIDA_READ_FROM_EEPROM_))
        {
            result = CamCalDrvBase::createInstance()->GetCamCalCalData(i4SensorDevID, CAMERA_CAM_CAL_DATA_PDAF, (void *)&CamCalData);
            CAM_LOGD( "[%s] PDAF calibration data hasn't read out. Reading calibration data from EEPROM by using cam_cal driver : result(0x%8x), DevID(%d)",
                      __FUNCTION__,
                      result,
                      i4SensorDevID);
            if( result==CAM_CAL_ERR_NO_ERR)
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_FROM_EEPROM_;
                m_sCaliData.i4Size = CamCalData.PDAF.Size_of_PDAF;
                memcpy( m_sCaliData.uData, CamCalData.PDAF.Data, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
            }
            else
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_ERROR_;
                m_sCaliData.i4Size = 0;
                CAM_LOGE("[%s] Getting CAMERA_CAM_CAL_DATA_PDAF from cam_cal driver fail !!! Please contact cam_cal driver porting owner!!", __FUNCTION__);
            }
        }
        else
        {
            CAM_LOGD( "[%s] PD calibration data is already read out normally from EEPROM (%d). Using it directly.", __FUNCTION__, m_u1CaliDaSrc);
        }
    }
    else
    {
        if( (m_sCaliData.i4Size<=0) || (PD_CALI_DATA_SIZE<m_sCaliData.i4Size) || (m_u1CaliDaSrc!=_PDCALIDA_READ_FROM_DEFAULT_))
        {
            FILE *fp = NULL;
            fp = fopen("/sdcard/pdo/pd_cali_data.bin", "r");

            if( fp)
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_FROM_DEFAULT_;
                m_sCaliData.i4Size = PD_CALI_DATA_SIZE;
                fread( reinterpret_cast<void *>(m_sCaliData.uData), 1, PD_CALI_DATA_SIZE, fp);
                fclose( fp);
                CAM_LOGW( "[%s] Read default PD calibration data from /sdcard/pdo/pd_cali_data.bin!! This is a debug flow!!", __FUNCTION__);
            }
            else if( ptrInPDNvRam->rCaliData.i4Size)
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_FROM_DEFAULT_;
                m_sCaliData.i4Size = PD_CALI_DATA_SIZE;
                memcpy( m_sCaliData.uData, ptrInPDNvRam->rCaliData.uData, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
                CAM_LOGW( "[%s] Read default PD calibration data from tuning file!! Please double check this setting is correct!!", __FUNCTION__);
            }
            else
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_ERROR_;
                m_sCaliData.i4Size = 0;
                CAM_LOGE("[%s] Read default PD calibration data FAIL. Please check setting of the tuning file or file /sdcard/pdo/pd_cali_data.bin", __FUNCTION__);
            }
        }
        else
        {
            CAM_LOGD( "[%s] Default PD calibration data is already read out normally (%d). Using it directly.", __FUNCTION__, m_u1CaliDaSrc);
        }
    }


    CAM_LOGD( "[%s] AF calibration information : Inf(%d) Macro(%d) 50cm(%d) Middle(%d) ,PDAF calibration information (%d): size(%d BYTE), first 4 dword(0x%08x 0x%08x 0x%08x 0x%08x)",
              __FUNCTION__,
              m_i4CaliAFPos_Inf,
              m_i4CaliAFPos_Macro,
              m_i4CaliAFPos_50cm,
              m_i4CaliAFPos_Middle,
              m_u1CaliDaSrc,
              m_sCaliData.i4Size,
              *(((MUINT32 *)m_sCaliData.uData)+0),
              *(((MUINT32 *)m_sCaliData.uData)+1),
              *(((MUINT32 *)m_sCaliData.uData)+2),
              *(((MUINT32 *)m_sCaliData.uData)+3));
    //
    if( m_bDebugEnable)
    {
        CAM_LOGD("dump calibration data:");
        for( int index = 0; index<m_sCaliData.i4Size; index+=32)
        {
            CAM_LOGD( "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                      m_sCaliData.uData[index+ 0], m_sCaliData.uData[index+ 1],
                      m_sCaliData.uData[index+ 2], m_sCaliData.uData[index+ 3],
                      m_sCaliData.uData[index+ 4], m_sCaliData.uData[index+ 5],
                      m_sCaliData.uData[index+ 6], m_sCaliData.uData[index+ 7],
                      m_sCaliData.uData[index+ 8], m_sCaliData.uData[index+ 9],
                      m_sCaliData.uData[index+10], m_sCaliData.uData[index+11],
                      m_sCaliData.uData[index+12], m_sCaliData.uData[index+13],
                      m_sCaliData.uData[index+14], m_sCaliData.uData[index+15],
                      m_sCaliData.uData[index+16], m_sCaliData.uData[index+17],
                      m_sCaliData.uData[index+18], m_sCaliData.uData[index+19],
                      m_sCaliData.uData[index+20], m_sCaliData.uData[index+21],
                      m_sCaliData.uData[index+22], m_sCaliData.uData[index+23],
                      m_sCaliData.uData[index+24], m_sCaliData.uData[index+25],
                      m_sCaliData.uData[index+26], m_sCaliData.uData[index+27],
                      m_sCaliData.uData[index+28], m_sCaliData.uData[index+29],
                      m_sCaliData.uData[index+30], m_sCaliData.uData[index+31]);
        }
    }

    //==========
    // (3) Set PDAF calibration data.
    //==========
    if( /* After related instance and PD thread is created, load calibration data*/
        ((m_bEnPDBufThd                                        )) &&
        ((m_sCaliData.i4Size) != (0                            )) &&
        ((m_profile.BufType ) != (EPDBUF_NOTDEF                )) &&
        ((m_pPDBufMgrOpen) || (m_pPDBufMgrCore && m_pIPdAlgo)))
    {

        if( m_profile.BufType & MSK_CATEGORY_OPEN)
            ret = SetCaliData2PDOpenCore( &(ptrInPDNvRam->rTuningData), &m_sCaliData);
        else
            ret = SetCaliData2PDCore( &(ptrInPDNvRam->rTuningData), &m_sCaliData);
    }
    else
    {
        ret = E_3A_ERR;
        CAM_LOGD( "%s No setting PD calibration and tuning data: type(%d), enThd(%d), instance(%p %p %p)",
                  __FUNCTION__,
                  m_profile.BufType,
                  m_bEnPDBufThd,
                  (void*)m_pPDBufMgrCore,
                  (void*)m_pIPdAlgo,
                  (void*)m_pPDBufMgrOpen);
    }

    return ret;
}

//
MRESULT PDMgr::UpdatePDParam( MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AFPD_BLOCK_ROI_T *tInputPDArea, MINT32 i4MinLensPos, MINT32 i4MaxLensPos)
{
    MRESULT ret = E_3A_ERR;

    if( m_profile.BufType==EPDBUF_VC_OPEN)
    {
        if( 0<i4InputPDAreaNum && i4InputPDAreaNum<=MAX_QUEUE_SIZE_OF_CONFIGURATION_FOR_3RD_PARTY_LIB && tInputPDArea!=NULL)
        {
            S_ANALYSIS_DATA_OPEN_T *cfg  = new S_ANALYSIS_DATA_OPEN_T;

            cfg->frmNum  = u4FrmNum; /* current magic number of statistic data */
            cfg->numROIs = i4InputPDAreaNum;
            for( MUINT32 i=0; i<cfg->numROIs; i++)
            {
                cfg->ROI[i].sRoiInfo = tInputPDArea[i];
            }
            cfg->nxt  = NULL;
            cfg->head = NULL;
            cfg->tail = NULL;

            {
                Mutex::Autolock lock(m_Lock_PDWinCfg);
                S_ANALYSIS_DATA_OPEN_T *tail = m_sAnalysisDataList.tail;

                if( tail)
                {
                    // add element at the end of list
                    cfg->prv  = tail;
                    tail->nxt = cfg;
                }
                else
                {
                    // add first element to list
                    cfg->prv  = NULL;
                    m_sAnalysisDataList.head = cfg;
                    m_sAnalysisDataList.nxt  = cfg;
                }
                m_sAnalysisDataList.tail = cfg;
                m_sAnalysisDataList.totalsz++;

                // Limit list size.
                if( MAX_QUEUE_SIZE_OF_CONFIGURATION_FOR_3RD_PARTY_LIB<m_sAnalysisDataList.totalsz)
                {
                    S_ANALYSIS_DATA_OPEN_T *del_element = m_sAnalysisDataList.head;

                    m_sAnalysisDataList.head = del_element->nxt;
                    m_sAnalysisDataList.nxt  = del_element->nxt;
                    m_sAnalysisDataList.head->prv = NULL;
                    m_sAnalysisDataList.totalsz--;

                    delete del_element;
                }


                //
                ConfigurePDHWSetting( &m_profile, NULL);
            }
        }
    }

    //update common parameters which is depended on hybrid af output
    m_minLensPos = i4MinLensPos;
    m_maxLensPos = i4MaxLensPos;

    return ret;
}

//
MRESULT PDMgr::postToPDTask( PD_CALCULATION_INPUT *ptrInputData)
{
    AAA_TRACE_D("postToPDTask");

    MRESULT ret=E_3A_ERR;

    if( m_bEnPDBufThd)
    {
        if( ptrInputData)
        {
            //Run thread if PD task is not busy.
            int Val;
            ::sem_getvalue(&m_semPDBufThdEnd, &Val);

            if( m_bDebugEnable)
            {
                if( Val==1)
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "#(%d) [%s] Value of semaphore(m_semPDBufThdEnd) is [1] : Normal case:PDBufThd is waiting signal for calculation.",
                                 ptrInputData->magicNumber,
                                 __FUNCTION__);
                else
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "#(%d) [%s] Value of semaphore(m_semPDBufThdEnd) is [%d] : Abnormal case:Calculation is still executed in PDBufThd",
                                 ptrInputData->magicNumber,
                                 __FUNCTION__,
                                 Val);
            }

            //checking PD thread is busy or not.
            if (Val == 1)
            {
                /***************************************************************
                 * (1) get information which is corresponding to current buffer.
                 ***************************************************************/
                m_MagicNumber = ptrInputData->magicNumber;
                m_frmCnt      = ptrInputData->frmNum;
                m_curLensPos  = ptrInputData->curLensPos;
                m_afeGain     = ptrInputData->afeGain;
                m_iso         = ptrInputData->iso;
                m_bAEStable   = ptrInputData->isAEStable;

                CAM_LOGD_IF( m_bDebugEnable,
                             "#(%d) [%s] Input buffer(va:%p sz:0x%x stride:0x%x AGain:%d iso:%d curlens:%d minlens:%d maxlens:%d)",
                             ptrInputData->magicNumber,
                             __FUNCTION__,
                             ptrInputData->databuf_virtAddr,
                             ptrInputData->databuf_size,
                             ptrInputData->databuf_stride,
                             m_afeGain,
                             m_iso,
                             m_curLensPos,
                             m_minLensPos,
                             m_maxLensPos);

                if( m_i4DbgPdDump)
                {
                    static MUINT32 dump_raw_data_cnt = 0;

                    if( /**/
                        ((m_profile.BufType & MSK_PDBUF_TYPE_RAW_LEGACY)==0) ||
                        ((m_profile.BufType & MSK_PDBUF_TYPE_RAW_LEGACY) && (dump_raw_data_cnt%10==0)) /*dump source buffer (raw data) for each 10 frames*/)
                    {
                        dump_raw_data_cnt = 0;

                        char fileName[256];
                        FILE *fp = nullptr;

                        //
                        sprintf(fileName,
                                "/sdcard/pdo/%llu_%05d_srcBuf_type_0x%x_size_0x%x_stride_0x%x_DAC_%d.raw",
                                _GET_TIME_STAMP_US_(),
                                ptrInputData->magicNumber,
                                m_profile.BufType,
                                ptrInputData->databuf_size,
                                ptrInputData->databuf_stride,
                                ptrInputData->curLensPos);

                        fp = fopen(fileName, "w");
                        if( fp)
                        {
                            CAM_LOGD( "dump file : %s", fileName);
                            fwrite(reinterpret_cast<void *>(ptrInputData->databuf_virtAddr), 1, ptrInputData->databuf_size, fp);
                            fclose(fp);
                        }
                        if( m_profile.BufType & MSK_PDBUF_TYPE_VC)
                        {
                            if( ptrInputData->databuf_size<((m_SensorVCInfo.VC2_SIZEH)*(m_SensorVCInfo.VC2_SIZEV)))
                            {
                                CAM_LOGE( "#(%d) CamSV buffer size is mismatch : %d < %d",
                                          m_MagicNumber,
                                          ptrInputData->databuf_size,
                                          ((m_SensorVCInfo.VC2_SIZEH)*(m_SensorVCInfo.VC2_SIZEV)));
                            }
                        }
                    }

                    dump_raw_data_cnt++;
                }


                /***************************************************************
                 * (2) get PD ROI from host.
                 ***************************************************************/
                MUINT32 max_num_roi = MAX_SIZE_OF_PD_ROI<AF_PSUBWIN_NUM ? MAX_SIZE_OF_PD_ROI : AF_PSUBWIN_NUM;
                m_sAnalysisData.numDa = ptrInputData->numROI<max_num_roi ? ptrInputData->numROI : max_num_roi;
                for( MUINT32 i=0; i<m_sAnalysisData.numDa; i++)
                {
                    m_sAnalysisData.Da[i].sRoiInfo = ptrInputData->ROI[i];
                }

                /***************************************************************
                 * (3) img size should be same as profile information
                 ***************************************************************/
                if( m_profile.uImgXsz!=ptrInputData->XSizeOfImage || m_profile.uImgYsz!=ptrInputData->YSizeOfImage)
                {
                    CAM_LOGW( "#(%d) ImgSz is mismatch profile(%d,%d), coordinate(%d, %d)",
                              m_MagicNumber,
                              m_profile.uImgXsz,
                              m_profile.uImgYsz,
                              ptrInputData->XSizeOfImage,
                              ptrInputData->YSizeOfImage);

                    m_profile.uImgXsz = ptrInputData->XSizeOfImage;
                    m_profile.uImgYsz = ptrInputData->YSizeOfImage;
                }

                /***************************************************************
                 * (4) prepare data buffer and post to calculation thread
                 ***************************************************************/
                if( m_profile.BufType & MSK_CATEGORY_OPEN)
                {
                    if( /* buffer information check*/
                        (m_databuf_size!=ptrInputData->databuf_size    ) ||
                        (m_databuf_stride!=ptrInputData->databuf_stride))
                    {
                        // Release buffer if need
                        if( m_databuf)
                            delete m_databuf;
                        m_databuf = nullptr;

                        //
                        m_databuf_size   = ptrInputData->databuf_size;
                        m_databuf_stride = ptrInputData->databuf_stride;
                        m_databuf        = new MUINT8 [m_databuf_size];

                        CAM_LOGD( "#(%d) allocate buffer(%p) for CamSV data : size %d(%d), stride %d(%d)",
                                  m_MagicNumber,
                                  m_databuf,
                                  m_databuf_size,
                                  ptrInputData->databuf_size,
                                  m_databuf_stride,
                                  ptrInputData->databuf_stride);
                    }
                    AAA_TRACE_D("ProcessBuf");
                    memcpy( m_databuf, ptrInputData->databuf_virtAddr, sizeof(MUINT8)*(m_databuf_size));
                    AAA_TRACE_END_D;

                    //post to thread
                    ::sem_wait( &m_semPDBufThdEnd); //to be 1, it won't block, 0 means PD task not ready yet
                    ::sem_post( &m_semPDBuf);
                }
                else
                {
                    if (m_curLensPos)
                    {
                        // without m_curLensPos,
                        // don't do pdextract and don't post to thread
                        // to ensure pdextract is paired with pdconvert

                        PD_EXTRACT_INPUT_T sPDExtractInput;

                        if( (m_profile.BufType==EPDBUF_VC || m_profile.BufType==EPDBUF_PDO) && m_pPDBufMgrCore)
                        {
                            CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] ConvertPDBufFormat +", m_MagicNumber, __FUNCTION__);
                            AAA_TRACE_D("ConvertPDBufFormat");
                            MUINT16  *ptrConvertedLRBuf = m_pPDBufMgrCore->ConvertPDBufFormat( ptrInputData->databuf_size,
                                                          ptrInputData->databuf_stride,
                                                          ptrInputData->databuf_virtAddr,
                                                          m_sPDOHWInfo.u4BitDepth);
                            AAA_TRACE_END_D;
                            CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] ConvertPDBufFormat -, address of result buffer(%p)", m_MagicNumber, __FUNCTION__, ptrConvertedLRBuf);

                            if( m_i4DbgPdDump)
                            {
                                char fileName[256];
                                FILE *fp = nullptr;

                                if( ptrConvertedLRBuf && m_PDBlockInfo.i4SubBlkW && m_PDBlockInfo.i4SubBlkH)
                                {
                                    MUINT32 _xsz   = m_PDBlockInfo.i4BlockNumX * (m_PDBlockInfo.i4PitchX/m_PDBlockInfo.i4SubBlkW);
                                    MUINT32 _ysz   = m_PDBlockInfo.i4BlockNumY * (m_PDBlockInfo.i4PitchY/m_PDBlockInfo.i4SubBlkH) * 2; //2: L and R pixels
                                    MUINT32 _bufSz = _xsz*_ysz; //Pixels

                                    // get porting result
                                    MUINT32 PDXsz, PDYsz, PDBufSz;
                                    m_pPDBufMgrCore->GetLRBufferInfo(PDXsz, PDYsz, PDBufSz);

                                    if( (PDXsz==_xsz) && (PDYsz==_ysz) && (PDBufSz==_bufSz))
                                    {
                                        sprintf(fileName,
                                                "/sdcard/pdo/%llu_%05d_cvtBuf_size_0x%x_stride_0x%x_pixelNumX_%d_pixelNumY_%d_DAC_%d.raw",
                                                _GET_TIME_STAMP_US_(),
                                                ptrInputData->magicNumber,
                                                _bufSz*sizeof(MUINT16),
                                                _xsz*sizeof(MUINT16),
                                                _xsz,
                                                _ysz,
                                                ptrInputData->curLensPos);
                                        fp = fopen(fileName, "w");
                                        if( fp)
                                        {
                                            CAM_LOGD( "dump file : %s", fileName);
                                            fwrite(reinterpret_cast<void *>(ptrConvertedLRBuf), 1, _bufSz*sizeof(MUINT16), fp);
                                            fclose(fp);
                                        }
                                    }
                                    else
                                    {
                                        CAM_LOGE( "#(%d) porting task is not correct !! size mismatch : PDXsz(%d!=%d) PDYsz(%d!=%d) PDBufSz(%d!=%d)",
                                                  m_MagicNumber,
                                                  PDXsz,
                                                  _xsz,
                                                  PDYsz,
                                                  _ysz,
                                                  PDBufSz,
                                                  _bufSz);
                                    }
                                }
                            }

                            sPDExtractInput.pRawBuf = NULL;
                            sPDExtractInput.pPDBuf = reinterpret_cast<MVOID *>(ptrConvertedLRBuf);
                        }
                        else if( m_profile.BufType & MSK_CATEGORY_DUALPD)
                        {
                            sPDExtractInput.pRawBuf = NULL;
                            sPDExtractInput.pPDBuf = reinterpret_cast<MVOID *>(ptrInputData->databuf_virtAddr);
                        }
                        else //EPDBUF_RAW_LEGACY
                        {
                            sPDExtractInput.pRawBuf = reinterpret_cast<MVOID *>(ptrInputData->databuf_virtAddr);
                            sPDExtractInput.pPDBuf  = NULL;
                        }

                        CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] extractPD +", m_MagicNumber, __FUNCTION__);
                        AAA_TRACE_D("extractPD");
                        for( MUINT32 i=0; i<m_sAnalysisData.numDa; i++)
                        {
                            sPDExtractInput.sFocusWin.i4X = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4X;
                            sPDExtractInput.sFocusWin.i4Y = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4Y;
                            sPDExtractInput.sFocusWin.i4W = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4W;
                            sPDExtractInput.sFocusWin.i4H = m_sAnalysisData.Da[i].sRoiInfo.sPDROI.i4H;
                            sPDExtractInput.sFocusWin.sAFType = (eAF_ROI_TYPE_T)(m_sAnalysisData.Da[i].sRoiInfo.sType);

                            // prepare input argument for handlePD
                            m_sAnalysisData.Da[i].sHandlePDInput.i4CurLensPos  = m_curLensPos;
                            m_sAnalysisData.Da[i].sHandlePDInput.i4SensorAGain = m_afeGain;
                            m_sAnalysisData.Da[i].sHandlePDInput.i4ISO         = m_iso;
                            m_sAnalysisData.Da[i].sHandlePDInput.i4MinLensPos  = m_minLensPos;
                            m_sAnalysisData.Da[i].sHandlePDInput.i4MaxLensPos  = m_maxLensPos;
                            ret = m_pIPdAlgo->extractPD( sPDExtractInput, m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData);

                            if( m_bDebugEnable || ret==E_3A_ERR)
                            {
                                CAM_LOGD( "#(%d) [%s] extractPD : win#%d(%d->%d,%d), ROI(x[%4d(%4d->%4d)],y[%4d(%4d->%4d)],w[%4d(%4d->%4d)],h[%4d(%4d->%4d)]), Buf(%p, %p, %p, %p)",
                                          m_MagicNumber,

                                          __FUNCTION__,
                                          i,
                                          sPDExtractInput.sFocusWin.sAFType,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.sAFType,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.sAFType,
                                          sPDExtractInput.sFocusWin.i4X,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4X,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4X,//refined focus PD window (refer to PD image coordinate));
                                          sPDExtractInput.sFocusWin.i4Y,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4Y,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4Y,
                                          sPDExtractInput.sFocusWin.i4W,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4W,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4W,
                                          sPDExtractInput.sFocusWin.i4H,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4H,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4H,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDLData,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDRData,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDLPos,
                                          m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData.pPDRPos);
                            }

                            if( ret==E_3A_ERR)
                            {
                                CAM_LOGE( "#(%d) extractPD fail #%d, rest extractPD result", m_MagicNumber, i);
                                memset( &m_sAnalysisData.Da[i].sHandlePDInput.sPDExtractData, 0, sizeof(PD_EXTRACT_DATA_T));
                            }
                        }
                        AAA_TRACE_END_D;
                        CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] extractPD -", m_MagicNumber, __FUNCTION__);

                        //post to thread
                        ::sem_wait( &m_semPDBufThdEnd); //to be 1, it won't block, 0 means PD task not ready yet
                        ::sem_post( &m_semPDBuf);
                    }
                    else
                    {
                        CAM_LOGD( "#(%d) [%s] Current lens position is 0, skip post command to pd calculation thread.",
                                  ptrInputData->magicNumber,
                                  __FUNCTION__);
                    }
                }
            }
            else
            {
                CAM_LOGD( "#(%d) [%s] semaphore(m_semPDBufThdEnd=%d), Can not post command to calculation thread(PDBufThd), because calculation still executed.",
                          m_MagicNumber,
                          __FUNCTION__,
                          Val);
            }
            ret=S_3A_OK;

        }
        else
        {
            CAM_LOGE( "#(%d) [%s] Input argumentis NULL, close thread", m_MagicNumber, __FUNCTION__);
            ret=E_3A_ERR;
        }

        if( ret==E_3A_ERR)
        {
            closeThread();
        }
    }
    else
    {
        CAM_LOGW( "#(%d) [%s] Can not post to pd thread because of thread is closed", m_MagicNumber, __FUNCTION__);
        ret=E_3A_ERR;
    }

    AAA_TRACE_END_D;

    return ret;
}

//
MRESULT PDMgr::getPDTaskResult( PD_CALCULATION_OUTPUT **ptrResultOutput)
{
    MRESULT ret=E_3A_ERR;

    if( /* checking API's condition */
        (m_bEnPDBufThd           ) &&
        ((*ptrResultOutput)==nullptr))
    {
        Mutex::Autolock lock(m_Lock_Result);
        if( m_vPDOutput.size())
        {
            vector<SPDResult_T>::iterator itr = m_vPDOutput.begin();

            //allocate result buffer
            (*ptrResultOutput) = new PD_CALCULATION_OUTPUT(itr->numROIs);

            //output
            (*ptrResultOutput)->frmNum     = itr->magicNumber;
            (*ptrResultOutput)->curLensPos = itr->curLensPos;

            if( itr->numROIs!=((*ptrResultOutput)->numRes))
            {
                CAM_LOGE( "[%s] Result size is mismatch (%d != %d). !!! should not be happened !!!",
                          __FUNCTION__,
                          itr->numROIs,
                          (*ptrResultOutput)->numRes);
                (*ptrResultOutput)->numRes = (itr->numROIs<((*ptrResultOutput)->numRes)) ? itr->numROIs : (*ptrResultOutput)->numRes;
            }
            memcpy( (*ptrResultOutput)->Res, itr->ROIRes, sizeof(SPDROIResult_T)*((*ptrResultOutput)->numRes));

            if( m_bDebugEnable)
            {
                CAM_LOGD_IF( m_bDebugEnable,
                             "[%s] frmNum(%d) curPos(%d) numRes(%d) innfo(fmt,x,y,w,h,pos,C,CL,PD,satPercent)",
                             __FUNCTION__,
                             (*ptrResultOutput)->frmNum,
                             (*ptrResultOutput)->curLensPos,
                             (*ptrResultOutput)->numRes);

                for( MUINT32 i=0; i<(*ptrResultOutput)->numRes;)
                {
                    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
                    char* ptrMsgBuf = dbgMsgBuf;

                    for( MUINT32 j=0; j<3 && i<(*ptrResultOutput)->numRes; j++)
                    {

                        ptrMsgBuf += sprintf( ptrMsgBuf,
                                              " #%d(%d,%4d,%4d,%4d,%4d,%4ld,%3d,%3lu,%6ld,%d)\n",
                                              i,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sType,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4X,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4Y,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4W,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4H,
                                              (*ptrResultOutput)->Res[i].Defocus,
                                              (*ptrResultOutput)->Res[i].DefocusConfidence,
                                              (*ptrResultOutput)->Res[i].DefocusConfidenceLevel,
                                              (*ptrResultOutput)->Res[i].PhaseDifference,
                                              (*ptrResultOutput)->Res[i].SaturationPercent);

                        i++;
                    }
                    CAM_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf);
                }
            }

            //pop_front
            m_vPDOutput.erase( m_vPDOutput.begin());
            ret = S_3A_OK;
        }
        else
        {
            CAM_LOGD_IF( m_bDebugEnable,
                         "[%s] No result in the queue. Result may be still calculated.",
                         __FUNCTION__);
            ret = E_AF_BUSY;
        }
    }
    else
    {
        CAM_LOGD_IF( m_bDebugEnable,
                     "[%s] Wrong input argument. PD buffer thread enable(%d), Output buffer(%p)",
                     __FUNCTION__,
                     m_bEnPDBufThd,
                     ptrResultOutput);

    }

    return ret;

}

//
MRESULT PDMgr::GetDebugInfo( AF_DEBUG_INFO_T &sOutDbgInfo)
{
    MRESULT ret = E_3A_ERR;

    //Not support open pd library.
    switch( m_profile.BufType)
    {
    case EPDBUF_VC :
    case EPDBUF_RAW_LEGACY :
        if( m_pIPdAlgo)
        {
            ret = m_pIPdAlgo->getDebugInfo(sOutDbgInfo);
        }
        ret = S_AE_OK;
        break;
    default :
        break;
    }

    return ret;
}

};  //  namespace NS3A

