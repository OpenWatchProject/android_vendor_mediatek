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
#define LOG_TAG "isp_tuning_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_tuning_cam_info.h>
#include <paramctrl_if.h>
#include "isp_tuning_mgr.h"
#include <faces.h>
#define ABS(a)    ((a) > 0 ? (a) : -(a))
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr&
IspTuningMgr::
getInstance()
{
    static  IspTuningMgr singleton;
    return  singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr::
IspTuningMgr()
    : m_pParamctrl_Main(MNULL)
    , m_pParamctrl_Sub(MNULL)
    , m_pParamctrl_Main2(MNULL)
    , m_pParamctrl_Sub2(MNULL)
    , m_i4SensorDev(0)
    , m_bDebugEnable(MFALSE)
    , m_i4IspProfile(0)
    , m_i4SensorIdx(0)
{
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr::
~IspTuningMgr()
{
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4SubsampleCount)
{
    m_i4SensorDev = i4SensorDev;
    m_i4SensorIdx = i4SensorIdx;
    CAM_LOGD("m_i4SensorDev= %d, m_i4SensorIdx = %d, i4SubsampleCount = %d", m_i4SensorDev, m_i4SensorIdx, i4SubsampleCount);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.isp_tuning_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    if (i4SensorDev & ESensorDev_Main) {
        if (!m_pParamctrl_Main) {
            m_pParamctrl_Main = IParamctrl::createInstance(ESensorDev_Main, m_i4SensorIdx);
            if (m_pParamctrl_Main) {
                m_pParamctrl_Main->init();
            }
            else {
                CAM_LOGE("m_pParamctrl_Main is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is not NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (!m_pParamctrl_Sub) {
            m_pParamctrl_Sub = IParamctrl::createInstance(ESensorDev_Sub, m_i4SensorIdx);
            if (m_pParamctrl_Sub) {
                m_pParamctrl_Sub->init();
            }
            else {
                CAM_LOGE("m_pParamctrl_Sub is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is not NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (!m_pParamctrl_Main2) {
            m_pParamctrl_Main2 = IParamctrl::createInstance(ESensorDev_MainSecond, m_i4SensorIdx);
            if (m_pParamctrl_Main2) {
                m_pParamctrl_Main2->init();
            }
            else {
                CAM_LOGE("m_pParamctrl_Main2 is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is not NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (!m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2 = IParamctrl::createInstance(ESensorDev_SubSecond, m_i4SensorIdx);
            if (m_pParamctrl_Sub2) {
                m_pParamctrl_Sub2->init();
            }
            else {
                CAM_LOGE("m_pParamctrl_SubSecond is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_SubSecond is not NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::uninit(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->uninit();
            m_pParamctrl_Main->destroyInstance();
            m_pParamctrl_Main = MNULL;
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->uninit();
            m_pParamctrl_Sub->destroyInstance();
            m_pParamctrl_Sub = MNULL;
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->uninit();
            m_pParamctrl_Main2->destroyInstance();
            m_pParamctrl_Main2 = MNULL;
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->uninit();
            m_pParamctrl_Sub2->destroyInstance();
            m_pParamctrl_Sub2 = MNULL;
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspProfile(MINT32 const i4SensorDev, MINT32 const i4IspProfile)
{
    CAM_LOGD_IF(m_bDebugEnable,"setIspProfile: %d\n", i4IspProfile);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspProfile(static_cast<EIspProfile_T>(i4IspProfile));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspProfile(static_cast<EIspProfile_T>(i4IspProfile));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspProfile(static_cast<EIspProfile_T>(i4IspProfile));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspProfile(static_cast<EIspProfile_T>(i4IspProfile));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    m_i4IspProfile = i4IspProfile;
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSceneMode(MINT32 const i4SensorDev, MUINT32 const u4Scene)
{
    CAM_LOGD_IF(m_bDebugEnable,"setSceneMode: %d\n", u4Scene);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEffect(MINT32 const i4SensorDev, MUINT32 const u4Effect)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setOperMode(MINT32 const i4SensorDev, MINT32 const i4OperMode)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getOperMode(MINT32 const i4SensorDev)
{
    MINT32 operMode = EOperMode_Normal;
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             operMode = m_pParamctrl_Main->getOperMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            operMode = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            operMode = m_pParamctrl_Sub->getOperMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            operMode = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            operMode = m_pParamctrl_Main2->getOperMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            operMode = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            operMode = m_pParamctrl_Sub2->getOperMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            operMode = -1;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getOperMode()\n");
        operMode = -1;
    }
    return operMode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicBypass(MINT32 const i4SensorDev, MBOOL i4Bypass)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicBypass(MINT32 const i4SensorDev)
{
    MINT32 bypass = MFALSE;
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             bypass = m_pParamctrl_Main->isDynamicBypass();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            bypass = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            bypass = m_pParamctrl_Sub->isDynamicBypass();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            bypass = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            bypass = m_pParamctrl_Main2->isDynamicBypass();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            bypass = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            bypass = m_pParamctrl_Sub2->isDynamicBypass();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            bypass = -1;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDynamicBypass()\n");
        bypass = -1;
    }
    return bypass;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicCCM(MINT32 const i4SensorDev, MBOOL bdynamic_ccm)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicCCM(MINT32 const i4SensorDev)
{
    MINT32 bypass = MFALSE;
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             bypass = m_pParamctrl_Main->isDynamicCCM();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            bypass = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            bypass = m_pParamctrl_Sub->isDynamicCCM();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            bypass = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            bypass = m_pParamctrl_Main2->isDynamicCCM();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            bypass = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            bypass = m_pParamctrl_Sub2->isDynamicCCM();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            bypass = -1;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDynamicCCM()\n");
        bypass = -1;
    }
    return bypass;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::enableDynamicShading(MINT32 const i4SensorDev, MBOOL const fgEnable)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSensorMode(MINT32 const i4SensorDev, MINT32 const i4SensorMode, MBOOL const bFrontalBin, MUINT32 const u4BinWidth, MUINT32 const u4BinHeight)
{
    ISP_BIN_INFO_T BinInfo;
    BinInfo.fgBIN = bFrontalBin;
    BinInfo.u4BIN_Width = u4BinWidth;
    BinInfo.u4BIN_Height = u4BinHeight;
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
            m_pParamctrl_Main->setFrontBinInfo(BinInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
            m_pParamctrl_Sub->setFrontBinInfo(BinInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
            m_pParamctrl_Main2->setFrontBinInfo(BinInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
            m_pParamctrl_Sub2->setFrontBinInfo(BinInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getSensorMode(MINT32 const i4SensorDev)
{
    MINT32 sensorMode = ESensorMode_Capture;
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             sensorMode = m_pParamctrl_Main->getSensorMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            sensorMode = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            sensorMode = m_pParamctrl_Sub->getSensorMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            sensorMode = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            sensorMode = m_pParamctrl_Main2->getSensorMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            sensorMode = -1;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            sensorMode = m_pParamctrl_Sub2->getSensorMode();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            sensorMode = -1;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getSensorMode()\n");
        sensorMode = -1;
    }
    return sensorMode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setZoomRatio(MINT32 const i4SensorDev, MINT32 const i4ZoomRatio_x100)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBInfo(MINT32 const i4SensorDev, AWB_INFO_T const &rAWBInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAWBInfo(rAWBInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAWBInfo(rAWBInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAWBInfo(rAWBInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setAWBInfo(rAWBInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAEInfo(MINT32 const i4SensorDev, AE_INFO_T const &rAEInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAEInfo(rAEInfo);
        } else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAEInfo(rAEInfo);
        } else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAEInfo(rAEInfo);
        } else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setAEInfo(rAEInfo);
        } else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAFInfo(MINT32 const i4SensorDev, AF_INFO_T const &rAFInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAFInfo(rAFInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAFInfo(rAFInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAFInfo(rAFInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setAFInfo(rAFInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFlashInfo(MINT32 const i4SensorDev, FLASH_INFO_T const &rFlashInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "IspTuningMgr::setFlashInfo rFlashInfo.isFlash(%d)", rFlashInfo.isFlash);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setFlashInfo(rFlashInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setFlashInfo(rFlashInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setFlashInfo(rFlashInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setFlashInfo(rFlashInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setPureOBCInfo(MINT32 const i4SensorDev, const ISP_NVRAM_OBC_T *pOBCInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setPureOBCInfo(pOBCInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setPureOBCInfo(pOBCInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setPureOBCInfo(pOBCInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setPureOBCInfo(pOBCInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setRRZInfo(MINT32 const i4SensorDev, const ISP_NVRAM_RRZ_T *pRRZInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setRRZInfo(pRRZInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setRRZInfo(pRRZInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setRRZInfo(pRRZInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setRRZInfo(pRRZInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDebugInfo4TG(MINT32 const i4SensorDev, MUINT32 const u4Rto, MINT32 const OBCGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] u4Rto: %d, OBCGain: %d, rDbgIspInfo: %p", __FUNCTION__, u4Rto, OBCGain, rDbgIspInfo);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            //m_pParamctrl_Main->setDebugInfo4TG(u4Rto, OBCGain, rDbgIspInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            //m_pParamctrl_Sub->setDebugInfo4TG(u4Rto, OBCGain, rDbgIspInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            //m_pParamctrl_Main2->setDebugInfo4TG(u4Rto, OBCGain, rDbgIspInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            //m_pParamctrl_Sub2->setDebugInfo4TG(u4Rto, OBCGain, rDbgIspInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIndex_Shading(MINT32 const i4SensorDev, MINT32 const i4IDX)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIndex_Shading(i4IDX);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIndex_Shading(i4IDX);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIndex_Shading(i4IDX);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIndex_Shading(i4IDX);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getIndex_Shading(MINT32 const i4SensorDev, MVOID*const pCmdArg)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getIndex_Shading(pCmdArg);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getIndex_Shading(pCmdArg);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getIndex_Shading(pCmdArg);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getIndex_Shading(pCmdArg);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getIndex_Shading()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getCamInfo(MINT32 const i4SensorDev, RAWIspCamInfo& rCamInfo) const
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            rCamInfo = m_pParamctrl_Main->getCamInfo();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            rCamInfo = m_pParamctrl_Sub->getCamInfo();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            rCamInfo = m_pParamctrl_Main2->getCamInfo();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            rCamInfo = m_pParamctrl_Sub2->getCamInfo();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getCamInfo()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFlkEnable(MINT32 const i4SensorDev, MBOOL const flkEnable)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setFlkEnable(flkEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setFlkEnable(flkEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setFlkEnable(flkEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setFlkEnable(flkEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Edge(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Hue(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Sat(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Bright(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Contrast(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validate(MINT32 const i4SensorDev,  RequestSet_T const RequestSet, MBOOL const fgForce, MINT32 i4SubsampleIdex)
{
    MINT32 i4FrameID = RequestSet.vNumberSet[0];
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, i4FrameID = %d, i4SubsampleIdex = %d\n", __FUNCTION__, i4SensorDev, i4FrameID, i4SubsampleIdex);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->validate(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->validate(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->validate(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->validate(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrame(MINT32 const i4SensorDev, RequestSet_T const RequestSet, MBOOL const fgForce, MINT32 i4SubsampleIdex)
{
    MINT32 i4FrameID =  RequestSet.vNumberSet.front();
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, i4FrameID = %d\n", __FUNCTION__, i4SensorDev, i4FrameID);

    IParamctrl::setPerFramePropEnable(::property_get_int32("vendor.debug.aaa.perframe_prop.enable", 0));

    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrameP2(MINT32 const i4SensorDev, MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf)
{
    TuningParam* _pTuningBuf = reinterpret_cast<TuningParam*>(pTuningBuf);

    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d\n", __FUNCTION__, i4SensorDev);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->validatePerFrameP2(flowType, rIspInfo, _pTuningBuf->pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->validatePerFrameP2(flowType, rIspInfo, _pTuningBuf->pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->validatePerFrameP2(flowType, rIspInfo, _pTuningBuf->pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->validatePerFrameP2(flowType, rIspInfo, _pTuningBuf->pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfoP1(MINT32 const i4SensorDev, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, MBOOL const fgReadFromHW) const
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getDebugInfoP1(rIspExifDebugInfo, fgReadFromHW);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getDebugInfoP1(rIspExifDebugInfo, fgReadFromHW);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getDebugInfoP1(rIspExifDebugInfo, fgReadFromHW);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getDebugInfoP1(rIspExifDebugInfo, fgReadFromHW);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDebugInfoP1()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfo(MINT32 const i4SensorDev, const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, const void* pTuningBuf)
{
    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;

    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getDebugInfo(rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getDebugInfo(rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getDebugInfo(rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getDebugInfo(rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDebugInfo()\n");
        return MFALSE;
    }
    return MTRUE;
}
MBOOL IspTuningMgr::getDebugInfo_MultiPassNR(MINT32 const i4SensorDev, const NSIspTuning::ISP_INFO_T& rIspInfo, DEBUG_RESERVEA_INFO_T& rDebugInfo, const void* pRegBuf)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, rIspInfo = %p, rDebugInfo = %p, pRegBuf = %p\n", __FUNCTION__, i4SensorDev, &rIspInfo, &rDebugInfo, pRegBuf);
    CAM_LOGE("This function is not support on this platform");
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            //m_pParamctrl_Main->getDebugInfo_MultiPassNR(rIspInfo, rDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            //m_pParamctrl_Sub->getDebugInfo_MultiPassNR(rIspInfo, rDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            //m_pParamctrl_Main2->getDebugInfo_MultiPassNR(rIspInfo, rDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            //m_pParamctrl_Sub2->getDebugInfo_MultiPassNR(rIspInfo, rDebugInfo, pRegBuf);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDebugInfo()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::notifyRPGEnable(MINT32 const i4SensorDev, MBOOL const bRPGEnable)
{
    CAM_LOGD_IF(m_bDebugEnable,"bRPGEnable: %d\n", bRPGEnable);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->notifyRPGEnable(bRPGEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->notifyRPGEnable(bRPGEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->notifyRPGEnable(bRPGEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->notifyRPGEnable(bRPGEnable);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::notifyRPGEnable()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*MBOOL IspTuningMgr::getLCSparam(MINT32 const i4SensorDev, ISP_LCS_IN_INFO_T &lcs_info)
{
    //LCS_ISP_INFO_T lcs_info;
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getLCSparam(lcs_info);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getLCSparam(lcs_info);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getLCSparam(lcs_info);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getLCSparam(lcs_info);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getLCSparam()\n");
        return MFALSE;
    }
    return MTRUE;
}*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IspTuningMgr::getDMGItable(MINT32 const i4SensorDev)
{
    CAM_LOGD("this function is not supported in isp_30");

    return NULL;

    /*if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            //return m_pParamctrl_Main->getDMGItable();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return NULL;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            //return m_pParamctrl_Sub->getDMGItable();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return NULL;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            //return m_pParamctrl_Main2->getDMGItable();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return NULL;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            //return m_pParamctrl_Sub2->getDMGItable();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return NULL;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDMGItable()\n");
        return NULL;
    }
    return NULL;*/
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBGain(MINT32 const i4SensorDev, AWB_GAIN_T& rIspAWBGain)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAWBGain(rIspAWBGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAWBGain(rIspAWBGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAWBGain(rIspAWBGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setAWBGain(rIspAWBGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setAWBGain()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setISPAEGain(MINT32 const i4SensorDev, MBOOL bEnableWorkAround, MUINT32 u4IspGain)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setISPAEGain(bEnableWorkAround, u4IspGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setISPAEGain(bEnableWorkAround, u4IspGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setISPAEGain(bEnableWorkAround, u4IspGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setISPAEGain(bEnableWorkAround, u4IspGain);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setISPAEGain()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspAEPreGain2(MINT32 const i4SensorDev, MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setIspAEPreGain2()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspFlareGainOffset(MINT32 const i4SensorDev, MINT16 i2FlareGain, MINT16 i2FlareOffset)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspFlareGainOffset(i2FlareGain, i2FlareOffset);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspFlareGainOffset(i2FlareGain, i2FlareOffset);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspFlareGainOffset(i2FlareGain, i2FlareOffset);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setIspFlareGainOffset(i2FlareGain, i2FlareOffset);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setIspFlareGainOffset()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, i4TGInfo = %d\n", __FUNCTION__, i4SensorDev, i4TGInfo);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTGInfo(i4TGInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTGInfo(i4TGInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTGInfo(i4TGInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setTGInfo(i4TGInfo);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setTGInfo()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFDEnable(MINT32 const i4SensorDev, MBOOL const bFDenable)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setFDEnable(bFDenable);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setFDEnable(bFDenable);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setFDEnable(bFDenable);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setFDEnable(bFDenable);
        }
        else {
            MY_ERR("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFDInfo(MINT32 const i4SensorDev, MVOID* const a_sFaces, MUINT32 const TGSize)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    MUINT32 FaceArea = 0;
    MUINT32 temp_area = 0;
    for(int i = 0; i < pFaces->number_of_faces; i++){
        temp_area = ((ABS(pFaces->faces[i].rect[2] - pFaces->faces[i].rect[0])) * (ABS(pFaces->faces[i].rect[3] - pFaces->faces[i].rect[1])));
        FaceArea += temp_area;
    }
    float FDRatio = (float)FaceArea / (float)TGSize;
    if( FDRatio > 1.0 ){
        FDRatio = 1.0;
    }
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setFDInfo(FDRatio);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setFDInfo(FDRatio);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setFDInfo(FDRatio);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setFDInfo(FDRatio);
        }
        else {
            MY_ERR("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*MVOID IspTuningMgr::getDefaultObc(MINT32 const i4SensorDev, MUINT32 eIspProfile, MUINT32 eSensorMode, MVOID*& pObc, MUINT16& i4idx)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getDefaultObc(eIspProfile, eSensorMode, pObc, i4idx);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getDefaultObc(eIspProfile, eSensorMode, pObc, i4idx);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getDefaultObc(eIspProfile, eSensorMode, pObc, i4idx);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getDefaultObc(eIspProfile, eSensorMode, pObc, i4idx);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDefaultObc()\n");
    }
}*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IspTuningMgr::getDefaultISPIndex(MINT32 const i4SensorDev, MUINT32 eIspProfile, MUINT32 eSensorMode, MUINT32 eIdx_ISO)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            return m_pParamctrl_Main->getDefaultISPIndex(eIspProfile, eSensorMode, eIdx_ISO);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return NULL;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            return m_pParamctrl_Sub->getDefaultISPIndex(eIspProfile, eSensorMode, eIdx_ISO);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return NULL;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            return m_pParamctrl_Main2->getDefaultISPIndex(eIspProfile, eSensorMode, eIdx_ISO);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return NULL;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            return m_pParamctrl_Sub2->getDefaultISPIndex(eIspProfile, eSensorMode, eIdx_ISO);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return NULL;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getDefaultISPIndex()\n");
        return NULL;
    }
    return NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IspTuningMgr::sendIspTuningIOCtrl(MINT32 const i4SensorDev, E_ISPTUNING_CTRL const ctrl, MINTPTR arg1, MINTPTR arg2)
{
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->sendIspTuningIOCtrl(ctrl, arg1, arg2);
            return;
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->sendIspTuningIOCtrl(ctrl, arg1, arg2);
            return;
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->sendIspTuningIOCtrl(ctrl, arg1, arg2);
            return;
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->sendIspTuningIOCtrl(ctrl, arg1, arg2);
            return;
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::sendIspTuningIOCtrl()\n");
        return;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::forceValidate(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d\n", __FUNCTION__, i4SensorDev);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->forceValidate();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->forceValidate();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->forceValidate();
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->forceValidate();
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::forceValidate()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setCamMode(MINT32 const i4SensorDev, MINT32 const i4CamMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d\n", __FUNCTION__, i4SensorDev);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setCamMode(i4CamMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setCamMode(i4CamMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setCamMode(i4CamMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setCamMode(i4CamMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setCamMode()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getColorCorrectionTransform(MINT32 const i4SensorDev,
                                                 MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                                 MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                                 MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getColorCorrectionTransform()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setColorCorrectionTransform(MINT32 const i4SensorDev,
                                                 MFLOAT M11, MFLOAT M12, MFLOAT M13,
                                                 MFLOAT M21, MFLOAT M22, MFLOAT M23,
                                                 MFLOAT M31, MFLOAT M32, MFLOAT M33)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setColorCorrectionTransform()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setColorCorrectionMode()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEdgeMode(MINT32 const i4SensorDev, MINT32 i4EdgeMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setEdgeMode(i4EdgeMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setEdgeMode(i4EdgeMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setEdgeMode(i4EdgeMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setEdgeMode(i4EdgeMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setEdgeMode()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setNoiseReductionMode(MINT32 const i4SensorDev, MINT32 i4NRMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setNoiseReductionMode(i4NRMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setNoiseReductionMode(i4NRMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setNoiseReductionMode(i4NRMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setNoiseReductionMode(i4NRMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setNoiseReductionMode()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setToneMapMode(MINT32 const i4SensorDev, MINT32 i4ToneMapMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setToneMapMode(i4ToneMapMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setToneMapMode(i4ToneMapMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setToneMapMode(i4ToneMapMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setToneMapMode(i4ToneMapMode);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setToneMapMode()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getTonemapCurve_Red()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getTonemapCurve_Green()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::getTonemapCurve_Blue()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setTonemapCurve_Red()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setTonemapCurve_Green()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::setTonemapCurve_Blue()\n");
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::convertPtPairsToGMA(MINT32 const i4SensorDev, const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(), i4SensorDev(0x%x)\n", __FUNCTION__, i4SensorDev);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            return m_pParamctrl_Main->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            return m_pParamctrl_Sub->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            return m_pParamctrl_Main2->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        if (m_pParamctrl_Sub2) {
            return m_pParamctrl_Sub2->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");
            return MFALSE;
        }
    }
    else {
        CAM_LOGE("Err IspTuningMgr::convertPtPairsToGMA()\n");
        return MFALSE;
    }
}

const char *IspTuningMgr::getIspProfileName(EIspProfile_T IspProfile)
{
    switch (IspProfile)
    {
    case EIspProfile_Preview                        :   return "Preview";
    case EIspProfile_Video                          :   return "Video";
    case EIspProfile_Capture                        :   return "Capture";
    case EIspProfile_ZSD_Capture                    :   return "ZSD_Capture";
    case EIspProfile_VSS_Capture                    :   return "VSS_Capture";
    case EIspProfile_PureRAW_Capture                :   return "PureRAW_Capture";
    case EIspProfile_N3D_Preview                    :   return "N3D_Preview";
    case EIspProfile_N3D_Video                      :   return "N3D_Video";
    case EIspProfile_N3D_Capture                    :   return "N3D_Capture";
    case EIspProfile_MFB_Capture_EE_Off             :   return "MFB_Capture_EE_Off";
    case EIspProfile_MFB_Capture_EE_Off_SWNR        :   return "MFB_Capture_EE_Off_SWNR";
    case EIspProfile_MFB_Blending_All_Off           :   return "MFB_Blending_All_Off";
    case EIspProfile_MFB_Blending_All_Off_SWNR      :   return "MFB_Blending_All_Off_SWNR";
    case EIspProfile_MFB_PostProc_EE_Off            :   return "MFB_PostProc_EE_Off";
    case EIspProfile_MFB_PostProc_ANR_EE            :   return "MFB_PostProc_ANR_EE";
    case EIspProfile_MFB_PostProc_ANR_EE_SWNR       :   return "MFB_PostProc_ANR_EE_SWNR";
    case EIspProfile_MFB_PostProc_Mixing            :   return "MFB_PostProc_Mixing";
    case EIspProfile_MFB_PostProc_Mixing_SWNR       :   return "MFB_PostProc_Mixing_SWNR";
    case EIspProfile_VFB_PostProc                   :   return "VFB_PostProc";
    case EIspProfile_IHDR_Preview                   :   return "IHDR_Preview";
    case EIspProfile_IHDR_Video                     :   return "IHDR_Video";
    case EIspProfile_Capture_MultiPass_ANR_1        :   return "Capture_MultiPass_ANR_1";
    case EIspProfile_Capture_MultiPass_ANR_2        :   return "Capture_MultiPass_ANR_2";
    case EIspProfile_VSS_Capture_MultiPass_ANR_1    :   return "VSS_Capture_MultiPass_ANR_1";
    case EIspProfile_VSS_Capture_MultiPass_ANR_2    :   return "VSS_Capture_MultiPass_ANR_2";
    case EIspProfile_MFB_MultiPass_ANR_1            :   return "MFB_MultiPass_ANR_1";
    case EIspProfile_MFB_MultiPass_ANR_2            :   return "MFB_MultiPass_ANR_2";
    case EIspProfile_Capture_SWNR                   :   return "Capture_SWNR";
    case EIspProfile_VSS_Capture_SWNR               :   return "VSS_Capture_SWNR";
    case EIspProfile_PureRAW_Capture_SWNR           :   return "PureRAW_Capture_SWNR";
    case EIspProfile_MHDR_Preview                   :   return "MHDR_Preview";
    case EIspProfile_MHDR_Video                     :   return "MHDR_Video";
    case EIspProfile_MHDR_Capture                   :   return "MHDR_Capture";
    case EIspProfile_VSS_MFB_Capture_EE_Off         :   return "VSS_MFB_Capture_EE_Off";
    case EIspProfile_VSS_MFB_Capture_EE_Off_SWNR    :   return "VSS_MFB_Capture_EE_Off_SWNR";
    case EIspProfile_VSS_MFB_Blending_All_Off       :   return "VSS_MFB_Blending_All_Off";
    case EIspProfile_VSS_MFB_Blending_All_Off_SWNR  :   return "VSS_MFB_Blending_All_Off_SWNR";
    case EIspProfile_VSS_MFB_PostProc_EE_Off        :   return "VSS_MFB_PostProc_EE_Off";
    case EIspProfile_VSS_MFB_PostProc_ANR_EE        :   return "VSS_MFB_PostProc_ANR_EE";
    case EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR   :   return "VSS_MFB_PostProc_ANR_EE_SWNR";
    case EIspProfile_VSS_MFB_PostProc_Mixing        :   return "VSS_MFB_PostProc_Mixing";
    case EIspProfile_VSS_MFB_PostProc_Mixing_SWNR   :   return "VSS_MFB_PostProc_Mixing_SWNR";
    case EIspProfile_VSS_MFB_MultiPass_ANR_1        :   return "VSS_MFB_MultiPass_ANR_1";
    case EIspProfile_VSS_MFB_MultiPass_ANR_2        :   return "VSS_MFB_MultiPass_ANR_2";
    case EIspProfile_FLASH_Capture                  :   return "FLASH_Capture";
    case EIspProfile_PureRAW_TPipe_Capture          :   return "PureRAW_TPipe_Capture";
    case EIspProfile_PureRAW_TPipe_Capture_SWNR     :   return "PureRAW_TPipe_Capture_SWNR";
    case EIspProfile_DUMMY                          :   return "DUMMY";
    default:                                            return "UnknownProfile";
    }
}
