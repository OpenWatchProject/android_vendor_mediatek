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
/**
 * @file af_mgr.h
 * @brief AF manager, do focusing for raw sensor.
 */
#ifndef _AfCPU_H_
#define _AfCPU_H_

#include "af_cxu_if.h"
#include <mcu_drv.h>

/**
 * @brief AF manager class
 */
class AfCPU : IAfCxU
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /*  Copy constructor is disallowed. */
    AfCPU( AfCPU const&);

    /*  Copy-assignment operator is disallowed. */
    AfCPU& operator=( AfCPU const&);

public:
    AfCPU(MINT32 dev)
    {
        memset(&a_sAFHwInitInput, 0, sizeof(AF_HW_INIT_INPUT_T));
        memset(&a_sAFHwInitOutput, 0, sizeof(AF_HW_INIT_OUTPUT_T));
        m_i4DgbLogLv = 0;
        m_pMcuDrv = NULL;
        m_sensorDev = dev;
        c_sensorDev = ext_ESensorDev_None;
        m_sensorIdx = 0;
        memset(&m_sCommand, 0, sizeof(AlgoCommand_T));
        motorDisable = 0;
        motorMFPos = 0;
        motorMFPosPre = 0;
        targetPosPre = 0;
        m_u8MvLensTS = 0;
        TGSize_W = 0;
        TGSize_H = 0;
        BINSize_W = 0;
        BINSize_H = 0;
    }
    virtual ~AfCPU() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @Brief :
     *          Get AF manager instance.
     * @Param :
     *          [ in] i4SensorDev - sensor devices.
     * @Return:
     *          [out] return instance.
     */
    static IAfCxU* getInstance(MINT32 sensorDev, MINT32 sensorIdx);
    MINT32 init(MINT32 sensorDev, MINT32 sensorIdx, MUINT8* mcuName);
    MINT32 config(ConfigAFInput_T &configInput, ConfigAFOutput_T &configOutput);
    MINT32 start(StartAFInput_T &StartInput, StartAFOutput_T &StartOutput);

    MINT32 stop(LENS_INFO_T &LensInfo);
    MINT32 uninit();

    MINT32 doStt(DoSttIn_T doSttIn, DoSttOut_T& doSttOut);
    MINT32 doAF(DoAFInput_T &afInput, DoAFOutput_T &afOutput, AF_STAT_T* afoBuf);
    MINT32 updateOutput(DoAFOutput_T &afOutput __unused)
    {
        return 0;
    }
    MINT32 updateNVRAM(StartAFInput_T &input, StartAFOutput_T &output);
    MRESULT getPdBlocks(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T& pdBlockOutput);
    MINT32 getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo);

    MINT32 resume();
    MINT32 suspend();
    MINT32 abort();
    MINT32 isCCUAF() {return 0;}
    // SyncAF
    MINT32 SyncAF_getSensorInfoForCCU(MINT32& slaveDevCCU __unused, MINT32& slaveIdxCCU __unused) {return 1;}
    MINT32 SyncAF_process(MINT32 slaveDevCCU __unused, MINT32 slaveIdxCCU __unused, MINT32 sync2AMode __unused, AF_SyncInfo_T syncInfo __unused) {return 0;}
    MINT32 SyncAF_setSyncMode(MINT32 syncMode);
    MINT32 SyncAF_motorRange(AF_SyncInfo_T& sCamInfo);
    MINT32 SyncAF_getSyncInfo(AF_SyncInfo_T& sCamInfo);
    MINT32 SyncAF_setSyncInfo(MINT32 pos, AF_SyncInfo_T& sCamInfo);
    MINT32 SyncAF_getMain2CalibPos(AF_SyncInfo_T& sCamInfo);
    MINT32 SyncAF_calibPos(AF_SyncInfo_T& sCamInfo);
    MUINT64 SyncAF_moveLensSyncAF(MINT32 targetPos);

private:
    MINT32 ConfigHWReg(ConfigAFOutput_T &ConfigOutput);
    MINT32 doAlgoCommand(AlgoCommand_T command);
    MUINT64 getTimeStamp_us();
    MUINT64 MoveLensTo(MINT32 targetPos);
    MRESULT getLensInfo(LENS_INFO_T &a_rLensInfo);
    MINT32 ConvertDMABufToStat( MINT32 & i4CurPos, MVOID * ptrInStatBuf, AF_STAT_PROFILE_T & sOutSata);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private :

    AF_HW_INIT_INPUT_T a_sAFHwInitInput;
    AF_HW_INIT_OUTPUT_T a_sAFHwInitOutput;

    MINT32         m_i4DgbLogLv;
    MCUDrv*        m_pMcuDrv;
    MINT32           m_sensorDev;
    ext_ESensorDev_T c_sensorDev;
    MINT32         m_sensorIdx;
    AlgoCommand_T  m_sCommand;
    MINT32 motorDisable;
    MINT32 motorMFPos;
    MINT32 motorMFPosPre;
    MINT32 targetPosPre;
    MUINT64 m_u8MvLensTS;

    MINT32 TGSize_W;
    MINT32 TGSize_H;
    MINT32 BINSize_W;
    MINT32 BINSize_H;
};

#endif // _AF_MGR_H_
