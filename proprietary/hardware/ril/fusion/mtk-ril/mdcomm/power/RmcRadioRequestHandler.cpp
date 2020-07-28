    /* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "RmcRadioRequestHandler.h"
#include "RmcCapabilitySwitchRequestHandler.h"
#include "RmcCapabilitySwitchUtil.h"
#include "RfxChannelManager.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxRilUtils.h"
#include <libmtkrilutils.h>
#include "ratconfig.h"
#include <mtkconfigutils.h>
#include "RfxDispatchThread.h"
#include "hardware/ccci_intf.h"


#define RFX_LOG_TAG "RmcRadioReq"
#define PROPERTY_AIRPLANE_MODE "persist.vendor.radio.airplane.mode.on"
#define PROPERTY_SIM_MODE "persist.vendor.radio.sim.mode"
#define MAX_RETRY_COUNT 20
#define PROPERTY_DECRYPT "vold.decrypt"
#define BOOTUP_REASON_FILE "/proc/aed/reboot-reason"

typedef enum {
    RADIO_MODE_SIM1_ONLY = 1,
    RADIO_MODE_SIM2_ONLY = (RADIO_MODE_SIM1_ONLY << 1),
    RADIO_MODE_SIM3_ONLY = (RADIO_MODE_SIM1_ONLY << 2),
    RADIO_MODE_SIM4_ONLY = (RADIO_MODE_SIM1_ONLY << 3),
} RadioMode;

// modem status reader
static int sMdStatusFd = -1;
static void *mdStatusReaderLoop(void *param);

/*
  * struct md_status_event {
  *     struct timeval time_stamp;
  *     int md_id;
  *     int event_type;
  *     char reason[32];
  * };
  * define at ccci_inft.h
  */
typedef struct md_status_event MdStatusEvent;

// register handler to channel
RFX_IMPLEMENT_OP_PARENT_HANDLER_CLASS(RmcRadioRequestHandler, RIL_CMD_PROXY_9);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData,
        RFX_MSG_REQUEST_RADIO_POWER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData,
        RFX_MSG_REQUEST_COMMAND_BEFORE_RADIO_POWER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData,
        RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData,
        RFX_MSG_REQUEST_ENABLE_MODEM);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_UNSOL_MODEM_RESTART);


RmcRadioRequestHandler::RmcRadioRequestHandler(int slotId, int channelId) :
        RfxBaseHandler (slotId, channelId) {
    logD(RFX_LOG_TAG, "RmcRadioRequestHandler constructor");
    const int request[] = {
        RFX_MSG_REQUEST_RADIO_POWER,
        RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO,
        RFX_MSG_REQUEST_ENABLE_MODEM,
    };
    memset(&mMdStatsuReaderThread, 0, sizeof(pthread_t));

    // SIM switch will reset modem if major sim is unsync with modem
    int old_major_sim = RfxRilUtils::getMajorSim();
    int main_sim = queryMainProtocol();
    int main_channel_sim = RfxChannelManager::getMainChannelSim();
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int muxreport_case = 0;
    logI(RFX_LOG_TAG, "slot_id = %d, main_sim = %d, %d, %d", slotId, main_channel_sim,
            old_major_sim, main_sim);
    if ((old_major_sim != main_sim) || (main_channel_sim != main_sim)) {
        getNonSlotMclStatusManager()->setBoolValue(
                RFX_STATUS_KEY_CAPABILITY_SWITCH_KEEP_EBOOT, true, false, false);
        if (slotId + 1 == main_channel_sim) {
            logI(RFX_LOG_TAG, "SIM switch will reset modem and rild.");
            rfx_property_set("persist.vendor.radio.simswitch",
                    String8::format("%d", main_sim).string());
            rfx_property_get("vendor.ril.mux.report.case", property_value, "0");
            muxreport_case = atoi(property_value);
            logD(RFX_LOG_TAG, "getprop vendor.ril.mux.report.case %d", muxreport_case);
            if (muxreport_case == 1 || muxreport_case == 2 ||
                    muxreport_case == 5 || muxreport_case == 6) {
                atSendCommand("AT+EBOOT=1");
            }
            resetRadio();
        }
        return;
    } else {
        rfx_property_set("persist.vendor.radio.simswitch",
                String8::format("%d", main_sim).string());
    }

    if (getNonSlotMclStatusManager()->getBoolValue(
            RFX_STATUS_KEY_CAPABILITY_SWITCH_KEEP_EBOOT, false)) {
        // SIM switch will reset modem and rild silently and set this key if major sim is unsync
        // with modem when booting up, so we do nothing here.
        logI(RFX_LOG_TAG, "SIM switch will reset modem and rild.");
        return;
    }
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    sp<RfxAtResponse> response;
    response = atSendCommand("ATE0Q0V1");
    response = atSendCommand("AT+CMEE=1");
    /*  Enable get +CIEV:7 URC to receive SMS SIM Storage Status*/
    // The command MUST send to modem before AT+EFUN=0
    response = atSendCommand("AT+CMER=1,0,0,2,0");

    // External SIM [Start]
    if (RfxRilUtils::isExternalSimSupport()) {
        initVsimConfiguration();
        queryModemVsimCapability();
        requestSetAkaSim();
        requestSwitchExternalSim();
    }
    // External SIM [End]

    // Enable the URC +ICCID
    response = atSendCommand("AT+ICCID=1");
    if (response->getError() < 0 || 0 == response->getSuccess()) {
        logE(RFX_LOG_TAG, "[SIM] MD does not support +ICCID");
        getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_MD_ICCID_SUPPORT, false);
    } else {
        getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_MD_ICCID_SUPPORT, true);
    }

    // Enable +ESIMS URC
    response = atSendCommand("AT+ESIMS=1");

    // Update RAT Before Radio on [Start]
    sendERAT();
    // Update RAT Before Radio on [End]

    // Enable/Disable SMS
    if (!RfxRilUtils::isSmsSupport()) {
        // Disable SMS
        response = atSendCommand("AT+ECFGSET=\"disable_sms\", \"1\"");
    } else {
        // Enable SMS
        response = atSendCommand("AT+ECFGSET=\"disable_sms\", \"0\"");
    }

    /// M: GSMA TS.27 13.3.7 @{
    if (RfxRilUtils::isUiccCLFSupport()) {
        /* TS26_NFC_REQ_166:
         * Send Terminal Capability command to the UICC indicating that the
         * UICC-CLF interface (SWP) is supported as per ETSI TS 102 221. It
         * should send before sending AT+ESIMS and AT+EFUN.
         */
        response = atSendCommand("AT+ESBP=5,\"SBP_TERMINAL_CAPABILITY_FLEX\",1");
    }
    /// M: GSMA TS.27 13.3.7 @}

    // POWER_ON_WITHOUT_ESIMS Support, 0:disable, 1:enable
    char feature[] = "POWER_ON_WITHOUT_ESIMS";
    int support = getFeatureVersion(feature);
    if (support != 1) {
        response = atSendCommand("AT+ESIMS");
    }

    /* HEX character set */
    response = atSendCommand("AT+CSCS=\"UCS2\"");
    updateDataCallPrefer();

    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    if (mainSlotId == slotId) {
        resetIpoProperty();
        enableMdProtocol();
        enableSilentReboot();
        updateSupportDSBP();
        response = atSendCommand("AT+EPOC");
        response = atSendCommand("AT+EFUN=0");

        initMdStatusReader();
        // MTK-START: eSIM
        response = atSendCommand("AT+ESLOTSINFO=1");
        // MTK-END
    }
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
}

RmcRadioRequestHandler::~RmcRadioRequestHandler() {
}

void RmcRadioRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(RFX_LOG_TAG, "onHandleRequest: %s(%d)", idToString(id), id);
    switch(id) {
        case RFX_MSG_REQUEST_RADIO_POWER:
        case RFX_MSG_REQUEST_ENABLE_MODEM:
            requestRadioPower(msg);
            break;
        case RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO:
            bootupSetRadioPower(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcRadioRequestHandler::onHandleTimer() {
    // do something
}

void RmcRadioRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(RFX_LOG_TAG, "onHandleEvent: %d", id);
    switch(id) {
        default:
            logE(RFX_LOG_TAG, "should not be here");
            break;
    }
}

void RmcRadioRequestHandler::requestRadioPower(const sp<RfxMclMessage> &msg) {
    sp<RfxAtResponse> response;
    RIL_RadioState curState = (RIL_RadioState) getMclStatusManager()->getIntValue(
            RFX_STATUS_KEY_RADIO_STATE, 0);
    RIL_RadioState newState;
    bool notSent = false;
    AT_CME_Error cause;
    RIL_Errno errNo = RIL_E_SUCCESS;
    int isRadioOn;
    sp<RfxMclMessage> resMsg;

    int onOff = ((int *)msg->getData()->getData())[0];
    int caller =  msg->getData()->getDataLength()/sizeof(int) > 1
            ? ((int *)msg->getData()->getData())[1] : -1;
    int slotId = msg->getSlotId();

    int originMode = 0;
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        if (i != slotId) {
            originMode |=getMclStatusManager(i)->getIntValue(
                    RFX_STATUS_KEY_RADIO_POWER_MSIM_MODE, 0) << i;
        } else {
            originMode |= onOff << i;
        }
    }
    logD(RFX_LOG_TAG, "original mode=%d", originMode);

    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    int targetMode = 0;
    int slotMode = 0;
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        slotMode = (originMode & (1 << i)) >> i;
        switch(mainSlotId) {
            case RFX_SLOT_ID_0:
                slotMode = slotMode << i;
                break;
            case RFX_SLOT_ID_1:
                if (RFX_SLOT_ID_0 == i) {
                    slotMode = slotMode << 1;
                } else if (RFX_SLOT_ID_2 == i) {
                    slotMode = slotMode << 2;
                } else if (RFX_SLOT_ID_3 == i) {
                    slotMode = slotMode << 3;
                }
                break;
            case RFX_SLOT_ID_2:
                if (RFX_SLOT_ID_0 == i) {
                    slotMode = slotMode << 1;
                } else if (RFX_SLOT_ID_1 == i) {
                    slotMode = slotMode << 2;
                } else if (RFX_SLOT_ID_3 == i) {
                    slotMode = slotMode << 3;
                }
                break;
            case RFX_SLOT_ID_3:
                if (RFX_SLOT_ID_0 == i) {
                    slotMode = slotMode << 1;
                } else if (RFX_SLOT_ID_1 == i) {
                    slotMode = slotMode << 2;
                } else if (RFX_SLOT_ID_2 == i) {
                    slotMode = slotMode << 3;
                }
                break;
            default:
                break;
        }
        targetMode |= slotMode;
        slotMode = 0;
    }

    logI(RFX_LOG_TAG, "requestRadioPower, desired power = %d, target mode = %d, caller: %d",
            onOff, targetMode, caller);

    /*
    * RFOFF_CAUSE_UNSPECIFIED = 0,
    * RFOFF_CAUSE_DUPLEX_MODE = 1,
    * RFOFF_CAUSE_POWER_OFF = 2,
    * RFOFF_CAUSE_SIM_SWITCH = 3,
    */
    if (targetMode == 0 /*&& !s_md_off*/) {
        if (caller >= 0) {
            response = atSendCommand(String8::format("AT+EFUN=0,%d", caller));
        } else {
            response = atSendCommand("AT+EFUN=0");
        }
        newState = RADIO_STATE_OFF;
    } else if (targetMode > 0) {
        if (!onOff) {
            response = atSendCommand(String8().format(("AT+EFUN=%d,%d"), targetMode, caller));
        } else {
            response = atSendCommand(String8().format(("AT+EFUN=%d"), targetMode));
        }
        newState = (onOff==1)? RADIO_STATE_ON: RADIO_STATE_OFF;
    } else {
        newState = curState;
        notSent = true;
    }

    // error handle
    if (!notSent && response->getSuccess() == 0) {
        cause = response->atGetCmeError();
        errNo = RIL_E_GENERIC_FAILURE;
        newState = curState;
        logD(RFX_LOG_TAG, "Get error cause: %d",cause);

        // always do EFUN retry when get response error
        errNo = RIL_E_OEM_ERROR_1;
    }

    // only update radio state when EFUN sended successfully
    if (errNo == RIL_E_SUCCESS) {
        getMclStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_RADIO_POWER_MSIM_MODE, onOff);
        getMclStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_RADIO_STATE, newState);
    }
    resMsg = RfxMclMessage::obtainResponse(msg->getId(), errNo, RfxVoidData(), msg,
            false);
    responseToTelCore(resMsg);
    return;
}

/** returns 1 if on, 0 if off, and -1 on error */
int RmcRadioRequestHandler::isModemRadioOn() {
    sp<RfxAtResponse> response;
    RfxAtLine *atLine = NULL;
    int err, ret;

    response = atSendCommandSingleline("AT+CFUN?", "+CFUN:");
    if (response->getError() < 0 || response->getSuccess() == 0) {
        // assume radio is off
        goto error;
    }

    atLine = response->getIntermediates();
    atLine->atTokStart(&err);
    if (err < 0) goto error;

    ret = atLine->atTokNextint(&err);
    if (err < 0) goto error;

    ret = (ret == 4 || ret == 0) ? 0 :    // phone off
          (ret == 1) ? 1 :              // phone on
          -1;                           // invalid value

    return ret;

error:
    return -1;
}

void RmcRadioRequestHandler::enableMdProtocol() {
    sp<RfxAtResponse> response;

    switch (RfxRilUtils::rfxGetSimCount()) {
        case 1:
            response = atSendCommand("AT+ESADM=1");
            break;
        case 2:
            response = atSendCommand("AT+ESADM=3");
            break;
        case 3:
            response = atSendCommand("AT+ESADM=7");
            break;
        case 4:
            response = atSendCommand("AT+ESADM=15");
            break;
    }
}

void RmcRadioRequestHandler::enableSilentReboot() {
    sp<RfxAtResponse> response;
    int muxreport_case = 0;
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int auto_unlock_pin = -1;
    int isSilentReboot = -1;
    int isDisableEboot = 0;

    rfx_property_get("vendor.ril.disable.eboot", property_value, "0");
    isDisableEboot = atoi(property_value);

    if (isDisableEboot == 1) {
        isSilentReboot = 0;
    } else {
        rfx_property_get("vendor.ril.mux.report.case", property_value, "0");
        muxreport_case = atoi(property_value);
        logD(RFX_LOG_TAG, "getprop vendor.ril.mux.report.case %d", muxreport_case);
        switch (muxreport_case) {
            case 0:
                isSilentReboot = 0;
                break;
            case 1:
            case 2:
            case 5:
            case 6:
                isSilentReboot = 1;
                break;
        }
    }
    rfx_property_set("vendor.ril.disable.eboot", "0");
    rfx_property_set("vendor.ril.mux.report.case", "0");

    // eboot property will be set to 0 when ipo shutdown, no needs to silent reboot in this case
    // ebbot property will be set to 1 when flight mode turn on, and 3g switch reset modem
    rfx_property_get("vendor.gsm.ril.eboot", property_value, "0");
    auto_unlock_pin = atoi(property_value);
    logD(RFX_LOG_TAG, "getprop vendor.gsm.ril.eboot %d", auto_unlock_pin);
    isSilentReboot |= auto_unlock_pin;

    if (isSilentReboot != 1) {
        if (isNormalBootUp() == 0) {
            isSilentReboot = 1;
        } else {
            isSilentReboot = 0;
        }
    }

    /********************************
     * AT+EBOOT=<mode>
     *
     * 0: Normal boot up
     * 1: Silent boot up (Verify PIN by modem internally)
     *********************************/
    switch (isSilentReboot) {
        case 0:
            response = atSendCommand("AT+EBOOT=0");
            break;
        case 1:
            response = atSendCommand("AT+EBOOT=1");
            break;
        default:
            response = atSendCommand("AT+EBOOT=0");
            break;
    }
    rfx_property_set("vendor.gsm.ril.eboot", "0");
}

void RmcRadioRequestHandler::resetIpoProperty() {
    rfx_property_set((char *) "vendor.ril.ipo.radiooff", "0");
}

void RmcRadioRequestHandler::updateSupportDSBP() {
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get((char *) "persist.vendor.radio.mtk_dsbp_support", prop, "0");
    atSendCommand(String8::format("AT+EDSBP=%s", prop));
}

void RmcRadioRequestHandler::updateDataCallPrefer() {
    // set data/call prefer
    // 0 : call prefer
    // 1 : data prefer
    char gprsPrefer[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.radio.gprs.prefer", gprsPrefer, "0");
    if ((atoi(gprsPrefer) == 0)) {
        // call prefer
        atSendCommand("AT+EGTP=1");
        atSendCommand("AT+EMPPCH=1");
    } else {
        // data prefer
        atSendCommand("AT+EGTP=0");
        atSendCommand("AT+EMPPCH=0");
    }
}

// External SIM [Start]
int RmcRadioRequestHandler::queryModemVsimCapability()
{
    sp<RfxAtResponse> p_response;
    int err, temp_cap = 0;
    String8 cmd("");
    RfxAtLine *line = NULL;
    char *capability = NULL;

    /**
     * Query if the VSIM has been enabled
     * AT+EAPVSIM?
     * success: +EAPVSIM: <capability and status>
     *          APVSIM Capability or Status Query (bit mask)
     *          0x01 : APVSIM ON/OFF status
     *          0x02 : APVSIM Support enable/disable via Hot Swap Mechanism
     * fail: ERROR
     *
     */

    cmd.append(String8::format("AT+EAPVSIM?"));
    p_response = atSendCommandSingleline(cmd, "+EAPVSIM:");
    cmd.clear();

    if (p_response == NULL || p_response->getError() < 0) {
        logE(RFX_LOG_TAG, "queryModemVsimCapability fail");
         goto done;
    }

    switch (p_response->atGetCmeError()) {
        logD(RFX_LOG_TAG, "p_response = %d /n", p_response->atGetCmeError());
        case CME_SUCCESS:
            if (p_response->getError() < 0 || 0 == p_response->getSuccess()) {
                goto done;
            } else {
                line = p_response->getIntermediates();

                line->atTokStart(&err);
                if(err < 0) goto done;

                temp_cap = line->atTokNextint(&err);
                if(err < 0) goto done;
            }
            break;
        case CME_UNKNOWN:
            logD(RFX_LOG_TAG, "queryModemVsimCapability p_response: CME_UNKNOWN");
            break;
        default:
            logD(RFX_LOG_TAG, "queryModemVsimCapability fail");
    }

done:
    logD(RFX_LOG_TAG, "queryModemVsimCapability done, capability: %d", temp_cap);
    asprintf(&capability, "%d", temp_cap);
    setExternalSimProperty(m_slot_id, (char*)"vendor.gsm.modem.vsim.capability", capability);
    free(capability);

    return temp_cap;
}

void RmcRadioRequestHandler::requestSetAkaSim() {
    sp<RfxAtResponse> p_response;
    int err, ret;
    String8 cmd("");
    char vsim_enabled_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char vsim_inserted_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char persist_vsim_inserted_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char rsim_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char akasim_prop[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("vendor.gsm.prefered.rsim.slot", rsim_prop, "-1");
    rfx_property_get("vendor.gsm.prefered.aka.sim.slot", akasim_prop, "-1");

    if (atoi(rsim_prop) == -1 || atoi(akasim_prop) != m_slot_id) {
        logE(RFX_LOG_TAG, "[VSIM] requestSetAkaSim rsim=%d, aka-sim=%d",
                atoi(rsim_prop), atoi(akasim_prop));
        return;
    }

    /* If rsim enabled, then should set aka sim protocol in case of it haven't be disabled. */
    if (atoi(akasim_prop) >= 0) {
        cmd.append(String8::format("AT+ERSIMAKA=1"));

        p_response = atSendCommand(cmd);
        cmd.clear();

        if (p_response->getError() < 0) {
            logE(RFX_LOG_TAG, "[VSIM] requestSetAkaSim Fail");
            ret = RIL_E_GENERIC_FAILURE;
        }

        if (0 == p_response->getSuccess()) {
            logE(RFX_LOG_TAG, "[VSIM] requestSetAkaSim p_response = %d /n", p_response->atGetCmeError());
            switch (p_response->atGetCmeError()) {
                default:
                    ret = RIL_E_GENERIC_FAILURE;
                    break;
            }
        }
    }
}

void RmcRadioRequestHandler::requestSwitchExternalSim() {
    sp<RfxAtResponse> p_response;
    int err, ret;
    String8 cmd("");
    char vsim_enabled_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char vsim_inserted_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char persist_vsim_inserted_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char rsim_prop[RFX_PROPERTY_VALUE_MAX] = {0};

    getMSimProperty(m_slot_id, (char*)"vendor.gsm.external.sim.enabled", vsim_enabled_prop);
    getMSimProperty(m_slot_id, (char*)"vendor.gsm.external.sim.inserted", vsim_inserted_prop);
    rfx_property_get("vendor.gsm.prefered.rsim.slot", rsim_prop, "-1");

    if (RfxRilUtils::isPersistExternalSimDisabled()) {
        rfx_property_set("persist.vendor.radio.external.sim", "");
    } else {
        getMSimProperty(m_slot_id, (char*)"persist.vendor.radio.external.sim",
                persist_vsim_inserted_prop);
    }

    if (atoi(persist_vsim_inserted_prop) > 0) {
        logD(RFX_LOG_TAG, "[VSIM] persist.vendor.radio.external.sim is 1.");
    }

    /* When to set true and when to set false ? */
    if (atoi(vsim_enabled_prop) > 0 && atoi(vsim_inserted_prop) > 0) {
        // Mean VSIM enabled and modem reset only rild reset case,
        // should recover to previous status
        if (atoi(rsim_prop) >= 0 && atoi(rsim_prop) == m_slot_id) {
            cmd.append(String8::format("AT+ERSIM"));
        } else if (RfxRilUtils::isExternalSimOnlySlot(m_slot_id) > 0) {
            cmd.append(String8::format("AT+EAPVSIM=1,1"));
        } else {
            cmd.append(String8::format("AT+EAPVSIM=1"));
        }
    } else {
        // Might reboot or VSIM did't be enabled
        if (atoi(persist_vsim_inserted_prop) > 0 && !(RfxRilUtils::isNonDsdaRemoteSupport())) {
            // Case 1. Device reboot and VSIM enabled before reboot. Keep slot to VSIM only.
            //cmd.append(String8::format("AT+EAPVSIM=1,0"));
            // Modem didn't support AT+EAPVSIM=1,0 with VSIM hot swap. We sent AT+EAPVSIM=1,1 to
            // trigger modem send +ERSAIND: 1 and sent AT+ERSA=3 to plug out VSIM to achieve the
            // same goal.
            cmd.append(String8::format("AT+EAPVSIM=1,1"));
            logD(RFX_LOG_TAG,
                    "[VSIM] Case 1. Device reboot and VSIM enabled before reboot. Keep slot to VSIM only.");
        } else if (RfxRilUtils::isExternalSimOnlySlot(m_slot_id) > 0
                && !(RfxRilUtils::isNonDsdaRemoteSupport())) {
            // Case 2. Device reboot and the slot has been set to VSIM only in configuration.
            cmd.append(String8::format("AT+EAPVSIM=1,0"));
            logD(RFX_LOG_TAG,
                    "[VSIM] Case 2. Device reboot and the slot has been set to VSIM only in configuration.");
        } else {
            // Case 3. Others. VSIM disabled and it is not VSIM only protocol.
            cmd.append(String8::format("AT+EAPVSIM=0"));
        }
    }
    p_response = atSendCommand(cmd);
    cmd.clear();

    if (p_response->getError() < 0) {
        logE(RFX_LOG_TAG, "[VSIM] requestSwitchExternalSim Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    if (0 == p_response->getSuccess()) {
        switch (p_response->atGetCmeError()) {
            logD(RFX_LOG_TAG, "[VSIM] requestSwitchExternalSim p_response = %d /n", p_response->atGetCmeError());
            default:
                ret = RIL_E_GENERIC_FAILURE;
                goto done;
        }
    }

done:
    logE(RFX_LOG_TAG, "[VSIM] requestSwitchExternalSim Done");
}
#define VSIM_NO_RESPONSE_TIMEOUT_DURATION "13"
void RmcRadioRequestHandler::initVsimConfiguration() {
    char persist[RFX_PROPERTY_VALUE_MAX] = { 0 };
    getMSimProperty(m_slot_id, (char*)"persist.vendor.radio.external.sim", persist);

    if (strcmp("1", persist) == 0) {
        setExternalSimProperty(m_slot_id, (char*)"vendor.gsm.external.sim.enabled", (char*)"1");
    }
    setExternalSimProperty(m_slot_id, (char*)"vendor.gsm.external.sim.timeout",
                    (char*)VSIM_NO_RESPONSE_TIMEOUT_DURATION);
}

// External SIM [End]

void RmcRadioRequestHandler::bootupSetRadioPower(const sp<RfxMclMessage> &msg) {
    logD(RFX_LOG_TAG, "bootupSetRadioPower");
    RIL_RadioState newState;
    RIL_Errno errNo = RIL_E_GENERIC_FAILURE;

    char filghtMode[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_AIRPLANE_MODE, filghtMode, "false");
    if (strcmp("false", filghtMode) || isUnderCryptKeeper()) {
        logE(RFX_LOG_TAG, "under airplane mode or in cryptKeeper, return");
        for(int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            getMclStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
        }
        errNo = RIL_E_SUCCESS;
        sp<RfxMclMessage> resMsg = RfxMclMessage::obtainResponse(msg->getId(), errNo, RfxVoidData(),
                msg, false);
        responseToTelCore(resMsg);
        return;
    }

    char simMode_prop[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_SIM_MODE, simMode_prop,
            (String8::format("%d", getSimCount())).string());
    int simMode = atoi(simMode_prop);
    int originMode = simMode & ((int *)msg->getData()->getData())[0];

    // EFUN affected by SIM switch
    int mainSlotId = RfxRilUtils::getMajorSim() - 1;
    int slotMode = 0;
    int targetMode = 0;
    logI(RFX_LOG_TAG, "bootupSetRadioPower, origin mode = %d", originMode);
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        slotMode = (originMode & (1 << i)) >> i;
        switch(mainSlotId) {
            case RFX_SLOT_ID_0:
                slotMode = slotMode << i;
                break;
            case RFX_SLOT_ID_1:
                if (RFX_SLOT_ID_0 == i) {
                    slotMode = slotMode << 1;
                } else if (RFX_SLOT_ID_2 == i) {
                    slotMode = slotMode << 2;
                } else if (RFX_SLOT_ID_3 == i) {
                    slotMode = slotMode << 3;
                }
                break;
            case RFX_SLOT_ID_2:
                if (RFX_SLOT_ID_0 == i) {
                    slotMode = slotMode << 1;
                } else if (RFX_SLOT_ID_1 == i) {
                    slotMode = slotMode << 2;
                } else if (RFX_SLOT_ID_3 == i) {
                    slotMode = slotMode << 3;
                }
                break;
            case RFX_SLOT_ID_3:
                if (RFX_SLOT_ID_0 == i) {
                    slotMode = slotMode << 1;
                } else if (RFX_SLOT_ID_1 == i) {
                    slotMode = slotMode << 2;
                } else if (RFX_SLOT_ID_2 == i) {
                    slotMode = slotMode << 3;
                }
                break;
            default:
                break;
        }
        targetMode |= slotMode;
        slotMode = 0;
    }
    logI(RFX_LOG_TAG, "bootupSetRadioPower, target mode = %d", targetMode);

    if (targetMode == 0) {
        for(int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            getMclStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
        }
        errNo = RIL_E_SUCCESS;
    } else {
        sp<RfxAtResponse> response = atSendCommand(String8().format(("AT+EFUN=%d"), targetMode));
        // add retry mechanism
        int retryCount = 0;
        while (response->getSuccess() == 0 && retryCount < MAX_RETRY_COUNT) {
            response = atSendCommand(String8().format(("AT+EFUN=%d"), targetMode));
            retryCount++;
            usleep(500*1000);
        }
        if (response->getSuccess() == 1) {
            errNo = RIL_E_SUCCESS;
            for(int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
                if (originMode & (RADIO_MODE_SIM1_ONLY << i)) {
                    getMclStatusManager(i)->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, true);
                }
                getMclStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_STATE,
                        (originMode & (RADIO_MODE_SIM1_ONLY << i)) ? RADIO_STATE_ON :
                         RADIO_STATE_OFF);
                getMclStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_POWER_MSIM_MODE,
                        (originMode & (RADIO_MODE_SIM1_ONLY << i)) ? 1 : 0);
            }
        }
    }
    sp<RfxMclMessage> resMsg = RfxMclMessage::obtainResponse(msg->getId(), errNo, RfxVoidData(),
            msg, false);
    responseToTelCore(resMsg);
}

void RmcRadioRequestHandler::sendERAT() {
    int raf = getMclStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_SLOT_FIXED_CAPABILITY, 0);
    int rat = -1;
    raf = RmcCapabilitySwitchUtil::getAdjustedRaf(raf);
    switch (raf) {
        case RAF_GSM_GROUP:
            rat = 0;
            break;
    }
    if (rat != -1) {
        logD(RFX_LOG_TAG, "sendERAT: raf=%d, rat=%d", raf, rat);
        atSendCommand(String8::format("AT+ERAT=%d", rat));
    }
}

void RmcRadioRequestHandler::initMdStatusReader() {
    logD(RFX_LOG_TAG, "initMdStatusReader");
    mMdStatsuReaderThread = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int result = pthread_create(&mMdStatsuReaderThread, &attr, mdStatusReaderLoop, NULL);
    if (result != 0) {
        logE("initMdStatusReader: Failed to create dispatch thread: %s", strerror(result));
        return;
    }
}

void *RmcRadioRequestHandler::mdStatusReaderLoop(void* params __unused) {
    int count = -1;

    RfxRilUtils::printLog(INFO, String8::format("%s", RFX_LOG_TAG),
            String8::format("mdStatusReaderLoop - md status working thread"), RFX_SLOT_ID_0);

    /* /dev/ccci_mdx_sta: contains all modem status
    *  /dev/ccci_md1_sta: contains gsm modem status
    *  /dev/ccci_md3_sta: contains c2k modem status
    */
    sMdStatusFd = open("/dev/ccci_mdx_sta", O_RDWR);
    RfxRilUtils::printLog(INFO, String8::format("%s", RFX_LOG_TAG),
            String8::format(
            "[RmcRadioRequestHandler] mdStatusReaderLoop: fd = %d", sMdStatusFd),
            RFX_SLOT_ID_0);

    if (sMdStatusFd < 0) {
        RfxRilUtils::printLog(ERROR, String8::format("%s", RFX_LOG_TAG),
                String8::format("ccci_mdx_sta port is not available"), RFX_SLOT_ID_0);
        return NULL;
    }

    getResetLock();
    MdStatusEvent *buffer = (MdStatusEvent *) calloc(1, sizeof(MdStatusEvent));
    if (buffer == NULL) {
        RfxRilUtils::printLog(ERROR, String8::format("%s", RFX_LOG_TAG),
                String8::format("OOM"), RFX_SLOT_ID_0);
        releaseResetLock();
        return NULL;
    }

    do {
        count = read(sMdStatusFd, buffer, sizeof(MdStatusEvent));
        RfxRilUtils::printLog(INFO, String8::format("%s", RFX_LOG_TAG),
                String8::format(
                "mdStatusReaderLoop: count: %d modem id: %d, event type: %d, reason: %s",
                count, buffer->md_id, buffer->event_type, buffer->reason), RFX_SLOT_ID_0);

        if (MD_STA_EV_RESET_REQUEST == buffer->event_type
                || MD_STA_EV_LEAVE_FLIGHT_REQUEST == buffer->event_type
                || MD_STA_EV_LEAVE_FLIGHT_E_REQUEST == buffer->event_type) {
            RfxRilUtils::printLog(ERROR, String8::format("%s", RFX_LOG_TAG),
                    String8::format("mdStatusReaderLoop: RESET/STOP/ENTER_FLIGHT, break"),
                    RFX_SLOT_ID_0);
            break;
        }
    } while (1);

    // send to slot1
    if (MD_STA_EV_RESET_REQUEST == buffer->event_type) {
        RfxRilUtils::printLog(INFO, String8::format("%s", RFX_LOG_TAG),
                String8::format(
                "mdStatusReaderLoop: update status to framework"), RFX_SLOT_ID_0);
        RfxMclStatusManager::getMclStatusManager(RFX_SLOT_ID_UNKNOWN)
                ->setBoolValue(RFX_STATUS_KEY_MODEM_RESET, true);
        sendUrcToTelCor(buffer->reason);
        sleep(1);
    }
    releaseResetLock();
    free(buffer);
    close(sMdStatusFd);

    return NULL;
}

// get reset lock at bootup
void RmcRadioRequestHandler::getResetLock() {
    RfxRilUtils::printLog(DEBUG, String8::format("%s", RFX_LOG_TAG),
            String8::format("getResetLock"), RFX_SLOT_ID_0);
    int ret = ioctl(sMdStatusFd, CCCI_IOC_HOLD_RST_LOCK, -1);
}

// release reset lock at following case
// 1. Leave flight mode
// 2. Receive MD_STA_EV_RESET_REQUEST
// 3. Receive MD_STA_EV_STOP_REQUEST
void RmcRadioRequestHandler::releaseResetLock() {
    RfxRilUtils::printLog(DEBUG, String8::format("%s", RFX_LOG_TAG),
            String8::format("releaseResetLock"), RFX_SLOT_ID_0);

    int ret = ioctl(sMdStatusFd, CCCI_IOC_FREE_RST_LOCK, -1);
}

int RmcRadioRequestHandler::isNormalBootUp() {
    int ret = 1;
    int len = 0, reason = 0, fip = 0, exp_type = 0;
    char line[RFX_PROPERTY_VALUE_MAX] = {0};
    FILE *fp = fopen(BOOTUP_REASON_FILE, "r");

    if (fp == NULL) {
        logE(RFX_LOG_TAG, "isNormalBootUp(): file not existed.");
        return ret;
    }

    if (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "WDT status: %d fiq step: %d  exception type: %d",
                &reason, &fip, &exp_type);
        logD(RFX_LOG_TAG, "isNormalBootUp(): line: %s reason: %d, fip: %d, exp_type: %d",
                line, reason, fip, exp_type);
    }

    if (reason == 0 || (reason == 2 && fip == 0 && exp_type == 0)) {
        ret = 1;
    } else {
        ret = 0;
    }

    logD(RFX_LOG_TAG, "isNormalBootUp(): ret = %d", ret);

    fclose(fp);

    return ret;
}

void RmcRadioRequestHandler::sendUrcToTelCor(char* buffer) {
    sp<RfxMclMessage> urc;
    for (int i=0; i<getSimCount(); i++) {
        RfxRilUtils::printLog(DEBUG, String8::format("%s", RFX_LOG_TAG),
                String8::format(
                "[RmcRadioRequestHandler] send modem reset urc"), i);
        urc = RfxMclMessage::obtainUrc(
                RFX_MSG_UNSOL_MODEM_RESTART, i,
                RfxStringData((void*)buffer, strlen(buffer)));

        RfxDispatchThread::enqueueUrcMessage(urc);
    }
}

/*
 *A special paragraph, not to trun off modem power under cryptkeeper
 */
bool RmcRadioRequestHandler::isUnderCryptKeeper() {
    char vold_decrypt[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_DECRYPT, vold_decrypt, "false");

    if (!strcmp("trigger_restart_min_framework", vold_decrypt)) {
        logI(RFX_LOG_TAG, "UnderCryptKeeper");
        return true;
    }
    return false;
}

/*
 * Query modem main protocol sim id
 */
int RmcRadioRequestHandler::queryMainProtocol() {
    sp<RfxAtResponse> p_response = NULL;
    int err;
    RfxAtLine *line;
    int ret = 0;
    int main_sim = 1;

    logD(RFX_LOG_TAG, "queryMainProtocol");
    p_response = atSendCommandSingleline("AT+ES3G?", "+ES3G:");
    do {
        if (p_response == NULL || p_response->getSuccess() == 0) {
            logE(RFX_LOG_TAG, "AT+ES3G return ERROR");
            break;
        }

        line = p_response->getIntermediates();

        line->atTokStart(&err);
        if (err < 0) {
            break;
        }
        ret = line->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        // Gemini+ , +ES3G response 1: SIM1 , 2: SIM2 , 4:SIM3 ,8: SIM4
        // For SIM3 and SIM4 we convert to 3 and 4
        if (ret == 2) {
            main_sim = 2;
        } else if (ret == 4) {
            main_sim = 3;
        } else if (ret == 8) {
            main_sim = 4;
        }
    } while (0);
    logD(RFX_LOG_TAG, "queryMainProtocol, ret=%d, main_sim=%d", ret, main_sim);
    return main_sim;
}

/*
 * reboot modem
 */
void RmcRadioRequestHandler::resetRadio() {
    logD(RFX_LOG_TAG, "start to reset radio");
    atSendCommand("AT+EFUN=0");
    atSendCommand("AT+ECUSD=2,2");
    atSendCommand("AT+EMDT=0");
    atSendCommand("AT+EPOF");
    RfxRilUtils::triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);
    //power on modem
    RfxRilUtils::triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);
}

