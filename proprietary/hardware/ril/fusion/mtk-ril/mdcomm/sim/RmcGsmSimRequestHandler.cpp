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
#include "RmcSimBaseHandler.h"
#include "RmcCommSimDefs.h"
#include "RmcGsmSimRequestHandler.h"
#include "RmcCommSimRequestHandler.h"
#include "RmcSimRequestEntryHandler.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxSimMeLockCatData.h"
#include "RfxVoidData.h"
#include <telephony/mtk_ril.h>
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include <time.h>
#include <sys/time.h>
#include <libmtkrilutils.h>
#include <cutils/properties.h>

static const int ch1ReqList[] = {
    RFX_MSG_REQUEST_GET_IMSI,
    RFX_MSG_REQUEST_QUERY_FACILITY_LOCK,
    RFX_MSG_REQUEST_SET_FACILITY_LOCK,
    RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK,
    RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK,
    RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS,
    RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS,
};

static const int ch3ReqList[] = {
};

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxStringData, RFX_MSG_REQUEST_GET_IMSI);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxIntsData,
        RFX_MSG_REQUEST_QUERY_FACILITY_LOCK);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxIntsData,
        RFX_MSG_REQUEST_SET_FACILITY_LOCK);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData,
        RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxSimMeLockCatData,
        RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxIntsData,
        RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxIntsData,
        RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS);

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RmcGsmSimRequestHandler::RmcGsmSimRequestHandler(int slot_id, int channel_id) :
        RmcSimBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcGsmSimRequest"));
}

RmcGsmSimRequestHandler::~RmcGsmSimRequestHandler() {
}



const int* RmcGsmSimRequestHandler::queryTable(int channel_id, int *record_num) {

    if (channel_id == RIL_CMD_PROXY_1) {
        *record_num = sizeof(ch1ReqList)/sizeof(int);
        return ch1ReqList;
    } else if (channel_id == RIL_CMD_PROXY_3) {
        *record_num = sizeof(ch3ReqList)/sizeof(int);
        return ch3ReqList;
    } else {
        // No request registered!
    }

    return NULL;
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcGsmSimRequestHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;

    switch(request) {
        case RFX_MSG_REQUEST_GET_IMSI:
            {
                int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
                char** pStrings = (char**)(msg->getData()->getData());

                if (pStrings == NULL && (cardType & RFX_CARD_TYPE_SIM)) {
                    result = RmcSimBaseHandler::RESULT_NEED;
                } else if (pStrings != NULL) {
                    String8 aid((pStrings[0] != NULL)? pStrings[0] : "");
                    if (strncmp(aid.string(), "A0000000871002", 14) == 0 ||
                        (aid.isEmpty() && (cardType & RFX_CARD_TYPE_SIM))) {
                        result = RmcSimBaseHandler::RESULT_NEED;
                    }
                }
            }
            break;
        case RFX_MSG_REQUEST_QUERY_FACILITY_LOCK:
        case RFX_MSG_REQUEST_SET_FACILITY_LOCK:
            {
                int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
                char** pStrings = (char**)(msg->getData()->getData());
                char *pAid = ((request == RFX_MSG_REQUEST_QUERY_FACILITY_LOCK)?
                        pStrings[3] : pStrings[4]);
                String8 aid((pAid != NULL)? pAid : "");
                String8 facility(pStrings[0]);
                String8 facFd("FD");
                String8 facSc("SC");

                if (facility != facFd && facility != facSc) {
                    logD(mTag, "No need to check aid for call barring, cardType: %x", cardType);
                    result = RmcSimBaseHandler::RESULT_NEED;
                } else if (aid.isEmpty() && (cardType & RFX_CARD_TYPE_SIM)) {
                    result = RmcSimBaseHandler::RESULT_NEED;
                } else if (!aid.isEmpty()) {
                    if ((strncmp(aid.string(), "A0000000871002", 14) == 0)
                            || (strncmp(aid.string(), "A0000000871004", 14) == 0)) {
                        result = RmcSimBaseHandler::RESULT_NEED;
                    }
                }
            }
            break;
        case RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK:
        case RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK:
        case RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS:
            result = RmcSimBaseHandler::RESULT_NEED;
            break;
        default:
            logE(mTag, "Not support the request!");
            break;
    }

    return result;
}

void RmcGsmSimRequestHandler::handleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_GET_IMSI:
            handleGetImsi(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_FACILITY_LOCK:
            handleQueryFacilityLock(msg);
            break;
        case RFX_MSG_REQUEST_SET_FACILITY_LOCK:
            handleSetFacilityLock(msg);
            break;
        case RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK:
            handleSetSimNetworkLock(msg);
            break;
        case RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK:
            handleQuerySimNetworkLock(msg);
            break;
        case RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS:
            handleReqSubsidyLockUpdateSettings(msg);
            break;
        case RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS:
            handleReqSubsidyLockGetModemStatus(msg);
            break;
        default:
            logE(mTag, "Not support the request!");
            break;
    }
}

void RmcGsmSimRequestHandler::handleReqSubsidyLockUpdateSettings(const sp<RfxMclMessage>& msg) {
    char property_value_status[RFX_PROPERTY_VALUE_MAX] = {0};
    String8 pString((char*)(msg->getData()->getData()));
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxAtResponse> p_response1 = NULL;
    sp<RfxAtResponse> p_response2 = NULL;

    sp<RfxMclMessage> response;
    RIL_Errno ril_error = RIL_E_GENERIC_FAILURE;
    int result[2] = {0};
    result[0] = 1;    // SUBSIDYLOCK COMMAND = 1

    // parse blob status to get lock status bit. 0 : SUBSIDYLOCK, 1: PERMANENT_UNLOCK
    int blob_status = (pString.string())[7] - '0';
    logD(mTag, "[SUBSIDY_LOCK]handleUpdateSimLockSettings update blob_status to: %d, strings %s\n",
            blob_status, pString.string());

    if (blob_status == 0) {
        logD(mTag, "[SUBSIDY_LOCK]blob update request to SUBSIDYLOCK");
    } else {
        logD(mTag, "[SUBSIDY_LOCK]blob update request to PERMANENT_UNLOCK");
    }

    cmd.append(String8::format("AT+ESLBLOB=1,%d,\"%s\"",
            (unsigned int)((pString.size() + 1)), pString.string()));

    p_response = atSendCommand(cmd.string());
    cmd.clear();
    err = p_response->getError();
    if (err < 0) {
        logD(mTag, "handleReqSubsidyLockUpdateSettings fail.\n");
        goto done;
    }

    if (p_response->getSuccess() == 0) {
        switch (p_response->atGetCmeError()) {
            logD(mTag, "p_response %d\n", p_response->atGetCmeError());
            case CME_BLOB_VALIDATION_FAILED:
                ril_error = RIL_E_VALIDATION_FAILED;
                logD(mTag, "p_response: CME_BLOB_VALIDATION_FAILED\n");
                break;
            case CME_BLOB_CONFIG_DATA_INVALID:
                ril_error = RIL_E_CONFIG_DATA_INVALID;
                logD(mTag, "p_response: CME_BLOB_CONFIG_DATA_INVALID\n");
                break;
            case CME_BLOB_IMEI_MISMATCH:
                ril_error = RIL_E_IMEI_MISMATCH;
                logD(mTag, "p_response: CME_BLOB_IMEI_MISMATCH\n");
                break;
            case CME_BLOB_LENGTH_MISMATCH:
                ril_error = RIL_E_LENGTH_MISMATCH;
                logD(mTag, "p_response: CME_BLOB_LENGTH_MISMATCH\n");
                break;
            case CME_UNKNOWN_ERROR:
                ril_error = RIL_E_UNKNOWN_ERROR;
                logD(mTag, "p_response: CME_UNKNOWN_ERROR\n");
                break;
            case CME_REBOOT_REQUIRED:
                ril_error = RIL_E_SUCCESS;
                logD(mTag, "p_response: CME_REBOOT_REQUIRED\n");
                cmd.append(String8::format("AT+EBTSAP=0"));
                p_response1 = atSendCommand(cmd.string());
                err = p_response1->getError();
                if (err < 0 || p_response1->getSuccess() == 0) {
                    logE(mTag, "AT+EBTSAP=0 Fail");
                   // ril_error = RIL_E_UNKNOWN_ERROR;
                }
                cmd.clear();

                cmd.append(String8::format("AT+EBTSAP=1"));
                p_response2 = atSendCommand(cmd.string());
                err = p_response2->getError();
                if (err < 0 || p_response2->getSuccess() == 0) {
                    logE(mTag, "AT+EBTSAP=1 Fail");
                    // ril_error = RIL_E_UNKNOWN_ERROR;
                }
                cmd.clear();
                break;
            default:
                goto done;
        }
    } else {
        logE(mTag, "handleReqSubsidyLockUpdateSettings success, E_REBOOT_NOT_REQUIRED\n");
        ril_error = RIL_E_SUCCESS;
    }

    if (ril_error == RIL_E_SUCCESS) {
        logE(mTag, "update blob_status property");
                if (blob_status == 1) {
                    // PERMANENT UNLOCK CASE:
            // property_set("persist.subsidylock.lockstatus_ril", "0");
                } else {
                    // SUBSIDYLOCK CASE:
            // property_set("persist.subsidylock.lockstatus_ril", "1");
        }
    }

done:
    result[1] = (int) ril_error;
    logD(mTag, "reqType = %d , result[1] = %d, ril_error = %d", result[0], result[1], ril_error);
    response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                            RfxIntsData((void*)result, 2*sizeof(int)), msg, false);
    responseToTelCore(response);
}

void RmcGsmSimRequestHandler::handleReqSubsidyLockGetModemStatus(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ril_error = RIL_E_GENERIC_FAILURE;
    int result[2] = {0};
    result[0] = 2;  // SUBSIDYLOCK COMMAND = 2
    result[1] = RIL_E_GENERIC_FAILURE;
    logD(mTag, "[SUBSIDY_LOCK]handleReqSubsidyLockGetModemStatus\n");

    cmd.append(String8::format("AT+ESLBLOB=2"));
    p_response = atSendCommandSingleline(cmd, "+ESLBLOB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "handleReqSubsidyLockGetModemStatus fail.\n");
        goto done;
    }

    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get status
    result[1] = line->atTokNextint(&err);
    if (err < 0) {
        result[1] = (int) ril_error;
        goto done;
    }

    logD(mTag, "modem status err = %d , status = %d", err, result[1]);

    ril_error = RIL_E_SUCCESS;
done:
    logD(mTag, "[SUBSIDY_LOCK]done.\n");
    if (p_response != NULL) {
        // logD(mTag, "[SUBSIDY_LOCK]status %s.\n", result[1]);
        response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                RfxIntsData((void*)result, 2*sizeof(int)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ril_error,
                RfxIntsData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcGsmSimRequestHandler::handleGetImsi(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response = NULL;
    int err;
    RfxStringData *pRspData = NULL;
    sp<RfxMclMessage> response;

    p_response = atSendCommandNumeric("AT+CIMI");

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxStringData(NULL, 0), msg, false);
    } else {
        String8 imsi(p_response->getIntermediates()->getLine());
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_IMSI, imsi);

        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxStringData((void*)imsi.string(), imsi.length()), msg, false);
    }
    responseToTelCore(response);
}

void RmcGsmSimRequestHandler::handleQueryFacilityLock(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response = NULL;
    int err, count = 0, enable = -1;
    int appTypeId = -1;
    int fdnServiceResult = -1;
    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
    RfxStringData *pRspData = NULL;
    char** pStrings = (char**)(msg->getData()->getData());
    String8 facFd("FD");
    String8 facSc("SC");
    String8 facility(pStrings[0]);
    String8 aid((pStrings[3] != NULL)? pStrings[3] : "");
    String8 cmd("");
    RfxAtLine *line = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ril_error = RIL_E_SIM_ERR;

    int currSimInsertedState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);

    if (facility.isEmpty()) {
        logE(mTag, "The facility string is empty.");
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
            if (cardType & RFX_CARD_TYPE_USIM) {
                cmd.append(String8::format("AT+ESSTQ=%d,%d", 1, 2));
            } else {
                cmd.append(String8::format("AT+ESSTQ=%d,%d", 0, 3));
            }
            p_response = atSendCommandSingleline(cmd, "+ESSTQ:");
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

        if(count == 13) {
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

void RmcGsmSimRequestHandler::handleSetSimNetworkLock(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    int err = -1;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    sp<RfxMclMessage> response;
    sp<RfxAtResponse> p_response = NULL;
    UICC_Status sim_status;
    char** strings = (char**)(msg->getData()->getData());
    char* key = NULL;
    char* imsi = NULL;
    char* gid1 = NULL;
    char* gid2 = NULL;

    if (strings == NULL || strings[0] == NULL ||
            strings[1] == NULL) {
         ret = RIL_E_INVALID_ARGUMENTS;
         logD(mTag, "handleSetSimNetworkLock invalid arguments.");
         goto error;
    }

    // strings[0]: cat
    // strings[1]: op
    key = (strings[2] == NULL) ? (char*)"" : (char*)strings[2];
    imsi = (strings[3] == NULL) ? (char*)"" : (char*)strings[3];
    gid1 = (strings[4] == NULL) ? (char*)"" : (char*)strings[4];
    gid2 = (strings[5] == NULL) ? (char*)"" : (char*)strings[5];

    logD(mTag, "simNetworkLock strings %s,%s,%s,%s,%s,%s \n",
            strings[0], strings[1], key, imsi, gid1, gid2);
    if(0 == strcmp (strings[1],"2")) { //add data
        if ((0 == strcmp(strings[0], "2")) || (0 == strcmp(strings[0], "5"))) {
            cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\",\"%s\",\"%s\"",
                    strings[0], strings[1], key, imsi, gid1));
        } else if ((0 == strcmp(strings[0], "3")) || (0 == strcmp(strings[0], "6"))) {
            cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\",\"%s\",\"%s\",\"%s\"",
                    strings[0], strings[1], key, imsi, gid1, gid2));
        } else {
            logD(mTag, "add data.");
            cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\",\"%s\"",
                    strings[0], strings[1], key, imsi));
        }
    } else if (0 == strcmp (strings[1],"3") || //remove data
             0 == strcmp (strings[1],"4")) { //disable data
        cmd.append(String8::format("AT+ESMLCK=%s,%s", strings[0], strings[1]));
    } else if (0 == strcmp (strings[1],"0") ) { //unlock
        sim_status = getSimStatus();//getSIMLockStatus(rid);
        logD(mTag, "unlock.");
        if ((UICC_NP == sim_status && (0 == strcmp(strings[0], "0"))) ||
                (UICC_NSP == sim_status && (0 == strcmp(strings[0], "1")))  ||
                (UICC_SP == sim_status && (0 == strcmp(strings[0], "2"))) ||
                (UICC_CP == sim_status && (0 == strcmp(strings[0], "3"))) ||
                (UICC_SIMP == sim_status && (0 == strcmp(strings[0], "4"))) ||
                (UICC_NS_SP == sim_status && (0 == strcmp(strings[0], "5"))) ||
                (UICC_SIM_C == sim_status && (0 == strcmp(strings[0], "6")))) {
            logE(mTag, "simsatus = %d, category = %s", sim_status, strings[0]);
            cmd.append(String8::format("AT+CPIN=\"%s\"", key));
            p_response = atSendCommand(cmd.string());
            err = p_response->getError();
            cmd.clear();
            if (err < 0) {
                logE(mTag, "err = %d", err);
                goto error;
            }
            if (p_response->getSuccess() == 0) {
                logE(mTag, "err = %d", p_response->atGetCmeError());
                switch (p_response->atGetCmeError()) {
                case CME_INCORRECT_PASSWORD:
                    goto error;
                    break;
                case CME_SUCCESS:
                    /* While p_response->success is 0, the CME_SUCCESS means CME ERROR:0 => it is phone failure */
                    goto error;
                    break;
                default:
                    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                            m_slot_id, RfxVoidData());
                    responseToTelCore(unsol);
                    cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key));
                    break;
                }
            } else {
                sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                        m_slot_id, RfxVoidData());
                responseToTelCore(unsol);
                cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key));
            }
        } else {
            cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key));
        }
    } else if (0 == strcmp (strings[1],"1")) { //lock
        cmd.append(String8::format("AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key));
    }
    p_response = atSendCommand(cmd.string());
    cmd.clear();
    logD(mTag, "network lock command sent.");
    err = p_response->getError();
    if (err < 0) {
        logE(mTag, "err = %d", err);
        goto error;
    }

    if (p_response->getSuccess() == 0) {
        logE(mTag, "p_response err = %d", p_response->atGetCmeError());
        switch (p_response->atGetCmeError()) {
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_UNKNOWN:
                break;
            default:
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
error:
    response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                        RfxVoidData(), msg, false);
    responseToTelCore(response);
}
void RmcGsmSimRequestHandler::handleQuerySimNetworkLock(const sp<RfxMclMessage>& msg) {
    int i = 0;
    int err = -1;
    String8 cmd("");
    RfxAtLine *line = NULL;
    char *value, *tmpStr;
    RIL_SimMeLockInfo result;
    RIL_SimMeLockCatInfo lockstate;
    sp<RfxAtResponse> p_response = NULL;
    int* cat = (int*)(msg->getData()->getData());
    sp<RfxMclMessage> response;

    logD(mTag, "handleQuerySimNetworkLock cat: %d", cat[0]);

    if (cat[0] < 0 || cat[0] >= MAX_SIM_ME_LOCK_CAT_NUM) {
        goto error;
    }
    cmd.append(String8::format("AT+ESMLCK"));
    p_response = atSendCommandSingleline(cmd, "+ESMLCK:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if(err < 0) goto error;

    for (i = 0; i < MAX_SIM_ME_LOCK_CAT_NUM; i++) {
        value = line->atTokChar(&err);
        if(err < 0) goto error;
        result.catagory[i].catagory = line->atTokNextint(&err);
        if(err < 0) goto error;
        result.catagory[i].state = line->atTokNextint(&err);
        if(err < 0) goto error;
        result.catagory[i].retry_cnt = line->atTokNextint(&err);
        if(err < 0) goto error;
        result.catagory[i].autolock_cnt = line->atTokNextint(&err);
        if(err < 0) goto error;
        result.catagory[i].num_set = line->atTokNextint(&err);
        if(err < 0) goto error;
        result.catagory[i].total_set = line->atTokNextint(&err);
        if(err < 0) goto error;
        result.catagory[i].key_state = line->atTokNextint(&err);
        if(err < 0) goto error;
    }
    tmpStr = line->atTokNextstr(&err);
    if(err < 0) goto error;
    strncpy(result.imsi, tmpStr, 15);

    result.isgid1 = line->atTokNextint(&err);
    if(err < 0) goto error;

    tmpStr = line->atTokNextstr(&err);
    if(err < 0) goto error;
    strncpy(result.gid1, tmpStr, 15);

    result.isgid2 = line->atTokNextint(&err);
    if(err < 0) goto error;

    tmpStr = line->atTokNextstr(&err);
    if(err < 0) goto error;

    strncpy(result.gid2, tmpStr, 15);
    result.mnclength = line->atTokNextint(&err);
    if(err < 0) goto error;

    lockstate.catagory= result.catagory[cat[0]].catagory;
    lockstate.state = result.catagory[cat[0]].state;
    lockstate.retry_cnt= result.catagory[cat[0]].retry_cnt;
    lockstate.autolock_cnt = result.catagory[cat[0]].autolock_cnt;
    lockstate.num_set = result.catagory[cat[0]].num_set;
    lockstate.total_set = result.catagory[cat[0]].total_set;
    lockstate.key_state = result.catagory[cat[0]].key_state;

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxSimMeLockCatData((RIL_SimMeLockCatInfo*)&lockstate, sizeof(RIL_SimMeLockCatInfo)), msg, false);
    responseToTelCore(response);
    return;
error:
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxSimMeLockCatData(NULL, 0), msg, false);
    responseToTelCore(response);
}

void RmcGsmSimRequestHandler::handleSetFacilityLock(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response = NULL;
    int err, count = 0;
    int appTypeId = -1;
    int fdnServiceResult = -1;
    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
    int attemptsRemaining[4] = {0};
    RfxStringData *pRspData = NULL;
    char** pStrings = (char**)(msg->getData()->getData());
    String8 facFd("FD");
    String8 facSc("SC");
    String8 facility((pStrings[0] != NULL)? pStrings[0] : "");
    String8 lockStr((pStrings[1] != NULL)? pStrings[1] : "");
    String8 pwd((pStrings[2] != NULL)? pStrings[2] : "");
    String8 aid((pStrings[4] != NULL)? pStrings[4] : "");
    String8 cmd("");
    sp<RfxMclMessage> response;
    RmcSimPinPukCount *retry = NULL;
    RIL_Errno ril_error = RIL_E_SIM_ERR;

    if (facility.isEmpty()) {
        logE(mTag, "The facility string is empty.");
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

            sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                    m_slot_id, RfxVoidData());
            responseToTelCore(unsol);
        }

        if(count == 13) {
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

    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
            m_slot_id, RfxVoidData());
    responseToTelCore(unsol);
}

void RmcGsmSimRequestHandler::sleepMsec(long long msec) {
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep(&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

