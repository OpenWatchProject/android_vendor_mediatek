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

#include <cutils/properties.h>
#include <mtkcam/common.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <camera_custom_AEPlinetable.h>
//#include <CamDefs.h>
#include <mtkcam/common/faces.h>
#include <isp_tuning.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
//#include <sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include <kd_camera_feature.h>
#include <mtkcam/hal/IHalSensor.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <aaa_scheduler.h>
#include <aaa_common_custom.h>
#include "aaa_sensor_buf_mgr.h"

static strAEMOVE  AESenstivityRatioTable[]=      // for preview / capture
{
    {-20,   25,    20}, //   mean below -2  move increase 20 index
    {-15,   33,    15}, //    -1.5
    {-10,   50,    10}, //    -1
    { -9,    55,      9}, //    -0.9
    { -8,    58,      8}, //    -0.8
    { -7,    63,      7}, //    -0.7
    { -6,    67,      6}, //    -0.6
    { -5,    72,      5}, //    -0.5
    { -4,    77,      4}, //    -0.4
    { -3,    82,      3}, //    -0.3
    { -2,    88,      2}, //    -0.2
    { -1,    94,      1}, //    -0.1
    {   0,  100,      0}, //        0
    {   0,  107,      0}, //        0
    {   1,  108,    -1}, //     0.1
    {   2,  115,    -2}, //     0.2
    {   3,  123,    -3}, //     0.3
    {   4,  132,    -4}, //     0.4
    {   5,  141,    -5}, //     0.5
    {   6,  152,    -6}, //     0.6
    {   7,  162,    -7}, //     0.7
    {   8,  174,    -8}, //     0.8
    {   9,  186,    -9}, //     0.9
    { 10,  200,   -10}, //    1.0
    { 11,  214,   -11}, //    1.1
    { 12,  230,   -12}, //    1.2
    { 13,  246,   -13}, //    1.3
    { 14,  264,   -14}, //    1.4
    { 15,  283,   -15}, //    1.5
    { 16,  303,   -16}, //    1.6
    { 17,  325,   -17}, //    1.7
    { 18,  348,   -18}, //    1.8
    { 19,  373,   -19}, //    1.9
    { 20,  400,   -20}, //    2 EV
    { 21,  429,   -20}, //    2.1 EV
    { 22,  459,   -20}, //    2.2 EV
    { 23,  492,   -20}, //    2.3 EV
    { 24,  528,   -20}, //    2.4 EV
    { 25,  566,   -25}, //    2.5 EV
    { 30,  800,   -30}, //    3 EV
};

using namespace NS3A;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setSceneMode(MUINT32 u4NewAEScene)
{
    mtk_camera_metadata_enum_android_control_scene_mode_t eNewAEScene = static_cast<mtk_camera_metadata_enum_android_control_scene_mode_t>(u4NewAEScene);
    LIB3A_AE_SCENE_T eAEScene;

    if (eNewAEScene < MTK_CONTROL_SCENE_MODE_DISABLED) {
        MY_ERR("Unsupport AE mode: %d\n", eNewAEScene);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEScene) {
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
#if 0    // mark for mtk control scene mode define
        case MTK_CONTROL_SCENE_MODE_CSHOT:
            eAEScene = LIB3A_AE_SCENE_CSHOT;
            break;
        case MTK_CONTROL_SCENE_MODE_SWEEP_PANORAMA:
            eAEScene = LIB3A_AE_SCENE_SWEEP_PANORAMA;
            break;
#endif
        default:
            MY_LOG("The Scene mode is not correctly: %d\n", eNewAEScene);
            eAEScene = LIB3A_AE_SCENE_AUTO;
            break;
    }

    if (m_eAEScene != eAEScene) {
        MY_LOG("m_eAEScene: %d old:%d\n", eAEScene, m_eAEScene);
        m_eAEScene = eAEScene;
        if(m_pIAeAlgo != NULL) {

            //New AE P-Line
            m_pIAeAlgo->getEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);
            setAEScene(m_eAEScene);
            m_pIAeAlgo->updateAEPlineInfo(m_pPreviewTableCurrent,m_pCaptureTable);
            m_pIAeAlgo->setEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);
    
            m_pIAeAlgo->setAEScene(m_eAEScene);
            m_pIAeAlgo->getAEMaxISO(m_u4MaxShutter, m_u4MaxISO);
            if(m_bAELock == MFALSE) {

                if (m_bAEMonitorStable == MTRUE){
                    m_bAEMonitorStable = MFALSE;
                    m_u4AEScheduleCnt = 0;
                }
            }
            MY_LOG("m_u4MaxShutter:%d m_u4MaxISO:%d\n", m_u4MaxShutter, m_u4MaxISO);
        } else {
            m_u4MaxShutter = 100000;
            m_u4MaxISO = 800;
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d MaxShutter:%d MaxISO:%d", __FUNCTION__, m_eSensorDev, __LINE__, m_u4MaxShutter, m_u4MaxISO);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEScene() const
{
    return static_cast<MINT32>(m_eAEScene);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEISOSpeed(MUINT32 u4NewAEISOSpeed)
{
    MUINT32 eAEISOSpeed;

    if ((!m_bRealISOSpeed) && (u4NewAEISOSpeed > LIB3A_AE_ISO_SPEED_MAX)) {
        MY_ERR("Unsupport AE ISO Speed: %d\n", u4NewAEISOSpeed);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(u4NewAEISOSpeed) {
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
        default:
            if(m_bRealISOSpeed == MTRUE) {   //
                MY_LOG("The Real ISO speed:%d m_bRealISOSpeed:%d \n", u4NewAEISOSpeed, m_bRealISOSpeed);
                eAEISOSpeed = u4NewAEISOSpeed;                
            } else {            
                MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);
                eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_AUTO;
            }
            break;
    }

    if (m_u4AEISOSpeed != eAEISOSpeed) {
        MY_LOG("m_u4AEISOSpeed: %d old:%d\n", eAEISOSpeed, m_u4AEISOSpeed);
        m_u4AEISOSpeed = eAEISOSpeed;
        if(m_pIAeAlgo != NULL) {
            //New AE-Pline
            m_pIAeAlgo->getEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);
            setIsoSpeed(m_u4AEISOSpeed);
            m_pIAeAlgo->updateAEPlineInfo(m_pPreviewTableCurrent,m_pCaptureTable);
            m_pIAeAlgo->setEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);

            m_pIAeAlgo->setIsoSpeed(m_u4AEISOSpeed);
            if (m_bAEMonitorStable == MTRUE){
                m_bAEMonitorStable = MFALSE;
                m_u4AEScheduleCnt = 0;
            }

        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEISOSpeedMode() const
{
    return static_cast<MINT32>(m_u4AEISOSpeed);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAECamMode(MUINT32 u4NewAECamMode)
{
    EAppMode eNewAECamMode = static_cast<EAppMode>(u4NewAECamMode);

    if (m_eCamMode != eNewAECamMode) {
        m_eCamMode = eNewAECamMode;
        MY_LOG("m_eCamMode:%d AECamMode:%d \n", m_eCamMode, m_eAECamMode);
    }
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode)
{
    LIB3A_AE_FLICKER_AUTO_MODE_T eNewAEAutoFLKMode = static_cast<LIB3A_AE_FLICKER_AUTO_MODE_T>(u4NewAEAutoFLKMode);

    if ((eNewAEAutoFLKMode <= LIB3A_AE_FLICKER_AUTO_MODE_UNSUPPORTED) || (eNewAEAutoFLKMode >= LIB3A_AE_FLICKER_AUTO_MODE_MAX)) {
        MY_ERR("Unsupport AE auto flicker mode: %d\n", eNewAEAutoFLKMode);
        return E_AE_UNSUPPORT_MODE;
    }

    if (m_eAEAutoFlickerMode != eNewAEAutoFLKMode) {
        m_eAEAutoFlickerMode = eNewAEAutoFLKMode;
        MY_LOG("m_eAEAutoFlickerMode: %d\n", m_eAEAutoFlickerMode);
        if(m_pIAeAlgo != NULL) {
            //AE P-line
            m_pIAeAlgo->getEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);
            setAEFlickerAutoModePlineIdx(eNewAEAutoFLKMode);
            m_pIAeAlgo->updateAEPlineInfo(m_pPreviewTableCurrent,m_pCaptureTable);
            m_pIAeAlgo->setEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);

            m_pIAeAlgo->setAEFlickerAutoMode(m_eAEAutoFlickerMode);
            if (m_bAEMonitorStable == MTRUE){
                m_bAEMonitorStable = MFALSE;
            }
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEFlickerMode(MUINT32 u4NewAEFLKMode)
{
    mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t eNewAEFLKMode = static_cast<mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t>(u4NewAEFLKMode);
    LIB3A_AE_FLICKER_MODE_T eAEFLKMode;

    switch(eNewAEFLKMode) {
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
            MY_LOG("The flicker enum value is incorrectly:%d\n", eNewAEFLKMode);
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
    }

    if (m_eAEFlickerMode != eAEFLKMode) {
        MY_LOG("AEFlickerMode: %d old:%d\n", eAEFLKMode, m_eAEFlickerMode);
        m_eAEFlickerMode = eAEFLKMode;
        if(m_pIAeAlgo != NULL) {
            //AE P-line
            m_pIAeAlgo->getEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);
            setAEFlickerModePlineIdx(eAEFLKMode);
            m_pIAeAlgo->updateAEPlineInfo(m_pPreviewTableCurrent,m_pCaptureTable);
            m_pIAeAlgo->setEVIdxInfo(m_u4IndexMax ,m_u4IndexMin ,m_u4Index);

            m_pIAeAlgo->setAEFlickerMode(m_eAEFlickerMode);
            if (m_bAEMonitorStable == MTRUE){
                m_bAEMonitorStable = MFALSE;
                m_u4AEScheduleCnt = 0;
            }
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}
/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEFlickerModePlineIdx(LIB3A_AE_FLICKER_MODE_T a_eAEFlickerMode)
{
    MY_LOG("Set Flicker value %d %d\n",m_eAEFlickerMode, a_eAEFlickerMode);
    setIsoSpeed(m_u4AEISOSpeed);
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEFlickerAutoModePlineIdx(LIB3A_AE_FLICKER_AUTO_MODE_T a_eAEFlickerAutoMode)
{
    MY_LOG("setAEFlickerAutoMode:%d %d\n", m_eAEAutoFlickerMode, a_eAEFlickerAutoMode);
    setIsoSpeed(m_u4AEISOSpeed);
    return S_AE_OK;
}



/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setIsoSpeed(MUINT32  a_u4ISO)
{
    MRESULT mr;
    eAETableID ePreviewPLineTableID, eCapturePLineTableID;
    LIB3A_AE_SCENE_T eAEScene;

    eAEScene = m_eAEScene;
    MY_LOG_IF(m_3ALogEnable,"setIsoSpeed:%d %d\n", a_u4ISO, m_u4AEISOSpeed);

    if((m_u4AEISOSpeed == LIB3A_AE_ISO_SPEED_AUTO) || (m_bRealISOSpeed == MTRUE)) {
        setAEScene(m_eAEScene);
    } else {
        switch(a_u4ISO) {
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
            default :
                MY_LOG("Wrong ISO setting:%d\n", m_u4AEISOSpeed);
                break;
        }

        if(eAEScene != m_eAEScene) {
            mr = getAEPLineMappingID(eAEScene, m_eSensorMode, &ePreviewPLineTableID, &eCapturePLineTableID);
            if(FAILED(mr)) {
                MY_ERR("[setAEScene]Get capture table ERROR :%d %d PLineID:%d %d\n", m_eAEScene, eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
            }
            mr = setAETable(ePreviewPLineTableID, eCapturePLineTableID);
        }
    }
    return S_AE_OK;
}


/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAEScene(LIB3A_AE_SCENE_T  a_eAEScene)
{
  MRESULT mr;
  eAETableID ePreviewPLineTableID, eCapturePLineTableID;
  MY_LOG_IF(m_3ALogEnable,"[setAEScene]setAEScene:%d \n",a_eAEScene);
  
  if(m_pAEPlineTable != NULL) {   // protect the AE Pline table don't ready
      mr = getAEPLineMappingID(m_eAEScene, m_eSensorMode, &ePreviewPLineTableID, &eCapturePLineTableID);
      if(FAILED(mr)) {
          MY_ERR("[setAEScene]Get capture table ERROR :%d PLineID:%d %d\n", m_eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
      }

      mr = setAETable(ePreviewPLineTableID, eCapturePLineTableID);
      if(FAILED(mr)) {
          MY_ERR("[setAEScene]Capture table ERROR :%d PLineID:%d %d\n", m_eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
      }
  } else {
      MY_LOG("[setAEScene]setAEScene:%d, AE Pline table is NULL \n",a_eAEScene);
  }

  m_pPreviewTableCurrent = m_pPreviewTableNew;
  return S_AE_OK;
}


/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::setAETable(eAETableID a_AEPreTableID, eAETableID a_AECapTableID)
{
    MRESULT mr = S_AE_OK;

    MY_LOG_IF(m_3ALogEnable,"[setAETable] TableID:%d %d Flicker:%d Flicker Auto:%d i4SensorMode:%d\n", a_AEPreTableID, a_AECapTableID, m_eAEFlickerMode, m_eAEAutoFlickerMode, m_eSensorMode);

    mr = searchAETable(m_pAEPlineTable, a_AEPreTableID, &m_pPreviewTableNew);

    if(FAILED(mr)) {
        MY_ERR("[setAETable]Search Preview Pline table:%d error \n", a_AEPreTableID);
    }

    mr = searchAETable(m_pAEPlineTable, a_AECapTableID, &m_pCaptureTable);

    if(FAILED(mr)) {
        MY_ERR("[setAETable]Search Preview Pline table:%d error \n", a_AEPreTableID);
    }

    mr = searchPreviewIndexLimit();

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo)
{
    if(m_pIAeAlgo != NULL) {
        getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable, m_CurrentStrobetureTable);
        a_PrvAEPlineTable =  m_CurrentPreviewTable;
        a_CapAEPlineTable = m_CurrentCaptureTable;
        a_StrobeAEPlineTable = m_CurrentStrobetureTable;
        MY_LOG("[%s()] i4SensorDev:%d PreId:%d CapId:%d Strobe:%d\n", __FUNCTION__, m_eSensorDev, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, m_CurrentStrobetureTable.eID);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    if(m_eAECamMode == LIB3A_AECAM_MODE_ZSD) {
        a_StrobeAEPlineInfo = m_rAEInitInput.rAEPARAM.strStrobeZSDPLine;
    } else {
        a_StrobeAEPlineInfo = m_rAEInitInput.rAEPARAM.strStrobePLine;
    }

    MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d Strobe enable:%d AECamMode:%d\n", __FUNCTION__, m_eSensorDev, a_StrobeAEPlineInfo.bAFPlineEnable, m_eAECamMode);
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEPlineTable(eAETableID eTableID, strAETable &a_AEPlineTable)
{
    strAETable strAEPlineTable;
    memset(&strAEPlineTable, 0, sizeof(strAETable));

    if(m_pIAeAlgo != NULL) {
        getOnePlineTable(eTableID, strAEPlineTable);
        a_AEPlineTable =  strAEPlineTable;
        MY_LOG("[%s()] i4SensorDev:%d PreId:%d CapId:%d GetID:%d\n", __FUNCTION__, m_eSensorDev, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, eTableID);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable)
{
    MRESULT mr = S_AE_OK;

    if(m_pPreviewTableCurrent != NULL) {
        a_PrvAEPlineTable =  (strAETable) *m_pPreviewTableCurrent;
    } else {
        MY_LOG("Preview table is NULL\n");
    }

    if(m_pCaptureTable != NULL) {
        a_CapAEPlineTable = (strAETable) *m_pCaptureTable;
    } else {
        a_CapAEPlineTable = (strAETable) *m_pPreviewTableCurrent;
        MY_LOG("[getPlineTable] Capture table is NULL, Using preview table to instead\n");
    }

    if(m_pStrobeTable != NULL) {
        a_StrobeAEPlineTable = (strAETable) *m_pStrobeTable;
    } else {
        mr = searchAETable(m_pAEPlineTable, AETABLE_STROBE, &m_pStrobeTable);
        if(FAILED(mr)) {
            MY_LOG("[getPlineTable] Search Preview Pline table:%d error \n", AETABLE_STROBE);
            a_StrobeAEPlineTable = a_CapAEPlineTable;            
        } else {
            a_StrobeAEPlineTable = (strAETable) *m_pStrobeTable;
        }

        MY_LOG("Capture table is NULL, Using preview table to instead\n");
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
    if(FAILED(mr)) {
        MY_LOG("[getOnePlineTable] Search Pline table:%d error \n", eTableID);
        a_AEPlineTable = (strAETable) *m_pPreviewTableCurrent;            
    } else {
        a_AEPlineTable = (strAETable) *pPlineTable;
    }

    return S_AE_OK;
}




/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::searchAETable(AE_PLINETABLE_T *a_aepara ,eAETableID id,strAETable** a_ppPreAETable)
{
  MUINT32 i;

  if(a_aepara != NULL) {
    for(i=0;i<(MUINT32)MAX_PLINE_TABLE;i++) {
        if(a_aepara->AEPlineTable.sPlineTable[i].eID == id) {
            *a_ppPreAETable= &(a_aepara->AEPlineTable.sPlineTable[i]);
            if((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_60HZ) || 
               ((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_AUTO) && (m_eAEAutoFlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_60HZ))) {
                (*a_ppPreAETable)->pCurrentTable = &(a_aepara->AEPlineTable.sPlineTable[i].sTable60Hz);       //copy the 60Hz for current table used
            } else {
                (*a_ppPreAETable)->pCurrentTable = &(a_aepara->AEPlineTable.sPlineTable[i].sTable50Hz);      //copy the 50Hz for current table used
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

MRESULT AeMgr::getAEPLineMappingID(LIB3A_AE_SCENE_T  a_eAESceneID, MINT32 i4SensorMode, eAETableID *pAEPrePLineID, eAETableID *pAECapPLineID)
{
    MUINT32 i;

    MY_LOG_IF(m_3ALogEnable,"[%s()] m_eSensorDev: %d SceneId:%d ISO:%d CamMode:%d SensorIdx:%d\n", __FUNCTION__, m_eSensorDev, a_eAESceneID, m_u4AEISOSpeed, m_eCamMode, i4SensorMode);

    if(m_pAEPlineTable == NULL) {
        MY_ERR("[%s()] m_eSensorDev: %d, i4SensorMode = %d, CamMode:%d The pointer is NULL\n", __FUNCTION__, m_eSensorDev, m_eSensorMode, m_eCamMode);
        return E_AE_NULL_AE_TABLE;
    }

    *pAEPrePLineID = m_pAEMapPlineTable[0].ePLineID[LIB3A_SENSOR_MODE_PRVIEW];
    if(i4SensorMode == LIB3A_SENSOR_MODE_PRVIEW) {
        *pAECapPLineID = m_pAEMapPlineTable[0].ePLineID[LIB3A_SENSOR_MODE_CAPTURE];
    } else {
        *pAECapPLineID = *pAEPrePLineID;
    }

    if((i4SensorMode > LIB3A_SENSOR_MODE_CAPTURE_ZSD) || (i4SensorMode < LIB3A_SENSOR_MODE_PRVIEW)) {
        MY_ERR("[%s()] m_eSensorDev: %d, i4SensorMode error = %d, CamMode:%d\n", __FUNCTION__, m_eSensorDev, m_eSensorMode, m_eCamMode);
        return E_AE_NULL_AE_TABLE;
    }

    for(i=0; i<MAX_MAPPING_PLINE_TABLE; i++) {
        if(m_pAEMapPlineTable[i].eAEScene == LIB3A_AE_SCENE_UNSUPPORTED) {
            MY_LOG_IF(m_3ALogEnable,"[getAEPLineMappingID] No find Mapping PLine ID :%d\n", a_eAESceneID);
            break;
        }

        if(m_pAEMapPlineTable[i].eAEScene == a_eAESceneID) {
            if(i4SensorMode == LIB3A_SENSOR_MODE_CAPTURE) {
                *pAEPrePLineID = m_pAEMapPlineTable[i].ePLineID[LIB3A_SENSOR_MODE_CAPTURE_ZSD];
            } else {
                *pAEPrePLineID = m_pAEMapPlineTable[i].ePLineID[i4SensorMode];
            }
            if(i4SensorMode == LIB3A_SENSOR_MODE_PRVIEW) {
                *pAECapPLineID = m_pAEMapPlineTable[i].ePLineID[LIB3A_SENSOR_MODE_CAPTURE];
            } else {
                *pAECapPLineID = *pAEPrePLineID;
            }
            MY_LOG("[getAEPLineMappingID] SceneID:%d Pre:%d CapID:%d\n", a_eAESceneID, *pAEPrePLineID, *pAECapPLineID);
            return S_AE_OK;
        }
    }
    return S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::getAETableMaxBV(const AE_PLINETABLE_T *a_aepara ,eAETableID id,MINT32 *i4MaxBV)
{
  MUINT32 i;

  if(a_aepara != NULL) {
    for(i=0;i<(MUINT32)MAX_PLINE_TABLE;i++) {
        if(a_aepara->AEPlineTable.sPlineTable[i].eID == id) {
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
MRESULT AeMgr::searchPreviewIndexLimit( )
{
    MINT32   itemp;
    MINT32   iSenstivityDeltaIdx = 0;
    MINT32   i4TableDeltaIdx = 0;

    if(m_pPreviewTableNew == NULL) {
        MY_ERR("[searchPreviewIndexLimit] No preview table\n");
        return E_AE_NOMATCH_TABLE;
    }

    m_u4IndexMin=(MUINT32)0;
    m_u4IndexMax=m_pPreviewTableNew->u4TotalIndex-(MUINT32)1;


    setAEMinMaxFrameRatePlineIdx( m_i4AEMinFps, m_i4AEMaxFps);

    setAEMaxFrameRateGainIndexRange();   // for LCE used only
    MY_LOG("[%s] use preview table range  %d ~ %d \n",__FUNCTION__, m_u4IndexMin, m_u4IndexMax);

    if(m_pPreviewTableCurrent != NULL){
        i4TableDeltaIdx =  m_pPreviewTableNew->i4MaxBV - m_pPreviewTableCurrent->i4MaxBV;
        m_u4Index =  m_u4Index + i4TableDeltaIdx;
        MY_LOG("[%s] m_u4Index :%d ZSD delta Idx :%d NewBV:%d OldBV:%d AEScene:%d u4Cap2PreRatio:%d\n", __FUNCTION__,
            m_u4Index, i4TableDeltaIdx, m_pPreviewTableNew->i4MaxBV, m_pPreviewTableCurrent->i4MaxBV, m_eAEScene, m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Cap2PreRatio);
    }

    if((m_eSensorMode == LIB3A_SENSOR_MODE_PRVIEW) && (m_i4AEMinFps != m_i4AEMaxFps) && (m_u4IndexMax > m_u4IndexMin)){
        iSenstivityDeltaIdx = getSenstivityDeltaIndex(m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Cap2PreRatio);

        // capture mode max BV = ( m_pCaptureTable->i4MaxBV-i4DeltaIdx)
        itemp=(m_pPreviewTableNew->i4MaxBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset)-(( m_pCaptureTable->i4MaxBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset) + iSenstivityDeltaIdx);

        //preview EV range size small then capture   // run from Preview first index
        if(itemp< 0) {
            m_u4IndexMin=0;
            MY_LOG("MinIndex(max BV) reach Preview limitation , need extend preview table \n");
            if(m_pPreviewTableCurrent != NULL)
            MY_LOG("itemp1:%d NewBV:%d OldBV:%d BVOffset:%d Idx:%d\n",itemp, m_pPreviewTableNew->i4MaxBV, m_pPreviewTableCurrent->i4MaxBV, m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset, iSenstivityDeltaIdx);
        } else {    //capture BV range small then preview ,make a offset
            m_u4IndexMin=(MUINT32)itemp;
        }

        itemp=(m_pPreviewTableNew->i4MaxBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset)-((m_pCaptureTable->i4MinBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset) + iSenstivityDeltaIdx);

        if(itemp >= (MINT32)m_pPreviewTableNew->u4TotalIndex) {   //capture range bigger then preview
            m_u4IndexMax=m_pPreviewTableNew->u4TotalIndex-(MUINT32)1;
            MY_LOG("Max (min BV) reach Preview limitation , need extend preview table \n");
            if(m_pPreviewTableCurrent != NULL)
            MY_LOG("itemp2:%d NewBV:%d OldBV:%d BVOffset:%d Idx:%d\n",itemp, m_pPreviewTableNew->i4MaxBV, m_pPreviewTableCurrent->i4MaxBV, m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset, iSenstivityDeltaIdx);
        } else {
            if (m_u4IndexMax >= (MUINT32)itemp) // Choose smaller index between capture max index and minfps index
                m_u4IndexMax=(MUINT32)itemp;
        }


            MY_LOG_IF(m_3ALogEnable,"Preview table range  BV %d~ %d   total %d index\n",(m_pPreviewTableNew->i4MaxBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset),(m_pPreviewTableNew->i4MinBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset),m_pPreviewTableNew->u4TotalIndex);
            MY_LOG_IF(m_3ALogEnable,"Capture table range  BV %d~ %d   total %d index\n",(m_pCaptureTable->i4MaxBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset),(m_pCaptureTable->i4MinBV-m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset),m_pCaptureTable->u4TotalIndex);
            MY_LOG_IF(m_3ALogEnable,"support range BV %d ~%d\n", m_pPreviewTableNew->i4MaxBV - m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset -(MINT32)m_u4IndexMin, m_pPreviewTableNew->i4MaxBV - m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset -(MINT32)m_u4IndexMax);
            MY_LOG_IF(m_3ALogEnable,"corresponding preview table index  %d~%d m_u4Index:%d Delta:%d\n",m_u4IndexMin, m_u4IndexMax, m_u4Index, iSenstivityDeltaIdx);

    }

    if(m_u4Index > m_u4IndexMax) {
        MY_LOG("m_u4Index1:%d m_u4IndexMax:%d m_u4IndexMin:%d\n", m_u4Index, m_u4IndexMax, m_u4IndexMin);
        m_u4Index = m_u4IndexMax;
    } else if(m_u4Index < m_u4IndexMin) {
        MY_LOG("m_u4Index2:%d m_u4IndexMax:%d m_u4IndexMin:%d\n", m_u4Index, m_u4IndexMax, m_u4IndexMin);
        m_u4Index = m_u4IndexMin;
    }

    m_pPreviewTableCurrent = m_pPreviewTableNew;  // copy to current table
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
  MY_LOG_IF(m_3ALogEnable,"The max frame rate :%d %d %d %d %d",a_eAEMaxFrameRate, m_u4IndexMax, m_u4IndexMin, m_u4Index, i4fps);

  i4fps = a_eAEMinFrameRate;
  m_i4AEMinFps = a_eAEMinFrameRate;

  if((m_i4AEMinFps != m_i4AEMaxFps) || (m_bVideoDynamic == MFALSE)) {
    for(i4PlineIndex = (MINT32)m_u4IndexMin; i4PlineIndex<= (MINT32)m_u4IndexMax ; i4PlineIndex++) {
      if(m_pPreviewTableNew != NULL) {
        if(m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime > 0) {
          i4fps = (10000000 / m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime);  // the last two values always "0"
          //MY_LOG("[setAEMinMaxFrameRate] Index:%d Exp:%d Sensor Gain:%d ISP Gain:%d\n", i4PlineIndex, m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime, m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4AfeGain, m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4IspGain);          
        } else {
          i4fps =  m_i4AEMinFps;
          MY_LOG("[setAEMinMaxFrameRate] The exposure time is zero\n");
        }

        if(i4fps < (m_i4AEMinFps - m_i4AEMinFps / 20)) {
          break;
        }
      } else {
        MY_LOG("The preview current Pline table is NULL :%d %d %d %d %d %d",m_i4AEMinFps, m_u4IndexMax, m_u4IndexMin, m_u4Index, i4PlineIndex, i4fps);
      }
    }
    if(i4PlineIndex > 0) {
       m_u4IndexMax = i4PlineIndex - 1;
    } else {
       m_u4IndexMax = i4PlineIndex;
    }

    if(m_u4Index > m_u4IndexMax) {
      m_u4Index = m_u4IndexMax;
    }
  }else {
    MY_LOG("[setAEMinMaxFrameRate] Skip search range MaxFps:%d MinFps:%d VideoDynamic:%d\n", m_i4AEMaxFps, m_i4AEMinFps, m_bVideoDynamic);
  }

  MY_LOG_IF(m_3ALogEnable,"The min frame rate :%d %d %d %d %d %d",m_i4AEMinFps, m_u4IndexMax, m_u4IndexMin, m_u4Index, i4PlineIndex, i4fps);
  return  S_AE_OK;
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
  MY_LOG("Set Gain Index Range for LCE");

  m_u4LCEGainStartIdx = 0;
  m_u4LCEGainEndIdx = 0;

  for(i4PlineIndex = (MINT32)m_u4IndexMin; i4PlineIndex<= (MINT32)m_u4IndexMax ; i4PlineIndex++) {
    if(m_pPreviewTableNew != NULL) {
      if(m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime > 0) {
        i4fps = (10000000 / m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4Eposuretime);  // the last two values always "0"
      } else {
        MY_LOG("[setAEMaxFrameRateGainIndexRange] The exposure time is zero\n");
        break;
      }

      if((i4fps > (i430fps*9/10)) && (i4fps < (i430fps*12/10))) {
        u4GainValue = (m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4AfeGain) * (m_pPreviewTableNew->pCurrentTable->sPlineTable[i4PlineIndex].u4IspGain) >>10;
        if(u4GainValue >= 4096) {
          if(m_u4LCEGainStartIdx == 0) {
            m_u4LCEGainStartIdx = i4PlineIndex;
          } else {
            m_u4LCEGainEndIdx = i4PlineIndex;
          }
        }
      }
    } else {
      MY_LOG("The preview new Pline table is NULL");
    }
  }

  MY_LOG("[setAEMaxFrameRateGainIndexRange]  m_u4LCEGainStartIdx:%d m_u4LCEGainEndIdx:%d", m_u4LCEGainStartIdx, m_u4LCEGainEndIdx);
  return  S_AE_OK;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32 AeMgr::getSenstivityDeltaIndex(MUINT32 u4NextSenstivity)
{
  MUINT32 u4Ratio = 100;
  MINT32  i4DeltaIndex;
  MUINT32 u4AEtablesize= sizeof(AESenstivityRatioTable)/sizeof(strAEMOVE);
  MUINT32 i;

  if(u4NextSenstivity != 0x00) {
    u4Ratio = 100*SENSTIVITY_UINT / u4NextSenstivity;
  } else {
    MY_LOG("[getSenstivityDeltaIndex] The senstivity is zero\n");
  }

  // calculate the different index for different senstivity
  /*if(AESenstivityRatioTable ==NULL) {
    MY_ERR("[getSenstivityDeltaIndex] No AE senstivity ratio table\n");
    return 0;
  }*/

  i4DeltaIndex= AESenstivityRatioTable[u4AEtablesize-(MUINT32)1].Diff_EV;

  for (i=0 ;i<u4AEtablesize;i++) {
    if( u4Ratio <= (MUINT32)AESenstivityRatioTable[i].Ration) {
      i4DeltaIndex=AESenstivityRatioTable[i].Diff_EV;
      break;
    }
  }
  if(i4DeltaIndex != 0) {
      MY_LOG_IF(m_3ALogEnable,"getSenstivityDeltaIndex:%d %d\n",i4DeltaIndex, u4NextSenstivity);
  }
  return i4DeltaIndex;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRESULT AeMgr::switchSensorModeMaxBVSensitivityDiff(MINT32 i4newSensorMode, MINT32 i4oldSensorMode, MINT32 &i4SenstivityDeltaIdx ,MINT32 &i4BVDeltaIdx )
{
    eAETableID ePreviewPLineTableID, eCapturePLineTableID;
    MUINT32 u4CurRatio, u4PreRatio;
    MINT32 i4CurMaxBV, i4PreMaxBV;

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
  
      switch(i4newSensorMode) {
          case LIB3A_SENSOR_MODE_CAPTURE:
          case LIB3A_SENSOR_MODE_CAPTURE_ZSD:
              u4CurRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Cap2PreRatio;
              i4CurMaxBV = m_i4ZSDMaxBV;
              break;
          case LIB3A_SENSOR_MODE_VIDEO:
              u4CurRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Video2PreRatio;
              i4CurMaxBV = m_i4VideoMaxBV;
              break;
          case LIB3A_SENSOR_MODE_VIDEO1:
              u4CurRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Video12PreRatio;
              i4CurMaxBV = m_i4Video1MaxBV;
              break;
          case LIB3A_SENSOR_MODE_VIDEO2:
              u4CurRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Video22PreRatio;
              i4CurMaxBV = m_i4Video2MaxBV;
              break;
          default:
          case LIB3A_SENSOR_MODE_PRVIEW:
              u4CurRatio = 1024;
              i4CurMaxBV = m_i4PreviewMaxBV;
              break;
      }
  
      switch(i4oldSensorMode) {
          case LIB3A_SENSOR_MODE_CAPTURE:
          case LIB3A_SENSOR_MODE_CAPTURE_ZSD:
              u4PreRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Cap2PreRatio;
              i4PreMaxBV = m_i4ZSDMaxBV;
              break;
          case LIB3A_SENSOR_MODE_VIDEO:
              u4PreRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Video2PreRatio;
              i4PreMaxBV = m_i4VideoMaxBV;
              break;
          case LIB3A_SENSOR_MODE_VIDEO1:
              u4PreRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Video12PreRatio;
              i4PreMaxBV = m_i4Video1MaxBV;
              break;
          case LIB3A_SENSOR_MODE_VIDEO2:
              u4PreRatio = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4Video22PreRatio;
              i4PreMaxBV = m_i4Video2MaxBV;
              break;
          default:
          case LIB3A_SENSOR_MODE_PRVIEW:
              u4PreRatio = 1024;
              i4PreMaxBV = m_i4PreviewMaxBV;
              break;
      }
  
      if(u4PreRatio != 0x00) {
          i4SenstivityDeltaIdx = getSenstivityDeltaIndex(SENSTIVITY_UINT*u4CurRatio / u4PreRatio);
      } else {
          MY_LOG("[switchAELock] u4PreRatio is zero:%d %d\n", u4PreRatio, u4CurRatio);
          i4SenstivityDeltaIdx = 0;
      }
      i4BVDeltaIdx =  i4CurMaxBV - i4PreMaxBV;
      m_i4DeltaSensitivityIdx = i4SenstivityDeltaIdx ;
      m_i4DeltaBVIdx = i4BVDeltaIdx;
  return  S_AE_OK;
}


