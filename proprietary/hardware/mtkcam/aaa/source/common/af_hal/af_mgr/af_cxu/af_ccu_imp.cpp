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
#define LOG_TAG "af_ccu_v3"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <mtkcam/utils/std/Log.h>
#include <private/aaa_hal_private.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
#include <drv/isp_reg.h>
#include <cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <mcu_drv.h>
#include <af_feature.h>
#include <isp_config/isp_af_config.h> /* AF config */
#include "af_ccu_imp.h"
extern "C" {
#include <af_algo_if.h>
}
#include "af_cxu_param.h"

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCcuIf;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAfCxU* AfCCU::getInstance(MINT32 sensorDev, MINT32 sensorIdx)
{
    uint8_t const UsrName[32] = "af_ccu_imp";

    MINT32 isCCUInitStatus = 1;
    ICcuCtrlAf *ptr = ICcuCtrlAf::createInstance(UsrName, (ESensorDev_T)sensorDev);
    if (ptr)
    {
        isCCUInitStatus = ptr->init(sensorIdx, (ESensorDev_T)sensorDev);
    }
    if (isCCUInitStatus == CCU_CTRL_SUCCEED)
    {
        CAM_LOGD("AfCCU createInstance pass");
        if (sensorDev == ESensorDev_MainSecond)
        {
            static AfCCU singletonMainSecond(sensorDev);
            return &singletonMainSecond;
        }
        if (sensorDev == ESensorDev_MainThird)
        {
            static AfCCU singletonMainThird(sensorDev);
            return &singletonMainThird;
        }
        else if (sensorDev == ESensorDev_Sub)
        {
            static AfCCU singletonSub(sensorDev);
            return &singletonSub;
        }
        else if (sensorDev == ESensorDev_SubSecond)
        {
            static AfCCU singletonSubSecond(sensorDev);
            return &singletonSubSecond;
        }
        else
        {
            static AfCCU singletonMain(sensorDev);
            return &singletonMain;
        }
    }
    else
    {
        CAM_LOGW("AfCCU createInstance fail");
        return (IAfCxU*)(NULL);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCCU::init(MINT32 sensorDev, MINT32 sensorIdx, MUINT8* mcuName)
{
    MINT32 err = 0;
    m_i4DgbLogLv = property_get_int32("vendor.debug.af_mgr.enable", 0);
    m_sensorDev = sensorDev;
    m_sensorIdx = sensorIdx;
    m_i4isMotorOpen = 0;

    memcpy(m_mcuName, mcuName, sizeof(char) * 32);
    CAM_LOGD("%s Dev=%d, sensorIdx=%d, mcuName=%s",
             __FUNCTION__,
             m_sensorDev,
             m_sensorIdx,
             m_mcuName);

    // ccu init flow
    m_ptrCcu = ICcuCtrlAf::createInstance(szUsrName, (ESensorDev_T)sensorDev);
    if (m_ptrCcu)
    {
        err = m_ptrCcu->init(m_sensorIdx, (ESensorDev_T)m_sensorDev); // CCUAF init
        m_ptrCcu->ccu_af_control_initialize(m_mcuName); // CCUAF init
    }
    return err;
}


MINT32 AfCCU::config(ConfigAFInput_T &configInput, ConfigAFOutput_T &configOutput)
{
    CAM_LOGD("%s Dev(%d) +", __FUNCTION__, m_sensorDev);
    MINT32 err = 0;
    TGSize_W  = configInput.initHWConfigInput.sDefaultHWCfg.sTG_SZ.i4W;
    TGSize_H  = configInput.initHWConfigInput.sDefaultHWCfg.sTG_SZ.i4H;
    BINSize_W = configInput.initHWConfigInput.sDefaultHWCfg.sBIN_SZ.i4W;
    BINSize_H = configInput.initHWConfigInput.sDefaultHWCfg.sBIN_SZ.i4H;

    // afc_initHwConfig + Setting isp
    if (m_ptrCcu)
    {
        err = m_ptrCcu->ccu_af_init_hwconfig(&configInput, &configOutput.initHWConfigOutput); // todo
        CAM_LOGD("%s Dev(%d) sTG_SZ(%d, %d), AF_BLK_NUM(%d, %d)",
                 __FUNCTION__,
                 m_sensorDev,
                 configOutput.initHWConfigOutput.sInitHWCfg.sTG_SZ.i4W,
                 configOutput.initHWConfigOutput.sInitHWCfg.sTG_SZ.i4H,
                 configOutput.initHWConfigOutput.sInitHWCfg.sConfigReg.AF_BLK_XNUM,
                 configOutput.initHWConfigOutput.sInitHWCfg.sConfigReg.AF_BLK_YNUM);
    }
    else
    {
        err = 1;
    }
    CAM_LOGD("%s Dev(%d) -", __FUNCTION__, m_sensorDev);
    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCCU::start(StartAFInput_T &input, StartAFOutput_T &output)
{
    CAM_LOGD("%s Dev(%d) +", __FUNCTION__, m_sensorDev);
    MINT32 err = 0;

    CCUStartAFInput_t ccuInitAFInput;
    memcpy(&ccuInitAFInput.initAlgoInput, &input.initAlgoInput, sizeof(AF_INIT_INPUT_T));
    memcpy(&ccuInitAFInput.initAlgoCommand, &input.initAlgoCommand, sizeof(AlgoCommand_T));

    m_pMcuDrv = (MCUDrv*)(input.ptrMCUDrv);

    motorInfPos = input.initAlgoInput.otpInfPos;
    motorMacroPos = input.initAlgoInput.otpMacroPos;

    if (m_ptrCcu)
    {
        // afc_initAF + doAlgoCommand
        err = m_ptrCcu->ccu_af_start_and_init_algo(&ccuInitAFInput, &output);
        CAM_LOGD("%s Dev(%d) posturedInitPos(%d), otpAfTableStr(%d), otpAfTableEnd(%d)",
                 __FUNCTION__,
                 m_sensorDev,
                 output.initAlgoOutput.posturedInitPos,
                 output.initAlgoOutput.otpAfTableStr,
                 output.initAlgoOutput.otpAfTableEnd);
    }
    else
    {
        err = 1;
        CAM_LOGD("%s ptrCcu NULL", __FUNCTION__);
    }

    m_i4PostureCompInitPos = output.initAlgoOutput.posturedInitPos;

    CAM_LOGD("%s Dev(%d) -", __FUNCTION__, m_sensorDev);

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCCU::stop(LENS_INFO_T &LensInfo __unused)
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    if (m_ptrCcu)
    {
        err = m_ptrCcu->ccu_af_stop();
        err = m_ptrCcu->uninit(); // CCUAF init
    }
    else
    {
        err = 1;
    }
    m_pMcuDrv = NULL;
    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCCU::uninit()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
MINT32 AfCCU::doStt(DoSttIn_T doSttIn, DoSttOut_T& doSttOut)
{
    MINT32 err = 0;
    doSttOut.u4StaMagicNum = (doSttIn.u4ReqMagicNum > 2) ? (doSttIn.u4ReqMagicNum - 2) : 0;
    return err;
}
MINT32 AfCCU::doAF(DoAFInput_T &doAfInput, DoAFOutput_T &doAfOutput __unused, AF_STAT_T* afoBuf __unused)
{
    MINT32 err = 0;
    if (m_i4isMotorOpen == 0)
    {
        MINT32 initPos = 0;
        isMotorOpen(m_i4isMotorOpen, initPos);
        if (m_i4isMotorOpen != 0)
        {
            CAM_LOGD("afccu : MotorOpen => call ccu_af_start to inform CCU initPos = %d", initPos);
            m_ptrCcu->ccu_af_start(initPos); // inform ccu to move lens to posturedInitPos
        }
        else
        {
            CAM_LOGD("afccu : skip ccu_af_doAF before ccu_af_start is called");
            return err;
        }
    }

    if (m_ptrCcu)
    {
        doAfInput.afInput.sLensInfo.i4InfPos   = 0;     //motorInfPos;
        doAfInput.afInput.sLensInfo.i4MacroPos = 1023;  //motorMacroPos;
        m_ptrCcu->ccu_af_doAF(&doAfInput);
    }
    else
    {
        err = 1;
    }

    return err;
}

MINT32 AfCCU::updateOutput(DoAFOutput_T &afOutput)
{
    m_ptrCcu->ccu_af_update_doAFoutput(&afOutput); // todo : called by reference
    return 0;
}

MINT32 AfCCU::updateNVRAM(StartAFInput_T &input, StartAFOutput_T &output)
{
    CAM_LOGD("%s Dev(%d)", __FUNCTION__, m_sensorDev);
    MINT32 err = 0;
    if (m_ptrCcu) // afc_initAF + doAlgoCommand
    {
        CCUStartAFInput_t ccuInitAFInput;
        memcpy(&ccuInitAFInput.initAlgoInput, &input.initAlgoInput, sizeof(AF_INIT_INPUT_T));
        memcpy(&ccuInitAFInput.initAlgoCommand, &input.initAlgoCommand, sizeof(AlgoCommand_T));
        err = m_ptrCcu->ccu_af_start_and_init_algo(&ccuInitAFInput, &output);
    }
    else
    {
        err = 1;
    }

    return err;
}
MINT32 AfCCU::getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo)
{
    MINT32 err = 0;

    if (m_ptrCcu)
    {
        m_ptrCcu->ccu_af_get_DebugInfo(&rAFDebugInfo);
    }

    return err;
}
MRESULT AfCCU::getPdBlocks(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T& pdBlockOutput)
{
    MRESULT err = 0;
    if (m_ptrCcu)
    {
        err = m_ptrCcu->ccu_af_get_PD_block(pdBlockInut, &pdBlockOutput);
    }
    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCCU::resume()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
MINT32 AfCCU::suspend()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;
    return err;
}
MINT32 AfCCU::abort()
{
    CAM_LOGD("%s ", __FUNCTION__);
    MINT32 err = 0;

    if (m_ptrCcu)
        err = m_ptrCcu->ccu_af_abort();

    return err;
}

/////===== Private =====/////
MUINT64 AfCCU::getTimeStamp_us()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp = ((t.tv_sec) * 1000000000LL + t.tv_nsec) / 1000;
    return timestamp; // from nano to us
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfCCU::isMotorOpen(MINT32 &isMotorOpen, MINT32 &initPos)
{
    MINT32 ret = 0;

    if ( m_pMcuDrv)
    {
        mcuMotorInfo rMotorInfo;
        m_pMcuDrv->getMCUInfo(&rMotorInfo);
        isMotorOpen = rMotorInfo.bIsMotorOpen;
        initPos = rMotorInfo.u4CurrentPosition;
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// master only
MINT32 AfCCU::SyncAF_process(MINT32 slaveDevCCU, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T syncInfo)
{
    MINT32 err = 0;
    if (m_ptrCcu)
    {
        // To pass masterDevCCU, masterIdx, slaveDevCCU, slaveIdx to CCU
        m_ptrCcu->ccu_af_syncAf_process(m_sensorDev, m_sensorIdx, slaveDevCCU, slaveIdx, sync2AMode, &syncInfo);
    }
    return err;
}
// slave only
MINT32 AfCCU::SyncAF_getSensorInfoForCCU(MINT32& slaveDevCCU, MINT32& slaveIdx)
{
    MINT32 ret = 0;
    slaveDevCCU = m_sensorDev;
    slaveIdx = m_sensorIdx;
    return ret;
}
