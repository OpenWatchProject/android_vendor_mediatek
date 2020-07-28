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


#include "af_state_mgr.h"
#include <aao_buf_mgr.h>
#include <afo_buf_mgr.h>
#include <aaa_scheduling_custom.h>

#ifndef LOG_TAG
#define LOG_TAG "af_state_mgr"
#endif

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

 using namespace NS3A;

AfStateMgr::AfStateMgr()
{
    mpCurrentState = NULL; //mpIState[eState_Uninit];

    mpAFOBuf = new BufInfo_T;
}

AfStateMgr::AfStateMgr(MINT32 sensorDevId)
            : m_Lock()
            , mSensorDevId(sensorDevId)
{
    /*
    #define STATE_INITIALIZE(_state_)\
        mpIState[eState_##_state_] = new State##_state_(sensorDevId, this);

    STATE_INITIALIZE(Init);
    STATE_INITIALIZE(Uninit);
    STATE_INITIALIZE(CameraPreview);
    STATE_INITIALIZE(CamcorderPreview);
    STATE_INITIALIZE(Recording);
    STATE_INITIALIZE(Precapture);
    STATE_INITIALIZE(Capture);
    STATE_INITIALIZE(AF);*/
    mpIState[eState_Uninit] = new StateUninitAF(sensorDevId, this);
    mpIState[eState_Init] = new StateInitAF(sensorDevId, this);
    mpIState[eState_CAF] = new StateCAF(sensorDevId, this);
    mpIState[eState_TAF] = new StateTAF(sensorDevId, this);

    mpCurrentState = mpIState[eState_Uninit];

    mpAFOBuf = (MVOID*) new BufInfo_T;
}

AfStateMgr::~AfStateMgr()
{
    /*
    #define STATE_UNINITIALIZE(_state_)\
        delete mpIState[eState_##_state_];\
    mpIState[eState_##_state_] = NULL;

    STATE_UNINITIALIZE(Init);
    STATE_UNINITIALIZE(Uninit);
    STATE_UNINITIALIZE(CameraPreview);
    STATE_UNINITIALIZE(CamcorderPreview);
    STATE_UNINITIALIZE(Recording);
    STATE_UNINITIALIZE(Precapture);
    STATE_UNINITIALIZE(Capture);
    STATE_UNINITIALIZE(AF);*/

    delete mpIState[eState_Uninit];
    mpIState[eState_Uninit] = NULL;

    delete mpIState[eState_Init];
    mpIState[eState_Init] = NULL;

    delete mpIState[eState_CAF];
    mpIState[eState_CAF] = NULL;

    delete mpIState[eState_TAF];
    mpIState[eState_TAF] = NULL;
    mpCurrentState = NULL;

    delete (BufInfo_T*)mpAFOBuf;
}

MRESULT AfStateMgr::transitState(EState_T const eCurrState, EState_T const eNewState)
{
    if (eCurrState != mStateStatus.eCurrState)
    {
        MY_ERR("[AfStateMgr::transitState] eCurrState != mStateStatus.eCurrState");
        return E_3A_INCORRECT_STATE;
    }

    MY_LOG("[StateCommon::transitState] %s --> %s", mpIState[mStateStatus.eCurrState]->getName(), mpIState[eNewState]->getName());
    mpCurrentState = mpIState[eNewState];
    mStateStatus.eCurrState = eNewState;
    mStateStatus.ePrevState = eCurrState;
    return S_3A_OK;
}

MRESULT AfStateMgr::sendCmd(ECmd_T eCmd)
{
    Mutex::Autolock lock(m_Lock);

    EIntent_T eNewIntent = static_cast<EIntent_T>(eCmd);

    #define SEND_INTENT(_intent_)\
        case _intent_: return mpCurrentState->sendIntent(intent2type<_intent_>());\

    switch (eNewIntent)
    {
        SEND_INTENT(eIntent_CameraPreviewStart)
        SEND_INTENT(eIntent_CameraPreviewEnd)
        SEND_INTENT(eIntent_CamcorderPreviewStart)
        SEND_INTENT(eIntent_CamcorderPreviewEnd)
        SEND_INTENT(eIntent_PrecaptureStart)
        SEND_INTENT(eIntent_PrecaptureEnd)
        SEND_INTENT(eIntent_CaptureStart)
        SEND_INTENT(eIntent_CaptureEnd)
        SEND_INTENT(eIntent_RecordingStart)
        SEND_INTENT(eIntent_RecordingEnd)
        SEND_INTENT(eIntent_VsyncUpdate)
        SEND_INTENT(eIntent_AFUpdate)
        SEND_INTENT(eIntent_AFStart)
        SEND_INTENT(eIntent_AFEnd)
        SEND_INTENT(eIntent_Init)
        SEND_INTENT(eIntent_Uninit)
    }
    return  -1;
}



