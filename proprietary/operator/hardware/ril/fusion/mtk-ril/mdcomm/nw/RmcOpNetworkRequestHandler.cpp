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

#include "RmcOpNetworkRequestHandler.h"

static const int request[] = {
    RFX_MSG_REQUEST_VSS_ANTENNA_CONF,
    RFX_MSG_REQUEST_VSS_ANTENNA_INFO
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_VSS_ANTENNA_CONF);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_VSS_ANTENNA_INFO);

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpNetworkRequestHandler, RIL_CMD_PROXY_3);

RmcOpNetworkRequestHandler::RmcOpNetworkRequestHandler(int slot_id, int channel_id) :
        RmcNetworkHandler(slot_id, channel_id) {
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    logV(LOG_TAG, "[RmcOpNetworkRequestHandler] init");
}

RmcOpNetworkRequestHandler::~RmcOpNetworkRequestHandler() {
}

void RmcOpNetworkRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(LOG_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(msg->getId()));
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_VSS_ANTENNA_CONF:
            requestAntennaConf(msg);
            break;
        case RFX_MSG_REQUEST_VSS_ANTENNA_INFO:
            requestAntennaInfo(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcOpNetworkRequestHandler::requestAntennaConf(const sp<RfxMclMessage>& msg) {
    int antennaType, err;
    int response[2] = { 0 };
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int *pInt = (int *)msg->getData()->getData();

    antennaType = pInt[0];
    response[0] = antennaType;
    response[1] = 0; // failed

    logD(LOG_TAG, "Enter requestAntennaConf(), antennaType = %d ", antennaType);
    // AT command format as below : (for VZ_REQ_LTEB13NAC_6290)
    // AT+ERFTX=8, <type>[,<param1>,<param2>]
    // <param1> is decoded as below:
    //    1 - Normal dual receiver operation(default UE behaviour)
    //    2 - Single receiver operation 'enable primary receiver only'(disable secondary/MIMO receiver)
    //    3 - Single receiver operation 'enable secondary/MIMO receiver only (disable primary receiver)
    switch(antennaType){
        case 0:    // 0: signal information is not available on all Rx chains
            antennaType = 0;
            break;
        case 1:    // 1: Rx diversity bitmask for chain 0
            antennaType = 2;
            break;
        case 2:    // 2: Rx diversity bitmask for chain 1 is available
            antennaType = 3;
            break;
        case 3:    // 3: Signal information on both Rx chains is available.
            antennaType = 1;
            break;
        default:
            logE(LOG_TAG, "requestAntennaConf: configuration is an invalid");
            break;
    }
    p_response = atSendCommand(String8::format("AT+ERFTX=8,1,%d", antennaType));
    if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
        if (antennaType == 0) {
            // This is special handl for disable all Rx chains
            // <param1>=0 - signal information is not available on all Rx chains
            ril_errno = RIL_E_SUCCESS;
            response[1] = 1;  // success
            antennaTestingType = antennaType;
        }
    } else {
        ril_errno = RIL_E_SUCCESS;
        response[1] = 1; // success
        // Keep this settings for query antenna info.
        antennaTestingType = antennaType;
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxIntsData(response, 2), msg, false);
    responseToTelCore(resp);
}
void RmcOpNetworkRequestHandler::requestAntennaInfo(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    int param1, param2, err, skip;
    int response[6] = { 0 };
    memset(response, 0, sizeof(response));
    int *primary_antenna_rssi   = &response[0];
    int *relative_phase         = &response[1];
    int *secondary_antenna_rssi = &response[2];
    int *phase1                 = &response[3];
    int *rxState_0              = &response[4];
    int *rxState_1              = &response[5];
    *primary_antenna_rssi   = 0;  // <primary_antenna_RSSI>
    *relative_phase         = 0;  // <relative_phase>
    *secondary_antenna_rssi = 0;  // <secondary_antenna_RSSI>
    *phase1                 = 0;  // N/A
    *rxState_0              = 0;  // rx0 status(0: not vaild; 1:valid)
    *rxState_1              = 0;  // rx1 status(0: not vaild; 1:valid)
    // AT+ERFTX=8, <type> [,<param1>,<param2>]
    // <type>=0 is used for VZ_REQ_LTEB13NAC_6290
    // <param1> represents the A0 bit in ANTENNA INFORMATION REQUEST message
    // <param2> represents the A1 bit in ANTENNA INFORMATION REQUEST message
    switch(antennaTestingType) {
        case 0:    // signal information is not available on all Rx chains
            param1 = 0;
            param2 = 0;
            break;
        case 1:    // Normal dual receiver operation (default UE behaviour)
            param1 = 1;
            param2 = 1;
            break;
        case 2:    // enable primary receiver only
            param1 = 1;
            param2 = 0;
            break;
        case 3:    // enable secondary/MIMO receiver only
            param1 = 0;
            param2 = 1;
            break;
        default:
            logE(LOG_TAG, "requestAntennaInfo: configuration is an invalid, antennaTestingType: %d", antennaTestingType);
            goto error;
    }
    logD(LOG_TAG, "requestAntennaInfo: antennaType=%d, param1=%d, param2=%d", antennaTestingType, param1, param2);
    if (antennaTestingType == 0) {
        p_response = atSendCommand(String8::format("AT+ERFTX=8,0,%d,%d", param1, param2));
        if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
        resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
                RfxIntsData(response, 6), msg, false);
        responseToTelCore(resp);
        return;
    }
    // set antenna testing type
    p_response = atSendCommand(String8::format("AT+ERFTX=8,1,%d", antennaTestingType));
    if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
        p_response = atSendCommandSingleline(String8::format("AT+ERFTX=8,0,%d,%d", param1, param2),
                "+ERFTX:");
        if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
            // handle intermediate
            line = p_response->getIntermediates();
            // go to start position
            line->atTokStart(&err);
            if (err < 0) goto error;
            // skip <op=8>
            skip = line->atTokNextint(&err);
            if (err < 0) goto error;
            // skip <type=0>
            skip = line->atTokNextint(&err);
            if (err < 0) goto error;
            (*primary_antenna_rssi) = line->atTokNextint(&err);
            if (err < 0) {
                // response for AT+ERFTX=8,0,0,1
                // Ex: +ERFTX: 8,0,,100
            } else {
                // response for AT+ERFTX=8,0,1,1 or AT+ERFTX=8,0,1,0
                // Ex: +ERFTX: 8,0,100,200,300 or +ERFTX: 8,0,100
                *rxState_0 = 1;
            }
            if (line->atTokHasmore()) {
                (*secondary_antenna_rssi) = line->atTokNextint(&err);
                if (err < 0) {
                    logE(LOG_TAG, "ERROR occurs <secondary_antenna_rssi> form antenna info request");
                    goto error;
                } else {
                    // response for AT+ERFTX=8,0,1,0
                    // Ex: +ERFTX: 8,0,100
                    *rxState_1 = 1;
                }
                if (line->atTokHasmore()) {
                    // response for AT+ERFTX=8,0,1,1
                    // Ex: +ERFTX: 8,0,100,200,300
                    (*relative_phase) = line->atTokNextint(&err);
                    if (err < 0) {
                        logE(LOG_TAG, "ERROR occurs <relative_phase> form antenna info request");
                        goto error;
                    }
                }
            }
            ril_errno = RIL_E_SUCCESS;
        }
    } else {
        logE(LOG_TAG, "Set antenna testing type getting ERROR");
        goto error;
    }
error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxIntsData(response, 6), msg, false);
    responseToTelCore(resp);
}
