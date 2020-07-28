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
#define LOG_TAG "af_mgr_v3"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <cutils/atomic.h>
#include <mtkcam/utils/std/Log.h>
#include <cct_feature.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <nvbuf_util.h>
#include <af_tuning_custom.h>
#include <dbg_aaa_param.h>
#include "af_mgr.h"
#include <private/aaa_utils.h>

using namespace NS3Av3;
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
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    MUINT32 i4HalSensorDev = mapSensorIdxToDev(m_i4SensorIdx);

    pIHalSensorList->querySensorStaticInfo(i4HalSensorDev, &rSensorStaticInfo);

    m_i4CurrSensorId = rSensorStaticInfo.sensorDevID;
    if (m_pMcuDrv == NULL)
    {
        m_pMcuDrv = MCUDrv::getInstance(m_i4CurrSensorDev);
    }
    MUINT32 ModuleID = 0;
    m_pMcuDrv->lensSearch(m_i4CurrSensorId, ModuleID);
    m_i4CurrLensId = m_pMcuDrv->getCurrLensID();
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
    CAM_LOGD_IF( LEVEL_FLOW, "%s +", __FUNCTION__);
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
    m_i4IsCctOper = 1;
    CAM_LOGD( "%s EnableAF(%d) + ", __FUNCTION__, m_i4EnableAF);

    if (m_i4EnableAF <= 0)
        return S_AF_OK;

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    m_i4CctAfMode = MTK_CONTROL_AF_MODE_AUTO;
    m_i4CctTriggerAf = 1;

    MINT32 i4TimeOutCnt = 0;
    while( !isFocusFinish() || m_eAFState==E_AF_INACTIVE )
    {
        usleep(30000); // 30ms
        i4TimeOutCnt++;
        if( i4TimeOutCnt>3000) // 3000*30ms = 90000ms = 90s
        {
            CAM_LOGD( "%s : TimeOutCnt %d", __FUNCTION__, i4TimeOutCnt);
            break;
        }
    }
    CAM_LOGD( "%s - ", __FUNCTION__);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPMFOpeartion( MINT32 a_i4MFpos)
{
    m_i4IsCctOper = 1;
    CAM_LOGD( "%s EnableAF(%d) a_i4MFpos(%d)", __FUNCTION__, m_i4EnableAF, a_i4MFpos);

    setAFMode( MTK_CONTROL_AF_MODE_OFF, AF_MGR_CALLER);
    m_i4CctAfMode = MTK_CONTROL_AF_MODE_OFF;
    m_i4CctTriggerAf = 1;
    if(a_i4MFpos<0 || a_i4MFpos>=1024)
    {
        a_i4MFpos = 1023;
        CAM_LOGW("%s MFPos from CCTOPMF is out of range", __FUNCTION__);
    }
    m_i4CctMfPos = a_i4MFpos;

    usleep(200000); // wait 200ms before check IsAFSearch
    MINT32 i4TimeOutCnt = 0;
    while(m_i4IsAFSearch_CurState!=AF_SEARCH_DONE)  // while( !isFocusFinish())
    {
        usleep(30000); // 30ms
        i4TimeOutCnt++;
        CAM_LOGD( "%s : IsAFSearch %d", __FUNCTION__, m_i4IsAFSearch_CurState);
        if( i4TimeOutCnt>100) // 100*30ms = 3000ms = 3s
        {
            CAM_LOGD( "%s : TimeOutCnt %d", __FUNCTION__, i4TimeOutCnt);
            break;
        }
    }
    CAM_LOGD( "%s -", __FUNCTION__);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPCheckAutoFocusDone()
{
    return (m_sAFOutput->i4IsAfSearch == AF_SEARCH_DONE) ? 1 : 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPWaitAutoFocusDone()
{
    MINT32 i4TimeOutCnt = 0;

    while (!isFocusFinish())
    {
        usleep(30000); // 30ms
        i4TimeOutCnt++;
        if (i4TimeOutCnt > 60)
        {
            CAM_LOGD("%s : TimeOut", __FUNCTION__);
            break;
        }
    }

    return (m_sAFOutput->i4IsAfSearch == AF_SEARCH_DONE) ? 1 : 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetAFInfo( MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    ACDK_AF_INFO_T *pAFInfo = (ACDK_AF_INFO_T *)a_pAFInfo;

    CAM_LOGD_IF( LEVEL_FLOW, "%s", __FUNCTION__);

    pAFInfo->i4AFMode  = m_eLIB3A_AFMode;
    pAFInfo->i4AFMeter = LIB3A_AF_METER_SPOT;
    pAFInfo->i4CurrPos = m_sAFOutput->i4AFPos;

    *a_pOutLen = sizeof( ACDK_AF_INFO_T);

    CAM_LOGD( "[AF Mode] = %d", pAFInfo->i4AFMode);
    CAM_LOGD( "[AF Meter] = %d", pAFInfo->i4AFMeter);
    CAM_LOGD( "[AF Current Pos] = %d", pAFInfo->i4CurrPos);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetBestPos( MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s %d", __FUNCTION__, m_sAFOutput->i4AFBestPos);
    *a_pAFBestPos = m_sAFOutput->i4AFBestPos;
    *a_pOutLen    = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFCaliOperation( MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    ACDK_AF_CALI_DATA_T *pAFCaliData = (ACDK_AF_CALI_DATA_T *)a_pAFCaliData;
    AF_DEBUG_INFO_T rAFDebugInfo;
    MINT32 i4TimeOutCnt = 0;

    CAM_LOGD( "%s", __FUNCTION__);

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    usleep( 500000);    // 500ms
    m_eLIB3A_AFMode = LIB3A_AF_MODE_CALIBRATION;
    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    afc_setAFMode(c_sensorDev, m_eLIB3A_AFMode);

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

    pAFCaliData->i4BestPos = m_sAFOutput->i4AFBestPos;

    CAM_LOGD( "[AFCaliData] Num = %d", pAFCaliData->i4Num);
    CAM_LOGD( "[AFCaliData] Gap = %d", pAFCaliData->i4Gap);

    for( MINT32 i=0; i<pAFCaliData->i4Num; i++)
    {
        CAM_LOGD( "[AFCaliData] Vlu %d = %lld", i, (long long)pAFCaliData->i8Vlu[i]);
    }

    CAM_LOGD( "[AFCaliData] Pos = %d", pAFCaliData->i4BestPos);

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    *a_pOutLen = sizeof(MINT32);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSetFocusRange( MVOID *a_pFocusRange)
{
    CAM_LOGD( "%s", __FUNCTION__);
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    if (m_ptrLensNVRam)
    {
        m_ptrLensNVRam->rFocusRange = *pFocusRange;
    }
    else
    {
        return S_AF_OK;
    }

    if( m_pMcuDrv)
    {
        m_pMcuDrv->setMCUInfPos( m_ptrLensNVRam->rFocusRange.i4InfPos);
        m_pMcuDrv->setMCUMacroPos( m_ptrLensNVRam->rFocusRange.i4MacroPos);
    }

// TODO : replace with cxuAF
#if 0
    if( m_pIAfAlgo)
    {
        AF_CONFIG_T const *ptrHWCfgDef = NULL;
        getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
        // TODO: replace with initAF
        m_pIAfAlgo->setAFParam( (*m_pAFParam), (*ptrHWCfgDef), m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);
    }
#endif
    CAM_LOGD( "[Inf Pos] = %d", m_ptrLensNVRam->rFocusRange.i4InfPos);
    CAM_LOGD( "[Marco Pos] = %d", m_ptrLensNVRam->rFocusRange.i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFocusRange( MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s", __FUNCTION__);
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    if (m_ptrLensNVRam)
    {
        *pFocusRange = m_ptrLensNVRam->rFocusRange;
    }
    else
    {
        pFocusRange->i4InfPos = 0;
        pFocusRange->i4MacroPos = 1023;
    }

    *a_pOutLen   = sizeof(FOCUS_RANGE_T);

    CAM_LOGD( "[Inf Pos] = %d",   pFocusRange->i4InfPos);
    CAM_LOGD( "[Marco Pos] = %d", pFocusRange->i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    CAM_LOGD( "%s", __FUNCTION__);

    /**
     * The last parameters 1 :
     * force read NVRam data from EMMC
     */
    if (!m_ptrLensNVRam)
    {
        return S_AF_OK;
    }

    MINT32 err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_ptrLensNVRam, 1);

    if( err!=0)
    {
        CAM_LOGE( "CCTOPAFGetNVRAMParam NvBufUtil get buf fail!");
    }

    memcpy( a_pAFNVRAM, m_ptrLensNVRam, sizeof(NVRAM_LENS_PARA_STRUCT));
    *a_pOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFApplyNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode)
{
    CAM_LOGD( "%s", __FUNCTION__);

    if (!m_ptrLensNVRam)
    {
        return S_AF_OK;
    }
    // set nvram
    if (u4CamScenarioMode < AF_CAM_SCENARIO_NUM_2)
    {
        memcpy( &m_ptrLensNVRam->AF[u4CamScenarioMode], (NVRAM_LENS_DATA_PARA_STRUCT *)a_pAFNVRAM, sizeof(NVRAM_LENS_DATA_PARA_STRUCT));
        CAM_LOGD( "%s, Apply to Phone[Scenario mode] %d", __FUNCTION__, u4CamScenarioMode);
        CAM_LOGD( "Apply to Phone[Thres Main] %d", m_ptrLensNVRam->AF[u4CamScenarioMode].rAFNVRAM.sAF_Coef.i4THRES_MAIN);
        CAM_LOGD( "Apply to Phone[Thres Sub] %d",  m_ptrLensNVRam->AF[u4CamScenarioMode].rAFNVRAM.sAF_Coef.i4THRES_SUB);
        CAM_LOGD( "Apply to Phone[HW_TH] %d",      m_ptrLensNVRam->AF[u4CamScenarioMode].rAFNVRAM.sAF_TH.i4HW_TH_PL_H[0]);
        CAM_LOGD( "Apply to Phone[Statgain] %d",   m_ptrLensNVRam->AF[u4CamScenarioMode].rAFNVRAM.i4Common[1]);
    }

    // apply m_sNVRam only when m_sNVRam is the same set with the new one(a_pAFNVRAM) ==> AFNVRAMMapping[m_eCamScenarioMode] == u4CamScenarioMode
    MUINT32 u4AFNVRAMIdx = AFNVRAMMapping[m_eCamScenarioMode];
    if (u4AFNVRAMIdx == u4CamScenarioMode)
    {
        m_ptrNVRam = (NVRAM_LENS_DATA_PARA_STRUCT*)a_pAFNVRAM;
#if 0// TODO : replace with cxuAF
        if( m_pIAfAlgo)
        {
            AF_CONFIG_T const *ptrHWCfgDef = NULL;
            getAFConfig( m_i4CurrSensorDev, &ptrHWCfgDef);
            // TODO: replace with initAF
            m_pIAfAlgo->setAFParam( (*m_pAFParam), (*ptrHWCfgDef), m_ptrNVRam->rAFNVRAM, m_ptrNVRam->rDualCamNVRAM);
        }
#endif
        if( m_pMcuDrv)
        {
            m_pMcuDrv->setMCUInfPos( m_ptrLensNVRam->rFocusRange.i4InfPos);
            m_pMcuDrv->setMCUMacroPos( m_ptrLensNVRam->rFocusRange.i4MacroPos);
        }
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSaveNVRAMParam()
{
    CAM_LOGD( "%s", __FUNCTION__);
    if (!m_ptrLensNVRam)
    {
        return S_AF_OK;
    }
    for(MUINT32 i = 0; i < AF_CAM_SCENARIO_NUM_2; i++)
    {
        CAM_LOGD( "%s, WriteNVRAM to Phone[Scenario mode] %d", __FUNCTION__, i);

        CAM_LOGD( "WriteNVRAM from Phone[Thres Main]%d\n", m_ptrLensNVRam->AF[i].rAFNVRAM.sAF_Coef.i4THRES_MAIN);
        CAM_LOGD( "WriteNVRAM from Phone[Thres Sub]%d\n",  m_ptrLensNVRam->AF[i].rAFNVRAM.sAF_Coef.i4THRES_SUB);
        CAM_LOGD( "WriteNVRAM from Phone[HW_TH]%d\n",      m_ptrLensNVRam->AF[i].rAFNVRAM.sAF_TH.i4HW_TH_PL_H[0]);
        CAM_LOGD( "WriteNVRAM from Phone[Statgain]%d\n",   m_ptrLensNVRam->AF[i].rAFNVRAM.i4Common[1]);
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
    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
    usleep( 500000); // 500ms
    setAFMode( MTK_CONTROL_AF_MODE_OFF, AF_MGR_CALLER);

    for( MINT32 i=0; i<pAFValue->i4Num; i++)
    {
        setMFPos( pAFPos->i4Pos[i], AF_MGR_CALLER);
        usleep( 500000); // 500ms
        pAFValue->i8Vlu[i] = TransStatProfileToAlgo( m_sAFInput->sStatProfile);
        CAM_LOGD( "[FV]pos = %d, value = %lld\n", pAFPos->i4Pos[i], (long long)pAFValue->i8Vlu[i]);
    }

    setAFMode( MTK_CONTROL_AF_MODE_AUTO, AF_MGR_CALLER);
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
    // CAM_LOGD( "%s %d", __FUNCTION__, m_i4EnableAF);

    MINT32 *pEnableAF = (MINT32 *)a_pEnableAF;
    *pEnableAF = m_i4EnableAF;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTOPAFSetAfArea(MUINT32 a_iPercent)
{
    m_i4CctTriggerAf = 1;
    CAM_LOGD( "%s %d", __FUNCTION__, a_iPercent);

    MINT32 AfAreaW = (MINT32)(m_i4TGSzW*a_iPercent/100);
    MINT32 AfAreaH = (MINT32)(m_i4TGSzH*a_iPercent/100);
    MINT32 AfAreaX = (MINT32)((m_i4TGSzW - AfAreaW)/2);
    MINT32 AfAreaY = (MINT32)((m_i4TGSzH - AfAreaH)/2);


    m_i4CctAfArea.u4Count = 1;
    m_i4CctAfArea.rAreas[0].i4Left = AfAreaX;
    m_i4CctAfArea.rAreas[0].i4Top = AfAreaY;
    m_i4CctAfArea.rAreas[0].i4Right = AfAreaX+AfAreaW;
    m_i4CctAfArea.rAreas[0].i4Bottom = AfAreaY+AfAreaH;
    m_i4CctAfArea.rAreas[0].i4Weight = 1;

    setAFArea(m_i4CctAfArea);

    CAM_LOGD( "TG size(%d,%d), Af Area(l,t,r,b)=(%d,%d,%d,%d) (w,h)=(%d,%d)\n",
              m_i4TGSzW,m_i4TGSzH,
              m_i4CctAfArea.rAreas[0].i4Left,
              m_i4CctAfArea.rAreas[0].i4Top,
              m_i4CctAfArea.rAreas[0].i4Right,
              m_i4CctAfArea.rAreas[0].i4Bottom,
              AfAreaW,AfAreaH);
    return S_AF_OK;
}

// CCT use only
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::TransStatProfileToAlgo( AF_STAT_PROFILE_T &sInSataProfile)
{
    /*[TODO] For preview IT only.*/
    MINT64 i8SataH0=0;

    //analyze center 6x6 blocks
    MUINT32 ablkW = 6;
    MUINT32 ablkH = 6;
    //total blocks
    MUINT32 tblkW = sInSataProfile.u4NumBlkX;
    MUINT32 tblkH = sInSataProfile.u4NumBlkY;
    //start address of block index.
    MUINT32 startX = (tblkW-ablkW)/2;
    MUINT32 startY = (tblkH-ablkH)/2;

    if(m_pAfoBuf)
    {
        for( MUINT32 j=0; j<ablkH; j++)
        {
            for( MUINT32 i=0; i<ablkW; i++)
            {
                MUINT32 idx = (startY+j)*tblkW + (startX+i);
                i8SataH0 += m_pAfoBuf[idx].u4FILH0;
            }
        }
    }

    return i8SataH0;
}
