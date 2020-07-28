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

#include "RmcImsControlRequestHandler.h"
#include "rfx_properties.h"
#include "RfxRilUtils.h"
#include <telephony/mtk_ril.h>
#include <string>
#include <string.h>
#include "ConfigUtil.h"
#include "ImsCustomizedUtils.h"

static const int requests[] = {
    RFX_MSG_REQUEST_SET_VOLTE_ENABLE,           //AT+EIMSVOLTE
    RFX_MSG_REQUEST_SET_WFC_ENABLE,             //AT+EIMSWFC
    RFX_MSG_REQUEST_SET_VILTE_ENABLE,           //AT+EIMSVILTE
    RFX_MSG_REQUEST_SET_VIWIFI_ENABLE,          //AT+EIMSVIWIFI
    RFX_MSG_REQUEST_SET_IMSCFG,                 //AT+EIMSCFG
    RFX_MSG_REQUEST_SET_MD_IMSCFG,              //AT+ECFGSET,AT+EWOCFGSET,AT+EIWLCFGSET
    RFX_MSG_REQUEST_SET_WFC_PROFILE,            //AT+EWFCP
    RFX_MSG_REQUEST_IMS_REGISTRATION_STATE,     //AT+CIREG
    RFX_MSG_REQUEST_SET_IMS_ENABLE,             //AT+EIMS
    RFX_MSG_REQUEST_IMS_DEREG_NOTIFICATION,     //AT+EIMSDEREG
    RFX_MSG_REQUEST_SET_IMS_REGISTRATION_REPORT,  //AT+CIREG=2
    RFX_MSG_REQUEST_SET_IMS_RTP_REPORT,         // AT+EIMSRTPTS=<default_ebi>,<network_id>,<timer>
    RFX_MSG_REQUEST_QUERY_VOPS_STATUS,          // AT+CIREP
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_VOLTE_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_WFC_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_VILTE_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_VIWIFI_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_IMSCFG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxStringData, RFX_MSG_REQUEST_SET_MD_IMSCFG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_WFC_PROFILE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_IMS_REGISTRATION_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_IMS_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_IMS_DEREG_NOTIFICATION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_SET_IMS_REGISTRATION_REPORT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_IMS_RTP_REPORT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_QUERY_VOPS_STATUS);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcImsControlRequestHandler, RIL_CMD_PROXY_1);

RmcImsControlRequestHandler::RmcImsControlRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcImsControlRequestHandler constructor");
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
    enableImsRegistrationReport(NULL);
    // enableImsEccSupportReport();
    requestMultiImsSupportCount();
    enableSipRegInfoInd();
    // enableImsRcsStateFeature();
    enableVopsStatusReport();

    userAgentformat = String8("");

    /// M: L+L/SRLTE+L need know ct volte Support or not @{
    static bool initDone = false;
    if (initDone == false) {
        initDone = true;
        char ctVolteSupport[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("persist.vendor.mtk_ct_volte_support", ctVolteSupport, "0");
        if (atoi(ctVolteSupport) == 1) {
            atSendCommand(String8::format("AT+ECFGSET=\"mtk_ct_volte_support\",\"1\""));
            logD(RFX_LOG_TAG, "CT VoLTE over IMS is support and AT+ECFGSET=...,1 is sent");
        } else if (atoi(ctVolteSupport) == 2 || atoi(ctVolteSupport) == 3) {
            atSendCommand(String8::format("AT+ECFGSET=\"mtk_ct_volte_support\",\"2\""));
            logD(RFX_LOG_TAG, "CT VoLTE over IMS is support and AT+ECFGSET=...,2 is sent");
        } else {
            atSendCommand(String8::format("AT+ECFGSET=\"mtk_ct_volte_support\",\"0\""));
            logD(RFX_LOG_TAG, "CT VoLTE over IMS is supported and AT+ECFGSET=...,0 is sent");
        }
    }
    /// @}
}

#define RIL_WIFI_PREFERRED     1
#define RIL_CELLULAR_PREFERRED 2
#define RIL_WIFI_ONLY          3
#define RIL_4G_PREFERRED       4

RmcImsControlRequestHandler::~RmcImsControlRequestHandler() {
}

void RmcImsControlRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    //logD(RFX_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SET_VOLTE_ENABLE:
            requestSetVolteEnabled(msg);
            break;
        case RFX_MSG_REQUEST_SET_WFC_ENABLE:
            requestSetWfcEnabled(msg);
            break;
        case RFX_MSG_REQUEST_SET_VILTE_ENABLE:
            requestSetVilteEnabled(msg);
            break;
        case RFX_MSG_REQUEST_SET_VIWIFI_ENABLE:
            requestSetViwifiEnabled(msg);
            break;
        case RFX_MSG_REQUEST_SET_IMSCFG:
            requestSetImsCfg(msg);
            break;
        case RFX_MSG_REQUEST_SET_MD_IMSCFG:
            requestSetModemImsCfg(msg);
            break;
        case RFX_MSG_REQUEST_SET_WFC_PROFILE:
            requestSendWfcProfile(msg);
            break;
        case RFX_MSG_REQUEST_IMS_REGISTRATION_STATE:
            requestImsRegistrationState(msg);
            break;
        case RFX_MSG_REQUEST_SET_IMS_ENABLE:
            requestSetImsEnabled(msg);
            break;
        case RFX_MSG_REQUEST_IMS_DEREG_NOTIFICATION:
            requestImsDereg(msg);
            break;
        case RFX_MSG_REQUEST_SET_IMS_REGISTRATION_REPORT:
            enableImsRegistrationReport(msg);
            break;
        case RFX_MSG_REQUEST_SET_IMS_RTP_REPORT:
            requestSetImsRtpReport(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_VOPS_STATUS:
            queryVopsStatus(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcImsControlRequestHandler::requestSetVolteEnabled(const sp<RfxMclMessage>& msg) {
    int *enable = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    logD(RFX_LOG_TAG, "requestSetVolteEnabled enable:%d", enable[0]);
    p_response = atSendCommand(String8::format("AT+EIMSVOLTE=%d", enable[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::requestSetWfcEnabled(const sp<RfxMclMessage>& msg) {
    int *enable = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    logD(RFX_LOG_TAG, "requestSetWfcEnabled enable:%d", enable[0]);
    p_response = atSendCommand(String8::format("AT+EIMSWFC=%d", enable[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::requestSetVilteEnabled(const sp<RfxMclMessage>& msg) {
    int *enable = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    logD(RFX_LOG_TAG, "requestSetVilteEnabled enable:%d", enable[0]);
    p_response = atSendCommand(String8::format("AT+EIMSVILTE=%d", enable[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);

}

void RmcImsControlRequestHandler::requestSetViwifiEnabled(const sp<RfxMclMessage>& msg) {
    int *enable = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    logD(RFX_LOG_TAG, "requestSetViwifiEnabled enable:%d", enable[0]);
    p_response = atSendCommand(String8::format("AT+EIMSVIWIFI=%d", enable[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::requestSetImsCfg(const sp<RfxMclMessage>& msg) {
    int *params = (int *)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    if (isCurrentSlotSupportIms()) {
        int volte = params[0];
        int vilte = params[1];
        int vowifi = params[2];
        int viwifi = params[3];
        int sms = params[4];
        int eims = params[5];

        /**
         * Used to initialize RCS related feature tags.
         * invoke here because RCS feature initialize also
         * need to be done before IMS registration started,
         * thus appropriate feature tags can be carried in
         * IMS Register.
         */
        enableImsRcsStateFeature();

        /**
         * Check IMS test mode setting
         *    1 : AT+EIMS=0
         *    0 : Normal operation
         */
        char value[MTK_PROPERTY_VALUE_MAX] = {0};
        int imsTestMode = -1;
        rfx_property_get("persist.vendor.radio.imstestmode", value, "0");
        imsTestMode = atoi(value);
        eims = (imsTestMode == 1) ? 0 : eims;

        // Set user agent before ims config to modem
        requestSetSipUserAgent();

        // send AT command
        logD(RFX_LOG_TAG, "requestSetImsCfg volte:%d, vilte:%d, vowifi:%d, viwifi:%d, sms:%d, \
                imsTestMode:%d, eims:%d", volte, vilte, vowifi, viwifi, sms, imsTestMode, eims);
        p_response = atSendCommand(String8::format("AT+EIMSCFG=%d,%d,%d,%d,%d,%d",
                volte, vilte, vowifi, viwifi, sms, eims));

        // set result
        if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
            rilErrNo = RIL_E_GENERIC_FAILURE;
        }
    } else {
        logD(RFX_LOG_TAG, "requestSetImsCfg slotId %d not support IMS", m_slot_id);
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::requestSetModemImsCfg(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "requestSetMdImsCfg enter");
    int err = 0;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int len, i;

    char **params = (char **)msg->getData()->getData();

    char *key_token = NULL;
    char *val_token = NULL;

    char *p_key_cur = (char *) calloc(strlen(params[0]) + 1, sizeof(char));
    char *p_val_cur = (char *) calloc(strlen(params[1]) + 1, sizeof(char));

    char *p_key_temp= p_key_cur;
    char *p_val_temp= p_val_cur;

    if ((p_key_cur == NULL) || (p_val_cur == NULL)) {
        logD(RFX_LOG_TAG, "requestSetMdImsCfg calloc failed!");

        if (p_key_cur != NULL) {
            free(p_key_cur);
        }

        if (p_val_cur != NULL) {
            free(p_val_cur);
        }

        return;
    }

    strncpy(p_key_cur, params[0], strlen(params[0]));
    strncpy(p_val_cur, params[1], strlen(params[1]));

    std::string result = "";
    char *rsp_string;

    int type = atoi(params[2]);

    if (type >= 0 && type < MAX_MD_CFGTYPE_NUM) {
        while ((key_token = nextTok(&p_key_cur)) != NULL) {
            if ((val_token = nextTok(&p_val_cur)) != NULL) {

                // send AT command, default send AT+ECFGSET (type is 0)
                logD(RFX_LOG_TAG, "requestSetMdImsCfg key:%s, value:%s", key_token, val_token);

                if (type == ECFGSET) {
                    p_response = atSendCommand(
                            String8::format("%s=\"%s\",\"%s\"", cfgAtCmd[ECFGSET], key_token, val_token));
                } else if(type == EWOCFGSET) {
                    p_response = atSendCommand(
                            String8::format("%s=\"%s\",\"%s\"", cfgAtCmd[EWOCFGSET], key_token, val_token));
                } else if(type == EIWLCFGSET) {
                    p_response = atSendCommand(
                            String8::format("%s=\"%s\",\"%s\"", cfgAtCmd[EIWLCFGSET], key_token, val_token));
                }

                // set result
                if (p_response == NULL ||
                        p_response->getError() != 0 ||
                        p_response->getSuccess() != 1) {
                    // default return fail, should return fail final if all fail
                    if (result.empty()) {
                        result += std::string("-1");
                    } else {
                        result += std::string(",-1");
                    }
                } else {
                    // if any of config have ever success, should return success final
                    if (result.empty()) {
                        result += std::string("0");
                    } else {
                        result += std::string(",0");
                    }
                    ril_errno = RIL_E_SUCCESS;
                }
            } else {
                logD(RFX_LOG_TAG, "cannot parse value for current key:%s", key_token);
                ril_errno = RIL_E_GENERIC_FAILURE;
                // key and value length not match
            }
        }
    } else {
        logD(RFX_LOG_TAG, "type:%d not match", type);
    }

    rsp_string = new char[result.size() + 1];
    std::copy(result.begin(), result.end(), rsp_string);
    rsp_string[result.size()] = '\0';

    responseMsg = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxStringData(rsp_string, strlen(rsp_string)), msg, false);
    // response to TeleCore
    responseToTelCore(responseMsg);

    free(p_key_temp);
    free(p_val_temp);
    delete[] rsp_string;
}

void RmcImsControlRequestHandler::requestSendWfcProfile(const sp<RfxMclMessage>& msg) {
    int *preference = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    int wfc_prefer = preference[0];

    // send AT command
    logE(RFX_LOG_TAG, "requestSendWfcProfile preference:%d", wfc_prefer);
    p_response = atSendCommand(String8::format("AT+EWFCP=%d", wfc_prefer));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::enableImsRegistrationReport(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;

    // send AT command
    logD(RFX_LOG_TAG, "Enable +CIREGU report");
    p_response = atSendCommand(String8::format("AT+CIREG=2"));
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
        logE(RFX_LOG_TAG, "Enable +CIREGU report error: %d", p_response->getError());
    }

    if(msg != NULL){
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

        // response to TeleCore
        responseToTelCore(response);
    }
}

void RmcImsControlRequestHandler::requestImsRegistrationState(const sp<RfxMclMessage>& msg) {
    int err;
    int skip;
    int response[2] = {0};
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;
    RfxAtLine* line;

    p_response = atSendCommandSingleline("AT+CIREG?", "+CIREG:");

    /* +CIREG: <n>,<reg_info>[,<ext_info>] */
    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) goto error;

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    /* <mode> */
    skip = line->atTokNextint(&err);
    if (err < 0 || skip < 0 ) {
        logE(RFX_LOG_TAG, "The <mode> is an invalid value!!!");
        goto error;
    } else {
        /* <reg_info> value 0: not registered , 1: registered */
        response[0] = line->atTokNextint(&err);
        if (err < 0 ) goto error;

        response[1] = 1; // RADIO_TECH_3GPP
    }
    responseMsg = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(response, 2), msg, false);
    // response to TeleCore
    responseToTelCore(responseMsg);

    return;
error:
    logE(RFX_LOG_TAG, "requestImsRegistrationState must not return error when radio is on");
    responseMsg = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData(response, 2), msg, false);
    // response to TeleCore
    responseToTelCore(responseMsg);
}

void RmcImsControlRequestHandler::requestSetImsEnabled(const sp<RfxMclMessage>& msg) {
    int *enable = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    logD(RFX_LOG_TAG, "requestSetImsEnabled enable:%d", enable[0]);
    p_response = atSendCommand(String8::format("AT+EIMS=%d", enable[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::requestImsDereg(const sp<RfxMclMessage>& msg) {
    int *cause = (int*)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    logD(RFX_LOG_TAG, "requestImsDereg cause:%d", cause[0]);
    p_response = atSendCommand(String8::format("AT+EIMSDEREG=%d", cause[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::enableImsEccSupportReport() {
    sp<RfxAtResponse> p_response;

    // send AT command
    logD(RFX_LOG_TAG, "Enable +EIMSESS report");
    p_response = atSendCommand(String8::format("AT+EIMSESS=1"));

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        logE(RFX_LOG_TAG, "Enable +EIMSESS report error: %d", p_response->getError());
    }
}

void RmcImsControlRequestHandler::requestSetImsRtpReport(const sp<RfxMclMessage>& msg) {
    int *params = (int *)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    // AT+EIMSRTPTS=<default_ebi>,<network_id>,<timer>
    logD(RFX_LOG_TAG, "requestSetImsRtpReport pdn_id:%d, network_id:%d, timer:%d",
            params[0], params[1], params[2]);
    p_response = atSendCommand(String8::format("AT+EIMSRTPTS=%d,%d,%d",
            params[0], params[1], params[2]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsControlRequestHandler::requestMultiImsSupportCount() {
    int err;
    sp<RfxAtResponse> p_response = NULL;
    int multiImsCount;
    sp<RfxMclMessage> urc = NULL;
    RfxAtLine* line;

    // use AP side system property as default return value.
    char sApMultiImsCount[RFX_PROPERTY_VALUE_MAX] = { 0 };
    char sMdMultiImsCount[RFX_PROPERTY_VALUE_MAX] = { 0 };
    const char* PROP_AP_MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    const char* PROP_MD_MULTI_IMS_SUPPORT = "ro.vendor.md_mims_support";

    rfx_property_get(PROP_MD_MULTI_IMS_SUPPORT, sMdMultiImsCount, "-1");
    if (strcmp(sMdMultiImsCount, "-1") != 0) {
        return;
    }
    rfx_property_get(PROP_AP_MULTI_IMS_SUPPORT, sApMultiImsCount, "1");

    // send AT command to get intermediate response
    p_response = atSendCommandSingleline("AT+EMULTIIMS?", "+EMULTIIMS:");

    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) goto error;

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    /* ims count */
    multiImsCount = line->atTokNextint(&err);
    if (err < 0) {
        goto error;
    }

    logD(RFX_LOG_TAG, "+EMULTIIMS get intermediate response successfully: %d", multiImsCount);
    sprintf(sMdMultiImsCount, "%d", multiImsCount);
    rfx_property_set("ro.vendor.md_mims_support", sMdMultiImsCount);
    return;
error:
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        logE(RFX_LOG_TAG, "Request +EMULTIIMS count error: %d", p_response->getError());
         switch (p_response->atGetCmeError()) {
             logD(RFX_LOG_TAG, "+EMULTIIMS CME error p_response = %d",
                    p_response->atGetCmeError());
             case CME_SUCCESS:
                 logD(RFX_LOG_TAG, "+EMULTIIMS generic failure p_response: CME_SUCCESS");
                 break;
             case CME_UNKNOWN:
                 logD(RFX_LOG_TAG, "+EMULTIIMS error p_response: CME_UNKNOWN");
                 // if MD not support +EMULTIIMS, return default MIMS count
                 rfx_property_set("ro.vendor.md_mims_support", sApMultiImsCount);
                 break;
             default:
                 return;
         }
    } else {
        logE(RFX_LOG_TAG, "handle response data count error: %d", err);
    }
}

void RmcImsControlRequestHandler::enableImsRcsStateFeature() {
    static bool initOnce[MAX_SIM_COUNT] = {false};

    if (initOnce[m_slot_id])
        return;

    // is rcs ua support?
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    int isRcsUaProxySupport = 0;
    rfx_property_get("persist.vendor.mtk_rcs_ua_support", property_value, "0");
    isRcsUaProxySupport = atoi(property_value);
    logD(RFX_LOG_TAG, "isRcsUaProxySupport: %d", isRcsUaProxySupport);

    if (isRcsUaProxySupport == 0) {
        /* MD will save RCS state once enabled, so always reset here
           for runtime switch RCS UA support case.
           Response of EIMSRCS not the issue here, just ignore it */
        atSendCommand("AT+EIMSRCS=0,0");
        initOnce[m_slot_id] = true;
        return;
    }

    // is rcs service enable?
    int isRcsServiceEnabled = 1;
    memset(property_value, 0, sizeof(property_value));
    rfx_property_get("persist.vendor.service.rcs", property_value, "1");
    isRcsServiceEnabled = atoi(property_value);
    logD(RFX_LOG_TAG, "isRcsServiceEnabled: %d", isRcsServiceEnabled);

    // what are the rcs service tags?
    /**
     * Checks which RCS service tag was supported.
     * @return RCS service tag bit-map:
     *         0x01  session
     *         0x02  filetransfer
     *         0x04  msg
     *         0x08  largemsg
     *         0x10  geopush
     *         0x20  geopull
     *         0x40  geopullft
     *         0x80  imdn aggregation
     *        0x100  geosms
     *        0x200  fthttp
     *         0x00  RCS service tag was removed
     */
    int rcsSrvTags = 0, rcsCustomized = 0;
    memset(property_value, 0, sizeof(property_value));
    rfx_property_get("persist.vendor.service.tag.rcs", property_value, "0");
    rcsSrvTags = atoi(property_value);
    logD(RFX_LOG_TAG, "rcsCustomized: %d, rcsSrvTags: %d", rcsCustomized, rcsSrvTags);
    if (rcsSrvTags < 0x00 || rcsSrvTags > 0xFFFF) {
        rcsSrvTags = 0;
    }

    sp<RfxAtResponse> p_response;
    // send AT command
    if (isRcsServiceEnabled == 1 && rcsSrvTags > 0) {
        p_response = atSendCommand(String8::format("AT+EIMSRCS=1,%d", rcsSrvTags));
    } else {
        p_response = atSendCommand(String8::format("AT+EIMSRCS=0,%d", rcsSrvTags));
    }

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        logE(RFX_LOG_TAG, "enableImsRcsStateFeature(): AT+EIMSRCS error: %d",
            p_response->getError());
    }
    initOnce[m_slot_id] = true;
}

void RmcImsControlRequestHandler::requestSetSipUserAgent() {
    logD(RFX_LOG_TAG, "requestSetSipUserAgent enter:");
    int err;
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response = NULL;
    RIL_Errno rilErrNo =  RIL_E_GENERIC_FAILURE;
    char *value = NULL;
    bool isReset = false;
    char feature[] = "ECFGRESET Supported";

    if (getFeatureVersion(feature) <= 0 || !ImsCustomizedUtils::isSupportCustomizedUserAgent()) {
        return ;
    }

    String8 format = getMclStatusManager()->getString8Value(
            RFX_STATUS_KEY_CUSTOMIZED_USER_AGENT_FORMAT);

    logI(RFX_LOG_TAG, "requestSetSipUserAgent format: old=%s, new = %s; reset version=%d",
            userAgentformat.string(), format.string(), getFeatureVersion(feature));

    while (rilErrNo == RIL_E_GENERIC_FAILURE) {
        // Check if the format already be set before

        if (userAgentformat == format) {
            logI(RFX_LOG_TAG, "requestSetSipUserAgent format no changed.");
            break;
        } else {
            logI(RFX_LOG_TAG, "requestSetSipUserAgent format changed.");
            if (String8("0") == format) {
                isReset = true;

                // Reset User-Agent Value to MD
                p_response = atSendCommand(String8::format("AT+ECFGRESET=\"user_agent\""));

                if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
                    logE(RFX_LOG_TAG, "Reset user agent error: %d", p_response->getError());
                    break;
                }

                rilErrNo = RIL_E_SUCCESS;

            } else {
                String8 newValue = ImsCustomizedUtils::getCustomizedUserAgent(format, m_slot_id);

                p_response = atSendCommandSingleline(
                                   String8::format("AT+ECFGGET=\"user_agent\""),"+ECFGGET");

                if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
                    logD(RFX_LOG_TAG, "Query user agent error: %d", p_response->getError());
                    break;
                }

                line = p_response->getIntermediates();

                // Move to first
                line->atTokStart(&err);
                if (err < 0) break;;

                // Read Key
                char *key= line->atTokNextstr(&err);
                if (err < 0) break;;
                if (strcmp("user_agent", key) != 0) break;

                value = line->atTokNextstr(&err);
                if (err < 0) break;

                logD(RFX_LOG_TAG, "requestSetSipUserAgent value = %s, newValue = %s", value, newValue.string());

                // If Value is not changed
                if(strcmp(newValue.string(), value) == 0) {
                    rilErrNo = RIL_E_SUCCESS;
                    break;
                }

                // Send User-Agent Value to MD
                p_response = atSendCommand(
                        String8::format("AT+ECFGSET=\"user_agent\",\"%s\"", newValue.string()));

                if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
                    logE(RFX_LOG_TAG, "Set user agent error: %d", p_response->getError());
                    break;
                }

                rilErrNo = RIL_E_SUCCESS;
            }
        }
    }

    if (rilErrNo == RIL_E_SUCCESS) {
        userAgentformat = format;
        logI(RFX_LOG_TAG, "requestSetSipUserAgent update userAgentformat to %s", userAgentformat.string());
    }

    logD(RFX_LOG_TAG, "requestSetSipUserAgent done (isReset = %d).", isReset);
}

bool RmcImsControlRequestHandler::isCurrentSlotSupportIms() {
    bool result = false;

    char isImsSupport[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.ims_support", isImsSupport, "0");

    if (atoi(isImsSupport) == 0) {
        return result;
    }

    char imsCount[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.mims_support", imsCount, "1");

    if (atoi(imsCount) == 1) {
        if (RfxRilUtils::getMajorSim() == (m_slot_id + 1)) {
            result = true;
        }
    } else {
        int protocalStackId = RfxRilUtils::getProtocolStackId(m_slot_id);

        if (protocalStackId <= atoi(imsCount)) {
            result = true;
        }
    }
    return result;
}

void RmcImsControlRequestHandler::queryVopsStatus(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int responses[2] = {0};
    sp<RfxMclMessage> rsp = NULL;

    /**
     * Query IMS network reporting
     * AT+CIREP?
     * +CIREP: <reporting>,<nwimsvops>
     */
    p_response = atSendCommandSingleline("AT+CIREP?", "+CIREP:");

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(RFX_LOG_TAG, "isVopsOn Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            break;
        default:     // AT CMD format error, should not be here
            goto error;
    }

    if (p_response->getIntermediates() != NULL) {
        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <reporting> : integer
         * Enables or disables reporting of changes in the IMSVOPS supported
         * indication received from the network and reporting of PS to CS SRVCC,
         * PS to CS vSRVCC and CS to PS SRVCC handover information.
         *
         * 0   Disable reporting
         * 1   Enable reporting
         */
        responses[0] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <nwimsvops> : integer
         * Gives the last IMS Voice Over PS sessions (IMSVOPS) supported
         * indication received from network.
         *
         * 0   IMSVOPS support indication is not received from network, or is negative
         * 1   IMSVOPS support indication as received from network is positive
         */
        responses[1] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
    }

    ret = RIL_E_SUCCESS;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxIntsData(responses, 2), msg, false);

    // response to TeleCore
    responseToTelCore(rsp);

    return;

error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxIntsData(responses, 2), msg, false);

    // response to TeleCore
    responseToTelCore(rsp);
}

void RmcImsControlRequestHandler::enableVopsStatusReport() {
    sp<RfxAtResponse> p_response;
    // send AT command
    logD(RFX_LOG_TAG, "enableVopsStatusReport");
    p_response = atSendCommand(String8::format("AT+CIREP=1"));

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        logE(RFX_LOG_TAG, "Fail to enable VOPS status report error: %d", p_response->getError());
    }
}

void RmcImsControlRequestHandler::enableSipRegInfoInd() {
    sp<RfxAtResponse> p_response;

    // send AT command
    logD(RFX_LOG_TAG, "Enable +ESIPREGINFO report");
    p_response = atSendCommand(String8::format("AT+ECFGSET=\"report_eimsreginfo_urc\",\"1\""));
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
        logE(RFX_LOG_TAG, "Enable +ESIPREGINFO report error: %d", p_response->getError());
    }
}

