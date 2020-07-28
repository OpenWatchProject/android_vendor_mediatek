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

#include "RmcEmbmsURCHandler.h"
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"

static const int events[] = {
    RFX_MSG_EVENT_EMBMS_INITIAL_VARIABLE,
};

#define VDBG 1

RFX_IMPLEMENT_HANDLER_CLASS(RmcEmbmsURCHandler, RIL_CMD_PROXY_URC);

RmcEmbmsURCHandler::RmcEmbmsURCHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
       const char* urc[] = {
        "+EMSRV:",
        "+EMSLU",
        "+EMSAIL",
        "+EMSESS:",
        //"+CEREG:", // Phase out, NW module will get EGREG and bypass
        "+EHVOLTE:"
       };

    memset(&oos_tmgi, 0, sizeof(RIL_EMBMS_LocalOosNotify));
    oos_tmgi_count = 0;

    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
    registerToHandleEvent(events, sizeof(events)/sizeof(int));
}

RmcEmbmsURCHandler::~RmcEmbmsURCHandler() {
}

bool RmcEmbmsURCHandler::isHvolteDisable() {
    bool ret = false;
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("persist.vendor.embms.hvolte.disable", prop, "0");

    if (!strcmp(prop, "1")) {
        logD(LOG_TAG, "Skip hVolte event");
        ret = true;
    }

    return ret;
}

void RmcEmbmsURCHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char* urc = msg->getRawUrc()->getLine();
    if(strStartsWith(urc, "+EMSRV:")) {
        onEmbmsSrvStatus(msg);
    } else if(strStartsWith(urc, "+EMSLU")) {
        onEmbmsSessionListUpdate(msg);
    } else if(strStartsWith(urc, "+EMSAILNF")) {
        onEmbmsSaiListUpdate(msg);
    } else if(strStartsWith(urc, "+EMSESS:")) {
        onEmbmsSessionActiveUpdate(msg);
    //} else if(strStartsWith(urc, "+CEREG:")) {
    //    onEpsNetworkUpdate(msg);
    } else if(strStartsWith(urc, "+EHVOLTE")) {
        if (!isHvolteDisable()) {
            onEmbmsHvolteUpdate(msg);
        }
    }
}

void RmcEmbmsURCHandler::onEmbmsHvolteUpdate(const sp<RfxMclMessage>& msg) {
    // +EHVOLTE: <mode>
    // <mode>: integer.
    // 0 SRLTE mode
    // 1 LTE-only mode (received in SIB-13 MBSFN-AreaInfoList)
    RfxAtLine *line = msg->getRawUrc();
    char* urc = line->getLine();
    if (VDBG) {
        logD(LOG_TAG, "[onEmbmsHvolteUpdate]%s, sendEvent RFX_MSG_EVENT_EMBMS_POST_HVOLTE_UPDATE"
            , urc);
    }
    sendEvent(RFX_MSG_EVENT_EMBMS_POST_HVOLTE_UPDATE, RfxStringData(urc, strlen(urc)),
        RIL_CMD_PROXY_EMBMS, msg->getSlotId());
}

void RmcEmbmsURCHandler::onEpsNetworkUpdate(const sp<RfxMclMessage>& msg) {
    // Note: In order not to effect other module, skip +CREG=1 in requestAtEnableUrcEvents now

    int err;
    int status;
    int result_status;
    RfxAtLine *line = msg->getRawUrc();
    char* urc = line->getLine();

    logD(LOG_TAG, "sendEvent RFX_MSG_EVENT_EMBMS_POST_NETWORK_UPDATE");
    sendEvent(RFX_MSG_EVENT_EMBMS_POST_NETWORK_UPDATE, RfxStringData(urc, strlen(urc)),
        RIL_CMD_PROXY_EMBMS, msg->getSlotId());
}

void RmcEmbmsURCHandler::onEmbmsSessionActiveUpdate(const sp<RfxMclMessage>& msg) {
    int err;
    int status;
    char* session_id;
    char* raw_tmgi;
    int cause = -1;
    int sub_cause;
    int num_sessions;
    int x;
    sp<RfxMclMessage> response;
    RfxAtLine *line = msg->getRawUrc();
    // +EMSESS: <num_sessions>,<x>,<tmgi>,[<session_id>],<status>[,<cause>[,<sub_cause>]]
    // Cause for session deactivation:
    // 0:    Normal deactivation (requested by
    //          AT+EMSESS=0)
    // 1:    Session activation failed
    // 2:    Moved out of MBMS service area that provides
    //          the session
    // 3:    Session lost due to loss of eMBMS coverage
    // 4:    Session lost due to loss of LTE coverage/LTE
    //          deactivation (e.g. CSFB triggered, or
    //          reception lost)

    // Sub-cause for activating session failed (cause=1):
    // 1:   Unknown error
    // 2:   The requested session is already active
    // 3:   MBMS service is disabled
    // 4:   Missing control info. Some or none of the
    //       MCCHs can be acquired
    // 5:   Missing TMGI. All MCCHs were read and TMGI
    //       of interest cannot be found
    // 6:   Out of MBMS coverage
    // 7:   Out of service
    // 8:   The frequency of requested activated session conflicts to the frequency of the current cell
    // 9:   Maximum number of sessions is activated.
    //       Now modem supports 8 concurrent sessions

    logI(LOG_TAG, "[onEmbmsSessionActiveUpdate]%s", line->getLine());

    line->atTokStart(&err);
    if (err < 0) return;

    // <num_sessions>
    num_sessions = line->atTokNextint(&err);
    if (err < 0) return;

    // <x>
    x = line->atTokNextint(&err);
    if (err < 0) return;

    // <tmgi>
    raw_tmgi = line->atTokNextstr(&err);
    if (err < 0) return;

    // <session_id>, might be empty
    session_id = line->atTokNextstr(&err);

    // <status>
    status = line->atTokNextint(&err);
    if (err < 0) return;
    if (VDBG) logD(LOG_TAG, "status = %d", status);

    // <cause>
    if (!status) {
        cause = line->atTokNextint(&err);
        if (err < 0) return;
        if (VDBG) logD(LOG_TAG, "cause = %d", cause);
    }
    // sub cause of session activation failed
    if (cause == 1) {
        sub_cause = line->atTokNextint(&err);
        if (err < 0) return;
        if (VDBG) logD(LOG_TAG, "sub_cause = %d", sub_cause);

        if (sub_cause == 2) {
            // Update session status to succeed.
            status = 1;
        }
    }

    // RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE
    RIL_EMBMS_LocalStartSessionResp start_ss_response;
    memset(&start_ss_response, 0, sizeof(start_ss_response));
    start_ss_response.trans_id = -1;
    start_ss_response.response = -1;
    logD(LOG_TAG, "status:%d, cause:%d", status, cause);

    if (status == 1) {
        start_ss_response.response = EMBMS_SESSION_SUCCESS;
    } else if (cause == 1) {
        int ss_response_reason;
        switch (sub_cause) {
            case 1:
                ss_response_reason = EMBMS_SESSION_UNKNOWN_ERROR;
                break;
            case 2:
                ss_response_reason = EMBMS_SESSION_ALREADY_ACTIVATED;
                break;
            case 3:
                ss_response_reason = EMBMS_SESSION_NOT_ALLOWED_DISABLED;
                break;
            case 4:
                ss_response_reason = EMBMS_SESSION_MISSING_CONTROL_INFO;
                break;
            case 5:
                ss_response_reason = EMBMS_SESSION_MISSING_TMGI;
                break;
            case 6:
                ss_response_reason = EMBMS_SESSION_OUT_OF_COVERAGE;
                break;
            case 7:
                ss_response_reason = EMBMS_SESSION_OUT_OF_SERVICE;
                break;
            case 8:
                ss_response_reason = EMBMS_SESSION_FREQ_CONFLICT;
                break;
            case 9:
                ss_response_reason = EMBMS_SESSION_MAX_NUM;
                break;
            default:
                ss_response_reason = EMBMS_SESSION_UNKNOWN_ERROR;
                break;
        }
        logD(LOG_TAG, "ss_response_reason:%d", ss_response_reason);
        start_ss_response.response = ss_response_reason;
    }

    if (start_ss_response.response != -1) {
        start_ss_response.tmgi_info_valid = 1;
        strncpy(start_ss_response.tmgi, raw_tmgi, EMBMS_MAX_LEN_TMGI);

        response = RfxMclMessage::obtainUrc(RFX_MSG_URC_EMBMS_START_SESSION_RESPONSE,
            msg->getSlotId(),
            RfxEmbmsLocalStartSessionRespData(&start_ss_response, sizeof(start_ss_response)));
        responseToTelCore(response);
    }

    // RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION
    // OOS Response reason:
    // UNICAST_OOS      0: Unicast is out of service
    // MULTICAST_OOS    1: Broadcast is out of service
    // EXIT_OOS         2: Both Unicast and Multicast out of service is cleared
    int response_reason = -1;
    switch (cause) {
        case 4:
            response_reason = EMBMS_UNICAST_OOS;
            break;
        case 3:
        case 2:
            response_reason = EMBMS_MULTICAST_OOS;
            break;
    }

    logD(LOG_TAG, "oos response_reason %d", response_reason);

    if ( x < 1 || x > EMBMS_MAX_NUM_SESSIONINFO ) {
        logE(LOG_TAG, "invalid x = %d !!", x);
    } else if (x == 1) {
        logD(LOG_TAG, "initial data base for x = %d", x);
        memset(&oos_tmgi, 0, sizeof(oos_tmgi));
        oos_tmgi_count = 0;
    }

    if (response_reason != -1) {
        oos_tmgi.reason = response_reason;
        strncpy(oos_tmgi.tmgix[oos_tmgi_count], raw_tmgi, EMBMS_MAX_LEN_TMGI);
        logD(LOG_TAG, "Save oos tmgi[%d] as: %s",
            x, oos_tmgi.tmgix[oos_tmgi_count]);

        oos_tmgi_count++;
        oos_tmgi.tmgi_info_count = oos_tmgi_count;
    }

    // Fire OOS URC only when index == num
    if (x == num_sessions && oos_tmgi_count > 0) {
        response = RfxMclMessage::obtainUrc(RFX_MSG_URC_EMBMS_OOS_NOTIFICATION,
            msg->getSlotId(),
            RfxEmbmsLocalOosNotifyData(&oos_tmgi, sizeof(oos_tmgi)));
        responseToTelCore(response);
        memset(&oos_tmgi, 0, sizeof(oos_tmgi));
        oos_tmgi_count = 0;
    }
}

void RmcEmbmsURCHandler::onEmbmsSaiListUpdate(const sp<RfxMclMessage>& msg) {
    int num;
    int index = 0;;
    int err = 0;
    RfxAtLine *line = msg->getRawUrc();
    logI(LOG_TAG, "[onEmbmsSaiListUpdate]%s", line->getLine());

    // wait for num_nf = x in +EMSAILNF: <num_nf>,<x>,<nfx>...,
    line->atTokStart(&err);
    if (err < 0) return;

    num = line->atTokNextint(&err);
    if (err < 0) return;

    if ( num > 0 ) {
        index = line->atTokNextint(&err);
    } else {
        index = 0;
    }
    if (err < 0) return;

    if (num == index) {
        logD(LOG_TAG, "sendEvent RFX_MSG_EVENT_EMBMS_POST_SAI_UPDATE");
        sendEvent(RFX_MSG_EVENT_EMBMS_POST_SAI_UPDATE, RfxVoidData(), RIL_CMD_PROXY_EMBMS,
            msg->getSlotId());
    }
}

void RmcEmbmsURCHandler::onEmbmsSessionListUpdate(const sp<RfxMclMessage>& msg) {
    /*
    %MBMSEV:<event>
        List of <event>:
        0 : Service change event
    */
    int num;
    int index;
    int err;
    RfxAtLine *line = msg->getRawUrc();
    // logI(LOG_TAG, "[onEmbmsSessionListUpdate]%s", line->getLine());

    // +EMSLUI: <num_sessions>,<x>,<tmgix>,[<session_idx>],<statusx>
    line->atTokStart(&err);
    if (err < 0) return;

    num = line->atTokNextint(&err);
    if (err < 0) return;

    if ( num == 0 ) {
        index = 0;
        logI(LOG_TAG, "[onEmbmsSessionListUpdate]%s", line->getLine());
    } else {
        index = line->atTokNextint(&err);
    }
    if (err < 0) return;

    if (num == index) {
        logI(LOG_TAG, "[onEmbmsSessionListUpdate]%s, sendEvent RFX_MSG_EVENT_EMBMS_POST_SESSION_UPDATE",
            line->getLine());
        sendEvent(RFX_MSG_EVENT_EMBMS_POST_SESSION_UPDATE, RfxVoidData(), RIL_CMD_PROXY_EMBMS,
            msg->getSlotId());
    }
    return;
}

void RmcEmbmsURCHandler::onEmbmsSrvStatus(const sp<RfxMclMessage>& msg) {
    int err;
    int status;
    sp<RfxMclMessage> response;
    RfxAtLine *line = msg->getRawUrc();
    // +EMSRV: <srv>[,<num_area_ids>,<area_id1>[,<area_id2>...]]
    // 0: No service, 1:only unicast available, 2:in eMBMS supporting area 3:e911 4:hVolte 5:flight mode
    // 6: Gemini suspend 7: Virtual suspend
    logI(LOG_TAG, "[onEmbmsSrvStatus]%s", line->getLine());

    line->atTokStart(&err);
    if (err < 0) return;

    status = line->atTokNextint(&err);
    if (err < 0) return;

    if (VDBG) logD(LOG_TAG, "status = %d", status);

    int intdata[1];  // embms_local_coverage_state
    intdata[0] = status;
    if (status < 7) {
        response = RfxMclMessage::obtainUrc(RFX_MSG_URC_EMBMS_COVERAGE_STATE,
            msg->getSlotId(), RfxIntsData(intdata, 1));
        responseToTelCore(response);
    }


    // 0: Unicast OOS
    // 1: Multicast OOS
    // 2: Exit OOS
    if (status == EMBMS_LOCAL_SRV_EMBMS) {
        RIL_EMBMS_LocalOosNotify oos_response;
        memset(&oos_response, 0, sizeof(oos_response));
        oos_response.reason = EMBMS_EXIT_OOS;
        oos_response.tmgi_info_count = 0;

        response = RfxMclMessage::obtainUrc(RFX_MSG_URC_EMBMS_OOS_NOTIFICATION,
            msg->getSlotId(), RfxEmbmsLocalOosNotifyData(&oos_response, sizeof(oos_response)));
        responseToTelCore(response);
    }
}

void RmcEmbmsURCHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    logD(LOG_TAG, "onHandleEvent: %s", idToString(msg->getId()));
    int id = msg->getId();
    switch(id) {
        case RFX_MSG_EVENT_EMBMS_INITIAL_VARIABLE:
            handleInitialVariable(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcEmbmsURCHandler::handleInitialVariable(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    memset(&oos_tmgi, 0, sizeof(RIL_EMBMS_LocalOosNotify));
    oos_tmgi_count = 0;
}
