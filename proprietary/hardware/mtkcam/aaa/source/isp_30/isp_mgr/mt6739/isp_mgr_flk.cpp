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
#include "log_utils.h"
#include <iopipe/CamIO/INormalPipe.h>

Mutex m_FLKOLockReg;

using namespace NSCam::NSIoPipeIsp3::NSCamIOPipe;

namespace NSIspTuningv3
{

#define IOPIPE_SET_MODUL_ENABLE(i4SensorIndex, module, enable) \
        { \
            if (MTRUE != sendFLKNormalPipe(i4SensorIndex, NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, module, enable, MNULL)) \
            { \
                MY_ERR("EPIPECmd_SET_MODULE_EN ConfigHWReg fail"); \
            } \
        }
#define IOPIPE_GET_MODUL_HANDLE(i4SensorIndex, module, handle) \
        { \
            if (MTRUE != sendFLKNormalPipe(i4SensorIndex, NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, module, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_flk")))) \
            { \
                MY_ERR("EPIPECmd_GET_MODULE_HANDLE ConfigHWReg fail"); \
            } \
        }
#define IOPIPE_SET_MODUL_CFG_DONE(i4SensorIndex, handle) \
        { \
            if (MTRUE != sendFLKNormalPipe(i4SensorIndex, NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) \
            { \
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE ConfigHWReg fail"); \
            } \
        }
#define IOPIPE_RELEASE_MODUL_HANDLE(i4SensorIndex, handle) \
        { \
            if (MTRUE != sendFLKNormalPipe(i4SensorIndex, NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_flk")), MNULL)) \
            { \
                MY_ERR("EPIPECmd_RELEASE_MODULE_HANDLE ConfigHWReg fail"); \
            } \
            handle = NULL; \
        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Flicker
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_FLK_CONFIG_T&
ISP_MGR_FLK_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Main Second Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_FLK_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_FLK_CONFIG_T::
config(MINT32 i4SensorIndex, FLKWinCFG_T& rFlkWinCfg)
{
    logI("[%s] ln=%d nx(%d), ny(%d), sx(%d), sy(%d), ox(%d), oy(%d) ,ds(%d)", __FUNCTION__, __LINE__, rFlkWinCfg.m_u4NumX, rFlkWinCfg.m_u4NumY,
         rFlkWinCfg.m_u4SizeX, rFlkWinCfg.m_u4SizeY, rFlkWinCfg.m_u4OffsetX, rFlkWinCfg.m_u4OffsetY, rFlkWinCfg.m_u4DmaSize);

    // SGG3 config
    CAM_REG_SGG3_PGN reg_sgg3_pgn;
    reg_sgg3_pgn.Raw = 0;
    reg_sgg3_pgn.Bits.SGG3_GAIN = 16;

    CAM_REG_SGG3_GMRC_1 reg_sgg3_gmrc_1;
    reg_sgg3_gmrc_1.Raw = 0;
    reg_sgg3_gmrc_1.Bits.SGG3_GMR_1 = 20;
    reg_sgg3_gmrc_1.Bits.SGG3_GMR_2 = 29;
    reg_sgg3_gmrc_1.Bits.SGG3_GMR_3 = 42;
    reg_sgg3_gmrc_1.Bits.SGG3_GMR_4 = 62;

    CAM_REG_SGG3_GMRC_2 reg_sgg3_gmrc_2;
    reg_sgg3_gmrc_2.Raw = 0;
    reg_sgg3_gmrc_2.Bits.SGG3_GMR_5 = 88;
    reg_sgg3_gmrc_2.Bits.SGG3_GMR_6 = 126;
    reg_sgg3_gmrc_2.Bits.SGG3_GMR_7 = 180;

    // FLK config
    CAM_REG_FLK_NUM reg_num;
    reg_num.Raw = 0;
    reg_num.Bits.FLK_NUM_X = rFlkWinCfg.m_u4NumX;
    reg_num.Bits.FLK_NUM_Y = rFlkWinCfg.m_u4NumY;

    CAM_REG_FLK_SIZE reg_size;
    reg_size.Raw = 0;
    reg_size.Bits.FLK_SIZE_X = rFlkWinCfg.m_u4SizeX;
    reg_size.Bits.FLK_SIZE_Y = rFlkWinCfg.m_u4SizeY;

    CAM_REG_FLK_OFST reg_ofst;
    reg_ofst.Raw = 0;
    reg_ofst.Bits.FLK_OFST_X = rFlkWinCfg.m_u4OffsetX;
    reg_ofst.Bits.FLK_OFST_Y = rFlkWinCfg.m_u4OffsetY;

    // ESFKO config
    CAM_REG_ESFKO_XSIZE reg_xsize;
    reg_xsize.Raw = rFlkWinCfg.m_u4DmaSize;

    CAM_REG_ESFKO_YSIZE reg_ysize;
    reg_ysize.Raw = 0;

    CAM_REG_ESFKO_STRIDE reg_stride;
    reg_stride.Raw = rFlkWinCfg.m_u4DmaSize;

    REG_INFO_VALUE(CAM_SGG3_PGN)     =  (MUINT32)reg_sgg3_pgn.Raw;
    REG_INFO_VALUE(CAM_SGG3_GMRC_1)  =  (MUINT32)reg_sgg3_gmrc_1.Raw;
    REG_INFO_VALUE(CAM_SGG3_GMRC_2)  =  (MUINT32)reg_sgg3_gmrc_2.Raw;
    REG_INFO_VALUE(CAM_FLK_NUM)      =  (MUINT32)reg_num.Raw;
    REG_INFO_VALUE(CAM_FLK_SIZE)     =  (MUINT32)reg_size.Raw;
    REG_INFO_VALUE(CAM_FLK_OFST)     =  (MUINT32)reg_ofst.Raw;
    REG_INFO_VALUE(CAM_ESFKO_XSIZE)  =  (MUINT32)reg_xsize.Raw;
    REG_INFO_VALUE(CAM_ESFKO_YSIZE)  =  (MUINT32)reg_ysize.Raw;
    REG_INFO_VALUE(CAM_ESFKO_STRIDE) =  (MUINT32)reg_stride.Raw;

    MINTPTR handle;

    Mutex::Autolock lock(m_FLKOLockReg);

    // FLK enable
    logI("[%s] ln=%d IOPIPE_SET_MODUL_ENABLE (EModule_FLK)", __FUNCTION__, __LINE__);
    IOPIPE_SET_MODUL_ENABLE(i4SensorIndex, NSImageioIsp3::NSIspio::EModule_FLK, MTRUE);

    // SGG3 enable
    logI("[%s] ln=%d IOPIPE_SET_MODUL_ENABLE (EModule_SGG3)", __FUNCTION__, __LINE__);
    IOPIPE_SET_MODUL_ENABLE(i4SensorIndex, NSImageioIsp3::NSIspio::EModule_SGG3, MTRUE);
    
    // Set SGG3 register
    logI("[%s] ln=%d IOPIPE_GET_MODUL_HANDLE (EModule_SGG3)", __FUNCTION__, __LINE__);
    IOPIPE_GET_MODUL_HANDLE(i4SensorIndex, NSImageioIsp3::NSIspio::EModule_SGG3, handle);
    if ((MINTPTR)NULL != handle)
    {
        IOPIPE_SET_MODUL_REG(handle,  CAM_SGG3_PGN,     REG_INFO_VALUE(CAM_SGG3_PGN));
        IOPIPE_SET_MODUL_REG(handle,  CAM_SGG3_GMRC_1,  REG_INFO_VALUE(CAM_SGG3_GMRC_1));
        IOPIPE_SET_MODUL_REG(handle,  CAM_SGG3_GMRC_2,  REG_INFO_VALUE(CAM_SGG3_GMRC_2));
        logI("[%s] ln=%d IOPIPE_SET_MODUL_CFG_DONE", __FUNCTION__, __LINE__);
        IOPIPE_SET_MODUL_CFG_DONE(i4SensorIndex, handle);
        logI("[%s] ln=%d IOPIPE_RELEASE_MODUL_HANDLE", __FUNCTION__, __LINE__);
        IOPIPE_RELEASE_MODUL_HANDLE(i4SensorIndex, handle);
    }

    // Set FLK register
    logI("[%s] ln=%d IOPIPE_GET_MODUL_HANDLE (EModule_FLK)", __FUNCTION__, __LINE__);
    IOPIPE_GET_MODUL_HANDLE(i4SensorIndex, NSImageioIsp3::NSIspio::EModule_FLK, handle);
    if ((MINTPTR)NULL != handle)
    {
        IOPIPE_SET_MODUL_REG(handle,  CAM_FLK_NUM,      REG_INFO_VALUE(CAM_FLK_NUM));
        IOPIPE_SET_MODUL_REG(handle,  CAM_FLK_SIZE,     REG_INFO_VALUE(CAM_FLK_SIZE));
        IOPIPE_SET_MODUL_REG(handle,  CAM_FLK_OFST,     REG_INFO_VALUE(CAM_FLK_OFST));
        logI("[%s] ln=%d IOPIPE_SET_MODUL_CFG_DONE", __FUNCTION__, __LINE__);
        IOPIPE_SET_MODUL_CFG_DONE(i4SensorIndex, handle);
        logI("[%s] ln=%d IOPIPE_RELEASE_MODUL_HANDLE", __FUNCTION__, __LINE__);
        IOPIPE_RELEASE_MODUL_HANDLE(i4SensorIndex, handle);
    }

    // Set ESFKO register
    logI("[%s] ln=%d IOPIPE_GET_MODUL_HANDLE (EModule_ESFKO)", __FUNCTION__, __LINE__);
    IOPIPE_GET_MODUL_HANDLE(i4SensorIndex, NSImageioIsp3::NSIspio::EModule_ESFKO, handle);
    if ((MINTPTR)NULL != handle)
    {
        IOPIPE_SET_MODUL_REG(handle,  CAM_ESFKO_XSIZE,  REG_INFO_VALUE(CAM_ESFKO_XSIZE));
        IOPIPE_SET_MODUL_REG(handle,  CAM_ESFKO_YSIZE,  REG_INFO_VALUE(CAM_ESFKO_YSIZE));
        IOPIPE_SET_MODUL_REG(handle,  CAM_ESFKO_STRIDE, REG_INFO_VALUE(CAM_ESFKO_STRIDE));
        logI("[%s] ln=%d IOPIPE_SET_MODUL_CFG_DONE", __FUNCTION__, __LINE__);
        IOPIPE_SET_MODUL_CFG_DONE(i4SensorIndex, handle);
        logI("[%s] ln=%d IOPIPE_RELEASE_MODUL_HANDLE", __FUNCTION__, __LINE__);
        IOPIPE_RELEASE_MODUL_HANDLE(i4SensorIndex, handle);
    }

    logI("[%s] ln=%d config done", __FUNCTION__, __LINE__);
    
    return MTRUE;
}

MBOOL
ISP_MGR_FLK_CONFIG_T::
apply()
{
    return MTRUE;
}

MBOOL
ISP_MGR_FLK_CONFIG_T::
apply(TuningMgr& rTuning, MBOOL const flkEnable, const ESensorTG_T sensorTG, MINT32 i4SubsampleIdex)
{
// FIX-ME, for IP-Base build pass +
#if 0
    if (!flkEnable){
        CAM_LOGD("Tuning apply bypass FLK");
        return MTRUE;
    }
    rTuning.updateEngine(eTuningMgrFunc_FLK, MTRUE, i4SubsampleIdex);
    rTuning.updateEngine(eTuningMgrFunc_SGG3, MTRUE, i4SubsampleIdex);

    // Register setting
    int regInfoNum = m_u4RegInfoNum / 2;
    switch (sensorTG) {
    case ESensorTG_1:
        rTuning.tuningMgrWriteRegs_Uni(
                static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                regInfoNum, i4SubsampleIdex);
        break;
    case ESensorTG_2:
        rTuning.tuningMgrWriteRegs_Uni(
                static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo) + regInfoNum,
                regInfoNum, i4SubsampleIdex);
        break;
    default:
        CAM_LOGE("Invalid tg = %d", sensorTG);
        return MFALSE;
    }
    dumpRegInfoP1("flk_cfg");
#endif
// FIX-ME, for IP-Base build pass -
    return MTRUE;
}

MBOOL
ISP_MGR_FLK_CONFIG_T::
sendFLKNormalPipe(MINT32 i4SensorIndex, MINT32 cmd, MUINTPTR arg1, MUINTPTR arg2, MUINTPTR arg3)
{
    MBOOL ret = MFALSE;

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(i4SensorIndex, LOG_TAG);
    if(pPipe==NULL)
    {
        MY_ERR( "Fail to create NormalPipe");
    }
    else
    {
        ret = pPipe->sendCommand( cmd, arg1, arg2, arg3);
        pPipe->destroyInstance( LOG_TAG);
    }

    return ret;
}


}
