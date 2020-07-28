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

#include "RmcSuppServUssdBaseHandler.h"
#include "RmcSuppServUssdDomainSelector.h"
#include "RmcSuppServUssdHandler.h"
#include "RmcSuppServRequestHandler.h"
#include "rfx_properties.h"
#include "RfxMessageId.h"
#include "GsmUtil.h"
#include "SSUtil.h"
#include "SSConfig.h"

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include <mtk_properties.h>
#include <mtkconfigutils.h>
#include <string.h>
#include <dlfcn.h>

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData,  RfxVoidData, RFX_MSG_REQUEST_SEND_USSD);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxVoidData, RFX_MSG_REQUEST_CANCEL_USSD);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_CLIR);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxCallForwardInfoData, RfxVoidData, RFX_MSG_REQUEST_SET_CALL_FORWARD);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxCallForwardInfoExData, RfxVoidData, RFX_MSG_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_CALL_WAITING);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_CHANGE_BARRING_PASSWORD);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxIntsData, RFX_MSG_REQUEST_QUERY_CLIP);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_CLIP);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_SUPP_SVC_NOTIFICATION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxIntsData, RFX_MSG_REQUEST_GET_COLP);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_COLP);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxIntsData, RFX_MSG_REQUEST_GET_COLR);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_COLR);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxIntsData, RFX_MSG_REQUEST_SEND_CNAP);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData,  RfxVoidData, RFX_MSG_REQUEST_SEND_USSI);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxVoidData, RFX_MSG_REQUEST_CANCEL_USSI);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData,    RfxVoidData, RFX_MSG_REQUEST_SET_CALL_WAITING_ATCI);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxVoidData, RFX_MSG_REQUEST_RESET_SUPP_SERV);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SETUP_XCAP_USER_AGENT_STRING);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_SS_PROPERTY);

RFX_REGISTER_DATA_TO_EVENT_ID(RfxStringsData, RFX_MSG_EVENT_SET_XCAP_CONFIG);

static const int requests[] = {
    RFX_MSG_REQUEST_SEND_USSD,
    RFX_MSG_REQUEST_CANCEL_USSD,
    RFX_MSG_REQUEST_SET_CLIR,
    RFX_MSG_REQUEST_SET_CALL_FORWARD,
    RFX_MSG_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT,
    RFX_MSG_REQUEST_SET_CALL_WAITING,
    RFX_MSG_REQUEST_CHANGE_BARRING_PASSWORD,
    RFX_MSG_REQUEST_QUERY_CLIP,
    RFX_MSG_REQUEST_SET_CLIP,
    RFX_MSG_REQUEST_SET_SUPP_SVC_NOTIFICATION,
    RFX_MSG_REQUEST_GET_COLP,
    RFX_MSG_REQUEST_SET_COLP,
    RFX_MSG_REQUEST_GET_COLR,
    RFX_MSG_REQUEST_SET_COLR,
    RFX_MSG_REQUEST_SEND_CNAP,
    RFX_MSG_REQUEST_SEND_USSI,
    RFX_MSG_REQUEST_CANCEL_USSI,
    RFX_MSG_REQUEST_SET_CALL_WAITING_ATCI,
    RFX_MSG_REQUEST_RESET_SUPP_SERV,
    RFX_MSG_REQUEST_SETUP_XCAP_USER_AGENT_STRING,
    RFX_MSG_REQUEST_SET_SS_PROPERTY
};

static const int event[] = {
    RFX_MSG_EVENT_SET_XCAP_CONFIG
};

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcSuppServRequestHandler, RIL_CMD_PROXY_6);

RmcSuppServRequestHandler::RmcSuppServRequestHandler(int slot_id, int channel_id) :
    RmcSuppServRequestBaseHandler(slot_id, channel_id),
    m_UssdHandler(NULL) {

    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
    registerToHandleEvent(event, sizeof(event)/sizeof(int));

    isResetSession = false;

    if (RfxRilUtils::getRilRunMode() != RIL_RUN_MODE_MOCK) {
        if (slot_id == 0) {
            // Compatible with IMS repo utinterface binary
            if (startUtInterface("libutinterface_md.so") == NULL) {
                startUtInterface("libutinterface.so");
            }

            // If current platform is an data only project, then send ECFGSET disable to MD
            FeatureValue featurevalue;
            memset(featurevalue.value, 0, sizeof(featurevalue.value));
            mtkGetFeature(CONFIG_SS_MODE, &featurevalue);
            logD(TAG, "Check data only project, CONFIG_SS_MODE = %s", featurevalue.value);
            if (strcmp(featurevalue.value, "1") == 0) {
                atSendCommand("AT+ECFGSET=\"disable_cs_ss\",\"0\"");  // enable MD's SS capability
            } else {
                atSendCommand("AT+ECFGSET=\"disable_cs_ss\",\"1\"");  // disable MD's SS capability
            }
        }
    }

    // Create RmcSuppServUssdBaseHandler instance
    if (getFeatureVersion((char *) FEATURE_MD_USSD_DOMAIN_SELECTION, 0) == 1) {
        logD(TAG, "Create RmcSuppServUssdHandler");
        RFX_HANDLER_CREATE(m_UssdHandler, RmcSuppServUssdHandler,
                (m_slot_id, m_channel_id));

        /*  Enable USSI URC */
        atSendCommand("AT+ECUSD=2,4,\"\",72,\"\"");
    } else {
        logD(TAG, "Create RmcSuppServUssdDomainSelector");
        RFX_HANDLER_CREATE(m_UssdHandler, RmcSuppServUssdDomainSelector,
                (m_slot_id, m_channel_id));

        /*  Enable USSI URC */
        atSendCommand("AT+EIUSD=2,4,1,\"\",\"\",0");
    }

    /*  +CSSU unsolicited supp service notifications */
    atSendCommand("AT+CSSN=1,1");

    /*  connected line identification on */
    atSendCommand("AT+COLP=1");

    /*  USSD unsolicited */
    atSendCommand("AT+CUSD=1");

    atSendCommand("AT+CLIP=1");

    atSendCommand("AT+CNAP=1");

    /* always not query for CFU status by modem itself after first camp-on network */
    atSendCommand("AT+ESSP=1");

    /* HEX character set */
    atSendCommand("AT+CSCS=\"UCS2\"");

    requestGetModemVersion();

    syncCLIRToStatusManager();
}

RmcSuppServRequestHandler::~RmcSuppServRequestHandler() {
    RFX_HANDLER_DESTROY(m_UssdHandler);
    if (hDll != NULL) dlclose(hDll);
    hDll = NULL;
}

void RmcSuppServRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SEND_USSD:
            requestSendUSSD(msg);
            break;

        case RFX_MSG_REQUEST_CANCEL_USSD:
            requestCancelUssd(msg);
            break;

        case RFX_MSG_REQUEST_SET_CLIR:
            requestSetClir(msg);
            break;

        case RFX_MSG_REQUEST_SET_CALL_FORWARD:
            requestSetCallForward(msg);
            break;

        case RFX_MSG_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT:
            requestSetCallForwardInTimeSlot(msg);
            break;

        case RFX_MSG_REQUEST_SET_CALL_WAITING:
        case RFX_MSG_REQUEST_SET_CALL_WAITING_ATCI:
            requestSetCallWaiting(msg);
            break;

        case RFX_MSG_REQUEST_CHANGE_BARRING_PASSWORD:
            requestChangeBarringPassword(msg);
            break;

        case RFX_MSG_REQUEST_QUERY_CLIP:
            requestQueryClip(msg);
            break;

        case RFX_MSG_REQUEST_SET_CLIP:
            requestSetClip(msg);
            break;

        case RFX_MSG_REQUEST_GET_COLP:
            requestGetColp(msg);
            break;

        case RFX_MSG_REQUEST_SET_COLP:
            requestSetColp(msg);
            break;

        case RFX_MSG_REQUEST_GET_COLR:
            requestGetColr(msg);
            break;

        case RFX_MSG_REQUEST_SET_COLR:
            requestSetColr(msg);
            break;

        case RFX_MSG_REQUEST_SEND_CNAP:
            ///M: For query CNAP
            requestSendCNAP(msg);
            break;

        case RFX_MSG_REQUEST_SEND_USSI:
            requestSendUSSI(msg);
            break;

        case RFX_MSG_REQUEST_CANCEL_USSI:
            requestCancelUssi(msg);
            break;

        case RFX_MSG_REQUEST_SET_SUPP_SVC_NOTIFICATION:
            requestSetSuppSvcNotification(msg);
            break;

        case RFX_MSG_REQUEST_RESET_SUPP_SERV:
            requestResetSuppServ(msg);
            break;

        case RFX_MSG_REQUEST_SETUP_XCAP_USER_AGENT_STRING:
            requestSetupXcapUserAgentString(msg);
            break;

        case RFX_MSG_REQUEST_SET_SS_PROPERTY:
            requestSetSuppServProperty(msg);
            break;
        default:
            logE(TAG, "Should not be here");
            break;
    }
}

void RmcSuppServRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch(id) {
        case RFX_MSG_EVENT_SET_XCAP_CONFIG:
            requestSetXcapConfig(msg);
            break;
        default:
            logE(TAG, "should not be here");
            break;
    }
}

void RmcSuppServRequestHandler::onHandleTimer() {
    // do something
}

void RmcSuppServRequestHandler::requestSetClir(const sp<RfxMclMessage>& msg) {
    requestClirOperation(msg);
}

void RmcSuppServRequestHandler::requestSetCallForward(const sp<RfxMclMessage>& msg) {
    requestCallForwardOperation(msg, CCFC_E_SET);
}

void RmcSuppServRequestHandler::requestSetCallForwardInTimeSlot(const sp<RfxMclMessage>& msg) {
    requestCallForwardExOperation(msg, CCFC_E_SET);
}

void RmcSuppServRequestHandler::requestSetCallWaiting(const sp<RfxMclMessage>& msg) {
    requestCallWaitingOperation(msg, CW_E_SET);
}

void RmcSuppServRequestHandler::requestChangeBarringPassword(const sp<RfxMclMessage>& msg) {
    const char** strings = (const char**) (msg->getData()->getData());
    sp<RfxAtResponse> p_response;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;

    /**
     * "data" is const char **
     *
     * ((const char **)data)[0] = facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
     * ((const char **)data)[1] = old password
     * ((const char **)data)[2] = new password
     * ((const char **)data)[3] = new password confirmed
     */
    if (msg->getData()->getDataLength() == 3 * sizeof(char*)) {
        if (strings[0] == NULL || strlen(strings[0]) == 0 ||
            strings[1] == NULL || strlen(strings[1]) == 0 ||
            strings[2] == NULL || strlen(strings[2]) == 0) {
            logE(TAG, "ChangeBarringPassword: Null parameters.");
            ret = RIL_E_INVALID_ARGUMENTS;
            goto error;
        }
        asprintf(&cmd, "AT+ECUSD=1,1,\"**03*%s*%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]), strings[1], strings[2], strings[2]);
    } else if (msg->getData()->getDataLength() == 4 * sizeof(char*)) {
        if (strings[0] == NULL || strlen(strings[0]) == 0 ||
            strings[1] == NULL || strlen(strings[1]) == 0 ||
            strings[2] == NULL || strlen(strings[2]) == 0 ||
            strings[3] == NULL || strlen(strings[3]) == 0) {
            logE(TAG, "ChangeBarringPassword: Null parameters.");
            ret = RIL_E_INVALID_ARGUMENTS;
            goto error;
        }
        asprintf(&cmd, "AT+ECUSD=1,1,\"**03*%s*%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]), strings[1], strings[2], strings[3]);
    } else {
        goto error;
    }

    p_response = atSendCommand(cmd);

    free(cmd);

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestChangeBarringPassword Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_INCORRECT_PASSWORD:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        case CME_OPERATION_NOT_SUPPORTED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            break;
        case CME_844_UNPROVISIONED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            break;
        case CME_845_NO_DATA_CONNECTION:
            ret = RIL_E_845_NO_DATA_CONNECTION;
            break;
        default:
            break;
    }

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestSendUSSD(const sp<RfxMclMessage>& msg) {
    RFX_ASSERT(m_UssdHandler != NULL);
    m_UssdHandler->requestSendUssdDomainSelect(msg);
}

void RmcSuppServRequestHandler::requestCancelUssd(const sp<RfxMclMessage>& msg) {
    RFX_ASSERT(m_UssdHandler != NULL);
    m_UssdHandler->requestCancelUssdDomainSelect(msg);
}

void RmcSuppServRequestHandler::requestGetColp(const sp<RfxMclMessage>& msg) {
    requestColpOperation(msg);
}

void RmcSuppServRequestHandler::requestSetColp(const sp<RfxMclMessage>& msg) {
    requestColpOperation(msg);
}

void RmcSuppServRequestHandler::requestGetColr(const sp<RfxMclMessage>& msg) {
    requestColrOperation(msg);
}

void RmcSuppServRequestHandler::requestSetColr(const sp<RfxMclMessage>& msg) {
    requestColrOperation(msg);
}

/**
 * This command refers to the supplementary service CNAP (Calling Name Presentation)
 * according to 3GPP TS 22.096 that enables a called subscriber to get a calling name
 * indication (CNI) of the calling party when receiving a mobile terminated call.
 *
 * Set command enables or disables the presentation of the CNI at the TE.
 * It has no effect on the execution of the supplementary service CNAP in the network.
 * When <n>=1, the presentation of the calling name indication at the TE is enabled and
 * CNI is provided the unsolicited result code. Read command gives the status of<n>,
 * and also triggers an interrogation of the provision status of the CNAP service
 * according 3GPP TS 22.096 (given in <m>). Test command returns values supported
 * as a compound value.
 */
void RmcSuppServRequestHandler::requestSendCNAP(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int responses[2]={0};

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */
    p_response = atSendCommandSingleline("AT+ECUSD=1,1,\"*#300#\"", "+CNAP:");

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
       logE(TAG, "requestSendCNAP Fail");
       goto error;
    }

    switch (p_response->atGetCmeError()) {
       case CME_SUCCESS:
          break;
       case CME_CALL_BARRED:
       case CME_OPR_DTR_BARRING:
          ret = RIL_E_CALL_BARRED;
          break;
       case CME_PHB_FDN_BLOCKED:
          ret = RIL_E_FDN_CHECK_FAILURE;
          break;
       case CME_OPERATION_NOT_SUPPORTED:
          ret = RIL_E_REQUEST_NOT_SUPPORTED;
          break;
       case CME_844_UNPROVISIONED:
          ret = RIL_E_REQUEST_NOT_SUPPORTED;
          break;
       case CME_845_NO_DATA_CONNECTION:
          ret = RIL_E_845_NO_DATA_CONNECTION;
          break;
       default:
          goto error;
    }

    if (p_response->getIntermediates() != NULL) {
        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <n> integer type (parameter sets/shows the result code presentation status to the TE)
         * 0   disable
         * 1   enable
         */
        responses[0] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <m> integer type (parameter shows the subscriber CNAP service status in the network)
         * 0   CNAP not provisioned
         * 1   CNAP provisioned
         * 2   unknown (e.g. no network, etc.)
         */
        responses[1] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxIntsData(responses, sizeof(responses)/sizeof(int)), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestQueryClip(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int responses[2]={0};
    int result = 0;
    String8 currentMccmnc = getMclStatusManager()->
            getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC, String8("0"));

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */
    p_response = atSendCommandSingleline("AT+ECUSD=1,1,\"*#30#\"", "+CLIP:");

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestQueryClip Fail");
        goto error;
    }


    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        case CME_403_FORBIDDEN:
            ret = RIL_E_UT_XCAP_403_FORBIDDEN;
            goto error;
            break;
        case CME_404_NOT_FOUND:
            ret = RIL_E_404_NOT_FOUND;
            goto error;
            break;
        case CME_409_CONFLICT:{
            if (SSConfig::convert409ToNotSupportCallerID(currentMccmnc)) {
                ret = RIL_E_REQUEST_NOT_SUPPORTED;
                goto error;
                break;
            }

            ret = RIL_E_409_CONFLICT;
            goto error;
            break;
        }
        case CME_412_PRECONDITION_FAILED:
            ret = RIL_E_412_PRECONDITION_FAILED;
            goto error;
            break;
        case CME_NETWORK_TIMEOUT:
            ret = RIL_E_UT_UNKNOWN_HOST;
            goto error;
            break;
        case CME_OPERATION_NOT_SUPPORTED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            goto error;
            break;
        case CME_844_UNPROVISIONED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            goto error;
            break;
        case CME_845_NO_DATA_CONNECTION:
            ret = RIL_E_845_NO_DATA_CONNECTION;
            goto error;
            break;
        default:
            goto error;
    }

    if ( p_response->getIntermediates() != NULL ) {
        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <n> (parameter sets/shows the result code presentation status in the MT/TA):
         * 0   disable
         * 1   enable
         */
        responses[0] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <m> (parameter shows the subscriber CLIP service status in the network):
         * 0   CLIP not provisioned
         * 1   CLIP provisioned
         * 2   unknown (e.g. no network, etc.)
         */
        responses[1] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
    }

    // check provision status first
    if (responses[1] == 0 || responses[1] == 2) {
        result = 0;
    } else {
        result = responses[0];
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxIntsData(&result, sizeof(result)/sizeof(int)), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestSetClip(const sp<RfxMclMessage>& msg) {
    int *n = (int *) (msg->getData()->getData());
    sp<RfxAtResponse> p_response = NULL;
    int err = 0; //Coverity, follow the err init value in at_send_command_full_nolock().
    char* cmd = NULL;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int responses[2]={0};
    String8 currentMccmnc = getMclStatusManager()->
            getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC, String8("0"));

    if (msg->getData()->getDataLength() != 0) {
        /**
         * Set CLIP: +CLIP=[<n>]
         * "data" is int *
         * ((int *)data)[0] is "n" parameter from TS 27.007 7.6
         *  <n> (Set command enables or disables the presentation of the CLI at the TE)
         */
        asprintf(&cmd, "AT+CLIP=%d", n[0]);

        // p_response = atSendCommand(cmd);
        p_response = atSendCommandMultiline(cmd, "+CLIP:");
        err = p_response->getError();

        free(cmd);
    }

    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestSetClip Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        case CME_403_FORBIDDEN:
            ret = RIL_E_UT_XCAP_403_FORBIDDEN;
            goto error;
            break;
        case CME_404_NOT_FOUND:
            ret = RIL_E_404_NOT_FOUND;
            goto error;
            break;
        case CME_409_CONFLICT: {
            if (SSConfig::convert409ToNotSupportCallerID(currentMccmnc)) {
                ret = RIL_E_REQUEST_NOT_SUPPORTED;
                goto error;
                break;
            }

            ret = RIL_E_409_CONFLICT;
            handleErrorMessageFromXcap(p_response, CME_409_CONFLICT);
            goto error;
            break;
        }
        case CME_412_PRECONDITION_FAILED:
            ret = RIL_E_412_PRECONDITION_FAILED;
            goto error;
            break;
        case CME_NETWORK_TIMEOUT:
            ret = RIL_E_UT_UNKNOWN_HOST;
            goto error;
            break;
        case CME_OPERATION_NOT_SUPPORTED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            goto error;
            break;
        case CME_844_UNPROVISIONED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            goto error;
            break;
        case CME_845_NO_DATA_CONNECTION:
            ret = RIL_E_845_NO_DATA_CONNECTION;
            goto error;
            break;
        default:
            goto error;
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    /* For SET CLIP responseVoid will ignore the responses */
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxIntsData(responses, sizeof(responses) / sizeof(int)), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestSendUSSI(const sp<RfxMclMessage>& msg) {
    RFX_ASSERT(m_UssdHandler != NULL);
    m_UssdHandler->requestSendUssdDomainSelect(msg);
}

void RmcSuppServRequestHandler::requestCancelUssi(const sp<RfxMclMessage>& msg) {
    RFX_ASSERT(m_UssdHandler != NULL);
    m_UssdHandler->requestCancelUssdDomainSelect(msg);
}

void RmcSuppServRequestHandler::requestSetupXcapUserAgentString(const sp<RfxMclMessage>& msg) {
    const char** params = (const char**) (msg->getData()->getData());
    sp<RfxAtResponse> p_response = NULL;
    int err = 0;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(TAG, "requestSetupXcapUserAgentString: userAgent = %s", params[0]);

    /**
     * AT+ECAPUAS=<str>
     * <str>:  Customized XCAP user agent string. The OEM name, model number and
     *         firmware version should be included. (e.g. "MTK/1.0.0 1.0.0")
     */
    if (params[0] != NULL && strlen(params[0]) != 0) {
        p_response = atSendCommand(String8::format("AT+EXCAPUAS=\"%s\"", params[0]));
    } else {
        logE(TAG, "requestSetupXcapUserAgentString: userAgent string is null or empty.");
        goto error;
    }

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestSetupXcapUserAgentString Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_OPERATION_NOT_SUPPORTED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            break;
        default:
            break;
    }

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestSetSuppSvcNotification(const sp<RfxMclMessage>& msg) {
    int *n = (int *) (msg->getData()->getData());
    sp<RfxAtResponse> p_response;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    //asprintf(&cmd, "AT+ECUSD=1,1,\"%s\",72", p_ussdRequest); /* <dcs> = 0x48 */
    asprintf(&cmd, "AT+CSSN=%d,%d", n[0], n[0]);

    p_response = atSendCommand(cmd);

    free(cmd);

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
       logE(TAG, "requestSetSuppSvcNotification Fail");
       goto error;
    }

    switch (p_response->atGetCmeError()) {
       case CME_SUCCESS:
          ret = RIL_E_SUCCESS;
          break;
       default:
          break;
    }

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void* RmcSuppServRequestHandler::startUtInterface(const char* libutinterfacePath) {
    char* dllerror;
    void (*fnstartUtInterface)();
    logD(TAG, "startUtInterface(): %s", libutinterfacePath);

    hDll = dlopen(libutinterfacePath, RTLD_NOW);
    if(hDll) {
        fnstartUtInterface = (void (*)())dlsym(hDll, "startUtInterface");
        if ((dllerror = (char*)dlerror()) != NULL) {
            logE(TAG, "dlerror: %s", dllerror);
        }
        if (fnstartUtInterface == NULL) {
            logE(TAG, "fnstartUtInterface is NULL");
            dlclose(hDll);
        } else {
            logD(TAG, "call fnstartUtInterface");
            (*fnstartUtInterface)();
        }
    } else {
        if ((dllerror = (char*)dlerror()) != NULL) {
            logE(TAG, "hDll is NULL, dlerror: %s", dllerror);
        } else {
            logE(TAG, "hDll is NULL");
        }
    }
    return hDll;
}

void RmcSuppServRequestHandler::requestSetSuppServProperty(const sp<RfxMclMessage>& msg) {
    const char** strings = (const char**) (msg->getData()->getData());
    rfx_property_set(strings[0], strings[1]);
    logD(TAG, "requestSetSuppServProperty: name = %s, value = %s", strings[0], strings[1]);

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestGetSuppServProperty(const sp<RfxMclMessage>& msg) {
    // For 95 modem
    const char *string = (const char*)msg->getData()->getData();
    logD(TAG, "requestGetSuppServProperty: name = %s", string);

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServRequestHandler::requestGetModemVersion() {
    sp<RfxAtResponse> p_response;
    int err, version;
    char* cmd = NULL;
    char* version_string = NULL;
    RfxAtLine *line;

    asprintf(&cmd, "AT+EXCAPCFGGET=\"version\"");

    p_response = atSendCommandSingleline(cmd, "+EXCAPCFGGET:");
    free(cmd);

    if (p_response == NULL) {
        logE(TAG, "requestGetModemVersion is not supported.");
        goto error;
    }

    err = p_response->getError();

    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestGetModemVersion failed, error code = %d", err);
        goto error;
    }

    /**
    * +EXCAPCFGGET: "version","1"
    * <m> version string
    * <n> version
    *     1 : 95modem
    *     0 : Non-95 modem
    */

    if (p_response->getIntermediates() != NULL) {
        line = p_response->getIntermediates();
        logD(TAG, "requestGetModemVersion response = %s", line->getLine());

        version_string = line->atTokNextstr(&err);
        if (err < 0) {
            goto error;
        }

        version = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
        rfx_property_set(PROPERTY_MODEM_VERSION, std::to_string(version).c_str());
        logD(TAG, "requestGetModemVersion version = %d", version);
        return;
    } else {
        logE(TAG, "requestGetModemVersion p_response->getIntermediates() == NULL");
    }

error:
    rfx_property_set(PROPERTY_MODEM_VERSION, "0");
}

void RmcSuppServRequestHandler::requestSetXcapConfig(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    char* cmd = NULL;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    char **data = (char **)msg->getData()->getData();

    char modem_version[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_MODEM_VERSION, modem_version, "0");

    const char *delim = ",";
    char* name = strtok(data[1], delim);
    char* value = strtok(NULL, delim);
    logD(TAG, "requestSetXcapConfig: name = %s, value = %s", name, value);

    if (name == NULL) {
        logE(TAG, "SetXcapConfig name is NULL");
        goto error;
    }

    if (atoi(modem_version) != 0) {
        int name_length = strlen(name);
        int prefix_length = strlen("persist.vendor.ss.");
        int config_length = name_length - prefix_length;
        char* config_name = (char *) alloca(config_length + 1);
        if (config_name == NULL) {
            logE(TAG, "Out of memory");
            goto error;
        }
        strncpy(config_name, name + prefix_length, config_length);
        config_name[config_length] = '\0';

        logD(TAG, "requestSetXcapConfig: sub_name = %s, value = %s", config_name, value);

        if (value != NULL && strlen(value) > 0) {
            rfx_property_set(name, value);
            if (!isResetSession) {
                asprintf(&cmd, "AT+EXCAPCFGSET=\"%s\",\"%s\"", config_name, value);
                p_response = atSendCommandSingleline(cmd, "+EXCAPCFGSET:");
                free(cmd);
            } else {
                // In reset session no need to send AT cmd to momem.
                ret = RIL_E_SUCCESS;
            }
        } else {
            // For 95 modem reset
            if (strcmp(config_name, "reset") == 0) {
                logD(TAG, "reset session start");
                isResetSession = true;
                // We can only send AT+EXCAPCFGSET="reset" to modem.
                asprintf(&cmd, "AT+EXCAPCFGSET=\"%s\"", config_name);
                p_response = atSendCommandSingleline(cmd, "+EXCAPCFGSET:");
                free(cmd);
            } else if (strcmp(config_name, "resetdone") == 0) {
                logD(TAG, "reset session end");
                isResetSession = false;
                ret = RIL_E_SUCCESS;
            } else {
                rfx_property_set(name, "");
                ret = RIL_E_SUCCESS;
            }
        }

        if (p_response == NULL) {
            logE(TAG, "requestSetXcapConfig has no response.");
            goto error;
        }

        if(strStartsWith(p_response->getFinalResponse()->getLine(), "ERROR")) {
            logE(TAG, "requestSetXcapConfig Error.");
            goto error;
        }

        ret = RIL_E_SUCCESS;
    } else {
        rfx_property_set(name, value);
        ret = RIL_E_SUCCESS;
    }

error:
    logD(TAG, "requestSetXcapConfig: pRequest = %d, token = %d", msg->getId(), msg->getToken());
    int msgid = RFX_MSG_REQUEST_OEM_HOOK_STRINGS;
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msgid, ret,
            RfxVoidData(), msg);

    responseToTelCore(response);
}

void RmcSuppServRequestHandler::syncCLIRToStatusManager() {
    char clir_n[MTK_PROPERTY_VALUE_MAX];

    snprintf(clir_n, sizeof(clir_n), "0");
    getMSimProperty(m_slot_id, (char *) PROPERTY_TBCLIR_N, clir_n);

    logD(TAG, "syncCLIRToStatusManager, clir_n = %s", clir_n);

    // sync value to RfxStatusManager
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_TBCLIR_N, atoi(clir_n));
}
