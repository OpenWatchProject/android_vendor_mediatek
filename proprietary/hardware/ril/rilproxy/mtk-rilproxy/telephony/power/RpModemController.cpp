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

#include <cutils/properties.h>
#include <cutils/jstring.h>
#include "RpModemController.h"
#include "RfxDefs.h"
#include "RfxLog.h"
#include "RfxStatusDefs.h"
#include "RfxStatusManager.h"
#include <telephony/mtk_ril.h>
#include "RadioConstants.h"
#include <log/log.h>
#include "RfxSocketState.h"
#include "nw/RpNwRatController.h"
#include "util/RpFeatureOptionUtils.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "RpMDCtrl"
#endif

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#define FIRST_TIME_RLOGD(last_step,cur_step,log) if(last_step!=cur_step){RLOGD(log);last_step=cur_step;}

#define CDMA "C"
#define LteFdd "Lf"
#define LteTdd "Lt"


/*****************************************************************************
 * Class RpModemController
 *****************************************************************************/

const char PROPERTY_NET_CDMA_MDMSTAT[25] = "vendor.net.cdma.mdmstat";
const char PROPERTY_CAPABILITY_SWITCH[35] = "persist.vendor.radio.simswitch";
const char PROPERTY_BUILD_RAT_CONFIG[40] = "ro.vendor.mtk_protocol1_rat_config";


static const int request[] = {
    RIL_REQUEST_SHUTDOWN,
    RIL_REQUEST_CONFIG_MODEM_STATUS,
    RIL_REQUEST_CONFIG_MODEM_STATUS_C2K,
    RIL_REQUEST_MODEM_POWERON,
    RIL_REQUEST_MODEM_POWEROFF,
    RIL_REQUEST_RESET_RADIO,
    RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K,
    RIL_REQUEST_RESTART_RILD
};

static const int urc[] = {
    RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER
};


static RpModemController* controllerInstance = NULL;

static const int MODEM_POWER_OFF = 0;
static const int MODEM_POWER_ON = 1;

RFX_IMPLEMENT_CLASS("RpModemController", RpModemController, RfxController);

TimerHandle RpModemController::s_cmd_startd_th = 0;// a invalid timer handle

void RpModemController::onInit() {
    if (controllerInstance == NULL) {
        //super.init
        RLOGD("First Init()");
        RfxController::onInit();
        cdmaSlotId = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
                    RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
        controllerInstance = this;
        if (RpFeatureOptionUtils::isC2kSupport()) {
            RLOGD("array size %d", (int)(ARRAY_LENGTH(request)));
            for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
                registerToHandleRequest(i, request, ARRAY_LENGTH(request));
                registerToHandleUrc(i, urc, ARRAY_LENGTH(urc));
            }
        }
        initModemMessages();
        getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_SOCKET_SLOT,
                RfxStatusChangeCallback(this, &RpModemController::onCdmaSocketSlotChange));
        action = NULL;
        cdmaLteModeControllerToken = -1;
        mModemStatus = -1;
        mRemoteProtocol = -1;
        mLastModemStatus = -1;
        mLastRemoteProtocol = -1;
        canHandleErrStep = 0;
        getStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_VERSION, -1);
        // handleQueueModemPower();

        // no need to reset modemMessage after RilProxy design change, RilProxy will reinit follow RILD
        /*for (int i = 0; i< RFX_SLOT_COUNT; i++) {
            getStatusManager(i)->registerStatusChangedEx(RFX_STATUS_KEY_SOCKET_STATE,
                    RfxStatusChangeCallbackEx(this, &RpModemController::onSocketStateChange));
        }*/
    } else {
        RLOGD("Already Init, don't init again");
    }
}

void RpModemController::onDeinit() {
    /*for (int i = 0; i< RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->unRegisterStatusChangedEx(RFX_STATUS_KEY_SOCKET_STATE,
                RfxStatusChangeCallbackEx(this, &RpModemController::onSocketStateChange));
    }*/
    RfxController::onDeinit();
}


void RpModemController::initModemMessages() {
    // TODO: only for DSDS, should handle with DSDA if necessary
    int modemCount = 1;
    modemMessages = new ModemMessage[MAX_SIM_COUNT];
    modemPower = new int[modemCount];
    emdstatusMessage = new ModemMessage();

    for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
        modemMessages[i] = ModemMessage();
        //TODO: only for DSDS part now, should handle with DSDA if necessary
        modemMessages[i].modemId = 0;
        modemMessages[i].connectWithCdmaModem = true;
        modemMessages[i].slotId = i;
        modemMessages[i].action = NULL;
        modemMessages[i].fromCdmaLteModeController = false;
        modemMessages[i].svlteModemStatusFirstDone = false;
    }
    for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
        RLOGD("modemMessage %d is %s", i, modemMessages[i].connectWithCdmaModem ? "true" : "false");
    }

    for (int i = 0; i < modemCount; i++) {
        modemPower[i] = MODEM_POWER_ON;
    }
}

RpModemController::RpModemController() {
}


RpModemController::~RpModemController() {
    delete(modemMessages);
    delete(modemPower);
    delete(emdstatusMessage);
}


RpModemController* RpModemController::getInstance() {
    if (controllerInstance != NULL) {
        return controllerInstance;
    } else {
        // TODO: should initialize
        return controllerInstance;
    }
}

bool RpModemController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (message -> getType() == REQUEST) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        RLOGD("Not Request, don't do check");
        return true;
    }
}

bool RpModemController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (message -> getType() == REQUEST) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool RpModemController::canHandleRequest(const sp<RfxMessage>& message) {
    int slotId = message -> getSlotId();
    if(modemMessages[slotId].originalMessage == NULL) {
        bool allSocketConnect = true;
        for (int i=0; i<RpFeatureOptionUtils::getSimCount(); i++) {
            RfxSocketState socketState = getStatusManager(i)->getSocketStateValue(
                    RFX_STATUS_KEY_SOCKET_STATE);
            allSocketConnect = allSocketConnect && socketState.getSocketState(
                        socketState.SOCKET_GSM);
            if (socketState.getIsCdmaSlot()) {
                 allSocketConnect = allSocketConnect && socketState.getSocketState(
                        socketState.SOCKET_C2K);
            }
        }
        if (!allSocketConnect) {
            FIRST_TIME_RLOGD(canHandleErrStep, 1, "RpModemController: allSocketConnect = false");
            return false;
        }
    } else {
        FIRST_TIME_RLOGD(canHandleErrStep, 2, "RpModemController: original message is null");
        return false;
    }

    // Avoid MD power off conflict
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    if (modemOffState != MODEM_OFF_IN_IDLE && modemOffState != MODEM_OFF_BY_POWER_OFF) {
        if (canHandleErrStep != (3 + modemOffState)) {
            RLOGD("RpModemController: canHandleRequest:0, modemOffState=%d", modemOffState);
            canHandleErrStep = 3 + modemOffState;
        }
        return false;
    }

    // check modem version
    int modemVersion = getStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_VERSION, -1);
    if (modemVersion == -1) {
        RLOGD("RpModemController: canHandleRequest:%d, modemVersion is not ready", false);
        return false;
    }

    canHandleErrStep = 0;
    RLOGD("RpModemController: canHandleRequest:%d", true);
    return true;
}

bool RpModemController::onHandleRequest(const sp<RfxMessage>& message) {

    int msg_id = message -> getId();
    RLOGD("RpModemController: handle request id %d", msg_id);

    // Avoid MD power off conflict
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);

    switch (msg_id) {
    case RIL_REQUEST_MODEM_POWERON:
        requestModemPower(MODEM_POWER_ON, message);
        break;
    case RIL_REQUEST_SHUTDOWN:
        requestModemPower(MODEM_POWER_OFF, message);
        break;
    case RIL_REQUEST_MODEM_POWEROFF:
        if (modemOffState == MODEM_OFF_IN_IDLE) {
            getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_BY_POWER_OFF);
            RLOGD("RpModemController: set RFX_STATUS_KEY_MODEM_OFF_STATE to %d", MODEM_OFF_BY_POWER_OFF);
        }
        requestModemPower(MODEM_POWER_OFF, message);
        break;
    case RIL_REQUEST_RESET_RADIO:
    case RIL_REQUEST_RESTART_RILD:
        if (modemOffState == MODEM_OFF_IN_IDLE) {
            getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_BY_RESET_RADIO);
            RLOGD("RpModemController: set RFX_STATUS_KEY_MODEM_OFF_STATE to %d", MODEM_OFF_BY_RESET_RADIO);
            requestResetRadio(message);
        }
        break;
    default:
        break;
    }
    return true;
}

void RpModemController::requestModemPower(int power, const sp<RfxMessage>& message) {
    int msg_id = message -> getId();
    int pToken = message -> getPToken();
    int slotId = message -> getSlotId();
    int mainSlotId = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
                    RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);

    RLOGD("requestModemPower, slotId=%d, power=%d", slotId, power);
    // TODO: need to handle DSDA
    if (power == MODEM_POWER_OFF) {
        for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
            bool modemPowerOff = getStatusManager(i)->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
            if (modemPowerOff) {
                RLOGD("Modem Off, return");
                sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
                responseToRilj(responseMsg);
                return;
            }
            getStatusManager(i)->setBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, true);
        }
    }

    if (power == MODEM_POWER_ON) {
        RLOGD("Modem Power on, clear last radio suggest capability.");
        RpNwRatController* nwRatController;
        for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
            nwRatController = (RpNwRatController *) findController(i, RFX_OBJ_CLASS_INFO(RpNwRatController));
            nwRatController->clearSuggetRadioCapability();
        }
    }

    RLOGD("Modem Power %s is apply on %d with serial %d", (power == 1) ? "on" : "off,", slotId,
            pToken);

    int modemId = modemMessages[slotId].modemId;

    // Modem message will only do once at a time
    ModemMessage* modemMessage = &modemMessages[slotId];
    modemMessage -> serial = pToken;
    modemMessage -> originalMessage = message;
    if (message->getToken() == cdmaLteModeControllerToken) {
        RLOGD("MD off by CdmaLteModeController ");
        modemMessage->action = action;
        modemMessage->fromCdmaLteModeController = true;
    }

    // create gsm Modem Request
    RLOGD("Create Gsm Modem Request with serial %d, slot %d", pToken, mainSlotId);
    sp<RfxMessage> gsmModemRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, msg_id,
            message);
    gsmModemRequest->setSlotId(mainSlotId);
    modemMessage -> gsmRequest = gsmModemRequest;
    requestToRild(gsmModemRequest);

    int modemVersion = getStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_VERSION, 0);
    if (modemVersion == 1 && power == MODEM_POWER_ON) {
        modemMessage->connectWithCdmaModem = false;
    } else {
        modemMessage->connectWithCdmaModem = true;
    }
    // create cdma Modem Request
    RLOGD("Connect with Cdma %s", (modemMessage->connectWithCdmaModem) ? "true" : "false");
    if (modemMessage->connectWithCdmaModem) {
        RLOGD("Create Cdma Modem Request with serial %d", pToken);
        sp<RfxMessage> cdmaModemRequest;
        if (modemVersion == 1 && power == MODEM_POWER_OFF) {
            cdmaModemRequest = RfxMessage::obtainRequest(cdmaSlotId, RADIO_TECH_GROUP_C2K,
                RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K);
        } else {
            cdmaModemRequest = RfxMessage::obtainRequest(cdmaSlotId, RADIO_TECH_GROUP_C2K, msg_id);
        }
        modemMessage->cdmaRequest = cdmaModemRequest;
        requestToRild(cdmaModemRequest);
    }
    modemPower[modemId] = power;
}

bool RpModemController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message -> getId();
    RLOGD("RpModemController: handle urc id %d", msg_id);
    switch (msg_id) {
    case RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER:
        handleEnhanceModemOffURC(message);
        break;
    default:
        break;
    }
    return true;
}

bool RpModemController::onHandleResponse(const sp<RfxMessage>& message) {

    int msg_id = message -> getId();
    RLOGD("RpModemController: handle response id %d", msg_id);

    switch (msg_id) {
    case RIL_REQUEST_MODEM_POWERON:
    case RIL_REQUEST_SHUTDOWN:
    case RIL_REQUEST_MODEM_POWEROFF:
    case RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K:
        handleModemPowerResponse(message);
        break;
    case RIL_REQUEST_RESET_RADIO:
    case RIL_REQUEST_RESTART_RILD:
        handleResetRadioResponse(message);
        break;
    case RIL_REQUEST_CONFIG_MODEM_STATUS:
    case RIL_REQUEST_CONFIG_MODEM_STATUS_C2K:
        handleModemStatusResponse(message);
        break;
    default:
        break;
    }
    return true;
}

bool RpModemController::handleModemPowerResponse(const sp<RfxMessage>& message) {
    RIL_Errno rilErrno = message -> getError();
    ModemMessage* modemMessage = NULL;
    // Find modem message using ptoken if is from cdma
    for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
        if (modemMessages[i].cdmaRequest == NULL) {
            RLOGD("modem message %d: cdma Request is null", i);
        } else {
            RLOGD("modem message %d: cdma Request ptoken is %d", i, modemMessages[i].cdmaRequest->getPToken());
        }
    }
    if (message->getSource() == RADIO_TECH_GROUP_C2K) {
        for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
            if (modemMessages[i].cdmaRequest != NULL
                    && modemMessages[i].cdmaRequest->getPToken() == message->getPToken()) {
                modemMessage = &modemMessages[i];
                RLOGD("Find Modem message %d using message pToken %d", i, message->getPToken());
                break;
            }
        }
    } else {
        for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
            if (modemMessages[i].gsmRequest != NULL
                    && modemMessages[i].gsmRequest->getPToken() == message->getPToken()) {
                modemMessage = &modemMessages[i];
                RLOGD("Find Modem message %d using message pToken %d", i, message->getPToken());
                break;
            }
        }
    }

    // Couldn't find the right modem message here, so we stop here
    if (modemMessage == NULL) {
        LOG_ALWAYS_FATAL("Can't find modem message here");
        return false;
    }

    if (message->getSource() == RADIO_TECH_GROUP_C2K) {
        RLOGD("Handle Response From Cdma");
        if (rilErrno != RIL_E_SUCCESS) {
            RLOGD("cdma modem power request fail, try again");
            modemMessage->cdmaRequest->setSlotId(cdmaSlotId);
            requestToRild(modemMessage->cdmaRequest);
            return true;
        } else {
            RLOGD("cdma modem power request sucess");
            modemMessage->cdmaResponse = message;
        }
    } else {
        RLOGD("Handle Response From Gsm");
        if (rilErrno != RIL_E_SUCCESS) {
            RLOGD("gsm modem poower request fail, try again");
            requestToRild(modemMessage->gsmRequest);
            return true;
        } else {
            RLOGD("gsm modem power request sucess");
            modemMessage->gsmResponse = message;
        }
    }
    bool ret = checkIfRequestComplete(modemMessage);
    if (ret) {
        if (modemMessage->fromCdmaLteModeController) {
            RLOGD("Request Complete, excute action (request from fromCdmaLteModeController)");
            modemMessage->action->act();
            action = NULL;
            cdmaLteModeControllerToken = -1;
        } else {
            sp<RfxMessage> response =  RfxMessage:: obtainResponse(RIL_E_SUCCESS,
                    modemMessage -> originalMessage);
            RLOGD("Request Complete, send response");
            responseToRilj(response);
        }
        modemMessage -> resetModemMessage();
        if (message -> getId() == RIL_REQUEST_RESET_RADIO ||
                message->getId() == RIL_REQUEST_RESTART_RILD) {
            getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
            RLOGD("RpModemController: set RFX_STATUS_KEY_MODEM_OFF_STATE to %d", MODEM_OFF_IN_IDLE);
        }
    }
    return true;
}

bool RpModemController::checkIfRequestComplete(ModemMessage* modemMessage){
    bool ret = false;
    bool sendToCdma = modemMessage->connectWithCdmaModem;
    RLOGD("modem message sendToCdma: %s", sendToCdma ? "true" : "false");
    if (sendToCdma == true) {
        if ((modemMessage->gsmResponse) != NULL && (modemMessage->cdmaResponse) != NULL) {
            RLOGD("both side is received, modemMessage %d is complete", modemMessage->serial);
            ret = true;
        }
    } else {
        if ((modemMessage->gsmResponse) != NULL) {
            RLOGD("modemMessage %d is complete", modemMessage->serial);
            ret = true;
        }
    }
    return ret;
}

// set cdma_slot when configModemStatus
void RpModemController::configModemStatus(int cardType1, int cardType2, int cdmaSlot, const sp<RfxAction>& action) {
    char md3State[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_NET_CDMA_MDMSTAT, md3State, "not ready");
    RLOGD("configModemStatus, net.cdma.mdmstat: %s", md3State);

    if (isSvlteSupport()){
        if ((strncmp(md3State, "ready", 5) != 0)) {
            if (action != NULL) {
                emdstatusMessage->action = action;
            }
            this->emdstatusCardType1 = cardType1;
            this->emdstatusCardType2 = cardType2;
            this->emdstatusCdmaSlot = cdmaSlot;
            s_cmd_startd_th = RfxTimer::start(
                    RfxCallback0(this, &RpModemController::onConfigModemTimer),
                    ms2ns(INITIAL_RETRY_INTERVAL_MSEC));
            return;
        } else {
            RfxTimer::stop(s_cmd_startd_th);
            s_cmd_startd_th = 0;
        }
    }

    RLOGD("configModemStatus, slot number: %d, cardType1: %d, cardType2: %d", RFX_SLOT_COUNT, cardType1, cardType2);
    calculateRemoteSim(cardType1, cardType2, cdmaSlot);
    mLastModemStatus = mModemStatus;
    mLastRemoteProtocol = mRemoteProtocol;
    RLOGD("update mLastModemStatus=%d, mLastRemoteProtocol=%d", mLastModemStatus, mLastRemoteProtocol);
    RLOGD("configModemStatus: (mModemStatus, mRemoteProtocol, cdma_slot)"
        ":(%d, %d, %d)", mModemStatus, mRemoteProtocol, cdmaSlot);

    if (isSvlteSupport()){
        //For SVLTE project, we need to send emdstatus to both MD/Slot
        for (int i = 0; i < RFX_SLOT_COUNT; i++) {
            sp<RfxMessage> gsmrequest = RfxMessage::obtainRequest(i, RADIO_TECH_GROUP_GSM, RIL_REQUEST_CONFIG_MODEM_STATUS);
            Parcel* p = gsmrequest->getParcel();
            p->writeInt32(2);
            p->writeInt32(mModemStatus);
            p->writeInt32(mRemoteProtocol);
            emdstatusMessage->gsmRequest = gsmrequest;
            emdstatusMessage->connectWithCdmaModem = true;
            if (action != NULL) {
                emdstatusMessage->action = action;
            }
            requestToRild(gsmrequest);
            RLOGD("configModemStatus: for slot[%d] gsm rild", i);
        }
        sp<RfxMessage> c2krequest = RfxMessage::obtainRequest(cdmaSlot,
                                             RADIO_TECH_GROUP_C2K,
                                             RIL_REQUEST_CONFIG_MODEM_STATUS_C2K);
        Parcel* c2kp = c2krequest->getParcel();
        c2kp->writeInt32(2);
        c2kp->writeInt32(mModemStatus);
        c2kp->writeInt32(mRemoteProtocol);
        emdstatusMessage->cdmaRequest = c2krequest;
        this -> cdmaModemStatusRequest = c2krequest;
        this -> sendCdmaModemStatus();
        RLOGD( "configModemStatus: for slot[%d] cdma rild", cdmaSlot);
    }
    else {
        //For SRLTE project, we need to send emdstatus to MD1 only
        int mainSlotId = getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
        sp<RfxMessage> request = RfxMessage::obtainRequest(mainSlotId, RADIO_TECH_GROUP_GSM, RIL_REQUEST_CONFIG_MODEM_STATUS);
        Parcel* p = request->getParcel();
        p->writeInt32(2);
        p->writeInt32(mModemStatus);
        p->writeInt32(mRemoteProtocol);
        emdstatusMessage->originalMessage = request;
        if (action != NULL) {
            emdstatusMessage->action = action;
        }

        requestToRild(request);
        RLOGD("configModemStatus: for slot[%d] gsm rild", mainSlotId);
    }


}


void RpModemController::sendCdmaModemStatus() {
    this -> requestToRild(cdmaModemStatusRequest);
}

void RpModemController::onConfigModemTimer() {
    configModemStatus(this->emdstatusCardType1, this->emdstatusCardType2, this->emdstatusCdmaSlot, NULL);
}

bool RpModemController::is4GCdmaCard(int cardType) {
    if ((cardType & RFX_CARD_TYPE_USIM) > 0 &&
        ((cardType & RFX_CARD_TYPE_CSIM) > 0 ||
        (cardType & RFX_CARD_TYPE_RUIM) > 0)) {
        return true;
    }
    return false;
}

bool RpModemController::is3GCdmaCard(int cardType) {
    if ((cardType & RFX_CARD_TYPE_SIM) == 0 &&
        (cardType & RFX_CARD_TYPE_USIM) == 0 &&
        ((cardType & RFX_CARD_TYPE_CSIM) > 0 ||
        (cardType & RFX_CARD_TYPE_RUIM) > 0)) {
        return true;
    }
    return false;
}

bool RpModemController::isGsmCard(int cardType) {
    if ((cardType & RFX_CARD_TYPE_CSIM) == 0 &&
        (cardType & RFX_CARD_TYPE_RUIM) == 0 &&
        ((cardType & RFX_CARD_TYPE_SIM) > 0 ||
        (cardType & RFX_CARD_TYPE_USIM) > 0)) {
        return true;
    }
    return false;
}

bool RpModemController::isNonCard(int cardType) {
    if (cardType == 0) {
        return true;
    }
    return false;
}

bool RpModemController::isCUUimCard(int cardType) {
    if (cardType == 10) {
        return true;
    }
    return false;
}

bool RpModemController::isCdmaIratSupport() {
    char temp[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_BUILD_RAT_CONFIG, temp, "");
    /* Check if RAT support CDMA and any LTE */
    if ((strstr(temp, CDMA)  !=  NULL) &&
        ((strstr(temp, LteFdd)  !=  NULL) || (strstr(temp, LteTdd)  !=  NULL))) {
        return true;
    } else {
        return false;
    }

}

void RpModemController::onCdmaSocketSlotChange(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int c2kSlot = value.asInt();
    cdmaSlotId = c2kSlot;
    RLOGD("onCdmaSocketSlotChange: update cdma slot %d", c2kSlot);
}

void RpModemController::onSocketStateChange(int slotId, RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(slotId);
    RFX_UNUSED(key);
    RfxSocketState old_state = old_value.asSocketState();
    RfxSocketState new_state = value.asSocketState();

    bool old_gsm = old_state.getSocketState(RfxSocketState::SOCKET_GSM);
    bool new_gsm = new_state.getSocketState(RfxSocketState::SOCKET_GSM);
    bool old_c2k = old_state.getSocketState(RfxSocketState::SOCKET_C2K);
    bool new_c2k = new_state.getSocketState(RfxSocketState::SOCKET_C2K);
    RLOGD("onSocketStateChange old_gsm=%d, new_gsm=%d, old_c2k=%d, new_c2k=%d", old_gsm, new_gsm, old_c2k, new_c2k);

    bool reset_msg = false;
    if ((old_gsm != new_gsm) && (new_gsm == SOCKET_DISCONNECTED)) {
        reset_msg = true;
    } else if ((old_c2k != new_c2k) && (new_c2k == SOCKET_DISCONNECTED)) {
        reset_msg = true;
    }

    RLOGD("onSocketStateChange: reset_msg=%d", reset_msg);
    if (reset_msg) {
        for (int i = 0; i < RpFeatureOptionUtils::getSimCount(); i++) {
            modemMessages[i].resetModemMessage();
        }
    }
}

bool RpModemController::isOP12Support() {
    bool result = false;
    char optr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.operator.optr", optr, "");
    if (strcmp(optr, "OP12") == 0 || strcmp(optr, "OP20") == 0) {
        result = true;
    }
    return result;
}

void RpModemController::calculateRemoteSim(int cardType1, int cardType2, int cdmaSlot) {
    char temp[PROPERTY_VALUE_MAX] = {0};
    int capability = 1;
    property_get(PROPERTY_CAPABILITY_SWITCH, temp, "1");
    capability = atoi(temp);
    RLOGD("calculateRemoteSim, cardType1:%d, cardType2:%d, persist.vendor.radio.simswitch:%s, cdmaSlot:%d",
            cardType1, cardType2, temp, cdmaSlot);

    mModemStatus = 2;
    mRemoteProtocol = 1;
    while (1) {
        if (RpFeatureOptionUtils::getSimCount() >= 2) {
            RLOGD("configModemStatus: Solution2");
            // case1: slot1 CDMA
            if ((is4GCdmaCard(cardType1) || is3GCdmaCard(cardType1))
                && (is4GCdmaCard(cardType2) || is3GCdmaCard(cardType2))) {
                // 2 CDMA SIM, C2K modem always follow 3/4G slot
                mRemoteProtocol = 1;
                if (capability == 1) {
                    if (is4GCdmaCard(cardType1)) {
                        mModemStatus = 2;
                    } else {
                        mModemStatus = 1;
                    }
                } else {
                    if (is4GCdmaCard(cardType2)) {
                        mModemStatus = 2;
                    } else {
                        mModemStatus = 1;
                    }
                }
                if (!isCdmaIratSupport()) {
                    mModemStatus = 1;
                }
                break;
            }

            // case2: slot1 CDMA && slot2 non-CDMA
            if (is4GCdmaCard(cardType1) &&
                (isGsmCard(cardType2) ||
                isNonCard(cardType2))) {
                // slot1 CT 4G, slot2 any
                if (!isCdmaIratSupport()) {
                    mModemStatus = 1;
                } else {
                    mModemStatus = 2;
                }

                if (capability == 1) {
                    mRemoteProtocol = 1;
                } else {
                    mRemoteProtocol = 2;
                    //EMDSTATUS : 2, 2 is not suitable for SRLTE
                    if (isSrlteSupport()) {
                        mModemStatus = 1;
                    }
                }
                break;
            } else if (is3GCdmaCard(cardType1) &&
                (isGsmCard(cardType2) ||
                isNonCard(cardType2))) {
                // slot1 CT 3G, slot2 any
                mModemStatus = 1;
                if (capability == 1) {
                    mRemoteProtocol = 1;
                } else {
                    mRemoteProtocol = 2;
                }
                break;
            }

            // case2: slot2 CDMA && slot1 non-CDMA
            if (isGsmCard(cardType1) || isNonCard(cardType1)) {
                if (is4GCdmaCard(cardType2)) {
                    // slot1 GSM or N/A, slot2 CT 4G
                    if (!isCdmaIratSupport()) {
                        mModemStatus = 1;
                    } else {
                        mModemStatus = 2;
                    }

                    if (capability == 1) {
                        mRemoteProtocol = 2;
                        //EMDSTATUS : 2, 2 is not suitable for SRLTE
                        if (isSrlteSupport()) {
                            mModemStatus = 1;
                        }
                    } else {
                        mRemoteProtocol = 1;
                    }
                    break;
                } else if (is3GCdmaCard(cardType2)) {
                    // slot1 GSM or N/A, slot2 CT 3G
                    mModemStatus = 1;
                    if (capability == 1) {
                        mRemoteProtocol = 2;
                    } else {
                        mRemoteProtocol = 1;
                    }
                    break;
                }
            }

            // case3: non-CDMA
            if (isGsmCard(cardType1) && isGsmCard(cardType2)) {
                // slot1 GSM, slot2 GSM
                mModemStatus = 0;
                if (capability == 1) {
                    if (cdmaSlot == 0) {
                        mRemoteProtocol = 1;
                    } else {
                        mRemoteProtocol = 2;
                    }
                } else {
                    if (cdmaSlot == 0) {
                        mRemoteProtocol = 2;
                    } else {
                        mRemoteProtocol = 1;
                    }
                }
                break;
            } else if (isGsmCard(cardType1) && isNonCard(cardType2)) {
                // slot1 GSM, slot2 N/A
                // As New ECC design we need to follow Cdmaslot when only 1 GSM card.
                if (capability == 1) {
                    if (cdmaSlot == 0) {
                        mModemStatus = 0;
                        mRemoteProtocol = 1;
                    } else {
                        mModemStatus = 1;
                        mRemoteProtocol = 2;
                    }
                } else {
                    if (cdmaSlot == 0) {
                        mModemStatus = 0;
                        mRemoteProtocol = 2;
                    } else {
                        mModemStatus = 1;
                        mRemoteProtocol = 1;
                    }
                }
                break;
            } else if (isNonCard(cardType1) && isGsmCard(cardType2)) {
                // slot1 N/A, slot2 GSM
                // As New ECC design we need to follow Cdmaslot when only 1 GSM card.
                if (capability == 1) {
                    if (cdmaSlot == 0) {
                        mModemStatus = 1;
                        mRemoteProtocol = 1;
                    } else {
                        mModemStatus = 0;
                        mRemoteProtocol = 2;
                    }
                } else {
                    if (cdmaSlot == 0) {
                        mModemStatus = 1;
                        mRemoteProtocol = 2;
                    } else {
                        mModemStatus = 0;
                        mRemoteProtocol = 1;
                    }
                }
                break;
            } else if (isNonCard(cardType1) && isNonCard(cardType2)) {
                // slot1 N/A, slot2 N/A
                if (capability == 1) {
                    if (cdmaSlot == 0) {
                        mModemStatus = 2;
                        mRemoteProtocol = 1;
                    } else {
                        mModemStatus = 1;
                        mRemoteProtocol = 2;
                    }
                } else {
                    if (cdmaSlot == 0) {
                        mModemStatus = 1;
                        mRemoteProtocol = 2;
                    } else {
                        mModemStatus = 2;
                        mRemoteProtocol = 1;
                    }
                }
                break;
            }
        } else {
            RLOGD("configModemStatus: Solution1 for SS[%d]", RpFeatureOptionUtils::getSimCount());
            bool md3AccessProtocol2 = (capability != 1);  // ES2g = 2
            if (isNonCard(cardType1)) {
                // no card
                RLOGD("configModemStatus: no card");
                mModemStatus = 2;
                mRemoteProtocol = md3AccessProtocol2 ? 2 : 1;
            } else if (isGsmCard(cardType1)) {
                if (isOP12Support()) {
                    RLOGD("configModemStatus: op12");
                    mModemStatus = 2;
                    mRemoteProtocol = 1;
                } else {
                    // GSM only
                    RLOGD("configModemStatus: GSM only");
                    mModemStatus = 0;
                    mRemoteProtocol = 1;
                }
            } else if (is3GCdmaCard(cardType1)) {
                // CT 3G
                RLOGD("configModemStatus: CT 3G");
                mModemStatus = 1;
                mRemoteProtocol = md3AccessProtocol2 ? 2 : 1;
            } else if (is4GCdmaCard(cardType1)) {
                // CT 4G
                RLOGD("configModemStatus: CT 4G");
                if (!isCdmaIratSupport()) {
                    mModemStatus = 1;
                } else {
                    mModemStatus = 2;
                }
                mRemoteProtocol = md3AccessProtocol2 ? 2 : 1;
            } else {
                // other case, may not happen!
                RLOGD("configModemStatus: other case, may not happen!");
                break;
            }
        }
        break;
    }

    RLOGD("calculateRemoteSim: mModemStatus:%d, mRemoteProtocol:%d", mModemStatus, mRemoteProtocol);
}

bool RpModemController::isEmdstatusChanged(int cardType1, int cardType2, int cdmaSlot) {
    RLOGD("isEmdstatusChanged, cardType1:%d, cardType2:%d, cdmaSlot:%d", cardType1, cardType2, cdmaSlot);
    calculateRemoteSim(cardType1, cardType2, cdmaSlot);
    RLOGD("isEmdstatusChanged, mLastModemStatus:%d, mModemStatus=%d; lastRemoteProtocol:%d, remoteProtocol:%d",
            mLastModemStatus, mModemStatus, mLastRemoteProtocol, mRemoteProtocol);
    if ((mLastModemStatus != mModemStatus) || (mLastRemoteProtocol != mRemoteProtocol)) {
        return true;
    } else {
        return false;
    }
}


bool RpModemController::isRemoteSimProtocolChanged(int cardType1, int cardType2, int cdmaSlot) {
    RLOGD("isEmdstatusChanged(RemoteSIMOnly), cardType1:%d, cardType2:%d, cdmaSlot:%d", cardType1, cardType2, cdmaSlot);
    calculateRemoteSim(cardType1, cardType2, cdmaSlot);
    RLOGD("isEmdstatusChanged(RemoteSIMOnly), mLastModemStatus:%d, mModemStatus=%d; lastRemoteProtocol:%d, remoteProtocol:%d",
            mLastModemStatus, mModemStatus, mLastRemoteProtocol, mRemoteProtocol);
    if ((mLastRemoteProtocol != -1) && (mLastRemoteProtocol != mRemoteProtocol)) {
        return true;
    } else {
        return false;
    }
}


void RpModemController::handleModemStatusResponse(const sp<RfxMessage>& message) {
    if (isSvlteSupport()) {
        RLOGD("SVLTE project need to wait both gsm and c2k response");
        if (emdstatusMessage->gsmRequest != NULL && emdstatusMessage->cdmaRequest != NULL) {
            if (message->getSource() == RADIO_TECH_GROUP_C2K) {
                RLOGD("SVLTE Handle Response From Cdma");
                emdstatusMessage->cdmaResponse = message;
            } else {
                RLOGD("SVLTE Handle Response From Gsm, [%d][%d]", emdstatusMessage->svlteModemStatusFirstDone, RFX_SLOT_COUNT);
                if (!emdstatusMessage->svlteModemStatusFirstDone && (RFX_SLOT_COUNT != 1)) {
                    emdstatusMessage->svlteModemStatusFirstDone = true;
                } else {
                    emdstatusMessage->gsmResponse = message;
                }
            }

            bool ret = checkIfRequestComplete(emdstatusMessage);
            if (ret) {
                property_set(emdstatusProperty, "1");
                RLOGD("SVLTE vendor.ril.cdma.emdstatus.send is set to 1");
                emdstatusMessage->action->act();
                emdstatusMessage->resetModemMessage();
            }
        } else {
            RLOGD("SVLTE handleModemStatusResponse error gsmRequest or cdmaRequest NULL");
        }
    }else {
        property_set(emdstatusProperty, "1");
        RLOGD("vendor.ril.cdma.emdstatus.send is set to 1");
        if (emdstatusMessage->originalMessage != NULL) {
            RLOGD("handleModemStatusResponse done.");
            emdstatusMessage->action->act();
            emdstatusMessage->resetModemMessage();
        } else {
            RLOGD("handleModemStatusResponse error, emdstatusMessage = NULL!");
        }
    }
}

void RpModemController::requestResetRadio(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int pToken = message->getPToken();
    int slotId = message->getSlotId();

    int modemId = modemMessages[slotId].modemId;
    ModemMessage* modemMessage = &modemMessages[slotId];
    modemMessage->serial = pToken;
    modemMessage->originalMessage = message;

    // create gsm Request
    RLOGD("Create Gsm Request with serial %d", pToken);
    sp<RfxMessage> gsmModemRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, msg_id,
            message);
    modemMessage->gsmRequest = gsmModemRequest;
    requestToRild(gsmModemRequest);

    int modemVersion = getStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_VERSION, 0);
    if (modemVersion != 1) {
        // create cdma Modem Request
        RLOGD("Create Cdma Request with serial %d", pToken);
        sp<RfxMessage> cdmaModemRequest = RfxMessage::obtainRequest(cdmaSlotId, RADIO_TECH_GROUP_C2K,
            msg_id);
        modemMessage->cdmaRequest = cdmaModemRequest;
        requestToRild(cdmaModemRequest);
    }
    for (int i = 0; i < SIM_COUNT; i++) {
        getStatusManager(i)->setBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, true);
    }
}

void RpModemController::handleResetRadioResponse(const sp<RfxMessage>& message) {
    handleModemPowerResponse(message);
}


void RpModemController::handleEnhanceModemOffURC(const sp<RfxMessage>& message) {
    int32_t count = 0;
    int modemVersion = 0;
    Parcel *p = message->getParcel();
    p->readInt32(&count);
    p->readInt32(&modemVersion);
    RLOGD("handleEnhanceModemOffURC version: %d", modemVersion);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_VERSION, modemVersion);
}
