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
#define LOG_TAG "isp_mgr_pdo"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <drv/tuning_mgr.h>
#include <isp_mgr_pdo.h>
#include <mtkcam/aaa/IIspMgr.h>

namespace NSIspTuningv3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PDO_CONFIG_T&
ISP_MGR_PDO_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_PDO_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_PDO_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_PDO_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Sensor
        return  ISP_MGR_PDO_CONFIG_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_PDO_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_PDO_CONFIG_T::
start(MINT32 i4SensorIdx)
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    m_bDebugEnable  = property_get_int32("vendor.debug.isp_mgr_af.enable", 0);
    m_bUpdateEngine = MFALSE;
    m_i4SensorIdx = i4SensorIdx;

    CAM_LOGD("[%s] -", __FUNCTION__);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_PDO_CONFIG_T::
stop()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    //disable PDC
    if ( auto pIspMgr = IIspMgr::getInstance() )
    {
        pIspMgr->setPDCEnable(m_i4SensorIdx, MFALSE);
    }
    m_bUpdateEngine = MFALSE;

    CAM_LOGD("[%s] -", __FUNCTION__);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_PDO_CONFIG_T::
config( SPDOHWINFO_T &sInPdohwCfg)
{

    if( sInPdohwCfg.u1IsDualPD)
    {
        CAM_REG_PBN_TYPE reg_pbn;
        reg_pbn.Raw = 0;
        reg_pbn.Bits.PBN_TYPE = sInPdohwCfg.u1PBinType;
        reg_pbn.Bits.PBN_SEP  = sInPdohwCfg.u1PdSeparateMode;
        reg_pbn.Bits.PBN_OBIT = sInPdohwCfg.u4BitDepth-1;

        REG_INFO_VALUE(CAM_PBN_TYPE)       = reg_pbn.Raw;
        REG_INFO_VALUE(CAM_PBN_LST)        = sInPdohwCfg.u1PBinStartLine;

        REG_INFO_VALUE(CAM_PDO_XSIZE)      = (sInPdohwCfg.u4Pdo_xsz+1) * sizeof(MUINT16);
        REG_INFO_VALUE(CAM_PDO_YSIZE)      = sInPdohwCfg.u4Pdo_ysz+1;
        REG_INFO_VALUE(CAM_PDO_STRIDE)     = _PDO_STRIDE_ALIGN_((sInPdohwCfg.u4Pdo_xsz+1)*sizeof(MUINT16));
    }
    else
    {
        //enable PDC
        if ( auto pIspMgr = IIspMgr::getInstance() )
        {
            pIspMgr->setPDCEnable(m_i4SensorIdx, MTRUE);
        }
        // bnr setting
        REG_INFO_VALUE(CAM_PDO_XSIZE)      = (sInPdohwCfg.u4Pdo_xsz+1)*2;
        REG_INFO_VALUE(CAM_PDO_YSIZE)      = sInPdohwCfg.u4Pdo_ysz+1;
        REG_INFO_VALUE(CAM_PDO_STRIDE)     = _PDO_STRIDE_ALIGN_((sInPdohwCfg.u4Pdo_xsz+1)*2);
    }

    m_sPDOHWInfo    = sInPdohwCfg;
    m_bUpdateEngine = MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_PDO_CONFIG_T::
apply(TuningMgr& rTuning, MINT32& i4Magic, MINT32 i4SubsampleIdex)
{
    if( m_bUpdateEngine)
    {
        if( m_sPDOHWInfo.u1IsDualPD)
        {
            rTuning.updateEngine(eTuningMgrFunc_PBN, MTRUE, i4SubsampleIdex);
        }
        else
        {
            rTuning.updateEngine(eTuningMgrFunc_BNR, MTRUE, i4SubsampleIdex);
            rTuning.updateEngineFD(eTuningMgrFunc_BNR, 0, m_sPDOHWInfo.i4memID, m_sPDOHWInfo.virAddrBpci_tbl);
        }

        // Register setting
        rTuning.tuningMgrWriteRegs( static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum, i4SubsampleIdex);

        dumpRegInfoP1("PDO");
    }

    CAM_LOGD_IF( m_bDebugEnable,
                 "%s, #(%d), bUpdateEngine(%d), memID(%d), IsDualPix(%d)",
                 __FUNCTION__,
                 i4Magic,
                 m_bUpdateEngine,
                 m_sPDOHWInfo.i4memID,
                 m_sPDOHWInfo.u1IsDualPD);

    return MTRUE;
}

} // namespace NSIspTuningv3