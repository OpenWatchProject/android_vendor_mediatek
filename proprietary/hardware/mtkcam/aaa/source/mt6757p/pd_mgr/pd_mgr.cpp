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

#include <cutils/properties.h>
#include <utils/threads.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include "aaa_sensor_buf_mgr.h"

#include "pd_mgr.h"
#include <pd_buf_mgr_open.h>
#include <pd_buf_mgr.h>
#include <af_feature.h>
#include <Local.h>

#include <isp_tuning.h>
#include <kd_camera_feature.h>
#include "kd_imgsensor.h"
#include <pd_algo_if.h>
#include <private/IopipeUtils.h>
#include <nvbuf_util.h>
#include <kd_imgsensor_define.h>

#include <isp_mgr_pdo.h>
#include <mtkcam/aaa/IIspMgr.h>


//------------Thread-------------
#include <mtkcam/def/PriorityDefs.h>
#include <sys/prctl.h>
#include <mtkcam/utils/std/common.h>
//-------------------------------

#include <sys/types.h>
#include <sys/stat.h>

using namespace NSCam::Utils;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSIspTuningv3;

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                Multi-instance.
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
//                                init function.
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
    MBOOL ret = MFALSE;
    CAM_LOGD("%s-", __FUNCTION__);
    return ret;
}
//----------------------------------------------------------------------------------------------------
MBOOL PDMgr::CamPwrOffState()
{
    CAM_LOGD("%s+", __FUNCTION__);
    MBOOL ret = uninitMemBuf( m_rMemBufInfo);
    CAM_LOGD("%s-", __FUNCTION__);
    return ret;
}
//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::configPdoHW()
{
    if(m_pPDBufMgrCore && m_pIPdAlgo)
    {
        //config PDO HW.
        MBOOL bEnablePDC;
        if (m_profile.BufType==EPDBuf_PDO)
            bEnablePDC = MTRUE;
        else if (m_profile.BufType==EPDBuf_DualPD_Raw)
            bEnablePDC = MFALSE;
        else
            return MFALSE;

        CAM_LOGD_IF(m_bDebugEnable, "[%s] PDOHWInfo %d %d 0x%x 0x%x 0x%x %d %d %d %d %d %d %d %d %d",
                    __FUNCTION__,
                    m_sPDOHWInfo.u4Bpci_xsz,
                    m_sPDOHWInfo.u4Bpci_ysz,
                    m_sPDOHWInfo.pu1Bpci_tbl,
                    m_sPDOHWInfo.phyAddrBpci_tbl,
                    m_sPDOHWInfo.virAddrBpci_tbl,
                    m_sPDOHWInfo.i4memID,
                    m_sPDOHWInfo.u4Pdo_xsz,
                    m_sPDOHWInfo.u4Pdo_ysz,
                    m_sPDOHWInfo.i4TGSzW,
                    m_sPDOHWInfo.u4BitDepth,
                    m_sPDOHWInfo.u1IsDualPD,
                    m_sPDOHWInfo.u1PBinType,
                    m_sPDOHWInfo.u1PBinStartLine,
                    m_sPDOHWInfo.u1PdSeparateMode);
        //configure PDO tuning mgr parameters
        ISP_MGR_PDO_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).config( m_sPDOHWInfo);
        //enable PDC
        if ( auto pIspMgr = IIspMgr::getInstance() )
        {
            pIspMgr->setPDCEnable(m_i4SensorIdx, bEnablePDC);
        }
        return MTRUE;
    }
    return MFALSE;
}
//----------------------------------------------------------------------------------------------------
EPDBuf_Type_t PDMgr::start(MINT32 i4SensorIdx, SPDProfile_t *iPDProfile, MUINT32 uPdPipeCtrl)
{
    CAM_LOGD("%s+", __FUNCTION__);

    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    m_pIHalSensor = pIHalSensorList->createSensor( "pd_mgr", m_i4CurrSensorDev);

    m_bDebugEnable =            property_get_int32("vendor.debug.af_mgr.enable", 0);
    m_i4DgbLogW =               property_get_int32("vendor.debug.af_logw.enable", 0);
    m_bPDVCTest =               property_get_int32("vendor.debug.pd_vc.enable", 0);
    m_i4DbgPdDump =             property_get_int32("vendor.pd.dump.enable", 0);
    m_i4DbgDisPdHandle =        property_get_int32("vendor.pdhandle.disable", 0);
    m_i4DbgVcDump =             property_get_int32("vendor.vc.dump.enable", 0);

    m_frmCnt = 0;

    //initial member.
    m_Status              = EPD_Not_Enabled;
    m_numROI              = 0;
    m_databuf_size        = 0;
    m_databuf_stride      = 0;
    m_databuf             = NULL;
    m_pPDBufMgrCore       = NULL;
    m_pPDBufMgrOpen       = NULL;
    m_pIPdAlgo            = NULL;
    m_i4SensorIdx         = i4SensorIdx;

    memset( m_sPDInput,          0, sizeof(PD_INPUT_T)*AF_PSUBWIN_NUM);
    memset( m_sPDOutput,         0, sizeof(PD_OUTPUT_T)*AF_PSUBWIN_NUM);
    memset( m_i4PDInfo2HybirdAF, 0, sizeof(MINT32)*10);
    memset( m_ROI,               0, sizeof(AREA_T)*AF_PSUBWIN_NUM);

    m_vPDOutput.clear();

    //get main information from host
    memcpy( &m_profile, iPDProfile, sizeof(m_profile));

    m_i4SensorMode = m_profile.i4SensorMode;
    m_AETargetMode = m_profile.AETargetMode;

    //get pd buffer type from custom setting.
    MUINT32 pdBufType = GetPDBuf_Type( m_i4CurrSensorDev, m_profile.i4CurrSensorId);

    //check sensor output pd data mode and PD buffer manager setting.
    m_pPDBufMgrCore = NULL;
    m_pPDBufMgrOpen = NULL;
    m_pIPdAlgo      = NULL;

    m_profile.BufType = EPDBuf_NotDef;
    m_sPDOHWInfo.u1IsDualPD = 0;
    switch (m_profile.uSensorType)
    {
    case SensorType_Pure_Raw:
        pdBufType &= ~(EPDBuf_Type_PDO_MSK | EPDBuf_Type_VC_MSK);
        if (pdBufType == EPDBuf_Raw || pdBufType == EPDBuf_Raw_Open)
            m_profile.BufType = (EPDBuf_Type_t)pdBufType;
        break;

    case SensorType_PDAF_Raw:
        pdBufType &= ~(EPDBuf_Type_Raw_MSK | EPDBuf_Type_VC_MSK);
        if (pdBufType == EPDBuf_PDO || pdBufType == EPDBuf_PDO_Open)
        {
            if (pdBufType == EPDBuf_PDO)
            {
                if (iPDProfile->u4IsFrontalBinning == 1)
            {
                CAM_LOGD( "Frontal binning should be disabled.");
                break;
            }
                else if (uPdPipeCtrl != FEATURE_PDAF_SUPPORT_BNR_PDO)
                {
                    CAM_LOGE( "support PDO but pipe(stt) didn't initialized.");
                    break;
                }
            }
            m_profile.BufType = (EPDBuf_Type_t)pdBufType;
        }
        break;

    case SensorType_PDAF_VC_HDR:
        pdBufType &= ~(EPDBuf_Type_Raw_MSK | EPDBuf_Type_PDO_MSK);
        if (pdBufType == EPDBuf_VC || pdBufType == EPDBuf_VC_Open)
        {
            if (uPdPipeCtrl != FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
            {
                CAM_LOGE( "support PDVC but pipe(camsv) didn't initialized.");
                break;
            }
            m_profile.BufType = (EPDBuf_Type_t)pdBufType;
        }
        break;

    case SensorType_DualPD_Raw:
        pdBufType &= ~EPDBuf_Type_VC_MSK;
        m_sPDOHWInfo.u1IsDualPD = 1;
        if (pdBufType == EPDBuf_DualPD_Raw)
        {
            if (uPdPipeCtrl != FEATURE_PDAF_SUPPORT_PBN_PDO)
            {
                CAM_LOGE( "support DualPD_Raw but pipe(stt) didn't initialized.");
                break;
            }
            if (m_profile.bEnablePBIN)
                m_profile.BufType = (EPDBuf_Type_t)pdBufType;
            else
            {
                CAM_LOGE("PBIN from 3A (%d) is not match with current PD sensor type (%d)", m_profile.bEnablePBIN, m_profile.uSensorType);
            }
        }
        break;

    case SensorType_DualPD_VC:
        pdBufType &= ~EPDBuf_Type_Raw_MSK;
        m_sPDOHWInfo.u1IsDualPD = 1;
        if (pdBufType == EPDBuf_DualPD_VC)
        {
            if (uPdPipeCtrl != FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
            {
                CAM_LOGE( "support DualPD_VC but pipe(camsv) didn't initialized.");
                break;
            }
            m_profile.BufType = (EPDBuf_Type_t)pdBufType;
        }
        break;

    default:
        CAM_LOGE("Unknown PD sensor type (%d)", m_profile.uSensorType);
        break;
    }

    if (m_profile.BufType == EPDBuf_NotDef);
    else if (m_profile.BufType & EPDBuf_Type_Open_MSK)
        m_pPDBufMgrOpen = ::PDBufMgrOpen::createInstance( m_profile);
    else if (m_profile.BufType & EPDBuf_Type_DualPD_MSK)
    {
        m_pPDBufMgrCore = ::PDBufMgr::createInstance( m_profile);
        m_pIPdAlgo      = NS3A::IPdAlgo::createInstance( m_i4CurrSensorDev, PDType_DualPD);
    }
    else
    {
        m_pPDBufMgrCore = ::PDBufMgr::createInstance( m_profile);
        m_pIPdAlgo      = NS3A::IPdAlgo::createInstance( m_i4CurrSensorDev, PDType_Legacy);
    }

    CAM_LOGD( "SensorID 0x%x, SensorType %d, BufType 0x%x(0x1:VC, 0x2:Raw, 0x4:PDO, 0x10:open, 0x20:dualPD)",
              m_profile.i4CurrSensorId,
              m_profile.uSensorType,
              m_profile.BufType);

    //engineer check only.
    if( m_bPDVCTest)
    {
        m_pPDBufMgrCore = NULL;
        m_pPDBufMgrOpen = NULL;
        m_pIPdAlgo      = NULL;
        CAM_LOGD( "Debug virtual channel only : for engineer testing only!!");
    }

    //check PD mgr initial status to deside thread should be created or not.
    if (m_pPDBufMgrOpen)
    {
        //start thread.
        createThread();

        // query sensor driver setting for 3rd party PD library.
        MUINT32  arg1=0;
        MUINT16 *arg2=NULL;
        if( m_pPDBufMgrOpen->sendCommand( 0, (MVOID *)(&arg1), (MVOID *)(&arg2)))
        {
            if( m_pIHalSensor)
            {
                m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING, (MUINTPTR)&arg1, (MUINTPTR)&arg2, 0);
            }
        }

    }
    else if (m_pPDBufMgrCore && m_pIPdAlgo)
    {
        //start thread.
        createThread();
    }
    else
    {
        m_profile.BufType = EPDBuf_NotDef;
        CAM_LOGD( "PD buffer type is not defined %x %x %x", m_pPDBufMgrCore, m_pIPdAlgo, m_pPDBufMgrOpen);
    }

    //config PDO HW
    //need to config PDO HW once when uPdPipeCtrl = 1 or system crash
    //no matter if m_pPDBufMgrCore or m_pIPdAlgo is NULL
    if (uPdPipeCtrl == FEATURE_PDAF_SUPPORT_PBN_PDO || uPdPipeCtrl == FEATURE_PDAF_SUPPORT_BNR_PDO)
    {
        if(m_profile.uSensorType == SensorType_PDAF_Raw)
        {
            //Get BPCI table information for PDO
            if (ISP_MGR_BNR2::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getBPCItable(
                    m_sPDOHWInfo.u4Bpci_xsz,
                    m_sPDOHWInfo.u4Bpci_ysz,
                    m_sPDOHWInfo.u4Pdo_xsz,
                    m_sPDOHWInfo.u4Pdo_ysz,
                    m_sPDOHWInfo.phyAddrBpci_tbl,
                    m_sPDOHWInfo.virAddrBpci_tbl) == MFALSE)
            {
                CAM_LOGE("Failed acquiring BPCI table!");
                m_profile.BufType = EPDBuf_NotDef;
            }
            else
            {
                if (m_pPDBufMgrCore != NULL)
                    m_pPDBufMgrCore->SetBpciInfo(m_sPDOHWInfo.u4Pdo_xsz, m_sPDOHWInfo.u4Pdo_ysz);
                m_sPDOHWInfo.i4TGSzW         = m_profile.uImgXsz;

                CAM_LOGD( "%s BPCI Pdo_Xsz(0x%x) Pdo_Ysz(0x%x) Bpci_Xsz(0x%x) Bpci_Ysz(0x%x) BpciVir(0x%x)",
                          __FUNCTION__,
                          m_sPDOHWInfo.u4Pdo_xsz,
                        m_sPDOHWInfo.u4Pdo_ysz,
                          m_sPDOHWInfo.u4Bpci_xsz,
                        m_sPDOHWInfo.u4Bpci_ysz,
                        m_sPDOHWInfo.virAddrBpci_tbl);
                CAM_LOGD( "bpci first 4 dword: %x %x %x %x",
                        ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[0],
                        ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[1],
                        ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[2],
                        ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[3]);
            }
        }
        else if (m_profile.uSensorType == SensorType_DualPD_Raw)
        {
            m_sPDOHWInfo.u1PdSeparateMode = m_profile.uPdSeparateMode;
            if (m_AETargetMode == AE_MODE_IVHDR_TARGET)
                m_sPDOHWInfo.u1PBinType = 0; // 1x4
            else
                m_sPDOHWInfo.u1PBinType = 1; // 4x4
        }
    }

    //output
    iPDProfile->BufType = m_profile.BufType;


    CAM_LOGD("%s-", __FUNCTION__);
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

    //disable PDC
    if ( auto pIspMgr = IIspMgr::getInstance() )
    {
        pIspMgr->setPDCEnable(m_i4SensorIdx, MFALSE);
    }

    //uninit member
    m_databuf_size   = 0;
    m_databuf_stride = 0;

    if( m_databuf)
        delete m_databuf;
    m_databuf=NULL;

    m_pPDBufMgrCore = NULL;
    m_pPDBufMgrOpen = NULL;

    m_pIPdAlgo = NULL;

    m_vPDOutput.clear();

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                               MEM buffer for PDO
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL PDMgr::initMemBuf( IMEM_BUF_INFO &rBufInfo, MUINT32 const u4Size)
{
    MBOOL ret = MFALSE;

    MUINT32 i = 0;

    ret = MTRUE;
    if( !m_pIMemDrv)
    {
        m_pIMemDrv = IMem3A::createInstance();

        if( !m_pIMemDrv)
        {
            CAM_LOGD( "m_pIMemDrv create Fail.");
            ret = MFALSE;
        }
        else
        {
            ret = m_pIMemDrv->init();
            if (ret == MTRUE)
            {
                if( !allocMemBuf( rBufInfo, u4Size)) //DEFAULT_TABLE_SIZE
                {
                    CAM_LOGE( "allocMemBuf sensor(0x%02x)", m_i4CurrSensorDev);
                }
                else
                {
                    CAM_LOGD( "sensor(0x%02x), memID(%d), virtAddr(0x%x), phyAddr(0x%x), size(%d)",
                              m_i4CurrSensorDev,
                              m_rMemBufInfo.memID,
                              m_rMemBufInfo.virtAddr,
                              m_rMemBufInfo.phyAddr,
                              m_rMemBufInfo.size);
                }
            }
            else
            {
                CAM_LOGE( "m_pIMemDrv init Fail!");
            }
        }
    }
    else
    {
        CAM_LOGD( "m_pIMemDrv(0x%x) already exists.", m_pIMemDrv);
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------
MBOOL PDMgr::uninitMemBuf( IMEM_BUF_INFO &rBufInfo)
{
    MUINT32 ret = 0;
    MUINT32 i = 0;

    freeMemBuf( rBufInfo);

    if( m_pIMemDrv)
    {
        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    return MTRUE;
}
//----------------------------------------------------------------------------------------------------
MBOOL PDMgr::allocMemBuf( IMEM_BUF_INFO& rBufInfo, MUINT32 const u4Size)
{
    MBOOL ret = MFALSE;

    if( !rBufInfo.virtAddr)
    {
        rBufInfo.size = u4Size;
        rBufInfo.useNoncache = 0;   // improve the performance
        if( 0 == m_pIMemDrv->allocVirtBuf(&rBufInfo))
        {
            if( 0 != m_pIMemDrv->mapPhyAddr(&rBufInfo))
            {
                CAM_LOGE( "mapPhyAddr error, virtAddr(0x%x), size(%d)\n", rBufInfo.virtAddr, rBufInfo.size);
                ret = MFALSE;
            }
            else
            {
                ret = MTRUE;
            }
        }
        else
        {
            CAM_LOGE( "allocVirtBuf error, size(%d)\n", rBufInfo.size);
            ret = MFALSE;
        }
    }
    else
    {
        ret = MTRUE;
        CAM_LOGD( "Already Exist! virtAddr(0x%x), size(%d)\n", rBufInfo.virtAddr, u4Size);
    }
    return ret;
}
//----------------------------------------------------------------------------------------------------
MBOOL PDMgr::freeMemBuf( IMEM_BUF_INFO& rBufInfo)
{
    MBOOL ret = MTRUE;

    if( !m_pIMemDrv || rBufInfo.virtAddr == 0)
    {
        if (m_profile.BufType==EPDBuf_PDO)
        {
            CAM_LOGD("m_pIMemDrv is NULL however BPCI table is no longer allocated from m_pIMemDrv\n");
            return MFALSE;
        }
        return ret;
    }

    if( 0 == m_pIMemDrv->unmapPhyAddr(&rBufInfo))
    {
        if( 0 == m_pIMemDrv->freeVirtBuf(&rBufInfo))
        {
            CAM_LOGD("freeVirtBuf OK, memID(%d), virtAddr(0x%x), phyAddr(0x%x)", rBufInfo.memID, rBufInfo.virtAddr, rBufInfo.phyAddr);
            rBufInfo.virtAddr = 0;
            ret = MTRUE;
        }
        else
        {
            CAM_LOGD("freeVirtBuf Fail, memID(%d), virtAddr(0x%x), phyAddr(0x%x)", rBufInfo.memID, rBufInfo.virtAddr, rBufInfo.phyAddr);
            ret = MFALSE;
        }
    }
    else
    {
        CAM_LOGE("memID(%d) unmapPhyAddr error", rBufInfo.memID);
        ret = MFALSE;
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                   Thread
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID PDMgr::createThread()
{
    CAM_LOGD( "create PD mgr thread");
    //create thread
    m_bEnPDBufThd = MTRUE;
    sem_init( &m_semPDBuf, 0, 0);
    sem_init( &m_semPDBufThdEnd, 0, 1);
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
    ::prctl( PR_SET_NAME, "PDBufThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AF;
        int policy = 0, priority = 0;
        setThreadPriority( expect_policy, expect_priority);
        getThreadPriority (policy, priority);
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
        ::sem_wait( &_this->m_semPDBuf);
        if ( ! _this->m_bEnPDBufThd)
        {
            break;
        }
        CAM_LOGD_IF( _this->m_bDebugEnable, "[PDBufThreadLoop] PD task, dev:%d", _this->m_i4CurrSensorDev);



        _this->m_Status = EPD_BZ;

        CAM_LOGD_IF( _this->m_bDebugEnable, "Run Core Flow +");

        AAA_TRACE_D( "PDBufThreadLoop");
        //run core pd flow to get pd algorithm result.
        switch( _this->m_profile.BufType)
        {
        case EPDBuf_VC :
        case EPDBuf_Raw :
        case EPDBuf_PDO :
        case EPDBuf_DualPD_VC:
        case EPDBuf_DualPD_Raw:
            ret = _this->PDCoreFlow();
            break;

        case EPDBuf_VC_Open :
        case EPDBuf_Raw_Open :
        case EPDBuf_PDO_Open :
            ret = _this->PDOpenCoreFlow();
            break;

        default :
            break;

        }
        CAM_LOGD_IF( _this->m_bDebugEnable, "Run Core Flow -");

        //thread control
        {
            int Val;
            ::sem_getvalue( &_this->m_semPDBufThdEnd, &Val);
            CAM_LOGD_IF( _this->m_bDebugEnable, "[PDBufThreadLoop] semPDThdEnd before post = %d\n", Val);
            if( Val==0)
            {
                ::sem_post(&_this->m_semPDBufThdEnd); //to be 1, 1 means PD task done
            }
        }

        //ready output data
        _this->m_Status = EPD_Data_Ready;

        //check pd flow status.
        if( ret==E_3A_ERR)
        {
            CAM_LOGD_IF( _this->m_bDebugEnable, "[PDBufThreadLoop] PD mgr thread is colsed because of PD flow error!!");
            break;
        }
        AAA_TRACE_END_D;
    }

    _this->m_bEnPDBufThd = MFALSE;
    _this->m_Status      = EPD_Not_Enabled;


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

    CAM_LOGD( "%d %d %d %d",
              initPdData.rPDNVRAM.rTuningData.i4ConfThr,
              initPdData.rPDNVRAM.rTuningData.i4SaturateLevel,
              initPdData.rPDNVRAM.rTuningData.i4SaturateThr,
              sizeof(NVRAM_LENS_PARA_STRUCT));


    CAM_LOGD( "FocusPDSzX %d, FocusPDSzY %d",
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeX,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeY);


    //(1) set calibration data to PD core.
    if ((ret = m_pIPdAlgo->initPD(initPdData)) != S_3A_OK)
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

    if (m_pIHalSensor)
    {
        m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_INFO, (MINTPTR)&m_i4SensorMode, (MINTPTR)&sPDSensorInfo, 0);
        m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&m_i4SensorMode, 0);
    }
    else
    {
        CAM_LOGE("m_pIHalSensor is NULL!");
    }

    CAM_LOGD_IF(m_bDebugEnable, "SensorMode: %d(%d)", m_i4SensorMode, SENSOR_SCENARIO_ID_NORMAL_CAPTURE);

    CAM_LOGD( "SensorInfo %d %d %d %d %d %d %d %d %d %d",
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
    if (!m_sPDOHWInfo.u1IsDualPD)
    {
        m_sPDOHWInfo.u4BitDepth = 10;
        m_pPDBufMgrCore->SetPDBlockInfo( sPDSensorInfo);
    }
    else
    {
        if (m_profile.uSensorType == SensorType_DualPD_Raw)
        {
            m_sPDOHWInfo.u4BitDepth = 12;
            binningX =            initPdData.rPDNVRAM.rTuningData.i4Reserved[0];
            binningY =            initPdData.rPDNVRAM.rTuningData.i4Reserved[1];
            MINT32 needShortExp = initPdData.rPDNVRAM.rTuningData.i4Reserved[2];
            if (binningX > 8 || binningY > 8 || needShortExp > 2 || needShortExp < 0 ||
                    binningX < 1 || binningY < 1)
            {
                CAM_LOGE("%s, dualpd tuning data error! (%d %d %d)", __FUNCTION__,
                         binningX, binningY, needShortExp);
                binningX = 8;
                binningY = 8;
                needShortExp = 0;
            }

            if (m_sPDOHWInfo.u1PdSeparateMode == 1)
            {
                m_sPDOHWInfo.u4Pdo_xsz = (m_profile.uImgXsz / binningX)     - 1;
                m_sPDOHWInfo.u4Pdo_ysz = (m_profile.uImgYsz / binningY) * 2 - 1;
            }
            else if (m_sPDOHWInfo.u1PdSeparateMode == 0)
            {
                m_sPDOHWInfo.u4Pdo_xsz = (m_profile.uImgXsz / binningX) * 2 - 1;
                m_sPDOHWInfo.u4Pdo_ysz = (m_profile.uImgYsz / binningY)     - 1;
            }
            else
                CAM_LOGE("[%s]- separate mode setting ERROR!", __FUNCTION__);

            m_sPDOHWInfo.u1PBinStartLine = needShortExp ? (sPDSensorInfo.i4LeFirst ? 1 : 0) :
                                           (sPDSensorInfo.i4LeFirst ? 0 : 1);
            m_sPDOHWInfo.u1PBinStartLine +=  (m_sPDOHWInfo.u1PBinType == 1) ? 0 : 2; // in 1x4, start line + 2

            CAM_LOGD( "DualPD PBIN Type (0:1x4, 1:4x4) %d, start line: %d", m_sPDOHWInfo.u1PBinType, m_sPDOHWInfo.u1PBinStartLine);
        }
        else //SensorType_DualPD_VC
            m_sPDOHWInfo.u4BitDepth = 10;
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
        a_sPDConfig.i4IsPacked  = 1;
        a_sPDConfig.i4RawStride = m_profile.uImgXsz*10/8;
        if (sPDSensorInfo.i4BlockNumX == 0 && sPDSensorInfo.i4BlockNumY == 0) //calculate blocknum in older version
        {
            a_sPDConfig.sPdBlockInfo.i4BlockNumX = (m_profile.uImgXsz-2*sPDSensorInfo.i4OffsetX)/sPDSensorInfo.i4PitchX;
            a_sPDConfig.sPdBlockInfo.i4BlockNumY = (m_profile.uImgYsz-2*sPDSensorInfo.i4OffsetY)/sPDSensorInfo.i4PitchY;
        }
        else
        {
            a_sPDConfig.sPdBlockInfo.i4BlockNumX = sPDSensorInfo.i4BlockNumX;
            a_sPDConfig.sPdBlockInfo.i4BlockNumY = sPDSensorInfo.i4BlockNumY;
        }
        a_sPDConfig.sPdBlockInfo.i4OffsetX   = sPDSensorInfo.i4OffsetX;
        a_sPDConfig.sPdBlockInfo.i4OffsetY   = sPDSensorInfo.i4OffsetY;

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

            CAM_LOGD( "PDPos [L](%d, %d)->(%d, %d), [R](%d, %d)->(%d, %d)",
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
            if (m_profile.BufType == EPDBuf_PDO)
            {
                for( MUINT32 Pidx=0; Pidx<pairNum; Pidx++)
                {
                    CAM_LOGD( "Pdo pixel position [R](%d, %d) [L](%d, %d)",
                        m_pdoPixels[Pidx][0], m_pdoPixels[Pidx][1], m_pdoPixels[Pidx + pairNum][0], m_pdoPixels[Pidx + pairNum][1]);
                }
                if (m_pPDBufMgrCore->pdoPixelPos(m_pdoPixels, pairNum*2) == MFALSE)
                {
                    CAM_LOGE("pdoPixelsPos parsing PD coordinates Error!");
                    retAll = E_3A_ERR;
                }
            }
        }
        CAM_LOGD_IF( 1,
                     "[Core] %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                     a_sPDConfig.i4Bits,
                     a_sPDConfig.i4IsPacked,
                     a_sPDConfig.i4RawHeight,
                     a_sPDConfig.i4RawStride,
                     a_sPDConfig.i4RawWidth,
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
        a_sPDConfig.i4FullRawXOffset = sPDSensorInfo.i4Crop[m_i4SensorMode][0];
        a_sPDConfig.i4FullRawYOffset = sPDSensorInfo.i4Crop[m_i4SensorMode][1];
        a_sPDConfig.sDPdFormat.i4Bits = m_sPDOHWInfo.u4BitDepth;

        if (m_profile.uSensorType == SensorType_DualPD_Raw)
        {
            a_sPDConfig.sDPdFormat.i4BinningX = binningX;
            a_sPDConfig.sDPdFormat.i4BinningY = binningY;
            a_sPDConfig.sDPdFormat.i4BufFmt   = (m_sPDOHWInfo.u1PdSeparateMode == 1) ? 1 : 0;
            a_sPDConfig.sDPdFormat.i4BufStride = _PDO_STRIDE_ALIGN_((m_sPDOHWInfo.u4Pdo_xsz+1)*sizeof(MUINT16));
            a_sPDConfig.sDPdFormat.i4BufHeight = m_sPDOHWInfo.u4Pdo_ysz + 1;
        }
        else if (m_profile.uSensorType == SensorType_DualPD_VC)
        {
            m_pPDBufMgrCore->GetDualPDVCInfo(0, m_sDualPDVCInfo, m_AETargetMode);
            a_sPDConfig.sDPdFormat.i4BinningX = m_sDualPDVCInfo.u4VCBinningX;
            a_sPDConfig.sDPdFormat.i4BinningY = m_sDualPDVCInfo.u4VCBinningY;
            a_sPDConfig.sDPdFormat.i4BufFmt   = m_sDualPDVCInfo.u4VCBufFmt;

            a_sPDConfig.sDPdFormat.i4BufStride = rSensorVCInfo.VC2_SIZEH; //[todo] info comes from sensor?
            a_sPDConfig.sDPdFormat.i4BufHeight = rSensorVCInfo.VC2_SIZEV; //[todo] info comes from sensor?

            CAM_LOGD_IF(m_bDebugEnable, "[Core][VC Info] 0x%x/ 0x%x/ 0x%x/ 0x%x",
                        rSensorVCInfo.VC2_DataType,
                        rSensorVCInfo.VC2_ID,
                        rSensorVCInfo.VC2_SIZEH,
                        rSensorVCInfo.VC2_SIZEV);
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

    ptrbuf = m_pPDBufMgrCore->ConvertPDBufFormat( inBufSize, inBufStride, pInBuf);

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
            CAM_LOGD_IF(m_bDebugEnable, "PDXsz(%d) PDYsz(%d) PDBufSz(%d)\n", PDXsz, PDYsz, PDBufSz);
            fwrite(reinterpret_cast<void *>(ptrbuf), 1, PDBufSz*2, fp);
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

    if( (m_pIPdAlgo!=NULL))
    {
        SPDResult_T PDRes;
        //reset output result.
        memset( &PDRes, 0, sizeof(SPDResult_T));

        //output all 0  result directly once current lens position is 0
        if( ( m_curLensPos                  ) &&
                ( 0<m_numROI                    ) &&
                (   m_numROI<=MAX_SIZE_OF_PD_ROI) )
        {
            AAA_TRACE_D("pdhandle #(%d)", m_MagicNumber);

            if (!m_i4DbgDisPdHandle)
            {
                for (MINT32 i = 0; i < m_numROI; i++)
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

                    CAM_LOGD_IF( m_bDebugEnable, "win %d [Confidence] %d [LensPos] %d->%d [value] %d\n",
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
            CAM_LOGD_IF( m_bDebugEnable, "Not execute %s, lens pos:%d, number of ROI:%d", __FUNCTION__, m_curLensPos, m_numROI);
        }

        if( ret==S_3A_OK)
        {
            Mutex::Autolock lock(m_Lock);
            m_vPDOutput.push_back( PDRes);
            if( 1<m_vPDOutput.size())
            {
                //pop_front and keep only last 2 result
                m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                CAM_LOGD("pop_front, sz=%d\n", m_vPDOutput.size());
            }
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "[%s]-", __FUNCTION__);
    return ret;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                     Data path 2 : using 3rd party PD algorithm
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


MRESULT PDMgr::SetCaliData2PDOpenCore(MVOID* ptrInTuningData, MVOID* ptrInCaliData, MINT32 i4InPDInfoSz, MINT32 *ptrOutPDInfo)
{
    CAM_LOGD("%s +", __FUNCTION__);

    PD_CALIBRATION_DATA_T *ptrCaliData = reinterpret_cast<PD_CALIBRATION_DATA_T *>(ptrInCaliData);
    MUINT8 *ptrData = ptrCaliData->uData;
    MINT32 sz = m_pPDBufMgrOpen->GetPDCalSz();

    AAA_TRACE_D("SetCaliData2PDOpenCore");

    //set calibration to PD open core.
    m_pPDBufMgrOpen->SetCalibrationData( sz, ptrData);

    //get PD related information form Hybrid AF.
    m_pPDBufMgrOpen->GetPDInfo2HybridAF( i4InPDInfoSz, ptrOutPDInfo);

    AAA_TRACE_END_D;

    CAM_LOGD("%s -", __FUNCTION__);
    return S_3A_OK;
}

MRESULT PDMgr::PDOpenCoreFlow()
{
    CAM_LOGD_IF( m_bDebugEnable, "[%s] %d +", __FUNCTION__, m_numROI);

    SPDROIInput_T iPDInputData;
    SPDROIResult_T oPdOutputData;

    AAA_TRACE_D("SetDataBuf #(%d)", m_MagicNumber);
    m_pPDBufMgrOpen->SetDataBuf( m_databuf_size, m_databuf, m_frmCnt);
    AAA_TRACE_END_D;

    SPDResult_T PDRes;

    //reset output result.
    memset( &PDRes, 0, sizeof(SPDResult_T));

    //output all 0  result directly once current lens position is 0
    if( ( m_curLensPos                  ) &&
            ( 0<m_numROI                    ) &&
            (   m_numROI<=MAX_SIZE_OF_PD_ROI) )
    {
        AAA_TRACE_D("GetDefocus #(%d)", m_MagicNumber);
        for( MINT32 i=0; i<m_numROI; i++)
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

            CAM_LOGD_IF( m_bDebugEnable, "win %d [Confidence] %d [LensPos] %d->%d [value] %d\n",
                         i,
                         PDRes.ROIRes[i].DefocusConfidence,
                         m_curLensPos,
                         PDRes.ROIRes[i].Defocus,
                         PDRes.ROIRes[i].PhaseDifference);
        }
        AAA_TRACE_END_D;
    }


    {
        Mutex::Autolock lock( m_Lock);
        m_vPDOutput.push_back( PDRes);
        if( 1<m_vPDOutput.size())
        {
            //pop_front and keep only last 2 result
            m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
            CAM_LOGD("pop_front, sz=%d", m_vPDOutput.size());
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "[%s]-", __FUNCTION__);
    return S_3A_OK;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//                                   Interface
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT PDMgr::GetVersionOfPdafLibrary(SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret=E_3A_ERR;

    if( m_bEnPDBufThd)
    {
        if (m_profile.BufType & EPDBuf_Type_Open_MSK)
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

        CAM_LOGD( "%s = %d.%d", __FUNCTION__, (int)tOutSWVer.MajorVersion, (int)tOutSWVer.MinorVersion);
    }
    return ret;
}

MRESULT PDMgr::setPDCaliData( MVOID *ptrInTuningData, MVOID *ptrInCaliData, MINT32 &i4OutInfoSz, MINT32 **ptrOutInfo)
{

    //This function is used to set PD calibration data and output PD related data for Hybrid AF.
    MRESULT ret=E_3A_ERR;

    if( m_bEnPDBufThd)
    {
        if (m_profile.BufType != EPDBuf_NotDef)
        {
            if(m_profile.BufType & EPDBuf_Type_Open_MSK)
                ret = SetCaliData2PDOpenCore( ptrInTuningData, ptrInCaliData, 10, m_i4PDInfo2HybirdAF);
            else
                ret = SetCaliData2PDCore( ptrInTuningData, ptrInCaliData, 10, m_i4PDInfo2HybirdAF);
        }

        if( ret==E_3A_ERR)
        {
            closeThread();
            i4OutInfoSz   = 0;
            (*ptrOutInfo) = NULL;
        }
        else
        {
            //This m_i4PDInfo2HybirdAF arrary size is defined by core pd algorithm
            i4OutInfoSz   = 10;
            (*ptrOutInfo) = m_i4PDInfo2HybirdAF;
        }
    }


    return ret;
}

//----------------------------------------------------------------------------------------------------
MRESULT PDMgr::postToPDTask( SPDInputData_t *ptrInputData)
{
    MRESULT ret = E_3A_ERR;

    if( m_bEnPDBufThd)
    {
        if( ptrInputData)
        {
            //Run thread if PD task is not busy.
            int Val;
            ::sem_getvalue( &m_semPDBufThdEnd, &Val);
            CAM_LOGD_IF( m_bDebugEnable, "[%s] sem_post m_semPDBuf, m_semPDBufThdEnd before wait = %d", __FUNCTION__, Val);

            //checking PD thread is busy or not.
            if( Val==1)
            {
                m_MagicNumber = ptrInputData->magicNumber;
                m_frmCnt = ptrInputData->frmNum;

                //(1) do pdconvert if need to.
                m_lrbuf = NULL;
                if( m_profile.BufType==EPDBuf_VC || m_profile.BufType==EPDBuf_PDO )
                {
                    m_lrbuf = PDConvert(ptrInputData->databuf_virtAddr, ptrInputData->databuf_size, ptrInputData->databuf_stride);
                }
                else if (m_profile.BufType & EPDBuf_Type_Open_MSK)
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
                else if (m_profile.BufType == EPDBuf_Raw)
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

                //(4) get current lens' position and AfeGain
                m_curLensPos = ptrInputData->curLensPos;
                m_afeGain = ptrInputData->afeGain;

                //(5) do pdextract & post to thread
                if (m_profile.BufType & EPDBuf_Type_Open_MSK)
                {
                    //post to thread
                    ::sem_wait( &m_semPDBufThdEnd); //to be 0, it won't block, 0 means PD task not ready yet
                    ::sem_post( &m_semPDBuf);
                }
                else //(!(m_profile.BufType & EPDBuf_Type_Open_MSK))
                {
                    if (m_curLensPos)
                    {
                        // without m_curLensPos,
                        // don't do pdextract and don't post to thread
                        // to ensure pdextract is paired with pdconvert

                        PD_EXTRACT_INPUT_T a_sPDInput;

                        //(1) select data buffer type
                        if( m_profile.BufType==EPDBuf_VC || m_profile.BufType==EPDBuf_PDO)
                        {
                            a_sPDInput.pRawBuf = NULL;
                            a_sPDInput.pPDBuf = reinterpret_cast<MVOID *>(m_lrbuf);
                        }
                        else if (m_profile.BufType & EPDBuf_Type_DualPD_MSK)
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
                                        m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&m_i4SensorMode, 0);
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
                        else //EPDBuf_Raw
                        {
                            a_sPDInput.pRawBuf = reinterpret_cast<MVOID *>(ptrInputData->databuf_virtAddr);
                            a_sPDInput.pPDBuf  = NULL;
                        }

                        AAA_TRACE_D("pdextract");

                        for( MINT32 i=0; i<m_numROI; i++)
                        {
                            a_sPDInput.sFocusWin.i4W = m_ROI[i].i4W;
                            a_sPDInput.sFocusWin.i4H = m_ROI[i].i4H;
                            a_sPDInput.sFocusWin.i4X = m_ROI[i].i4X;
                            a_sPDInput.sFocusWin.i4Y = m_ROI[i].i4Y;

                            CAM_LOGD_IF( m_bDebugEnable, "[pdextract] win %d [WinPos] %d, %d, %d, %d\n",
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
            ret = S_3A_OK;

        }
        else
        {
            CAM_LOGD_IF( m_bDebugEnable, "[%s] ERR : input is NULL, close thread", __FUNCTION__);
            ret = E_3A_ERR;
        }


        if( ret==E_3A_ERR)
        {
            closeThread();
        }
    }
    else
    {
        CAM_LOGD_IF( m_bDebugEnable, "[%s] Can not post to pd thread because of thread is closed", __FUNCTION__);
        ret = E_3A_ERR;
    }


    return ret;
}


MRESULT PDMgr::getPDTaskResult( SPDOutputData_t *ptrOutputRes)
{
    MRESULT ret = E_3A_ERR;

    if( m_bEnPDBufThd && ptrOutputRes->numRes!=0)
    {
        Mutex::Autolock lock(m_Lock);
        if( m_vPDOutput.size())
        {
            //output
            ptrOutputRes->numRes = ptrOutputRes->numRes>m_numROI ? m_numROI : ptrOutputRes->numRes;
            memcpy( ptrOutputRes->Res, m_vPDOutput.begin()->ROIRes, sizeof(SPDROIResult_T)*(ptrOutputRes->numRes));

            for( MUINT32 i=0; i<ptrOutputRes->numRes; i++)
            {

                CAM_LOGD_IF( m_bDebugEnable,
                             "[getPDRes][%d] ToPos %4d, C %3d, CL %3d, PD %5d\n",
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
    case EPDBuf_VC :
    case EPDBuf_Raw :
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

