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
#include <private/aaa_utils.h>

#define ABS(a)    ((a) > 0 ? (a) : -(a))
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

#define PARAMCTRL_SENSORDEV_INSTANCE(api,...)\
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);\
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {\
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);\
        return MFALSE;\
    }\
    IParamctrl* pCtrl = m_pParamctrlList[i4SensorIdx].instance;\
    if (pCtrl) {\
        pCtrl->api(__VA_ARGS__);\
    }\
    else {\
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);\
        return MFALSE;\
    }

#define PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(api,...)\
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);\
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {\
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);\
        return ret;\
    }\
    IParamctrl* pCtrl = m_pParamctrlList[i4SensorIdx].instance;\
    if (pCtrl) {\
        ret = pCtrl->api(__VA_ARGS__);\
    }\
    else {\
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);\
        return ret;\
    }

static NS3Av3::INST_T<IspTuningMgr> singleton;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr&
IspTuningMgr::
getInstance()
{
    std::call_once(singleton.onceFlag, [&] {
        singleton.instance = std::make_unique<IspTuningMgr>();
    } );

    return  *(singleton.instance);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr::
IspTuningMgr()
    : m_i4SensorDev(0)
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
    CAM_LOGD("m_i4SensorDev= %d, m_i4SensorIdx = %d\n", i4SensorDev, i4SensorIdx);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.isp_tuning_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
            return MFALSE;
        }

    INST_T& rSingleton = m_pParamctrlList[i4SensorIdx];

    Mutex::Autolock _l(rSingleton.lock);
    if(rSingleton.instance == nullptr) {
        rSingleton.instance = IParamctrl::createInstance((ESensorDev_T)i4SensorDev, i4SensorIdx);
        (rSingleton.instance)->init(i4SubsampleCount);
    }

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::uninit(MINT32 const i4SensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
            return MFALSE;
        }

    INST_T& rSingleton = m_pParamctrlList[i4SensorIdx];

    Mutex::Autolock _l(rSingleton.lock);
    IParamctrl*& pCtrl = rSingleton.instance;
    if (pCtrl) {
        pCtrl->uninit();
        pCtrl->destroyInstance();
        pCtrl = nullptr;
        }
        else {
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);
            return MFALSE;
        }

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspProfile(MINT32 const i4SensorDev, MINT32 const i4IspProfile)
{
    CAM_LOGD_IF(m_bDebugEnable,"setIspProfile: %d\n", i4IspProfile);

    PARAMCTRL_SENSORDEV_INSTANCE(setIspProfile, static_cast<EIspProfile_T>(i4IspProfile));

    m_i4IspProfile = i4IspProfile;
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSceneMode(MINT32 const i4SensorDev, MUINT32 const u4Scene)
{
    CAM_LOGD_IF(m_bDebugEnable,"setSceneMode: %d\n", u4Scene);

    PARAMCTRL_SENSORDEV_INSTANCE(setSceneMode, static_cast<EIndex_Scene_T>(u4Scene));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEffect(MINT32 const i4SensorDev, MUINT32 const u4Effect)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setEffect, static_cast<EIndex_Effect_T>(u4Effect));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setOperMode(MINT32 const i4SensorDev, MINT32 const i4OperMode)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setOperMode, static_cast<EOperMode_T>(i4OperMode));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getOperMode(MINT32 const i4SensorDev)
{
    MINT32 ret = -1;

    PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(getOperMode, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicBypass(MINT32 const i4SensorDev, MBOOL i4Bypass)
{
    PARAMCTRL_SENSORDEV_INSTANCE(enableDynamicBypass, static_cast<MBOOL>(i4Bypass));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicBypass(MINT32 const i4SensorDev)
{
    MINT32 ret = -1;

    PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(isDynamicBypass, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicCCM(MINT32 const i4SensorDev, MBOOL bdynamic_ccm)
{
    PARAMCTRL_SENSORDEV_INSTANCE(enableDynamicCCM, static_cast<MBOOL>(bdynamic_ccm));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicCCM(MINT32 const i4SensorDev)
{
    MINT32 ret = -1;

    PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(isDynamicCCM, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::enableDynamicShading(MINT32 const i4SensorDev, MBOOL const fgEnable)
{
    PARAMCTRL_SENSORDEV_INSTANCE(enableDynamicShading, static_cast<MBOOL>(fgEnable));

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

    {PARAMCTRL_SENSORDEV_INSTANCE(setSensorMode, static_cast<ESensorMode_T>(i4SensorMode));}
    {PARAMCTRL_SENSORDEV_INSTANCE(setFrontBinInfo, BinInfo);}

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getSensorMode(MINT32 const i4SensorDev)
{
    MINT32 ret = -1;

    PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(getSensorMode, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setZoomRatio(MINT32 const i4SensorDev, MINT32 const i4ZoomRatio_x100)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setZoomRatio, i4ZoomRatio_x100);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBInfo(MINT32 const i4SensorDev, AWB_INFO_T const &rAWBInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setAWBInfo, rAWBInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAEInfo(MINT32 const i4SensorDev, AE_INFO_T const &rAEInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setAEInfo, rAEInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAEInfoP2(MINT32 const i4SensorDev, AE_INFO_T const &rAEInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setAEInfoP2, rAEInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAFInfo(MINT32 const i4SensorDev, AF_INFO_T const &rAFInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setAFInfo, rAFInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFlashInfo(MINT32 const i4SensorDev, FLASH_INFO_T const &rFlashInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setFlashInfo, rFlashInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setPureOBCInfo(MINT32 const i4SensorDev, const ISP_NVRAM_OBC_T *pOBCInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setPureOBCInfo, pOBCInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setRRZInfo(MINT32 const i4SensorDev, const ISP_NVRAM_RRZ_T *pRRZInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setRRZInfo, pRRZInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDebugInfo4TG(MINT32 const i4SensorDev, MUINT32 const u4Rto, MINT32 const OBCGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setDebugInfo4TG, u4Rto, OBCGain, rDbgIspInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIndex_Shading(MINT32 const i4SensorDev, MINT32 const i4IDX)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIndex_Shading, i4IDX);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getIndex_Shading(MINT32 const i4SensorDev, MVOID*const pCmdArg)
{
    PARAMCTRL_SENSORDEV_INSTANCE(getIndex_Shading, pCmdArg);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getCamInfo(MINT32 const i4SensorDev, RAWIspCamInfo& rCamInfo) const
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
            return MFALSE;
        }

    IParamctrl* pCtrl = m_pParamctrlList[i4SensorIdx].instance;
    if (pCtrl) {
        rCamInfo = pCtrl->getCamInfo();
        }
        else {
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);
        return MFALSE;
    }
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFlkEnable(MINT32 const i4SensorDev, MBOOL const flkEnable)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setFlkEnable, flkEnable);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Edge(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Edge, static_cast<EIndex_Isp_Edge_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Hue(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Hue, static_cast<EIndex_Isp_Hue_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Sat(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Sat, static_cast<EIndex_Isp_Saturation_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Bright(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Bright, static_cast<EIndex_Isp_Brightness_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Contrast(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Contrast, static_cast<EIndex_Isp_Contrast_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validate(MINT32 const i4SensorDev,  RequestSet_T const RequestSet, MBOOL const fgForce, MINT32 i4SubsampleIdex)
{
    MINT32 i4FrameID = RequestSet.vNumberSet[0];
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, i4FrameID = %d\n", __FUNCTION__, i4SensorDev, i4FrameID);

    PARAMCTRL_SENSORDEV_INSTANCE(validate, RequestSet, fgForce, i4SubsampleIdex);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrame(MINT32 const i4SensorDev, RequestSet_T const RequestSet, MBOOL const fgForce, MINT32 i4SubsampleIdex)
{
    if(RequestSet.vNumberSet.empty())
        return MFALSE;

    MINT32 i4FrameID =  RequestSet.vNumberSet.front();
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, i4FrameID = %d\n", __FUNCTION__, i4SensorDev, i4FrameID);

    IParamctrl::setPerFramePropEnable(::property_get_int32("vendor.debug.aaa.perframe_prop.enable", 0));

    PARAMCTRL_SENSORDEV_INSTANCE(validatePerFrame, RequestSet, fgForce, i4SubsampleIdex);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrameP2(MINT32 const i4SensorDev, MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pRegBuf)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d\n", __FUNCTION__, i4SensorDev);

    PARAMCTRL_SENSORDEV_INSTANCE(validatePerFrameP2, flowType, rIspInfo, pRegBuf);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfoP1(MINT32 const i4SensorDev, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, MBOOL const fgReadFromHW) const
{
    PARAMCTRL_SENSORDEV_INSTANCE(getDebugInfoP1, rIspExifDebugInfo, fgReadFromHW);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfo(MINT32 const i4SensorDev, const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, const void* pRegBuf)
{
    PARAMCTRL_SENSORDEV_INSTANCE(getDebugInfo, rIspInfo, rIspExifDebugInfo, pRegBuf);

    return MTRUE;
}
MBOOL IspTuningMgr::getDebugInfo_MultiPassNR(MINT32 const i4SensorDev, const NSIspTuning::ISP_INFO_T& rIspInfo, DEBUG_RESERVEA_INFO_T& rDebugInfo, const void* pRegBuf)
{
    PARAMCTRL_SENSORDEV_INSTANCE(getDebugInfo_MultiPassNR, rIspInfo, rDebugInfo, pRegBuf);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::notifyRPGEnable(MINT32 const i4SensorDev, MBOOL const bRPGEnable)
{
    PARAMCTRL_SENSORDEV_INSTANCE(notifyRPGEnable, bRPGEnable);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getLCSparam(MINT32 const i4SensorDev, ISP_LCS_IN_INFO_T &lcs_info)
{
    PARAMCTRL_SENSORDEV_INSTANCE(getLCSparam, lcs_info);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IspTuningMgr::getDMGItable(MINT32 const i4SensorDev)
{
    MVOID* ret = NULL;

    PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(getDMGItable, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBGain(MINT32 const i4SensorDev, AWB_GAIN_T& rIspAWBGain)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setAWBGain, rIspAWBGain);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setISPAEGain(MINT32 const i4SensorDev, MBOOL bEnableWorkAround, MUINT32 u4IspGain)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setISPAEGain, bEnableWorkAround, u4IspGain);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspAEPreGain2(MINT32 const i4SensorDev, MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspAEPreGain2, i4SensorIndex, rNewIspAEPreGain2);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspFlareGainOffset(MINT32 const i4SensorDev, MINT16 i2FlareGain, MINT16 i2FlareOffset)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setIspFlareGainOffset, i2FlareGain, i2FlareOffset);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d, i4TGInfo = %d\n", __FUNCTION__, i4SensorDev, i4TGInfo);

    PARAMCTRL_SENSORDEV_INSTANCE(setTGInfo, i4TGInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFDEnable(MINT32 const i4SensorDev, MBOOL const bFDenable)
{
    PARAMCTRL_SENSORDEV_INSTANCE(setFDEnable, bFDenable);

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

    PARAMCTRL_SENSORDEV_INSTANCE(setFDInfo, FDRatio);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IspTuningMgr::getDefaultObc(MINT32 const i4SensorDev, MUINT32 eIspProfile, MUINT32 eSensorMode, MVOID*& pObc, MUINT16& i4idx)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return;
        }

    IParamctrl* pCtrl = m_pParamctrlList[i4SensorIdx].instance;
    if (pCtrl) {
        pCtrl->getDefaultObc(eIspProfile, eSensorMode, pObc, i4idx);
        return;
        }
        else {
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);
        return;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IspTuningMgr::getDefaultISPIndex(MINT32 const i4SensorDev, MUINT32 eIspProfile, MUINT32 eSensorMode, MUINT32 eIdx_ISO)
{
    MVOID* ret = NULL;

    PARAMCTRL_SENSORDEV_INSTANCE_WITH_RET(getDefaultISPIndex, eIspProfile, eSensorMode, eIdx_ISO);

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IspTuningMgr::sendIspTuningIOCtrl(MINT32 const i4SensorDev, E_ISPTUNING_CTRL const ctrl, MINTPTR arg1, MINTPTR arg2)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
            return;
        }

    IParamctrl* pCtrl = m_pParamctrlList[i4SensorIdx].instance;
    if (pCtrl) {
        pCtrl->sendIspTuningIOCtrl(ctrl, arg1, arg2);
            return;
        }
        else {
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);
        return;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::forceValidate(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d\n", __FUNCTION__, i4SensorDev);

    PARAMCTRL_SENSORDEV_INSTANCE(forceValidate, );

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setCamMode(MINT32 const i4SensorDev, MINT32 const i4CamMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4SensorDev = %d\n", __FUNCTION__, i4SensorDev);

    PARAMCTRL_SENSORDEV_INSTANCE(setCamMode, i4CamMode);

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

    PARAMCTRL_SENSORDEV_INSTANCE(getColorCorrectionTransform, M11, M12, M13, M21, M22, M23, M31, M32, M33);

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

    PARAMCTRL_SENSORDEV_INSTANCE(setColorCorrectionTransform, M11, M12, M13, M21, M22, M23, M31, M32, M33);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setColorCorrectionMode, i4ColorCorrectionMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEdgeMode(MINT32 const i4SensorDev, MINT32 i4EdgeMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setEdgeMode, i4EdgeMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setNoiseReductionMode(MINT32 const i4SensorDev, MINT32 i4NRMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setNoiseReductionMode, i4NRMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setToneMapMode(MINT32 const i4SensorDev, MINT32 i4ToneMapMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setToneMapMode, i4ToneMapMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(getTonemapCurve_Red, p_in_red, p_out_red, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(getTonemapCurve_Green, p_in_green, p_out_green, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(getTonemapCurve_Blue, p_in_blue, p_out_blue, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setTonemapCurve_Red, p_in_red, p_out_red, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setTonemapCurve_Green, p_in_green, p_out_green, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(setTonemapCurve_Blue, p_in_blue, p_out_blue, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::convertPtPairsToGMA(MINT32 const i4SensorDev, const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(), i4SensorDev(0x%x)\n", __FUNCTION__, i4SensorDev);

    PARAMCTRL_SENSORDEV_INSTANCE(convertPtPairsToGMA, inPtPairs, u4NumOfPts, outGMA);

    return MTRUE;
}

const char *IspTuningMgr::getIspProfileName(EIspProfile_T IspProfile)
{
    switch (IspProfile)
    {
    case EIspProfile_Preview:                           return "Preview";
    case EIspProfile_Video:                             return "Video";
    case EIspProfile_Capture:                           return "Capture";
    case EIspProfile_iHDR_Preview:                      return "iHDR_Preview";
    case EIspProfile_zHDR_Preview:                      return "zHDR_Preview";
    case EIspProfile_mHDR_Preview:                      return "mHDR_Preview";
    case EIspProfile_iHDR_Video:                        return "iHDR_Video";
    case EIspProfile_zHDR_Video:                        return "zHDR_Video";
    case EIspProfile_mHDR_Video:                        return "mHDR_Video";
    case EIspProfile_iHDR_Preview_VSS:                  return "iHDR_Preview_VSS";
    case EIspProfile_zHDR_Preview_VSS:                  return "zHDR_Preview_VSS";
    case EIspProfile_mHDR_Preview_VSS:                  return "mHDR_Preview_VSS";
    case EIspProfile_iHDR_Video_VSS:                    return "iHDR_Video_VSS";
    case EIspProfile_zHDR_Video_VSS:                    return "zHDR_Video_VSS";
    case EIspProfile_mHDR_Video_VSS:                    return "mHDR_Video_VSS";
    case EIspProfile_zHDR_Capture:                      return "zHDR_Capture";
    case EIspProfile_mHDR_Capture:                      return "mHDR_Capture";
    case EIspProfile_Auto_iHDR_Preview:                 return "Auto_iHDR_Preview";
    case EIspProfile_Auto_zHDR_Preview:                 return "Auto_zHDR_Preview";
    case EIspProfile_Auto_mHDR_Preview:                 return "Auto_mHDR_Preview";
    case EIspProfile_Auto_iHDR_Video:                   return "Auto_iHDR_Video";
    case EIspProfile_Auto_zHDR_Video:                   return "Auto_zHDR_Video";
    case EIspProfile_Auto_mHDR_Video:                   return "Auto_mHDR_Video";
    case EIspProfile_Auto_iHDR_Preview_VSS:             return "Auto_iHDR_Preview_VSS";
    case EIspProfile_Auto_zHDR_Preview_VSS:             return "Auto_zHDR_Preview_VSS";
    case EIspProfile_Auto_mHDR_Preview_VSS:             return "Auto_mHDR_Preview_VSS";
    case EIspProfile_Auto_iHDR_Video_VSS:               return "Auto_iHDR_Video_VSS";
    case EIspProfile_Auto_zHDR_Video_VSS:               return "Auto_zHDR_Video_VSS";
    case EIspProfile_Auto_mHDR_Video_VSS:               return "Auto_mHDR_Video_VSS";
    case EIspProfile_Auto_zHDR_Capture:                 return "Auto_zHDR_Capture";
    case EIspProfile_Auto_mHDR_Capture:                 return "Auto_mHDR_Capture";
    case EIspProfile_MFNR_Before_Blend:                 return "MFNR_Before_Blend";
    case EIspProfile_MFNR_Single:                       return "MFNR_Single";
    case EIspProfile_MFNR_MFB:                          return "MFNR_MFB";
    case EIspProfile_MFNR_After_Blend:                  return "MFNR_After_Blend";
    case EIspProfile_zHDR_Capture_MFNR_Before_Blend:    return "zHDR_Capture_MFNR_Before_Blend";
    case EIspProfile_zHDR_Capture_MFNR_Single:          return "zHDR_Capture_MFNR_Single";
    case EIspProfile_zHDR_Capture_MFNR_MFB:             return "zHDR_Capture_MFNR_MFB";
    case EIspProfile_zHDR_Capture_MFNR_After_Blend:     return "zHDR_Capture_MFNR_After_Blend";
    case EIspProfile_EIS_Preview:                       return "EIS_Preview";
    case EIspProfile_EIS_Video:                         return "EIS_Video";
    case EIspProfile_EIS_iHDR_Preview:                  return "EIS_iHDR_Preview";
    case EIspProfile_EIS_zHDR_Preview:                  return "EIS_zHDR_Preview";
    case EIspProfile_EIS_mHDR_Preview:                  return "EIS_mHDR_Preview";
    case EIspProfile_EIS_iHDR_Video:                    return "EIS_iHDR_Video";
    case EIspProfile_EIS_zHDR_Video:                    return "EIS_zHDR_Video";
    case EIspProfile_EIS_mHDR_Video:                    return "EIS_mHDR_Video";
    case EIspProfile_EIS_Auto_iHDR_Preview:             return "EIS_Auto_iHDR_Preview";
    case EIspProfile_EIS_Auto_zHDR_Preview:             return "EIS_Auto_zHDR_Preview";
    case EIspProfile_EIS_Auto_mHDR_Preview:             return "EIS_Auto_mHDR_Preview";
    case EIspProfile_EIS_Auto_iHDR_Video:               return "EIS_Auto_iHDR_Video";
    case EIspProfile_EIS_Auto_zHDR_Video:               return "EIS_Auto_zHDR_Video";
    case EIspProfile_EIS_Auto_mHDR_Video:               return "EIS_Auto_mHDR_Video";
    case EIspProfile_Capture_MultiPass_HWNR:            return "Capture_MultiPass_HWNR";
    case EIspProfile_YUV_Reprocess:                     return "YUV_Reprocess";
    case EIspProfile_Flash_Capture:                     return "Flash_Capture";
    case EIspProfile_Preview_Capture_ZOOM1:             return "Preview_Capture_ZOOM1";
    case EIspProfile_Preview_Capture_ZOOM2:             return "Preview_Capture_ZOOM2";
    case EIspProfile_Video_Preview_ZOOM1:               return "Video_Preview_ZOOM1";
    case EIspProfile_Video_Preview_ZOOM2:               return "Video_Preview_ZOOM2";
    case EIspProfile_Video_Video_ZOOM1:                 return "Video_Video_ZOOM1";
    case EIspProfile_Video_Video_ZOOM2:                 return "Video_Video_ZOOM2";
    case EIspProfile_Capture_Capture_ZOOM1:             return "Capture_Capture_ZOOM1";
    case EIspProfile_Capture_Capture_ZOOM2:             return "Capture_Capture_ZOOM2";
    case EIspProfile_MFNR_Before_ZOOM1:                 return "MFNR_Before_ZOOM1";
    case EIspProfile_MFNR_Before_ZOOM2:                 return "MFNR_Before_ZOOM2";
    case EIspProfile_MFNR_Single_ZOOM1:                 return "MFNR_Single_ZOOM1";
    case EIspProfile_MFNR_Single_ZOOM2:                 return "MFNR_Single_ZOOM2";
    case EIspProfile_MFNR_MFB_ZOOM1:                    return "MFNR_MFB_ZOOM1";
    case EIspProfile_MFNR_MFB_ZOOM2:                    return "MFNR_MFB_ZOOM2";
    case EIspProfile_MFNR_After_ZOOM1:                  return "MFNR_After_ZOOM1";
    case EIspProfile_MFNR_After_ZOOM2:                  return "MFNR_After_ZOOM2";
    case EIspProfile_Flash_Capture_ZOOM1:               return "Flash_Capture_ZOOM1";
    case EIspProfile_Flash_Capture_ZOOM2:               return "Flash_Capture_ZOOM2";
    case EIspProfile_N3D_Preview:                       return "N3D_Preview";
    case EIspProfile_N3D_Video:                         return "N3D_Video";
    case EIspProfile_N3D_Capture:                       return "N3D_Capture";
    case EIspProfile_N3D_Denoise:                       return "N3D_Denoise";
    //case EIspProfile_N3D_HighRes:                       return "N3D_HighRes";
    case EIspProfile_N3D_Preview_toW:                   return "N3D_Preview_toW";
    case EIspProfile_N3D_Video_toW:                     return "N3D_Video_toW";
    case EIspProfile_N3D_Capture_toW:                   return "N3D_Capture_toW";
    case EIspProfile_N3D_Denoise_toGGM:                 return "N3D_Denoise_toGGM";
    case EIspProfile_N3D_Denoise_toYUV:                 return "N3D_Denoise_toYUV";
    case EIspProfile_N3D_Denoise_toW:                   return "N3D_Denoise_toW";
    //case EIspProfile_N3D_HighRes_toYUV:                 return "N3D_HighRes_toYUV";
    case EIspProfile_N3D_MFHR_Before_Blend:             return "N3D_MFHR_Before_Blend";
    case EIspProfile_N3D_MFHR_Single:                   return "N3D_MFHR_Single";
    case EIspProfile_N3D_MFHR_MFB:                      return "N3D_MFHR_MFB";
    case EIspProfile_N3D_MFHR_After_Blend:              return "N3D_MFHR_After_Blend";
    case EIspProfile_SWHDR_Phase1:                      return "SWHDR_Phase1";
    case EIspProfile_SWHDR_Phase2:                      return "SWHDR_Phase2";
    case EIspProfile_MFNR_Before_ZOOM0:                 return "MFNR_Before_ZOOM0";
    case EIspProfile_MFNR_Single_ZOOM0:                 return "MFNR_Single_ZOOM0";
    case EIspProfile_MFNR_MFB_ZOOM0:                    return "MFNR_MFB_ZOOM0";
    case EIspProfile_MFNR_After_ZOOM0:                  return "MFNR_After_ZOOM0";
    default:                                            return "UnknownProfile";
    }
}
