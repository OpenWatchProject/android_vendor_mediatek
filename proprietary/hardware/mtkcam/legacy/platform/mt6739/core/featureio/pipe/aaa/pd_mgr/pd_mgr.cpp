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

#include <cutils/properties.h>
#include <utils/threads.h>

#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_sensor_buf_mgr.h"

#include "pd_mgr.h"
#include <pd_buf_mgr_open.h>
#include <pd_buf_mgr.h>
#include <af_feature.h>
#include <Local.h>

#include <isp_tuning.h>
#include <kd_camera_feature.h>
#include "kd_imgsensor.h"
#include <mtkcam/algorithm/lib3a/pd_algo_if.h>
#include <mtkcam/drv/IHalSensor.h>
#include <nvbuf_util.h>
#include <kd_imgsensor_define.h>
#include "camera_custom_cam_cal.h"
#include "cam_cal_drv.h"

//ae_target mode def.
#include <hal/inc/custom/aaa/ae_param.h>

//------------Thread-------------
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
#include <mtkcam/utils/common.h>
//-------------------------------

using namespace NSCam::Utils;
using namespace NSCam;


namespace NS3A
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                    Multi-instance.
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDMgr* PDMgr::s_pPDMgr = MNULL;
//----------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------
PDMgr &PDMgr::getInstance(MINT32 const i4SensorDev)
{
    switch ( i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  PDMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  PDMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  PDMgrDev<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Second Sensor
        return  PDMgrDev<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGD("i4SensorDev = %d", i4SensorDev);
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                    init function.
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDMgr::PDMgr(ESensorDev_T eSensorDev)
{
    m_i4CurrSensorDev=(MINT32)eSensorDev;
}

//----------------------------------------------------------------------------------------------------
PDMgr::~PDMgr()
{
}
//----------------------------------------------------------------------------------------------------
MBOOL PDMgr::CamPwrOnState()
{
    CAM_LOGD("%s+", __FUNCTION__);
    MBOOL ret = MTRUE;
    CAM_LOGD("%s-", __FUNCTION__);
    return ret;
}
//----------------------------------------------------------------------------------------------------
MBOOL PDMgr::CamPwrOffState()
{
    CAM_LOGD("%s+", __FUNCTION__);
    MBOOL ret = MTRUE;
    CAM_LOGD("%s-", __FUNCTION__);
    return ret;
}
//----------------------------------------------------------------------------------------------------
EPDBUF_TYPE_t PDMgr::start(MINT32 i4SensorIdx, SPDProfile_t *iPDProfile, MVOID *ptrInTuningData, MINT32 &i4OutInfoSz, MINT32 **ptrOutInfo)
{
    m_bDebugEnable     = property_get_int32("vendor.debug.af_mgr.enable",  0);
    m_i4DgbLogW        = property_get_int32("vendor.debug.af_logw.enable", 0);
    m_i4DbgPdDump      = property_get_int32("vendor.pd.dump.enable",       0);
    m_i4DbgDisPdHandle = property_get_int32("vendor.pdhandle.disable",     0);
    m_i4DbgVcDump      = property_get_int32("vendor.vc.dump.enable",       0);

    CAM_LOGD_IF( m_bDebugEnable,"%s+", __FUNCTION__);

    //create folder for saving debug information as m_i4DbgPdDump is set.
    if( m_i4DbgPdDump)
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
    m_pIHalSensor = pIHalSensorList->createSensor( "pd_mgr", m_i4CurrSensorDev);

    //initial member.
    m_frmCnt         = 0;
    m_Status         = EPD_Not_Enabled;
    m_numROI         = 0;
    m_databuf_size   = 0;
    m_databuf_stride = 0;
    m_databuf        = NULL;
    m_pPDBufMgrCore  = NULL;
    m_pPDBufMgrOpen  = NULL;
    m_pIPdAlgo       = NULL;
    m_i4SensorIdx    = i4SensorIdx;

    memset( m_sPDInput,          0, sizeof(PD_INPUT_T)*AF_PSUBWIN_NUM);
    memset( m_sPDOutput,         0, sizeof(PD_OUTPUT_T)*AF_PSUBWIN_NUM);
    memset( m_i4PDInfo2HybirdAF, 0, sizeof(MINT32)*10);
    memset( m_ROI, 0, sizeof(AREA_T)*AF_PSUBWIN_NUM);

    m_vPDOutput.clear();
    m_vecCfgSetting.clear();

#if 0
    // isp_mgr_pdo should be initialized before operation isp_mgr_pdo.
    ISP_MGR_PDO_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).start(m_i4SensorIdx);
#endif

    //get main information from host
    memcpy( &m_profile, iPDProfile, sizeof(SPDProfile_t));

    // initial profile in pd mgr.
    m_profile.BufType = EPDBUF_NOTDEF;

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
                    CAM_LOGE( "support PDO but pipe(stt) didn't initialized.");
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
                    CAM_LOGE( "support PDVC but pipe(camsv) didn't initialized.");
                }
                else
                {
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
                    CAM_LOGE( "support DualPD_Raw but pipe(stt) didn't initialized.");
                }
                else if( !m_profile.bEnablePBIN)
                {
                    CAM_LOGE("PBIN from 3A (%d) is not match with current PD sensor type (%d)", m_profile.bEnablePBIN, PDAF_support_type);
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
                    CAM_LOGE( "support DualPD_VC but pipe(camsv) didn't initialized.");
                }
                else
                {
                    m_profile.BufType = (EPDBUF_TYPE_t)pdBufType;
                }
            }
            break;

        default:
            CAM_LOGE( "Unknown PD sensor type (%d)", PDAF_support_type);
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
                // Set PD calibration data to PD_mgr and get PD information to Hybrid AF.
                if( setPDCaliData( ptrInTuningData, i4OutInfoSz, ptrOutInfo)!=S_3A_OK)
                {
                    CAM_LOGD( "Set PD calibration and tuning data fail, close PD thread !!");
                    closeThread();
                    m_profile.BufType = EPDBUF_NOTDEF;
                }
            }
        }
        else
        {
            m_profile.BufType = EPDBUF_NOTDEF;
            CAM_LOGD( "Instance is not ready : PDBufMgrCore(%p) PDAlgo(%p) PDBufMgrOpen(%p)", (void*)m_pPDBufMgrCore, (void*)m_pIPdAlgo, (void*)m_pPDBufMgrOpen);
        }
    }
    else
    {
        m_profile.BufType = EPDBUF_NOTDEF;
        CAM_LOGD("Current sensor mode is not support PD");
    }

    //==========
    // Initial PD related HW module.
    //==========
    ConfigurePDHWSetting();

    CAM_LOGD( "SensorID(0x%x), PDAF_Support(%d), SensorModeSupportPD(%d), BufType(0x%x) : (NOTDEF(0x00), VC(0x01), VC_OPEN(0x11), RAW_LEGACY(0x02), RAW_LEGACY_OPEN(0x12), PDO(0x04), PDO_OPEN(0x14), DUALPD_VC(0x21), DUALPD_RAW(0x24)",
              m_profile.i4CurrSensorId,
              m_profile.u4PDAF_support_type,
              m_profile.bSensorModeSupportPD,
              m_profile.BufType);

    //==========
    // Output
    //==========
    iPDProfile->BufType = m_profile.BufType;

    CAM_LOGD_IF( m_bDebugEnable,"%s-", __FUNCTION__);

    return m_profile.BufType;
}
//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::stop()
{
    CAM_LOGD("%s+", __FUNCTION__);

    if( m_pIHalSensor)
    {
        m_pIHalSensor->destroyInstance("pd_mgr");
    }

    if( m_bEnPDBufThd)
    {
        //close thread
        closeThread();
    }

    //
    {
        Mutex::Autolock lock(m_Lock_PDWinCfg);

        for( MUINT32 i=0; i<m_vecCfgSetting.size(); i++)
        {
            m_vecCfgSetting.at(i).vecROIs.clear();
        }
        m_vecCfgSetting.clear();
    }

#if 0
    // stop isp_mgr_pdo
    ISP_MGR_PDO_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).stop();
#endif

    //uninit member
    m_databuf_size=0;
    m_databuf_stride = 0;

    if( m_databuf)
        delete m_databuf;
    m_databuf=NULL;

    m_pPDBufMgrCore = NULL;
    m_pPDBufMgrOpen = NULL;

    m_pIPdAlgo = NULL;

    {
        Mutex::Autolock lock(m_Lock_Result);
        m_vPDOutput.clear();
    }

    CAM_LOGD("%s-", __FUNCTION__);

    return S_3A_OK;
}
//----------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::ConfigurePDHWSetting()
{
    MRESULT ret=E_3A_ERR;

    if( m_profile.u4PDPipeCtrl==FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
    {
        if( m_profile.BufType==EPDBUF_VC_OPEN)
        {
            if( m_vecCfgSetting.size()!=0)
            {
                // query sensor driver setting for 3rd party PD library.
                MUINT32  arg2=0;
                MUINT16 *arg3=NULL;
                MBOOL    res  = MFALSE;

                if( m_pPDBufMgrOpen)
                {
                    AAA_TRACE_D("PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING");
                    res  = m_pPDBufMgrOpen->sendCommand(
                               PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING,
                               (MVOID *)(&(m_vecCfgSetting.at(m_vecCfgSetting.size()-1))),
                               (MVOID *)(&arg2),
                               (MVOID *)(&arg3));
                    AAA_TRACE_END_D;
                }

                if( res && m_pIHalSensor && arg2 && arg3)
                {
                    AAA_TRACE_D("SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING");
                    m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING, (MUINTPTR)&arg2, (MUINTPTR)&arg3, 0);
                    AAA_TRACE_END_D;
                }
                else
                {
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "%s Can not get PD windor HW setting. Use setting in kernel driver. Send command result(%d) : size(%d), addr(%p)",
                                 __FUNCTION__,
                                 res,
                                 arg2,
                                 arg3);
                }
            }
            else
            {
                CAM_LOGD_IF( m_bDebugEnable, "%s No vaild PD window configuration", __FUNCTION__);
            }
        }
        else
        {
            // Do nothing.
            ret = E_3A_ERR;
        }
    }
    else if( (m_profile.u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_PBN_PDO) || (m_profile.u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_BNR_PDO))
    {
#if 0
        //
        IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM PDAF_support_type = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)m_profile.u4PDAF_support_type;

        if( PDAF_support_type == PDAF_SUPPORT_RAW)
        {
            // Get BPCI table information for PDO
            if( ISP_MGR_BNR2::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getBPCItable(
                        m_sPDOHWInfo.u4Bpci_xsz,
                        m_sPDOHWInfo.u4Bpci_ysz,
                        m_sPDOHWInfo.u4Pdo_xsz,
                        m_sPDOHWInfo.u4Pdo_ysz,
                        m_sPDOHWInfo.phyAddrBpci_tbl,
                        m_sPDOHWInfo.virAddrBpci_tbl,
                        m_sPDOHWInfo.i4memID,
                        eIDX_PDE) == MFALSE)
            {
                CAM_LOGE("Failed acquiring BPCI table!");
                ret = E_3A_ERR;
            }
            else
            {
                if( m_pPDBufMgrCore != NULL)
                {
                    m_pPDBufMgrCore->SetBpciInfo( m_sPDOHWInfo.u4Pdo_xsz, m_sPDOHWInfo.u4Pdo_ysz);
                }


                CAM_LOGD( "%s BPCI table first 4 dword: %x %x %x %x",
                          __FUNCTION__,
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[0],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[1],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[2],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[3]);

                ret = S_3A_OK;
            }
        }

        /**********************************************************************************************************************************************
         *
         * Notice :
         *   m_sPDOHWInfo is initial during setting PDAF and tuning data, which means that ConfigurePDHWSetting is vaild after setPDCaliData is exectued.
         *   Need to config PDO HW once when u4PDPipeCtrl is FEATURE_PDAF_SUPPORT_PBN_PDO or FEATURE_PDAF_SUPPORT_BNR_PDO, otherwise system crash.
         *   No matter if m_pPDBufMgrCore or m_pIPdAlgo is NULL
         *
         ***********************************************************************************************************************************************/
        CAM_LOGD( "%s PDOHWInfo Bpci_Xsz(%d) Bpci_Ysz(%d) Bpci_tbl(%p) pa(%p) va(%p) memID(%d) Pdo_Xsz(%d) Pdo_Ysz(%d) BitDepth(%d) IsDualPD(%d) PBinType(%d) PBinStartLine(%d) PdSeparateMode(%d)",
                  __FUNCTION__,
                  m_sPDOHWInfo.u4Bpci_xsz,
                  m_sPDOHWInfo.u4Bpci_ysz,
                  (void*)m_sPDOHWInfo.pu1Bpci_tbl,
                  (void*)m_sPDOHWInfo.phyAddrBpci_tbl,
                  (void*)m_sPDOHWInfo.virAddrBpci_tbl,
                  m_sPDOHWInfo.i4memID,
                  m_sPDOHWInfo.u4Pdo_xsz,
                  m_sPDOHWInfo.u4Pdo_ysz,
                  m_sPDOHWInfo.u4BitDepth,
                  m_sPDOHWInfo.u1IsDualPD,
                  m_sPDOHWInfo.u1PBinType,
                  m_sPDOHWInfo.u1PBinStartLine,
                  m_sPDOHWInfo.u1PdSeparateMode);


        //configure ISP PD related HW module, if PDO port is configured to sttPipe by 3A framework.
        ISP_MGR_PDO_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).config( m_sPDOHWInfo);
#endif
    }
    else
    {
        // Do nothing.
        ret = E_3A_ERR;
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                        Thread
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID PDMgr::createThread()
{
    CAM_LOGD( "create PD mgr thread");
    //create thread
    m_bEnPDBufThd = MTRUE;
    sem_init(&m_semPDBuf, 0, 0);
    sem_init(&m_semPDBufThdEnd, 0, 1);
    pthread_create( &m_PDBufThread, NULL, PDBufThreadLoop, this);
}
//----------------------------------------------------------------------------------------------------
MVOID PDMgr::closeThread()
{
    CAM_LOGD("close PD mgr thread");
    //close thread
    m_bEnPDBufThd = MFALSE;
    ::sem_post(&m_semPDBuf);
    pthread_join( m_PDBufThread, NULL);
}
//----------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------
MVOID* PDMgr::PDBufThreadLoop(MVOID *arg)
{
    MRESULT ret=E_3A_ERR;

    PDMgr *_this = reinterpret_cast<PDMgr*>(arg);

    CAM_LOGD( "[%s]+", __FUNCTION__);

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
        CAM_LOGD_IF(_this->m_bDebugEnable, "[PDBufThreadLoop] PD task, dev:%d", _this->m_i4CurrSensorDev);



        _this->m_Status = EPD_BZ;

        CAM_LOGD_IF(_this->m_bDebugEnable, "Run Core Flow +");

        AAA_TRACE_D( "PDBufThreadLoop");
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
        CAM_LOGD_IF(_this->m_bDebugEnable, "Run Core Flow -");

        //thread control
        {
            int Val;
            ::sem_getvalue(&_this->m_semPDBufThdEnd, &Val);
            CAM_LOGD_IF(_this->m_bDebugEnable, "[PDBufThreadLoop] semPDThdEnd before post = %d\n", Val);
            if (Val == 0)
            {
                ::sem_post(&_this->m_semPDBufThdEnd); //to be 1, 1 means PD task done
            }
        }

        //ready output data
        _this->m_Status = EPD_Data_Ready;

        //check pd flow status.
        if(ret==E_3A_ERR)
        {
            CAM_LOGD_IF(_this->m_bDebugEnable, "[PDBufThreadLoop] PD mgr thread is colsed because of PD flow error!!");
            break;
        }
        AAA_TRACE_END_D;
    }

    _this->m_bEnPDBufThd = MFALSE;
    _this->m_Status = EPD_Not_Enabled;


    CAM_LOGD( "[%s]-", __FUNCTION__);
    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                      Data path 1 : using protect PD algorithm
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT PDMgr::SetCaliData2PDCore(MVOID* ptrInTuningData, MVOID* ptrInCaliData, MINT32 i4InPDInfoSz, MINT *ptrOutPDInfo)
{
    CAM_LOGD( "[Core] set calibration data flow");

    MRESULT ret = E_3A_ERR;
    MRESULT retAll = S_3A_OK;

    PD_ALGO_TUNING_T *ptrTuningData = reinterpret_cast<PD_ALGO_TUNING_T *>(ptrInTuningData);
    PD_CALIBRATION_DATA_T *ptrCaliData = reinterpret_cast<PD_CALIBRATION_DATA_T *>(ptrInCaliData);

    PD_INIT_T initPdData;

    //for dualpd
    MINT32 binningX;
    MINT32 binningY;

    //get calibration data from host
    initPdData.rPDNVRAM.rCaliData.i4Size = ptrCaliData->i4Size;
    memcpy( initPdData.rPDNVRAM.rCaliData.uData, ptrCaliData->uData, ptrCaliData->i4Size);

    //get tuning data
    memcpy( &initPdData.rPDNVRAM.rTuningData, ptrTuningData, sizeof(PD_ALGO_TUNING_T));


    CAM_LOGD( "Get calibration/tuning data");

    char strMsgBuf[256];
    char* strTarget = strMsgBuf;

    strTarget += sprintf(strMsgBuf, "T");
    for (MINT32 i=0; i<PD_CONF_IDX_SIZE; i++)
        strTarget += sprintf(strTarget, " %d", initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[i]);
    for (MINT32 i=0; i<PD_CONF_IDX_SIZE; i++)
        strTarget += sprintf(strTarget, " %d", initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[i]);
    CAM_LOGD( "%s", strMsgBuf);

    for( MINT32 i=0; i<(PD_CONF_IDX_SIZE+1); i++)
    {
        strTarget = strMsgBuf;
        strTarget += sprintf(strTarget, "-");
        for (MINT32 j=0; j<(PD_CONF_IDX_SIZE+1); j++)
            strTarget += sprintf(strTarget, " %d", initPdData.rPDNVRAM.rTuningData.i4ConfTbl[i][j]);
        CAM_LOGD( "%s", strMsgBuf);
    }

    CAM_LOGD( "ConfThr=%d, SaturateLevel=%d, i4SaturateThr=%d, FocusPDSizeX=%d, FocusPDSizeY=%d, sz=%zu",
              initPdData.rPDNVRAM.rTuningData.i4ConfThr,
              initPdData.rPDNVRAM.rTuningData.i4SaturateLevel,
              initPdData.rPDNVRAM.rTuningData.i4SaturateThr,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeX,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeY,
              sizeof(NVRAM_LENS_PARA_STRUCT));

    //(1) set calibration data to PD core.
    if( m_i4DbgPdDump)
    {
        FILE *fp = NULL;

        fp = fopen("/sdcard/pdo/__pd_cali_data.raw", "w");
        fwrite( reinterpret_cast<void *>(initPdData.rPDNVRAM.rCaliData.uData), 1, 0x800, fp);
        fclose( fp);

        fp = fopen("/sdcard/pdo/__pd_tuning_data.raw", "w");
        fwrite( reinterpret_cast<void *>(&initPdData.rPDNVRAM.rTuningData), 1, sizeof(PD_ALGO_TUNING_T), fp);
        fclose( fp);
    }

    if (m_pIPdAlgo == NULL || (ret = m_pIPdAlgo->initPD(initPdData)) != S_3A_OK)
    {
        CAM_LOGD( "Load PDAF calib data error!!");

        for( MINT32 kidx=0; kidx<0x600; kidx=kidx+16)
        {
            strTarget = strMsgBuf;
            strTarget += sprintf(strTarget, "[Core] ===");
            for (MINT i=0; i<16; i++)
                strTarget += sprintf(strTarget, " 0x%02x,", initPdData.rPDNVRAM.rCaliData.uData[kidx+i]);
            CAM_LOGD( "%s", strMsgBuf);
        }
        retAll = E_3A_ERR;
    }


    //(2) set pd block info or dualpd hw info
    SET_PD_BLOCK_INFO_T sPDSensorInfo;
    SensorVCInfo rSensorVCInfo;
    memset( &sPDSensorInfo, 0, sizeof(SET_PD_BLOCK_INFO_T));
    memset( &rSensorVCInfo, 0, sizeof(SensorVCInfo));

    MINT32 sensorMode = m_profile.i4SensorMode;
    if (m_pIHalSensor)
    {
        m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_INFO, (MINTPTR)&sensorMode, (MINTPTR)&sPDSensorInfo, 0);
        m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&sensorMode, 0);
    }
    else
    {
        CAM_LOGE("m_pIHalSensor is NULL!");
    }

    CAM_LOGD( "SensorMode: %d(%d), SensorInfo : OffsetX(%d), OffsetY(%d), PitchX(%d), PitchY(%d), PairNu(%d), SubBlkW(%d), SubBlkH(%d), BlockNumX(%d), BlockNumY(%d), LeFirst(%d)",
              sensorMode,
              SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
              sPDSensorInfo.i4OffsetX,
              sPDSensorInfo.i4OffsetY,
              sPDSensorInfo.i4PitchX,
              sPDSensorInfo.i4PitchY,
              sPDSensorInfo.i4PairNum,
              sPDSensorInfo.i4SubBlkW,
              sPDSensorInfo.i4SubBlkH,
              sPDSensorInfo.i4BlockNumX,
              sPDSensorInfo.i4BlockNumY,
              sPDSensorInfo.i4LeFirst);

    CAM_LOGD( "VC2 Info : DataType(0x%x), ID(0x%x), SIZEH(0x%x), SIZEV(0x%x)",
              rSensorVCInfo.VC2_DataType,
              rSensorVCInfo.VC2_ID,
              rSensorVCInfo.VC2_SIZEH,
              rSensorVCInfo.VC2_SIZEV);

    //
    if( m_profile.u4PDAF_support_type == PDAF_SUPPORT_RAW_DUALPD)
    {
        m_sPDOHWInfo.u1IsDualPD = 1;

        m_sPDOHWInfo.u4BitDepth = 12;

        m_sPDOHWInfo.u1PBinType = (m_profile.AETargetMode == AE_MODE_IVHDR_TARGET) ? 0 : 1; /*0:1x4, 1:4x4*/

        m_sPDOHWInfo.u1PdSeparateMode = m_profile.uPdSeparateMode;


        binningX                = initPdData.rPDNVRAM.rTuningData.i4Reserved[0];
        binningY                = initPdData.rPDNVRAM.rTuningData.i4Reserved[1];

        MINT32 needShortExp     = initPdData.rPDNVRAM.rTuningData.i4Reserved[2];

        if( /* error check */
            (binningX > 8) ||
            (binningY > 8) ||
            (needShortExp > 2) ||
            (needShortExp < 0) ||
            (binningX < 1) ||
            (binningY < 1))
        {
            CAM_LOGE( "%s, dualpd tuning data error! (%d %d %d)",
                      __FUNCTION__,
                      binningX,
                      binningY,
                      needShortExp);
            binningX     = 8;
            binningY     = 8;
            needShortExp = 0;
        }

        if( m_sPDOHWInfo.u1PdSeparateMode == 1)
        {
            m_sPDOHWInfo.u4Pdo_xsz = (m_profile.uImgXsz / binningX)     - 1;
            m_sPDOHWInfo.u4Pdo_ysz = (m_profile.uImgYsz / binningY) * 2 - 1;
        }
        else if( m_sPDOHWInfo.u1PdSeparateMode == 0)
        {
            m_sPDOHWInfo.u4Pdo_xsz = (m_profile.uImgXsz / binningX) * 2 - 1;
            m_sPDOHWInfo.u4Pdo_ysz = (m_profile.uImgYsz / binningY)     - 1;
        }
        else
        {
            CAM_LOGE("[%s]- separate mode setting ERROR!", __FUNCTION__);
        }

        m_sPDOHWInfo.u1PBinStartLine = needShortExp                      ?
                                       (sPDSensorInfo.i4LeFirst ? 1 : 0) :
                                       (sPDSensorInfo.i4LeFirst ? 0 : 1) ;
        m_sPDOHWInfo.u1PBinStartLine +=  (m_sPDOHWInfo.u1PBinType == 1) ? 0 : 2; // in 1x4, start line + 2

        CAM_LOGD( "DualPD PBIN Type (0:1x4, 1:4x4) %d, start line: %d", m_sPDOHWInfo.u1PBinType, m_sPDOHWInfo.u1PBinStartLine);
    }
    else if( m_profile.u4PDAF_support_type == PDAF_SUPPORT_CAMSV_DUALPD)
    {
        m_sPDOHWInfo.u1IsDualPD = 1;
        m_sPDOHWInfo.u4BitDepth = 10;
    }
    else
    {
        m_sPDOHWInfo.u1IsDualPD = 0;
        m_sPDOHWInfo.u4BitDepth = m_profile.u4RawBit;

        //
        m_pPDBufMgrCore->SetPDBlockInfo( sPDSensorInfo);
    }


    //(3) configure pd orientation information.
    ePDWIN_ORIENTATION_T PDOrientation;
    switch( sPDSensorInfo.iMirrorFlip)
    {
    /* 0:IMAGE_NORMAL,1:IMAGE_H_MIRROR,2:IMAGE_V_MIRROR,3:IMAGE_HV_MIRROR*/
    case 3 :
        PDOrientation = ePDWIN_M1F1;
        break;
    case 2 :
        PDOrientation = ePDWIN_M0F1;
        break;
    case 1 :
        PDOrientation = ePDWIN_M1F0;
        break;
    case 0 :
    default :
        PDOrientation = ePDWIN_M0F0;
        break;
    }
    CAM_LOGD( "Orientation %d", PDOrientation);

    if ((ret = m_pIPdAlgo->setPDOrientation(PDOrientation)) != S_3A_OK)
        retAll = E_3A_ERR;


    //(4) configure pd algorithm.
    PD_CONFIG_T a_sPDConfig;
    memset (&a_sPDConfig, 0, sizeof(PD_CONFIG_T));

    a_sPDConfig.i4RawWidth  = m_profile.uImgXsz;
    a_sPDConfig.i4RawHeight = m_profile.uImgYsz;
    a_sPDConfig.i4Bits      = m_sPDOHWInfo.u4BitDepth;

    if (!m_sPDOHWInfo.u1IsDualPD)
    {
        a_sPDConfig.i4IsPacked = 1;
        a_sPDConfig.i4RawStride = m_profile.uImgXsz*a_sPDConfig.i4Bits/8;
        a_sPDConfig.sPdBlockInfo.i4BlockNumX = sPDSensorInfo.i4BlockNumX;
        a_sPDConfig.sPdBlockInfo.i4BlockNumY = sPDSensorInfo.i4BlockNumY;
        a_sPDConfig.sPdBlockInfo.i4OffsetX = sPDSensorInfo.i4OffsetX;
        a_sPDConfig.sPdBlockInfo.i4OffsetY = sPDSensorInfo.i4OffsetY;

        sPDSensorInfo.i4PairNum = (sPDSensorInfo.i4PairNum > MAX_PD_PAIR_NUM) ? MAX_PD_PAIR_NUM : sPDSensorInfo.i4PairNum;

        a_sPDConfig.sPdBlockInfo.i4PairNum = sPDSensorInfo.i4PairNum;
        a_sPDConfig.sPdBlockInfo.i4PitchX  = sPDSensorInfo.i4PitchX;
        a_sPDConfig.sPdBlockInfo.i4PitchY  = sPDSensorInfo.i4PitchY;
        a_sPDConfig.sPdBlockInfo.i4SubBlkH = sPDSensorInfo.i4SubBlkH;
        a_sPDConfig.sPdBlockInfo.i4SubBlkW = sPDSensorInfo.i4SubBlkW;

        //calculate pd pixels' position by orientation
        {
            MUINT32 offX = sPDSensorInfo.i4OffsetX;
            MUINT32 offY = sPDSensorInfo.i4OffsetY;
            MUINT32 pitX = sPDSensorInfo.i4PitchX;
            MUINT32 pitY = sPDSensorInfo.i4PitchY;
            MUINT32 pairNum =    sPDSensorInfo.i4PairNum;
            MUINT32 pdPixels[MAX_PAIR_NUM * 2][2];

            for( MUINT32 Pidx=0; Pidx<pairNum; Pidx++)
            {
                MUINT32 PosL_X = sPDSensorInfo.i4PosL[Pidx][0];
                MUINT32 PosL_Y = sPDSensorInfo.i4PosL[Pidx][1];
                MUINT32 PosR_X = sPDSensorInfo.i4PosR[Pidx][0];
                MUINT32 PosR_Y = sPDSensorInfo.i4PosR[Pidx][1];

                if (sPDSensorInfo.iMirrorFlip & 0x1) //mirror
                {
                    PosL_X = pitX - (PosL_X - offX) - 1 + offX;
                    PosR_X = pitX - (PosR_X - offX) - 1 + offX;
                }
                if (sPDSensorInfo.iMirrorFlip & 0x2) //flip
                {
                    PosL_Y = pitY - (PosL_Y - offY) - 1 + offY;
                    PosR_Y = pitY - (PosR_Y - offY) - 1 + offY;
                }
                pdPixels[Pidx][0] = PosR_X;
                pdPixels[Pidx][1] = PosR_Y;
                pdPixels[Pidx + pairNum][0] = PosL_X;
                pdPixels[Pidx + pairNum][1] = PosL_Y;

                CAM_LOGD( "PDPos [L](%3d, %3d)->(%3d, %3d), [R](%3d, %3d)->(%3d, %3d)",
                          sPDSensorInfo.i4PosL[Pidx][0],
                          sPDSensorInfo.i4PosL[Pidx][1],
                          PosL_X,
                          PosL_Y,
                          sPDSensorInfo.i4PosR[Pidx][0],
                          sPDSensorInfo.i4PosR[Pidx][1],
                          PosR_X,
                          PosR_Y);

                MUINT32 idx = (sPDSensorInfo.iMirrorFlip & 0x2) ? ((pairNum - 1) - Pidx) : Pidx;
                a_sPDConfig.sPdBlockInfo.i4PosR[idx][0] = m_pdoPixels[idx][0] =           pdPixels[Pidx][0];
                a_sPDConfig.sPdBlockInfo.i4PosR[idx][1] = m_pdoPixels[idx][1] =           pdPixels[Pidx][1];
                a_sPDConfig.sPdBlockInfo.i4PosL[idx][0] = m_pdoPixels[idx + pairNum][0] = pdPixels[Pidx + pairNum][0];
                a_sPDConfig.sPdBlockInfo.i4PosL[idx][1] = m_pdoPixels[idx + pairNum][1] = pdPixels[Pidx + pairNum][1];
            }

            //
            if (m_profile.BufType == EPDBUF_PDO)
            {
                for( MUINT32 Pidx=0; Pidx<pairNum; Pidx++)
                {
                    CAM_LOGD( "Pdo pixel position [R](%3d, %3d) [L](%3d, %3d)",
                              m_pdoPixels[Pidx][0],
                              m_pdoPixels[Pidx][1],
                              m_pdoPixels[Pidx + pairNum][0],
                              m_pdoPixels[Pidx + pairNum][1]);
                }
                if (m_pPDBufMgrCore->pdoPixelPos(m_pdoPixels, pairNum*2) == MFALSE)
                {
                    CAM_LOGE("pdoPixelsPos parsing PD coordinates Error!");
                    retAll = E_3A_ERR;
                }
                if (m_pPDBufMgrCore->pdoPixelPos(m_pdoPixels, pairNum*2) == MFALSE)
                {
                    CAM_LOGE("pdoPixelsPos parsing PD coordinates Error!");
                    retAll = E_3A_ERR;
                }
            }
        }
        CAM_LOGD( "[Core] bits(%d) isPack(%d) w(%d) h(%d) stride(%d) blknumX(%d) blknumY(%d) OffX(%d) OffY(%d) pairNum(%d) pitchX(%d) pitchY(%d) subblkH(%d) subblkW(%d)",
                  a_sPDConfig.i4Bits,
                  a_sPDConfig.i4IsPacked,
                  a_sPDConfig.i4RawWidth,
                  a_sPDConfig.i4RawHeight,
                  a_sPDConfig.i4RawStride,
                  a_sPDConfig.sPdBlockInfo.i4BlockNumX,
                  a_sPDConfig.sPdBlockInfo.i4BlockNumY,
                  a_sPDConfig.sPdBlockInfo.i4OffsetX,
                  a_sPDConfig.sPdBlockInfo.i4OffsetY,
                  a_sPDConfig.sPdBlockInfo.i4PairNum,
                  a_sPDConfig.sPdBlockInfo.i4PitchX,
                  a_sPDConfig.sPdBlockInfo.i4PitchY,
                  a_sPDConfig.sPdBlockInfo.i4SubBlkH,
                  a_sPDConfig.sPdBlockInfo.i4SubBlkW);
    }
    else //dualpd related
    {
        a_sPDConfig.i4FullRawWidth = m_profile.uFullXsz;
        a_sPDConfig.i4FullRawHeight = m_profile.uFullYsz;

        MINT32 sensorMode = m_profile.i4SensorMode;
        a_sPDConfig.i4FullRawXOffset = sPDSensorInfo.i4Crop[sensorMode][0];
        a_sPDConfig.i4FullRawYOffset = sPDSensorInfo.i4Crop[sensorMode][1];

        a_sPDConfig.sDPdFormat.i4Bits = m_sPDOHWInfo.u4BitDepth;

        if (m_profile.u4PDAF_support_type == PDAF_SUPPORT_RAW_DUALPD)
        {
            a_sPDConfig.sDPdFormat.i4BinningX = binningX;
            a_sPDConfig.sDPdFormat.i4BinningY = binningY;
            a_sPDConfig.sDPdFormat.i4BufFmt   = (m_sPDOHWInfo.u1PdSeparateMode == 1) ? 1 : 0;
            a_sPDConfig.sDPdFormat.i4BufStride = _PDO_STRIDE_ALIGN_((m_sPDOHWInfo.u4Pdo_xsz+1)*sizeof(MUINT16));
            a_sPDConfig.sDPdFormat.i4BufHeight = m_sPDOHWInfo.u4Pdo_ysz + 1;
        }
        else if (m_profile.u4PDAF_support_type == PDAF_SUPPORT_CAMSV_DUALPD && m_pPDBufMgrCore != NULL)
        {
            //
            m_pPDBufMgrCore->GetDualPDVCInfo( 0, m_sDualPDVCInfo, m_profile.AETargetMode);
            //
            a_sPDConfig.sDPdFormat.i4BinningX = m_sDualPDVCInfo.u4VCBinningX;
            a_sPDConfig.sDPdFormat.i4BinningY = m_sDualPDVCInfo.u4VCBinningY;
            a_sPDConfig.sDPdFormat.i4BufFmt   = m_sDualPDVCInfo.u4VCBufFmt;
            //
            a_sPDConfig.sDPdFormat.i4BufStride = rSensorVCInfo.VC2_SIZEH;
            a_sPDConfig.sDPdFormat.i4BufHeight = rSensorVCInfo.VC2_SIZEV;
        }

        CAM_LOGD_IF(m_bDebugEnable, "[Core] %d/%d, %d/%d, %d/%d/%d, %d/%d, %d",
                    a_sPDConfig.i4FullRawWidth, a_sPDConfig.i4FullRawHeight,
                    a_sPDConfig.i4FullRawXOffset, a_sPDConfig.i4FullRawXOffset,
                    a_sPDConfig.sDPdFormat.i4BufStride, a_sPDConfig.sDPdFormat.i4BufHeight, a_sPDConfig.sDPdFormat.i4Bits,
                    a_sPDConfig.sDPdFormat.i4BinningX, a_sPDConfig.sDPdFormat.i4BinningY,
                    a_sPDConfig.sDPdFormat.i4BufFmt);
    }

    //check configure core PD algorithm is correct or not.
    if ((ret = m_pIPdAlgo->setPDBlockInfo(a_sPDConfig)) != S_3A_OK)
    {
        CAM_LOGD("[Core] PD init data error");
        retAll = E_3A_ERR;
    }

    //(5) get information for hybrid af.
    memset( ptrOutPDInfo, 0, i4InPDInfoSz*sizeof(MINT32));
    if ((ret = m_pIPdAlgo->getInfoForHybridAF( ptrOutPDInfo)) != S_3A_OK)
        retAll = E_3A_ERR;

    if (retAll == S_3A_OK)
        CAM_LOGD( "[Core] configure PD algo done %d %d", ptrOutPDInfo[0], ptrOutPDInfo[1]);
    else
        CAM_LOGD( "[Core] configure PD algo failed! close PDAF! %d %d", ptrOutPDInfo[0], ptrOutPDInfo[1]);

    return retAll;
}

MUINT16* PDMgr::PDConvert(MUINT8* pInBuf, MUINT32 inBufSize, MUINT32 inBufStride)
{
    CAM_LOGD_IF( m_bDebugEnable, "[%s]+", __FUNCTION__);

    MUINT16 *ptrbuf = NULL;

    AAA_TRACE_D("pdconvert");
    ptrbuf = m_pPDBufMgrCore->ConvertPDBufFormat( inBufSize, inBufStride, pInBuf, MFALSE);
    AAA_TRACE_END_D;

    if (m_i4DbgPdDump)
    {
        char fileName[64];
        sprintf(fileName, "/sdcard/pdo/pd_LR_%d_%d.raw", m_MagicNumber, m_frmCnt);
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            MINT32 err = mkdir("/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("create folder /sdcard/pdo (%d)", err);
        }
        else
        {
            MUINT32 PDXsz, PDYsz, PDBufSz;
            m_pPDBufMgrCore->GetLRBufferInfo(PDXsz, PDYsz, PDBufSz);
            CAM_LOGD_IF(m_bDebugEnable, "fileName %s : PDXsz(%d) PDYsz(%d) PDBufSz(%d)\n", fileName, PDXsz, PDYsz, PDBufSz);
            fwrite(reinterpret_cast<void *>(ptrbuf), 1, PDBufSz*2, fp);
            fclose(fp);
        }
    }

    if (m_i4DbgPdDump)
    {
        char fileName[64];
        sprintf(fileName, "/sdcard/pdo/pd_%d_%d.raw", m_MagicNumber, m_frmCnt);
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            MINT32 err = mkdir("/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("create folder /sdcard/pdo (%d)", err);
        }
        else
        {
            CAM_LOGD_IF(m_bDebugEnable, "fileName %s : inBufSize(%d)", fileName, inBufSize);
            fwrite(reinterpret_cast<void *>(pInBuf), 1, inBufSize, fp);
            fclose(fp);
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "[%s]-", __FUNCTION__);
    return ptrbuf;
}


MRESULT PDMgr::PDCoreFlow()
{
    CAM_LOGD_IF( m_bDebugEnable, "[%s]+", __FUNCTION__);

    MRESULT ret=S_3A_OK;

    if ((m_pIPdAlgo!=NULL))
    {
        SPDResult_T PDRes;
        //reset output result.
        memset( &PDRes, 0, sizeof(SPDResult_T));

        //output all 0	result directly once current lens position is 0
        if(
            ( m_curLensPos                  ) &&
            ( 0<m_numROI                    ) &&
            (   m_numROI<=MAX_SIZE_OF_PD_ROI) )
        {
            AAA_TRACE_D("pdhandle #(%d)", m_MagicNumber);

            if (!m_i4DbgDisPdHandle)
            {
                for (MUINT32 i = 0; i < m_numROI; i++)
                {
                    m_sPDInput[i].bIsFace      = 0;
                    m_sPDInput[i].i4CurLensPos = m_curLensPos;
                    m_sPDInput[i].sPDExtractData.pPDLData   = m_sPDExtractOutput[i].pPDLData;
                    m_sPDInput[i].sPDExtractData.pPDLPos    = m_sPDExtractOutput[i].pPDLPos;
                    m_sPDInput[i].sPDExtractData.pPDRData   = m_sPDExtractOutput[i].pPDRData;
                    m_sPDInput[i].sPDExtractData.pPDRPos    = m_sPDExtractOutput[i].pPDRPos;
                    m_sPDInput[i].sPDExtractData.sPdWin.i4H = m_sPDExtractOutput[i].sPdWin.i4H;
                    m_sPDInput[i].sPDExtractData.sPdWin.i4W = m_sPDExtractOutput[i].sPdWin.i4W;
                    m_sPDInput[i].sPDExtractData.sPdWin.i4X = m_sPDExtractOutput[i].sPdWin.i4X;
                    m_sPDInput[i].sPDExtractData.sPdWin.i4Y = m_sPDExtractOutput[i].sPdWin.i4Y;
                    //dualpd related
                    m_sPDInput[i].sPDExtractData.sPdBlk.i4H = m_sPDExtractOutput[i].sPdBlk.i4H;
                    m_sPDInput[i].sPDExtractData.sPdBlk.i4W = m_sPDExtractOutput[i].sPdBlk.i4W;
                    m_sPDInput[i].sPDExtractData.sPdBlk.i4X = m_sPDExtractOutput[i].sPdBlk.i4X;
                    m_sPDInput[i].sPDExtractData.sPdBlk.i4Y = m_sPDExtractOutput[i].sPdBlk.i4Y;
                    m_sPDInput[i].i4SensorAGain = m_afeGain;

                    ret = m_pIPdAlgo->handlePD(m_sPDInput[i],m_sPDOutput[i]);

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "win %d [Confidence] %d [LensPos] %d->%d [value] %d\n",
                                 i,
                                 m_sPDOutput[i].i4ConfidenceLevel,
                                 m_curLensPos,
                                 m_sPDOutput[i].i4FocusLensPos,
                                 (MINT32)(m_sPDOutput[i].fPdValue*1000));

                    if( ret==E_3A_ERR)
                    {
                        CAM_LOGE("%s", __FUNCTION__);
                        break;
                    }

                    //set output data puffer.
                    PDRes.ROIRes[i].Defocus                = Boundary(0, m_sPDOutput[i].i4FocusLensPos, 1023);
                    PDRes.ROIRes[i].DefocusConfidence      = m_sPDOutput[i].i4ConfidenceLevel;
                    PDRes.ROIRes[i].DefocusConfidenceLevel = m_sPDOutput[i].i4ConfidenceLevel;
                    PDRes.ROIRes[i].PhaseDifference        = (MINT32)(m_sPDOutput[i].fPdValue*1000);
                }
            }
            else
                ret = S_3A_OK;

            AAA_TRACE_END_D;
        }
        else
        {
            CAM_LOGD_IF( m_bDebugEnable,
                         "Not execute %s, lens pos:%d, number of ROI:%d",
                         __FUNCTION__,
                         m_curLensPos,
                         m_numROI);
        }

        if( ret==S_3A_OK)
        {
            Mutex::Autolock lock(m_Lock_Result);
            m_vPDOutput.push_back( PDRes);
            if( 1<m_vPDOutput.size())
            {
                //pop_front and keep only last 2 result
                m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                CAM_LOGD("pop_front, sz=%zu\n", m_vPDOutput.size());
            }
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "[%s]-", __FUNCTION__);
    return ret;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                                                Data path 2 : using 3rd party PD algorithm
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT PDMgr::SetCaliData2PDOpenCore(MVOID* /*ptrInTuningData*/, MVOID* ptrInCaliData, MINT32 i4InPDInfoSz, MINT32 *ptrOutPDInfo)
{
    CAM_LOGD("%s +", __FUNCTION__);

    MRESULT ret=E_3A_ERR;

    if( m_pPDBufMgrOpen)
    {
        PD_CALIBRATION_DATA_T *ptrCaliData = reinterpret_cast<PD_CALIBRATION_DATA_T *>(ptrInCaliData);
        MUINT8 *ptrData = ptrCaliData->uData;
        MINT32 sz = m_pPDBufMgrOpen->GetPDCalSz();

        AAA_TRACE_D("SetCaliData2PDOpenCore");

        // query sensor driver setting for 3rd party PD library.
        MUINT32  arg1=0;
        MUINT16 *arg2=NULL;
        if( m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST, (MVOID *)(&arg1), (MVOID *)(&arg2), NULL))
        {
            if( m_pIHalSensor)
            {
                m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING, (MUINTPTR)&arg1, (MUINTPTR)&arg2, 0);
            }
        }

        //set calibration to PD open core.
        m_pPDBufMgrOpen->SetCalibrationData( sz, ptrData);

        //get PD related information form Hybrid AF.
        m_pPDBufMgrOpen->GetPDInfo2HybridAF( i4InPDInfoSz, ptrOutPDInfo);

        //
        ret = S_3A_OK;

        AAA_TRACE_END_D;
    }

    CAM_LOGD("%s -", __FUNCTION__);
    return ret;
}

MRESULT PDMgr::PDOpenCoreFlow()
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] %d +", __FUNCTION__, m_numROI);

    SPDROIInput_T iPDInputData;
    SPDROIResult_T oPdOutputData;

    AAA_TRACE_D("SetDataBuf #(%d)", m_MagicNumber);
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
        MUINT32 flag = 0;

        AAA_TRACE_D("PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO");
        MBOOL   res  = m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO, (MVOID *)(&mode), (MVOID *)(&flag));
        AAA_TRACE_END_D;

        AAA_TRACE_D("GetDefocus #(%d)", m_MagicNumber);
        {
            Mutex::Autolock lock(m_Lock_PDWinCfg);

            // Debug only
            if( m_bDebugEnable)
            {
                CAM_LOGD_IF( m_bDebugEnable, "dump cfgSetting +");
                for( MUINT32 i=0; i<m_vecCfgSetting.size(); i++)
                {
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "[%d] frmNum(%d) cfgNum(0x%x) szROI(%d)",
                                 i,
                                 m_vecCfgSetting.at(i).u4frmNum,
                                 m_vecCfgSetting.at(i).u4cfgNum,
                                 m_vecCfgSetting.at(i).vecROIs.size());
                }
                CAM_LOGD_IF( m_bDebugEnable, "dump cfgSetting -");
            }

            //
            if( res & mode==2)
            {
                if( m_vecCfgSetting.size()!=0)
                {
                    // Find matched setting by configuration number.
                    MBOOL idx_vd = MFALSE;

                    MUINT32 idx = m_vecCfgSetting.size()-1;
                    for( MUINT32 i=0; i<m_vecCfgSetting.size(); i++, idx--)
                    {
                        if( m_vecCfgSetting.at(idx).u4cfgNum==flag)
                        {
                            idx_vd = MTRUE;
                            break;
                        }
                    }

                    if( /* match */
                        ( idx_vd                       ) &&
                        ( 0<=idx                       ) &&
                        (    idx<m_vecCfgSetting.size()))
                    {
                        vector<PDBUF_CFG_T>::iterator itr = m_vecCfgSetting.begin() + idx;

                        CAM_LOGD_IF( m_bDebugEnable,
                                     "mode(0x%x) flag(0x%x) cfgNum(0x%x) szROI(%d) idx(%d) idx_vd(%d)",
                                     mode,
                                     flag,
                                     itr->u4cfgNum,
                                     itr->vecROIs.size(),
                                     idx,
                                     idx_vd);

                        for( MUINT32 i=0; i<itr->vecROIs.size(); i++)
                        {
                            iPDInputData.curLensPos   = m_curLensPos;
                            iPDInputData.XSizeOfImage = m_profile.uImgXsz;
                            iPDInputData.YSizeOfImage = m_profile.uImgYsz;
                            iPDInputData.ROI.i4XStart = itr->vecROIs.at(i).i4XStart;
                            iPDInputData.ROI.i4YStart = itr->vecROIs.at(i).i4YStart;
                            iPDInputData.ROI.i4XEnd   = itr->vecROIs.at(i).i4XEnd;
                            iPDInputData.ROI.i4YEnd   = itr->vecROIs.at(i).i4YEnd;
                            iPDInputData.ROI.i4Info   = itr->vecROIs.at(i).i4Info;
                            iPDInputData.afegain      = m_afeGain;
                            iPDInputData.mode         = mode;

                            m_pPDBufMgrOpen->GetDefocus( iPDInputData, oPdOutputData);

                            PDRes.ROIRes[i].Defocus                = Boundary(0, oPdOutputData.Defocus, 1023);
                            PDRes.ROIRes[i].DefocusConfidence      = oPdOutputData.DefocusConfidence;
                            PDRes.ROIRes[i].DefocusConfidenceLevel = oPdOutputData.DefocusConfidenceLevel;
                            PDRes.ROIRes[i].PhaseDifference        = oPdOutputData.PhaseDifference;

                            CAM_LOGD_IF( m_bDebugEnable,
                                         "win %d [Confidence] %d [LensPos] %d->%ld [value] %ld",
                                         i,
                                         PDRes.ROIRes[i].DefocusConfidence,
                                         m_curLensPos,
                                         PDRes.ROIRes[i].Defocus,
                                         PDRes.ROIRes[i].PhaseDifference);
                        }

                        MUINT8 removeNum = idx+1;
                        for( MUINT32 i=0; i<removeNum; i++)
                        {
                            m_vecCfgSetting.at(i).vecROIs.clear();
                        }
                        m_vecCfgSetting.erase(m_vecCfgSetting.begin(),m_vecCfgSetting.begin()+removeNum);
                    }
                }
            }
            else
            {
                if( /* checking ROI numbers */
                    ( 0<m_numROI                    ) &&
                    (   m_numROI<=MAX_SIZE_OF_PD_ROI) )
                {
                    for( MUINT32 i = 0; i < m_numROI; i++)
                    {
                        iPDInputData.curLensPos   = m_curLensPos;
                        iPDInputData.XSizeOfImage = m_profile.uImgXsz;
                        iPDInputData.YSizeOfImage = m_profile.uImgYsz;
                        iPDInputData.ROI.i4XStart = m_ROI[i].i4X;
                        iPDInputData.ROI.i4YStart = m_ROI[i].i4Y;
                        iPDInputData.ROI.i4XEnd   = m_ROI[i].i4X + m_ROI[i].i4W;
                        iPDInputData.ROI.i4YEnd   = m_ROI[i].i4Y + m_ROI[i].i4H;
                        iPDInputData.ROI.i4Info   = m_ROI[i].i4Info;
                        iPDInputData.afegain      = m_afeGain;
                        iPDInputData.mode         = 0x0;

                        m_pPDBufMgrOpen->GetDefocus( iPDInputData, oPdOutputData);

                        PDRes.ROIRes[i].Defocus                = Boundary(0, oPdOutputData.Defocus, 1023);
                        PDRes.ROIRes[i].DefocusConfidence      = oPdOutputData.DefocusConfidence;
                        PDRes.ROIRes[i].DefocusConfidenceLevel = oPdOutputData.DefocusConfidenceLevel;
                        PDRes.ROIRes[i].PhaseDifference        = oPdOutputData.PhaseDifference;

                        CAM_LOGD_IF( m_bDebugEnable,
                                     "win %d [Confidence] %d [LensPos] %d->%ld [value] %ld\n",
                                     i,
                                     PDRes.ROIRes[i].DefocusConfidence,
                                     m_curLensPos,
                                     PDRes.ROIRes[i].Defocus,
                                     PDRes.ROIRes[i].PhaseDifference);
                    }
                }

                // No use in this mode. clean directly.
                for( MUINT32 i=0; i<m_vecCfgSetting.size(); i++)
                {
                    m_vecCfgSetting.at(i).vecROIs.clear();
                }
                m_vecCfgSetting.clear();
            }
        }
        AAA_TRACE_END_D;
    }


    {
        Mutex::Autolock lock( m_Lock_Result);
        m_vPDOutput.push_back( PDRes);
        if( 1<m_vPDOutput.size())
        {
            //pop_front and keep only last 2 result
            m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
            CAM_LOGD("pop_front, sz=%zu", m_vPDOutput.size());
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "[%s]-", __FUNCTION__);
    return S_3A_OK;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                                                                     Interface
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT PDMgr::GetVersionOfPdafLibrary(SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret=E_3A_ERR;

    if( m_bEnPDBufThd)
    {
        if (m_profile.BufType & MSK_CATEGORY_OPEN)
        {
            //set calibration to PD open core.
            ret = m_pPDBufMgrOpen->GetVersionOfPdafLibrary( tOutSWVer);
        }
        else
        {
            tOutSWVer.MajorVersion = 1;
            tOutSWVer.MinorVersion = 1;
            ret = S_3A_OK;
        }

        CAM_LOGD("%s = %d.%d", __FUNCTION__, (int)tOutSWVer.MajorVersion, (int)tOutSWVer.MinorVersion);
    }
    return ret;
}

MRESULT PDMgr::setPDCaliData( MVOID *ptrInTuningData, MINT32 &i4OutInfoSz, MINT32 **ptrOutInfo)
{
    //This function is used to set PD calibration data and output PD related data for Hybrid AF.
    MRESULT ret=E_3A_ERR;

    //==========
    // (1) Read PDAF calibration data from NVRAM.
    //==========
    NvBufUtil::getInstance().getBufAndReadNoDefault( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev, (void*&)m_ptrAfPara);

    //==========
    // (2) If there is no PDAF calibration data in NVRAM, reading calibration in EEPROM by using cam_cal driver.
    //==========
    if( m_ptrAfPara->rPdCaliData.i4Size==0)
    {
        MINT32 i4SensorDevID;
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

        CAM_CAL_DATA_STRUCT CamCalData;
        MUINT32 result = CamCalDrvBase::createInstance()->GetCamCalCalData(i4SensorDevID, CAMERA_CAM_CAL_DATA_PDAF, (void *)&CamCalData);

        CAM_LOGD( "%s NO PDAF calib data in NVRam drv. Read data from EEPROM by using cam_cal driver : result(0x%8x), DevID(%d)",
                  __FUNCTION__,
                  result,
                  i4SensorDevID);


        if( result==CAM_CAL_ERR_NO_ERR)
        {
            /**********************************************************************************
             *
             * After PDAF calibration is successfully read from EEPROM,
             * storing PDAF calibration to NVRAM in order to speed up initial flow next time.
             *
             **********************************************************************************/
            memcpy( m_ptrAfPara->rPdCaliData.uData, CamCalData.PDAF.Data, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
            m_ptrAfPara->rPdCaliData.i4Size = CamCalData.PDAF.Size_of_PDAF;

            // Storing PDAF calibration data into NVRAM.
            NvBufUtil::getInstance().write( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev);
        }
        else
        {
            CAM_LOGE("FAILED to get PD calibration data from cam_cal driver!! Please contact cam_cal driver porting owner!!");
        }
    }
    else
    {
        CAM_LOGD( "%s There is PDAF calibration data in NVRAM.", __FUNCTION__);
    }


    CAM_LOGD( "%s Size of PD calibration data = %d, first 4 dword: %x %x %x %x",
              __FUNCTION__,
              m_ptrAfPara->rPdCaliData.i4Size,
              m_ptrAfPara->rPdCaliData.uData[0],
              m_ptrAfPara->rPdCaliData.uData[1],
              m_ptrAfPara->rPdCaliData.uData[2],
              m_ptrAfPara->rPdCaliData.uData[3]);

    CAM_LOGD_IF(m_bDebugEnable, "pd calibration data:");
    for( int index = 0; index < m_ptrAfPara->rPdCaliData.i4Size;)
    {
        CAM_LOGD_IF( m_bDebugEnable,
                     "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
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

    //==========
    // (3) Set PDAF calibration data.
    //==========
    if( /* After related instance and PD thread is created, load calibration data*/
        (m_profile.BufType!=EPDBUF_NOTDEF) &&
        (m_bEnPDBufThd) &&
        ((m_pPDBufMgrOpen) || (m_pPDBufMgrCore && m_pIPdAlgo)))
    {

        if( m_profile.BufType & MSK_CATEGORY_OPEN)
            ret = SetCaliData2PDOpenCore( ptrInTuningData, reinterpret_cast<MVOID *>(&m_ptrAfPara->rPdCaliData), 10, m_i4PDInfo2HybirdAF);
        else
            ret = SetCaliData2PDCore( ptrInTuningData, reinterpret_cast<MVOID *>(&m_ptrAfPara->rPdCaliData), 10, m_i4PDInfo2HybirdAF);
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


    //==========
    // (4) Data output
    //==========
    if( ret==E_3A_ERR)
    {
        i4OutInfoSz   = 0;
        (*ptrOutInfo) = NULL;
    }
    else
    {
        //This m_i4PDInfo2HybirdAF arrary size is defined by core pd algorithm
        i4OutInfoSz   = 10;
        (*ptrOutInfo) = m_i4PDInfo2HybirdAF;
    }


    if( property_get_int32("vendor.debug.dump_pdaf_cali.enable", 0)==1)
    {
        FILE *fp;
        fp = fopen("/sdcard/pdaf_calibration_data.bin","wb");
        fwrite((void *)m_ptrAfPara->rPdCaliData.uData, sizeof(char), m_ptrAfPara->rPdCaliData.i4Size, fp);
        fclose(fp);
    }

    return ret;
}

//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::UpdatePDParam( MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AREA_T *tInputPDArea)
{
    MRESULT ret = E_3A_ERR;

    if( m_profile.BufType==EPDBUF_VC_OPEN)
    {
        if( 0<i4InputPDAreaNum && i4InputPDAreaNum<=9 && tInputPDArea!=NULL)
        {
            PDBUF_CFG_T cfg;

            cfg.u4frmNum = u4FrmNum;

            for( MUINT32 i=0; i<i4InputPDAreaNum; i++)
            {
                SPDROI_T roi;
                roi.i4XStart = tInputPDArea[i].i4X;
                roi.i4YStart = tInputPDArea[i].i4Y;
                roi.i4XEnd   = tInputPDArea[i].i4X + tInputPDArea[i].i4W;
                roi.i4YEnd   = tInputPDArea[i].i4Y + tInputPDArea[i].i4H;
                roi.i4Info   = tInputPDArea[i].i4Info;

                cfg.vecROIs.push_back( roi);
            }


            {
                Mutex::Autolock lock(m_Lock_PDWinCfg);

                if( m_vecCfgSetting.size()!=0)
                {
                    vector<PDBUF_CFG_T>::iterator itr = m_vecCfgSetting.end()-1;

                    if( i4InputPDAreaNum!=itr->vecROIs.size())
                    {
                        cfg.u4cfgNum = (1<<i4InputPDAreaNum)-1;
                    }
                    else
                    {
                        MUINT32 tmp = (itr->u4cfgNum & 0xFF) << 1;
                        cfg.u4cfgNum = (tmp & 0xFE) | ((tmp & 0x100)>>8);
                    }
                }
                else
                {
                    cfg.u4cfgNum = (1<<i4InputPDAreaNum)-1;
                }

                // Maximum store last 10 configure setting history.
                if( m_vecCfgSetting.size()>10)
                {
                    MUINT8 removeNum = m_vecCfgSetting.size()-10;
                    for( MUINT32 i=0; i<removeNum; i++)
                    {
                        m_vecCfgSetting.at(i).vecROIs.clear();
                    }
                    m_vecCfgSetting.erase(m_vecCfgSetting.begin(),m_vecCfgSetting.begin()+removeNum);
                }

                //
                m_vecCfgSetting.push_back(cfg);

                //
                ConfigurePDHWSetting();

                // free local memory
                cfg.vecROIs.clear();
            }
        }
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::postToPDTask(SPDInputData_t *ptrInputData)
{
    MRESULT ret=E_3A_ERR;

    if( m_bEnPDBufThd)
    {
        if( ptrInputData)
        {
            //Run thread if PD task is not busy.
            int Val;
            ::sem_getvalue(&m_semPDBufThdEnd, &Val);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] sem_post m_semPDBuf, m_semPDBufThdEnd before wait = %d", __FUNCTION__, Val);

            //checking PD thread is busy or not.
            if (Val == 1)
            {
                m_MagicNumber = ptrInputData->magicNumber;
                m_frmCnt = ptrInputData->frmNum;

                //(1) do pdconvert if need to.
                m_lrbuf = NULL;
                if( m_profile.BufType==EPDBUF_VC || m_profile.BufType==EPDBUF_PDO )
                {
                    m_lrbuf = PDConvert(ptrInputData->databuf_virtAddr, ptrInputData->databuf_size, ptrInputData->databuf_stride);
                }
                else if( m_profile.BufType & MSK_CATEGORY_OPEN)
                {
                    m_databuf_size   = ptrInputData->databuf_size;
                    m_databuf_stride = ptrInputData->databuf_stride;
                    if( !m_databuf)
                        m_databuf = new MUINT8 [m_databuf_size];

                    memcpy( m_databuf, ptrInputData->databuf_virtAddr, sizeof(MUINT8)*m_databuf_size);

                    if(m_i4DbgVcDump)
                    {
                        char fileName[64];
                        sprintf(fileName, "/sdcard/vc/%d_dma_in.raw", m_frmCnt);
                        FILE *fp = fopen(fileName, "w");
                        if (fp)
                        {
                            fwrite(reinterpret_cast<void *>(m_databuf), 1, m_databuf_size, fp);
                            fclose(fp);
                        }
                    }
                }
                else if( m_profile.BufType == EPDBUF_RAW_LEGACY)
                {
                    if (m_i4DbgPdDump)
                    {
                        char fileName[64];
                        sprintf(fileName, "/sdcard/pdo/pd_raw_%d_%d.raw", m_MagicNumber, m_frmCnt);
                        FILE *fp = fopen(fileName, "w");
                        if (NULL == fp)
                        {
                            MINT32 err = mkdir("/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);
                            CAM_LOGD("create folder /sdcard/pdo (%d)", err);
                        }
                        else
                        {
                            fwrite(reinterpret_cast<void *>(ptrInputData->databuf_virtAddr), 1, ptrInputData->databuf_size, fp);
                            fclose(fp);
                        }
                    }
                }

                //(2) get PD ROI from host.
                MUINT32 max_num_roi = MAX_SIZE_OF_PD_ROI<AF_PSUBWIN_NUM ? MAX_SIZE_OF_PD_ROI : AF_PSUBWIN_NUM;
                m_numROI = ptrInputData->numROI<max_num_roi ? ptrInputData->numROI : max_num_roi;
                for( MUINT32 i=0; i<m_numROI; i++)
                {
                    m_ROI[i].i4X    = ptrInputData->ROI[i].i4XStart;
                    m_ROI[i].i4Y    = ptrInputData->ROI[i].i4YStart;
                    m_ROI[i].i4W    = ptrInputData->ROI[i].i4XEnd - ptrInputData->ROI[i].i4XStart;
                    m_ROI[i].i4H    = ptrInputData->ROI[i].i4YEnd - ptrInputData->ROI[i].i4YStart;
                    m_ROI[i].i4Info = ptrInputData->ROI[i].i4Info;
                }

                //(3) img size should be same as profile information
                if( m_profile.uImgXsz!=ptrInputData->XSizeOfImage || m_profile.uImgYsz!=ptrInputData->YSizeOfImage)
                {
                    CAM_LOGD( "[PDAF] profile (%d, %d), coordinate (%d, %d), lensPos (%d)",
                              m_profile.uImgXsz,
                              m_profile.uImgYsz,
                              ptrInputData->XSizeOfImage,
                              ptrInputData->YSizeOfImage,
                              ptrInputData->curLensPos);

                    m_profile.uImgXsz = ptrInputData->XSizeOfImage;
                    m_profile.uImgYsz = ptrInputData->YSizeOfImage;
                }

                //(4) get current lens' position  and AfeGain
                m_curLensPos = ptrInputData->curLensPos;
                m_afeGain = ptrInputData->afeGain;

                //(5) do pdextract & post to thread
                if( m_profile.BufType & MSK_CATEGORY_OPEN)
                {
                    //post to thread
                    ::sem_wait( &m_semPDBufThdEnd); //to be 0, it won't block, 0 means PD task not ready yet
                    ::sem_post( &m_semPDBuf);
                }
                else //(!(m_profile.BufType & MSK_CATEGORY_OPEN))
                {
                    if (m_curLensPos)
                    {
                        // without m_curLensPos,
                        // don't do pdextract and don't post to thread
                        // to ensure pdextract is paired with pdconvert

                        PD_EXTRACT_INPUT_T a_sPDInput;

                        //(1) select data buffer type
                        if( m_profile.BufType==EPDBUF_VC || m_profile.BufType==EPDBUF_PDO)
                        {
                            a_sPDInput.pRawBuf = NULL;
                            a_sPDInput.pPDBuf = reinterpret_cast<MVOID *>(m_lrbuf);
                        }
                        else if (m_profile.BufType & MSK_CATEGORY_DUALPD)
                        {
                            a_sPDInput.pRawBuf = NULL;
                            a_sPDInput.pPDBuf = reinterpret_cast<MVOID *>(ptrInputData->databuf_virtAddr);

                            if(m_i4DbgVcDump)
                            {
                                char fileName[64];
                                sprintf( fileName, "/sdcard/vc/%d_dma_in.raw", m_frmCnt);
                                FILE *fp = fopen( fileName, "w");
                                if( NULL==fp)
                                {
                                    CAM_LOGD( "Can not open file %s", fileName);
                                }
                                else
                                {
                                    SensorVCInfo rSensorVCInfo;
                                    memset( &rSensorVCInfo, 0, sizeof(SensorVCInfo));

                                    if( m_pIHalSensor)
                                    {
                                        MINT32 sensorMode = m_profile.i4SensorMode;
                                        m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&sensorMode, 0);
                                    }
                                    else
                                    {
                                        CAM_LOGE("m_pIHalSensor is NULL!");
                                    }

                                    fwrite(reinterpret_cast<void *>(ptrInputData->databuf_virtAddr), 1, (rSensorVCInfo.VC2_SIZEH)*(rSensorVCInfo.VC2_SIZEV), fp);
                                    fclose(fp);
                                }
                            }
                        }
                        else //EPDBUF_RAW_LEGACY
                        {
                            a_sPDInput.pRawBuf = reinterpret_cast<MVOID *>(ptrInputData->databuf_virtAddr);
                            a_sPDInput.pPDBuf  = NULL;
                        }

                        AAA_TRACE_D("pdextract");

                        for( MUINT32 i=0; i<m_numROI; i++)
                        {
                            a_sPDInput.sFocusWin.i4W = m_ROI[i].i4W;
                            a_sPDInput.sFocusWin.i4H = m_ROI[i].i4H;
                            a_sPDInput.sFocusWin.i4X = m_ROI[i].i4X;
                            a_sPDInput.sFocusWin.i4Y = m_ROI[i].i4Y;

                            CAM_LOGD_IF( m_bDebugEnable,
                                         "[pdextract] win %d [WinPos] %d, %d, %d, %d\n",
                                         i,
                                         a_sPDInput.sFocusWin.i4X,
                                         a_sPDInput.sFocusWin.i4Y,
                                         a_sPDInput.sFocusWin.i4W,
                                         a_sPDInput.sFocusWin.i4H);

                            ret = m_pIPdAlgo->extractPD( a_sPDInput, m_sPDExtractOutput[i]);
                            if( ret==E_3A_ERR)
                            {
                                CAM_LOGE( "extractPD fail");
                                memset( &m_sPDExtractOutput[i], 0, sizeof(PD_EXTRACT_DATA_T));
                            }
                        }

                        AAA_TRACE_END_D;

                        //post to thread
                        ::sem_wait( &m_semPDBufThdEnd); //to be 0, it won't block, 0 means PD task not ready yet
                        ::sem_post( &m_semPDBuf);
                    }
                }
            }
            else
            {
                CAM_LOGD( "[%s] Can not post to PD thread because thread is busy.", __FUNCTION__);

            }
            ret=S_3A_OK;

        }
        else
        {
            CAM_LOGD_IF(m_bDebugEnable, "[%s] ERR : input is NULL, close thread", __FUNCTION__);
            ret=E_3A_ERR;
        }


        if( ret==E_3A_ERR)
        {
            closeThread();
        }
    }
    else
    {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] Can not post to pd thread because of thread is closed", __FUNCTION__);
        ret=E_3A_ERR;
    }


    return ret;
}


MRESULT PDMgr::getPDTaskResult( SPDOutputData_t *ptrOutputRes)
{
    MRESULT ret=E_3A_ERR;

    if( m_bEnPDBufThd && ptrOutputRes->numRes!=0)
    {
        Mutex::Autolock lock(m_Lock_Result);
        if( m_vPDOutput.size())
        {
            //output
            ptrOutputRes->numRes = ptrOutputRes->numRes>m_numROI ? m_numROI : ptrOutputRes->numRes;
            memcpy( ptrOutputRes->Res, m_vPDOutput.begin()->ROIRes, sizeof(SPDROIResult_T)*(ptrOutputRes->numRes));

            for( MUINT32 i=0; i<ptrOutputRes->numRes; i++)
            {

                CAM_LOGD_IF( m_bDebugEnable,
                             "[getPDRes][%d] ToPos %4ld, C %3d, CL %3lu, PD %5ld\n",
                             i,
                             ptrOutputRes->Res[i].Defocus,
                             ptrOutputRes->Res[i].DefocusConfidence,
                             ptrOutputRes->Res[i].DefocusConfidenceLevel,
                             ptrOutputRes->Res[i].PhaseDifference);
            }

            //pop_front
            m_vPDOutput.erase( m_vPDOutput.begin());
            ret = S_3A_OK;
        }
        else
        {
            ret = E_AF_BUSY;
        }
    }

    return ret;

}


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

