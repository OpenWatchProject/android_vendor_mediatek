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

#include "isp_mgr.h"
#include "isp_mgr_config.h"
#include <mtkcam/aaa/IIspMgr.h>
#include <drv/tuning_mgr.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>


using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


namespace NSIspTuning
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
/*

        __I__(CAMCTL_R1_CAMCTL_DMA_EN);
        __I__(CAMCTL_R1_CAMCTL_EN2);
        __I__(PDO_R1_PDO_XSIZE);
        __I__(PDO_R1_PDO_YSIZE);
        __I__(PDO_R1_PDO_STRIDE);
        __I__(PBN_R1_PBN_PBN_TYPE);
        __I__(PBN_R1_PBN_PBN_LST);
        __I__(PDE_R1_PDE_TBLI1);
        __I__(PDE_R1_PDE_CTL);
        __I__(PDI_R1_PDI_BASE_ADDR);
        __I__(PDI_R1_PDI_XSIZE);
        __I__(PDI_R1_PDI_YSIZE);
        __I__(BPC_R1_BPC_PDC_GAIN_L0);
        __I__(BPC_R1_BPC_PDC_GAIN_R0);
*/
    }

    typedef enum
    {
        E_DUMP_CAMCTL_R1_CAMCTL_DMA_EN = 0,
        E_DUMP_CAMCTL_R1_CAMCTL_EN2,
        E_DUMP_PDO_R1_PDO_XSIZE,
        E_DUMP_PDO_R1_PDO_YSIZE,
        E_DUMP_PDO_R1_PDO_STRIDE,
        E_DUMP_PBN_R1_PBN_PBN_TYPE,
        E_DUMP_PBN_R1_PBN_PBN_LST,
        E_DUMP_PDE_R1_PDE_TBLI1,
        E_DUMP_PDE_R1_PDE_CTL,
        E_DUMP_PDI_R1_PDI_BASE_ADDR,
        E_DUMP_PDI_R1_PDI_XSIZE,
        E_DUMP_PDI_R1_PDI_YSIZE,
        E_DUMP_BPC_R1_BPC_PDC_GAIN_L0,
        E_DUMP_BPC_R1_BPC_PDC_GAIN_R0,
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
    ISP_MGR_MODULE_GET_INSTANCE(PDO_CONFIG);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_PDO_CONFIG_T::
configReg( ISP_PDO_CFG_T *sInPdohwCfg)
{
#if 0
HHHHHH
    //
    m_bDebugEnable  = property_get_int32("vendor.debug.isp_mgr_pdo.enable", 0);

    //
    m_sPDOHWCfg = *sInPdohwCfg;

    if( m_sPDOHWCfg.pdo_ctl.Bits.pdo_en)
    {
        if( m_sPDOHWCfg.pdo_ctl.Bits.pbn_en)
        {
            REG_PBN_R1_PBN_PBN_TYPE reg_pbn;
            reg_pbn.Raw = 0;
            reg_pbn.Bits.PBN_PBN_TYPE = m_sPDOHWCfg.pbn_type;
            reg_pbn.Bits.PBN_PBN_SEP  = m_sPDOHWCfg.pbn_separate_mode;
            reg_pbn.Bits.PBN_PBN_OBIT = m_sPDOHWCfg.bit_depth-1;

            REG_INFO_VALUE_MULTI(ER1, PBN_R1_PBN_PBN_TYPE) = reg_pbn.Raw;
            REG_INFO_VALUE_MULTI(ER1, PBN_R1_PBN_PBN_LST)  = m_sPDOHWCfg.pbn_start_line;

            REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_XSIZE)    = m_sPDOHWCfg.pdo_xsz+1;
            REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_YSIZE)    = m_sPDOHWCfg.pdo_ysz+1;
            REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_STRIDE)   = _PDO_STRIDE_ALIGN_(m_sPDOHWCfg.pdo_xsz+1);

            CAM_LOGD( "[%s] dualPD PBN_R1_PBN_PBN_TYPE(0x%08x) PBN_R1_PBN_PBN_LST(0x%08x) PDO_R1_PDO_XSIZE(0x%08x) PDO_R1_PDO_YSIZE(0x%08x) PDO_R1_PDO_STRIDE(0x%08x)",
                      __FUNCTION__,
                      REG_INFO_VALUE_MULTI(ER1, PBN_R1_PBN_PBN_TYPE),
                      REG_INFO_VALUE_MULTI(ER1, PBN_R1_PBN_PBN_LST),
                      REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_XSIZE),
                      REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_YSIZE),
                      REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_STRIDE));
        }
        else if( m_sPDOHWCfg.pdo_ctl.Bits.pde_en)
        {
            /* pdo setting :
             * Please refernece in Isp_function_cam.dmax.cpp for more detail.
             *
             * Unit of u4Pdo_xsz is BYTE and start from 0, so total byte of PDO x size is (u4Pdo_xsz+1).
             * Unit of PDO_R1_PDO_XSIZE and PDO_R1_PDO_YSIZE is BYTE and value starts from 1.
             * PDO_R1_PDO_STRIDE should be aligned by 16 byte.
             *
             * So PDO_R1_PDO_XSIZE is pdo x size in BYTE(u4Pdo_xsz+1) + one EOL data(2 byte).
             */
            REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_XSIZE)  = (m_sPDOHWCfg.pdo_xsz+1) + 2;
            REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_YSIZE)  = m_sPDOHWCfg.pdo_ysz+1;
            REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_STRIDE) = _PDO_STRIDE_ALIGN_((m_sPDOHWCfg.pdo_xsz+1)/2 + 1) * 2; /* Actually, this value is not used inside pass1 dirvier*/

            CAM_LOGD( "[%s] shieldPD PDO_R1_PDO_XSIZE(0x%08x) PDO_R1_PDO_YSIZE(0x%08x) PDO_R1_PDO_STRIDE(0x%08x)",
                      __FUNCTION__,
                      REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_XSIZE),
                      REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_YSIZE),
                      REG_INFO_VALUE_MULTI(ER1, PDO_R1_PDO_STRIDE));

#if EN_BNR_PDO == 1
#else
            REG_PDE_R1_PDE_CTL reg_pde_ctl;
            reg_pde_ctl.Raw = 0;
            reg_pde_ctl.Bits.PDE_OBIT = m_sPDOHWCfg.bit_depth;

            // pde setting
            REG_INFO_VALUE_MULTI(ER1, PDE_R1_PDE_TBLI1)     = 0;
            REG_INFO_VALUE_MULTI(ER1, PDE_R1_PDE_CTL)       = reg_pde_ctl.Raw;
            REG_INFO_VALUE_MULTI(ER1, PDI_R1_PDI_BASE_ADDR) = m_sPDOHWCfg.pdi_tbl_pa;
            REG_INFO_VALUE_MULTI(ER1, PDI_R1_PDI_XSIZE)     = m_sPDOHWCfg.pdi_tbl_xsz+1;
            REG_INFO_VALUE_MULTI(ER1, PDI_R1_PDI_YSIZE)     = m_sPDOHWCfg.pdi_tbl_ysz+1;

            CAM_LOGD( "[%s] shieldPD PDE_R1_PDE_TBLI1(0x%08x) PDE_R1_PDE_CTL(0x%08x) PDI_R1_PDI_BASE_ADDR(0x%08x) PDI_R1_PDI_XSIZE(0x%08x) PDI_R1_PDI_YSIZE(0x%08x)",
                      __FUNCTION__,
                      REG_INFO_VALUE_MULTI(ER1, PDE_R1_PDE_TBLI1),
                      REG_INFO_VALUE_MULTI(ER1, PDE_R1_PDE_CTL),
                      REG_INFO_VALUE_MULTI(ER1, PDI_R1_PDI_BASE_ADDR),
                      REG_INFO_VALUE_MULTI(ER1, PDI_R1_PDI_XSIZE),
                      REG_INFO_VALUE_MULTI(ER1, PDI_R1_PDI_YSIZE));
#endif

            //enable PDC
            if ( auto pIspMgr = IIspMgr::getInstance() )
            {
                pIspMgr->setPDCEnable(m_sPDOHWCfg.sensor_idx, MTRUE);
            }

        }
        else
        {
            CAM_LOGE( "%s, setting is not correct!");
        }
    }
    else
    {
        //disable PDC
        if ( auto pIspMgr = IIspMgr::getInstance() )
        {
            pIspMgr->setPDCEnable(m_sPDOHWCfg.sensor_idx, MFALSE);
        }
    }
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_PDO_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 &i4Magic, MINT32 i4SubsampleIdex)
{
    if( m_sPDOHWCfg.pdo_ctl.Bits.pdo_en)
    {
        if( m_sPDOHWCfg.pdo_ctl.Bits.pbn_en)
        {
            //PBN is removed on isp6s
            //rTuning.updateEngine(eTuningMgrFunc_PBN_R1, MTRUE, i4SubsampleIdex);
        }
        else if( m_sPDOHWCfg.pdo_ctl.Bits.pde_en)
        {
#if EN_BNR_PDO == 1
            rTuning.updateEngine(eTuningMgrFunc_BNR, MTRUE, i4SubsampleIdex);
            rTuning.updateEngineFD(eTuningMgrFunc_BNR, 0, m_sPDOHWCfg.pdi_tbl_memID, m_sPDOHWCfg.pdi_tbl_va);
#else
            rTuning.updateEngine(eTuningMgrFunc_PDE_R1, MTRUE, i4SubsampleIdex);
            rTuning.updateEngineFD(eTuningMgrFunc_PDE_R1, i4SubsampleIdex, m_sPDOHWCfg.pdi_tbl_memID, m_sPDOHWCfg.pdi_tbl_va);
#endif
        }
        else
        {
            CAM_LOGE( "%s, setting is not correct!");
        }


        AAA_TRACE_DRV(DRV_PDO);
        // Register setting
        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[ER1][0])), m_u4RegInfoNum, i4SubsampleIdex);
        AAA_TRACE_END_DRV;

        dumpRegInfoP1("PDO");
    }
    else
    {
        /*****************************************************
         * engine should be disabled when moidule is no need *
         *****************************************************/
        //PBN is removed on isp6s
        //rTuning.enableEngine(eTuningMgrFunc_PBN_R1, MFALSE, i4SubsampleIdex);
#if EN_BNR_PDO == 1

#else
        rTuning.enableEngine(eTuningMgrFunc_PDE_R1, MFALSE, i4SubsampleIdex);
#endif
    }
/*
    CAM_LOGD_IF( m_bDebugEnable,
                 "%s, #(%d) memID(%d), pdo_ctrl(%d/%d/%d)",
                 __FUNCTION__,
                 i4Magic,
                 m_sPDOHWCfg.pdi_tbl_memID,
                 m_sPDOHWCfg.pdo_ctl.Bits.pdo_en,
                 m_sPDOHWCfg.pdo_ctl.Bits.pde_en,
                 m_sPDOHWCfg.pdo_ctl.Bits.pbn_en);

    if( m_bDebugEnable)
    {
        INormalPipe* pPipe = (INormalPipe*)createDefaultNormalPipe( m_sPDOHWCfg.sensor_idx, LOG_TAG);

#define _DUMP(param) CAM_LOGD("%-40s:    0x%08X\n", #param, rDump.rRegs[DUMP_INFO::E_DUMP_##param].Data)

        DUMP_INFO rDump;
        pPipe->getIspReg(rDump.rRegs, DUMP_INFO::E_DUMP_NUM);

        _DUMP(CAMCTL_R1_CAMCTL_DMA_EN);
        _DUMP(CAMCTL_R1_CAMCTL_EN2);
        _DUMP(PDO_R1_PDO_XSIZE);
        _DUMP(PDO_R1_PDO_YSIZE);
        _DUMP(PDO_R1_PDO_STRIDE);
        _DUMP(PBN_R1_PBN_PBN_TYPE);
        _DUMP(PBN_R1_PBN_PBN_LST);
        _DUMP(PDE_R1_PDE_TBLI1);
        _DUMP(PDE_R1_PDE_CTL);
        _DUMP(PDI_R1_PDI_BASE_ADDR);
        _DUMP(PDI_R1_PDI_XSIZE);
        _DUMP(PDI_R1_PDI_YSIZE);
        _DUMP(BPC_R1_BPC_PDC_GAIN_L0);
        _DUMP(BPC_R1_BPC_PDC_GAIN_R0);

        pPipe->destroyInstance( LOG_TAG);

        REG_CAMCTL_R1_CAMCTL_DMA_EN cam_ctl_dma_en;
        cam_ctl_dma_en.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_CAMCTL_R1_CAMCTL_DMA_EN].Data;
        CAM_LOGD( "PDO_R1_EN(%d), PDI_R1_EN(%d)", cam_ctl_dma_en.Bits.CAMCTL_PDO_R1_EN, cam_ctl_dma_en.Bits.CAMCTL_PDI_R1_EN);

        REG_CAMCTL_R1_CAMCTL_EN2 cam_ctl_en2;
        cam_ctl_en2.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_CAMCTL_R1_CAMCTL_EN2].Data;
        CAM_LOGD( "PDE_R1_EN(%d), PBN_R1_EN(%d)", cam_ctl_en2.Bits.CAMCTL_PDE_R1_EN, cam_ctl_en2.Bits.CAMCTL_PBN_R1_EN);

        REG_BPC_R1_BPC_PDC_GAIN_L0 cam_bnr_pdc_gain_l0;
        cam_bnr_pdc_gain_l0.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_BPC_R1_BPC_PDC_GAIN_L0].Data;

        REG_BPC_R1_BPC_PDC_GAIN_R0 cam_bnr_pdc_gain_r0;
        cam_bnr_pdc_gain_r0.Raw = rDump.rRegs[DUMP_INFO::E_DUMP_BPC_R1_BPC_PDC_GAIN_R0].Data;

        CAM_LOGD("BPC_PDC_GCF_L00=%d, BPC_PDC_GCF_R00=%d", cam_bnr_pdc_gain_l0.Bits.BPC_PDC_GCF_L00, cam_bnr_pdc_gain_r0.Bits.BPC_PDC_GCF_R00);
    }
*/
    return MTRUE;
}

} // namespace NSIspTuningv3
