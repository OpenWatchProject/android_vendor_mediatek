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
#define LOG_TAG "ae_cct_feature"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

//#include <CamDefs.h>
#include <sys/stat.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_hal_if.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <flash_param.h>
//
#include <ae_tuning_custom.h>
//#include <isp_mgr.h>
//#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <cct_feature.h>
#include <nvbuf_util.h>
#include <flash_mgr.h>
#include <cutils/properties.h>

using namespace NS3Av3;
//using namespace NSIspTuningv3;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::
CCTOPAEEnable()
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_ENABLE_AUTO_RUN]\n");

    m_bEnableAE = MTRUE;
    
    CAM_LOGD( "enableAE()\n");
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::
CCTOPAEDisable()
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_DISABLE_AUTO_RUN]\n");

    m_bEnableAE = MFALSE;
    
    CAM_LOGD( "disableAE()\n");
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEGetEnableInfo(
    MINT32 *a_pEnableAE,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_OP_AE_GET_ENABLE_INFO]\n");

    *a_pEnableAE = m_bEnableAE;
    *a_pOutLen = sizeof(MINT32);

    CAM_LOGD("AE Enable = %d\n", *a_pEnableAE);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPSetAETargetValue(
    MUINT32 u4AETargetValue
)
{
    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_SET_TARGET]\n");

    m_rAEInitInput.rAENVRAM->rCCTConfig.u4AETarget = u4AETargetValue;
    
    CAM_LOGD("[AE Target] = %d\n", u4AETargetValue);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAESetSceneMode(
    MINT32 a_AEScene
)
{
    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE]\n");

    setSceneMode((MUINT32) a_AEScene);

    CAM_LOGD("[AE Scene] = %d\n", a_AEScene);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEGetAEScene(
    MINT32 *a_pAEScene,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_GET_SCENE_MODE]\n");

    *a_pAEScene = m_eAEScene;

    *a_pOutLen = sizeof(MINT32);

    CAM_LOGD("[AE Scene] = %d\n", *a_pAEScene);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAESetMeteringMode(
    MINT32 a_AEMeteringMode
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_SET_METERING_MODE]\n");

    setAEMeteringMode((MUINT32)a_AEMeteringMode);

    CAM_LOGD("[AE Metering Mode] = %d\n", a_AEMeteringMode);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEApplyExpParam(
    MVOID *a_pAEExpParam
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO]\n");
    ACDK_AE_MODE_CFG_T *pAEExpParam = (ACDK_AE_MODE_CFG_T *)a_pAEExpParam;
    strAERealSetting rAERealSetting;
    
    memset(&rAERealSetting, 0, sizeof(strAERealSetting));

    // Set exposure mode
    rAERealSetting.u4ExposureMode = pAEExpParam->u4ExposureMode;
    // Set exposure time
    rAERealSetting.EvSetting.u4Eposuretime = pAEExpParam->u4Eposuretime;   // updateExpoSettingbyI2C decide send exposure time or exposure line?

    // Set sensor gain and DNG gain
    if (pAEExpParam->u4GainMode == 0) { // AfeGain and isp gain
        rAERealSetting.EvSetting.u4AfeGain = pAEExpParam->u4AfeGain;
        rAERealSetting.EvSetting.u4IspGain = pAEExpParam->u4IspGain;
        rAERealSetting.u4ISO = (m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain * rAERealSetting.EvSetting.u4AfeGain)/1024 * rAERealSetting.EvSetting.u4IspGain/1024;
    } else { // ISO
        if(m_p3ANVRAM != NULL) {
            rAERealSetting.EvSetting.u4AfeGain = (pAEExpParam->u4ISO * AE_GAIN_BASE_AFE) / m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        } else {
            CAM_LOGD("NVRAM is NULL\n");
            rAERealSetting.EvSetting.u4AfeGain = AE_GAIN_BASE_AFE;
        }
        rAERealSetting.EvSetting.u4IspGain = AE_GAIN_BASE_ISP;
        rAERealSetting.u4ISO = pAEExpParam->u4ISO;
    }

    CAM_LOGD("[Exp Mode] = %d->%d\n", pAEExpParam->u4ExposureMode, rAERealSetting.u4ExposureMode);
    CAM_LOGD("[Exp Time] = %d->%d\n", pAEExpParam->u4Eposuretime, rAERealSetting.EvSetting.u4Eposuretime);
    CAM_LOGD("[ISO] = %d->%d\n", pAEExpParam->u4ISO, rAERealSetting.u4ISO);
    CAM_LOGD("[AFE Gain] = %d->%d\n", rAERealSetting.EvSetting.u4AfeGain, rAERealSetting.EvSetting.u4AfeGain);
    CAM_LOGD("[Isp Gain] = %d->%d\n", rAERealSetting.EvSetting.u4IspGain, rAERealSetting.EvSetting.u4IspGain);

    if (m_eSettingType == E_AE_SETTING_CPU)
        controlCPUExpoSetting(rAERealSetting);
    else
        controlCCUExpoSetting(rAERealSetting, rAERealSetting);
    updateAEInfo2ISP(AE_FLOW_STATE_PREVIEW, rAERealSetting, rAERealSetting);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOSetCaptureParams(
    MVOID *a_pAEExpParam
)
{
    CAM_LOGD("[CCTOSetCaptureParams]\n");
    AE_MODE_CFG_T cap_mode;
    ACDK_AE_MODE_CFG_T *pcfg_in = (ACDK_AE_MODE_CFG_T *)a_pAEExpParam;

    cap_mode.u4ExposureMode = pcfg_in->u4ExposureMode;
    cap_mode.u4Eposuretime = pcfg_in->u4Eposuretime;

    // Set sensor gain
    if (pcfg_in->u4GainMode == 0) { // AfeGain and isp gain
        cap_mode.u4AfeGain = pcfg_in->u4AfeGain;
        cap_mode.u4IspGain = pcfg_in->u4IspGain;
    }
    else { // ISO
        if(m_p3ANVRAM != NULL) {
            cap_mode.u4AfeGain = (pcfg_in->u4ISO * AE_GAIN_BASE_AFE) / m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        } else {
            CAM_LOGD("NVRAM is NULL\n");
            cap_mode.u4AfeGain = AE_GAIN_BASE_AFE;
        }
        cap_mode.u4IspGain = AE_GAIN_BASE_ISP;
    }

    cap_mode.u2FrameRate = pcfg_in->u2FrameRate;
    cap_mode.u4RealISO = pcfg_in->u4ISO;

    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. Exp Mode = %d\n", cap_mode.u4ExposureMode);
    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. Exp Time = %d\n", cap_mode.u4Eposuretime);
    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. Gain Mode = %d\n", pcfg_in->u4GainMode);
    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. Afe Gain = %d\n", cap_mode.u4AfeGain);
    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. Isp Gain = %d\n", cap_mode.u4IspGain);
    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. Frame Rate = %d\n", cap_mode.u2FrameRate);
    CAM_LOGD("[CCTOSetCaptureParams] -- Cap. ISO = %d\n", cap_mode.u4RealISO);
    
    updateCaptureParams(cap_mode);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOGetCaptureParams(
    MVOID *a_pAEExpParam
)
{
    CAM_LOGD("[CCTOGetCaptureParams]\n");
    ACDK_AE_MODE_CFG_T *pout_cfg = (ACDK_AE_MODE_CFG_T *)a_pAEExpParam;
    AE_MODE_CFG_T ae_mode;
    
    getCaptureParams(ae_mode);
    
    CAM_LOGD("[CCTOGetCaptureParams] -- Cap. Exp Mode = %d\n", ae_mode.u4ExposureMode);
    CAM_LOGD("[CCTOGetCaptureParams] -- Cap Exp Time = %d\n", ae_mode.u4Eposuretime);
    CAM_LOGD("[CCTOGetCaptureParams] -- Cap Afe Gain = %d\n", ae_mode.u4AfeGain);
    CAM_LOGD("[CCTOGetCaptureParams] -- Cap Isp Gain = %d\n", ae_mode.u4IspGain);
    CAM_LOGD("[CCTOGetCaptureParams] -- Cap Frame Rate = %d\n", ae_mode.u2FrameRate);
    CAM_LOGD("[CCTOGetCaptureParams] -- Cap ISO = %d\n", ae_mode.u4RealISO);
    
    pout_cfg->u4ExposureMode = ae_mode.u4ExposureMode;
    pout_cfg->u4Eposuretime = ae_mode.u4Eposuretime;
    pout_cfg->u4GainMode = 0;
    pout_cfg->u4AfeGain = ae_mode.u4AfeGain;
    pout_cfg->u4IspGain = ae_mode.u4IspGain;
    pout_cfg->u4ISO = ae_mode.u4RealISO;
    pout_cfg->u2FrameRate = ae_mode.u2FrameRate;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAESetFlickerMode(
    MINT32 a_AEFlickerMode
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_SELECT_BAND]\n");

    setAEFlickerMode((MUINT32) a_AEFlickerMode);

    CAM_LOGD("[AE Flicker Mode] = %d\n", a_AEFlickerMode);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEGetExpParam(
    MVOID *a_pAEExpParamIn,
    MVOID *a_pAEExpParamOut,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA]\n");
    AE_MODE_CFG_T ae_mode;

    memset(&ae_mode, 0, sizeof(AE_MODE_CFG_T));

    getPreviewParams(ae_mode);

    ACDK_AE_MODE_CFG_T *pAEExpParamIn = (ACDK_AE_MODE_CFG_T *)a_pAEExpParamIn;
    ACDK_AE_MODE_CFG_T *pAEExpParamOut = (ACDK_AE_MODE_CFG_T *)a_pAEExpParamOut;

    pAEExpParamOut->u4GainMode = pAEExpParamIn->u4GainMode;
    CAM_LOGD("[Gain Mode] = %d\n", pAEExpParamOut->u4GainMode);
    pAEExpParamOut->u4AfeGain = ae_mode.u4AfeGain;
    CAM_LOGD("[AFE Gain] = %d\n", pAEExpParamOut->u4AfeGain);
    pAEExpParamOut->u4IspGain = ae_mode.u4IspGain;
    CAM_LOGD("[Isp Gain] = %d\n", pAEExpParamOut->u4IspGain);
    
     if(m_p3ANVRAM != NULL) {
         pAEExpParamOut->u4ISO = ae_mode.u4IspGain*(((ae_mode.u4AfeGain * m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain) + AE_GAIN_BASE_AFE>>1) / AE_GAIN_BASE_AFE) / AE_GAIN_BASE_AFE;
     } else {
         CAM_LOGD("NVRAM is NULL\n");
         pAEExpParamOut->u4ISO =  ae_mode.u4IspGain*(((ae_mode.u4AfeGain * 100) + AE_GAIN_BASE_AFE>>1) / AE_GAIN_BASE_AFE) / AE_GAIN_BASE_AFE;
     }
     CAM_LOGD("[ISO] = %d\n", pAEExpParamOut->u4ISO);
     pAEExpParamOut->u4ExposureMode = ae_mode.u4ExposureMode;
     CAM_LOGD("[Exp Mode] = %d\n", pAEExpParamOut->u4ExposureMode);
     pAEExpParamOut->u4Eposuretime = ae_mode.u4Eposuretime;
     CAM_LOGD("[Exp Time] = %d\n", pAEExpParamOut->u4Eposuretime);

     *a_pOutLen = sizeof(ACDK_AE_MODE_CFG_T);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEGetFlickerMode(
    MINT32 *a_pAEFlickerMode,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_GET_BAND]\n");

    *a_pAEFlickerMode = m_eAEFlickerMode;

    *a_pOutLen = sizeof(MUINT32);

    CAM_LOGD("[AE Flicker Mode] = %d\n", *a_pAEFlickerMode);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEGetMeteringMode(
    MINT32 *a_pAEMEteringMode,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AE_GET_METERING_RESULT]\n");

    *a_pAEMEteringMode = m_eAEMeterMode;

    *a_pOutLen = sizeof(MUINT32);

    CAM_LOGD("[AE Metering Mode] = %d\n", *a_pAEMEteringMode);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEApplyNVRAMParam(
    MVOID *a_pAENVRAM,
    MUINT32 u4AENVRAMIdx
)
{
    AE_NVRAM_T *pAENVRAM = reinterpret_cast<AE_NVRAM_T*>(a_pAENVRAM);
    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_APPLY_INFO] u4AENVRAMIdx(%d) Max:%d \n", u4AENVRAMIdx, AE_CAM_SCENARIO_NUM);
    if(u4AENVRAMIdx < AE_CAM_SCENARIO_NUM) {
        // set nvram
        m_p3ANVRAM->AE[u4AENVRAMIdx] = *pAENVRAM;
        setNVRAMIndex(u4AENVRAMIdx, MFALSE);    
    } else {
        CAM_LOGE("[%s] u4AENVRAMIdx too big\n", __FUNCTION__);
        m_p3ANVRAM->AE[0] = *pAENVRAM;
        setNVRAMIndex(0, MFALSE);        
    }
    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_TRIGGER_FRAME_INFO_UPDATE, MFALSE);
    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_APPLY_INFO - reschedule]\n");

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAEGetNVRAMParam(
    MVOID *a_pAENVRAM,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[%s][ACDK_CCT_OP_DEV_AE_GET_INFO]\n", __FUNCTION__);

    if(a_pAENVRAM == NULL) {
        CAM_LOGE("[%s] a_pAFNVRAM is Null\n", __FUNCTION__);
    }

    getNvramData(m_eSensorDev, MTRUE);

    memcpy(a_pAENVRAM, m_p3ANVRAM->AE, sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM);
    *a_pOutLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAESaveNVRAMParam(
)
{
    MUINT32 u4SensorID;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum;
    MRESULT err = S_AE_OK;

    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM]\n");

    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, m_eSensorDev);
    if (err != 0)
    {
        CAM_LOGE("NvBufUtil::getInstance().write\n");
        return E_NVRAM_BAD_PARAM;
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AeMgr::CCTOPAEApplyPlineNVRAM(
    MVOID *a_pAEPlineNVRAM
)
{
    char value[256] = {'\0'};
    MBOOL dumpEnable;
    property_get("vendor.debug.cct_aepline_apply.enable", value, "0");
    dumpEnable = atoi(value);

    if(dumpEnable)
    {
        char fileName[64];
        sprintf(fileName, "/data/AEPline_apply_before.bin");
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/data/", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
            return MFALSE;
        }
        CAM_LOGD("%s\n", fileName);
        fwrite(m_rAEInitInput.rAEPlineTable, 1, sizeof(AE_PLINETABLE_T), fp);
        fclose(fp);
    }

    AE_PLINETABLE_T *pAEPlineNVRAM = reinterpret_cast<AE_PLINETABLE_T*>(a_pAEPlineNVRAM);

    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_APPLY_PLINE] \n");

    memcpy(m_rAEInitInput.rAEPlineTable, a_pAEPlineNVRAM, sizeof(AE_PLINETABLE_T));
    bApplyAEPlineTableByCCT = MTRUE;

    if(dumpEnable)
    {
        char fileName[64];
        sprintf(fileName, "/data/AEPline_apply_after.bin");
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/data/", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
            return MFALSE;
        }
        CAM_LOGD("%s\n", fileName);
        fwrite(m_rAEInitInput.rAEPlineTable, 1, sizeof(AE_PLINETABLE_T), fp);
        fclose(fp);
    }

    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].eAEScene);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[0]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[1]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[2]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[3]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[4]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[5]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[6]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[7]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[8]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[9]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[10]);

    CAM_LOGD("Preview ID = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].eID);
    CAM_LOGD("Preview Total Idx = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].u4TotalIndex);
    CAM_LOGD("Preview StrobeBV = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].i4StrobeTrigerBV);
    CAM_LOGD("Preview i4MaxBV = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].i4MaxBV);
    CAM_LOGD("Preview i4MinBV = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].i4MinBV);
    CAM_LOGD("Preview ISO = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].ISOSpeed);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4IspGain);

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAEGetPlineNVRAM(
    MVOID *a_pAEPlineNVRAM,
    MUINT32 *a_pOutLen
)
{
    char value[256] = {'\0'};
    MBOOL dumpEnable;
    property_get("vendor.debug.cct_aepline_apply.enable", value, "0");
    dumpEnable = atoi(value);

    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_GET_PLINE]\n");

    AE_PLINETABLE_T *pAEPlineNVRAM = reinterpret_cast<AE_PLINETABLE_T*>(a_pAEPlineNVRAM);

    getNvramData(m_eSensorDev, MTRUE);

    if(pAEPlineNVRAM != NULL) {
        *pAEPlineNVRAM = *m_pAEPlineTable;
    } else {
        CAM_LOGD("AE Pline NVRAM is NULL\n");
    }

    if(pAEPlineNVRAM != NULL) {
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].eAEScene);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[0]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[1]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[2]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[3]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[4]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[5]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[6]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[7]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[8]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[9]);
    CAM_LOGD("Mapping0[0] = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[10]);

    CAM_LOGD("Preview ID = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].eID);
    CAM_LOGD("Preview Total Idx = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].u4TotalIndex);
    CAM_LOGD("Preview StrobeBV = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].i4StrobeTrigerBV);
    CAM_LOGD("Preview i4MaxBV = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].i4MaxBV);
    CAM_LOGD("Preview i4MinBV = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].i4MinBV);
    CAM_LOGD("Preview ISO = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].ISOSpeed);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", ((AE_PLINETABLE_T*)pAEPlineNVRAM)->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4IspGain);
    }

    *a_pOutLen = sizeof(AE_PLINETABLE_T);

 if(dumpEnable)
    {
        char fileName[64];
        sprintf(fileName, "/data/AEPline_get.bin");
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/data/", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
            return MFALSE;
        }
        CAM_LOGD("%s\n", fileName);
        fwrite(m_pAEPlineTable, 1, sizeof(AE_PLINETABLE_T), fp);
        fclose(fp);
    }

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAESavePlineNVRAM(
)
{
    char value[256] = {'\0'};
    MBOOL dumpEnable;
    property_get("vendor.debug.cct_aepline_save.enable", value, "0");
    dumpEnable = atoi(value);

    MUINT32 u4SensorID;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum;
    MRESULT err = S_AE_OK;

    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_SAVE_PLINE]\n");

    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].eAEScene);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[0]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[1]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[2]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[3]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[4]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[5]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[6]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[7]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[8]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[9]);
    CAM_LOGD("Mapping2[0] = %d\n", m_rAEInitInput.rAEPlineTable->sAEScenePLineMapping.sAESceneMapping[0].ePLineID[10]);

    CAM_LOGD("Preview ID = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].eID);
    CAM_LOGD("Preview Total Idx = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].u4TotalIndex);
    CAM_LOGD("Preview StrobeBV = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].i4StrobeTrigerBV);
    CAM_LOGD("Preview i4MaxBV = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].i4MaxBV);
    CAM_LOGD("Preview i4MinBV = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].i4MinBV);
    CAM_LOGD("Preview ISO = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].ISOSpeed);
    CAM_LOGD("Preview Exp = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[0].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[1].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[2].u4IspGain);
    CAM_LOGD("Preview Exp = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4Eposuretime);
    CAM_LOGD("Preview SGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4AfeGain);
    CAM_LOGD("Preview IGain = %d\n", m_rAEInitInput.rAEPlineTable->AEPlineTable.sPlineTable[0].sTable60Hz.sPlineTable[3].u4IspGain);

    if(dumpEnable)
    {
        char fileName[64];
        sprintf(fileName, "/data/AEPline_save_before.bin");
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/data/", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
            return MFALSE;
        }
        CAM_LOGD("%s\n", fileName);
        fwrite(m_pAEPlineTable, 1, sizeof(AE_PLINETABLE_T), fp);
        fclose(fp);
    }

    bApplyAEPlineTableByCCT = MFALSE;
    memcpy(m_pAEPlineTable, m_rAEInitInput.rAEPlineTable, sizeof(AE_PLINETABLE_T));

    if(dumpEnable)
    {
        char fileName[64];
        sprintf(fileName, "/data/AEPline_save_after.bin");
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/data/", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
            return MFALSE;
        }
        CAM_LOGD("%s\n", fileName);
        fwrite(m_pAEPlineTable, 1, sizeof(AE_PLINETABLE_T), fp);
        fclose(fp);
    }

    err = NvBufUtil::getInstance().write(CAMERA_DATA_AE_PLINETABLE, m_eSensorDev);
    if (err != 0)
    {
        CAM_LOGE("NvBufUtil::getInstance().write\n");
        return E_NVRAM_BAD_PARAM;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAEGetCurrentEV(
    MINT32 *a_pAECurrentEV,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION]\n");

    *a_pAECurrentEV = m_i4BVvalue;
    *a_pOutLen = sizeof(MINT32);

    CAM_LOGD("[AE Current EV] = %d\n", *a_pAECurrentEV);

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAELockExpSetting(
)
{
    CAM_LOGD("[ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING]\n");

    m_bLockExposureSetting = TRUE;

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AeMgr::CCTOPAEUnLockExpSetting(
)
{
    CAM_LOGD("[ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING]\n");

    m_bLockExposureSetting = FALSE;

    return S_AE_OK;
}

