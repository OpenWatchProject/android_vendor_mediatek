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

#include "property_utils.h"
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

#include <mtkcam/utils/hw/HwTransform.h>
#include "isp_tuning_mgr.h"


using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSCamHW;



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
    , m_pParamctrl_Main3(MNULL)
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
    CAM_LOGD("m_i4SensorDev= %d, m_i4SensorIdx = %d\n", m_i4SensorDev, m_i4SensorIdx);

    getPropInt("vendor.debug.isp_tuning_mgr.enable", &m_bDebugEnable, 0);

    if (i4SensorDev & ESensorDev_Main) {
        if (!m_pParamctrl_Main) {
            m_pParamctrl_Main = IParamctrl::createInstance(ESensorDev_Main, m_i4SensorIdx);
            if (m_pParamctrl_Main) {
                m_pParamctrl_Main->init(i4SubsampleCount);
            }
            else {
                CAM_LOGE("m_pParamctrl_Main is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Main is not NULL");
            return MTRUE;
        }
    }
    if (i4SensorDev & ESensorDev_Sub) {
        if (!m_pParamctrl_Sub) {
            m_pParamctrl_Sub = IParamctrl::createInstance(ESensorDev_Sub, m_i4SensorIdx);
            if (m_pParamctrl_Sub) {
                m_pParamctrl_Sub->init(i4SubsampleCount);
            }
            else {
                CAM_LOGE("m_pParamctrl_Sub is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Sub is not NULL");
            return MTRUE;
        }
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        if (!m_pParamctrl_Main2) {
            m_pParamctrl_Main2 = IParamctrl::createInstance(ESensorDev_MainSecond, m_i4SensorIdx);
            if (m_pParamctrl_Main2) {
                m_pParamctrl_Main2->init(i4SubsampleCount);
            }
            else {
                CAM_LOGE("m_pParamctrl_Main2 is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Main2 is not NULL");
            return MTRUE;
        }
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        if (!m_pParamctrl_Sub2) {
            m_pParamctrl_Sub2 = IParamctrl::createInstance(ESensorDev_SubSecond, m_i4SensorIdx);
            if (m_pParamctrl_Sub2) {
                m_pParamctrl_Sub2->init(i4SubsampleCount);
            }
            else {
                CAM_LOGE("m_pParamctrl_SubSecond is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_SubSecond is not NULL");
            return MTRUE;
        }
    }
    if (i4SensorDev & ESensorDev_MainThird) {
        if (!m_pParamctrl_Main3) {
            m_pParamctrl_Main3 = IParamctrl::createInstance(ESensorDev_MainThird, m_i4SensorIdx);
            if (m_pParamctrl_Main3) {
                m_pParamctrl_Main3->init(i4SubsampleCount);
            }
            else {
                CAM_LOGE("m_pParamctrl_Main3 is NULL");
                return MFALSE;
            }
        }
        else {
            CAM_LOGE("m_pParamctrl_Main3 is not NULL");
            return MTRUE;
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
    if (i4SensorDev & ESensorDev_MainThird) {
        if (m_pParamctrl_Main3) {
            m_pParamctrl_Main3->uninit();
            m_pParamctrl_Main3->destroyInstance();
            m_pParamctrl_Main3 = MNULL;
        }
        else {
            CAM_LOGE("m_pParamctrl_Main3 is NULL");
            return MFALSE;
        }
    }
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
MBOOL IspTuningMgr::setHLRDebugInfo4CCU(MINT32 const i4SensorDev, ISP_NVRAM_HLR_T const HLR, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setHLRDebugInfo4CCU, HLR, rDbgIspInfo);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setLTMnLTMSDebugInfo4CCU(MINT32 const i4SensorDev, ISP_NVRAM_LTM_T const LTM_R1, ISP_NVRAM_LTM_T const LTM_R2, ISP_NVRAM_LTMS_T const LTMS, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(setLTMnLTMSDebugInfo4CCU, LTM_R1, LTM_R2, LTMS, rDbgIspInfo);

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
MBOOL IspTuningMgr::validatePerFrameP1(MINT32 const i4SensorDev, RequestSet_T const RequestSet, RAWIspCamInfo &rP1CamInfo,  MBOOL bReCalc, MINT32 i4SubsampleIdex)
{
    MINT32 i4FrameID = RequestSet.vNumberSet.front();

    CAM_LOGD_IF(m_bDebugEnable,"%s: SensorDev(%d), FrmId(%d)\n", __FUNCTION__, i4SensorDev, i4FrameID);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(validatePerFrameP1, RequestSet, rP1CamInfo, bReCalc, i4SubsampleIdex);

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
MBOOL IspTuningMgr::getDebugInfoP1(MINT32 const i4SensorDev, const RAWIspCamInfo &rP1CamInfo, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, MBOOL const fgReadFromHW) const
{
    CAM_LOGD_IF(m_bDebugEnable,"%s: fgReadFromHW(%d)\n", __FUNCTION__, fgReadFromHW);

    MINT32 ret = 0;
    PARAMCTRL_SENSORDEV_INSTANCE(getDebugInfoP1, rP1CamInfo, rIspExifDebugInfo, fgReadFromHW);

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

    //LCESHO Info
    bufferInfo.LCESHO_Param.bSupport = MTRUE;
    bufferInfo.LCESHO_Param.size.w = ISP_LCESHO_OUT_WD;
    bufferInfo.LCESHO_Param.size.h = ISP_LCESHO_OUT_HT;
    bufferInfo.LCESHO_Param.stride = (ISP_LCESHO_OUT_WD * 2); // depth: 2 byte
    bufferInfo.LCESHO_Param.format = NSCam::eImgFmt_STA_2BYTE;
    bufferInfo.LCESHO_Param.bitDepth = 12;

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
