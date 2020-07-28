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
#define LOG_TAG "af_state"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <aaa_hal.h>
#include "af_state.h"
#include "af_state_mgr.h"
//#include <ae_mgr_if.h>
#include <af_mgr_if.h>
//#include <flash_mgr.h>
#include <afo_buf_mgr.h>
//#include <awb_mgr_if.h>
//#include <aao_buf_mgr.h>
//#include <mtkcam/featureio/flicker_hal_base.h>
#include <ae_param.h>
#include <aaa_sensor_mgr.h>
//#include "aaa_sensor_buf_mgr.h"
//#include <isp_tuning_mgr.h>




using namespace NS3A;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateUninitAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateUninitAF::
StateUninitAF(MINT32 sensorDevId, AfStateMgr* pStateMgr)
    : IState("StateUninitAF", sensorDevId, pStateMgr)
{
    MY_LOG("[%s] sensorDevId(%d)", __FUNCTION__, sensorDevId);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Init
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateUninitAF::
sendIntent(intent2type<eIntent_Init>)
{
    MY_LOG("[StateUninitAF::sendIntent]<eIntent_Init>");
    MINT32 i4SensorIdx = m_pHal3A->getSensorOpenIdx();

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMA buffer init
    if (!IAFOBufMgr::getInstance().init(m_SensorDevId, i4SensorIdx)) {
        MY_ERR("IAFOBufMgr::getInstance().init() fail");
        return E_3A_ERR;
    }
}

    // State transition: eState_Uninit --> eState_Init
    m_pAfStateMgr->transitState(eState_Uninit, eState_Init);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateInitAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateInitAF::
StateInitAF(MINT32 sensorDevId, AfStateMgr* pStateMgr)
    : IState("StateInitAF", sensorDevId, pStateMgr)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateInitAF::
sendIntent(intent2type<eIntent_Uninit>)
{
    MY_LOG("[StateInitAF::sendIntent]<eIntent_Uninit>");

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMA buffer uninit
    if (!IAFOBufMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().uninit() fail");
        return E_3A_ERR;
    }
}
    // State transition: eState_Init --> eState_Uninit
    m_pAfStateMgr->transitState(eState_Init, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateInitAF::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
    MY_LOG("[StateInit::sendIntent]<eIntent_CameraPreviewStart>");

    MRESULT err = S_3A_OK;
    MBOOL bRet = MTRUE;
    MINT32 i4SensorIdx = m_pHal3A->getSensorOpenIdx();

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAInit + AFStatEnable
    if (!IAFOBufMgr::getInstance().DMAInit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().DMAInit() fail");
        return E_3A_ERR;
    }
    if (!IAFOBufMgr::getInstance().AFStatEnable(m_SensorDevId, MTRUE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}

if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {

    if(m_pHal3A->mbAFUninit)
    {
        err = IAfMgr::getInstance().uninit(m_SensorDevId,0);
        if (FAILED(err)) {
            MY_ERR("IAfMgr::getInstance().uninit() fail\n");
            return err;
        }
    }

    // AF init
    if(m_pHal3A->m_bKeepLastStatus){
        err = IAfMgr::getInstance().init(m_SensorDevId, i4SensorIdx, 0);
        m_pHal3A->m_bKeepLastStatus = MFALSE;
        MY_LOG("m_bKeepLastStatus = %d\n", m_pHal3A->m_bKeepLastStatus);
    }
    else{
        if(m_pHal3A->mbAFUninit == MTRUE){
            MY_LOG("[%s] Don't move lens for capture flow",__FUNCTION__);
            err = IAfMgr::getInstance().init(m_SensorDevId, i4SensorIdx, 0);
        }
        else{
            MY_LOG("[%s] Move lens",__FUNCTION__);
            err = IAfMgr::getInstance().init(m_SensorDevId, i4SensorIdx);
        }
    }
    if (FAILED(err)) {
        MY_ERR("AfMgr::getInstance().init() fail\n");
        return err;
    }
    MY_LOG("[%s] [StateInit::sendIntent]<eIntent_CameraPreviewStart> mbAFUninit(%d)->reset 0",__FUNCTION__,m_pHal3A->mbAFUninit);
    m_pHal3A->mbAFUninit = MFALSE;
}
}
    // State transition: eState_Init --> eState_CAF
    m_pAfStateMgr->transitState(eState_Init, eState_CAF);

    return  S_3A_OK;
}

/*
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateInit::
sendIntent(intent2type<eIntent_CamcorderPreviewStart>)
{
    MRESULT err = S_3A_OK;

    MY_LOG("[StateInit::sendIntent]<eIntent_CamcorderPreviewStart>");

    MINT32 i4SensorIdx = m_pHal3A->getSensorOpenIdx();

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAInit + AFStatEnable
    if (!IAFOBufMgr::getInstance().DMAInit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().DMAInit() fail");
        return E_3A_ERR;
    }
    if (!IAFOBufMgr::getInstance().AFStatEnable(m_SensorDevId, MTRUE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}

if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    // AF init
    err = IAfMgr::getInstance().init(m_SensorDevId, i4SensorIdx);
    if (FAILED(err)) {
        MY_ERR("AfMgr::getInstance().init() fail\n");
        return err;
    }
}

}

    // Reset frame count to -2
    m_pAfStateMgr->resetFrameCount();

    // State transition: eState_Init --> eState_CamcorderPreview
    m_pAfStateMgr->transitState(eState_Init, eState_CamcorderPreview);


    MY_LOG("[StateInit::sendIntent]<eIntent_CamcorderPreviewStart>--");

    return  S_3A_OK;
}
*/
