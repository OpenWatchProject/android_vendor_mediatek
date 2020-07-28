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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <alloca.h>
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxViaUtils.h"
#include "RfxVoidData.h"
#include "RmcCdmaSimRequestHandler.h"
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>
#include "ratconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "usim_fcp_parser.h"
#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * Variable
 *****************************************************************************/
using ::android::String8;

static const int ch1ReqList[] = {
    RFX_MSG_REQUEST_GET_IMSI,
    RFX_MSG_REQUEST_CDMA_SUBSCRIPTION,
    RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE,
    RFX_MSG_REQUEST_QUERY_FACILITY_LOCK,
    RFX_MSG_REQUEST_SET_FACILITY_LOCK
};

// CDMA subscription source
#define SUBSCRIPTION_FROM_RUIM 0
#define SUBSCRIPTION_FROM_NV 1

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringsData, RFX_MSG_REQUEST_CDMA_SUBSCRIPTION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData,
    RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE);

/*****************************************************************************
 * Class RmcCdmaSimRequestHandler
 *****************************************************************************/
RmcCdmaSimRequestHandler::RmcCdmaSimRequestHandler(int slot_id, int channel_id) :
        RmcSimBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcCdmaSimRequest"));

    // Check if modem supports CDMA 3g dual activation. It is a const value set in modem and is
    // enough to query just once in a channel.
    if (channel_id == RIL_CMD_PROXY_1) {
        queryCdma3gDualActivation();
    }
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcCdmaSimRequestHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;

    switch (request) {
        case RFX_MSG_REQUEST_GET_IMSI:
            {
                char** pStrings = (msg->getData() != NULL) ? ((char**)(msg->getData()->getData()))
                        : NULL;
                String8 aid(((pStrings != NULL) && (pStrings[0] != NULL))? pStrings[0] : "");

                if (!aid.isEmpty()) {
                    if ((aid.find("A0000003431002") >= 0) || (aid.find("A000000000RUIM") >= 0)) {
                        result = RmcSimBaseHandler::RESULT_NEED;
                    }
                } else {
                    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE,
                            -1);
                    if ((cardType & RFX_CARD_TYPE_RUIM) || (cardType & RFX_CARD_TYPE_CSIM)) {
                        result = RmcSimBaseHandler::RESULT_NEED;
                    }
                }
            }
            break;
        case RFX_MSG_REQUEST_CDMA_SUBSCRIPTION:
        case RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
            result = RmcSimBaseHandler::RESULT_NEED;
            break;
        case RFX_MSG_REQUEST_QUERY_FACILITY_LOCK:
        case RFX_MSG_REQUEST_SET_FACILITY_LOCK:
            {
                char** pStrings = (msg->getData() != NULL) ? ((char**)(msg->getData()->getData()))
                        : NULL;
                char *pAid;
                if (pStrings != NULL) {
                    if (request == RFX_MSG_REQUEST_QUERY_FACILITY_LOCK) {
                        pAid = pStrings[3];
                    } else {
                        pAid = pStrings[4];
                    }
                } else {
                    pAid = NULL;
                }

                String8 aid((pAid != NULL)? pAid : "");

                if (!aid.isEmpty()) {
                    if ((aid.find("A0000003431002") >= 0) || (aid.find("A000000000RUIM") >= 0)) {
                        result = RmcSimBaseHandler::RESULT_NEED;
                    }
                } else {
                    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE,
                            -1);
                    if ((cardType & RFX_CARD_TYPE_RUIM) || (cardType & RFX_CARD_TYPE_CSIM)) {
                        result = RmcSimBaseHandler::RESULT_NEED;
                    }
                }
            }
            break;
        default:
            logE(mTag, "Not support the request!");
            break;
    }

    logD(mTag, "needHandle => req %d, result %d", request, result);
    return result;
}

void RmcCdmaSimRequestHandler::handleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();

    if ((getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA3G_SWITCH_CARD) ==
            AP_TRIGGER_SWITCH_SIM) ||
            (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA3G_SWITCH_CARD) ==
            GMSS_TRIGGER_SWITCH_SIM)) {
        // Ignore request if the CDMA 3G card is switched into SIM.
        logE(mTag, "Ignore the invalid request: %d .", request);

        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(request, RIL_E_SIM_ERR,
                RfxVoidData(), msg, false);
        responseToTelCore(response);

        return;
    }

    switch(request) {
        case RFX_MSG_REQUEST_GET_IMSI:
            handleGetImsi(msg);
            break;
        case RFX_MSG_REQUEST_CDMA_SUBSCRIPTION:
            handleCdmaSubscription(msg);
            break;
        case RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
            handleCdmaGetSubscriptionSource(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_FACILITY_LOCK:
            handleQueryFacilityLock(msg);
            break;
        case RFX_MSG_REQUEST_SET_FACILITY_LOCK:
            handleSetFacilityLock(msg);
            break;
        default:
            logE(mTag, "Not support the request!");
            break;
    }
}

const int* RmcCdmaSimRequestHandler::queryTable(int channel_id, int *record_num) {
    if (channel_id == RIL_CMD_PROXY_1) {
        *record_num = sizeof(ch1ReqList)/sizeof(int);
        return ch1ReqList;
    } else {
        // No request registered!
    }

    return NULL;
}

void RmcCdmaSimRequestHandler::handleGetImsi(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response = NULL;
    int err;
    sp<RfxMclMessage> response;
    int appTypeId = -1;
    String8 cmd("");
    String8 c2kimsi("");
    String8 cdmaSubscriberId("vendor.ril.uim.subscriberid");
    cdmaSubscriberId.append(String8::format(".%d", (m_slot_id + 1)));
    char** pStrings = (msg->getData() != NULL) ? ((char**)(msg->getData()->getData())) : NULL;
    String8 aid(((pStrings != NULL) && (pStrings[0] != NULL))? pStrings[0] : "");

    appTypeId = queryAppTypeId(aid);
    cmd.append(String8::format("AT+ECIMI=%d", appTypeId));
    p_response = atSendCommandNumeric(cmd);

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logE(mTag, "handleGetImsi fail!");

        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ERR,
                RfxStringData(NULL, 0), msg, false);
    } else {
        String8 c2kimsi(p_response->getIntermediates()->getLine());

        rfx_property_set(cdmaSubscriberId, c2kimsi.string());
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_C2K_IMSI, c2kimsi);

        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxStringData((void*)c2kimsi.string(), c2kimsi.length()), msg);
    }

    responseToTelCore(response);
}

void RmcCdmaSimRequestHandler::handleCdmaSubscription(const sp<RfxMclMessage>& msg) {
    char *p_response[5] = {NULL, NULL, NULL, NULL, NULL};
    RIL_Errno *result = (RIL_Errno *)calloc(1, sizeof(RIL_Errno));
    assert(result != NULL);

    logI(mTag, "handleCdmaSubscription!");
    // Query CDMA EFmlpl and EFmspl file.
    queryCdmaMlplAndMspl();

    if (RfxViaUtils::getViaHandler() != NULL) {
        RfxViaUtils::getViaHandler()->handleCdmaSubscription(this, p_response, result);
    } else {
        *result = RIL_E_REQUEST_NOT_SUPPORTED;
        logE(mTag, "handleCdmaSubscription RIL_E_REQUEST_NOT_SUPPORTED!");
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), *result,
            RfxStringsData((void*)p_response, sizeof(p_response)), msg);
    responseToTelCore(response);

    for (unsigned int i = 0; i < sizeof(p_response)/sizeof(p_response[0]); i++) {
        if (p_response[i] != NULL) {
            free(p_response[i]);
        }
    }

    free(result);
}

void RmcCdmaSimRequestHandler::handleCdmaGetSubscriptionSource(const sp<RfxMclMessage>& msg) {
    int err = 0;
    sp<RfxAtResponse> p_response = NULL;
    RfxAtLine *line = NULL;
    int state = 255;
    int source = -1;
    RIL_Errno result = RIL_E_SIM_ERR;

    if (!RatConfig_isC2kSupported()) {
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_REQUEST_NOT_SUPPORTED,
                RfxIntsData((void*)&source, sizeof(int)), msg);
        responseToTelCore(response);
        return;
    }

    do {
        p_response = atSendCommandSingleline("AT+EUIMCFG?", "+EUIMCFG");
        if ((p_response == NULL) || (p_response->getSuccess()== 0)) {
            break;
        }

        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            break;
        }

        state = line->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        // State value
        // 0: UIM disabled
        // 1: UIM enabled
        if (state == 1) {
            source = SUBSCRIPTION_FROM_RUIM;
        } else if (state == 0) {
            source = SUBSCRIPTION_FROM_NV;
        }
        logD(mTag, "handleCdmaGetSubscriptionSource State: %d,  source: %d", state, source);
        result = RIL_E_SUCCESS;
    } while (0);

    if (source == -1) {
        logE(mTag, "handleCdmaGetSubscriptionSource failed to get subscription source!");
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), result,
            RfxIntsData((void*)&source, sizeof(int)), msg);
    responseToTelCore(response);

    p_response = NULL;
}

void RmcCdmaSimRequestHandler::handleQueryFacilityLock(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response = NULL;
    int err, count = 0, enable = -1;
    int appTypeId = -1;
    int fdnServiceResult = -1;
    RfxStringData *pRspData = NULL;
    String8 facFd("FD");
    String8 facSc("SC");
    char** pStrings = (msg->getData() != NULL) ? ((char**)(msg->getData()->getData())) : NULL;
    String8 facility(((pStrings != NULL) && (pStrings[0] != NULL))? pStrings[0] : "");
    String8 aid(((pStrings != NULL) && (pStrings[3] != NULL))? pStrings[3] : "");
    String8 cmd("");
    RfxAtLine *line = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ril_error = RIL_E_SIM_ERR;

    if (facility != facFd && facility != facSc) {
        logE(mTag, "The fac is invalid");
        ril_error = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    // Get app type id
    appTypeId = queryAppTypeId(aid);

    if (appTypeId == -1) {
        ril_error = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    do {
        /* No network related query. CLASS is unnecessary */
        if (facility == facFd) {
            // Use AT+ESSTQ=<app>,<service> to query service table
            // 0:  Service not supported
            // 1:  Service supported
            // 2:  Service allocated but not activated
            if (appTypeId == UICC_APP_CSIM) {
                // CSIM
                cmd.append(String8::format("AT+ECSTQ=%d,%d", 1, 2));
            } else {
                // RUIM
                cmd.append(String8::format("AT+ECSTQ=%d,%d", 0, 3));
            }
            p_response = atSendCommandSingleline(cmd, "+ECSTQ:");
            cmd.clear();

            err = p_response->getError();
            // The same as AOSP. 0 - Available & Disabled, 1-Available & Enabled, 2-Unavailable.
            if (err < 0 || p_response->getSuccess() == 0) {
                logE(mTag, "Fail to query service table");
            } else {
                line = p_response->getIntermediates();

                line->atTokStart(&err);
                fdnServiceResult = line->atTokNextint(&err);
                if (err < 0) goto error;

                if (fdnServiceResult == 0) {
                    fdnServiceResult = 2;
                } else if (fdnServiceResult == 2) {
                    fdnServiceResult = 0;
                }
                logD(mTag, "FDN available: %d", fdnServiceResult);
            }
            p_response = NULL;
            line = NULL;
        }
        cmd.append(String8::format("AT+ECLCK=%d,\"%s\",2", appTypeId, facility.string()));

        p_response = atSendCommandSingleline(cmd, "+ECLCK:");
        err = p_response->getError();

        cmd.clear();

        if (err < 0) {
            logE(mTag, "getFacilityLock Fail");
            goto error;
        } else if (p_response->getSuccess() == 0) {
            switch (p_response->atGetCmeError()) {
                case CME_SIM_BUSY:
                    logD(mTag, "simFacilityLock: CME_SIM_BUSY");
                    sleepMsec(200);
                    count++;
                    p_response = NULL;
                    ril_error = RIL_E_SIM_BUSY;
                    break;
                case CME_SIM_PIN_REQUIRED:
                case CME_SIM_PUK_REQUIRED:
                    ril_error = RIL_E_PASSWORD_INCORRECT;
                    goto error;
                case CME_SIM_PIN2_REQUIRED:
                    ril_error = RIL_E_SIM_PIN2;
                    goto error;
                case CME_SIM_PUK2_REQUIRED:
                    ril_error = RIL_E_SIM_PUK2;
                    goto error;
                case CME_INCORRECT_PASSWORD:
                    ril_error = RIL_E_PASSWORD_INCORRECT;
                    goto error;
                case CME_PHB_FDN_BLOCKED:
                    ril_error = RIL_E_FDN_CHECK_FAILURE;
                    goto error;
                default:
                    logD(mTag, "simFacilityLock: default");
                    goto error;
            }
        } else {
            // Success
            line = p_response->getIntermediates();

            line->atTokStart(&err);
            if (err < 0) goto error;

            /* 0 disable 1 enable */
            enable = line->atTokNextint(&err);
            if (err < 0) goto error;

            ril_error = RIL_E_SUCCESS;
            if (fdnServiceResult == -1) {
                response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                        RfxIntsData((void*)&enable, sizeof(int)), msg, false);
            } else {
                if (fdnServiceResult == 1 && enable == 0) {
                    fdnServiceResult = 0;
                }
                logD(mTag, "final FDN result: %d", fdnServiceResult);
                response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                        RfxIntsData((void*)&fdnServiceResult, sizeof(int)), msg, false);
            }
            responseToTelCore(response);
        }

        if (count == 13) {
            logE(mTag, "Set Facility Lock: CME_SIM_BUSY and time out.");
            goto error;
        }
    } while (ril_error == RIL_E_SIM_BUSY);

    return;
error:
    response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
            RfxIntsData(), msg, false);
    responseToTelCore(response);
}

void RmcCdmaSimRequestHandler::handleSetFacilityLock(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response = NULL;
    int err, count = 0;
    int appTypeId = -1;
    int fdnServiceResult = -1;
    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
    int attemptsRemaining[4] = {0};
    RfxStringData *pRspData = NULL;
    String8 facFd("FD");
    String8 facSc("SC");
    char** pStrings = (msg->getData() != NULL) ? ((char**)(msg->getData()->getData())) : NULL;
    String8 facility(((pStrings != NULL) && (pStrings[0] != NULL))? pStrings[0] : "");
    String8 lockStr(((pStrings != NULL) && (pStrings[1] != NULL))? pStrings[1] : "");
    String8 pwd(((pStrings != NULL) && (pStrings[2] != NULL))? pStrings[2] : "");
    String8 aid(((pStrings != NULL) && (pStrings[4] != NULL))? pStrings[4] : "");
    String8 cmd("");
    sp<RfxMclMessage> response;
    RmcSimPinPukCount *retry = NULL;
    RIL_Errno ril_error = RIL_E_SIM_ERR;

    if (facility != facFd && facility != facSc) {
        logE(mTag, "The fac is invalid");
        ril_error = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    // Get app type id
    appTypeId = queryAppTypeId(aid);

    if (appTypeId == -1) {
        logE(mTag, "The appTypeId is invalid");
        ril_error = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    do {
        if (pwd.isEmpty()) {
            logE(mTag, "The password can't be empty");
            ril_error = RIL_E_PASSWORD_INCORRECT;
            goto error;
        }
        cmd.append(String8::format("AT+ECLCK=%d,\"%s\",%s,\"%s\"", appTypeId, facility.string(),
                lockStr.string(), pwd.string()));
        p_response = atSendCommand(cmd);
        err = p_response->getError();
        cmd.clear();

        if (err < 0) {
            logE(mTag, "set facility lock Fail");
            goto error;
        } else if (p_response->getSuccess() == 0) {
            switch (p_response->atGetCmeError()) {
                case CME_SIM_BUSY:
                    logD(mTag, "simFacilityLock: CME_SIM_BUSY");
                    sleepMsec(200);
                    count++;
                    p_response = NULL;
                    ril_error = RIL_E_SIM_BUSY;
                    break;
                case CME_SIM_PIN_REQUIRED:
                case CME_SIM_PUK_REQUIRED:
                    ril_error = RIL_E_PASSWORD_INCORRECT;
                    goto error;
                case CME_SIM_PIN2_REQUIRED:
                    ril_error = RIL_E_SIM_PIN2;
                    goto error;
                case CME_SIM_PUK2_REQUIRED:
                    ril_error = RIL_E_SIM_PUK2;
                    goto error;
                case CME_INCORRECT_PASSWORD:
                    ril_error = RIL_E_PASSWORD_INCORRECT;
                    goto error;
                case CME_PHB_FDN_BLOCKED:
                    ril_error = RIL_E_FDN_CHECK_FAILURE;
                    goto error;
                default:
                    logD(mTag, "simFacilityLock: default");
                    goto error;
            }
        } else {
            // Success
            logD(mTag, "Set facility lock successfully");
            ril_error = RIL_E_SUCCESS;

            /* SIM operation we shall return pin retry counts */
            retry = getPinPukRetryCount();
            attemptsRemaining[0] = retry->pin1;
            attemptsRemaining[1] = retry->pin2;
            attemptsRemaining[2] = retry->puk1;
            attemptsRemaining[3] = retry->puk2;
            if (facility == facFd) {
                attemptsRemaining[0] = retry->pin2;
            }
            free(retry);
            response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                    RfxIntsData((void*)attemptsRemaining, sizeof(int)), msg, false);
            responseToTelCore(response);
        }

        if (count == 13) {
            logE(mTag, "Set Facility Lock: CME_SIM_BUSY and time out.");
            goto error;
        }
    } while (ril_error == RIL_E_SIM_BUSY);

    return;
error:
    retry = getPinPukRetryCount();

    if (retry != NULL) {
        /* SIM operation we shall return pin retry counts */
        attemptsRemaining[0] = retry->pin1;
        attemptsRemaining[1] = retry->pin2;
        attemptsRemaining[2] = retry->puk1;
        attemptsRemaining[3] = retry->puk2;
        if (facility == facFd) {
            attemptsRemaining[0] = retry->pin2;
        }
        free(retry);
        response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                RfxIntsData((void*)attemptsRemaining, sizeof(int)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                RfxIntsData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCdmaSimRequestHandler::sleepMsec(long long msec) {
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep(&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

void RmcCdmaSimRequestHandler::queryCdma3gDualActivation() {
    char feature[] = "CDMA3G_DUAL_ACTIVATION";
    int support = getFeatureVersion(feature);

    // CDMA3G_DUAL_ACTIVATION Support, 0:disable, 1:enable
    if (support == 1) {
        rfx_property_set("vendor.ril.cdma.3g.dualact", String8::format("%d", support).string());
    } else {
        rfx_property_set("vendor.ril.cdma.3g.dualact", String8::format("%d", 0).string());
    }
}

void RmcCdmaSimRequestHandler::queryCdmaMlplAndMspl() {
    String8 cmd("");
    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
    sp<RfxAtResponse> p_response = NULL;
    RfxAtLine *line = NULL;
    int err = 0, intPara = 0;
    int channelId = 0;
    String8 result("");
    RIL_SIM_IO_Response sr;
    unsigned char *response = NULL;
    unsigned short  responseLen = 0;
    int mlplVer = -1, msplVer = -1;
    String8 prop("ril.csim.mlpl_mspl_ver");

    prop.append(String8::format("%d", m_slot_id));
    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));

    // if (!(cardType & RFX_CARD_TYPE_CSIM)) {
    //    logE(mTag, "queryCdmaMlplAndMspl Not support MLPL and MSPL!");
    //    goto error;
    // }

    p_response = atSendCommandSingleline("AT+ESIMAPP=2,0", "+ESIMAPP:");

    if (p_response != NULL && p_response->getSuccess() > 0) {
        // Success
        line = p_response->getIntermediates();
        line->atTokStart(&err);

        // application id
        intPara = line->atTokNextint(&err);

        // channel id
        channelId = line->atTokNextint(&err);
        if (channelId == 255) {
            logE(mTag, "channel id is 255!");
            goto error;
        }

        p_response = NULL;
        line = NULL;
    } else {
        logE(mTag, "queryCdmaMlplAndMspl Fail to get channel id!");
        goto error;
    }

    // Read EFmlpl
    cmd.append(String8::format("AT+ECRLA=%d,%d,%d,%d,%d,%d,%d,,\"%s\"", 2, channelId, 176, 20256,
            0/*P1*/, 0/*P2*/, 0/*P3*/, "7F105F3C"));
    p_response = atSendCommandSingleline(cmd.string(), "+ECRLA:");

    if (p_response != NULL && p_response->getSuccess() == 1) {
        // Success
        line = p_response->getIntermediates();

        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        sr.sw1 = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
        sr.sw2 = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        if (line->atTokHasmore()) {
            char *pRes = NULL;
            pRes = line->atTokNextstr(&err);
            if (err < 0) {
                goto error;
            }
            asprintf(&sr.simResponse, "%s", pRes);

            responseLen = hexStringToByteArray((unsigned char*)(sr.simResponse), &response);
            if (responseLen >=4) {
                mlplVer = (response[3] << 4) | response[4];
            } else {
                goto error;
            }
        }

        p_response = NULL;
        line = NULL;
        cmd.clear();
        if (response != NULL) {
            free(response);
            response = NULL;
        }
        responseLen = 0;
    }
    logI(mTag, "queryCdmaMlplAndMspl mlplVer: %04x, sr: %s, %02x, %02x", mlplVer, sr.simResponse,
            sr.sw1, sr.sw2);

    // Read EFmspl
    cmd.append(String8::format("AT+ECRLA=%d,%d,%d,%d,%d,%d,%d,,\"%s\"", 2, channelId, 176, 20257,
            0/*P1*/, 0/*P2*/, 0/*P3*/, "7F105F3C"));
    p_response = atSendCommandSingleline(cmd.string(), "+ECRLA:");

    if (p_response != NULL && p_response->getSuccess() == 1) {
        // Success
        line = p_response->getIntermediates();

        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        sr.sw1 = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
        sr.sw2 = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        if (line->atTokHasmore()) {
            char *pRes = NULL;
            pRes = line->atTokNextstr(&err);
            if (err < 0) {
                goto error;
            }
            asprintf(&sr.simResponse, "%s", pRes);

            responseLen = hexStringToByteArray((unsigned char*)(sr.simResponse), &response);
            if (responseLen >=4) {
                msplVer = (response[3] << 4) | response[4];
            } else {
                goto error;
            }
        }

        p_response = NULL;
        line = NULL;
        cmd.clear();
    }
    logI(mTag, "queryCdmaMlplAndMspl msplVer: %04x, sr: %s, %02x, %02x", msplVer, sr.simResponse,
            sr.sw1, sr.sw2);

    // Set property
    if ((mlplVer != -1) && (msplVer != -1)) {
        result.append(String8::format("%x,%x", mlplVer, msplVer));
        rfx_property_set(prop, result);
        logI(mTag, "queryCdmaMlplAndMspl result: %s", result.string());
        return;
    }

error:
    rfx_property_set(prop, ",");
    logE(mTag, "queryCdmaMlplAndMspl Fail to query MLPL and MSPL!");
}

