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

#define EN_BNR_PDO 0

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <drv/tuning_mgr.h>
#include <isp_mgr_pdo.h>
#include <mtkcam/aaa/IIspMgr.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_PDO_CONFIG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


namespace NSIspTuningv3
{

/******************************************************************************
 *
 ******************************************************************************/
#define _A_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

struct DUMP_INFO
{
    DUMP_INFO()
    {
#define __I__(param) rRegs[E_DUMP_##param].Addr = _A_P1(param)


        __I__(CAM_CTL_DMA_EN);     /* 0x1A004008 */
        __I__(CAM_PDO_XSIZE);      /* 0x1A004350 */
        __I__(CAM_PDO_YSIZE);      /* 0x1A004354 */
        __I__(CAM_PDO_STRIDE);     /* 0x1A004358 */
        __I__(CAM_PBN_TYPE);       /* 0x1A004BB0 */
        __I__(CAM_PBN_LST);        /* 0x1A004BB4 */
        __I__(CAM_PDE_TBLI1);      /* 0x1A004CF0 */
        __I__(CAM_PDI_BASE_ADDR);  /* 0x1A004D50 */
        __I__(CAM_PDI_XSIZE);      /* 0x1A004D5C */
        __I__(CAM_PDI_YSIZE);      /* 0x1A004D60 */
        __I__(CAM_BNR_PDC_GAIN_L0);/* 0x1A004664 */
        __I__(CAM_BNR_PDC_GAIN_R0);/* 0x1A004678 */
    }

    typedef enum
    {
        E_DUMP_CAM_CTL_DMA_EN = 0, /* 0x1A004008 */
        E_DUMP_CAM_PDO_XSIZE,      /* 0x1A004350 */
        E_DUMP_CAM_PDO_YSIZE,      /* 0x1A004354 */
        E_DUMP_CAM_PDO_STRIDE,     /* 0x1A004358 */
        E_DUMP_CAM_PBN_TYPE,       /* 0x1A004BB0 */
        E_DUMP_CAM_PBN_LST,        /* 0x1A004BB4 */
        E_DUMP_CAM_PDE_TBLI1,      /* 0x1A004CF0 */
        E_DUMP_CAM_PDI_BASE_ADDR,  /* 0x1A004D50 */
        E_DUMP_CAM_PDI_XSIZE,      /* 0x1A004D5C */
        E_DUMP_CAM_PDI_YSIZE,      /* 0x1A004D60 */
        E_DUMP_CAM_BNR_PDC_GAIN_L0,/* 0x1A004664 */
        E_DUMP_CAM_BNR_PDC_GAIN_R0,/* 0x1A004678 */
        E_DUMP_NUM
    } E_DUMP_REG_T;

    IspP1RegIo rRegs[E_DUMP_NUM];
};


/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    CAM_LOGE_IF(!pModule, "INormalPipeModule::get() fail");
    return pModule;
}


static MVOID* createDefaultNormalPipe(MUINT32 sensorIndex, char const* szCallerName)
{
    auto pModule = getNormalPipeModule();
    if  ( ! pModule )
    {
        CAM_LOGE("getNormalPipeModule() fail");
        return NULL;
    }

    //  Select CamIO version
    size_t count = 0;
    MUINT32 const* version = NULL;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIndex);
    if  ( err < 0 || ! count || ! version )
    {
        CAM_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIndex, err, count, version
        );
        return NULL;
    }

    MUINT32 const selected_version = *(version + count - 1); //Select max. version
    CAM_LOGD("[%d] count:%zu Selected CamIO Version:%0#x", sensorIndex, count, selected_version);

    MVOID* pPipe = NULL;
    pModule->createSubModule(sensorIndex, szCallerName, selected_version, (MVOID**)&pPipe);
    return pPipe;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PDO Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PDO_CONFIG_T&
ISP_MGR_PDO_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_PDO_CONFIG_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
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
    if( 16<=sInPdohwCfg.u4BitDepth)
    {
        CAM_LOGW( "[%s] wrong bitDepth setting, change setting :%d->12",
                  __FUNCTION__,
                  sInPdohwCfg.u4BitDepth);

        sInPdohwCfg.u4BitDepth = 12;
    }


    if (sInPdohwCfg.u1IsDualPD)
    {
        CAM_REG_PBN_TYPE reg_pbn;
        reg_pbn.Raw = 0;
        reg_pbn.Bits.PBN_TYPE = sInPdohwCfg.u1PBinType;
        reg_pbn.Bits.PBN_SEP  = sInPdohwCfg.u1PdSeparateMode;
        reg_pbn.Bits.PBN_OBIT = sInPdohwCfg.u4BitDepth-1;

        REG_INFO_VALUE(CAM_PBN_TYPE)       = reg_pbn.Raw;
        REG_INFO_VALUE(CAM_PBN_LST)        = sInPdohwCfg.u1PBinStartLine;

        REG_INFO_VALUE(CAM_PDO_XSIZE)      = sInPdohwCfg.u4Pdo_xsz+1;
        REG_INFO_VALUE(CAM_PDO_YSIZE)      = sInPdohwCfg.u4Pdo_ysz+1;
        REG_INFO_VALUE(CAM_PDO_STRIDE)     = _PDO_STRIDE_ALIGN_(sInPdohwCfg.u4Pdo_xsz+1);

        CAM_LOGD( "[%s] dualPD CAM_PBN_TYPE(0x%08x) CAM_PBN_LST(0x%08x) CAM_PDO_XSIZE(0x%08x) CAM_PDO_YSIZE(0x%08x) CAM_PDO_STRIDE(0x%08x) CAM_PDO_STRIDE(0x%08x)",
                  __FUNCTION__,
                  REG_INFO_VALUE(CAM_PBN_TYPE),
                  REG_INFO_VALUE(CAM_PBN_LST),
                  REG_INFO_VALUE(CAM_PDO_XSIZE),
                  REG_INFO_VALUE(CAM_PDO_YSIZE),
                  REG_INFO_VALUE(CAM_PDO_STRIDE));
    }
    else
    {
        /* pdo setting :
         * Please refernece in Isp_function_cam.dmax.cpp for more detail.
         *
         * Unit of u4Pdo_xsz is BYTE and start from 0, so total byte of PDO x size is (u4Pdo_xsz+1).
         * Unit of CAM_PDO_XSIZE and CAM_PDO_YSIZE is BYTE and value starts from 1.
         * CAM_PDO_STRIDE should be aligned by 16 byte.
         *
         * So CAM_PDO_XSIZE is pdo x size in BYTE(u4Pdo_xsz+1) + one EOL data(2 byte).
         */
        REG_INFO_VALUE(CAM_PDO_XSIZE)      = (sInPdohwCfg.u4Pdo_xsz+1) + 2;
        REG_INFO_VALUE(CAM_PDO_YSIZE)      = sInPdohwCfg.u4Pdo_ysz+1;
        REG_INFO_VALUE(CAM_PDO_STRIDE)     = _PDO_STRIDE_ALIGN_((sInPdohwCfg.u4Pdo_xsz+1)/2 + 1) * 2; /* Actually, this value is not used inside pass1 dirvier*/

        CAM_LOGD( "[%s] shieldPD CAM_PDO_XSIZE(0x%08x) CAM_PDO_YSIZE(0x%08x) CAM_PDO_STRIDE(0x%08x)",
                  __FUNCTION__,
                  REG_INFO_VALUE(CAM_PDO_XSIZE),
                  REG_INFO_VALUE(CAM_PDO_YSIZE),
                  REG_INFO_VALUE(CAM_PDO_STRIDE));

#if EN_BNR_PDO == 1

        sInPdohwCfg.u4BitDepth = 12;

#else
        sInPdohwCfg.u4BitDepth = 12;

        // pde setting
        REG_INFO_VALUE(CAM_PDE_TBLI1)      = 0;
        REG_INFO_VALUE(CAM_PDI_BASE_ADDR)  = sInPdohwCfg.phyAddrBpci_tbl;
        REG_INFO_VALUE(CAM_PDI_XSIZE)      = sInPdohwCfg.u4Bpci_xsz+1;
        REG_INFO_VALUE(CAM_PDI_YSIZE)      = sInPdohwCfg.u4Bpci_ysz+1;

        CAM_LOGD( "[%s] shieldPD CAM_PDE_TBLI1(0x%08x) CAM_PDI_BASE_ADDR(0x%08x) CAM_PDI_XSIZE(0x%08x) CAM_PDI_YSIZE(0x%08x)",
                  __FUNCTION__,
                  REG_INFO_VALUE(CAM_PDE_TBLI1),
                  REG_INFO_VALUE(CAM_PDI_BASE_ADDR),
                  REG_INFO_VALUE(CAM_PDI_XSIZE),
                  REG_INFO_VALUE(CAM_PDI_YSIZE));
#endif

        //enable PDC
        if ( auto pIspMgr = IIspMgr::getInstance() )
        {
            pIspMgr->setPDCEnable(m_i4SensorIdx, MTRUE);
        }

    }

    m_sPDOHWInfo = sInPdohwCfg;
    m_bUpdateEngine = MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_PDO_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 &i4Magic, MINT32 i4SubsampleIdex)
{
    if( m_bUpdateEngine)
    {
        if (m_sPDOHWInfo.u1IsDualPD)
        {
            rTuning.updateEngine(eTuningMgrFunc_PBN, MTRUE, i4SubsampleIdex);
        }
        else
        {
#if EN_BNR_PDO == 1
            rTuning.updateEngine(eTuningMgrFunc_BNR, MTRUE, i4SubsampleIdex);
            rTuning.updateEngineFD(eTuningMgrFunc_BNR, 0, m_sPDOHWInfo.i4memID, m_sPDOHWInfo.virAddrBpci_tbl);
#else
            rTuning.updateEngine(eTuningMgrFunc_PDE, MTRUE, i4SubsampleIdex);
            rTuning.updateEngineFD(eTuningMgrFunc_PDE, i4SubsampleIdex, m_sPDOHWInfo.i4memID, m_sPDOHWInfo.virAddrBpci_tbl);
#endif
        }

        // Register setting
        rTuning.tuningMgrWriteRegs( static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum, i4SubsampleIdex);

        dumpRegInfoP1("PDO");
    }
    else
    {
        /*****************************************************
         * engine should be disabled when moidule is no need *
         *****************************************************/
        rTuning.enableEngine(eTuningMgrFunc_PBN, MFALSE, i4SubsampleIdex);
#if EN_BNR_PDO == 1

#else
        rTuning.enableEngine(eTuningMgrFunc_PDE, MFALSE, i4SubsampleIdex);
#endif
    }

    CAM_LOGD_IF( m_bDebugEnable,
                 "%s, #(%d), bUpdateEngine(%d), memID(%d), IsDualPix(%d)",
                 __FUNCTION__,
                 i4Magic,
                 m_bUpdateEngine,
                 m_sPDOHWInfo.i4memID,
                 m_sPDOHWInfo.u1IsDualPD);

    if( m_bDebugEnable)
    {
        INormalPipe* pPipe = (INormalPipe*)createDefaultNormalPipe( m_i4SensorIdx, LOG_TAG);

#define _DUMP(param) CAM_LOGD("%-40s:    0x%08X\n", #param, rDump.rRegs[DUMP_INFO::E_DUMP_##param].Data)

        DUMP_INFO rDump;
        pPipe->getIspReg(rDump.rRegs, DUMP_INFO::E_DUMP_NUM);

        _DUMP(CAM_CTL_DMA_EN);
        _DUMP(CAM_PDO_XSIZE);
        _DUMP(CAM_PDO_YSIZE);
        _DUMP(CAM_PDO_STRIDE);
        _DUMP(CAM_PBN_TYPE);
        _DUMP(CAM_PBN_LST);
        _DUMP(CAM_PDE_TBLI1);
        _DUMP(CAM_PDI_BASE_ADDR);
        _DUMP(CAM_PDI_XSIZE);
        _DUMP(CAM_PDI_YSIZE);
        _DUMP(CAM_BNR_PDC_GAIN_L0);
        _DUMP(CAM_BNR_PDC_GAIN_R0);

        pPipe->destroyInstance( LOG_TAG);

        CAM_REG_CTL_DMA_EN cam_ctl_dma_en;
        cam_ctl_dma_en.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_CAM_CTL_DMA_EN].Data;
        CAM_LOGD( "PDE_EN(%d), PDI_EN(%d)", cam_ctl_dma_en.Bits.PDE_EN, cam_ctl_dma_en.Bits.PDI_EN);

        CAM_REG_BNR_PDC_GAIN_L0 cam_bnr_pdc_gain_l0;
        cam_bnr_pdc_gain_l0.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_CAM_BNR_PDC_GAIN_L0].Data;

        CAM_REG_BNR_PDC_GAIN_R0 cam_bnr_pdc_gain_r0;
        cam_bnr_pdc_gain_r0.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_CAM_BNR_PDC_GAIN_R0].Data;

        CAM_LOGW("PDC_GCF_L00=%d, PDC_GCF_R00=%d", cam_bnr_pdc_gain_l0.Bits.PDC_GCF_L00, cam_bnr_pdc_gain_r0.Bits.PDC_GCF_R00);
    }

    return MTRUE;
}

} // namespace NSIspTuningv3
