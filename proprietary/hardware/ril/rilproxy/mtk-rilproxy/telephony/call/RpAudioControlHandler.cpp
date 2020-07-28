/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include "assert.h"

#include <cutils/properties.h>
#include <cutils/jstring.h>
#include <media/AudioSystem.h>
#include <pthread.h>
#include "RfxMainThread.h"
#include "RpAudioControlHandler.h"
#include "RpCallController.h"
#include "util/RpFeatureOptionUtils.h"
#include "MTKPrimaryDevicesHalClientInterface.h"

typedef void* (*PthreadPtr)(void*);

using namespace android;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpAudioControlHandler", RpAudioControlHandler, RfxController);

String8 RpAudioControlHandler::modemGsm = String8("MD1");
String8 RpAudioControlHandler::modemC2k = String8("MD3");
String8 RpAudioControlHandler::audioPhone1Md = String8("Phone1Modem=");
String8 RpAudioControlHandler::audioPhone2Md = String8("Phone2Modem=");
String8 RpAudioControlHandler::refreshMd = String8("RefreshModem=");

RpAudioControlHandler::RpAudioControlHandler() {
}

RpAudioControlHandler::~RpAudioControlHandler() {
}

void RpAudioControlHandler::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    RFX_LOG_D(RP_AUDIOCONTROL_TAG, "onInit !!");

    mAudioSwitchMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&mAudioSwitchMutex, NULL);

    // For Non-SVLTE project, always set default modem as MD1,
    // IMS call cannot tolerate audio path switch delay (SIP invite msg takes less than 300ms)
    updateAudioModem(RADIO_TECH_GROUP_GSM);
}

void RpAudioControlHandler::setRefreshModem(bool isSvlte,
        const String8& activeModem, audioModeParameterStruct* parameter) {
    int isModeReady = 0;

    /**
     * check audio mode (always AUDIO_MODE_IN_CALL==2)
     */
    String8 currentAudioModeStr =
            android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                    String8("getMode="));

    if (currentAudioModeStr == String8("getMode=2")) {
        RFX_LOG_D(RP_AUDIOCONTROL_TAG, "%s[%d] activeModem(%s)",
                (isSvlte ? "svlte setRefreshModem" : "setRefreshModem"),
                parameter->slotId, activeModem.string());
        isModeReady = 1;
    }

    if (!isModeReady) {
        RFX_LOG_E(RP_AUDIOCONTROL_TAG,
                "%s[%d] mode not ready, AudioMode(%s)",
                (isSvlte ? "svlte setRefreshModem" : "setRefreshModem"),
                parameter->slotId,
                currentAudioModeStr.string());
        return;
    }

    if (activeModem == modemGsm) {
        android::MTKPrimaryDevicesHalClient::getInstance()->setParameters(
                (refreshMd + modemGsm));
    } else if (activeModem == modemC2k) {
        android::MTKPrimaryDevicesHalClient::getInstance()->setParameters(
                (refreshMd + modemC2k));
    } else {
        RFX_LOG_E(RP_AUDIOCONTROL_TAG,
                "%s[%d] refreshModem property unknown (activeModem=%s)",
                (isSvlte ? "svlte setRefreshModem" : "setRefreshModem"),
                parameter->slotId,
                activeModem.string());
    }
}

void * RpAudioControlHandler::refreshAudioMode(void *arg) {
    int result = 0;
    String8 currentAudioPhoneModemStr = String8("");
    String8 expectAudioPhoneModemStr = String8("");
    bool isSvlte = false;

    audioModeParameterStruct* parameter = (audioModeParameterStruct*)arg;
    result = pthread_mutex_lock(parameter->mutexObject);
    if (result != 0) {
        RFX_LOG_E(RP_AUDIOCONTROL_TAG,
                "refreshAudioMode, cannot lock parameter->mutexObject");
        return NULL;
    }

    if (!RpFeatureOptionUtils::isSvlteSupport()) {
        currentAudioPhoneModemStr =
                android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                        audioPhone1Md);
        expectAudioPhoneModemStr += audioPhone1Md;
        isSvlte = false;
    } else {
        switch (parameter->slotId) {
        case SIM_ID_1:
            currentAudioPhoneModemStr =
                    android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                            audioPhone1Md);
            expectAudioPhoneModemStr += audioPhone1Md;
            isSvlte = true;
            break;
        case SIM_ID_2:
            currentAudioPhoneModemStr =
                    android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                            audioPhone2Md);
            expectAudioPhoneModemStr += audioPhone2Md;
            isSvlte = true;
            break;
        }
    }

    if (currentAudioPhoneModemStr == (expectAudioPhoneModemStr + modemC2k)) {
        setRefreshModem(isSvlte, modemC2k, parameter);
    } else if (currentAudioPhoneModemStr == (expectAudioPhoneModemStr + modemGsm)) {
        setRefreshModem(isSvlte, modemGsm, parameter);
    } else {
        RFX_LOG_D(RP_AUDIOCONTROL_TAG, "%s, unknown phone modem(%s)",
                (isSvlte ? "svlte refreshAudioMode" : "refreshAudioMode"),
                currentAudioPhoneModemStr.string());
    }

    if (parameter->action != NULL) {
        RFX_LOG_D(RP_AUDIOCONTROL_TAG, "refreshAudioMode, Call callback function");
        parameter->action->act();
    }

    pthread_mutex_unlock(parameter->mutexObject);
    return NULL;
}

void RpAudioControlHandler::launchThreadToRefreshAudioMode() {
    PthreadPtr pptr = refreshAudioMode;
    pthread_t audioThreadId;
    pthread_attr_t attr;
    int result;

    RFX_LOG_D(RP_AUDIOCONTROL_TAG, "Start a thread to refresh audio mode");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    mAudioModeParameter.action = mAction;
    mAudioModeParameter.mutexObject = &mAudioSwitchMutex;
    result = pthread_create(&audioThreadId, &attr, pptr, &mAudioModeParameter);
    pthread_attr_destroy(&attr);
    if (result < 0) {
        RLOGE("pthread_create failed with result:%d", result);
    }
}

bool RpAudioControlHandler::needToRefreshAudioModem(int csPhone) {
    bool bNeedRefresh = false;
    String8 currentAudioPhoneModemStr = String8("");
    String8 expectAudioPhoneModemStr = String8("");

    if (!RpFeatureOptionUtils::isSvlteSupport()) {
        currentAudioPhoneModemStr =
                android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                        audioPhone1Md);
        expectAudioPhoneModemStr += audioPhone1Md;
    } else {
        switch (getSlotId()) {
            case SIM_ID_1:
                currentAudioPhoneModemStr =
                        android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                                audioPhone1Md);
                expectAudioPhoneModemStr += audioPhone1Md;
                break;
            case SIM_ID_2:
                currentAudioPhoneModemStr =
                        android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                                audioPhone2Md);
                expectAudioPhoneModemStr += audioPhone2Md;
                break;
        }
    }

    if (csPhone == RADIO_TECH_GROUP_C2K) {
        expectAudioPhoneModemStr += modemC2k;
    }
    if (csPhone == RADIO_TECH_GROUP_GSM) {
        expectAudioPhoneModemStr += modemGsm;
    }

    if (currentAudioPhoneModemStr != expectAudioPhoneModemStr) {
        bNeedRefresh = true;
    }

    RFX_LOG_D(RP_AUDIOCONTROL_TAG, "bNeedRefresh = %d, current(%s), expect(%s)",
            bNeedRefresh,
            currentAudioPhoneModemStr.string(), expectAudioPhoneModemStr.string());

    return bNeedRefresh;
}

/**
  * To notify AudioControlHandler that the current call should be redialed.
  * The handler have to check if the audio mode should be refreshed.
  *
  * @return True if audio mode should be refreshed, else return false.
  */

bool RpAudioControlHandler::updateAudioPathSync(int csPhone, const sp<RfxAction>& action) {
    if (needToRefreshAudioModem(csPhone)) {
        RFX_LOG_D(RP_AUDIOCONTROL_TAG, "updateAudioPathSync, csPhone = %d", csPhone);
        mAction = action;
        updateAudioModem(csPhone);
        mAudioModeParameter.action = mAction;
        mAudioModeParameter.mutexObject = &mAudioSwitchMutex;
        refreshAudioMode(&mAudioModeParameter);
        return true;
    }
    return false;
}


bool RpAudioControlHandler::updateAudioPathAsync(int csPhone) {
    if (needToRefreshAudioModem(csPhone)) {
        RFX_LOG_D(RP_AUDIOCONTROL_TAG, "updateAudioPathAsync, csPhone = %d", csPhone);
        mAction = NULL;
        updateAudioModem(csPhone);
        launchThreadToRefreshAudioMode();
        return true;
    }
    return false;
}

/**
  * Public interface to update audio mapping modem system property.
  *
  * @param csPhone To indicate the current active modem
  */
void RpAudioControlHandler::updateAudioModem(int csPhone) {
    String8 currentAudioPhoneModemStr = String8("");
    String8 newAudioPhoneModemStr = String8("");

    if (!RpFeatureOptionUtils::isSvlteSupport()) {
        newAudioPhoneModemStr += audioPhone1Md;
    } else {
        switch (getSlotId()) {
            case SIM_ID_1:
                newAudioPhoneModemStr += audioPhone1Md;
                break;
            case SIM_ID_2:
                newAudioPhoneModemStr += audioPhone2Md;
                break;
        }
    }

    if (csPhone == RADIO_TECH_GROUP_C2K) {
        newAudioPhoneModemStr += modemC2k;
    }
    if (csPhone == RADIO_TECH_GROUP_GSM) {
        newAudioPhoneModemStr += modemGsm;
    }
    android::MTKPrimaryDevicesHalClient::getInstance()->setParameters(
            newAudioPhoneModemStr);

    RFX_LOG_D(RP_AUDIOCONTROL_TAG, "updateAudioModem, new audio modem(%s)",
            newAudioPhoneModemStr.string());
}

