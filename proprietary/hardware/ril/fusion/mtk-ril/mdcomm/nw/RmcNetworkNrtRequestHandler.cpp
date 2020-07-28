/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "RmcNetworkNrtRequestHandler.h"
#include "rfx_properties.h"
#include "ViaBaseHandler.h"
#include "RfxViaUtils.h"
#include <libmtkrilutils.h>

static const int request[] = {
    RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS,
    RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT,
    RFX_MSG_RIL_REQUEST_START_NETWORK_SCAN
};

static const int events[] = {
    RFX_MSG_EVENT_SIMULATE_NETWORK_SCAN
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringsData, RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringsData, RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxNetworkScanData, RfxVoidData, RFX_MSG_RIL_REQUEST_START_NETWORK_SCAN);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxVoidData, RFX_MSG_EVENT_SIMULATE_NETWORK_SCAN);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcNetworkNrtRequestHandler, RIL_CMD_PROXY_8);

RmcNetworkNrtRequestHandler::RmcNetworkNrtRequestHandler(int slot_id, int channel_id) :
        RmcNetworkHandler(slot_id, channel_id),
        mPlmn_list_format(0) {
    int err;
    sp<RfxAtResponse> p_response;
    logV(LOG_TAG, "%s[%d] start", __FUNCTION__, slot_id);
    m_slot_id = slot_id;
    m_channel_id = channel_id;
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    registerToHandleEvent(events, sizeof(events)/sizeof(int));

    p_response = atSendCommand("AT+COPS=3,3");
    err = p_response->getError();
    if (err >= 0 || p_response->getSuccess() != 0) {
        mPlmn_list_format = 1;
    }

}

RmcNetworkNrtRequestHandler::~RmcNetworkNrtRequestHandler() {
}

void RmcNetworkNrtRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    // logD(LOG_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(msg->getId()));
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS:
            requestQueryAvailableNetworks(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
            requestQueryAvailableNetworksWithAct(msg);
            break;
        case RFX_MSG_RIL_REQUEST_START_NETWORK_SCAN:
            requestStartNetworkScan(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}


void RmcNetworkNrtRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch (id) {
        case RFX_MSG_EVENT_SIMULATE_NETWORK_SCAN:
            handleSimulatedNetworkScan();
            break;
        default:
            logE(LOG_TAG, "onHandleEvent, should not be here");
            break;
    }
}

void RmcNetworkNrtRequestHandler::requestQueryAvailableNetworks(const sp<RfxMclMessage>& msg) {
    int err, len, i, j, k, num, num_filter;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;
    char **response = NULL, **response_filter = NULL;
    char *tmp, *block_p = NULL;
    char *lacStr = NULL;
    unsigned int lac = 0;

    // logD(LOG_TAG, "requestQueryAvailableNetworks set plmnListOngoing flag");
    mPlmnListOngoing = 1;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, true);
    p_response = atSendCommandSingleline("AT+COPS=?", "+COPS:");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();
    // count the number of operator
    tmp = line->getLine();
    len = strlen(tmp);
    for(i = 0, num = 0, num_filter = 0; i < len ; i++ ) {
        // here we assume that there is no nested ()
        if (tmp[i] == '(') {
            num++;
            num_filter++;
        } else if (tmp[i] == ',' && tmp[i+1] == ',') {
            break;
        }
    }

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)
    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*) * num *4); // for string, each one is 20 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*4*MAX_OPER_NAME_LENGTH);
    lacStr = (char *) calloc(1, num* sizeof(char)*4+1);

    if (response == NULL || block_p == NULL || lacStr == NULL) {
        logE(LOG_TAG, "requestQueryAvailableNetworks calloc fail");
        goto error;
    }

    for (i = 0, j=0 ; i < num ; i++, j+=4) {
        /* get "(<stat>" */
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];

        switch(tmp[1]) {
        case '0':
            sprintf(response[j+3], "unknown");
            break;
        case '1':
            sprintf(response[j+3], "available");
            break;
        case '2':
            sprintf(response[j+3], "current");
            break;
        case '3':
            sprintf(response[j+3], "forbidden");
            break;
        default:
            logE(LOG_TAG, "The %d-th <stat> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }

        /* get long name*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
        sprintf(response[j+0], "%s", tmp);

        /* get short name*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
        sprintf(response[j+1], "%s", tmp);

        /* get <oper> numeric code*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
        sprintf(response[j+2], "%s", tmp);

        // ALPS00353868 START
        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if(mPlmn_list_format == 1) {
            /* get <lac> numeric code*/
                tmp = line->atTokNextstr(&err);
            if (err < 0) {
                logE(LOG_TAG, "No <lac> in +COPS response");
                goto error;
            }
            memcpy(&(lacStr[i*4]), tmp, 4);
            lac = (unsigned int) strtoul(tmp, NULL, 16);
        }
        // ALPS00353868 END

        len = strlen(response[j+2]);
        if (len == 5 || len == 6) {
            if (isOp08Support()) {
                // AT&T consider EONS/TS25 in result.
                err = getOperatorNamesFromNumericCodeByDisplay(
                        response[j+2], lac, response[j+0], response[j+1],
                        MAX_OPER_NAME_LENGTH, (DISPLAY_EONS | DISPLAY_TS25));
            } else {
                err = getOperatorNamesFromNumericCode(
                        response[j+2], lac, response[j+0], response[j+1], MAX_OPER_NAME_LENGTH);
            }
            if(err < 0) goto error;
        } else {
            logE(LOG_TAG, "The length of the numeric code is incorrect");
            goto error;
        }

        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // check if this plmn is redundant
        for (k=0; k < j; k+=4)
        {
            // compare numeric
            if (0 == strcmp(response[j+2], response[k+2])) {
                response[j+0] = response[j+1] = response[j+2] = response[j+3] = (char *)"";
                num_filter--;
                break;
            }
        }
    }

    // filter the response
    response_filter = (char**)calloc(1, sizeof(char*) * num_filter * 4);
    if (NULL == response_filter) {
        logE(LOG_TAG, "malloc response_filter failed");
        goto error;
    }

    for (i=0, j=0, k=0; i < num; i++, j+=4) {
        if (0 < strlen(response[j+2])) {
            response_filter[k+0] = response[j+0];
            response_filter[k+1] = response[j+1];
            response_filter[k+2] = response[j+2];
            response_filter[k+3] = response[j+3];
            k += 4;
        }
    }

    logD(LOG_TAG, "requestQueryAvailableNetworks sucess, clear plmnListOngoing and plmnListAbort flag");
    mPlmnListOngoing = 0;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, false);
    mPlmnListAbort = 0; /* always clear here to prevent race condition scenario */
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringsData(response_filter, num_filter*4), msg, false);
    // response to TeleCore
    responseToTelCore(resp);
    free(response);
    free(response_filter);
    free(block_p);
    free(lacStr);
    return;
error:
    // logE(LOG_TAG, "FREE!!");
    if (block_p) free(block_p);
    if (response) free(response);
    if (lacStr) free(lacStr);

    logE(LOG_TAG, "requestQueryAvailableNetworks must never return error when radio is on, plmnListAbort=%d",
            mPlmnListAbort);
    if (mPlmnListAbort == 1){
        resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_CANCELLED,
                RfxVoidData(), msg, false);
    } else {
        resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_MODEM_ERR,
                RfxVoidData(), msg, false);
    }
    mPlmnListOngoing = 0;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, false);
    mPlmnListAbort =0; /* always clear here to prevent race condition scenario */
    // response to TeleCore
    responseToTelCore(resp);
}

void RmcNetworkNrtRequestHandler::requestQueryAvailableNetworksWithAct(const sp<RfxMclMessage>& msg) {
    int err, len, i, j, num;
    char **response = NULL;
    char *tmp, *block_p = NULL;
    char *lacStr = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;
    unsigned int lac = 0;

    mPlmnListOngoing = 1;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, true);
    p_response = atSendCommandSingleline("AT+COPS=?", "+COPS:");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();
    // count the number of operator
    tmp = line->getLine();
    len = strlen(tmp);
    for(i = 0, num = 0; i < len ; i++ ) {
        // here we assume that there is no nested ()
        if (tmp[i] == '(') {
            num++;
        } else if (tmp[i] == ',' && tmp[i+1] == ',') {
            break;
        }
    }

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)
    line->atTokStart(&err);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*) * num *6); // for string, each one is 25 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*6*MAX_OPER_NAME_LENGTH);
    lacStr = (char *) calloc(1, num* sizeof(char)*4+1);

    if (response == NULL || block_p == NULL || lacStr == NULL) {
        logE(LOG_TAG, "requestQueryAvailableNetworksWithAct calloc fail");
        goto error;
    }

    for (i = 0, j=0 ; i < num ; i++, j+=6) {
        /* get "(<stat>" */
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];
        response[j+4] = &block_p[(j+4)*MAX_OPER_NAME_LENGTH];
        response[j+5] = &block_p[(j+5)*MAX_OPER_NAME_LENGTH];

        switch(tmp[1]) {
        case '0':
            sprintf(response[j+3], "unknown");
            break;
        case '1':
            sprintf(response[j+3], "available");
            break;
        case '2':
            sprintf(response[j+3], "current");
            break;
        case '3':
            sprintf(response[j+3], "forbidden");
            break;
        default:
            logE(LOG_TAG, "The %d-th <stat> is an invalid value!!! : %d", i, tmp[1]);
            goto error;
        }

        /* skip long name*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        /* skip short name*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        /* get <oper> numeric code*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
        sprintf(response[j+2], "%s", tmp);

        // ALPS00353868 START
        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if (mPlmn_list_format == 1) {
            /* get <lac> numeric code*/
            tmp = line->atTokNextstr(&err);
            if (err < 0){
            logE(LOG_TAG, "No <lac> in +COPS response");
                goto error;
            }
            memcpy(&(lacStr[i*4]), tmp, 4);
            lac = (unsigned int) strtoul(tmp, NULL, 16);
            sprintf(response[j+4], "%s", tmp);
        }
        // ALPS00353868 END

        len = strlen(response[j+2]);
        if (len == 5 || len == 6) {
            err = getOperatorNamesFromNumericCode(
                      response[j+2], lac, response[j+0], response[j+1], MAX_OPER_NAME_LENGTH);
            if (err < 0) goto error;
        } else {
            logE(LOG_TAG, "The length of the numeric code is incorrect");
            goto error;
        }

        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        switch(tmp[0]) {
        case '0':
            sprintf(response[j+5], "2G");
            break;
        case '2':
            sprintf(response[j+5], "3G");
            break;
        case '7':    //for  LTE
            sprintf(response[j+5], "4G");
            break;
        default:
            logE(LOG_TAG, "The %d-th <Act> is an invalid value!!! : %d", i, tmp[1]);
            goto error;
        }
    }

    logD(LOG_TAG, "requestQueryAvailableNetworksWithAct sucess, clear plmnListOngoing and plmnListAbort flag");
    mPlmnListOngoing = 0;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, false);
    mPlmnListAbort =0; /* always clear here to prevent race condition scenario */
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringsData(response, num*6), msg, false);
    // response to TeleCore
    responseToTelCore(resp);

    free(response);
    free(block_p);
    free(lacStr);
    return;
error:
    logE(LOG_TAG, "FREE!!");
    if (block_p) free(block_p);
    if (response) free(response);
    if (lacStr) free(lacStr);
    logV(LOG_TAG, "requestQueryAvailableNetworksWithAct must never return error when radio is on, plmnListAbort=%d",
            mPlmnListAbort);
    if (mPlmnListAbort == 1){
        resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_CANCELLED,
                RfxVoidData(), msg, false);
    } else {
        resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    }
    mPlmnListOngoing = 0;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, false);
    mPlmnListAbort =0; /* always clear here to prevent race condition scenario */
    responseToTelCore(resp);
}

void RmcNetworkNrtRequestHandler::requestStartNetworkScan(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> resp;
    RIL_Errno ril_err = RIL_E_INVALID_ARGUMENTS;
    RIL_NetworkScanRequest* p_args = (RIL_NetworkScanRequest*) msg->getData()->getData();
    logV(LOG_TAG, "requestStartNetworkScan type=%d", p_args->type);
    logV(LOG_TAG, "requestStartNetworkScan interval=%d", p_args->interval);
    if (p_args->interval < 5 || p_args->interval > 300) goto error;
    logV(LOG_TAG, "requestStartNetworkScan specifiers_length=%d", p_args->specifiers_length);
    if (p_args->specifiers_length <= 0 || p_args->specifiers_length > MAX_RADIO_ACCESS_NETWORKS)
        goto error;
    for (size_t i = 0; i < (p_args->specifiers_length); i++) {
        logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu]->radio_access_network=%d",
                i, p_args->specifiers[i].radio_access_network);
        logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu]->radio_access_network=%d",
                i, p_args->specifiers[i].bands_length);
        for (size_t j = 0; j < (p_args->specifiers[i].bands_length); j++) {
            if (p_args->specifiers[i].radio_access_network == 0x01) {
                logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu].bands.geran_bands[%zu]=%d",
                        i, j, p_args->specifiers[i].bands.geran_bands[j]);
            } else if (p_args->specifiers[i].radio_access_network == 0x02) {
                logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu].bands.utran_bands[%zu]=%d",
                        i, j, p_args->specifiers[i].bands.utran_bands[j]);
            } else if (p_args->specifiers[i].radio_access_network == 0x03) {
                logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu].bands.eutran_bands[%zu]=%d",
                    i, j, p_args->specifiers[i].bands.eutran_bands[j]);
            } else {
                logV(LOG_TAG, "Should not be here");
            }
        }
        logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu]->channels_length=%d",
                i, p_args->specifiers[i].channels_length);
        for (size_t j = 0; j < (p_args->specifiers[i].channels_length); j++) {
            logV(LOG_TAG, "requestStartNetworkScan p_args->specifiers[%zu]->channels[%zu]=%d",
                    i, j, p_args->specifiers[i].channels[j]);
        }
    }
    // for VTS NW scan case.
    if (p_args->maxSearchTime == 0 && p_args->incrementalResultsPeriodicity == 0) {
        p_args->maxSearchTime = 3600;
        p_args->incrementalResultsPeriodicity = 1;
    }
    logV(LOG_TAG, "requestStartNetworkScan maxSearchTime=%d", p_args->maxSearchTime);
    if (p_args->maxSearchTime < 70 || p_args->maxSearchTime > 3600) {
        if (p_args->maxSearchTime >= 60 && p_args->maxSearchTime < 70) ril_err = RIL_E_REQUEST_NOT_SUPPORTED;
        goto error;
    }
    logV(LOG_TAG, "requestStartNetworkScan incrementalResults=%d", p_args->incrementalResults);
    logV(LOG_TAG, "requestStartNetworkScan incrementalResultsPeriodicity=%d", p_args->incrementalResultsPeriodicity);
    if (p_args->incrementalResultsPeriodicity < 1 || p_args->incrementalResultsPeriodicity > 10) goto error;
    logV(LOG_TAG, "requestStartNetworkScan mccMncs_length=%d", p_args->mccMncs_length);
    for (size_t i = 0; i < (p_args->mccMncs_length); i++) {
        logV(LOG_TAG, "requestStartNetworkScan mccMncs[%zu]=%s", i, p_args->mccMncs[i]);
    }

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    responseToTelCore(resp);

    // send simulate network scan event
    sendEvent(RFX_MSG_EVENT_SIMULATE_NETWORK_SCAN, RfxVoidData(), RIL_CMD_PROXY_8, m_slot_id,
            -1, -1, ms2ns(200), MTK_RIL_REQUEST_PRIORITY_MEDIUM);
    return;

error:
    logE(LOG_TAG, "requestStartNetworkScan Invalid Args");
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_err,
                RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(resp);
    return;
}

void RmcNetworkNrtRequestHandler::handleSimulatedNetworkScan() {
    RIL_NetworkScanResult* result = (RIL_NetworkScanResult*) calloc(1, sizeof(RIL_NetworkScanResult));
    if (result == NULL) {
        logE(LOG_TAG, "handleSimulatedNetworkScan result = null");
        return;
    }
    memset(result, 0, sizeof(RIL_NetworkScanResult));
    result->status = COMPLETE; // for now we don't support PARTIAL.

    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> urc;
    RfxAtLine* line;
    int err = 0, len = 0, num = 0, mcc = 0, mnc = 0, mnc_len = 0;
    unsigned int lac = 0;
    bool isSuccess = false;
    char *tmp = NULL;
    char mccmnc[8] = {0};
    char longname[MAX_OPER_NAME_LENGTH] = {0};
    char shortname[MAX_OPER_NAME_LENGTH] = {0};

    if (isAPInCall() == true) {
        goto error;
    }
    mPlmnListOngoing = 1;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, true);
    p_response = atSendCommandSingleline("AT+COPS=?", "+COPS:");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();
    // count the number of operator
    tmp = line->getLine();
    len = strlen(tmp);
    for(int i = 0; i < len ; i++ ) {
        // here we assume that there is no nested ()
        if (tmp[i] == '(') {
            num++;
        } else if (tmp[i] == ',' && tmp[i+1] == ',') {
            break;
        }
    }
    result->network_infos_length = num;
    result->network_infos = (RIL_CellInfo_v12*) calloc(num, sizeof(RIL_CellInfo_v12));
    if (result->network_infos == NULL) goto error;
    memset(result->network_infos, 0, num*sizeof(RIL_CellInfo_v12));

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)
    line->atTokStart(&err);
    if (err < 0) goto error;

    for (int i = 0; i < num ; i++) {
        // clear the tmp value for this round.
        memset(mccmnc, 0, 8);
        mcc = 0;
        mnc = 0;
        lac = 0;
        mnc_len = 0;
        memset(longname, 0, MAX_OPER_NAME_LENGTH);
        memset(shortname, 0, MAX_OPER_NAME_LENGTH);

        /* get "(<stat>" */
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // 0: unknown, 1: available, 2: current, 3: forbidden
        if (tmp[1] == '2') {
            result->network_infos[i].registered = 1;
            result->network_infos[i].connectionStatus = PRIMARY_SERVING;
        } else {
            result->network_infos[i].registered = 0;
            result->network_infos[i].connectionStatus = NONE_SERVING;
        }

        /* skip long name*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        /* skip short name*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        /* get <oper> numeric code*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
        // we only have 7 space for string and 1 for end of string.
        // we need eAct to fill the result, so we keep it before we get eAct.
        snprintf(mccmnc, 7, "%s", tmp);

        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if (mPlmn_list_format == 1) {
            /* get <lac> numeric code*/
            tmp = line->atTokNextstr(&err);
            if (err < 0){
            logE(LOG_TAG, "No <lac> in +COPS response");
                goto error;
            }
            lac = (unsigned int) strtoul(tmp, NULL, 16);
        }

        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        switch(tmp[0]) {
        case '0':
            result->network_infos[i].cellInfoType = RIL_CELL_INFO_TYPE_GSM;
            break;
        case '2':
            result->network_infos[i].cellInfoType = RIL_CELL_INFO_TYPE_WCDMA;
            break;
        case '7':    //for  LTE
            result->network_infos[i].cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            break;
        default:
            result->network_infos[i].cellInfoType = RIL_CELL_INFO_TYPE_NONE;
            goto error;
        }

        len = strlen(mccmnc);
        if (len == 5) {
            mnc_len = 2;
            mcc = (mccmnc[0]-'0') * 100 + (mccmnc[1]-'0') * 10 + (mccmnc[2]-'0');
            mnc = (mccmnc[3]-'0') * 10 + (mccmnc[4]-'0');
        } else if (len == 6) {
            mnc_len = 3;
            mcc = (mccmnc[0]-'0') * 100 + (mccmnc[1]-'0') * 10 + (mccmnc[2]-'0');
            mnc = (mccmnc[3]-'0') * 100 + (mccmnc[4]-'0') * 10 + (mccmnc[5]-'0');
        } else {
            logE(LOG_TAG, "The length of the numeric code is incorrect");
            goto error;
        }

        // start to fill the network_info[i]
        switch(result->network_infos[i].cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM:
            result->network_infos[i].CellInfo.gsm.cellIdentityGsm.mcc = mcc;
            result->network_infos[i].CellInfo.gsm.cellIdentityGsm.mnc = mnc;
            result->network_infos[i].CellInfo.gsm.cellIdentityGsm.mnc_len = mnc_len;
            result->network_infos[i].CellInfo.gsm.cellIdentityGsm.lac = lac;
            getOperatorNamesFromNumericCode(
                    mccmnc, lac, longname, shortname, MAX_OPER_NAME_LENGTH);
            asprintf(&(result->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name),
                    "%s", longname);
            asprintf(&(result->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name),
                    "%s", shortname);
            break;
        case RIL_CELL_INFO_TYPE_WCDMA:
            result->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.mcc = mcc;
            result->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.mnc = mnc;
            result->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.mnc_len = mnc_len;
            result->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.lac = lac;
            getOperatorNamesFromNumericCode(
                    mccmnc, lac, longname, shortname, MAX_OPER_NAME_LENGTH);
            asprintf(&(result->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name),
                    "%s", longname);
            asprintf(&(result->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name),
                    "%s", shortname);
            break;
        case RIL_CELL_INFO_TYPE_LTE:
            result->network_infos[i].CellInfo.lte.cellIdentityLte.mcc = mcc;
            result->network_infos[i].CellInfo.lte.cellIdentityLte.mnc = mnc;
            result->network_infos[i].CellInfo.lte.cellIdentityLte.mnc_len = mnc_len;
            result->network_infos[i].CellInfo.lte.cellIdentityLte.tac = lac;
            getOperatorNamesFromNumericCode(
                    mccmnc, lac, longname, shortname, MAX_OPER_NAME_LENGTH);
            asprintf(&(result->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name),
                    "%s", longname);
            asprintf(&(result->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name),
                    "%s", shortname);
            break;
        case RIL_CELL_INFO_TYPE_TD_SCDMA:
        case RIL_CELL_INFO_TYPE_CDMA:
        case RIL_CELL_INFO_TYPE_NR:
        case RIL_CELL_INFO_TYPE_NONE:
            break;
        }
    }

    urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_NETWORK_SCAN_RESULT,
            m_slot_id, RfxNetworkScanResultData((void*) result, sizeof(RIL_NetworkScanResult)));
    // response to TeleCore
    responseToTelCore(urc);
    isSuccess = true;

    // release any resource, it should be as the same as RfxNetworkScanResultData's destructor
error:
    mPlmnListOngoing = 0;
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING, false);

    RIL_NetworkScanResult * pData = (RIL_NetworkScanResult *) result;
    if (pData->network_infos_length > 0 && pData->network_infos != NULL) {
        int countCell = pData->network_infos_length;
        for (int i = 0; i < countCell; i++) {
            switch(pData->network_infos[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM:
                    if (pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                        free(pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                    if (pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                        free(pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                    break;
                case RIL_CELL_INFO_TYPE_WCDMA:
                    if (pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                        free(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                    if(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                        free(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                    break;
                case RIL_CELL_INFO_TYPE_LTE:
                    if (pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                        free(pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                    if (pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                        free(pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                    break;
                case RIL_CELL_INFO_TYPE_TD_SCDMA:
                case RIL_CELL_INFO_TYPE_CDMA:
                case RIL_CELL_INFO_TYPE_NR:
                case RIL_CELL_INFO_TYPE_NONE:
                    break;
            }
        }
    }
    if (pData->network_infos != NULL) free(pData->network_infos);
    free(pData);

    // If this failure comes from ABORT, sending URC will cause JE.
    // So we send URC only when mPlmnListAbort is false.
    if (isSuccess != true && mPlmnListAbort == 0) {
        RIL_NetworkScanResult* resp = (RIL_NetworkScanResult*) calloc(1, sizeof(RIL_NetworkScanResult));
        if (resp == NULL) {
            logE(LOG_TAG, "handleSimulatedNetworkScan resp==null");
            mPlmnListAbort = 0;
            return;
        }
        memset(resp, 0, sizeof(RIL_NetworkScanResult));
        resp->status = COMPLETE; // for now we don't support PARTIAL.
        urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_NETWORK_SCAN_RESULT,
                m_slot_id, RfxNetworkScanResultData((void*) resp, sizeof(RIL_NetworkScanResult)));
        // response to TeleCore
        responseToTelCore(urc);
        free(resp);
    }
    mPlmnListAbort = 0; /* always clear here to prevent race condition scenario */
}
