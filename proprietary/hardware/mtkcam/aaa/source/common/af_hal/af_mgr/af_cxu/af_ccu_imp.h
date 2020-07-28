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
#ifndef _AfCCU_H_
#define _AfCCU_H_

#include "af_cxu_if.h"
#include <iccu_ctrl_af.h>
#include <ccu_ext_interface/ccu_sensor_extif.h>

/**
 * @brief AF manager class
 */
class AfCCU : IAfCxU
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /*  Copy constructor is disallowed. */
    AfCCU( AfCCU const&);

    /*  Copy-assignment operator is disallowed. */
    AfCCU& operator=( AfCCU const&);

public:
    AfCCU(MINT32 dev):
        szUsrName("af_ccu_imp")
    {
        m_ptrCcu = NULL;
        memset(&a_sAFHwInitInput, 0, sizeof(AF_HW_INIT_INPUT_T));
        memset(&a_sAFHwInitOutput, 0, sizeof(AF_HW_INIT_OUTPUT_T));
        m_pMcuDrv = NULL;
        m_i4isMotorOpen = 0;
        m_i4PostureCompInitPos = 0;
        m_i4DgbLogLv = 0;
        m_sensorDev = dev;
        m_sensorDevCCU = WHICH_SENSOR_NONE;
        m_sensorIdx = 0;
        memset(&m_mcuName, 0, sizeof(char) * 32);
        memset(&m_sCommand, 0, sizeof(AlgoCommand_T));
        motorDisable = 0;
        motorMFPosPre = motorMFPos = 0;
        targetPosPre = 0;
        m_u8MvLensTS = 0;
        motorInfPos = 0;
        motorMacroPos = 0;
        TGSize_W = 0;
        TGSize_H = 0;
        BINSize_W = 0;
        BINSize_H = 0;
    }
    virtual ~AfCCU() {};

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
    MINT32 updateOutput(DoAFOutput_T &afOutput);
    MINT32 updateNVRAM(StartAFInput_T &input, StartAFOutput_T &output);
    MRESULT getPdBlocks(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T& pdBlockOutput);
    MINT32 getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo);

    MINT32 resume();
    MINT32 suspend();
    MINT32 abort();
    MINT32 isCCUAF() {return 1;}

    MINT32 SyncAF_process(MINT32 slaveDevCCU, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T syncInfo);
    MINT32 SyncAF_getSensorInfoForCCU(MINT32& slaveDevCCU, MINT32& slaveIdx);
    MINT32 SyncAF_setSyncMode(MINT32 syncMode __unused) {return 0;}
    MINT32 SyncAF_motorRange(AF_SyncInfo_T& sCamInfo __unused) {return 0;}
    MINT32 SyncAF_getSyncInfo(AF_SyncInfo_T& sCamInfo __unused) {return 0;}
    MINT32 SyncAF_setSyncInfo(MINT32 pos __unused, AF_SyncInfo_T& sCamInfo __unused) {return 0;}
    MINT32 SyncAF_getMain2CalibPos(AF_SyncInfo_T& sCamInfo __unused) {return 0;}
    MINT32 SyncAF_calibPos(AF_SyncInfo_T& sCamInfo __unused) {return 0;}
    MUINT64 SyncAF_moveLensSyncAF(MINT32 targetPos __unused) {return 0;}
private:
    MUINT64 getTimeStamp_us();
    MINT32 isMotorOpen(MINT32 &isMotorOpen, MINT32 &initPos);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private :

    NSCcuIf::ICcuCtrlAf*  m_ptrCcu;

    AF_HW_INIT_INPUT_T a_sAFHwInitInput;
    AF_HW_INIT_OUTPUT_T a_sAFHwInitOutput;

    MCUDrv*          m_pMcuDrv;
    MINT32           m_i4isMotorOpen;
    MINT32           m_i4PostureCompInitPos;

    MINT32           m_i4DgbLogLv;
    MINT32           m_sensorDev;
    WHICH_SENSOR_T   m_sensorDevCCU;
    MINT32           m_sensorIdx;
    char             m_mcuName[32];
    AlgoCommand_T    m_sCommand;
    MINT32           motorDisable;
    MINT32           motorMFPos;
    MINT32           motorMFPosPre;
    MINT32           targetPosPre;
    MUINT64          m_u8MvLensTS;

    MINT32           motorInfPos;
    MINT32           motorMacroPos;

    MINT32           TGSize_W;
    MINT32           TGSize_H;
    MINT32           BINSize_W;
    MINT32           BINSize_H;
    uint8_t const szUsrName[32];
};

#endif // _AF_MGR_H_
