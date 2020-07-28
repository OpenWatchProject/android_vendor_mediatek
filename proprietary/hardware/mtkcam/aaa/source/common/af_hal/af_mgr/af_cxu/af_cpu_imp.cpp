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
#define LOG_TAG "af_cpu_v3"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <mtkcam/utils/std/Log.h>
#include <stdio.h>
#include <string.h>
//#include <faces.h>
#include <private/aaa_hal_private.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
#include <drv/isp_reg.h>
#include <StatisticBuf.h>
//
#include <cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
//#include <isp_tuning_mgr.h>
#include <mcu_drv.h>

#include <af_feature.h>
#include <isp_config/isp_af_config.h> /* AF config */
#include "af_cpu_imp.h"
extern "C" {
#include <af_algo_if.h>
}
// common code for cxu
#include "af_cxu_flow.h"
using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAfCxU* AfCPU::getInstance(MINT32 sensorDev, MINT32 sensorIdx __unused)
{
    if (sensorDev == ESensorDev_MainSecond)
    {
        static AfCPU singletonMainSecond(sensorDev);
        return &singletonMainSecond;
    }
    if (sensorDev == ESensorDev_MainThird)
    {
        static AfCPU singletonMainThird(sensorDev);
        return &singletonMainThird;
    }
    else if (sensorDev == ESensorDev_Sub)
    {
        static AfCPU singletonSub(sensorDev);
        return &singletonSub;
    }
    else if (sensorDev == ESensorDev_SubSecond)
    {
        static AfCPU singletonSubSecond(sensorDev);
        return &singletonSubSecond;
    }
    else
    {
        static AfCPU singletonMain(sensorDev);
        return &singletonMain;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::init(MINT32 sensorDev, MINT32 sensorIdx, MUINT8* mcuName __unused)
{
    MINT32 err = 0;
    m_i4DgbLogLv = property_get_int32("vendor.debug.af_mgr.enable", 0);
    m_sensorDev = sensorDev;
    c_sensorDev = (ext_ESensorDev_T)m_sensorDev;
    m_sensorIdx = sensorIdx;
    CAM_LOGD("%s Dev=%d, sensorIdx=%d",
             __FUNCTION__,
             m_sensorDev,
             m_sensorIdx);

    ISP_AF_CONFIG_T::getInstance(m_sensorDev).resetParam();

    return err;
}


MINT32 AfCPU::config(ConfigAFInput_T &configInput, ConfigAFOutput_T &configOutput)
{
    CAM_LOGD("%s Dev(%d) +", __FUNCTION__, m_sensorDev);
    MINT32 err = 0;
    TGSize_W  = configInput.initHWConfigInput.sDefaultHWCfg.sTG_SZ.i4W;
    TGSize_H  = configInput.initHWConfigInput.sDefaultHWCfg.sTG_SZ.i4H;
    BINSize_W = configInput.initHWConfigInput.sDefaultHWCfg.sBIN_SZ.i4W;
    BINSize_H = configInput.initHWConfigInput.sDefaultHWCfg.sBIN_SZ.i4H;

    // initHwConfig by algo
    afc_initHwConfig(c_sensorDev, &configInput.initHWConfigInput, &configOutput.initHWConfigOutput);

    // First time HW setting is got after calling initAF.
    err = ConfigHWReg( configOutput);


    CAM_LOGD("%s Dev(%d) -", __FUNCTION__, m_sensorDev);

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::start(StartAFInput_T &input, StartAFOutput_T &output)
{
    CAM_LOGD("%s Dev(%d) +", __FUNCTION__, m_sensorDev);
    MINT32 err = 0;

    // initAF
    ext_ESensorDev_T c_sensorDev = (ext_ESensorDev_T)(m_sensorDev);
    afc_initAF(c_sensorDev, &input.initAlgoInput, &output.initAlgoOutput);
    // lockAlgo if required
    doAlgoCommand(input.initAlgoCommand);

    // pass mcuDrv (CPUAF only)
    if (!err)
    {
        if (input.ptrMCUDrv != NULL)
        {
            m_pMcuDrv = (MCUDrv*)input.ptrMCUDrv;
        }
        else
        {
            err = 1;
            CAM_LOGE("%s m_pMcuDrv NULL", __FUNCTION__);
        }
    }

    CAM_LOGD("%s Dev(%d) -", __FUNCTION__, m_sensorDev);

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::stop(LENS_INFO_T &LensInfo)
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;

    getLensInfo(LensInfo); // To get lensInfo before stop preview

    m_pMcuDrv = NULL;
    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::uninit()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
MINT32 AfCPU::doStt(DoSttIn_T doSttIn, DoSttOut_T& doSttOut)
{
    MINT32 err = 0;
    if (doSttIn.ptrAfStt)
    {
        MUINT32 u4BlockStatSize = sizeof(AF_STAT_T);
        StatisticBufInfo *ptrStatInfo = reinterpret_cast<StatisticBufInfo *>( doSttIn.ptrAfStt);
        doSttOut.pAfoBuf = reinterpret_cast<AF_STAT_T*>( ptrStatInfo->mVa);
        doSttOut.u4StaMagicNum = ptrStatInfo->mMagicNumber;
        doSttOut.u4ConfigHWNum = ptrStatInfo->mConfigNum;
        doSttOut.u4ConfigLatency = ptrStatInfo->mConfigLatency;
        doSttOut.u8SofTimeStamp = (ptrStatInfo->mTimeStamp) / 1000; // (unit: us) the time stamp of SOF of the current AFO
        MUINT32 u4BlockStrideOfDMABuf = ptrStatInfo->mStride / u4BlockStatSize; // (unit:Blocks)BlockNumX + 1 (1 comes from the requirement of twin mode)

        // To update af stt profile
        if ( doSttIn.i4HWBlkNumX <= 0 || doSttIn.i4HWBlkNumY <= 0 || doSttIn.i4HWBlkNumX > MAX_AF_HW_WIN_X || doSttIn.i4HWBlkNumY > MAX_AF_HW_WIN_Y)
        {
            CAM_LOGE( "#(%5d,%5d) %s HW-Setting Fail: i4HWBlkNumX = %d, i4HWBlkNumY = %d",
                      doSttIn.u4ReqMagicNum,
                      doSttOut.u4StaMagicNum,
                      __FUNCTION__,
                      doSttIn.i4HWBlkNumX, doSttIn.i4HWBlkNumY);
        }
        else if ( doSttOut.pAfoBuf == NULL)
        {
            CAM_LOGE( "#(%5d,%5d) %s AFO Buffer NULL",
                      doSttIn.u4ReqMagicNum,
                      doSttOut.u4StaMagicNum,
                      __FUNCTION__);
        }
        else if ( ptrStatInfo->mSize < doSttIn.i4HWBlkNumX * doSttIn.i4HWBlkNumY * u4BlockStatSize)
        {
            CAM_LOGE( "#(%5d,%5d) %s AFO Size Fail: mSize = %d",
                      doSttIn.u4ReqMagicNum,
                      doSttOut.u4StaMagicNum,
                      __FUNCTION__,
                      ptrStatInfo->mSize);
        }
        else
        {
            doSttOut.pAfSttProfile->u4NumBlkX = doSttIn.i4HWBlkNumX;   // unit: blocks
            doSttOut.pAfSttProfile->u4NumBlkY = doSttIn.i4HWBlkNumY;   // unit: blocks
            doSttOut.pAfSttProfile->u4NumStride = u4BlockStrideOfDMABuf; // unit: blocks
            doSttOut.pAfSttProfile->u4SizeBlkX = doSttIn.i4HWBlkSizeX;  // unit: pixels
            doSttOut.pAfSttProfile->u4SizeBlkY = doSttIn.i4HWBlkSizeY;  // unit: pixels
            doSttOut.pAfSttProfile->u4ConfigNum = doSttOut.u4ConfigHWNum;
            doSttOut.pAfSttProfile->i4AFPos = doSttIn.i4AfPos;

            CAM_LOGD_IF( LEVEL_FLOW,
                         "#(%5d,%5d) %s(%d) Latency(%d) BlkW,H(%3d,%3d blocks) Str(%3d blocks) BlkSzW,H(%4d,%4d pixels) FullSz(%d bytes)",
                         doSttIn.u4ReqMagicNum,
                         doSttOut.u4StaMagicNum,
                         __FUNCTION__,
                         doSttOut.u4ConfigHWNum,
                         doSttOut.u4ConfigLatency,
                         doSttOut.pAfSttProfile->u4NumBlkX, // Width (blocks)
                         doSttOut.pAfSttProfile->u4NumBlkY, // Height (blocks)
                         u4BlockStrideOfDMABuf, // Stride (blocks)
                         doSttOut.pAfSttProfile->u4SizeBlkX, // Width (pixels)
                         doSttOut.pAfSttProfile->u4SizeBlkY, // Height (pixels)
                         ptrStatInfo->mSize); // AFO size (bytes)
        }
    }
    return err;
}
MINT32 AfCPU::doAF(DoAFInput_T &doAfInput, DoAFOutput_T &doAfOutput, AF_STAT_T* afoBuf)
{
    MINT32 err = 0;

    if (!m_pMcuDrv)
    {
        err = 1;
        CAM_LOGE("%s m_pMcuDrv NULL", __FUNCTION__);
    }

    // Skipping handleAF if HW not ready.
    if (doAfInput.algoCommand.isSkipAf == -1)
    {
        MBOOL isHWRdy = ISP_AF_CONFIG_T::getInstance(m_sensorDev).isHWRdy(doAfInput.afInput.sStatProfile.u4ConfigNum);
        doAfInput.algoCommand.isSkipAf = isHWRdy ? 0 : 1;
    }

    // doAlgoCommand
    if (!err)
    {
        AlgoCommand_T& newCommand = doAfInput.algoCommand;
        doAlgoCommand(newCommand);
    }

    if (!err)
    {
        if (doAfInput.algoCommand.isSkipAf == 0)
        {
            // get LensInfo
            getLensInfo( doAfInput.afInput.sLensInfo);

            // parse afo by bank
            afc_setupAFO(c_sensorDev, &doAfInput.afInput);
            MINT32 afoBlkSzByte = doAfOutput.afOutput.sAFStatConfig.u4AfoBlkSzByte;     // bytes/block
            MINT32 bankSize = doAfInput.afInput.sStatProfile.u4NumBlkX * afoBlkSzByte;  // bytes/bank = bytes/block * block/bank
            MUINT32 afoStride = doAfInput.afInput.sStatProfile.u4NumStride;             // blocks
            CAM_LOGD_IF(LEVEL_VERBOSE, "%s: afoBlkSzByte %d, bankSize %d, afoStride %d", __FUNCTION__, afoBlkSzByte, bankSize, afoStride);
            for (int i = 0; i < doAfInput.afInput.sStatProfile.u4NumBlkY; i++)
            {
                AF_STAT_T* ptr = &afoBuf[i * afoStride];
                afc_doBankAFO(c_sensorDev, (void*)ptr, i, bankSize);
            }
            // doAlgo
            afc_handleAF(c_sensorDev, &doAfInput.afInput, &doAfOutput.afOutput);
        }

        // MoveLens
        motorDisable = doAfInput.afInput.dProperty.motor_disable;
        motorMFPos = doAfInput.afInput.dProperty.motor_position;
        m_u8MvLensTS = MoveLensTo(doAfOutput.afOutput.i4AFPos);
        doAfOutput.mvLensTS = m_u8MvLensTS;

        // To update config
        memcpy(&doAfOutput.ispOutput.initHWConfigOutput.sInitHWCfg, &doAfOutput.afOutput.sAFStatConfig, sizeof(AF_CONFIG_T));
        ConfigHWReg(doAfOutput.ispOutput);
    }

    return err;
}
MINT32 AfCPU::updateNVRAM(StartAFInput_T &input, StartAFOutput_T &output)
{
    CAM_LOGD("%s Dev(%d) +", __FUNCTION__, m_sensorDev);
    MINT32 err = 0;

    afc_initAF(c_sensorDev, &input.initAlgoInput, &output.initAlgoOutput);
    // cancel algo if required
    doAlgoCommand(input.initAlgoCommand);

    CAM_LOGD("%s Dev(%d) -", __FUNCTION__, m_sensorDev);

    return err;
}
MINT32 AfCPU::getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s ", __FUNCTION__);
    MINT32 err = 0;
    afc_getDebugInfo(c_sensorDev, &rAFDebugInfo);
    return err;
}
MRESULT AfCPU::getPdBlocks(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T& pdBlockOutput)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s ", __FUNCTION__);

    MRESULT err;
    err = afc_getPdBlocks(c_sensorDev, pdBlockInut.inAfRoi, pdBlockInut.inAfRoiNum, pdBlockInut.inWinBufSz, pdBlockInut.inFdInfo, pdBlockOutput.outWins, &(pdBlockOutput.outWinNum));
    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::resume()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
MINT32 AfCPU::suspend()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
MINT32 AfCPU::abort()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
/////===== Private =====/////
MINT32 AfCPU::ConfigHWReg(ConfigAFOutput_T &ConfigOutput)
{
    MINT32 err = 0;

    if (ConfigOutput.initHWConfigOutput.sInitHWCfg.u4IsConfigChanged)
    {
        AF_CONFIG_T &sInHWCfg = ConfigOutput.initHWConfigOutput.sInitHWCfg;
        if (BINSize_W != 0 && TGSize_W != 0)
        {
            sInHWCfg.sTG_SZ.i4W  = TGSize_W;
            sInHWCfg.sTG_SZ.i4H  = TGSize_H;
            sInHWCfg.sBIN_SZ.i4W = BINSize_W;
            sInHWCfg.sBIN_SZ.i4H = BINSize_H;
        }
        else
        {
            CAM_LOGE("%s Dev(%d) TG/BIN size is wrong!!!", __FUNCTION__, m_sensorDev);
        }
        ISP_AF_CONFIG_T::getInstance(m_sensorDev).AFConfig(&sInHWCfg, &ConfigOutput.hwConfigInfo, ConfigOutput.resultConfig);

        // need to applied by isp_mgr
        ConfigOutput.resultConfig->isApplied  = 1;
    }
    else
    {
        ConfigOutput.resultConfig->isApplied = 0;
    }

    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::doAlgoCommand(AlgoCommand_T newCommand)
{
    MINT32 err = 0;

    // update requestNum first
    m_sCommand.requestNum = newCommand.requestNum;
    // print log if AlgoCommand change
    if (memcmp(&newCommand,  &m_sCommand, sizeof(AlgoCommand_T)) != 0 || LEVEL_FLOW)
    {
        CAM_LOGD("doAlgoCommand: requestNum = %4d, afLibMode = %2d, mfPos = %4d, eLockAlgo = %1d, bTrigger = %1d, bCancel = %1d, bTargetAssistMove = %1d, isSkipAf = %1d / ",
                 newCommand.requestNum,
                 newCommand.afLibMode,
                 newCommand.mfPos,
                 newCommand.eLockAlgo,
                 newCommand.bTrigger,
                 newCommand.bCancel,
                 newCommand.bTargetAssistMove,
                 newCommand.isSkipAf);
    }
    // doAlgoCommand
    CxUFlow_doAlgoCommand(c_sensorDev, newCommand, &m_sCommand);

    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfCPU::getTimeStamp_us()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp = ((t.tv_sec) * 1000000000LL + t.tv_nsec) / 1000;
    return timestamp; // from nano to us
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfCPU::MoveLensTo( MINT32 targetPos)
{
    MUINT64 TS_BeforeMoveMCU = 0; // timestamp of before moveMCU
    MUINT64 TS_AfterMoveMCU = 0;  // timestamp of after moveMCU

    if ( m_pMcuDrv)
    {
        if ( motorDisable == 1)
        {
            if ( motorMFPos < 1024 && motorMFPos != motorMFPosPre)
            {
                motorMFPosPre = motorMFPos;
                m_pMcuDrv->moveMCU( motorMFPos);
            }
        }
        else if ( targetPosPre != targetPos)
        {
            CAM_LOGD("#(%5d) %s Dev(%d) DAC(%d)", m_sCommand.requestNum, __FUNCTION__, m_sensorDev, targetPos);
            TS_BeforeMoveMCU = getTimeStamp_us();
            if (m_pMcuDrv->moveMCU( targetPos))
            {
                targetPosPre = targetPos;
            }
            TS_AfterMoveMCU = getTimeStamp_us();
        }
    }
    else
    {
        CAM_LOGE( "%s Fail, Dev %d", __FUNCTION__, m_sensorDev);
    }

    if (TS_BeforeMoveMCU != 0)
    {
        // update time stamp of moveMCU
        return (MUINT64)((TS_BeforeMoveMCU + TS_AfterMoveMCU) / 2);
    }
    else
    {
        // inherit the original one
        return m_u8MvLensTS;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfCPU::getLensInfo( LENS_INFO_T &a_rLensInfo)
{
    MRESULT ret = E_3A_ERR;
    mcuMotorInfo rMotorInfo;

    if ( m_pMcuDrv)
    {
        ret = m_pMcuDrv->getMCUInfo(&rMotorInfo);

        if ( a_rLensInfo.i4CurrentPos != (MINT32)rMotorInfo.u4CurrentPosition)
        {
            CAM_LOGD_IF( LEVEL_VERBOSE,
                         "%s Dev %d, curPos %d, ",
                         __FUNCTION__,
                         m_sensorDev,
                         (MINT32)rMotorInfo.u4CurrentPosition);
        }

        a_rLensInfo.i4CurrentPos   = (MINT32)rMotorInfo.u4CurrentPosition;
        a_rLensInfo.bIsMotorOpen   =         rMotorInfo.bIsMotorOpen;
        a_rLensInfo.bIsMotorMoving =         rMotorInfo.bIsMotorMoving;
        a_rLensInfo.i4InfPos       = (MINT32)rMotorInfo.u4InfPosition;
        a_rLensInfo.i4MacroPos     = (MINT32)rMotorInfo.u4MacroPosition;
        a_rLensInfo.bIsSupportSR   =         rMotorInfo.bIsSupportSR;

        CAM_LOGD_IF( LEVEL_VERBOSE,
                     "%s Dev %d, %d, %d, %d, %d, %d, %d",
                     __FUNCTION__,
                     m_sensorDev,
                     a_rLensInfo.i4CurrentPos,                     a_rLensInfo.bIsMotorOpen,
                     a_rLensInfo.bIsMotorMoving,
                     a_rLensInfo.i4InfPos,
                     a_rLensInfo.i4MacroPos,
                     a_rLensInfo.bIsSupportSR);

        ret = S_AF_OK;
    }
    else
    {
        CAM_LOGD_IF( LEVEL_FLOW,
                     "%s Fail, Dev %d",
                     __FUNCTION__,
                     m_sensorDev);
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCPU::SyncAF_setSyncMode(MINT32 syncMode)
{
    MINT32 err = 0;
    afc_SetSyncMode(c_sensorDev, syncMode);
    return err;
}
MINT32 AfCPU::SyncAF_motorRange(AF_SyncInfo_T& sCamInfo)
{
    MINT32 err = 0;
    afc_MotorRange(c_sensorDev, &sCamInfo);
    return err;
}
MINT32 AfCPU::SyncAF_getSyncInfo(AF_SyncInfo_T& sCamInfo)
{
    MINT32 ret = 0;
    ret = afc_GetSyncInfo(c_sensorDev, &sCamInfo);
    return ret;
}
MINT32 AfCPU::SyncAF_setSyncInfo(MINT32 pos, AF_SyncInfo_T& sCamInfo)
{
    MINT32 err = 0;
    afc_SetSyncInfo(c_sensorDev, pos, &sCamInfo);
    return err;
}
MINT32 AfCPU::SyncAF_getMain2CalibPos(AF_SyncInfo_T& sCamInfo)
{
    MINT32 err = 0;
    afc_GetMain2CalibPos(c_sensorDev, &sCamInfo);
    return err;
}
MINT32 AfCPU::SyncAF_calibPos(AF_SyncInfo_T& sCamInfo)
{
    MINT32 err = 0;
    afc_CalibAFPos(c_sensorDev, &sCamInfo);
    return err;
}
MUINT64 AfCPU::SyncAF_moveLensSyncAF(MINT32 targetPos)
{
    MINT32 err = 0;
    MoveLensTo(targetPos);
    return err;
}
