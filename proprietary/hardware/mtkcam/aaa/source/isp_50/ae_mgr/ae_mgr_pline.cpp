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
#define ENABLE_MY_LOG (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
//#include <CamDefs.h>
#include <faces.h>
#include <isp_tuning.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <ae_algo_if.h>
//
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include <kd_camera_feature.h>
#include <mtkcam/drv/IHalSensor.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <aaa_scheduler.h>
#include <aaa_common_custom.h>

static strAEMOVE AESenstivityRatioTable[] = // for preview / capture
{
    { -20,  25,  20}, // mean below -2 move increase 20 index
    { -15,  33,  15}, // -1.5
    { -10,  50,  10}, // -1
    {  -9,  55,   9}, // -0.9
    {  -8,  58,   8}, // -0.8
    {  -7,  63,   7}, // -0.7
    {  -6,  67,   6}, // -0.6
    {  -5,  72,   5}, // -0.5
    {  -4,  77,   4}, // -0.4
    {  -3,  82,   3}, // -0.3
    {  -2,  88,   2}, // -0.2
    {  -1,  94,   1}, // -0.1
    {   0, 100,   0}, // 0
    {   0, 107,   0}, // 0
    {   1, 108,  -1}, // 0.1
    {   2, 115,  -2}, // 0.2
    {   3, 123,  -3}, // 0.3
    {   4, 132,  -4}, // 0.4
    {   5, 141,  -5}, // 0.5
    {   6, 152,  -6}, // 0.6
    {   7, 162,  -7}, // 0.7
    {   8, 174,  -8}, // 0.8
    {   9, 186,  -9}, // 0.9
    {  10, 200, -10}, // 1.0
    {  11, 214, -11}, // 1.1
    {  12, 230, -12}, // 1.2
    {  13, 246, -13}, // 1.3
    {  14, 264, -14}, // 1.4
    {  15, 283, -15}, // 1.5
    {  16, 303, -16}, // 1.6
    {  17, 325, -17}, // 1.7
    {  18, 348, -18}, // 1.8
    {  19, 373, -19}, // 1.9
    {  20, 400, -20}, // 2 EV
    {  21, 429, -20}, // 2.1 EV
    {  22, 459, -20}, // 2.2 EV
    {  23, 492, -20}, // 2.3 EV
    {  24, 528, -20}, // 2.4 EV
    {  25, 566, -25}, // 2.5 EV
    {  30, 800, -30}, // 3 EV
};

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setSceneMode(MUINT32 u4NewAEScene)
{
    mtk_camera_metadata_enum_android_control_scene_mode_t eNewAEScene = static_cast < mtk_camera_metadata_enum_android_control_scene_mode_t > (u4NewAEScene);
    LIB3A_AE_SCENE_T eAEScene;

    if (eNewAEScene < MTK_CONTROL_SCENE_MODE_DISABLED)
    {
        CAM_LOGE("[%s()] Unsupport AE mode:%d\n", __FUNCTION__, eNewAEScene);
        return E_AE_UNSUPPORT_MODE;
    }

    switch (eNewAEScene)
    {
        case MTK_CONTROL_SCENE_MODE_FACE_PRIORITY:
        case MTK_CONTROL_SCENE_MODE_BARCODE:
            eAEScene = LIB3A_AE_SCENE_AUTO;
            break;
        case MTK_CONTROL_SCENE_MODE_ACTION:
            eAEScene = LIB3A_AE_SCENE_ACTION;
            break;
        case MTK_CONTROL_SCENE_MODE_PORTRAIT:
            eAEScene = LIB3A_AE_SCENE_PORTRAIT;
            break;
        case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
            eAEScene = LIB3A_AE_SCENE_LANDSCAPE;
            break;
        case MTK_CONTROL_SCENE_MODE_NIGHT:
            eAEScene = LIB3A_AE_SCENE_NIGHT;
            break;
        case MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
            eAEScene = LIB3A_AE_SCENE_NIGHT_PORTRAIT;
            break;
        case MTK_CONTROL_SCENE_MODE_THEATRE:
            eAEScene = LIB3A_AE_SCENE_THEATRE;
            break;
        case MTK_CONTROL_SCENE_MODE_BEACH:
            eAEScene = LIB3A_AE_SCENE_BEACH;
            break;
        case MTK_CONTROL_SCENE_MODE_SNOW:
            eAEScene = LIB3A_AE_SCENE_SNOW;
            break;
        case MTK_CONTROL_SCENE_MODE_SUNSET:
            eAEScene = LIB3A_AE_SCENE_SUNSET;
            break;
        case MTK_CONTROL_SCENE_MODE_STEADYPHOTO:
            eAEScene = LIB3A_AE_SCENE_STEADYPHOTO;
            break;
        case MTK_CONTROL_SCENE_MODE_FIREWORKS:
            eAEScene = LIB3A_AE_SCENE_FIREWORKS;
            break;
        case MTK_CONTROL_SCENE_MODE_SPORTS:
            eAEScene = LIB3A_AE_SCENE_SPORTS;
            break;
        case MTK_CONTROL_SCENE_MODE_PARTY:
            eAEScene = LIB3A_AE_SCENE_PARTY;
            break;
        case MTK_CONTROL_SCENE_MODE_CANDLELIGHT:
            eAEScene = LIB3A_AE_SCENE_CANDLELIGHT;
            break;
        #if 0 // mark for mtk control scene mode define
        case MTK_CONTROL_SCENE_MODE_CSHOT:
            eAEScene = LIB3A_AE_SCENE_CSHOT;
            break;
        case MTK_CONTROL_SCENE_MODE_SWEEP_PANORAMA:
            eAEScene = LIB3A_AE_SCENE_SWEEP_PANORAMA;
            break;
        #endif
        default:
            AE_LOG_IF(m_3ALogEnable, "[%s()] The Scene mode is not correctly:%d\n", __FUNCTION__, eNewAEScene);
            eAEScene = LIB3A_AE_SCENE_AUTO;
            break;
    }

    if (m_eAEScene != eAEScene)
    {
        AE_LOG("[%s()] m_eAEScene:%d->%d\n", __FUNCTION__, m_eAEScene, eAEScene);
        if (m_eAEScene == LIB3A_AE_SCENE_FIREWORKS)
        {
            m_bUpdateAELockIdx = MTRUE;
        }
        m_eAEScene = eAEScene;
        if (m_pIAeAlgo != NULL)
        {
            // AE Auto Test
            m_pIAeTest->recordAvgY(IAeTest::AE_AT_TYPE_SCENE, m_pIAeAlgo->getBrightnessAverageValue());

            //New AE P -Line
            m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            AE_LOG_IF(m_3ALogEnable, "[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

            setAEScene(m_eAEScene);
            MUINT32 u4FinerEVIdxBase = 0;
            m_pIAeAlgo->setAEScene(m_eAEScene);
            m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
            if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
            {
                m_u4FinerEVIdxBase = u4FinerEVIdxBase;
                m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
                m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
                m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
                searchPreviewIndexLimit();
            }
            AE_LOG_IF(m_3ALogEnable, "[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

            m_pIAeAlgo->getAEMaxISO(m_u4MaxShutter, m_u4MaxISO);
            if (m_bAELock == MFALSE)
            {
                m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
            }
            AE_LOG("[%s()] m_u4MaxShutter:%d m_u4MaxISO:%d\n", __FUNCTION__, m_u4MaxShutter, m_u4MaxISO);
        }
        else
        {
            m_u4MaxShutter = 100000;
            m_u4MaxISO = 800;
            AE_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d line:%d MaxShutter:%d MaxISO:%d", __FUNCTION__, m_eSensorDev, __LINE__, m_u4MaxShutter, m_u4MaxISO);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEScene() const
{
    return static_cast < MINT32 > (m_eAEScene);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEISOSpeed(MUINT32 u4NewAEISOSpeed)
{
    MUINT32 eAEISOSpeed;

    if (u4NewAEISOSpeed > LIB3A_AE_ISO_SPEED_MAX)
    {
        CAM_LOGE("[%s()] Unsupport AE ISO Speed: %d\n", __FUNCTION__, u4NewAEISOSpeed);
        return E_AE_UNSUPPORT_MODE;
    }
    switch (u4NewAEISOSpeed)
    {
        case 0:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_AUTO;
            break;
        case 50:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_50;
            break;
        case 100:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_100;
            break;
        case 150:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_150;
            break;
        case 200:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_200;
            break;
        case 300:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_300;
            break;
        case 400:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_400;
            break;
        case 600:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_600;
            break;
        case 800:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_800;
            break;
        case 1200:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_1200;
            break;
        case 1600:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_1600;
            break;
        case 2400:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_2400;
            break;
        case 3200:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_3200;
            break;
        case 6400:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_6400;
            break;
        case 12800:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_12800;
            break;
        default:
            if (m_bRealISOSpeed == MTRUE)
            {
                AE_LOG("[%s()] The Real ISO speed:%d m_bRealISOSpeed:%d\n", __FUNCTION__, u4NewAEISOSpeed, m_bRealISOSpeed);
                eAEISOSpeed = u4NewAEISOSpeed;
            }
            else
            {
                AE_LOG("[%s()] The iso enum value is incorrectly:%d\n", __FUNCTION__, u4NewAEISOSpeed);
                eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_AUTO;
            }
            break;
    }

    if (m_u4AEISOSpeed != eAEISOSpeed)
    {
        AE_LOG("[%s()] m_u4AEISOSpeed:%d->%d\n", __FUNCTION__, m_u4AEISOSpeed, eAEISOSpeed);
        m_u4AEISOSpeed = eAEISOSpeed;
        if (m_pIAeAlgo != NULL)
        {
            // AE Auto Test
            m_pIAeTest->recordAvgY(IAeTest::AE_AT_TYPE_ISO, m_pIAeAlgo->getBrightnessAverageValue());

            //New AE -Pline
            m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            AE_LOG_IF(m_3ALogEnable, "[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

            setIsoSpeed(m_u4AEISOSpeed);
            MUINT32 u4FinerEVIdxBase = 0;
            m_pIAeAlgo->setIsoSpeed(m_u4AEISOSpeed);
            m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
            if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
            {
                m_u4FinerEVIdxBase = u4FinerEVIdxBase;
                m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
                m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
                m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
                searchPreviewIndexLimit();
            }
            AE_LOG_IF(m_3ALogEnable, "[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);

        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEISOSpeedMode() const
{
    return static_cast < MINT32 > (m_u4AEISOSpeed);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAECamMode(MUINT32 u4NewAECamMode)
{
    EAppMode eNewAECamMode = static_cast < EAppMode > (u4NewAECamMode);

    if (m_eCamMode != eNewAECamMode)
    {
        m_eCamMode = eNewAECamMode;
        AE_LOG("[%s()] m_eCamMode:%d AECamMode:%d \n", __FUNCTION__, m_eCamMode, m_eAECamMode);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode)
{
    LIB3A_AE_FLICKER_AUTO_MODE_T eNewAEAutoFLKMode = static_cast < LIB3A_AE_FLICKER_AUTO_MODE_T > (u4NewAEAutoFLKMode);

    if ((eNewAEAutoFLKMode <= LIB3A_AE_FLICKER_AUTO_MODE_UNSUPPORTED) || (eNewAEAutoFLKMode >= LIB3A_AE_FLICKER_AUTO_MODE_MAX))
    {
        CAM_LOGE("[%s()] Unsupport AE auto flicker mode: %d\n", __FUNCTION__, eNewAEAutoFLKMode);
        return E_AE_UNSUPPORT_MODE;
    }

    if (m_eAEAutoFlickerMode != eNewAEAutoFLKMode)
    {
        m_eAEAutoFlickerMode = eNewAEAutoFLKMode;
        AE_LOG("[%s()] m_eAEAutoFlickerMode:%d (0->50Hz;1->60Hz)\n", __FUNCTION__, m_eAEAutoFlickerMode);
        if (m_pIAeAlgo != NULL)
        {
            //AE P -line
            m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            AE_LOG_IF(m_3ALogEnable, "[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

            setAEFlickerAutoModePlineIdx(eNewAEAutoFLKMode);
            MUINT32 u4FinerEVIdxBase = 0;
            m_pIAeAlgo->setAEFlickerAutoMode(m_eAEAutoFlickerMode);
            m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
            if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
            {
                m_u4FinerEVIdxBase = u4FinerEVIdxBase;
                m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
                m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
                m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
                searchPreviewIndexLimit();
            }
            AE_LOG_IF(m_3ALogEnable, "[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEFlickerMode(MUINT32 u4NewAEFLKMode)
{
    mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t eNewAEFLKMode = static_cast < mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t > (u4NewAEFLKMode);
    LIB3A_AE_FLICKER_MODE_T eAEFLKMode;

    switch (eNewAEFLKMode)
    {
        case MTK_CONTROL_AE_ANTIBANDING_MODE_OFF:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_OFF;
            break;
        case MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
        case MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_60HZ;
            break;
        case MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_AUTO;
            break;
        default:
            AE_LOG("[%s()] The flicker enum value is incorrectly:%d\n", __FUNCTION__, eNewAEFLKMode);
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
    }

    if (m_eAEFlickerMode != eAEFLKMode)
    {
        AE_LOG("[%s()] AEFlickerMode:%d->%d (0->60Hz;1->50Hz;2->Auto;3->Off)\n", __FUNCTION__, m_eAEFlickerMode, eAEFLKMode);
        m_eAEFlickerMode = eAEFLKMode;
        if (m_pIAeAlgo != NULL)
        {
            //AE P -line
            m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            AE_LOG_IF(m_3ALogEnable, "[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

            setAEFlickerModePlineIdx(eAEFLKMode);
            MUINT32 u4FinerEVIdxBase = 0;
            m_pIAeAlgo->setAEFlickerMode(m_eAEFlickerMode);
            m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
            if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
            {
                m_u4FinerEVIdxBase = u4FinerEVIdxBase;
                m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
                m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
                m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
                searchPreviewIndexLimit();
            }
            AE_LOG_IF(m_3ALogEnable, "[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);

        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}
/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEFlickerModePlineIdx(LIB3A_AE_FLICKER_MODE_T a_eAEFlickerMode)
{
    AE_LOG("[%s()] Set Flicker value %d \n", __FUNCTION__, a_eAEFlickerMode);
    setIsoSpeed(m_u4AEISOSpeed);
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEFlickerAutoModePlineIdx(LIB3A_AE_FLICKER_AUTO_MODE_T a_eAEFlickerAutoMode)
{
    AE_LOG("[%s()] %d \n", __FUNCTION__, a_eAEFlickerAutoMode);
    setIsoSpeed(m_u4AEISOSpeed);
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setIsoSpeed(MUINT32 a_u4ISO)
{
    MRESULT mr;
    eAETableID ePreviewPLineTableID, eCapturePLineTableID;
    LIB3A_AE_SCENE_T eAEScene;

    memset(&ePreviewPLineTableID, 0, sizeof(eAETableID));
    memset(&eCapturePLineTableID, 0, sizeof(eAETableID));

    eAEScene = m_eAEScene;
    AE_LOG_IF(m_3ALogEnable, "[%s()] setIsoSpeed:%d %d\n", __FUNCTION__, a_u4ISO, m_u4AEISOSpeed);

    if ((m_u4AEISOSpeed == LIB3A_AE_ISO_SPEED_AUTO) || (m_bRealISOSpeed == MTRUE))
    {
        setAEScene(m_eAEScene);
    }
    else
    {
        switch (a_u4ISO)
        {
            case LIB3A_AE_ISO_SPEED_12800:
                eAEScene = LIB3A_AE_SCENE_ISO12800;
                break;
            case LIB3A_AE_ISO_SPEED_6400:
                eAEScene = LIB3A_AE_SCENE_ISO6400;
                break;
            case LIB3A_AE_ISO_SPEED_3200:
                eAEScene = LIB3A_AE_SCENE_ISO3200;
                break;
            case LIB3A_AE_ISO_SPEED_1600:
                eAEScene = LIB3A_AE_SCENE_ISO1600;
                break;
            case LIB3A_AE_ISO_SPEED_800:
                eAEScene = LIB3A_AE_SCENE_ISO800;
                break;
            case LIB3A_AE_ISO_SPEED_400:
                eAEScene = LIB3A_AE_SCENE_ISO400;
                break;
            case LIB3A_AE_ISO_SPEED_200:
                eAEScene = LIB3A_AE_SCENE_ISO200;
                break;
            case LIB3A_AE_ISO_SPEED_100:
                eAEScene = LIB3A_AE_SCENE_ISO100;
                break;
            default:
                AE_LOG("[%s()] Wrong ISO setting:%d\n", __FUNCTION__, m_u4AEISOSpeed);
                break;
        }

        if (eAEScene != m_eAEScene)
        {
            mr = getAEPLineMappingID(eAEScene, m_eSensorMode, &ePreviewPLineTableID, &eCapturePLineTableID);
            if (FAILED(mr))
            {
                CAM_LOGE("[%s()] Get capture table ERROR :%d %d PLineID:%d %d\n", __FUNCTION__, m_eAEScene, eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
            }
            mr = setAETable(ePreviewPLineTableID, eCapturePLineTableID);
        }
    }
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEScene(LIB3A_AE_SCENE_T a_eAEScene)
{
    MRESULT mr;
    eAETableID ePreviewPLineTableID, eCapturePLineTableID;
    memset(&ePreviewPLineTableID, 0, sizeof(eAETableID));
    memset(&eCapturePLineTableID, 0, sizeof(eAETableID));

    AE_LOG_IF(m_3ALogEnable, "[%s()] setAEScene:%d\n", __FUNCTION__, a_eAEScene);

    if (m_pAEPlineTable != NULL)
    {
        // protect the AE Pline table don't ready
        mr = getAEPLineMappingID(m_eAEScene, m_eSensorMode, &ePreviewPLineTableID, &eCapturePLineTableID);
        if (FAILED(mr))
        {
            CAM_LOGE("[%s()] Get capture table ERROR:%d PLineID:%d %d\n", __FUNCTION__, m_eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
        }

        mr = setAETable(ePreviewPLineTableID, eCapturePLineTableID);
        if (FAILED(mr))
        {
            CAM_LOGE("[%s()] Capture table ERROR:%d PLineID:%d %d\n", __FUNCTION__, m_eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
        }

        if ((m_eAEScene == LIB3A_AE_SCENE_AUTO) && (m_u4AEISOSpeed != (MUINT32) LIB3A_AE_ISO_SPEED_AUTO) && (!m_bRealISOSpeed))
        {
            setIsoSpeed(m_u4AEISOSpeed);
        }

    }
    else
    {
        AE_LOG("[%s()] setAEScene:%d, AE Pline table is NULL\n", __FUNCTION__, a_eAEScene);
    }

    m_pPreviewTableCurrent = m_pPreviewTableNew;
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAETable(eAETableID a_eAEPreTableID, eAETableID a_eAECapTableID)
{
    MRESULT mr = S_AE_OK;
    eAETableID a_AEPreTableID = a_eAEPreTableID;
    eAETableID a_AECapTableID = a_eAECapTableID;
    MBOOL bPreTableExist = MFALSE;
    MBOOL bCapTableExist = MFALSE;
    char logString[100] = {"\0"};
    char tmpString[40] = {"\0"};

    AE_LOG_IF(m_3ALogEnable, "[%s()] TableID(Pre/Cap):%d/%d Flicker(Mode/AutoMode):%d/%d i4SensorMode:%d mbIsPlineChange = MTRUE\n", __FUNCTION__, a_AEPreTableID, a_AECapTableID, m_eAEFlickerMode, m_eAEAutoFlickerMode, m_eSensorMode);
    if (CUST_AE_MP_PARAM)
    {
        ManualPlineParam sMPParam;
        sMPParam = ManualPlineParam((MINT32) m_i4AEMinFps
            , (MINT32) m_i4AEMaxFps
            , (MINT32) m_eSensorMode
            , (MINT32) m_eAEScene
            , (MINT32) m_u4AEISOSpeed);
        getAEManualPline(sMPParam, a_AEPreTableID, a_AECapTableID);
        AE_LOG("[%s()] custom AE manual pline, ePreviewPLineTableID/eCapturePLineTableID:%d/%d, MinFps/MaxFps/SensorMode/SceneMode/ISOSpeed: %d/%d/%d/%d/%d\n", __FUNCTION__,
            a_AEPreTableID, a_AECapTableID, sMPParam.MinFps, sMPParam.MaxFps, sMPParam.SensorMode, sMPParam.SceneMode, sMPParam.ISOSpeed);
    }

    //check table existence
    for (MUINT32 i = 0; i < (MUINT32)MAX_PLINE_TABLE; i++)
    {
        if (m_3ALogEnable)
        {
            if (i % 10 == 0)
            {
                memset(logString, 0, sizeof(char)*100);
                sprintf(logString, "i/TotalIdx:");
            }
            memset(tmpString, 0, sizeof(char)*40);
            sprintf(tmpString, "%2d/%3d ", i, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
            strncat(logString, tmpString, 100 - strlen(logString) - 1);
            if (i % 10 == 9 || i == MAX_PLINE_TABLE - 1)
                AE_LOG("[%s()] %s\n", __FUNCTION__, logString);
        }
        if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].eID == a_AEPreTableID)
        {
            if(m_pPreviewTableCurrent != NULL){
			    if((a_AEPreTableID != m_pPreviewTableCurrent->eID) || (m_3ALogEnable != 0)){
                    AE_LOG("[%s()] i/ePreviewPLineTableID/TotalIdx:%d/%d->%d/%d\n", __FUNCTION__, i, m_pPreviewTableCurrent->eID, a_AEPreTableID, m_pPreviewTableCurrent->u4TotalIndex, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
			    }
            }else{
     			AE_LOG("[%s()] i/ePreviewPLineTableID/TotalIdx:%d/0->%d/0->%d\n", __FUNCTION__, i, a_AEPreTableID, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);				
            }
            bPreTableExist = MTRUE;
            if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex == 0)
            {
                AE_LOG("[%s()] The Preview AE table total index is zero:%d->%d\n", __FUNCTION__, a_AEPreTableID, m_pPreviewTableCurrent->eID);
                a_AEPreTableID = m_pPreviewTableCurrent->eID;
            }
        }
        if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].eID == a_AECapTableID)
        {
            if(m_pCaptureTable != NULL){
                if((a_AECapTableID != m_pCaptureTable->eID) || (m_3ALogEnable != 0)){
                    AE_LOG("[%s()] i/eCapturePLineTableID/TotalIdx:%d/%d->%d/%d->%d\n", __FUNCTION__, i, m_pCaptureTable->eID, a_AECapTableID, m_pCaptureTable->u4TotalIndex, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
                }
            }else{
                AE_LOG("[%s()] i/eCapturePLineTableID/TotalIdx:%d/0->%d/0->%d\n", __FUNCTION__, i, a_AECapTableID, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
            }
            bCapTableExist = MTRUE;
            if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex == 0)
            {
                AE_LOG("[%s()] The Capture AE table total index is zero:%d->%d\n", __FUNCTION__, a_AECapTableID, m_pCaptureTable->eID);
                a_AECapTableID = m_pCaptureTable->eID;
            }
        }
    }
    if (!bPreTableExist)
    {
        AE_LOG("[%s()] The Preview AE table doesn't exist:%d->%d\n", __FUNCTION__, a_AEPreTableID, m_pPreviewTableCurrent->eID);
        a_AEPreTableID = m_pPreviewTableCurrent->eID;
    }
    if (!bCapTableExist)
    {
        AE_LOG("[%s()] The Capture AE table doesn't exist:%d->%d\n", __FUNCTION__, a_AECapTableID, m_pCaptureTable->eID);
        a_AECapTableID = m_pCaptureTable->eID;
    }

    if (m_eAEManualPline == EAEManualPline_Default)
    {
        mr = searchAETable(m_pAEPlineTable, a_AEPreTableID, &m_pPreviewTableNew);

        if (FAILED(mr))
        {
            CAM_LOGE("[%s()] Search Preview Pline table:%d error\n", __FUNCTION__, a_AEPreTableID);
        }

        mr = searchAETable(m_pAEPlineTable, a_AECapTableID, &m_pCaptureTable);

        if (FAILED(mr))
        {
            CAM_LOGE("[%s()] Search Capture Pline table:%d error\n", __FUNCTION__, a_AEPreTableID);
        }
    }
    else
    {
        mr = setManualAETable(m_eAEManualPline);

    }
    m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
    mbIsPlineChange = MTRUE;
    mr = searchPreviewIndexLimit();

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo)
{
    if (m_pIAeAlgo != NULL)
    {
        getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable, m_CurrentStrobetureTable);
        a_PrvAEPlineTable = m_CurrentPreviewTable;
        a_CapAEPlineTable = m_CurrentCaptureTable;
        a_StrobeAEPlineTable = m_CurrentStrobetureTable;
        AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d PreId:%d CapId:%d Strobe:%d\n", __FUNCTION__, m_eSensorDev, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, m_CurrentStrobetureTable.eID);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    if (m_eAECamMode == LIB3A_AECAM_MODE_ZSD)
    {
        a_StrobeAEPlineInfo = m_rAEInitInput.rAENVRAM->rAeParam.strStrobeZSDPLine;
    }
    else
    {
        a_StrobeAEPlineInfo = m_rAEInitInput.rAENVRAM->rAeParam.strStrobePLine;
    }

    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d Strobe enable:%d AECamMode:%d\n", __FUNCTION__, m_eSensorDev, a_StrobeAEPlineInfo.bAFPlineEnable, m_eAECamMode);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCurrentPlineTableF(strFinerEvPline &a_PrvAEPlineTable, strFinerEvPline &a_CapAEPlineTable)
{
    if (m_pIAeAlgo != NULL)
    {
        a_PrvAEPlineTable = m_PreviewTableF;
        a_CapAEPlineTable = m_CaptureTableF;
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d AECamMode:%d\n", __FUNCTION__, m_eSensorDev, m_eAECamMode);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEPlineTable(eAETableID eTableID, strAETable &a_AEPlineTable)
{
    strAETable strAEPlineTable;
    memset(&strAEPlineTable, 0, sizeof(strAETable));

    if (m_pIAeAlgo != NULL)
    {
        getOnePlineTable(eTableID, strAEPlineTable);
        a_AEPlineTable = strAEPlineTable;
        AE_LOG("[%s()] i4SensorDev:%d PreId:%d CapId:%d GetID:%d\n", __FUNCTION__, m_eSensorDev, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, eTableID);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable)
{
    MRESULT mr = S_AE_OK;

    if (m_pPreviewTableCurrent != NULL)
    {
        a_PrvAEPlineTable = (strAETable) *m_pPreviewTableCurrent;
    }
    else
    {
        AE_LOG("[%s()] Preview table is NULL\n", __FUNCTION__);
    }

    if (m_pCaptureTable != NULL)
    {
        a_CapAEPlineTable = (strAETable) *m_pCaptureTable;
    }
    else
    {
        a_CapAEPlineTable = (strAETable) *m_pPreviewTableCurrent;
        AE_LOG("[%s()] Capture table is NULL, Using preview table to instead\n", __FUNCTION__);
    }

    if (m_pStrobeTable != NULL)
    {
        a_StrobeAEPlineTable = (strAETable) *m_pStrobeTable;
    }
    else
    {
        mr = searchAETable(m_pAEPlineTable, AETABLE_STROBE, &m_pStrobeTable);
        if (FAILED(mr))
        {
            AE_LOG("[%s()] Search Preview Pline table:%d error \n", __FUNCTION__, AETABLE_STROBE);
            a_StrobeAEPlineTable = a_CapAEPlineTable;
        }
        else
        {
            a_StrobeAEPlineTable = (strAETable) *m_pStrobeTable;
        }

        AE_LOG("[%s()] Capture table is NULL, Using preview table to instead\n", __FUNCTION__);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getOnePlineTable(eAETableID eTableID, strAETable &a_AEPlineTable)
{
    strAETable *pPlineTable;
    MRESULT mr = S_AE_OK;

    mr = searchAETable(m_pAEPlineTable, eTableID, &pPlineTable);
    if (FAILED(mr))
    {
        AE_LOG("[%s()] Search Pline table:%d error \n", __FUNCTION__, eTableID);
        a_AEPlineTable = (strAETable) *m_pPreviewTableCurrent;
    }
    else
    {
        a_AEPlineTable = (strAETable) *pPlineTable;
    }

    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::searchAETable(AE_PLINETABLE_T *a_aepara , eAETableID id, strAETable** a_ppPreAETable)
{
    MUINT32 i;

    if (a_aepara != NULL)
    {
        for (i = 0; i < (MUINT32)MAX_PLINE_TABLE; i++)
        {
            if (a_aepara->AEPlineTable.sPlineTable[i].eID == id)
            {
                *a_ppPreAETable = &(a_aepara->AEPlineTable.sPlineTable[i]);
                if ((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_60HZ) ||
                    ((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_AUTO) && (m_eAEAutoFlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_60HZ)))
                {
                    ( *a_ppPreAETable)->pCurrentTable = &(a_aepara->AEPlineTable.sPlineTable[i].sTable60Hz); //copy the 60Hz for current table used
                }
                else
                {
                    ( *a_ppPreAETable)->pCurrentTable = &(a_aepara->AEPlineTable.sPlineTable[i].sTable50Hz); //copy the 50Hz for current table used
                }

                return S_AE_OK;
            }
        }
    }

    *a_ppPreAETable = NULL;
    return E_AE_NOMATCH_TABLE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEPLineMappingID(LIB3A_AE_SCENE_T a_eAESceneID, MINT32 i4SensorMode, eAETableID *pAEPrePLineID, eAETableID *pAECapPLineID)
{
    MUINT32 i;

    AE_LOG_IF(m_3ALogEnable, "[%s()] m_eSensorDev:%d SceneId:%d ISO:%d CamMode:%d SensorIdx:%d\n", __FUNCTION__, m_eSensorDev, a_eAESceneID, m_u4AEISOSpeed, m_eCamMode, i4SensorMode);

    if (m_pAEPlineTable == NULL || m_pAEMapPlineTable == NULL)
    {
        CAM_LOGE("[%s()] m_eSensorDev: %d, i4SensorMode = %d, CamMode:%d The pointer is NULL\n", __FUNCTION__, m_eSensorDev, m_eSensorMode, m_eCamMode);
        return E_AE_NULL_AE_TABLE;
    }

    *pAEPrePLineID = m_pAEMapPlineTable[0].ePLineID[LIB3A_SENSOR_MODE_PRVIEW];

    if (i4SensorMode == LIB3A_SENSOR_MODE_PRVIEW)
    {
        *pAECapPLineID = m_pAEMapPlineTable[0].ePLineID[LIB3A_SENSOR_MODE_CAPTURE];
    }
    else
    {
        *pAECapPLineID = *pAEPrePLineID;
    }

    if ((i4SensorMode > LIB3A_SENSOR_MODE_CAPTURE_ZSD) || (i4SensorMode < LIB3A_SENSOR_MODE_PRVIEW))
    {
        CAM_LOGE("[%s()] m_eSensorDev: %d, i4SensorMode error = %d, CamMode:%d\n", __FUNCTION__, m_eSensorDev, m_eSensorMode, m_eCamMode);
        return E_AE_NULL_AE_TABLE;
    }

    for (i = 0; i < MAX_MAPPING_PLINE_TABLE; i++)
    {
        if (m_pAEMapPlineTable[i].eAEScene == LIB3A_AE_SCENE_UNSUPPORTED)
        {
            AE_LOG_IF(m_3ALogEnable, "[%s()] No find Mapping PLine ID :%d\n", __FUNCTION__, a_eAESceneID);
            break;
        }

        if (m_pAEMapPlineTable[i].eAEScene == a_eAESceneID)
        {
            if (i4SensorMode == LIB3A_SENSOR_MODE_CAPTURE)
            {
                *pAEPrePLineID = m_pAEMapPlineTable[i].ePLineID[LIB3A_SENSOR_MODE_CAPTURE_ZSD];
            }
            else
            {
                *pAEPrePLineID = m_pAEMapPlineTable[i].ePLineID[i4SensorMode];
            }
            if (i4SensorMode == LIB3A_SENSOR_MODE_PRVIEW)
            {
                *pAECapPLineID = m_pAEMapPlineTable[i].ePLineID[LIB3A_SENSOR_MODE_CAPTURE];
            }
            else
            {
                *pAECapPLineID = *pAEPrePLineID;
            }
            AE_LOG_IF(m_3ALogEnable, "[%s()] SceneID:%d Pre:%d CapID:%d\n", __FUNCTION__, a_eAESceneID, *pAEPrePLineID, *pAECapPLineID);
            return S_AE_OK;
        }
    }
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::getAETableMaxBV(const AE_PLINETABLE_T *a_aepara , eAETableID id, MINT32 *i4MaxBV)
{
    MUINT32 i;

    if (a_aepara != NULL)
    {
        for (i = 0; i < (MUINT32)MAX_PLINE_TABLE; i++)
        {
            if (a_aepara->AEPlineTable.sPlineTable[i].eID == id)
            {
                *i4MaxBV = a_aepara->AEPlineTable.sPlineTable[i].i4MaxBV;
                break;
            }
        }
    }
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::searchPreviewIndexLimit()
{
    MINT32 itemp;
    MINT32 iSenstivityDeltaIdx = 0;
    MINT32 i4TableDeltaIdx = 0;
    MUINT32 itempIndexcur = m_u4Index;

    if (m_pPreviewTableNew == NULL)
    {
        CAM_LOGE("[%s] No preview table\n", __FUNCTION__);
        return E_AE_NOMATCH_TABLE;
    }

    m_u4IndexMin = (MUINT32)0;
    m_u4IndexFMin = (MUINT32)0;
    m_u4IndexMax = m_pPreviewTableNew->u4TotalIndex -(MUINT32)1;
    m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;

    if (m_pPreviewTableCurrent != NULL)
    {
        i4TableDeltaIdx = m_pPreviewTableNew->i4MaxBV - m_pPreviewTableCurrent->i4MaxBV;
        if ((MINT32)(m_u4Index + i4TableDeltaIdx) >= 0)
        {
            m_u4Index = m_u4Index + i4TableDeltaIdx;
            m_i4AEidxNext = m_u4Index;
            m_u4IndexF = m_u4IndexF + (i4TableDeltaIdx * m_u4FinerEVIdxBase);
        }
        else
        {
            m_u4Index = 0;
            m_i4AEidxNext = m_u4Index;
            AE_LOG("[%s] pline minimum index limitation!", __FUNCTION__);
        }
        AE_LOG_IF(m_3ALogEnable, "[%s] m_u4FinerEVIdxBase:%d m_u4Index/m_u4IndexF:%d/%d ZSD delta Idx:%d NewBV:%d OldBV:%d AEScene:%d u4Cap2PreRatio:%d\n", __FUNCTION__,
            m_u4FinerEVIdxBase, m_u4Index, m_u4IndexF, i4TableDeltaIdx, m_pPreviewTableNew->i4MaxBV, m_pPreviewTableCurrent->i4MaxBV, m_eAEScene, m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio);
    }
    // Capture and Preview index synchronization by MaxBV
    if (m_u4IndexMax > m_u4IndexMin)
    {
        if ((m_pPreviewTableNew->eID != m_pCaptureTable->eID) && (m_pCaptureTable->eID == AETABLE_CAPTURE_AUTO))
            iSenstivityDeltaIdx = getSenstivityDeltaIndex(m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio);

        // capture mode max BV = (m_pCaptureTable->i4MaxBV -i4DeltaIdx)
        itemp = (m_pPreviewTableNew->i4MaxBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset) -((m_pCaptureTable->i4MaxBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset) + iSenstivityDeltaIdx);

        //preview EV range size small then capture // run from Preview first index
        if (itemp < 0)
        {
            m_u4IndexMin = 0;
            AE_LOG("[%s] MinIndex(max BV) reach Preview limitation, need extend preview table\n", __FUNCTION__);
            if (m_pPreviewTableCurrent != NULL)
                AE_LOG("[%s] itemp1:%d NewBV:%d OldBV:%d BVOffset:%d Idx:%d\n", __FUNCTION__, itemp, m_pPreviewTableNew->i4MaxBV, m_pPreviewTableCurrent->i4MaxBV, m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset, iSenstivityDeltaIdx);
        }
        else
        {
            //capture BV range small then preview , make a offset
            m_u4IndexMin = (MUINT32)itemp;
        }

        itemp = (m_pPreviewTableNew->i4MaxBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset) -((m_pCaptureTable->i4MinBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset) + iSenstivityDeltaIdx);

        if (itemp > (MINT32)m_pPreviewTableNew->u4TotalIndex)
        {
            //capture range bigger then preview
            m_u4IndexMax = m_pPreviewTableNew->u4TotalIndex -(MUINT32)1;
            AE_LOG("[%s] MaxIndex(min BV) reach Preview limitation, need extend preview table\n", __FUNCTION__);
            if (m_pPreviewTableCurrent != NULL)
                AE_LOG("[%s] itemp2:%d NewBV:%d OldBV:%d BVOffset:%d Idx:%d\n", __FUNCTION__, itemp, m_pPreviewTableNew->i4MaxBV, m_pPreviewTableCurrent->i4MaxBV, m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset, iSenstivityDeltaIdx);
        }
        else
        {
            if (m_u4IndexMax >= (MUINT32)itemp) // Choose smaller index between capture max index and minfps index
                m_u4IndexMax = (MUINT32)itemp;
        }

        // Update max and min index from max and min fps
        setAEMinMaxFrameRatePlineIdx(m_i4AEMinFps, m_i4AEMaxFps);

        setAEMaxFrameRateGainIndexRange(); // for LCE used only
        AE_LOG_IF(m_3ALogEnable, "[%s] use preview table range normal/FinerEV: %d ~ %d / %d ~ %d \n", __FUNCTION__, m_u4IndexMin, m_u4IndexMax, m_u4IndexFMin, m_u4IndexFMax);

        m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
        m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;

        AE_LOG_IF(m_3ALogEnable, "[%s] Preview table range BV %d~%d total %d index\n", __FUNCTION__, (m_pPreviewTableNew->i4MaxBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset), (m_pPreviewTableNew->i4MinBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset), m_pPreviewTableNew->u4TotalIndex);
        AE_LOG_IF(m_3ALogEnable, "[%s] Capture table range BV %d~%d total %d index\n", __FUNCTION__, (m_pCaptureTable->i4MaxBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset), (m_pCaptureTable->i4MinBV -m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset), m_pCaptureTable->u4TotalIndex);
        AE_LOG_IF(m_3ALogEnable, "[%s] support range BV %d~%d\n", __FUNCTION__, m_pPreviewTableNew->i4MaxBV - m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset -(MINT32)m_u4IndexMin, m_pPreviewTableNew->i4MaxBV - m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset -(MINT32)m_u4IndexMax);
        AE_LOG_IF(m_3ALogEnable, "[%s] Preview table index normal/finerEV:%d~%d/%d~%d m_u4FinerEVIdxBase:%d m_u4Index/m_u4IndexF:%d/%d Delta:%d itemp:%d \n", __FUNCTION__, m_u4IndexMin, m_u4IndexMax, m_u4IndexFMin, m_u4IndexFMax, m_u4FinerEVIdxBase, m_u4Index, m_u4IndexF, iSenstivityDeltaIdx, itemp);

    }

    // Protect index range
    if (m_u4Index > m_u4IndexMax)
    {
        AE_LOG("[%s] m_u4Index1:%d m_u4IndexMax:%d m_u4IndexMin:%d\n", __FUNCTION__, m_u4Index, m_u4IndexMax, m_u4IndexMin);
        m_u4Index = m_u4IndexMax;
    }
    else if (m_u4Index < m_u4IndexMin)
    {
        AE_LOG("[%s] m_u4Index2:%d m_u4IndexMax:%d m_u4IndexMin:%d\n", __FUNCTION__, m_u4Index, m_u4IndexMax, m_u4IndexMin);
        m_u4Index = m_u4IndexMin;
    }
    if (m_u4IndexF > m_u4IndexFMax)
    {
        AE_LOG("[%s] m_u4IndexF1:%d m_u4IndexFMax:%d m_u4IndexFMin:%d\n", __FUNCTION__, m_u4IndexF, m_u4IndexFMax, m_u4IndexFMin);
        m_u4IndexF = m_u4IndexFMax;
    }
    else if (m_u4IndexF < m_u4IndexFMin)
    {
        AE_LOG("[%s] m_u4IndexF2:%d m_u4IndexFMax:%d m_u4IndexFMin:%d\n", __FUNCTION__, m_u4IndexF, m_u4IndexFMax, m_u4IndexFMin);
        m_u4IndexF = m_u4IndexFMin;
    }
    // Index modification will be applied
    if (itempIndexcur != m_u4Index)
    {
        AE_LOG("[%s] Current Index:%d->%d Exp/Afe/Isp:%d/%d/%d->%d/%d/%d\n", __FUNCTION__,
            itempIndexcur, m_u4Index, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain,
            m_pPreviewTableNew->pCurrentTable->sPlineTable[m_u4Index].u4Eposuretime,
            m_pPreviewTableNew->pCurrentTable->sPlineTable[m_u4Index].u4AfeGain,
            m_pPreviewTableNew->pCurrentTable->sPlineTable[m_u4Index].u4IspGain);
        m_rAEOutput.rPreviewMode.u4Eposuretime = m_pPreviewTableNew->pCurrentTable->sPlineTable[m_u4Index].u4Eposuretime;
        m_rAEOutput.rPreviewMode.u4AfeGain = m_pPreviewTableNew->pCurrentTable->sPlineTable[m_u4Index].u4AfeGain;
        m_rAEOutput.rPreviewMode.u4IspGain = m_pPreviewTableNew->pCurrentTable->sPlineTable[m_u4Index].u4IspGain;
        mPreviewMode = m_rAEOutput.rPreviewMode;
        AE_LOG("[%s] mPreviewMode = m_rAEOutput.rPreviewMode mbIsIndexChange = MTRUE\n", __FUNCTION__);
        m_i4DeltaBVIdx = 0;
        mbIsIndexChange = MTRUE;
    }
    m_pPreviewTableCurrent = m_pPreviewTableNew; // copy to current table
    m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
    mbIsPlineChange = MTRUE;
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEMinMaxFrameRatePlineIdx(MINT32 a_eAEMinFrameRate, MINT32 a_eAEMaxFrameRate)
{
    MINT32 i4fps;
    MINT32 i4PlineIndex = 0;

    i4fps = a_eAEMaxFrameRate;
    m_i4AEMaxFps = a_eAEMaxFrameRate;
    AE_LOG_IF(m_3ALogEnable, "[%s()] m_u4FinerEVIdxBase:%d MaxFps/MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF/Fps:%d/%d/%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, a_eAEMaxFrameRate, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF, i4fps);

    i4fps = a_eAEMinFrameRate;
    m_i4AEMinFps = a_eAEMinFrameRate;

    for (i4PlineIndex = (MINT32)m_u4IndexMin; i4PlineIndex <= (MINT32)m_u4IndexMax; i4PlineIndex++)
    {
        if (m_pPreviewTableNew != NULL)
        {
            if (m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime > 0)
            {
                i4fps = (10000000 / m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime); // the last two values always "0"
                //AE_LOG("[%s()] Index:%d Exp:%d Sensor Gain:%d ISP Gain:%d\n", __FUNCTION__, i4PlineIndex, m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime, m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4AfeGain, m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4IspGain);
            }
            else
            {
                i4fps = m_i4AEMinFps;
                AE_LOG("[%s()] The exposure time is zero\n", __FUNCTION__);
            }

            if (i4fps < (m_i4AEMinFps - m_i4AEMinFps / 20))
            {
                break;
            }
        }
        else
        {
            AE_LOG("[%s()] The preview current Pline table is NULL :%d %d %d %d %d %d", __FUNCTION__, m_i4AEMinFps, m_u4IndexMax, m_u4IndexMin, m_u4Index, i4PlineIndex, i4fps);
        }
    }

    if (i4PlineIndex > 0)
    {
        m_u4IndexMax = i4PlineIndex - 1;
    }
    else
    {
        m_u4IndexMax = i4PlineIndex;
    }
    m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
    if (m_u4Index > m_u4IndexMax)
    {
        m_u4Index = m_u4IndexMax;
    }
    if (m_u4IndexF > m_u4IndexFMax)
    {
        m_u4IndexF = m_u4IndexFMax;
    }

    AE_LOG_IF(m_3ALogEnable, "[%s()] m_u4FinerEVIdxBase:%d MaxFps/MinFps/MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF/SearchIdx/Searchfps:%d/%d/%d/%d/%d/%d/%d/%d/%d \n", __FUNCTION__, m_u4FinerEVIdxBase, a_eAEMaxFrameRate, m_i4AEMinFps, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF, i4PlineIndex, i4fps);

    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEMaxFrameRateGainIndexRange()
{
    MINT32 i4fps;
    MINT32 i430fps;
    MINT32 i4PlineIndex;
    MUINT32 u4GainValue;

    i430fps = 300;
    AE_LOG_IF(m_3ALogEnable, "[%s] Set Gain Index Range for LCE", __FUNCTION__);

    m_u4LCEGainStartIdx = 0;
    m_u4LCEGainEndIdx = 0;

    for (i4PlineIndex = (MINT32)m_u4IndexMin; i4PlineIndex <= (MINT32)m_u4IndexMax; i4PlineIndex++)
    {
        if (m_pPreviewTableNew != NULL)
        {
            if (m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime > 0)
            {
                i4fps = (10000000 / m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime); // the last two values always "0"
            }
            else
            {
                AE_LOG("[%s] The exposure time is zero\n", __FUNCTION__);
                break;
            }

            if ((i4fps > (i430fps * 9 / 10)) && (i4fps < (i430fps * 12 / 10)))
            {
                u4GainValue = (m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4AfeGain) * (m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4IspGain) >> 10;
                if (u4GainValue >= 4096)
                {
                    if (m_u4LCEGainStartIdx == 0)
                    {
                        m_u4LCEGainStartIdx = i4PlineIndex;
                    }
                    else
                    {
                        m_u4LCEGainEndIdx = i4PlineIndex;
                    }
                }
            }
        }
        else
        {
            AE_LOG("[%s] The preview new Pline table is NULL", __FUNCTION__);
        }
    }

    AE_LOG_IF(m_3ALogEnable, "[%s] m_u4LCEGainStartIdx:%d m_u4LCEGainEndIdx:%d", __FUNCTION__, m_u4LCEGainStartIdx, m_u4LCEGainEndIdx);
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32 AeMgr::getSenstivityDeltaIndex(MUINT32 u4NextSenstivity)
{
    MUINT32 u4Ratio = 100;
    MINT32 i4DeltaIndex;
    MUINT32 u4AEtablesize = sizeof(AESenstivityRatioTable) / sizeof(strAEMOVE);
    MUINT32 i;

    if (u4NextSenstivity != 0x00)
    {
        u4Ratio = 100 *SENSTIVITY_UINT / u4NextSenstivity;
    }
    else
    {
        AE_LOG("[%s()] The senstivity is zero\n", __FUNCTION__);
    }

    // calculate the different index for different senstivity
    if (sizeof(AESenstivityRatioTable) == 0)
    {
        CAM_LOGE("[%s()] No AE senstivity ratio table\n", __FUNCTION__);
        return 0;
    }

    i4DeltaIndex = AESenstivityRatioTable[u4AEtablesize -(MUINT32)1].Diff_EV;

    for (i = 0;i < u4AEtablesize;i++)
    {
        if (u4Ratio <= (MUINT32)AESenstivityRatioTable[i].Ration)
        {
            i4DeltaIndex = AESenstivityRatioTable[i].Diff_EV;
            break;
        }
    }
    if (i4DeltaIndex != 0)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s()] getSenstivityDeltaIndex:%d %d\n", __FUNCTION__, i4DeltaIndex, u4NextSenstivity);
    }
    return i4DeltaIndex;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::switchSensorModeMaxBVSensitivityDiff(MINT32 i4newSensorMode, MINT32 i4oldSensorMode, MINT32 &i4SenstivityDeltaIdx , MINT32 &i4BVDeltaIdx)
{
    eAETableID ePreviewPLineTableID, eCapturePLineTableID;
    MUINT32 u4CurRatio, u4PreRatio;
    MINT32 i4CurMaxBV, i4PreMaxBV;

    memset(&ePreviewPLineTableID, 0, sizeof(eAETableID));
    memset(&eCapturePLineTableID, 0, sizeof(eAETableID));

    // for sensor mode change to adjust the Pline index
    getAEPLineMappingID(m_eAEScene, LIB3A_SENSOR_MODE_PRVIEW, &ePreviewPLineTableID, &eCapturePLineTableID);
    getAETableMaxBV(m_pAEPlineTable, ePreviewPLineTableID, &m_i4PreviewMaxBV);
    getAEPLineMappingID(m_eAEScene, LIB3A_SENSOR_MODE_VIDEO, &ePreviewPLineTableID, &eCapturePLineTableID);
    getAETableMaxBV(m_pAEPlineTable, ePreviewPLineTableID, &m_i4VideoMaxBV);
    getAEPLineMappingID(m_eAEScene, LIB3A_SENSOR_MODE_CAPTURE_ZSD, &ePreviewPLineTableID, &eCapturePLineTableID);
    getAETableMaxBV(m_pAEPlineTable, ePreviewPLineTableID, &m_i4ZSDMaxBV);
    getAEPLineMappingID(m_eAEScene, LIB3A_SENSOR_MODE_VIDEO1, &ePreviewPLineTableID, &eCapturePLineTableID);
    getAETableMaxBV(m_pAEPlineTable, ePreviewPLineTableID, &m_i4Video1MaxBV);
    getAEPLineMappingID(m_eAEScene, LIB3A_SENSOR_MODE_VIDEO2, &ePreviewPLineTableID, &eCapturePLineTableID);
    getAETableMaxBV(m_pAEPlineTable, ePreviewPLineTableID, &m_i4Video2MaxBV);

    switch (i4newSensorMode)
    {
        case LIB3A_SENSOR_MODE_CAPTURE:
        case LIB3A_SENSOR_MODE_CAPTURE_ZSD:
            u4CurRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio;
            i4CurMaxBV = m_i4ZSDMaxBV;
            break;
        case LIB3A_SENSOR_MODE_VIDEO:
            u4CurRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Video2PreRatio;
            i4CurMaxBV = m_i4VideoMaxBV;
            break;
        case LIB3A_SENSOR_MODE_VIDEO1:
            u4CurRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Video12PreRatio;
            i4CurMaxBV = m_i4Video1MaxBV;
            break;
        case LIB3A_SENSOR_MODE_VIDEO2:
            u4CurRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Video22PreRatio;
            i4CurMaxBV = m_i4Video2MaxBV;
            break;
        default:
        case LIB3A_SENSOR_MODE_PRVIEW:
            u4CurRatio = 1024;
            i4CurMaxBV = m_i4PreviewMaxBV;
            break;
    }

    switch (i4oldSensorMode)
    {
        case LIB3A_SENSOR_MODE_CAPTURE:
        case LIB3A_SENSOR_MODE_CAPTURE_ZSD:
            u4PreRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio;
            i4PreMaxBV = m_i4ZSDMaxBV;
            break;
        case LIB3A_SENSOR_MODE_VIDEO:
            u4PreRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Video2PreRatio;
            i4PreMaxBV = m_i4VideoMaxBV;
            break;
        case LIB3A_SENSOR_MODE_VIDEO1:
            u4PreRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Video12PreRatio;
            i4PreMaxBV = m_i4Video1MaxBV;
            break;
        case LIB3A_SENSOR_MODE_VIDEO2:
            u4PreRatio = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Video22PreRatio;
            i4PreMaxBV = m_i4Video2MaxBV;
            break;
        default:
        case LIB3A_SENSOR_MODE_PRVIEW:
            u4PreRatio = 1024;
            i4PreMaxBV = m_i4PreviewMaxBV;
            break;
    }

    if (u4PreRatio != 0x00)
    {
        i4SenstivityDeltaIdx = getSenstivityDeltaIndex(SENSTIVITY_UINT *u4CurRatio / u4PreRatio);
    }
    else
    {
        AE_LOG("[%s()] u4PreRatio is zero:%d %d\n", __FUNCTION__, u4PreRatio, u4CurRatio);
        i4SenstivityDeltaIdx = 0;
    }
    i4BVDeltaIdx = i4CurMaxBV - i4PreMaxBV;
    m_i4DeltaSensitivityIdx = i4SenstivityDeltaIdx;
    m_i4DeltaBVIdx = i4BVDeltaIdx;
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setManualAETable(EAEManualPline_T a_eAEManualPline)
{
    MRESULT mr = S_AE_OK;
    eAETableID ePreviewPLineTableID = m_pPreviewTableCurrent->eID;
    eAETableID eCapturePLineTableID = m_pCaptureTable->eID;
    MUINT32 i;
    MBOOL bNewPreviewPLineTableID = MFALSE;
    MBOOL bNewCapturePLineTableID = MFALSE;

    getAEManualPline(a_eAEManualPline, ePreviewPLineTableID, eCapturePLineTableID);
    AE_LOG("[%s()] Preview TableID:%d->%d Capture TableID:%d->%d\n", __FUNCTION__,
        m_pPreviewTableCurrent->eID , ePreviewPLineTableID, m_pCaptureTable->eID , eCapturePLineTableID);
    char logString[100] = {"\0"};
    char tmpString[40] = {"\0"};
    // Search for eID
    for (i = 0; i < (MUINT32)MAX_PLINE_TABLE; i++)
    {
        if (m_3ALogEnable)
        {
            if (i % 10 == 0)
            {
                memset(logString, 0, sizeof(char)*100);
                sprintf(logString, "i/TotalIdx:");
            }
            memset(tmpString, 0, sizeof(char)*40);
            sprintf(tmpString, "%2d/%3d ", i, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
            strncat(logString, tmpString, 100 - strlen(logString) - 1);
            if (i % 10 == 9 || i == MAX_PLINE_TABLE - 1)
                AE_LOG("[%s()] %s\n", __FUNCTION__, logString);
        }
        if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].eID == ePreviewPLineTableID)
        {
            AE_LOG("[%s()] i/ePreviewPLineTableID/TotalIdx:%d/%d/%d\n", __FUNCTION__, i, ePreviewPLineTableID, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
            bNewPreviewPLineTableID = MTRUE;
            if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex == 0)
            {
                AE_LOG("[%s()] The Preview AE table total index is zero:%d->%d\n", __FUNCTION__, ePreviewPLineTableID, m_pPreviewTableCurrent->eID);
                ePreviewPLineTableID = m_pPreviewTableCurrent->eID;
                m_eAEManualPline = EAEManualPline_Default;
            }
        }
        if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].eID == eCapturePLineTableID)
        {
            AE_LOG("[%s()] i/eCapturePLineTableID/TotalIdx:%d/%d/%d\n", __FUNCTION__, i, eCapturePLineTableID, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
            bNewCapturePLineTableID = MTRUE;
            if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex == 0)
            {
                AE_LOG("[%s()] The Capture AE table total index is zero:%d->%d\n", __FUNCTION__, eCapturePLineTableID, m_pCaptureTable->eID);
                eCapturePLineTableID = m_pCaptureTable->eID;
                m_eAEManualPline = EAEManualPline_Default;
            }
        }
    }
    // Check AE Pline if not existed
    if (!bNewPreviewPLineTableID)
    {
        ePreviewPLineTableID = m_pPreviewTableCurrent->eID;
        AE_LOG("[%s()] The Preview AE table enum value is not supported:%d->%d\n", __FUNCTION__, ePreviewPLineTableID, m_pPreviewTableCurrent->eID);
    }
    if (!bNewCapturePLineTableID)
    {
        eCapturePLineTableID = m_pCaptureTable->eID;
        AE_LOG("[%s()] The Capture AE table enum value is not supported:%d->%d\n", __FUNCTION__, eCapturePLineTableID, m_pCaptureTable->eID);
    }

    mr = searchAETable(m_pAEPlineTable, ePreviewPLineTableID, &m_pPreviewTableNew);

    if (FAILED(mr))
    {
        CAM_LOGE("[%s()] Search Preview Pline table:%d error \n", __FUNCTION__, ePreviewPLineTableID);
    }

    mr = searchAETable(m_pAEPlineTable, eCapturePLineTableID, &m_pCaptureTable);

    if (FAILED(mr))
    {
        CAM_LOGE("[%s()] Search Capture Pline table:%d error \n", __FUNCTION__, eCapturePLineTableID);
    }

    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::updateAEidxtoExpsetting()
{
    AE_LOG_IF(m_3ALogEnable, "[%s()] m_u4Index/m_u4IndexF/m_i4DeltaSensitivityIdx/m_i4DeltaBVIdx:%d/%d/%d/%d\n", __FUNCTION__, m_u4Index, m_u4IndexF, m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);

    if (m_pPreviewTableCurrent->pCurrentTable != NULL)
    {
        if (m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_ADB_UPDATE))
        {
            m_pIAeFlowCPU->getParam(E_AE_FLOW_CPU_ADB_EXP, m_rAEOutput.rPreviewMode.u4Eposuretime);
            m_pIAeFlowCPU->getParam(E_AE_FLOW_CPU_ADB_AFE, m_rAEOutput.rPreviewMode.u4AfeGain);
            m_pIAeFlowCPU->getParam(E_AE_FLOW_CPU_ADB_ISP, m_rAEOutput.rPreviewMode.u4IspGain);
        }
        else
        {
            AE_LOG_IF(m_3ALogEnable, "[%s()] exp:%d\n", __FUNCTION__, m_PreviewTableF.sPlineTable[0].u4Eposuretime);

            if (m_PreviewTableF.sPlineTable[0].u4Eposuretime != 0)
            {
                m_rAEOutput.rPreviewMode.u4Eposuretime = m_PreviewTableF.sPlineTable[m_u4IndexF].u4Eposuretime;
                m_rAEOutput.rPreviewMode.u4AfeGain = m_PreviewTableF.sPlineTable[m_u4IndexF].u4AfeGain;
                m_rAEOutput.rPreviewMode.u4IspGain = m_PreviewTableF.sPlineTable[m_u4IndexF].u4IspGain;
            }
            else
            {
                m_rAEOutput.rPreviewMode.u4Eposuretime = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4Index].u4Eposuretime;
                m_rAEOutput.rPreviewMode.u4AfeGain = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4Index].u4AfeGain;
                m_rAEOutput.rPreviewMode.u4IspGain = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4Index].u4IspGain;
            }
        }
    }
    //calcuate ISO
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain = 1024; //m_p3AFactor->rAEParam.strAEPara.DeviceInfo.PreviewGain;
    MUINT32 u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;

    u4FinalGain = (m_rAEOutput.rPreviewMode.u4AfeGain * m_rAEOutput.rPreviewMode.u4IspGain) >> 10; //TransferMicroDb2Gain(aeoutput->EvSetting.u4AfeGain);
    m_rAEOutput.rPreviewMode.u4RealISO = u4PreviewBaseISO * u4FinalGain / u4PreviewBaseGain;
    AE_LOG("[%s()] m_u4Index/m_u4IndexF/Exp/Afe/Isp/ISO:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4Index, m_u4IndexF, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain , m_rAEOutput.rPreviewMode.u4RealISO);
    return S_AE_OK;
}

