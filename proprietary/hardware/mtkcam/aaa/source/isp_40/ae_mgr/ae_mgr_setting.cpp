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
#define LOG_TAG "ae_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"


using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;

#define AE_LOG(fmt, arg...) \
    do { \
        if (m_i4DgbLogE) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_LOG_IF(cond, ...) \
    do { \
        if (m_i4DgbLogE) { \
            CAM_LOGE_IF(cond, __VA_ARGS__); \
        } else { \
            CAM_LOGD_IF(cond, __VA_ARGS__); \
        } \
    }while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                           CCU setting implement                            //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                           CPU setting implement                            //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

MRESULT AeMgr::prepareSetStartParam(AE_SETTING_INPUT_INFO_T* pinput)
{
    pinput->i4SensorIdx = m_i4SensorIdx;
    pinput->eSensorTG = m_eSensorTG;
    pinput->eSensorMode = m_eSensorMode;
    pinput->eAETargetMode = m_eAETargetMode;
    return S_AE_OK;
}

MRESULT AeMgr::prepareSetControlParam(AE_SETTING_CONTROL_INFO_T* pinput)
{
    MBOOL bPerframeAndExpChanged = MTRUE;
    MUINT32 u4prevExp = 30000;
    MUINT32 u4prevAfe = 1024;
    m_pIAeFlowCPU->getParam(E_AE_FLOW_CPU_PREVQUEUE_EXP, u4prevExp);
    m_pIAeFlowCPU->getParam(E_AE_FLOW_CPU_PREVQUEUE_AFE, u4prevAfe);
    if (  m_rAEInitInput.rAEPARAM.bPerFrameAESmooth      &&
         (m_bStereoManualPline == MFALSE)                &&
         (m_u4UpdateShutterValue == m_u4PrevExposureTime)&&
         (m_u4UpdateShutterValue == u4prevExp)           &&
         (m_u4UpdateGainValue    == m_u4PrevSensorGain)  &&
         (m_u4UpdateGainValue    == u4prevExp)           &&
          m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_CYCLECOUNT_VALID) ){
         bPerframeAndExpChanged = MFALSE;
    }
    pinput->eAETargetMode = m_eAETargetMode;
    pinput->bPerframeAndExpChanged = bPerframeAndExpChanged;
    pinput->u4LEHDRshutter = m_u4LEHDRshutter;
    pinput->u4SEHDRshutter = m_u4SEHDRshutter;
    pinput->u4UpdateFrameRate_x10 = (MUINT32)m_u4UpdateFrameRate_x10;
    pinput->u4UpdateGainValue = m_u4UpdateGainValue;
    pinput->u4UpdateGainValue2 = m_u4UpdateGainValue2;
    pinput->u4UpdateShutterValue = m_u4UpdateShutterValue;
    pinput->u4PreExposureTime = m_u4PreExposureTime;
    pinput->u4PreSensorGain = m_u4PreSensorGain;
    pinput->bSensorAWBGain = bUpdateSensorAWBGain;
    pinput->i4Pass2SensorLVvalue = m_BVvalue +50;
    return S_AE_OK;
}

