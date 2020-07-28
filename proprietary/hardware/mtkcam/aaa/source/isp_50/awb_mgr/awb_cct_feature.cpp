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
#define LOG_TAG "awb_cct_feature"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <af_param.h>
#include <awb_param.h>
//#include <mtkcam/algorithm/lib3a/awb_algo_if.h>

#include <mtkcam/drv/IHalSensor.h>
#include <awb_tuning_custom.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <nvbuf_util.h>
#include <awb_mgr_if.h>
#include "awb_mgr.h"

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBEnable()
{
    MBOOL ret = MFALSE;
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN]\n");

    ret = enableAWB();

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBDisable()
{
    MBOOL ret = MFALSE;
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN]\n");

    ret = disableAWB();

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBGetEnableInfo(
    MINT32 *a_pEnableAWB,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO]\n");

    *a_pEnableAWB = isAWBEnable();

    *a_pOutLen = sizeof(MINT32);

    CAM_LOGD("AWB Enable = %d\n", *a_pEnableAWB);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBGetAWBGain(
    MVOID *a_pAWBGain,
    MUINT32 *a_pOutLen
)
{
    AWB_GAIN_T *pAWBGain = (AWB_GAIN_T *)a_pAWBGain;

    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_GET_GAIN]\n");

    *pAWBGain = m_rAWBOutput.rPreviewAWBGain;

    *a_pOutLen = sizeof(AWB_GAIN_T);

    CAM_LOGD("[RGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4R);
    CAM_LOGD("[GGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4G);
    CAM_LOGD("[BGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4B);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBSetAWBGain(
    MVOID *a_pAWBGain
)
{
    AWB_GAIN_T *pAWBGain = (AWB_GAIN_T *)a_pAWBGain; 

    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_SET_GAIN]\n"); 
    memcpy(&m_rAWBOutput.rAWBInfo.rRPG, pAWBGain, sizeof(AWB_GAIN_T)); 
    memcpy(&m_rAWBOutput.rAWBInfo.rPGN, pAWBGain, sizeof(AWB_GAIN_T)); 
    IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo); 

    CAM_LOGD( "[%s()] awb_gain: %d/%d/%d, PreGain1: %d/%d/%d, RPG: %d/%d/%d, PGN: %d/%d/%d, bAWBLock: %d (m_bAWBLock:%d, m_eAWBMode:%d)\n",
    __FUNCTION__,m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R, m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G, m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B,
    m_rAWBOutput.rAWBInfo.rPregain1.i4R, m_rAWBOutput.rAWBInfo.rPregain1.i4G, m_rAWBOutput.rAWBInfo.rPregain1.i4B, 
    m_rAWBOutput.rAWBInfo.rRPG.i4R, m_rAWBOutput.rAWBInfo.rRPG.i4G, m_rAWBOutput.rAWBInfo.rRPG.i4B, 
    m_rAWBOutput.rAWBInfo.rPGN.i4R, m_rAWBOutput.rAWBInfo.rPGN.i4G, m_rAWBOutput.rAWBInfo.rPGN.i4B, 
    m_rAWBOutput.rAWBInfo.bAWBLock, m_bAWBLock, m_eAWBMode); 

return S_AWB_OK; 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBApplyNVRAMParam(
    MVOID *a_pAWBNVRAM,
    MUINT32 u4CamScenarioMode
)
{

    CAM_SCENARIO_T eCamScenarioMode = static_cast<CAM_SCENARIO_T>(u4CamScenarioMode);
    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    // set nvram
    m_pNVRAM_3A->AWB[eCamScenarioMode] = *pAWBNVRAM;

    // apply rAWBNVRAM only when rAWBNVRAM is the same set with the new one(pAWBNVRAM) ==> AWBNVRAMMapping[m_eCamScenarioMode] == u4CamScenarioMode
    MUINT32 u4AWBNVRAMIdx = AWBNVRAMMapping[m_eCamScenarioMode];
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2] applyScenario(%d), currentScenario(%d) -> mappingScenario(%d) \n", (int)eCamScenarioMode, (int)m_eCamScenarioMode, u4AWBNVRAMIdx);

    if (u4AWBNVRAMIdx == eCamScenarioMode)
    {
    m_rAWBInitInput.rAWBNVRAM = *pAWBNVRAM;

    AWBWindowConfig();
    AWBStatConfig();

    m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                                  m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                  m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

    m_pIAwbAlgo->updateAWBParam(m_rAWBInitInput);

    AWBRAWPreGain1Config();

    m_bAWBModeChanged = MTRUE;
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBApplyK71NVRAMParam(
    MVOID *a_pAWBNVRAM
)
{

    NVRAM_CAMERA_3A_STRUCT *pNvram = reinterpret_cast<NVRAM_CAMERA_3A_STRUCT*>(a_pAWBNVRAM);
    memcpy(&m_pNVRAM_3A->AWB,&pNvram->AWB, sizeof (m_pNVRAM_3A->AWB));
    CAM_LOGD("[%s] pAWBNVRAM = %d/%d/%d  m_pNVRAM_3A->AWB= %d/%d/%d m_i4AWBNvramIdx:%d" ,__FUNCTION__,
    pNvram->AWB[0].rAlgoTuningParam.rLightPrefGain[3][10].i4R,
    pNvram->AWB[0].rAlgoTuningParam.rLightPrefGain[3][10].i4G,
    pNvram->AWB[0].rAlgoTuningParam.rLightPrefGain[3][10].i4B,
    m_pNVRAM_3A->AWB[0].rAlgoTuningParam.rLightPrefGain[3][10].i4R,
    m_pNVRAM_3A->AWB[0].rAlgoTuningParam.rLightPrefGain[3][10].i4G,
    m_pNVRAM_3A->AWB[0].rAlgoTuningParam.rLightPrefGain[3][10].i4B, m_i4AWBNvramIdx);
    memcpy(&m_rAWBInitInput.rAWBNVRAM, &pNvram->AWB, sizeof(m_rAWBInitInput.rAWBNVRAM));

        if(a_pAWBNVRAM != NULL)
        {
            getSensorResolution();

            if (!AWBInit())
            {
                CAM_LOGE("AWBInit() fail\n");
                return MFALSE;
            }
        }
    return S_AWB_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetNVRAMParam(
    MVOID *a_pAWBNVRAM,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_GET_AWB_PARA]\n");
    if(a_pAWBNVRAM == NULL)
    {
        CAM_LOGE("[%s] a_pAWBNVRAM is Null\n", __FUNCTION__);
        return E_AWB_NVRAM_ERROR;
    }

    // Get default AWB calibration data
    MRESULT err = S_AWB_OK;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A, MTRUE);
    if(err!=0)
    {
        CAM_LOGE("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

    memcpy(a_pAWBNVRAM, m_pNVRAM_3A->AWB, sizeof(AWB_NVRAM_T)*AWB_CAM_SCENARIO_NUM);
    *a_pOutLen = sizeof(AWB_NVRAM_T)*CAM_SCENARIO_NUM;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetNVRAMParam(
    CAM_SCENARIO_T eIdx,
    MVOID *a_pAWBNVRAM
)
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_GET_AWB_PARA]\n");

    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    // Get default AWB calibration data
    MRESULT err = S_AWB_OK;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A, MTRUE);
    if(err!=0)
    {
        CAM_LOGE("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

    *pAWBNVRAM = m_pNVRAM_3A->AWB[eIdx];

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBSaveNVRAMParam(
 )
{
    CAM_LOGD("[ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA]\n");

    MRESULT err = S_AWB_OK;

    //m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx] = m_rAWBInitInput.rAWBNVRAM;

    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, m_eSensorDev);
    if (err != 0)
    {
        CAM_LOGE("NvBufUtil::getInstance().write\n");
        return E_AWB_NVRAM_ERROR;
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBSetAWBMode(
    MINT32 a_AWBMode
)
{
    CAM_LOGD("[ACDK_CCT_OP_AWB_SET_AWB_MODE]\n");

    //setAWBMode(a_AWBMode);

    LIB3A_AWB_MODE_T eNewAWBMode = static_cast<LIB3A_AWB_MODE_T>(a_AWBMode);

    if (m_eAWBMode != eNewAWBMode)
    {
        m_eAWBMode = eNewAWBMode;
        m_bAWBModeChanged = MTRUE;
        CAM_LOGD("m_eAWBMode: %d\n", m_eAWBMode);
    }

    CAM_LOGD("[AWB Mode] = %d\n", a_AWBMode);


    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetAWBMode(
    MINT32 *a_pAWBMode,
    MUINT32 *a_pOutLen
)
{
    CAM_LOGD("[ACDK_CCT_OP_AWB_GET_AWB_MODE]\n");

    *a_pAWBMode = getAWBMode();

    *a_pOutLen = sizeof(MINT32);

    CAM_LOGD("[AWB Mode] = %d\n", *a_pAWBMode);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetLightProb(
    MVOID *a_pAWBLightProb,
    MUINT32 *a_pOutLen
)
{
#if 1 //acdk and awbMgr has different AWB_LIGHT_PROBABILITY_T define
    AWB_LIGHT_PROBABILITY_T pAWBLightProb;
    
    CAM_LOGD("[ACDK_CCT_OP_AWB_GET_LIGHT_PROB] fixed size for ACDK\n");

    m_pIAwbAlgo->getLightProb(pAWBLightProb);

    *a_pOutLen = sizeof(MINT32) * AWB_LIGHT_NUM * 4;
    
    memcpy(a_pAWBLightProb, &pAWBLightProb, *a_pOutLen);

    return S_AWB_OK;
#else
    AWB_LIGHT_PROBABILITY_T *pAWBLightProb = reinterpret_cast<AWB_LIGHT_PROBABILITY_T*>(a_pAWBLightProb);

    CAM_LOGD("[ACDK_CCT_OP_AWB_GET_LIGHT_PROB]\n");

    m_pIAwbAlgo->getLightProb(*pAWBLightProb);

    *a_pOutLen = sizeof(AWB_LIGHT_PROBABILITY_T);

    return S_AWB_OK;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBBypassCalibration(
    MBOOL bBypassCalibration
)
{
    m_bAWBCalibrationBypassed =  bBypassCalibration;

    CAM_LOGD("[AWB Calibration Bypass] = %d\n", m_bAWBCalibrationBypassed);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashAWBApplyNVRAMParam(
    MVOID *a_pFlashAWBNVRAM
)
{
    FLASH_AWB_NVRAM_T *pFlashAWBNVRAM = reinterpret_cast<FLASH_AWB_NVRAM_T*>(a_pFlashAWBNVRAM);

    CAM_LOGD("[ACDK_CCT_OP_AWB_APPLY_FLASH_AWB_PARA]\n");

    m_pNVRAM_STROBE->Flash_AWB[0] = *pFlashAWBNVRAM;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashAWBGetNVRAMParam(
    MVOID *a_pFlashAWBNVRAM,
    MUINT32 *a_pOutLen
)
{
    (void)a_pOutLen;

    FLASH_AWB_NVRAM_T *pFlashAWBNVRAM = reinterpret_cast<FLASH_AWB_NVRAM_T*>(a_pFlashAWBNVRAM);

    CAM_LOGD("[ACDK_CCT_OP_AWB_GET_FLASH_AWB_PARA]\n");

    // Get default AWB calibration data
    MRESULT err = S_AWB_OK;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A, MTRUE);
    if (err!=0)
    {
        CAM_LOGE("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

    *pFlashAWBNVRAM = m_pNVRAM_STROBE->Flash_AWB[0];

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashAWBSaveNVRAMParam(
 )
{
    MRESULT err = S_AWB_OK;

    CAM_LOGD("[ACDK_CCT_OP_AWB_SAVE_FLASH_AWB_PARA]\n");

    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, m_eSensorDev);
    if (err != 0)
    {
        CAM_LOGE("NvBufUtil::getInstance().write\n");
        return E_AWB_NVRAM_ERROR;
    }

    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashCalibrationSaveNVRAMParam(
 )
{
    MRESULT err = S_AWB_OK;

    CAM_LOGD("[CCTOPFlashCalibrationSaveNVRAMParam]\n");

    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, m_eSensorDev);
    if (err != 0)
    {
        CAM_LOGE("NvBufUtil::getInstance().write\n");
        return E_AWB_NVRAM_ERROR;
    }

    return err;
}


