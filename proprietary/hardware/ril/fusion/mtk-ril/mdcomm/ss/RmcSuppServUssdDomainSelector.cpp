/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "RmcSuppServUssdDomainSelector.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxStringsData.h"
#include "RfxStringData.h"
#include "RfxMessageId.h"
#include "RfxRilUtils.h"
#include "GsmUtil.h"
#include "SSUtil.h"
#include "SSConfig.h"
#include "SuppServDef.h"
#include "rfx_properties.h"

#include <telephony/mtk_ril.h>
#include <mtkconfigutils.h>
#include <libmtkrilutils.h>
#include <mtk_properties.h>
#include <string.h>
#include <unistd.h>


RmcSuppServUssdDomainSelector::RmcSuppServUssdDomainSelector(int slot_id, int channel_id) :
    RmcSuppServUssdBaseHandler(slot_id, channel_id) {
    setUssiAction(USSI_REQUEST);
    mOngoingSessionDomain = 0;
    mUssiSnapshot = NULL;
}

RmcSuppServUssdDomainSelector::~RmcSuppServUssdDomainSelector() {
    if (mUssiSnapshot != NULL) {
        free(mUssiSnapshot);
        mUssiSnapshot = NULL;
    }
}

void RmcSuppServUssdDomainSelector::handleOnUssd(const sp<RfxMclMessage>& msg) {
    logD(TAG, "handleOnUssd, from UrcHandler");
    sp<RfxMclMessage> urc;
    const char **ussdUrcStrings = (const char**) (msg->getData()->getData());

    if (atoi(ussdUrcStrings[0]) == 1) {
        logD(TAG, "handleOnUssd, further user action required, mOngoingSessionDomain: CS");
        mOngoingSessionDomain = 0;
    }

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_ON_USSD, m_slot_id,
            RfxStringsData(msg->getData()->getData(), msg->getData()->getDataLength()));
    responseToTelCore(urc);
}

void RmcSuppServUssdDomainSelector::handleOnUssi(const sp<RfxMclMessage>& msg) {
    logD(TAG, "handleOnUssi, from UrcHandler");

    sp<RfxMclMessage> newUssdUrc = convertUssiToUssdUrc(msg);
    int ussdMode = atoi(((char **) newUssdUrc->getData()->getData())[0]);

    /**
     * USSD mode meanings
     *  0 : USSD_MODE_NOTIFY (0)
     *  1 : USSD_MODE_REQUEST (1)
     *  2 : USSD_MODE_NW_RELEASE (2)
     *  3 : USSD_MODE_LOCAL_CLIENT (3)
     *  4 : USSD_MODE_NOT_SUPPORTED (4)
     *  5 : USSD_MODE_NW_TIMEOUT (5)
     *  6 : USSD_MODE_NOT_SUPPORTED_NO_CSFB (6)
     */
    if (ussdMode != 4) {
        logD(TAG, "handleOnUssi, no need to do CSFB, ussdMode = %d", ussdMode);

        if (ussdMode == 1) {
            logD(TAG, "handleOnUssi, further user action required, mOngoingSessionDomain: IMS");
            mOngoingSessionDomain = 1;
        } else {
            mOngoingSessionDomain = 0;
        }
        responseToTelCore(newUssdUrc);
    } else {
        int wfcState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_WFC_STATE, -1);
        if (isInImsCall() && wfcState != 1) {
            logD(TAG, "handleOnUssi, do not CSFB, isInImsCall = true");
            responseToTelCore(newUssdUrc);
        } else {
            logD(TAG, "handleOnUssi, need to do CSFB, ussdMode = %d", ussdMode);
            handleUssiCSFB(newUssdUrc);
        }
        mOngoingSessionDomain = 0;
    }
}

/**
 * Handle USSI CSFB, param "ussi" is the string need to do CSFB
 */
void RmcSuppServUssdDomainSelector::handleUssiCSFB(const sp<RfxMclMessage>& msg) {
    if (mUssiSnapshot != NULL) {
        logD(TAG, "handleUssiCSFB, mUssiSnapshot = %s", mUssiSnapshot);

        RfxStringData strData((void *) mUssiSnapshot, strlen(mUssiSnapshot));
        sp<RfxMclMessage> tmpMsg = RfxMclMessage::obtainRequest(
                RFX_MSG_REQUEST_SEND_USSD,
                &strData,
                msg->getSlotId(),
                msg->getToken(),
                msg->getSendToMainProtocol(),
                msg->getRilToken(),
                msg->getTimeStamp(),
                msg->getAddAtFront());
        requestSendUSSD(tmpMsg, SEND_URC_BACK);

        free(mUssiSnapshot);
        mUssiSnapshot = NULL;
    } else {
        logE(TAG, "handleUssiCSFB fails, mUssiSnapshot is NULL");
    }
}

void RmcSuppServUssdDomainSelector::requestSendUSSD(const sp<RfxMclMessage>& msg,
        UssdReportCase reportCase) {
    logD(TAG, "requestSendUSSD, reportCase = %d", reportCase);

    sp<RfxAtResponse> p_response;
    int err = 0;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int strLen = 0;
    char* pTmpStr = NULL;
    char* p_ussdRequest = NULL;
    char* p_input_ussdRequest = (char*) msg->getData()->getData();

    if (p_input_ussdRequest == NULL || strlen(p_input_ussdRequest) == 0) {
        logE(TAG, "requestSendUSSD:p_ussdRequest null or empty.");
        goto error;
    }
    p_ussdRequest = convertToUCS2(p_input_ussdRequest);

    if (p_ussdRequest == NULL) {
        logE(TAG, "requestSendUSSD:p_ussdRequest malloc fail");
        goto error;
    }

    /**
     * AT+ECUSD=<m>,<n>,<str>,<dcs>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     * 01xx    General Data Coding indication
     *
     * Bits 5..0 indicate the following:
     *   Bit 5, if set to 0, indicates the text is uncompressed
     *   Bit 5, if set to 1, indicates the text is compressed using the compression algorithm defined in 3GPP TS 23.042 [13]
     *
     *   Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class meaning
     *   Bit 4, if set to 1, indicates that bits 1 to 0 have a message class meaning:
     *
     *     Bit 1   Bit 0       Message Class:
     *       0       0           Class 0
     *       0       1           Class 1 Default meaning: ME-specific.
     *       1       0           Class 2 (U)SIM specific message.
     *       1       1           Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
     *
     *   Bits 3 and 2 indicate the character set being used, as follows:
     *
     *     Bit 3   Bit 2       Character set:
     *       0       0           GSM 7 bit default alphabet
     *       0       1           8 bit data
     *       1       0           UCS2 (16 bit) [10]
     *       1       1           Reserved
     */
    //BEGIN mtk08470 [20130109][ALPS00436983]
    // USSD string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
    // We convert input char to unicode hex string and store it to p_ussdRequest.
    // For example, convert input "1" to "3100"; So len of p_ussdRequest is 4 times of input
    strLen = strlen(p_ussdRequest)/4;
    if (strLen > MAX_RIL_USSD_NUMBER_LENGTH) {
        logW(TAG, "USSD stringlen = %d, max = %d", strLen, MAX_RIL_USSD_NUMBER_LENGTH);
        strLen = MAX_RIL_USSD_NUMBER_LENGTH;
    }
    pTmpStr = (char*) calloc(1, (4*strLen+1));
    if(pTmpStr == NULL) {
        logE(TAG, "Malloc fail");
        free((char *)p_ussdRequest);
        goto error;
    }
    memcpy(pTmpStr, p_ussdRequest, 4*strLen);
    //END mtk08470 [20130109][ALPS00436983]
    asprintf(&cmd, "AT+ECUSD=2,1,\"%s\",72", pTmpStr); /* <dcs> = 0x48 */

    p_response = atSendCommand(cmd);

    free(cmd);
    free(pTmpStr);
    free((char *)p_ussdRequest);

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestSendUSSD Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        default:
            ret = RIL_E_INTERNAL_ERR;
            break;
    }

error:
    if (reportCase == SEND_RESPONSE_BACK) {
        logD(TAG, "requestSendUSSD: send response back to framework");

        if (ret == RIL_E_SUCCESS) {
            sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
            responseToTelCore(response);
        } else {
            sendFailureReport(msg, ret);
        }
    } else if (reportCase == SEND_URC_BACK) {
        if (ret != RIL_E_SUCCESS) {
            logD(TAG, "requestSendUSSD: send urc back to framework");
            sp<RfxMclMessage> ussdUrcMsg;
            char *genericUssdFail[2] = {(char *) "4", (char *) ""};

            ussdUrcMsg = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_ON_USSD,
                    m_slot_id, RfxStringsData(genericUssdFail, 2));
            responseToTelCore(ussdUrcMsg);
        } else {
            logD(TAG, "requestSendUSSD: no need to send anything to framework");
        }
    } else {
        logD(TAG, "requestSendUSSD: no need to send anything to framework");
    }

}

void RmcSuppServUssdDomainSelector::requestCancelUssd(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    p_response = atSendCommand("AT+ECUSD=2,2");
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) {
        logE(TAG, "Cancel USSD failed.");
    } else {
        ret = RIL_E_SUCCESS;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcSuppServUssdDomainSelector::requestSendUSSI(const sp<RfxMclMessage>& msg) {
    const char* ussi = (const char*) (msg->getData()->getData());
    sp<RfxAtResponse> p_response;
    int action = (getUssiAction() == USSI_REQUEST) ? 1 : 2;
    String8 currentMccmnc = getMclStatusManager()->
            getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC, String8("0"));
    logD(TAG, "requestSendUSSI: action = %d, ussi = %s", action, ussi);

    // Snapshot the USSI string in USSD domain selector
    if (mUssiSnapshot != NULL) {
        free(mUssiSnapshot);
        mUssiSnapshot = NULL;
    }

    mUssiSnapshot = strdup(ussi);
    if (mUssiSnapshot == NULL) {
        logE(TAG, "requestSendUSSI: mUssiSnapshot strdup fail");
    }

    int err;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;

    if (SSConfig::ussiWithNoLang(currentMccmnc.string())) {
        p_response = atSendCommand(String8::format("AT+EIUSD=2,1,%d,\"%s\",\"\",0",
                action, ussi));
    } else {
        p_response = atSendCommand(String8::format("AT+EIUSD=2,1,%d,\"%s\",\"en\",0",
                action, ussi));
    }

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "requestSendUSSI Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
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
        default:
            ret = RIL_E_INTERNAL_ERR;
            break;
    }

    if (strStartsWith(p_response->getFinalResponse()->getLine(), "ERROR")) {
        logE(TAG, "requestSendUSSI Error.");
        ret = RIL_E_INTERNAL_ERR;
    }

error:
    if (ret == RIL_E_SUCCESS) {
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
        responseToTelCore(response);
    } else {
        // CSFB if INTERNAL_ERR occurs and no "VoLTE call" ongoing. (WFC Call is allowed)
        int wfcState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_WFC_STATE, -1);
        if (ret == RIL_E_INTERNAL_ERR) {
            if (wfcState == 1) {
                logD(TAG, "requestSendUSSI: generic fail, do CSFB directly (camps on WFC)");
                requestSendUSSD(msg, SEND_RESPONSE_BACK);
                return;
            } else {
                if (!isInImsCall()) {
                    logD(TAG, "requestSendUSSI: generic fail, do CSFB directly (no VoLTE Call)");
                    requestSendUSSD(msg, SEND_RESPONSE_BACK);
                    return;
                }
            }
        }
        sendFailureReport(msg, ret);
    }
}

void RmcSuppServUssdDomainSelector::requestCancelUssi(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;

    p_response = atSendCommand("AT+EIUSD=2,2,2,\"\",\"en\",0");

    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) {
        logD(TAG, "Cancel USSD failed.");
    } else {
        ret = RIL_E_SUCCESS;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);

    // Reset mUssiAction to USSI_REQUEST
    setUssiAction(USSI_REQUEST);
}

/**
 * Do USSD domain selection
 */
void RmcSuppServUssdDomainSelector::requestSendUssdDomainSelect(const sp<RfxMclMessage>& msg) {
    const char* ussd = (const char*) (msg->getData()->getData());
    int wfcState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_WFC_STATE, -1);
    String8 currentMccmnc = getMclStatusManager()->
            getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC, String8("0"));


    // Check input USSD string first
    if (ussd == NULL || strlen(ussd) == 0) {
        logD(TAG, "requestSendUssdDomainSelect: ussd null or empty.");
        sendFailureReport(msg, RIL_E_INVALID_ARGUMENTS);
        return;
    }

    // Some operators do not allow send USSI during call, it'll cause call dropped
    if (SSConfig::isNotSupportUSSIOverCall(currentMccmnc.string()) && isInImsCall()) {
        logD(TAG, "requestSendUssdDomainSelect: doesn't allow to send USSI during call");
        sendFailureReport(msg, RIL_E_OPERATION_NOT_ALLOWED);
        return;
    }

    // Check FDN
    //    Modem IMC / Call UA do not check FDN, AP need to make sure if FDN allow this
    //    USSD code or not by itself. (Actually, modem L4 does check FDN. But here, AP checks
    //    it anyway no matter what the domain it is)
    if (!isFdnAllowed(ussd)) {
        logD(TAG, "requestSendUssdDomainSelect: it's not allowed by FDN check");
        sendFailureReport(msg, RIL_E_FDN_CHECK_FAILURE);
        return;
    }

    if (isImsRegOn()) {
        if (wfcState == 1) {
            // VoPS works only for cellular netowrk.
            // If UE is registered on VoWiFi, no need to check VoPS.
            logD(TAG, "requestSendUssdDomainSelect: requestSendUSSI() over VoWiFi");
            requestSendUSSI(msg);
        } else {
            // Check VoPS.
            //    From spec IR.92 D.5.2, the domain selection rules follow voice call.
            //    So, if VoPS is disabled, we should not try USSD over IMS.
            if (!isVopsOn()) {
                logD(TAG, "requestSendUssdDomainSelect: it's not allowed by VoPS check, CSFB");
                requestSendUSSD(msg, SEND_RESPONSE_BACK);
            } else {
                logD(TAG, "requestSendUssdDomainSelect: requestSendUSSI() over VoLTE");
                requestSendUSSI(msg);
            }
        }
    } else {
        logD(TAG, "requestSendUssdDomainSelect: requestSendUSSD()");
        requestSendUSSD(msg, SEND_RESPONSE_BACK);
    }
}

/**
 * If using AOSP framework, need to convert "cancel" USSD reuqest to USSI request
 * when IMS is registered on.
 */
void RmcSuppServUssdDomainSelector::requestCancelUssdDomainSelect(const sp<RfxMclMessage>& msg) {
    logD(TAG, "requestCancelUssdDomainSelect: mOngoingSessionDomain: %s",
            (mOngoingSessionDomain == 0) ? "CS" : "IMS");
    if (mOngoingSessionDomain == 1) {
        requestCancelUssi(msg);
    } else {
        requestCancelUssd(msg);
    }

    // Reset the flag
    mOngoingSessionDomain = 0;
}

/**
 * The API is used for FDN check and emergnecy number approved.
 */
sp<RfxMclMessage> RmcSuppServUssdDomainSelector::convertUssiToUssdUrc(
        const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> newUssdUrcMsg;
    char *newUssdUrcStrings[2] = {0};
    const char **ussiUrcStrings = (const char**) (msg->getData()->getData());

    /**
     * USSI response from the network, or network initiated operation
     * +EIUSD: <m>,<n>,<str>,<lang>,<error_code>,<alertingpattern>,<sip_cause>
     * <m>:
     *    1   USSD notify
     *    2   SS notify
     *    3   MD execute result
     * <n>:  if m=1
     *    0   no further user action required
     *    1   further user action required
     *    2   USSD terminated by network
     *    3   other local client has responded
     *    4   operation not supported
     *    5   network time out
     * <n>:if m=3, value is return value of MD
     *    0   execute success
     *    1   common error
     *    2   IMS unregistered
     *    3   IMS busy
     *    4   NW error response
     *    5   session not exist
     *    6   NW not support(404)
     * <str>: USSD/SS string
     * <lang>: USSD language
     * <error_code> USSD error code in xml
     * <alertingpattern> alerting pattern of NW initiated INVITE
     * <sip_cause> sip error code
     */
    const char *m = ussiUrcStrings[0];
    const char *n = ussiUrcStrings[1];
    const char *error_code = ussiUrcStrings[4];

    if (DBG) {
        logD(TAG, "convertUssiToUssdUrc: m = %s, n = %s", m, n);
    }

    // Initialization
    setUssiAction(USSI_REQUEST);  // Every time UE receives an USSI URC from modem,
                                  // we recover the state of mUssiAction to USSI_REQUEST.
                                  // If we found the EIUSD URC is "further user action required",
                                  // then change it to USSI_RESPONSE.
    asprintf(&newUssdUrcStrings[0], "%d", 4);  // Set USSD mode as general USSD error (4) first,
                                               // e.g., not supported, time out, network error

    // USSD mode
    if (strcmp(m, "1") == 0) {
        if (strcmp(n, "0") == 0 || strcmp(n, "2") == 0) {
            asprintf(&newUssdUrcStrings[0], "%d", 0);  // USSD_MODE_NOTIFY (0)
                                                       // <m> = 1, <n> = 0, no further action
                                                       // <m> = 1, <n> = 2, terminated by network
        } else if (strcmp(n, "1") == 0) {
            asprintf(&newUssdUrcStrings[0], "%d", 1);  // USSD_MODE_REQUEST (1)
                                                       // <m> = 1, <n> = 1, further action required
            setUssiAction(USSI_RESPONSE);  // Pull up the flag to let USSD domain selector
                                           // know the action of "next" received USSD
                                           // is a response, not a request
        } else if (strcmp(n, "4") == 0) {
            /**
            * If AP receives <error_code> != 0 from modem when <m> = 1 & <n> = 4,
            * no need to CSFB
            *
            * From TS 24.390:
            * <error-code> is an integer. The following values are defined.
            * If the received value is not listed below, it must be treated as 1.
            * 1    error - unspecified
            * 2    language/alphabet not supported
            * 3    unexpected data value
            * 4    USSD-busy
            */
            if (strcmp(error_code, "0") != 0) {
                asprintf(&newUssdUrcStrings[0], "%d", 6);  // USSD_MODE_NOT_SUPPORTED_NO_CSFB (6)
                                                           // <m> = 1, <n> = 4, <error-code> != 0
                                                           // It means modem UA get normal response
                                                           // from IMS server, no need to CSFB
            } else {
                asprintf(&newUssdUrcStrings[0], "%d", 4);  // USSD_MODE_NOT_SUPPORTED (4)
                                                           // <m> = 1, <n> = 4, <error-code> = 0
                                                           // operation not supported
            }
        } else if (strcmp(n, "5") == 0) {
            if (mOngoingSessionDomain == 1) {
                asprintf(&newUssdUrcStrings[0], "%d", 5);  // USSD_MODE_NW_TIMEOUT (5)
                                                           // <m> = 1, <n> = 5, netowrk time out and
                                                           // modem has get response from IMS server
                                                           // before.
            } else {
                asprintf(&newUssdUrcStrings[0], "%d", 4);  // USSD_MODE_NOT_SUPPORTED (4)
                                                           // <m> = 1, <n> = 5, netowrk time out
                                                           // and this is the first response from
                                                           // the modem UA.
            }
            setUssiAction(USSI_REQUEST);   // Reset the flag to let USSD domain selector
                                           // know the action of "next" received USSD
                                           // is a request, not a response. Because the current
                                           // USSI session is over
        }
    } else if (strcmp(m, "3") == 0) {
        if (strcmp(n, "0") == 0) {
            asprintf(&newUssdUrcStrings[0], "%d", 0);  // USSD_MODE_NOTIFY (0)
                                                       // <m> = 3, <n> = 5, execute success
        }
    }

    // USSD string
    asprintf(&newUssdUrcStrings[1], "%s", ussiUrcStrings[2]);

    if (DBG) {
        logD(TAG, "convertUssiToUssdUrc: return ussdMode = %s", newUssdUrcStrings[0]);
    }

    // Change the message id from USSI URC to USSD URC
    newUssdUrcMsg = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_ON_USSD, m_slot_id,
            RfxStringsData(newUssdUrcStrings, 2));

    free(newUssdUrcStrings[0]);
    free(newUssdUrcStrings[1]);

    return newUssdUrcMsg;
}

UssiAction RmcSuppServUssdDomainSelector::getUssiAction() {
    logD(TAG, "getUssiAction(): mUssiAction = %s", ussiActionToString(mUssiAction));
    return mUssiAction;
}

void RmcSuppServUssdDomainSelector::setUssiAction(UssiAction action) {
    logD(TAG, "setUssiAction(): %s -> %s", ussiActionToString(mUssiAction),
            ussiActionToString(action));
    if (mUssiAction == action) {
        return;
    }
    mUssiAction = action;
}

const char *RmcSuppServUssdDomainSelector::ussiActionToString(UssiAction action) {
    switch (action) {
        case USSI_REQUEST:
            return "USSI_REQUEST";
        case USSI_RESPONSE:
            return "USSI_RESPONSE";
        default:
            // not possible here!
            return NULL;
    }
}
