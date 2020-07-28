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

#include "RmcSuppServUssdHandler.h"
#include "RfxBaseHandler.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxStringsData.h"
#include "SSUtil.h"
#include "GsmUtil.h"
#include "SSConfig.h"
#include "SuppServDef.h"

#include <telephony/mtk_ril.h>
#include <mtkconfigutils.h>
#include <libmtkrilutils.h>
#include <mtk_properties.h>
#include <string.h>
#include <unistd.h>

RmcSuppServUssdHandler::RmcSuppServUssdHandler(int slot_id, int channel_id) :
    RmcSuppServUssdBaseHandler(slot_id, channel_id) {
    // do nothing
}

RmcSuppServUssdHandler::~RmcSuppServUssdHandler() {
    // do nothing
}

void RmcSuppServUssdHandler::requestSendUssdDomainSelect(const sp<RfxMclMessage>& msg) {
    // For Gen97 or later version of modem platform, no need to do USSD domain selection
    requestSendUSSD(msg);
}

void RmcSuppServUssdHandler::requestCancelUssdDomainSelect(const sp<RfxMclMessage>& msg) {
    // For Gen97 or later version of modem platform, no need to do USSD domain selection
    requestCancelUssd(msg);
}

void RmcSuppServUssdHandler::requestSendUSSD(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err = 0;
    int length = 0;
    char* cmd = NULL;
    char* p_tmpStr = NULL;
    char* p_ussdRequest = NULL;
    char* p_input_ussdRequest = (char*) msg->getData()->getData();
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    String8 currentMccmnc = getMclStatusManager()->
            getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC, String8("0"));

    // Check input USSD string first
    if (p_input_ussdRequest == NULL || strlen(p_input_ussdRequest) == 0) {
        logE(TAG, "requestSendUSSD:p_ussdRequest null or empty.");
        goto error;
    }

    // Convet USSD string from UTF-8 to UCS2
    p_ussdRequest = convertToUCS2(p_input_ussdRequest);
    if (p_ussdRequest == NULL) {
        logE(TAG, "requestSendUSSD:p_ussdRequest malloc fail");
        goto error;
    }

    /**
     * USSD string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
     * We convert input char to unicode hex string and store it to p_ussdRequest.
     * For example, convert input "1" to "3100"; So len of p_ussdRequest is 4 times of input
     */
    length = strlen(p_ussdRequest) / 4;
    if (length > MAX_RIL_USSD_NUMBER_LENGTH) {
        logW(TAG, "USSD stringlen = %d, max = %d", length, MAX_RIL_USSD_NUMBER_LENGTH);
        length = MAX_RIL_USSD_NUMBER_LENGTH;
    }

    p_tmpStr = (char*) calloc(1, (4 * length + 1));
    if(p_tmpStr == NULL) {
        logE(TAG, "Malloc fail");
        free((char *)p_ussdRequest);
        goto error;
    }
    memcpy(p_tmpStr, p_ussdRequest, 4 * length);

    /**
     * AT+ECUSD=<m>,<n>,<string>,<dcs>,<lang>[,<from>]
     *     <m>: 1 for SS, 2 for USSD
     *     <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     *     <string>: USSD string
     *     <dca>: data coding scheme (72: UCS2)
     *     <lang>: default value is "en", and empty string means without language information
     *     <from> : the from info (phone number) for TMO US Digits USSD (optional)
     */
    if (SSConfig::ussiWithNoLang(currentMccmnc.string())) {
        asprintf(&cmd, "AT+ECUSD=2,1,\"%s\",72,\"\"", p_tmpStr);
    } else {
        asprintf(&cmd, "AT+ECUSD=2,1,\"%s\",72,\"en\"", p_tmpStr);
    }

    p_response = atSendCommand(cmd);

    free(cmd);
    free(p_tmpStr);
    free(p_ussdRequest);

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

    // Sometimes, MD would not return fail with this kind of format: "+CME ERROR: XXX"
    // Make a convert if receive "ERROR"
    if (strStartsWith(p_response->getFinalResponse()->getLine(), "ERROR")) {
        logE(TAG, "requestSendUSSD Error.");
        ret = RIL_E_INTERNAL_ERR;
    }

error:
    if (ret == RIL_E_SUCCESS) {
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
        responseToTelCore(response);
    } else {
        sendFailureReport(msg, ret);
    }
}

void RmcSuppServUssdHandler::requestCancelUssd(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;

    /**
     * AT+ECUSD=<m>,<n>,<string>,<dcs>,<lang>,<from>
     *     <m>: 1 for SS, 2 for USSD
     *     <n>: 1 for execute SS or USSD, 2 for cancel USSD session
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

void RmcSuppServUssdHandler::handleOnUssd(const sp<RfxMclMessage>& msg) {
    logD(TAG, "handleOnUssd, from UrcHandler");
    sp<RfxMclMessage> urc;

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_ON_USSD, m_slot_id,
            RfxStringsData(msg->getData()->getData(), msg->getData()->getDataLength()));
    responseToTelCore(urc);
}

void RmcSuppServUssdHandler::handleOnUssi(const sp<RfxMclMessage>& msg) {
    logE(TAG, "handleOnUssi, from UrcHandler, should not be here");
    UNUSED(msg);
}

