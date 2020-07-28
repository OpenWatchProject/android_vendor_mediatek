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
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
//#include <awb_param.h>
//#include <af_param.h>
//#include <flash_param.h>
//#include <ae_param.h>
#include <isp_tuning_cam_info.h>
#include <paramctrl_if.h>
#include <isp_mgr.h>
#include <faces.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include "isp_tuning_mgr.h"
#include <private/aaa_utils.h>

#define ABS(a)    ((a) > 0 ? (a) : -(a))
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSCamHW;

#define PARAMCTRL_SENSORDEV_INSTANCE(api,...)\
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);\
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {\
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);\
        return MFALSE;\
    }\
    IParamctrl* pCtrl = m_pParamctrlList[i4SensorIdx].instance;\
    if (pCtrl) {\
        ret = pCtrl->api(__VA_ARGS__);\
    }\
    else {\
        CAM_LOGE("Paramctrl index(%d) is NULL", i4SensorIdx);\
        return MFALSE;\
    }



static NS3Av3::INST_T<IspTuningMgr> singleton;


static std::map<MINT32,NSCamHW::HwMatrix>       mMapMat;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline static
CameraArea_T _transformArea(const MUINT32 i4SensorIdx, const MINT32 i4SensorMode, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }

    CameraArea_T rOut;
    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;

    HwTransHelper helper(i4SensorIdx);
    HwMatrix mat;
    if(!helper.getMatrixFromActive(i4SensorMode, mat))
        CAM_LOGE("Get hw matrix failed");
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;
/*
    ALOGD("[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d) sensormode(%d)", __FUNCTION__,
        input.p.x, input.p.y, input.s.w, input.s.h,
        output.p.x, output.p.y, output.s.w, output.s.h,
        i4SensorMode);
    ALOGD("[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
*/
    return rOut;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline static
CameraArea_T _clipArea(const MINT32 i4TgWidth, const MINT32 i4TgHeight, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    if (i4TgWidth == 0 && i4TgHeight == 0)
    {
        return rArea;
    }
    CameraArea_T rOut;
    MINT32 i4AreaWidth  = rArea.i4Right - rArea.i4Left;
    MINT32 i4AreaHeight = rArea.i4Bottom - rArea.i4Top;
    MINT32 xOffset = 0;
    MINT32 yOffset = 0;

    if(i4AreaWidth > i4TgWidth)
        xOffset = (i4AreaWidth - i4TgWidth) / 2;
    if(i4AreaHeight > i4TgHeight)
        yOffset = (i4AreaHeight - i4TgHeight) / 2;

    rOut.i4Left   = rArea.i4Left + xOffset;
    rOut.i4Top    = rArea.i4Top + yOffset;
    rOut.i4Right  = rArea.i4Right - xOffset;
    rOut.i4Bottom = rArea.i4Bottom - yOffset;
    rOut.i4Weight = rArea.i4Weight;
/*
    CAM_LOGD("[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d) offset(%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom,
        xOffset, yOffset);
*/
    return rOut;
}


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
    :m_i4SensorDev(1)//dev :1,2,4,8...
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
    CAM_LOGD("m_i4SensorDev= %d, m_i4SensorIdx = %d\n", m_i4SensorDev, m_i4SensorIdx);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.isp_tuning_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
            return MFALSE;
        }

    INST_T& rSingleton = m_pParamctrlList[i4SensorIdx];

    Mutex::Autolock _l(rSingleton.lock);
    if(!rSingleton.instance) {
        rSingleton.instance = IParamctrl::createInstance((MUINT32)i4SensorDev, i4SensorIdx);
		
	if(rSingleton.instance) {
	        (rSingleton.instance)->init(i4SubsampleCount);
	}
        else {
                CAM_LOGE("Fail to init Paramctrl instance");
                return MFALSE;
    	}
    }

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::reinitP1TuningMgr(MINT32 const i4SensorDev, MINT32 const i4SubsampleCount)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4SubsampleCount);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(reinitP1TuningMgr, i4SubsampleCount);

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
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4IspProfile);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspProfile, static_cast<EIspProfile_T>(i4IspProfile));

    m_i4IspProfile = i4IspProfile;
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setRequestNumber(MINT32 const i4SensorDev, MINT32 const i4RequestNumber)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4RequestNumber);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setRequestNumber, i4RequestNumber);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSceneMode(MINT32 const i4SensorDev, MUINT32 const u4Scene)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Scene);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setSceneMode, static_cast<EIndex_Scene_T>(u4Scene));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEffect(MINT32 const i4SensorDev, MUINT32 const u4Effect)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Effect);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setEffect, static_cast<EIndex_Effect_T>(u4Effect));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setOperMode(MINT32 const i4SensorDev, MINT32 const i4OperMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4OperMode);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setOperMode, static_cast<EOperMode_T>(i4OperMode));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getOperMode(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = EOperMode_Normal;
    PARAMCTRL_SENSORDEV_INSTANCE(getOperMode, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicBypass(MINT32 const i4SensorDev, MBOOL i4Bypass)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4Bypass);

    MINT32 ret;
    PARAMCTRL_SENSORDEV_INSTANCE(enableDynamicBypass, static_cast<MBOOL>(i4Bypass));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicBypass(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = MFALSE;
    PARAMCTRL_SENSORDEV_INSTANCE(isDynamicBypass, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicCCM(MINT32 const i4SensorDev, MBOOL bdynamic_ccm)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, bdynamic_ccm);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(enableDynamicCCM, static_cast<MBOOL>(bdynamic_ccm));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicCCM(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = MFALSE;
    PARAMCTRL_SENSORDEV_INSTANCE(isDynamicCCM, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::enableDynamicShading(MINT32 const i4SensorDev, MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, fgEnable);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(enableDynamicShading, static_cast<MBOOL>(fgEnable));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSensorMode(MINT32 const i4SensorDev, MINT32 const i4SensorMode, MBOOL const bFrontalBin, MUINT32 const u4RawWidth, MUINT32 const u4RawHeight)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d), FBin: %d, Raw: (%d, %d)\n", __FUNCTION__, i4SensorMode, bFrontalBin, u4RawWidth, u4RawHeight);

    NSCam::MSize RawSize;
    RawSize.w = u4RawWidth;
    RawSize.h = u4RawHeight;
#if 0
    HwTransHelper helper(m_i4SensorIdx);
    NSCamHW::HwMatrix mat;
    if(!helper.getMatrixFromActive(m_bDebugEnable, mat))
        MY_ERR("Get hw matrix failed");
    if(m_bDebugEnable)
        mat.dump(__FUNCTION__);
    mMapMat[m_i4SensorIdx] = mat;
#endif
    MINT32 ret = 0;

    {PARAMCTRL_SENSORDEV_INSTANCE(setSensorMode, static_cast<ESensorMode_T>(i4SensorMode));}
    {PARAMCTRL_SENSORDEV_INSTANCE(setRawSize, bFrontalBin, RawSize);}

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setRawPath(MINT32 const i4SensorDev, MUINT32 const u4RawPath)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s RawPath(%d)\n", __FUNCTION__, u4RawPath);
    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setRawPath, u4RawPath);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getSensorMode(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = ESensorMode_Capture;
    PARAMCTRL_SENSORDEV_INSTANCE(getSensorMode, );

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setZoomRatio(MINT32 const i4SensorDev, MINT32 const i4ZoomRatio_x100)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4ZoomRatio_x100);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setZoomRatio, i4ZoomRatio_x100);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBInfo2ISP(MINT32 const i4SensorDev, AWB_ISP_INFO_T const &rAWBInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s() RGB(%d, %d, %d)\n", __FUNCTION__, rAWBInfo.rRPG.i4R, rAWBInfo.rRPG.i4G, rAWBInfo.rRPG.i4B);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setAWBInfo2ISP, rAWBInfo);
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAEInfo2ISP(MINT32 const i4SensorDev, AE_ISP_INFO_T const &rAEInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setAEInfo2ISP, rAEInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setCCUInfo2ISP(MINT32 const i4SensorDev, ISP_CCU_RESULT_T const &rCCUInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setCCUInfo2ISP, rCCUInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFlashInfo(MINT32 const i4SensorDev, MUINT32 const u4FlashMappingInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setFlashInfo, u4FlashMappingInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setPureOBCInfo(MINT32 const i4SensorDev, const ISP_NVRAM_OBC_T *pOBCInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setPureOBCInfo, pOBCInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setRRZInfo(MINT32 const i4SensorDev, const ISP_NVRAM_RRZ_T *pRRZInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setRRZInfo, pRRZInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDGNDebugInfo4CCU(MINT32 const i4SensorDev, MUINT32 const u4Rto, MUINT32 const u4P1DGNGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setDGNDebugInfo4CCU, u4Rto, u4P1DGNGain, rDbgIspInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setHLRDebugInfo4CCU(MINT32 const i4SensorDev, MBOOL Enable, ISP_NVRAM_HLR_T const HLR, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setHLRDebugInfo4CCU, Enable, HLR, rDbgIspInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setLTMnLTMSDebugInfo4CCU(MINT32 const i4SensorDev, MBOOL Enable, ISP_NVRAM_LTM_T const LTM_R1, ISP_NVRAM_LTM_T const LTM_R2, ISP_NVRAM_LTMS_T const LTMS, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setLTMnLTMSDebugInfo4CCU, Enable, LTM_R1, LTM_R2, LTMS, rDbgIspInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIndex_Shading(MINT32 const i4SensorDev, MINT32 const i4IDX)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, i4IDX);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIndex_Shading, i4IDX);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getIndex_Shading(MINT32 const i4SensorDev, MVOID*const pCmdArg)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getIndex_Shading, pCmdArg);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getCamInfo(MINT32 const i4SensorDev, RAWIspCamInfo& ret)const
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    PARAMCTRL_SENSORDEV_INSTANCE(getCamInfo, );

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Edge(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Index);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Edge, static_cast<EIndex_Isp_Edge_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Hue(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Index);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Hue, static_cast<EIndex_Isp_Hue_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Sat(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Index);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Sat, static_cast<EIndex_Isp_Saturation_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Bright(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Index);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Bright, static_cast<EIndex_Isp_Brightness_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Contrast(MINT32 const i4SensorDev, MUINT32 const u4Index)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s (%d)\n", __FUNCTION__, u4Index);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspUserIdx_Contrast, static_cast<EIndex_Isp_Contrast_T>(u4Index));

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrameP1(MINT32 const i4SensorDev, RequestSet_T const RequestSet,  MBOOL bReCalc, MINT32 i4SubsampleIdex)
{
    MINT32 i4FrameID = RequestSet.vNumberSet.front();

    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), FrmId(%d)\n", __FUNCTION__, i4SensorDev, i4FrameID);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(validatePerFrameP1, RequestSet, bReCalc, i4SubsampleIdex);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrameP2(MINT32 const i4SensorDev, MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(validatePerFrameP2, flowType, rIspInfo, pTuningBuf);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfoP1(MINT32 const i4SensorDev, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, MBOOL const fgReadFromHW) const
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: fgReadFromHW(%d)\n", __FUNCTION__, fgReadFromHW);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getDebugInfoP1, rIspExifDebugInfo, fgReadFromHW);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfoP2(MINT32 const i4SensorDev, const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, const void* pTuningBuf)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d)\n", __FUNCTION__, i4SensorDev);

    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    void* pMfbBuf = ((TuningParam*)pTuningBuf)->pMfbBuf;

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getDebugInfoP2, rIspInfo, rIspExifDebugInfo, pRegBuf, pMfbBuf);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::notifyRPGEnable(MINT32 const i4SensorDev, MBOOL const bRPGEnable)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: (%d)\n", __FUNCTION__, bRPGEnable);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(notifyRPGEnable, bRPGEnable);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::notifyIMGOType(MINT32 const i4SensorDev, MBOOL const bIMGO_RawType)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: (%d)\n", __FUNCTION__, bIMGO_RawType);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(notifyIMGOType, bIMGO_RawType);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IspTuningMgr::getDMGItable(MINT32 const i4SensorDev, MBOOL const fgRPGEnable)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d)\n", __FUNCTION__, i4SensorDev);

    MVOID* ret = NULL;
    PARAMCTRL_SENSORDEV_INSTANCE(getDMGItable, fgRPGEnable);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspAEPreGain2(MINT32 const i4SensorDev, MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), i4SensorIndex(%d)\n", __FUNCTION__, i4SensorDev, i4SensorIndex);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setIspAEPreGain2, i4SensorIndex, rNewIspAEPreGain2);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), i4TGInfo(%d), i4Width(%d), i4Height(%d) \n", __FUNCTION__, i4SensorDev, i4TGInfo, i4Width, i4Height);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setTGInfo, i4TGInfo, i4Width, i4Height);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFDEnable(MINT32 const i4SensorDev, MBOOL const bFDenable)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), bFDenable(%d)\n", __FUNCTION__, i4SensorDev, bFDenable);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setFDEnable, bFDenable);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFDInfo(MINT32 const i4SensorDev, MVOID* const a_sFaces, MUINT32 const TGWidth, MUINT32 const TGHeight)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    MINT32 i4SensorMode = this->getSensorMode(i4SensorDev);

    CAMERA_TUNING_FD_INFO_T fd_tuning_info;
    memset(&fd_tuning_info, 0, sizeof(CAMERA_TUNING_FD_INFO_T));
    fd_tuning_info.FaceNum = pFaces->number_of_faces;

    static_assert( sizeof(pFaces->YUVsts) <= sizeof(fd_tuning_info.YUVsts),"face struct YUVsts size error");
    static_assert( sizeof(pFaces->GenderLabel) <= sizeof(fd_tuning_info.fld_GenderLabel),"face struct fld_GenderLabel size error");
    static_assert( sizeof(pFaces->fld_GenderInfo) <= sizeof(fd_tuning_info.fld_GenderInfo),"face struct fld_GenderInfo size error");
    static_assert( sizeof(pFaces->fld_rop) <= sizeof(fd_tuning_info.fld_rop),"face struct fld_rop size error");


    memcpy(&(fd_tuning_info.YUVsts), &(pFaces->YUVsts), sizeof(pFaces->YUVsts));
    memcpy(&(fd_tuning_info.fld_GenderLabel), &(pFaces->GenderLabel), sizeof(pFaces->GenderLabel));
    memcpy(&(fd_tuning_info.fld_GenderInfo), &(pFaces->fld_GenderInfo), sizeof(pFaces->fld_GenderInfo));
    memcpy(&(fd_tuning_info.fld_rop), &(pFaces->fld_rop), sizeof(pFaces->fld_rop));
    memcpy(&(fd_tuning_info.Landmark_CV), &(pFaces->fa_cv), sizeof(pFaces->fa_cv));

    fd_tuning_info.FaceNum = pFaces->number_of_faces;
    fd_tuning_info.GenderNum = pFaces->genderNum;
    fd_tuning_info.LandmarkNum = pFaces->poseNum;

    fd_tuning_info.FaceGGM_Idx = pFaces->gmIndex;

    if((pFaces->gmSize) == GGM_LUT_SIZE)
        for(int i=0; i< (pFaces->gmSize); i++ ){
            fd_tuning_info.FaceGGM.lut[i].bits.GGM_R
                = fd_tuning_info.FaceGGM.lut[i].bits.GGM_G
                = fd_tuning_info.FaceGGM.lut[i].bits.GGM_B
                = pFaces->gmData[i];
    }

    MUINT32 FaceArea = 0;
    MUINT32 temp_area = 0;

    if (pFaces->number_of_faces != 0)
    {
        // calculate face TG size
        int i = 0;
        MINT32 i4TgWidth = TGWidth;
        MINT32 i4TgHeight = TGHeight;
        CameraArea_T rArea;

        for(i = 0; i < pFaces->number_of_faces; i++) {
            fd_tuning_info.fld_rip[i] = pFaces->posInfo[i].rip_dir;

            // face
            rArea.i4Left   = pFaces->faces[i].rect[0];  //Left
            rArea.i4Top    = pFaces->faces[i].rect[1];  //Top
            rArea.i4Right  = pFaces->faces[i].rect[2];  //Right
            rArea.i4Bottom = pFaces->faces[i].rect[3];  //Bottom
            rArea.i4Weight = 0;

            //TG Domain
            temp_area = ABS(rArea.i4Right - rArea.i4Left)* ABS(rArea.i4Bottom - rArea.i4Top);
            FaceArea += temp_area;

            rArea = _transformArea(m_i4SensorIdx, i4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);

            CAM_LOGD_IF(m_bDebugEnable,"After _transformArea x0(%d), y0(%d), x1(%d), y1(%d)", rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);

            fd_tuning_info.rect[i][0] = rArea.i4Left;
            fd_tuning_info.rect[i][1] = rArea.i4Top;
            fd_tuning_info.rect[i][2] = rArea.i4Right;
            fd_tuning_info.rect[i][3] = rArea.i4Bottom;

            if(pFaces->fa_cv[i] > 0) // 0 is invalid value
            {
                // left eye
                rArea.i4Left   = pFaces->leyex0[i];  //Left
                rArea.i4Top    = pFaces->leyey0[i];  //Top
                rArea.i4Right  = pFaces->leyex1[i];  //Right
                rArea.i4Bottom = pFaces->leyey1[i];  //Bottom
                rArea = _transformArea(m_i4SensorIdx, i4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);

                fd_tuning_info.Face_Leye[i][0] = rArea.i4Left;
                fd_tuning_info.Face_Leye[i][1] = rArea.i4Top;
                fd_tuning_info.Face_Leye[i][2] = rArea.i4Right;
                fd_tuning_info.Face_Leye[i][3] = rArea.i4Bottom;

                // right eye
                rArea.i4Left   = pFaces->reyex0[i];  //Left
                rArea.i4Top    = pFaces->reyey0[i];  //Top
                rArea.i4Right  = pFaces->reyex1[i];  //Right
                rArea.i4Bottom = pFaces->reyey1[i];  //Bottom
                rArea = _transformArea(m_i4SensorIdx, i4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                fd_tuning_info.Face_Reye[i][0] = rArea.i4Left;
                fd_tuning_info.Face_Reye[i][1] = rArea.i4Top;
                fd_tuning_info.Face_Reye[i][2] = rArea.i4Right;
                fd_tuning_info.Face_Reye[i][3] = rArea.i4Bottom;
            }
         }
    }



    for(int i = 0; i < pFaces->number_of_faces; i++){

    }

    //Face Ratio
    MUINT32 TGSize = TGWidth*TGHeight;
    float FDRatio = (float)FaceArea / (float)TGSize;
    if( FDRatio > 1.0 ){
        FDRatio = 1.0;
    }
    fd_tuning_info.FDRatio = FDRatio;

    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), TGSize(%d)\n", __FUNCTION__, i4SensorDev, TGSize);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setFDInfo, fd_tuning_info);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDefaultObc(MINT32 const i4SensorDev, MVOID*& pISP_NVRAM_Reg)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getDefaultObc, pISP_NVRAM_Reg);

    return MTRUE;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::sendIspTuningIOCtrl(MINT32 const i4SensorDev, E_ISPTUNING_CTRL const ctrl, MINTPTR arg1, MINTPTR arg2)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), CTRL(%d)\n", __FUNCTION__, i4SensorDev, ctrl);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(sendIspTuningIOCtrl, ctrl, arg1, arg2);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::forceValidate(MINT32 const i4SensorDev)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(forceValidate, );

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

    MINT32 ret = 0;
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

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setColorCorrectionTransform, M11, M12, M13, M21, M22, M23, M31, M32, M33);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4ColorCorrectionMode);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setColorCorrectionMode, i4ColorCorrectionMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEdgeMode(MINT32 const i4SensorDev, MINT32 i4EdgeMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4EdgeMode);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setEdgeMode, i4EdgeMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setNoiseReductionMode(MINT32 const i4SensorDev, MINT32 i4NRMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4NRMode);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setNoiseReductionMode, i4NRMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setToneMapMode(MINT32 const i4SensorDev, MINT32 i4ToneMapMode)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4ToneMapMode);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setToneMapMode, i4ToneMapMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getTonemapCurve_Red, p_in_red, p_out_red, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getTonemapCurve_Green, p_in_green, p_out_green, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getTonemapCurve_Blue, p_in_blue, p_out_blue, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setTonemapCurve_Red, p_in_red, p_out_red, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setTonemapCurve_Green, p_in_green, p_out_green, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setTonemapCurve_Blue, p_in_blue, p_out_blue, pCurvePointNum);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFdGamma(MINT32 const i4SensorDev, MINT32 i4Index, MINT32* fdGammaCurve, MUINT32 u4size)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setFdGamma, i4Index, fdGammaCurve, u4size);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::convertPtPairsToGMA(MINT32 const i4SensorDev, const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4SensorDev);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(convertPtPairsToGMA, inPtPairs, u4NumOfPts, outGMA);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 IspTuningMgr::getZoomIdx(MINT32 const i4SensorDev, MUINT32 const i4ZoomRatio_x100)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s(%d)\n", __FUNCTION__, i4ZoomRatio_x100);

    EZoom_T ret = EZoom_IDX_00;
    PARAMCTRL_SENSORDEV_INSTANCE(map_Zoom_value2index, i4ZoomRatio_x100);

    return (MUINT32)ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL IspTuningMgr::queryISPBufferInfo(MINT32 const m_i4SensorDev, Buffer_Info& bufferInfo)
{
    //LCESO Info
    bufferInfo.LCESO_Param.bSupport = MTRUE;
    bufferInfo.LCESO_Param.size.w = ISP_LCS_OUT_WD;
    bufferInfo.LCESO_Param.size.h = ISP_LCS_OUT_HT;
    bufferInfo.LCESO_Param.stride = (ISP_LCS_OUT_WD * 2); // depth: 2 byte
    bufferInfo.LCESO_Param.format = NSCam::eImgFmt_STA_2BYTE;
    bufferInfo.LCESO_Param.bitDepth = 12;

    //DCESO Info
    bufferInfo.DCESO_Param.bSupport = MTRUE;
    bufferInfo.DCESO_Param.size.w = ISP_DCS_OUT_WD;
    bufferInfo.DCESO_Param.size.h = ISP_DCS_OUT_HT;
    bufferInfo.DCESO_Param.stride = (ISP_DCS_OUT_WD * 4); // depth: 4 byte
    bufferInfo.DCESO_Param.format = NSCam::eImgFmt_STA_4BYTE;
    bufferInfo.DCESO_Param.bitDepth = 32;

    bufferInfo.u4DualSyncInfoSize = sizeof(DUAL_ISP_SYNC_INFO_T) ;

    return MTRUE;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL IspTuningMgr::setP1DirectYUV_Port(MINT32 const i4SensorDev, MUINT32 const u4P1DirectYUV_Port)
{
    MBOOL ret = MTRUE;

    PARAMCTRL_SENSORDEV_INSTANCE(setP1DirectYUV_Port, u4P1DirectYUV_Port);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// following header file is generated by Android.mk using LOCAL_GENERATED_SOURCES
#include <EIspProfile_string.h>
const char *IspTuningMgr::getIspProfileName(EIspProfile_T IspProfile)
{
    if (IspProfile < 0) {
        return NULL;
    } else if ((int)IspProfile >= (int)(sizeof(strEIspProfile)/sizeof(char*))) {
        return NULL;
    } else {
        return strEIspProfile[IspProfile];
    }
}
