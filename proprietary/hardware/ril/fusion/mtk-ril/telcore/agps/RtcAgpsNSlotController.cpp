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
#include "RtcAgpsNSlotController.h"
#include <stdlib.h>
#include "RfxIntsData.h"
#include "telephony/mtk_ril.h"
#include "RfxRilUtils.h"
#include "rfx_properties.h"


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

static const int URC_LIST[] = {
    RFX_MSG_URC_VIA_GPS_EVENT
};
static const int AGPS_REQUEST[] = {
    RFX_MSG_REQUEST_AGPS_TCP_CONNIND,
};


RFX_IMPLEMENT_CLASS("RtcAgpsNSlotController", RtcAgpsNSlotController, RfxController);
//RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RIL_UNSOL_VIA_GPS_EVENT);

RtcAgpsNSlotController::RtcAgpsNSlotController() :
    m_agpsdAdapter(NULL),
    m_agpsThread(NULL),
    m_activeSlotId(RFX_SLOT_ID_UNKNOWN),
    m_apnState(APN_STATE_UNKNOWN),
    m_networkType(NETWORK_TYPE_UNKNOWN),
    m_serviceState(OUT_OF_SERVICE),
    m_dataEnabledStatus(DATA_ENABLED_ON_GSM) {
    logV(AGPS_TAG, "[RtcAgpsNSlotController]Constructor 0x%p", this);
}

RtcAgpsNSlotController::~RtcAgpsNSlotController() {
    logV(AGPS_TAG, "[RtcAgpsNSlotController]Destructor 0x%p", this);
}

void RtcAgpsNSlotController::onInit() {
    RfxController::onInit();

    logV(AGPS_TAG, "[RtcAgpsNSlotController]onInit(). this=%p!", this);

    RFX_OBJ_CREATE(m_agpsdAdapter, RtcAgpsdAdapter, this);
    m_agpsThread = RtcAgpsThread::create();
    m_agpsThread->run("RtcAgpsThread");

    // register callbacks to get required information
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        registerToHandleUrc(i, URC_LIST, sizeof(URC_LIST)/sizeof(const int));
        registerToHandleRequest(i, AGPS_REQUEST, sizeof(AGPS_REQUEST)/sizeof(const int));
        getStatusManager(i)->registerStatusChangedEx(
                RFX_STATUS_KEY_DATA_CONNECTION,
                RfxStatusChangeCallbackEx(this,
                    &RtcAgpsNSlotController::onDataConnectionChanged));
        getStatusManager(i)->registerStatusChangedEx(
                RFX_STATUS_KEY_SERVICE_STATE,
                RfxStatusChangeCallbackEx(this,
                    &RtcAgpsNSlotController::onServiceStateChanged));
    }
}

void RtcAgpsNSlotController::onDeinit() {
    logV(AGPS_TAG, "[RtcAgpsController]onDeinit(). this=%p!", this);
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        unregisterToHandleUrc(i, URC_LIST, sizeof(URC_LIST)/sizeof(const int));
        unregisterToHandleRequest(i, AGPS_REQUEST, sizeof(AGPS_REQUEST)/sizeof(const int));
        getStatusManager(i)->unRegisterStatusChangedEx(
                RFX_STATUS_KEY_SERVICE_STATE,
                RfxStatusChangeCallbackEx(this,
                    &RtcAgpsNSlotController::onServiceStateChanged));
        getStatusManager(i)->unRegisterStatusChangedEx(
                RFX_STATUS_KEY_DATA_CONNECTION,
                RfxStatusChangeCallbackEx(this,
                    &RtcAgpsNSlotController::onDataConnectionChanged));
    }

    m_agpsThread->requestExit();
    if (m_agpsThread->getLooper() != NULL) {
        m_agpsThread->getLooper()->wake();
    }
    m_agpsThread=NULL;

    RfxController::onDeinit();
}

bool RtcAgpsNSlotController::onHandleUrc(const sp<RfxMessage>& message) {
    bool ret = false;
    int msg_id = message->getId();
    int slot_id = message->getSlotId();

    if (msg_id != RFX_MSG_URC_VIA_GPS_EVENT) {
        logE(AGPS_TAG, "[RtcAgpsNSlotController]onHandleUrc unknown msg=%d !", msg_id);
        return ret;
    }

    int event = ((int*)message->getData()->getData())[0];
    m_activeSlotId = slot_id;
    logD(AGPS_TAG, "[RtcAgpsNSlotController]onHandleUrc(%d) msg=%d, event=%d", slot_id, msg_id, event);

    switch (event) {
    case REQUEST_DATA_CONNECTION:
        if (isDataOff(slot_id)) {
            setConnectionState(0);
        } else {
            setActiveSlotId(slot_id);
            controlApn(EVENT_AGPS_SET_APN, true);
        }
        ret = true;
        break;

    case CLOSE_DATA_CONNECTION:
        controlApn(EVENT_AGPS_DESTROY_APN, true);
        ret = true;
        break;

    default:
        logE(AGPS_TAG, "[RtcAgpsNSlotController]onHandleUrc unkown event!");
        break;
    }
    return ret;
}

bool RtcAgpsNSlotController::onHandleResponse(const sp<RfxMessage>& message){
    logD(AGPS_TAG, "[RtcAgpsNSlotController]response %d, %d",
            message->getPToken(), message->getId());

    switch (message->getId()) {
    case RFX_MSG_REQUEST_AGPS_TCP_CONNIND:
        logD(AGPS_TAG, "[RtcAgpsNSlotController]RFX_MSG_REQUEST_AGPS_TCP_CONNIND response");
        break;
    default:
        logD(AGPS_TAG, "[RtcAgpsNSlotController]unknown response %d", message->getId());
        break;
    }
    return true;
}

void RtcAgpsNSlotController::onDataConnectionChanged(int slotId, RfxStatusKeyEnum key,
                                RfxVariant oldValue, RfxVariant newValue) {
    RFX_ASSERT(RFX_STATUS_KEY_DATA_CONNECTION == key);
    int status = newValue.asInt();
    logD(AGPS_TAG, "[RtcAgpsNSlotController]onDataConnectionChanged. slotId=%d, status=%d",
                slotId, status);
    if (status == DATA_STATE_DISCONNECTED && slotId == m_slot_id) {
        RFX_ASSERT(oldValue.asInt() == DATA_STATE_CONNECTED);
        controlApn(EVENT_AGPS_DESTROY_APN, false);
    }
}

void RtcAgpsNSlotController::onServiceStateChanged(int slotId, RfxStatusKeyEnum key,
                                RfxVariant oldValue,RfxVariant newValue) {
    RFX_ASSERT(key == RFX_STATUS_KEY_SERVICE_STATE);
    RFX_UNUSED(oldValue);
    RfxNwServiceState nwSS = newValue.asServiceState();
    logD(AGPS_TAG, "[RtcAgpsNSlotController]onServiceStateChanged(%d) %s",
                    slotId, nwSS.toString().string());
    if (slotId == getDefaultDataSlotId()) {
        NetworkType type = convertRadioTech(nwSS.getRilDataRadioTech());
        if (type != m_networkType) {
            sendAgpsMessage(EVENT_AGPS_NETWORK_TYPE, type);
        }
        m_networkType = type;

        ServiceState state = convertServiceState(nwSS.getRilVoiceRegState());
        if (state != m_serviceState) {
            sendAgpsMessage(EVENT_AGPS_CDMA_PHONE_STATUS, state);
            m_serviceState = state;
        }
        int status = DATA_ENABLED_ON_CDMA_LTE;
        if (status != m_dataEnabledStatus) {
            sendAgpsMessage(EVENT_AGPS_MOBILE_DATA_STATUS, status);
            m_dataEnabledStatus = status;
        }
    }
}

void RtcAgpsNSlotController::controlApn(int event, bool force) {
    logD(AGPS_TAG, "[RtcAgpsNSlotController]controlApn event=%d, force=%d, m_apnState=%s",
                event, force, stateToString(m_apnState));
    ApnStateEnum newState;
    if (event == EVENT_AGPS_SET_APN) {
        if (m_apnState == APN_STATE_SETUP_BEGIN ||
            m_apnState == APN_STATE_SETUP_DONE) {
            RFX_LOG_W(AGPS_TAG, "[RtcAgpsNSlotController]ignore duplicate reqeuest!");
            return;
        }
        newState = APN_STATE_SETUP_BEGIN;
    } else { /* EVENT_AGPS_DESTROY_APN */
        if (m_apnState == APN_STATE_RESET_BEGIN ||
            m_apnState == APN_STATE_RESET_DONE) {
            RFX_LOG_W(AGPS_TAG, "[RtcAgpsNSlotController]ignore duplicate reqeuest!");
            return;
        } else if (m_apnState == APN_STATE_SETUP_BEGIN && !force) {
            /*
             * for Single PDP senario, destory original connection firstly 
             * while request CTWAP apn, so ignore this case 
             */
            RFX_LOG_W(AGPS_TAG, "[RtcAgpsNSlotController]ignore destory apn request!");
            return;
        }
        newState = APN_STATE_RESET_BEGIN;
    }
    /*
    Parcel *p = new Parcel();
    p->writeInt32(APN_TYPE_CTWAP);
    sp<RtcAgpsMessage> msg = RtcAgpsMessage::obtainMessage(event, p);
    sp<RtcAgpsWorkingThreadHandler> handler
                = new RtcAgpsWorkingThreadHandler(msg);
    handler->sendMessage(m_agpsThread->getLooper());*/
    sendAgpsMessage(event, APN_TYPE_CTWAP);

    m_apnState = newState;
}

void RtcAgpsNSlotController::onHandleAgpsMessage(sp<RtcAgpsMessage> & message) {
    RFX_LOG_D(AGPS_TAG, "[RtcAgpsNSlotController]onHandleAgpsMessage %d",
        message->getId());
    switch (message->getId()) {
        case EVENT_UPDATE_STATE_TO_AGPSD:
        {
            //sendDataEnabledStatus(m_dataEnabledStatus);
            //sendNetworkType(m_networkType);
            //sendC2KMDStatus(m_serviceState);
            break;
        }
        case EVENT_SET_APN_RESULT:
        {
            Parcel *p = message->getParcel();
            p->setDataPosition(0);
            handleApnResult(p->readInt32());
            break;
        }
        default:
            RFX_LOG_E(AGPS_TAG, "[RtcAgpsNSlotController]onHandleAgpsMessage unknown!");
            break;
    }
}

void RtcAgpsNSlotController::handleApnResult(int result) {
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsNSlotController]handleApnResult state=%s, result=%d",
                stateToString(m_apnState), result);
    switch (m_apnState) {
    case APN_STATE_SETUP_BEGIN:
        if (result == 1) {
            m_apnState = APN_STATE_SETUP_DONE;
        }
        onApnSetResult(result);
        break;

    case APN_STATE_RESET_BEGIN:
        if (result == 0) {
            m_apnState = APN_STATE_RESET_DONE;
        } else {
            RFX_LOG_E(AGPS_TAG, "[RtcAgpsManager]Error result");
        }
        break;

    case APN_STATE_SETUP_DONE:
    case APN_STATE_RESET_DONE:
    default:
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsManager]Error state");
        break;
    }
}

void RtcAgpsNSlotController::onApnSetResult(int result) {
    logD(AGPS_TAG, "[RtcAgpsController]onApnSetResult %d", result);
    setConnectionState(result);
    if (0 == result) {
        /* If set data connection fail(eg. timeout), destory apn */
        logD(AGPS_TAG, "[RtcAgpsController]onApnSetResult() Apn set fail and destory !");
        controlApn(EVENT_AGPS_DESTROY_APN, true);
    }
}

RtcAgpsdAdapter *RtcAgpsNSlotController::getAgpsdAdapter(){
    return m_agpsdAdapter;
}


sp<RtcAgpsThread> RtcAgpsNSlotController::getAgpsThread() {
    return m_agpsThread;
}

int RtcAgpsNSlotController::getDefaultDataSlotId() {
    return getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_DEFAULT_DATA_SIM, -1);
}

void RtcAgpsNSlotController::sendAgpsMessage(int msgType, int value) {
    logD(AGPS_TAG, "[RtcAgpsNSlotController]sendAgpsMessage: %s(%d)=%d",
                    msgTypeToString(msgType), msgType, value);
    Parcel *p = new Parcel();
    p->writeInt32(value);
    sp<RtcAgpsMessage> msg = RtcAgpsMessage::obtainMessage(msgType, p);
    sp<RtcAgpsWorkingThreadHandler> handler = new RtcAgpsWorkingThreadHandler(msg);
    if (m_agpsThread->getLooper() == NULL) {
        logD(AGPS_TAG, "[RtcAgpsNSlotController]sendAgpsMessage: ignore message");

        return;
    }
    handler->sendMessage(m_agpsThread->getLooper());
}


void RtcAgpsNSlotController::setActiveSlotId(int slotId) {
    RFX_LOG_D(AGPS_TAG, "[RtcAgpsNSlotController]setActiveSlotId=%d", slotId);
    m_slot_id = slotId;
}

void RtcAgpsNSlotController::setConnectionState(int state) {
    m_activeSlotId = 0;
    int val = state;
    sp<RfxMessage> request = RfxMessage::obtainRequest(m_activeSlotId,
        RFX_MSG_REQUEST_AGPS_TCP_CONNIND, RfxIntsData((void *)&state, sizeof(int)),
        RADIO_TECH_GROUP_C2K);
    RFX_LOG_D(AGPS_TAG, "[RtcAgpsNSlotController]setConnectionState token=%d", request->getToken());
    requestToMcl(request);
}

bool RtcAgpsNSlotController::isDataOff(int slotId) {
    int status = getStatusManager(slotId)->getIntValue(
            RFX_STATUS_KEY_DATA_CONNECTION,  DATA_STATE_DISCONNECTED);
    logD(AGPS_TAG, "[RtcAgpsNSlotController]isDataOff status = %d", status);
    return (status == DATA_STATE_DISCONNECTED) ? true : false;
}

RtcAgpsNSlotController::NetworkType RtcAgpsNSlotController::convertRadioTech(
        int radioTech) {
    radioTech = convertMtkRadioTech(radioTech);
    switch (radioTech) {
        case RADIO_TECH_GPRS:
            return NETWORK_TYPE_GPRS;
        case RADIO_TECH_EDGE:
            return NETWORK_TYPE_EDGE;
        case RADIO_TECH_UMTS:
            return NETWORK_TYPE_UMTS;
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
            return NETWORK_TYPE_CDMA;
        case RADIO_TECH_1xRTT:
            return NETWORK_TYPE_1xRTT;
        case RADIO_TECH_EVDO_0:
            return NETWORK_TYPE_EVDO_0;
        case RADIO_TECH_EVDO_A:
            return NETWORK_TYPE_EVDO_A;
        case RADIO_TECH_HSDPA:
            return NETWORK_TYPE_HSDPA;
        case RADIO_TECH_HSUPA:
            return NETWORK_TYPE_HSUPA;
        case RADIO_TECH_HSPA:
            return NETWORK_TYPE_HSPA;
        case RADIO_TECH_EVDO_B:
            return NETWORK_TYPE_EVDO_B;
        case RADIO_TECH_EHRPD:          //13
            return NETWORK_TYPE_EHRPD;  //14
        case RADIO_TECH_LTE:            //14
        //RIL_RADIO_TECHNOLOTY_LTE_CA(19) instead of 139 from N1
        case 19:
            return NETWORK_TYPE_LTE;    //13
        case RADIO_TECH_HSPAP:
            return NETWORK_TYPE_HSPAP;
        case RADIO_TECH_GSM:
            return NETWORK_TYPE_GSM;
        default:
            return NETWORK_TYPE_UNKNOWN;
    }
}

int RtcAgpsNSlotController::convertMtkRadioTech(int radioTech) {
    if (radioTech == RIL_RADIO_TECHNOLOGY_LTEA) {
        return RADIO_TECH_LTE;
    } else if (radioTech > RIL_RADIO_TECHNOLOGY_MTK &&
               radioTech <= RIL_RADIO_TECHNOLOGY_DC_HSPAP) {
        return RADIO_TECH_HSPAP;
    } else {
        return radioTech;
    }
}


RtcAgpsNSlotController::ServiceState RtcAgpsNSlotController::convertServiceState(
        int regState) {
    if (RfxNwServiceState::isInService(regState)) {
        return IN_SERVICE;
    } else {
        return OUT_OF_SERVICE;
    }
}

char* RtcAgpsNSlotController::stateToString(ApnStateEnum state) {
    switch(state) {
        case APN_STATE_UNKNOWN:     return (char*)"STATE_UNKNOWN";
        case APN_STATE_SETUP_BEGIN: return (char*)"STATE_SETUP_BEGIN";
        case APN_STATE_SETUP_DONE:  return (char*)"STATE_SETUP_DONE";
        case APN_STATE_RESET_BEGIN: return (char*)"STATE_RESET_BEGIN";
        case APN_STATE_RESET_DONE:  return (char*)"STATE_RESET_DONE";
        default :                   return (char*)"Illegal state";
    }
}

char* RtcAgpsNSlotController::msgTypeToString(int msgType) {
    switch(msgType) {
        case EVENT_AGPS_NETWORK_TYPE:       return (char*)"EVENT_AGPS_NETWORK_TYPE";
        case EVENT_AGPS_CDMA_PHONE_STATUS:  return (char*)"EVENT_AGPS_CDMA_PHONE_STATUS";
        case EVENT_AGPS_MOBILE_DATA_STATUS: return (char*)"EVENT_AGPS_MOBILE_DATA_STATUS";
        case EVENT_AGPS_SET_APN:            return (char*)"EVENT_AGPS_SET_APN";
        case EVENT_AGPS_DESTROY_APN:        return (char*)"EVENT_AGPS_DESTROY_APN";
        case EVENT_MTK_RILP_INIT:           return (char*)"EVENT_MTK_RILP_INIT";
        case EVENT_UPDATE_STATE_TO_AGPSD:   return (char*)"EVENT_UPDATE_STATE_TO_AGPSD";
        case EVENT_SET_APN_RESULT:          return (char*)"EVENT_SET_APN_RESULT";
        default :                           return (char*)"Unknown event";
    }
}
