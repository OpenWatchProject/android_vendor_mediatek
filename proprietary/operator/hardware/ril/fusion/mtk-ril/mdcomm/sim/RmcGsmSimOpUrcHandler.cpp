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
#include "RmcGsmSimOpUrcHandler.h"
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"
#include "RfxTokUtils.h"
#include "RmcCommSimDefs.h"
#include "RfxIntsData.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "usim_fcp_parser.h"
#ifdef __cplusplus
}
#endif


using ::android::String8;

static const char* gsmOpUrcList[] = {
    "+ETMOEVT:",
};

RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, \
        RFX_MSG_URC_SIM_MELOCK_NOTIFICATION);


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcGsmSimOpUrcHandler::RmcGsmSimOpUrcHandler(int slot_id, int channel_id) :
        RmcGsmSimUrcHandler(slot_id, channel_id){
    setTag(String8("RmcGsmSimOpUrcHandler"));
    logD(mTag, "RmcGsmSimOpUrcHandler %d, %d", slot_id, m_channel_id);
}

RmcGsmSimOpUrcHandler::~RmcGsmSimOpUrcHandler() {
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcGsmSimOpUrcHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;
    char* ss = msg->getRawUrc()->getLine();

    if (strStartsWith(ss, "+ETMOEVT:")) {
        result = RmcSimBaseHandler::RESULT_NEED;
    } else {
        result = RmcGsmSimUrcHandler::needHandle(msg);
    }
    return result;
}

void RmcGsmSimOpUrcHandler::handleUrc(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    String8 ss(urc->getLine());

    if (strStartsWith(ss, "+ETMOEVT:")) {
//        handleRemoteSimUnlockEvent(msg, ss);
    } else {
        RmcGsmSimUrcHandler::handleUrc(msg, urc);
    }
}

const char** RmcGsmSimOpUrcHandler::queryUrcTable(int *record_num) {
    const char **superTable = RmcGsmSimUrcHandler::queryUrcTable(record_num);
    int subRecordNumber = 0;
    int supRecordNumber = *record_num;
    char **bufTable = NULL;
    int i = 0, j = 0;

    subRecordNumber = sizeof(gsmOpUrcList)/sizeof(char*);
    *record_num = subRecordNumber + supRecordNumber;
    bufTable = (char **)calloc(1, *record_num * sizeof(char*));
    RFX_ASSERT(bufTable != NULL);
    for(i = 0; i < supRecordNumber; i++) {
        asprintf(&(bufTable[i]), "%s", superTable[i]);
    }
    for(j = i; j < *record_num; j++) {
        asprintf(&(bufTable[j]), "%s", gsmOpUrcList[j - supRecordNumber]);
    }
    return (const char**)bufTable;
}

void RmcGsmSimOpUrcHandler::handleRemoteSimUnlockEvent(const sp<RfxMclMessage>& msg, String8 urc) {
    int eventId = -1;

    RFX_UNUSED(msg);

    if (strStartsWith(urc, "+ETMOEVT: 0")) { // +ETMOEVT
        eventId = 0;
    } else {
        logD(mTag, "handleRemoteSimUnlockEvent: unsupport type");
    }

    logD(mTag, "handleRemoteSimUnlockEvent: eventId = %d", eventId);
    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_SIM_MELOCK_NOTIFICATION,
                    m_slot_id, RfxIntsData(&eventId, 1));
    responseToTelCore(unsol);
}
