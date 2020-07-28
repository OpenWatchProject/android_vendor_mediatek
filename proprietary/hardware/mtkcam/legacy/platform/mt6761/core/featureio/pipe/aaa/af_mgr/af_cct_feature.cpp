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
#define LOG_TAG "af_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/acdk/cct_feature.h>
#include <af_feature.h>
#include "af_mgr.h"
#include "nvbuf_util.h"

using namespace NS3A;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTMCUNameinit( MINT32 /*i4SensorIdx*/)
{
    CAM_LOGD( "%s +", __FUNCTION__);
    Mutex::Autolock lock( m_Lock);

    if( m_CCTUsers > 0)
    {
        CAM_LOGD( "[CCTMCUNameinit] no init, %d has created", m_CCTUsers);
        android_atomic_inc( &m_CCTUsers);
        return S_3A_OK;
    }
    android_atomic_inc(&m_CCTUsers);
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    switch( m_i4CurrSensorDev)
    {
    case ESensorDev_Main:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;
    case ESensorDev_Sub:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;
    case ESensorDev_MainSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;
    case ESensorDev_SubSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        break;
    default:
        CAM_LOGE( "Invalid sensor device: %d", m_i4CurrSensorDev);
        break;
    }
    m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
    MCUDrv::lensSearch( m_i4CurrSensorDev, m_i4CurrSensorId);
    m_i4CurrLensId   = MCUDrv::getCurrLensID( m_i4CurrSensorDev);
    CAM_LOGD( "[lens][SensorDev]0x%04x, [SensorId]0x%04x, [CurrLensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

    if( m_i4CurrLensId==0xFFFF) m_i4EnableAF = 0;
    else                        m_i4EnableAF = 1;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrLensNVRam);
    if(err!=0)
    {
        CAM_LOGE( "AfAlgo NvBufUtil get buf fail!");
    }


    CAM_LOGD( "%s -", __FUNCTION__);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTMCUNameuninit()
{
    CAM_LOGD_IF( m_i4DgbLogLv, "%s +", __FUNCTION__);
    Mutex::Autolock lock( m_Lock);

    if( m_CCTUsers<=0)
    {
        return S_3A_OK;
    }

    android_atomic_dec( &m_CCTUsers);
    if( m_CCTUsers!=0)
    {
        CAM_LOGD( "[CCTMCUNameuninit] Still %d users", m_CCTUsers);
        return S_AF_OK;
    }

    m_i4EnableAF = -1;
    CAM_LOGD( "%s - %d %d", __FUNCTION__, m_i4EnableAF, m_CCTUsers);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFOpeartion()
{
    CAM_LOGD( "%s", __FUNCTION__);
    setAFMode( AF_MODE_AFS, AF_MGR_CALLER);
    triggerAF( AF_MGR_CALLER);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPMFOpeartion( MINT32 a_i4MFpos)
{
    MINT32 i4TimeOutCnt = 0;

    CAM_LOGD( "%s %d", __FUNCTION__, a_i4MFpos);

    setAFMode( AF_MODE_MF, AF_MGR_CALLER);
    triggerAF( AF_MGR_CALLER);
    setMFPos(  a_i4MFpos, AF_MGR_CALLER);

    while( !isFocusFinish())
    {
        usleep( 5000); // 5ms
        i4TimeOutCnt++;
        if( i4TimeOutCnt>100)
        {
            break;
        }
    }

    //[TODO]:CAM_LOGD("[MF]pos:%d, value:%lld\n", a_i4MFpos, m_sAFInput.sAFStat.i8Stat24);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetAFInfo( MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    ACDK_AF_INFO_T *pAFInfo = (ACDK_AF_INFO_T *)a_pAFInfo;

    CAM_LOGD_IF( m_i4DgbLogLv, "%s", __FUNCTION__);

    pAFInfo->i4AFMode  = m_eLIB3A_AFMode;
    pAFInfo->i4AFMeter = LIB3A_AF_METER_SPOT;
    pAFInfo->i4CurrPos = m_sAFOutput.i4AFPos;

    *a_pOutLen = sizeof( ACDK_AF_INFO_T);

    CAM_LOGD( "[AF Mode] = %d", pAFInfo->i4AFMode);
    CAM_LOGD( "[AF Meter] = %d", pAFInfo->i4AFMeter);
    CAM_LOGD( "[AF Current Pos] = %d", pAFInfo->i4CurrPos);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetBestPos( MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s %d", __FUNCTION__, m_sAFOutput.i4AFBestPos);
    *a_pAFBestPos = m_sAFOutput.i4AFBestPos;
    *a_pOutLen    = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFCaliOperation( MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    ACDK_AF_CALI_DATA_T *pAFCaliData = (ACDK_AF_CALI_DATA_T *)a_pAFCaliData;
    AF_DEBUG_INFO_T rAFDebugInfo;
    MUINT32 aaaDebugSize;
    MINT32 i4TimeOutCnt = 0;

    CAM_LOGD( "%s", __FUNCTION__);

    setAFMode(AF_MODE_AFS, AF_MGR_CALLER);
    usleep( 500000);    // 500ms
    m_eLIB3A_AFMode = LIB3A_AF_MODE_CALIBRATION;
    if(m_pIAfAlgo)
        m_pIAfAlgo->setAFMode( m_eLIB3A_AFMode);

    usleep( 500000);    // 500ms
    while( !isFocusFinish())
    {
        usleep( 30000); // 30ms
        i4TimeOutCnt++;
        if( i4TimeOutCnt>2000)
        {
            break;
        }
    }

    getDebugInfo( rAFDebugInfo);
    pAFCaliData->i4Gap = (MINT32)rAFDebugInfo.Tag[3].u4FieldValue;

    for( MINT32 i=0; i<512; i++)
    {
        if( rAFDebugInfo.Tag[i+4].u4FieldValue != 0)
        {
            pAFCaliData->i8Vlu[i] = (MINT64)rAFDebugInfo.Tag[i+4].u4FieldValue;  // need fix it
            pAFCaliData->i4Num = i+1;
        }
        else
        {
            break;
        }
    }

    pAFCaliData->i4BestPos = m_sAFOutput.i4AFBestPos;

    CAM_LOGD( "[AFCaliData] Num = %d", pAFCaliData->i4Num);
    CAM_LOGD( "[AFCaliData] Gap = %d", pAFCaliData->i4Gap);

    for( MINT32 i=0; i<pAFCaliData->i4Num; i++)
    {
        CAM_LOGD( "[AFCaliData] Vlu %d = %lld", i, (long long)pAFCaliData->i8Vlu[i]);
    }

    CAM_LOGD( "[AFCaliData] Pos = %d", pAFCaliData->i4BestPos);

    setAFMode(AF_MODE_AFS, AF_MGR_CALLER);
    *a_pOutLen = sizeof(MINT32);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSetFocusRange( MVOID *a_pFocusRange)
{
    CAM_LOGD( "%s", __FUNCTION__);
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    m_ptrLensNVRam->rFocusRange = *pFocusRange;

    if( m_pMcuDrv)
    {
        m_pMcuDrv->setMCUInfPos( m_ptrLensNVRam->rFocusRange.i4InfPos,m_i4CurrSensorDev);
        m_pMcuDrv->setMCUMacroPos( m_ptrLensNVRam->rFocusRange.i4MacroPos,m_i4CurrSensorDev);
    }

    if( m_pIAfAlgo)
    {
        AF_CONFIG_T const *ptrHWCfgDef = NULL;
        getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
        m_pIAfAlgo->setAFParam( (*m_pAFParam), (*ptrHWCfgDef), m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);
    }

    CAM_LOGD( "[Inf Pos] = %d", m_ptrLensNVRam->rFocusRange.i4InfPos);
    CAM_LOGD( "[Marco Pos] = %d", m_ptrLensNVRam->rFocusRange.i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFocusRange( MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s", __FUNCTION__);
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    *pFocusRange = m_ptrLensNVRam->rFocusRange;
    *a_pOutLen   = sizeof(FOCUS_RANGE_T);

    CAM_LOGD( "[Inf Pos] = %d",   pFocusRange->i4InfPos);
    CAM_LOGD( "[Marco Pos] = %d", pFocusRange->i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s", __FUNCTION__);
    if(a_pAFNVRAM == NULL)
    {
        CAM_LOGE("[%s] a_pAFNVRAM is Null\n", __FUNCTION__);
        return E_AF_NULL_POINTER;
    }
    /**
     * The last parameters 1 :
     * force read NVRam data from EMMC
     */
    MINT32 err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrLensNVRam);

    if(err!=0)
    {
        CAM_LOGE("CCTOPAFGetNVRAMParam NvBufUtil get buf fail!");
        return E_AF_NULL_POINTER;
    }

    memcpy( a_pAFNVRAM, m_ptrLensNVRam, sizeof(NVRAM_LENS_PARA_STRUCT));
    *a_pOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFApplyNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode)
{
    CAM_LOGD( "%s, scenario=(%d,%d)", __FUNCTION__, u4CamScenarioMode, m_eCamScenarioMode);

    memcpy( m_ptrLensNVRam, (NVRAM_LENS_PARA_STRUCT *)a_pAFNVRAM, sizeof(NVRAM_LENS_PARA_STRUCT));

    // set nvram
    m_ptrNVRam = &(m_ptrLensNVRam->rLENSNVRAM[m_eCamScenarioMode]);
    CAM_LOGD( "%s, Apply to Phone[Scenario mode] %d", __FUNCTION__, m_eCamScenarioMode);
    CAM_LOGD( "Apply to Phone[Thres Main] %d", m_ptrNVRam->rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    CAM_LOGD( "Apply to Phone[Thres Sub] %d", m_ptrNVRam->rAFNVRAM.sAF_Coef.i4THRES_SUB);
    CAM_LOGD( "Apply to Phone[HW_TH] %d", m_ptrNVRam->rAFNVRAM.sAF_TH.i4HW_TH[0]);
    CAM_LOGD( "Apply to Phone[Statgain] %d", m_ptrNVRam->rAFNVRAM.i4StatGain);

    //MUINT32 oriCamScenarioMode = m_eCamScenarioMode;
    //m_eCamScenarioMode = (oriCamScenarioMode + 1) % AF_CAM_SCENARIO_NUM;
    //setCamScenarioMode(oriCamScenarioMode);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSaveNVRAMParam()
{
    CAM_LOGD( "%s", __FUNCTION__);
    for(MUINT32 i = 0; i < AF_CAM_SCENARIO_NUM; i++)
    {
        CAM_LOGD( "%s, WriteNVRAM to Phone[Scenario mode] %d", __FUNCTION__, i);

        CAM_LOGD( "WriteNVRAM from Phone[Thres Main]%d\n", m_ptrLensNVRam->rLENSNVRAM[i].rAFNVRAM.sAF_Coef.i4THRES_MAIN);
        CAM_LOGD( "WriteNVRAM from Phone[Thres Sub]%d\n",  m_ptrLensNVRam->rLENSNVRAM[i].rAFNVRAM.sAF_Coef.i4THRES_SUB);
        CAM_LOGD( "WriteNVRAM from Phone[HW_TH]%d\n",      m_ptrLensNVRam->rLENSNVRAM[i].rAFNVRAM.sAF_TH.i4HW_TH[0]);
        CAM_LOGD( "WriteNVRAM from Phone[Statgain]%d\n",   m_ptrLensNVRam->rLENSNVRAM[i].rAFNVRAM.i4StatGain);
    }

    MINT32 err2 = NvBufUtil::getInstance().write( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev);

    return err2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFV( MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    ACDK_AF_POS_T *pAFPos   = (ACDK_AF_POS_T *) a_pAFPosIn;
    ACDK_AF_VLU_T *pAFValue = (ACDK_AF_VLU_T *) a_pAFValueOut;

    CAM_LOGD( "%s", __FUNCTION__);

    pAFValue->i4Num = pAFPos->i4Num;
    setAFMode(AF_MODE_AFS, AF_MGR_CALLER);
    usleep( 500000); // 500ms
    setAFMode(AF_MODE_MF, AF_MGR_CALLER);

    for( MINT32 i=0; i<pAFValue->i4Num; i++)
    {
        setMFPos( pAFPos->i4Pos[i], AF_MGR_CALLER);
        usleep( 500000); // 500ms
    }

    setAFMode(AF_MODE_AFS, AF_MGR_CALLER);
    *a_pOutLen = sizeof( ACDK_AF_VLU_T);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFEnable()
{
    CAM_LOGD( "%s", __FUNCTION__);
    m_i4EnableAF = 1;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFDisable()
{
    CAM_LOGD( "%s", __FUNCTION__);
    m_i4EnableAF = 0;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetEnableInfo( MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s %d", __FUNCTION__, m_i4EnableAF);

    MINT32 *pEnableAF = (MINT32 *)a_pEnableAF;
    *pEnableAF = m_i4EnableAF;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}
