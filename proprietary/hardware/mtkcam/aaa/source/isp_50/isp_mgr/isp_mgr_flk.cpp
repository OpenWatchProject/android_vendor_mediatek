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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_mgr_flk"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <drv/tuning_mgr.h>

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>

#include "isp_mgr.h"
#include <drv/isp_reg.h>
#include <mtkcam/drv/IHalSensor.h>

/* Dynamic Bin */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_FLK_CONFIG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Flicker
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_FLK_CONFIG_T&
ISP_MGR_FLK_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_FLK_CONFIG_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

MBOOL
ISP_MGR_FLK_CONFIG_T::
config(FLKWinCFG_T& rFlkWinCfg)
{
    CAM_REG_FLK_A_OFST reg_ofst;
    reg_ofst.Bits.FLK_OFST_X = rFlkWinCfg.m_u4OffsetX;
    reg_ofst.Bits.FLK_OFST_Y = rFlkWinCfg.m_u4OffsetY;

    CAM_REG_FLK_A_SIZE reg_size;
    reg_size.Bits.FLK_SIZE_X = rFlkWinCfg.m_u4SizeX;
    reg_size.Bits.FLK_SIZE_Y = rFlkWinCfg.m_u4SizeY;

    CAM_REG_FLK_A_NUM reg_num;
    reg_num.Bits.FLK_NUM_X = rFlkWinCfg.m_u4NumX;
    reg_num.Bits.FLK_NUM_Y = rFlkWinCfg.m_u4NumY;

    CAM_REG_FLKO_XSIZE reg_xsize;
    reg_xsize.Raw = rFlkWinCfg.m_u4DmaSize;

    CAM_REG_FLKO_YSIZE reg_ysize;
    reg_ysize.Raw = 0x0001;

    CAM_REG_FLKO_STRIDE reg_stride;
    reg_stride.Raw = rFlkWinCfg.m_u4DmaSize;

    CAM_REG_SGG3_PGN reg_pgn;
    reg_pgn.Bits.SGG_GAIN = rFlkWinCfg.m_u4SGG3_PGN;

    CAM_REG_SGG3_GMRC_1 reg_gmrc1;
    reg_gmrc1.Bits.SGG_GMR_1 = rFlkWinCfg.m_u4SGG3_GMR1;
    reg_gmrc1.Bits.SGG_GMR_2 = rFlkWinCfg.m_u4SGG3_GMR2;
    reg_gmrc1.Bits.SGG_GMR_3 = rFlkWinCfg.m_u4SGG3_GMR3;
    reg_gmrc1.Bits.SGG_GMR_4 = rFlkWinCfg.m_u4SGG3_GMR4;

    CAM_REG_SGG3_GMRC_2 reg_gmrc2;
    reg_gmrc2.Bits.SGG_GMR_5 = rFlkWinCfg.m_u4SGG3_GMR5;
    reg_gmrc2.Bits.SGG_GMR_6 = rFlkWinCfg.m_u4SGG3_GMR6;
    reg_gmrc2.Bits.SGG_GMR_7 = rFlkWinCfg.m_u4SGG3_GMR7;

    REG_INFO_VALUE(CAM_FLK_A_OFST) = (MUINT32)reg_ofst.Raw;
    REG_INFO_VALUE(CAM_FLK_A_SIZE) = (MUINT32)reg_size.Raw;
    REG_INFO_VALUE(CAM_FLK_A_NUM) = (MUINT32)reg_num.Raw;
    REG_INFO_VALUE(CAM_FLKO_XSIZE) = (MUINT32)reg_xsize.Raw;
    REG_INFO_VALUE(CAM_FLKO_YSIZE) = (MUINT32)reg_ysize.Raw;
    REG_INFO_VALUE(CAM_FLKO_STRIDE) = (MUINT32)reg_stride.Raw;
    REG_INFO_VALUE(CAM_SGG3_PGN) = (MUINT32)reg_pgn.Raw;
    REG_INFO_VALUE(CAM_SGG3_GMRC_1) = (MUINT32)reg_gmrc1.Raw;
    REG_INFO_VALUE(CAM_SGG3_GMRC_2) = (MUINT32)reg_gmrc2.Raw;

    return MTRUE;
}

MBOOL
ISP_MGR_FLK_CONFIG_T::
reconfig(FLKWinCFG_T &rFlkWinCfg, MVOID *pOutRegCfg)
{
    MUINT32 u4Offset = 0;
    MUINT32* pIspReg = NULL;

    Tuning_CFG *psOutRegCfg = static_cast<Tuning_CFG*>(pOutRegCfg);
    psOutRegCfg->Bypass.Bits.FLK = 0;

    pIspReg = static_cast<MUINT32*>(psOutRegCfg->pIspReg);

    // CAM_FLK_A_SIZE
    CAM_REG_FLK_A_SIZE reg_size;
    u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_FLK_A_SIZE))/4;
    reg_size.Raw = *(pIspReg+u4Offset);
    CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_FLK_A_SIZE : %x +", __FUNCTION__, *(pIspReg+u4Offset));
    reg_size.Bits.FLK_SIZE_X = rFlkWinCfg.m_u4SizeX;
    reg_size.Bits.FLK_SIZE_Y = rFlkWinCfg.m_u4SizeY;
    *(pIspReg+u4Offset) = reg_size.Raw;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_FLK_A_SIZE : %x -", __FUNCTION__, *(pIspReg+u4Offset));

    // CAM_FLKO_XSIZE
    CAM_REG_FLKO_XSIZE reg_xsize;
    u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_FLKO_XSIZE))/4;
    reg_xsize.Raw = *(pIspReg+u4Offset);
    CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_FLKO_XSIZE : %x +", __FUNCTION__, *(pIspReg+u4Offset));
    reg_xsize.Raw = rFlkWinCfg.m_u4DmaSize;
    *(pIspReg+u4Offset) = reg_xsize.Raw;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_FLKO_XSIZE : %x -", __FUNCTION__, *(pIspReg+u4Offset));

    // CAM_FLKO_STRIDE
    CAM_REG_FLKO_STRIDE reg_stride;
    u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_FLKO_STRIDE))/4;
    reg_stride.Raw = *(pIspReg+u4Offset);
    CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_FLKO_STRIDE : %x +", __FUNCTION__, *(pIspReg+u4Offset));
    reg_stride.Raw = rFlkWinCfg.m_u4DmaSize;
    *(pIspReg+u4Offset) = reg_stride.Raw;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_FLKO_STRIDE : %x -", __FUNCTION__, *(pIspReg+u4Offset));

    return MTRUE;
}


MBOOL
ISP_MGR_FLK_CONFIG_T::
apply(TuningMgr& rTuning, MBOOL const flkEnable, MINT32 i4SubsampleIdex)
{
    if (!flkEnable) {
        CAM_LOGD("apply(): no flicker info, tuning apply bypass FLK");
        return MTRUE;
    }

    rTuning.updateEngine(eTuningMgrFunc_FLK, MTRUE, i4SubsampleIdex);
    rTuning.updateEngine(eTuningMgrFunc_SGG3, MTRUE, i4SubsampleIdex);
    rTuning.tuningMgrWriteRegs(
            static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("flk_cfg");

    return MTRUE;
}

}
