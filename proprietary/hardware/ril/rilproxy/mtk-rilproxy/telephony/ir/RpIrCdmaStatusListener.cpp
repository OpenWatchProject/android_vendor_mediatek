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
#include "RpIrCdmaStatusListener.h"
#include "RpIrInvalidMccConvert.h"
#include "RpIrStrategyCommon.h"

#define RFX_LOG_TAG "[IRC][RpIrCdmaStatusListener]"
/*****************************************************************************
 * Class RpIrCdmaStatusListener
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpIrCdmaStatusListener", RpIrCdmaStatusListener, RfxController);

RpIrCdmaStatusListener::RpIrCdmaStatusListener():
        m_RpIrStrategyCommon(NULL),
        m_RpIrController(NULL) {
}

RpIrCdmaStatusListener::RpIrCdmaStatusListener(RpIrStrategyCommon *rpIrStrategyCommon, RpIrController *rpIrController) :
        m_RpIrStrategyCommon(rpIrStrategyCommon),
        m_RpIrController(rpIrController) {
}

void RpIrCdmaStatusListener::onRadioStateChanged(bool isRadioOn) {
    if (m_RpIrStrategyCommon->getIsCdmaRadioOn() != isRadioOn) {
        logD(RFX_LOG_TAG, "[CDMA]onRadioStateChanged : %s", Bool2Str(isRadioOn));
        m_RpIrStrategyCommon->setIsCdmaRadioOn(isRadioOn);
        m_RpIrStrategyCommon->onRadioStateChanged();
    }
}

void RpIrCdmaStatusListener::onPlmnChanged(String8 &plmn) {
    logD(RFX_LOG_TAG, "[CDMA]onPlmnChanged : %s", plmn.string());
    NwsMode oldMode = m_RpIrController->getNwsMode();

    plmn = convertInvalidMccBySidNid(plmn);

    m_RpIrStrategyCommon->onCdmaPlmnChanged(plmn);

    if (oldMode != m_RpIrController->getNwsMode()) {
        m_RpIrStrategyCommon->restartNoSerivceWatchdogIfNeeded();
    } else {
        m_RpIrStrategyCommon->postponeNoServiceWatchdogIfNeeded();
    }
}

bool RpIrCdmaStatusListener::needWaitImsi() {
    return m_RpIrController->needWaitImsi();
}

String8 RpIrCdmaStatusListener::convertInvalidMccBySidNid(const String8 &plmn) {
    logD(RFX_LOG_TAG, "[CDMA] convertInvalidMccBySidNid, plmn=%s", plmn.string());
    String8 convertCdmaPlmn = plmn;
    String8 first4Str = String8(string(convertCdmaPlmn.string()).substr(0, 4).c_str());
    logD(RFX_LOG_TAG, "[CDMA] convertInvalidMccBySidNid, mcc=%s", first4Str.string());
    if ((first4Str == "2134" && plmn.size() == 7) || first4Str == "0000") {
        // Re-get plmn for special operator which doesn't release plmn when
        // network searched.
        convertCdmaPlmn = RpIrInvalidMccConvert::checkMccBySidLtmOff(plmn);
        logD(RFX_LOG_TAG, "[CDMA] convertInvalidMccBySidNid, convertCdmaPlmn = %s", convertCdmaPlmn.string());
    }
    return convertCdmaPlmn;
}

void RpIrCdmaStatusListener::onServiceStateChanged(ServiceType serviceType, bool forceSwitch) {
    RFX_UNUSED(forceSwitch);
    logD(RFX_LOG_TAG, "[CDMA]onServiceStateChanged(%s)", ServiceType2Str(serviceType));
    if (serviceType != m_RpIrStrategyCommon->getCdmaServiceState()) {
        m_RpIrStrategyCommon->setCdmaServiceState(serviceType);
        if (serviceType == OUT_OF_SERVICE
                && m_RpIrStrategyCommon->getLwgServiceState() == OUT_OF_SERVICE
                && getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER)) {
            m_RpIrStrategyCommon->switchForNoService(true);
        }
        m_RpIrStrategyCommon->updateWatchdog();
    }
}

void RpIrCdmaStatusListener::onNwsModeSwitchDone() {
    m_RpIrStrategyCommon->getLwgController()->startNewSearchRound();
    m_RpIrStrategyCommon->getCdmaController()->startNewSearchRound();
}

