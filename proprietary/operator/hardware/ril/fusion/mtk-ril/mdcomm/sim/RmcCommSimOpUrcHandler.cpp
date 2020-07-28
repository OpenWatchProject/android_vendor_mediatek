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
#include "RfxStatusDefs.h"
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RmcSimBaseHandler.h"
#include "RmcCommSimUrcHandler.h"
#include "RmcCommSimOpUrcHandler.h"
#include "RmcCommSimDefs.h"
#include "utils/String8.h"
#include "rfx_properties.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include "RfxMisc.h"
#include "RfxRilUtils.h"
#include <cutils/properties.h>
#include <telephony/mtk_ril.h>

using ::android::String8;

static const char* commOpUrcList[] = {
    "+EATTEVT:",
    "+ESMLRSUEVT:"
};

RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_ATT_SIM_LOCK_NOTIFICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_RSU_EVENT);

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcCommSimOpUrcHandler::RmcCommSimOpUrcHandler(int slot_id, int channel_id) :
        RmcCommSimUrcHandler(slot_id, channel_id) {
    setTag(String8("RmcCommSimOpUrc"));
}

RmcCommSimOpUrcHandler::~RmcCommSimOpUrcHandler() {
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcCommSimOpUrcHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;
    char* ss = msg->getRawUrc()->getLine();

    if (strStartsWith(ss, "+EATTEVT:") ||
            strStartsWith(ss, "+ESMLRSUEVT:")) {
        result = RmcSimBaseHandler::RESULT_NEED;
    } else {
        result = RmcCommSimUrcHandler::needHandle(msg);
    }
    return result;
}

void RmcCommSimOpUrcHandler::handleUrc(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    String8 ss(urc->getLine());

    if (strStartsWith(ss, "+EATTEVT:")) {
        handleAttRsuSimLockEvent(msg, ss);
    } else if (strStartsWith(ss, "+ESMLRSUEVT:")) {
        handleRsuEvent(msg, urc);
    } else {
        RmcCommSimUrcHandler::handleUrc(msg, urc);
    }
}

const char** RmcCommSimOpUrcHandler::queryUrcTable(int *record_num) {
    const char **superTable = RmcCommSimUrcHandler::queryUrcTable(record_num);
    int subRecordNumber = 0;
    int supRecordNumber = *record_num;
    char **bufTable = NULL;
    int i = 0, j = 0;

    subRecordNumber = sizeof(commOpUrcList)/sizeof(char*);
    *record_num = subRecordNumber + supRecordNumber;
    bufTable = (char **)calloc(1, *record_num * sizeof(char*));
    RFX_ASSERT(bufTable != NULL);
    for(i = 0; i < supRecordNumber; i++) {
        asprintf(&(bufTable[i]), "%s", superTable[i]);
    }
    for(j = i; j < *record_num; j++) {
        asprintf(&(bufTable[j]), "%s", commOpUrcList[j - supRecordNumber]);
    }
    return (const char**)bufTable;
}

void RmcCommSimOpUrcHandler::handleAttRsuSimLockEvent(const sp<RfxMclMessage>& msg, String8 urc) {
    int eventId = -1;

    RFX_UNUSED(msg);

    if (strStartsWith(urc, "+EATTEVT: 0")) {  // +EATTEVT
        eventId = 0;
    } else {
        logD(mTag, "[RSU-SIMLOCK] handleAttRsuSimLockEvent: unsupport type");
    }

    logD(mTag, "[RSU-SIMLOCK] handleAttRsuSimLockEvent: eventId = %d", eventId);
    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_ATT_SIM_LOCK_NOTIFICATION,
                    m_slot_id, RfxIntsData(&eventId, 1));
    responseToTelCore(unsol);
}

void RmcCommSimOpUrcHandler::handleRsuEvent(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    int err = 0;
    int opId = 0;
    int eventId = -1;
    char* eventString = NULL;
    char* event[2] = {NULL, NULL};
    RfxAtLine *pLine = urc;

    RFX_UNUSED(msg);

    if (pLine == NULL) {
        return;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0) {
        goto error;
    }

    if (pLine->atTokHasmore()) {
        eventId = pLine->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
        asprintf(&event[0], "%d", eventId);
        if (pLine->atTokHasmore()) {
            eventString = pLine->atTokNextstr(&err);
        }
        asprintf(&event[1], "%s", (eventString == NULL ? "" : eventString));
        sp<RfxMclMessage> p_urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_RSU_EVENT,
                m_slot_id, RfxStringsData(event, 2));
        responseToTelCore(p_urc);
        if (event[0] != NULL) {
            free(event[0]);
        }
        if (event[1] != NULL) {
            free(event[1]);
        }
        return;
    }

error:
    logE(mTag, "handleRsuEvent: error");
}
