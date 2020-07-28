/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxLog.h"
#include "RpIrLwgStatusListener.h"
#include "RpIrStrategyCommon.h"
#include "RpIrController.h"

#define RFX_LOG_TAG "[IRC][RpIrLwgStatusListener]"
/*****************************************************************************
 * Class RpIrLwgStatusListener
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpIrLwgStatusListener", RpIrLwgStatusListener, RfxController);

RpIrLwgStatusListener::RpIrLwgStatusListener() :
        m_RpIrStrategyCommon(NULL),
        m_RpIrController(NULL) {
}

RpIrLwgStatusListener::RpIrLwgStatusListener(RpIrStrategyCommon *rpIrStrategyCommon, RpIrController *rpIrController) :
        m_RpIrStrategyCommon(rpIrStrategyCommon),
        m_RpIrController(rpIrController) {
}

void RpIrLwgStatusListener::onRadioStateChanged(bool isRadioOn) {
    if ((m_RpIrStrategyCommon->getIsLwgRadioOn()) != isRadioOn) {
        logD(RFX_LOG_TAG, "[LWG]onRadioStateChanged : %s", Bool2Str(isRadioOn));
        m_RpIrStrategyCommon->setIsLwgRadioOn(isRadioOn);
        m_RpIrStrategyCommon->onRadioStateChanged();
    }
}

bool RpIrLwgStatusListener::onPreviewPlmn(String8 plmn) {
    logD(RFX_LOG_TAG, "[LWG]onPreviewPlmn : %s", plmn.string());
    NwsMode oldMode = m_RpIrController->getNwsMode();

    bool ret = m_RpIrStrategyCommon->onPreviewLwgPlmn(plmn);

    if (oldMode != m_RpIrController->getNwsMode()) {
        m_RpIrStrategyCommon->restartNoSerivceWatchdogIfNeeded();
    }
    return ret;
}

void RpIrLwgStatusListener::onPlmnChanged(String8 &plmn) {
    logD(RFX_LOG_TAG, "[LWG]onPlmnChanged : %s", plmn.string());
    NwsMode oldMode = m_RpIrController->getNwsMode();

    m_RpIrStrategyCommon->onLwgPlmnChanged(plmn);

    if (oldMode != m_RpIrController->getNwsMode()) {
        m_RpIrStrategyCommon->restartNoSerivceWatchdogIfNeeded();
    } else {
        m_RpIrStrategyCommon->postponeNoServiceWatchdogIfNeeded();
    }
}

bool RpIrLwgStatusListener::needWaitImsi() {
    return m_RpIrController->needWaitImsi();
}

void RpIrLwgStatusListener::onServiceStateChanged(ServiceType serviceType, bool forceSwitch) {
    logD(RFX_LOG_TAG, "[LWG]onServiceStateChanged(%s, %s)", ServiceType2Str(serviceType),
            Bool2Str(forceSwitch));
    if (serviceType != m_RpIrStrategyCommon->getLwgServiceState()) {
        m_RpIrStrategyCommon->setLwgServiceState(serviceType);
        if (serviceType == OUT_OF_SERVICE
                && m_RpIrStrategyCommon->getCdmaServiceState() == OUT_OF_SERVICE
                && getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER)) {
            if (!forceSwitch || !m_RpIrStrategyCommon->switchForNoService(true)) {
                rfxPostInvoke0(this, &RpIrLwgStatusListener::startLwgNewSearchRound);
            }
        }
        m_RpIrStrategyCommon->updateWatchdog();
    }
}

void RpIrLwgStatusListener::onNwsModeSwitchDone() {
    m_RpIrStrategyCommon->getLwgController()->startNewSearchRound();
    m_RpIrStrategyCommon->getCdmaController()->startNewSearchRound();
}

void RpIrLwgStatusListener::startLwgNewSearchRound() {
    m_RpIrStrategyCommon->getLwgController()->startNewSearchRound();
}
