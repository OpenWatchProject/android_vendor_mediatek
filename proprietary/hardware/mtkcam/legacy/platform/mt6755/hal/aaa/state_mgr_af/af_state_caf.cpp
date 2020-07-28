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
#define LOG_TAG "af_state_caf"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <aaa_hal_raw.h>
#include <af_state.h>
#include <af_state_mgr.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <ae_mgr_if.h>
#include <flash_mgr.h>
#include <afo_buf_mgr.h>
#include <aaa_sensor_mgr.h>

/*#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <aaa_hal_raw.h>
#include <aaa_state.h>
#include <aaa_state_mgr.h>

#include <flash_awb_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <ae_mgr_if.h>
#include <flash_mgr.h>
#include <flash_feature.h>
#include <mcu_drv.h>
#include <af_mgr_if.h>
#include <afo_buf_mgr.h>
#include <awb_mgr_if.h>
#include <aao_buf_mgr.h>
#include <lsc_mgr2.h>
#include <flicker_hal_base.h>
#include <aaa_sensor_mgr.h>
#include <aaa_sensor_buf_mgr.h>
#include <aaa_common_custom.h>*/

using namespace NS3Av3;
//using namespace NSIspTuning;
//using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCAF::
StateCAF(MINT32 sensorDevId, AfStateMgr* pStateMgr)
    : IState("StateCAF", sensorDevId, pStateMgr)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCAF::
sendIntent(intent2type<eIntent_Uninit>)
{
    //ERROR HANDLING:
    //in preview state, eIntent_Uninit wont be called in normal cases,
    //its only for flow protection
    // = CameraPreviewEnd then Uninit
    MY_LOG("[StateCAF::sendIntent]<eIntent_Uninit>");

    MRESULT err = S_3A_OK;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

    // ----- CameraPreviewEnd operations go here -----

if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    // AF stop
    err = IAfMgr::getInstance().Stop(m_SensorDevId);
    if (FAILED(err)) {
        MY_ERR("IAfMgr::getInstance().Stop() fail\n");
        return err;
    }
}

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAUninit + AFStatEnable
    if (!IAFOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAFOBufMgr::getInstance().AFStatEnable(m_SensorDevId, MFALSE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMA buffer uninit
    if (!IAFOBufMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().uninit() fail");
        return E_3A_ERR;
    }
}
}

    // State transition: eState_CameraPreview --> eState_Uninit
    m_pAfStateMgr->transitState(eState_CAF, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCAF::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MRESULT err = S_3A_OK;

    MY_LOG("[StateCAF::sendIntent]<eIntent_CameraPreviewEnd>");

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    // AF stop
    err = IAfMgr::getInstance().Stop(m_SensorDevId);
    if (FAILED(err)) {
        MY_ERR("IAfMgr::getInstance().Stop() fail\n");
        return err;
    }
}

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAUninit + AFStatEnable
    if (!IAFOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAFOBufMgr::getInstance().AFStatEnable(m_SensorDevId, MFALSE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}
}
    // State transition: eState_CameraPreview --> eState_Init
    m_pAfStateMgr->transitState(eState_CAF, eState_Init);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCAF::
sendIntent(intent2type<eIntent_AFUpdate>)
{

    AaaTimer AFUpdateTimer("AFUpdate", m_SensorDevId, (m_pHal3A->m_3ALogEnable & EN_3A_SCHEDULE_LOG));


    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] enter\n");
    MRESULT err = S_3A_OK;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

    //BufInfo_T rBufInfo;
    BufInfo_T & rBufInfo = *(BufInfo_T*)(m_pAfStateMgr->mpAFOBuf);

    //MY_LOG("[StateCameraPreview::sendIntent]<eIntent_AFUpdate>");
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Dequeue AFO DMA buffer
    IAFOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] deQHwBufAFO done\n");
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.

    AE_MODE_CFG_T rPreviewInfo;
    CameraArea_T  rWinSize;
    AEMeterArea_T rAeWinSize;
    MUINT8 iYvalue;
    AE2AFInfo_T rAEInfo;
    FrameOutputParam_T AEFrameParam;
    //get AF window from AF, and set to AE meter, then get Y value.
    IAfMgr::getInstance().getAFRefWin(m_SensorDevId, rWinSize);
    rAeWinSize.i4Left  =rWinSize.i4Left;
    rAeWinSize.i4Right =rWinSize.i4Right;
    rAeWinSize.i4Top   =rWinSize.i4Top;
    rAeWinSize.i4Bottom=rWinSize.i4Bottom;
    rAeWinSize.i4Weight=rWinSize.i4Weight;
    IAeMgr::getInstance().getAEMeteringYvalue(m_SensorDevId, rAeWinSize, &iYvalue);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "AFAEInfo[L]%d[R]%d[T]%d[B]%d\n",rAeWinSize.i4Left,rAeWinSize.i4Right,rAeWinSize.i4Top,rAeWinSize.i4Bottom);

    //get current AE info, and write to AF for reference.
    IAeMgr::getInstance().getAEBlockYvalues(m_SensorDevId, rAEInfo.aeBlockV, 25);
    IAeMgr::getInstance().getPreviewParams(m_SensorDevId, rPreviewInfo);
    IAeMgr::getInstance().getRTParams(m_SensorDevId, AEFrameParam);
    rAEInfo.i4IsAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
    rAEInfo.i4ISO = rPreviewInfo.u4RealISO;
    rAEInfo.i4SceneLV = IAeMgr::getInstance().getLVvalue(m_SensorDevId,MTRUE);
    rAEInfo.iYvalue = (MINT64)iYvalue;
    rAEInfo.ishutterValue = AEFrameParam.u4PreviewShutterSpeed_us;

    MUINT16 u2YCnt;
    IAeMgr::getInstance().getAEMeteringBlockAreaValue(m_SensorDevId,rAeWinSize, m_pAfStateMgr->m_uAEBlockAreaY, &u2YCnt);
    rAEInfo.i4AEBlockAreaYCnt = u2YCnt;
    rAEInfo.pAEBlockAreaYvalue = m_pAfStateMgr->m_uAEBlockAreaY;
    rAEInfo.i4IsFlashFrm = FlashMgr::getInstance().isAFLampOn(m_SensorDevId);
    rAEInfo.i4IsAELocked = 0;//TODO IAeMgr::getInstance().IsAELock(m_SensorDevId);

    IAfMgr::getInstance().setAE2AFInfo(m_SensorDevId, rAEInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "AFAEInfo[AEStable]%d[ISO]%d[LV]%d[Y]%d\n",rAEInfo.i4IsAEStable,rAEInfo.i4ISO,rAEInfo.i4SceneLV,(MINT32)rAEInfo.iYvalue);

    AaaTimer doAFTimer("doAF", m_SensorDevId, (m_pHal3A->m_3ALogEnable & EN_3A_SCHEDULE_LOG));
    IAfMgr::getInstance().doAF(m_SensorDevId, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    doAFTimer.End();

    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);    // Profiling Start.
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] doAF done\n");
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Enqueue AFO DMA buffer
    IAFOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] enQHwBuf done\n");
}
}

    AFUpdateTimer.End();
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCAF::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateCAF::sendIntent]<eIntent_AFStart>");
    IAfMgr::getInstance().autoFocus(m_SensorDevId);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCAF::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateCAF::sendIntent]<eIntent_AFEnd>");
    IAfMgr::getInstance().cancelAutoFocus(m_SensorDevId);
    return  S_3A_OK;
}
